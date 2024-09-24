/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_common_sns.h
 * Description:
 */

#ifndef _CVI_COMM_SNS_H_
#define _CVI_COMM_SNS_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_type.h>
#include <cvi_defines.h>
#include "cvi_debug.h"
#include "cvi_comm_isp.h"

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++

#define NOISE_PROFILE_CHANNEL_NUM 4
#define NOISE_PROFILE_LEVEL_NUM 2
#define NOISE_PROFILE_ISO_NUM 16
#define USE_USER_SEN_DRIVER 1


// Structure representing the image mode of a CMOS sensor.
typedef struct _ISP_CMOS_SENSOR_IMAGE_MODE_S {
    CVI_U16 u16Width;     // Image width in pixels.
    CVI_U16 u16Height;    // Image height in pixels.
    CVI_FLOAT f32Fps;     // Frame rate in frames per second.
    CVI_U8 u8SnsMode;     // Sensor mode identifier.
} ISP_CMOS_SENSOR_IMAGE_MODE_S;


// Structure representing the black level settings for a CMOS sensor.
typedef struct _ISP_CMOS_BLACK_LEVEL_S {
    CVI_BOOL bUpdate;            // Flag indicating whether to update the black level settings.
    ISP_BLACK_LEVEL_ATTR_S blcAttr; // Attributes for black level configuration.
} ISP_CMOS_BLACK_LEVEL_S;


// Structure representing sensor attribute information.
typedef struct _ISP_SNS_ATTR_INFO_S {
    CVI_U32 eSensorId; // Identifier for the sensor.
} ISP_SNS_ATTR_INFO_S;


// Structure representing noise calibration parameters for a CMOS sensor.
typedef struct cviISP_CMOS_NOISE_CALIBRATION_S {
	// Calibration coefficients for different ISO levels, channels, and noise levels.
    CVI_FLOAT CalibrationCoef[NOISE_PROFILE_ISO_NUM][NOISE_PROFILE_CHANNEL_NUM][NOISE_PROFILE_LEVEL_NUM];
} ISP_CMOS_NOISE_CALIBRATION_S;


// Structure representing default settings for a CMOS sensor.
typedef struct _ISP_CMOS_DEFAULT_S {
    ISP_CMOS_NOISE_CALIBRATION_S stNoiseCalibration; // Noise calibration settings for the sensor.
} ISP_CMOS_DEFAULT_S;


// Structure representing the function pointers for sensor operations.
typedef struct _ISP_SENSOR_EXP_FUNC_S {
    CVI_VOID (*pfn_cmos_sensor_init)(VI_PIPE ViPipe);                     // Function to initialize the CMOS sensor.
    CVI_VOID (*pfn_cmos_sensor_exit)(VI_PIPE ViPipe);                     // Function to exit and clean up the CMOS sensor.
    CVI_VOID (*pfn_cmos_sensor_global_init)(VI_PIPE ViPipe);              // Function for global initialization of the CMOS sensor.
    CVI_S32 (*pfn_cmos_set_image_mode)(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode); // Function to set the image mode of the sensor.
    CVI_S32 (*pfn_cmos_set_wdr_mode)(VI_PIPE ViPipe, CVI_U8 u8Mode);     // Function to set the Wide Dynamic Range (WDR) mode of the sensor.

    /* Functions to retrieve sensor-related data, excluding 3A algorithms */
    CVI_S32 (*pfn_cmos_get_isp_default)(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef); // Function to get ISP default settings.
    CVI_S32 (*pfn_cmos_get_isp_black_level)(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel); // Function to get black level settings.
    CVI_S32 (*pfn_cmos_get_sns_reg_info)(VI_PIPE ViPipe, ISP_SNS_SYNC_INFO_S *pstSnsRegsInfo); // Function to get sensor register information.

    /* Function to enable or disable pixel detection */
    // CVI_VOID (*pfn_cmos_set_pixel_detect)(VI_PIPE ViPipe, bool bEnable);
} ISP_SENSOR_EXP_FUNC_S;


// Structure representing the registration of a sensor with its associated functions.
typedef struct bmISP_SENSOR_REGISTER_S {
    ISP_SENSOR_EXP_FUNC_S stSnsExp; // Structure containing function pointers for sensor operations.
} ISP_SENSOR_REGISTER_S;


// Enumeration representing the various master clock frequencies.
typedef enum _MCLK_FREQ_E {
    MCLK_FREQ_NONE = 0,       // No master clock frequency specified.
    MCLK_FREQ_37P125M,       // Master clock frequency of 37.125 MHz.
    MCLK_FREQ_25M,           // Master clock frequency of 25 MHz.
    MCLK_FREQ_27M,           // Master clock frequency of 27 MHz.
    MCLK_FREQ_NUM             // Total number of defined master clock frequencies.
} MCLK_FREQ_E;


// Structure representing the master clock configuration for a sensor.
typedef struct _SNS_MCLK_S {
    CVI_U32 u8Cam;          // Camera identifier.
    MCLK_FREQ_E enFreq;    // Selected master clock frequency for the sensor.
} SNS_MCLK_S;

// -------- If you want to change these interfaces, please contact the isp team. --------

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _CVI_COMM_SNS_H_ */
