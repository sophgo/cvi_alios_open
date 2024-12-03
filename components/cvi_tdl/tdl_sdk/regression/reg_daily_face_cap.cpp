#include <fstream>
#include <string>
#include <unordered_map>
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_app/cvi_tdl_app.h"
#include "cvi_tdl_evaluation.h"
#include "cvi_tdl_media.h"
#include "cvi_tdl_test.hpp"
#include "json.hpp"
#include "raii.hpp"
#include "regression_utils.hpp"

#define FACE_FEAT_SIZE 256

namespace cvitdl {
namespace unitest {

typedef enum {
  FaceDetection = 0,
  FaceRecognition = 10,
  Pedestrian = 20,
  FaceLandmark = 30,
  FaceLandmark2 = 31,
  FaceLandmark3 = 32,
} ModelType;

// FaceRecognitionTestSuite
class FaceCaptureTestSuite : public CVI_TDLModelTestSuite {
 public:
  struct ModelInfo {
    CVI_TDL_SUPPORTED_MODEL_E index;
    std::string model_path;
  };

  FaceCaptureTestSuite() : CVI_TDLModelTestSuite("daily_reg_face_cap.json", "reg_daily_face_cap") {}

  virtual ~FaceCaptureTestSuite() = default;

 protected:
  virtual void SetUp() {
    m_tdl_handle = NULL;
    ASSERT_EQ(CVI_TDL_CreateHandle2(&m_tdl_handle, 0, 0), CVI_TDL_SUCCESS);
    ASSERT_EQ(CVI_TDL_Service_CreateHandle(&m_service_handle, m_tdl_handle), CVI_TDL_SUCCESS);
    ASSERT_EQ(CVI_TDL_APP_CreateHandle(&m_app_handle, m_tdl_handle), CVI_TDL_SUCCESS);
    // ASSERT_EQ(CVI_TDL_SetVpssTimeout(m_tdl_handle, 1000), CVI_TDL_SUCCESS);
  }

  virtual void TearDown() {
    CVI_TDL_APP_DestroyHandle(m_app_handle);
    CVI_TDL_Service_DestroyHandle(m_service_handle);
    CVI_TDL_DestroyHandle(m_tdl_handle);
    m_app_handle = NULL;
    m_tdl_handle = NULL;
    m_service_handle = NULL;
    // CVI_SYS_Exit();
    // CVI_VB_Exit();
  }

