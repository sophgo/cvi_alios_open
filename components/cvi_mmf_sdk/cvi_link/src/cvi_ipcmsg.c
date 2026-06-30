#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <errno.h>
#include <semaphore.h>
#include <inttypes.h>

#include "cvi_type.h"
#include "list.h"
#include "cvi_ipcmsg.h"

#include "ipcm_port.h"
#include "ipcm_message.h"
#include "cvi_debug.h"
#if defined(CONFIG_KERNEL_RHINO)
#include <aos/cli.h>
#include "debug/dbg.h"
#endif
//#define IPCM_DEV "/dev/ipcm"

#define MSG_MAGIC 0x9472
#define SERVICE_NAME_MAX_LEN 32
#define MSG_MAX_LEN 4096


#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/****
#if defined(CONFIG_KERNEL_RHINO)
#define CVI_TRACE_IPCMSG(level, fmt, ...)  \
		aos_debug_printf("%s:%d:%s(): " fmt"\r", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define CVI_TRACE_IPCMSG(fmt, ...)  \
		printf("%s:%d:%s(): " fmt"\r", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
#endif
****/

#define IPCMSG_CHECK_NULL_PTR(ptr)  \
	do {  \
		if (!(ptr)) {  \
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "NULL pointer.\n");  \
			return CVI_IPCMSG_ENULL_PTR;  \
		}  \
	} while (0)


struct ipcmsg_service {
	CVI_IPCMSG_CONNECT_S stConnect;
	CVI_CHAR aszServiceName[SERVICE_NAME_MAX_LEN];
	CVI_S32 fd;
	CVI_BOOL isConnected;
	CVI_BOOL isRun;
	CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle;
	struct list_head node;
};

struct msg_item {
	CVI_IPCMSG_MESSAGE_S *pstMsg;
	CVI_IPCMSG_MESSAGE_S *pstRespMsg;
	CVI_IPCMSG_RESPHANDLE_FN_PTR pfnRespHandle;
	pthread_cond_t cond;
	pthread_mutex_t lock;
	CVI_U64 u64PtsUs;
	CVI_U64 u64PtsTmp;
	CVI_BOOL isSync;
	struct list_head node;
};

struct msg_head {
	CVI_U16 u16Magic;
	CVI_U16 u16MsgLen;
};

struct ipcmsg_status_info {
	atomic_uint RecvCnt;
	atomic_uint SendCnt;
	atomic_uint SendSyncCnt;
	atomic_uint SendASyncCnt;
	atomic_uint SendOnlyCnt;
	atomic_uint ThreadStep; //0:poll  1:read msg  2:message 3:resp message
};


static pthread_mutex_t s_Service_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_req_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_uint s_msg_id = ATOMIC_VAR_INIT(0);
static struct ipcmsg_status_info s_status;
static pthread_once_t ipcmsg_once = PTHREAD_ONCE_INIT;
static CVI_IPCMSG_MESSAGE_S s_stCurMsg;


LIST_HEAD(s_ServiceList);
LIST_HEAD(s_ReqList);

void ipcmsg_param_init(void)
{
	atomic_init(&s_status.RecvCnt, 0);
	atomic_init(&s_status.SendCnt, 0);
	atomic_init(&s_status.SendSyncCnt, 0);
	atomic_init(&s_status.SendASyncCnt, 0);
	atomic_init(&s_status.SendOnlyCnt, 0);
	atomic_init(&s_status.ThreadStep, 0);
}

static struct ipcmsg_service *_find_service_by_id(CVI_S32 s32Id)
{
	struct ipcmsg_service *pstService = NULL;

	pthread_mutex_lock(&s_Service_lock);
	if (list_empty(&s_ServiceList)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "s_ServiceList empty, s32Id(%d).\n", s32Id);
		pthread_mutex_unlock(&s_Service_lock);
		return NULL;
	}

	list_for_each_entry(pstService, &s_ServiceList, node) {
		if (s32Id == pstService->fd) {
			pthread_mutex_unlock(&s_Service_lock);
			return pstService;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);

	return NULL;
}

CVI_U64 _GetCurUsPTS(CVI_VOID)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

static CVI_S32 _send_msg(CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = 0;
	struct msg_head stHead;
	CVI_U8 msg_id = 0;
	CVI_VOID *p = NULL;
	CVI_U32 u32MsgLen = sizeof(*pstMsg) + pstMsg->u32BodyLen;
	CVI_U32 u32HeadLen = sizeof(stHead);
	CVI_U32 u32TotalLen = u32HeadLen + u32MsgLen;
	//CVI_U64 u64Time1, u64Time2;

	if (u32TotalLen > MSG_MAX_LEN) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Message len(%d), exceeds the maximum value(%d)\n", u32TotalLen, MSG_MAX_LEN);
		return CVI_FAILURE;
	}
	//u64Time1 = csi_tick_get_us();

	//CVI_TRACE_IPCMSG(CVI_DBG_ERR, "[send] resp(%d) ID(%ld) mod(%x) cmd(%d), msg:%ld body_len:%d\n",
	//	pstMsg->bIsResp, pstMsg->u64Id, pstMsg->u32Module, pstMsg->u32CMD,
	//	sizeof(CVI_IPCMSG_MESSAGE_S), pstMsg->u32BodyLen);

	p = ipcm_msg_get_buff(u32TotalLen);
	if (!p) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ipcm_msg_get_buff failed.\n");
		return CVI_FAILURE;
	}
	//CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ipcm_msg_get_buff:%p.\n", p);

	stHead.u16Magic = MSG_MAGIC;
	stHead.u16MsgLen = u32MsgLen;
	memcpy(p, &stHead, u32HeadLen);
	memcpy(p + u32HeadLen, pstMsg, u32MsgLen);

	s32Ret = ipcm_msg_send_msg(0, msg_id, p, u32TotalLen);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ipcm_msg_send_msg failed.\n");
		return CVI_FAILURE;
	}

	atomic_fetch_add(&s_status.SendCnt, 1);
	//u64Time2 = csi_tick_get_us();
	//printf("_send_msg cost:%ld\n", u64Time2 - u64Time1);

	return CVI_SUCCESS;
}

