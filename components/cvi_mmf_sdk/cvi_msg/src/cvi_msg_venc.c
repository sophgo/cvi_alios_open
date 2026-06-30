#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cvi_type.h"
#include "cvi_venc.h"
#include "cvi_ipcmsg.h"
#include "msg_venc.h"
#include "msg_ctx.h"
#include "cvi_datafifo.h"
#include "sys/prctl.h"
#include "aos/kernel.h"
#include "cvi_debug.h"


#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

typedef struct _VENC_STREAM_PACK_S {
	VENC_PACK_S pstPack[8];
	VENC_STREAM_S stStream;
	VENC_CHN VeChn;
} VENC_STREAM_PACK_S;

//#define FLOW_DEBUG 1
#ifdef FLOW_DEBUG
#define CVI_MSG_API(msg, ...)		\
	do { \
		if (1) \
			printf("[API] %s = %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define CVI_MSG_VENC_API_IN  CVI_MSG_API("Chn:%d In\n", VeChn);
#define CVI_MSG_VENC_API_OUT CVI_MSG_API("Chn:%d Out\n", VeChn);

#define DATA_DUMP(data, len)  data_dump((unsigned char *)data, len, __FUNCTION__, __LINE__)

static void data_dump(unsigned char *pu8, int len, const char *func, int line)
{
	int i = 0;
	int sum = 0;

	printf("=====%s %d====,len:%d\n", func, line, len);
	for (i = 0; i < len; i++) {
		//printf("%d:0x%x\n", i, pu8[i]);
		sum += pu8[i];
	}
	printf("sum:%d\n", sum);
}
#else
#define CVI_MSG_VENC_API_IN
#define CVI_MSG_VENC_API_OUT
#define DATA_DUMP(data, len)
#endif

CVI_S32 CVI_VENC_GetDataFifoAddr(VENC_CHN VeChn, CVI_U64 *pu64PhyAddr);

static CVI_S32 MSG_VENC_CreateChn(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CHN_ATTR_S *pstAttr = (VENC_CHN_ATTR_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstAttr, sizeof(VENC_CHN_ATTR_S));

	s32Ret = CVI_VENC_CreateChn(VeChn, pstAttr);
	if (s32Ret != CVI_SUCCESS) {
//		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateChn fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_DestroyChn(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	CVI_MSG_VENC_API_IN;
	s32Ret = CVI_VENC_DestroyChn(VeChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyChn fail, Chn:%d\n", VeChn);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_ResetChn(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_ResetChn(VeChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ResetChn fail, Chn:%d\n", VeChn);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_StartRecvFrame(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_RECV_PIC_PARAM_S *pstRecvParam = (VENC_RECV_PIC_PARAM_S *)pstMsg->pBody;
	CVI_U64 u64PhyAddr = 0;
	CVI_MSG_VENC_API_IN;
	DATA_DUMP(pstRecvParam, sizeof(VENC_RECV_PIC_PARAM_S));
	//DATA_DUMP(VENC_STREAM_PACK_S, sizeof(VENC_STREAM_PACK_S));
	s32Ret = CVI_VENC_GetDataFifoAddr(VeChn, &u64PhyAddr);
	if (s32Ret == CVI_SUCCESS) {
		goto OUT;
	}

	s32Ret = CVI_VENC_StartRecvFrame(VeChn, pstRecvParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "StartRecvFrame fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	s32Ret = CVI_VENC_GetDataFifoAddr(VeChn, &u64PhyAddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetDataFifoAddr fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
	}
OUT:
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	memcpy(respMsg->as32PrivData, &u64PhyAddr, sizeof(u64PhyAddr));

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_StopRecvFrame(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_StopRecvFrame(VeChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "StopRecvFrame fail, Chn:%d\n", VeChn);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_QueryStatus(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CHN_STATUS_S stStatus = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stStatus, sizeof(VENC_CHN_STATUS_S));
	s32Ret = CVI_VENC_QueryStatus(VeChn, &stStatus);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "StopRecvFrame fail, Chn:%d\n", VeChn);
	}

	DATA_DUMP(&stStatus, sizeof(VENC_CHN_STATUS_S));
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stStatus, sizeof(VENC_CHN_STATUS_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetChnAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CHN_ATTR_S *pstAttr = (VENC_CHN_ATTR_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstAttr, sizeof(VENC_CHN_ATTR_S));

	s32Ret = CVI_VENC_SetChnAttr(VeChn, pstAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnAttr fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetChnAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CHN_ATTR_S stChnAttr = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stChnAttr, sizeof(VENC_CHN_ATTR_S));
	s32Ret = CVI_VENC_GetChnAttr(VeChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnAttr fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stChnAttr, sizeof(VENC_CHN_ATTR_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stChnAttr, sizeof(VENC_CHN_ATTR_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetStream(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_STREAM_PACK_S StreamPack = {0};
	VENC_STREAM_S *pstStream = &StreamPack.stStream;
	CVI_S32 s32MilliSec = pstMsg->as32PrivData[0];

	CVI_MSG_VENC_API_IN;
	pstStream->pstPack = StreamPack.pstPack;
	s32Ret = CVI_VENC_GetStream(VeChn, pstStream, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		//CVI_TRACE_MSG(CVI_DBG_ERR, "GetStream fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(pstStream, sizeof(VENC_STREAM_S));
	DATA_DUMP(&StreamPack, sizeof(VENC_STREAM_PACK_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &StreamPack, sizeof(VENC_STREAM_PACK_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_ReleaseStream(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_STREAM_PACK_S *pStreamPack = (VENC_STREAM_PACK_S *)pstMsg->pBody;
	VENC_STREAM_S *pstStream = NULL;

	CVI_MSG_VENC_API_IN;
	pstStream = &pStreamPack->stStream;
	pstStream->pstPack = pStreamPack->pstPack;

	for (int i = 0; i < pstStream->u32PackCount; i++) {
		pstStream->pstPack[i].pu8Addr = (CVI_U8 *)pstStream->pstPack[i].u64PhyAddr;
	}

	s32Ret = CVI_VENC_ReleaseStream(VeChn, pstStream);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ReleaseStream fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SendFrame(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VIDEO_FRAME_INFO_S *pstFrame = (VIDEO_FRAME_INFO_S *)pstMsg->pBody;
	CVI_S32 s32MilliSec = pstMsg->as32PrivData[0];

	CVI_MSG_VENC_API_IN;
    DATA_DUMP(pstFrame, sizeof(VIDEO_FRAME_INFO_S));
	s32Ret = CVI_VENC_SendFrame(VeChn, pstFrame, s32MilliSec);
	if (s32Ret != CVI_SUCCESS && s32Ret != CVI_ERR_VENC_FRC_NO_ENC) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SendFrame fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SendFrame_Ex(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	USER_FRAME_INFO_S *pstUsrFrame = (USER_FRAME_INFO_S *)pstMsg->pBody;
	CVI_S32 s32MilliSec = pstMsg->as32PrivData[0];

	CVI_MSG_VENC_API_IN;
    DATA_DUMP(pstUsrFrame, sizeof(USER_FRAME_INFO_S));
	s32Ret = CVI_VENC_SendFrameEx(VeChn, pstUsrFrame, s32MilliSec);
	if (s32Ret != CVI_SUCCESS && s32Ret != CVI_ERR_VENC_FRC_NO_ENC) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SendFrame fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_RequestIDR(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_BOOL bInstant  = pstMsg->as32PrivData[0];

	CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_RequestIDR(VeChn, bInstant);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RequestIDR fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetRoiAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_ROI_ATTR_S *pstRoiAttr = (VENC_ROI_ATTR_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;
	s32Ret = CVI_VENC_SetRoiAttr(VeChn, pstRoiAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetRoiAttr fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetRoiAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_ROI_ATTR_S stRoiAttr = {0};
	CVI_U32 u32Index = pstMsg->as32PrivData[0];

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stRoiAttr, sizeof(VENC_ROI_ATTR_S));
	s32Ret = CVI_VENC_GetRoiAttr(VeChn, u32Index, &stRoiAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetRoiAttr fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stRoiAttr, sizeof(VENC_ROI_ATTR_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stRoiAttr, sizeof(VENC_ROI_ATTR_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetH264Trans(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H264_TRANS_S *pstH264Trans = (VENC_H264_TRANS_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstH264Trans, sizeof(VENC_H264_TRANS_S));

	s32Ret = CVI_VENC_SetH264Trans(VeChn, pstH264Trans);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetH264Trans fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetH264Trans(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H264_TRANS_S stH264Trans = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stH264Trans, sizeof(VENC_H264_TRANS_S));
	s32Ret = CVI_VENC_GetH264Trans(VeChn, &stH264Trans);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetH264Trans fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stH264Trans, sizeof(VENC_H264_TRANS_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stH264Trans, sizeof(VENC_H264_TRANS_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetH264Entropy(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H264_ENTROPY_S *pstH264EntropyEnc = (VENC_H264_ENTROPY_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstH264EntropyEnc, sizeof(VENC_H264_ENTROPY_S));

	s32Ret = CVI_VENC_SetH264Entropy(VeChn, pstH264EntropyEnc);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetH264Entropy fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetH264Entropy(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H264_ENTROPY_S stH264EntropyEnc = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stH264EntropyEnc, sizeof(VENC_H264_ENTROPY_S));
	s32Ret = CVI_VENC_GetH264Entropy(VeChn, &stH264EntropyEnc);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetH264Entropy fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stH264EntropyEnc, sizeof(VENC_H264_ENTROPY_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stH264EntropyEnc, sizeof(VENC_H264_ENTROPY_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetJpegParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_JPEG_PARAM_S *pstJpegParam = (VENC_JPEG_PARAM_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstJpegParam, sizeof(VENC_JPEG_PARAM_S));

	s32Ret = CVI_VENC_SetJpegParam(VeChn, pstJpegParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetJpegParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetJpegParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_JPEG_PARAM_S stJpegParam = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stJpegParam, sizeof(VENC_JPEG_PARAM_S));
	s32Ret = CVI_VENC_GetJpegParam(VeChn, &stJpegParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetJpegParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stJpegParam, sizeof(VENC_JPEG_PARAM_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stJpegParam, sizeof(VENC_JPEG_PARAM_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetRcParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_RC_PARAM_S *pstRcParam = (VENC_RC_PARAM_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstRcParam, sizeof(VENC_RC_PARAM_S));

	s32Ret = CVI_VENC_SetRcParam(VeChn, pstRcParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetRcParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetRcParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_RC_PARAM_S stRcParam = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stRcParam, sizeof(VENC_RC_PARAM_S));
	s32Ret = CVI_VENC_GetRcParam(VeChn, &stRcParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetRcParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stRcParam, sizeof(VENC_RC_PARAM_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stRcParam, sizeof(VENC_RC_PARAM_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetRefParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_REF_PARAM_S *pstRefParam = (VENC_REF_PARAM_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstRefParam, sizeof(VENC_REF_PARAM_S));

	s32Ret = CVI_VENC_SetRefParam(VeChn, pstRefParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetRefParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetRefParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_REF_PARAM_S stRefParam = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stRefParam, sizeof(VENC_REF_PARAM_S));
	s32Ret = CVI_VENC_GetRefParam(VeChn, &stRefParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetRefParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stRefParam, sizeof(VENC_REF_PARAM_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stRefParam, sizeof(VENC_REF_PARAM_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetChnParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CHN_PARAM_S *pstChnParam = (VENC_CHN_PARAM_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstChnParam, sizeof(VENC_CHN_PARAM_S));

	s32Ret = CVI_VENC_SetChnParam(VeChn, pstChnParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetChnParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CHN_PARAM_S stChnParam = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stChnParam, sizeof(VENC_CHN_PARAM_S));
	s32Ret = CVI_VENC_GetChnParam(VeChn, &stChnParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stChnParam, sizeof(VENC_CHN_PARAM_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stChnParam, sizeof(VENC_CHN_PARAM_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetModParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_PARAM_MOD_S *pstModParam = (VENC_PARAM_MOD_S *)pstMsg->pBody;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	UNUSED(VeChn);

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstModParam, sizeof(VENC_PARAM_MOD_S));

	s32Ret = CVI_VENC_SetModParam(pstModParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetModParam fail, ret:0x%x\n",s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail\n");
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetModParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_PARAM_MOD_S *pstModParam = (VENC_PARAM_MOD_S *)pstMsg->pBody;
	VENC_PARAM_MOD_S stModParam = {0};
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	UNUSED(VeChn);

	CVI_MSG_VENC_API_IN;

	stModParam.enVencModType = pstModParam->enVencModType;
	DATA_DUMP(&stModParam, sizeof(VENC_PARAM_MOD_S));
	s32Ret = CVI_VENC_GetModParam(&stModParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "stModParam fail, ret:0x%x\n", s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stModParam, sizeof(VENC_PARAM_MOD_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stModParam, sizeof(VENC_PARAM_MOD_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail\n");
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetFrameLostStrategy(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_FRAMELOST_S *pstFrmLostParam = (VENC_FRAMELOST_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstFrmLostParam, sizeof(VENC_FRAMELOST_S));

	s32Ret = CVI_VENC_SetFrameLostStrategy(VeChn, pstFrmLostParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetFrameLostStrategy fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetFrameLostStrategy(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_FRAMELOST_S stFrmLostParam = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stFrmLostParam, sizeof(VENC_FRAMELOST_S));
	s32Ret = CVI_VENC_GetFrameLostStrategy(VeChn, &stFrmLostParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetFrameLostStrategy fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stFrmLostParam, sizeof(VENC_FRAMELOST_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stFrmLostParam, sizeof(VENC_FRAMELOST_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_AttachVbPool(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CHN_POOL_S *pstPool = (VENC_CHN_POOL_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstPool, sizeof(VENC_CHN_POOL_S));

	s32Ret = CVI_VENC_AttachVbPool(VeChn, pstPool);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "AttachVbPool fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_DetachVbPool(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_DetachVbPool(VeChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DetachVbPool fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetH264Vui(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	VENC_H264_VUI_S * pstH264Vui = (VENC_H264_VUI_S *)pstMsg->pBody;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstH264Vui, sizeof(VENC_H264_VUI_S));

	s32Ret = CVI_VENC_SetH264Vui(VeChn, pstH264Vui);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetH264Vui fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetH264Vui(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H264_VUI_S stH264Vui = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stH264Vui, sizeof(VENC_H264_VUI_S));
	s32Ret = CVI_VENC_GetH264Vui(VeChn, &stH264Vui);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetH264Vui fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stH264Vui, sizeof(VENC_H264_VUI_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stH264Vui, sizeof(VENC_H264_VUI_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetH265Vui(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	VENC_H265_VUI_S * pstH265Vui = (VENC_H265_VUI_S *)pstMsg->pBody;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstH265Vui, sizeof(VENC_H265_VUI_S));

	s32Ret = CVI_VENC_SetH265Vui(VeChn, pstH265Vui);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetH265Vui fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetH265Vui(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H265_VUI_S stH265Vui = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stH265Vui, sizeof(VENC_H265_VUI_S));
	s32Ret = CVI_VENC_GetH265Vui(VeChn, &stH265Vui);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetH265Vui fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stH265Vui, sizeof(VENC_H265_VUI_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stH265Vui, sizeof(VENC_H265_VUI_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_Suspend(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	//CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_Suspend();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetH265Vui fail, ret:0x%x\n", s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail\n");
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	//CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_Resume(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	//CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_Resume();

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail\n");
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	//CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetDataFifoLen(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 *pu32Len = (CVI_U32 *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_SetDataFifoLen(VeChn, *pu32Len);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetDataFifo fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetDataFifoLen(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 u32Len = 0;

	CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_GetDataFifoLen(VeChn, &u32Len);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetDataFifo fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &u32Len, sizeof(u32Len));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}
static CVI_S32 MSG_VENC_EnableSVC(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_BOOL bInstant  = pstMsg->as32PrivData[0];

	CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_EnableSvc(VeChn, bInstant);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "EnableSVC fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetSvcParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_SVC_PARAM_S stSvcParam = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stSvcParam, sizeof(VENC_SVC_PARAM_S));
	s32Ret = CVI_VENC_GetSvcParam(VeChn, &stSvcParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SvcParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stSvcParam, sizeof(VENC_SVC_PARAM_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stSvcParam, sizeof(VENC_SVC_PARAM_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetSvcParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	VENC_SVC_PARAM_S *pstSvcParam = (VENC_SVC_PARAM_S *)pstMsg->pBody;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstSvcParam, sizeof(VENC_SVC_PARAM_S));

	s32Ret = CVI_VENC_SetSvcParam(VeChn, pstSvcParam);
        if (s32Ret != CVI_SUCCESS) {
                CVI_TRACE_MSG(CVI_DBG_ERR, "SetSvcParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
	}
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
			return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
			CVI_IPCMSG_DestroyMessage(respMsg);
			return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;

}

static CVI_S32 MSG_VENC_SetUserData(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U8 *pu8Data = (CVI_U8 *)pstMsg->pBody;
	CVI_U32 u32Len = pstMsg->u32BodyLen;

	CVI_MSG_VENC_API_IN;

	s32Ret = CVI_VENC_InsertUserData(VeChn, pu8Data, u32Len);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Insert User Data fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetMjpegParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_MJPEG_PARAM_S *pstMjpegParam = (VENC_MJPEG_PARAM_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstMjpegParam, sizeof(VENC_MJPEG_PARAM_S));

	s32Ret = CVI_VENC_SetMjpegParam(VeChn, pstMjpegParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetMjpegParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetMjpegParam(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_MJPEG_PARAM_S stMjpegParam = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stMjpegParam, sizeof(VENC_MJPEG_PARAM_S));
	s32Ret = CVI_VENC_GetMjpegParam(VeChn, &stMjpegParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetMjpegParam fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stMjpegParam, sizeof(VENC_MJPEG_PARAM_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stMjpegParam, sizeof(VENC_MJPEG_PARAM_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetH264Deblk(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H264_DBLK_S *pstH264Dblk = (VENC_H264_DBLK_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstH264Dblk, sizeof(VENC_H264_DBLK_S));

	s32Ret = CVI_VENC_SetH264Dblk(VeChn, pstH264Dblk);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetH264Dblk fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetH264Deblk(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H264_DBLK_S stH264Dblk = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stH264Dblk, sizeof(VENC_H264_DBLK_S));
	s32Ret = CVI_VENC_GetH264Dblk(VeChn, &stH264Dblk);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetH264Dblk fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stH264Dblk, sizeof(VENC_H264_DBLK_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stH264Dblk, sizeof(VENC_H264_DBLK_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_SetH265Deblk(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H265_DBLK_S *pstH265Dblk = (VENC_H265_DBLK_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstH265Dblk, sizeof(VENC_H265_DBLK_S));

	s32Ret = CVI_VENC_SetH265Dblk(VeChn, pstH265Dblk);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetH265Dblk fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetH265Deblk(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H265_DBLK_S stH265Dblk = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stH265Dblk, sizeof(VENC_H265_DBLK_S));
	s32Ret = CVI_VENC_GetH265Dblk(VeChn, &stH265Dblk);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetH265Dblk fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stH265Dblk, sizeof(VENC_H265_DBLK_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stH265Dblk, sizeof(VENC_H265_DBLK_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}
static CVI_S32 MSG_VENC_SetH265Trans(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H265_TRANS_S *pstH265Trans = (VENC_H265_TRANS_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstH265Trans, sizeof(VENC_H265_TRANS_S));

	s32Ret = CVI_VENC_SetH265Trans(VeChn, pstH265Trans);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetH265Trans fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetH265Trans(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_H265_TRANS_S stH265Trans = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stH265Trans, sizeof(VENC_H265_TRANS_S));
	s32Ret = CVI_VENC_GetH265Trans(VeChn, &stH265Trans);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetH265Trans fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stH265Trans, sizeof(VENC_H265_TRANS_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stH265Trans, sizeof(VENC_H265_TRANS_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}
static CVI_S32 MSG_VENC_SetSuperFrameStrategy(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_SUPERFRAME_CFG_S *pstSuperFrmParam = (VENC_SUPERFRAME_CFG_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstSuperFrmParam, sizeof(VENC_SUPERFRAME_CFG_S));

	s32Ret = CVI_VENC_SetSuperFrameStrategy(VeChn, pstSuperFrmParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetSuperFrameStrategy fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetSuperFrameStrategy(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_SUPERFRAME_CFG_S stSuperFrmParam = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stSuperFrmParam, sizeof(VENC_SUPERFRAME_CFG_S));
	s32Ret = CVI_VENC_GetSuperFrameStrategy(VeChn, &stSuperFrmParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetSuperFrameStrategy fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stSuperFrmParam, sizeof(VENC_SUPERFRAME_CFG_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stSuperFrmParam, sizeof(VENC_SUPERFRAME_CFG_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}
static CVI_S32 MSG_VENC_SetCuPediction(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CU_PREDICTION_S  *pstCuPrediction = (VENC_CU_PREDICTION_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstCuPrediction, sizeof(VENC_CU_PREDICTION_S));

	s32Ret = CVI_VENC_SetCuPrediction(VeChn, pstCuPrediction);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetCuPrediction fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetCuPediction(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_CU_PREDICTION_S stCuPrediction = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stCuPrediction, sizeof(VENC_CU_PREDICTION_S));
	s32Ret = CVI_VENC_GetCuPrediction(VeChn, &stCuPrediction);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetCuPrediction fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stCuPrediction, sizeof(VENC_CU_PREDICTION_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stCuPrediction, sizeof(VENC_CU_PREDICTION_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}
static CVI_S32 MSG_VENC_SetDebreathEffect(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_DEBREATHEFFECT_S *pstDebreathEffect = (VENC_DEBREATHEFFECT_S *)pstMsg->pBody;

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(pstDebreathEffect, sizeof(VENC_DEBREATHEFFECT_S));

	s32Ret = CVI_VENC_SetDebreathEffect(VeChn, pstDebreathEffect);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetDebreathEffect fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 MSG_VENC_GetDebreathEffect(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	VENC_CHN VeChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VENC_DEBREATHEFFECT_S stDebreathEffect = {0};

	CVI_MSG_VENC_API_IN;

	DATA_DUMP(&stDebreathEffect, sizeof(VENC_DEBREATHEFFECT_S));
	s32Ret = CVI_VENC_GetDebreathEffect(VeChn, &stDebreathEffect);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetDebreathEffect fail, Chn:%d ret:0x%x\n", VeChn, s32Ret);
		// don't return, send s32Ret to client
	}
	DATA_DUMP(&stDebreathEffect, sizeof(VENC_DEBREATHEFFECT_S));

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stDebreathEffect, sizeof(VENC_DEBREATHEFFECT_S));
	if (!respMsg) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateRespMessage fail, Chn:%d\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "IPCMSG_SendOnly fail, Chn:%d\n", VeChn);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return CVI_FAILURE;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	CVI_MSG_VENC_API_OUT;

	return CVI_SUCCESS;
}
static MSG_MODULE_CMD_S g_stVencCmdTable[] = {
	{ MSG_CMD_VENC_CREATE_CHN,        MSG_VENC_CreateChn      },
	{ MSG_CMD_VENC_DESTROY_CHN,       MSG_VENC_DestroyChn     },
	{ MSG_CMD_VENC_RESET_CHN,         MSG_VENC_ResetChn       },
	{ MSG_CMD_VENC_START_RECV_FRAME,  MSG_VENC_StartRecvFrame },
	{ MSG_CMD_VENC_STOP_RECV_FRAME,   MSG_VENC_StopRecvFrame  },
	{ MSG_CMD_VENC_QUERY_STATUS,      MSG_VENC_QueryStatus    },
	{ MSG_CMD_VENC_SET_CHN_ATTR,      MSG_VENC_SetChnAttr     },
	{ MSG_CMD_VENC_GET_CHN_ATTR,      MSG_VENC_GetChnAttr     },
	{ MSG_CMD_VENC_GET_STREAM,        MSG_VENC_GetStream      },
	{ MSG_CMD_VENC_RELEASE_STREAM,    MSG_VENC_ReleaseStream  },
	{ MSG_CMD_VENC_SEND_FRAME,        MSG_VENC_SendFrame      },
	{ MSG_CMD_VENC_REQUEST_IDR,       MSG_VENC_RequestIDR     },
	{ MSG_CMD_VENC_SET_ROI_ATTR,      MSG_VENC_SetRoiAttr     },
	{ MSG_CMD_VENC_GET_ROI_ATTR,      MSG_VENC_GetRoiAttr     },
	{ MSG_CMD_VENC_SET_H264_TRANS,    MSG_VENC_SetH264Trans   },
	{ MSG_CMD_VENC_GET_H264_TRANS,    MSG_VENC_GetH264Trans   },
	{ MSG_CMD_VENC_SET_H264_ENTROPY,  MSG_VENC_SetH264Entropy },
	{ MSG_CMD_VENC_GET_H264_ENTROPY,  MSG_VENC_GetH264Entropy },
	{ MSG_CMD_VENC_SET_JPEG_PARAM,    MSG_VENC_SetJpegParam   },
	{ MSG_CMD_VENC_GET_JPEG_PARAM,    MSG_VENC_GetJpegParam   },
	{ MSG_CMD_VENC_SET_RC_PARAM,      MSG_VENC_SetRcParam     },
	{ MSG_CMD_VENC_GET_RC_PARAM,      MSG_VENC_GetRcParam     },
	{ MSG_CMD_VENC_SET_REF_PARAM,     MSG_VENC_SetRefParam    },
	{ MSG_CMD_VENC_GET_REF_PARAM,     MSG_VENC_GetRefParam    },
	{ MSG_CMD_VENC_SET_CHN_PARAM,     MSG_VENC_SetChnParam    },
	{ MSG_CMD_VENC_GET_CHN_PARAM,     MSG_VENC_GetChnParam    },
	{ MSG_CMD_VENC_SET_MOD_PARAM,     MSG_VENC_SetModParam    },
	{ MSG_CMD_VENC_GET_MOD_PARAM,     MSG_VENC_GetModParam    },
	{ MSG_CMD_VENC_SET_FRAME_LOST,    MSG_VENC_SetFrameLostStrategy},
	{ MSG_CMD_VENC_GET_FRAME_LOST,    MSG_VENC_GetFrameLostStrategy},
	{ MSG_CMD_VENC_ATTACH_VBPOOL,	  MSG_VENC_AttachVbPool   },
	{ MSG_CMD_VENC_DETACH_VBPOOL,	  MSG_VENC_DetachVbPool   },
	{ MSG_CMD_VENC_SET_H264VUI,	      MSG_VENC_SetH264Vui     },
	{ MSG_CMD_VENC_GET_H264VUI,	      MSG_VENC_GetH264Vui     },
	{ MSG_CMD_VENC_SET_H265VUI,	      MSG_VENC_SetH265Vui     },
	{ MSG_CMD_VENC_GET_H265VUI,	      MSG_VENC_GetH265Vui     },
	{ MSG_CMD_VENC_SUSPEND,           MSG_VENC_Suspend        },
	{ MSG_CMD_VENC_RESUME,            MSG_VENC_Resume         },
	{ MSG_CMD_VENC_SET_DATA_FIFO_LEN, MSG_VENC_SetDataFifoLen },
	{ MSG_CMD_VENC_GET_DATA_FIFO_LEN, MSG_VENC_GetDataFifoLen },
	{ MSG_CMD_VENC_SET_USER_DATA,     MSG_VENC_SetUserData },
	{ MSG_CMD_VENC_ENABLE_SVC,        MSG_VENC_EnableSVC },
	{ MSG_CMD_VENC_GET_SVC_PARAM,     MSG_VENC_GetSvcParam  },
	{ MSG_CMD_VENC_SET_SVC_PARAM,     MSG_VENC_SetSvcParam  },
	{ MSG_CMD_VENC_SEND_FRAME_EX,     MSG_VENC_SendFrame_Ex },
	{ MSG_CMD_VENC_SET_MJPEG_PARAM,    MSG_VENC_SetMjpegParam},
	{ MSG_CMD_VENC_GET_MJPEG_PARAM,    MSG_VENC_GetMjpegParam   },
	{ MSG_CMD_VENC_SET_H264_DBLK,     MSG_VENC_SetH264Deblk },
	{ MSG_CMD_VENC_GET_H264_DBLK,     MSG_VENC_GetH264Deblk },
	{ MSG_CMD_VENC_SET_H265_DBLK,     MSG_VENC_SetH265Deblk },
	{ MSG_CMD_VENC_GET_H265_DBLK,     MSG_VENC_GetH265Deblk },
	{ MSG_CMD_VENC_SET_H265_TRANS,    MSG_VENC_SetH265Trans },
	{ MSG_CMD_VENC_GET_H265_TRANS,    MSG_VENC_GetH265Trans },
	{ MSG_CMD_VENC_SET_CUPREDICTION,    MSG_VENC_SetCuPediction},
	{ MSG_CMD_VENC_GET_CUPREDICTION,    MSG_VENC_GetCuPediction},
	{ MSG_CMD_VENC_SET_SUPERFRAME_STRATEGY,    MSG_VENC_SetSuperFrameStrategy},
	{ MSG_CMD_VENC_GET_SUPERFRAME_STRATEGY,    MSG_VENC_GetSuperFrameStrategy},
	{ MSG_CMD_VENC_SET_DEBREATH_EFFECT,    MSG_VENC_SetDebreathEffect},
	{ MSG_CMD_VENC_GET_DEBREATH_EFFECT,    MSG_VENC_GetDebreathEffect},
};

MSG_SERVER_MODULE_S g_stModuleVenc = {
	CVI_ID_VENC,
	"venc",
	sizeof(g_stVencCmdTable) / sizeof(MSG_MODULE_CMD_S),
	&g_stVencCmdTable[0]
};

MSG_SERVER_MODULE_S *MSG_GetVencMod(CVI_VOID)
{
	return &g_stModuleVenc;
}

