#include "core/cviai_core.h"
#include "cviai_core_internal.hpp"
#include "cviai_log.hpp"

#include "core/core/cvai_errno.h"
#include "object_detection/yolox/yolox.hpp"
#include "retina_face/retina_face.hpp"
#include "face_attribute/face_attribute.hpp"
#include "license_plate_detection/license_plate_detection.hpp"
#include "object_detection/mobiledetv2/mobiledetv2.hpp"
// #define STB_IMAGE_IMPLEMENTATION
// #include "stb/stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb/stb_image_write.h"

#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace cviai;

struct ModelParams {
  VpssEngine *vpss_engine;
  uint32_t vpss_timeout_value;
};

using CreatorFunc = std::function<Core *(const ModelParams &)>;
using namespace std::placeholders;

template <typename C, typename... Args>
Core *create_model(const ModelParams &params, Args... arg) {
  C *instance = new C(arg...);

  instance->setVpssEngine(params.vpss_engine);
  instance->setVpssTimeout(params.vpss_timeout_value);
  return instance;
}

#ifdef ENABLE_IVE
static void createIVEHandleIfNeeded(IVE_HANDLE *ive_handle) {
  if (*ive_handle == NULL) {
    *ive_handle = CVI_IVE_CreateHandle();
    if (*ive_handle == NULL) {
      LOGC(AISDK_TAG, "IVE handle init failed.\n");
    }
  }
}
#endif

// Convenience macros for creator
#define CREATOR(type) CreatorFunc(create_model<type>)

// Convenience macros for creator, P{NUM} stands for how many parameters for creator
#define CREATOR_P1(type, arg_type, arg1) \
  CreatorFunc(std::bind(create_model<type, arg_type>, _1, arg1))

/**
 * IMPORTANT!!
 * Creators for all DNN model. Please remember to register model creator here, or
 * AISDK cannot instantiate model correctly.
 */
unordered_map<int, CreatorFunc> MODEL_CREATORS = {
    {CVI_AI_SUPPORTED_MODEL_YOLOX, CREATOR(YoloX)},
    {CVI_AI_SUPPORTED_MODEL_RETINAFACE, CREATOR_P1(RetinaFace, PROCESS, CAFFE)},
    {CVI_AI_SUPPORTED_MODEL_FACERECOGNITION, CREATOR_P1(FaceAttribute, bool, false)},
    {CVI_AI_SUPPORTED_MODEL_WPODNET, CREATOR(LicensePlateDetection)},
    {CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_COCO80,
     CREATOR_P1(MobileDetV2, MobileDetV2::Category, MobileDetV2::Category::coco80)},
    {CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PERSON_VEHICLE,
     CREATOR_P1(MobileDetV2, MobileDetV2::Category, MobileDetV2::Category::person_vehicle)},
    {CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_VEHICLE,
     CREATOR_P1(MobileDetV2, MobileDetV2::Category, MobileDetV2::Category::vehicle)},
    {CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN,
     CREATOR_P1(MobileDetV2, MobileDetV2::Category, MobileDetV2::Category::pedestrian)},
    {CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PERSON_PETS,
     CREATOR_P1(MobileDetV2, MobileDetV2::Category, MobileDetV2::Category::person_pets)},
};

inline void __attribute__((always_inline)) removeCtx(cviai_context_t *ctx) {
#ifdef ENABLE_IVE
  if (ctx->ive_handle) {
    CVI_IVE_DestroyHandle(ctx->ive_handle);
  }
#endif

  for (auto it : ctx->vec_vpss_engine) {
    delete it;
  }
  delete ctx;
}

inline Core *__attribute__((always_inline))
getInferenceInstance(const CVI_AI_SUPPORTED_MODEL_E index, cviai_context_t *ctx) {
  cviai_model_t &m_t = ctx->model_cont[index];
  if (m_t.instance == nullptr) {
    if (MODEL_CREATORS.find(index) == MODEL_CREATORS.end()) {
      LOGE(AISDK_TAG, "Cannot find creator for %s, Please register a creator for this model!\n",
           CVI_AI_GetModelName(index));
      return nullptr;
    }
    
    auto creator = MODEL_CREATORS[index];
    ModelParams params = {.vpss_engine = ctx->vec_vpss_engine[m_t.vpss_thread],
                          .vpss_timeout_value = ctx->vpss_timeout_value};

    m_t.instance = creator(params);
  }

  return m_t.instance;
}

CVI_S32 CVI_AI_CreateHandle(cviai_handle_t *handle) { return CVI_AI_CreateHandle2(handle, -1, 0); }

