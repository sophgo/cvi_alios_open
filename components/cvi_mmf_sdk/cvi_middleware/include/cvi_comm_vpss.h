/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_comm_vpss.h
 * Description:
 *   The common data type defination for VPSS module.
 */

#ifndef __CVI_COMM_VPSS_H__
#define __CVI_COMM_VPSS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <cvi_type.h>
#include <cvi_common.h>
#include <cvi_comm_vb.h>
#include <cvi_comm_video.h>
#include <cvi_comm_vpss.h>

#define VPSS_INVALID_FRMRATE -1
#define VPSS_CHN0             0
#define VPSS_CHN1             1
#define VPSS_CHN2             2
#define VPSS_CHN3             3
#define VPSS_INVALID_CHN     -1
#define VPSS_INVALID_GRP     -1
#define CVI_STITCH_CHN_MAX_NUM 4

/*
 * VPSS_MODE_SINGLE: vpss work as 1 device, Device has 4 channels
 * VPSS_MODE_DUAL: vpss work as 2 device, Device 0 has 1 channel, Device 1 has 3 channels.
 */
typedef enum _VPSS_MODE_E {
	VPSS_MODE_SINGLE = 0,
	VPSS_MODE_DUAL,
	VPSS_MODE_BUTT
} VPSS_MODE_E;

/*
 * VPSS_INPUT_MEM: vpss input from memory.
 * VPSS_INPUT_ISP: vpss input from isp.
 */
typedef enum _VPSS_INPUT_E {
	VPSS_INPUT_MEM = 0,
	VPSS_INPUT_ISP,
	VPSS_INPUT_BUTT
} VPSS_INPUT_E;

/*
 * enMode: decide vpss work as 1/2 device.
 * aenInput: decide the input of each vpss device.
 */
typedef struct _VPSS_MODE_S {
	VPSS_MODE_E enMode;
	VPSS_INPUT_E aenInput[VPSS_DEVICE_NUM];
} VPSS_MODE_S;

/*
 * VPSS_CROP_RATIO_COOR: Ratio coordinate.
 * VPSS_CROP_ABS_COOR: Absolute coordinate.
 */
typedef enum _VPSS_CROP_COORDINATE_E {
	VPSS_CROP_RATIO_COOR = 0,
	VPSS_CROP_ABS_COOR,
	VPSS_CROP_BUTT,
} VPSS_CROP_COORDINATE_E;

typedef enum _VPSS_ROUNDING_E {
	VPSS_ROUNDING_TO_EVEN = 0,
	VPSS_ROUNDING_AWAY_FROM_ZERO,
	VPSS_ROUNDING_TRUNCATE,
	VPSS_ROUNDING_MAX,
} VPSS_ROUNDING_E;

/*
 * bEnable: Whether Normalize is enabled.
 * factor: scaling factors for 3 planes, [1.0f/8192, 1].
 * mean: minus means for 3 planes, [0, 255].
 */
typedef struct _VPSS_NORMALIZE_S {
	CVI_BOOL bEnable;
	CVI_FLOAT factor[3];
	CVI_FLOAT mean[3];
	VPSS_ROUNDING_E rounding;
} VPSS_NORMALIZE_S;

/*
 * u32MaxW: Range: Width of source image.
 * u32MaxH: Range: Height of source image.
 * enPixelFormat: Pixel format of target image.
 * stFrameRate: Frame rate control info.
 * u8VpssDev: Only meaningful if VPSS_MODE_DUAL.
 */
typedef struct _VPSS_GRP_ATTR_S {
	CVI_U32 u32MaxW;
	CVI_U32 u32MaxH;
	PIXEL_FORMAT_E enPixelFormat;
	FRAME_RATE_CTRL_S stFrameRate;
	CVI_U8 u8VpssDev;
} VPSS_GRP_ATTR_S;

/*
 * u32Width: Width of target image.
 * u32Height: Height of target image.
 * enVideoFormat: Video format of target image.
 * enPixelFormat: Pixel format of target image.
 * stFrameRate: Frame rate control info.
 * bMirror: Mirror enable.
 * bFlip: Flip enable.
 * u32Depth: User get list depth.
 * stAspectRatio: Aspect Ratio info.
 */
typedef struct _VPSS_CHN_ATTR_S {
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	VIDEO_FORMAT_E enVideoFormat;
	PIXEL_FORMAT_E enPixelFormat;
	FRAME_RATE_CTRL_S stFrameRate;
	CVI_BOOL bMirror;
	CVI_BOOL bFlip;
	CVI_U32 u32Depth;
	ASPECT_RATIO_S stAspectRatio;
	VPSS_NORMALIZE_S stNormalize;
} VPSS_CHN_ATTR_S;

/*
 * bEnable: RW; CROP enable.
 * enCropCoordinate: RW; Coordinate mode of the crop start point.
 * stCropRect: CROP rectangle.
 */
