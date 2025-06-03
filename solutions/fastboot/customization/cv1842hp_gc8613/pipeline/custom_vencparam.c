/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_vencparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "cvi_venc.h"

PARAM_CLASSDEFINE(PARAM_VENC_CHN_CFG_S,VENCCFG,CTX,VENC)[] = {
    {
        .stChnParam = {
            .bEnable = 1,
            .u8VencChn = 0,
            .enBindMode = VENC_BIND_VPSS,
            .astChn[0] = {
                .enModId = CVI_ID_VPSS,
                .s32DevId = 0,
                .s32ChnId = 0,
            },
            .astChn[1] = {
                .enModId = CVI_ID_VENC,
                .s32DevId = 0,
                .s32ChnId = 0,
            },
            .u8Profile = 0,
            .u16Width = 1920,
            .u16Height = 1080,
            .u8EsBufQueueEn = 1,
            .u16EnType = PT_H265,
            .u32BitStreamBufSize = 838656,
            .StreamTo = 1,
            .u8EntropyEncModeI = 0,
            .u8EntropyEncModeP = 0,
        },
        .stGopParam = {
            .u16gopMode = 0,
            .s8IPQpDelta = 0,
        },
        .stRcParam = {
            .u16Gop = 50,
            .u8SrcFrameRate = 25,
            .u8DstFrameRate = 25,
            .u16BitRate = 3000,
            .u8Qfactor = 60,
            .u32MaxBitRate = 3000,
            .u8VariFpsEn = 0,
            .u8StartTime = 2,
            .u16RcMode = VENC_RC_MODE_H265CBR,
            .u16FirstFrmstartQp = 30,
            .u16InitialDelay = 1000,
            .u16ThrdLv = 2,
            .u16BgDeltaQp = 0,
            .u8MinIprop = 1,
            .u8MaxIprop = 100,
            .u8MinIqp = 1,
            .u8MaxIqp = 50,
            .u8MinQp = 1,
            .u8MaxQp = 50,
            .u8MaxReEncodeTimes = 0,
            .u8QpMapEn = 0,
            .u8ChangePos = 75,
            .bSingleCore = 0,
            .u32FixedIQp = 38,
            .u32FixedPQp = 38,
            .u32Duration = 75,
            .s32MinStillPercent = 10,
            .u32MaxStillQP = 38,
            .u32MinStillPSNR = 0,
            .u32MotionSensitivity = 24,
            .s32AvbrFrmLostOpen = 0,
            .s32AvbrFrmGap = 1,
            .s32AvbrPureStillThr = 4,
        },
    },
    {
        .stChnParam = {
            .bEnable = 1,
            .u8VencChn = 1,
            .enBindMode = VENC_BIND_VPSS,
            .astChn[0] = {
                .enModId = CVI_ID_VPSS,
                .s32DevId = 2,
                .s32ChnId = 0,
            },
            .astChn[1] = {
                .enModId = CVI_ID_VENC,
                .s32DevId = 0,
                .s32ChnId = 1,
            },
            .u8Profile = 0,
            .u16Width = 720,
            .u16Height = 576,
            .u8EsBufQueueEn = 1,
            .u16EnType = PT_H264,
            .u32BitStreamBufSize = 524288,
            .StreamTo = 1,
            .u8EntropyEncModeI = 0,
            .u8EntropyEncModeP = 0,
        },
        .stGopParam = {
            .u16gopMode = 0,
            .s8IPQpDelta = 0,
        },
        .stRcParam = {
            .u16Gop = 50,
            .u8SrcFrameRate = 25,
            .u8DstFrameRate = 25,
            .u16BitRate = 1000,
            .u8Qfactor = 60,
            .u32MaxBitRate = 1000,
            .u8VariFpsEn = 0,
            .u8StartTime = 2,
            .u16RcMode = VENC_RC_MODE_H264CBR,
            .u16FirstFrmstartQp = 30,
            .u16InitialDelay = 1000,
            .u16ThrdLv = 2,
            .u16BgDeltaQp = 0,
            .u8MinIprop = 1,
            .u8MaxIprop = 100,
            .u8MinIqp = 1,
            .u8MaxIqp = 51,
            .u8MinQp = 1,
            .u8MaxQp = 50,
            .u8MaxReEncodeTimes = 0,
            .u8QpMapEn = 0,
            .u8ChangePos = 75,
            .bSingleCore = 0,
            .u32FixedIQp = 38,
            .u32FixedPQp = 38,
            .u32Duration = 75,
            .s32MinStillPercent = 10,
            .u32MaxStillQP = 38,
            .u32MinStillPSNR = 0,
            .u32MotionSensitivity = 24,
            .s32AvbrFrmLostOpen = 0,
            .s32AvbrFrmGap = 1,
            .s32AvbrPureStillThr = 4,
        },
    },
    {
        .stChnParam = {
            .bEnable = 1,
            .u8VencChn = 2,
            .enBindMode = VENC_BIND_DISABLE,
            .astChn[0] = {
                .enModId = CVI_ID_VPSS,
                .s32DevId = 0,
                .s32ChnId = 1,
            },
            .astChn[1] = {
                .enModId = CVI_ID_VENC,
                .s32DevId = 0,
                .s32ChnId = 2,
            },
            .u8Profile = 0,
            .u16Width = 720,
            .u16Height = 576,
            .u16EnType = PT_JPEG,
            .u32BitStreamBufSize = 524288,
        },
        .stRcParam = {
            .u16RcMode = VENC_RC_MODE_MJPEGCBR,
            .u16InitialDelay = 1000,
            .s16Quality = 20,
        },
    },
};

PARAM_CLASSDEFINE(PARAM_ROI_PARAM_S,ROICFG,CTX,VENC)[] = {
    {
        .u8Index = 0,
        .bEnable = 0,
        .VencChn = 0,
        .bAbsQp = 1,
        .s32Qp = 20,
        .s32X = 10,
        .s32Y = 10,
        .u32Width = 256,
        .u32Height = 256,
    },
};

PARAM_VENC_CFG_S  g_stVencCtx = {
    .s32VencChnCnt = 3,
    .pstVencChnCfg = PARAM_CLASS(VENCCFG,CTX,VENC),
    .s8RoiNumber = 0,
    .pstRoiParam = PARAM_CLASS(ROICFG,CTX,VENC),
};

PARAM_VENC_CFG_S * PARAM_GET_VENC_CFG(void) {
    return &g_stVencCtx;
}
