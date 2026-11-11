#ifndef __PLATFORM_SENSOR_H__
#define __PLATFORM_SENSOR_H__

#include "sensor_cfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

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
CVI_S32 platform_sns_SetSnsRxAttr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxAttr);

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
CVI_S32 platform_sns_SetSnsI2c(VI_PIPE ViPipe, CVI_S32 astI2cDev, CVI_S32 s32I2cAddr);

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
CVI_S32 platform_sns_SetSnsIspAttr(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr);

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
CVI_S32 platform_sns_RegCallback(VI_PIPE ViPipe, ISP_DEV IspDev);

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
CVI_S32 platform_sns_UnRegCallback(VI_PIPE ViPipe, ISP_DEV IspDev);

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
CVI_S32 platform_sns_SetSnsImgMode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *stSnsrMode);

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
CVI_S32 platform_sns_SetSnsWdrMode(VI_PIPE ViPipe, WDR_MODE_E wdrMode);

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
CVI_S32 platform_sns_GetSnsRxAttr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *stDevAttr);

/**
 * @brief Sets the sensor probe for the specified video input pipe.
 *
 * This function activates the sensor probing mechanism for the given video input pipe.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_SetSnsProbe(VI_PIPE ViPipe);

/**
 * @brief Puts the sensor into standby mode for the specified video input pipe.
 *
 * This function sets the sensor to a low-power standby state.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_SetSnsStandby(VI_PIPE ViPipe);

/**
 * @brief Puts the sensor into restart mode for the specified video input pipe.
 *
 * This function sets the sensor to restart state.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_SetSnsRestart(VI_PIPE ViPipe);

/**
 * @brief Initializes the sensor for the specified video input pipe.
 *
 * This function performs the necessary initialization steps for the sensor.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_SetSnsInit(VI_PIPE ViPipe);

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
CVI_S32 platform_sns_SetVIFlipMirrorCB(VI_PIPE ViPipe, VI_DEV ViDev);

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
CVI_S32 platform_sns_GetAeDefault(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *stAeDefault);

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
CVI_S32 platform_sns_GetIspBlkLev(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *stBlc);

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
CVI_S32 platform_sns_SetSnsFps(VI_PIPE ViPipe, CVI_U8 fps, AE_SENSOR_DEFAULT_S *stSnsDft);

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
CVI_S32 platform_sns_GetExpRatio(VI_PIPE ViPipe, SNS_EXP_MAX_S *stExpMax);

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
CVI_S32 platform_sns_SetDgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stDgain);

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
CVI_S32 platform_sns_SetAgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stAgain);

/**
 * @brief configures the sensor to driver.
 *
 * This function configures the sensor to driver.
 *
 * @param sensor_cfg Pointer to the structure sensor config.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_setsnsdrvcfg(SENSOR_CFG_S *sensor_cfg);

/**
 * @brief gets the sensor config info.
 *
 * This function gets the sensor config info.
 *
 * @param sensor_cfg Pointer to the structure sensor config.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_getconfiginfo(SENSOR_CFG_S *sensor_cfg);

/**
 * @brief configures the sensor to detect.
 *
 * This function configures the sensor to detect.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param CB ahd trig callback.
 *
 * @return CVI_VOID Returns VOID.
 */
CVI_VOID platform_AHDRegisterDetect(VI_PIPE ViPipe, AHD_Callback CB);

/**
 * @brief enables the sensor detect.
 *
 * This function enables the sensor detect.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_EnableDetect(VI_PIPE ViPipe);

/**
 * @brief get sensor ahd mode.
 *
 * This function get sensor ahd mode.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param AhdMode The AHD mode to set.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_SetAHDMode(VI_PIPE ViPipe, CVI_U32 AhdMode);

/**
 * @brief get sensor ahd status.
 *
 * This function get sensor ahd status.
 *
 * @param pstStatus Pointer to the structure sensor status.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_GetAhdStatus(SNS_STATUS_MSG_S *pstStatus);

/**
 * @brief get sensor ahd init.
 *
 * This function get sensor ahd init.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param isFirstInit The flag to indicate if this is the first initialization.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_SetAHDInit(VI_PIPE ViPipe, bool isFirstInit);

/**
 * @brief get sensor ahd deinit.
 *
 * This function get sensor ahd deinit.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_SetAHDDeInit(VI_PIPE ViPipe);

/**
 * @brief get sensor ahd mode.
 *
 * This function get sensor ahd mode.
 *
 * @param ViPipe The identifier for the video input pipe.
 *
 * @return CVI_S32 Returns AHD mode.
 */
CVI_S32 platform_sns_GetAHDMode(VI_PIPE ViPipe);

/**
 * @brief set sensor ahd bus info.
 *
 * This function set sensor ahd bus info.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param astI2cDev The I2C device identifier.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_SetAHDBusInfo(VI_PIPE ViPipe, CVI_S32 astI2cDev);

/**
 * @brief detect ahd status.
 *
 * This function detect ahd status.
 *
 * @param ViPipe The identifier for the video input pipe.
 * @param ahdOldType The old AHD type.
 * @param ahdType The AHD type to set.
 *
 * @return CVI_S32 Returns 0 on success, or a negative error code on failure.
 */
CVI_S32 platform_sns_DetectAhdStatus(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif