#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cvi_type.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_mipi.h"
#include "cvi_ipcmsg.h"
#include "msg_sensor.h"
#include "msg_ctx.h"
#include "cvi_debug.h"
#include "cvi_sensor.h"
#include "cvi_msg_server.h"

#define CHECK_MSG_SIZE(type, size)												\
	if (sizeof(type) != size) {												\
		CVI_TRACE_MSG(CVI_DBG_ERR, "size mismatch !!! expect size: %lu, actual size: %u\n", sizeof(type), size);			\
		return CVI_FAILURE;												\
	}

static pthread_t g_sensorDet_thid[VI_MAX_PIPE_NUM];
static CVI_S32 g_AhdMode[VI_MAX_PIPE_NUM] = {[0 ... VI_MAX_PIPE_NUM-1] = -1};

static CVI_S32 MSG_SENSOR_SetAhdMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_S32 AhdMode = pstMsg->as32PrivData[0];

	s32Ret = CVI_SNS_SetAHDMode(ViPipe, AhdMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_SNS_SetAHDMode fail\n");
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetAhdInit(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	bool isFirstInit = pstMsg->as32PrivData[0];

	s32Ret = CVI_SNS_SetAHDInit(ViPipe, isFirstInit);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_SNS_SetAHDInit fail\n");
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetAhdDeInit(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_SNS_SetAHDDeInit(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_SNS_SetAHDDeInit fail\n");
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}
static CVI_S32 MSG_SENSOR_GetAhdMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	SNS_AHD_MODE_S *ahd_mode = (SNS_AHD_MODE_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(SNS_AHD_MODE_S, pstMsg->u32BodyLen);
	if (!ahd_mode) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ahd_mode is NULL\n");
		return CVI_FAILURE;
	}

	*ahd_mode = CVI_SNS_GetAHDMode(ViPipe);
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, ahd_mode, sizeof(SNS_AHD_MODE_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, pipe:%d\n", ViPipe);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, ViPipe:%d\n", ViPipe);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetAhdBusInfo(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_S32 astI2cDev = pstMsg->as32PrivData[0];
	CHECK_MSG_SIZE(CVI_S32, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_SetAHDBusInfo(ViPipe, astI2cDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_SNS_SetAHDBusInfo fail\n");
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_DetctAhdStatus(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_S32 *ahdType = (CVI_S32 *)pstMsg->pBody;
	CVI_S32 ahdOldType = pstMsg->as32PrivData[0];
	CHECK_MSG_SIZE(CVI_S32, pstMsg->u32BodyLen);
	if (!ahdType) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ahdType is NULL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_SNS_DetectAhdStatus(ViPipe, ahdOldType, ahdType);
	if (s32Ret!= CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_SNS_DetectAhdStatus fail\n");
	}
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, ahdType, sizeof(CVI_S32));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, pipe:%d\n", ViPipe);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, ViPipe:%d\n", ViPipe);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static void *device_auto_detect(void *arg)
{
	VI_PIPE ViPipe = *(CVI_U8 *)arg;
	free(arg);
	MSG_PRIV_DATA_S stPrivData = {0};
	CVI_S32 signal_type = -1;
	CVI_S32 signal_type_old = g_AhdMode[ViPipe];
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, 0, 0);
	CVI_U32 detect_cnt = 0;

	// CVI_SNS_SetAHDInit(ViPipe, false);
	while (1) {
		usleep(100 * 1000);
		signal_type = CVI_SNS_GetAHDMode(ViPipe);
		if (signal_type_old == signal_type) {
			detect_cnt = 0;
			continue;
		}
		if (signal_type_old != signal_type && detect_cnt < 3) {
			detect_cnt++;
			continue;
		}

		CVI_TRACE_SNS(CVI_DBG_WARN, "send signal ======== %d ========\n", signal_type);
		stPrivData.as32PrivData[0] = signal_type;
		stPrivData.as32PrivData[1] = ViPipe;
		s32Ret = CVI_MSG_SendSync_CB(u32ModFd, 0, CVI_NULL, 0, &stPrivData);
		if (s32Ret != CVI_SUCCESS)
			CVI_TRACE_SNS(CVI_DBG_ERR, "send ahd message [%d] fail!!!\n", stPrivData.as32PrivData[0]);
		g_AhdMode[ViPipe] = signal_type_old = signal_type;
	}
	return NULL;
}

static CVI_S32 MSG_SENSOR_EnableAhdThread(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_S32 ViPipe = pstMsg->as32PrivData[0];
	CVI_U8 *arg = malloc(sizeof(*arg));
	CHECK_MSG_SIZE(CVI_S32, pstMsg->u32BodyLen);

	*arg = ViPipe;
	s32Ret = pthread_create(&g_sensorDet_thid[ViPipe], NULL, device_auto_detect, arg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create_thread Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_GetAhdStatus(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	SNS_STATUS_MSG_S *status = (SNS_STATUS_MSG_S *)pstMsg->pBody;

	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CHECK_MSG_SIZE(SNS_STATUS_MSG_S, pstMsg->u32BodyLen);

	if (ViPipe != status->s32SnsId) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get ahd ststus ViPipe:%d, status->s32SnsId:%d, not equal\n",
					ViPipe, status->s32SnsId);
		return CVI_FAILURE;
	}

	s32Ret = CVI_SNS_GetAhdStatus(status);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_SNS_GetAhdStatus fail\n");
	}
	g_AhdMode[ViPipe] = status->eMode;
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, status, sizeof(SNS_STATUS_MSG_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, pipe:%d\n", ViPipe);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, ViPipe:%d\n", ViPipe);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetRxAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	RX_INIT_ATTR_S *stRxInitAttr = (RX_INIT_ATTR_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(RX_INIT_ATTR_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_SetSnsRxAttr(ViPipe, stRxInitAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetRxAttr callback failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsI2c(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_S32 astI2cDev = pstMsg->as32PrivData[0];
	CVI_S32 s32I2cAddr = pstMsg->as32PrivData[1];

	s32Ret = CVI_SNS_SetSnsI2c(ViPipe, astI2cDev, s32I2cAddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsI2c callback failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetIspAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_INIT_ATTR_S *stIspAttr = (ISP_INIT_ATTR_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(ISP_INIT_ATTR_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_SetSnsIspAttr(ViPipe, stIspAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetIspAttr callback failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_RegCallback(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_DEV IspDev = pstMsg->as32PrivData[0];

	s32Ret = CVI_SNS_RegCallback(ViPipe, IspDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_RegCallback Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_UnRegCallback(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_DEV IspDev = pstMsg->as32PrivData[0];

	s32Ret = CVI_SNS_UnRegCallback(ViPipe, IspDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_UnRegCallback Failed : %#x!\n", s32Ret);
		}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsImgMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_CMOS_SENSOR_IMAGE_MODE_S *stSnsrMode = (ISP_CMOS_SENSOR_IMAGE_MODE_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(ISP_CMOS_SENSOR_IMAGE_MODE_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_SetSnsImgMode(ViPipe, stSnsrMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsImgMode callback failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsWdrMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	WDR_MODE_E wdrMode = pstMsg->as32PrivData[0];

	s32Ret = CVI_SNS_SetSnsWdrMode(ViPipe, wdrMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsWdrMode Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_GetSnsRxAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	SNS_COMBO_DEV_ATTR_S *stDevAttr = (SNS_COMBO_DEV_ATTR_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(SNS_COMBO_DEV_ATTR_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_GetSnsRxAttr(ViPipe, stDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_GetSnsRxAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stDevAttr, sizeof(SNS_COMBO_DEV_ATTR_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsProbe(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_SNS_SetSnsProbe(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsProbe Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_RstSnsGpio(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 rstEnable = pstMsg->as32PrivData[0];
	sns_rst_config *rst_config = (sns_rst_config *)pstMsg->pBody;
	CVI_U32 reset_port, reset_pin, reset_pol, devno;
	CHECK_MSG_SIZE(sns_rst_config, pstMsg->u32BodyLen);

	reset_port = rst_config->gpio_port;
	reset_pin = rst_config->gpio_pin;
	reset_pol = rst_config->gpio_active;
	devno = rst_config->devno;

	s32Ret = CVI_MIPI_SetSensorReset(devno, reset_port, reset_pin, reset_pol, rstEnable);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_RstSnsGpio Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_RstMipi(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 devNo = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 reset = pstMsg->as32PrivData[0];

	s32Ret = CVI_MIPI_SetMipiReset(devNo, reset);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_RstMipi Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetMipiAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	SNS_COMBO_DEV_ATTR_S *stDevAttr = (SNS_COMBO_DEV_ATTR_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(SNS_COMBO_DEV_ATTR_S, pstMsg->u32BodyLen);
	CVI_U32 devno = pstMsg->as32PrivData[0];



	s32Ret = CVI_MIPI_SetMipiAttr(devno, stDevAttr);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetMipiAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_EnableSnsClk(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 devNo = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 clkEnable = pstMsg->as32PrivData[0];

	s32Ret = CVI_MIPI_SetSensorClock(devNo, clkEnable);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_EnableSnsClk Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsMclk(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	struct mclk_pll_s *mclk = (struct mclk_pll_s *)pstMsg->pBody;
	CHECK_MSG_SIZE(struct mclk_pll_s, pstMsg->u32BodyLen);

	s32Ret = CVI_MIPI_SetSnsMclk(mclk);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsMclk Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetOutClkEdge(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_S32 devno;
	CVI_S32 is_up;

	struct clk_edge_s *clk = (struct clk_edge_s *)pstMsg->pBody;
	CHECK_MSG_SIZE(struct clk_edge_s, pstMsg->u32BodyLen);

	devno = clk->devno;
	is_up = clk->edge;

	s32Ret = CVI_MIPI_SetClkEdge(devno, is_up);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetOutClkEdge Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsStandby(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_SNS_SetSnsStandby(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsStandby Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsRestart(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_SNS_SetSnsRestart(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsRestart Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsInit(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_SNS_SetSnsInit(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsInit Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetVIFlipMirrorCB(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	VI_DEV ViDev = pstMsg->as32PrivData[0];

	s32Ret = CVI_SNS_SetVIFlipMirrorCB(ViPipe, ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetVIFlipMirrorCB Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_GetAeDefault(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	AE_SENSOR_DEFAULT_S *stAeDefault = (AE_SENSOR_DEFAULT_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(AE_SENSOR_DEFAULT_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_GetAeDefault(ViPipe, stAeDefault);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_GetAeDefault Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stAeDefault, sizeof(AE_SENSOR_DEFAULT_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_GetIspBlkLev(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_CMOS_BLACK_LEVEL_S *stBlc = (ISP_CMOS_BLACK_LEVEL_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(ISP_CMOS_BLACK_LEVEL_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_GetIspBlkLev(ViPipe, stBlc);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_GetIspBlkLev Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stBlc, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetSnsFps(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U8 fps = GET_CHN_ID(pstMsg->u32Module);
	AE_SENSOR_DEFAULT_S *stSnsDft = (AE_SENSOR_DEFAULT_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(AE_SENSOR_DEFAULT_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_SetSnsFps(ViPipe, fps, stSnsDft);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetSnsFps Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stSnsDft, sizeof(AE_SENSOR_DEFAULT_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_GetExpRatio(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	SNS_EXP_MAX_S *stExpMax = (SNS_EXP_MAX_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(SNS_EXP_MAX_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_GetExpRatio(ViPipe, stExpMax);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_GetExpRatio Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stExpMax, sizeof(SNS_EXP_MAX_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetDgainCalc(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	SNS_GAIN_S *stDgain = (SNS_GAIN_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(SNS_GAIN_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_SetDgainCalc(ViPipe, stDgain);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetDgainCalc Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stDgain, sizeof(SNS_GAIN_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetAgainCalc(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	SNS_GAIN_S *stAgain = (SNS_GAIN_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(SNS_GAIN_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_SetAgainCalc(ViPipe, stAgain);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetAgainCalc Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stAgain, sizeof(SNS_GAIN_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_GetConfigInfo(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	SENSOR_CFG_S *sensor_cfg = (SENSOR_CFG_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(SENSOR_CFG_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_GetConfigInfo(sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_GetConfigInfo Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, sensor_cfg, sizeof(SENSOR_CFG_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SENSOR_SetDrvCfg(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	SENSOR_CFG_S *sensor_cfg = (SENSOR_CFG_S *)pstMsg->pBody;
	CHECK_MSG_SIZE(SENSOR_CFG_S, pstMsg->u32BodyLen);

	s32Ret = CVI_SNS_SetSnsDrvCfg(sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "MSG_SENSOR_SetDrvCfg Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, CVI_NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stSensorCmdTable[] = {
	{ MSG_CMD_SENSOR_EN_AHD_THREAD,				MSG_SENSOR_EnableAhdThread },
	{ MSG_CMD_SENSOR_GET_STATUS,				MSG_SENSOR_GetAhdStatus },
	{ MSG_CMD_SENSOR_SET_AHD_INIT,				MSG_SENSOR_SetAhdInit },
	{ MSG_CMD_SENSOR_SET_AHD_DEINIT,			MSG_SENSOR_SetAhdDeInit },
	{ MSG_CMD_SENSOR_SET_AHD_MODE,				MSG_SENSOR_SetAhdMode },
	{ MSG_CMD_SENSOR_GET_AHD_MODE,				MSG_SENSOR_GetAhdMode },
	{ MSG_CMD_SENSOR_SET_AHD_BUSINFO,			MSG_SENSOR_SetAhdBusInfo },
	{ MSG_CMD_SENSOR_DETECT_AHD_STATUS,			MSG_SENSOR_DetctAhdStatus },
	{ MSG_CMD_SENSOR_SET_RX_ATTR,				MSG_SENSOR_SetRxAttr },
	{ MSG_CMD_SENSOR_SET_SNS_I2C,				MSG_SENSOR_SetSnsI2c },
	{ MSG_CMD_SENSOR_SET_ISP_INIT,				MSG_SENSOR_SetIspAttr },
	{ MSG_CMD_SENSOR_ISP_REG_CB,				MSG_SENSOR_RegCallback },
	{ MSG_CMD_SENSOR_ISP_UNREG_CB,				MSG_SENSOR_UnRegCallback },
	{ MSG_CMD_SENSOR_SET_IMG_MODE,				MSG_SENSOR_SetSnsImgMode },
	{ MSG_CMD_SENSOR_SET_WDR_MODE,				MSG_SENSOR_SetSnsWdrMode },
	{ MSG_CMD_SENSOR_GET_RX_ATTR,				MSG_SENSOR_GetSnsRxAttr },
	{ MSG_CMD_SENSOR_SET_SNS_PROBE,				MSG_SENSOR_SetSnsProbe },
	{ MSG_CMD_SENSOR_RESET_GPIO,				MSG_SENSOR_RstSnsGpio },
	{ MSG_CMD_SENSOR_RESET_MIPI,				MSG_SENSOR_RstMipi },
	{ MSG_CMD_SENSOR_SET_MIPI_ATTR,				MSG_SENSOR_SetMipiAttr },
	{ MSG_CMD_SENSOR_EN_SNS_CLK,				MSG_SENSOR_EnableSnsClk },
	{ MSG_CMD_SENSOR_SET_SNS_CLK,				MSG_SENSOR_SetSnsMclk },
	{ MSG_CMD_SENSOR_SET_OUTPUT_CLK_EDGE,		MSG_SENSOR_SetOutClkEdge },
	{ MSG_CMD_SENSOR_SET_SNS_STANDBY,			MSG_SENSOR_SetSnsStandby },
	{ MSG_CMD_SENSOR_SET_SNS_RESTART,			MSG_SENSOR_SetSnsRestart },
	{ MSG_CMD_SENSOR_SET_SNS_INIT,				MSG_SENSOR_SetSnsInit },
	{ MSG_CMD_SENSOR_SET_FLIPMIRROR_CB,			MSG_SENSOR_SetVIFlipMirrorCB },
	{ MSG_CMD_SENSOR_GET_AE_DEFAULT,			MSG_SENSOR_GetAeDefault },
	{ MSG_CMD_SENSOR_GET_BLK_LEVEL,				MSG_SENSOR_GetIspBlkLev },
	{ MSG_CMD_SENSOR_SET_SNS_FPS,				MSG_SENSOR_SetSnsFps },
	{ MSG_CMD_SENSOR_GET_EXP_RAT,				MSG_SENSOR_GetExpRatio },
	{ MSG_CMD_SENSOR_SET_DGAIN_CALC,			MSG_SENSOR_SetDgainCalc },
	{ MSG_CMD_SENSOR_SET_AGAIN_CALC,			MSG_SENSOR_SetAgainCalc },
	{ MSG_CMD_SENSOR_GET_CONFIG_INFO,			MSG_SENSOR_GetConfigInfo },
	{ MSG_CMD_SENSOR_SET_DRV_CFG,				MSG_SENSOR_SetDrvCfg },
};

MSG_SERVER_MODULE_S g_stModuleSensor = {
	CVI_ID_SENSOR,
	"sensor",
	sizeof(g_stSensorCmdTable) / sizeof(MSG_MODULE_CMD_S),
	&g_stSensorCmdTable[0]
};

MSG_SERVER_MODULE_S *MSG_GetSensorMod(CVI_VOID)
{
	return &g_stModuleSensor;
}