  cvitdl_service_handle_t m_service_handle;
  cvitdl_app_handle_t m_app_handle;
  float bbox_threshold = 0.95;
  ModelInfo getModel(ModelType model_type, const std::string &model_name);
};

FaceCaptureTestSuite::ModelInfo FaceCaptureTestSuite::getModel(ModelType model_type,
                                                               const std::string &model_name) {
  ModelInfo model_info;
  model_info.index = CVI_TDL_SUPPORTED_MODEL_END;

  if (model_name.empty())
    model_info.model_path = "";
  else
    model_info.model_path = (m_model_dir / model_name).string();

  switch (model_type) {
    case FaceDetection: {
      model_info.index = CVI_TDL_SUPPORTED_MODEL_SCRFDFACE;
    } break;
    case FaceRecognition: {
      model_info.index = CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION;
    } break;
    case Pedestrian: {
      model_info.index = CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN;
    } break;
    case FaceLandmark: {
      model_info.index = CVI_TDL_SUPPORTED_MODEL_FACELANDMARKER;
    } break;
    case FaceLandmark2: {
      model_info.index = CVI_TDL_SUPPORTED_MODEL_FACELANDMARKERDET2;
    } break;
    case FaceLandmark3: {
      model_info.index = CVI_TDL_SUPPORTED_MODEL_LANDMARK_DET3;
    } break;
    default:
      printf("unsupported model type: %d\n", model_type);
  }

  return model_info;
}

TEST_F(FaceCaptureTestSuite, match_det) {
  ASSERT_EQ(CVI_TDL_APP_FaceCapture_Init(m_app_handle, (uint32_t)5), CVI_TDL_SUCCESS);

  // Setup fd fr fl ped ModelInfo
  std::string model_fd_name = std::string(std::string(m_json_object[0]["model_fd_name"]).c_str());
  ModelType model_fd_type = m_json_object[0]["model_fd_type"];
  ModelInfo fd_info = getModel(model_fd_type, model_fd_name);

  std::string model_fr_name = std::string(std::string(m_json_object[0]["model_fr_name"]).c_str());
  ModelType model_fr_type = m_json_object[0]["model_fr_type"];
  ModelInfo fr_info = getModel(model_fr_type, model_fr_name);

  // FaceCapture Models Setup
  ASSERT_EQ(CVI_TDL_APP_FaceCapture_QuickSetUp(m_app_handle, fd_info.index, fr_info.index,
                                               fd_info.model_path.c_str(),
                                               fr_info.model_path.c_str(), NULL, NULL, NULL),
            CVI_TDL_SUCCESS);
  int feature_len = m_json_object[0]["reg_feature"].size();
  std::string reg_face_str =
      (m_image_dir / std::string(m_json_object[0]["register_face"])).string();
  uint8_t reg_feature[feature_len];

  for (int i = 0; i < feature_len; i++) {
    reg_feature[i] = uint8_t(m_json_object[0]["reg_feature"][i]);
  }

  cvtdl_service_feature_array_t feat_gallery;
  memset(&feat_gallery, 0, sizeof(feat_gallery));

  cvtdl_face_t faceinfo;
  memset(&faceinfo, 0, sizeof(faceinfo));

  Image regFace(reg_face_str, PIXEL_FORMAT_RGB_888);
  ASSERT_TRUE(regFace.open());

  ASSERT_EQ(CVI_TDL_APP_FaceCapture_FDFR(m_app_handle, regFace.getFrame(), &faceinfo),
            CVI_TDL_SUCCESS);
  ASSERT_TRUE(faceinfo.size == 1);

  feat_gallery.type = faceinfo.info[0].feature.type;
  feat_gallery.feature_length = faceinfo.info[0].feature.size;
  feat_gallery.ptr = (int8_t *)malloc(faceinfo.info[0].feature.size);
  memcpy(feat_gallery.ptr, faceinfo.info[0].feature.ptr, faceinfo.info[0].feature.size);

  feat_gallery.data_num = 1;
  ASSERT_EQ(CVI_TDL_Service_RegisterFeatureArray(m_service_handle, feat_gallery, COS_SIMILARITY),
            CVI_TDL_SUCCESS);
  free(feat_gallery.ptr);

  cvtdl_face_info_t face_info;
  face_info.feature.size = feature_len;
  face_info.feature.type = TYPE_INT8;
  face_info.feature.ptr = (int8_t *)malloc(feature_len);
  memcpy(face_info.feature.ptr, reg_feature, feature_len);

  uint32_t ind = 0, size;
  float score = 0;

  ASSERT_EQ(
      CVI_TDL_Service_FaceInfoMatching(m_service_handle, &face_info, 1, 0.1, &ind, &score, &size),
      CVI_TDL_SUCCESS);
  ASSERT_NEAR(score, 1, 0.4) << "expected matching score: (" << score << " != " << 1 << ")\n";

  free(face_info.feature.ptr);
  CVI_TDL_Free(&faceinfo);
}

TEST_F(FaceCaptureTestSuite, accuracy) {
  // Vb request
  const CVI_S32 vpssgrp_width = 2560;
  const CVI_S32 vpssgrp_height = 1440;
  MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 3, vpssgrp_width,
                   vpssgrp_height, PIXEL_FORMAT_RGB_888_PLANAR, 3);

  // FaceCapture Init
  ASSERT_EQ(CVI_TDL_APP_FaceCapture_Init(m_app_handle, (uint32_t)5), CVI_TDL_SUCCESS);

  // Setup fd fr fl ped ModelInfo
  std::string model_fd_name = std::string(std::string(m_json_object[0]["model_fd_name"]).c_str());
  printf("%s\n", model_fd_name.c_str());
  ModelType model_fd_type = m_json_object[0]["model_fd_type"];
  ModelInfo fd_info = getModel(model_fd_type, model_fd_name);

  std::string model_fr_name = std::string(std::string(m_json_object[0]["model_fr_name"]).c_str());
  printf("%s\n", model_fr_name.c_str());
  ModelType model_fr_type = m_json_object[0]["model_fr_type"];
  ModelInfo fr_info = getModel(model_fr_type, model_fr_name);

  std::string model_fl_name = std::string(std::string(m_json_object[0]["model_fl_name"]).c_str());
  printf("%s\n", model_fl_name.c_str());
  ModelType model_fl_type = m_json_object[0]["model_fl_type"];
  ModelInfo fl_info = getModel(model_fl_type, model_fl_name);

  std::string model_ped_name = std::string(std::string(m_json_object[0]["model_ped_name"]).c_str());
  printf("%s\n", model_ped_name.c_str());
  ModelType model_ped_type = m_json_object[0]["model_ped_type"];
  ModelInfo ped_info = getModel(model_ped_type, model_ped_name);

  // FaceCapture Models Setup
  ASSERT_EQ(CVI_TDL_APP_FaceCapture_QuickSetUp(
                m_app_handle, fd_info.index, fr_info.index, fd_info.model_path.c_str(),
                fr_info.model_path.c_str(), NULL, fl_info.model_path.c_str(), NULL),
            CVI_TDL_SUCCESS);

