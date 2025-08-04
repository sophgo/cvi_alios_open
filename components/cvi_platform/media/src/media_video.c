#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <aos/cli.h>
#include "media_video.h"
#include "rtos_types.h"
#include "cvi_common.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_vi.h"
#include "sys_uapi.h"
#include "vi_uapi.h"
#include "vpss_uapi.h"
#include "vo_uapi.h"
#include "rgn_uapi.h"
#include "ldc_uapi.h"
#if CONFIG_SUPPORT_VI
#include "sensor_cfg.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_comm_cif.h"
#include "cvi_sns_ctrl.h"
#include "vi_isp.h"
#include "vi_snsr_i2c.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_awb.h"
#include "cvi_ae.h"
#endif
#include "cvi_type.h"
#include "cvi_param.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_buffer.h"
#include "cvi_math.h"
#include "cvi_mipi_tx.h"
#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
#include "dsi_panels.h"
#endif
//#include "ldc_platform.h"
#include "cvi_gdc.h"
#include "cvi_vo.h"
#include "cvi_venc.h"
#include "cvi_region.h"
#include "cvi_bin.h"
#include "drv/common.h"

#if CONFIG_DUALOS_NO_CROP
#include "app_ai.h"
#include "drv/efuse.h"
#include "cvi_efuse.h"
#endif //#if CONFIG_DUALOS_NO_CROP

#include "cvi_misc.h"
#include "cvi_sys.h"

static PARAM_VENC_CFG_S *g_pstVencCfg = NULL;
int MEDIA_VIDEO_Init(CVI_BOOL isRawReplayMode);
int MEDIA_VIDEO_Deinit(CVI_BOOL isRawReplayMode);

#if 0
CVI_S32 _getFileSize(FILE *fp, CVI_U32 *size)
{
	CVI_S32 ret = CVI_SUCCESS;

	fseek(fp, 0L, SEEK_END);
	*size = ftell(fp);
	rewind(fp);

	return ret;
}

