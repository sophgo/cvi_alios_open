#ifndef __CSI_CV_ERROR_H__
#define __CSI_CV_ERROR_H__

#include "csi_cv_base_define.h"
#include "csi_cv_tensor.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif


#define ERROR_SUCCESS                       (0)  // no error
#define ERROR_DTYPE                         (-1) // unsupported data type
#define ERROR_MEMORY                        (-2) // out of memory
#define ERROR_PARAM                         (-3) // invlalided parameter
#define ERROR_IMAGESIZE                     (-4) // unsupported imaged size
#define ERROR_UNIMPLEMENT                   (-5) // unimplemented

#ifdef __cplusplus
}
#endif

#endif

