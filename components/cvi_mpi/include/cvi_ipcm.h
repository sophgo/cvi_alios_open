
#ifndef __CVI_IPCM_H__
#define __CVI_IPCM_H__

#include "cvi_common.h"
#include "cvi_comm_ipcm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

CVI_S32 CVI_IPCM_Init(void);

CVI_S32 CVI_IPCM_Uninit(void);

CVI_S32 CVI_IPCM_InvData(CVI_VOID *pData, CVI_U32 u32Size);

CVI_S32 CVI_IPCM_FlushData(CVI_VOID *pData, CVI_U32 u32Size);

CVI_S32 CVI_IPCM_DataLock(CVI_U8 u8LockID);

CVI_S32 CVI_IPCM_DataUnlock(CVI_U8 u8LockID);

CVI_VOID *CVI_IPCM_GetBuff(CVI_U32 u32Size);

CVI_S32 CVI_IPCM_ReleaseBuff(CVI_VOID *pData);

CVI_VOID *CVI_IPCM_GetUserAddr(CVI_U32 paddr);

CVI_S32 CVI_IPCM_PoolReset(void);

CVI_U32 CVI_IPCM_GetParamBinAddr(void);

CVI_U32 CVI_IPCM_GetParamBakBinAddr(void);

CVI_U32 CVI_IPCM_GetPQBinQddr(void);

CVI_S32 CVI_IPCM_AnonInit(void);

CVI_S32 CVI_IPCM_AnonUninit(void);

CVI_S32 CVI_IPCM_RegisterAnonHandle(CVI_U8 u8PortID, IPCM_ANON_MSGPROC_FN pfnHandler, CVI_VOID *pData);

CVI_S32 CVI_IPCM_DeregisterAnonHandle(CVI_U8 u8PortID);

// send msg if msg len > 4; max msg length is limited by pool block (2048?)
CVI_S32 CVI_IPCM_AnonSendMsg(CVI_U8 u8PortID, CVI_U8 u8MsgID, CVI_VOID *pData, CVI_U32 u32Len);

// send param if msg len <= 4 or send 32 bits addr
CVI_S32 CVI_IPCM_AnonSendParam(CVI_U8 u8PortID, CVI_U8 u8MsgID, CVI_U32 u32Param);

CVI_VOID *CVI_IPCM_AnonGetUserAddr(CVI_U32 u32Paddr);

// only effect in rtos
CVI_S32 CVI_IPCM_SetRtosSysBootStat(void);

// only effect in rtos
CVI_S32 CVI_IPCM_ClrRtosSysBootStat(void);

CVI_S32 CVI_IPCM_GetRtosBootStatus(CVI_U32 *pBootStatus);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