void * load_para_from_bin(void *data)
{
	CVI_S32 ret = CVI_SUCCESS;
	FILE *fp = NULL;
	CVI_U8 *buf = NULL;
	CVI_CHAR binName[BIN_FILE_LENGTH] = { 0 };
	CVI_U32 u32TempLen = 0, u32FileSize = 0;
	enum CVI_BIN_SECTION_ID id = (enum CVI_BIN_SECTION_ID)data;

	CVI_BIN_GetBinName(binName);
	fp = fopen((const CVI_CHAR *)binName, "rb");
	if (fp == NULL) {
		MEDIABUG_PRINTF("Can't find bin(%s)\n", binName);
		ret = CVI_FAILURE;
		goto ERROR_HANDLER;
	}
	_getFileSize(fp, &u32FileSize);

	buf = (CVI_U8 *)malloc(u32FileSize);
	if (buf == NULL) {
		ret = CVI_FAILURE;
		MEDIABUG_PRINTF("Allocate memory fail\n");
		goto ERROR_HANDLER;
	}
	u32TempLen = fread(buf, u32FileSize, 1, fp);
	if (u32TempLen <= 0) {
		MEDIABUG_PRINTF("read data to buff fail!\n");
		ret = CVI_FAILURE;
		goto ERROR_HANDLER;
	}

	if (id >= CVI_BIN_ID_ISP0 && id <= CVI_BIN_ID_ISP3) {
		CVI_BIN_LoadParamFromBin(CVI_BIN_ID_HEADER, buf);
	}
	ret = CVI_BIN_LoadParamFromBin(id, buf);
	if (ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("id[%d] Bin Version not match, use default parameters\n", id);
	}

ERROR_HANDLER:
	if (fp != NULL) {
		fclose(fp);
	}
	if (buf != NULL) {
		free(buf);
	}


	return 0;
}
#endif
#if CONFIG_SUPPORT_VI
static int start_isp(ISP_PUB_ATTR_S stPubAttr, ISP_FUNC_FROM_OUT_S stISP_FunctionFromOut, VI_PIPE ViPipe)
{
	//Param init
	CVI_S32 s32Ret;
	ISP_STATISTICS_CFG_S stsCfg = {0};
	CVI_ISP_GetStatisticsConfig(ViPipe, &stsCfg);
	stsCfg.stAECfg.stCrop[0].bEnable = 0;
	stsCfg.stAECfg.stCrop[0].u16X = stsCfg.stAECfg.stCrop[0].u16Y = 0;
	stsCfg.stAECfg.stCrop[0].u16W = stPubAttr.stWndRect.u32Width;
	stsCfg.stAECfg.stCrop[0].u16H = stPubAttr.stWndRect.u32Height;
	memset(stsCfg.stAECfg.au8Weight, 1, AE_WEIGHT_ZONE_ROW * AE_WEIGHT_ZONE_COLUMN * sizeof(CVI_U8));
	stsCfg.stWBCfg.u16ZoneRow = AWB_ZONE_ORIG_ROW;
	stsCfg.stWBCfg.u16ZoneCol = AWB_ZONE_ORIG_COLUMN;
	stsCfg.stWBCfg.stCrop.bEnable = 0;
	stsCfg.stWBCfg.stCrop.u16X = stsCfg.stWBCfg.stCrop.u16Y = 0;
	stsCfg.stWBCfg.stCrop.u16W = stPubAttr.stWndRect.u32Width;
	stsCfg.stWBCfg.stCrop.u16H = stPubAttr.stWndRect.u32Height;
	stsCfg.stWBCfg.u16BlackLevel = 0;
	stsCfg.stWBCfg.u16WhiteLevel = 4095;
	stsCfg.stFocusCfg.stConfig.bEnable = 1;
	stsCfg.stFocusCfg.stConfig.u8HFltShift = 1;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[0] = 1;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[1] = 2;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[2] = 3;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[3] = 5;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[4] = 10;
	stsCfg.stFocusCfg.stConfig.stRawCfg.PreGammaEn = 0;
	stsCfg.stFocusCfg.stConfig.stPreFltCfg.PreFltEn = 1;
	stsCfg.stFocusCfg.stConfig.u16Hwnd = 17;
	stsCfg.stFocusCfg.stConfig.u16Vwnd = 15;
	stsCfg.stFocusCfg.stConfig.stCrop.bEnable = 0;
	// AF offset and size has some limitation.
	stsCfg.stFocusCfg.stConfig.stCrop.u16X = AF_XOFFSET_MIN;
	stsCfg.stFocusCfg.stConfig.stCrop.u16Y = AF_YOFFSET_MIN;
	stsCfg.stFocusCfg.stConfig.stCrop.u16W = stPubAttr.stWndRect.u32Width - AF_XOFFSET_MIN * 2;
	stsCfg.stFocusCfg.stConfig.stCrop.u16H = stPubAttr.stWndRect.u32Height - AF_YOFFSET_MIN * 2;
	//Horizontal HP0
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[0] = 0;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[1] = 0;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[2] = 13;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[3] = 24;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[4] = 0;
	//Horizontal HP1
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[0] = 1;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[1] = 2;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[2] = 4;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[3] = 8;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[4] = 0;
	//Vertical HP
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[0] = 13;
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[1] = 24;
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[2] = 0;
	stsCfg.unKey.bit1FEAeGloStat = stsCfg.unKey.bit1FEAeLocStat =
	stsCfg.unKey.bit1AwbStat1 = stsCfg.unKey.bit1AwbStat2 = stsCfg.unKey.bit1FEAfStat = 1;
	//LDG
	stsCfg.stFocusCfg.stConfig.u8ThLow = 0;
	stsCfg.stFocusCfg.stConfig.u8ThHigh = 255;
	stsCfg.stFocusCfg.stConfig.u8GainLow = 30;
	stsCfg.stFocusCfg.stConfig.u8GainHigh = 20;
	stsCfg.stFocusCfg.stConfig.u8SlopLow = 8;
	stsCfg.stFocusCfg.stConfig.u8SlopHigh = 15;
	stsCfg.stFocusCfg.stConfig.u16HighLumaTh = 230;
	//Register callback & call API
	ALG_LIB_S stAeLib, stAwbLib;
	stAeLib.s32Id = stAwbLib.s32Id = ViPipe;
	ISP_BIND_ATTR_S stBindAttr;
	stBindAttr.stAeLib.s32Id = stBindAttr.stAwbLib.s32Id = ViPipe;
	CVI_ISP_Register(stISP_FunctionFromOut);

	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
	strncpy(stBindAttr.stAeLib.acLibName, CVI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
	strncpy(stBindAttr.stAwbLib.acLibName, CVI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
	s32Ret = CVI_AE_Register(ViPipe, &stAeLib);
	if (s32Ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("AE Algo register failed!, error: %d\n",	s32Ret);
		return s32Ret;
	}
	s32Ret = CVI_AWB_Register(ViPipe, &stAwbLib);
	if (s32Ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("AWB Algo register failed!, error: %d\n", s32Ret);
		return s32Ret;
	}
	s32Ret = CVI_ISP_SetBindAttr(ViPipe, &stBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("Bind Algo failed with %d!\n", s32Ret);
	}
	s32Ret = CVI_ISP_MemInit(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("Init Ext memory failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	s32Ret = CVI_ISP_SetPubAttr(ViPipe, &stPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("SetPubAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	s32Ret = CVI_ISP_SetStatisticsConfig(ViPipe, &stsCfg);
	if (s32Ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("ISP Set Statistic failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	s32Ret = CVI_ISP_Init(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("ISP Init failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	//Run ISP
	s32Ret = CVI_ISP_Run(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		MEDIABUG_PRINTF("ISP Run failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	MEDIABUG_PRINTF("******start isp******\n");
	return CVI_SUCCESS;
}

static int stop_isp(VI_PIPE ViPipe, CVI_U8 devNum)
{
    //Param init
    CVI_S32 s32Ret;
    ALG_LIB_S stAeLib, stAwbLib;
    stAeLib.s32Id = stAwbLib.s32Id = ViPipe;
    strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
    strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
    //Stop ISP
    s32Ret = CVI_ISP_Exit(ViPipe);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("ISP Exit failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    CVI_S32 snsr_type[MAX_SENSOR_NUM];
    ISP_SNS_OBJ_S *pSnsObj;
    MEDIA_CHECK_RET(getSnsType(snsr_type, devNum), "getSnsType fail");
    pSnsObj = getSnsObj(snsr_type[ViPipe]);
    MEDIA_CHECK_RET( pSnsObj->pfnUnRegisterCallback(ViPipe, &stAeLib, &stAwbLib), "sensor_unregister_callback failed");

    s32Ret = CVI_AE_UnRegister(ViPipe, &stAeLib);
    if (s32Ret) {
        MEDIABUG_PRINTF("AE Algo unRegister failed!, error: %d\n", s32Ret);
        return s32Ret;
    }
    s32Ret = CVI_AWB_UnRegister(ViPipe, &stAwbLib);
    if (s32Ret) {
        MEDIABUG_PRINTF("AWB Algo unRegister failed!, error: %d\n", s32Ret);
        return s32Ret;
    }
    MEDIABUG_PRINTF("******stop isp******\n");

    return CVI_SUCCESS;
}

static int _meida_sensor_init(PARAM_VI_CFG_S * pstViCtx,CVI_U8 devNum)
{
    //Sensor
    SNS_COMBO_DEV_ATTR_S devAttr = {0};
    CVI_S32 snsr_type[MAX_SENSOR_NUM];
    ISP_SNS_OBJ_S *pSnsObj[MAX_SENSOR_NUM];
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode[MAX_SENSOR_NUM];
    ISP_SNS_COMMBUS_U unSnsrBusInfo = {0};
    ISP_SENSOR_EXP_FUNC_S stSnsrSensorFunc[MAX_SENSOR_NUM] = {0};
    ISP_INIT_ATTR_S InitAttr = {0};
    ALG_LIB_S stAeLib = {0};
    ALG_LIB_S stAwbLib = {0};
    CVI_S32 s32Ret;
    struct snsr_rst_gpio_s snsr_gpio;
    CVI_U8 ViPipe;
    PARAM_SYS_CFG_S * pstSysCtx = PARAM_getSysCtx();

    if ((pstViCtx == NULL) || (pstSysCtx == NULL)) {
        MEDIABUG_PRINTF("pstViCfg/pstSysCtx null err\n");
        return -1;
    }

    MEDIA_CHECK_RET(getSnsType(snsr_type, devNum), "getSnsType fail");
    for (CVI_U8 i = 0; i < devNum; i++) {
        pSnsObj[i] = getSnsObj(snsr_type[i]);
        MEDIA_CHECK_RET(getSnsMode(i, &stSnsrMode[i]), "stSnsrMode fail");
    }

    /* clock enable */
    // vip_clk_en();
    /************************************************
     * start sensor
     ************************************************/
    InitAttr.enGainMode = SNS_GAIN_MODE_SHARE;

    for (CVI_U8 i = 0; i < devNum; i++) {
        ViPipe = pstViCtx->pstPipeInfo[i].pipe[0];
        if (ViPipe < 0) {
            MEDIABUG_PRINTF("ViPipe err\n");
            return -1;
        }
        if (!pSnsObj[i]) {
            MEDIABUG_PRINTF("sns obj[%d] is null.\n", i);
            return CVI_FAILURE;
        }
        unSnsrBusInfo.s8I2cDev = pstViCtx->pstSensorCfg[i].s8I2cDev;
        snsr_gpio.snsr_rst_port_idx = pstViCtx->pstSensorCfg[i].u32Rst_port_idx;
        snsr_gpio.snsr_rst_pin = pstViCtx->pstSensorCfg[i].u32Rst_pin;
        snsr_gpio.snsr_rst_pol = pstViCtx->pstSensorCfg[i].u32Rst_pol;
        cvi_cif_reset_snsr_gpio_init(i, &snsr_gpio);
		InitAttr.u16UseHwSync = pstViCtx->pstSensorCfg[i].bHwSync;
        if((pstViCtx->pstSensorCfg[i].s32I2cAddr != -1) && pSnsObj[i]->pfnPatchI2cAddr) {
            pSnsObj[i]->pfnPatchI2cAddr(pstViCtx->pstSensorCfg[i].s32I2cAddr);
        }
        if (pSnsObj[i]->pfnSetInit)
            pSnsObj[i]->pfnSetInit(ViPipe, &InitAttr);
        MEDIABUG_PRINTF("bus info:%d,slave addr:%X\n", unSnsrBusInfo.s8I2cDev,
                pstViCtx->pstSensorCfg[i].s32I2cAddr);
        if (pSnsObj[i]->pfnSetBusInfo)
            pSnsObj[i]->pfnSetBusInfo(ViPipe, unSnsrBusInfo);
        if (pSnsObj[i]->pfnRegisterCallback)
            pSnsObj[i]->pfnRegisterCallback(ViPipe, &stAeLib, &stAwbLib);
        if (pSnsObj[i]->pfnExpSensorCb)
            pSnsObj[i]->pfnExpSensorCb(&stSnsrSensorFunc[i]);
        stSnsrSensorFunc[i].pfn_cmos_sensor_global_init(ViPipe);

        s32Ret = stSnsrSensorFunc[i].pfn_cmos_set_image_mode(ViPipe, &stSnsrMode[i]);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("sensor set image mode failed!\n");
            return CVI_FAILURE;
        }
        if (stSnsrSensorFunc[i].pfn_cmos_set_wdr_mode) {
            s32Ret = stSnsrSensorFunc[i].pfn_cmos_set_wdr_mode(ViPipe, stSnsrMode[i].u8SnsMode);
            if (s32Ret != CVI_SUCCESS) {
                MEDIABUG_PRINTF("sensor set wdr mode failed!\n");
                return CVI_FAILURE;
            }
        }
        if ((pstSysCtx->stSwitchCfg.bMipiSwitchEnable == 0) || (ViPipe != pstSysCtx->stSwitchCfg.u32SwitchPipe1)) {
            MEDIABUG_PRINTF("sensor reset pipe:%d\n", ViPipe);
            cif_reset_snsr_gpio(ViPipe, 1);
            cif_reset_mipi(i);
        }
        usleep(100);
        pSnsObj[i]->pfnGetRxAttr(i, &devAttr);
        if (pstViCtx->pstSensorCfg[i].as16LaneId[0] != 0 ||
            pstViCtx->pstSensorCfg[i].as16LaneId[1] != 0) {
            devAttr.mipi_attr.lane_id[0] = pstViCtx->pstSensorCfg[i].as16LaneId[0];
            devAttr.mipi_attr.lane_id[1] = pstViCtx->pstSensorCfg[i].as16LaneId[1];
            devAttr.mipi_attr.lane_id[2] = pstViCtx->pstSensorCfg[i].as16LaneId[2];
            devAttr.mipi_attr.lane_id[3] = pstViCtx->pstSensorCfg[i].as16LaneId[3];
            devAttr.mipi_attr.lane_id[4] = pstViCtx->pstSensorCfg[i].as16LaneId[4];
            devAttr.mipi_attr.pn_swap[0] = pstViCtx->pstSensorCfg[i].as8PNSwap[0];
            devAttr.mipi_attr.pn_swap[1] = pstViCtx->pstSensorCfg[i].as8PNSwap[1];
            devAttr.mipi_attr.pn_swap[2] = pstViCtx->pstSensorCfg[i].as8PNSwap[2];
            devAttr.mipi_attr.pn_swap[3] = pstViCtx->pstSensorCfg[i].as8PNSwap[3];
            devAttr.mipi_attr.pn_swap[4] = pstViCtx->pstSensorCfg[i].as8PNSwap[4];
        }
        if (pstViCtx->pstSensorCfg[i].bMclkEn == CVI_TRUE) {
            //u8Mclk
            devAttr.mclk.cam = pstViCtx->pstSensorCfg[i].u8MclkCam;
            devAttr.mclk.freq = pstViCtx->pstSensorCfg[i].u8MclkFreq;
            devAttr.mac_clk = pstViCtx->pstSensorCfg[i].u8Mclk;
        }
        if (pstViCtx->pstSensorCfg[i].MipiDev != -1) {
            devAttr.devno = pstViCtx->pstSensorCfg[i].MipiDev;
        }
        //devAttr.mipi_attr.lane_id = pstViCtx->pstSensorCfg[i].as16LaneId;

        cif_set_dev_attr(&devAttr);

        if ((pstSysCtx->stSwitchCfg.bMipiSwitchEnable == 0) || (ViPipe != pstSysCtx->stSwitchCfg.u32SwitchPipe1)) {
            cif_enable_snsr_clk(i, 1);
            usleep(100);
            cif_reset_snsr_gpio(i, 0);
        }
        usleep(1000);
        if (pSnsObj[i]->pfnSnsProbe) {
            s32Ret = pSnsObj[i]->pfnSnsProbe(ViPipe);
            if (s32Ret) {
                MEDIABUG_PRINTF("sensor probe failed!\n");
                return CVI_FAILURE;
            }
        }
    }

    return CVI_SUCCESS;
}
#if 0
static  int _media_sensor_deinit()
{
    return CVI_SUCCESS;
}
#endif

//设置开机快速收敛参数 5 个节点，Luma 和Bv 一一对应
static int setFastConvergeAttr(VI_PIPE ViPipe, ISP_AE_BOOT_FAST_CONVERGE_S stConvergeAttr)
{
    CVI_ISP_SetFastConvergeAttr(ViPipe, &stConvergeAttr);
    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_ViInit()
{
    PARAM_VI_CFG_S * pstViCfg = PARAM_getViCtx();
    return MEDIA_VIDEO_ViInit(pstViCfg);
}

int MEDIA_VIDEO_ViInit(PARAM_VI_CFG_S * pstViCfg)
{
    CVI_U8 devNum = 0;
    VI_DEV ViDev = 0;
    CVI_U8 ViPipe = 0;
    VI_DEV_ATTR_S stViDevAttr[VI_MAX_DEV_NUM];
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode[MAX_SENSOR_NUM];
    CVI_S32 snsr_type[MAX_SENSOR_NUM];
    ISP_SNS_OBJ_S *pSnsObj[MAX_SENSOR_NUM];
    ISP_FUNC_FROM_OUT_S stISP_FunctionFromOut = {0};
    PARAM_SYS_CFG_S * pstSysCtx = PARAM_getSysCtx();

    if ((pstViCfg == NULL) || (pstSysCtx == NULL)) {
        MEDIABUG_PRINTF("pstViCfg/pstSysCtx null err\n");
        return -1;
    }

    devNum = pstViCfg->u32WorkSnsCnt;


    for (CVI_U8 i = 0; i < devNum; i++) {
        MEDIA_CHECK_RET(getSnsType(snsr_type, devNum), "getSnsType fail");
        if (snsr_type[i] != pstViCfg->pstSensorCfg[i].enSnsType) {
            MEDIA_CHECK_RET(set_sensor_type(pstViCfg->pstSensorCfg[i].enSnsType, i), "set_sensor_type fail");
        }
    }

    MEDIA_CHECK_RET(getSnsType(snsr_type, devNum), "getSnsType fail");

    for (CVI_U8 i = 0; i < devNum; i++) {
        pSnsObj[i] = getSnsObj(snsr_type[i]);
    }

    MEDIA_CHECK_RET(_meida_sensor_init(pstViCfg, devNum),"_meida_sensor_init fail");
    CVI_VI_SetDevNum(devNum);

    for (int i = 0; i < devNum; i++) {
        ViPipe = pstViCfg->pstPipeInfo[i].pipe[0];
        if (ViPipe < 0) {
            MEDIABUG_PRINTF("ViPipe err\n");
            return -1;
        }
        ViDev = ViPipe;
        MEDIA_CHECK_RET(getDevAttr(i, &stViDevAttr[i]), "getDevAttr fail");

        if ((pstSysCtx->stSwitchCfg.bMipiSwitchEnable) &&
            ((ViPipe == pstSysCtx->stSwitchCfg.u32SwitchPipe0) || (ViPipe == pstSysCtx->stSwitchCfg.u32SwitchPipe1))) {
            MEDIABUG_PRINTF("Switch ViPipe:%d\n", ViPipe);
            stViDevAttr[i].isMux = 1;
            stViDevAttr[i].switchGpioIdx = pstSysCtx->stSwitchCfg.u32MipiSwitchGpioIdx;
            stViDevAttr[i].switchGpioPin = pstSysCtx->stSwitchCfg.u32MipiSwitchGpio;
            stViDevAttr[i].switchGPioPol = pstSysCtx->stSwitchCfg.bMipiSwitchPull;
            stViDevAttr[i].isFrmCtrl = 0;
            stViDevAttr[i].dstFrm = 0;
        }

        MEDIA_CHECK_RET(getSnsMode(i, &stSnsrMode[i]), "stSnsrMode fail");
        if((ViDev != pstSysCtx->stSwitchCfg.u32SwitchPipe1) && pstViCfg->pstDevInfo && (pstViCfg->pstDevInfo[i].pViDmaBuf != NULL)) {
            stViDevAttr[i].phy_addr = (intptr_t)pstViCfg->pstDevInfo[i].pViDmaBuf;
            stViDevAttr[i].phy_size = pstViCfg->pstDevInfo[i].u32ViDmaBufSize;
        }
        if (pstViCfg->pstDevInfo) {
            stViDevAttr[i].disEnableSbm = !pstSysCtx->pstSbmCfg[i].bEnable;
        }
        stViDevAttr[i].stWDRAttr.enWDRMode = stSnsrMode[i].u8SnsMode;
        MEDIA_CHECK_RET(CVI_VI_SetDevAttr(ViDev, &stViDevAttr[i]), "CVI_VI_SetDevAttr fail");
        MEDIA_CHECK_RET(CVI_VI_EnableDev(ViDev), "CVI_VI_EnableDev fail");
    }

    VI_PIPE_ATTR_S stPipeAttr;

    for (int i = 0; i < devNum; i++) {
        ViPipe = pstViCfg->pstPipeInfo[i].pipe[0];
        if (ViPipe < 0) {
            MEDIABUG_PRINTF("ViPipe err\n");
            return -1;
        }
        ViDev = ViPipe;
        MEDIA_CHECK_RET(getPipeAttr(i, &stPipeAttr), "getPipeAttr fail");
        stPipeAttr.enCompressMode = pstViCfg->pstChnInfo[i].enCompressMode;
        stPipeAttr.bYuvBypassPath = pstViCfg->pstChnInfo[i].bYuvBypassPath;
        MEDIA_CHECK_RET(CVI_VI_CreatePipe(ViDev, &stPipeAttr), "CVI_VI_CreatePipe fail");
        MEDIA_CHECK_RET(CVI_VI_StartPipe(ViDev), "CVI_VI_StartPipe fail");
        if (pstViCfg->bFastConverge == CVI_TRUE) {
            ISP_AE_BOOT_FAST_CONVERGE_S stConvergeAttr = {0};
            stConvergeAttr.bEnable = CVI_TRUE;
            if (pstViCfg->pstIspCfg[i].s8FastConvergeAvailableNode == 0) {
                //default
                CVI_S16 firstFrLuma[5] = {62, 77, 173, 343, 724};
                CVI_S16 targetBv[5] = {89, 194, 479, 533, 721};
                memcpy(stConvergeAttr.firstFrLuma, firstFrLuma,sizeof(firstFrLuma));
                memcpy(stConvergeAttr.targetBv, targetBv, sizeof(targetBv));
            } else {
                stConvergeAttr.availableNode = pstViCfg->pstIspCfg[i].s8FastConvergeAvailableNode;
                for (int j = 0; j < pstViCfg->pstIspCfg[i].s8FastConvergeAvailableNode; j++) {
                    stConvergeAttr.targetBv[j] = pstViCfg->pstIspCfg[i].as16targetBv[j];
                    stConvergeAttr.firstFrLuma[j] = pstViCfg->pstIspCfg[i].as16firstFrLuma[j];
                }
            }
            setFastConvergeAttr(ViDev, stConvergeAttr);
        }
    }

    for (int i = 0; i < devNum; i++) {
        ViPipe = pstViCfg->pstPipeInfo[i].pipe[0];
        if (ViPipe < 0) {
            MEDIABUG_PRINTF("ViPipe err\n");
            return -1;
        }
        ViDev = ViPipe;
        ISP_PUB_ATTR_S stPubAttr = { 0 };
        if (pstViCfg->pstSensorCfg[i].s32Framerate != 0) {
            stPubAttr.f32FrameRate = pstViCfg->pstSensorCfg[i].s32Framerate;
        } else {
            stPubAttr.f32FrameRate = 30;
        }
        stPubAttr.stWndRect.u32Width = stPubAttr.stSnsSize.u32Width = stViDevAttr[i].stSize.u32Width;
        stPubAttr.stWndRect.u32Height = stPubAttr.stSnsSize.u32Height = stViDevAttr[i].stSize.u32Height;
        stPubAttr.enBayer = stViDevAttr[i].enBayerFormat;
        stPubAttr.enWDRMode = stViDevAttr[i].stWDRAttr.enWDRMode;

        stISP_FunctionFromOut.pfnMediaVideoInit = MEDIA_VIDEO_Init;
        stISP_FunctionFromOut.pfnMediaVideoDeinit = MEDIA_VIDEO_Deinit;
        if (pstViCfg->pstIspCfg[i].s8ByPassNum != 0) {
            CVI_ISP_SetBypassFrm(ViDev, pstViCfg->pstIspCfg[i].s8ByPassNum);// set by pass frm
        }
        MEDIA_CHECK_RET(start_isp(stPubAttr, stISP_FunctionFromOut, ViDev),"start_isp fail");
    }

    VI_CHN_ATTR_S stChnAttr = {0};

    for (int i = 0; i < devNum; i++) {
        ViPipe = pstViCfg->pstPipeInfo[i].pipe[0];
        if (ViPipe < 0) {
            MEDIABUG_PRINTF("ViPipe err\n");
            return -1;
        }
        ViDev = ViPipe;
        MEDIA_CHECK_RET(getChnAttr(i, &stChnAttr), "getChnAttr fail");
        stChnAttr.enCompressMode = pstViCfg->pstChnInfo[i].enCompressMode;
	if (pstViCfg->pstSensorCfg[i].u8Orien <= 3) {
            stChnAttr.bMirror = pstViCfg->pstSensorCfg[i].u8Orien & 0x1;
            stChnAttr.bFlip = pstViCfg->pstSensorCfg[i].u8Orien & 0x2;
        }
        MEDIA_CHECK_RET(CVI_VI_SetChnAttr(ViDev, i, &stChnAttr), "CVI_VI_SetChnAttr fail");
        if (pSnsObj[i]->pfnMirrorFlip) {
            CVI_VI_RegChnFlipMirrorCallBack(ViDev, i, (void *)pSnsObj[i]->pfnMirrorFlip);
        }
        MEDIA_CHECK_RET(CVI_VI_EnableChn(ViDev, i), "CVI_VI_EnableChn fail");
        if(pstViCfg->pstSensorCfg[i].u8Rotation != ROTATION_0) {
            MEDIA_CHECK_RET(CVI_VI_SetChnRotation(ViDev,i,pstViCfg->pstSensorCfg[i].u8Rotation),"CVI_VI_SetChnRotation fail");
        }
    }

    int scene_mode = PARAM_getSceneMode();
    CVI_BIN_ImportBinData(pstViCfg->pstIspCfg[scene_mode].stPQBinDes[scene_mode].pIspBinData,
    pstViCfg->pstIspCfg[scene_mode].stPQBinDes[scene_mode].u32IspBinDataLen);

    for (int i = 0; i < devNum; i++) {
        ViPipe = pstViCfg->pstPipeInfo[i].pipe[0];
        if (ViPipe < 0) {
            MEDIABUG_PRINTF("ViPipe err\n");
            return -1;
        }
        ViDev = ViPipe;
        if(pstViCfg->pstIspCfg[scene_mode].bMonoSet[i]) {
            MEDIA_VIDEO_ViSetImageMono(ViDev);
        }
    }

    return CVI_SUCCESS;
}

int MEDIA_VIDEO_ViSetImageMono(VI_PIPE ViPipe)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    ISP_MONO_ATTR_S stMonoAttr;
    stMonoAttr.Enable = 1;
    s32Ret = CVI_ISP_SetMonoAttr(ViPipe, &stMonoAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("SetMonoAttr pipe(%d) failed with %#x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

int MEDIA_VIDEO_ViDeinit(PARAM_VI_CFG_S * pstViCfg)
{
    VI_DEV ViDev = 0;
    VI_CHN ViChn = 0;
    CVI_S32 ret = CVI_SUCCESS;
    CVI_U8 i = 0;

    if(pstViCfg == NULL) {
        return CVI_FAILURE;
    }
    CVI_U8 dev_num = pstViCfg->u32WorkSnsCnt;

    for (i = 0; i < pstViCfg->u32WorkSnsCnt; i++) {
        ViChn = ViDev = i;
        MEDIA_CHECK_RET(stop_isp(ViDev, dev_num), "stop_isp fail");
        ret = CVI_VI_DisableChn(ViDev, ViChn);
        if (ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("CVI_VI_DisableChn FAIL!\n");
            return CVI_FAILURE;
        }

        MEDIA_CHECK_RET(CVI_VI_DestroyPipe(ViDev), "CVI_VI_DestroyPipe fail");
        //disable vi_dev
        MEDIA_CHECK_RET(CVI_VI_DisableDev(ViDev), "CVI_VI_DisableDev fail");
        MEDIA_CHECK_RET(CVI_VI_UnRegChnFlipMirrorCallBack(0, ViDev), "CVI_VI_UnRegChnFlipMirrorCallBack");
    }

    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_ViDeInit()
{
    PARAM_VI_CFG_S * pstViCfg = PARAM_getViCtx();
    return MEDIA_VIDEO_ViDeinit(pstViCfg);
}
#endif

int MEDIA_VIDEO_SysInit()
{
    //sys/base init
    sys_core_init();
    //vip init
#if CONFIG_SUPPORT_VI
    cvi_cif_init();
    cvi_snsr_i2c_probe();
    vi_core_init();
#endif
    vpss_core_init();
#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
    vo_core_init();
#endif
    rgn_core_init();
    cvi_ldc_probe();
#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
    mipi_tx_probe();
#endif
    return CVI_SUCCESS;
}

int MEDIA_VIDEO_SysDeinit()
{
    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_SysVbInit()
{
    PARAM_SYS_CFG_S * pstSysCtx = PARAM_getSysCtx();
    return MEDIA_VIDEO_SysVbInit(pstSysCtx);
}

int MEDIA_VIDEO_SysVbInit(PARAM_SYS_CFG_S * pstSysCtx)
{
    VPSS_MODE_S stVPSSMode = {0};
    VI_VPSS_MODE_S stVIVPSSMode = {0};
    CVI_U32 	u32BlkSize;
    CVI_U32 	u32RotBlkSize;
    VB_CONFIG_S stVbConfig;
    CVI_U32 i = 0;

    if(pstSysCtx == NULL) {
        return CVI_SUCCESS;
    }
    MEDIA_CHECK_RET(CVI_SYS_Init(), "CVI_SYS_Init failed\n");
    stVPSSMode.enMode = pstSysCtx->stVPSSMode.enMode;
    for(i = 0; i < VI_MAX_PIPE_NUM; i++) {
        stVIVPSSMode.aenMode[i] = pstSysCtx->stVIVPSSMode.aenMode[i];
    }
    for(i = 0; i < VPSS_IP_NUM; i++) {
        stVPSSMode.aenInput[i] = pstSysCtx->stVPSSMode.aenInput[i];
        stVPSSMode.ViPipe[i] = pstSysCtx->stVPSSMode.ViPipe[i];
    }
    MEDIA_CHECK_RET(CVI_SYS_SetVIVPSSMode(&stVIVPSSMode),"CVI_SYS_SetVIVPSSMode failed\n");
    CVI_SYS_SetVPSSModeEx(&stVPSSMode);

    //vb init
    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt = pstSysCtx->u8VbPoolCnt;
    if (stVbConfig.u32MaxPoolCnt <= 0) {
        return CVI_FAILURE;
    }
    for(i = 0 ; i < stVbConfig.u32MaxPoolCnt ; i++) {
        u32BlkSize = COMMON_GetPicBufferSize(pstSysCtx->pstVbPool[i].u16width,pstSysCtx->pstVbPool[i].u16height,pstSysCtx->pstVbPool[i].fmt,
        pstSysCtx->pstVbPool[i].enBitWidth,pstSysCtx->pstVbPool[i].enCmpMode,DEFAULT_ALIGN);
        u32RotBlkSize = COMMON_GetPicBufferSize(pstSysCtx->pstVbPool[i].u16height,pstSysCtx->pstVbPool[i].u16width,pstSysCtx->pstVbPool[i].fmt,
        pstSysCtx->pstVbPool[i].enBitWidth,pstSysCtx->pstVbPool[i].enCmpMode,DEFAULT_ALIGN);
        u32BlkSize = MAX2(u32BlkSize, u32RotBlkSize);
        stVbConfig.astCommPool[i].u32BlkSize	= u32BlkSize;
        stVbConfig.astCommPool[i].u32BlkCnt = pstSysCtx->pstVbPool[i].u8VbBlkCnt;
    }
    MEDIA_CHECK_RET(CVI_VB_SetConfig(&stVbConfig), "CVI_VB_SetConfig failed\n");
    MEDIA_CHECK_RET(CVI_VB_Init(), "CVI_VB_Init failed\n");
    return CVI_SUCCESS;
}

int MEDIA_VIDEO_SysVbDeinit()
{
    CVI_VB_Exit();
    CVI_SYS_Exit();
    return CVI_SUCCESS;
}

int _MEDIA_VIDEO_VpssInit()
{
    PARAM_VPSS_CFG_S * pstVpssCtx = PARAM_getVpssCtx();
    return MEDIA_VIDEO_VpssInit(pstVpssCtx);
}

int MEDIA_VIDEO_VpssInit(PARAM_VPSS_CFG_S * pstVpssCtx)
{
    VPSS_GRP VpssGrp = 0;
    VPSS_GRP_ATTR_S * pstVpssGrp = NULL;
    VPSS_CHN_ATTR_S * pstVpssChn = NULL;
    #if CONFIG_SUPPORT_VI
    VI_DEV_ATTR_S stViDevAttr = {0};
    #endif

    CVI_U8 i = 0;
    CVI_U8 j = 0;
    PARAM_SYS_CFG_S * pstSysCtx = PARAM_getSysCtx();

    if(pstVpssCtx == NULL) {
        MEDIABUG_PRINTF("********MEDIA_VIDEO_VpssInit pstVpssCtx NULL err \n");
        return -1;
    }
    for(i = 0; i < pstVpssCtx->u8GrpCnt; i++) {
        if (!pstVpssCtx->pstVpssGrpCfg[i].bEnable) {
            continue;
        }
        pstVpssGrp = &pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr;
        VpssGrp = pstVpssCtx->pstVpssGrpCfg[i].VpssGrp;
        #if CONFIG_SUPPORT_VI
        if (pstVpssCtx->pstVpssGrpCfg[i].s32BindVidev != -1) {
            MEDIA_CHECK_RET(getDevAttr(pstVpssCtx->pstVpssGrpCfg[i].s32BindVidev, &stViDevAttr), "getDevAttr fail");
            if(pstVpssCtx->pstVpssGrpCfg[i].u8ViRotation == 90) {
                pstVpssGrp->u32MaxW = stViDevAttr.stSize.u32Height;
                pstVpssGrp->u32MaxH = stViDevAttr.stSize.u32Width;
            } else {
                pstVpssGrp->u32MaxW = stViDevAttr.stSize.u32Width;
                pstVpssGrp->u32MaxH = stViDevAttr.stSize.u32Height;
            }
        }
        #endif
        MEDIA_CHECK_RET(CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrp), "CVI_VPSS_CreateGrp failed\n");
        for(j = 0; j < pstVpssCtx->pstVpssGrpCfg[i].u8ChnCnt; j++) {
            pstVpssChn = &pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr;
            MEDIA_CHECK_RET(CVI_VPSS_SetChnAttr(VpssGrp, j, pstVpssChn), "CVI_VPSS_SetChnAttr failed\n");

            for (int idx = 0; idx < pstSysCtx->u8SbmCnt; ++idx) {
                if ((i == pstSysCtx->pstSbmCfg[idx].s32SbmGrp) && (j == pstSysCtx->pstSbmCfg[idx].s32SbmChn)) {
                    pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.bEnable = CVI_TRUE;
                    pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.u32BufLine =
                        pstSysCtx->pstSbmCfg[idx].s32WrapBufLine;
                    pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.u32WrapBufferSize =
                        pstSysCtx->pstSbmCfg[idx].s32WrapBufSize;

                    MEDIA_CHECK_RET(CVI_VPSS_SetChnBufWrapAttr(VpssGrp, j,
                        &pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap),
                        "CVI_VPSS_SetChnBufWrapAttr failed\n");
                    break;
                }
            }

            MEDIA_CHECK_RET(CVI_VPSS_EnableChn(VpssGrp, j), "CVI_VPSS_EnableChn failed\n");

            /*set vpss chn rotation*/
            if(pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8Rotation != ROTATION_0) {
                MEDIA_CHECK_RET(CVI_VPSS_SetChnRotation(VpssGrp, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8Rotation)
                                , "CVI_VPSS_SetChnRotation failed\n");
            }

            /*set vpss chn bind vb pool*/
            if(pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8VpssAttachEnable != ROTATION_0) {
                MEDIA_CHECK_RET(CVI_VPSS_AttachVbPool(VpssGrp, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8VpssAttachId)
                                , "CVI_VPSS_AttachVbPool failed\n");
            }
        }
        MEDIA_CHECK_RET(CVI_VPSS_StartGrp(VpssGrp), "CVI_VPSS_StartGrp failed\n");
        if(pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.bEnable == CVI_TRUE) {
            CVI_VPSS_SetGrpCrop(VpssGrp, &pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo);
        }
        if(pstVpssCtx->pstVpssGrpCfg[i].bBindMode == CVI_TRUE) {
            MEDIA_CHECK_RET(CVI_SYS_Bind(&pstVpssCtx->pstVpssGrpCfg[i].astChn[0], &pstVpssCtx->pstVpssGrpCfg[i].astChn[1]), "CVI_SYS_Bind err");
        }
    }
    return CVI_SUCCESS;
}

int MEDIA_VIDEO_VpssDeinit(PARAM_VPSS_CFG_S * pstVpssCtx)
{
    CVI_S32 i,j;
    CVI_S32 s32Ret = CVI_SUCCESS;
    PARAM_VPSS_GRP_CFG_S *pstVpssGrpCfg = NULL;

    if(pstVpssCtx == NULL) {
        return CVI_FAILURE;
    }
    for(i = 0; i < pstVpssCtx->u8GrpCnt; i++) {
        pstVpssGrpCfg = &pstVpssCtx->pstVpssGrpCfg[i];
        if(pstVpssCtx->pstVpssGrpCfg[i].bBindMode == CVI_TRUE) {
            MEDIA_CHECK_RET(CVI_SYS_UnBind(&pstVpssCtx->pstVpssGrpCfg[i].astChn[0], &pstVpssCtx->pstVpssGrpCfg[i].astChn[1]),"CVI_SYS_UnBind fail");
        }
        for(j = 0; j < pstVpssGrpCfg->u8ChnCnt; j++) {
            s32Ret = CVI_VPSS_DisableChn(pstVpssGrpCfg->VpssGrp,j);
            if (s32Ret != CVI_SUCCESS) {
                MEDIABUG_PRINTF("CVI_VPSS_DISABLE_CHN[%d][%d] fialed with %#x\n",pstVpssGrpCfg->VpssGrp,j,s32Ret);
                continue;
            }
        }
        s32Ret = CVI_VPSS_StopGrp(pstVpssGrpCfg->VpssGrp);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("CVI_VPSS_StopGrp[%d] fialed with %#x\n",pstVpssGrpCfg->VpssGrp,s32Ret);
            return CVI_FAILURE;
        }
        s32Ret = CVI_VPSS_DestroyGrp(pstVpssGrpCfg->VpssGrp);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("CVI_VPSS_DestroyGrp[%d] fialed with %#x\n",pstVpssGrpCfg->VpssGrp,s32Ret);
            return CVI_FAILURE;
        }
    }
    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_VpssDeinit()
{
    PARAM_VPSS_CFG_S * pstVpssCtx = PARAM_getVpssCtx();
    return MEDIA_VIDEO_VpssDeinit(pstVpssCtx);
}


int _MEDIA_VIDEO_DSIInit(int devno, const struct dsc_instr *cmds, int size)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    for (int i = 0; i < size; i++) {
        const struct dsc_instr *instr = &cmds[i];
        struct cmd_info_s cmd_info = {
            .devno = devno,
            .cmd_size = instr->size,
            .data_type = instr->data_type,
            .cmd = (void *)instr->data
        };

        s32Ret = CVI_MIPI_TX_SendCmd(0, &cmd_info);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("dsi init failed at %d instr.\n", i);
            return CVI_FAILURE;
        }
        if (instr->delay) {
            udelay(instr->delay * 1000);
        }
    }

    return 0;
}

#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
void _MEDIA_VIDEO_PanelPreInit(void *data)
 {
     int fd = 0;
     CVI_S32 s32Ret = CVI_SUCCESS;
     struct panel_desc_s *panel_desc = (struct panel_desc_s *)data;

     s32Ret = CVI_MIPI_TX_Disable(fd);
     if (s32Ret != CVI_SUCCESS) {
         MEDIABUG_PRINTF("CVI_MIPI_TX_Disable failed with %#x\n", s32Ret);
         return;
     }

     s32Ret = CVI_MIPI_TX_Cfg(fd, panel_desc->dev_cfg);
     if (s32Ret != CVI_SUCCESS) {
         MEDIABUG_PRINTF("CVI_MIPI_TX_Cfg failed with %#x\n", s32Ret);
     }

    return;
}

void *_MEDIA_VIDEO_PanelInit(void *data)
{
    int fd = 0;
    CVI_S32 s32Ret = CVI_SUCCESS;
    struct panel_desc_s *panel_desc = (struct panel_desc_s *)data;

    s32Ret = _MEDIA_VIDEO_DSIInit(0, panel_desc->dsi_init_cmds, panel_desc->dsi_init_cmds_size);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("dsi_init failed with %#x\n", s32Ret);
        return NULL;
    }
    s32Ret = CVI_MIPI_TX_SetHsSettle(fd, panel_desc->hs_timing_cfg);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("CVI_MIPI_TX_SetHsSettle failed with %#x\n", s32Ret);
        return NULL;
    }
    s32Ret = CVI_MIPI_TX_Enable(fd);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("CVI_MIPI_TX_Enable failed with %#x\n", s32Ret);
        return NULL;
    }
    printf("Init for MIPI-Driver-%s\n", panel_desc->panel_name);

    pthread_exit(NULL);
    return NULL;
}

int MEDIA_VIDEO_PanelInit(void)
{
    struct sched_param param;
    pthread_attr_t attr;
    pthread_condattr_t cattr;
    pthread_t thread;

    _MEDIA_VIDEO_PanelPreInit((void *)&panel_desc);

    param.sched_priority = MIPI_TX_RT_PRIO;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_create(&thread, &attr, _MEDIA_VIDEO_PanelInit, (void *)&panel_desc);
    pthread_setname_np(thread, "cvi_mipi_tx");
    // *(unsigned int *)0x0a088094 = 0x0701000a; //colorbar
    return 0;
}

int MEDIA_VIDEO_VoInit(PARAM_VO_CFG_S * pstVoCtx)
{
    //vo init
    CVI_S32 i = 0;
    CVI_S32 j = 0;
    VO_CONFIG_S * pstVoConfig = NULL;
    VO_VIDEO_LAYER_ATTR_S * pstLayerAttr = NULL;
    VO_CHN_ATTR_S * pstVOChnAttr = NULL;

    if(pstVoCtx == NULL) {
        return CVI_FAILURE;
    }

    if (pstVoCtx->u8VoCnt == 0) {
        return CVI_SUCCESS;
    }
    MEDIA_VIDEO_PanelInit();
    return 0;
    for(i = 0 ;i < pstVoCtx->u8VoCnt;i++) {
        pstVoConfig = &pstVoCtx->pstVoCfg[i].stVoConfig;
        pstLayerAttr = &pstVoCtx->pstVoCfg[i].stLayerAttr;
        MEDIA_CHECK_RET(CVI_VO_SetPubAttr(pstVoConfig->VoDev, &pstVoConfig->stVoPubAttr),"CVI_VO_SetPubAttr failed!\n");
        MEDIA_CHECK_RET(CVI_VO_Enable(pstVoConfig->VoDev), "CVI_VO_Enable failed!\n");
        MEDIA_CHECK_RET(CVI_VO_SetDisplayBufLen(pstVoCtx->pstVoCfg[i].VoLayer, pstVoConfig->u32DisBufLen),"CVI_VO_SetDisplayBufLen failed!\n");
        MEDIA_CHECK_RET(CVI_VO_SetVideoLayerAttr(pstVoCtx->pstVoCfg[i].VoLayer, pstLayerAttr),
            "CVI_VO_SetVideoLayerAttr failed!\n");
        MEDIA_CHECK_RET(CVI_VO_EnableVideoLayer(pstVoCtx->pstVoCfg[i].VoLayer),
            "CVI_VO_EnableVideoLayer failed!\n");
        for(j = 0; j < pstVoCtx->pstVoCfg[i].u8ChnCnt; j++) {
            pstVOChnAttr = &pstVoCtx->pstVoCfg[i].pstVOChnAttr[j];
            MEDIA_CHECK_RET(CVI_VO_SetChnAttr(pstVoCtx->pstVoCfg[i].VoLayer, j, pstVOChnAttr),
            "CVI_VO_SetChnAttr failed!\n");
            CVI_VO_SetChnRotation(pstVoCtx->pstVoCfg[i].VoLayer,j,pstVoCtx->pstVoCfg[i].u8ChnRotation);
            MEDIA_CHECK_RET(CVI_VO_EnableChn(pstVoCtx->pstVoCfg[i].VoLayer, j), "CVI_VO_EnableChn failed!\n");
        }
        MEDIA_CHECK_RET(CVI_SYS_Bind(&pstVoCtx->pstVoCfg[i].stSrcChn, &pstVoCtx->pstVoCfg[i].stDestChn), "CVI_SYS_Bind(VPSS-VO)");
    }
    MEDIABUG_PRINTF("******start vo******\n");
    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_VoInit()
{
    PARAM_VO_CFG_S * pstVoCtx = PARAM_getVoCtx();
    return MEDIA_VIDEO_VoInit(pstVoCtx);
}

int MEDIA_VIDEO_VoDeinit(PARAM_VO_CFG_S * pstVoCtx)
{
    CVI_S32 i = 0;
    CVI_S32 j = 0;
    VO_DEV VoDev = 0;
    VO_LAYER VoLayer = 0;
    VO_CHN VoChn = 0;

    if(!pstVoCtx) {
        return CVI_FAILURE;
    }
    for(i = 0 ;i < pstVoCtx->u8VoCnt;i++) {
        VoDev = pstVoCtx->pstVoCfg[i].stVoConfig.VoDev;
        VoLayer = pstVoCtx->pstVoCfg[i].VoLayer;
        MEDIA_CHECK_RET(CVI_SYS_UnBind(&pstVoCtx->pstVoCfg[i].stSrcChn, &pstVoCtx->pstVoCfg[i].stDestChn), "CVI_SYS_UnBind(VPSS-VO) fail");
        for(j = 0; j < pstVoCtx->pstVoCfg[i].u8ChnCnt; j++) {
            VoChn = j ;
            MEDIA_CHECK_RET(CVI_VO_DisableChn(VoLayer, VoChn), "CVI_VO_DisableChn failed!\n");
        }
        MEDIA_CHECK_RET(CVI_VO_DisableVideoLayer(VoLayer), "CVI_VO_DisableVideoLayer failed!\n");
        MEDIA_CHECK_RET(CVI_VO_Disable(VoDev), "CVI_VO_Disable failed!\n");
    }
    MEDIABUG_PRINTF("******stop vo******\n");
    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_VoDeinit()
{
    PARAM_VO_CFG_S * pstVoCtx = PARAM_getVoCtx();
    return MEDIA_VIDEO_VoDeinit(pstVoCtx);
}
#endif

int MEDIA_VIDEO_VencChnInit(PARAM_VENC_CFG_S *pstVencCfg,int VencChn)
{
    VENC_CHN_ATTR_S stAttr = {0};
    VENC_PARAM_MOD_S stModParam = {0};
    VENC_RC_PARAM_S stRcParam = {0};
    VENC_REF_PARAM_S stRefParam = {0};
    VENC_CHN_PARAM_S stChnParam = {0};
    VENC_ROI_ATTR_S stRoiAttr = {0};
    VENC_H265_VUI_S stH265Vui = {0};
    VENC_H264_ENTROPY_S stH264EntropyEnc = {0};
    VENC_H264_VUI_S stH264Vui = {0};
    VENC_JPEG_PARAM_S stJpegParam = {0};
    VPSS_CHN_ATTR_S stVpssChnAttr = {0};
    VENC_RECV_PIC_PARAM_S stRecvParam = {0};
    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDestChn;
    ROTATION_E enRotation;
    PARAM_VENC_CHN_CFG_S *pstVecncChnCtx = NULL;
    PARAM_SYS_CFG_S * pstSysCtx = PARAM_getSysCtx();

    if(!pstVencCfg) {
        MEDIABUG_PRINTF("pstVencCfg is null. \n");
        return CVI_FAILURE;
    }
    if (VencChn >= pstVencCfg->s32VencChnCnt) {
        MEDIABUG_PRINTF("VencChn >= VencChnCnt. \n");
        return CVI_FAILURE;
    }

    pstVecncChnCtx = &pstVencCfg->pstVencChnCfg[VencChn];

    if(pstVecncChnCtx == NULL) {
        MEDIABUG_PRINTF("pstVecncChnCtx is null. \n");
        return CVI_FAILURE;
    }

    if (0 == pstVecncChnCtx->stChnParam.bEnable) {
        return CVI_SUCCESS;
    }

    VencChn = pstVecncChnCtx->stChnParam.u8VencChn;
    if(pstVecncChnCtx->stChnParam.astChn[0].enModId == CVI_ID_VPSS && pstVecncChnCtx->stChnParam.u16Width == 0 &&  pstVecncChnCtx->stChnParam.u16Height == 0) {
        MEDIA_CHECK_RET(CVI_VPSS_GetChnAttr(pstVecncChnCtx->stChnParam.astChn[0].s32DevId, pstVecncChnCtx->stChnParam.astChn[0].s32ChnId, &stVpssChnAttr), "CVI_VPSS_GetChnAttr");
        // if VPSS enable rotation, need exchange width and height
        MEDIA_CHECK_RET(CVI_VPSS_GetChnRotation(pstVecncChnCtx->stChnParam.astChn[0].s32DevId, pstVecncChnCtx->stChnParam.astChn[0].s32ChnId, &enRotation), "CVI_VPSS_GetChnRotation");
        if (ROTATION_90 == enRotation || ROTATION_270 == enRotation) {
            pstVecncChnCtx->stChnParam.u16Width = (CVI_U16)stVpssChnAttr.u32Height;
            pstVecncChnCtx->stChnParam.u16Height = (CVI_U16)stVpssChnAttr.u32Width;
        } else {
            pstVecncChnCtx->stChnParam.u16Height = (CVI_U16)stVpssChnAttr.u32Height;
            pstVecncChnCtx->stChnParam.u16Width = (CVI_U16)stVpssChnAttr.u32Width;
        }
    }

    stAttr.stVencAttr.enType = pstVecncChnCtx->stChnParam.u16EnType;

    stAttr.stVencAttr.u32MaxPicWidth = pstVecncChnCtx->stChnParam.u16Width;
    stAttr.stVencAttr.u32MaxPicHeight = pstVecncChnCtx->stChnParam.u16Height;

    stAttr.stVencAttr.u32BufSize = pstVecncChnCtx->stChnParam.u32BitStreamBufSize;
    stAttr.stVencAttr.u32Profile = pstVecncChnCtx->stChnParam.u8Profile;
    stAttr.stVencAttr.bByFrame = CVI_TRUE;
    stAttr.stVencAttr.bIsoSendFrmEn = CVI_FALSE;
    stAttr.stVencAttr.u32PicWidth = pstVecncChnCtx->stChnParam.u16Width;
    stAttr.stVencAttr.u32PicHeight = pstVecncChnCtx->stChnParam.u16Height;
    stAttr.stVencAttr.bSingleCore = 0;
    stAttr.stVencAttr.bEsBufQueueEn = pstVecncChnCtx->stChnParam.u8EsBufQueueEn;

    stAttr.stRcAttr.enRcMode = pstVecncChnCtx->stRcParam.u16RcMode;
    switch (pstVecncChnCtx->stChnParam.u16EnType) {
        case PT_H264: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264CBR) {
                stAttr.stRcAttr.stH264Cbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH264Cbr.u32BitRate = pstVecncChnCtx->stRcParam.u16BitRate;
                stAttr.stRcAttr.stH264Cbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH264Cbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264VBR) {
                stAttr.stRcAttr.stH264Vbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH264Vbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH264Vbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH264Vbr.u32MaxBitRate = pstVecncChnCtx->stRcParam.u16BitRate;
                stAttr.stRcAttr.stH264Vbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH264Vbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264AVBR) {
                stAttr.stRcAttr.stH264AVbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH264AVbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH264AVbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH264AVbr.u32MaxBitRate = pstVecncChnCtx->stRcParam.u16BitRate;
                stAttr.stRcAttr.stH264AVbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH264AVbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else {
                MEDIABUG_PRINTF("PT_H264 Don't support RcMode:%d. \n", stAttr.stRcAttr.enRcMode);
                return CVI_FAILURE;
            }
            stAttr.stVencAttr.stAttrH264e.addrRemapEn = CVI_TRUE;
            break;
        }

        case PT_H265: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265CBR) {
                stAttr.stRcAttr.stH265Cbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH265Cbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH265Cbr.u32BitRate = pstVecncChnCtx->stRcParam.u16BitRate;
                stAttr.stRcAttr.stH265Cbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH265Cbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265VBR) {
                stAttr.stRcAttr.stH265Vbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH265Vbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH265Vbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH265Vbr.u32MaxBitRate = pstVecncChnCtx->stRcParam.u16BitRate;
                stAttr.stRcAttr.stH265Vbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH265Vbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265AVBR) {
                stAttr.stRcAttr.stH265AVbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH265AVbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH265AVbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH265AVbr.u32MaxBitRate = pstVecncChnCtx->stRcParam.u16BitRate;
                stAttr.stRcAttr.stH265AVbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH265AVbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else {
                MEDIABUG_PRINTF("PT_H265 Don't support RcMode:%d. \n", stAttr.stRcAttr.enRcMode);
                return CVI_FAILURE;
            }
            stAttr.stVencAttr.stAttrH265e.addrRemapEn = CVI_TRUE;
            break;
        }

        case PT_MJPEG: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGCBR) {
                stAttr.stRcAttr.stMjpegCbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stMjpegCbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stMjpegCbr.u32BitRate = pstVecncChnCtx->stRcParam.u16BitRate;
                stAttr.stRcAttr.stMjpegCbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGVBR) {
                stAttr.stRcAttr.stMjpegVbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stMjpegVbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stMjpegVbr.u32MaxBitRate = pstVecncChnCtx->stRcParam.u32MaxBitRate;
                stAttr.stRcAttr.stMjpegVbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else {
                MEDIABUG_PRINTF("PT_MJPEG Don't support RcMode:%d. \n", stAttr.stRcAttr.enRcMode);
                return CVI_FAILURE;
            }
            break;
        }

        case PT_JPEG: {
            stAttr.stVencAttr.stAttrJpege.bSupportDCF = CVI_FALSE;
            stAttr.stVencAttr.stAttrJpege.enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
            stAttr.stVencAttr.stAttrJpege.stMPFCfg.u8LargeThumbNailNum = 0;
            break;
        }

        default : {
            MEDIABUG_PRINTF("Don't support enType:%d. \n", pstVecncChnCtx->stChnParam.u16EnType);
            return CVI_FAILURE;
        }
    }
    stAttr.stGopAttr.enGopMode = pstVecncChnCtx->stGopParam.u16gopMode;
    stAttr.stGopAttr.stNormalP.s32IPQpDelta = pstVecncChnCtx->stGopParam.s8IPQpDelta;
    MEDIA_CHECK_RET(CVI_VENC_CreateChn(VencChn, &stAttr), "CVI_VENC_CreateChn");

    MEDIA_CHECK_RET(CVI_VENC_GetModParam(&stModParam), "CVI_VENC_GetModParam");
    MEDIA_CHECK_RET(CVI_VENC_SetModParam(&stModParam), "CVI_VENC_SetModParam");

    MEDIA_CHECK_RET(CVI_VENC_GetRcParam(VencChn, &stRcParam), "CVI_VENC_GetRcParam");
    stRcParam.s32FirstFrameStartQp = pstVecncChnCtx->stRcParam.u16FirstFrmstartQp;
    stRcParam.s32InitialDelay = pstVecncChnCtx->stRcParam.u16InitialDelay;
    stRcParam.u32ThrdLv = pstVecncChnCtx->stRcParam.u16ThrdLv;
    stRcParam.s32BgDeltaQp = pstVecncChnCtx->stRcParam.u16BgDeltaQp;

    switch (pstVecncChnCtx->stChnParam.u16EnType) {
        case PT_H264: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264CBR) {
                stRcParam.stParamH264Cbr.bQpMapEn = CVI_FALSE;
                stRcParam.stParamH264Cbr.s32MaxReEncodeTimes = pstVecncChnCtx->stRcParam.u8MaxReEncodeTimes;
                stRcParam.stParamH264Cbr.u32MaxIprop = pstVecncChnCtx->stRcParam.u8MaxIprop;
                stRcParam.stParamH264Cbr.u32MinIprop = pstVecncChnCtx->stRcParam.u8MinIprop;
                stRcParam.stParamH264Cbr.u32MaxIQp = pstVecncChnCtx->stRcParam.u8MaxIqp;
                stRcParam.stParamH264Cbr.u32MaxQp = pstVecncChnCtx->stRcParam.u8MaxQp;
                stRcParam.stParamH264Cbr.u32MinIQp = pstVecncChnCtx->stRcParam.u8MinIqp;
                stRcParam.stParamH264Cbr.u32MinQp = pstVecncChnCtx->stRcParam.u8MinQp;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264VBR) {
                stRcParam.stParamH264Vbr.bQpMapEn = CVI_FALSE;
                stRcParam.stParamH264Vbr.s32MaxReEncodeTimes = pstVecncChnCtx->stRcParam.u8MaxReEncodeTimes;
                stRcParam.stParamH264Vbr.u32MaxIprop = pstVecncChnCtx->stRcParam.u8MaxIprop;
                stRcParam.stParamH264Vbr.u32MinIprop = pstVecncChnCtx->stRcParam.u8MinIprop;
                stRcParam.stParamH264Vbr.u32MaxIQp = pstVecncChnCtx->stRcParam.u8MaxIqp;
                stRcParam.stParamH264Vbr.u32MaxQp = pstVecncChnCtx->stRcParam.u8MaxQp;
                stRcParam.stParamH264Vbr.u32MinIQp = pstVecncChnCtx->stRcParam.u8MinIqp;
                stRcParam.stParamH264Vbr.u32MinQp = pstVecncChnCtx->stRcParam.u8MinQp;
                stRcParam.stParamH264Vbr.s32ChangePos = pstVecncChnCtx->stRcParam.u8ChangePos;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264AVBR) {
                stRcParam.stParamH264AVbr.bQpMapEn = CVI_FALSE;
                stRcParam.stParamH264AVbr.s32MaxReEncodeTimes = pstVecncChnCtx->stRcParam.u8MaxReEncodeTimes;
                stRcParam.stParamH264AVbr.u32MaxIprop = pstVecncChnCtx->stRcParam.u8MaxIprop;
                stRcParam.stParamH264AVbr.u32MinIprop = pstVecncChnCtx->stRcParam.u8MinIprop;
                stRcParam.stParamH264AVbr.u32MaxIQp = pstVecncChnCtx->stRcParam.u8MaxIqp;
                stRcParam.stParamH264AVbr.u32MaxQp = pstVecncChnCtx->stRcParam.u8MaxQp;
                stRcParam.stParamH264AVbr.u32MinIQp = pstVecncChnCtx->stRcParam.u8MinIqp;
                stRcParam.stParamH264AVbr.u32MinQp = pstVecncChnCtx->stRcParam.u8MinQp;
                stRcParam.stParamH264AVbr.s32ChangePos = pstVecncChnCtx->stRcParam.u8ChangePos;
                stRcParam.stParamH264AVbr.s32MinStillPercent = pstVecncChnCtx->stRcParam.s32MinStillPercent;
                stRcParam.stParamH264AVbr.u32MaxStillQP = pstVecncChnCtx->stRcParam.u32MaxStillQP;
                stRcParam.stParamH264AVbr.u32MotionSensitivity = pstVecncChnCtx->stRcParam.u32MotionSensitivity;
                stRcParam.stParamH264AVbr.s32AvbrFrmLostOpen = pstVecncChnCtx->stRcParam.s32AvbrFrmLostOpen;
                stRcParam.stParamH264AVbr.s32AvbrFrmGap = pstVecncChnCtx->stRcParam.s32AvbrFrmGap;
                stRcParam.stParamH264AVbr.s32AvbrPureStillThr = pstVecncChnCtx->stRcParam.s32AvbrPureStillThr;
            } else {
                return CVI_FAILURE;
            }
            break;
        }

        case PT_H265: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265CBR) {
                stRcParam.stParamH265Cbr.bQpMapEn = CVI_FALSE;
                stRcParam.stParamH265Cbr.s32MaxReEncodeTimes = pstVecncChnCtx->stRcParam.u8MaxReEncodeTimes;
                stRcParam.stParamH265Cbr.u32MaxIprop = pstVecncChnCtx->stRcParam.u8MaxIprop;
                stRcParam.stParamH265Cbr.u32MinIprop = pstVecncChnCtx->stRcParam.u8MinIprop;
                stRcParam.stParamH265Cbr.u32MaxIQp = pstVecncChnCtx->stRcParam.u8MaxIqp;
                stRcParam.stParamH265Cbr.u32MaxQp = pstVecncChnCtx->stRcParam.u8MaxQp;
                stRcParam.stParamH265Cbr.u32MinIQp =  pstVecncChnCtx->stRcParam.u8MinIqp;
                stRcParam.stParamH265Cbr.u32MinQp = pstVecncChnCtx->stRcParam.u8MinQp;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265VBR) {
                stRcParam.stParamH265Vbr.bQpMapEn = CVI_FALSE;
                stRcParam.stParamH265Vbr.s32MaxReEncodeTimes = pstVecncChnCtx->stRcParam.u8MaxReEncodeTimes;
                stRcParam.stParamH265Vbr.u32MaxIprop = pstVecncChnCtx->stRcParam.u8MaxIprop;
                stRcParam.stParamH265Vbr.u32MinIprop = pstVecncChnCtx->stRcParam.u8MinIprop;
                stRcParam.stParamH265Vbr.u32MaxIQp = pstVecncChnCtx->stRcParam.u8MaxIqp;
                stRcParam.stParamH265Vbr.u32MaxQp = pstVecncChnCtx->stRcParam.u8MaxQp;
                stRcParam.stParamH265Vbr.u32MinIQp =  pstVecncChnCtx->stRcParam.u8MinIqp;
                stRcParam.stParamH265Vbr.u32MinQp = pstVecncChnCtx->stRcParam.u8MinQp;
                stRcParam.stParamH265Vbr.s32ChangePos = pstVecncChnCtx->stRcParam.u8ChangePos;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265AVBR) {
                stRcParam.stParamH265AVbr.bQpMapEn = CVI_FALSE;
                stRcParam.stParamH265AVbr.s32MaxReEncodeTimes = pstVecncChnCtx->stRcParam.u8MaxReEncodeTimes;
                stRcParam.stParamH265AVbr.u32MaxIprop = pstVecncChnCtx->stRcParam.u8MaxIprop;
                stRcParam.stParamH265AVbr.u32MinIprop = pstVecncChnCtx->stRcParam.u8MinIprop;
                stRcParam.stParamH265AVbr.u32MaxIQp = pstVecncChnCtx->stRcParam.u8MaxIqp;
                stRcParam.stParamH265AVbr.u32MaxQp = pstVecncChnCtx->stRcParam.u8MaxQp;
                stRcParam.stParamH265AVbr.u32MinIQp =  pstVecncChnCtx->stRcParam.u8MinIqp;
                stRcParam.stParamH265AVbr.u32MinQp = pstVecncChnCtx->stRcParam.u8MinQp;
                stRcParam.stParamH265AVbr.s32ChangePos = pstVecncChnCtx->stRcParam.u8ChangePos;
                stRcParam.stParamH265AVbr.s32MinStillPercent = pstVecncChnCtx->stRcParam.s32MinStillPercent;
                stRcParam.stParamH265AVbr.u32MaxStillQP = pstVecncChnCtx->stRcParam.u32MaxStillQP;
                stRcParam.stParamH265AVbr.u32MotionSensitivity = pstVecncChnCtx->stRcParam.u32MotionSensitivity;
                stRcParam.stParamH265AVbr.s32AvbrFrmLostOpen = pstVecncChnCtx->stRcParam.s32AvbrFrmLostOpen;
                stRcParam.stParamH265AVbr.s32AvbrFrmGap = pstVecncChnCtx->stRcParam.s32AvbrFrmGap;
                stRcParam.stParamH265AVbr.s32AvbrPureStillThr = pstVecncChnCtx->stRcParam.s32AvbrPureStillThr;
            } else {
                return CVI_FAILURE;
            }
            break;
        }

        case PT_MJPEG: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGCBR) {
                stRcParam.stParamMjpegCbr.u32MaxQfactor = pstVecncChnCtx->stRcParam.u8MaxQfactor;
                stRcParam.stParamMjpegCbr.u32MinQfactor = pstVecncChnCtx->stRcParam.u8MinQfactor;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGFIXQP) {
                break;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGVBR) {
                stRcParam.stParamMjpegVbr.u32MaxQfactor = pstVecncChnCtx->stRcParam.u8MaxQfactor;
                stRcParam.stParamMjpegVbr.u32MinQfactor = pstVecncChnCtx->stRcParam.u8MinQfactor;
                stRcParam.stParamMjpegVbr.s32ChangePos  = pstVecncChnCtx->stRcParam.u8ChangePos;
            } else {
                return CVI_FAILURE;
            }
            break;
        }

        case PT_JPEG: {
            // nothing
            break;
        }

        default : {
            return -1;
        }
    }

    MEDIA_CHECK_RET(CVI_VENC_SetRcParam(VencChn, &stRcParam), "CVI_VENC_SetRcParam");

    MEDIA_CHECK_RET(CVI_VENC_GetRefParam(VencChn, &stRefParam), "CVI_VENC_GetRefParam");
    stRefParam.bEnablePred = 0;
    stRefParam.u32Base = 0;
    stRefParam.u32Enhance = 0;
    MEDIA_CHECK_RET(CVI_VENC_SetRefParam(VencChn, &stRefParam), "CVI_VENC_SetRefParam");

    MEDIA_CHECK_RET(CVI_VENC_GetChnParam(VencChn, &stChnParam), "CVI_VENC_GetChnParam");
    stChnParam.stFrameRate.s32SrcFrmRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
    stChnParam.stFrameRate.s32DstFrmRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;

    MEDIA_CHECK_RET(CVI_VENC_SetChnParam(VencChn, &stChnParam), "CVI_VENC_SetChnParam");
