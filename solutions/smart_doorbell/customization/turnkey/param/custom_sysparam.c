/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_sysparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "board_config.h"

PARAM_CLASSDEFINE(PARAM_VB_CFG_S,VBPOOL,CTX,VB)[] = {
    {
        .u16width = 1200,
        .u16height = 1600,
        .u8VbBlkCnt = 6,
        .fmt = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
    },
    {
        .u16width = PANEL_WIDTH,
        .u16height = PANEL_HEIGHT,
        .u8VbBlkCnt = 4,
        .fmt = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
    },
    {
        .u16width = 480,
        .u16height = 720,
        .u8VbBlkCnt = 3,
        .fmt = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
    },
    {
        .u16width = 342,
        .u16height = 608,
        .u8VbBlkCnt = 1,//AI使用
        .fmt = PIXEL_FORMAT_RGB_888_PLANAR,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
    },
};

PARAM_SYS_CFG_S  g_stSysCtx = {
    .u8VbPoolCnt = 4,
    .u8ViCnt = 2,
    .stVPSSMode.enMode = VPSS_MODE_DUAL,
    .stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_OFFLINE,
    .stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_OFFLINE,
    .stVPSSMode.ViPipe[0] = 0,
    .stVPSSMode.ViPipe[1] = 0,
    .stVPSSMode.aenInput[0] = VPSS_INPUT_MEM,
    .stVPSSMode.aenInput[1] = VPSS_INPUT_MEM,
    .pstVbPool = PARAM_CLASS(VBPOOL,CTX,VB),
};

PARAM_SYS_CFG_S * PARAM_GET_SYS_CFG(void) {
    return &g_stSysCtx;
}
