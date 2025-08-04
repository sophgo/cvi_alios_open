#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cvi_type.h"
#include "cvi_sys.h"
#include "cvi_ipcmsg.h"
#include "msg_sys.h"
#include "msg_ctx.h"
#include "cvi_debug.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

//extern CVI_BOOL bFbOnSc;

static CVI_S32 MSG_SYS_Init(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	UNUSED(siId);
	UNUSED(pstMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_SYS_Exit(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	UNUSED(siId);
	UNUSED(pstMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_SYS_Bind(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	MMF_CHN_S *pstChn = pstMsg->pBody;

	s32Ret = CVI_SYS_Bind(&pstChn[0], &pstChn[1]);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_Bind Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_SYS_UnBind(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	MMF_CHN_S *pstChn = pstMsg->pBody;

	s32Ret = CVI_SYS_UnBind(&pstChn[0], &pstChn[1]);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_UnBind Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_SYS_GetBindbyDest(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	MMF_CHN_S stSrcChn;

	s32Ret = CVI_SYS_GetBindbyDest((MMF_CHN_S *)pstMsg->pBody, &stSrcChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_GetBindbyDest Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stSrcChn, sizeof(stSrcChn));
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

static CVI_S32 MSG_SYS_GetBindbySrc(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	MMF_BIND_DEST_S stBindDest;

	s32Ret = CVI_SYS_GetBindbySrc((MMF_CHN_S *)pstMsg->pBody, &stBindDest);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_GetBindbySrc Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stBindDest.astMmfChn,
				sizeof(MMF_CHN_S) * stBindDest.u32Num);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}
	respMsg->as32PrivData[0] = stBindDest.u32Num;

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SYS_GetVersion(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	MMF_VERSION_S stVersion;

	s32Ret = CVI_SYS_GetVersion(&stVersion);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_GetVersion Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVersion, sizeof(stVersion));
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

static CVI_S32 MSG_SYS_GetChipId(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 u32ChipId;

	s32Ret = CVI_SYS_GetChipId(&u32ChipId);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_GetChipId Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &u32ChipId, sizeof(u32ChipId));
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

static CVI_S32 MSG_SYS_GetPowerOnReason(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 u32PowerOnReason;

	s32Ret = CVI_SYS_GetPowerOnReason(&u32PowerOnReason);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_GetPowerOnReason Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &u32PowerOnReason,
				sizeof(u32PowerOnReason));
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

static CVI_S32 MSG_SYS_GetChipVersion(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 u32ChipVersion;

	s32Ret = CVI_SYS_GetChipVersion(&u32ChipVersion);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_GetChipVersion Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &u32ChipVersion,
				sizeof(u32ChipVersion));
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

static CVI_S32 MSG_SYS_SetVIVPSSMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_SYS_SetVIVPSSMode((VI_VPSS_MODE_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_SetVIVPSSMode Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_SYS_GetVIVPSSMode(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VI_VPSS_MODE_S stVIVPSSMode;

	s32Ret = CVI_SYS_GetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_GetVIVPSSMode Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVIVPSSMode, sizeof(stVIVPSSMode));
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

static CVI_S32 MSG_LOG_SetLevelConf(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_LOG_SetLevelConf((LOG_LEVEL_CONF_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_LOG_SetLevelConf Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_LOG_GetLevelConf(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	LOG_LEVEL_CONF_S *pstConf = (LOG_LEVEL_CONF_S *)pstMsg->pBody;

	s32Ret = CVI_LOG_GetLevelConf(pstConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_LOG_GetLevelConf Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, pstConf, sizeof(LOG_LEVEL_CONF_S));
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

static CVI_S32 MSG_SYS_Alios_Init(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_Alios_Init Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Alios_Init call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Alios_Init call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SYS_Alios_Exit(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_Alios_Exit Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Alios_Exit call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Alios_Exit call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

CVI_S32 MSG_SYS_GetMemoryState(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
#if 0
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	ION_MEM_STATE_S state;

	s32Ret = CVI_SYS_GetMemoryState(&state);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_SYS_GetVPSSModeEx Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &state, sizeof(state));
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
#endif
	return CVI_SUCCESS;
}

static CVI_S32 MSG_SYS_SetFbOnSc(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	//bFbOnSc = true;
	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Alios_Exit call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Alios_Exit call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static MSG_MODULE_CMD_S g_stSysCmdTable[] = {
	{ MSG_CMD_SYS_INIT,               MSG_SYS_Init },
	{ MSG_CMD_SYS_EXIT,               MSG_SYS_Exit },
	{ MSG_CMD_SYS_BIND,               MSG_SYS_Bind },
	{ MSG_CMD_SYS_UNBIND,             MSG_SYS_UnBind },
	{ MSG_CMD_SYS_GET_BIND_BY_DEST,   MSG_SYS_GetBindbyDest },
	{ MSG_CMD_SYS_GET_BIND_BY_SRC,    MSG_SYS_GetBindbySrc },
	{ MSG_CMD_SYS_GET_VERSION,        MSG_SYS_GetVersion },
	{ MSG_CMD_SYS_GET_CHIP_ID,        MSG_SYS_GetChipId },
	{ MSG_CMD_SYS_GET_POWER_REASON,   MSG_SYS_GetPowerOnReason },
	{ MSG_CMD_SYS_GET_CHIP_VERSION,   MSG_SYS_GetChipVersion },
	{ MSG_CMD_SYS_SET_VI_VPSS_MODE,   MSG_SYS_SetVIVPSSMode },
	{ MSG_CMD_SYS_GET_VI_VPSS_MODE,   MSG_SYS_GetVIVPSSMode },
	{ MSG_CMD_SYS_ALIOS_INIT,         MSG_SYS_Alios_Init },
	{ MSG_CMD_SYS_ALIOS_EXIT,         MSG_SYS_Alios_Exit },
	{MSG_CMD_SYS_GET_ALIOS_MEM_STATE, MSG_SYS_GetMemoryState },
	{ MSG_CMD_SYS_FB_ON_SC,           MSG_SYS_SetFbOnSc },
	{ MSG_CMD_LOG_SET_LEVEL_CONF,     MSG_LOG_SetLevelConf },
	{ MSG_CMD_LOG_GET_LEVEL_CONF,     MSG_LOG_GetLevelConf }
};

MSG_SERVER_MODULE_S g_stModuleSys = {
	CVI_ID_SYS,
	"sys",
	sizeof(g_stSysCmdTable) / sizeof(MSG_MODULE_CMD_S),
	&g_stSysCmdTable[0]
};

MSG_SERVER_MODULE_S *MSG_GetSysMod(CVI_VOID)
{
	return &g_stModuleSys;
}

