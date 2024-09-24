/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_sensor.h
 * Description:
 *   MMF Programe Interface for sensor management moudle
 */

#ifndef __CVI_SENSOR_H__
#define __CVI_SENSOR_H__

#include <stdio.h>
#include <cvi_sns_ctrl.h>

typedef CVI_S32 (*AHD_Callback)(CVI_S32, CVI_S32);

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

// Structure representing the configuration for I2C GPIO settings.
typedef struct _SNS_I2C_GPIO_INFO_S {
    CVI_S8 s8I2cDev;          // I2C device identifier.
    CVI_S32 s32I2cAddr;      // I2C address of the sensor.
    CVI_U32 u32Rst_port_idx;  // Port index for the reset pin.
    CVI_U32 u32Rst_pin;       // Reset pin number.
    CVI_U32 u32Rst_pol;       // Polarity of the reset pin (0 for active low, 1 for active high).
} SNS_I2C_GPIO_INFO_S;

 // Enum type for AHD modes
typedef enum _SNS_AHD_MODE_E {
    AHD_MODE_NONE,                // No AHD mode selected
    AHD_MODE_1280X720H_NTSC,     // 1280x720 resolution, NTSC format
    AHD_MODE_1280X720H_PAL,      // 1280x720 resolution, PAL format
    AHD_MODE_1280X720P25,        // 1280x720 resolution, 25 frames per second
    AHD_MODE_1280X720P30,        // 1280x720 resolution, 30 frames per second
    AHD_MODE_1280X720P50,        // 1280x720 resolution, 50 frames per second
    AHD_MODE_1280X720P60,        // 1280x720 resolution, 60 frames per second
    AHD_MODE_1920X1080P25,       // 1920x1080 resolution, 25 frames per second
    AHD_MODE_1920X1080P30,       // 1920x1080 resolution, 30 frames per second
    AHD_MODE_2304X1296P25,       // 2304x1296 resolution, 25 frames per second
    AHD_MODE_2304X1296P30,       // 2304x1296 resolution, 30 frames per second
    AHD_MODE_BUIT,               // Reserved or undefined mode
} SNS_AHD_MODE_S;

// Structure representing the status message of a sensor.
typedef struct _SNS_STATUS_MSG_S {
    CVI_S32 s32SnsId;  // Identifier for the sensor.
    CVI_S32 s32Status;  // Current status of the sensor (e.g., operational, error).
    CVI_S32 eMode;      // Operational mode of the sensor (e.g., normal, standby).
} SNS_STATUS_MSG_S;


/**
 * @brief Registers a callback function for detecting AHD (Analog High Definition) status.
 *
 * This function allows the user to provide a callback that will be invoked
 * when the AHD status changes or needs to be detected. The callback can be used
 * to handle specific actions based on the detected AHD status.
 *
 * @param CB The callback function to be registered. It should match the expected
 *           signature for AHD detection callbacks.
 */
CVI_VOID CVI_SENSOR_AHDRegisterDetect(AHD_Callback CB);