CVI_S32 CVI_AI_CreateHandle2(cviai_handle_t *handle, const VPSS_GRP vpssGroupId,
                             const CVI_U8 vpssDev) {
  cviai_context_t *ctx = new cviai_context_t;
#ifdef ENABLE_IVE
  ctx->ive_handle = NULL;
#endif

  ctx->vec_vpss_engine.push_back(new VpssEngine());
  if (ctx->vec_vpss_engine[0]->init(vpssGroupId, vpssDev) != CVI_SUCCESS) {
    LOGE(AISDK_TAG, "cviai_handle_t create failed.");
    removeCtx(ctx);
    return CVIAI_ERR_INIT_VPSS;
  }
  const char timestamp[] = __DATE__ " " __TIME__;
  LOGI(AISDK_TAG, "cviai_handle_t is created, %s", timestamp);
  *handle = ctx;
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_DestroyHandle(cviai_handle_t handle) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  CVI_AI_CloseAllModel(handle);
  removeCtx(ctx);
  LOGI(AISDK_TAG, "cviai_handle_t is destroyed.");
  return CVIAI_SUCCESS;
}

static bool checkModelFile(const char *filepath) {
  struct stat buffer;
  bool ret = false;
  if (stat(filepath, &buffer) == 0) {
    if (S_ISREG(buffer.st_mode)) {
      ret = true;
    } else {
      LOGE(AISDK_TAG, "Path of model file isn't a regular file: %s\n", filepath);
    }
  } else {
    LOGE(AISDK_TAG, "Model file doesn't exists: %s\n", filepath);
  }
  return ret;
}

CVI_S32 CVI_AI_OpenModel(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                         const char *filepath) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  cviai_model_t &m_t = ctx->model_cont[config];
  Core *instance = getInferenceInstance(config, ctx);

  if (instance != nullptr) {
    if (instance->isInitialized()) {
      LOGW(AISDK_TAG, "%s: Inference has already initialized. Please call CVI_AI_CloseModel to reset.\n",
           CVI_AI_GetModelName(config));
      return CVIAI_ERR_MODEL_INITIALIZED;
    }
  } else {
    LOGE(AISDK_TAG, "Cannot create model: %s\n", CVI_AI_GetModelName(config));
    return CVIAI_ERR_OPEN_MODEL;
  }

  if (!checkModelFile(filepath)) {
    return CVIAI_ERR_INVALID_MODEL_PATH;
  }

  m_t.model_path = filepath;
  CVI_S32 ret = m_t.instance->modelOpen(m_t.model_path.c_str());
  if (ret != CVIAI_SUCCESS) {
    LOGE(AISDK_TAG, "Failed to open model: %s (%s)", CVI_AI_GetModelName(config), m_t.model_path.c_str());
    return ret;
  }
  LOGI(AISDK_TAG, "Model is opened successfully: %s \n", CVI_AI_GetModelName(config));
  return CVIAI_SUCCESS;
}

const char *CVI_AI_GetModelPath(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  return GetModelName(ctx->model_cont[config]);
}

CVI_S32 CVI_AI_SetSkipVpssPreprocess(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                                     bool skip) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  Core *instance = getInferenceInstance(config, ctx);
  if (instance != nullptr) {
    instance->skipVpssPreprocess(skip);
  } else {
    LOGE(AISDK_TAG, "Cannot create model: %s\n", CVI_AI_GetModelName(config));
    return CVIAI_ERR_OPEN_MODEL;
  }
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_GetSkipVpssPreprocess(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                                     bool *skip) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  Core *instance = getInferenceInstance(config, ctx);
  if (instance != nullptr) {
    *skip = instance->hasSkippedVpssPreprocess();
  } else {
    LOGE(AISDK_TAG, "Cannot create model: %s\n", CVI_AI_GetModelName(config));
    return CVIAI_ERR_OPEN_MODEL;
  }
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_SetModelThreshold(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                                 float threshold) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  Core *instance = getInferenceInstance(config, ctx);
  if (instance != nullptr) {
    instance->setModelThreshold(threshold);
  } else {
    LOGE(AISDK_TAG, "Cannot create model: %s\n", CVI_AI_GetModelName(config));
    return CVIAI_ERR_OPEN_MODEL;
  }
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_GetModelThreshold(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                                 float *threshold) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  Core *instance = getInferenceInstance(config, ctx);
  if (instance != nullptr) {
    *threshold = instance->getModelThreshold();
  } else {
    LOGE(AISDK_TAG, "Cannot create model: %s\n", CVI_AI_GetModelName(config));
    return CVIAI_ERR_OPEN_MODEL;
  }
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_SetVpssThread(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                             const uint32_t thread) {
  return CVI_AI_SetVpssThread2(handle, config, thread, -1);
}

