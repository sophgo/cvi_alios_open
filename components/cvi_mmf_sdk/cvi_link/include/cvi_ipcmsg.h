#ifndef __IPCMSG_H
#define __IPCMSG_H
#include "cvi_comm_ipcmsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** \addtogroup     IPCMSG*/
/** @{ */  /** <!-- [IPCMSG] */



/**
 * @brief Add services to global service table before use of IPCMSG.
 * This function should be called in two side arm with the same parameters.
 * @param[in] pszServiceName Service running on two side(a7/a17).
 * @param[in] u32Port The two service has unique port.
 * @param[in] u32Priority Priority of the communication.
 * @return CVI_SUCCESS Register success.
 * @return CVI_FAILURE Register fail.
 * @return CVI_IPCMSG_EINVAL Parameter is invalid
 * @return CVI_IPCMSG_EINTER Open device fail or other internal error
 */
CVI_S32 CVI_IPCMSG_AddService(const CVI_CHAR *pszServiceName, const CVI_IPCMSG_CONNECT_S *pstConnectAttr);

/**
 * @brief Delete service from global service table when don't need IPCMSG.
 * @param[in] pszServiceName Service running on two side(a7/a17).
 * @return CVI_IPCMSG_EINVAL Parameter is invalid
 * @return CVI_SUCCESS Unregister success.
 * @return CVI_FAILURE Unregister fail.
 */
CVI_S32 CVI_IPCMSG_DelService(const CVI_CHAR *pszServiceName);

/**
 * @brief Establish the connection between master and slave.Attention:
   Message can send successfuly only the two side call this function.
   So ::CVI_IPCMSG_IsConnected should be called to ensure the connection
   is established before send or receive
 * @param[out] ps32Id Handle of IPCMSG. All operation in IPCMSG need this handle
 * @param[in] pszServiceName Service name running on the other side.
 * @param[in] pfnMessageHandle Callback function to receive message.
 * @return CVI_IPCMSG_EINVAL Parameter is invalid
 * @return CVI_IPCMSG_EINTER Open device fail or other internal error
 * @return CVI_SUCCESS Connect success.
 * @return CVI_FAILURE Conectt fail.
 */
CVI_S32 CVI_IPCMSG_TryConnect(CVI_S32 *ps32Id, const CVI_CHAR *pszServiceName,
	CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle);

/**
 * @brief Establish the connection between master and slave. This function
    will block until two side connections established.
 * @param[out] ps32Id Handle of IPCMSG. All operation in IPCMSG need this handle
 * @param[in] pszServiceName Service name running on the other side.
 * @param[in] pfnMessageHandle Callback function to receive message.
 * @return CVI_IPCMSG_EINVAL Parameter is invalid
 * @return CVI_IPCMSG_EINTER Open device fail or other internal error
 * @return CVI_SUCCESS Connect success.
 * @return CVI_FAILURE Conectt fail.
 */
CVI_S32 CVI_IPCMSG_Connect(CVI_S32 *ps32Id, const CVI_CHAR *pszServiceName,
	CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle);

/**
 * @brief Disconnect when don't want to send or receive message.
 * @param[in] s32Id Handle of IPCMSG.
 * @return CVI_IPCMSG_EINVAL Parameter is invalid
 * @return CVI_SUCCESS Disconnect success.
 * @return CVI_FAILURE Disconnect fail.
 */
CVI_S32 CVI_IPCMSG_Disconnect(CVI_S32 s32Id);

/**
 * @brief Whether the communication can work.
 * @param[in] s32Id Handle of IPCMSG
 * @return CVI_TRUE connection is done. can send message.
 * @return CVI_FALSE connection is not finish yet. send message will return failure.
 */
CVI_BOOL CVI_IPCMSG_IsConnected(CVI_S32 s32Id);

/**
 * @brief Send message only, don't need response.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @return CVI_SUCCESS Send success.
 * @return CVI_FAILURE Send fail.
 */
CVI_S32 CVI_IPCMSG_SendOnly(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstRequest);

/**
 * @brief Send message asynchronously. the function will return immediately.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @param[in] pfnRespHandle Callback function to receive response.
 * @return CVI_SUCCESS Send success.
 * @return CVI_FAILURE Send fail.
 */
CVI_S32 CVI_IPCMSG_SendAsync(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg,
	CVI_IPCMSG_RESPHANDLE_FN_PTR pfnRespHandle);

/**
 * @brief Send message synchronously. the function will block until response message received.
 * @param[in] s32Id Handle of IPCMSG.
 * @param[in] pstMsg Message to send.
 * @param[out] ppstMsg Received response message.
 * @param[in] s32TimeoutMs When response not received in s32TimeoutMs, function will return ::CVI_IPCMSG_ETIMEOUT
 * @return CVI_SUCCESS Send success.
 * @return CVI_FAILURE Send fail.
 * #return CVI_IPCMSG_ETIMEOUT Timeout to receive response
 */
CVI_S32 CVI_IPCMSG_SendSync(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg,
	CVI_IPCMSG_MESSAGE_S **ppstMsg, CVI_S32 s32TimeoutMs);

/**
 * @brief In this function, Message will be received and dispatched to message callback function.
 * User should create thread to run this function.
 * @param[in] s32Id Handle of IPCMSG.
 */
CVI_VOID CVI_IPCMSG_Run(CVI_S32 s32Id);

/**
 * @brief Create the message, used by CVI_IPCMSG_SendXXX and receive callback function.
 * @param[in] u32Module Module ID defined by user. user can use it to dispatch to different modules.
 * @param[in] u32CMD CMD ID, defined by user. user can use it to identify which command.
 * @param[in] pBody Message body, mustn't contain pointer because pointer will be useless in other side.
 * @param[in] u32BodyLen Length of pBody.
 * @return ::CVI_IPCMSG_MESSAGE_S* Created message.
 */
CVI_IPCMSG_MESSAGE_S *CVI_IPCMSG_CreateMessage(CVI_U32 u32Module,
	CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen);

/**
 * @brief Create the response message.
 * @param[in] pstRequest Request message received by user.
 * @param[in] s32RetVal Integer return value.
 * @param[in] pBody Message body.
 * @param[in] u32BodyLen Length of pBody.
 * @return ::CVI_IPCMSG_MESSAGE_S* Created message.
 */
CVI_IPCMSG_MESSAGE_S *CVI_IPCMSG_CreateRespMessage(CVI_IPCMSG_MESSAGE_S *pstRequest,
	CVI_S32 s32RetVal, CVI_VOID *pBody, CVI_U32 u32BodyLen);

/**
 * @brief Destroy the message. Message must be destroyed when send and receive finish
 * @param[in] pstMsg Message to destroy.
 */
CVI_VOID CVI_IPCMSG_DestroyMessage(CVI_IPCMSG_MESSAGE_S *pstMsg);


/** @}*/  /** <!-- ==== IPCMSG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
