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
#include "cvi_rpmsg.h"
#include "cvi_ipcmsg.h"

#if defined(CONFIG_KERNEL_RHINO)
#include <aos/cli.h>
#include "debug/dbg.h"
#endif

#define USE_NOCOPY 1
#define IPCMSG_EPT_NAME  "rpmsg-ipcmsg"

#define MSG_MAGIC 0x9472
#define SERVICE_NAME_MAX_LEN 32
#define MSG_MAX_LEN 4080


#if 1
#include "cvi_debug.h"
#else
#define CVI_TRACE_IPCMSG(level, fmt, ...)                         \
do {                                                              \
	printf("[%s-%d] " fmt, __func__, __LINE__,  ##__VA_ARGS__);   \
} while (0)
#endif

#define IPCMSG_CHECK_NULL_PTR(ptr)  \
	do {  \
		if (!(ptr)) {  \
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "NULL pointer.\n");  \
			return CVI_IPCMSG_ENULL_PTR;  \
		}  \
	} while (0)


struct ipcmsg_status_info {
	atomic_uint RecvCnt;
	atomic_uint SendCnt;
	atomic_uint SendSyncCnt;
	atomic_uint SendASyncCnt;
	atomic_uint SendOnlyCnt;
	atomic_uint ThreadStep; //0:poll  1:read msg  2:message 3:resp message
};

struct ipcmsg_service {
	CVI_IPCMSG_CONNECT_S stConnect;
	CVI_CHAR aszServiceName[SERVICE_NAME_MAX_LEN];
	CVI_U32 s32ServiceId;
	CVI_U32 remote_addr;     /* Linux endpoint address, learned from first rx */
	CVI_BOOL isConnected;
	CVI_BOOL isRun;
	CVI_BOOL isLive;
	CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle;
	sem_t cbMsgSem;
	pthread_mutex_t lock;
	pthread_mutex_t cbMsgLock;
	pthread_mutex_t req_lock;
	struct list_head cbMsgList;
	struct list_head reqList;
	struct list_head node;
};

struct msg_item {
	CVI_IPCMSG_MESSAGE_S *pstRespMsg;
	CVI_IPCMSG_RESPHANDLE_FN_PTR pfnRespHandle;
	pthread_cond_t cond;
	pthread_mutex_t lock;
	CVI_U64 u64Id;
	CVI_U64 u64PtsUs;
	CVI_U64 u64PtsTmp;
	CVI_BOOL isSync;
	struct list_head node;
};

struct msg_head {
	CVI_U16 u16Magic;
	CVI_U16 u16MsgLen;
	CVI_CHAR aszServiceName[SERVICE_NAME_MAX_LEN];
};

struct msg_queue_item {
	CVI_IPCMSG_MESSAGE_S *pstMsg;
	uint32_t remote_addr;    /* sender's rpmsg address, for sendto reply */
	struct list_head node;
};


#ifdef USE_NOCOPY 
static pthread_mutex_t s_nocopy_send_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
static atomic_uint s_msg_id = ATOMIC_VAR_INIT(0);
static pthread_mutex_t s_Service_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_uint s_service_id = ATOMIC_VAR_INIT(0);
LIST_HEAD(s_ServiceList);

/* Shared rpmsg endpoint — all services multiplex over one NS name.
 * Created on first Connect/TryConnect; never destroyed. */
static cvi_rpmsg_ept_t *g_shared_ept;
static int g_shared_ept_inited;



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
		if (s32Id == pstService->s32ServiceId) {
			pthread_mutex_unlock(&s_Service_lock);
			return pstService;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);

	return NULL;
}

static struct ipcmsg_service *_find_service_by_name(const CVI_CHAR *name)
{
	struct ipcmsg_service *pstService = NULL;