CVI_S32 CVI_AI_SetVpssThread2(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                              const uint32_t thread, const VPSS_GRP vpssGroupId) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  Core *instance = getInferenceInstance(config, ctx);
  if (instance != nullptr) {
    return setVPSSThread(ctx->model_cont[config], ctx->vec_vpss_engine, thread, vpssGroupId);
  } else {
    LOGE(AISDK_TAG, "Cannot create model: %s\n", CVI_AI_GetModelName(config));
    return CVIAI_ERR_OPEN_MODEL;
  }
}

CVI_S32 CVI_AI_SetVBPool(cviai_handle_t handle, uint32_t thread, VB_POOL pool_id) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  if (thread >= ctx->vec_vpss_engine.size()) {
    LOGE(AISDK_TAG, "Invalid vpss thread: %d, should be 0 to %d\n", thread,
         static_cast<uint32_t>(ctx->vec_vpss_engine.size() - 1));
    return CVIAI_FAILURE;
  }
  ctx->vec_vpss_engine[thread]->attachVBPool(pool_id);
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_GetVBPool(cviai_handle_t handle, uint32_t thread, VB_POOL *pool_id) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  if (thread >= ctx->vec_vpss_engine.size()) {
    LOGE(AISDK_TAG, "Invalid vpss thread: %d, should be 0 to %d\n", thread,
         static_cast<uint32_t>(ctx->vec_vpss_engine.size() - 1));
    return CVIAI_FAILURE;
  }
  *pool_id = ctx->vec_vpss_engine[thread]->getVBPool();
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_GetVpssThread(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                             uint32_t *thread) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  *thread = ctx->model_cont[config].vpss_thread;
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_GetVpssGrpIds(cviai_handle_t handle, VPSS_GRP **groups, uint32_t *num) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  VPSS_GRP *ids = (VPSS_GRP *)malloc(ctx->vec_vpss_engine.size() * sizeof(VPSS_GRP));
  for (size_t i = 0; i < ctx->vec_vpss_engine.size(); i++) {
    ids[i] = ctx->vec_vpss_engine[i]->getGrpId();
  }
  *groups = ids;
  *num = ctx->vec_vpss_engine.size();
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_SetVpssTimeout(cviai_handle_t handle, uint32_t timeout) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  ctx->vpss_timeout_value = timeout;

  for (auto &m_inst : ctx->model_cont) {
    if (m_inst.second.instance != nullptr) {
      m_inst.second.instance->setVpssTimeout(timeout);
    }
  }
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_CloseAllModel(cviai_handle_t handle) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  for (auto &m_inst : ctx->model_cont) {
    if (m_inst.second.instance != nullptr) {
      m_inst.second.instance->modelClose();
      LOGI(AISDK_TAG, "Model is closed: %s\n", CVI_AI_GetModelName(m_inst.first));
      delete m_inst.second.instance;
      m_inst.second.instance = nullptr;
    }
  }
  for (auto &m_inst : ctx->custom_cont) {
    if (m_inst.instance != nullptr) {
      m_inst.instance->modelClose();
      delete m_inst.instance;
      m_inst.instance = nullptr;
    }
  }
  ctx->model_cont.clear();
  ctx->custom_cont.clear();
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_CloseModel(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  cviai_model_t &m_t = ctx->model_cont[config];
  if (m_t.instance == nullptr) {
    return CVIAI_ERR_CLOSE_MODEL;
  }

  m_t.instance->modelClose();
  LOGI(AISDK_TAG, "Model is closed: %s\n", CVI_AI_GetModelName(config));
  delete m_t.instance;
  m_t.instance = nullptr;
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_SelectDetectClass(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                                 uint32_t num_selection, ...) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  va_list args;
  va_start(args, num_selection);

  std::vector<uint32_t> selected_classes;
  for (uint32_t i = 0; i < num_selection; i++) {
    uint32_t selected_class = va_arg(args, uint32_t);

    if (selected_class & CVI_AI_DET_GROUP_MASK_HEAD) {
      uint32_t group_start = (selected_class & CVI_AI_DET_GROUP_MASK_START) >> 16;
      uint32_t group_end = (selected_class & CVI_AI_DET_GROUP_MASK_END);
      for (uint32_t i = group_start; i <= group_end; i++) {
        selected_classes.push_back(i);
      }
    } else {
      if (selected_class >= CVI_AI_DET_TYPE_END) {
        LOGE(AISDK_TAG,"Invalid class id: %d\n", selected_class);
        return CVIAI_ERR_INVALID_ARGS;
      }
      selected_classes.push_back(selected_class);
    }
  }

  Core *instance = getInferenceInstance(config, ctx);
  if (instance != nullptr) {
    // TODO: only supports MobileDetV2 and YOLOX for now
    if (MobileDetV2 *mdetv2 = dynamic_cast<MobileDetV2 *>(instance)) {
      mdetv2->select_classes(selected_classes);
    } else if (YoloX *yolox = dynamic_cast<YoloX *>(instance)) {
      yolox->select_classes(selected_classes);
    } else {
      LOGW(AISDK_TAG,"CVI_AI_SelectDetectClass only supports MobileDetV2 and YOLOX model for now.\n");
    }
  } else {
    LOGE(AISDK_TAG,"Failed to create model: %s\n", CVI_AI_GetModelName(config));
    return CVIAI_ERR_OPEN_MODEL;
  }
  return CVIAI_SUCCESS;
}

