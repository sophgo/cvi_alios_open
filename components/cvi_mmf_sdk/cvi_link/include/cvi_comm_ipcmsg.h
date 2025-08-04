#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdlib.h>
#include <string.h>
#include "cvi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus*/


#define CVI_IPCMSG_MAX_CONTENT_LEN (1024)
#define CVI_IPCMSG_PRIVDATA_NUM (8)
#define CVI_IPCMSG_INVALID_MSGID (0xFFFFFFFFFFFFFFFF)

/**Connection structure*/
typedef struct cviIPCMSG_CONNECT_S {
	CVI_U32 u32RemoteId;	 /**<Indicate the enumeration values for connecting to remote CPUs*/
	CVI_U32 u32Port;		 /**<Custom port number for message communication*/
	CVI_U32 u32Priority;	 /**<Priority of message transmission*/
} CVI_IPCMSG_CONNECT_S;

/**Message structure*/
typedef struct cviIPCMSG_MESSAGE_S {
	CVI_BOOL bIsResp;	 /**<Identify the response messgae*/
	CVI_U64 u64Id;		 /**<Message ID*/
	CVI_U32 u32Module;	 /**<Module ID, user-defined*/
	CVI_U32 u32CMD;	     /**<CMD ID, user-defined*/
	CVI_S32 s32RetVal;	 /**<Retrun Value in response message*/
	CVI_U32 u32BodyLen;  /**<Length of pBody*/
	/**<Private data, can be modify directly after ::CVI_IPCMSG_CreateMessage
	or ::CVI_IPCMSG_CreateRespMessage*/
	CVI_S32 as32PrivData[CVI_IPCMSG_PRIVDATA_NUM];
	CVI_VOID *pBody;	 /**<Message body*/
#ifdef __arm__
	CVI_U32 u32VirAddrPadding;
#endif
} CVI_IPCMSG_MESSAGE_S;


/** Error number base */
#define CVI_IPCMSG_ERRNO_BASE 0x1900
/** Parameter is invalid */
#define CVI_IPCMSG_EINVAL (CVI_IPCMSG_ERRNO_BASE+1)
/** The function run timeout */
#define CVI_IPCMSG_ETIMEOUT (CVI_IPCMSG_ERRNO_BASE+2)
/** IPC driver open fail */
#define CVI_IPCMSG_ENOOP (CVI_IPCMSG_ERRNO_BASE+3)
/** Internal error */
#define CVI_IPCMSG_EINTER (CVI_IPCMSG_ERRNO_BASE+4)
/** Null pointer*/
#define CVI_IPCMSG_ENULL_PTR (CVI_IPCMSG_ERRNO_BASE+5)


#define CVI_IPCMSG_MAX_SERVICENAME_LEN (16)


/**
 * @brief Callback of receiving message.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Received message.
 */
typedef void (*CVI_IPCMSG_HANDLE_FN_PTR)(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg);

/**
 * @brief Callback of receiving response message. used by CVI_IPCMSG_SendAsync
 * @param[in] pstMsg Response message.
 */
typedef void (*CVI_IPCMSG_RESPHANDLE_FN_PTR)(CVI_IPCMSG_MESSAGE_S *pstMsg);


/** @}*/  /** <!-- ==== IPCMSG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus*/

#endif
