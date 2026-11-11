#include "core/cviai_core.h"
#include "transform.hpp"
#include "Eigen/Core"
#include "Eigen/Dense"
#include "cviai_log.hpp"

#include <iostream>

typedef enum {UNKNOWN = 0, SVD_Decomposition, QR_Decomposition, Normal_Equations} LLSS_solve_method_e;

/**
 * Solve similarity transform matrix (no reflection)
 */
Eigen::Matrix<float, 4, 1> solve_stm_no_reflection(const cvai_pts_t &src_pts, const cvai_pts_t &dst_pts, float *distance) {
  const LLSS_solve_method_e solve_method = SVD_Decomposition;

  Eigen::Matrix<float, 10, 4> A;
  A << src_pts.x[0],  src_pts.y[0], 1, 0,
       src_pts.x[1],  src_pts.y[1], 1, 0,
       src_pts.x[2],  src_pts.y[2], 1, 0,
       src_pts.x[3],  src_pts.y[3], 1, 0,
       src_pts.x[4],  src_pts.y[4], 1, 0,
       src_pts.y[0], -src_pts.x[0], 0, 1, 
       src_pts.y[1], -src_pts.x[1], 0, 1, 
       src_pts.y[2], -src_pts.x[2], 0, 1, 
       src_pts.y[3], -src_pts.x[3], 0, 1, 
       src_pts.y[4], -src_pts.x[4], 0, 1; 
  Eigen::Matrix<float, 10, 1> b;
  b << dst_pts.x[0], 
       dst_pts.x[1], 
       dst_pts.x[2], 
       dst_pts.x[3], 
       dst_pts.x[4], 
       dst_pts.y[0], 
       dst_pts.y[1], 
       dst_pts.y[2], 
       dst_pts.y[3], 
       dst_pts.y[4]; 
  Eigen::Matrix<float, -1, -1> x;

  switch (solve_method) {
  case SVD_Decomposition: {
    x = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
  } break;
  case QR_Decomposition: {
    x = A.colPivHouseholderQr().solve(b);
  } break;
  case Normal_Equations: {
    x = (A.transpose() * A).ldlt().solve(A.transpose() * b);
  } break;
  default: {
    LOGE(AISDK_TAG, "Unknown solve method %x\n", solve_method);
  } break;
  }

  if (distance != NULL) {
    *distance = (A * x - b).squaredNorm();
  }

  return x;
}

/**
 * Solve Perspective transform matrix
 */
Eigen::Matrix<float, 8, 1> solve_ptm(cvai_4_pts_t &src_pts, cvai_4_pts_t &dst_pts) {
  float *x = src_pts.x;
  float *y = src_pts.y;
  float *u = dst_pts.x;
  float *v = dst_pts.y;
// clang-format off
  Eigen::Matrix<float, 8, 8> A;
  A << x[0],  y[0],     1,     0,     0,     0, -x[0] * u[0], -y[0] * u[0], 
       x[1],  y[1],     1,     0,     0,     0, -x[1] * u[1], -y[1] * u[1],
       x[2],  y[2],     1,     0,     0,     0, -x[2] * u[2], -y[2] * u[2],
       x[3],  y[3],     1,     0,     0,     0, -x[3] * u[3], -y[3] * u[3],
          0,     0,     0,  x[0],  y[0],     1, -x[0] * v[0], -y[0] * v[0],
          0,     0,     0,  x[1],  y[1],     1, -x[1] * v[1], -y[1] * v[1],
          0,     0,     0,  x[2],  y[2],     1, -x[2] * v[2], -y[2] * v[2],
          0,     0,     0,  x[3],  y[3],     1, -x[3] * v[3], -y[3] * v[3]; 
  Eigen::Matrix<float, 8, 1> b;
  b << u[0], 
       u[1], 
       u[2], 
       u[3], 
       v[0], 
       v[1], 
       v[2],
       v[3];
  Eigen::Matrix<float, -1, -1> _x;

  _x = A.colPivHouseholderQr().solve(b);
  // _x = A.ldlt().solve(b);
// clang-format on

  return _x;
}

namespace cviai {
  cvai_affine_matrix_t get_similarity_transform_matrix(const cvai_pts_t &src_pts, const cvai_pts_t &dst_pts) {
    cvai_affine_matrix_t m;
    memset(&m, 0, sizeof(cvai_affine_matrix_t));

    float d_0;
    Eigen::Matrix<float, 4, 1> stm_0 = solve_stm_no_reflection(src_pts, dst_pts, &d_0);

    cvai_pts_t src_pts_reflect;
    src_pts_reflect.size = 5;
    src_pts_reflect.x = (float *)malloc(5 * sizeof(float));
    src_pts_reflect.y = (float *)malloc(5 * sizeof(float));
    for (uint32_t i = 0; i < 5; ++i) {
      src_pts_reflect.x[i] = -src_pts.x[i];
      src_pts_reflect.y[i] = src_pts.y[i];
    }
    float d_1;
    Eigen::Matrix<float, 4, 1> stm_1 = solve_stm_no_reflection(src_pts_reflect, dst_pts, &d_1);

    bool do_reflect = d_0 > d_1;
    Eigen::Matrix<float, 4, 1> stm = (do_reflect) ? stm_1 : stm_0;

    m.m[0][0] = stm[0];
    m.m[0][1] = stm[1];
    m.m[0][2] = stm[2];
    m.m[1][0] = -stm[1];
    m.m[1][1] = stm[0];
    m.m[1][2] = stm[3];
    if (do_reflect) {
      m.m[0][0] *= -1.;
      m.m[1][0] *= -1.;
    }

    CVI_AI_FreeCpp(&src_pts_reflect);

    return m;
  }
}