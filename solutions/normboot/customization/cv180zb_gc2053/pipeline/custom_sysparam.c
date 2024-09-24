/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_sysparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "board_config.h"

PARAM_CLASSDEFINE(PARAM_SBM_CFG_S,SBM,CTX,CFG)[] = {
    {
        .bEnable = 1,
        .s32SbmGrp = 0,
        .s32SbmChn = 0,
        .s32WrapBufLine = 64,
        .s32WrapBufSize = 5,
    },
};

PARAM_CLASSDEFINE(PARAM_VB_CFG_S,VBPOOL,CTX,VB)[] = {
    {
        .u16width = 720,
        .u16height = 576,
        .fmt = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 4,
    },
    {
        .u16width = 720,
        .u16height = 576,
        .fmt = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 4,
    },
    {
        .u16width = 384,
        .u16height = 256,
        .fmt = PIXEL_FORMAT_RGB_888_PLANAR,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 2,
    },
    {
        .u16width = 640,
        .u16height = 360,
        .fmt = PIXEL_FORMAT_YUV_400,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 2,
    }
};

PARAM_SYS_CFG_S  g_stSysCtx = {
    .u8SbmCnt = 1,
    .pstSbmCfg = PARAM_CLASS(SBM,CTX,CFG),
    .u8VbPoolCnt = 4,
    .pstVbPool = PARAM_CLASS(VBPOOL,CTX,VB),
    .stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_ONLINE,
    .stVPSSMode.enMode = VPSS_MODE_DUAL,
    .stVPSSMode.aenInput[0] = VPSS_INPUT_MEM,
    .stVPSSMode.aenInput[1] = VPSS_INPUT_ISP,
};

PARAM_SYS_CFG_S * PARAM_GET_SYS_CFG(void) {
    return &g_stSysCtx;
}
