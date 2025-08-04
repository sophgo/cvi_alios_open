#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cvi_type.h"
#include "cvi_ipcmsg.h"
#include "msg_isp.h"
#include "msg_ctx.h"
//#include "isp_mgr_buf.h"
#include "cvi_comm_isp.h"
#include "3A_internal.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_af.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "cvi_sns_ctrl.h"
#include "sensor_cfg.h"
#include "cvi_debug.h"
#if ENABLE_AF_LIB
#include "af_motor.h"
#endif
#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define CHECK_MSG_SIZE(type, size) \
	if (sizeof(type) != size) { \
		CVI_TRACE_MSG(CVI_DBG_ERR, "size mismatch !!! expect size: %lu, actual size: %u\n", sizeof(type), size); \
		return CVI_FAILURE; \
	}

static int start_isp(VI_PIPE ViPipe)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	VI_DEV_ATTR_S stViDevAttr;

	memset(&stViDevAttr, 0, sizeof(VI_DEV_ATTR_S));

	s32Ret = CVI_VI_GetDevAttr(ViPipe, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VI_GetDevAttr failed!\n");
	}

	//Register callback & call API
	if (stViDevAttr.enInputDataType != VI_DATA_TYPE_YUV) {
		ALG_LIB_S stAeLib, stAwbLib;

		stAeLib.s32Id = stAwbLib.s32Id = ViPipe;

		ISP_BIND_ATTR_S stBindAttr;

		stBindAttr.stAeLib.s32Id = ViPipe;
		stBindAttr.stAwbLib.s32Id = ViPipe;

		strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
		strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
		strncpy(stBindAttr.stAeLib.acLibName, CVI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
		strncpy(stBindAttr.stAwbLib.acLibName, CVI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

		s32Ret = CVI_AE_Register(ViPipe, &stAeLib);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "AE Algo register failed!, error: %d\n",	s32Ret);
			return s32Ret;
		}
		s32Ret = CVI_AWB_Register(ViPipe, &stAwbLib);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "AWB Algo register failed!, error: %d\n", s32Ret);
			return s32Ret;
		}

#if ENABLE_AF_LIB
		ALG_LIB_S stAfLib;
		ISP_AF_MOTOR_FUNC_S motorCb = {0};

		stAfLib.s32Id = ViPipe;
		stBindAttr.stAfLib.s32Id = ViPipe;
		strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
		strncpy(stBindAttr.stAfLib.acLibName, CVI_AF_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

		s32Ret = CVI_AF_Register(ViPipe, &stAfLib);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "AF Algo register failed!, error: %d\n", s32Ret);
			return s32Ret;
		}

		//register control motor cb func if you use sophgo af algo
		//you can implement control motor cb func by yourself
		//use sophgo cb func for example
		motorCb.pfn_af_set_zoom_in = SAMPLE_COMM_ISP_Motor_SetZoomInCb;
		motorCb.pfn_af_set_zoom_out = SAMPLE_COMM_ISP_Motor_SetZoomOutCb;
		motorCb.pfn_af_set_zoom_speed = SAMPLE_COMM_ISP_Motor_SetZoomSpeedCb;
		motorCb.pfn_af_set_focus_in = SAMPLE_COMM_ISP_Motor_SetFocusInCb;
		motorCb.pfn_af_set_focus_out = SAMPLE_COMM_ISP_Motor_SetFocusOutCb;
		motorCb.pfn_af_set_focus_speed = SAMPLE_COMM_ISP_Motor_SetFocusSpeedCb;
		motorCb.pfn_af_set_zoom_focus = SAMPLE_COMM_ISP_Motor_SetZoomAndFocusCb;
		motorCb.pfn_af_get_len_info = SAMPLE_COMM_ISP_Motor_GetLensInfoCb;
		CVI_AF_MOTOR_Register(ViPipe, &motorCb);
