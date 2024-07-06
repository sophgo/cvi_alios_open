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
#include "common_vi.h"
#include "cvi_type.h"
#include "cvi_param.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cvi_isp.h"
#include "cvi_buffer.h"
#include "cvi_math.h"
#include "cif_uapi.h"
#include "cvi_sns_ctrl.h"
#include "vi_isp.h"
#include "cvi_mipi_tx.h"
#include "dsi_panels.h"
#include "ldc_platform.h"
#include "cvi_ive_interface.h"

#include "cvi_vo.h"
#include "cvi_venc.h"
#include "vi_snsr_i2c.h"
#include "cvi_region.h"

#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_awb.h"
#include "cvi_ae.h"
#include "cvi_bin.h"
#include "app_ai.h"
#include "drv/common.h"
#include "drv/efuse.h"

#include "cvi_efuse.h"
#include "cvi_misc.h"
#include "cvi_sys.h"
#include "gui_display.h"

#include "sensor_i2c.h"

#if CONFIG_APP_DUMP_FRAME
#include <drv/tick.h>
#include "fatfs_vfs.h"
#include "vfs.h"
#endif

static PARAM_VENC_CFG_S *g_pstVencCfg = NULL;
static int g_mediaVideoRunStatus = 0;

static int start_isp(ISP_PUB_ATTR_S stPubAttr, VI_PIPE ViPipe)
{
	//Param init
	CVI_S32 s32Ret;
	ISP_STATISTICS_CFG_S stsCfg = {0};
	CVI_ISP_GetStatisticsConfig(0, &stsCfg);
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
	//Register callback & call API
	ALG_LIB_S stAeLib, stAwbLib;
	stAeLib.s32Id = stAwbLib.s32Id = ViPipe;
	ISP_BIND_ATTR_S stBindAttr;
	stBindAttr.stAeLib.s32Id = stBindAttr.stAwbLib.s32Id = ViPipe;

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

#if(CONFIG_APP_ISP_BYPASS == 0)
    CVI_ISP_SetBypassFrm(0, 0);
#endif

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

static int stop_isp(VI_PIPE ViPipe)
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


static int _meida_sensor_init(PARAM_VI_CFG_S * pstViCtx,CVI_U8 *devNum)
{
    //Sensor
    SNS_COMBO_DEV_ATTR_S devAttr = {0};
    CVI_S32 snsr_type[VI_MAX_DEV_NUM];
    VI_DEV ViDev;
    ISP_SNS_OBJ_S *pSnsObj[VI_MAX_DEV_NUM];
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode[VI_MAX_DEV_NUM];
    CVI_U8 dev_num;
    ISP_SNS_COMMBUS_U unSnsrBusInfo = {0};
    ISP_SENSOR_EXP_FUNC_S stSnsrSensorFunc[VI_MAX_DEV_NUM] = {0};
    ISP_INIT_ATTR_S InitAttr = {0};
    ALG_LIB_S stAeLib = {0};
    ALG_LIB_S stAwbLib = {0};
    CVI_S32 s32Ret;
#if (!CONFIG_SENSOR_QUICK_STARTUP)
    struct snsr_rst_gpio_s snsr_gpio;
#endif
    if(pstViCtx == NULL) {
        MEDIABUG_PRINTF("%s pstViCtx NULL err \n",__func__);
        return CVI_FAILURE;
    }
    MEDIA_CHECK_RET(getSnsType(snsr_type, &dev_num), "getSnsType fail");
    for (CVI_U8 i = 0; i < *devNum; i++) {
        pSnsObj[i] = getSnsObj(snsr_type[i]);
        MEDIA_CHECK_RET(getSnsMode(i, &stSnsrMode[i]), "stSnsrMode fail");
    }
    /* clock enable */
    // vip_clk_en();
    /************************************************
     * start sensor
     ************************************************/
    InitAttr.enGainMode = SNS_GAIN_MODE_SHARE;

    for (CVI_U8  i = 0; i < *devNum; ++i) {
        ViDev = pstViCtx->pstDevInfo[i].u8AttachDev > 0 ?
        VI_MAX_PHY_DEV_NUM + pstViCtx->pstDevInfo[i].u8AttachDev - 1 : i;

        if (!pSnsObj[i]) {
            MEDIABUG_PRINTF("sns obj[%d] is null.\n", i);
            return CVI_FAILURE;
        }
        pstViCtx->pstSensorCfg[i].pSnsObj = pSnsObj[i];
        unSnsrBusInfo.s8I2cDev = pstViCtx->pstSensorCfg[i].s8I2cDev;
    #if (!CONFIG_SENSOR_QUICK_STARTUP)
        snsr_gpio.snsr_rst_port_idx = pstViCtx->pstSensorCfg[i].u32Rst_port_idx;
        snsr_gpio.snsr_rst_pin = pstViCtx->pstSensorCfg[i].u32Rst_pin;
        snsr_gpio.snsr_rst_pol = pstViCtx->pstSensorCfg[i].u32Rst_pol;
        cvi_cif_reset_snsr_gpio_init(i, &snsr_gpio);
    #endif
        InitAttr.u16UseHwSync = pstViCtx->pstSensorCfg[i].bHwSync;
        InitAttr.bInitByUser =  pstViCtx->pstSensorCfg[i].bSnsInitByUser;
        if(pstViCtx->pstSensorCfg[i].s32I2cAddr != -1) {
            if(pSnsObj[i]->pfnPatchI2cAddr != NULL)
            pSnsObj[i]->pfnPatchI2cAddr(pstViCtx->pstSensorCfg[i].s32I2cAddr);
        }
        pSnsObj[i]->pfnSetInit(ViDev, &InitAttr);
        MEDIABUG_PRINTF("bus info:%d\n", unSnsrBusInfo.s8I2cDev);
        pSnsObj[i]->pfnSetBusInfo(ViDev, unSnsrBusInfo);
        pSnsObj[i]->pfnRegisterCallback(ViDev, &stAeLib, &stAwbLib);
        pSnsObj[i]->pfnExpSensorCb(&stSnsrSensorFunc[i]);
        stSnsrSensorFunc[i].pfn_cmos_sensor_global_init(ViDev);
        s32Ret = stSnsrSensorFunc[i].pfn_cmos_set_image_mode(ViDev, &stSnsrMode[i]);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("sensor set image mode failed!\n");
            return CVI_FAILURE;
        }
        s32Ret = stSnsrSensorFunc[i].pfn_cmos_set_wdr_mode(ViDev, stSnsrMode[i].u8SnsMode);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("sensor set wdr mode failed!\n");
            return CVI_FAILURE;
        }

        if(pstViCtx->pstSensorCfg[i].u8DisableRst != CVI_TRUE) {
        #if (!CONFIG_SENSOR_QUICK_STARTUP)
            cif_reset_snsr_gpio(i, 1);
        #endif
        }

        /*virtual dev do not config cif again*/
        if (ViDev >= VI_MAX_PHY_DEV_NUM) {
            continue;
        }

        cif_reset_mipi(i);
        udelay(100);
        pSnsObj[i]->pfnGetRxAttr(i, &devAttr);
        if (pstViCtx->pstSensorCfg[i].bSetDevAttrMipi != 0) {
            for (CVI_U8 j = 0; j < MIPI_LANE_NUM+1; j++) {
                devAttr.mipi_attr.lane_id[j] = pstViCtx->pstSensorCfg[i].as16LaneId[j];
                devAttr.mipi_attr.pn_swap[j] = pstViCtx->pstSensorCfg[i].as8PNSwap[j];
            }
            devAttr.mipi_attr.wdr_mode = pstViCtx->pstSensorCfg[i].s32WDRMode;
        }
        if(pstViCtx->pstSensorCfg[i].bSetDevAttr != 0) {
            devAttr.devno = i;
            devAttr.mac_clk = pstViCtx->pstSensorCfg[i].s16MacClk;
            devAttr.mclk.cam = pstViCtx->pstSensorCfg[i].u8MclkCam;
            devAttr.mclk.freq = pstViCtx->pstSensorCfg[i].u8MclkFreq;
        }
        cif_set_dev_attr(&devAttr);
    }
    for (CVI_U8  i = 0; i < *devNum; ++i) {
        ViDev = pstViCtx->pstDevInfo[i].u8AttachDev > 0 ?
                VI_MAX_PHY_DEV_NUM + pstViCtx->pstDevInfo[i].u8AttachDev - 1 : i;
            if(pstViCtx->pstSensorCfg[i].u8DisableRst != CVI_TRUE) {
                if (ViDev < VI_MAX_PHY_DEV_NUM) {
                    pSnsObj[i]->pfnGetRxAttr(i, &devAttr);
                    cif_enable_snsr_clk(i, 1);
                    usleep(100);
                }
            #if (!CONFIG_SENSOR_QUICK_STARTUP)
                cif_reset_snsr_gpio(i, 0);
                udelay(100);
            #endif
        }

        if (pSnsObj[i]->pfnSnsProbe && !pstViCtx->pstSensorCfg[i].bSnsInitByUser) {
            s32Ret = pSnsObj[i]->pfnSnsProbe(ViDev);
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
static int setFastConvergeAttr(VI_PIPE ViPipe, CVI_BOOL en)
{
    CVI_S16 firstFrLuma[5] = {62, 77, 173, 343, 724};
	CVI_S16 targetBv[5] = {89, 194, 479, 533, 721};
    ISP_AE_BOOT_FAST_CONVERGE_S stConvergeAttr;

    stConvergeAttr.bEnable = en;
    stConvergeAttr.availableNode = 5;
    memcpy(stConvergeAttr.firstFrLuma, firstFrLuma,sizeof(firstFrLuma));
    memcpy(stConvergeAttr.targetBv, targetBv, sizeof(targetBv));
    CVI_ISP_SetFastConvergeAttr(ViPipe, &stConvergeAttr);
    return CVI_SUCCESS;
}


#if CONFIG_APP_DUMP_FRAME
static void getFmtName(PIXEL_FORMAT_E enPixFmt, CVI_CHAR *szName)
{
	switch (enPixFmt)
	{
		case PIXEL_FORMAT_RGB_888:
			snprintf(szName, 10, "rgb");
			break;
		case PIXEL_FORMAT_BGR_888:
			snprintf(szName, 10, "bgr");
			break;
		case PIXEL_FORMAT_RGB_888_PLANAR:
			snprintf(szName, 10, "rgbm");
			break;
		case PIXEL_FORMAT_BGR_888_PLANAR:
			snprintf(szName, 10, "bgrm");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_422:
			snprintf(szName, 10, "p422");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_420:
			snprintf(szName, 10, "p420");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_444:
			snprintf(szName, 10, "p444");
			break;
		case PIXEL_FORMAT_YUV_400:
			snprintf(szName, 10, "y");
			break;
		case PIXEL_FORMAT_HSV_888:
			snprintf(szName, 10, "hsv");
			break;
		case PIXEL_FORMAT_HSV_888_PLANAR:
			snprintf(szName, 10, "hsvm");
			break;
		case PIXEL_FORMAT_NV12:
			snprintf(szName, 10, "nv12");
			break;
		case PIXEL_FORMAT_NV21:
			snprintf(szName, 10, "nv21");
			break;
		case PIXEL_FORMAT_NV16:
			snprintf(szName, 10, "nv16");
			break;
		case PIXEL_FORMAT_NV61:
			snprintf(szName, 10, "nv61");
			break;
		case PIXEL_FORMAT_YUYV:
			snprintf(szName, 10, "yuyv");
			break;
		case PIXEL_FORMAT_UYVY:
			snprintf(szName, 10, "uyvy");
			break;
		case PIXEL_FORMAT_YVYU:
			snprintf(szName, 10, "yvyu");
			break;
		case PIXEL_FORMAT_VYUY:
			snprintf(szName, 10, "vyuy");
			break;

		default:
			snprintf(szName, 10, "unknown");
			break;
	}

}

static CVI_S32 _getFrame(MMF_CHN_S *pstChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
    if (pstChn->enModId == CVI_ID_VI) {
        return CVI_VI_GetChnFrame(pstChn->s32DevId, pstChn->s32ChnId, pstFrameInfo, s32MilliSec);
    }
    else {
        return CVI_VPSS_GetChnFrame(pstChn->s32DevId, pstChn->s32ChnId, pstFrameInfo, s32MilliSec);
    }
}

static CVI_S32 _releaseFrame(MMF_CHN_S *pstChn, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    if (pstChn->enModId == CVI_ID_VI) {
        return CVI_VI_ReleaseChnFrame(pstChn->s32DevId, pstChn->s32ChnId, pstFrameInfo);
    }
    else {
        return CVI_VI_ReleaseChnFrame(pstChn->s32DevId, pstChn->s32ChnId, pstFrameInfo);
    }
}

static void dumpFrame(MMF_CHN_S *pstChn, CVI_U32 u32FrameCnt)
{
	CVI_S32 s32MilliSec = 1000;
	CVI_U32 u32Cnt = u32FrameCnt;
	CVI_CHAR szFrameName[128], szPixFrm[10];
	CVI_BOOL bFlag = CVI_TRUE;
	int fd = 0;
	CVI_S32 i;
    VI_DEV ViDev = 0;
	CVI_U32 u32DataLen;
	VIDEO_FRAME_INFO_S stFrameInfo;
    VI_DEV_ATTR_S stViDevAttr[VI_MAX_DEV_NUM];

    CVI_U32 image_offset = 0;
    CVI_U32 image_size = 0;
    CVI_U8 *image_buf = NULL;
    getDevAttr(ViDev, &stViDevAttr[ViDev]);
    image_size = (stViDevAttr[ViDev].stSize.u32Width * stViDevAttr[ViDev].stSize.u32Height * 3) / 2;
    image_buf = (CVI_U8 *)malloc(image_size * u32FrameCnt);
    if (!image_buf) {
        printf("malloc failed\n");
        return ;
    }

	/* get frame  */
	while (u32Cnt--) {
		if (_getFrame(pstChn, &stFrameInfo, s32MilliSec) != CVI_SUCCESS) {
			printf("Get frame fail \n");
			usleep(1000);
			continue;
		}

        printf("dump frame %d done. ##cur_ms:%d\n", u32Cnt, csi_tick_get_ms());

		for (i = 0; i < 3; ++i) {
			u32DataLen = stFrameInfo.stVFrame.u32Stride[i] * stFrameInfo.stVFrame.u32Height;
			if (u32DataLen == 0)
				continue;
			if (i > 0 && ((stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
				(stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
				(stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
				u32DataLen >>= 1;

			// printf("plane(%d): paddr(%lx) vaddr(%p) stride(%d)\n",
			// 	   i, stFrameInfo.stVFrame.u64PhyAddr[i],
			// 	   stFrameInfo.stVFrame.pu8VirAddr[i],
			// 	   stFrameInfo.stVFrame.u32Stride[i]);
			// printf(" data_len(%d) plane_len(%d)\n",
			// 		  u32DataLen, stFrameInfo.stVFrame.u32Length[i]);
            memcpy(image_buf + image_offset, (void *)stFrameInfo.stVFrame.u64PhyAddr[i], u32DataLen);
            image_offset += u32DataLen;
			// aos_write(fd, (CVI_U8 *)stFrameInfo.stVFrame.u64PhyAddr[i], u32DataLen);
		}

		if (_releaseFrame(pstChn, &stFrameInfo) != CVI_SUCCESS)
			printf("_releaseFrame fail\n");
	}

    if (bFlag) {
        /* make file name */
        getFmtName(stFrameInfo.stVFrame.enPixelFormat, szPixFrm);
        snprintf(szFrameName, 128, SD_FATFS_MOUNTPOINT"/pipe%d_chn%d_%dx%d_%s_%d.yuv", pstChn->s32DevId, pstChn->s32ChnId,
                    stFrameInfo.stVFrame.u32Width, stFrameInfo.stVFrame.u32Height,
                    szPixFrm, u32FrameCnt);

        fd = aos_open(szFrameName, O_CREAT | O_RDWR | O_TRUNC);
        if (fd <= 0) {
            printf("aos_open dst file failed\n");
            if (image_buf) {
                free(image_buf);
				image_buf = NULL;
            }
            _releaseFrame(pstChn, &stFrameInfo);
            return;
        }
        bFlag = CVI_FALSE;
    }

    aos_write(fd, image_buf, image_offset);
	if (fd) {
		aos_sync(fd);
		aos_close(fd);
	}
	if (image_buf) {
		free(image_buf);
		image_buf = NULL;
    }
}

#endif

#if CONFIG_SENSOR_DUAL_SWITCH
CVI_S32 dual_sns_sync_task_callback(VI_SYNC_TASK_DATA_S *data)
{
    if (data->sync_event != VI_SYNC_EVENT_FE_DONE) {
        return 0;
    }

    ISP_SNS_OBJ_S *pSnsObj = NULL;
    ISP_SNS_OBJ_S *pSnsObj2 = NULL;
    PARAM_VI_CFG_S * pstViCfg = PARAM_getViCtx();

    for (CVI_U32 i = 0; i < pstViCfg->u32WorkSnsCnt; i++) {
        if (pstViCfg->pstSensorCfg[i].bDualSwitch) {
            if (!pSnsObj) {
                pSnsObj = pstViCfg->pstSensorCfg[i].pSnsObj;
            }
            else if (!pSnsObj2) {
                pSnsObj2 = pstViCfg->pstSensorCfg[i].pSnsObj;
            }
        }
    }

    if (!pSnsObj || !pSnsObj2) {
        aos_debug_printf("pSnsObj is NULL %p, %p\n", pSnsObj, pSnsObj2);
        return 0;
    }
    if (data->ViPipe == data->value)
        return 0;
    if (data->value == 0) {
        pSnsObj2->pfnStandby(2);
        pSnsObj->pfnRestart(0);
    } else {
        pSnsObj->pfnStandby(0);
        pSnsObj2->pfnRestart(2);
    }
    return 0;
}
#endif
static int _MEDIA_VIDEO_ViInit()
{
    PARAM_VI_CFG_S * pstViCfg = PARAM_getViCtx();
    return MEDIA_VIDEO_ViInit(pstViCfg);
}

int MEDIA_VIDEO_ViInit(PARAM_VI_CFG_S * pstViCfg)
{
    CVI_U8 devNum = 0;
    CVI_U8 tmpdevNum = 0;
    VI_DEV ViDev = 0;
    VI_CHN ViChn = 0;
    VI_DEV_ATTR_S stViDevAttr[VI_MAX_DEV_NUM];
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode[VI_MAX_DEV_NUM];
    CVI_S32 snsr_type[VI_MAX_DEV_NUM];
    ISP_SNS_OBJ_S *pSnsObj[VI_MAX_DEV_NUM];

    if(pstViCfg == NULL) {
        MEDIABUG_PRINTF("%s pstViCfg null err\n",__func__);
        return -1;
    }
    if(pstViCfg->u32WorkSnsCnt == 0) {
        return CVI_SUCCESS;
    }
    devNum = pstViCfg->u32WorkSnsCnt;
    MEDIA_CHECK_RET(getSnsType(snsr_type, &tmpdevNum), "getSnsType fail");
    for (CVI_U8 i = 0; i < devNum; i++) {
        pSnsObj[i] = getSnsObj(snsr_type[i]);
    }

    MEDIA_CHECK_RET(_meida_sensor_init(pstViCfg,&devNum),"_meida_sensor_init fail");

    CVI_VI_SetDevNum(devNum);

    for (int i = 0; i < devNum; i++) {
        ViDev = i;

        ViDev = pstViCfg->pstDevInfo[i].u8AttachDev > 0 ?
                VI_MAX_PHY_DEV_NUM + pstViCfg->pstDevInfo[i].u8AttachDev - 1 : i;

        MEDIA_CHECK_RET(getDevAttr(i, &stViDevAttr[ViDev]), "getDevAttr fail");
        if (pstViCfg->pstDevInfo[i].isMux) {
            stViDevAttr[ViDev].isMux = pstViCfg->pstDevInfo[i].isMux;
            stViDevAttr[ViDev].switchGpioIdx = pstViCfg->pstDevInfo[i].switchGpioIdx;
            stViDevAttr[ViDev].switchGpioPin = pstViCfg->pstDevInfo[i].switchGpioPin;
            stViDevAttr[ViDev].switchGPioPol = pstViCfg->pstDevInfo[i].switchGPioPol;
            stViDevAttr[ViDev].dstFrm = pstViCfg->pstDevInfo[i].dstFrm;
            stViDevAttr[ViDev].isFrmCtrl = pstViCfg->pstDevInfo[i].isFrmCtrl;;
        }

        MEDIA_CHECK_RET(getSnsMode(i, &stSnsrMode[i]), "stSnsrMode fail");
        if(ViDev == 0 && pstViCfg->pstDevInfo && pstViCfg->pstDevInfo->pViDmaBuf != NULL) {
            stViDevAttr[ViDev].phy_addr = (intptr_t)pstViCfg->pstDevInfo->pViDmaBuf;
            stViDevAttr[ViDev].phy_size = pstViCfg->pstDevInfo->u32ViDmaBufSize;
        }
        stViDevAttr[ViDev].stWDRAttr.enWDRMode = stSnsrMode[i].u8SnsMode;
        MEDIA_CHECK_RET(CVI_VI_SetDevAttr(ViDev, &stViDevAttr[ViDev]), "CVI_VI_SetDevAttr fail");
        MEDIA_CHECK_RET(CVI_VI_EnableDev(ViDev), "CVI_VI_EnableDev fail");
    }

    VI_PIPE_ATTR_S stPipeAttr;

    for (int i = 0; i < devNum; i++) {
        ViDev = pstViCfg->pstDevInfo[i].u8AttachDev > 0 ?
                VI_MAX_PHY_DEV_NUM + pstViCfg->pstDevInfo[i].u8AttachDev - 1 : i;
        MEDIA_CHECK_RET(getPipeAttr(i, &stPipeAttr), "getPipeAttr fail");
        MEDIA_CHECK_RET(CVI_VI_CreatePipe(ViDev, &stPipeAttr), "CVI_VI_CreatePipe fail");
        MEDIA_CHECK_RET(CVI_VI_StartPipe(ViDev), "CVI_VI_StartPipe fail");
        if (pstViCfg->bFastConverge == CVI_TRUE)
            setFastConvergeAttr(ViDev, CVI_TRUE);
    }

    for (int i = 0; i < devNum; i++) {
        ViDev = pstViCfg->pstDevInfo[i].u8AttachDev > 0 ?
                VI_MAX_PHY_DEV_NUM + pstViCfg->pstDevInfo[i].u8AttachDev - 1 : i;
        ISP_PUB_ATTR_S stPubAttr = { 0 };

        stPubAttr.stWndRect.u32Width = stPubAttr.stSnsSize.u32Width = stViDevAttr[ViDev].stSize.u32Width;
        stPubAttr.stWndRect.u32Height = stPubAttr.stSnsSize.u32Height = stViDevAttr[ViDev].stSize.u32Height;
        if (pstViCfg->pstSensorCfg[i].s32Framerate != 0) {
            stPubAttr.f32FrameRate = pstViCfg->pstSensorCfg[i].s32Framerate;
        } else {
            stPubAttr.f32FrameRate = 30;
        }
        stPubAttr.enBayer = stViDevAttr[ViDev].enBayerFormat;
        stPubAttr.enWDRMode = stViDevAttr[ViDev].stWDRAttr.enWDRMode;
        MEDIA_CHECK_RET(start_isp(stPubAttr, ViDev),"start_isp fail");
    }

    VI_CHN_ATTR_S stChnAttr = {0};

    for (int i = 0; i < devNum; i++) {
        ViChn = i;
        ViDev = pstViCfg->pstDevInfo[i].u8AttachDev > 0 ?
                VI_MAX_PHY_DEV_NUM + pstViCfg->pstDevInfo[i].u8AttachDev - 1 : i;
        MEDIA_CHECK_RET(getChnAttr(i, &stChnAttr), "getChnAttr fail");
        MEDIA_CHECK_RET(CVI_VI_SetChnAttr(ViDev, ViChn, &stChnAttr), "CVI_VI_SetChnAttr fail");
        if(pSnsObj[i]->pfnMirrorFlip) {
            CVI_VI_RegChnFlipMirrorCallBack(i,ViDev,(void *)pSnsObj[i]->pfnMirrorFlip);
        }
        MEDIA_CHECK_RET(CVI_VI_EnableChn(ViDev, ViChn), "CVI_VI_EnableChn fail");
        if(pstViCfg->pstSensorCfg[i].u8Rotation != ROTATION_0) {
            MEDIA_CHECK_RET(CVI_VI_SetChnRotation(ViDev,ViChn,pstViCfg->pstSensorCfg[i].u8Rotation),"CVI_VI_SetChnRotation fail");
        }
    }

    if (devNum > 1 && pstViCfg->pstIspCfg[0].bUseSingleBin == CVI_TRUE) {
        CVI_BIN_EnSingleMode();
    }

    int scene_mode = PARAM_getSceneMode();

    for (int i = 0; i < devNum; i++) {
        if (pstViCfg->pstIspCfg[scene_mode].astPQBinDes[i].pIspBinData && pstViCfg->pstIspCfg[scene_mode].astPQBinDes[i].u32IspBinDataLen > 0) {
            MEDIA_CHECK_RET(CVI_BIN_LoadParamFromBinEx(pstViCfg->pstIspCfg[scene_mode].astPQBinDes[i].binID,
                pstViCfg->pstIspCfg[scene_mode].astPQBinDes[i].pIspBinData,
                pstViCfg->pstIspCfg[scene_mode].astPQBinDes[i].u32IspBinDataLen),
                "CVI_BIN_LoadParamFromBinEx(%d) fail", pstViCfg->pstIspCfg[scene_mode].astPQBinDes[i].binID);
        }

    }

    for (int i = 0; i < devNum; i++) {
        ViDev = pstViCfg->pstDevInfo[i].u8AttachDev > 0 ?
                VI_MAX_PHY_DEV_NUM + pstViCfg->pstDevInfo[i].u8AttachDev - 1 : i;
        if(pstViCfg->pstIspCfg[scene_mode].bMonoSet[i]) {
            MEDIA_VIDEO_ViSetImageMono(ViDev);
        }
    }

#if CONFIG_SENSOR_DUAL_SWITCH
    static VI_SYNC_TASK_NODE_S stSyncTask;
    stSyncTask.name = "sns_switch0";
    stSyncTask.isp_sync_task_call_back = dual_sns_sync_task_callback;

    static VI_SYNC_TASK_NODE_S stSyncTask1;
    stSyncTask1.name = "sns_switch1";
    stSyncTask1.isp_sync_task_call_back = dual_sns_sync_task_callback;

    MEDIA_CHECK_RET(CVI_VI_RegSyncTask(0, &stSyncTask), "CVI_VI_RegSyncTask fail");
    MEDIA_CHECK_RET(CVI_VI_RegSyncTask(2, &stSyncTask1), "CVI_VI_RegSyncTask fail");
#endif
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

int MEDIA_VIDEO_ViDeinit(PARAM_VI_CFG_S* pstViCfg) {
	VI_DEV ViDev = 0;
	VI_CHN ViChn = 0;
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U8 i = 0;

	if (pstViCfg == NULL) {
		return CVI_FAILURE;
	}
	if (pstViCfg->u32WorkSnsCnt == 0) {
		return CVI_SUCCESS;
	}

	CVI_S32 snsr_type[VI_MAX_DEV_NUM];
	ISP_SNS_OBJ_S* pSnsObj;
	CVI_U8 dev_num;
	MEDIA_CHECK_RET(getSnsType(snsr_type, &dev_num), "getSnsType fail");

	ALG_LIB_S stAeLib, stAwbLib;

	for (i = 0; i < pstViCfg->u32WorkSnsCnt; i++) {
		ViChn = i;

		ViDev = pstViCfg->pstDevInfo[i].u8AttachDev > 0
				  ? VI_MAX_PHY_DEV_NUM + pstViCfg->pstDevInfo[i].u8AttachDev - 1
				  : i;
		stAeLib.s32Id = stAwbLib.s32Id = ViDev;
		strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
		strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

		pSnsObj = getSnsObj(snsr_type[i]);
		MEDIA_CHECK_RET(pSnsObj->pfnUnRegisterCallback(ViDev, &stAeLib, &stAwbLib),
						"sensor_unregister_callback failed");
		MEDIA_CHECK_RET(stop_isp(ViDev), "stop_isp fail");

		ret = CVI_VI_DisableChn(ViDev, ViChn);
		if (ret != CVI_SUCCESS) {
			MEDIABUG_PRINTF("CVI_VI_DisableChn FAIL!\n");
			return CVI_FAILURE;
		}

		MEDIA_CHECK_RET(CVI_VI_DestroyPipe(ViDev), "CVI_VI_DestroyPipe fail");
		//disable vi_dev
		MEDIA_CHECK_RET(CVI_VI_DisableDev(ViDev), "CVI_VI_DisableDev fail");
		MEDIA_CHECK_RET(CVI_VI_UnRegChnFlipMirrorCallBack(0, ViDev),
						"CVI_VI_UnRegChnFlipMirrorCallBack");
	}
	MEDIA_CHECK_RET(CVI_SYS_VI_Close(), "CVI_SYS_VI_Close fail");
	return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_ViDeInit()
{
    PARAM_VI_CFG_S * pstViCfg = PARAM_getViCtx();
    return MEDIA_VIDEO_ViDeinit(pstViCfg);
}

int MEDIA_VIDEO_SysInit()
{
    //sys/base init
    sys_core_init();
    //vip init
    cvi_cif_init();
    cvi_snsr_i2c_probe();
    vi_core_init();
    vpss_core_init();
#if (CONFIG_APP_VO_SUPPORT)
    vo_core_init();
#endif
    rgn_core_init();
    cvi_ldc_probe();
#if (CONFIG_SUPPORT_TEST_IVE == 1)
	cvi_ive_init();
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
    CVI_U16 width;
    CVI_U16 height;

    if(pstSysCtx == NULL) {
        return CVI_SUCCESS;
    }

    MEDIA_CHECK_RET(CVI_SYS_Init(), "CVI_SYS_Init failed\n");
    stVPSSMode.enMode = pstSysCtx->stVPSSMode.enMode;
    for(i = 0; i < pstSysCtx->u8ViCnt; i++) {
        stVIVPSSMode.aenMode[i] = pstSysCtx->stVIVPSSMode.aenMode[i];
        stVPSSMode.aenInput[i] = pstSysCtx->stVPSSMode.aenInput[i];
        stVPSSMode.ViPipe[i] = pstSysCtx->stVPSSMode.ViPipe[i];
    }
    MEDIA_CHECK_RET(CVI_SYS_SetVIVPSSMode(&stVIVPSSMode),"CVI_SYS_SetVIVPSSMode failed\n");
    CVI_SYS_SetVPSSModeEx(&stVPSSMode);
    //vb init
    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt = pstSysCtx->u8VbPoolCnt;
    for(i = 0 ; i<stVbConfig.u32MaxPoolCnt ; i++) {
        width = ALIGN(pstSysCtx->pstVbPool[i].u16width, DEFAULT_ALIGN);
        height = ALIGN(pstSysCtx->pstVbPool[i].u16height, DEFAULT_ALIGN);
        u32BlkSize = COMMON_GetPicBufferSize(width, height, pstSysCtx->pstVbPool[i].fmt,
            pstSysCtx->pstVbPool[i].enBitWidth, pstSysCtx->pstVbPool[i].enCmpMode, DEFAULT_ALIGN);
        u32RotBlkSize = COMMON_GetPicBufferSize(height, width,pstSysCtx->pstVbPool[i].fmt,
            pstSysCtx->pstVbPool[i].enBitWidth, pstSysCtx->pstVbPool[i].enCmpMode, DEFAULT_ALIGN);
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
    VI_DEV_ATTR_S stViDevAttr = {0};
    VPSS_GRP_ATTR_S * pstVpssGrp = NULL;
    VPSS_CHN_ATTR_S * pstVpssChn = NULL;
    CVI_U8 i = 0;
    CVI_U8 j = 0;


    if(pstVpssCtx == NULL) {
        MEDIABUG_PRINTF("********MEDIA_VIDEO_VpssInit pstVpssCtx NULL err \n");
        return -1;
    }
    if (pstVpssCtx->u8GrpCnt == 0) {
        return CVI_SUCCESS;
    }
    for(i = 0; i < pstVpssCtx->u8GrpCnt; i++) {
        pstVpssGrp = &pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr;
        VpssGrp = pstVpssCtx->pstVpssGrpCfg[i].VpssGrp;
        if(pstVpssCtx->pstVpssGrpCfg[i].s32BindVidev != -1) {
			MEDIA_CHECK_RET(getDevAttr(pstVpssCtx->pstVpssGrpCfg[i].s32BindVidev, &stViDevAttr),
							"getDevAttr fail");
			if(pstVpssCtx->pstVpssGrpCfg[i].u8ViRotation == 90) {
                pstVpssGrp->u32MaxW = stViDevAttr.stSize.u32Height;
                pstVpssGrp->u32MaxH = stViDevAttr.stSize.u32Width;
            } else {
                pstVpssGrp->u32MaxW = stViDevAttr.stSize.u32Width;
                pstVpssGrp->u32MaxH = stViDevAttr.stSize.u32Height;
            }
        }
        MEDIA_CHECK_RET(CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrp), "CVI_VPSS_CreateGrp failed\n");

		// Set Vpss Grp[i] Crop Configuration
		if(pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.bEnable == CVI_TRUE) {
			MEDIA_CHECK_RET(CVI_VPSS_SetGrpCrop(VpssGrp, &pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo), "CVI_VPSS_SetGrpCrop failed\n");
		}

        for(j = 0; j < pstVpssCtx->pstVpssGrpCfg[i].u8ChnCnt; j++) {
            pstVpssChn = &pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr;
            MEDIA_CHECK_RET(CVI_VPSS_SetChnAttr(VpssGrp, j, pstVpssChn), "CVI_VPSS_SetChnAttr failed\n");
            MEDIA_CHECK_RET(CVI_VPSS_EnableChn(VpssGrp, j), "CVI_VPSS_EnableChn failed\n");
            if(pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8Rotation != ROTATION_0) {
                CVI_VPSS_SetChnRotation(VpssGrp, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8Rotation);
            }
			// Set VPSS Grp[i] Chn[j] Crop Configuration
			if (pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.bEnable == CVI_TRUE) {
				MEDIA_CHECK_RET(CVI_VPSS_SetChnCrop(VpssGrp, j, &pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo), "CVI_VPSS_SetChnCrop failed\n");
			}
        }
        MEDIA_CHECK_RET(CVI_VPSS_StartGrp(VpssGrp), "CVI_VPSS_StartGrp failed\n");
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
    if (pstVpssCtx->u8GrpCnt == 0) {
        return CVI_SUCCESS;
    }
    for (i = 0; i < pstVpssCtx->u8GrpCnt; i++) {
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

        s32Ret = mipi_tx_send_cmd(0, &cmd_info);
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

void *_MEDIA_VIDEO_PanelInit(void *data)
{
    int fd = 0;
    CVI_S32 s32Ret = CVI_SUCCESS;
    struct panel_desc_s *panel_desc = (struct panel_desc_s *)data;

    s32Ret = mipi_tx_rstpin(0);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("mipi_tx_rstpin failed with %#x\n", s32Ret);
        return NULL;
    }

#if (CONFIG_PANEL_READID == 1)
    CVI_U32 panelid[3] = {0}, param[3] = {0xDA, 0xDB, 0xDC};
    CVI_U8 buf[4], i;

    for (i = 0; i < 3; ++i) {
        struct get_cmd_info_s get_cmd_info = {
            .devno = 0,
            .data_type = 0x06,
            .data_param = param[i],
            .get_data_size = 0x01,
            .get_data = buf
        };

        memset(buf, 0, sizeof(buf));
        if (mipi_tx_recv_cmd(0, &get_cmd_info)) {
            MEDIABUG_PRINTF("%s get panel id fialed!\n", __func__);
            return NULL;
        }
        panelid[i] = buf[0];
    }
    printf("%s panel id (0x%02x 0x%02x 0x%02x)!\n", __func__, panelid[0], panelid[1], panelid[2]);

    // modify panel_desc accord to panel id.
    s32Ret = mipi_tx_cfg(0, panel_desc->dev_cfg);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("mipi_tx_cfg failed with %#x\n", s32Ret);
        return NULL;
    }
#endif

    s32Ret = _MEDIA_VIDEO_DSIInit(0, panel_desc->dsi_init_cmds, panel_desc->dsi_init_cmds_size);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("dsi_init failed with %#x\n", s32Ret);
        return NULL;
    }
    s32Ret = mipi_tx_set_hs_settle(fd, panel_desc->hs_timing_cfg);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("mipi_tx_set_hs_settle failed with %#x\n", s32Ret);
        return NULL;
    }
    s32Ret = mipi_tx_enable(fd);
    if (s32Ret != CVI_SUCCESS) {
        MEDIABUG_PRINTF("mipi_tx_enable failed with %#x\n", s32Ret);
        return NULL;
    }
    MEDIABUG_PRINTF("Init for MIPI-Driver-%s\n", panel_desc->panel_name);

    pthread_exit(NULL);
    return NULL;
}

int MEDIA_VIDEO_PanelInit(void)
{
    static int initstatus = 0;
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (initstatus == 0) {
        initstatus = 1;
        s32Ret = mipi_tx_init(panel_desc.dev_cfg);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("mipi_tx_init failed with %#x\n", s32Ret);
            return s32Ret;
        }

        s32Ret = mipi_tx_disable(0);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("mipi_tx_disable failed with %#x\n", s32Ret);
            return s32Ret;
        }
        s32Ret = mipi_tx_cfg(0, panel_desc.dev_cfg);
        if (s32Ret != CVI_SUCCESS) {
            MEDIABUG_PRINTF("mipi_tx_cfg failed with %#x\n", s32Ret);
            return s32Ret;
        }

        struct sched_param param;
        pthread_attr_t attr;
        pthread_condattr_t cattr;
        pthread_t thread;

        param.sched_priority = MIPI_TX_RT_PRIO;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_condattr_init(&cattr);
        pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
        pthread_create(&thread, &attr, _MEDIA_VIDEO_PanelInit, (void *)&panel_desc);
        pthread_setname_np(thread, "cvi_mipi_tx");
    }
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
    if (pstVoCtx->pstPanelTxCfg != NULL && panel_desc.dev_cfg != NULL) {
        if (pstVoCtx->pstPanelTxCfg->bset_attr == 1) {
            for(int i = 0; i < 5; i++) {
                panel_desc.dev_cfg->lane_id[i] = pstVoCtx->pstPanelTxCfg->lane_id[i];
                panel_desc.dev_cfg->lane_pn_swap[i] = pstVoCtx->pstPanelTxCfg->lane_pn_swap[i];
            }
            panel_desc.dev_cfg->output_mode = pstVoCtx->pstPanelTxCfg->output_mode;
            panel_desc.dev_cfg->video_mode = pstVoCtx->pstPanelTxCfg->video_mode;
            panel_desc.dev_cfg->output_format = pstVoCtx->pstPanelTxCfg->output_format;
        }
        if (pstVoCtx->pstPanelTxCfg->bset_pin == 1) {
            panel_desc.dev_cfg->reset_pin = pstVoCtx->pstPanelTxCfg->reset_pin;
            panel_desc.dev_cfg->pwm_pin = pstVoCtx->pstPanelTxCfg->pwm_pin;
            panel_desc.dev_cfg->power_ct_pin = pstVoCtx->pstPanelTxCfg->power_ct_pin;
        }
    }
    MEDIA_VIDEO_PanelInit();
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
#if (CONFIG_APP_VO_SUPPORT == 0)
    return 0;
#endif
    PARAM_VO_CFG_S * pstVoCtx = PARAM_getVoCtx();
    return MEDIA_VIDEO_VoInit(pstVoCtx);
}

int MEDIA_VIDEO_VoDeinit(PARAM_VO_CFG_S * pstVoCtx)
{
#if (CONFIG_APP_VO_SUPPORT == 0)
    return 0;
#endif
    CVI_S32 i = 0;
    CVI_S32 j = 0;
    VO_DEV VoDev = 0;
    VO_LAYER VoLayer = 0;
    VO_CHN VoChn = 0;

    if(!pstVoCtx) {
        return CVI_FAILURE;
    }
    if (pstVoCtx->u8VoCnt == 0) {
        return CVI_SUCCESS;
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
#if !(CONFIG_USBD_UVC)
    VENC_RECV_PIC_PARAM_S stRecvParam = {0};
#endif
    VPSS_CHN_ATTR_S stVpssChnAttr = {0};
    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDestChn;
    ROTATION_E enRotation;
    PARAM_VENC_CHN_CFG_S *pstVecncChnCtx = NULL;

    if(!pstVencCfg) {
        return CVI_FAILURE;
    }
    if (VencChn >= pstVencCfg->s32VencChnCnt) {
        return CVI_FAILURE;
    }
    if(pstVencCfg->s32VencChnCnt == 0) {
        return CVI_SUCCESS;
    }
    pstVecncChnCtx = &pstVencCfg->pstVencChnCfg[VencChn];

    if(pstVecncChnCtx == NULL) {
        return CVI_FAILURE;
    }

    VencChn = pstVecncChnCtx->stChnParam.u8VencChn;
    if(pstVecncChnCtx->stChnParam.u8ModId == CVI_ID_VPSS && pstVecncChnCtx->stChnParam.u16Width == 0 &&  pstVecncChnCtx->stChnParam.u16Height == 0) {
        MEDIA_CHECK_RET(CVI_VPSS_GetChnAttr(pstVecncChnCtx->stChnParam.u8DevId, pstVecncChnCtx->stChnParam.u8DevChnid, &stVpssChnAttr), "CVI_VPSS_GetChnAttr");
        // if VPSS enable rotation, need exchange width and height
        MEDIA_CHECK_RET(CVI_VPSS_GetChnRotation(pstVecncChnCtx->stChnParam.u8DevId, pstVecncChnCtx->stChnParam.u8DevChnid, &enRotation), "CVI_VPSS_GetChnRotation");
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
                stAttr.stRcAttr.stH264Cbr.u32BitRate = pstVecncChnCtx->stRcParam.u32BitRate;
                stAttr.stRcAttr.stH264Cbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH264Cbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264VBR) {
                stAttr.stRcAttr.stH264Vbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH264Vbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH264Vbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH264Vbr.u32MaxBitRate = pstVecncChnCtx->stRcParam.u32BitRate;
                stAttr.stRcAttr.stH264Vbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH264Vbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else {
                return CVI_FAILURE;
            }
            stAttr.stVencAttr.stAttrH264e.bSingleLumaBuf = 0;
            break;
        }

        case PT_H265: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265CBR) {
                stAttr.stRcAttr.stH265Cbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH265Cbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH265Cbr.u32BitRate = pstVecncChnCtx->stRcParam.u32BitRate;
                stAttr.stRcAttr.stH265Cbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH265Cbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265VBR) {
                stAttr.stRcAttr.stH265Vbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stH265Vbr.u32SrcFrameRate = pstVecncChnCtx->stRcParam.u8SrcFrameRate;
                stAttr.stRcAttr.stH265Vbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stH265Vbr.u32MaxBitRate = pstVecncChnCtx->stRcParam.u32BitRate;
                stAttr.stRcAttr.stH265Vbr.u32Gop = pstVecncChnCtx->stRcParam.u16Gop;
                stAttr.stRcAttr.stH265Vbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else {
                return CVI_FAILURE;
            }
            break;
        }

        case PT_MJPEG: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGCBR) {
                stAttr.stRcAttr.stMjpegCbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stMjpegCbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stMjpegCbr.u32BitRate = pstVecncChnCtx->stRcParam.u32BitRate;
                stAttr.stRcAttr.stMjpegCbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGFIXQP) {
                stAttr.stRcAttr.stMjpegFixQp.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
                stAttr.stRcAttr.stMjpegFixQp.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
                stAttr.stRcAttr.stMjpegFixQp.u32Qfactor = pstVecncChnCtx->stRcParam.u8Qfactor;
            } else if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGVBR) {
		stAttr.stRcAttr.stMjpegVbr.bVariFpsEn = pstVecncChnCtx->stRcParam.u8VariFpsEn;
		stAttr.stRcAttr.stMjpegVbr.fr32DstFrameRate = pstVecncChnCtx->stRcParam.u8DstFrameRate;
		stAttr.stRcAttr.stMjpegVbr.u32MaxBitRate = pstVecncChnCtx->stRcParam.u32MaxBitRate;
		stAttr.stRcAttr.stMjpegVbr.u32StatTime = pstVecncChnCtx->stRcParam.u8StartTime;
            } else {
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
            return CVI_FAILURE;
        }
    }
    stAttr.stGopAttr.enGopMode = pstVecncChnCtx->stGopParam.u16gopMode;
    stAttr.stGopAttr.stNormalP.s32IPQpDelta = pstVecncChnCtx->stGopParam.s8IPQpDelta;

    MEDIA_CHECK_RET(CVI_VENC_GetModParam(&stModParam), "CVI_VENC_GetModParam");
#if (CONFIG_USBD_UVC)
	stModParam.enVencModType = MODTYPE_JPEGE;
    stModParam.stJpegeModParam.enJpegeFormat = JPEGE_FORMAT_CUSTOM;
    stModParam.stJpegeModParam.JpegMarkerOrder[0] = JPEGE_MARKER_SOI;
    stModParam.stJpegeModParam.JpegMarkerOrder[1] = JPEGE_MARKER_JFIF;
    stModParam.stJpegeModParam.JpegMarkerOrder[2] = JPEGE_MARKER_DQT_MERGE;
    stModParam.stJpegeModParam.JpegMarkerOrder[3] = JPEGE_MARKER_SOF0;
    stModParam.stJpegeModParam.JpegMarkerOrder[4] = JPEGE_MARKER_DHT_MERGE;
    stModParam.stJpegeModParam.JpegMarkerOrder[5] = JPEGE_MARKER_DRI;
    stModParam.stJpegeModParam.JpegMarkerOrder[6] = JPEGE_MARKER_BUTT;
#endif
    MEDIA_CHECK_RET(CVI_VENC_SetModParam(&stModParam), "CVI_VENC_SetModParam");

    MEDIA_CHECK_RET(CVI_VENC_CreateChn(VencChn, &stAttr), "CVI_VENC_CreateChn");
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
            } else {
                return CVI_FAILURE;
            }
            break;
        }

        case PT_MJPEG: {
            if (stAttr.stRcAttr.enRcMode == VENC_RC_MODE_MJPEGCBR) {
		stRcParam.stParamMjpegCbr.u32MaxQfactor = 99;
		stRcParam.stParamMjpegCbr.u32MinQfactor = 1;
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
        stH265Vui.stVuiTimeInfo.timing_info_present_flag = 0;
        stH265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = 0;
        stH265Vui.stVuiBitstreamRestric.bitstream_restriction_flag = 0;
        stH265Vui.stVuiVideoSignal.video_signal_type_present_flag = 0;

        MEDIA_CHECK_RET(CVI_VENC_SetH265Vui(VencChn, &stH265Vui), "CVI_VENC_SetH265Vui");

    } else if (pstVecncChnCtx->stChnParam.u16EnType == PT_H264) {

        MEDIA_CHECK_RET(CVI_VENC_GetH264Entropy(VencChn, &stH264EntropyEnc), "CVI_VENC_GetH264Entropy");
        stH264EntropyEnc.u32EntropyEncModeI = pstVecncChnCtx->stChnParam.u8EntropyEncModeI;
        stH264EntropyEnc.u32EntropyEncModeP = pstVecncChnCtx->stChnParam.u8EntropyEncModeP;
        MEDIA_CHECK_RET(CVI_VENC_SetH264Entropy(VencChn, &stH264EntropyEnc), "CVI_VENC_SetH264Entropy");

        MEDIA_CHECK_RET(CVI_VENC_GetH264Vui(VencChn, &stH264Vui), "CVI_VENC_GetH264Vui");
        stH264Vui.stVuiTimeInfo.timing_info_present_flag = 0;
        stH264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = 0;
        stH264Vui.stVuiBitstreamRestric.bitstream_restriction_flag = 0;
        stH264Vui.stVuiVideoSignal.video_signal_type_present_flag = 0;
        MEDIA_CHECK_RET(CVI_VENC_SetH264Vui(VencChn, &stH264Vui), "CVI_VENC_SetH264Vui");
    } else if (pstVecncChnCtx->stChnParam.u16EnType == PT_JPEG) {
        MEDIA_CHECK_RET(CVI_VENC_GetJpegParam(VencChn, &stJpegParam), "CVI_VENC_GetJpegParam");
        MEDIA_CHECK_RET(CVI_VENC_SetJpegParam(VencChn, &stJpegParam), "CVI_VENC_SetJpegParam");
    }
    if(pstVecncChnCtx->stChnParam.u8ModId == CVI_ID_VPSS || pstVecncChnCtx->stChnParam.u8ModId == CVI_ID_VI ) {
        stDestChn.enModId = CVI_ID_VENC;
        stDestChn.s32DevId = 0;
        stDestChn.s32ChnId = pstVecncChnCtx->stChnParam.u8VencChn;
        stSrcChn.enModId = pstVecncChnCtx->stChnParam.u8ModId;
        stSrcChn.s32DevId = pstVecncChnCtx->stChnParam.u8DevId;
        stSrcChn.s32ChnId = pstVecncChnCtx->stChnParam.u8DevChnid;
        MEDIA_CHECK_RET(CVI_SYS_Bind(&stSrcChn, &stDestChn), "CVI_SYS_Bind err");
    }

#if !(CONFIG_USBD_UVC)
    stRecvParam.s32RecvPicNum = -1;
    MEDIA_CHECK_RET(CVI_VENC_StartRecvFrame(VencChn, &stRecvParam), "CVI_VENC_StartRecvFrame");
    pstVecncChnCtx->stChnParam.u8InitStatus = 1;
#endif
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
    if(pstVencCfg->s32VencChnCnt == 0) {
        return CVI_SUCCESS;
    }
    pstVecncChnCtx = &pstVencCfg->pstVencChnCfg[VencChn];
    if(pstVecncChnCtx->stChnParam.u8ModId == CVI_ID_VPSS || pstVecncChnCtx->stChnParam.u8ModId == CVI_ID_VI ) {
        stDestChn.enModId = CVI_ID_VENC;
        stDestChn.s32DevId = 0;
        stDestChn.s32ChnId = pstVecncChnCtx->stChnParam.u8VencChn;
        stSrcChn.enModId = pstVecncChnCtx->stChnParam.u8ModId;
        stSrcChn.s32DevId = pstVecncChnCtx->stChnParam.u8DevId;
        stSrcChn.s32ChnId = pstVecncChnCtx->stChnParam.u8DevChnid;
        MEDIA_CHECK_RET(CVI_SYS_UnBind(&stSrcChn, &stDestChn), "CVI_SYS_UnBind err");
    }
    pstVecncChnCtx->stChnParam.u8InitStatus = 0;
    while(pstVecncChnCtx->stChnParam.u8RunStatus == 1) {
        usleep(1*1000);
    }
    MEDIA_CHECK_RET(CVI_VENC_StopRecvFrame(pstVecncChnCtx->stChnParam.u8VencChn), "CVI_VENC_StopRecvFrame");

    MEDIA_CHECK_RET(CVI_VENC_ResetChn(pstVecncChnCtx->stChnParam.u8VencChn), "CVI_VENC_ResetChn");

    MEDIA_CHECK_RET(CVI_VENC_DestroyChn(pstVecncChnCtx->stChnParam.u8VencChn), "CVI_VENC_DestroyChn");
    return CVI_SUCCESS;
}

int MEDIA_VIDEO_VencInit(PARAM_VENC_CFG_S *pstVencCfg)
{
    if(!pstVencCfg) {
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
    if(pstVencCfg->pstVencChnCfg[VencChn].stChnParam.u8InitStatus == 0) {
        return CVI_FAILURE;
    }
    pstVecncChnCtx = &pstVencCfg->pstVencChnCfg[VencChn];
    pstVecncChnCtx->stChnParam.u8RunStatus = 1;
    if(pstVecncChnCtx->stChnParam.u8ModId != CVI_ID_VPSS && pstVecncChnCtx->stChnParam.u8ModId != CVI_ID_VI ) {
        MEDIA_CHECK_RET(CVI_VPSS_GetChnFrame(pstVecncChnCtx->stChnParam.u8DevId, pstVecncChnCtx->stChnParam.u8DevChnid, &stSrcFrame, 2000), "CVI_VPSS_GetChnFrame");
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
    if(pstVecncChnCtx->stChnParam.u8ModId != CVI_ID_VPSS && pstVecncChnCtx->stChnParam.u8ModId != CVI_ID_VI ) {
        MEDIA_CHECK_RET(CVI_VPSS_ReleaseChnFrame(pstVecncChnCtx->stChnParam.u8DevId, pstVecncChnCtx->stChnParam.u8DevChnid, &stSrcFrame), "CVI_VPSS_ReleaseChnFrame");
    }
    pstVecncChnCtx->stChnParam.u8RunStatus = 0;
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
    if(pstVencCfg->pstVencChnCfg[VencChn].stChnParam.u8InitStatus == 0) {
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

#if CONFIG_BOOT_FREQ_HIGHER
void efuse_bootFreqHigher()
{
    csi_efuse_t efuse = {0};
    csi_efuse_init(&efuse, 0);
    int ret;

    ret = CVI_EFUSE_IsBootFreqHigher();
    if (ret == CVI_SUCCESS) {
        printf("Boot freq higher\n");
        csi_efuse_uninit(&efuse);
        return;
    }

    ret = CVI_EFUSE_BootFreqHigher();
    if (ret != CVI_SUCCESS) {
        printf("CVI_EFUSE_BootFreqHigher ret=%d\n", ret);
    }

    ret = CVI_EFUSE_IsBootFreqHigher();
    if (ret == CVI_SUCCESS) {
        printf("Boot freq higher\n");
    }
    else {
        printf("CVI_EFUSE_IsBootFreqHigher ret=%d\n", ret);
    }

    csi_efuse_uninit(&efuse);
}
#endif

__attribute__((weak)) int MISC_WaitVideoStep1()
{
	printf("weak %s\n", __FUNCTION__);
	return 0;
}
static int _MEDIA_VIDEO_Step1Init()
{
    if(g_mediaVideoRunStatus == 1) {
        printf("Media_Video_Init please deinit and try again \n");
        return 0;
    }
    MEDIA_CHECK_RET(_MEDIA_VIDEO_SysVbInit(),"MEDIA_VIDEO_SysVbInit failed");
    MEDIA_CHECK_RET(_MEDIA_VIDEO_ViInit(),"MEDIA_VIDEO_ViInit failed");
    MEDIA_CHECK_RET(_MEDIA_VIDEO_VpssInit(),"MEDIA_VIDEO_VpssInit failed");
#if CONFIG_APP_DUMP_FRAME
    PARAM_SYS_CFG_S *pstSysCtx = PARAM_GET_SYS_CFG();
    MMF_CHN_S stChn = {
        .s32DevId = 0,
        .s32ChnId = 0,
        .enModId = CVI_ID_VI,
    };
    if (pstSysCtx->stVIVPSSMode.aenMode[0] == VI_OFFLINE_VPSS_ONLINE) {
        stChn.enModId = CVI_ID_VPSS;
    }
    dumpFrame(&stChn, 1);
#endif
    return CVI_SUCCESS;
}

static int _MEDIA_VIDEO_Step2Init()
{
    if(g_mediaVideoRunStatus == 1) {
        printf("Media_Video_Init please deinit and try again \n");
        return 0;
    }

    MEDIA_CHECK_RET(_MEDIA_VIDEO_VoInit(),"MEDIA_VIDEO_VoInit failed");
#if (CONFIG_APP_GUI_SUPPORT == 1)
    GUI_Display_Start();
#endif
    MEDIA_CHECK_RET(_MEDIA_VIDEO_VencInit(),"MEDIA_VIDEO_VencInit failed");
#if (CONFIG_APP_AI_SUPPORT == 1)
    APP_AiStart();
#endif
#if CONFIG_BOOT_FREQ_HIGHER
    efuse_bootFreqHigher();
#endif
    g_mediaVideoRunStatus = 1;

    return CVI_SUCCESS;
}

int MEDIA_VIDEO_Init()
{
    MEDIA_CHECK_RET(_MEDIA_VIDEO_Step1Init(),"_MEDIA_VIDEO_Step1Init failed");
    MISC_WaitVideoStep1();
    MEDIA_CHECK_RET(_MEDIA_VIDEO_Step2Init(),"_MEDIA_VIDEO_Step2Init failed");
    return CVI_SUCCESS;
}

int MEDIA_VIDEO_Deinit()
{
    if(g_mediaVideoRunStatus == 0) {
        printf("MEDIA_VIDEO_Deinit please init and try again \n");
        return 0;
    }
#if (CONFIG_APP_GUI_SUPPORT == 1)
    GUI_Display_Stop();
#endif
#if (CONFIG_APP_AI_SUPPORT == 1)
    APP_AiStop();
#endif
    MEDIA_CHECK_RET(_MEDIA_VIDEO_VencDeInit(),"MEDIA_VIDEO_VencDeInit failed");
    MEDIA_CHECK_RET(_MEDIA_VIDEO_VoDeinit(),"MEDIA_VIDEO_VoDeinit failed");
    MEDIA_CHECK_RET(_MEDIA_VIDEO_VpssDeinit(),"MEDIA_VIDEO_VpssDeinit failed");
    MEDIA_CHECK_RET(_MEDIA_VIDEO_ViDeInit(),"MEDIA_VIDEO_ViDeInit failed");
    MEDIA_CHECK_RET(MEDIA_VIDEO_SysVbDeinit(),"MEDIA_VIDEO_SysVbDeinit failed");
    g_mediaVideoRunStatus = 0;
    return CVI_SUCCESS;
}

/**
 * @brief  Modify frame rate ratio based in dual_switch mode
 * @note
 * @param  dstFrm0: sensor0 target frame rate
 * @param  dstFrm1: sensor1 target frame rate
 * @retval CVI_SUCCESS if switch sensor's frame rate ratio successfully
 */
int32_t switch_frame_rate_ratio(int32_t dstFrm0, int32_t dstFrm1)
{
    /* Confirm target AEResponseFrame value */
    int32_t aeRespFrm0, aeRespFrm1;
    if (dstFrm0 == 1) {
        aeRespFrm0 = 3;
        if (dstFrm1 == 1) {
            aeRespFrm1 = 3;
        } else {
            aeRespFrm1 = 2;
        }
    } else {
        aeRespFrm0 = 2;
        aeRespFrm1 = 3;
    }

    /* Modify dstFrm */
    uint8_t devNum = 2;
    VI_DEV ViDev;
    VI_DEV_ATTR_S stViDevAttr[VI_MAX_DEV_NUM];
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode[VI_MAX_DEV_NUM];
    PARAM_VI_CFG_S* pstViCfg = PARAM_getViCtx();

    for (int32_t i = 0; i < devNum; i++) {
        ViDev = pstViCfg->pstDevInfo[i].u8AttachDev > 0
                ? VI_MAX_PHY_DEV_NUM + pstViCfg->pstDevInfo[i].u8AttachDev - 1
                : i;
        /* Disable VI dev for reload dev attribute configuration */
        MEDIA_CHECK_RET(CVI_VI_DisableDev(ViDev), "CVI_VI_DisableDev fail");

        /* Update AEResponseFrame */
        if (ViDev == 0) {
            CVI_ISP_SetResponseFrame(ViDev, aeRespFrm0);
        } else {
            CVI_ISP_SetResponseFrame(ViDev, aeRespFrm1);
        }

        MEDIA_CHECK_RET(getDevAttr(i, &stViDevAttr[ViDev]), "getDevAttr fail");
        if (pstViCfg->pstDevInfo[i].isMux) {
            stViDevAttr[ViDev].isMux = pstViCfg->pstDevInfo[i].isMux;
            stViDevAttr[ViDev].switchGpioIdx = pstViCfg->pstDevInfo[i].switchGpioIdx;
            stViDevAttr[ViDev].switchGpioPin = pstViCfg->pstDevInfo[i].switchGpioPin;
            stViDevAttr[ViDev].switchGPioPol = pstViCfg->pstDevInfo[i].switchGPioPol;
            /* Update frame rate ratio */
            if (ViDev == 0) {
                stViDevAttr[0].dstFrm = dstFrm0;
            } else {
                stViDevAttr[2].dstFrm = dstFrm1;
            }
            stViDevAttr[ViDev].isFrmCtrl = pstViCfg->pstDevInfo[i].isFrmCtrl;
        }

        MEDIA_CHECK_RET(getSnsMode(i, &stSnsrMode[i]), "stSnsrMode fail");
        if (ViDev == 0 && pstViCfg->pstDevInfo && pstViCfg->pstDevInfo->pViDmaBuf != NULL) {
            stViDevAttr[ViDev].phy_addr = (intptr_t)pstViCfg->pstDevInfo->pViDmaBuf;
            stViDevAttr[ViDev].phy_size = pstViCfg->pstDevInfo->u32ViDmaBufSize;
        }
        stViDevAttr[ViDev].stWDRAttr.enWDRMode = stSnsrMode[i].u8SnsMode;
        MEDIA_CHECK_RET(CVI_VI_SetDevAttr(ViDev, &stViDevAttr[ViDev]), "CVI_VI_SetDevAttr fail");
        MEDIA_CHECK_RET(CVI_VI_EnableDev(ViDev), "CVI_VI_EnableDev fail");
    }
    return CVI_SUCCESS;
}

/*------------------------------FOLLOWING FUNCS FOR MEDIA TEST-----------------------------------*/

/**
 * @brief  switch vpss channel's scale coefficient level
 *
 * @retval CVI_SUCCESS if vpss scale mode modified
 */
int testMedia_switch_vpss_scale_mode(int32_t argc, char** argv)
{
    if (argc != 2) {
        printf("Usage: switch_vpss_scale_mode [grp] [chn] [coef_level(0~6)]\n");
        return CVI_FAILURE;
    }
    int coef_level = atoi(argv[1]);
    MEDIA_CHECK_RET(CVI_VPSS_SetChnScaleCoefLevel(0, 0, coef_level),
                    "CVI_VPSS_SetChnScaleCoefLevel Fail");
    VPSS_SCALE_COEF_E cur_level;
    CVI_VPSS_GetChnScaleCoefLevel(0, 0, &cur_level);

    printf("vpss grp %d chn %d cur_level = %d\n", 0, 0, cur_level);
    return CVI_SUCCESS;
}


void testMedia_video_Deinit(int32_t argc, char** argv)
{
    MEDIA_VIDEO_Deinit();
}

void testMedia_video_init(int32_t argc, char** argv)
{
    MEDIA_VIDEO_Init();
}

void testMedia_switch_pipeline(int32_t argc, char** argv)
{
    if (argc < 2) {
        printf("please input 0/1 chose RGB or IR\n");
        printf("testMedia_switch_pipeline 0/1 \n");
        return;
    }
    if (atoi(argv[1]) == 0 || atoi(argv[1]) == 1) {
        PARAM_setPipeline(atoi(argv[1]));
    }
}


int32_t testMedia_switch_frame_rate_ratio(int32_t argc, char** argv)
{
    if (argc != 3) {
        printf("Usage: switch_frame_rate_ratio dstFrm0 dstFrm1\n");
        return CVI_FAILURE;
    }
    int32_t dstFrm0 = atoi(argv[1]);
    int32_t dstFrm1 = atoi(argv[2]);

    if (dstFrm0 == 0 || dstFrm1 == 0) {
        printf("Input dstFrm value is invalid\n");
        return CVI_FAILURE;
    }
    printf("dstFrm 0 = %d\n", dstFrm0);
    printf("dstFrm 1 = %d\n", dstFrm1);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (switch_frame_rate_ratio(dstFrm0, dstFrm1)) {
        return CVI_FAILURE;
    }

    gettimeofday(&end, NULL);
    printf("switch_frame_rate_ratio cost time = %ldus\n",
           (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));

    return CVI_SUCCESS;
}


/**
 * @brief  switch video pipeline in scene of single mipi multiplex
 * @note
 * * when sensor doesn't support resume/standby, user should restart VI for single mipi multiplex
 *
 * @retval None
 */
void testMedia_multiplex_switch_pipeline(int32_t argc, char** argv)
{
    if (argc < 2) {
        printf("please input 0/1 chose RGB or IR\n");
        printf("testMedia_switch_pipeline 0/1 \n");
        return;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    /* For some sensors, user should set proper page select, current config is for SP2509 */
    if (atoi(argv[1]) == 0) {
        sensor_i2c_write(0x1, 0x3d, 0xfd, 0x0, 0x1, 0x1);
    } else {
        sensor_i2c_write(0x0, 0x3d, 0xfd, 0x0, 0x1, 0x1);
    }

    _MEDIA_VIDEO_ViDeInit();
    g_mediaVideoRunStatus = 0;

    /* set specific pipeline param */
    if (atoi(argv[1]) == 0 || atoi(argv[1]) == 1) {
        PARAM_setPipeline(atoi(argv[1]));
    }

    _MEDIA_VIDEO_ViInit();
    g_mediaVideoRunStatus = 1;
    gettimeofday(&end, NULL);
    printf("switch pipeline cost time = %ldus\n",
           (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));
}

ALIOS_CLI_CMD_REGISTER(testMedia_switch_vpss_scale_mode, testMedia_switch_vpss_scale_mode,
                       switch vpss scale coefficient level);
ALIOS_CLI_CMD_REGISTER(testMedia_video_Deinit, testMedia_video_Deinit, testMedia_video_Deinit);
ALIOS_CLI_CMD_REGISTER(testMedia_video_init, testMedia_video_init, testMedia_video_init);
ALIOS_CLI_CMD_REGISTER(testMedia_switch_pipeline, testMedia_switch_pipeline,
                       testMedia_switch_pipeline);
ALIOS_CLI_CMD_REGISTER(testMedia_switch_frame_rate_ratio, testMedia_switch_frame_rate_ratio,
                       swtich frame rate ratio in dual_switch mode);
ALIOS_CLI_CMD_REGISTER(testMedia_multiplex_switch_pipeline, testMedia_multiplex_switch_pipeline,
                       switch video pipeline in scene of single mipi multiplex);

#if CONFIG_SENSOR_DUAL_SWITCH
void testMedia_sensor_switch(int32_t argc, char **argv)
{
    CVI_S32 snsr_type;
    CVI_U8 dev_num;
    ISP_SNS_OBJ_S *pSnsObj;

    if(argc < 2) {
        printf("please input 0/1 chose sensor switch\n");
        printf("testMedia_sensor_switch 0/1 \n");
        return ;
    }

    int sns_idx = atoi(argv[1]);
    if (sns_idx > 1 || sns_idx < 0){
        printf("input illegal\n");
        return ;
    }

    getSnsType(&snsr_type, &dev_num);
    pSnsObj = getSnsObj(snsr_type);
    if (!pSnsObj) {
        printf("pSnsObj null, %d\n", snsr_type);
        return;
    }

    if (pSnsObj->pfnSnsDualSwitch) {
        pSnsObj->pfnSnsDualSwitch(sns_idx);
    }
    else {
        printf("sns %d not support dual switch\n", snsr_type);
    }
}
ALIOS_CLI_CMD_REGISTER(testMedia_sensor_switch, testMedia_sensor_switch, testMedia_sensor_switch);
#endif