	pthread_mutex_lock(&s_Service_lock);
	list_for_each_entry(pstService, &s_ServiceList, node) {
		if (!strcmp(name, pstService->aszServiceName)) {
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

static CVI_S32 _send_msg(CVI_IPCMSG_MESSAGE_S *pstMsg, struct ipcmsg_service *pstService)
{
	struct msg_head stHead;
	CVI_VOID *p = NULL;
	CVI_U32 u32MsgLen = sizeof(*pstMsg) + pstMsg->u32BodyLen;
	CVI_U32 u32HeadLen = sizeof(stHead);
	CVI_U32 u32TotalLen = u32HeadLen + u32MsgLen;
	uint32_t dst = pstService->remote_addr;

	if (u32TotalLen > MSG_MAX_LEN) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Message len(%d), exceeds the maximum value(%d)\n", u32TotalLen, MSG_MAX_LEN);
		return CVI_FAILURE;
	}

	if (dst == 0) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "remote_addr not known for service %s, "
			"waiting for Linux to send first.\n", pstService->aszServiceName);
		return CVI_FAILURE;
	}

	CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "[send] resp(%d) ID(%ld) mod(%x) cmd(%d).\n",
		pstMsg->bIsResp, pstMsg->u64Id, pstMsg->u32Module, pstMsg->u32CMD);

	stHead.u16Magic = MSG_MAGIC;
	stHead.u16MsgLen = u32MsgLen;
	memset(stHead.aszServiceName, 0, sizeof(stHead.aszServiceName));
	strncpy(stHead.aszServiceName, pstService->aszServiceName, SERVICE_NAME_MAX_LEN - 1);

#ifdef USE_NOCOPY
	CVI_U32 u32Tmp = u32TotalLen;

	pthread_mutex_lock(&s_nocopy_send_lock);
	p = cvi_rpmsg_get_tx_payload_buffer(g_shared_ept, &u32Tmp, 1);
	if (!p || (u32Tmp < u32TotalLen)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "cvi_rpmsg_get_tx_payload_buffer failed.\n");
		pthread_mutex_unlock(&s_nocopy_send_lock);
		return CVI_FAILURE;
	}
	memcpy(p, &stHead, u32HeadLen);
	memcpy(p + u32HeadLen, pstMsg, u32MsgLen);
	if (cvi_rpmsg_sendto_nocopy(g_shared_ept, p, u32TotalLen, dst) != u32TotalLen) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "cvi_rpmsg_sendto_nocopy failed, len(%d)\n", u32TotalLen);
		pthread_mutex_unlock(&s_nocopy_send_lock);
		free(p);
		return CVI_FAILURE;
	}
	pthread_mutex_unlock(&s_nocopy_send_lock);
#else
	p = malloc(u32TotalLen);
	if (!p) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32TotalLen);
		return CVI_FAILURE;
	}
	memcpy(p, &stHead, u32HeadLen);
	memcpy(p + u32HeadLen, pstMsg, u32MsgLen);
	if (cvi_rpmsg_sendto(g_shared_ept, p, u32TotalLen, dst) != u32TotalLen) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "cvi_rpmsg_sendto failed, len(%d)\n", u32TotalLen);
		free(p);
		return CVI_FAILURE;
	}
	free(p);
#endif

	return CVI_SUCCESS;
}

static CVI_S32 _resp_proc(CVI_IPCMSG_MESSAGE_S *pstMsg, struct ipcmsg_service *pstService)
{
	struct msg_item *item, *tmp;

	pthread_mutex_lock(&pstService->req_lock);
	if (list_empty(&pstService->reqList)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "reqList empty, msg id(%llu).\n", (unsigned long long)pstMsg->u64Id);
		goto empty_err;
	}

	list_for_each_entry_safe(item, tmp, &pstService->reqList, node) {
		if (item->u64Id == pstMsg->u64Id) {
			pthread_mutex_unlock(&pstService->req_lock);

			if (item->isSync) {
				pthread_mutex_lock(&item->lock);
				item->pstRespMsg = pstMsg;
				item->u64PtsTmp = _GetCurUsPTS();
				pthread_cond_signal(&item->cond);
				pthread_mutex_unlock(&item->lock);
			} else {
				item->pfnRespHandle(pstMsg);
				pthread_mutex_lock(&pstService->req_lock);
				list_del(&item->node);
				pthread_mutex_unlock(&pstService->req_lock);
				free(item);
				CVI_IPCMSG_DestroyMessage(pstMsg);
			}
			return CVI_SUCCESS;
		}
	}