  // ASSERT_EQ(CVI_TDL_APP_FaceCapture_FusePedSetup(m_app_handle, ped_info.index,
  // ped_info.model_path.c_str()), CVI_TDL_SUCCESS);

  float fdet_threshold = float(m_json_object[0]["fd_threshold"]);
  printf("face_det_threshold = %f\n", fdet_threshold);
  ASSERT_EQ(CVI_TDL_SetModelThreshold(m_tdl_handle, fd_info.index, fdet_threshold),
            CVI_TDL_SUCCESS);

  // config setting face_cap
  face_capture_config_t app_cfg;
  ASSERT_EQ(CVI_TDL_APP_FaceCapture_GetDefaultConfig(&app_cfg), CVI_TDL_SUCCESS);

  app_cfg.thr_quality = 0.1;
  app_cfg.thr_size_min = 20;
  app_cfg.miss_time_limit = 20;
  app_cfg.store_feature = true;
  app_cfg.qa_method = 0;
  app_cfg.img_capture_flag = 0;  // capture whole frame
  app_cfg.m_interval = 1000;     // only export one when leaving
  ASSERT_EQ(CVI_TDL_APP_FaceCapture_SetConfig(m_app_handle, &app_cfg), CVI_TDL_SUCCESS);
  ASSERT_EQ(CVI_TDL_APP_FaceCapture_SetMode(m_app_handle, FAST), CVI_TDL_SUCCESS);

  auto results = m_json_object[0]["test_images"];

  for (nlohmann::json::iterator iter = results.begin(); iter != results.end(); iter++) {
    std::string image_path = (m_image_dir / iter.key()).string();

    Image fdFrame(image_path, PIXEL_FORMAT_RGB_888);
    ASSERT_TRUE(fdFrame.open());

    ASSERT_EQ(CVI_TDL_APP_FaceCapture_Run(m_app_handle, fdFrame.getFrame()), CVI_TDL_SUCCESS);
    cvtdl_face_t *p_objinfo = &(m_app_handle->face_cpt_info->last_faces);
    auto expected_dets = iter.value();

    ASSERT_EQ(p_objinfo->size, expected_dets.size());

    // Verify result
    for (uint32_t det_index = 0; det_index < expected_dets.size(); det_index++) {
      auto bbox = expected_dets[det_index]["bbox"];

      cvtdl_bbox_t expected_bbox = {
          .x1 = float(bbox[0]),
          .y1 = float(bbox[1]),
          .x2 = float(bbox[2]),
          .y2 = float(bbox[3]),
      };

      ASSERT_TRUE(iou(p_objinfo->info[det_index].bbox, expected_bbox) >= bbox_threshold)
          << "image path: " << image_path << "\n"
          << "expected bbox: (" << expected_bbox.x1 << ", " << expected_bbox.y1 << ", "
          << expected_bbox.x2 << ", " << expected_bbox.y2 << ")\n";

      auto unique_id = expected_dets[det_index]["unique_id"];
      ASSERT_TRUE(p_objinfo->info[det_index].unique_id == unique_id)
          << "image path: " << image_path << "\n"
          << "expected unique_id: (" << p_objinfo->info[det_index].unique_id << " != " << unique_id
          << ")\n";

      auto quality = expected_dets[det_index]["quality"];
      ASSERT_NEAR(p_objinfo->info[det_index].face_quality, quality, 0.1)
          << "image path: " << image_path << "\n"
          << "expected quality: (" << p_objinfo->info[det_index].face_quality << " != " << quality
          << ")\n";

      auto score = expected_dets[det_index]["score"];
      ASSERT_NEAR(p_objinfo->info[det_index].bbox.score, score, 0.1)
          << "image path: " << image_path << "\n"
          << "expected score: (" << p_objinfo->info[det_index].bbox.score << " != " << score
          << ")\n";
    }

#if 0
      printf("\"%s\": [\n", std::string(iter.key()).c_str());
      for (uint32_t i = 0; i < p_objinfo->size; i++) {
        printf("{\n  \"unique_id\": %d,\n  \"quality\": %f,\n  \"score\": %f,\n  \"bbox\": [\n    %f,\n    %f,\n    %f,\n    %f\n  ]\n},\n", 
        int(p_objinfo->info[i].unique_id), p_objinfo->info[i].face_quality, p_objinfo->info[i].bbox.score, p_objinfo->info[i].bbox.x1, p_objinfo->info[i].bbox.y1, p_objinfo->info[i].bbox.x2, p_objinfo->info[i].bbox.y2);
      }
      printf("],\n");
#endif
  }
}
}  // namespace unitest
}  // namespace cvitdl
