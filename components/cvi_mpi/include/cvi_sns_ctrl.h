#ifndef __CVI_SNS_CTRL_H__
#define __CVI_SNS_CTRL_H__

#include <cvi_comm_cif.h>
#include <cvi_errno.h>
#include <cvi_type.h>
#include "cvi_debug.h"
#include "cvi_comm_3a.h"
#include "cvi_comm_isp.h"
#include "cvi_ae_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct combo_dev_attr_s SNS_COMBO_DEV_ATTR_S;
typedef struct mclk_pll_s SNS_MCLK_ATTR_S;

// Structure representing the maximum exposure settings for a sensor.
typedef struct _SNS_EXP_MAX_S {
    CVI_U16 manual;               // Manual exposure setting (1: enabled, 0: disabled).
    CVI_U32 ratio[3];            // Exposure ratios for different modes.
    CVI_U32 IntTimeMax[4];       // Maximum integration time for different scenarios.
    CVI_U32 IntTimeMin[4];       // Minimum integration time for different scenarios.
    CVI_U32 LFMaxIntTime[4];     // Maximum low-frequency integration times.
} SNS_EXP_MAX_S;


// Structure representing the gain settings for a sensor.
typedef struct _SNS_GAIN_S {
    CVI_U32 gain;      // Gain value for the sensor.
    CVI_U32 gainDb;    // Gain in decibels.
} SNS_GAIN_S;

// Structure representing the attributes of a sensor with small range values.
typedef struct _SNS_ATTR_S {
    CVI_U16 u16Min;    // Minimum attribute value.
    CVI_U16 u16Max;    // Maximum attribute value.
    CVI_U16 u16Def;    // Default attribute value.
    CVI_U16 u16Step;   // Step size for attribute increments.
} SNS_ATTR_S;

// Structure representing the attributes of a sensor with large range values.
typedef struct _SNS_ATTR_LARGE_S {
    CVI_U32 u32Min;    // Minimum attribute value.
    CVI_U32 u32Max;    // Maximum attribute value.
    CVI_U32 u32Def;    // Default attribute value.
    CVI_U32 u32Step;   // Step size for attribute increments.
} SNS_ATTR_LARGE_S;


typedef struct _ISP_SNS_STATE_S {
	CVI_BOOL     bInit;                  /* CVI_TRUE: Sensor init */
	CVI_BOOL     bSyncInit;              /* CVI_TRUE: Sync Reg init */
	CVI_U8       u8ImgMode;
	CVI_U8       u8Hdr;               /* CVI_TRUE: HDR enbale */
	WDR_MODE_E  enWDRMode;

	ISP_SNS_SYNC_INFO_S astSyncInfo[2]; /* [0]: Sensor reg info of cur-frame; [1]: Sensor reg info of pre-frame ; */

	CVI_U32      au32FL[2];              /* [0]: FullLines of cur-frame; [1]: Pre FullLines of pre-frame */
	CVI_U32      u32FLStd;               /* FullLines std */
	CVI_U32      au32WDRIntTime[4];
} ISP_SNS_STATE_S;

// Enumeration representing the mirror and flip types for a sensor.
typedef enum _ISP_SNS_MIRRORFLIP_TYPE_E {
    ISP_SNS_NORMAL      = 0,  // No mirror or flip.
    ISP_SNS_MIRROR      = 1,  // Mirror the image horizontally.
    ISP_SNS_FLIP        = 2,  // Flip the image vertically.
    ISP_SNS_MIRROR_FLIP = 3,  // Mirror and flip the image.
    ISP_SNS_BUTT        // Placeholder for invalid type.
} ISP_SNS_MIRRORFLIP_TYPE_E;


typedef enum _ISP_SNS_L2S_MODE_E {
	SNS_L2S_MODE_AUTO = 0,	/* sensor l2s distance varies by the inttime of sef. */
	SNS_L2S_MODE_FIX,	/* sensor l2s distance is fixed. */
} ISP_SNS_INTTIME_MODE_E;

// Structure representing the attributes for the master clock (MCLK).
typedef struct _MCLK_ATTR_S {
    CVI_U8 u8Mclk;      // Master clock frequency.
    CVI_BOOL bMclkEn;   // Enable/disable the master clock.
} MCLK_ATTR_S;

// Structure representing the initialization attributes for a receiver.
typedef struct _RX_INIT_ATTR_S {
    CVI_U32 MipiDev;               // MIPI device identifier.
    CVI_S16 as16LaneId[5];        // Array of lane IDs for MIPI communication.
    CVI_S8  as8PNSwap[5];          // Array for pin swap configuration.
    MCLK_ATTR_S stMclkAttr;        // Master clock attributes.
} RX_INIT_ATTR_S;


typedef enum _SNS_BDG_MUX_MODE_E {
	SNS_BDG_MUX_NONE = 0,	/* sensor bridge mux is disabled */
	SNS_BDG_MUX_2,		/* sensor bridge mux 2 input */
	SNS_BDG_MUX_3,		/* sensor bridge mux 3 input */
	SNS_BDG_MUX_4,		/* sensor bridge mux 4 input */
} SNS_BDG_MUX_MODE_E;

