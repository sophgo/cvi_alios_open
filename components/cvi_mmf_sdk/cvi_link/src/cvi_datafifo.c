#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <inttypes.h>
#include "cvi_type.h"
#include "list.h"
#include "cvi_datafifo.h"

#include "ipcm_message.h"
#include "cvi_debug.h"
#include "cvi_sys.h"

#if defined(CONFIG_KERNEL_RHINO)
#include <aos/cli.h>
#include "debug/dbg.h"
#endif

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/****
#define CVI_TRACE_DATAFIFO(fmt, ...)  \
		printf("%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
****/

#define CVI_DATAFIFO_CHECK_NULL(p) \
	do { \
		if (!p) { \
			CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "NULL pointer.\n"); \
			return CVI_FAILURE; \
		} \
	} while (0);

#define UPDATE_POINTER(p, len)  \
	(*(p) = (*(p) + 1) == (len) ? 0 : *(p) + 1)

#define GET_RING_BUF_ADDR(pstCtrl, p)  \
	(pstCtrl->pRingBufAddr + (*p) * pstCtrl->u32step)


struct ctrl_buf {
	CVI_U8 u8LockId; //spin lock
	CVI_U64 u64PhyAddr;
	CVI_VOID *pBaseAddr;
#ifdef __arm__
	CVI_VOID *VirAddrPadding1;
#endif
	CVI_VOID *pRingBufAddr;
#ifdef __arm__
	CVI_VOID *VirAddrPadding2;
#endif
	CVI_U32 u32Len; //The number of items
	CVI_U32 u32step;
	CVI_U32 offset_wh;
	CVI_U32 offset_wt;
	CVI_U32 offset_rh;
	CVI_U32 offset_rt;
	CVI_U32 *pu32wh; //write head
#ifdef __arm__
	CVI_VOID *VirAddrPadding3;
#endif
	CVI_U32 *pu32wt; //write tail
#ifdef __arm__
	CVI_VOID *VirAddrPadding4;
#endif
	CVI_U32 *pu32rh; //read head
#ifdef __arm__
	CVI_VOID *VirAddrPadding5;
#endif
	CVI_U32 *pu32rt; //read tail
#ifdef __arm__
	CVI_VOID *VirAddrPadding6;
#endif
};

struct datafifo_context {
	CVI_U64 u64SharePhyAddr;
	CVI_VOID *pSharePVirAddr;
	CVI_BOOL isMaster;
	CVI_DATAFIFO_PARAMS_S stParams;
	CVI_DATAFIFO_RELEASESTREAM_FN_PTR pfnRelease;
	CVI_VOID *pstCtrl;
	struct list_head node;
};


static CVI_BOOL s_abSpinLock[IPCM_DATA_LOCK_NUM];
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t datafifo_list_lock = PTHREAD_MUTEX_INITIALIZER;
LIST_HEAD(datafifo_list);

CVI_U8 _get_spin_lock_id()
{
	CVI_U8 i;

	pthread_mutex_lock(&s_lock);
	for (i = 0; i < IPCM_DATA_LOCK_NUM; i++)
		if (!s_abSpinLock[i])
			break;
	if (i < IPCM_DATA_LOCK_NUM)
		s_abSpinLock[i] = CVI_TRUE;
	pthread_mutex_unlock(&s_lock);
	return i;
}

CVI_VOID _release_spin_lock(CVI_U8 u8LockId)
{
	if (u8LockId >= IPCM_DATA_LOCK_NUM) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "u8LockId error\n");
		return;
	}
	pthread_mutex_lock(&s_lock);
	s_abSpinLock[u8LockId] = CVI_FALSE;
	pthread_mutex_unlock(&s_lock);
}

static CVI_VOID _print_ctrl_buf(struct ctrl_buf *pstCtrl)
{
	CVI_TRACE_DATAFIFO(CVI_DBG_NOTICE, "[wh:%d wt:%d rh:%d rt:%d]\n", *pstCtrl->pu32wh, *pstCtrl->pu32wt,
		*pstCtrl->pu32rh, *pstCtrl->pu32rt);
}

