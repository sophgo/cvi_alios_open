#ifndef __PARAM_VO_H__
#define __PARAM_VO_H__
#include <stdio.h>
#include "cvi_defines.h"
#include "cvi_comm_vo.h"

//多级别结构 确定使用的多级数组加number作标定 (多级限定场景 串行结构 不允许存在同一级别有多个多级存在) 大结构必须精简尽量使用指针

#define COLOR_RGB_RED RGB_8BIT(0xFF, 0, 0)
#define COLOR_RGB_GREEN RGB_8BIT(0, 0xFF, 0)
#define COLOR_RGB_BLUE RGB_8BIT(0, 0, 0xFF)
#define COLOR_RGB_BLACK RGB_8BIT(0, 0, 0)
#define COLOR_RGB_YELLOW RGB_8BIT(0xFF, 0xFF, 0)
#define COLOR_RGB_CYN RGB_8BIT(0, 0xFF, 0xFF)
#define COLOR_RGB_WHITE RGB_8BIT(0xFF, 0xFF, 0xFF)

#define COLOR_10_RGB_RED RGB(0x3FF, 0, 0)
#define COLOR_10_RGB_GREEN RGB(0, 0x3FF, 0)
#define COLOR_10_RGB_BLUE RGB(0, 0, 0x3FF)
#define COLOR_10_RGB_BLACK RGB(0, 0, 0)
#define COLOR_10_RGB_YELLOW RGB(0x3FF, 0x3FF, 0)
#define COLOR_10_RGB_CYN RGB(0, 0x3FF, 0x3FF)
#define COLOR_10_RGB_WHITE RGB(0x3FF, 0x3FF, 0x3FF)

typedef enum _PARAM_VO_MODE_E {
	VO_MODE_1MUX,
	VO_MODE_2MUX,
	VO_MODE_4MUX,
	VO_MODE_8MUX,
	VO_MODE_9MUX,
	VO_MODE_16MUX,
	VO_MODE_25MUX,
	VO_MODE_36MUX,
	VO_MODE_49MUX,
	VO_MODE_64MUX,
	VO_MODE_2X4,
	VO_MODE_BUTT
} PARAM_VO_MODE_E;

typedef struct _VO_CONFIG_S {
	/* for device */
	VO_DEV VoDev;
	VO_PUB_ATTR_S stVoPubAttr;

	/* for layer */
	PIXEL_FORMAT_E enPixFormat;
	RECT_S stDispRect;
	SIZE_S stImageSize;

	CVI_U32 u32DisBufLen;

	/* for channel */
	PARAM_VO_MODE_E enVoMode;
} PARAM_VO_CONFIG_S;

typedef struct _PARAM_VODEV_S {
    PARAM_VO_CONFIG_S stVoConfig;
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