static CVI_IPCMSG_MESSAGE_S *_read_msg(CVI_VOID)
{
	struct msg_head *pstHead;
	CVI_IPCMSG_MESSAGE_S *pstMsg;
	CVI_U8 *buf = NULL;
	CVI_S32 s32RecvLen;
	CVI_U32 u32MsgLen;
	CVI_U8 *data = NULL;
	CVI_U32 u32HeadLen = sizeof(struct msg_head);
	//CVI_U64 u64Time1, u64Time2;

	//u64Time1 = csi_tick_get_us();
	s32RecvLen = ipcm_msg_read_data(0, (void **)&data, u32HeadLen);
	if (s32RecvLen != u32HeadLen) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "read head failed, recv_len(%d)\n", s32RecvLen);
		return NULL;
	}

	//CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ipcm_msg_read_data:%p\n", data);
	pstHead = (struct msg_head *)data;
	u32MsgLen = pstHead->u16MsgLen;
	//CVI_TRACE_IPCMSG(CVI_DBG_ERR, "[recv] head u16MsgLen:%d,\n", u32MsgLen);

	if (pstHead->u16Magic != MSG_MAGIC) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "magic error, magic(%d), recv magic(%d).\n", MSG_MAGIC, pstHead->u16Magic);
		return NULL;
	}
	if (u32MsgLen > MSG_MAX_LEN) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "len error, len(%d).\n", u32MsgLen);
		return NULL;
	}

	s32RecvLen = ipcm_msg_read_data(0, (void **)&data, u32MsgLen);
	if (s32RecvLen != u32MsgLen) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ipcm_msg_read_data failed, u16MsgLen(%d), recv_len(%d)\n",
			u32MsgLen, s32RecvLen);
		return NULL;
	}
	atomic_fetch_add(&s_status.RecvCnt, 1);

	buf = malloc(u32MsgLen);
	if (!buf) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32MsgLen);
		return NULL;
	}
	memcpy(buf, data, u32MsgLen);
	pstMsg = (CVI_IPCMSG_MESSAGE_S *)buf;

	//CVI_TRACE_IPCMSG(CVI_DBG_ERR, "[recv] resp(%d) ID(%ld) mod(%x) cmd(%d), msg:%ld body_len:%d\n",
	//	pstMsg->bIsResp, pstMsg->u64Id, pstMsg->u32Module, pstMsg->u32CMD,
	//	sizeof(CVI_IPCMSG_MESSAGE_S), pstMsg->u32BodyLen);

	if (pstMsg->u32BodyLen > u32MsgLen) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "u32BodyLen(%d) error.\n", pstMsg->u32BodyLen);
		free(buf);
		return NULL;
	}

	pstMsg->pBody = buf + sizeof(CVI_IPCMSG_MESSAGE_S);
	//u64Time2 = csi_tick_get_us();
	//printf("_read_msg cost:%ld\n", u64Time2 - u64Time1);

	return pstMsg;
}

