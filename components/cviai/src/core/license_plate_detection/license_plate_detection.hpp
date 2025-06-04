#pragma once
#include <cvi_comm_vb.h>
#include "core.hpp"
#include "core/object/cvai_object_types.h"

#include "license_plate_detection_utils.hpp"
// #include "opencv2/opencv.hpp"

namespace cviai {

/* WPODNet */
class LicensePlateDetection final : public Core {
 public:
  LicensePlateDetection();
  virtual ~LicensePlateDetection();
  int inference(VIDEO_FRAME_INFO_S *frame, cvai_object_t *vehicle_meta);
  int setupInputPreprocess(std::vector<InputPreprecessSetup> *data) override;

 private:
  int vpssPreprocess(VIDEO_FRAME_INFO_S *srcFrame, VIDEO_FRAME_INFO_S *dstFrame,
                     VPSSConfig &vpss_config) override;
  bool reconstruct(float *t_prob, float *t_trans, CornerPts &c_pts, float &ret_prob,
                   float threshold_prob = 0.9);
#if 0
  void prepareInputTensor(cv::Mat &input_mat);
#endif

  int vehicle_h, vehicle_w;
  int out_tensor_h, out_tensor_w;
};
}  // namespace cviai
