#include "face_utils.hpp"
#include "transform.hpp"
#include "cviai_log.hpp"

// #include "opencv2/imgproc.hpp"

#include <algorithm>

const static float reference_points[2][5] = {
  {38.29459953, 73.53179932, 56.02519989, 41.54930115, 70.72990036},
  {51.69630051, 51.50139999, 71.73660278, 92.36550140, 92.20410156}};

using namespace std;

namespace cviai {

int face_align(cvai_image_t &src_image, cvai_image_t &dst_image, const cvai_face_info_t &face_info) {
  cvai_pts_t ref_pts;
  ref_pts.size = 5;
  ref_pts.x = (float *) malloc(ref_pts.size * sizeof(float));
  ref_pts.y = (float *) malloc(ref_pts.size * sizeof(float));
  memcpy(ref_pts.x, &reference_points[0], 5 * sizeof(float));
  memcpy(ref_pts.y, &reference_points[1], 5 * sizeof(float));

  cvai_affine_matrix_t m;
  m = get_similarity_transform_matrix(face_info.pts, ref_pts);

  /* TODO: warpAffine (csi-cv tool) */


  return 0;
}

}  // namespace cviai