static CVI_S32 _resp_proc(CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	struct msg_item *item, *tmp;

	pthread_mutex_lock(&s_req_lock);
	if (list_empty(&s_ReqList)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "s_ReqList empty, msg id(%llu).\n", (unsigned long long)pstMsg->u64Id);
		goto empty_err;
	}

	list_for_each_entry_safe(item, tmp, &s_ReqList, node) {
		if (item->pstMsg->u64Id == pstMsg->u64Id) {
			pthread_mutex_unlock(&s_req_lock);

			if (item->isSync) {
				pthread_mutex_lock(&item->lock);
				item->pstRespMsg = pstMsg;
				item->u64PtsTmp = _GetCurUsPTS();
				pthread_cond_signal(&item->cond);
				pthread_mutex_unlock(&item->lock);
			} else {
				item->pfnRespHandle(pstMsg);
				pthread_mutex_lock(&s_req_lock);
				list_del(&item->node);
				pthread_mutex_unlock(&s_req_lock);
				free(item);
				CVI_IPCMSG_DestroyMessage(pstMsg);
			}
			return CVI_SUCCESS;
		}
	}

empty_err:
	pthread_mutex_unlock(&s_req_lock);
	CVI_IPCMSG_DestroyMessage(pstMsg);

	return CVI_FAILURE;
}

CVI_S32 CVI_IPCMSG_AddService(const CVI_CHAR *pszServiceName, const CVI_IPCMSG_CONNECT_S *pstConnectAttr)
{
	int fd = 0;
	struct ipcmsg_service *pstService;

	IPCMSG_CHECK_NULL_PTR(pszServiceName);
	IPCMSG_CHECK_NULL_PTR(pstConnectAttr);

	pthread_mutex_lock(&s_Service_lock);
	list_for_each_entry(pstService, &s_ServiceList, node) {
		if (pstService->stConnect.u32Port == pstConnectAttr->u32Port) {
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "The port is occupied, prot(%d)\n", pstConnectAttr->u32Port);
			pthread_mutex_unlock(&s_Service_lock);
			return CVI_IPCMSG_EINVAL;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);

	ipcm_port_init();
#if defined(CONFIG_KERNEL_RHINO)
	PoolConfig *config = NULL;

	ipcm_port_reset_pool_mgr(); // reset pool mgr
	config = (PoolConfig *)malloc(sizeof(PoolConfig) + 2 * sizeof(BlockConfig));
	if (config) {
		config->num = 2;
		config->blk_conf[0].size = sizeof(struct msg_head) + sizeof(CVI_IPCMSG_MESSAGE_S);
		config->blk_conf[0].num = 10;
		config->blk_conf[1].size = MSG_MAX_LEN;
		config->blk_conf[1].num = 10;
		ipcm_msg_srv_init(config);
		free(config);
	} else {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%zu)\n", sizeof(PoolConfig) + 2 * sizeof(BlockConfig));
		return CVI_FAILURE;
	}
#else
	fd = ipcm_msg_cli_init();
