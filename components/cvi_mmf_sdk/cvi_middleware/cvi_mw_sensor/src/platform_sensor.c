#include "sensor_cfg.h"
#include "cvi_vi.h"
#include "cvi_awb_comm.h"
#include "cvi_ae_comm.h"

#define MAX_SNSR_NUM VI_MAX_DEV_NUM

static ISP_SNS_OBJ_S *pstSnsObj[MAX_SNSR_NUM];
static ISP_SENSOR_EXP_FUNC_S *stSnsrSensorFunc[MAX_SNSR_NUM];
static AE_SENSOR_EXP_FUNC_S *stSensorExpFunc[MAX_SNSR_NUM];
static SNS_AHD_OBJ_S *stSnsAhdObj[MAX_SNSR_NUM];

CVI_S32 platform_sns_SetSnsRxAttr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (pstSnsObj[ViPipe]->pfnPatchRxAttr) ?
		pstSnsObj[ViPipe]->pfnPatchRxAttr(ViPipe, pstRxAttr) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("pfnPatchRxAttr callback failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsI2c(VI_PIPE ViPipe, CVI_S32 astI2cDev, CVI_S32 s32I2cAddr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	ISP_SNS_COMMBUS_U unSnsrBusInfo = {
		.s8I2cDev = -1,
	};

	unSnsrBusInfo.s8I2cDev = (CVI_S8)astI2cDev;

	if (pstSnsObj[ViPipe]->pfnPatchI2cAddr) {
		pstSnsObj[ViPipe]->pfnPatchI2cAddr(ViPipe, s32I2cAddr);
	} else {
		SNS_DBG_PRT("sensor can not set i2c addr :%d Failed : %x!\n", s32I2cAddr, s32Ret);
		s32Ret = CVI_SUCCESS;
	}
	if ((pstSnsObj[ViPipe]->pfnSetBusInfo) && (unSnsrBusInfo.s8I2cDev != -1)) {
		s32Ret = pstSnsObj[ViPipe]->pfnSetBusInfo(ViPipe, unSnsrBusInfo);
	} else {
		SNS_DBG_PRT("sensor can not set i2c busid :%d Failed : %x!\n", astI2cDev, s32Ret);
		s32Ret = CVI_SUCCESS;
	}

	return s32Ret;
}
CVI_S32 platform_sns_SetSnsIspAttr(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (pstSnsObj[ViPipe]->pfnSetInit) ? pstSnsObj[ViPipe]->pfnSetInit(ViPipe, pstInitAttr) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetIspAttr callback failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_RegCallback(VI_PIPE ViPipe, ISP_DEV IspDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	ALG_LIB_S stAeLib;
	ALG_LIB_S stAwbLib;
	stAeLib.s32Id = IspDev;
	stAwbLib.s32Id = IspDev;

	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	//  strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(CVI_AF_LIB_NAME));

	s32Ret = (pstSnsObj[ViPipe]->pfnRegisterCallback) ?
		pstSnsObj[ViPipe]->pfnRegisterCallback(ViPipe, &stAeLib, &stAwbLib) : CVI_SUCCESS;
	if (s32Ret != 0) {
		SNS_DBG_PRT("platform_sns_RegCallback Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_UnRegCallback(VI_PIPE ViPipe, ISP_DEV IspDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	ALG_LIB_S stAeLib;
	ALG_LIB_S stAwbLib;
	stAeLib.s32Id = IspDev;
	stAwbLib.s32Id = IspDev;

	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	//  strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(CVI_AF_LIB_NAME));

	s32Ret = (pstSnsObj[ViPipe]->pfnUnRegisterCallback) ?
		pstSnsObj[ViPipe]->pfnUnRegisterCallback(ViPipe, &stAeLib, &stAwbLib) : CVI_SUCCESS;
	if (s32Ret != 0) {
		SNS_DBG_PRT("platform_sns_UnRegCallback Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsImgMode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *stSnsrMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSnsrSensorFunc[ViPipe]->pfn_cmos_set_image_mode) ?
		stSnsrSensorFunc[ViPipe]->pfn_cmos_set_image_mode(ViPipe, stSnsrMode) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetSnsImgMode callback failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsWdrMode(VI_PIPE ViPipe, WDR_MODE_E wdrMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSnsrSensorFunc[ViPipe]->pfn_cmos_set_wdr_mode) ?
		stSnsrSensorFunc[ViPipe]->pfn_cmos_set_wdr_mode(ViPipe, wdrMode) : CVI_SUCCESS;
	if (s32Ret != 0) {
		SNS_DBG_PRT("platform_sns_SetSnsWdrMode Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetSnsRxAttr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *stDevAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (pstSnsObj[ViPipe]->pfnGetRxAttr) ?
		pstSnsObj[ViPipe]->pfnGetRxAttr(ViPipe, stDevAttr) : CVI_SUCCESS;
	if (s32Ret != 0) {
		SNS_DBG_PRT("platform_sns_GetSnsRxAttr Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsProbe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (pstSnsObj[ViPipe]->pfnSnsProbe) ?
		pstSnsObj[ViPipe]->pfnSnsProbe(ViPipe) : CVI_SUCCESS;
	if (s32Ret != 0) {
		SNS_DBG_PRT("platform_sns_SetSnsProbe Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsStandby(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pstSnsObj[ViPipe]->pfnStandby) {
		pstSnsObj[ViPipe]->pfnStandby(ViPipe);
	} else {
		SNS_DBG_PRT("sensor no standby func !\n");
		s32Ret = CVI_SUCCESS;
	}
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetSnsStandby Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsRestart(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pstSnsObj[ViPipe]->pfnRestart) {
		pstSnsObj[ViPipe]->pfnRestart(ViPipe);
	} else {
		SNS_DBG_PRT("sensor no Restart func !\n");
		s32Ret = CVI_SUCCESS;
	}
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetSnsRestart Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsInit(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (stSnsrSensorFunc[ViPipe]->pfn_cmos_sensor_init) {
		stSnsrSensorFunc[ViPipe]->pfn_cmos_sensor_init(ViPipe);
	} else {
		SNS_DBG_PRT("sensor no set Isp Init func !\n");
		s32Ret = CVI_SUCCESS;
	}
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetSnsInit Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetVIFlipMirrorCB(VI_PIPE ViPipe, VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pstSnsObj[ViPipe]->pfnMirrorFlip) {
		CVI_VI_RegChnFlipMirrorCallBack(ViPipe, ViDev, (void *)pstSnsObj[ViPipe]->pfnMirrorFlip);
	}
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetVIFlipMirrorCB Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetAeDefault(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *stAeDefault)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSensorExpFunc[ViPipe]->pfn_cmos_get_ae_default) ?
		stSensorExpFunc[ViPipe]->pfn_cmos_get_ae_default(ViPipe, stAeDefault) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_GetAeDefault Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetIspBlkLev(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *stBlc)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSnsrSensorFunc[ViPipe]->pfn_cmos_get_isp_black_level) ?
		stSnsrSensorFunc[ViPipe]->pfn_cmos_get_isp_black_level(ViPipe, stBlc) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_GetIspBlkLev Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsFps(VI_PIPE ViPipe, CVI_U8 fps, AE_SENSOR_DEFAULT_S *stSnsDft)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSensorExpFunc[ViPipe]->pfn_cmos_fps_set) ?
		stSensorExpFunc[ViPipe]->pfn_cmos_fps_set(ViPipe, fps, stSnsDft) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetSnsFps Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetExpRatio(VI_PIPE ViPipe, SNS_EXP_MAX_S *stExpMax)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSensorExpFunc[ViPipe]->pfn_cmos_get_inttime_max) ?
		stSensorExpFunc[ViPipe]->pfn_cmos_get_inttime_max(ViPipe, stExpMax->manual, stExpMax->ratio,
			stExpMax->IntTimeMax, stExpMax->IntTimeMin, stExpMax->LFMaxIntTime) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_GetExpRatio Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetDgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stDgain)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSensorExpFunc[ViPipe]->pfn_cmos_dgain_calc_table) ?
		stSensorExpFunc[ViPipe]->pfn_cmos_dgain_calc_table(ViPipe, &(stDgain->gain), &(stDgain->gainDb)) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetDgainCalc Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetAgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stAgain)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSensorExpFunc[ViPipe]->pfn_cmos_again_calc_table) ?
		stSensorExpFunc[ViPipe]->pfn_cmos_again_calc_table(ViPipe, &(stAgain->gain), &(stAgain->gainDb)) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetAgainCalc Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_getconfiginfo(SENSOR_CFG_S *sensor_cfg)
{
	int i;
	if (sensor_cfg == CVI_NULL) {
		SNS_DBG_PRT("sensor_cfg is NULL point\n");
		return CVI_FAILURE;
	}

	if (sensor_cfg->sns_ini_cfg.enSnsType[0] == CVI_SNS_TYPE_BUTT) {
		SNS_DBG_PRT("Need call parse ini first\n");
		return CVI_FAILURE;
	}

	for (i = 0; i < sensor_cfg->sns_ini_cfg.devNum; i++) {
		if (cvi_sns_getsize(i, sensor_cfg)) {
			SNS_DBG_PRT("cvi_sns_getsize failed!\n");
			return CVI_FAILURE;
		}
		if (cvi_sns_getdevattr(i, sensor_cfg)) {
			SNS_DBG_PRT("cvi_sns_getdevattr failed!\n");
			return CVI_FAILURE;
		}
		if (cvi_sns_getyuvbypass(i, sensor_cfg)) {
			SNS_DBG_PRT("cvi_sns_getyuvbypass failed!\n");
			return CVI_FAILURE;
		}
		if (cvi_sns_getispattr(i, sensor_cfg)) {
			SNS_DBG_PRT("cvi_sns_getispattr failed!\n");
			return CVI_FAILURE;
		}
		if (cvi_sns_getsnsobj(i, sensor_cfg)) {
			SNS_DBG_PRT("cvi_sns_getsnsobj failed!\n");
			return CVI_FAILURE;
		}
		if (cvi_sns_getsnswdrattr(i, sensor_cfg)) {
			SNS_DBG_PRT("cvi_sns_getsnswdrattr failed!\n");
			return CVI_FAILURE;
		}
		if (sensor_cfg->sns_cfg.pstSensorExpFunc[i]) {
			stSnsrSensorFunc[i] = sensor_cfg->sns_cfg.pstSensorExpFunc[i];
		} else {
			stSnsrSensorFunc[i] = NULL;
			SNS_DBG_PRT("set sensor func failed in because stSensorFunc is NULL!\n");
		}
		if (sensor_cfg->sns_cfg.pstExpFuncs[i]) {
			stSensorExpFunc[i] = sensor_cfg->sns_cfg.pstExpFuncs[i];
		} else {
			stSensorExpFunc[i] = NULL;
			SNS_DBG_PRT("set sensor exp func failed in because stSensorExpFunc is NULL!\n");
		}
		if (sensor_cfg->sns_cfg.pstAhdFuncs[i]) {
			stSnsAhdObj[i] = sensor_cfg->sns_cfg.pstAhdFuncs[i];
		} else {
			stSnsAhdObj[i] = NULL;
		}
		if (sensor_cfg->sns_cfg.pstSnsObj[i]) {
			pstSnsObj[i] = sensor_cfg->sns_cfg.pstSnsObj[i];
		} else {
			pstSnsObj[i] = NULL;
			SNS_DBG_PRT("set sensor obj failed in because pstSnsObj is NULL!\n");
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sns_setsnsdrvcfg(SENSOR_CFG_S *sensor_cfg)
{
	int i;
	CVI_S32 lane_num;
	CVI_S32 s32Ret = CVI_SUCCESS;
	RX_INIT_ATTR_S stRxInitAttr;
	ISP_SNS_COMMBUS_U unSnsrBusInfo;
	ALG_LIB_S stAeLib;
	ALG_LIB_S stAwbLib;
	ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode;

	if (sensor_cfg == CVI_NULL) {
		SNS_DBG_PRT("input sensor_cfg is NULL !!\n");
		return CVI_FAILURE;
	}
	ISP_INIT_ATTR_S *pstInitAttr = &sensor_cfg->pstInitAttr;
	SNS_INI_CFG_S *sns_ini_cfg = &sensor_cfg->sns_ini_cfg;
	SNS_CFG_S *sns_cfg = &sensor_cfg->sns_cfg;

	if (sns_ini_cfg->enSnsType[0] == CVI_SNS_TYPE_BUTT) {
		SNS_DBG_PRT("Need call parse ini first\n");
		return CVI_FAILURE;
	}

	for (i = 0; i < sns_ini_cfg->devNum; i++) {
		if (sns_cfg->pstSnsObj[i] == CVI_NULL) {
			SNS_DBG_PRT("dev_%d object is NULL\n", i);
			continue;
		}

		memset(&stRxInitAttr, 0, sizeof(RX_INIT_ATTR_S));
		stRxInitAttr.MipiDev = sns_ini_cfg->MipiDev[i];
		if (sns_ini_cfg->bHsettlen[i]) {
			stRxInitAttr.hsettlen = CVI_TRUE;
			stRxInitAttr.hsettle = sns_ini_cfg->u8Hsettle[i];
		}
		if (sns_ini_cfg->stMclkAttr[i].bMclkEn) {
			stRxInitAttr.stMclkAttr.bMclkEn = CVI_TRUE;
			stRxInitAttr.stMclkAttr.u8Mclk = sns_ini_cfg->stMclkAttr[i].u8Mclk;
		}
		stRxInitAttr.MipiMode = sns_ini_cfg->enSnsMode;
		for (lane_num = 0; lane_num < MIPI_LANE_NUM; lane_num++) {
			stRxInitAttr.as16LaneId[lane_num] = sns_ini_cfg->as16LaneId[i][lane_num];
		}
		for (lane_num = 0; lane_num < TTL_PIN_FUNC_NUM; lane_num++) {
			stRxInitAttr.as16FuncId[lane_num] = sns_ini_cfg->as16FuncId[i][lane_num];
		}
		for (lane_num = 0; lane_num < MIPI_LANE_NUM; lane_num++) {
			stRxInitAttr.as8PNSwap[lane_num] = sns_ini_cfg->as8PNSwap[i][lane_num];
		}
		if (sns_cfg->pstSnsObj[i]->pfnPatchRxAttr == CVI_NULL) {
			SNS_DBG_PRT("dev_%d patch RX_ATTR func NULL\n", i);
		} else {
			s32Ret = sns_cfg->pstSnsObj[i]->pfnPatchRxAttr(i, &stRxInitAttr);
			if (s32Ret == CVI_FAILURE) {
				SNS_DBG_PRT("dev_%d patch RX_ATTR failed\n", i);
				continue;
			}
		}

		switch (sns_cfg->enChnMode[i]) {
		case SNS_CHN_MODE_2Multiplex:
			pstInitAttr->enSnsBdgMuxMode = SNS_BDG_MUX_2;
			break;
		case SNS_CHN_MODE_3Multiplex:
			pstInitAttr->enSnsBdgMuxMode = SNS_BDG_MUX_3;
			break;
		case SNS_CHN_MODE_4Multiplex:
			pstInitAttr->enSnsBdgMuxMode = SNS_BDG_MUX_4;
			break;
		case SNS_CHN_MODE_1Multiplex:
		default:
			pstInitAttr->enSnsBdgMuxMode = SNS_BDG_MUX_NONE;
			break;
		}
		if (sns_cfg->pstSnsObj[i]->pfnSetInit == CVI_NULL) {
			SNS_DBG_PRT("dev_%d init sensor param func NULL\n", i);
		} else {
			s32Ret = sns_cfg->pstSnsObj[i]->pfnSetInit(i, pstInitAttr);
			if (s32Ret == CVI_FAILURE) {
				SNS_DBG_PRT("dev_%d init sensor param failed\n", i);
				continue;
			}
		}

		memset(&unSnsrBusInfo, 0, sizeof(ISP_SNS_COMMBUS_U));
		unSnsrBusInfo.s8I2cDev = (CVI_S8)sns_ini_cfg->s32BusId[i];
		if (sns_cfg->pstSnsObj[i]->pfnSetBusInfo == CVI_NULL) {
			SNS_DBG_PRT("dev_%d set sensor I2C bus func NULL\n", i);
		} else {
			s32Ret = sns_cfg->pstSnsObj[i]->pfnSetBusInfo(i, unSnsrBusInfo);
			if (s32Ret == CVI_FAILURE) {
				SNS_DBG_PRT("dev_%d set sensor I2C bus failed\n", i);
				continue;
			}
		}

		if (sns_cfg->pstSnsObj[i]->pfnPatchI2cAddr == CVI_NULL) {
			SNS_DBG_PRT("dev_%d set sensor I2C addr func NULL\n", i);
		} else {
			sns_cfg->pstSnsObj[i]->pfnPatchI2cAddr(i, sns_ini_cfg->s32SnsI2cAddr[i]);
		}

		memset(&stAeLib, 0, sizeof(ALG_LIB_S));
		memset(&stAwbLib, 0, sizeof(ALG_LIB_S));
		stAeLib.s32Id = i;
		stAwbLib.s32Id = i;
		strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
		strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
		if (sns_cfg->pstSnsObj[i]->pfnRegisterCallback == CVI_NULL) {
			SNS_DBG_PRT("dev_%d set ISP reg callback func NULL\n", i);
		} else {
			s32Ret = sns_cfg->pstSnsObj[i]->pfnRegisterCallback(i, &stAeLib, &stAwbLib);
			if (s32Ret == CVI_FAILURE) {
				SNS_DBG_PRT("dev_%d set ISP reg callback failed\n", i);
				continue;
			}
		}

		memset(&stSnsrMode, 0, sizeof(ISP_CMOS_SENSOR_IMAGE_MODE_S));
		stSnsrMode.u16Width = sns_cfg->u32ImageWigth[i];
		stSnsrMode.u16Height = sns_cfg->u32ImageHeight[i];
		stSnsrMode.f32Fps = sns_cfg->f32FrameRate[i];
		stSnsrMode.u8LaneNum = sns_cfg->u8LaneNumber[i];
		stSnsrMode.u8EnableMaster = sns_cfg->u8EnMasterMode[i];
		if (sns_cfg->pstSensorExpFunc[i] == CVI_NULL) {
			SNS_DBG_PRT("dev_%d link to sensor exp ctrl func NULL\n", i);
		} else {
			if (sns_cfg->pstSensorExpFunc[i]->pfn_cmos_set_image_mode) {
				s32Ret = sns_cfg->pstSensorExpFunc[i]->pfn_cmos_set_image_mode(i, &stSnsrMode);
				if (s32Ret != CVI_SUCCESS) {
					SNS_DBG_PRT("dev_%d set sensor image mode failed\n", i);
					continue;
				}
			}
			if (sns_cfg->pstSensorExpFunc[i]->pfn_cmos_set_wdr_mode) {
				s32Ret = sns_cfg->pstSensorExpFunc[i]->pfn_cmos_set_wdr_mode(i, sns_cfg->enWDRMode[i]);
				if (s32Ret != CVI_SUCCESS) {
					SNS_DBG_PRT("dev_%d set sensor image wdr mode failed\n", i);
					continue;
				}
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sns_SetAHDMode(VI_PIPE ViPipe, CVI_U32 AhdMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSnsAhdObj[ViPipe]->pfnSetAhdMode) ?
		stSnsAhdObj[ViPipe]->pfnSetAhdMode(ViPipe, AhdMode) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetAHDMode Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetAhdStatus(SNS_STATUS_MSG_S *pstStatus)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((pstStatus->s32SnsId < 0) || (pstStatus->s32SnsId >= VI_MAX_PIPE_NUM)) {
		SNS_DBG_PRT("ViPipe %d value error\n", pstStatus->s32SnsId);
		return CVI_FAILURE;
	}

	pstStatus->s32Status = 0;
	pstStatus->eMode = -1;

	VI_PIPE ViPipe = pstStatus->s32SnsId;
	// CVI_S32 mode = -1;
	if (!stSnsAhdObj[ViPipe]) {
		SNS_DBG_PRT("fail to get Ahd obj !!!");
		pstStatus->s32Status = 1;
		pstStatus->eMode = -1;
		return s32Ret;
		// return CVI_FAILURE;
	}
	// stSnsAhdObj[ViPipe]->pfnAhdInit(ViPipe, false);

	stSnsAhdObj[ViPipe]->pfnGetAhdMode(ViPipe);  //skip first catch
	pstStatus->eMode = stSnsAhdObj[ViPipe]->pfnGetAhdMode(ViPipe);
	SNS_DBG_PRT("get ahd mode:%d\n", pstStatus->eMode);
	if (pstStatus->eMode != 0){
		pstStatus->s32Status = 1;
		stSnsAhdObj[ViPipe]->pfnSetAhdMode(ViPipe, pstStatus->eMode);  //load current mode setting
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetAHDInit(VI_PIPE ViPipe, bool isFirstInit)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSnsAhdObj[ViPipe]->pfnAhdInit) ?
		stSnsAhdObj[ViPipe]->pfnAhdInit(ViPipe, isFirstInit) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetAHDInit Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetAHDDeInit(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSnsAhdObj[ViPipe]->pfnAhdDeinit) ?
		stSnsAhdObj[ViPipe]->pfnAhdDeinit(ViPipe) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetAHDDeInit Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetAHDMode(VI_PIPE ViPipe)
{
	CVI_S32 Ahdmode = AHD_MODE_NONE;

	Ahdmode = (stSnsAhdObj[ViPipe]->pfnGetAhdMode) ?
		stSnsAhdObj[ViPipe]->pfnGetAhdMode(ViPipe) : AHD_MODE_NONE;

	return Ahdmode;
}

CVI_S32 platform_sns_SetAHDBusInfo(VI_PIPE ViPipe, CVI_S32 astI2cDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSnsAhdObj[ViPipe]->pfnSetAhdBusInfo) ?
		stSnsAhdObj[ViPipe]->pfnSetAhdBusInfo(ViPipe, astI2cDev) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_SetAHDBusInfo Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_sns_DetectAhdStatus(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = (stSnsAhdObj[ViPipe]->pfnDetectAhdStatus) ?
		stSnsAhdObj[ViPipe]->pfnDetectAhdStatus(ViPipe, ahdOldType, ahdType) : CVI_SUCCESS;
	if (s32Ret != CVI_SUCCESS) {
		SNS_DBG_PRT("platform_sns_DetectAhdStatus Failed : %#x!\n", s32Ret);
	}

	return s32Ret;
}