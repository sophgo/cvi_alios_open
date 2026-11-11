#ifndef __CVI_MSG_SERVER_H__
#define __CVI_MSG_SERVER_H__

typedef struct tagMSG_PRIV_DATA_S {
	CVI_S32 as32PrivData[8];
} MSG_PRIV_DATA_S;

CVI_S32 CVI_MSG_Init(CVI_VOID);
CVI_S32 CVI_MSG_Deinit(CVI_VOID);
CVI_S32 CVI_MSG_SendSync_CB(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					 MSG_PRIV_DATA_S *pstPrivData);
#endif
