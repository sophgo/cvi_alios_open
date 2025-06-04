#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <cvi_type.h>
#include <cvi_comm_video.h>
#include "cvi_debug.h"
#include "cvi_comm_sns.h"
#include "cvi_sns_ctrl.h"
#include "cvi_isp.h"

#include "pr2100_cmos_ex.h"
#include "pr2100_cmos_param.h"

#define PR2100_I2C_ADDR_MASTER 0x5F
#define PR2100_I2C_ADDR_SLAVE 0x5C
#define PR2100_I2C_ADDR_NUM VI_MAX_PIPE_NUM * 2
/****************************************************************************
 * global variables                                                         *
 ****************************************************************************/
ISP_SNS_COMMBUS_U g_aunPr2100_BusInfo[PR2100_I2C_ADDR_NUM] = {
	[0] = { .s8I2cDev = 0},
	[1 ... PR2100_I2C_ADDR_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_COMMADDR_U g_aunPr2100_AddrInfo[PR2100_I2C_ADDR_NUM] = {
	[0] = { .s8I2cAddr = 0},
	[1 ... PR2100_I2C_ADDR_NUM - 1] = { .s8I2cAddr = -1}
};

ISP_SNS_STATE_S *g_pastPr2100[VI_MAX_PIPE_NUM] = {CVI_NULL};
SNS_COMBO_DEV_ATTR_S *g_pastPr2100ComboDevArray[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define PR2100_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastPr2100[dev])
#define PR2100_SENSOR_SET_CTX(dev, pstCtx)   (g_pastPr2100[dev] = pstCtx)
#define PR2100_SENSOR_RESET_CTX(dev)         (g_pastPr2100[dev] = CVI_NULL)
#define PR2100_SENSOR_SET_COMBO(dev, pstCtx)   (g_pastPr2100ComboDevArray[dev] = pstCtx)
#define PR2100_SENSOR_GET_COMBO(dev, pstCtx)   (pstCtx = g_pastPr2100ComboDevArray[dev])

#define PR2100_RES_IS_2M(w, h)      ((w) <= 1920 && (h) <= 1080)
#define PR2100_ID 2100

CVI_U16 g_au16Pr2100_BdgMuxMode[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                            *
 ****************************************************************************/
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
	const PR2100_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	PR2100_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstMode = &g_astPr2100_mode[pstSnsState->u8ImgMode];
	pstIspCfg->frm_num = 1;
	memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_SYNC_INFO_S *pstSnsSyncInfo)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg0 = CVI_NULL;

	CMOS_CHECK_POINTER(pstSnsSyncInfo);
	PR2100_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstCfg0 = &pstSnsState->astSyncInfo[0];
	cmos_get_wdr_size(ViPipe, &pstCfg0->ispCfg);
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	PR2100_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;

	if (pstSensorImageMode->f32Fps <= 25) {
		if (PR2100_RES_IS_2M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
			switch (g_au16Pr2100_BdgMuxMode[ViPipe]) {
			case SNS_BDG_MUX_NONE:
				u8SensorImageMode = PR2100_MODE_1080P25;
				break;
			case SNS_BDG_MUX_2:
				if(pstSensorImageMode->u8LaneNum == 2) {
					u8SensorImageMode = PR2100_MODE_1080P25_2CH_2L;
				} else {
					u8SensorImageMode = PR2100_MODE_1080P25_2CH;
				}
				break;
			case SNS_BDG_MUX_3:
			case SNS_BDG_MUX_4:
				u8SensorImageMode = PR2100_MODE_1080P25_4CH;
				break;
			}
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
				pstSensorImageMode->u16Width,
				pstSensorImageMode->u16Height,
				pstSensorImageMode->f32Fps,
				pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
	} else if (pstSensorImageMode->f32Fps <= 30) {
		if (PR2100_RES_IS_2M(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
			switch (g_au16Pr2100_BdgMuxMode[ViPipe]) {
			case SNS_BDG_MUX_NONE:
			case SNS_BDG_MUX_2:
			case SNS_BDG_MUX_3:
			case SNS_BDG_MUX_4:
				u8SensorImageMode = PR2100_MODE_1080P30_4CH;
				break;
			}
		}
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		/* Don't need to switch SensorImageMode */
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	PR2100_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->u8ImgMode = PR2100_MODE_1080P25;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	SNS_COMBO_DEV_ATTR_S *pstRxAttrSrc = CVI_NULL;

	PR2100_SENSOR_GET_CTX(ViPipe, pstSnsState);
	PR2100_SENSOR_GET_COMBO(ViPipe, pstRxAttrSrc);

	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);
	CMOS_CHECK_POINTER(pstRxAttrSrc);

	memcpy(pstRxAttr, pstRxAttrSrc, sizeof(*pstRxAttr));

	pstRxAttr->img_size.start_x = g_astPr2100_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32X;
	pstRxAttr->img_size.start_y = g_astPr2100_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32Y;
	pstRxAttr->img_size.active_w = g_astPr2100_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Width;
	pstRxAttr->img_size.active_h = g_astPr2100_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Height;
	pstRxAttr->img_size.width = g_astPr2100_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astPr2100_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	pstRxAttr->img_size.max_width = g_astPr2100_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Width;
	pstRxAttr->img_size.max_height = g_astPr2100_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Height;


	if (pstSnsState->u8ImgMode == PR2100_MODE_1080P25) {
		pstRxAttr->mac_clk = RX_MAC_CLK_400M;
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
		pstRxAttr->mipi_attr.demux.demux_en = 0;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = CVI_NULL;
	int i;

	if (!g_pastPr2100ComboDevArray[ViPipe]) {
		pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));
	} else {
		PR2100_SENSOR_GET_COMBO(ViPipe, pstRxAttr);
	}
	memcpy(pstRxAttr, &pr2100_multi_rx_attr, sizeof(SNS_COMBO_DEV_ATTR_S));
	PR2100_SENSOR_SET_COMBO(ViPipe, pstRxAttr);

	CMOS_CHECK_POINTER(pstRxInitAttr);

	if (pstRxInitAttr->stMclkAttr.bMclkEn)
		pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

	if (pstRxInitAttr->MipiDev >= VI_MAX_DEV_NUM)
		return CVI_SUCCESS;

	pstRxAttr->devno = pstRxInitAttr->MipiDev;

	if (pstRxAttr->input_mode == INPUT_MODE_MIPI) {
		struct mipi_dev_attr_s *attr = &pstRxAttr->mipi_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	} else {
		struct lvds_dev_attr_s *attr = &pstRxAttr->lvds_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	}

	return CVI_SUCCESS;
}


void pr2100_exit(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "Exit Pixelplus PR2100 Sensor\n");

	if (g_pastPr2100ComboDevArray[ViPipe]) {
		free(g_pastPr2100ComboDevArray[ViPipe]);
		g_pastPr2100ComboDevArray[ViPipe] = CVI_NULL;
	}
	pr2100_i2c_exit(ViPipe);

	if (g_pastPr2100[ViPipe]->u8ImgMode == PR2100_MODE_1080P25_4CH)
		pr2100_i2c_exit(ViPipe + VI_MAX_PIPE_NUM);

	if (g_pastPr2100[ViPipe]->u8ImgMode == PR2100_MODE_1080P30_4CH)
		pr2100_i2c_exit(ViPipe + VI_MAX_PIPE_NUM);
}

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = pr2100_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = pr2100_exit;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = sensor_global_init;
	pstSensorExpFunc->pfn_cmos_set_image_mode = cmos_set_image_mode;
	pstSensorExpFunc->pfn_cmos_get_sns_reg_info = cmos_get_sns_regs_info;

	return CVI_SUCCESS;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
static CVI_VOID sensor_patch_i2c_addr(VI_PIPE ViPipe, CVI_S32 s32I2cAddr)
{
	if (s32I2cAddr < 0) {
		g_aunPr2100_AddrInfo[ViPipe].s8I2cAddr = PR2100_I2C_ADDR_MASTER;
		g_aunPr2100_AddrInfo[ViPipe + VI_MAX_PIPE_NUM].s8I2cAddr = PR2100_I2C_ADDR_SLAVE;
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C addr input error ,please check [0x%x]\n", s32I2cAddr);
	} else {
		g_aunPr2100_AddrInfo[ViPipe].s8I2cAddr = s32I2cAddr;
		g_aunPr2100_AddrInfo[ViPipe + VI_MAX_PIPE_NUM].s8I2cAddr = s32I2cAddr + 1;
	}

}

static CVI_S32 pr2100_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunPr2100_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;
	g_aunPr2100_BusInfo[ViPipe + VI_MAX_PIPE_NUM].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	PR2100_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));
	PR2100_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	PR2100_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	PR2100_SENSOR_RESET_CTX(ViPipe);
}