static CVI_VOID _invalid_cache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_SYS_IonInvalidateCache(u64PhyAddr, pVirAddr, u32Len);
}

static CVI_VOID _flush_cache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_SYS_IonFlushCache(u64PhyAddr, pVirAddr, u32Len);
}

static CVI_VOID ctrl_buf_init_writer(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr,
	CVI_U32 u32Len, CVI_U32 u32step, CVI_U8 u8LockId)
{
	struct ctrl_buf *pstCtrl = (struct ctrl_buf *)pVirAddr;

	pstCtrl->u8LockId = u8LockId;
	pstCtrl->u64PhyAddr = u64PhyAddr;
	pstCtrl->pBaseAddr = pVirAddr;
	pstCtrl->pRingBufAddr = pVirAddr + sizeof(struct ctrl_buf);
	pstCtrl->u32Len = u32Len;
	pstCtrl->u32step = u32step;
	pstCtrl->offset_wh = 0;
	pstCtrl->offset_wt = 0;
	pstCtrl->offset_rh = 0;
	pstCtrl->offset_rt = 0;
	pstCtrl->pu32wh = &pstCtrl->offset_wh;
	pstCtrl->pu32wt = &pstCtrl->offset_wt;
	pstCtrl->pu32rh = &pstCtrl->offset_rh;
	pstCtrl->pu32rt = &pstCtrl->offset_rt;
}

static CVI_VOID ctrl_buf_init_reader(CVI_VOID *pShareAddr, CVI_VOID *pstCtrl)
{
	struct ctrl_buf *pstCtrl0 = (struct ctrl_buf *)pShareAddr;
	struct ctrl_buf *pstCtrl1 = (struct ctrl_buf *)pstCtrl;

	pstCtrl1->u8LockId = pstCtrl0->u8LockId;
	pstCtrl1->u64PhyAddr = pstCtrl0->u64PhyAddr;
	pstCtrl1->pBaseAddr = pShareAddr;
	pstCtrl1->pRingBufAddr = pShareAddr + sizeof(struct ctrl_buf);
	pstCtrl1->u32Len = pstCtrl0->u32Len;
	pstCtrl1->u32step = pstCtrl0->u32step;
	pstCtrl1->offset_wh = 0;
	pstCtrl1->offset_wt = 0;
	pstCtrl1->offset_rh = pstCtrl0->offset_rh;
	pstCtrl1->offset_rt = 0;
	pstCtrl1->pu32wh = &pstCtrl0->offset_wh;
	pstCtrl1->pu32wt = &pstCtrl0->offset_wt;
	pstCtrl1->pu32rh = &pstCtrl1->offset_rh;
	pstCtrl1->pu32rt = &pstCtrl0->offset_rh;
}

static CVI_BOOL isInit(struct ctrl_buf *pstCtrl)
{
	if ((pstCtrl->offset_wh == 0) &&
		(pstCtrl->offset_wt == 0) &&
		(pstCtrl->offset_rh == 0) &&
		(pstCtrl->offset_rt == 0))
		return CVI_TRUE;
	return CVI_FALSE;
}

static CVI_U32 get_avail_write_len(struct ctrl_buf *pstCtrl)
{
	CVI_U32 u32Len;

	if (isInit(pstCtrl))
		return pstCtrl->u32Len - 1;

	if (*pstCtrl->pu32wh >= *pstCtrl->pu32rt)
		u32Len = pstCtrl->u32Len - *pstCtrl->pu32wh + *pstCtrl->pu32rt - 1;
	else
		u32Len = *pstCtrl->pu32rt - *pstCtrl->pu32wh - 1;
	return u32Len;
}

static CVI_U32 get_avail_read_len(struct ctrl_buf *pstCtrl)
{
	CVI_U32 u32Len;

	if (*pstCtrl->pu32wt >= *pstCtrl->pu32rh)
		u32Len = *pstCtrl->pu32wt - *pstCtrl->pu32rh;
	else
		u32Len = pstCtrl->u32Len - *pstCtrl->pu32rh + *pstCtrl->pu32wt;
	return u32Len;
}

