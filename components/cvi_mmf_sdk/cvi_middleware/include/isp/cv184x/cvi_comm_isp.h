/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: include/cvi_common_isp.h
 * Description:
 */

#ifndef __CVI_COMM_ISP_H__
#define __CVI_COMM_ISP_H__

#include <stdint.h>
#include "cvi_comm_inc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define CCM_MATRIX_SIZE (9)
#define AE_MAX_ZONE_ROW (30)
#define AE_MAX_ZONE_COLUMN (34)
#define AE_ZONE_ROW (30)
#define AE_ZONE_COLUMN (34)
#define AE_ZONE_NUM (AE_ZONE_ROW * AE_ZONE_COLUMN)
#define AE_MAX_NUM (1)
#define AWB_ZONE_ORIG_ROW (30)
#define AWB_ZONE_ORIG_COLUMN (34)
#define AWB_ZONE_MAX_COLUMN (34)
#define AWB_ZONE_NUM (AWB_ZONE_ORIG_ROW * AWB_ZONE_ORIG_COLUMN)
#define AWB_MAX_NUM (1)
#define HIST_BIN_DEPTH (8)
#define MAX_HIST_BINS (1 << HIST_BIN_DEPTH)
#define DCF_DRSCRIPTION_LENGTH (32)
#define DCF_CAPTURE_TIME_LENGTH (20)
#define ISP_AUTO_ISO_STRENGTH_NUM (16)
#define ISP_AUTO_EXP_RATIO_NUM (16)
#define ISP_AUTO_COLORTEMP_NUM (7)
#define CVI_ISP_LSC_GRID_COL (37)
#define CVI_ISP_LSC_GRID_ROW (37)
#define DCI_HIST_BINS_NUM (256)
#define DRC_HIST_BINS_NUM (128)
#define DRC_CURVE_L_LUT_NUM (65)
#define DRC_CURVE_R_LUT_NUM (17)
#define DRC_MAX_TMP_LUT_NUM (4097)
#define RLSC_HIST_BINS_NUM (256)
#define ISP_MAX_SNS_REGS 32
#define ISP_MAX_WDR_FRAME_NUM 2
#define BAYER_PATTERN_NUM 4
#define MG_ZONE_ROW (15)
#define MG_ZONE_COLUMN (17)
#define AF_ZONE_ROW (15)
#define AF_ZONE_COLUMN (17)
#define AF_XOFFSET_MIN (8)
#define AF_YOFFSET_MIN (2)
#define MAX_AWB_LIB_NUM (VI_MAX_PIPE_NUM)
#define MAX_AE_LIB_NUM (VI_MAX_PIPE_NUM)
#define MAX_AF_LIB_NUM (VI_MAX_PIPE_NUM)
#define LTM_DARK_CURVE_NODE_NUM (257)
#define LTM_BRIGHT_CURVE_NODE_NUM (513)
#define LTM_GLOBAL_CURVE_NODE_NUM (769)
#define ISP_3ALIB_FIND_FAIL (-1)
#define PRE_EE_GAMMA_LUT_NUM 65
#define PRE_E5_COEF_LUT_NUM 6
#define PRE_E7_COEF_LUT_NUM 10
#define TNR_FREQ_NUM 64
#define TNR_MV_CAND_NUM 8
#define GAIN_LUT_NUM 9
#define MOTION_NR_TABLE_NUM 16
#define HIST_NUM 256
#define ISP_BASE_ADDR 0x0A000000
#define ISP_REG_RANGE 0x80000
#define DEHAZE_LUT_NUM 32

#define MIN_LV	(-5)
#define MAX_LV	(15)
#define LV_TOTAL_NUM	(MAX_LV - MIN_LV + 1)
#define AE_LV_FACTOR	(100)
#define ISP_AUTO_LV_NUM (LV_TOTAL_NUM)

/*Defines the format of the input Bayer image*/
typedef enum _ISP_BAYER_FORMAT_E {
	BAYER_BGGR,
	BAYER_GBRG,
	BAYER_GRBG,
	BAYER_RGGB,
	//for RGBIR sensor
	BAYER_GRGBI = 8,
	BAYER_RGBGI,
	BAYER_GBGRI,
	BAYER_BGRGI,
	BAYER_IGRGB,
	BAYER_IRGBG,
	BAYER_IBGRG,
	BAYER_IGBGR,
	BAYER_BUTT
} ISP_BAYER_FORMAT_E;

typedef enum _ISP_OP_TYPE_E {
	OP_TYPE_AUTO,
	OP_TYPE_MANUAL,
	OP_TYPE_BUTT
} ISP_OP_TYPE_E;

typedef enum _ISP_FMW_STATE_E {
	ISP_FMW_STATE_RUN,
	ISP_FMW_STATE_FREEZE,
	ISP_FMW_STATE_BUTT
} ISP_FMW_STATE_E;

typedef enum _ISP_CHANNEL_LIST_E {
	ISP_CHANNEL_LE,
	ISP_CHANNEL_SE,
	ISP_CHANNEL_MAX_NUM,
} ISP_CHANNEL_LIST_E;

typedef enum _ISP_BAYER_CHANNEL_E {
	ISP_BAYER_CHN_R,
	ISP_BAYER_CHN_GR,
	ISP_BAYER_CHN_GB,
	ISP_BAYER_CHN_B,
	ISP_BAYER_CHN_NUM,
} ISP_BAYER_CHANNEL_E;

typedef enum _ISP_SNS_TYPE_E {
	SNS_I2C_TYPE,
	SNS_SSP_TYPE,
	SNS_TYPE_BUTT,
} ISP_SNS_TYPE_E;

typedef enum _ISP_WDR_FRAME_IDX_E {
	ISP_WDR_FRAME_IDX_1,
	ISP_WDR_FRAME_IDX_2,
	ISP_WDR_FRAME_IDX_3,
	ISP_WDR_FRAME_IDX_4,
	ISP_WDR_FRAME_IDX_SIZE
} ISP_WDR_FRAME_IDX_E;

typedef union _ISP_SNS_COMMBUS_U {
	CVI_S8 s8I2cDev;
	struct {
		CVI_S8 bit4SspDev : 4;
		CVI_S8 bit4SspCs : 4;
	} s8SspDev;
} ISP_SNS_COMMBUS_U;

typedef union _ISP_SNS_COMMADDR_U {
	CVI_S32 s8I2cAddr;
} ISP_SNS_COMMADDR_U;

typedef struct _ISP_I2C_DATA_S {
	CVI_BOOL bUpdate;
	CVI_BOOL bDropFrm;
	CVI_BOOL bvblankUpdate;
	CVI_U8 u8DelayFrmNum; /*RW; Number of delayed frames for the sensor register*/
	CVI_U8 u8DropFrmNum; /*RW; Number of frame to drop*/
	CVI_U8 u8IntPos; /*RW;Position where the configuration of the sensor register takes effect */
	CVI_U8 u8DevAddr; /*RW;Sensor device address*/
	CVI_U32 u32RegAddr; /*RW;Sensor register address*/
	CVI_U32 u32AddrByteNum; /*RW;Bit width of the sensor register address*/
	CVI_U32 u32Data; /*RW;Sensor register data*/
	CVI_U32 u32DataByteNum; /*RW;Bit width of sensor register data*/
} ISP_I2C_DATA_S;

typedef struct _ISP_SSP_DATA_S {
	CVI_BOOL bUpdate;
	CVI_U8 u8DelayFrmNum; /*RW; Number of delayed frames for the sensor register*/
	CVI_U8 u8IntPos; /*RW;Position where the configuration of the sensor register takes effect */
	CVI_U32 u32DevAddr; /*RW;Sensor device address*/
	CVI_U32 u32DevAddrByteNum; /*RW;Bit width of the sensor device address*/
	CVI_U32 u32RegAddr; /*RW;Sensor register address*/
	CVI_U32 u32RegAddrByteNum; /*RW;Bit width of the sensor register address*/
	CVI_U32 u32Data; /*RW;Sensor register data*/
	CVI_U32 u32DataByteNum; /*RW;Bit width of sensor register data*/
} ISP_SSP_DATA_S;

typedef struct _ISP_SNS_REGS_INFO_S {
	ISP_SNS_TYPE_E enSnsType;
	CVI_U32 u32RegNum;
	CVI_U8 u8Cfg2ValidDelayMax;
	ISP_SNS_COMMBUS_U unComBus;
	union {
		ISP_I2C_DATA_S astI2cData[ISP_MAX_SNS_REGS];
		ISP_SSP_DATA_S astSspData[ISP_MAX_SNS_REGS];
	};

	struct {
		CVI_BOOL bUpdate;
		CVI_U8 u8DelayFrmNum;
		CVI_U32 u32SlaveVsTime; /* RW;time of vsync. Unit: inck clock cycle */
		CVI_U32 u32SlaveBindDev;
	} stSlvSync;

	CVI_BOOL bConfig;
	CVI_U8 use_snsr_sram;
	CVI_U8 need_update;
} ISP_SNS_REGS_INFO_S;

typedef enum _ISP_VD_TYPE_E {
	ISP_VD_FE_START = 0,
	ISP_VD_FE_END,
	ISP_VD_BE_END,
	ISP_VD_MAX
} ISP_VD_TYPE_E;

