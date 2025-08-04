#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cvi_type.h"
#include "cvi_vb.h"
#include "cvi_ipcmsg.h"
#include "msg_vb.h"
#include "msg_ctx.h"
#include "cvi_debug.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif


static CVI_S32 MSG_VB_Init(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_Init Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_VB_Exit(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_VB_Exit();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_Exit Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_VB_GetBlock(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VB_BLK blk = VB_INVALID_HANDLE;

	blk = CVI_VB_GetBlock(pstMsg->as32PrivData[0], pstMsg->as32PrivData[1]);
	if (blk == VB_INVALID_HANDLE) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_GetBlock Failed!\n");
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, 0, &blk, sizeof(blk));
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

static CVI_S32 MSG_VB_ReleaseBlock(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VB_BLK *pBlock = (VB_BLK *)pstMsg->pBody;

	s32Ret = CVI_VB_ReleaseBlock(*pBlock);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_ReleaseBlock Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_VB_PhysAddr2Handle(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U64 *pu64PhyAddr = (CVI_U64 *)pstMsg->pBody;
	VB_BLK blk;

	blk = CVI_VB_PhysAddr2Handle(*pu64PhyAddr);
	if (blk == VB_INVALID_HANDLE) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_PhysAddr2Handle Failed, invalid handle.\n");
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, 0, &blk, sizeof(blk));
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

static CVI_S32 MSG_VB_Handle2PhysAddr(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U64 u64PhyAddr;
	VB_BLK *pBlock = (VB_BLK *)pstMsg->pBody;

	u64PhyAddr = CVI_VB_Handle2PhysAddr(*pBlock);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, 0, &u64PhyAddr, sizeof(u64PhyAddr));
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

static CVI_S32 MSG_VB_Handle2PoolId(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VB_POOL pool;
	VB_BLK *pBlock = (VB_BLK *)pstMsg->pBody;

	pool = CVI_VB_Handle2PoolId(*pBlock);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, pool, NULL, 0);
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

static CVI_S32 MSG_VB_InquireUserCnt(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_U32 u32Cnt;
	VB_BLK *pBlock = (VB_BLK *)pstMsg->pBody;

	s32Ret = CVI_VB_InquireUserCnt(*pBlock, &u32Cnt);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_InquireUserCnt Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateGrp call CVI_IPCMSG_CreateRespMessage fail\n");
	}
	respMsg->as32PrivData[0] = u32Cnt;

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CreateGrp call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static CVI_S32 MSG_VB_CreatePool(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VB_POOL pool;

	pool = CVI_VB_CreatePool((VB_POOL_CONFIG_S *)pstMsg->pBody);
	if (pool == VB_INVALID_POOLID) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_CreatePool Failed.\n");
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, pool, NULL, 0);
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

static CVI_S32 MSG_VB_CreateExPool(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VB_POOL pool;

	pool = CVI_VB_CreateExPool((VB_POOL_CONFIG_EX_S *)pstMsg->pBody);
	if (pool == VB_INVALID_POOLID) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_CreatePool Failed.\n");
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, pool, NULL, 0);
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

static CVI_S32 MSG_VB_DestroyPool(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_VB_DestroyPool(pstMsg->as32PrivData[0]);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_DestroyPool Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_VB_SetConfig(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	s32Ret = CVI_VB_SetConfig((VB_CONFIG_S *)pstMsg->pBody);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_SetConfig Failed : %#x!\n", s32Ret);
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

static CVI_S32 MSG_VB_GetConfig(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	VB_CONFIG_S stVbConfig;

	s32Ret = CVI_VB_GetConfig(&stVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_GetConfig Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVbConfig, sizeof(stVbConfig));
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

static CVI_S32 MSG_VB_PrintPool(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	CVI_VB_PrintPool(pstMsg->as32PrivData[0]);

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, 0, NULL, 0);
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

static MSG_MODULE_CMD_S g_stVbCmdTable[] = {
	{ MSG_CMD_VB_INIT,        MSG_VB_Init },
	{ MSG_CMD_VB_EXIT,        MSG_VB_Exit },
	{ MSG_CMD_VB_GET_BLOCK,   MSG_VB_GetBlock },
	{ MSG_CMD_VB_RELEASE_BLOCK,   MSG_VB_ReleaseBlock },
	{ MSG_CMD_VB_PHYS_ADDR2_HANDLE,   MSG_VB_PhysAddr2Handle },
	{ MSG_CMD_VB_HANDLE2_PHYS_ADDR,   MSG_VB_Handle2PhysAddr },
	{ MSG_CMD_VB_HANDLE2_POOL_ID,   MSG_VB_Handle2PoolId },
	{ MSG_CMD_VB_INQUIRE_USER_CNT,	MSG_VB_InquireUserCnt },
	{ MSG_CMD_VB_CREATE_POOL,	MSG_VB_CreatePool },
	{ MSG_CMD_VB_CREATE_EX_POOL,	MSG_VB_CreateExPool },
	{ MSG_CMD_VB_DESTROY_POOL,	MSG_VB_DestroyPool },
	{ MSG_CMD_VB_SET_CONFIG,	MSG_VB_SetConfig },
	{ MSG_CMD_VB_GET_CONFIG,	MSG_VB_GetConfig },
	{ MSG_CMD_VB_PRINT_POOL,	MSG_VB_PrintPool },
};

MSG_SERVER_MODULE_S g_stModuleVb = {
	CVI_ID_VB,
	"vb",
	sizeof(g_stVbCmdTable) / sizeof(MSG_MODULE_CMD_S),
	&g_stVbCmdTable[0]
};

MSG_SERVER_MODULE_S *MSG_GetVbMod(CVI_VOID)
{
	return &g_stModuleVb;
}