CVI_S32 CVI_AI_GetVpssChnConfig(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                                const CVI_U32 frameWidth, const CVI_U32 frameHeight,
                                const CVI_U32 idx, cvai_vpssconfig_t *chnConfig) {
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  cviai::Core *instance = getInferenceInstance(config, ctx);
  if (instance == nullptr) {
    LOGE(AISDK_TAG, "Instance is null.\n");
    return CVIAI_ERR_OPEN_MODEL;
  }

  return instance->getChnConfig(frameWidth, frameHeight, idx, chnConfig);
}

CVI_S32 CVI_AI_EnalbeDumpInput(cviai_handle_t handle, CVI_AI_SUPPORTED_MODEL_E config,
                               const char *dump_path, bool enable) {
  CVI_S32 ret = CVIAI_SUCCESS;
  cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);
  cviai::Core *instance = getInferenceInstance(config, ctx);
  if (instance == nullptr) {
    LOGE(AISDK_TAG, "Instance is null.\n");
    return CVIAI_ERR_OPEN_MODEL;
  }

#ifdef ENABLE_DEBUGGER
  instance->enableDebugger(enable);
  instance->setDebuggerOutputPath(dump_path);
#endif
  return ret;
}

/**
 *  Convenience macros for defining inference functions. F{NUM} stands for how many input frame
 *  variables, P{NUM} stands for how many input parameters in inference function. All inference
 *  function should follow same function signature, that is,
 *  CVI_S32 inference(Frame1, Frame2, ... FrameN, Param1, Param2, ..., ParamN)
 */
#define DEFINE_INF_FUNC_F1_P1(func_name, class_name, model_index, arg_type)                   \
  CVI_S32 func_name(const cviai_handle_t handle, VIDEO_FRAME_INFO_S *frame, arg_type arg1) {  \
    cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);                            \
    class_name *obj = dynamic_cast<class_name *>(getInferenceInstance(model_index, ctx));     \
    if (obj == nullptr) {                                                                     \
      LOGE(AISDK_TAG, "No instance found for %s.\n", #class_name);                                       \
      return CVIAI_ERR_OPEN_MODEL;                                                            \
    }                                                                                         \
    if (obj->isInitialized()) {                                                               \
      return obj->inference(frame, arg1);                                                     \
    } else {                                                                                  \
      LOGE(AISDK_TAG, "Model (%s)is not yet opened! Please call CVI_AI_OpenModel to initialize model\n", \
           CVI_AI_GetModelName(model_index));                                                 \
      return CVIAI_ERR_NOT_YET_INITIALIZED;                                                   \
    }                                                                                         \
  }

#define DEFINE_INF_FUNC_F1_P2(func_name, class_name, model_index, arg1_type, arg2_type)       \
  CVI_S32 func_name(const cviai_handle_t handle, VIDEO_FRAME_INFO_S *frame, arg1_type arg1,   \
                    arg2_type arg2) {                                                         \
    cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);                            \
    class_name *obj = dynamic_cast<class_name *>(getInferenceInstance(model_index, ctx));     \
    if (obj == nullptr) {                                                                     \
      LOGE(AISDK_TAG, "No instance found for %s.\n", #class_name);                                       \
      return CVIAI_ERR_OPEN_MODEL;                                                            \
    }                                                                                         \
    if (obj->isInitialized()) {                                                               \
      return obj->inference(frame, arg1, arg2);                                               \
    } else {                                                                                  \
      LOGE(AISDK_TAG, "Model (%s)is not yet opened! Please call CVI_AI_OpenModel to initialize model\n", \
           CVI_AI_GetModelName(model_index));                                                 \
      return CVIAI_ERR_NOT_YET_INITIALIZED;                                                   \
    }                                                                                         \
  }

