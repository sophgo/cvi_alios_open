#ifndef __CVI_SNS_CTRL_H__
#define __CVI_SNS_CTRL_H__

#include "cvi_comm_cif.h"
#include "cvi_comm_video.h"
#include "cvi_errno.h"
#include "cvi_type.h"
#include "cvi_debug.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct combo_dev_attr_s SNS_COMBO_DEV_ATTR_S;
typedef CVI_S32 (*AHD_Callback)(CVI_S32, CVI_S32);

// Structure representing the configuration for I2C GPIO settings.
typedef struct _SNS_I2C_GPIO_INFO_S {
	CVI_S8 s8I2cDev;			// I2C device identifier.
	CVI_S32 s32I2cAddr;			// I2C address of the sensor.
	CVI_U32 u32Rst_port_idx;	// Port index for the reset pin.
	CVI_U32 u32Rst_pin;			// Reset pin number.
	CVI_U32 u32Rst_pol;			// Polarity of the reset pin (0 for active low, 1 for active high).
} SNS_I2C_GPIO_INFO_S;

 // Enum type for AHD modes
typedef enum _SNS_AHD_MODE_E {
	AHD_MODE_NONE,				// No AHD mode selected
	AHD_MODE_1280X720H_NTSC,	// 1280x720 resolution, NTSC format
	AHD_MODE_1280X720H_PAL,		// 1280x720 resolution, PAL format
	AHD_MODE_1280X720P25,		// 1280x720 resolution, 25 frames per second
	AHD_MODE_1280X720P30,		// 1280x720 resolution, 30 frames per second
	AHD_MODE_1280X720P50,		// 1280x720 resolution, 50 frames per second
	AHD_MODE_1280X720P60,		// 1280x720 resolution, 60 frames per second
	AHD_MODE_1920X1080P25,		// 1920x1080 resolution, 25 frames per second
	AHD_MODE_1920X1080P30,		// 1920x1080 resolution, 30 frames per second
	AHD_MODE_2304X1296P25,		// 2304x1296 resolution, 25 frames per second
	AHD_MODE_2304X1296P30,		// 2304x1296 resolution, 30 frames per second
	AHD_MODE_BUIT,				// Reserved or undefined mode
} SNS_AHD_MODE_S;

// Structure representing the status message of a sensor.
typedef struct _SNS_STATUS_MSG_S {
	CVI_S32 s32SnsId;		// Identifier for the sensor.
	CVI_S32 s32Status;		// Current status of the sensor (e.g., operational, error).
	CVI_S32 eMode;			// Operational mode of the sensor (e.g., normal, standby).
} SNS_STATUS_MSG_S;

typedef struct _SNS_EXP_MAX_S {
	CVI_U16 manual;				/*Manual exposure setting (1: enabled, 0: disabled). */
	CVI_U32 ratio[3];			/*Exposure ratios for different modes. */
	CVI_U32 IntTimeMax[4];		/*Maximum integration time for different scenarios. */
	CVI_U32 IntTimeMin[4];		/*Minimum integration time for different scenarios. */
	CVI_U32 LFMaxIntTime[4];	/*Maximum low-frequency integration times. */
} SNS_EXP_MAX_S;

// Structure representing the gain settings for a sensor.
typedef struct _SNS_GAIN_S {
	CVI_U32 gain;			/*Gain value for the sensor.*/
	CVI_U32 gainDb;			/*Gain in decibels.*/
} SNS_GAIN_S;

typedef struct _SNS_ATTR_S {
	CVI_U16	u16Min;			/* Current MIN exposure of sensor */
	CVI_U16 u16Max;			/* Current MAX exposure of sensor */
	CVI_U16 u16Def;			/* Current default setting exposure of sensor */
	CVI_U16 u16Step;		/* Current exposure step of sensor */
} SNS_ATTR_S;

typedef struct _SNS_ATTR_LARGE_S {
	CVI_U32	u32Min;			/* Current MIN gain of sensor */
	CVI_U32 u32Max;			/* Current MAX gain of sensor */
	CVI_U32 u32Def;			/* Current default setting gain of sensor */
	CVI_U32 u32Step;		/* Current gain step of sensor */
} SNS_ATTR_LARGE_S;