static CVI_U32 get_release_len(struct ctrl_buf *pstCtrl)
{
	CVI_U32 u32Len;

	if (*pstCtrl->pu32rh >= *pstCtrl->pu32rt)
		u32Len = *pstCtrl->pu32rh - *pstCtrl->pu32rt;
	else
		u32Len = pstCtrl->u32Len - *pstCtrl->pu32rt + *pstCtrl->pu32rh;
	return u32Len;
}

static CVI_VOID ctrl_buf_write(struct ctrl_buf *pstCtrl, CVI_VOID *pData)
{
	CVI_VOID *p = GET_RING_BUF_ADDR(pstCtrl, pstCtrl->pu32wh);

	memcpy(p, pData, pstCtrl->u32step);

	//update write-head pointer
	UPDATE_POINTER(pstCtrl->pu32wh, pstCtrl->u32Len);
	_flush_cache(pstCtrl->u64PhyAddr + (p - pstCtrl->pBaseAddr), p, pstCtrl->u32step);
}

static CVI_VOID ctrl_buf_read(struct ctrl_buf *pstCtrl, CVI_VOID **pData)
{
	CVI_VOID *p = GET_RING_BUF_ADDR(pstCtrl, pstCtrl->pu32rh);

	if (pData)
		*pData = p;
	//update read-head pointer
	UPDATE_POINTER(pstCtrl->pu32rh, pstCtrl->u32Len);
	_invalid_cache(pstCtrl->u64PhyAddr + (p - pstCtrl->pBaseAddr), p, pstCtrl->u32step);
}

static CVI_VOID release_buf(struct datafifo_context *ctx)
{
	CVI_VOID *p;
	CVI_U32 u32Releaselen;
	struct ctrl_buf *pstCtrl = (struct ctrl_buf *)ctx->pstCtrl;

	while (1) {
		_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		u32Releaselen = get_release_len(pstCtrl);
		if (!u32Releaselen)
			break;
		p = GET_RING_BUF_ADDR(pstCtrl, pstCtrl->pu32rt);
		if (ctx->pfnRelease)
			ctx->pfnRelease(p);

		if (ipcm_msg_data_lock(pstCtrl->u8LockId))
			break;
		_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		UPDATE_POINTER(pstCtrl->pu32rt, pstCtrl->u32Len);
		_flush_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		ipcm_msg_data_unlock(pstCtrl->u8LockId);
	}
}


CVI_S32 CVI_DATAFIFO_Open(CVI_DATAFIFO_HANDLE *Handle, CVI_DATAFIFO_PARAMS_S *pstParams)
{
	CVI_U64 u64PhyAddr;
	CVI_VOID *pVirAddr;
	CVI_U8 u8LockId;
	struct datafifo_context *ctx = NULL;
	CVI_U32 u32CtxSize = sizeof(struct datafifo_context);
	CVI_U32 u32ShareSize = pstParams->u32CacheLineSize * pstParams->u32EntriesNum + sizeof(struct ctrl_buf);

	if (!Handle) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "Handle is NULL\n");
		return CVI_FAILURE;
	}

	if (!pstParams) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "pstParams is NULL\n");
		return CVI_FAILURE;
	}

	ctx = (struct datafifo_context *)calloc(u32CtxSize, 1);
	if (!ctx) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "calloc failed\n");
		return CVI_FAILURE;
	}
	u8LockId = _get_spin_lock_id();
	if (u8LockId >= IPCM_DATA_LOCK_NUM) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "_get_spin_lock_id failed\n");
		free(ctx);
		return CVI_FAILURE;
	}

	if (CVI_SYS_IonAlloc_Cached(&u64PhyAddr, &pVirAddr, "datafifo", u32ShareSize)) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "CVI_SYS_IonAlloc_Cached failed\n");
		free(ctx);
		_release_spin_lock(u8LockId);
		return CVI_FAILURE;
	}
	memset(pVirAddr, 0, u32ShareSize);

	ctx->stParams = *pstParams;
	ctx->pstCtrl = pVirAddr;
	ctx->u64SharePhyAddr = u64PhyAddr;
	ctx->pSharePVirAddr = pVirAddr;
	ctx->isMaster = CVI_TRUE;
	ctx->pfnRelease = NULL;

	ctrl_buf_init_writer(u64PhyAddr, pVirAddr, pstParams->u32EntriesNum,
		pstParams->u32CacheLineSize, u8LockId);

	_flush_cache(u64PhyAddr, pVirAddr, u32ShareSize);

	//add list
	pthread_mutex_lock(&datafifo_list_lock);
	list_add_tail(&ctx->node, &datafifo_list);
	pthread_mutex_unlock(&datafifo_list_lock);

	*Handle = (CVI_DATAFIFO_HANDLE)ctx;

	CVI_TRACE_DATAFIFO(CVI_DBG_DEBUG, "CVI_DATAFIFO_Open ctx Handle:0x%lx u64PhyAddr:0x%llx, spin lock id:%d stack:%p\n", *Handle, (unsigned long long)u64PhyAddr, u8LockId, __builtin_return_address(0));

	return CVI_SUCCESS;
}

