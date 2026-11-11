#include "core.hpp"
#include <stdexcept>
#include "core/utils/vpss_helper.h"
#include "demangle.hpp"
#include "error_msg.hpp"
#include <vfs.h>
// #define OUTPUT_SRC_FRAME

// #ifdef OUTPUT_SRC_FRAME
// // #define STB_IMAGE_IMPLEMENTATION
// #include "stb/stb_image.h"
// // #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb/stb_image_write.h"
// #endif
#include <iostream>
#include <sstream>

namespace cviai {

Core::Core(CVI_MEM_TYPE_E input_mem_type) {
  // mp_mi = std::make_unique<CvimodelInfo>();
  // mp_mi->conf = {.debug_mode = false, .input_mem_type = input_mem_type};
  mp_mi = std::unique_ptr<CvimodelInfo>(new CvimodelInfo);
  mp_mi->conf.debug_mode = false;
  mp_mi->conf.input_mem_type = input_mem_type;

}

Core::Core() : Core(CVI_MEM_SYSTEM) {}

#define CLOSE_MODEL_IF_FAILED(x, errmsg) \
  do {                                   \
    if ((x) != CVIAI_SUCCESS) {          \
      LOGE(AISDK_TAG, errmsg "\n");                 \
      modelClose();                      \
      return CVIAI_ERR_OPEN_MODEL;       \
    }                                    \
  } while (0)

#define CLOSE_MODEL_IF_TPU_FAILED(x, errmsg)         \
  do {                                               \
    if (int ret = (x)) {                             \
      LOGE(AISDK_TAG, errmsg ": %s\n", get_tpu_error_msg(ret)); \
      modelClose();                                  \
      return CVIAI_ERR_OPEN_MODEL;                   \
    }                                                \
  } while (0)

int Core::modelOpen(const char *filepath) {
  LOGI(AISDK_TAG, "TRACE Core::modelOpen(0)");
  LOGI(AISDK_TAG, "filepath: %s", filepath);
  if (!mp_mi) {
    LOGE(AISDK_TAG, "config not set\n");
    return CVIAI_ERR_OPEN_MODEL;
  }

  if (mp_mi->handle != nullptr) {
    LOGE(AISDK_TAG, "failed to open model: \"%s\" has already opened.\n", filepath);
    return CVIAI_FAILURE;
  }
  m_model_file = filepath;
  CLOSE_MODEL_IF_TPU_FAILED(CVI_NN_RegisterModel(filepath, &mp_mi->handle),
                            "CVI_NN_RegisterModel failed");

  CVI_NN_SetConfig(mp_mi->handle, OPTION_OUTPUT_ALL_TENSORS,
                   static_cast<int>(mp_mi->conf.debug_mode));

  CLOSE_MODEL_IF_TPU_FAILED(
      CVI_NN_GetInputOutputTensors(mp_mi->handle, &mp_mi->in.tensors, &mp_mi->in.num,
                                   &mp_mi->out.tensors, &mp_mi->out.num),
      "CVI_NN_GetINputsOutputs failed");

  setupTensorInfo(mp_mi->in.tensors, mp_mi->in.num, &m_input_tensor_info);
  setupTensorInfo(mp_mi->out.tensors, mp_mi->out.num, &m_output_tensor_info);

  CVI_TENSOR *input =
      CVI_NN_GetTensorByName(CVI_NN_DEFAULT_TENSOR, mp_mi->in.tensors, mp_mi->in.num);
  // Assigning default values.
  std::vector<InputPreprecessSetup> data(mp_mi->in.num);
  for (uint32_t i = 0; i < (uint32_t)mp_mi->in.num; i++) {
    CVI_TENSOR *tensor = mp_mi->in.tensors + i;
    float quant_scale = CVI_NN_TensorQuantScale(tensor);
    data[i].use_quantize_scale = quant_scale == 0 ? false : true;
  }

  CLOSE_MODEL_IF_FAILED(setupInputPreprocess(&data), "Failed to setup preprocess setting.");
  CLOSE_MODEL_IF_FAILED(onModelOpened(), "return failed in onModelOpened");

  m_vpss_config.clear();
  for (uint32_t i = 0; i < (uint32_t)mp_mi->in.num; i++) {
    if (data[i].use_quantize_scale) {
      CVI_TENSOR *tensor = mp_mi->in.tensors + i;
      float quant_scale = CVI_NN_TensorQuantScale(tensor);
      for (uint32_t j = 0; j < 3; j++) {
        data[i].factor[j] *= quant_scale;
        data[i].mean[j] *= quant_scale;
      }
      // FIXME: Behavior will changed in 1822.
      float factor_limit = 8191.f / 8192;
      for (uint32_t j = 0; j < 3; j++) {
        if (data[i].factor[j] > factor_limit) {
          LOGW(AISDK_TAG, "factor[%d] is bigger than limit: %f\n", i, data[i].factor[j]);
          data[i].factor[j] = factor_limit;
        }
      }
    }
    VPSSConfig vcfg;
    int32_t width, height;
    // FIXME: Future support for nhwc input. Currently disabled.
    if (false) {
      width = input->shape.dim[2];
      height = input->shape.dim[1];
      vcfg.frame_type = CVI_FRAME_PACKAGE;
    } else {
      width = input->shape.dim[3];
      height = input->shape.dim[2];
      vcfg.frame_type = CVI_FRAME_PLANAR;
    }
    vcfg.rescale_type = data[i].rescale_type;
    vcfg.crop_attr.bEnable = data[i].use_crop;

    VPSS_CHN_SQ_HELPER(&vcfg.chn_attr, width, height, data[i].format, data[i].factor, data[i].mean,
                       data[i].pad_reverse);
    if (!data[i].keep_aspect_ratio) {
      vcfg.chn_attr.stAspectRatio.enMode = ASPECT_RATIO_NONE;
    }
    vcfg.chn_coeff = data[i].resize_method;
    m_vpss_config.push_back(vcfg);
  }

  return CVIAI_SUCCESS;
}

void Core::setupTensorInfo(CVI_TENSOR *tensor, int32_t num_tensors,
                           std::map<std::string, TensorInfo> *tensor_info) {
  for (int32_t i = 0; i < num_tensors; i++) {
    TensorInfo tinfo;
    memset((void*)&tinfo,0,sizeof(TensorInfo));
    tinfo.tensor_handle = tensor + i;
    tinfo.tensor_name = CVI_NN_TensorName(tinfo.tensor_handle);
    tinfo.shape = CVI_NN_TensorShape(tinfo.tensor_handle);
    tinfo.raw_pointer = CVI_NN_TensorPtr(tinfo.tensor_handle);
    tinfo.tensor_elem = CVI_NN_TensorCount(tinfo.tensor_handle);
    tinfo.tensor_size = CVI_NN_TensorSize(tinfo.tensor_handle);
    tinfo.qscale = CVI_NN_TensorQuantScale(tinfo.tensor_handle);
    tensor_info->insert(std::pair<std::string, TensorInfo>(tinfo.tensor_name, tinfo));
  }
}

int Core::modelClose() {
  int ret = CVIAI_SUCCESS;

  if (mp_mi->handle != nullptr) {
    ret = CVI_NN_CleanupModel(mp_mi->handle);
    if (ret != CVI_RC_SUCCESS) {  // NOLINT
      LOGE(AISDK_TAG, "CVI_NN_CleanupModel failed: %s\n", get_tpu_error_msg(ret));
      mp_mi->handle = nullptr;
      onModelClosed();
      return CVIAI_ERR_CLOSE_MODEL;
    }
    mp_mi->handle = nullptr;
  }
  onModelClosed();
  return ret;
}

CVI_TENSOR *Core::getInputTensor(int idx) {
  if (idx >= mp_mi->in.num) {
    return NULL;
  }
  return mp_mi->in.tensors + idx;
}

CVI_TENSOR *Core::getOutputTensor(int idx) {
  if (idx >= mp_mi->out.num) {
    return NULL;
  }
  return mp_mi->out.tensors + idx;
}

const TensorInfo &Core::getOutputTensorInfo(const std::string &name) {
  if (m_output_tensor_info.find(name) != m_output_tensor_info.end()) {
    return m_output_tensor_info[name];
  }
  throw std::invalid_argument("cannot find output tensor name: " + name);
}

const TensorInfo &Core::getInputTensorInfo(const std::string &name) {
  if (m_input_tensor_info.find(name) != m_input_tensor_info.end()) {
    return m_input_tensor_info[name];
  }
  throw std::invalid_argument("cannot find input tensor name: " + name);
}

const TensorInfo &Core::getOutputTensorInfo(size_t index) {
  size_t cur = 0;
  for (auto iter = m_output_tensor_info.begin(); iter != m_output_tensor_info.end(); iter++) {
    if (cur == index) {
      return iter->second;
    }
    cur++;
  }
  throw std::out_of_range("out of range");
}

const TensorInfo &Core::getInputTensorInfo(size_t index) {
  size_t cur = 0;
  for (auto iter = m_input_tensor_info.begin(); iter != m_input_tensor_info.end(); iter++) {
    if (cur == index) {
      return iter->second;
    }
    cur++;
  }
  throw std::out_of_range("out of range");
}

size_t Core::getNumInputTensor() const { return static_cast<size_t>(mp_mi->in.num); }

size_t Core::getNumOutputTensor() const { return static_cast<size_t>(mp_mi->out.num); }

#ifdef ENABLE_IVE
int Core::setIveInstance(IVE_HANDLE handle) {
  ive_handle = handle;
  return CVIAI_SUCCESS;
}
#endif

int Core::setVpssTimeout(uint32_t timeout) {
  m_vpss_timeout = timeout;
  return CVIAI_SUCCESS;
}

int Core::setVpssEngine(VpssEngine *engine) {
  mp_vpss_inst = engine;
  return CVIAI_SUCCESS;
}

void Core::skipVpssPreprocess(bool skip) { m_skip_vpss_preprocess = skip; }

int Core::getChnConfig(const uint32_t width, const uint32_t height, const uint32_t idx,
                       cvai_vpssconfig_t *chn_config) {
  if (!allowExportChannelAttribute()) {
    LOGE(AISDK_TAG, "This model does not support exporting channel attributes.\n");
    return CVIAI_ERR_GET_VPSS_CHN_CONFIG;
  }

  if (!isInitialized()) {
    LOGE(AISDK_TAG, 
        "Model is not yet opened. Please call CVI_AI_OpenModel to initialize model before getting "
        "channel config.\n");
    return CVIAI_ERR_NOT_YET_INITIALIZED;
  }

  if (idx >= (uint32_t)mp_mi->in.num) {
    LOGE(AISDK_TAG, "Input index exceed input tensor num.\n");
    return CVIAI_ERR_GET_VPSS_CHN_CONFIG;
  }

  if (!m_skip_vpss_preprocess) {
    LOGW(AISDK_TAG, "VPSS preprocessing is enabled. Remember to skip vpss preprocess.\n");
  }

  switch (m_vpss_config[idx].rescale_type) {
    case RESCALE_CENTER: {
      chn_config->chn_attr = m_vpss_config[idx].chn_attr;
    } break;
    case RESCALE_RB: {
      CVI_TENSOR *input = mp_mi->in.tensors + idx;
      auto &factor = m_vpss_config[idx].chn_attr.stNormalize.factor;
      auto &mean = m_vpss_config[idx].chn_attr.stNormalize.mean;
      VPSS_CHN_SQ_RB_HELPER(&chn_config->chn_attr, width, height, input->shape.dim[3],
                            input->shape.dim[2], m_vpss_config[idx].chn_attr.enPixelFormat, factor,
                            mean, false);
      chn_config->chn_attr.stAspectRatio.u32BgColor =
          m_vpss_config[idx].chn_attr.stAspectRatio.u32BgColor;
    } break;
    default: {
      LOGE(AISDK_TAG, "Unsupported rescale type.\n");
      return CVIAI_ERR_GET_VPSS_CHN_CONFIG;
    } break;
  }
  chn_config->chn_coeff = m_vpss_config[idx].chn_coeff;
  return CVIAI_SUCCESS;
}

void Core::setModelThreshold(float threshold) { m_model_threshold = threshold; }
float Core::getModelThreshold() { return m_model_threshold; };
bool Core::isInitialized() { return mp_mi->handle == nullptr ? false : true; }

int Core::setupInputPreprocess(std::vector<InputPreprecessSetup> *data) { return CVIAI_SUCCESS; }

CVI_SHAPE Core::getInputShape(size_t index) { return getInputTensorInfo(index).shape; }

CVI_SHAPE Core::getOutputShape(size_t index) { return getOutputTensorInfo(index).shape; }

CVI_SHAPE Core::getInputShape(const std::string &name) { return getInputTensorInfo(name).shape; }

CVI_SHAPE Core::getOutputShape(const std::string &name) { return getOutputTensorInfo(name).shape; }

float Core::getInputQuantScale(size_t index) {
  return CVI_NN_TensorQuantScale(getInputTensorInfo(index).tensor_handle);
}

float Core::getInputQuantScale(const std::string &name) {
  return CVI_NN_TensorQuantScale(getInputTensorInfo(name).tensor_handle);
}

size_t Core::getOutputTensorElem(size_t index) { return getOutputTensorInfo(index).tensor_elem; }

size_t Core::getOutputTensorElem(const std::string &name) {
  return getOutputTensorInfo(name).tensor_elem;
}

size_t Core::getInputTensorElem(size_t index) { return getInputTensorInfo(index).tensor_elem; }

size_t Core::getInputTensorElem(const std::string &name) {
  return getInputTensorInfo(name).tensor_elem;
}

size_t Core::getOutputTensorSize(size_t index) { return getOutputTensorInfo(index).tensor_size; }

size_t Core::getOutputTensorSize(const std::string &name) {
  return getOutputTensorInfo(name).tensor_size;
}

size_t Core::getInputTensorSize(size_t index) { return getInputTensorInfo(index).tensor_size; }

size_t Core::getInputTensorSize(const std::string &name) {
  return getInputTensorInfo(name).tensor_size;
}

int Core::vpssPreprocess(VIDEO_FRAME_INFO_S *srcFrame, VIDEO_FRAME_INFO_S *dstFrame,
                         VPSSConfig &vpss_config) {
  int ret;
  if (!vpss_config.crop_attr.bEnable) {
    ret = mp_vpss_inst->sendFrame(srcFrame, &vpss_config.chn_attr, &vpss_config.chn_coeff, 1);
  } else {
    ret = mp_vpss_inst->sendCropChnFrame(srcFrame, &vpss_config.crop_attr, &vpss_config.chn_attr,
                                         &vpss_config.chn_coeff, 1);
  }
  if (ret != CVI_SUCCESS) {
    LOGE(AISDK_TAG, "Send frame failed with %s!\n", get_vpss_error_msg(ret));
    return CVIAI_ERR_VPSS_SEND_FRAME;
  }

  ret = mp_vpss_inst->getFrame(dstFrame, 0, m_vpss_timeout);
  if (ret != CVI_SUCCESS) {
    LOGE(AISDK_TAG, "Send frame failed with %s!\n", get_vpss_error_msg(ret));
    return CVIAI_ERR_VPSS_GET_FRAME;
  }
  return CVIAI_SUCCESS;
}

int Core::run(std::vector<VIDEO_FRAME_INFO_S *> &frames) {
  int ret = CVIAI_SUCCESS;

  if (m_skip_vpss_preprocess && !allowExportChannelAttribute()) {
    LOGE(AISDK_TAG, 
        "cannot skip vpss preprocessing for model: %s, please set false to "
        "CVI_AI_SetSkipVpssPreprocess\n",
        demangle::type_no_scope(*this).c_str());
    return CVIAI_ERR_INVALID_ARGS;
  }

  std::vector<std::shared_ptr<VIDEO_FRAME_INFO_S>> dstFrames;

#ifdef ENABLE_DEBUGGER
  m_debugger.newSession(demangle::type_no_scope(*this));
  m_debugger.save_field("skip_vpss_preprocess", ((uint8_t)m_skip_vpss_preprocess));
  m_debugger.save_field("model_file", m_model_file.c_str(), {m_model_file.size()});
  m_debugger.save_field("input_mem_type", (uint8_t)mp_mi->conf.input_mem_type);
#endif

  if (mp_mi->conf.input_mem_type == CVI_MEM_DEVICE) {
    if (m_skip_vpss_preprocess) {
      // skip vpss preprocess is true, just register frame directly.
      ret = registerFrame2Tensor(frames);
    } else {
      if (m_vpss_config.size() != frames.size()) {
        LOGE(AISDK_TAG, "The size of vpss config does not match the number of frames. (%zu vs %zu)\n",
             m_vpss_config.size(), frames.size());
        return CVIAI_ERR_INFERENCE;
      }

      dstFrames.reserve(frames.size());
      for (uint32_t i = 0; i < frames.size(); i++) {
        VIDEO_FRAME_INFO_S *f = new VIDEO_FRAME_INFO_S;
#ifdef ENABLE_DEBUGGER
        m_debugger.save_origin_frame(frames[i], mp_mi->in.tensors + i);
#endif

        memset(f, 0, sizeof(VIDEO_FRAME_INFO_S));
        int vpssret = vpssPreprocess(frames[i], f, m_vpss_config[i]);
        f->stVFrame.pu8VirAddr[0] = (uint8_t *) f->stVFrame.u64PhyAddr[0];
        f->stVFrame.pu8VirAddr[1] = (uint8_t *) f->stVFrame.u64PhyAddr[1];
        f->stVFrame.pu8VirAddr[2] = (uint8_t *) f->stVFrame.u64PhyAddr[2];
        if (vpssret != CVIAI_SUCCESS) {
          // if preprocess fail, just delete frame.
          if (f->stVFrame.u64PhyAddr[0] != 0) {
            mp_vpss_inst->releaseFrame(f, 0);
          }
          delete f;
          return vpssret;
        } else {
          dstFrames.push_back(
              std::shared_ptr<VIDEO_FRAME_INFO_S>({f, [this](VIDEO_FRAME_INFO_S *f) {
                                                     this->mp_vpss_inst->releaseFrame(f, 0);
                                                     delete f;
                                                   }}));
        }
      }
      // char szname[128];
      // sprintf(szname,"/mnt/sd/crop_%u.bin",dstFrames[0]->stVFrame.u32Width);
      // dump_vpss_frame_to_file(szname,dstFrames[0].get());
      ret = registerFrame2Tensor(dstFrames);
    }
  }

  if (ret == CVIAI_SUCCESS) {
    
    
    int rcret = CVI_NN_Forward(mp_mi->handle, mp_mi->in.tensors, mp_mi->in.num, mp_mi->out.tensors,
                               mp_mi->out.num);

    if (rcret == CVI_RC_SUCCESS) {
      // save debuginfo
#ifdef ENABLE_DEBUGGER
      for (int32_t i = 0; i < mp_mi->in.num; i++) {
        CVI_TENSOR *tensor = mp_mi->in.tensors + i;
        m_debugger.save_tensor(tensor, getInputRawPtr<void>(0));

        // save normalizer only if model needs vpss precprcossing
        if (!m_skip_vpss_preprocess && mp_mi->conf.input_mem_type == CVI_MEM_DEVICE) {
          m_debugger.save_normalizer(tensor, m_vpss_config[i].chn_attr.stNormalize);
        }
      }
#endif
    } else {
      LOGE(AISDK_TAG, "NN forward failed: %s\n", get_tpu_error_msg(rcret));
      ret = CVIAI_ERR_INFERENCE;
    }
  }

  return ret;
}
int Core::dump_vpss_frame_to_file(const char *sz_file, VIDEO_FRAME_INFO_S *frame){
  int fd = aos_open(sz_file,  0100 | 02 | 01000);
  CVI_S32 s32Ret = CVI_SUCCESS;
	if (fd <= 0) {
		printf("aos_open dst file failed\n");
		return CVI_FAILURE;
	}
  printf("to dump vpss frame,pixel format:%d,width:%u,height:%u\n",
        frame->stVFrame.enPixelFormat,frame->stVFrame.u32Width,frame->stVFrame.u32Height);
  aos_write(fd,&frame->stVFrame.u32Width,sizeof(uint32_t));
  aos_write(fd,&frame->stVFrame.u32Height,sizeof(uint32_t));

	for (int i = 0; i < 3; ++i) {
    size_t u32DataLen = frame->stVFrame.u32Stride[i] * frame->stVFrame.u32Height;
    if (u32DataLen == 0)
                    continue;
    if (i > 0 && ((frame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
                    (frame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
                    (frame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
                    u32DataLen >>= 1;

    printf("plane(%d): paddr(%ld) vaddr(%p) stride(%d),data_len(%ld) plane_len(%d)\n",
                            i, frame->stVFrame.u64PhyAddr[i],
                            frame->stVFrame.pu8VirAddr[i],
                            frame->stVFrame.u32Stride[i],
                            u32DataLen, frame->stVFrame.u32Length[i]);

    s32Ret = aos_write(fd, (CVI_U8 *)frame->stVFrame.u64PhyAddr[i], u32DataLen);
    if (s32Ret <= 0) {
            printf("fwrite data(%d) error\n", i);
            break;
    }
	}
	aos_sync(fd);
	aos_close(fd);
  return s32Ret;
}

int dump_tensor_impl(const std::string &str_dir,CVI_TENSOR *p_tensor){
  printf("tensor_address:%p\n",CVI_NN_TensorPtr(p_tensor));
  return 0;
  std::string str_dstf = str_dir + std::string("/") + std::string(p_tensor->name)+std::string(".bin");
  int fd = aos_open(str_dstf.c_str(),  0100 | 02 | 01000);
  // CVI_S32 s32Ret = CVI_SUCCESS;
	if (fd <= 0) {
		printf("aos_open dst file failed\n");
		return CVI_FAILURE;
	}
  printf("to dump vpss frame,channel:%d,height:%d,width:%d\n",
        p_tensor->shape.dim[1],p_tensor->shape.dim[2],p_tensor->shape.dim[3]);
  aos_write(fd,CVI_NN_TensorPtr(p_tensor),CVI_NN_TensorSize(p_tensor));
  aos_sync(fd);
	aos_close(fd);
  return CVI_SUCCESS;
}
int Core::dump_output_tensor(const char *sz_dst_dir){
  for(int i = 0; i < mp_mi->out.num;i++){
    dump_tensor_impl(std::string(sz_dst_dir),mp_mi->out.tensors+i);
  }
  return 0;
}
template <typename T>
int Core::registerFrame2Tensor(std::vector<T> &frames) {
  std::vector<uint64_t> paddrs;
  for (uint32_t i = 0; i < (uint32_t)frames.size(); i++) {
    T frame = frames[i];
#if 0
    printf("frames[%u] height[%u], width[%u], stride[%u],datasize:%lu\n", i, 
           frame->stVFrame.u32Height, frame->stVFrame.u32Width, frame->stVFrame.u32Stride[0], 
           CVI_NN_TensorSize(mp_mi->in.tensors));
#endif
    switch (frame->stVFrame.enPixelFormat) {
      case PIXEL_FORMAT_RGB_888_PLANAR:
      case PIXEL_FORMAT_BGR_888_PLANAR:
        paddrs.push_back(frame->stVFrame.u64PhyAddr[0]);
        paddrs.push_back(frame->stVFrame.u64PhyAddr[1]);
        paddrs.push_back(frame->stVFrame.u64PhyAddr[2]);
        break;
      default:
        LOGE(AISDK_TAG, "Unsupported image type: %x.\n", frame->stVFrame.enPixelFormat);
        return CVIAI_ERR_INFERENCE;
    }
  }
  if (int ret =
          CVI_NN_FeedTensorWithFrames(mp_mi->handle, mp_mi->in.tensors, m_vpss_config[0].frame_type,
                                      CVI_FMT_INT8, paddrs.size(), paddrs.data(),
                                      frames[0]->stVFrame.u32Height, frames[0]->stVFrame.u32Width,
                                      frames[0]->stVFrame.u32Stride[0]) != CVI_RC_SUCCESS) {
    LOGE(AISDK_TAG, "NN set tensor with vi failed: %s\n", get_tpu_error_msg(ret));
    return CVIAI_ERR_INFERENCE;
  }
  return CVIAI_SUCCESS;
}

}  // namespace cviai
