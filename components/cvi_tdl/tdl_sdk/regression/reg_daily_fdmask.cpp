#include <experimental/filesystem>
#include <fstream>
#include <memory>
#include <string>
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_evaluation.h"
#include "cvi_tdl_media.h"
#include "cvi_tdl_test.hpp"
#include "gtest.h"
#include "json.hpp"
#include "raii.hpp"
#include "regression_utils.hpp"

namespace fs = std::experimental::filesystem;
namespace cvitdl {
namespace unitest {

class FaceMaskDetectionTestSuite : public CVI_TDLModelTestSuite {
 public:
  FaceMaskDetectionTestSuite()
      : CVI_TDLModelTestSuite("reg_daily_fdmask.json", "reg_daily_fdmask") {}

  virtual ~FaceMaskDetectionTestSuite() = default;

  std::string m_model_path;

 protected:
  virtual void SetUp() {
    m_tdl_handle = NULL;
    ASSERT_EQ(CVI_TDL_CreateHandle2(&m_tdl_handle, 1, 0), CVI_TDL_SUCCESS);
    ASSERT_EQ(CVI_TDL_SetVpssTimeout(m_tdl_handle, 1000), CVI_TDL_SUCCESS);
  }

  virtual void TearDown() {
    CVI_TDL_DestroyHandle(m_tdl_handle);
    m_tdl_handle = NULL;
  }
};

TEST_F(FaceMaskDetectionTestSuite, open_close_model) {
  for (size_t test_index = 0; test_index < m_json_object.size(); test_index++) {
    std::string model_name = std::string(m_json_object[test_index]["model"]);
    m_model_path = (m_model_dir / fs::path(model_name)).string();

    TDLModelHandler tdlmodel(m_tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION,
                             m_model_path.c_str(), false);
    ASSERT_NO_FATAL_FAILURE(tdlmodel.open());

    const char *model_path_get =
        CVI_TDL_GetModelPath(m_tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION);

    EXPECT_PRED2([](auto s1, auto s2) { return s1 == s2; }, m_model_path,
                 std::string(model_path_get));
  }
}

TEST_F(FaceMaskDetectionTestSuite, get_vpss_config) {
  uint32_t dstWidth[3] = {768};
  uint32_t dstHeight[3] = {432};

  for (size_t test_index = 0; test_index < m_json_object.size(); test_index++) {
    std::string model_name = std::string(m_json_object[test_index]["model"]);
    m_model_path = (m_model_dir / fs::path(model_name)).string();

    TDLModelHandler tdlmodel(m_tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION,
                             m_model_path.c_str(), false);
    ASSERT_NO_FATAL_FAILURE(tdlmodel.open());
    cvtdl_vpssconfig_t vpssconfig;
    vpssconfig.chn_attr.u32Height = 200;
    vpssconfig.chn_attr.u32Width = 200;
    vpssconfig.chn_attr.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
    vpssconfig.chn_attr.stNormalize.bEnable = false;

    EXPECT_EQ(CVI_TDL_GetVpssChnConfig(m_tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION, 342,
                                       608, 0, &vpssconfig),
              CVI_TDL_SUCCESS);

    float factor[3] = {0.8254323, 0.84385717, 0.84010673};
    float mean[3] = {102.48568, 98.508514, 87.317329};

    uint32_t model_width = dstWidth[test_index];
    uint32_t model_height = dstHeight[test_index];

    EXPECT_EQ(vpssconfig.chn_attr.u32Width, model_width);
    EXPECT_EQ(vpssconfig.chn_attr.u32Height, model_height);
    EXPECT_EQ(vpssconfig.chn_attr.enVideoFormat, VIDEO_FORMAT_LINEAR);
    EXPECT_EQ(vpssconfig.chn_attr.enPixelFormat, PIXEL_FORMAT_RGB_888_PLANAR);

    for (uint32_t i = 0; i < 3; i++) {
      EXPECT_FLOAT_EQ(vpssconfig.chn_attr.stNormalize.factor[i], factor[i]);
    }
    for (uint32_t i = 0; i < 3; i++) {
      EXPECT_FLOAT_EQ(vpssconfig.chn_attr.stNormalize.mean[i], mean[i]);
    }
  }
}

TEST_F(FaceMaskDetectionTestSuite, skip_vpss_preprocess) {
  for (size_t test_index = 0; test_index < m_json_object.size(); test_index++) {
    std::string model_name = std::string(m_json_object[test_index]["model"]);
    m_model_path = (m_model_dir / fs::path(model_name)).string();

    // select image_0 for test
    std::string image_path =
        (m_image_dir / std::string(m_json_object[test_index]["test_images"][0])).string();

    Image frame(image_path, PIXEL_FORMAT_BGR_888);
    ASSERT_TRUE(frame.open());

    {
      TDLModelHandler tdlmodel(m_tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION,
                               m_model_path.c_str(), false);
      ASSERT_NO_FATAL_FAILURE(tdlmodel.open());
      cvtdl_face_t face_meta;
      memset(&face_meta, 0, sizeof(cvtdl_face_t));
      EXPECT_EQ(CVI_TDL_FaceDetection(m_tdl_handle, frame.getFrame(),
                                      CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION, &face_meta),
                CVI_TDL_SUCCESS);
    }
    {
      TDLModelHandler tdlmodel(m_tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION,
                               m_model_path.c_str(), true);
      ASSERT_NO_FATAL_FAILURE(tdlmodel.open());
      TDLObject<cvtdl_face_t> face_meta;
      init_face_meta(face_meta, 1);
      EXPECT_EQ(CVI_TDL_FaceDetection(m_tdl_handle, frame.getFrame(),
                                      CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION, face_meta),
                CVI_TDL_ERR_INFERENCE);
    }
  }
}

TEST_F(FaceMaskDetectionTestSuite, inference) {
  for (size_t test_index = 0; test_index < m_json_object.size(); test_index++) {
    std::string model_name = std::string(m_json_object[test_index]["model"]);
    m_model_path = (m_model_dir / fs::path(model_name)).string();

    TDLModelHandler tdlmodel(m_tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION,
                             m_model_path.c_str(), false);
    ASSERT_NO_FATAL_FAILURE(tdlmodel.open());

    for (int img_idx = 0; img_idx < 1; img_idx++) {
      // select image_0 for test
      std::string image_path =
          (m_image_dir / std::string(m_json_object[test_index]["test_images"][img_idx])).string();

      {
        Image frame(image_path, PIXEL_FORMAT_RGB_888_PLANAR);
        ASSERT_TRUE(frame.open());

        cvtdl_face_t face_meta;
        memset(&face_meta, 0, sizeof(cvtdl_face_t));
        EXPECT_EQ(CVI_TDL_FaceDetection(m_tdl_handle, frame.getFrame(),
                                        CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION, &face_meta),
                  CVI_TDL_SUCCESS);
      }

      {
        Image frame(image_path, PIXEL_FORMAT_BGR_888);
        ASSERT_TRUE(frame.open());

        cvtdl_face_t face_meta;
        memset(&face_meta, 0, sizeof(cvtdl_face_t));
        EXPECT_EQ(CVI_TDL_FaceDetection(m_tdl_handle, frame.getFrame(),
                                        CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION, &face_meta),
                  CVI_TDL_SUCCESS);
      }
    }
  }
}

TEST_F(FaceMaskDetectionTestSuite, accruacy) {
  for (size_t test_index = 0; test_index < m_json_object.size(); test_index++) {
    std::string model_name = std::string(m_json_object[test_index]["model"]);
    m_model_path = (m_model_dir / fs::path(model_name)).string();

    TDLModelHandler tdlmodel(m_tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION,
                             m_model_path.c_str(), false);
    ASSERT_NO_FATAL_FAILURE(tdlmodel.open());

    int img_num = int(m_json_object[test_index]["test_images"].size());
    float iou_threshold = float(m_json_object[test_index]["threshold"]);
    float score_threshold = float(m_json_object[test_index]["threshold_score"]);

    for (int img_idx = 0; img_idx < img_num; img_idx++) {
      std::string image_path =
          (m_image_dir / std::string(m_json_object[test_index]["test_images"][img_idx])).string();

      Image frame(image_path, PIXEL_FORMAT_BGR_888);
      ASSERT_TRUE(frame.open());

      TDLObject<cvtdl_face_t> face_meta;

      {
        EXPECT_EQ(CVI_TDL_FaceDetection(m_tdl_handle, frame.getFrame(),
                                        CVI_TDL_SUPPORTED_MODEL_FACEMASKDETECTION, face_meta),
                  CVI_TDL_SUCCESS);
      }

      for (uint32_t i = 0; i < face_meta->size; i++) {
        float expected_res_x1 =
            float(m_json_object[test_index]["expected_results"][img_idx][1][i][0]);
        float expected_res_y1 =
            float(m_json_object[test_index]["expected_results"][img_idx][1][i][1]);
        float expected_res_x2 =
            float(m_json_object[test_index]["expected_results"][img_idx][1][i][2]);
        float expected_res_y2 =
            float(m_json_object[test_index]["expected_results"][img_idx][1][i][3]);
        float expected_res_mask_score =
            float(m_json_object[test_index]["expected_results"][img_idx][1][i][4]);

        cvtdl_face_info_t expected_faceinfo = {0};
        expected_faceinfo.bbox.x1 = expected_res_x1;
        expected_faceinfo.bbox.y1 = expected_res_y1;
        expected_faceinfo.bbox.x2 = expected_res_x2;
        expected_faceinfo.bbox.y2 = expected_res_y2;
        expected_faceinfo.mask_score = expected_res_mask_score;

        auto comp = [=](cvtdl_face_info_t &pred, cvtdl_face_info_t &expected) {
          if (iou(pred.bbox, expected.bbox) >= iou_threshold &&
              abs(pred.mask_score - expected.mask_score) < score_threshold) {
            return true;
          }
          return false;
        };

        bool matched = match_dets(*face_meta, expected_faceinfo, comp);
        EXPECT_TRUE(matched) << "image path: " << image_path << "\n"
                             << "model path: " << m_model_path << "\n"
                             << "expected bbox: (" << expected_faceinfo.bbox.x1 << ", "
                             << expected_faceinfo.bbox.y1 << ", " << expected_faceinfo.bbox.x2
                             << ", " << expected_faceinfo.bbox.y2 << ", "
                             << expected_faceinfo.mask_score << ")\n"
                             << "[" << face_meta->info[i].bbox.x1 << ","
                             << face_meta->info[i].bbox.y1 << "," << face_meta->info[i].bbox.x2
                             << "," << face_meta->info[i].bbox.y2 << ","
                             << face_meta->info[i].bbox.score << "],\n";
        ;
      }
      CVI_TDL_FreeCpp(face_meta);
    }
  }
}

}  // namespace unitest
}  // namespace cvitdl