empty_err:
	pthread_mutex_unlock(&pstService->req_lock);
	CVI_IPCMSG_DestroyMessage(pstMsg);

	return CVI_FAILURE;
}

static void _rx_msg_handle(CVI_IPCMSG_MESSAGE_S *pstMsg, struct ipcmsg_service *pstService)
{
	if (pstMsg->bIsResp) {
		if ( _resp_proc(pstMsg, pstService)) {
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "No corresponding message sender was found.\n");
		}
	} else {
		pstService->pfnMessageHandle(pstService->s32ServiceId, pstMsg);
	}
}

/*
 * Shared rx callback for the global rpmsg endpoint.
 * Dispatches incoming messages to the correct ipcmsg_service by reading
 * the service name embedded in msg_head.  Also learns the Linux sender's
 * rpmsg address so that replies can be routed back via sendto.
 */
static int _shared_rx_cb(cvi_rpmsg_ept_t *ept, const void *data,
	size_t len, uint32_t src, void *priv)
{
	struct msg_head *pstHead = (struct msg_head *)data;
	struct ipcmsg_service *pstService = NULL;
	CVI_IPCMSG_MESSAGE_S *pstMsg = NULL;
	CVI_U8 *buf = NULL;

	(void)priv;

	if (pstHead->u16Magic != MSG_MAGIC) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "magic error, magic(%d), recv magic(%d).\n", MSG_MAGIC, pstHead->u16Magic);
		return 0;
	}
	if (pstHead->u16MsgLen > MSG_MAX_LEN) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "len error, len(%d).\n", pstHead->u16MsgLen);
		return 0;
	}
	if (len < (pstHead->u16MsgLen + sizeof(*pstHead))) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Len=%ld is too samll.\n", len);
		return 0;
	}

	/* Look up the target service by name */
	pstService = _find_service_by_name(pstHead->aszServiceName);
	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "service name(%s) not registered, dropping.\n",
			pstHead->aszServiceName);
		return 0;
	}

	/* Learn the remote address so we can sendto later */
	if (pstService->remote_addr == 0 || pstService->remote_addr != src) {
		pstService->remote_addr = src;
		CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "name(%s) learned remote_addr=0x%x.\n",
			pstService->aszServiceName, src);
	}

	if (pstHead->u16MsgLen == 0) {
		//heartbeat
		CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "name(%s), heartbeat.\n", pstService->aszServiceName);
		pstService->isLive = CVI_TRUE;
		cvi_rpmsg_sendto(ept, data, (int)len, src);
		return 0;
	}

	if (!pstService->isConnected || !pstService->pfnMessageHandle) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "service name(%s) not connected, dropping.\n",
			pstHead->aszServiceName);
		return 0;
	}

	buf = malloc(pstHead->u16MsgLen);
	if (!buf) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "malloc failed, size(%d)\n", pstHead->u16MsgLen);
		return 0;
	}
	memcpy(buf, data + sizeof(*pstHead), pstHead->u16MsgLen);
	pstMsg = (CVI_IPCMSG_MESSAGE_S *)buf;
	if (pstMsg->u32BodyLen > pstHead->u16MsgLen) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "u32BodyLen(%d) error.\n", pstMsg->u32BodyLen);
		free(buf);
		return 0;
	}
	pstMsg->pBody = buf + sizeof(CVI_IPCMSG_MESSAGE_S);
	CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "[recv] resp(%d) ID(%ld) mod(%x) cmd(%d).\n",
		pstMsg->bIsResp, pstMsg->u64Id, pstMsg->u32Module, pstMsg->u32CMD);

	if (pstService->isRun) {
		/* 将收到的消息放入服务消息队列，由用户RUN处理线程消费 */
		struct msg_queue_item *qitem = (struct msg_queue_item *)calloc(sizeof(*qitem), 1);
		if (!qitem) {
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc msg_queue_item failed.\n");
			free(buf);
			return 0;
		}
		qitem->pstMsg = pstMsg;
		qitem->remote_addr = src;
		pthread_mutex_lock(&pstService->cbMsgLock);
		list_add_tail(&qitem->node, &pstService->cbMsgList);
		pthread_mutex_unlock(&pstService->cbMsgLock);
		sem_post(&pstService->cbMsgSem);
	} else {
		/*直接在回调中处理消息*/
		_rx_msg_handle(pstMsg, pstService);
	}

	return 0;
}