// Structure representing the initialization attributes for the ISP (Image Signal Processor).
typedef struct _ISP_INIT_ATTR_S {
    CVI_U32 u32ExpTime;          // Exposure time in microseconds.
    CVI_U32 u32AGain;            // Analog gain value.
    CVI_U32 u32DGain;            // Digital gain value.
    CVI_U32 u32ISPDGain;         // ISP gain value.
    CVI_U32 u32Exposure;         // Overall exposure setting.
    CVI_U32 u32LinesPer500ms;    // Number of lines processed in 500 milliseconds.
    CVI_U32 u32PirisFNO;         // F-number for the lens aperture.
    CVI_U16 u16WBRgain;          // White balance red gain.
    CVI_U16 u16WBGgain;          // White balance green gain.
    CVI_U16 u16WBBgain;          // White balance blue gain.
    CVI_U16 u16SampleRgain;      // Sample red gain.
    CVI_U16 u16SampleBgain;      // Sample blue gain.
    CVI_U16 u16UseHwSync;        // Hardware synchronization flag.
    ISP_SNS_GAIN_MODE_E enGainMode;    // Gain mode enumeration.
    ISP_SNS_INTTIME_MODE_E enL2SMode;  // Integration time mode enumeration.
    SNS_BDG_MUX_MODE_E enSnsBdgMuxMode; // Sensor bandwidth multiplexing mode.
} ISP_INIT_ATTR_S;


// Structure representing the sensor object for the ISP (Image Signal Processor).
typedef struct _ISP_SNS_OBJ_S {
    CVI_S32 (*pfnRegisterCallback)(VI_PIPE ViPipe, ALG_LIB_S *, ALG_LIB_S *); // Register callback function.
    CVI_S32 (*pfnUnRegisterCallback)(VI_PIPE ViPipe, ALG_LIB_S *, ALG_LIB_S *); // Unregister callback function.
    CVI_S32 (*pfnSetBusInfo)(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo); // Set communication bus information.
    CVI_VOID (*pfnStandby)(VI_PIPE ViPipe); // Put the sensor into standby mode.
    CVI_VOID (*pfnRestart)(VI_PIPE ViPipe); // Restart the sensor.
    CVI_VOID (*pfnMirrorFlip)(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip); // Set mirror/flip mode.
    CVI_S32 (*pfnWriteReg)(VI_PIPE ViPipe, CVI_S32 s32Addr, CVI_S32 s32Data); // Write to a sensor register.
    CVI_S32 (*pfnReadReg)(VI_PIPE ViPipe, CVI_S32 s32Addr); // Read from a sensor register.
    CVI_S32 (*pfnSetInit)(VI_PIPE ViPipe, ISP_INIT_ATTR_S *); // Initialize the sensor with attributes.
    CVI_S32 (*pfnPatchRxAttr)(RX_INIT_ATTR_S *); // Patch receiver attributes.
    CVI_VOID (*pfnPatchI2cAddr)(CVI_S32 s32I2cAddr); // Patch I2C address.
    CVI_S32 (*pfnGetRxAttr)(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *); // Get receiver attributes.
    CVI_S32 (*pfnExpSensorCb)(ISP_SENSOR_EXP_FUNC_S *); // Callback for sensor exposure.
    CVI_S32 (*pfnExpAeCb)(AE_SENSOR_EXP_FUNC_S *); // Callback for AE (Auto Exposure) sensor.
    CVI_S32 (*pfnSnsProbe)(VI_PIPE ViPipe); // Probe the sensor for status.
} ISP_SNS_OBJ_S;

// Structure representing the AHD (Analog High Definition) object.
typedef struct _SNS_AHD_OBJ_S {
    CVI_S32 (*pfnAhdInit)(VI_PIPE ViPipe, bool isFirstInit); // Initialize AHD sensor.
    CVI_S32 (*pfnAhdDeinit)(VI_PIPE ViPipe); // Deinitialize AHD sensor.
    CVI_S32 (*pfnGetAhdMode)(VI_PIPE ViPipe); // Get current AHD mode.
    CVI_S32 (*pfnSetAhdMode)(VI_PIPE ViPipe, CVI_S32 astAhdMode); // Set AHD mode.
    CVI_S32 (*pfnSetAhdBusInfo)(VI_PIPE ViPipe, CVI_S32 astI2cDev); // Set AHD communication bus information.
    CVI_S32 (*pfnDetectAhdStatus)(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType); // Detect AHD status.
} SNS_AHD_OBJ_S;


#define CMOS_CHECK_POINTER(ptr)\
	do {\
		if (ptr == CVI_NULL) {\
			syslog(LOG_ERR, "Null Pointer!\n");\
			return CVI_ERR_VI_INVALID_NULL_PTR;\
		} \
	} while (0)

#define CMOS_CHECK_POINTER_VOID(ptr)\
	do {\
		if (ptr == CVI_NULL) {\
			syslog(LOG_ERR, "Null Pointer!\n");\
			return;\
		} \
	} while (0)

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
