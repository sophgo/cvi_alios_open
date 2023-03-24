#include <board_config.h>
#include "ir_custom_param.h"


PARAM_CLASSDEFINE(PARAM_VB_CFG_S,VBPOOL,CTX,VB)[] = {
    {
        .u16width = 480,
        .u16height = 640,
        .u8VbBlkCnt = 8,
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
};

PARAM_SYS_CFG_S  g_stFcSysCtx = {
    .u8VbPoolCnt = 2,
    .stVPSSMode.enMode = VPSS_MODE_DUAL,
    .u8ViCnt = 2,
    .stVIVPSSMode.aenMode[0] = VI_ONLINE_VPSS_ONLINE,
    .stVIVPSSMode.aenMode[1] = VI_ONLINE_VPSS_ONLINE,
    .stVPSSMode.ViPipe[0] = 0,
    .stVPSSMode.aenInput[0] = VPSS_INPUT_MEM,
    .stVPSSMode.aenInput[1] = VPSS_INPUT_ISP,
    .pstVbPool = PARAM_CLASS(VBPOOL,CTX,VB),
};

PARAM_SYS_CFG_S * IR_PARAM_GET_SYS_CFG(void)
{
    return &g_stFcSysCtx;
}