//ROI 暂时不设置
    MEDIA_CHECK_RET(CVI_VENC_GetRoiAttr(VencChn, 0, &stRoiAttr), "CVI_VENC_GetRoiAttr");
    stRoiAttr.bEnable = CVI_FALSE;
    stRoiAttr.bAbsQp = CVI_FALSE;
    stRoiAttr.s32Qp = -2;
    stRoiAttr.u32Index = 0;
    MEDIA_CHECK_RET(CVI_VENC_SetRoiAttr(VencChn, &stRoiAttr), "CVI_VENC_SetRoiAttr");

    if (pstVecncChnCtx->stChnParam.u16EnType == PT_H265) {

        MEDIA_CHECK_RET(CVI_VENC_GetH265Vui(VencChn, &stH265Vui), "CVI_VENC_GetH265Vui");
        stH265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_DEFAULT;
        if (stH265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag) {
            stH265Vui.stVuiAspectRatio.aspect_ratio_idc = CVI_H26X_ASPECT_RATIO_IDC_DEFAULT;
            stH265Vui.stVuiAspectRatio.sar_width = CVI_H26X_SAR_WIDTH_DEFAULT;
            stH265Vui.stVuiAspectRatio.sar_height = CVI_H26X_SAR_HEIGHT_DEFAULT;
        }

        stH265Vui.stVuiAspectRatio.overscan_info_present_flag = CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_DEFAULT;
        if (stH265Vui.stVuiAspectRatio.overscan_info_present_flag) {
            stH265Vui.stVuiAspectRatio.overscan_appropriate_flag = CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_DEFAULT;
        }

        stH265Vui.stVuiTimeInfo.timing_info_present_flag = 1;
        if (stH265Vui.stVuiTimeInfo.timing_info_present_flag) {
            stH265Vui.stVuiTimeInfo.num_units_in_tick = 1;
            //265 fps = time_scale / num_units_in_tick
            stH265Vui.stVuiTimeInfo.time_scale = pstVecncChnCtx->stRcParam.u8DstFrameRate * 1;
        }

        stH265Vui.stVuiVideoSignal.video_signal_type_present_flag = CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_DEFAULT;
        if (stH265Vui.stVuiVideoSignal.video_signal_type_present_flag) {
            stH265Vui.stVuiVideoSignal.video_format = CVI_H26X_VIDEO_FORMAT_DEFAULT;
            stH265Vui.stVuiVideoSignal.video_full_range_flag = CVI_H26X_VIDEO_FULL_RANGE_FLAG_DEFAULT;
            stH265Vui.stVuiVideoSignal.colour_description_present_flag = CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_DEFAULT;
            if (stH265Vui.stVuiVideoSignal.colour_description_present_flag) {
                stH265Vui.stVuiVideoSignal.colour_primaries = CVI_H26X_COLOUR_PRIMARIES_DEFAULT;
                stH265Vui.stVuiVideoSignal.transfer_characteristics = CVI_H26X_TRANSFER_CHARACTERISTICS_DEFAULT;
                stH265Vui.stVuiVideoSignal.matrix_coefficients = CVI_H26X_MATRIX_COEFFICIENTS_DEFAULT;
            }
        }

        MEDIA_CHECK_RET(CVI_VENC_SetH265Vui(VencChn, &stH265Vui), "CVI_VENC_SetH265Vui");

    } else if (pstVecncChnCtx->stChnParam.u16EnType == PT_H264) {

        MEDIA_CHECK_RET(CVI_VENC_GetH264Entropy(VencChn, &stH264EntropyEnc), "CVI_VENC_GetH264Entropy");
        stH264EntropyEnc.u32EntropyEncModeI = pstVecncChnCtx->stChnParam.u8EntropyEncModeI;
        stH264EntropyEnc.u32EntropyEncModeP = pstVecncChnCtx->stChnParam.u8EntropyEncModeP;
        MEDIA_CHECK_RET(CVI_VENC_SetH264Entropy(VencChn, &stH264EntropyEnc), "CVI_VENC_SetH264Entropy");

        MEDIA_CHECK_RET(CVI_VENC_GetH264Vui(VencChn, &stH264Vui), "CVI_VENC_GetH264Vui");
        stH264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_DEFAULT;
        if (stH264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag) {
            stH264Vui.stVuiAspectRatio.aspect_ratio_idc = CVI_H26X_ASPECT_RATIO_IDC_DEFAULT;
            stH264Vui.stVuiAspectRatio.sar_width = CVI_H26X_SAR_WIDTH_DEFAULT;
            stH264Vui.stVuiAspectRatio.sar_height = CVI_H26X_SAR_HEIGHT_DEFAULT;
        }

        stH264Vui.stVuiAspectRatio.overscan_info_present_flag = CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_DEFAULT;
        if (stH264Vui.stVuiAspectRatio.overscan_info_present_flag) {
            stH264Vui.stVuiAspectRatio.overscan_appropriate_flag = CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_DEFAULT;
        }

        stH264Vui.stVuiTimeInfo.timing_info_present_flag = 1;
        if (stH264Vui.stVuiTimeInfo.timing_info_present_flag) {
            stH264Vui.stVuiTimeInfo.num_units_in_tick = 1;
            //265 fps = time_scale / num_units_in_tick
            stH264Vui.stVuiTimeInfo.time_scale = pstVecncChnCtx->stRcParam.u8DstFrameRate * 1;
        }

        stH264Vui.stVuiVideoSignal.video_signal_type_present_flag = CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_DEFAULT;
        if (stH264Vui.stVuiVideoSignal.video_signal_type_present_flag) {
            stH264Vui.stVuiVideoSignal.video_format = CVI_H26X_VIDEO_FORMAT_DEFAULT;
            stH264Vui.stVuiVideoSignal.video_full_range_flag = CVI_H26X_VIDEO_FULL_RANGE_FLAG_DEFAULT;
            stH264Vui.stVuiVideoSignal.colour_description_present_flag = CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_DEFAULT;
            if (stH264Vui.stVuiVideoSignal.colour_description_present_flag) {
                stH264Vui.stVuiVideoSignal.colour_primaries = CVI_H26X_COLOUR_PRIMARIES_DEFAULT;
                stH264Vui.stVuiVideoSignal.transfer_characteristics = CVI_H26X_TRANSFER_CHARACTERISTICS_DEFAULT;
                stH264Vui.stVuiVideoSignal.matrix_coefficients = CVI_H26X_MATRIX_COEFFICIENTS_DEFAULT;
            }
        }
        MEDIA_CHECK_RET(CVI_VENC_SetH264Vui(VencChn, &stH264Vui), "CVI_VENC_SetH264Vui");
    } else if (pstVecncChnCtx->stChnParam.u16EnType == PT_JPEG) {
        MEDIA_CHECK_RET(CVI_VENC_GetJpegParam(VencChn, &stJpegParam), "CVI_VENC_GetJpegParam");
        stJpegParam.u32Qfactor = pstVecncChnCtx->stRcParam.s16Quality;
        stJpegParam.u32MCUPerECS = pstVecncChnCtx->stRcParam.s16MCUPerECS;
        MEDIA_CHECK_RET(CVI_VENC_SetJpegParam(VencChn, &stJpegParam), "CVI_VENC_SetJpegParam");
    }

    if(pstVecncChnCtx->stChnParam.astChn[0].enModId == CVI_ID_VPSS || pstVecncChnCtx->stChnParam.astChn[0].enModId == CVI_ID_VI ) {
        stDestChn.enModId = pstVecncChnCtx->stChnParam.astChn[1].enModId;
        stDestChn.s32DevId = pstVecncChnCtx->stChnParam.astChn[1].s32DevId;
        stDestChn.s32ChnId = pstVecncChnCtx->stChnParam.astChn[1].s32ChnId;
        stSrcChn.enModId = pstVecncChnCtx->stChnParam.astChn[0].enModId;
        stSrcChn.s32DevId = pstVecncChnCtx->stChnParam.astChn[0].s32DevId;
        stSrcChn.s32ChnId = pstVecncChnCtx->stChnParam.astChn[0].s32ChnId;
        stRecvParam.s32RecvPicNum = -1;

        CVI_BOOL bSbmMode = CVI_FALSE;
        // Check if the current channel is one of the SBM channels
        for (int idx = 0; idx < pstSysCtx->u8SbmCnt; ++idx) {
            if ((stSrcChn.s32DevId == pstSysCtx->pstSbmCfg[idx].s32SbmGrp) &&
                (stSrcChn.s32ChnId == pstSysCtx->pstSbmCfg[idx].s32SbmChn)) {
                bSbmMode = CVI_TRUE;
                break; // No need to check further if a match is found
            }
        }

        // If in SBM mode, start receiving frames and then bind
        // Otherwise, bind first and then start receiving frames
        if (bSbmMode) {
            MEDIA_CHECK_RET(CVI_VENC_StartRecvFrame(VencChn, &stRecvParam), "CVI_VENC_StartRecvFrame");
            if (pstVecncChnCtx->stChnParam.enBindMode) {
                MEDIA_CHECK_RET(CVI_SYS_Bind(&stSrcChn, &stDestChn), "CVI_SYS_Bind err");
            }
        } else {
            if (pstVecncChnCtx->stChnParam.enBindMode) {
                MEDIA_CHECK_RET(CVI_SYS_Bind(&stSrcChn, &stDestChn), "CVI_SYS_Bind err");
            }
            MEDIA_CHECK_RET(CVI_VENC_StartRecvFrame(VencChn, &stRecvParam), "CVI_VENC_StartRecvFrame");
        }
    }
    pstVecncChnCtx->stChnParam.bStart = CVI_TRUE;

    return CVI_SUCCESS;
}

