#ifndef _CVI_FACE_UTILS_H_
#define _CVI_FACE_UTILS_H_

#include "core/face/cvai_face_types.h"
// #include "opencv2/opencv.hpp"

#include <cvi_comm_video.h>

namespace cviai {
#if 0
int face_align(const cv::Mat &image, cv::Mat &aligned, const cvai_face_info_t &face_info);
#endif
int face_align(cvai_image_t &src_image, cvai_image_t &dst_image, const cvai_face_info_t &face_info);
}  // namespace cviai
#endif