#endif
	if (fd < 0) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ipcm_msg_init failed.\n");
		return CVI_IPCMSG_EINTER;
	}

	pstService = (struct ipcmsg_service *)calloc(sizeof(*pstService), 1);
	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%zu)\n", sizeof(*pstService));
	#if defined(CONFIG_KERNEL_RHINO)
		ipcm_msg_srv_uninit();
	#else
		ipcm_msg_cli_uninit();
	#endif
		return CVI_FAILURE;
	}

	pstService->fd = fd;
	pstService->stConnect = *pstConnectAttr;
	strncpy(pstService->aszServiceName, pszServiceName, SERVICE_NAME_MAX_LEN - 1);
	pthread_mutex_lock(&s_Service_lock);
	list_add_tail(&pstService->node, &s_ServiceList);
	pthread_mutex_unlock(&s_Service_lock);

	pthread_once(&ipcmsg_once, ipcmsg_param_init);

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_DelService(const CVI_CHAR *pszServiceName)
{
	struct ipcmsg_service *pstService, *tmp;

	IPCMSG_CHECK_NULL_PTR(pszServiceName);

	pthread_mutex_lock(&s_Service_lock);
	list_for_each_entry_safe(pstService, tmp, &s_ServiceList, node) {
		if (!strcmp(pszServiceName, pstService->aszServiceName)) {
			list_del(&pstService->node);
		#if defined(CONFIG_KERNEL_RHINO)
			ipcm_msg_srv_uninit();
		#else
			ipcm_msg_cli_uninit();
		#endif
			free(pstService);
			break;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);
	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_TryConnect(CVI_S32 *ps32Id, const CVI_CHAR *pszServiceName,
		CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle)
{
	int fd = -1;
	struct ipcmsg_service *pstService = NULL;

	IPCMSG_CHECK_NULL_PTR(ps32Id);
	IPCMSG_CHECK_NULL_PTR(pszServiceName);
	IPCMSG_CHECK_NULL_PTR(pfnMessageHandle);

	pthread_mutex_lock(&s_Service_lock);
	list_for_each_entry(pstService, &s_ServiceList, node) {
		if (!strcmp(pszServiceName, pstService->aszServiceName)) {
			fd = pstService->fd;
			break;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);

	if (fd < 0) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "%s, service not find.\n", pszServiceName);
		return CVI_FAILURE;
	}

	//send message

	pstService->pfnMessageHandle = pfnMessageHandle;
	pstService->isConnected = CVI_TRUE;
	*ps32Id = fd;

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_Connect(CVI_S32 *ps32Id, const CVI_CHAR *pszServiceName,
		CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle)
{
	return CVI_IPCMSG_TryConnect(ps32Id, pszServiceName, pfnMessageHandle);
}

CVI_S32 CVI_IPCMSG_Disconnect(CVI_S32 s32Id)
{
	struct ipcmsg_service *pstService = _find_service_by_id(s32Id);

	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "service not find, ID(%d).\n", s32Id);
		return CVI_FAILURE;
	}

	pstService->isConnected = CVI_FALSE;
	pstService->pfnMessageHandle = NULL;

	//send message

	while (pstService->isRun)
		usleep(2000);

	return CVI_SUCCESS;
}

CVI_BOOL CVI_IPCMSG_IsConnected(CVI_S32 s32Id)
{
	struct ipcmsg_service *pstService = _find_service_by_id(s32Id);

	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "service not find, ID(%d).\n", s32Id);
		return CVI_FALSE;
	}
	return pstService->isConnected;
}

CVI_IPCMSG_MESSAGE_S *CVI_IPCMSG_CreateMessage(CVI_U32 u32Module, CVI_U32 u32CMD,
		CVI_VOID *pBody, CVI_U32 u32BodyLen)
{
	CVI_IPCMSG_MESSAGE_S *pstMsg = NULL;
	CVI_U32 u32Len = u32BodyLen + sizeof(CVI_IPCMSG_MESSAGE_S);

	pstMsg = (CVI_IPCMSG_MESSAGE_S *)calloc(u32Len, 1);
	if (!pstMsg) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return NULL;
	}

	pstMsg->bIsResp = CVI_FALSE;
	pstMsg->u32Module = u32Module;
	pstMsg->u64Id = atomic_fetch_add(&s_msg_id, 1);
	pstMsg->u32CMD = u32CMD;
	pstMsg->s32RetVal = 0;
	pstMsg->u32BodyLen = u32BodyLen;
	if ((u32BodyLen > 0) && pBody) {
		pstMsg->pBody = (CVI_VOID *)pstMsg + sizeof(CVI_IPCMSG_MESSAGE_S);
		memcpy(pstMsg->pBody, pBody, u32BodyLen);
	}
	return pstMsg;
}