typedef struct _ISP_SNS_STATE_S {
	CVI_BOOL	 bInit;					/* CVI_TRUE: Sensor init */
	CVI_BOOL	 bSyncInit;				/* CVI_TRUE: Sync Reg init */
	CVI_U8	   u8ImgMode;				/* Current image mode of sensor */
	CVI_U8	   u8Hdr;					/* CVI_TRUE: HDR enbale */
	WDR_MODE_E   enWDRMode;				/* Current wdr mode of sensor */

	ISP_SNS_SYNC_INFO_S astSyncInfo[2];	/* [0]: Sensor reg info of cur-frame; [1]: Sensor reg info of pre-frame ; */

	CVI_U32	  au32FL[2];				/* [0]: FullLines of cur-frame; [1]: Pre FullLines of pre-frame */
	CVI_U32	  u32FLStd;				/* FullLines std */
	CVI_U32	  au32WDRIntTime[4];		/* Number of exposure lines under WDR */
} ISP_SNS_STATE_S;

typedef enum _ISP_SNS_MIRRORFLIP_TYPE_E {	/* Flip mirror function of sensor */
	ISP_SNS_NORMAL	  = 0,
	ISP_SNS_MIRROR	  = 1,
	ISP_SNS_FLIP		= 2,
	ISP_SNS_MIRROR_FLIP = 3,
	ISP_SNS_BUTT
} ISP_SNS_MIRRORFLIP_TYPE_E;

typedef enum _ISP_SNS_L2S_MODE_E {
	SNS_L2S_MODE_AUTO = 0,	/* sensor l2s distance varies by the inttime of sef. */
	SNS_L2S_MODE_FIX,		/* sensor l2s distance is fixed. */
} ISP_SNS_INTTIME_MODE_E;

typedef struct _MCLK_ATTR_S {
	CVI_U8 u8Mclk;			/* Select which mclk the Sensor uses. */
	CVI_BOOL bMclkEn;		/* Select whether the Sensor uses the current mclk. */
} MCLK_ATTR_S;

typedef struct _RX_INIT_ATTR_S {
	CVI_U32 MipiDev;		/* Select which MAC the Sensor uses. */
	CVI_U32 MipiMode;
	CVI_S16 as16LaneId[MIPI_LANE_NUM + 1];	/* Select MIPI lane sequence the Sensor uses. */
	CVI_S16 as16FuncId[TTL_PIN_FUNC_NUM];
	CVI_S8  as8PNSwap[MIPI_LANE_NUM + 1];	/* Select whether MIPI lane swap the Sensor uses. */
	MCLK_ATTR_S stMclkAttr;	/* Set MCLK information. */
	CVI_BOOL hsettlen;
	CVI_U8 hsettle;
} RX_INIT_ATTR_S;

typedef enum _SNS_BDG_MUX_MODE_E {
	SNS_BDG_MUX_NONE = 0,	/* sensor bridge mux is disabled */
	SNS_BDG_MUX_2,			/* sensor bridge mux 2 input */
	SNS_BDG_MUX_3,			/* sensor bridge mux 3 input */
	SNS_BDG_MUX_4,			/* sensor bridge mux 4 input */
} SNS_BDG_MUX_MODE_E;

typedef enum _ISP_SNS_SYNCMODE_TYPE_E {	/* Master and salve function of sensor */
	ISP_SNS_SLAVE_BY_FSYNC			= 0,
	ISP_SNS_MASTER_BY_FSYNC			= 1,
	ISP_SNS_NORMAL_MODE				= 2,
	ISP_SNS_SLAVE_BY_HIGH_ACTIVE	= 3,
	ISP_SNS_SLAVE_BY_LOW_ACTIVE		= 4,
} ISP_SNS_SYNCMODE_TYPE_E;

typedef struct _ISP_INIT_ATTR_S {	/* Sensor exp ctrl info */
	CVI_U32 u32ExpTime;
	CVI_U32 u32AGain;
	CVI_U32 u32DGain;
	CVI_U32 u32ISPDGain;
	CVI_U32 u32Exposure;
	CVI_U32 u32LinesPer500ms;
	CVI_U32 u32PirisFNO;
	CVI_U16 u16WBRgain;
	CVI_U16 u16WBGgain;
	CVI_U16 u16WBBgain;
	CVI_U16 u16SampleRgain;
	CVI_U16 u16SampleBgain;
	CVI_U16 u16UseHwSync;
	ISP_SNS_GAIN_MODE_E enGainMode;
	ISP_SNS_INTTIME_MODE_E enL2SMode;
	SNS_BDG_MUX_MODE_E enSnsBdgMuxMode;
} ISP_INIT_ATTR_S;

