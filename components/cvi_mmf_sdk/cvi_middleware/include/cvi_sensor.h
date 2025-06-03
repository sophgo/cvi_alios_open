/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *   MMF Programe Interface for video input management moudle
 */

#ifndef __CVI_VSENSOR_H__
#define __CVI_VSENSOR_H__

#include <stdio.h>
#include "sensor_cfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @brief Parse sensor_cfg.ini to sensor_cfg.
 *
 * This function parses the sensor_cfg.ini file and populates the sensor_cfg structure.
 *
 * @param sensor_cfg Pointer to the sensor_cfg structure.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_ParseIni(SENSOR_CFG_S *sensor_cfg);

/**
 * @brief Sets the sensor configuration for the specified video input pipe.
 *
 * This function configures the sensor settings according to the provided sensor configuration.
 *
 * @param iniPath The path to the sensor configuration file.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetIniPath(const CVI_CHAR *iniPath);

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
CVI_S32 CVI_SNS_SetSnsRxAttr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxAttr);

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
CVI_S32 CVI_SNS_SetSnsI2c(VI_PIPE ViPipe, CVI_S32 astI2cDev, CVI_S32 s32I2cAddr);

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
CVI_S32 CVI_SNS_SetSnsIspAttr(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr);

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
CVI_S32 CVI_SNS_RegCallback(VI_PIPE ViPipe, ISP_DEV IspDev);

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
CVI_S32 CVI_SNS_UnRegCallback(VI_PIPE ViPipe, ISP_DEV IspDev);

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
CVI_S32 CVI_SNS_SetSnsImgMode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *stSnsrMode);

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
CVI_S32 CVI_SNS_SetSnsWdrMode(VI_PIPE ViPipe, WDR_MODE_E wdrMode);

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
CVI_S32 CVI_SNS_GetSnsRxAttr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *stDevAttr);

/**
 * @brief Sets the sensor probe for the specified video input pipe.
 *
 * This function activates the sensor probing mechanism for the given video input pipe.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetSnsProbe(VI_PIPE ViPipe);

/**
 * @brief Puts the sensor into standby mode for the specified video input pipe.
 *
 * This function sets the sensor to a low-power standby state.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetSnsStandby(VI_PIPE ViPipe);

/**
 * @brief Puts the sensor into restart mode for the specified video input pipe.
 *
 * This function sets the sensor to restart state.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetSnsRestart(VI_PIPE ViPipe);


/**
 * @brief Initializes the sensor for the specified video input pipe.
 *
 * This function performs the necessary initialization steps for the sensor.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetSnsInit(VI_PIPE ViPipe);

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
CVI_S32 CVI_SNS_SetVIFlipMirrorCB(VI_PIPE ViPipe, VI_DEV ViDev);

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
CVI_S32 CVI_SNS_GetAeDefault(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *stAeDefault);

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
CVI_S32 CVI_SNS_GetIspBlkLev(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *stBlc);

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
CVI_S32 CVI_SNS_SetSnsFps(VI_PIPE ViPipe, CVI_U8 fps, AE_SENSOR_DEFAULT_S *stSnsDft);

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
CVI_S32 CVI_SNS_GetExpRatio(VI_PIPE ViPipe, SNS_EXP_MAX_S *stExpMax);

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
CVI_S32 CVI_SNS_SetDgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stDgain);

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
CVI_S32 CVI_SNS_SetAgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stAgain);

/**
 * @brief configures the sensor to driver.
 *
 * This function configures the sensor to driver.
 *
 * @param sensor_cfg Pointer to the structure sensor config.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetSnsDrvCfg(SENSOR_CFG_S *sensor_cfg);

/**
 * @brief gets the sensor config info.
 *
 * This function gets the sensor config info.
 *
 * @param sensor_cfg Pointer to the structure sensor config.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_GetConfigInfo(SENSOR_CFG_S *sensor_cfg);

/**
 * @brief Registers a callback for the AHD option.
 *
 * This function allows the user to register a callback function for handling events.
 *
 * @param CB The callback for the ahd trig.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_AHDRegisterDetect(AHD_Callback CB);


/**
 * @brief Enables the AHD detection for the specified video input pipe.
 *
 * This function enables the AHD detection for the given video input pipe.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_EnableDetect(VI_PIPE ViPipe);

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
CVI_S32 CVI_SNS_SetAHDMode(VI_PIPE ViPipe, CVI_U32 AhdMode);

/**
 * @brief Retrieves the AHD status for the specified video input pipe.
 *
 * This function fetches the AHD status and stores it in the provided structure.
 *
 * @param pstStatus Pointer to the structure that will hold the AHD status.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_GetAhdStatus(SNS_STATUS_MSG_S *pstStatus);

/**
 * @brief Initializes the AHD for the specified video input pipe.
 *
 * This function performs the necessary initialization steps for the AHD.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param isFirstInit Indicates whether it's the first initialization.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetAHDInit(VI_PIPE ViPipe, bool isFirstInit);

/**
 * @brief Deinitializes the AHD for the specified video input pipe.
 *
 * This function performs the necessary deinitialization steps for the AHD.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetAHDDeInit(VI_PIPE ViPipe);

/**
 * @brief Retrieves the current AHD mode for the specified video input pipe.
 *
 * This function fetches the current AHD mode and stores it in the provided structure.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns AHD mode.
 */
CVI_S32 CVI_SNS_GetAHDMode(VI_PIPE ViPipe);

/**
 * @brief Sets the AHD communication bus information for the specified video input pipe.
 *
 * This function configures the AHD communication bus information for the sensor.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param astI2cDev The AHD communication bus information.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_SetAHDBusInfo(VI_PIPE ViPipe, CVI_S32 astI2cDev);

/**
 * @brief Detects the AHD status for the specified video input pipe.
 *
 * This function detects the AHD status and stores it in the provided structure.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param ahdOldType The old AHD type.
 * @param ahdType Pointer to the structure that will hold the AHD status.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 CVI_SNS_DetectAhdStatus(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__CVI_SENSOR_H__ */