CVI_IPCMSG_MESSAGE_S *CVI_IPCMSG_CreateRespMessage(CVI_IPCMSG_MESSAGE_S *pstRequest,
		CVI_S32 s32RetVal, CVI_VOID *pBody, CVI_U32 u32BodyLen)
{
	CVI_IPCMSG_MESSAGE_S *pRespMsg = NULL;
	CVI_U32 u32Len = u32BodyLen + sizeof(CVI_IPCMSG_MESSAGE_S);

	if(!pstRequest) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "NULL pointer.\n");
		return NULL;
	}

	pRespMsg = (CVI_IPCMSG_MESSAGE_S *)calloc(u32Len, 1);
	if (!pRespMsg) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return NULL;
	}

	pRespMsg->bIsResp = CVI_TRUE;
	pRespMsg->u64Id = pstRequest->u64Id;
	pRespMsg->u32Module = pstRequest->u32Module;
	pRespMsg->u32CMD = pstRequest->u32CMD;
	pRespMsg->s32RetVal = s32RetVal;
	pRespMsg->u32BodyLen = u32BodyLen;
	if ((u32BodyLen > 0) && pBody) {
		pRespMsg->pBody = (CVI_VOID *)pRespMsg + sizeof(CVI_IPCMSG_MESSAGE_S);
		memcpy(pRespMsg->pBody, pBody, u32BodyLen);
	}
	return pRespMsg;
}

CVI_VOID CVI_IPCMSG_DestroyMessage(CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	if (pstMsg)
		free(pstMsg);
}

CVI_S32 CVI_IPCMSG_SendOnly(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstRequest)
{
	struct ipcmsg_service *pstService = _find_service_by_id(s32Id);

	IPCMSG_CHECK_NULL_PTR(pstRequest);

	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Invalid s32Id(%d).\n", s32Id);
		return CVI_FAILURE;
	}

	//send msg
	if (_send_msg(pstRequest)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "_send_msg failed.");
		return CVI_FAILURE;
	}
	atomic_fetch_add(&s_status.SendOnlyCnt, 1);

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_SendAsync(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg,
		CVI_IPCMSG_RESPHANDLE_FN_PTR pfnRespHandle)
{
	struct ipcmsg_service *pstService = _find_service_by_id(s32Id);
	struct msg_item *item = NULL;
	CVI_S32 s32Ret = CVI_FAILURE;

	IPCMSG_CHECK_NULL_PTR(pstMsg);

	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Invalid s32Id(%d).\n", s32Id);
		return CVI_FAILURE;
	}

	if (pfnRespHandle) {
		item = (struct msg_item *)calloc(sizeof(*item), 1);
		if (!item) {
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%zu)\n", sizeof(*item));
			return CVI_FAILURE;
		}

		item->pstMsg = pstMsg;
		item->isSync = CVI_FALSE;
		item->pfnRespHandle = pfnRespHandle;
		item->u64PtsUs = _GetCurUsPTS();

		pthread_mutex_lock(&s_req_lock);
		list_add_tail(&item->node, &s_ReqList);
		pthread_mutex_unlock(&s_req_lock);
	}

	//send msg
	if (_send_msg(pstMsg)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "_send_msg failed.");
		goto err;
	}
	atomic_fetch_add(&s_status.SendASyncCnt, 1);

	return CVI_SUCCESS;
err:
	if (pfnRespHandle && item) {
		pthread_mutex_lock(&s_req_lock);
		list_del(&item->node);
		pthread_mutex_unlock(&s_req_lock);
		free(item);
	}
	return s32Ret;
}

CVI_S32 CVI_IPCMSG_SendSync(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg,
		CVI_IPCMSG_MESSAGE_S **ppstMsg, CVI_S32 s32TimeoutMs)
{
	struct ipcmsg_service *pstService = _find_service_by_id(s32Id);
	struct msg_item *item;
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_U32 timeout_s = s32TimeoutMs / 1000;
	pthread_condattr_t condAttr;
	struct timespec abstime;

	IPCMSG_CHECK_NULL_PTR(pstMsg);
	IPCMSG_CHECK_NULL_PTR(ppstMsg);

	*ppstMsg = NULL;
	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Invalid s32Id(%d).\n", s32Id);
		return CVI_FAILURE;
	}

	item = (struct msg_item *)calloc(sizeof(*item), 1);
	if (!item) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%zu)\n", sizeof(*item));
		return CVI_FAILURE;
	}

	pthread_condattr_init(&condAttr);
	pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
	if (pthread_cond_init(&item->cond, &condAttr) != 0) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "cond init failed.");
		free(item);
		return CVI_FAILURE;
	}
	pthread_mutex_init(&item->lock, NULL);
	item->pstMsg = pstMsg;
	item->isSync = CVI_TRUE;
	item->u64PtsUs = _GetCurUsPTS();

	pthread_mutex_lock(&s_req_lock);
	list_add_tail(&item->node, &s_ReqList);
	pthread_mutex_unlock(&s_req_lock);

	while (clock_gettime(CLOCK_MONOTONIC, &abstime) != 0)
		continue;

	pthread_mutex_lock(&item->lock);
	s32TimeoutMs %= 1000;
	abstime.tv_nsec += s32TimeoutMs * 1000000;
	abstime.tv_sec += (timeout_s + abstime.tv_nsec / 1000000000L);
	abstime.tv_nsec = abstime.tv_nsec % 1000000000L;
	//send msg
	if (_send_msg(pstMsg)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "_send_msg failed.");
		pthread_mutex_unlock(&item->lock);
		goto err;
	}

	atomic_fetch_add(&s_status.SendSyncCnt, 1);
	s32Ret = pthread_cond_timedwait(&item->cond, &item->lock, &abstime);
	pthread_mutex_unlock(&item->lock);

	if (s32Ret == 0)
		*ppstMsg = item->pstRespMsg;
	else
		s32Ret = CVI_FAILURE;

