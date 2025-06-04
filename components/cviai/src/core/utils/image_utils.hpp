#pragma once
#include "core/core/cvai_core_types.h"
#include "core/face/cvai_face_types.h"
// #include "face_utils.hpp"

namespace cviai {

CVI_S32 ALIGN_FACE_TO_FRAME(VIDEO_FRAME_INFO_S *srcFrame, VIDEO_FRAME_INFO_S *dstFrame,
                            cvai_face_info_t &face_info);

CVI_S32 crop_image(VIDEO_FRAME_INFO_S *srcFrame, cvai_image_t *dst, cvai_bbox_t *bbox,
                   bool cvtRGB888 = false);

CVI_S32 crop_image_face(VIDEO_FRAME_INFO_S *srcFrame, cvai_image_t *dst,
                        cvai_face_info_t *face_info, bool align = false, bool cvtRGB888 = false);

}  // namespace cviai