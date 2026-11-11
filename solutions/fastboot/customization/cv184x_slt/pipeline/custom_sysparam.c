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
        .u16width = 1920,
        .u16height = 1080,
        .fmt = PIXEL_FORMAT_NV12,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 2,
    },
    {
        .u16width = 640,
        .u16height = 480,
        .fmt = PIXEL_FORMAT_NV12,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
        .u8VbBlkCnt = 3,
    },
};

PARAM_SYS_CFG_S  g_stSysCtx = {
    .u8VbPoolCnt = 2,
    .pstVbPool = PARAM_CLASS(VBPOOL,CTX,VB),
    .stVIVPSSMode.aenMode[0] = VI_ONLINE_VPSS_ONLINE,
};

PARAM_SYS_CFG_S * PARAM_GET_SYS_CFG(void) {
    return &g_stSysCtx;
}
