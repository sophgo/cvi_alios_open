#ifndef __PARAM_VO_H__
#define __PARAM_VO_H__
#include <stdio.h>
#include "cvi_defines.h"
#include "cvi_comm_vo.h"

//多级别结构 确定使用的多级数组加number作标定 (多级限定场景 串行结构 不允许存在同一级别有多个多级存在) 大结构必须精简尽量使用指针

typedef struct _PARAM_VODEV_S {
    VO_CONFIG_S stVoConfig;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_LAYER VoLayer;
    CVI_U8 u8ChnCnt;
    CVI_U8 u8Bindmode;
    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDestChn;
    VO_CHN_ATTR_S *pstVOChnAttr;
    CVI_U8 u8ChnRotation;
}PARAM_VODEV_S;

typedef struct _PARAM_VO_CFG_S {
    CVI_U8 u8VoCnt;
    PARAM_VODEV_S *pstVoCfg;
} PARAM_VO_CFG_S;



#endif
