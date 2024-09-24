#ifndef __CVI_SYSCFG_H_
#define __CVI_SYSCFG_H_
#include "cvi_type.h"
#include "cvi_common.h"
#include "cvi_comm_vi.h"
#include "cvi_comm_sys.h"

//多级别结构 确定使用的多级数组加number作标定 (多级限定场景 串行结构 不允许存在同一级别有多个多级存在) 大结构必须精简尽量使用指针

typedef struct _PARAM_VB_CFG_S {
    CVI_U16            u16width;
    CVI_U16            u16height;
    PIXEL_FORMAT_E      fmt;
    DATA_BITWIDTH_E     enBitWidth;
    COMPRESS_MODE_E     enCmpMode;
    CVI_U8            u8VbBlkCnt;
} PARAM_VB_CFG_S;

typedef struct PARAM_SWITCH_CFG_T {
    CVI_BOOL bMipiSwitchEnable;
    CVI_U32  u32MipiSwitchGpioIdx;
    CVI_U32  u32MipiSwitchGpio;
    CVI_BOOL bMipiSwitchPull;
    CVI_U32  u32SwitchPipe0;
    CVI_U32  u32SwitchPipe1;
} PARAM_SWITCH_CFG_S;

typedef struct _PARAM_SBM_CFG_S {
    CVI_BOOL bEnable;
    CVI_S32 s32SbmGrp;
    CVI_S32 s32SbmChn;
    CVI_U32 s32WrapBufLine;
    CVI_U32 s32WrapBufSize;
} PARAM_SBM_CFG_S;

typedef struct _PARAM_SYS_CFG_S {
    PARAM_VB_CFG_S * pstVbPool;
    CVI_U8 u8VbPoolCnt;
    CVI_U8 u8ViCnt;
    CVI_U8 u8VpssCnt;
    VI_VPSS_MODE_S stVIVPSSMode;
    VPSS_MODE_S stVPSSMode;
    PARAM_SWITCH_CFG_S stSwitchCfg;
    CVI_U8 u8SbmCnt;
    PARAM_SBM_CFG_S *pstSbmCfg;
} PARAM_SYS_CFG_S;

#endif