#endif

		s32Ret = CVI_ISP_SetBindAttr(ViPipe, &stBindAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "Bind Algo failed with %d!\n", s32Ret);
		}

		s32Ret = CVI_ISP_Init(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "ISP Init failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		//Run ISP
		s32Ret = CVI_ISP_Run(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "ISP Run failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	} else {
		s32Ret = CVI_ISP_Init(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "ISP Init failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

static int stop_isp(VI_PIPE ViPipe)
{
	//Param init
	CVI_S32 s32Ret;
	VI_DEV_ATTR_S stViDevAttr;

	memset(&stViDevAttr, 0, sizeof(VI_DEV_ATTR_S));

	s32Ret = CVI_VI_GetDevAttr(ViPipe, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VI_GetDevAttr failed!\n");
	}

	if (stViDevAttr.enInputDataType != VI_DATA_TYPE_YUV) {
		ALG_LIB_S stAeLib, stAwbLib;

		stAeLib.s32Id = stAwbLib.s32Id = ViPipe;
		strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
		strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
		//Stop ISP
		s32Ret = CVI_ISP_Exit(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "ISP Exit failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_AE_UnRegister(ViPipe, &stAeLib);
		if (s32Ret) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "AE Algo unRegister failed!, error: %d\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_AWB_UnRegister(ViPipe, &stAwbLib);
		if (s32Ret) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "AWB Algo unRegister failed!, error: %d\n", s32Ret);
			return s32Ret;
		}

#if ENABLE_AF_LIB
		ALG_LIB_S stAfLib;
		ISP_AF_MOTOR_FUNC_S motorCb;

		stAfLib.s32Id = ViPipe;
		strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

		CVI_AF_MOTOR_UnRegister(ViPipe, &motorCb);

		s32Ret = CVI_AF_UnRegister(ViPipe, &stAfLib);
		if (s32Ret) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "AF Algo unRegister failed!, error: %d\n", s32Ret);
			return s32Ret;
		}
#endif
	}

	return s32Ret;
}

extern CVI_S32 isp_mgr_buf_get_shared_buf_paddr(VI_PIPE ViPipe, CVI_U64 *paddr);