#define DEFINE_INF_FUNC_F2_P1(func_name, class_name, model_index, arg_type)                   \
  CVI_S32 func_name(const cviai_handle_t handle, VIDEO_FRAME_INFO_S *frame1,                  \
                    VIDEO_FRAME_INFO_S *frame2, arg_type arg1) {                              \
    cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);                            \
    class_name *obj = dynamic_cast<class_name *>(getInferenceInstance(model_index, ctx));     \
    if (obj == nullptr) {                                                                     \
      LOGE(AISDK_TAG, "No instance found for %s.\n", #class_name);                                       \
      return CVIAI_ERR_OPEN_MODEL;                                                            \
    }                                                                                         \
    if (obj->isInitialized()) {                                                               \
      return obj->inference(frame1, frame2, arg1);                                            \
    } else {                                                                                  \
      LOGE(AISDK_TAG, "Model (%s)is not yet opened! Please call CVI_AI_OpenModel to initialize model\n", \
           CVI_AI_GetModelName(model_index));                                                 \
      return CVIAI_ERR_NOT_YET_INITIALIZED;                                                   \
    }                                                                                         \
  }

#define DEFINE_INF_FUNC_F2_P2(func_name, class_name, model_index, arg1_type, arg2_type)       \
  CVI_S32 func_name(const cviai_handle_t handle, VIDEO_FRAME_INFO_S *frame1,                  \
                    VIDEO_FRAME_INFO_S *frame2, arg1_type arg1, arg2_type arg2) {             \
    cviai_context_t *ctx = static_cast<cviai_context_t *>(handle);                            \
    class_name *obj = dynamic_cast<class_name *>(getInferenceInstance(model_index, ctx));     \
    if (obj == nullptr) {                                                                     \
      LOGE(AISDK_TAG, "No instance found for %s.\n", #class_name);                                       \
      return CVIAI_ERR_OPEN_MODEL;                                                            \
    }                                                                                         \
    if (obj->isInitialized()) {                                                               \
      return obj->inference(frame1, frame2, arg1, arg2);                                      \
    } else {                                                                                  \
      LOGE(AISDK_TAG, "Model (%s)is not yet opened! Please call CVI_AI_OpenModel to initialize model\n", \
           CVI_AI_GetModelName(model_index));                                                 \
      return CVIAI_ERR_NOT_YET_INITIALIZED;                                                   \
    }                                                                                         \
  }

/**
 *  Define model inference function here.
 *
 *  IMPORTANT!!
 *  Please remember to register creator function in MODEL_CREATORS first, or AISDK cannot
 *  find a correct way to create model object.
 *
 */
DEFINE_INF_FUNC_F1_P1(CVI_AI_YoloX, YoloX, CVI_AI_SUPPORTED_MODEL_YOLOX, cvai_object_t *)
DEFINE_INF_FUNC_F1_P1(CVI_AI_RetinaFace, RetinaFace, CVI_AI_SUPPORTED_MODEL_RETINAFACE,
                      cvai_face_t *)
DEFINE_INF_FUNC_F1_P1(CVI_AI_FaceRecognition, FaceAttribute, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION,
                      cvai_face_t *)
DEFINE_INF_FUNC_F1_P1(CVI_AI_LicensePlateDetection, LicensePlateDetection,
                      CVI_AI_SUPPORTED_MODEL_WPODNET, cvai_object_t *)

DEFINE_INF_FUNC_F1_P1(CVI_AI_MobileDetV2_Vehicle, MobileDetV2,
                      CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_VEHICLE, cvai_object_t *)
DEFINE_INF_FUNC_F1_P1(CVI_AI_MobileDetV2_Pedestrian, MobileDetV2,
                      CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, cvai_object_t *)
DEFINE_INF_FUNC_F1_P1(CVI_AI_MobileDetV2_Person_Vehicle, MobileDetV2,
                      CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PERSON_VEHICLE, cvai_object_t *)
DEFINE_INF_FUNC_F1_P1(CVI_AI_MobileDetV2_Person_Pets, MobileDetV2,
                      CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PERSON_PETS, cvai_object_t *)
DEFINE_INF_FUNC_F1_P1(CVI_AI_MobileDetV2_COCO80, MobileDetV2,
                      CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_COCO80, cvai_object_t *)
