/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_sysparam.c
 * Description:
 *   ....
 */
#include "board_config.h"
#include "custom_param.h"

PARAM_CLASSDEFINE(PARAM_VB_CFG_S, VBPOOL, CTX, VB)
[] = {
#if !CONFIG_SBM_ENABLE
    {
        .u16width   = 1920,
        .u16height  = 1080,
        .u8VbBlkCnt = 4,
        .fmt        = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode  = COMPRESS_MODE_NONE,
    },
#else
    /* SBM 2UVC 1080p: chn0 uses wrap buffer; pool only serves chn1 (FBM), 2 blocks by design. */
    {
        .u16width   = 1920,
        .u16height  = 1080,
        .u8VbBlkCnt = 2,
        .fmt        = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode  = COMPRESS_MODE_NONE,
    },
#endif
};

PARAM_SYS_CFG_S g_stSysCtx = {
    .u8VbPoolCnt             = 1,
    .u8ViCnt                 = 2,
    .stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_ONLINE,
    // .stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE,
    .stVPSSMode.enMode       = VPSS_MODE_DUAL,
    .stVPSSMode.ViPipe[0]    = 0,
    .stVPSSMode.aenInput[0]  = VPSS_INPUT_MEM,
    .stVPSSMode.ViPipe[1]    = 0,
    .stVPSSMode.aenInput[1]  = VPSS_INPUT_ISP,
    .pstVbPool               = PARAM_CLASS(VBPOOL, CTX, VB),
};

PARAM_SYS_CFG_S* PARAM_GET_SYS_CFG(void)
{
    return &g_stSysCtx;
}
