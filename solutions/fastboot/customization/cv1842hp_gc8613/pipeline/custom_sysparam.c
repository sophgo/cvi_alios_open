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
        .bEnable = 0,
        .s32SbmGrp = 0,
        .s32SbmChn = 0,
        .s32WrapBufLine = 64,
        .s32WrapBufSize = 5,
    },
};

PARAM_CLASSDEFINE(PARAM_VB_CFG_S,VBPOOL,CTX,VB)[] = {
    {
        .u16width = 3840,
        .u16height = 2160,
        .fmt = PIXEL_FORMAT_NV12,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 3,
    },
    {
        .u16width = 768,
        .u16height = 572,
        .fmt = PIXEL_FORMAT_NV12,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 3,
    },
	{
		.u16width = 960,
		.u16height = 540,
		.fmt = PIXEL_FORMAT_NV12,
		.enBitWidth = DATA_BITWIDTH_8,
		.enCmpMode = COMPRESS_MODE_NONE,
		.u8VbBlkCnt = 3,
	},
    {
        .u16width = 640,
        .u16height = 384,
        .fmt = PIXEL_FORMAT_RGB_888_PLANAR,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 3,
    },
    {
        .u16width = 768,
        .u16height = 432,
        .fmt = PIXEL_FORMAT_RGB_888_PLANAR,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 3,
    }
};

PARAM_SYS_CFG_S  g_stSysCtx = {
    .u8SbmCnt = 0,
    .pstSbmCfg = PARAM_CLASS(SBM,CTX,CFG),
    .u8VbPoolCnt = 5,
    .pstVbPool = PARAM_CLASS(VBPOOL,CTX,VB),
    .stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_ONLINE,
};

PARAM_SYS_CFG_S * PARAM_GET_SYS_CFG(void) {
    return &g_stSysCtx;
}
