#ifndef __MSG_CTX_H__
#define __MSG_CTX_H__

#include "cvi_common.h"
#include "cvi_comm_ipcmsg.h"
#include "cvi_workq.h"


#define THREAD_POOL_NUM 4
#define MSG_SERVER_MODULE_NAME_LEN (16)

typedef CVI_S32 (*CMDPROC_FN)(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg);

typedef struct tagMSG_MODULE_CMD_S {
    CVI_U32 u32Cmd;
    CMDPROC_FN CMDPROC_FN_PTR;
} MSG_MODULE_CMD_S;

typedef struct tagMSG_SERVER_MODULE_S {
    CVI_U32 u32ModID;
    CVI_CHAR saModuleName[MSG_SERVER_MODULE_NAME_LEN];
    CVI_U32 u32ModuleCmdAmount;
    MSG_MODULE_CMD_S *pstModuleCmdTable;
} MSG_SERVER_MODULE_S;

typedef struct tagMSG_SERVER_CONTEXT {
    MSG_SERVER_MODULE_S *pstServermodules[CVI_ID_BUTT];
} MSG_SERVER_CONTEXT;


#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/*
#define CVI_TRACE_MSG(fmt, ...)  \
		printf("%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
*/

CVI_S32 CVI_MSG_RegMod(MOD_ID_E enModId, MSG_SERVER_MODULE_S *pstServermodules);
CVI_S32 CVI_MSG_GetSiId(CVI_VOID);


MSG_SERVER_MODULE_S *MSG_GetSysMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetVbMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetViMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetVpssMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetVencMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetVdecMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetGdcMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetIspMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetRgnMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetAudioMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetVoMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetMipitxMod(CVI_VOID);
MSG_SERVER_MODULE_S *MSG_GetSensorMod(CVI_VOID);

#endif