int MEDAI_VIDEO_VencChnDeinit(PARAM_VENC_CFG_S *pstVencCfg, int VencChn)
{
    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDestChn;
    PARAM_VENC_CHN_CFG_S *pstVecncChnCtx = NULL;

    if(!pstVencCfg) {
        return CVI_FAILURE;
    }
    if (VencChn >= pstVencCfg->s32VencChnCnt) {
        return CVI_FAILURE;
    }

    pstVecncChnCtx = &pstVencCfg->pstVencChnCfg[VencChn];
    if(pstVecncChnCtx->stChnParam.astChn[0].enModId == CVI_ID_VPSS || pstVecncChnCtx->stChnParam.astChn[0].enModId == CVI_ID_VI ) {
        stDestChn.enModId = pstVecncChnCtx->stChnParam.astChn[1].enModId;
        stDestChn.s32DevId = pstVecncChnCtx->stChnParam.astChn[1].s32DevId;
        stDestChn.s32ChnId = pstVecncChnCtx->stChnParam.astChn[1].s32ChnId;
        stSrcChn.enModId = pstVecncChnCtx->stChnParam.astChn[0].enModId;
        stSrcChn.s32DevId = pstVecncChnCtx->stChnParam.astChn[0].s32DevId;
        stSrcChn.s32ChnId = pstVecncChnCtx->stChnParam.astChn[0].s32ChnId;
        MEDIA_CHECK_RET(CVI_SYS_UnBind(&stSrcChn, &stDestChn), "CVI_SYS_UnBind err");
    }

    MEDIA_CHECK_RET(CVI_VENC_StopRecvFrame(pstVecncChnCtx->stChnParam.u8VencChn), "CVI_VENC_StopRecvFrame");

    MEDIA_CHECK_RET(CVI_VENC_ResetChn(pstVecncChnCtx->stChnParam.u8VencChn), "CVI_VENC_ResetChn");

    MEDIA_CHECK_RET(CVI_VENC_DestroyChn(pstVecncChnCtx->stChnParam.u8VencChn), "CVI_VENC_DestroyChn");
    pstVecncChnCtx->stChnParam.bStart = CVI_FALSE;
    return CVI_SUCCESS;
}

