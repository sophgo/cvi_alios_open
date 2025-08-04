#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cvi_type.h"
#include "cvi_vpss.h"
#include "cvi_ipcmsg.h"
#include "msg_vpss.h"
#include "msg_ctx.h"
#include "cvi_debug.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

extern CVI_S32 platform_vpss_setchnldcmeshattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VPSS_LDC_ATTR_S *pstLDCAttr, CVI_U64 Paddr);

static CVI_S32 MSG_VPSS_SetMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_VPSS_SetMode((VPSS_MODE_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetMode Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetMode call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetMode call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_MODE_S stVpssMode;

	s32Ret = CVI_VPSS_GetMode(&stVpssMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetMode Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVpssMode, sizeof(VPSS_MODE_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetMode call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetMode call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_CreateGrp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, (VPSS_GRP_ATTR_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_CreateGrp Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_DestroyGrp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_DestroyGrp Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetAvailableGrp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_VPSS_GetAvailableGrp();

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetAvailableGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetAvailableGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_StartGrp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_StartGrp Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "StartGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "StartGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_StopGrp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_StopGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_StopGrp Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "StopGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "StopGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_ResetGrp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_ResetGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_ResetGrp Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ResetGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ResetGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetGrpAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetGrpAttr(VpssGrp, &stGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetGrpAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stGrpAttr, sizeof(VPSS_GRP_ATTR_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetGrpAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetGrpAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetGrpAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, (VPSS_GRP_ATTR_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetGrpAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetGrpAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetGrpAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetGrpCrop(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_CROP_INFO_S stCropInfo;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetGrpCrop(VpssGrp, &stCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetGrpAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stCropInfo, sizeof(VPSS_CROP_INFO_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetGrpAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetGrpAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetGrpCrop(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetGrpCrop(VpssGrp, (VPSS_CROP_INFO_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetGrpCrop Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetGrpCrop call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetGrpCrop call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SendFrame(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	CVI_S32 s32MilliSec = pstMsg->as32PrivData[0];

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, (VIDEO_FRAME_INFO_S *)pstMsg->pBody, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SendFrame Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_SendFrame call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_SendFrame call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetGrpProcAmpCtrl(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	PROC_AMP_CTRL_S stCtrl;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	PROC_AMP_E stType = pstMsg->as32PrivData[0];

	s32Ret = CVI_VPSS_GetGrpProcAmpCtrl(VpssGrp, stType, &stCtrl);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetGrpProcAmpCtrl Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stCtrl, sizeof(PROC_AMP_CTRL_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetGrpProcAmpCtrl call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetGrpProcAmpCtrl call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetGrpProcAmp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 stValue;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	PROC_AMP_E stType = pstMsg->as32PrivData[0];

	s32Ret = CVI_VPSS_GetGrpProcAmp(VpssGrp, stType, &stValue);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetGrpProcAmp Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stValue, sizeof(CVI_S32));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetGrpProcAmp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetGrpProcAmp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetGrpProcAmp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	PROC_AMP_E stType = pstMsg->as32PrivData[0];
	CVI_S32 stValue = pstMsg->as32PrivData[1];

	s32Ret = CVI_VPSS_SetGrpProcAmp(VpssGrp, stType, stValue);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetGrpProcAmp Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetGrpProcAmp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetGrpProcAmp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetAllGrpProcAmp(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_ALL_PROC_AMP_S stAllProcAmp;

	s32Ret = CVI_VPSS_GetAllProcAmp(&stAllProcAmp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetAllProcAmp Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stAllProcAmp, sizeof(stAllProcAmp));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetAllProcAmp call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetAllProcAmp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetGrpPQBin(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetGrpParamfromBin(VpssGrp, (VPSS_BIN_DATA *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetGrpParamfromBin Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_SetGrpPQBin call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_SetGrpPQBin call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetBinScene(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8 scene;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetBinScene(VpssGrp, &scene);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetBinScene Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &scene, sizeof(CVI_U8));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetBinScene call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetBinScene call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, (VPSS_CHN_ATTR_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_CHN_ATTR_S stChnAttr;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_Get Grp:%d Chn:%d ChnAttr Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stChnAttr, sizeof(VPSS_CHN_ATTR_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_EnableChn(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_EnableChn:%d %d Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_EnableChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_EnableChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_DisableChn(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_DisableChn:%d %d Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_DisableChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_DisableChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnCrop(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, (VPSS_CROP_INFO_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnCrop Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnCrop call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnCrop call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnCrop(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_CROP_INFO_S stChnCrop;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnCrop(VpssGrp, VpssChn, &stChnCrop);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_Get Grp:%d Chn:%d ChnCrop Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stChnCrop, sizeof(VPSS_CROP_INFO_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnCrop call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnCrop call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnRotation(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);
	ROTATION_E enRotation = pstMsg->as32PrivData[0];

	s32Ret = CVI_VPSS_SetChnRotation(VpssGrp, VpssChn, enRotation);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnRotation Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnRotation call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnRotation call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnRotation(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);
	ROTATION_E enRotation;

	s32Ret = CVI_VPSS_GetChnRotation(VpssGrp, VpssChn, &enRotation);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetChnRotation Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &enRotation, sizeof(ROTATION_E));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnRotation call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnRotation call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnLDCAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_U64 Paddr;

	Paddr = ((CVI_U64)pstMsg->as32PrivData[2] << 56) | ((CVI_U64)pstMsg->as32PrivData[1] << 28) |
		(CVI_U32)pstMsg->as32PrivData[0];

	s32Ret = platform_vpss_setchnldcmeshattr(VpssGrp, VpssChn, (VPSS_LDC_ATTR_S *)pstMsg->pBody, Paddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnLDCAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnLDCAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnLDCAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnLDCAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_LDC_ATTR_S stVPSSLDCAttrOut;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnLDCAttr(VpssGrp, VpssChn, &stVPSSLDCAttrOut);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_Get Grp:%d Chn:%d ChnLDCAttr Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVPSSLDCAttrOut, sizeof(VPSS_LDC_ATTR_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnLDCAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnLDCAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SendChnFrame(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_S32 s32MilliSec = pstMsg->as32PrivData[0];

	s32Ret = CVI_VPSS_SendChnFrame(VpssGrp, VpssChn, (VIDEO_FRAME_INFO_S *)pstMsg->pBody, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SendChnFrame Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_SendChnFrame call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_SendChnFrame call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}


static CVI_S32 MSG_VPSS_GetChnFrame(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_S32 s32MilliSec = pstMsg->as32PrivData[0];
	VIDEO_FRAME_INFO_S stVideoFrame = {0};

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVideoFrame, sizeof(stVideoFrame));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnFrame call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnFrame call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_ReleaseChnFrame(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, (VIDEO_FRAME_INFO_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_ReleaseChnFrame Grp:%d Chn:%d Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_ReleaseChnFrame call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VPSS_ReleaseChnFrame call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_AttachVbPool(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	VB_POOL hVbPool = pstMsg->as32PrivData[0];

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, hVbPool);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_AttachVbPool Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "AttachVbPool call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "AttachVbPool call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_DetachVbPool(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_DetachVbPool(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_DetachVbPool Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DetachVbPool call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DetachVbPool call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnAlign(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	CVI_U32 align = pstMsg->as32PrivData[0];

	s32Ret = CVI_VPSS_SetChnAlign(VpssGrp, VpssChn, align);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnAlign Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VpssSetChnAlign call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "VpssSetChnAlign call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnAlign(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 stAlign;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnAlign(VpssGrp, VpssChn, &stAlign);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetChnAlign Grp:%d Chn:%d Align Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stAlign, sizeof(CVI_U32));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnAlign call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnAlign call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnYRatio(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);
	CVI_FLOAT *stYRatio = (CVI_FLOAT *)pstMsg->pBody;

	s32Ret = CVI_VPSS_SetChnYRatio(VpssGrp, VpssChn, *stYRatio);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnYRatio Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnYRatio call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnYRatio call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnYRatio(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_FLOAT stYRatio;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnYRatio(VpssGrp, VpssChn, &stYRatio);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetChnYRatio Grp:%d Chn:%d Align Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stYRatio, sizeof(CVI_FLOAT));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnYRatio call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnYRatio call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnScaleCoefLevel(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);
	VPSS_SCALE_COEF_E *stCoef = (VPSS_SCALE_COEF_E *)pstMsg->pBody;

	s32Ret = CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, *stCoef);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnScaleCoefLevel Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnScaleCoefLevel call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnScaleCoefLevel call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnScaleCoefLevel(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_SCALE_COEF_E stCoef;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnScaleCoefLevel(VpssGrp, VpssChn, &stCoef);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetChnScaleCoefLevel Grp:%d Chn:%d Align Failed : %#x!\n",
			VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stCoef, sizeof(VPSS_SCALE_COEF_E));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnScaleCoefLevel call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnScaleCoefLevel call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnDrawRect(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetChnDrawRect(VpssGrp, VpssChn, (VPSS_DRAW_RECT_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnDrawRect Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnDrawRect call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnDrawRect call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnDrawRect(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_DRAW_RECT_S stDrawRect;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnDrawRect(VpssGrp, VpssChn, &stDrawRect);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_Get Grp:%d Chn:%d ChnDrawRect Failed : %#x!\n",
			VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stDrawRect, sizeof(VPSS_DRAW_RECT_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnDrawRect call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnDrawRect call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnConvert(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetChnConvert(VpssGrp, VpssChn, (VPSS_CONVERT_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnConvert Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnConvert call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnConvert call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnConvert(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_CONVERT_S stConvert;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnConvert(VpssGrp, VpssChn, &stConvert);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_Get Grp:%d Chn:%d ChnConvert Failed : %#x!\n",
			VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stConvert, sizeof(VPSS_DRAW_RECT_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnConvert call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnConvert call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetChnBufWrapAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn, (VPSS_CHN_BUF_WRAP_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnBufWrapAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnBufWrapAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "SetChnBufWrapAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetChnBufWrapAttr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetChnBufWrapAttr(VpssGrp, VpssChn, &stVpssChnBufWrap);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_Get Grp:%d Chn:%d BufWrapAttr Failed : %#x!\n", VpssGrp, VpssChn, s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVpssChnBufWrap, sizeof(VPSS_CHN_BUF_WRAP_S));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnBufWrapAttr call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "GetChnBufWrapAttr call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_ShowChn(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_ShowChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_ShowChn Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ShowChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ShowChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_HideChn(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);
	VPSS_CHN VpssChn = GET_CHN_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_HideChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_HideChn Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_CreateStitch(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_CreateStitch(VpssGrp, (CVI_STITCH_ATTR_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_CreateStitch Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_DestroyStitch(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_DestroyStitch(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_DestroyStitch Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_SetStitch(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_SetStitchAttr(VpssGrp, (CVI_STITCH_ATTR_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetStitchAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_GetStitch(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_STITCH_ATTR_S stStitchAttr;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_GetStitchAttr(VpssGrp, &stStitchAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetStitchAttr Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stStitchAttr, sizeof(stStitchAttr));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_StartStitch(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_StartStitch(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_StartStitch Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VPSS_StopStitch(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VPSS_GRP VpssGrp = GET_DEV_ID(pstMsg->u32Module);

	s32Ret = CVI_VPSS_StopStitch(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_StopStitch Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "HideChn call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stVpssCmdTable[] = {
	{ MSG_CMD_VPSS_CREATE,			MSG_VPSS_CreateGrp },
	{ MSG_CMD_VPSS_DESTROY,			MSG_VPSS_DestroyGrp },
	{ MSG_CMD_VPSS_GET_AVAILABLE_GRP,   MSG_VPSS_GetAvailableGrp },
	{ MSG_CMD_VPSS_START,			MSG_VPSS_StartGrp },
	{ MSG_CMD_VPSS_STOP,				MSG_VPSS_StopGrp },
	{ MSG_CMD_VPSS_RESET,			MSG_VPSS_ResetGrp },
	{ MSG_CMD_VPSS_ENABLE,			MSG_VPSS_EnableChn },
	{ MSG_CMD_VPSS_DISABLE,			MSG_VPSS_DisableChn },
	{ MSG_CMD_VPSS_GET_GRP_ATTR,		MSG_VPSS_GetGrpAttr },
	{ MSG_CMD_VPSS_SET_GRP_ATTR,		MSG_VPSS_SetGrpAttr },
	{ MSG_CMD_VPSS_SET_CHN_ATTR,		MSG_VPSS_SetChnAttr },
	{ MSG_CMD_VPSS_GET_CHN_ATTR,		MSG_VPSS_GetChnAttr },
	{ MSG_CMD_VPSS_SET_GRP_CROP,		MSG_VPSS_SetGrpCrop },
	{ MSG_CMD_VPSS_GET_GRP_CROP,		MSG_VPSS_GetGrpCrop },
	{ MSG_CMD_VPSS_SEND_FRAME,		MSG_VPSS_SendFrame },
	{ MSG_CMD_VPSS_SEND_CHN_FRAME,	MSG_VPSS_SendChnFrame },
	{ MSG_CMD_VPSS_GET_CHN_FRAME,	MSG_VPSS_GetChnFrame },
	{ MSG_CMD_VPSS_RELEASE_CHN_FRAME,   MSG_VPSS_ReleaseChnFrame },
	{ MSG_CMD_VPSS_SET_CHN_ROTATION,	MSG_VPSS_SetChnRotation },
	{ MSG_CMD_VPSS_GET_CHN_ROTATION,	MSG_VPSS_GetChnRotation },
	{ MSG_CMD_VPSS_SET_CHN_LDCATTR,	MSG_VPSS_SetChnLDCAttr },
	{ MSG_CMD_VPSS_GET_CHN_LDCATTR,	MSG_VPSS_GetChnLDCAttr },
	{ MSG_CMD_VPSS_SET_CHN_CROP,		MSG_VPSS_SetChnCrop },
	{ MSG_CMD_VPSS_GET_CHN_CROP,		MSG_VPSS_GetChnCrop },
	{ MSG_CMD_VPSS_GET_GRP_PROCAMPCTRL, MSG_VPSS_GetGrpProcAmpCtrl },
	{ MSG_CMD_VPSS_GET_GRP_PROCAMP,	MSG_VPSS_GetGrpProcAmp },
	{ MSG_CMD_VPSS_SET_GRP_PROCAMP,	MSG_VPSS_SetGrpProcAmp },
	{ MSG_CMD_VPSS_GET_ALL_GRP_PROCAMP,	MSG_VPSS_GetAllGrpProcAmp },
	{ MSG_CMD_VPSS_SET_GRP_PQBIN,	MSG_VPSS_SetGrpPQBin },
	{ MSG_CMD_VPSS_GET_GRP_SCENE,	MSG_VPSS_GetBinScene },
	{ MSG_CMD_VPSS_ATTACH_VBPOOL,	MSG_VPSS_AttachVbPool },
	{ MSG_CMD_VPSS_DETACH_VBPOOL,	MSG_VPSS_DetachVbPool },
	{ MSG_CMD_VPSS_SET_CHN_ALIGN,	MSG_VPSS_SetChnAlign },
	{ MSG_CMD_VPSS_GET_CHN_ALIGN,	MSG_VPSS_GetChnAlign },
	{ MSG_CMD_VPSS_SET_CHN_YRATIO,	MSG_VPSS_SetChnYRatio },
	{ MSG_CMD_VPSS_GET_CHN_YRATIO,	MSG_VPSS_GetChnYRatio },
	{ MSG_CMD_VPSS_SET_CHN_SCALECOEF,   MSG_VPSS_SetChnScaleCoefLevel },
	{ MSG_CMD_VPSS_GET_CHN_SCALECOEF,   MSG_VPSS_GetChnScaleCoefLevel },
	{ MSG_CMD_VPSS_SHOW_CHN,			MSG_VPSS_ShowChn },
	{ MSG_CMD_VPSS_HIDE_CHN,			MSG_VPSS_HideChn },
	{ MSG_CMD_VPSS_SET_CHN_BUFWRAPATTR, MSG_VPSS_SetChnBufWrapAttr },
	{ MSG_CMD_VPSS_GET_CHN_BUFWRAPATTR, MSG_VPSS_GetChnBufWrapAttr },
	{ MSG_CMD_VPSS_SET_MODE, MSG_VPSS_SetMode },
	{ MSG_CMD_VPSS_GET_MODE, MSG_VPSS_GetMode },
	{ MSG_CMD_VPSS_SET_CHN_DRAW_RECT, MSG_VPSS_SetChnDrawRect },
	{ MSG_CMD_VPSS_GET_CHN_DRAW_RECT, MSG_VPSS_GetChnDrawRect },
	{ MSG_CMD_VPSS_SET_CHN_CONVERT, MSG_VPSS_SetChnConvert },
	{ MSG_CMD_VPSS_GET_CHN_CONVERT, MSG_VPSS_GetChnConvert },
	{ MSG_CMD_VPSS_CREATE_STITCH, MSG_VPSS_CreateStitch },
	{ MSG_CMD_VPSS_DESTROY_STITCH, MSG_VPSS_DestroyStitch },
	{ MSG_CMD_VPSS_SET_STITCH, MSG_VPSS_SetStitch },
	{ MSG_CMD_VPSS_GET_STITCH, MSG_VPSS_GetStitch },
	{ MSG_CMD_VPSS_START_STITCH, MSG_VPSS_StartStitch },
	{ MSG_CMD_VPSS_STOP_STITCH, MSG_VPSS_StopStitch },
};

MSG_SERVER_MODULE_S g_stModuleVpss = {
	CVI_ID_VPSS,
	"vpss",
	sizeof(g_stVpssCmdTable) / sizeof(MSG_MODULE_CMD_S),
	&g_stVpssCmdTable[0]
};

MSG_SERVER_MODULE_S *MSG_GetVpssMod(CVI_VOID)
{
	return &g_stModuleVpss;
}
