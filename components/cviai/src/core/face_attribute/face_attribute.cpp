#include "face_attribute.hpp"
#include "cvi_sys.h"
#include "core/cviai_types_mem.h"
#include "core/cviai_types_mem_internal.h"
// #include "core/face/cvai_face_helper.h"
#include "core/utils/vpss_helper.h"
#include "core_utils.hpp"
#include "cviai_log.hpp"
#include "face_attribute_types.hpp"
#include "face_utils.hpp"
#include "image_utils.hpp"

#include "core/core/cvai_errno.h"
#include "cvi_sys.h"

#define FACE_ATTRIBUTE_FACTOR (1 / 128.f)
#define FACE_ATTRIBUTE_MEAN (0.99609375)

#define RECOGNITION_OUT_NAME "pre_fc1"

static bool IS_SUPPORTED_FORMAT(VIDEO_FRAME_INFO_S *frame) {
  if (frame->stVFrame.enPixelFormat != PIXEL_FORMAT_RGB_888 &&
      frame->stVFrame.enPixelFormat != PIXEL_FORMAT_RGB_888_PLANAR &&
      frame->stVFrame.enPixelFormat != PIXEL_FORMAT_YUV_PLANAR_420 &&
      frame->stVFrame.enPixelFormat != PIXEL_FORMAT_NV21) {
    LOGE(AISDK_TAG, "Pixel format [%d] is not supported.\n", frame->stVFrame.enPixelFormat);
    return false;
  }
  return true;
}

static void CHECK_VIRTUAL_ADDRESS(VIDEO_FRAME_INFO_S *frame) {
  frame->stVFrame.pu8VirAddr[0] = (CVI_U8 *) frame->stVFrame.u64PhyAddr[0];
  frame->stVFrame.pu8VirAddr[1] = (CVI_U8 *) frame->stVFrame.u64PhyAddr[1];
  frame->stVFrame.pu8VirAddr[2] = (CVI_U8 *) frame->stVFrame.u64PhyAddr[2];
}