typedef struct _ISP_MANUAL_WDR_ATTR_S {
	CVI_S32 devno;
	CVI_S32 manual_en;
	CVI_S16 l2s_distance;
	CVI_S16 lsef_length;
	CVI_S32 discard_padding_lines;
	CVI_S32 update;
} ISP_MANUAL_WDR_ATTR_S;

typedef struct _ISP_SNS_CIF_INFO_S {
	ISP_MANUAL_WDR_ATTR_S wdr_manual;
	CVI_U8 u8DelayFrmNum; /*RW; Number of delayed frames for the cif setting */
	CVI_U8 need_update;
} ISP_SNS_CIF_INFO_S;

typedef struct _ISP_WDR_SIZE_S {
	RECT_S stWndRect;
	SIZE_S stSnsSize;
	SIZE_S stMaxSize;
} ISP_WDR_SIZE_S;

typedef struct _ISP_SNS_ISP_INFO_S {
	CVI_U32 frm_num;
	ISP_WDR_SIZE_S img_size[ISP_MAX_WDR_FRAME_NUM];
	CVI_U8 u8DelayFrmNum; /*RW; Number of delayed frames for the isp setting */
	CVI_U8 need_update;
} ISP_SNS_ISP_INFO_S;

typedef struct _ISP_SNS_SYNC_INFO_S {
	ISP_SNS_REGS_INFO_S snsCfg;
	ISP_SNS_ISP_INFO_S ispCfg;
	ISP_SNS_CIF_INFO_S cifCfg;
} ISP_SNS_SYNC_INFO_S;

typedef struct _ISP_PUB_ATTR_S {
	RECT_S stWndRect;
	SIZE_S stSnsSize;
	CVI_FLOAT f32FrameRate;
	ISP_BAYER_FORMAT_E enBayer;
	WDR_MODE_E enWDRMode;
	CVI_U8 u8SnsMode;
	CVI_U8 u8LaneNum; /*RW; Number of sensor data lane */
	CVI_U8 u8EnableMaster; /*RW; master(1) or slave(0) mode */
} ISP_PUB_ATTR_S;

typedef struct _ISP_CTRL_PARAM_S {
	CVI_U32 u32AEStatIntvl;
	CVI_U32 u32AWBStatIntvl;
	CVI_U32 u32AFStatIntvl;
	CVI_U32 u32ProcParam;
	CVI_U32 u32ProcLevel;
	CVI_U32 u32UpdatePos;
	CVI_U32 u32IntTimeOut;
	CVI_U32 u32PwmNumber;
	CVI_U32 u32PortIntDelay;
} ISP_CTRL_PARAM_S;

typedef union _ISP_MODULE_CTRL_U {
	CVI_U64 u64Key;
	struct {
		CVI_U64 bitBypassBlc : 1;		/*RW:[0]*/
		CVI_U64 bitBypassRlsc : 1;		/*RW:[1]*/
		CVI_U64 bitBypassFpn : 1;		/*RW:[2]*/
		CVI_U64 bitBypassDpc : 1;		/*RW:[3]*/
		CVI_U64 bitBypassCrosstalk : 1;	/*RW:[4]*/
		CVI_U64 bitBypassWBGain : 1;	/*RW:[5]*/
		CVI_U64 bitBypassDis : 1;		/*RW:[6]*/
		CVI_U64 bitBypassBnr : 1;		/*RW:[7]*/
		CVI_U64 bitBypassDemosaic : 1;	/*RW:[8]*/
		CVI_U64 bitBypassRgbcac : 1;	/*RW:[9]*/
		CVI_U64 bitBypassLcac : 1;		/*RW:[10]*/
		CVI_U64 bitBypassMlsc : 1;		/*RW:[11]*/
		CVI_U64 bitBypassCcm : 1;		/*RW:[12]*/
		CVI_U64 bitBypassFusion : 1;	/*RW:[13]*/
		CVI_U64 bitBypassDrc : 1;		/*RW:[14]*/
		CVI_U64 bitBypassGamma : 1;		/*RW:[15]*/
		CVI_U64 bitBypassDehaze : 1;	/*RW:[16]*/
		CVI_U64 bitBypassClut : 1;		/*RW:[17]*/
		CVI_U64 bitBypassCsc : 1;		/*RW:[18]*/
		CVI_U64 bitBypassDci : 1;		/*RW:[19]*/
		CVI_U64 bitBypassLdci : 1;		/*RW:[20]*/
		CVI_U64 bitBypassCa : 1;		/*RW:[21]*/
		CVI_U64 bitBypassPreyee : 1;	/*RW:[22]*/
		CVI_U64 bitBypassMotion : 1;	/*RW:[23]*/
		CVI_U64 bitBypass3dnr : 1;		/*RW:[24]*/
		CVI_U64 bitBypassYnr : 1;		/*RW:[25]*/
		CVI_U64 bitBypassCnr : 1;		/*RW:[26]*/
		CVI_U64 bitBypassCac : 1;		/*RW:[27]*/
		CVI_U64 bitBypassCa2 : 1;		/*RW:[28]*/
		CVI_U64 bitBypassYee : 1;		/*RW:[29]*/
		CVI_U64 bitBypassYcontrast : 1;	/*RW:[30]*/
		CVI_U64 bitBypassMono : 1;		/*RW:[31]*/
		CVI_U64 bitBypassBnrlite : 1;	/*RW:[32]*/
		CVI_U64 bitBypassPfr : 1;		/*RW:[33]*/
		CVI_U64 bitBypassLblc : 1;		/*RW:[34]*/
		CVI_U64 bitBypassAiBnr : 1;		/*RW:[35]*/
		CVI_U64 bitRsv : 28;			/*H; [36:63] */
	};
} ISP_MODULE_CTRL_U;

typedef union _ISP_STATISTICS_CTRL_U {
	CVI_U64 u64Key;
	struct {
		CVI_U64 bit1FEAeGloStat : 1; /* [0] */
		CVI_U64 bit1FEAeLocStat : 1; /* [1] */
		CVI_U64 bit1AwbStat1 : 1; /* [2] Awb Stat1 means global awb data. */
		CVI_U64 bit1AwbStat2 : 1; /* [3] Awb Stat2 means local awb data. */
		CVI_U64 bit1FEAfStat : 1; /* [4] */
		CVI_U64 bit14Rsv : 59; /* [5:63] */
	};
} ISP_STATISTICS_CTRL_U;

typedef enum _ISP_AE_STAT_MODE_E {
	ISP_AE_MODE_BUTT,
} ISP_AE_STAT_MODE_E;

typedef enum _ISP_AE_SWITCH_E {
	ISP_AE_SWITCH_BUTT,
} ISP_AE_SWITCH_E;

typedef enum _ISP_AE_FOUR_PLANE_MODE_E {
	ISP_AE_FOUR_PLANE_MODE_BUTT,
} ISP_AE_FOUR_PLANE_MODE_E;

typedef enum _ISP_AE_HIST_SKIP_E {
	ISP_AE_HIST_SKIP_BUTT,
} ISP_AE_HIST_SKIP_E;

typedef enum _ISP_AE_HIST_OFFSET_X_E {
	ISP_AE_HIST_OFFSET_X_BUTT,
} ISP_AE_HIST_OFFSET_X_E;

typedef enum _ISP_AE_HIST_OFFSET_Y_E {
	ISP_AE_HIST_OFFSET_Y_BUTT,
} ISP_AE_HIST_OFFSET_Y_E;

typedef enum _ISP_AE_ANTIFLICKER_FREQUENCE_E {
	AE_FREQUENCE_60HZ = 0,
	AE_FREQUENCE_50HZ,
} ISP_AE_ANTIFLICKER_FREQUENCE_E;

typedef enum _ISP_AE_GAIN_TYPE_E {
	AE_TYPE_GAIN = 0,
	AE_TYPE_ISO = 1,
	AE_TYPE_BUTT
} ISP_AE_GAIN_TYPE_E;

typedef struct _ISP_AE_HIST_CONFIG_S {
	ISP_AE_HIST_SKIP_E enHistSkipX;
	ISP_AE_HIST_SKIP_E enHistSkipY;
	ISP_AE_HIST_OFFSET_X_E enHistOffsetX;
	ISP_AE_HIST_OFFSET_Y_E enHistOffsetY;
} ISP_AE_HIST_CONFIG_S;

typedef struct _ISP_3AWIN_CONFIG_S {
	CVI_U16 winWidth;
	CVI_U16 winHeight;
	CVI_U16 winXOffset;
	CVI_U16 winYOffset;
	CVI_U8 winXNum;
	CVI_U8 winYNum;
} ISP_3AWIN_CONFIG_S;

typedef struct _ISP_AE_CROP_S {
	CVI_BOOL bEnable; /*RW; Range:[0x0,0x1]*/
	CVI_U16 u16X; /*RW; Range:[0x00,0x1FFF]*/
	CVI_U16 u16Y; /*RW; Range:[0x00,0x1FFF]*/
	CVI_U16 u16W; /*RW; Range:[0x00,0x1FFF]*/
	CVI_U16 u16H; /*RW; Range:[0x00,0x1FFF]*/
} ISP_AE_CROP_S;

typedef struct _ISP_AE_FACE_CROP_S {
	CVI_BOOL bEnable; /*RW; Range:[0x0,0x1]*/
	CVI_U16 u16X; /*RW; Range:[0x00,0x1FFF]*/
	CVI_U16 u16Y; /*RW; Range:[0x00,0x1FFF]*/
	CVI_U8 u16W; /*RW; Range:[0x00,0xFF]*/
	CVI_U8 u16H; /*RW; Range:[0x00,0xFF]*/
} ISP_AE_FACE_CROP_S;