int MEDIA_VIDEO_VencInit(PARAM_VENC_CFG_S *pstVencCfg)
{
    if(!pstVencCfg) {
        MEDIABUG_PRINTF("********MEDIA_VIDEO_VencInit pstVencCfg NULL err \n");
        return CVI_FAILURE;
    }
    for(int i = 0 ; i < pstVencCfg->s32VencChnCnt; i++) {
        if(MEDIA_VIDEO_VencChnInit(pstVencCfg,i) != CVI_SUCCESS) {
            MEDIABUG_PRINTF("MEDIA_VIDEO_VencChnInit %d err \n",i);
        }
    }
    g_pstVencCfg = pstVencCfg;
    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_VencInit()
{
#if (CONFIG_APP_VENC_SUPPORT == 0)
    return CVI_SUCCESS;
#endif
    PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
    return MEDIA_VIDEO_VencInit(pstVencCfg);
}

int MEDIA_VIDEO_VencDeInit(PARAM_VENC_CFG_S *pstVencCfg)
{
    if(!pstVencCfg) {
        return CVI_FAILURE;
    }
    for(int i = 0 ; i < pstVencCfg->s32VencChnCnt; i++) {
        MEDAI_VIDEO_VencChnDeinit(pstVencCfg,i);
    }
    g_pstVencCfg = NULL;
    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_VencDeInit()
{
#if (CONFIG_APP_VENC_SUPPORT == 0)
    return CVI_SUCCESS;
#endif
    PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
    return MEDIA_VIDEO_VencDeInit(pstVencCfg);
}

int MEDIA_VIDEO_VencGetStream(int VencChn,VENC_STREAM_S *pstStreamFrame,unsigned int blocktimeMs)
{
    CVI_S32 s32ret = CVI_SUCCESS;
    PARAM_VENC_CFG_S *pstVencCfg = g_pstVencCfg;
    PARAM_VENC_CHN_CFG_S *pstVecncChnCtx = NULL;
    VIDEO_FRAME_INFO_S stSrcFrame = {0};
    VENC_CHN_STATUS_S stStatus = {0};

    if(!pstVencCfg) {
        return CVI_FAILURE;
    }
    if (VencChn >= pstVencCfg->s32VencChnCnt) {
        return CVI_FAILURE;
    }
    if(pstVencCfg->pstVencChnCfg[VencChn].stChnParam.bStart == CVI_FALSE) {
        return CVI_FAILURE;
    }
    pstVecncChnCtx = &pstVencCfg->pstVencChnCfg[VencChn];

    if(pstVecncChnCtx->stChnParam.astChn[0].enModId != CVI_ID_VPSS && pstVecncChnCtx->stChnParam.astChn[0].enModId != CVI_ID_VI ) {
        MEDIA_CHECK_RET(CVI_VPSS_GetChnFrame(pstVecncChnCtx->stChnParam.astChn[0].s32DevId, pstVecncChnCtx->stChnParam.astChn[0].s32ChnId, &stSrcFrame, 2000), "CVI_VPSS_GetChnFrame");
        s32ret = CVI_VENC_SendFrame(VencChn, &stSrcFrame, 2000);
        if(s32ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("%d CVI_VENC_SendFrame err \n",VencChn);
            goto EXIT;
        }
    }
    MEDIA_CHECK_RET(CVI_VENC_QueryStatus(VencChn, &stStatus), "CVI_VENC_QueryStatus");
    if(stStatus.u32LeftStreamFrames <= 0) {
        s32ret = CVI_FAILURE;
        goto EXIT;
    }
    pstStreamFrame->pstPack = malloc(sizeof(VENC_PACK_S) * stStatus.u32CurPacks);
    if (pstStreamFrame->pstPack == NULL) {
        MEDIABUG_PRINTF("stStream.pstPack, malloc memory failed\n");
        s32ret = CVI_FAILURE;
        goto EXIT;
    }

    s32ret = CVI_VENC_GetStream(VencChn, pstStreamFrame, blocktimeMs);
    if (s32ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("CVI_VENC_GetStream ret=0x%x\n", s32ret);
        if(pstStreamFrame->pstPack) {
            free(pstStreamFrame->pstPack);
            pstStreamFrame->pstPack = NULL;
        }
    }
EXIT:
    if(pstVecncChnCtx->stChnParam.astChn[0].enModId != CVI_ID_VPSS && pstVecncChnCtx->stChnParam.astChn[0].enModId != CVI_ID_VI ) {
        MEDIA_CHECK_RET(CVI_VPSS_ReleaseChnFrame(pstVecncChnCtx->stChnParam.astChn[0].s32DevId, pstVecncChnCtx->stChnParam.astChn[0].s32ChnId, &stSrcFrame), "CVI_VPSS_ReleaseChnFrame");
    }
    return s32ret;
}

int MEDIA_VIDEO_VencReleaseStream(int VencChn,VENC_STREAM_S *pstStreamFrame)
{
    CVI_S32 s32ret = CVI_SUCCESS;
    PARAM_VENC_CFG_S *pstVencCfg = g_pstVencCfg;

    if(!pstVencCfg ) {
        return CVI_FAILURE;
    }
    if (VencChn >= pstVencCfg->s32VencChnCnt ) {
        return CVI_FAILURE;
    }
    if(pstVencCfg->pstVencChnCfg[VencChn].stChnParam.bStart == CVI_FALSE) {
        return CVI_FAILURE;
    }
    s32ret = CVI_VENC_ReleaseStream(VencChn, pstStreamFrame);
    if (pstStreamFrame->pstPack != NULL) {
        free(pstStreamFrame->pstPack);
        pstStreamFrame->pstPack = NULL;
    }
    return s32ret;
}

int MEDIA_VIDEO_VencRequstIDR(int VencChn)
{
    return CVI_VENC_RequestIDR(VencChn,0);
}

#if CONFIG_DUALOS_NO_CROP
#if (CONFIG_RTOS_INIT_MEDIA == 1)
void efuse_fastboot()
{
    csi_efuse_t efuse = {0};
    csi_efuse_init(&efuse, 0);
    int ret = CVI_EFUSE_EnableFastBoot();
    if (ret == CVI_SUCCESS) {
        printf("fast boot enable\n");
    }else {
        printf("CVI_EFUSE_EnableFastBoot ret=%d\n", ret);
    }

    ret = CVI_EFUSE_IsFastBootEnabled();
    if (ret == CVI_SUCCESS) {
        printf("fast boot enable\n");
    }
    else {
        printf("CVI_EFUSE_IsFastBootEnabled ret=%d\n", ret);
    }

    csi_efuse_uninit(&efuse);
}
#endif
#endif //#if CONFIG_DUALOS_NO_CROP


int MEDIA_VIDEO_Init(CVI_BOOL isRawReplayMode)
{
    PARAM_MODULE_CFG_S * pstModuleCtx = PARAM_getModuleCtx();

    if (pstModuleCtx == NULL) {
        MEDIABUG_PRINTF("pstModuleCtx null err\n");
        return CVI_FAILURE;
    }
#if CONFIG_SUPPORT_VI
    if (isRawReplayMode) {
        PARAM_Reinit_RawReplay();
    }
#endif
    //calling sys init after deinit when raw replay
    if (!isRawReplayMode && pstModuleCtx->alios_sys_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_SysVbInit(),"MEDIA_VIDEO_SysVbInit failed");
    }
#if CONFIG_SUPPORT_VI
    if (pstModuleCtx->alios_vi_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_ViInit(),"MEDIA_VIDEO_ViInit failed");
    }
#endif
    if (pstModuleCtx->alios_vpss_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_VpssInit(),"MEDIA_VIDEO_VpssInit failed");
    }