CVI_S32 CVI_DATAFIFO_OpenByAddr(CVI_DATAFIFO_HANDLE *Handle,
			CVI_DATAFIFO_PARAMS_S *pstParams, CVI_U64 u64PhyAddr)
{
	CVI_VOID *pShareAddr = NULL;
	struct datafifo_context *ctx = NULL;
	CVI_U32 u32CtxSize = sizeof(struct datafifo_context);
	CVI_U32 u32CtrlBufSize = sizeof(struct ctrl_buf);
	CVI_U32 u32ShareSize = pstParams->u32CacheLineSize * pstParams->u32EntriesNum + sizeof(struct ctrl_buf);
	struct ctrl_buf *pstCtrl;

	pShareAddr = CVI_SYS_MmapCache(u64PhyAddr, u32ShareSize);
	if (!pShareAddr) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "CVI_SYS_MmapCache failed\n");
		return CVI_FAILURE;
	}

	pstCtrl = (struct ctrl_buf *)pShareAddr;
	_invalid_cache(u64PhyAddr, pShareAddr, u32ShareSize);

	if ((pstCtrl->u32Len != pstParams->u32EntriesNum) ||
		(pstCtrl->u32step != pstParams->u32CacheLineSize)) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "The parameters on both sides are inconsistent.\n");
		return CVI_FAILURE;
	}

	ctx = (struct datafifo_context *)calloc(u32CtxSize, 1);
	if (!ctx) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "calloc failed\n");
		//unmap
		return CVI_FAILURE;
	}

	ctx->pstCtrl = calloc(u32CtrlBufSize, 1);
	if (!ctx->pstCtrl) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "calloc failed\n");
		free(ctx);
		//unmap
		return CVI_FAILURE;
	}

	ctx->u64SharePhyAddr = u64PhyAddr;
	ctx->pSharePVirAddr = pShareAddr;
	ctx->stParams = *pstParams;
	ctx->isMaster = CVI_FALSE;
	ctx->pfnRelease = NULL;

	ctrl_buf_init_reader(pShareAddr, ctx->pstCtrl);

	//add list
	pthread_mutex_lock(&datafifo_list_lock);
	list_add_tail(&ctx->node, &datafifo_list);
	pthread_mutex_unlock(&datafifo_list_lock);

	*Handle = (CVI_DATAFIFO_HANDLE)ctx;

	CVI_TRACE_DATAFIFO(CVI_DBG_DEBUG, "CVI_DATAFIFO_OpenByAddr u64PhyAddr:0x%llx, spin lock id:%d\n",
		(unsigned long long)u64PhyAddr, ((struct ctrl_buf *)ctx->pstCtrl)->u8LockId);

	return CVI_SUCCESS;
}

