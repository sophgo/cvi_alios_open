#ifndef __CVI_MODULECFG_H_
#define __CVI_MODULECFG_H_
#include "cvi_type.h"
#include "cvi_common.h"

//多级别结构 确定使用的多级数组加number作标定 (多级限定场景 串行结构 不允许存在同一级别有多个多级存在) 大结构必须精简尽量使用指针

typedef struct _PARAM_MODULE_CFG_S {
    CVI_BOOL alios_sys_mode;
    CVI_BOOL alios_vi_mode;
    CVI_BOOL alios_vpss_mode;
    CVI_BOOL alios_venc_mode;
    CVI_BOOL alios_vo_mode;
} PARAM_MODULE_CFG_S;

#endif