// Structure representing the AHD (Analog High Definition) object.
typedef struct _SNS_AHD_OBJ_S {
	CVI_S32 (*pfnAhdInit)(VI_PIPE ViPipe, bool isFirstInit); // Initialize AHD sensor.
	CVI_S32 (*pfnAhdDeinit)(VI_PIPE ViPipe); // Deinitialize AHD sensor.
	CVI_S32 (*pfnGetAhdMode)(VI_PIPE ViPipe); // Get current AHD mode.
	CVI_S32 (*pfnSetAhdMode)(VI_PIPE ViPipe, CVI_S32 astAhdMode); // Set AHD mode.
	CVI_S32 (*pfnSetAhdBusInfo)(VI_PIPE ViPipe, CVI_S32 astI2cDev); // Set AHD communication bus information.
	CVI_S32 (*pfnDetectAhdStatus)(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType); // Detect AHD status.
} SNS_AHD_OBJ_S;

typedef struct _ISP_SNS_OBJ_S {		/* Sensor callback object struct */
	CVI_S32(*pfnRegisterCallback)(VI_PIPE ViPipe, ALG_LIB_S *, ALG_LIB_S *);
	CVI_S32(*pfnUnRegisterCallback)(VI_PIPE ViPipe, ALG_LIB_S *, ALG_LIB_S *);
	CVI_S32(*pfnSetBusInfo)(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo);
	CVI_VOID(*pfnStandby)(VI_PIPE ViPipe);
	CVI_VOID(*pfnRestart)(VI_PIPE ViPipe);
	CVI_VOID(*pfnMirrorFlip)(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
	CVI_S32(*pfnWriteReg)(VI_PIPE ViPipe, CVI_S32 s32Addr, CVI_S32 s32Data);
	CVI_S32(*pfnReadReg)(VI_PIPE ViPipe, CVI_S32 s32Addr);
	CVI_S32(*pfnSetInit)(VI_PIPE ViPipe, ISP_INIT_ATTR_S *);
	CVI_S32(*pfnPatchRxAttr)(VI_PIPE ViPipe, RX_INIT_ATTR_S *);
	CVI_VOID(*pfnPatchI2cAddr)(VI_PIPE ViPipe, CVI_S32 s32I2cAddr);
	CVI_S32(*pfnGetRxAttr)(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *);
	CVI_S32(*pfnExpSensorCb)(ISP_SENSOR_EXP_FUNC_S *);
	CVI_S32(*pfnExpAeCb)(AE_SENSOR_EXP_FUNC_S *);
	CVI_S32(*pfnAHDCb)(SNS_AHD_OBJ_S *);
	CVI_S32(*pfnSnsProbe)(VI_PIPE ViPipe);
} ISP_SNS_OBJ_S;

#if defined(CONFIG_DUAL_OS)
#define CMOS_CHECK_POINTER(ptr)\
	do {\
		if (ptr == CVI_NULL) {\
			printf("[ERROR]Null Pointer!\n");\
			return CVI_ERR_VI_INVALID_NULL_PTR;\
		} \
	} while (0)
#else
#define CMOS_CHECK_POINTER(ptr)\
	do {\
		if (ptr == CVI_NULL) {\
			syslog(LOG_ERR, "Null Pointer!\n");\
			return CVI_ERR_VI_INVALID_NULL_PTR;\
		} \
	} while (0)
#endif

#if defined(CONFIG_DUAL_OS)
#define CMOS_CHECK_POINTER_VOID(ptr)\
	do {\
		if (ptr == CVI_NULL) {\
			printf("[ERROR]Null Pointer!\n");\
			return;\
		} \
	} while (0)
#else
#define CMOS_CHECK_POINTER_VOID(ptr)\
	do {\
		if (ptr == CVI_NULL) {\
			syslog(LOG_ERR, "Null Pointer!\n");\
			return;\
		} \
	} while (0)
#endif

#define SENSOR_FREE(ptr)\
	do {\
		if (ptr != CVI_NULL) {\
			free(ptr);\
			ptr = CVI_NULL;\
		} \
	} while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_SNS_CTRL_H__ */