static int _init_shared_ept(void)
{
	if (g_shared_ept_inited)
		return 0;

	g_shared_ept = cvi_rpmsg_create_ept(IPCMSG_EPT_NAME, CVI_RPMSG_ADDR_ANY,
		CVI_RPMSG_ADDR_ANY, _shared_rx_cb, NULL, NULL);
	if (!g_shared_ept) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "cvi_rpmsg_create_ept(rpmsg-echo-test) failed.\n");
		return CVI_FAILURE;
	}

	g_shared_ept_inited = 1;
	CVI_TRACE_IPCMSG(CVI_DBG_INFO, "shared rpmsg ept created, name=rpmsg-echo-test.\n");
	return 0;
}

CVI_S32 CVI_IPCMSG_AddService(const CVI_CHAR *pszServiceName, const CVI_IPCMSG_CONNECT_S *pstConnectAttr)
{
	struct ipcmsg_service *pstService;

	IPCMSG_CHECK_NULL_PTR(pszServiceName);
	IPCMSG_CHECK_NULL_PTR(pstConnectAttr);

	pthread_mutex_lock(&s_Service_lock);
	list_for_each_entry(pstService, &s_ServiceList, node) {
		if (!strcmp(pstService->aszServiceName, pszServiceName)) {
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "The name is occupied, name(%s)\n", pszServiceName);
			pthread_mutex_unlock(&s_Service_lock);
			return CVI_IPCMSG_EINVAL;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);

	pstService = (struct ipcmsg_service *)calloc(sizeof(*pstService), 1);
	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%zu)\n", sizeof(*pstService));
		return CVI_FAILURE;
	}

	pstService->stConnect = *pstConnectAttr;
	pstService->s32ServiceId = atomic_fetch_add(&s_service_id, 1);
	strncpy(pstService->aszServiceName, pszServiceName, SERVICE_NAME_MAX_LEN - 1);

	sem_init(&pstService->cbMsgSem, 0, 0);
	pthread_mutex_init(&pstService->cbMsgLock, NULL);
	INIT_LIST_HEAD(&pstService->cbMsgList);
	INIT_LIST_HEAD(&pstService->reqList);
	pthread_mutex_init(&pstService->req_lock, NULL);
	pthread_mutex_init(&pstService->lock, NULL);

	pthread_mutex_lock(&s_Service_lock);
	list_add_tail(&pstService->node, &s_ServiceList);
	pthread_mutex_unlock(&s_Service_lock);

	CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "name(%s) service created.\n", pszServiceName);

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

			/* Clean up remaining messages in cbMsgList */
			pthread_mutex_lock(&pstService->cbMsgLock);
			while (!list_empty(&pstService->cbMsgList)) {
				struct msg_queue_item *qitem = list_first_entry(&pstService->cbMsgList,
					struct msg_queue_item, node);
				list_del(&qitem->node);
				CVI_IPCMSG_DestroyMessage(qitem->pstMsg);
				free(qitem);
			}
			pthread_mutex_unlock(&pstService->cbMsgLock);

			pthread_mutex_destroy(&pstService->cbMsgLock);
			sem_destroy(&pstService->cbMsgSem);
			pthread_mutex_destroy(&pstService->req_lock);
			pthread_mutex_destroy(&pstService->lock);
			free(pstService);
			break;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);
	CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "name(%s) service destroy.\n", pszServiceName);

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_TryConnect(CVI_S32 *ps32Id, const CVI_CHAR *pszServiceName,
		CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle)
{
	CVI_S32 fd = -1;
	struct ipcmsg_service *pstService = NULL;

	IPCMSG_CHECK_NULL_PTR(ps32Id);
	IPCMSG_CHECK_NULL_PTR(pszServiceName);
	IPCMSG_CHECK_NULL_PTR(pfnMessageHandle);

	pthread_mutex_lock(&s_Service_lock);
	list_for_each_entry(pstService, &s_ServiceList, node) {
		if (!strcmp(pszServiceName, pstService->aszServiceName)) {
			fd = pstService->s32ServiceId;
			break;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);

	if (fd < 0) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "%s, service not find.\n", pszServiceName);
		return CVI_IPCMSG_EINVAL;
	}

	//check rpmsg ready
	if (!cvi_rpmsg_is_ready())
		return CVI_IPCMSG_EINTER;

	/* Lazy-init the shared rpmsg endpoint (once) */
	if (_init_shared_ept() != 0)
		return CVI_IPCMSG_EINTER;

	usleep(1000);
	if (!pstService->isLive)
		return CVI_IPCMSG_EINTER;

	pstService->pfnMessageHandle = pfnMessageHandle;
	pstService->isConnected = CVI_TRUE;
	*ps32Id = fd;
	CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "s32Id:%d, name(%s) connected.\n", fd, pszServiceName);

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_Connect(CVI_S32 *ps32Id, const CVI_CHAR *pszServiceName,
		CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle)
{
	CVI_S32 fd = -1;
	struct ipcmsg_service *pstService = NULL;

	IPCMSG_CHECK_NULL_PTR(ps32Id);
	IPCMSG_CHECK_NULL_PTR(pszServiceName);
	IPCMSG_CHECK_NULL_PTR(pfnMessageHandle);

	pthread_mutex_lock(&s_Service_lock);
	list_for_each_entry(pstService, &s_ServiceList, node) {
		if (!strcmp(pszServiceName, pstService->aszServiceName)) {
			fd = pstService->s32ServiceId;
			break;
		}
	}
	pthread_mutex_unlock(&s_Service_lock);

	if (fd < 0) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "%s, service not find.\n", pszServiceName);
		return CVI_IPCMSG_EINVAL;
	}

	//block until rpmsg init complete
	while (!cvi_rpmsg_is_ready()) {
		usleep(10000);
	}

	/* Lazy-init the shared rpmsg endpoint (once) */
	if (_init_shared_ept() != 0)
		return CVI_IPCMSG_EINTER;

	while (!pstService->isLive) {
		usleep(100);
	}

	pstService->pfnMessageHandle = pfnMessageHandle;
	pstService->isConnected = CVI_TRUE;
	*ps32Id = fd;
	CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "s32Id:%d, name(%s) connected.\n", fd, pszServiceName);
	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_Disconnect(CVI_S32 s32Id)
{
	struct ipcmsg_service *pstService = _find_service_by_id(s32Id);
	struct msg_item *item, *tmp;

	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "service not find, ID(%d).\n", s32Id);
		return CVI_FAILURE;
	}

	pstService->isConnected = CVI_FALSE;
	pstService->pfnMessageHandle = NULL;
	sem_post(&pstService->cbMsgSem);

	while (pstService->isRun)
		usleep(2000);

	/* Clean up pending requests in reqList */
	pthread_mutex_lock(&pstService->req_lock);
	list_for_each_entry_safe(item, tmp, &pstService->reqList, node) {
		if (item->isSync) {
			/*
			* Wake up the blocked sync caller.
			* Hold item->lock for proper cond_signal semantics,
			* then re-init the list node so the caller's list_del
			* on an empty head is harmless.
			*/
			pthread_mutex_lock(&item->lock);
			pthread_cond_signal(&item->cond);
			pthread_mutex_unlock(&item->lock);
			list_del(&item->node);
			INIT_LIST_HEAD(&item->node);
		} else {
			list_del(&item->node);
			free(item);
		}
	}
	pthread_mutex_unlock(&pstService->req_lock);

	CVI_TRACE_IPCMSG(CVI_DBG_DEBUG, "s32Id:%d, disconnected.\n", s32Id);

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

	if (!pstService->isConnected) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Service not connected, s32Id(%d).\n", s32Id);
		return CVI_IPCMSG_ENOTCONN;
	}

	//send msg
	if (_send_msg(pstRequest, pstService)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "_send_msg failed.");
		return CVI_FAILURE;
	}

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

	if (!pstService->isConnected) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Service not connected, s32Id(%d).\n", s32Id);
		return CVI_IPCMSG_ENOTCONN;
	}

	if (pfnRespHandle) {
		item = (struct msg_item *)calloc(sizeof(*item), 1);
		if (!item) {
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%zu)\n", sizeof(*item));
			return CVI_FAILURE;
		}

		item->u64Id = pstMsg->u64Id;
		item->isSync = CVI_FALSE;
		item->pfnRespHandle = pfnRespHandle;
		item->u64PtsUs = _GetCurUsPTS();

		pthread_mutex_lock(&pstService->req_lock);
		list_add_tail(&item->node, &pstService->reqList);
		pthread_mutex_unlock(&pstService->req_lock);
	}

	//send msg
	if (_send_msg(pstMsg, pstService)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "_send_msg failed.");
		goto err;
	}

	return CVI_SUCCESS;