static CVI_S32 sensor_register_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	(void) pstAeLib;
	(void) pstAwbLib;

	CVI_S32 s32Ret;
	ISP_SENSOR_REGISTER_S stIspRegister;
	ISP_SNS_ATTR_INFO_S   stSnsAttrInfo;

	s32Ret = sensor_ctx_init(ViPipe);

	if (s32Ret != CVI_SUCCESS)
		return CVI_FAILURE;

	stSnsAttrInfo.eSensorId = PR2100_ID;

	s32Ret  = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
	s32Ret |= CVI_ISP_SensorRegCallBack(ViPipe, &stSnsAttrInfo, &stIspRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_unregister_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	(void) pstAeLib;
	(void) pstAwbLib;

	CVI_S32 s32Ret;

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, PR2100_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	sensor_ctx_exit(ViPipe);

	return CVI_SUCCESS;
}

static CVI_S32 sensor_set_init(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	g_au16Pr2100_BdgMuxMode[ViPipe] = pstInitAttr->enSnsBdgMuxMode;

	return CVI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsPR2100_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = CVI_NULL,
	.pfnRestart             = CVI_NULL,
	.pfnMirrorFlip          = CVI_NULL,
	.pfnWriteReg            = pr2100_write_register,
	.pfnReadReg             = pr2100_read_register,
	.pfnSetBusInfo          = pr2100_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr         = sensor_patch_rx_attr,
	.pfnPatchI2cAddr        = sensor_patch_i2c_addr,
	.pfnGetRxAttr           = sensor_rx_attr,
	.pfnExpSensorCb         = cmos_init_sensor_exp_function,
	.pfnExpAeCb             = CVI_NULL,
	.pfnSnsProbe            = CVI_NULL,
};

