#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cvi_type.h"
#include "cvi_ipcmsg.h"
#include "msg_audio.h"
#include "msg_ctx.h"
#include "cvi_ai.h"
#include "cvi_ssp.h"
#include "cvi_ao.h"
#include "cvi_debug.h"

static CVI_S32 MSG_AUDIO_AI_Open(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U64 addr;

	s32Ret = cvi_audio_ai_open(pstMsg->pBody, &addr);
	if (s32Ret < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "cvi_audio_ai_open fail,s32Ret:%d\n", s32Ret);
		return s32Ret;
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}
	respMsg->as32PrivData[0] = (CVI_U32)addr;
	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}
	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_AUDIO_AO_Open(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = cvi_audio_ao_open(pstMsg->pBody);
	if (s32Ret < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "cvi_audio_ao_open fail,s32Ret:%d\n", s32Ret);
		return s32Ret;
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

static CVI_S32 MSG_AUDIO_AI_Close(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = cvi_audio_ai_close((CVI_VOID *)&pstMsg->as32PrivData[0]);
	if (s32Ret < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "cvi_audio_ai_close fail,s32Ret:%d\n", s32Ret);
		return s32Ret;
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

static CVI_S32 MSG_AUDIO_AO_Close(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = cvi_audio_ao_close((CVI_VOID *)&pstMsg->as32PrivData[0]);
	if (s32Ret < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "cvi_audio_ao_close fail,s32Ret:%d\n", s32Ret);
		return s32Ret;
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

static CVI_S32 MSG_AUDIO_SetVolume(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U8 card_id = pstMsg->as32PrivData[0];

	if (card_id == 0) {
		s32Ret = cvi_audio_ai_setvol(card_id, pstMsg->as32PrivData[1]);
	} else if (card_id == 1) {
		s32Ret = cvi_audio_ao_setvol(card_id, pstMsg->as32PrivData[1]);
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
static CVI_S32 MSG_AUDIO_GetVolume(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U8 card_id = pstMsg->as32PrivData[0];

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}
	if (card_id == 0) {
		cvi_audio_ai_getvol(card_id, &respMsg->as32PrivData[0]);
	} else if (card_id == 1) {
		cvi_audio_ao_getvol(card_id, &respMsg->as32PrivData[0]);
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

static CVI_S32 MSG_AUDIO_SetStereoVolume(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U8 card_id = pstMsg->as32PrivData[0];
	CVI_S32 stereo_id = pstMsg->as32PrivData[2];

	if (card_id == 0) {
		cvi_audio_ai_setstereovol(card_id, stereo_id, pstMsg->as32PrivData[1]);
	} else if (card_id == 1) {
		cvi_audio_ao_setstereovol(card_id, stereo_id, pstMsg->as32PrivData[1]);
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

static CVI_S32 MSG_AUDIO_GetStereoVolume(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U8 card_id = pstMsg->as32PrivData[0];
	CVI_S32 stereo_id = pstMsg->as32PrivData[1];

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}
	if (card_id == 0) {
		cvi_audio_ai_getstereovol(card_id, stereo_id, &respMsg->as32PrivData[0]);
	} else if (card_id == 1) {
		cvi_audio_ao_getstereovol(card_id, stereo_id, &respMsg->as32PrivData[0]);
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

static CVI_S32 MSG_AUDIO_SetMute(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U8 card_id = pstMsg->as32PrivData[0];

	if (card_id == 0) {
		s32Ret = cvi_audio_ai_setmute(card_id, pstMsg->as32PrivData[1]);
	} else if (card_id == 1) {
		s32Ret = cvi_audio_ao_setmute(card_id, pstMsg->as32PrivData[1]);
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

static CVI_S32 MSG_AUDIO_GetMute(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U8 card_id = pstMsg->as32PrivData[0];

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}
	if (card_id == 0) {
		cvi_audio_ai_getmute(card_id, &respMsg->as32PrivData[0]);
	} else if (card_id == 1) {
		cvi_audio_ao_getmute(card_id, &respMsg->as32PrivData[0]);
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


static CVI_S32 MSG_AUDIO_SSP_OPEN(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U64 addr;

	s32Ret = cvi_audio_ssp_open(pstMsg->pBody, &addr);
	if (s32Ret < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "cvi_audio_ai_open fail,s32Ret:%d\n", s32Ret);
		return s32Ret;
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}
	respMsg->as32PrivData[0] = (CVI_U32)addr;
	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DestroyGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}
	CVI_IPCMSG_DestroyMessage(respMsg);

	return CVI_SUCCESS;
}

static CVI_S32 MSG_AUDIO_SSP_CLOSE(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = cvi_audio_ssp_close((CVI_VOID *)&pstMsg->as32PrivData[0]);
	if (s32Ret < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "cvi_audio_ai_close fail,s32Ret:%d\n", s32Ret);
		return s32Ret;
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

static MSG_MODULE_CMD_S g_stAudioCmdTable[] = {
	{ MSG_CMD_AUDIO_AI_OPEN, MSG_AUDIO_AI_Open },
	{ MSG_CMD_AUDIO_AO_OPEN, MSG_AUDIO_AO_Open },
	{ MSG_CMD_AUDIO_AI_CLOSE, MSG_AUDIO_AI_Close },
	{ MSG_CMD_AUDIO_AO_CLOSE, MSG_AUDIO_AO_Close },
	{ MSG_CMD_AUDIO_SET_VOL, MSG_AUDIO_SetVolume },
	{ MSG_CMD_AUDIO_GET_VOL, MSG_AUDIO_GetVolume },
	{ MSG_CMD_AUDIO_SET_STEREO_VOL, MSG_AUDIO_SetStereoVolume },
	{ MSG_CMD_AUDIO_GET_STEREO_VOL, MSG_AUDIO_GetStereoVolume },
	{ MSG_CMD_AUDIO_SET_MUTE, MSG_AUDIO_SetMute },
	{ MSG_CMD_AUDIO_GET_MUTE, MSG_AUDIO_GetMute },
	{ MSG_CMD_AUDIO_SSP_OPEN, MSG_AUDIO_SSP_OPEN },
	{ MSG_CMD_AUDIO_SSP_CLOSE, MSG_AUDIO_SSP_CLOSE },
};

MSG_SERVER_MODULE_S g_stModuleAudio = {
	CVI_ID_AUD,
	"audio",
	sizeof(g_stAudioCmdTable) / sizeof(MSG_MODULE_CMD_S),
	&g_stAudioCmdTable[0]
};

MSG_SERVER_MODULE_S *MSG_GetAudioMod(CVI_VOID)
{
	return &g_stModuleAudio;
}
