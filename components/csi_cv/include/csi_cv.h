#ifndef __CSI_CV_H__
#define __CSI_CV_H__

#include "csi_cv_base_define.h"
#include "csi_cv_tensor.h"
#include "csi_cv_error.h"
#include "csi_cv_alg.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"{
#endif

csi_cv_s32_t csi_cv_gaussian(csi_cv_tensor_t src_tensor,
                                csi_cv_tensor_t dst_tensor,
                                csi_cv_s32_t kernel_width,
                                csi_cv_s32_t kernel_height,
                                csi_cv_f64_t sigma_x,
                                csi_cv_f64_t sigma_y,
                                csi_cv_s32_t border_type);

csi_cv_s32_t csi_cv_dilate(csi_cv_tensor_t src_tensor,
                                csi_cv_tensor_t dst_tensor,
                                csi_cv_s32_t border_type,
                                csi_cv_s32_t kernel_x,
                                csi_cv_s32_t kernel_y,
                                csi_cv_s32_t anchor_x,
                                csi_cv_s32_t anchor_y);

csi_cv_s32_t csi_cv_erode(csi_cv_tensor_t src_tensor,
                                csi_cv_tensor_t dst_tensor,
                                csi_cv_s32_t border_type,
                                csi_cv_s32_t kernel_x,
                                csi_cv_s32_t kernel_y,
                                csi_cv_s32_t anchor_x,
                                csi_cv_s32_t anchor_y);

csi_cv_s32_t csi_cv_filter(csi_cv_tensor_t src_tensor,
                                csi_cv_tensor_t kernel_tensor,
                                csi_cv_tensor_t dst_tensor,
                                csi_cv_s32_t border_type,
                                csi_cv_s32_t anchor_x,
                                csi_cv_s32_t anchor_y,
                                csi_cv_f64_t delta);

csi_cv_s32_t csi_cv_mean_blur(csi_cv_tensor_t src_tensor,
                                    csi_cv_tensor_t dst_tensor,
                                    csi_cv_s32_t border_type,
                                    csi_cv_s32_t kernel_x,
                                    csi_cv_s32_t kernel_y,
                                    csi_cv_s32_t anchor_x,
                                    csi_cv_s32_t anchor_y,
                                    csi_cv_s32_t normalize);

csi_cv_s32_t csi_cv_threshold(csi_cv_tensor_t src_tensor,
                                csi_cv_tensor_t dst_tensor,
                                csi_cv_f64_t thresh,
                                csi_cv_f64_t max_value,
                                csi_cv_s32_t threshold_type);

csi_cv_s32_t csi_cv_sobel(csi_cv_tensor_t src_tensor,
                                csi_cv_tensor_t dst_tensor,
                                csi_cv_s32_t border_type,
                                csi_cv_s32_t kernel_size,
                                csi_cv_f64_t scale,
                                csi_cv_f64_t delta,
                                csi_cv_s32_t dx,
                                csi_cv_s32_t dy);

csi_cv_s32_t csi_cv_find_contours(csi_cv_tensor_t src,
                                    csi_cv_s32_t mode,
                                    csi_cv_s32_t method,
                                    csi_cv_point_t *offset,
                                    csi_cv_s32_t max_num,
                                    csi_cv_s32_t *dst_num,
                                    csi_cv_tensor_t *dst);

csi_cv_s32_t csi_cv_median_blur(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t ksize);
csi_cv_s32_t csi_cv_resize(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t interpolation);