err:
	if (pfnRespHandle && item) {
		pthread_mutex_lock(&pstService->req_lock);
		list_del(&item->node);
		pthread_mutex_unlock(&pstService->req_lock);
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

	if (!pstService->isConnected) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Service not connected, s32Id(%d).\n", s32Id);
		return CVI_IPCMSG_ENOTCONN;
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
		pthread_condattr_destroy(&condAttr);
		free(item);
		return CVI_FAILURE;
	}
	pthread_condattr_destroy(&condAttr);
	pthread_mutex_init(&item->lock, NULL);
	item->u64Id = pstMsg->u64Id;
	item->isSync = CVI_TRUE;
	item->u64PtsUs = _GetCurUsPTS();

	pthread_mutex_lock(&pstService->req_lock);
	list_add_tail(&item->node, &pstService->reqList);
	pthread_mutex_unlock(&pstService->req_lock);

	while (clock_gettime(CLOCK_MONOTONIC, &abstime) != 0)
		continue;

	pthread_mutex_lock(&item->lock);
	s32TimeoutMs %= 1000;
	abstime.tv_nsec += s32TimeoutMs * 1000000;
	abstime.tv_sec += (timeout_s + abstime.tv_nsec / 1000000000L);
	abstime.tv_nsec = abstime.tv_nsec % 1000000000L;
	//send msg
	if (_send_msg(pstMsg, pstService)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "_send_msg failed.");
		pthread_mutex_unlock(&item->lock);
		goto err;
	}

	s32Ret = pthread_cond_timedwait(&item->cond, &item->lock, &abstime);
	pthread_mutex_unlock(&item->lock);

	if (s32Ret == 0)
		*ppstMsg = item->pstRespMsg;
	else
		s32Ret = CVI_FAILURE;

