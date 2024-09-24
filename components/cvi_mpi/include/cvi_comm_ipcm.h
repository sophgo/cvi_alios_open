
#ifndef __CVI_COMM_IPCM_H__
#define __CVI_COMM_IPCM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "cvi_common.h"
#include "cvi_type.h"

#define CVI_IPCM_PORT_ANON_VUART 0
#define CVI_IPCM_PORT_ANON_MSG 1
#define CVI_IPCM_ANON_KER_PORT_ST 64

typedef enum _IPCM_RTOS_BOOT_STATUS_E {
    IPCM_RTOS_SYS_INIT_STAT = 0,
    IPCM_RTOS_PANIC,
    IPCM_RTOS_IPCM_DONE,
    IPCM_RTOS_IPCMSG_DONE, // TODO
    IPCM_RTOS_VI_DONE, // TODO
    IPCM_RTOS_VPSS_DONE, // TODO
    IPCM_RTOS_VENC_DONE, // TODO

    IPCM_RTOS_BOOT_STATUS_BUTT,
} IPCM_RTOS_BOOT_STATUS_E;

typedef enum _IPCM_MSG_TYPE_E {
	IPCM_MSG_TYPE_SHM = 0,	// msg_param is share memory addr
	IPCM_MSG_TYPE_RAW_PARAM,	// msg_param is the param
} IPCM_MSG_TYPE_E;

typedef struct _IPCM_ANON_SHM_DATA_S {
    CVI_VOID *pData;
    CVI_U32 u32Size;
} IPCM_ANON_SHM_DATA_S;

typedef struct _IPCM_ANON_MSG_S {
    CVI_U8 u8PortID;
    CVI_U8 u8MsgID : 7;
	CVI_U8 u8DataType : 1;
	union {
		IPCM_ANON_SHM_DATA_S stData; // while u8DataType is 0
		CVI_U32 u32Param; // while u8DataType is 1
	};
} IPCM_ANON_MSG_S;

typedef CVI_S32 (*IPCM_ANON_MSGPROC_FN)(CVI_VOID *pPriv, IPCM_ANON_MSG_S *pstData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_COMM_IPCM_H__ */
