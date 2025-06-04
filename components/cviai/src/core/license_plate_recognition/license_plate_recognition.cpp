#include "license_plate_recognition.hpp"
#include "decode_tool.hpp"

#include "core/cviai_types_mem.h"
#include "core/cviai_types_mem_internal.h"
#include "core_utils.hpp"
#include "face_utils.hpp"

#include "cvi_sys.h"
#include "opencv2/opencv.hpp"

#include <iostream>
#include <sstream>

#define LICENSE_PLATE_TW_HEIGHT 24
#define LICENSE_PLATE_TW_WIDTH 94
#define LICENSE_PLATE_CN_HEIGHT 30
#define LICENSE_PLATE_CN_WIDTH 122

#define OUTPUT_NAME "id_code_ReduceMean_dequant"

#define DEBUG_LICENSE_PLATE_DETECTION 0

namespace cviai {

LicensePlateRecognition::LicensePlateRecognition(LP_FORMAT format) : Core(CVI_MEM_SYSTEM) {
  if (format == TAIWAN) {
    this->format = TAIWAN;
    this->lp_height = LICENSE_PLATE_TW_HEIGHT;
    this->lp_width = LICENSE_PLATE_TW_WIDTH;
  } else if (format == CHINA) {
    this->format = CHINA;
    this->lp_height = LICENSE_PLATE_CN_HEIGHT;
    this->lp_width = LICENSE_PLATE_CN_WIDTH;
  } else {
    LOGE("unknown region: %d\n", format);
    exit(CVI_FAILURE);
  }
}

LicensePlateRecognition::~LicensePlateRecognition() {}

int LicensePlateRecognition::inference(VIDEO_FRAME_INFO_S *frame,
                                       cvai_object_t *vehicle_plate_meta) {
  if (frame->stVFrame.enPixelFormat != PIXEL_FORMAT_RGB_888) {
    LOGE("Error: pixel format not match PIXEL_FORMAT_RGB_888.\n");
    return CVI_FAILURE;
  }
#if DEBUG_LICENSE_PLATE_DETECTION
  printf("[%s:%d] inference\n", __FILE__, __LINE__);
  std::stringstream s_str;
#endif
  bool do_unmap = false;
  if (frame->stVFrame.pu8VirAddr[0] == NULL) {
    frame->stVFrame.pu8VirAddr[0] =
        (CVI_U8 *)CVI_SYS_MmapCache(frame->stVFrame.u64PhyAddr[0], frame->stVFrame.u32Length[0]);
    do_unmap = true;
  }
  cv::Mat cv_frame(frame->stVFrame.u32Height, frame->stVFrame.u32Width, CV_8UC3,
                   frame->stVFrame.pu8VirAddr[0], frame->stVFrame.u32Stride[0]);
  for (size_t n = 0; n < vehicle_plate_meta->size; n++) {
    cvai_vehicle_meta *v_meta = vehicle_plate_meta->info[n].vehicle_properity;
    if (v_meta == NULL) {
      continue;
    }
    cv::Point2f src_points[4] = {
        cv::Point2f(v_meta->license_pts.x[0], v_meta->license_pts.y[0]),
        cv::Point2f(v_meta->license_pts.x[1], v_meta->license_pts.y[1]),
        cv::Point2f(v_meta->license_pts.x[2], v_meta->license_pts.y[2]),
        cv::Point2f(v_meta->license_pts.x[3], v_meta->license_pts.y[3]),
    };
    cv::Point2f dst_points[4] = {
        cv::Point2f(0, 0),
        cv::Point2f(this->lp_width, 0),
        cv::Point2f(this->lp_width, this->lp_height),
        cv::Point2f(0, this->lp_height),
    };
    cv::Mat M = cv::getPerspectiveTransform(src_points, dst_points);
    cv::Mat sub_cvFrame;
    cv::warpPerspective(cv_frame, sub_cvFrame, M, cv::Size(this->lp_width, this->lp_height),
                        cv::INTER_LINEAR);
    cv::Mat greyMat;
    cv::cvtColor(sub_cvFrame, greyMat, cv::COLOR_RGB2GRAY); /* BGR or RGB ? */
    cv::cvtColor(greyMat, sub_cvFrame, cv::COLOR_GRAY2RGB);

    prepareInputTensor(sub_cvFrame);

    std::vector<VIDEO_FRAME_INFO_S *> dummyFrames = {frame};
    run(dummyFrames);

    float *out_code = getOutputRawPtr<float>(OUTPUT_NAME);

    std::string id_number;
    if (!LPR::greedy_decode(out_code, id_number, format)) {
      LOGE("LPR::decode error!!\n");
      return CVI_FAILURE;
    }

    strncpy(v_meta->license_char, id_number.c_str(), sizeof(v_meta->license_char));
  }
  if (do_unmap) {
    CVI_SYS_Munmap((void *)frame->stVFrame.pu8VirAddr[0], frame->stVFrame.u32Length[0]);
    frame->stVFrame.pu8VirAddr[0] = NULL;
  }
  return CVI_SUCCESS;
}

void LicensePlateRecognition::prepareInputTensor(cv::Mat &input_mat) {
  const TensorInfo &tinfo = getInputTensorInfo(0);
  int8_t *input_ptr = tinfo.get<int8_t>();

  cv::Mat tmpchannels[3];
  cv::split(input_mat, tmpchannels);

  for (int c = 0; c < 3; ++c) {
    tmpchannels[c].convertTo(tmpchannels[c], CV_8UC1);

    int size = tmpchannels[c].rows * tmpchannels[c].cols;
    for (int r = 0; r < tmpchannels[c].rows; ++r) {
      memcpy(input_ptr + size * c + tmpchannels[c].cols * r, tmpchannels[c].ptr(r, 0),
             tmpchannels[c].cols);
    }
  }
}

}  // namespace cviai
