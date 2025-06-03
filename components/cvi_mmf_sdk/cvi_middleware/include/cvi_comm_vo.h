/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *   The common data type defination for VO module.
 */

#ifndef __CVI_COMM_VO_H__
#define __CVI_COMM_VO_H__

#include <cvi_type.h>
#include <cvi_common.h>
#include <cvi_comm_video.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define VO_GAMMA_NODENUM 65
#define MAX_VO_PINS 32

/* VO video output interface type */
typedef enum _VO_INTF_TYPE_E {
	VO_INTF_BT656 = (0x01L << 7),
	VO_INTF_BT1120 = (0x01L << 8),
	VO_INTF_PARALLEL_RGB = (0x01L << 9),
	VO_INTF_SERIAL_RGB = (0x01L << 10),
	VO_INTF_I80 = (0x01L << 11),
	VO_INTF_HW_MCU = (0x01L << 12),
	VO_INTF_MIPI = (0x01L << 13),
	VO_INTF_LVDS = (0x01L << 14),
	VO_INTF_BUTT
} VO_INTF_TYPE_E;

/* VO video output sync type */
typedef enum _VO_INTF_SYNC_E {
	VO_OUTPUT_PAL = 0, /* PAL standard*/
	VO_OUTPUT_NTSC, /* NTSC standard */
	VO_OUTPUT_1080P24, /* 1920 x 1080 at 24 Hz. */
	VO_OUTPUT_1080P25, /* 1920 x 1080 at 25 Hz. */
	VO_OUTPUT_1080P30, /* 1920 x 1080 at 30 Hz. */
	VO_OUTPUT_720P50, /* 1280 x  720 at 50 Hz. */
	VO_OUTPUT_720P60, /* 1280 x  720 at 60 Hz. */
	VO_OUTPUT_1080P50, /* 1920 x 1080 at 50 Hz. */
	VO_OUTPUT_1080P60, /* 1920 x 1080 at 60 Hz. */
	VO_OUTPUT_576P50, /* 720  x  576 at 50 Hz. */
	VO_OUTPUT_480P60, /* 720  x  480 at 60 Hz. */
	VO_OUTPUT_800x600_60, /* VESA 800 x 600 at 60 Hz (non-interlaced) */
	VO_OUTPUT_1024x768_60, /* VESA 1024 x 768 at 60 Hz (non-interlaced) */
	VO_OUTPUT_1280x1024_60, /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
	VO_OUTPUT_1366x768_60, /* VESA 1366 x 768 at 60 Hz (non-interlaced) */
	VO_OUTPUT_1440x900_60, /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */
	VO_OUTPUT_1280x800_60, /* 1280*800@60Hz VGA@60Hz*/
	VO_OUTPUT_1600x1200_60, /* VESA 1600 x 1200 at 60 Hz (non-interlaced) */
	VO_OUTPUT_1680x1050_60, /* VESA 1680 x 1050 at 60 Hz (non-interlaced) */
	VO_OUTPUT_1920x1200_60, /* VESA 1920 x 1600 at 60 Hz (non-interlaced) CVT (Reduced Blanking)*/
	VO_OUTPUT_640x480_60, /* VESA 640 x 480 at 60 Hz (non-interlaced) CVT */
	VO_OUTPUT_720x1280_60, /* For MIPI DSI Tx 720 x1280 at 60 Hz */
	VO_OUTPUT_1080x1920_60, /* For MIPI DSI Tx 1080x1920 at 60 Hz */
	VO_OUTPUT_480x800_60, /* For MIPI DSI Tx 480x800 at 60 Hz */
	VO_OUTPUT_1440P60, /* 2560 x 1440 at 60 Hz. */
	VO_OUTPUT_2160P24, /* 3840 x 2160 at 24 Hz. */
	VO_OUTPUT_2160P25, /* 3840 x 2160 at 25 Hz. */
	VO_OUTPUT_2160P30, /* 3840 x 2160 at 30 Hz. */
	VO_OUTPUT_2160P50, /* 3840 x 2160 at 50 Hz. */
	VO_OUTPUT_2160P60, /* 3840 x 2160 at 60 Hz. */
	VO_OUTPUT_4096x2160P24, /* 4096 x 2160 at 24 Hz. */
	VO_OUTPUT_4096x2160P25, /* 4096 x 2160 at 25 Hz. */
	VO_OUTPUT_4096x2160P30, /* 4096 x 2160 at 30 Hz. */
	VO_OUTPUT_4096x2160P50, /* 4096 x 2160 at 50 Hz. */
	VO_OUTPUT_4096x2160P60, /* 4096 x 2160 at 60 Hz. */
	VO_OUTPUT_USER, /* User timing. */
	VO_OUTPUT_BUTT
} VO_INTF_SYNC_E;

/*
 * bSynm: sync mode(0:timing,as BT.656; 1:signal,as LCD)
 * bIop: interlaced or progressive display(0:i; 1:p)
 * u16FrameRate: frame-rate
 * u16Vact: vertical active area
 * u16Vbb: vertical back blank porch
 * u16Vfb: vertical front blank porch
 * u16Hact: horizontal active area
 * u16Hbb: horizontal back blank porch
 * u16Hfb: horizontal front blank porch
 * u16Hpw: horizontal pulse width
 * u16Vpw: vertical pulse width
 * bIdv: inverse data valid of output
 * bIhs: inverse horizontal synch signal
 * bIvs: inverse vertical synch signal
 */
