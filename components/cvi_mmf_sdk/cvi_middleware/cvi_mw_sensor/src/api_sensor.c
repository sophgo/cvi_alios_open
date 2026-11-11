#include "platform_sensor.h"
#include "sensor_cfg.h"

CVI_S32 CVI_SNS_ParseIni(SENSOR_CFG_S *sensor_cfg)
{
	return CVI_SUCCESS;
}

CVI_S32 CVI_SNS_SetIniPath(const CVI_CHAR *iniPath)
{
	return CVI_SUCCESS;
}

CVI_S32 CVI_SNS_SetSnsRxAttr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxAttr)
{
	return platform_sns_SetSnsRxAttr(ViPipe, pstRxAttr);
}

CVI_S32 CVI_SNS_SetSnsI2c(VI_PIPE ViPipe, CVI_S32 astI2cDev, CVI_S32 s32I2cAddr)
{
	return platform_sns_SetSnsI2c(ViPipe, astI2cDev, s32I2cAddr);
}

CVI_S32 CVI_SNS_SetSnsIspAttr(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	return platform_sns_SetSnsIspAttr(ViPipe, pstInitAttr);
}

CVI_S32 CVI_SNS_RegCallback(VI_PIPE ViPipe, ISP_DEV IspDev)
{
	return platform_sns_RegCallback(ViPipe, IspDev);
}

CVI_S32 CVI_SNS_UnRegCallback(VI_PIPE ViPipe, ISP_DEV IspDev)
{
	return platform_sns_UnRegCallback(ViPipe, IspDev);
}

CVI_S32 CVI_SNS_SetSnsImgMode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *stSnsrMode)
{
	return platform_sns_SetSnsImgMode(ViPipe, stSnsrMode);
}

CVI_S32 CVI_SNS_SetSnsWdrMode(VI_PIPE ViPipe, WDR_MODE_E wdrMode)
{
	return platform_sns_SetSnsWdrMode(ViPipe, wdrMode);
}

CVI_S32 CVI_SNS_GetSnsRxAttr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *stDevAttr)
{
	return platform_sns_GetSnsRxAttr(ViPipe, stDevAttr);
}

CVI_S32 CVI_SNS_SetSnsProbe(VI_PIPE ViPipe)
{
	return platform_sns_SetSnsProbe(ViPipe);
}

CVI_S32 CVI_SNS_SetSnsStandby(VI_PIPE ViPipe)
{
	return platform_sns_SetSnsStandby(ViPipe);
}

CVI_S32 CVI_SNS_SetSnsRestart(VI_PIPE ViPipe)
{
	return platform_sns_SetSnsRestart(ViPipe);
}

CVI_S32 CVI_SNS_SetSnsInit(VI_PIPE ViPipe)
{
	return platform_sns_SetSnsInit(ViPipe);
}

CVI_S32 CVI_SNS_SetVIFlipMirrorCB(VI_PIPE ViPipe, VI_DEV ViDev)
{
	return platform_sns_SetVIFlipMirrorCB(ViPipe, ViDev);
}

CVI_S32 CVI_SNS_GetAeDefault(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *stAeDefault)
{
	return platform_sns_GetAeDefault(ViPipe, stAeDefault);
}

CVI_S32 CVI_SNS_GetIspBlkLev(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *stBlc)
{
	return platform_sns_GetIspBlkLev(ViPipe, stBlc);
}

CVI_S32 CVI_SNS_SetSnsFps(VI_PIPE ViPipe, CVI_U8 fps, AE_SENSOR_DEFAULT_S *stSnsDft)
{
	return platform_sns_SetSnsFps(ViPipe, fps, stSnsDft);
}

CVI_S32 CVI_SNS_GetExpRatio(VI_PIPE ViPipe, SNS_EXP_MAX_S *stExpMax)
{
	return platform_sns_GetExpRatio(ViPipe, stExpMax);
}

CVI_S32 CVI_SNS_SetDgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stDgain)
{
	return platform_sns_SetDgainCalc(ViPipe, stDgain);
}

CVI_S32 CVI_SNS_SetAgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stAgain)
{
	return platform_sns_SetAgainCalc(ViPipe, stAgain);
}

CVI_S32 CVI_SNS_GetConfigInfo(SENSOR_CFG_S *sensor_cfg)
{
	return platform_sns_getconfiginfo(sensor_cfg);
}

CVI_S32 CVI_SNS_SetSnsDrvCfg(SENSOR_CFG_S *sensor_cfg)
{
	return platform_sns_setsnsdrvcfg(sensor_cfg);
}

CVI_S32 CVI_SNS_SetAHDMode(VI_PIPE ViPipe, CVI_U32 AhdMode)
{
	return platform_sns_SetAHDMode(ViPipe, AhdMode);
}

CVI_S32 CVI_SNS_GetAhdStatus(SNS_STATUS_MSG_S *pstStatus)
{
	return platform_sns_GetAhdStatus(pstStatus);
}

CVI_S32 CVI_SNS_SetAHDInit(VI_PIPE ViPipe, bool isFirstInit)
{
	return platform_sns_SetAHDInit(ViPipe, isFirstInit);
}

CVI_S32 CVI_SNS_SetAHDDeInit(VI_PIPE ViPipe)
{
	return platform_sns_SetAHDDeInit(ViPipe);
}

CVI_S32 CVI_SNS_GetAHDMode(VI_PIPE ViPipe)
{
	return platform_sns_GetAHDMode(ViPipe);
}

CVI_S32 CVI_SNS_SetAHDBusInfo(VI_PIPE ViPipe, CVI_S32 astI2cDev)
{
	return platform_sns_SetAHDBusInfo(ViPipe, astI2cDev);
}

CVI_S32 CVI_SNS_DetectAhdStatus(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType)
{
	return platform_sns_DetectAhdStatus(ViPipe, ahdOldType, ahdType);
}