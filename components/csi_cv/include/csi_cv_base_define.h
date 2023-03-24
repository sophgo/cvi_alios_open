#ifndef __CSI_CV_BASE_DEFINE_H__
#define __CSI_CV_BASE_DEFINE_H__

#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"{
#endif

typedef float                       csi_cv_f32_t;
typedef double                      csi_cv_f64_t;
typedef signed int                  csi_cv_s32_t;
typedef unsigned int                csi_cv_u32_t;
typedef signed short                csi_cv_s16_t;
typedef unsigned short              csi_cv_u16_t;
typedef signed char                 csi_cv_s8_t;
typedef unsigned char               csi_cv_u8_t;
typedef void                        csi_cv_void_t;

typedef int64_t                     csi_cv_s64_t;
typedef uint64_t                    csi_cv_u64_t;

typedef void *                      csi_cv_handle_t;

typedef unsigned char               csi_cv_bool8_t;
typedef unsigned short              csi_cv_bool16_t;
typedef unsigned int                csi_cv_bool32_t;

#define CSI_CV_PI   3.1415926535897932384626433832795
#define CSI_CV_2PI  6.283185307179586476925286766559
#define CSI_CV_LOG2 0.69314718055994530941723212145818

enum csi_cv_dtype_enum {
    CSI_CV_DTYPE_UINT8,
    CSI_CV_DTYPE_INT8,
    CSI_CV_DTYPE_UINT16,
    CSI_CV_DTYPE_INT16,
    CSI_CV_DTYPE_UINT32,
    CSI_CV_DTYPE_INT32,
    CSI_CV_DTYPE_UINT64,
    CSI_CV_DTYPE_INT64,
    CSI_CV_DTYPE_FLOAT16,
    CSI_CV_DTYPE_FLOAT32,
    CSI_CV_DTYPE_FLOAT64,
    CSI_CV_DTYPE_SIZE,
};

typedef struct csi_cv_point_tag {
    csi_cv_s32_t                    x;
    csi_cv_s32_t                    y;
}csi_cv_point_t, *pcsi_cv_point_t;

typedef struct csi_cv_rect_tag {   
    csi_cv_point_t               start;
    csi_cv_s32_t                 width;
    csi_cv_s32_t                height;
}csi_cv_rect_t, *pcsi_cv_rect_t;

typedef struct csi_cv_size_tag {
    csi_cv_s32_t                    width;
    csi_cv_s32_t                    height;   
}csi_cv_size_t, *pcsi_cv_size_t;

#define KP_DEFAULT_NUM 128

typedef struct csi_cv_pointf_tag {
    csi_cv_f32_t                x;
    csi_cv_f32_t                y;
}csi_cv_pointf_t, *pcsi_cv_pointf_t;

#ifdef __cplusplus
}
#endif

#endif