typedef struct _VO_SYNC_INFO_S {
	CVI_BOOL bSynm;
	CVI_BOOL bIop;

	CVI_U16 u16FrameRate;

	CVI_U16 u16Vact;
	CVI_U16 u16Vbb;
	CVI_U16 u16Vfb;

	CVI_U16 u16Hact;
	CVI_U16 u16Hbb;
	CVI_U16 u16Hfb;

	CVI_U16 u16Hpw;
	CVI_U16 u16Vpw;

	CVI_BOOL bIdv;
	CVI_BOOL bIhs;
	CVI_BOOL bIvs;
} VO_SYNC_INFO_S;

typedef enum _VO_MAC_BT_MUX_E {
	VO_MUX_BT_VS = 0,
	VO_MUX_BT_HS,
	VO_MUX_BT_HDE,
	VO_MUX_BT_DATA0,
	VO_MUX_BT_DATA1,
	VO_MUX_BT_DATA2,
	VO_MUX_BT_DATA3,
	VO_MUX_BT_DATA4,
	VO_MUX_BT_DATA5,
	VO_MUX_BT_DATA6,
	VO_MUX_BT_DATA7,
	VO_MUX_BT_DATA8,
	VO_MUX_BT_DATA9,
	VO_MUX_BT_DATA10,
	VO_MUX_BT_DATA11,
	VO_MUX_BT_DATA12,
	VO_MUX_BT_DATA13,
	VO_MUX_BT_DATA14,
	VO_MUX_BT_DATA15,
	VO_MUX_TG_HS_TILE = 30,
	VO_MUX_TG_VS_TILE,
	VO_MUX_BT_CLK,
	VO_BT_MUX_MAX,
} VO_MAC_BT_MUX_E;

typedef enum _VO_MAC_D_SEL {
	VO_VIVO_CLK  = 0,
	VO_VIV1_CLK  = 1,
	VO_MIPI1_TXN1 = 2,
	VO_MIPI1_TXP2 = 3,
	VO_MIPI1_TXN2 = 4,
	VO_MIPI1_TXP3 = 5,
	VO_MIPI1_TXN3 = 6,
	VO_MIPI1_TXP4 = 7,
	VO_MIPI1_TXN4 = 8,
	VO_VIVO0_D10 = 9,
	VO_VIVO0_D11 = 10,
	VO_VIVO0_D12 = 11,
	VO_VIVO0_D13 = 12,
	VO_VIVO0_D14 = 13,
	VO_VIVO0_D15 = 14,
	VO_VIVO0_D16 = 15,
	VO_MIPI0_TXP0 = 16,
	VO_MIPI0_TXN0 = 17,
	VO_MIPI0_TXP1 = 18,
	VO_MIPI0_TXN1 = 19,
	VO_MIPI0_TXP2 = 20,
	VO_MIPI0_TXN2 = 21,
	VO_MIPI0_TXP3 = 22,
	VO_MIPI0_TXN3 = 23,
	VO_MIPI0_TXP4 = 24,
	VO_MIPI0_TXN4 = 25,
	VO_MIPI1_TXP0 = 26,
	VO_MIPI1_TXN0 = 27,
	VO_MIPI1_TXP1 = 28,
	VO_PAD_MAX,
} VO_MAC_D_SEL_E;

struct VO_D_REMAP {
	VO_MAC_D_SEL_E sel;
	VO_MAC_BT_MUX_E mux;
};

typedef enum _VO_BT_MODE_E {
	VO_BT_MODE_656 = 0,
	VO_BT_MODE_1120,
	VO_BT_MODE_601,
	VO_BT_MODE_MAX
} VO_BT_MODE_E;

typedef enum _VO_BT_DATA_SEQ_E {
	VO_BT_DATA_SEQ0 = 0,
	VO_BT_DATA_SEQ1,
	VO_BT_DATA_SEQ2,
	VO_BT_DATA_SEQ3,
} VO_BT_DATA_SEQ_E;

typedef struct _VO_BT_ATTR_S {
	CVI_U8 pin_num;
	CVI_BOOL bt_clk_inv;
	CVI_BOOL bt_vs_inv;
	CVI_BOOL bt_hs_inv;
	VO_BT_DATA_SEQ_E data_seq;
	struct VO_D_REMAP d_pins[MAX_VO_PINS];
} VO_BT_ATTR_S;

/*
 * u32BgColor: Background color of a device, in RGB format.
 * enIntfType: Type of a VO interface.
 * enIntfSync: Type of a VO interface timing.
 * stSyncInfo: Information about VO interface timings if customed type.
 */
typedef struct _VO_PUB_ATTR_S {
	CVI_U32 u32BgColor;
	VO_INTF_TYPE_E enIntfType;
	VO_INTF_SYNC_E enIntfSync;
	VO_SYNC_INFO_S stSyncInfo;
} VO_PUB_ATTR_S;