err:
	pthread_mutex_lock(&s_req_lock);
	list_del(&item->node);
	pthread_mutex_unlock(&s_req_lock);
	pthread_mutex_destroy(&item->lock);
	pthread_cond_destroy(&item->cond);
	free(item);
	return s32Ret;
}

CVI_VOID CVI_IPCMSG_Run(CVI_S32 s32Id)
{
#if defined(CONFIG_KERNEL_RHINO)
#else
	fd_set rfds;
	struct timeval timeout;
	CVI_S32 fd;
#endif
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pstMsg;
	struct ipcmsg_service *pstService = _find_service_by_id(s32Id);

	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Invalid s32Id(%d).\n", s32Id);
		return;
	}
	pstService->isRun = CVI_TRUE;

	while (pstService->isConnected) {
		atomic_store(&s_status.ThreadStep, 0);
#if defined(CONFIG_KERNEL_RHINO)
		s32Ret = ipcm_msg_poll(0, 1000);
		if (s32Ret)
			continue;
#else
		fd = pstService->fd;
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		s32Ret = select(fd + 1, &rfds, NULL, NULL, &timeout);
		if (s32Ret < 0) {
			if (errno == EINTR)
				continue;
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "SELECT error\n");
			break;
		} else if (!s32Ret) {
			//timeout
			//Clear invalid nodes of async
			continue;
		}

		if (!FD_ISSET(fd, &rfds))
			continue;
#endif
		atomic_store(&s_status.ThreadStep, 1);

		pstMsg = _read_msg();
		if (!pstMsg) {
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "read msg ERR\n");
			break;
		}
		memcpy(&s_stCurMsg, pstMsg, sizeof(s_stCurMsg));

		if (pstMsg->bIsResp) {
			atomic_store(&s_status.ThreadStep, 3);
			s32Ret = _resp_proc(pstMsg);
			if (s32Ret) {
				CVI_TRACE_IPCMSG(CVI_DBG_ERR, "No corresponding message sender was found.\n");
			}
		} else {
			atomic_store(&s_status.ThreadStep, 2);
			pstService->pfnMessageHandle(s32Id, pstMsg);
		}

	}

	pstService->isRun = CVI_FALSE;
}

#if defined(CONFIG_KERNEL_RHINO)
static void ipcmsg_status_show(int32_t argc, char **argv)
{
	printf("------ipcmsg status------\n");
	printf("---thread step(%d)---\n", atomic_load(&s_status.ThreadStep));
	printf("---current msg: resp(%d) ID(%llu) mod(%x) cmd(%d)\n",
		s_stCurMsg.bIsResp, (unsigned long long)s_stCurMsg.u64Id, s_stCurMsg.u32Module, s_stCurMsg.u32CMD);
	printf("---recv(%d) send(%d)---\n", atomic_load(&s_status.RecvCnt),
		atomic_load(&s_status.SendCnt));
	printf("---SendSync(%d) SendASync(%d) SendOnly(%d)---\n",
		atomic_load(&s_status.SendSyncCnt),
		atomic_load(&s_status.SendASyncCnt),
		atomic_load(&s_status.SendOnlyCnt));
	printf("------------\n");
}

ALIOS_CLI_CMD_REGISTER(ipcmsg_status_show, ipcmsg_status, ipcmsg status);
#endif