/**
 * @brief Enables detection for the specified video input pipe.
 *
 * This function activates the detection mechanism for the given video input pipe,
 * allowing the system to monitor and respond to changes in sensor status or conditions.
 *
 * @param ViPipe The identifier for the video input pipe to enable detection on.
 *               This should correspond to a valid video input pipeline.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_EnableDetect(CVI_S32 ViPipe);

/**
 * @brief Initializes GPIO settings for the specified video input pipe.
 *
 * This function configures the GPIO pins according to the provided settings.
 *
 * @param ViPipe The identifier for the video input pipe to initialize.
 * @param pstGpioCfg Pointer to the GPIO configuration structure.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_GPIO_Init(VI_PIPE ViPipe, SNS_I2C_GPIO_INFO_S *pstGpioCfg);

/**
 * @brief Retrieves the current AHD status.
 *
 * This function fetches the status of the AHD sensor and stores it in the provided structure.
 *
 * @param pstStatus Pointer to the structure that will hold the AHD status.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_GetAhdStatus(SNS_STATUS_MSG_S *pstStatus);

/**
 * @brief Sets the sensor type for the specified video input pipe.
 *
 * This function defines the type of sensor to be used with the specified video input pipe.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param SnsType The type of sensor to set.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsType(VI_PIPE ViPipe, CVI_U32 SnsType);

/**
 * @brief Sets the RX attributes for the specified video input pipe.
 *
 * This function configures the RX attributes according to the provided settings.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param pstRxAttr Pointer to the RX attribute structure.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsRxAttr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxAttr);

/**
 * @brief Configures the I2C settings for the specified video input pipe.
 *
 * This function sets the I2C device and address for communication with the sensor.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param astI2cDev The I2C device identifier.
 * @param s32I2cAddr The I2C address of the sensor.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsI2c(VI_PIPE ViPipe, CVI_S32 astI2cDev, CVI_S32 s32I2cAddr);

/**
 * @brief Sets the ISP attributes for the specified video input pipe.
 *
 * This function configures the ISP settings according to the provided initialization attributes.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param pstInitAttr Pointer to the ISP initialization attribute structure.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsIspAttr(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr);

/**
 * @brief Registers a callback for the specified video input pipe and ISP device.
 *
 * This function allows the user to register a callback function for handling events.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param IspDev The identifier for the ISP device.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_RegCallback(VI_PIPE ViPipe, ISP_DEV IspDev);

/**
 * @brief Unregisters a callback for the specified video input pipe and ISP device.
 *
 * This function removes a previously registered callback function.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param IspDev The identifier for the ISP device.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_UnRegCallback(VI_PIPE ViPipe, ISP_DEV IspDev);

/**
 * @brief Sets the image mode for the specified video input pipe.
 *
 * This function configures the image mode settings for the sensor.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param stSnsrMode Pointer to the sensor image mode structure.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsImgMode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *stSnsrMode);

/**
 * @brief Sets the WDR mode for the specified video input pipe.
 *
 * This function configures the Wide Dynamic Range (WDR) settings for the sensor.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param wdrMode The WDR mode to set.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsWdrMode(VI_PIPE ViPipe, WDR_MODE_E wdrMode);

/**
 * @brief Retrieves the RX attributes for the specified video input pipe.
 *
 * This function fetches the RX attributes and stores them in the provided structure.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param stDevAttr Pointer to the structure that will hold the RX attributes.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_GetSnsRxAttr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *stDevAttr);

/**
 * @brief Sets the sensor probe for the specified video input pipe.
 *
 * This function activates the sensor probing mechanism for the given video input pipe.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsProbe(VI_PIPE ViPipe);

/**
 * @brief Initializes GPIO settings for the sensor.
 *
 * This function configures the GPIO settings for the specified device number and pin.
 *
 * @param devNo The device number for the GPIO configuration.
 * @param u32Rst_port_idx The port index for the reset pin.
 * @param u32Rst_pin The reset pin number.
 * @param u32Rst_pol The reset pin polarity.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsGpioInit(CVI_U32 devNo, CVI_U32 u32Rst_port_idx, CVI_U32 u32Rst_pin,
                                   CVI_U32 u32Rst_pol);

/**
 * @brief Resets the sensor GPIO for the specified device number.
 *
 * This function enables or disables the reset functionality for the sensor GPIO.
 *
 * @param devNo The device number for the GPIO reset.
 * @param rstEnable 1 to enable reset, 0 to disable.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_RstSnsGpio(CVI_U32 devNo, CVI_U32 rstEnable);

/**
 * @brief Resets the MIPI interface for the specified device number.
 *
 * This function enables or disables the reset functionality for the MIPI interface.
 *
 * @param devNo The device number for the MIPI reset.
 * @param rstEnable 1 to enable reset, 0 to disable.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_RstMipi(CVI_U32 devNo, CVI_U32 rstEnable);

/**
 * @brief Sets the MIPI attributes for the specified video input pipe.
 *
 * This function configures the MIPI settings based on the specified sensor type.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param SnsType The type of sensor for MIPI configuration.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetMipiAttr(VI_PIPE ViPipe, CVI_U32 SnsType);

/**
 * @brief Enables or disables the sensor clock for the specified device number.
 *
 * This function controls the clock signal for the sensor.
 *
 * @param devNo The device number for the clock control.
 * @param clkEnable 1 to enable the clock, 0 to disable.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_EnableSnsClk(CVI_U32 devNo, CVI_U32 clkEnable);

/**
 * @brief Puts the sensor into standby mode for the specified video input pipe.
 *
 * This function sets the sensor to a low-power standby state.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsStandby(VI_PIPE ViPipe);

/**
 * @brief Initializes the sensor for the specified video input pipe.
 *
 * This function performs the necessary initialization steps for the sensor.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsInit(VI_PIPE ViPipe);

/**
 * @brief Sets the flip and mirror callback for the specified video input pipe and device.
 *
 * This function allows the user to register a callback for handling flip and mirror operations.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param ViDev The identifier for the video device.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetVIFlipMirrorCB(VI_PIPE ViPipe, VI_DEV ViDev);

/**
 * @brief Retrieves the default AE settings for the specified video input pipe.
 *
 * This function fetches the default Auto Exposure (AE) settings and stores them in the provided structure.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param stAeDefault Pointer to the structure that will hold the AE default settings.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_GetAeDefault(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *stAeDefault);

/**
 * @brief Retrieves the ISP black level settings for the specified video input pipe.
 *
 * This function fetches the black level settings and stores them in the provided structure.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param stBlc Pointer to the structure that will hold the black level settings.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_GetIspBlkLev(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *stBlc);

/**
 * @brief Sets the frames per second (FPS) for the specified video input pipe.
 *
 * This function configures the sensor's frame rate settings based on the specified FPS value.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param fps The desired frames per second.
 * @param stSnsDft Pointer to the structure containing default AE settings.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetSnsFps(VI_PIPE ViPipe, CVI_U8 fps, AE_SENSOR_DEFAULT_S *stSnsDft);

/**
 * @brief Retrieves the exposure ratio for the specified video input pipe.
 *
 * This function fetches the maximum exposure ratio and stores it in the provided structure.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param stExpMax Pointer to the structure that will hold the exposure ratio.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_GetExpRatio(VI_PIPE ViPipe, SNS_EXP_MAX_S *stExpMax);

/**
 * @brief Sets the digital gain calculation for the specified video input pipe.
 *
 * This function configures the digital gain settings based on the provided structure.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param stDgain Pointer to the structure containing digital gain settings.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetDgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stDgain);

/**
 * @brief Sets the analog gain calculation for the specified video input pipe.
 *
 * This function configures the analog gain settings based on the provided structure.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param stAgain Pointer to the structure containing analog gain settings.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetAgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stAgain);

/**
 * @brief Sets the AHD mode for the specified video input pipe.
 *
 * This function configures the AHD mode settings for the sensor.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param AhdMode The AHD mode to set.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SENSOR_SetAHDMode(VI_PIPE ViPipe, CVI_U32 AhdMode);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__CVI_SENSOR_H__ */