CVI_S32 CVI_DATAFIFO_Close(CVI_DATAFIFO_HANDLE Handle)
{
	struct datafifo_context *ctx;
	struct datafifo_context *item, *tmp;
	CVI_U32 u32ShareSize;
	struct ctrl_buf *pstCtrl;

	if (!Handle) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "NULL pointer, Invalid Handle.\n");
		return CVI_FAILURE;
	}
	ctx = (struct datafifo_context *)Handle;

	//delete list
	pthread_mutex_lock(&datafifo_list_lock);
	list_for_each_entry_safe(item, tmp, &datafifo_list, node) {
		if (item == ctx) {
			list_del(&item->node);
			break;
		}
	}
	pthread_mutex_unlock(&datafifo_list_lock);

	if (ctx->isMaster) {
		pstCtrl = (struct ctrl_buf *)ctx->pstCtrl;
		_release_spin_lock(pstCtrl->u8LockId);
		CVI_SYS_IonFree(ctx->u64SharePhyAddr, ctx->pSharePVirAddr);
		free(ctx); //ctx free
	} else {
		free(ctx->pstCtrl); //ctrl buf free
		u32ShareSize = ctx->stParams.u32CacheLineSize * ctx->stParams.u32EntriesNum +
						sizeof(struct ctrl_buf);
		CVI_SYS_Munmap(ctx->pSharePVirAddr, u32ShareSize);
		free(ctx); //ctx free
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DATAFIFO_Read(CVI_DATAFIFO_HANDLE Handle, CVI_VOID **ppData)
{
	struct datafifo_context *ctx;
	struct ctrl_buf *pstCtrl;

	if (!Handle) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "NULL pointer, Invalid Handle.\n");
		return CVI_FAILURE;
	}
	ctx = (struct datafifo_context *)Handle;
	pstCtrl = (struct ctrl_buf *)ctx->pstCtrl;

	if (ctx->stParams.enOpenMode != DATAFIFO_READER) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "It can only be read by the reader.\n");
		return CVI_FAILURE;
	}

	if (ipcm_msg_data_lock(pstCtrl->u8LockId))
		return CVI_FAILURE;

	_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
	if (get_avail_read_len(pstCtrl)) {
		ctrl_buf_read(pstCtrl, ppData);
	} else {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "datafifo is empty.\n");
		ipcm_msg_data_unlock(pstCtrl->u8LockId);
		return CVI_FAILURE;
	}
	_flush_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
	if (ipcm_msg_data_unlock(pstCtrl->u8LockId))
		return CVI_FAILURE;

	return CVI_SUCCESS;
}

CVI_S32 CVI_DATAFIFO_Write(CVI_DATAFIFO_HANDLE Handle, CVI_VOID *pData)
{
	struct datafifo_context *ctx;
	struct ctrl_buf *pstCtrl;

	if (!Handle) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "NULL pointer, Invalid Handle.\n");
		return CVI_FAILURE;
	}
	ctx = (struct datafifo_context *)Handle;
	pstCtrl = (struct ctrl_buf *)ctx->pstCtrl;

	if (ctx->stParams.enOpenMode != DATAFIFO_WRITER) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "It can only be written by the writer.\n");
		return CVI_FAILURE;
	}
	//pData = NULL, release data
	if (pData == NULL) {
		if (ctx->pfnRelease) {
			release_buf(ctx);
			return CVI_SUCCESS;
		} else {
			CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "release function is NULL\n");
			return CVI_FAILURE;
		}
	}

	if (ipcm_msg_data_lock(pstCtrl->u8LockId))
		return CVI_FAILURE;
	_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
	if (get_avail_write_len(pstCtrl)) {
		ctrl_buf_write(pstCtrl, pData);
	} else {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "datafifo is full.\n");
		ipcm_msg_data_unlock(pstCtrl->u8LockId);
		return CVI_FAILURE;
	}
	_flush_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
	if (ipcm_msg_data_unlock(pstCtrl->u8LockId))
		return CVI_FAILURE;

	return CVI_SUCCESS;
}