typedef struct _VPSS_CROP_INFO_S {
	CVI_BOOL bEnable;
	VPSS_CROP_COORDINATE_E enCropCoordinate;
	RECT_S stCropRect;
} VPSS_CROP_INFO_S;

/*
 * bEnable: Whether LDC is enbale.
 * stAttr: LDC Attribute.
 */
typedef struct _VPSS_LDC_ATTR_S {
	CVI_BOOL bEnable;
	LDC_ATTR_S stAttr;
} VPSS_LDC_ATTR_S;

typedef enum _VPSS_BUF_SOURCE_E {
	VPSS_COMMON_VB = 0,
	VPSS_USER_VB,
	VPSS_USER_ION,
} VPSS_BUF_SOURCE_E;

typedef struct _VPSS_PARAM_MOD_S {
	VPSS_BUF_SOURCE_E enVpssBufSource;
} VPSS_MOD_PARAM_S;

typedef enum _VPSS_SCALE_COEF_E {
	VPSS_SCALE_COEF_BICUBIC = 0,
	VPSS_SCALE_COEF_BILINEAR,
	VPSS_SCALE_COEF_NEAREST,
	VPSS_SCALE_COEF_BICUBIC_OPENCV,
	VPSS_SCALE_COEF_MAX,
} VPSS_SCALE_COEF_E;

typedef struct _VPSS_RECT_S {
	CVI_BOOL bEnable;
	CVI_U16 u16Thick;   /* Width of line */
	CVI_U32 u32BgColor; /* RGB888, B:bit0 - bit7, G:bit8 - bit15, R:bit16 - bit23 */
	RECT_S stRect;      /*outside rectangle*/
} VPSS_RECT_S;

typedef struct _VPSS_DRAW_RECT_S {
	VPSS_RECT_S astRect[VPSS_RECT_NUM];
} VPSS_DRAW_RECT_S;

typedef struct _VPSS_CONVERT_S {
	CVI_BOOL bEnable;
	CVI_U32 u32aFactor[3];
	CVI_U32 u32bFactor[3];
} VPSS_CONVERT_S;

typedef struct _VPSS_CHN_BUF_WRAP_S {
	CVI_BOOL bEnable;
	CVI_U32 u32BufLine;	// 64, 128
	CVI_U32 u32WrapBufferSize;	// depth for now
} VPSS_CHN_BUF_WRAP_S;

typedef struct _VPSS_STITCH_CHN_ATTR_S {
	VIDEO_FRAME_INFO_S stVideoFrame;
	RECT_S stDstRect;
} VPSS_STITCH_CHN_ATTR_S;

typedef struct _VPSS_STITCH_OUTPUT_ATTR_S {
	CVI_U32 u32Color; // backgroud color RGB888, B:bit0 - bit7, G:bit8 - bit15, R:bit16 - bit23
	PIXEL_FORMAT_E enPixelformat;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
} VPSS_STITCH_OUTPUT_ATTR_S;

typedef struct _VPSS_ALL_PROC_AMP_S {
	CVI_S32 s32ProcAmp[VPSS_MAX_GRP_NUM][PROC_AMP_MAX];
} VPSS_ALL_PROC_AMP_S;

typedef struct _VPSS_BIN_DATA {
	CVI_S32 proc_amp[PROC_AMP_MAX];
} VPSS_BIN_DATA;

typedef struct _VPSS_PARAMETER_BUFFER {
	VPSS_BIN_DATA vpss_bin_data[VPSS_MAX_GRP_NUM];
} VPSS_PARAMETER_BUFFER;

/*vpss stitch src info*/
typedef struct _CVI_STITCH_SRC_S {
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
} CVI_STITCH_SRC_S;

/**
 * stStitchSrc: vpss stitch src info
 * stDstRect: vpss stitch dest position
 * u8Priority: Priority
 */
typedef struct _CVI_STITCH_CHN_S {
	CVI_STITCH_SRC_S stStitchSrc;
	RECT_S stDstRect;
	CVI_U8 u8Priority;
} CVI_STITCH_CHN_S;

/**
 * u8ChnNum: the number of vpss stitch chn
 * VoChn: Vo chn id
 * s32OutFps: Output FPS
 * enOutPixelFormat: image pixel format
 * stOutSize: Output size
 * hVbPool: Attached vb pool
 * astStitchChn: vpss stitch chn attr
 */
typedef struct _CVI_STITCH_ATTR_S {
	CVI_U8 u8ChnNum;
	CVI_U8 VoChn;
	CVI_S32 s32OutFps;
	PIXEL_FORMAT_E enOutPixelFormat;
	SIZE_S stOutSize;
	VB_POOL hVbPool;
	CVI_STITCH_CHN_S astStitchChn[CVI_STITCH_CHN_MAX_NUM];
} CVI_STITCH_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __CVI_COMM_VPSS_H__ */