namespace cviai {

FaceAttribute::FaceAttribute(bool with_attr)
    : Core(CVI_MEM_DEVICE), m_with_attribute(with_attr) {
}

int FaceAttribute::setupInputPreprocess(std::vector<InputPreprecessSetup> *data) {
  if (data->size() != 1) {
    LOGE(AISDK_TAG, "Face attribute only has 1 input.\n");
    return CVIAI_ERR_INVALID_ARGS;
  }
  for (uint32_t i = 0; i < 3; i++) {
    (*data)[0].factor[i] = FACE_ATTRIBUTE_FACTOR;
    (*data)[0].mean[i] = FACE_ATTRIBUTE_MEAN;
  }
  (*data)[0].use_quantize_scale = true;

  return CVIAI_SUCCESS;
}

int FaceAttribute::onModelOpened() { 
  CVI_SHAPE shape = getInputShape(0);
  PIXEL_FORMAT_E format = PIXEL_FORMAT_RGB_888;
  if (CREATE_VBFRAME_HELPER(&m_gdc_blk, &m_wrap_frame, shape.dim[3], shape.dim[2], format) !=
      CVIAI_SUCCESS) {
    LOGE(AISDK_TAG, "Cannot allocate ion for preprocess\n");
    return CVIAI_ERR_ALLOC_ION_FAIL;
  }
  return CVIAI_SUCCESS;
}

int FaceAttribute::onModelClosed() {
  return CVIAI_SUCCESS;
}

FaceAttribute::~FaceAttribute() {
  if (m_gdc_blk != (VB_BLK)-1) {
    CVI_VB_ReleaseBlock(m_gdc_blk);
  }
}

// int FaceAttribute::vpssPreprocess(VIDEO_FRAME_INFO_S *srcFrame,
//                                           VIDEO_FRAME_INFO_S *dstFrame, VPSSConfig &vpss_config) {
//   auto &vpssChnAttr = vpss_config.chn_attr;
//   auto &vpssCropAttr = vpss_config.crop_attr;
//   auto &factor = vpssChnAttr.stNormalize.factor;
//   auto &mean = vpssChnAttr.stNormalize.mean;
//   VPSS_CHN_SQ_RB_HELPER(&vpssChnAttr, vpssCropAttr.stCropRect.u32Width,
//                         vpssCropAttr.stCropRect.u32Height, vpssChnAttr.u32Width,
//                         vpssChnAttr.u32Height, PIXEL_FORMAT_RGB_888_PLANAR, factor, mean, false);
//   CVI_S32 ret = mp_vpss_inst->sendCropChnFrame(srcFrame, &vpss_config.crop_attr,
//                                                &vpss_config.chn_attr, &vpss_config.chn_coeff, 1);
//   if (ret != CVI_SUCCESS) {
//     return ret;
//   }
//   return mp_vpss_inst->getFrame(dstFrame, 0);
// }
int FaceAttribute::inference(VIDEO_FRAME_INFO_S *frame, cvai_face_t *meta, int face_idx) {
  if (false == IS_SUPPORTED_FORMAT(frame)) {
    return CVIAI_ERR_INVALID_ARGS;
  }

  CHECK_VIRTUAL_ADDRESS(frame);
  uint32_t img_width = frame->stVFrame.u32Width;
  uint32_t img_height = frame->stVFrame.u32Height;
  for (uint32_t i = 0; i < meta->size; ++i) {
    if (face_idx != -1 && i != (uint32_t)face_idx) continue;


    cvai_face_info_t face_info = info_rescale_c(img_width, img_height, *meta, i);

#if 1
    Preprocessing(&face_info, img_width, img_height);
    // auto &pstCropInfo = m_vpss_config[0].crop_attr;
    // m_vpss_config[0].crop_attr.bEnable = CVI_TRUE;
		// pstCropInfo.stCropRect.s32X = ALIGN_DOWN((int)face_info.bbox.x1, 2);
		// pstCropInfo.stCropRect.s32Y = ALIGN_DOWN((int)face_info.bbox.y1, 2);
    // uint32_t boxw = (uint32_t)(face_info.bbox.x2 - face_info.bbox.x1);
    // uint32_t boxh = (uint32_t)(face_info.bbox.y2 - face_info.bbox.y1);
		// pstCropInfo.stCropRect.u32Width = ALIGN_DOWN(boxw, 2);
		// pstCropInfo.stCropRect.u32Height = ALIGN_DOWN(boxh, 2);

    m_vpss_config[0].crop_attr.bEnable = CVI_TRUE;
    m_vpss_config[0].crop_attr.enCropCoordinate = VPSS_CROP_ABS_COOR;
    float x1 = face_info.bbox.x1;
    float y1 = face_info.bbox.y1;
    float x2 = face_info.bbox.x2;
    float y2 = face_info.bbox.y2;
    m_vpss_config[0].crop_attr.stCropRect = {(int32_t)x1, (int32_t)y1, (uint32_t)(x2 - x1),
                                             (uint32_t)(y2 - y1)};
    // m_vpss_config[0].crop_attr.enCropCoordinate = VPSS_CROP_RATIO_COOR;
    // m_vpss_config[0].crop_attr.stCropRect = {(int)face_info.bbox.x1, (int)face_info.bbox.y1,
    //                                          (uint32_t)(face_info.bbox.x2 - face_info.bbox.x1),
    //                                          (uint32_t)(face_info.bbox.y2 - face_info.bbox.y1)};

    std::vector<VIDEO_FRAME_INFO_S *> frames = {frame};

#else
    ALIGN_FACE_TO_FRAME(stOutFrame, &m_wrap_frame, face_info);
    std::vector<VIDEO_FRAME_INFO_S *> frames = {&m_wrap_frame};
#endif
    
    int ret = run(frames);
    if (ret != CVIAI_SUCCESS) {
      return ret;
    }

    outputParser(meta, i);
    CVI_AI_FreeCpp(&face_info);
  }

  return CVIAI_SUCCESS;
}

void FaceAttribute::outputParser(cvai_face_t *meta, int meta_i) {
  FaceAttributeInfo result;

  // feature
  std::string feature_out_name = RECOGNITION_OUT_NAME;
  const TensorInfo &tinfo = getOutputTensorInfo(feature_out_name);
  int8_t *face_blob = tinfo.get<int8_t>();
  size_t face_feature_size = tinfo.tensor_elem;
  // Create feature
  CVI_AI_MemAlloc(sizeof(int8_t), face_feature_size, TYPE_INT8, &meta->info[meta_i].feature);
  memcpy(meta->info[meta_i].feature.ptr, face_blob, face_feature_size);

}

void FaceAttribute::Preprocessing(cvai_face_info_t *face_info, int img_width,
                                   int img_height) {
  // scale to 1.5 times
  float half_width = (face_info->bbox.x2 - face_info->bbox.x1) / 4;
  float half_height = (face_info->bbox.y2 - face_info->bbox.y1) / 4;
  face_info->bbox.x1 = face_info->bbox.x1 - half_width;
  face_info->bbox.x2 = face_info->bbox.x2 + half_width;
  face_info->bbox.y1 = face_info->bbox.y1 - half_height;
  face_info->bbox.y2 = face_info->bbox.y2 + half_height;

  // square the roi
  int max_side =
      std::max(face_info->bbox.x2 - face_info->bbox.x1, face_info->bbox.y2 - face_info->bbox.y1);
  int offset_x = (max_side - (int)face_info->bbox.x2 + face_info->bbox.x1) / 2;
  int offset_y = (max_side - (int)face_info->bbox.y2 + face_info->bbox.y1) / 2;
  face_info->bbox.x1 = std::max(((int)face_info->bbox.x1 - offset_x), 0);
  face_info->bbox.x2 = std::min(((int)face_info->bbox.x1 + max_side), img_width);
  face_info->bbox.y1 = std::max(((int)face_info->bbox.y1 - offset_y), 0);
  face_info->bbox.y2 = std::min(((int)face_info->bbox.y1 + max_side), img_height);
}

}  // namespace cviai