static CVI_S32 MSG_ISP_Init(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_BOOL bIspRun;

	CVI_ISP_GetIspRunState(ViPipe, &bIspRun);

	if (bIspRun == 0) {
		s32Ret = start_isp(ViPipe);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_Exit(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U64 u64PhyAddr = 0x00;

	s32Ret = isp_mgr_buf_get_shared_buf_paddr(ViPipe, &u64PhyAddr);

	if (s32Ret == CVI_SUCCESS) {
		s32Ret = stop_isp(ViPipe);
	}
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_MemInit(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U64 u64PhyAddr = 0x00;

	s32Ret = CVI_ISP_MemInit(ViPipe);
	s32Ret |= isp_mgr_buf_get_shared_buf_paddr(ViPipe, &u64PhyAddr);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &u64PhyAddr, sizeof(CVI_U64));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_GetVDTimeout(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_VD_TYPE_E enIspVDType = (ISP_VD_TYPE_E) pstMsg->as32PrivData[0];
	CVI_U32 u32MilliSec = pstMsg->as32PrivData[1];

	s32Ret = CVI_ISP_GetVDTimeOut(ViPipe, enIspVDType, u32MilliSec);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_SetSmartInfo(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U8 TimeOut = pstMsg->as32PrivData[0];

	s32Ret = CVI_ISP_SetSmartInfo(ViPipe, (ISP_SMART_INFO_S *) pstMsg->pBody, TimeOut);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_GetSmartInfo(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);

	ISP_SMART_INFO_S info;

	s32Ret = CVI_ISP_GetSmartInfo(ViPipe, &info);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &info, sizeof(ISP_SMART_INFO_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_GetIonInfo(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_S32 total;
	CVI_S32 used;
	CVI_S32 mfree;
	CVI_S32 peak;
	CVI_S32 dummy;

	s32Ret = CVI_ISP_GetIonInfo(&total, &used, &mfree, &peak);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_ISP_GetAERawReplayFrmNum Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &dummy, sizeof(dummy));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	respMsg->as32PrivData[0] = total;
	respMsg->as32PrivData[1] = used;
	respMsg->as32PrivData[2] = mfree;
	respMsg->as32PrivData[3] = peak;

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AEGetLogBufSize(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;

	s32Ret = CVI_ISP_GetAELogBufSize(ViPipe, &bufSize);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, bufSize, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AEGetLogBuf(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;
	CVI_U64 u64TempPhyAddr;
	CVI_U8 *pbuf;

	bufSize = pstMsg->as32PrivData[0];
	memcpy(&u64TempPhyAddr, pstMsg->pBody, sizeof(CVI_U64));

	pbuf = (CVI_U8 *) u64TempPhyAddr;

	s32Ret = CVI_ISP_GetAELogBuf(ViPipe, pbuf, bufSize);

	CVI_SYS_IonFlushCache(u64TempPhyAddr, (CVI_VOID *) pbuf, bufSize);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AEGetBinBufSize(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;

	s32Ret = CVI_ISP_GetAEBinBufSize(ViPipe, &bufSize);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, bufSize, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AESetRawDumpFrameID(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 fid = pstMsg->as32PrivData[0];
	CVI_U16 frmNum = pstMsg->as32PrivData[1];

	s32Ret = CVI_ISP_AESetRawDumpFrameID(ViPipe, fid, frmNum);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AE_GetRawReplayExpBuf(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;
	CVI_U32 return_bufSize;
	CVI_U64 u64TempPhyAddr;
	CVI_U8 *pbuf;

	bufSize = pstMsg->as32PrivData[0];

	memcpy(&u64TempPhyAddr, pstMsg->pBody, sizeof(CVI_U64));

	pbuf = (CVI_U8 *) u64TempPhyAddr;

	s32Ret = CVI_ISP_AEGetRawReplayExpBuf(ViPipe, pbuf, &return_bufSize);

	CVI_SYS_IonFlushCache(u64TempPhyAddr, (CVI_VOID *) pbuf, bufSize);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &u64TempPhyAddr, sizeof(CVI_U64));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	respMsg->as32PrivData[0] = return_bufSize;

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AESetRawReplayExposure(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);

	CHECK_MSG_SIZE(ISP_EXP_INFO_S, pstMsg->u32BodyLen);
	s32Ret = CVI_ISP_AESetRawReplayExposure(ViPipe, (ISP_EXP_INFO_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_ISP_AESetRawReplayExposure Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_ISP_AEGetBinBuf(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;
	CVI_U64 u64TempPhyAddr;
	CVI_U8 *pbuf;

	bufSize = pstMsg->as32PrivData[0];
	memcpy(&u64TempPhyAddr, pstMsg->pBody, sizeof(CVI_U64));

	pbuf = (CVI_U8 *) u64TempPhyAddr;

	s32Ret = CVI_ISP_GetAEBinBuf(ViPipe, pbuf, bufSize);

	CVI_SYS_IonFlushCache(u64TempPhyAddr, (CVI_VOID *) pbuf, bufSize);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AWBGetLogBuf(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;
	CVI_U64 u64TempPhyAddr;
	CVI_U8 *pbuf;

	bufSize = pstMsg->as32PrivData[0];
	memcpy(&u64TempPhyAddr, pstMsg->pBody, sizeof(CVI_U64));

	pbuf = (CVI_U8 *) u64TempPhyAddr;

	s32Ret = CVI_ISP_GetAWBSnapLogBuf(ViPipe, pbuf, bufSize);

	CVI_SYS_IonFlushCache(u64TempPhyAddr, (CVI_VOID *) pbuf, bufSize);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AWBGetBinSize(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	//VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;

	bufSize = CVI_ISP_GetAWBDbgBinSize();

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, bufSize, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AWBGetBinBuf(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;
	CVI_U64 u64TempPhyAddr;
	CVI_U8 *pbuf;

	bufSize = pstMsg->as32PrivData[0];
	memcpy(&u64TempPhyAddr, pstMsg->pBody, sizeof(CVI_U64));

	pbuf = (CVI_U8 *) u64TempPhyAddr;

	s32Ret = CVI_ISP_GetAWBDbgBinBuf(ViPipe, pbuf, bufSize);

	CVI_SYS_IonFlushCache(u64TempPhyAddr, (CVI_VOID *) pbuf, bufSize);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AEGetFrameID(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 frameID;

	s32Ret = CVI_ISP_GetFrameID(ViPipe, &frameID);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, frameID, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AEGetFps(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_FLOAT fps;

	s32Ret = CVI_ISP_QueryFps(ViPipe, &fps);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, fps * 100, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AEGetLVX100(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_S16 s16Lv;

	s32Ret = CVI_ISP_GetCurrentLvX100(ViPipe, &s16Lv);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s16Lv, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AESetFastBootExposure(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 expLine = pstMsg->as32PrivData[0];
	CVI_U32 again = pstMsg->as32PrivData[1];
	CVI_U32 dgain = pstMsg->as32PrivData[2];
	CVI_U32 ispdgain = pstMsg->as32PrivData[3];

	s32Ret = CVI_ISP_SetFastBootExposure(ViPipe, expLine, again, dgain, ispdgain);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AESetAeSimMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_BOOL bMode = pstMsg->as32PrivData[0];

	CVI_AE_SetAeSimMode(bMode);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}


static CVI_S32 MSG_ISP_AWBSetSimMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_BOOL bMode = pstMsg->as32PrivData[0];

	CVI_ISP_SetAwbSimMode(bMode);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AWBGetRunStatus(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_S32 dummy;
	CVI_BOOL bState;

	bState = CVI_ISP_GetAwbRunStatus(ViPipe);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &dummy, sizeof(dummy));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	respMsg->as32PrivData[0] = bState;

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AWBSetRunStatus(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_BOOL bState = pstMsg->as32PrivData[0];

	CVI_ISP_SetAwbRunStatus(ViPipe, bState);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AFAutoFocus(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_ISP_AFAutoFocus(ViPipe);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AFGetFv(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 fv;

	s32Ret = CVI_ISP_AFGetFv(ViPipe, &fv);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, fv, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AFSetZoomSpeed(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_AF_MOTOR_SPEED_E eSpeed = pstMsg->as32PrivData[0];

	s32Ret = CVI_ISP_AFSetZoomSpeed(ViPipe, eSpeed);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AFSetZoom(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_BOOL direct = pstMsg->as32PrivData[0];
	CVI_U8 step = pstMsg->as32PrivData[1];

	s32Ret = CVI_ISP_AFSetZoom(ViPipe, direct, step);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AFSetFocusSpeed(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_AF_MOTOR_SPEED_E eSpeed = pstMsg->as32PrivData[0];

	s32Ret = CVI_ISP_AFSetFocusSpeed(ViPipe, eSpeed);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AFSetFocus(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_BOOL direct = pstMsg->as32PrivData[0];
	CVI_U8 step = pstMsg->as32PrivData[1];

	s32Ret = CVI_ISP_AFSetFocus(ViPipe, direct, step);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_AFQueryFocusInfo(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U32 bufSize;
	CVI_U64 u64TempPhyAddr;
	ISP_FOCUS_Q_INFO_S *pbuf;

	bufSize = pstMsg->as32PrivData[0];
	memcpy(&u64TempPhyAddr, pstMsg->pBody, sizeof(CVI_U64));

	pbuf = (ISP_FOCUS_Q_INFO_S *) u64TempPhyAddr;

	s32Ret = CVI_ISP_AFQueryFocusInfo(ViPipe, pbuf);

	CVI_SYS_IonFlushCache(u64TempPhyAddr, (CVI_VOID *) pbuf, bufSize);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_SetFMWState(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_FMW_STATE_E enState = (ISP_FMW_STATE_E) pstMsg->as32PrivData[0];

	s32Ret = CVI_ISP_SetFMWState(ViPipe, enState);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_GetFMWState(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	ISP_FMW_STATE_E enState;

	s32Ret = CVI_ISP_GetFMWState(ViPipe, &enState);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, enState, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_SetBypassFrm(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U8 bypassNum = pstMsg->as32PrivData[0];

	s32Ret = CVI_ISP_SetBypassFrm(ViPipe, bypassNum);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_ISP_GetBypassFrm(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_PIPE ViPipe = GET_DEV_ID(pstMsg->u32Module);
	CVI_U8 bypassNum;

	s32Ret = CVI_ISP_GetBypassFrm(ViPipe, &bypassNum);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "create respMsg error\n");
	}

	respMsg->as32PrivData[0] = bypassNum;

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "send msg fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stIspCmdTable[] = {
	{ MSG_CMD_ISP_INIT, MSG_ISP_Init},
	{ MSG_CMD_ISP_EXIT, MSG_ISP_Exit},
	{ MSG_CMD_ISP_MEM_INIT, MSG_ISP_MemInit},
	{ MSG_CMD_ISP_SET_FMW_STATE, MSG_ISP_SetFMWState},
	{ MSG_CMD_ISP_GET_FMW_STATE, MSG_ISP_GetFMWState},
	{ MSG_CMD_ISP_GET_VD_TIMEOUT, MSG_ISP_GetVDTimeout},
	{ MSG_CMD_ISP_SET_SMART_INFO, MSG_ISP_SetSmartInfo},
	{ MSG_CMD_ISP_GET_SMART_INFO, MSG_ISP_GetSmartInfo},
	{ MSG_CMD_ISP_GET_ION_INFO, MSG_ISP_GetIonInfo},
	{ MSG_CMD_ISP_AE_GET_BUF_SIZE, MSG_ISP_AEGetLogBufSize},
	{ MSG_CMD_ISP_AE_GET_BUF, MSG_ISP_AEGetLogBuf},
	{ MSG_CMD_ISP_AE_SET_RAW_DUMP_FID,  MSG_ISP_AESetRawDumpFrameID},
	{ MSG_CMD_ISP_AE_GET_RAW_REPLAY_EXP_BUF, MSG_ISP_AE_GetRawReplayExpBuf},
	{ MSG_CMD_ISP_AE_SET_RAW_REPLAY_EXPOSURE, MSG_ISP_AESetRawReplayExposure},
	{ MSG_CMD_ISP_AE_GET_BIN_SIZE, MSG_ISP_AEGetBinBufSize},
	{ MSG_CMD_ISP_AE_GET_BIN_BUF, MSG_ISP_AEGetBinBuf},
	{ MSG_CMD_ISP_AE_GET_FRAME_ID, MSG_ISP_AEGetFrameID},
	{ MSG_CMD_ISP_AE_GET_FPS, MSG_ISP_AEGetFps},
	{ MSG_CMD_ISP_AE_GET_LVX100, MSG_ISP_AEGetLVX100},
	{ MSG_CMD_ISP_AE_SET_FASTBOOT_EXPOSURE, MSG_ISP_AESetFastBootExposure},
	{ MSG_CMD_ISP_AE_SET_SIM_MODE, MSG_ISP_AESetAeSimMode},
	{ MSG_CMD_ISP_AWB_GET_LOG_BUF, MSG_ISP_AWBGetLogBuf},
	{ MSG_CMD_ISP_AWB_GET_BIN_SIZE, MSG_ISP_AWBGetBinSize},
	{ MSG_CMD_ISP_AWB_GET_BIN_BUF, MSG_ISP_AWBGetBinBuf},
	{ MSG_CMD_ISP_AWB_SET_SIM_MODE, MSG_ISP_AWBSetSimMode},
	{ MSG_CMD_ISP_AWB_GET_RUN_STATUS, MSG_ISP_AWBGetRunStatus},
	{ MSG_CMD_ISP_AWB_SET_RUN_STATUS, MSG_ISP_AWBSetRunStatus},
	{ MSG_CMD_ISP_AF_AUTO_FOCUS, MSG_ISP_AFAutoFocus},
	{ MSG_CMD_ISP_AF_GET_FV, MSG_ISP_AFGetFv},
	{ MSG_CMD_ISP_AF_SET_ZOOM_SPEED, MSG_ISP_AFSetZoomSpeed},
	{ MSG_CMD_ISP_AF_SET_ZOOM, MSG_ISP_AFSetZoom},
	{ MSG_CMD_ISP_AF_SET_FOCUS_SPEED, MSG_ISP_AFSetFocusSpeed},
	{ MSG_CMD_ISP_AF_SET_FOCUS, MSG_ISP_AFSetFocus},
	{ MSG_CMD_ISP_AF_QUERY_FOCUS_INFO, MSG_ISP_AFQueryFocusInfo},
	{ MSG_CMD_ISP_SET_BYPASS_FRM, MSG_ISP_SetBypassFrm},
	{ MSG_CMD_ISP_GET_BYPASS_FRM, MSG_ISP_GetBypassFrm},
};

MSG_SERVER_MODULE_S g_stModuleIsp = {
	CVI_ID_ISP,
	"isp",
	sizeof(g_stIspCmdTable) / sizeof(MSG_MODULE_CMD_S),
	&g_stIspCmdTable[0]
};

MSG_SERVER_MODULE_S *MSG_GetIspMod(CVI_VOID)
{
	return &g_stModuleIsp;
}