#define AE_WEIGHT_ZONE_ROW	15
#define AE_WEIGHT_ZONE_COLUMN	17
#define FACE_WIN_NUM 4
typedef struct _ISP_AE_STATISTICS_CFG_S {
	CVI_BOOL bHisStatisticsEnable; /*RW; Range:[0x0,0x1]*/
	ISP_AE_CROP_S stCrop[AE_MAX_NUM];
	ISP_AE_FACE_CROP_S stFaceCrop[FACE_WIN_NUM];
	CVI_BOOL fast2A_ena; /*RW; Range:[0x0,0x1]*/
	CVI_U8 fast2A_ae_low; /*RW; Range:[0x0,0xFF]*/
	CVI_U8 fast2A_ae_high; /*RW; Range:[0x0,0xFF]*/
	CVI_U16 fast2A_awb_top; /*RW; Range:[0x0,0xFFF]*/
	CVI_U16 fast2A_awb_bot; /*RW; Range:[0x0,0xFFF]*/
	CVI_U16 over_exp_thr; /*RW; Range:[0x0,0x3FF]*/
	CVI_U8 au8Weight[AE_WEIGHT_ZONE_ROW][AE_WEIGHT_ZONE_COLUMN]; /*RW; Range:[0x0, 0xF]*/
} ISP_AE_STATISTICS_CFG_S;

#define FAST_CONVERGE_NODES	 (5)
typedef struct _ISP_AE_BOOT_FAST_CONVERGE_S {
	CVI_BOOL bEnable;
	CVI_U8  availableNode;
	CVI_S16 firstFrLuma[FAST_CONVERGE_NODES];
	CVI_S16 targetBv[FAST_CONVERGE_NODES];
} ISP_AE_BOOT_FAST_CONVERGE_S;

//-----------------------------------------------------------------------------
//  IRIS
//-----------------------------------------------------------------------------
typedef enum _ISP_IRIS_TYPE_E {
	ISP_IRIS_DC_TYPE = 0,
	ISP_IRIS_P_TYPE,
	ISP_IRIS_TYPE_BUTT,
} ISP_IRIS_TYPE_E;

typedef enum _ISP_IRIS_F_NO_E {
	ISP_IRIS_F_NO_32_0 = 0,
	ISP_IRIS_F_NO_22_0,
	ISP_IRIS_F_NO_16_0,
	ISP_IRIS_F_NO_11_0,
	ISP_IRIS_F_NO_8_0,
	ISP_IRIS_F_NO_5_6,
	ISP_IRIS_F_NO_4_0,
	ISP_IRIS_F_NO_2_8,
	ISP_IRIS_F_NO_2_0,
	ISP_IRIS_F_NO_1_4,
	ISP_IRIS_F_NO_1_0,
	ISP_IRIS_F_NO_BUTT,
} ISP_IRIS_F_NO_E;

typedef enum _ISP_IRIS_PID_MODE_E {
	ISP_IRIS_DELTA_PID,
	ISP_IRIS_DELTA_INTG_PID,
	ISP_IRIS_POSITION_PID
} ISP_IRIS_PID_MODE_E;

typedef struct _ISP_DCIRIS_ATTR_S {
	CVI_S32 s32Kp; /*RW; Range:[0x0, 0xF4240]*/
	CVI_S32 s32Ki; /*RW; Range:[0x0, 0xF4240]*/
	CVI_S32 s32Kd; /*RW; Range:[0x0, 0xF4240]*/
	CVI_U32 u32MinPwmDuty; /*RW; Range:[0x0, 0x3E8]*/
	CVI_U32 u32MaxPwmDuty; /*RW; Range:[0x0, 0x3E8]*/
	CVI_U32 u32OpenPwmDuty; /*RW; Range:[0x0, 0x3E8]*/
	CVI_U32 u32MaxSafeLuma; /*RW; Range:[0x0, 0x400]*/
	CVI_U32 u32ToSafePwmDuty; /*RW; Range:[0x0, 0x3E8]*/
	ISP_IRIS_PID_MODE_E enIrisPIDMode;
} ISP_DCIRIS_ATTR_S;

#define AI_MAX_STEP_FNO_NUM (1024)
typedef struct _ISP_PIRIS_ATTR_S {
	CVI_BOOL bStepFNOTableChange;
	CVI_BOOL bZeroIsMax;
	CVI_U16 u16TotalStep;
	CVI_U16 u16StepCount;
	CVI_U16 au16StepFNOTable[AI_MAX_STEP_FNO_NUM];
	ISP_IRIS_F_NO_E enMaxIrisFNOTarget;
	ISP_IRIS_F_NO_E enMinIrisFNOTarget;
	CVI_BOOL bFNOExValid;
	CVI_U32 u32MaxIrisFNOTarget;
	CVI_U32 u32MinIrisFNOTarget;
} ISP_PIRIS_ATTR_S;

typedef struct _ISP_MI_ATTR_S {
	CVI_U32 u32HoldValue; /*RW; Range:[0x0, 0x3E8]*/
	ISP_IRIS_F_NO_E enIrisFNO; /*RW; Range:[0x0, 0xA]*/
} ISP_MI_ATTR_S;

typedef enum _ISP_IRIS_STATUS_E {
	ISP_IRIS_KEEP  = 0,
	ISP_IRIS_OPEN  = 1,
	ISP_IRIS_CLOSE = 2,
	ISP_IRIS_BUTT
} ISP_IRIS_STATUS_E;

typedef struct _ISP_IRIS_ATTR_S {
	CVI_BOOL bEnable;
	ISP_OP_TYPE_E enOpType;
	ISP_IRIS_TYPE_E enIrisType;
	ISP_IRIS_STATUS_E enIrisStatus;
	ISP_MI_ATTR_S stMIAttr;
} ISP_IRIS_ATTR_S;

//-----------------------------------------------------------------------------
//  AE Route
//-----------------------------------------------------------------------------
#define ISP_AE_ROUTE_MIN_NODES (1)
#define ISP_AE_ROUTE_MAX_NODES (16)
typedef struct _ISP_AE_ROUTE_NODE_S {
	CVI_U32 u32IntTime; /*RW; Range:[0x0, 0x7FFFFFFF]*/
	CVI_U32 u32SysGain; /*RW; Range:[0x400, 0x7FFFFFFF]*/
	ISP_IRIS_F_NO_E enIrisFNO; /*RW; Range:[0x0, 0xA]*/
	CVI_U32 u32IrisFNOLin; /*RW; Range:[0x1, 0x400]*/
} ISP_AE_ROUTE_NODE_S;

typedef struct _ISP_AE_ROUTE_S {
	CVI_U32 u32TotalNum; /*RW; Range:[0x1, 0x10]*/
	ISP_AE_ROUTE_NODE_S astRouteNode[ISP_AE_ROUTE_MAX_NODES];
} ISP_AE_ROUTE_S;

#define ISP_AE_ROUTE_EX_MIN_NODES (1)
#define ISP_AE_ROUTE_EX_MAX_NODES (16)
typedef struct _ISP_AE_ROUTE_EX_NODE_S {
	CVI_U32 u32IntTime; /*RW; Range:[0x0, 0x7FFFFFFF]*/
	CVI_U32 u32Again; /*RW; Range:[0x0, 0x7FFFFFFF]*/
	CVI_U32 u32Dgain; /*RW; Range:[0x0, 0x7FFFFFFF]*/
	CVI_U32 u32IspDgain; /*RW; Range:[0x0, 0x7FFFFFFF]*/
	ISP_IRIS_F_NO_E enIrisFNO; /*RW; Range:[0x0, 0xA]*/
	CVI_U32 u32IrisFNOLin; /*RW; Range:[0x0, 0x400]*/
} ISP_AE_ROUTE_EX_NODE_S;

typedef struct _ISP_AE_ROUTE_EX_S {
	CVI_U32 u32TotalNum; /*RW; Range:[0x1, 0x10]*/
	ISP_AE_ROUTE_EX_NODE_S astRouteExNode[ISP_AE_ROUTE_EX_MAX_NODES];
} ISP_AE_ROUTE_EX_S;

//-----------------------------------------------------------------------------
//  Exposure Info
//-----------------------------------------------------------------------------
typedef struct _ISP_EXP_INFO_S {
	CVI_U32 u32ExpTime; /*R;*/
	CVI_U32 u32ShortExpTime; /*R;*/
	CVI_U32 u32MedianExpTime; /*R;*/
	CVI_U32 u32LongExpTime; /*R;*/
	CVI_U32 u32AGain; /*R;*/
	CVI_U32 u32DGain; /*R;*/
	CVI_U32 u32ISPDGain; /*R;*/
	CVI_U32 u32Exposure; /*R;*/
	CVI_BOOL bExposureIsMAX; /*R;*/
	CVI_BOOL bIsOverExpRange; /*R;*/
	CVI_BOOL bIsStable; /*R;*/
	CVI_S16 s16HistError; /*R;*/
	CVI_U32 au32AE_Hist256Value[HIST_NUM]; /*R;*/
	CVI_U8 u8AveLum; /*R;*/
	CVI_U32 u32LinesPer500ms; /*R;*/
	CVI_U32 u32PirisFNO; /*R;*/
	CVI_U32 u32Fps; /*R;*/
	CVI_U32 u32ISO; /*R;*/
	CVI_U32 u32ISOCalibrate; /*R;*/
	CVI_U32 u32RefExpRatio; /*R;*/
	CVI_U32 u32FirstStableTime; /*R;*/
	ISP_AE_ROUTE_S stAERoute; /*R;*/
	ISP_AE_ROUTE_EX_S stAERouteEx; /*R;*/
	CVI_U8 u8WDRShortAveLuma; /*R;*/
	CVI_U32 u32WDRExpRatio; /*R;*/
	CVI_U8 u8LEFrameAvgLuma; /*R;*/
	CVI_U8 u8SEFrameAvgLuma; /*R;*/
	CVI_FLOAT fLightValue; /*R;*/
	CVI_U32 u32AGainSF; /*R;*/
	CVI_U32 u32DGainSF; /*R;*/
	CVI_U32 u32ISPDGainSF; /*R;*/
	CVI_U32 u32ISOSF; /*R;*/
	ISP_AE_ROUTE_S stAERouteSF; /*R;*/
	ISP_AE_ROUTE_EX_S stAERouteSFEx; /*R;*/
	CVI_BOOL bGainSepStatus; /*R;*/
} ISP_EXP_INFO_S;