CVI_S32 CVI_DATAFIFO_CMD(CVI_DATAFIFO_HANDLE Handle, CVI_DATAFIFO_CMD_E enCMD, CVI_VOID *pArg)
{
	struct datafifo_context *ctx;
	struct ctrl_buf *pstCtrl;

	if (!Handle) {
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "NULL pointer, Invalid Handle.\n");
		return CVI_FAILURE;
	}
	ctx = (struct datafifo_context *)Handle;
	pstCtrl = (struct ctrl_buf *)ctx->pstCtrl;

	switch (enCMD) {
	case DATAFIFO_CMD_GET_PHY_ADDR:
		CVI_DATAFIFO_CHECK_NULL(pArg);
		*((CVI_U64 *)pArg) = ctx->u64SharePhyAddr;
		break;
	case DATAFIFO_CMD_READ_DONE:
		if (ipcm_msg_data_lock(pstCtrl->u8LockId))
			return CVI_FAILURE;
		_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		UPDATE_POINTER(pstCtrl->pu32rt, pstCtrl->u32Len);
		_flush_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		ipcm_msg_data_unlock(pstCtrl->u8LockId);
		//_print_ctrl_buf(pstCtrl);
		break;
	case DATAFIFO_CMD_WRITE_DONE:
		if (ipcm_msg_data_lock(pstCtrl->u8LockId))
			return CVI_FAILURE;
		_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		UPDATE_POINTER(pstCtrl->pu32wt, pstCtrl->u32Len);
		_flush_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		ipcm_msg_data_unlock(pstCtrl->u8LockId);
		//_print_ctrl_buf(pstCtrl);
		break;
	case DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK:
		ctx->pfnRelease = (CVI_DATAFIFO_RELEASESTREAM_FN_PTR)pArg;
		break;
	case DATAFIFO_CMD_GET_AVAIL_WRITE_LEN:
		CVI_DATAFIFO_CHECK_NULL(pArg);
		_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		*((CVI_U32 *)pArg) = get_avail_write_len(pstCtrl);
		//_print_ctrl_buf(pstCtrl);
		break;
	case DATAFIFO_CMD_GET_AVAIL_READ_LEN:
		CVI_DATAFIFO_CHECK_NULL(pArg);
		_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		*((CVI_U32 *)pArg) = get_avail_read_len(pstCtrl);
		//_print_ctrl_buf(pstCtrl);
		break;
	case DATAFIFO_CMD_SHOW_POINTER:
		_invalid_cache(ctx->u64SharePhyAddr, ctx->pSharePVirAddr, sizeof(struct ctrl_buf));
		_print_ctrl_buf(pstCtrl);
		break;
	default:
		CVI_TRACE_DATAFIFO(CVI_DBG_ERR, "cmd error.\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

#if defined(CONFIG_KERNEL_RHINO)
static void datafifo_status_show(int32_t argc, char **argv)
{
	struct datafifo_context *item;
	struct ctrl_buf *pstCtrl;

	printf("-------------------------------datafifo info-------------------------------\n");
	printf("%14s%14s%14s%6s%8s%10s%10s%6s%6s%6s%6s\n",
		"Handle", "PhyAddr", "hw_lock_id", "W/R", "Master",
		"ItemSize", "ItemNum", "pwh", "pwt", "prh", "prt");
	pthread_mutex_lock(&datafifo_list_lock);
	list_for_each_entry(item, &datafifo_list, node) {
		pstCtrl = (struct ctrl_buf *)item->pstCtrl;
		_invalid_cache(item->u64SharePhyAddr, item->pSharePVirAddr, sizeof(struct ctrl_buf));
		printf("%14p%#14lx%14d%6s%8s%10d%10d%6d%6d%6d%6d\n",
			(void *)item,
			item->u64SharePhyAddr,
			pstCtrl->u8LockId,
			(item->stParams.enOpenMode == DATAFIFO_READER) ? "R" : "W",
			item->isMaster ? "Y" : "N",
			pstCtrl->u32step,
			pstCtrl->u32Len,
			*pstCtrl->pu32wh,
			*pstCtrl->pu32wt,
			*pstCtrl->pu32rh,
			*pstCtrl->pu32rt);
	}
	pthread_mutex_unlock(&datafifo_list_lock);
}

ALIOS_CLI_CMD_REGISTER(datafifo_status_show, datafifo_status, datafifo status);
#endif