typedef enum _VO_LVDS_MODE_E {
	VO_LVDS_MODE_JEIDA = 0,
	VO_LVDS_MODE_VESA,
	VO_LVDS_MODE_MAX,
} VO_LVDS_MODE_E;

typedef enum _VO_LVDS_OUT_BIT_E {
	VO_LVDS_OUT_6BIT = 0,
	VO_LVDS_OUT_8BIT,
	VO_LVDS_OUT_10BIT,
	VO_LVDS_OUT_MAX,
} VO_LVDS_OUT_BIT_E;

typedef enum _VO_LVDS_LANE_ID {
	VO_LVDS_LANE_CLK = 0,
	VO_LVDS_LANE_0,
	VO_LVDS_LANE_1,
	VO_LVDS_LANE_2,
	VO_LVDS_LANE_3,
	VO_LVDS_LANE_MAX,
} VO_LVDS_LANE_ID;

/* Define LVDS's config
 *
 * lvds_vesa_mode: true for VESA mode; false for JEIDA mode
 * out_bits: 6/8/10 bit
 * chn_num: output channel num
 * data_big_endian: true for big endian; false for little endian
 * lane_id: lane mapping, -1 no used
 * lane_pn_swap: lane pn-swap if true
 */
typedef struct _VO_LVDS_ATTR_S {
	VO_LVDS_MODE_E lvds_vesa_mode;
	VO_LVDS_OUT_BIT_E out_bits;
	CVI_U8 chn_num;
	CVI_BOOL data_big_endian;
	VO_LVDS_LANE_ID lane_id[VO_LVDS_LANE_MAX];
	CVI_BOOL lane_pn_swap[VO_LVDS_LANE_MAX];
} VO_LVDS_ATTR_S;

/*
 * stDispRect: Display resolution
 * stImageSize: Original ImageSize.
 *              Only useful if vo support scaling, otherwise, it should be the same width stDispRect.
 * u32DispFrmRt: frame rate.
 * enPixFormat: Pixel format of the video layer
 */
typedef struct _VO_VIDEO_LAYER_ATTR_S {
	RECT_S stDispRect;
	SIZE_S stImageSize;
	CVI_U32 u32DispFrmRt;
	PIXEL_FORMAT_E enPixFormat;
} VO_VIDEO_LAYER_ATTR_S;

typedef enum _VO_CSC_MATRIX_E {
	VO_CSC_MATRIX_IDENTITY = 0,

	VO_CSC_MATRIX_601_LIMIT_YUV2RGB,
	VO_CSC_MATRIX_601_FULL_YUV2RGB,

	VO_CSC_MATRIX_709_LIMIT_YUV2RGB,
	VO_CSC_MATRIX_709_FULL_YUV2RGB,

	VO_CSC_MATRIX_601_LIMIT_RGB2YUV,
	VO_CSC_MATRIX_601_FULL_RGB2YUV,

	VO_CSC_MATRIX_709_LIMIT_RGB2YUV,
	VO_CSC_MATRIX_709_FULL_RGB2YUV,

	VO_CSC_MATRIX_BUTT
} VO_CSC_MATRIX_E;

/*
 * enCscMatrix: CSC matrix
 */
typedef struct _VO_CSC_S {
	VO_CSC_MATRIX_E enCscMatrix;
} VO_CSC_S;

/*
 * u32Priority: Video out overlay priority.
 * stRect: Rectangle of video output channel.
 */
typedef struct _VO_CHN_ATTR_S {
	CVI_U32 u32Priority;
	RECT_S stRect;
} VO_CHN_ATTR_S;

/*
 * u32ChnBufUsed: Channel buffer that been occupied.
 */
typedef struct _VO_QUERY_STATUS_S {
	CVI_U32 u32ChnBufUsed;
} VO_QUERY_STATUS_S;

typedef struct _VO_GAMMA_INFO_S {
	VO_DEV s32VoDev;
	CVI_BOOL enable;
	CVI_BOOL osd_apply;
	CVI_U32 value[VO_GAMMA_NODENUM];
} VO_GAMMA_INFO_S;

typedef struct _VO_BIN_INFO_S {
	VO_GAMMA_INFO_S gamma_info;
	CVI_U32 guard_magic;
} VO_BIN_INFO_S;

typedef enum _VO_PATTERN_MODE {
	VO_PAT_OFF = 0,
	VO_PAT_SNOW,
	VO_PAT_AUTO,
	VO_PAT_RED,
	VO_PAT_GREEN,
	VO_PAT_BLUE,
	VO_PAT_COLORBAR,
	VO_PAT_GRAY_GRAD_H,
	VO_PAT_GRAY_GRAD_V,
	VO_PAT_BLACK,
	VO_PAT_MAX,
} VO_PATTERN_MODE;

typedef CVI_S32(*pfnVoDevPmOps)(void *pvData);

typedef struct _VO_PM_OPS_S {
	pfnVoDevPmOps pfnPanelSuspend;
	pfnVoDevPmOps pfnPanelResume;
} VO_PM_OPS_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __CVI_COMM_VO_H__ */
