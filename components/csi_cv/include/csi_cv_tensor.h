#ifndef __CSI_CV_TENSOR_H__
#define __CSI_CV_TENSOR_H__

#include "csi_cv_base_define.h"

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_DIM (8)
#define NAME_LEN (16)

typedef void *                      csi_cv_tensor_t;

csi_cv_tensor_t csi_cv_create_tensor(csi_cv_s8_t *name,
                                     csi_cv_void_t *data,
                                     csi_cv_s32_t dtype,
                                     csi_cv_s32_t *dim,
                                     csi_cv_s32_t dim_count,
                                     csi_cv_u64_t *stride,
                                     csi_cv_void_t *user_data);

csi_cv_tensor_t csi_cv_create_im_tensor(csi_cv_s8_t *name,
                                        csi_cv_void_t *data,
                                        csi_cv_s32_t dtype,
                                        csi_cv_s32_t cn,
                                        csi_cv_s32_t width,
                                        csi_cv_s32_t height,
                                        csi_cv_s32_t stride,
                                        csi_cv_void_t *user_data);

csi_cv_void_t csi_cv_destroy_tensor(csi_cv_tensor_t tensor);
csi_cv_tensor_t csi_cv_clone_tensor(csi_cv_tensor_t tensor);
bool csi_cv_tensor_is_continue(csi_cv_tensor_t tensor);

csi_cv_void_t *csi_cv_tensor_get_data(csi_cv_tensor_t tensor);
csi_cv_s32_t csi_cv_tensor_get_dtype(csi_cv_tensor_t tensor);
csi_cv_u64_t csi_cv_tensor_get_stride(csi_cv_tensor_t tensor, csi_cv_s32_t index);
csi_cv_s32_t csi_cv_tensor_get_dim(csi_cv_tensor_t tensor, csi_cv_s32_t index);
csi_cv_void_t *csi_cv_tensor_get_userdata(csi_cv_tensor_t tensor);
csi_cv_s8_t *csi_cv_tensor_get_name(csi_cv_tensor_t tensor);

csi_cv_s32_t csi_cv_tensor_get_esize(csi_cv_tensor_t tensor);
csi_cv_s32_t csi_cv_tensor_get_dim_count(csi_cv_tensor_t tensor);
csi_cv_s32_t csi_cv_tensor_set_data(csi_cv_tensor_t tensor, csi_cv_void_t *data);
csi_cv_s32_t csi_cv_tensor_set_val(csi_cv_tensor_t tensor, csi_cv_f64_t val);
csi_cv_tensor_t csi_cv_tensor_create_from(csi_cv_tensor_t src_tensor, csi_cv_s32_t dtype);
csi_cv_tensor_t  csi_cv_tensor_rect(csi_cv_tensor_t src_tensor, csi_cv_rect_t rect);
csi_cv_s32_t  csi_cv_tensor_loateROI(csi_cv_tensor_t src_tensor, pcsi_cv_size_t wholesize, pcsi_cv_point_t ofs);
csi_cv_s32_t csi_cv_tensor_adjustROI(csi_cv_tensor_t src_tensor, csi_cv_s32_t dtop, csi_cv_s32_t dbottom, csi_cv_s32_t dleft, csi_cv_s32_t dright);
#ifdef __cplusplus
}
#endif

#endif