csi_cv_s32_t csi_cv_csc_bgr_bgr(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue);
csi_cv_s32_t csi_cv_csc_bgr_bgr5x5(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue, csi_cv_s32_t green_bits);
csi_cv_s32_t csi_cv_csc_bgr5x5_bgr(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue, csi_cv_s32_t green_bits);
csi_cv_s32_t csi_cv_csc_bgr_gray(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue);
csi_cv_s32_t csi_cv_csc_gray_bgr(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_csc_bgr_yuv(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue, csi_cv_s32_t is_cbcr);
csi_cv_s32_t csi_cv_csc_yuv_bgr(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue, csi_cv_s32_t is_cbcr);
csi_cv_s32_t csi_cv_csc_bgr_xyz(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue);
csi_cv_s32_t csi_cv_csc_xyz_bgr(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue);
csi_cv_s32_t csi_cv_csc_yuv420sp_bgr(csi_cv_tensor_t src_tensor, csi_cv_tensor_t uv_tensor, csi_cv_tensor_t dst_tensor,
                                        csi_cv_s32_t swap_blue, csi_cv_s32_t uidx);
csi_cv_s32_t csi_cv_csc_yuv420p_bgr(csi_cv_tensor_t src_tensor, csi_cv_tensor_t u_tensor, csi_cv_tensor_t v_tensor,
                                    csi_cv_tensor_t dst_tensor, csi_cv_s32_t swap_blue, csi_cv_s32_t uidx);
csi_cv_s32_t csi_cv_csc_bgr_yuv420p(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor,
                                    csi_cv_tensor_t uv_tensor, csi_cv_s32_t swap_blue, csi_cv_s32_t uidx);
csi_cv_s32_t csi_cv_csc_bgr_yuv420sp(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor,
                                    csi_cv_tensor_t uv_tensor, csi_cv_s32_t swap_blue, csi_cv_s32_t uidx);
csi_cv_s32_t csi_cv_csc_yuv422_bgr(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor,
                                    csi_cv_s32_t swap_blue, csi_cv_s32_t uidx, csi_cv_s32_t ycn);

csi_cv_s32_t csi_cv_add(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_absdiff(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor, csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_max(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_min(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_mul(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor, csi_cv_f64_t scalar);
csi_cv_s32_t csi_cv_sub(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_div(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor, csi_cv_f64_t scalar);
csi_cv_s32_t csi_cv_recip(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_f64_t scalar);
csi_cv_s32_t csi_cv_add_weighted(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor, csi_cv_f64_t *scalar);
csi_cv_s32_t csi_cv_cmp(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor, csi_cv_s32_t operation);
csi_cv_s32_t csi_cv_split(csi_cv_tensor_t src_tensor, csi_cv_tensor_t *dst_tensor);
csi_cv_s32_t csi_cv_merge(csi_cv_tensor_t *src_tensor, csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_abs(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_sqrt(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_convert_scale_abs(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_f64_t alpha, csi_cv_f64_t beta);
csi_cv_s32_t csi_cv_convert(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_f64_t alpha, csi_cv_f64_t beta);
csi_cv_s32_t csi_cv_pow(csi_cv_tensor_t src_tensor, csi_cv_f64_t power, csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_min_max_loc(csi_cv_tensor_t src_tensor, csi_cv_tensor_t mask, csi_cv_f64_t *min_val,
                                csi_cv_f64_t *max_val, csi_cv_point_t *min_loc, csi_cv_point_t *max_loc);
csi_cv_s32_t csi_cv_norm(csi_cv_tensor_t src_tensor, csi_cv_tensor_t mask, csi_cv_s32_t type, csi_cv_f64_t *res);
csi_cv_s32_t csi_cv_contour_area(csi_cv_tensor_t src_tensor, csi_cv_s32_t oriented, csi_cv_f64_t *area);
csi_cv_s32_t csi_cv_bounding_rect(csi_cv_tensor_t src_tensor, csi_cv_rect_t *rect);

csi_cv_s32_t csi_cv_and8u(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_or8u(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_xor8u(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t src2_tensor,  csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_not8u(csi_cv_tensor_t src1_tensor, csi_cv_tensor_t dst_tensor);
csi_cv_s32_t csi_cv_fast_atan32f(csi_cv_f32_t *src1_data, csi_cv_f32_t *src2_data, csi_cv_f32_t *dst_data, csi_cv_s32_t length, bool in_degree);
csi_cv_s32_t csi_cv_fast_atan64f(csi_cv_f64_t *src1_data, csi_cv_f64_t *src2_data, csi_cv_f64_t *dst_data, csi_cv_s32_t length, bool in_degree);

csi_cv_s32_t csi_cv_warp_affine(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_tensor_t map_tensor, csi_cv_s32_t interpolation,  csi_cv_s32_t border_mode, csi_cv_f64_t *border_value);
csi_cv_s32_t csi_cv_warp_perspective(csi_cv_tensor_t src_tensor, csi_cv_tensor_t dst_tensor, csi_cv_tensor_t map_tensor, csi_cv_s32_t interpolation,  csi_cv_s32_t border_mode, csi_cv_f64_t *border_value);
csi_cv_tensor_t get_affine_transform(const csi_cv_pointf_t *src, const csi_cv_pointf_t *dst);

void csi_cv_match_template( csi_cv_tensor_t img, csi_cv_tensor_t templ, csi_cv_tensor_t result, csi_cv_s32_t method, csi_cv_tensor_t mask );
#ifdef __cplusplus
}
#endif

#endif