#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
    if (!isRawReplayMode && pstModuleCtx->alios_vo_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_VoInit(),"MEDIA_VIDEO_VoInit failed");
    }
#endif

    if (pstModuleCtx->alios_venc_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_VencInit(),"MEDIA_VIDEO_VencInit failed");
    }

#if (CONFIG_APP_AI_SUPPORT == 1)
    if (!isRawReplayMode) {
        APP_AiStart();
    }
#endif

#if (CONFIG_RTOS_INIT_MEDIA == 1)
    //efuse_fastboot();
#endif
    return CVI_SUCCESS;
}

int MEDIA_VIDEO_Deinit(CVI_BOOL isRawReplayMode)
{
    PARAM_MODULE_CFG_S * pstModuleCtx = PARAM_getModuleCtx();

    if (pstModuleCtx == NULL) {
        MEDIABUG_PRINTF("pstModuleCtx null err\n");
        return CVI_FAILURE;
    }

#if (CONFIG_APP_AI_SUPPORT == 1)
    APP_AiStop();
#endif

    if (pstModuleCtx->alios_venc_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_VencDeInit(),"MEDIA_VIDEO_VencDeInit failed");
    }

#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
    if (pstModuleCtx->alios_vo_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_VoDeinit(),"MEDIA_VIDEO_VoDeinit failed");
    }