err:
	pthread_mutex_lock(&pstService->req_lock);
	list_del(&item->node);
	pthread_mutex_unlock(&pstService->req_lock);
	pthread_mutex_destroy(&item->lock);
	pthread_cond_destroy(&item->cond);
	free(item);
	return s32Ret;
}

CVI_VOID CVI_IPCMSG_Run(CVI_S32 s32Id)
{
	struct timespec abstime;
	CVI_S32 s32Timeout = 1; //1s
	struct ipcmsg_service *pstService = _find_service_by_id(s32Id);

	if (!pstService) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Invalid s32Id(%d).\n", s32Id);
		return;
	}
	pstService->isRun = CVI_TRUE;

	while (pstService->isConnected) {
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec += s32Timeout;

		sem_timedwait(&pstService->cbMsgSem, &abstime);

		pthread_mutex_lock(&pstService->cbMsgLock);
		/* Drain all queued callback messages */
		while (!list_empty(&pstService->cbMsgList)) {
			struct msg_queue_item *qitem = list_first_entry(&pstService->cbMsgList,
				struct msg_queue_item, node);
			list_del(&qitem->node);
			pthread_mutex_unlock(&pstService->cbMsgLock);

			_rx_msg_handle(qitem->pstMsg, pstService);
			free(qitem);

			pthread_mutex_lock(&pstService->cbMsgLock);
		}
		pthread_mutex_unlock(&pstService->cbMsgLock);
	}

	pstService->isRun = CVI_FALSE;
}
