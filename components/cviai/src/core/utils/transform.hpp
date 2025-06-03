#pragma once
#include "core/core/cvai_core_types.h"

namespace cviai {
  cvai_affine_matrix_t get_similarity_transform_matrix(const cvai_pts_t &src_pts, const cvai_pts_t &dst_pts);
}