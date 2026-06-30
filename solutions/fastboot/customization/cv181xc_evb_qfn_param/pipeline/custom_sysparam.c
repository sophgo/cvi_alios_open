/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_sysparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "board_config.h"

// #define FBM_ENABLE

PARAM_CLASSDEFINE(PARAM_SBM_CFG_S,SBMCFG,CTX,SBM)[] = {
    {
        .bEnable        = 1,
        .s32SbmGrp      = 0,
        .s32SbmChn      = 0,
        .s32WrapBufLine = 64,
        .s32WrapBufSize = 5,
    },
};

PARAM_CLASSDEFINE(PARAM_VB_CFG_S,VBPOOL,CTX,VB)[] = {
    {
        .u16width   = 720,
        .u16height  = 576,
        .fmt        = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode  = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 3,
    },
    {
        .u16width   = 960,
        .u16height  = 540,
        .fmt        = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode  = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 3,
    },
    {
        .u16width   = 384,
        .u16height  = 256,
        .fmt        = PIXEL_FORMAT_RGB_888_PLANAR,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode  = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 2,
    },
    {
        .u16width   = 640,
        .u16height  = 360,
        .fmt        = PIXEL_FORMAT_YUV_400,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode  = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 2,
    },
#ifdef FBM_ENABLE
    {
        .u16width   = 2560,
        .u16height  = 1440,
        .fmt        = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode  = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 4,
    },
#endif
};

PARAM_SYS_CFG_S  g_stSysCtx = {
#ifdef FBM_ENABLE
    .u8SbmCnt = 0,
    .u8VbPoolCnt = 5,
#else
    .u8SbmCnt = 1,
    .u8VbPoolCnt = 4,
#endif
    .pstSbmCfg = PARAM_CLASS(SBMCFG,CTX,SBM),
    .pstVbPool = PARAM_CLASS(VBPOOL,CTX,VB),
    .stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_ONLINE,
    .stVPSSMode = {
        .enMode = VPSS_MODE_DUAL,
        .aenInput[0] = VPSS_INPUT_MEM,
        .ViPipe[0] = 0,
        .aenInput[1] = VPSS_INPUT_ISP,
        .ViPipe[1] = 1,
    },
    .u8ViCnt = 1,
};

PARAM_SYS_CFG_S * PARAM_GET_SYS_CFG(void) {
    return &g_stSysCtx;
}