typedef struct _ISP_AE_CONVERGE_INFO_S {
	CVI_BOOL bIsStable; /*R;*/
	CVI_BOOL bIsOverExpRange; /*R;*/
	CVI_S16 s16HistErrorWithTolerance; /*R;*/
	CVI_U8 u8Tolerance; /*R;*/
} ISP_AE_CONVERGE_INFO_S;

#define SMART_MAX_NUM (3)
typedef struct _ISP_SMART_ROI_S {
	CVI_BOOL bEnable;
	CVI_BOOL bAvailable;
	CVI_U8 u8Luma;
	CVI_U8 u8Num;
	CVI_U16 u16PosX[SMART_MAX_NUM];
	CVI_U16 u16PosY[SMART_MAX_NUM];
	CVI_U16 u16Width[SMART_MAX_NUM];
	CVI_U16 u16Height[SMART_MAX_NUM];
	CVI_U16 u16FrameWidth;
	CVI_U16 u16FrameHeight;
} ISP_SMART_ROI_S;

#define SMART_CLASS_MAX (2)
typedef struct _ISP_SMART_INFO_S {
	ISP_SMART_ROI_S stROI[SMART_CLASS_MAX];
} ISP_SMART_INFO_S;

//-----------------------------------------------------------------------------
//  WDR Exposure Attr
//-----------------------------------------------------------------------------
#define WDR_EXP_RATIO_NUM (3)
typedef struct _ISP_WDR_EXPOSURE_ATTR_S {
	ISP_OP_TYPE_E enExpRatioType;
	CVI_U32 au32ExpRatio[WDR_EXP_RATIO_NUM]; /*RW; Range:[0x80, 0x400]*/
	CVI_U32 u32ExpRatioMax; /*RW; Range:[0x80, 0x400]*/
	CVI_U32 u32ExpRatioMin; /*RW; Range:[0x80, 0x400]*/
	CVI_U16 u16Tolerance; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 u16Speed; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 u16RatioBias; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U8 u8SECompensation; /*RW; Range:[0x0, 0xFF]*/
	ISP_CHANNEL_LIST_E enExpMainChn;
	CVI_U8 au8LEAdjustTargetMin[LV_TOTAL_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 au8LEAdjustTargetMax[LV_TOTAL_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 au8SEAdjustTargetMin[LV_TOTAL_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 au8SEAdjustTargetMax[LV_TOTAL_NUM]; /*RW; Range:[0x0, 0xFF]*/
} ISP_WDR_EXPOSURE_ATTR_S;

//-----------------------------------------------------------------------------
//  Smart Exposure Attr
//-----------------------------------------------------------------------------
typedef struct _ISP_SMART_EXPOSURE_ATTR_S {
	CVI_BOOL bEnable;
	CVI_BOOL bIRMode;
	ISP_OP_TYPE_E enSmartExpType;
	CVI_U8 u8LumaTarget; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 u16ExpCoef; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 u16ExpCoefMax; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 u16ExpCoefMin; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U8 u8SmartInterval; /*RW; Range:[0x1, 0xFF]*/
	CVI_U8 u8SmartSpeed; /*RW; Range:[0x1, 0xFF]*/
	CVI_U16 u16SmartDelayNum; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 u8Weight; /*RW; Range:[0x0, 0x64]*/
	CVI_U8 u8NarrowRatio; /*RW; Range:[0x0, 0x64]*/
} ISP_SMART_EXPOSURE_ATTR_S;

typedef enum _ISP_IR_STATUS_E {
	ISP_IR_STATUS_NORMAL = 0,
	ISP_IR_STATUS_IR = 1,
	ISP_IR_BUTT
} ISP_IR_STATUS_E;

typedef enum _ISP_IR_SWITCH_STATUS_E {
	ISP_IR_SWITCH_NONE = 0,
	ISP_IR_SWITCH_TO_NORMAL = 1,
	ISP_IR_SWITCH_TO_IR = 2,
	ISP_IR_SWITCH_BUTT
} ISP_IR_SWITCH_STATUS_E;

typedef struct _ISP_IR_AUTO_ATTR_S {
	CVI_BOOL bEnable;
	CVI_U32 u32Normal2IrIsoThr;
	CVI_U32 u32Ir2NormalIsoThr;
	CVI_U32 u32RGMax;
	CVI_U32 u32RGMin;
	CVI_U32 u32BGMax;
	CVI_U32 u32BGMin;
	ISP_IR_STATUS_E enIrStatus;
	ISP_IR_SWITCH_STATUS_E enIrSwitch;
} ISP_IR_AUTO_ATTR_S;

typedef struct _ISP_AWB_CROP_S {
	CVI_BOOL bEnable;
	CVI_U16 u16X; /*RW; Range:[0x0, 0x1000]*/
	CVI_U16 u16Y; /*RW; Range:[0x0, 0x1000]*/
	CVI_U16 u16W; /*RW; Range:[0x0, 0x1000]*/
	CVI_U16 u16H; /*RW; Range:[0x0, 0x1000]*/
} ISP_AWB_CROP_S;

typedef enum _ISP_AWB_SWITCH_E {
	ISP_AWB_AFTER_DG,
	ISP_AWB_AFTER_DRC,
	ISP_AWB_SWITCH_BUTT,
} ISP_AWB_SWITCH_E;

typedef enum _ISP_AWB_ALG_TYPE_E {
	AWB_ALG_LOWCOST,
	AWB_ALG_ADVANCE,
	AWB_ALG_BUTT
} ISP_AWB_ALG_TYPE_E;

typedef enum _ISP_AWB_MULTI_LS_TYPE_E {
	AWB_MULTI_LS_SAT,
	AWB_MULTI_LS_CCM,
	AWB_MULTI_LS_BUTT
} ISP_AWB_MULTI_LS_TYPE_E;

typedef enum _ISP_AWB_INDOOR_OUTDOOR_STATUS_E {
	AWB_INDOOR_MODE,
	AWB_OUTDOOR_MODE,
	AWB_INDOOR_OUTDOOR_BUTT
} ISP_AWB_INDOOR_OUTDOOR_STATUS_E;

typedef struct _ISP_WB_STATISTICS_CFG_S {
	ISP_AWB_SWITCH_E enAWBSwitch;
	CVI_U16 u16ZoneRow; /*RW; Range:[0x0, AWB_ZONE_ORIG_ROW]*/
	CVI_U16 u16ZoneCol; /*RW; Range:[0x0, AWB_ZONE_ORIG_COLUMN]*/
	CVI_U16 u16ZoneBin;
	CVI_U16 au16HistBinThresh[4];
	CVI_U16 u16WhiteLevel; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U16 u16BlackLevel; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U16 u16CbMax;
	CVI_U16 u16CbMin;
	CVI_U16 u16CrMax;
	CVI_U16 u16CrMin;
	ISP_AWB_CROP_S stCrop;
} ISP_WB_STATISTICS_CFG_S;

#define FIR_H_GAIN_NUM (5)
#define FIR_V_GAIN_NUM (3)
#define AF_GAMMA_NUM (256)
// AF window X region must between [0x8 ~ image xsize - 8].
// Y region must between [0x2 ~ image ysize - 2].
typedef struct _ISP_AF_CROP_S {
	CVI_BOOL bEnable;
	CVI_U16 u16X; /*RW; Range:[0x8, 0xFFF]*/
	CVI_U16 u16Y; /*RW; Range:[0x2, 0xFFF]*/
	CVI_U16 u16W; /*RW; Range:[0x110, 0xFFF]*/
	CVI_U16 u16H; /*RW; Range:[0xF0, 0xFFF]*/
} ISP_AF_CROP_S;

typedef struct _ISP_AF_RAW_CFG_S {
	CVI_BOOL PreGammaEn;
	CVI_U8 PreGammaTable[AF_GAMMA_NUM]; /*RW; Range:[0x0, 0xFF]*/
} ISP_AF_RAW_CFG_S;

typedef struct _ISP_AF_PRE_FILTER_CFG_S {
	CVI_BOOL PreFltEn;
} ISP_AF_PRE_FILTER_CFG_S;

typedef struct _ISP_AF_CFG_S {
	CVI_BOOL bEnable;
	CVI_U16 u16Hwnd; /*RW; Range:[0x2, 0x11]*/
	CVI_U16 u16Vwnd; /*RW; Range:[0x2, 0xF]*/
	CVI_U8 u8HFltShift; /*RW; Range:[0x0, 0xF]*/
	CVI_S8 s8HVFltLpCoeff[FIR_H_GAIN_NUM]; /*RW; Range:[0x0, 0x1F]*/
	ISP_AF_RAW_CFG_S stRawCfg;
	ISP_AF_PRE_FILTER_CFG_S stPreFltCfg;
	ISP_AF_CROP_S stCrop;
	CVI_U8 H0FltCoring; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 H1FltCoring; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 V0FltCoring; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 u16HighLumaTh; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 u8ThLow;
	CVI_U8 u8ThHigh;
	CVI_U8 u8GainLow; /*RW; Range:[0x0, 0xFE]*/
	CVI_U8 u8GainHigh; /*RW; Range:[0x0, 0xFE]*/
	CVI_U8 u8SlopLow; /*RW; Range:[0x0, 0xF]*/
	CVI_U8 u8SlopHigh; /*RW; Range:[0x0, 0xF]*/
} ISP_AF_CFG_S;

typedef struct _ISP_AF_H_PARAM_S {
	CVI_S8 s8HFltHpCoeff[FIR_H_GAIN_NUM]; /*RW; Range:[0x0, 0x1F]*/
} ISP_AF_H_PARAM_S;

typedef struct _ISP_AF_V_PARAM_S {
	CVI_S8 s8VFltHpCoeff[FIR_V_GAIN_NUM]; /*RW; Range:[0x0, 0x1F]*/
} ISP_AF_V_PARAM_S;

#define AF_WEIGHT_ZONE_ROW	15
#define AF_WEIGHT_ZONE_COLUMN	17
typedef struct _ISP_FOCUS_STATISTICS_CFG_S {
	ISP_AF_CFG_S stConfig;
	ISP_AF_H_PARAM_S stHParam_FIR0;
	ISP_AF_H_PARAM_S stHParam_FIR1;
	ISP_AF_V_PARAM_S stVParam_FIR;
	CVI_U8 au8Weight[AF_WEIGHT_ZONE_ROW][AF_WEIGHT_ZONE_COLUMN]; /*RW; Range:[0x0, 0xF]*/
} ISP_FOCUS_STATISTICS_CFG_S;

typedef struct _ISP_STATISTICS_CFG_S {
	ISP_STATISTICS_CTRL_U unKey;
	ISP_AE_STATISTICS_CFG_S stAECfg;
	ISP_WB_STATISTICS_CFG_S stWBCfg;
	ISP_FOCUS_STATISTICS_CFG_S stFocusCfg;
} ISP_STATISTICS_CFG_S;

typedef struct _ISP_AE_GRID_INFO_S {
	CVI_U16 au16GridYPos[AE_ZONE_ROW + 1];
	CVI_U16 au16GridXPos[AE_ZONE_COLUMN + 1];
	CVI_U8 u8Status;
} ISP_AE_GRID_INFO_S;

typedef struct _ISP_AE_STATISTICS_S {
	CVI_U32 au32FEHist1024Value[ISP_CHANNEL_MAX_NUM][AE_MAX_NUM][MAX_HIST_BINS];
	CVI_U16 au16FEGlobalAvg[ISP_CHANNEL_MAX_NUM][AE_MAX_NUM][BAYER_PATTERN_NUM];
	CVI_U16 au16FEZoneAvg[ISP_CHANNEL_MAX_NUM][AE_MAX_NUM][AE_ZONE_ROW][AE_ZONE_COLUMN][BAYER_PATTERN_NUM];
	CVI_U32 au32BEHist1024Value[MAX_HIST_BINS];
	CVI_U16 au16BEGlobalAvg[BAYER_PATTERN_NUM];
	CVI_U16 au16BEZoneAvg[AE_ZONE_ROW][AE_ZONE_COLUMN][BAYER_PATTERN_NUM];
	ISP_AE_GRID_INFO_S stFEGridInfo;
} ISP_AE_STATISTICS_S;

typedef struct _ISP_AWB_GRID_INFO_S {
	CVI_U16 au16GridYPos[AWB_ZONE_ORIG_ROW + 1];
	CVI_U16 au16GridXPos[AWB_ZONE_ORIG_COLUMN + 1];
	CVI_U8 u8Status;
} ISP_AWB_GRID_INFO_S;

typedef struct _ISP_WB_STATISTICS_S {
	CVI_U16 u16GlobalR[ISP_CHANNEL_MAX_NUM]; /*RW; Range:[0x0, 0x3FF]*/
	CVI_U16 u16GlobalG[ISP_CHANNEL_MAX_NUM]; /*RW; Range:[0x0, 0x3FF]*/
	CVI_U16 u16GlobalB[ISP_CHANNEL_MAX_NUM]; /*RW; Range:[0x0, 0x3FF]*/
	CVI_U16 u16CountAll[ISP_CHANNEL_MAX_NUM]; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 au16ZoneAvgR[ISP_CHANNEL_MAX_NUM][AWB_ZONE_NUM]; /*RW; Range:[0x0, 0x3FF]*/
	CVI_U16 au16ZoneAvgG[ISP_CHANNEL_MAX_NUM][AWB_ZONE_NUM]; /*RW; Range:[0x0, 0x3FF]*/
	CVI_U16 au16ZoneAvgB[ISP_CHANNEL_MAX_NUM][AWB_ZONE_NUM]; /*RW; Range:[0x0, 0x3FF]*/
	CVI_U16 au16ZoneCountAll[ISP_CHANNEL_MAX_NUM][AWB_ZONE_NUM]; /*RW; Range:[0x0, 0xFFFF]*/
	ISP_AWB_GRID_INFO_S stGridInfo;
} ISP_WB_STATISTICS_S;

typedef struct _ISP_FOCUS_ZONE_S {
	CVI_U16 u16HlCnt;
	CVI_U64 u64h0;
	CVI_U64 u64h1;
	CVI_U32 u32v0;
} ISP_FOCUS_ZONE_S;

typedef struct _ISP_FE_FOCUS_STATISTICS_S {
	ISP_FOCUS_ZONE_S stZoneMetrics[AF_ZONE_ROW][AF_ZONE_COLUMN]; /*R; The zoned measure of contrast*/
} ISP_FE_FOCUS_STATISTICS_S;

typedef struct _ISP_AF_STATISTICS_S {
	ISP_FE_FOCUS_STATISTICS_S stFEAFStat;
} ISP_AF_STATISTICS_S;

typedef struct _ISP_MG_STATISTICS_S {
	CVI_U16 au16ZoneAvg[MG_ZONE_ROW][MG_ZONE_COLUMN][BAYER_PATTERN_NUM];
} ISP_MG_STATISTICS_S;

//-----------------------------------------------------------------------------
//  WB Attr
//-----------------------------------------------------------------------------
#define AWB_CALIB_PTS_NUM (3)
#define AWB_CT_BIN_NUM (8)

typedef struct _ISP_AWB_LightBox_Gain_S {
	CVI_U16 u16AvgRgain;
	CVI_U16 u16AvgBgain;
} ISP_AWB_LightBox_Gain_S;

typedef struct _ISP_AWB_Calibration_Gain_S {
	CVI_U16 u16AvgRgain[AWB_CALIB_PTS_NUM]; /*RW; Range:[0x0, 0x3FFF]*/
	CVI_U16 u16AvgBgain[AWB_CALIB_PTS_NUM]; /*RW; Range:[0x0, 0x3FFF]*/
	CVI_U16 u16ColorTemperature[AWB_CALIB_PTS_NUM]; /*RW; Range:[0x1F4, 0x7530]*/
} ISP_AWB_Calibration_Gain_S;

#define AWB_CALIB_PTS_NUM_EX (10)
typedef struct _ISP_AWB_Calibration_Gain_S_EX {
	CVI_U16 u16AvgRgain[AWB_CALIB_PTS_NUM_EX]; /*RW; Range:[0x0, 0x3FFF]*/
	CVI_U16 u16AvgBgain[AWB_CALIB_PTS_NUM_EX]; /*RW; Range:[0x0, 0x3FFF]*/
	CVI_U16 u16ColorTemperature[AWB_CALIB_PTS_NUM_EX]; /*RW; Range:[0x1F4, 0x7530]*/
	CVI_U8 u8Weight[AWB_CALIB_PTS_NUM_EX];
} ISP_AWB_Calibration_Gain_S_EX;

#define AWB_LUM_HIST_NUM (6)
typedef struct _ISP_AWB_LUM_HISTGRAM_ATTR_S {
	CVI_BOOL bEnable;
	ISP_OP_TYPE_E enOpType;
	CVI_U8 au8HistThresh[AWB_LUM_HIST_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 au16HistWt[AWB_LUM_HIST_NUM]; /*RW; Range:[0x0, 0x200]*/
} ISP_AWB_LUM_HISTGRAM_ATTR_S;

typedef struct _ISP_AWB_CT_LIMIT_ATTR_S {
	CVI_BOOL bEnable;
	ISP_OP_TYPE_E enOpType;
	CVI_U16 u16HighRgLimit; /*RW; Range:[0x0, 0x3FFF] H temp,Rgain max*/
	CVI_U16 u16HighBgLimit; /*RW; Range:[0x0, 0x3FFF] H temp,Bgain max*/
	CVI_U16 u16LowRgLimit; /*RW; Range:[0x0, 0x3FFF] Low temp,Rgain min*/
	CVI_U16 u16LowBgLimit; /*RW; Range:[0x0, 0x3FFF] Low temp,Bgain min*/
} ISP_AWB_CT_LIMIT_ATTR_S;

typedef struct _ISP_AWB_CBCR_TRACK_ATTR_S {
	CVI_BOOL bEnable;
	CVI_U16 au16CrMax[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0x3FFF] R/G at L tmp*/
	CVI_U16 au16CrMin[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0x3FFF] R/G at H tmp*/
	CVI_U16 au16CbMax[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0x3FFF] B/G at H tmp*/
	CVI_U16 au16CbMin[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0x3FFF] B/G at L tmp*/
} ISP_AWB_CBCR_TRACK_ATTR_S;

typedef struct _ISP_AWB_IN_OUT_ATTR_S {
	CVI_BOOL bEnable;
	ISP_OP_TYPE_E enOpType;
	ISP_AWB_INDOOR_OUTDOOR_STATUS_E enOutdoorStatus;
	CVI_U32 u32OutThresh; /*RW; Range:[0x0, 0x14]*/
	CVI_U16 u16LowStart; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 u16LowStop; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 u16HighStart; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 u16HighStop; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_BOOL bGreenEnhanceEn;
	CVI_U8 u8OutShiftLimit;
} ISP_AWB_IN_OUT_ATTR_S;

typedef struct _ISP_AWB_EXTRA_LIGHTSOURCE_INFO_S {
	CVI_U16 u16WhiteRgain; /*RW; Range:[0x0, 0x3FFF]*/
	CVI_U16 u16WhiteBgain; /*RW; Range:[0x0, 0x3FFF]*/
	CVI_U16 u16ExpQuant; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U8 u8LightStatus; /*RW; Range:[0x0, 0x2]*/
	CVI_U8 u8Radius; /*RW; Range:[0x1, 0xFF]*/
} ISP_AWB_EXTRA_LIGHTSOURCE_INFO_S;

struct ST_ISP_AWB_INTERFERENCE_S {
	CVI_U8 u8Mode;	/*RW; Range:[0x0, 0x1]*/
	CVI_U8 u8Limit; /*RW; Range:[0x32, 0x64]*/
	CVI_U8 u8Radius; /*RW; Range:[0x1, 0xFF]*/
	CVI_U8 u8Ratio; /*RW; Range:[0x1, 0xFF]*/
	CVI_U8 u8Distance; /*RW; Range:[0x1, 0xFF]*/
};

struct ST_ISP_AWB_SKIN_S {
	CVI_U8 u8Mode;
	CVI_U16 u16RgainDiff;
	CVI_U16 u16BgainDiff;
	CVI_U8 u8Radius;
};

struct ST_ISP_AWB_SKY_S {
	CVI_U8 u8Mode;
	CVI_U8 u8ThrLv;
	CVI_U16 u16Rgain;
	CVI_U16 u16Bgain;
	CVI_U16 u16MapRgain;
	CVI_U16 u16MapBgain;
	CVI_U8 u8Radius;
};

struct ST_ISP_AWB_GRASS_S {
	CVI_U8 u8Mode;
	CVI_U8 u8ThrLv;
	CVI_U16 u16Rgain;
	CVI_U16 u16Bgain;
	CVI_U16 u16MapRgain;
	CVI_U16 u16MapBgain;
	CVI_U8 u8Radius;
};

enum ISP_AWB_TEMP_E {
	ISP_AWB_COLORTEMP_LOW,
	ISP_AWB_COLORTEMP_HIGH,
	ISP_AWB_COLORTEMP_NUM
};

struct ST_ISP_AWB_SHIFT_LV_S {
	CVI_U8 u8LowLvMode; /*RW; Range:[0x0, 0x1]*/
	CVI_U16 u16LowLvCT[ISP_AWB_COLORTEMP_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 u16LowLvThr[ISP_AWB_COLORTEMP_NUM]; /*RW; Range:[0x0, 0x5DC]*/
	CVI_U16 u16LowLvRatio[ISP_AWB_COLORTEMP_NUM]; /*RW; Range:[0x64, 0x3E8]*/
	CVI_U8 u8HighLvMode; /*RW; Range:[0x0, 0x1]*/
	CVI_U16 u16HighLvCT[ISP_AWB_COLORTEMP_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 u16HighLvThr[ISP_AWB_COLORTEMP_NUM]; /*RW; Range:[0x0, 0x5DC]*/
	CVI_U16 u16HighLvRatio[ISP_AWB_COLORTEMP_NUM]; /*RW; Range:[0x64, 0x3E8]*/
};

struct ST_ISP_AWB_REGION_S {
	CVI_U16 u16Region1; /*RW; Range:[0x1, 0xFFFF]*/
	CVI_U16 u16Region2; /*RW; Range:[0x1, 0xFFFF]*/
	CVI_U16 u16Region3; /*RW; Range:[0x1, 0xFFFF]*/
};

#define AWB_CT_LV_NUM (4)
struct ST_ISP_AWB_CT_WGT_S {
	CVI_BOOL bEnable;
	CVI_U16 au16MultiCTBin[AWB_CT_BIN_NUM];//low to High ColorTemp
	CVI_S8 s8ThrLv[AWB_CT_LV_NUM];//low to high light
	CVI_U16 au16MultiCTWt[AWB_CT_LV_NUM][AWB_CT_BIN_NUM];//0~1024
};

#define AWB_CURVE_PARA_NUM (6)
#define AWB_CURVE_BOUND_NUM (8)
#define AWB_LS_NUM (4)
#define AWB_ZONE_WT_W (32)
#define AWB_ZONE_WT_H (32)
#define AWB_ZONE_WT_NUM (AWB_ZONE_WT_W * AWB_ZONE_WT_H)
#define AWB_TARGET_RATIO_NUM (3)

typedef struct _ISP_AWB_ATTR_S {
	CVI_BOOL bEnable;
	CVI_U16 u16RefColorTemp; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 au16StaticWB[ISP_BAYER_CHN_NUM]; /*RW; Range:[0x0, 0xFFFF] G/R*/
	CVI_S32 as32CurvePara[AWB_CURVE_PARA_NUM];
	ISP_AWB_ALG_TYPE_E enAlgType;
	CVI_U16 u16Speed; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U16 u16ZoneSel; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 u16HighColorTemp; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 u16LowColorTemp; /*RW; Range:[0x0, 0xFFFF]*/
	ISP_AWB_CT_LIMIT_ATTR_S stCTLimit;
	CVI_BOOL bShiftLimitEn;
	CVI_U16 u16ShiftLimit[AWB_CURVE_BOUND_NUM]; /*RW; Range:[0x0, 0x3FF]*/
	CVI_BOOL bGainNormEn;
	CVI_BOOL bNaturalCastEn;
	ISP_AWB_CBCR_TRACK_ATTR_S stCbCrTrack;// R/G
	ISP_AWB_LUM_HISTGRAM_ATTR_S stLumaHist;
	CVI_BOOL bAWBZoneWtEn;
	CVI_U8 au8ZoneWt[AWB_ZONE_WT_NUM];
} ISP_AWB_ATTR_S;

typedef struct _ISP_AWB_ATTR_EX_S {
	CVI_U8 u8Tolerance;
	CVI_U8 u8ZoneRadius;
	CVI_U16 u16CurveLLimit; /*RW; Range:[0x0, 0x400]*/
	CVI_U16 u16CurveRLimit; /*RW; Range:[0x200, 0x3FFF]*/
	CVI_BOOL bExtraLightEn;
	ISP_AWB_EXTRA_LIGHTSOURCE_INFO_S stLightInfo[AWB_LS_NUM];
	ISP_AWB_IN_OUT_ATTR_S stInOrOut;
	CVI_BOOL bMultiLightSourceEn;
	ISP_AWB_MULTI_LS_TYPE_E enMultiLSType;
	CVI_U16 u16MultiLSScaler; /*RW; Range:[0x0, 0x100]*/
	CVI_U16 u16MultiLSThr; /*RW; Range:[0x20, 0x200]*/
	CVI_U16 u16CALumaDiff; /*RW; Range:[0x0, 0x100]*/
	CVI_U16 u16CAAdjustRatio; /*RW; Range:[0x0, 0x10]*/
	CVI_U16 au16MultiCTBin[AWB_CT_BIN_NUM]; /*RW; Range:[0x0, 0xFFFF]*/
	CVI_U16 au16MultiCTWt[AWB_CT_BIN_NUM]; /*RW; Range:[0x0, 0x400]*/
	CVI_BOOL bFineTunEn;
	CVI_U8 u8FineTunStrength;
	CVI_U16 u16TargetCT[AWB_TARGET_RATIO_NUM]; /*RW; Range:[0x1, 0x4000]*/
	CVI_U8 u8TargetStrengthR[AWB_TARGET_RATIO_NUM]; /*RW; Range:[0x40, 0xFF]*/
	CVI_U8 u8TargetStrengthB[AWB_TARGET_RATIO_NUM]; /*RW; Range:[0x40, 0xFF]*/
	//AWB Algo 6
	struct ST_ISP_AWB_INTERFERENCE_S stInterference;
	struct ST_ISP_AWB_SKIN_S stSkin;
	struct ST_ISP_AWB_SKY_S stSky;
	struct ST_ISP_AWB_GRASS_S stGrass;
	struct ST_ISP_AWB_CT_WGT_S stCtLv;
	struct ST_ISP_AWB_SHIFT_LV_S stShiftLv;
	struct ST_ISP_AWB_REGION_S stRegion;
	CVI_U8 adjBgainMode;
} ISP_AWB_ATTR_EX_S;//keep size to 512 bytes

typedef struct _ISP_MWB_ATTR_S {
	CVI_U16 u16Rgain; /*RW; Range:[0x1, 0x3FFF]*/
	CVI_U16 u16Grgain; /*RW; Range:[0x1, 0x3FFF]*/
	CVI_U16 u16Gbgain; /*RW; Range:[0x1, 0x3FFF]*/
	CVI_U16 u16Bgain; /*RW; Range:[0x1, 0x3FFF]*/
} ISP_MWB_ATTR_S;

typedef enum _ISP_AWB_ALG_E {
	ALG_AWB,
	ALG_AWB_SPEC,
	ALG_BUTT
} ISP_AWB_ALG_E;

typedef enum _ISP_AWB_WDR_STS_E {
	WDR_STS_NORMAL,
	WDR_STS_LE,
	WDR_STS_SE,
	WDR_STS_BUTT
} ISP_AWB_WDR_STS_E;

typedef struct _ISP_WB_ATTR_S {
	CVI_BOOL bByPass;
	CVI_U8 u8AWBRunInterval; /*RW; Range:[0x1, 0xFF]*/
	ISP_OP_TYPE_E enOpType;
	ISP_MWB_ATTR_S stManual;
	ISP_AWB_ATTR_S stAuto;
	ISP_AWB_ALG_E enAlgType;
	ISP_AWB_WDR_STS_E enWdrStsMode;
	CVI_U8 u8DebugMode;
} ISP_WB_ATTR_S;

//-----------------------------------------------------------------------------
//  WB Info
//-----------------------------------------------------------------------------
typedef struct _ISP_WB_INFO_S {
	CVI_U16 u16Rgain; /*R; Range:[0x0, 0xFFF]*/
	CVI_U16 u16Grgain; /*R; Range:[0x0, 0xFFF]*/
	CVI_U16 u16Gbgain; /*R; Range:[0x0, 0xFFF]*/
	CVI_U16 u16Bgain; /*R; Range:[0x0, 0xFFF]*/
	CVI_U16 u16Saturation; /*R; Range:[0x0, 0xFF]*/
	CVI_U16 u16ColorTemp; /*R;*/
	CVI_U16 au16CCM[CCM_MATRIX_SIZE]; /*R;*/
	CVI_U16 u16LS0CT; /*R;*/
	CVI_U16 u16LS1CT; /*R;*/
	CVI_U16 u16LS0Area; /*R;*/
	CVI_U16 u16LS1Area; /*R;*/
	CVI_U8 u8MultiDegree; /*R;*/
	CVI_U16 u16ActiveShift; /*R;*/
	CVI_U32 u32FirstStableTime; /*R;*/
	ISP_AWB_INDOOR_OUTDOOR_STATUS_E enInOutStatus; /*R;*/
	CVI_S16 s16Bv; /*R;*/
} ISP_WB_INFO_S;

//-----------------------------------------------------------------------------
//  FOCUS Attr
//-----------------------------------------------------------------------------
typedef enum _AF_DIRECTION {
	AF_DIR_NEAR,
	AF_DIR_FAR,
} AF_DIRECTION;

typedef enum _AF_CHASINGFOCUS_MODE {
	AF_CHASINGFOCUS_FAST_MODE,
	AF_CHASINGFOCUS_NORMAL_MODE,
} AF_CHASINGFOCUS_MODE;

typedef enum _AF_STATUS {
	AF_NOT_INIT,
	AF_INIT_POSITION,
	AF_INIT,
	AF_TRIGGER_FOCUS,
	AF_DETECT_DIRECTION,
	AF_SERACH_BEST_POS,
	AF_LOCATE_BEST_POS,
	AF_FOCUSED,
	AF_REVERSE_DIRECTION, //for hw limit
	AF_CHASING_FOCUS
} AF_STATUS;

typedef enum _AF_MANUAL_TYPE {
	OP_TYPE_ZOOM,
	OP_TYPE_FOCUS,
	OP_TYPE_AUTO_FOCUS,
	OP_TYPE_ZOOM_POS,
	OP_TYPE_FOCUS_POS,
	OP_TYPE_AF_BUTT
} AF_MANUAL_TYPE;

typedef struct _ISP_FOCUS_MANUAL_ATTR_S {
	AF_MANUAL_TYPE enOpType;
	AF_DIRECTION enManualDir;
	CVI_U16 u16ManualStep; /*RW; Range:[0x0, 0x400]*/
	CVI_U16 u16ManualPos; /*RW; Range:[0x0, 0x8000]*/
} ISP_FOCUS_MANUAL_ATTR_S;

typedef struct _ISP_FOCUS_ATTR_S {
	CVI_BOOL bEnable;
	CVI_U8 u8DebugMode;
	ISP_OP_TYPE_E enOpType;
	CVI_U8 u8RunInterval; /*RW; Range:[0x1, 0x10]*/
	CVI_BOOL bRealTimeFocus;
	CVI_BOOL bChasingFocus;
	CVI_BOOL bReFocus;
	CVI_BOOL bMixHlc;
	CVI_U8 u8RtFocusStableFrm; /*RW; Range:[0x1, 0xFF]*/
	CVI_U16 u16RtMaxDiffFvRatio; /*RW; Range:[0x400, 0x1000]*/
	CVI_U16 u16MaxDiffFvRatio; /*RW; Range:[0x400, 0x4000]*/
	CVI_U16 u16MaxDiffLumaRatio; /*RW; Range:[0x400, 0x1000]*/
	CVI_U16 u16DetectDiffRatio; /*RW; Range:[0x400, 0x1000]*/
	CVI_U16 u16SearchDiffRatio; /*RW; Range:[0x400, 0x1000]*/
	CVI_U16 u16LocalDiffRatio; /*RW; Range:[0x400, 0x1000]*/
	CVI_U8 u8InitStep; /*RW; Range:[0x1, 0xFF]*/
	CVI_U8 u8FindStep; /*RW; Range:[0x1, 0xFF]*/
	CVI_U8 u8LocateStep; /*RW; Range:[0x1, 0xFF]*/
	AF_DIRECTION enInitDir;
	CVI_U16 u16MaxRotateCnt; /*RW; Range:[0x400, 0x4000]*/
	AF_CHASINGFOCUS_MODE enChasingFocusMode;
	ISP_FOCUS_MANUAL_ATTR_S stManual;
} ISP_FOCUS_ATTR_S;

//-----------------------------------------------------------------------------
//  FOCUS Info
//-----------------------------------------------------------------------------
typedef enum _ISP_AF_MOTOR_SPEED_E {
	AF_MOTOR_SPEED_4X,
	AF_MOTOR_SPEED_2X,
	AF_MOTOR_SPEED_1X,
	AF_MOTOR_SPEED_HALF,
} ISP_AF_MOTOR_SPEED_E;

typedef struct _ISP_AF_Q_INFO_S {
	AF_STATUS eStatus;      /*R;*/
	CVI_U32 u32PreFv;       /*R;*/
	CVI_U32 u32CurFv;       /*R;*/
	CVI_U32 u32MaxFv;       /*R;*/
	CVI_U8 u8PreLuma;       /*R;*/
	CVI_U8 u8CurLuma;       /*R;*/
	AF_DIRECTION eZoomDir;  /*R;*/
	CVI_U16 u16ZoomStep;    /*R;*/
	CVI_U16 u16ZoomPos;     /*R;*/
	AF_DIRECTION eFocusDir; /*R;*/
	CVI_U16 u16FocusStep;   /*R;*/
	CVI_U16 u16FocusPos;    /*R;*/
	CVI_U16 u16MaxFvPos;    /*R;*/
	CVI_U16 u16ZoomMinPos;    /*R;*/
	CVI_U16 u16ZoomMaxPos;    /*R;*/
	CVI_U16 u16FocusMinPos;    /*R;*/
	CVI_U16 u16FocusMaxPos;    /*R;*/
	ISP_AF_MOTOR_SPEED_E eZoomSpeed;  /*R;*/
	ISP_AF_MOTOR_SPEED_E eFocusSpeed; /*R;*/
} ISP_FOCUS_Q_INFO_S;

typedef struct _ISP_DCF_CONST_INFO_S {
	CVI_U8 au8ImageDescription[DCF_DRSCRIPTION_LENGTH]; /*Describes image*/
	CVI_U8 au8Make[DCF_DRSCRIPTION_LENGTH]; /*Shows manufacturer of digital cameras*/
	CVI_U8 au8Model[DCF_DRSCRIPTION_LENGTH]; /*Shows model number of digital cameras*/
	CVI_U8 au8Software[DCF_DRSCRIPTION_LENGTH];
	/*Shows firmware (internal software of digital cameras) version number*/
	CVI_U8 u8LightSource;
	CVI_U32 u32FocalLength; /*Focal length of lens used to take image. Unit is millimeter*/
	CVI_U8 u8SceneType;
	/*Indicates the type of scene. Value '0x01' means that the image was directly photographed.*/
	CVI_U8 u8CustomRendered;
	CVI_U8 u8FocalLengthIn35mmFilm; /*Indicates the equivalent focal length assuming a 35mm film camera, in mm*/
	CVI_U8 u8SceneCaptureType;
	CVI_U8 u8GainControl;
	CVI_U8 u8Contrast;
	CVI_U8 u8Saturation;
	CVI_U8 u8Sharpness;
	CVI_U8 u8MeteringMode;
} ISP_DCF_CONST_INFO_S;

typedef struct _ISP_DCF_UPDATE_INFO_S {
	CVI_U32 u32ISOSpeedRatings; /*CCD sensitivity equivalent to Ag-Hr film speedrate*/
	CVI_U32 u32ExposureTime; /*Exposure time (reciprocal of shutter speed).*/
	CVI_U32 u32ExposureBiasValue; /*Exposure bias (compensation) value of taking picture*/
	CVI_U8 u8ExposureProgram;
	CVI_U32 u32FNumber; /*The actual F-number (F-stop) of lens when the image was taken*/
	CVI_U32 u32MaxApertureValue; /*Maximum aperture value of lens.*/
	CVI_U8 u8ExposureMode;
	CVI_U8 u8WhiteBalance;
} ISP_DCF_UPDATE_INFO_S;

typedef struct _ISP_DCF_INFO_S {
	ISP_DCF_CONST_INFO_S stIspDCFConstInfo;
	ISP_DCF_UPDATE_INFO_S stIspDCFUpdateInfo;
} ISP_DCF_INFO_S;

typedef struct _ISP_MOD_PARAM_S {
	CVI_U32 u32IntBotHalf;
} ISP_MOD_PARAM_S;

//-----------------------------------------------------------------------------
//  Color tone
//-----------------------------------------------------------------------------
typedef struct _ISP_COLOR_TONE_ATTR_S {
	CVI_U16 u16RedCastGain; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U16 u16GreenCastGain; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U16 u16BlueCastGain; /*RW; Range:[0x0, 0xFFF]*/
} ISP_COLOR_TONE_ATTR_S;

typedef struct _ISP_FSHDR_MAP_CURVE_INFO_S {
	CVI_U8 EV_FMT;
	CVI_U32 MapCurveL[65];
	CVI_U32 MapCurveR[17];
	CVI_U32 MapCurveEnd[17];
} ISP_FSHDR_MAP_CURVE_INFO_S;

//-----------------------------------------------------------------------------
//  Local Black Level Correction(LBLC)
//-----------------------------------------------------------------------------
#define ISP_LBLC_ISO_SIZE (7)
#define ISP_LBLC_GRID_COL (8)
#define ISP_LBLC_GRID_ROW (8)
#define ISP_LBLC_GRID_POINTS (ISP_LBLC_GRID_COL * ISP_LBLC_GRID_ROW)

typedef struct _ISP_LBLC_LUT_S {
	CVI_U32 iso; /*RW; Range:[0x64, 0x320000]*/
	CVI_U16 lblcOffsetR[ISP_LBLC_GRID_POINTS]; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U16 lblcOffsetGr[ISP_LBLC_GRID_POINTS]; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U16 lblcOffsetGb[ISP_LBLC_GRID_POINTS]; /*RW; Range:[0x0, 0xFFF]*/
	CVI_U16 lblcOffsetB[ISP_LBLC_GRID_POINTS]; /*RW; Range:[0x0, 0xFFF]*/
} ISP_LBLC_LUT_S;

typedef struct _ISP_LBLC_LUT_ATTR_S {
	CVI_U8 size; /*RW; Range:[0x1, 0x7]*/
	ISP_LBLC_LUT_S lblcLut[ISP_LBLC_ISO_SIZE];
} ISP_LBLC_LUT_ATTR_S;

typedef struct _ISP_LBLC_MANUAL_ATTR_S {
	CVI_U16 strength; /*RW; Range:[0x0, 0xFFF]*/
} ISP_LBLC_MANUAL_ATTR_S;

typedef struct _ISP_LBLC_AUTO_ATTR_S {
	CVI_U16 strength[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0xFFF]*/
} ISP_LBLC_AUTO_ATTR_S;

typedef struct _ISP_LBLC_ATTR_S {
	CVI_BOOL enable; /*RW; Range:[0x0, 0x1]*/
	ISP_OP_TYPE_E enOpType;
	CVI_U8 UpdateInterval; /*RW; Range:[0x1, 0xFF]*/
	ISP_LBLC_MANUAL_ATTR_S stManual;
	ISP_LBLC_AUTO_ATTR_S stAuto;
} ISP_LBLC_ATTR_S;

//-----------------------------------------------------------------------------
//  TEAISP BNR
//-----------------------------------------------------------------------------
#define TEAISP_MODEL_PATH_LEN (128)
typedef struct _TEAISP_BNR_MODEL_INFO_S {
	char path[TEAISP_MODEL_PATH_LEN];
	CVI_U32 enterISO;
	CVI_U32 tolerance;
} TEAISP_BNR_MODEL_INFO_S;

typedef struct _TEAISP_BNR_MANUAL_ATTR_S {
	CVI_U8 FilterMotionStr2D; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 FilterStaticStr2D; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 FilterStr3D; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 FilterStr2D; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 NoiseLevel; /*RW; Range:[0x0, 0x3FFF]*/
	CVI_U16 NoiseHiLevel; /*RW; Range:[0x0, 0x3FFF]*/
} TEAISP_BNR_MANUAL_ATTR_S;

typedef struct _TEAISP_BNR_AUTO_ATTR_S {
	CVI_U8 FilterMotionStr2D[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 FilterStaticStr2D[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 FilterStr3D[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U8 FilterStr2D[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0xFF]*/
	CVI_U16 NoiseLevel[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0x3FFF]*/
	CVI_U16 NoiseHiLevel[ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0x3FFF]*/
} TEAISP_BNR_AUTO_ATTR_S;

typedef struct _TEAISP_BNR_ATTR_S {
	CVI_BOOL enable; /*RW; Range:[0x0, 0x1]*/
	ISP_OP_TYPE_E enOpType;
	CVI_U8 UpdateInterval; /*RW; Range:[0x1, 0xFF]*/
	CVI_U8 DebugMode; /*RW; Range:[0x0, 0x6]*/
	TEAISP_BNR_MANUAL_ATTR_S stManual;
	TEAISP_BNR_AUTO_ATTR_S stAuto;
} TEAISP_BNR_ATTR_S;

typedef struct _TEAISP_BNR_NP_S {
	CVI_FLOAT CalibrationCoef[2][ISP_AUTO_ISO_STRENGTH_NUM]; /*RW; Range:[0x0, 0xFF]*/
} TEAISP_BNR_NP_S;


//-----------------------------------------------------------------------------
//  isp module (auto gen)
//-----------------------------------------------------------------------------
#include "cvi_comm_isp_module.h"

//-----------------------------------------------------------------------------
//  INNER INFO
//-----------------------------------------------------------------------------
typedef struct _ISP_INNER_STATE_INFO_S {
	CVI_U32 blcOffsetR;
	CVI_U32 blcOffsetGr;
	CVI_U32 blcOffsetGb;
	CVI_U32 blcOffsetB;
	CVI_U32 blcDgainR;
	CVI_U32 blcDgainGr;
	CVI_U32 blcDgainGb;
	CVI_U32 blcDgainB;
	CVI_S32 ccm[9];
	CVI_U16 drc_outbld_curve_l_lut[DRC_CURVE_L_LUT_NUM];
	CVI_U16 drc_outbld_curve_r_lut[DRC_CURVE_R_LUT_NUM];
	CVI_U32 dci_hist[DCI_HIST_BINS_NUM];
	CVI_U32 drc_hist[DRC_HIST_BINS_NUM];
	CVI_U32 rlsc_hist_r[RLSC_HIST_BINS_NUM];
	CVI_U32 rlsc_hist_g[RLSC_HIST_BINS_NUM];
	CVI_U32 rlsc_hist_b[RLSC_HIST_BINS_NUM];
	CVI_BOOL bWDRSwitchFinish;
	// For 2TO1/3TO1/4TO1 use.
	CVI_U32 u32WDRExpRatioActual[ISP_WDR_FRAME_IDX_SIZE];
	ISP_MESH_SHADING_GAIN_LUT_S mlscGainTable;
} ISP_INNER_STATE_INFO_S;

//-----------------------------------------------------------------------------
//  TOP Page
//-----------------------------------------------------------------------------
typedef struct _ISP_ModuleEn_ATTR_S {
	CVI_BOOL Mlsc;
	CVI_BOOL Fusion;
	CVI_BOOL MapCurve;
	CVI_BOOL Dpc;
	CVI_BOOL CrossTalk;
	CVI_BOOL Bnr;
	CVI_BOOL Blc;
	CVI_BOOL Rlsc;
	CVI_BOOL Drc;
	CVI_BOOL Demosaic;
	CVI_BOOL Pfr;
	CVI_BOOL Ccm;
	CVI_BOOL Gamma;
	CVI_BOOL Clut;
	CVI_BOOL Csc;
	CVI_BOOL Ldci;
	CVI_BOOL Dci;
	CVI_BOOL PreSharpen;
	CVI_BOOL Tnr;
	CVI_BOOL Cnr;
	CVI_BOOL Sharpen;
	CVI_BOOL Ca;
	CVI_BOOL Ca2;
	CVI_BOOL YContrast;
	CVI_BOOL Mono;
} ISP_ModuleEn_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_COMM_ISP_H__ */