#endif

    if (pstModuleCtx->alios_vpss_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_VpssDeinit(),"MEDIA_VIDEO_VpssDeinit failed");
    }
#if CONFIG_SUPPORT_VI
    if (pstModuleCtx->alios_vi_mode) {
        MEDIA_CHECK_RET(_MEDIA_VIDEO_ViDeInit(),"MEDIA_VIDEO_ViDeInit failed");
    }
#endif
    if (pstModuleCtx->alios_sys_mode) {
        MEDIA_CHECK_RET(MEDIA_VIDEO_SysVbDeinit(),"MEDIA_VIDEO_SysVbDeinit failed");
    }
#if CONFIG_SUPPORT_VI
    if (isRawReplayMode) {//calling sys init after deinit for that mallocing vb for raw
        PARAM_Reinit_RawReplay();
        if (pstModuleCtx->alios_sys_mode) {
            MEDIA_CHECK_RET(_MEDIA_VIDEO_SysVbInit(),"MEDIA_VIDEO_SysVbInit failed");
        }
    }
#endif
    return CVI_SUCCESS;
}

void testMedia_video_Deinit(int32_t argc, char **argv)
{
    MEDIA_VIDEO_Deinit(0);
}

void testMedia_video_init(int32_t argc, char **argv)
{
    MEDIA_VIDEO_Init(0);
}

void testMedia_switch_pipeline(int32_t argc, char **argv)
{
    if(argc < 2) {
        printf("please input 0/1 chose RGB or IR\n");
        printf("testMedia_switch_pipeline 0/1 \n");
        return ;
    }
    if(atoi(argv[1]) == 0 || atoi(argv[1]) == 1)
        PARAM_setPipeline(atoi(argv[1]));
}

ALIOS_CLI_CMD_REGISTER(testMedia_video_init, testMedia_video_init, testMedia_video_init);
ALIOS_CLI_CMD_REGISTER(testMedia_video_Deinit, testMedia_video_Deinit, testMedia_video_Deinit);
ALIOS_CLI_CMD_REGISTER(testMedia_switch_pipeline, testMedia_switch_pipeline, testMedia_switch_pipeline);
