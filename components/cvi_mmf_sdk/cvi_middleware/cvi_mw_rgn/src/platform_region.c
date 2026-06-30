#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <cvi_math.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <semaphore.h>

#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_region.h"
#include "driver_rgn.h"
#include "rgn_uapi.h"
#include "cvi_comm_osdc.h"
#include "cvi_osdc.h"

#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

#if CONFIG_OSDC_DUAL_906
struct osdc_work {
    osal_task *thread;
    pthread_mutex_t lock;
    sem_t sem_work;
    sem_t sem_done;
    bool thread_running;
    int create_num;

    OSDC_Canvas_Attr_S osdc_canvas;
    OSDC_DRAW_OBJ_S *obj_vec;
    CVI_U32 obj_num;
    CVI_U8 *virt_addr;
    CVI_S32 ion_len;
    CVI_U32 *bs_size;
    CVI_S32 handle;
    CVI_S32 status;
    struct cvi_rgn_bitmap *pstBitmaps;
};

static struct osdc_work g_osdc_work;
static pthread_mutex_t g_init_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_thread_initialized = false;

struct rgn_canvas {
	STAILQ_ENTRY(rgn_canvas) stailq;
	RGN_HANDLE Handle;
	CVI_U64 u64PhyAddr;
	CVI_U8 *pu8VirtAddr;
	CVI_U32 u32Size;
};

struct cvi_rgn_bitmap {
	CVI_VOID *pBitmapVAddr;
	CVI_U32 u32BitmapSize;
};

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t canvas_q_lock = PTHREAD_MUTEX_INITIALIZER;
STAILQ_HEAD(rgn_canvas_q, rgn_canvas) canvas_q;

static void rgn_init(void)
{
	STAILQ_INIT(&canvas_q);
}

static inline CVI_S32 rgn_get_bytesperline(PIXEL_FORMAT_E enPixelFormat, CVI_U32 width, CVI_U32 *bytesperline)
{
	switch (enPixelFormat) {
	case PIXEL_FORMAT_ARGB_8888:
		*bytesperline = width << 2;
		break;
	case PIXEL_FORMAT_ARGB_4444:
	case PIXEL_FORMAT_ARGB_1555:
		*bytesperline = width << 1;
		break;
	case PIXEL_FORMAT_8BIT_MODE:
		*bytesperline = width;
		break;
	case PIXEL_FORMAT_4BIT_MODE:
		*bytesperline = width >> 1;
		break;
	default:
		CVI_TRACE_RGN(CVI_DBG_ERR, "not supported pxl-fmt(%d).\n", enPixelFormat);
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static int osdc_draw_thread(void *data)
{
    struct osdc_work *work = (struct osdc_work *)data;
    int ret;

    while (1) {
        ret = sem_wait(&work->sem_work);
        if (ret != 0) {
            if (errno == EINTR) continue;
            CVI_TRACE_RGN(CVI_DBG_ERR, "Sem wait failed: %d\n", errno);
            break;
        }

	if (!work->thread_running) {
		osal_kthread_should_stop();
		break;
	}

	work->status = CVI_OSDC_DrawCmprCanvas(&work->osdc_canvas,
						work->obj_vec,
						work->obj_num,
						work->virt_addr,
						work->ion_len,
						work->bs_size);
	sem_post(&work->sem_done);
    }

    CVI_TRACE_RGN(CVI_DBG_INFO, "OSDC draw thread exit\n");
    return 0;
}

static CVI_S32 init_osdc_work_thread(void)
{
    struct osdc_work *work = &g_osdc_work;
    int ret;

    work->create_num = 0;

    ret = sem_init(&work->sem_work, 0, 0);
    if (ret != 0) {
        CVI_TRACE_RGN(CVI_DBG_ERR, "Failed to init work sem: %d\n", ret);
        return CVI_ERR_RGN_NOMEM;
    }

    ret = sem_init(&work->sem_done, 0, 0);
    if (ret != 0) {
        CVI_TRACE_RGN(CVI_DBG_ERR, "Failed to init done sem: %d\n", ret);
        sem_destroy(&work->sem_work);
        return CVI_ERR_RGN_NOMEM;
    }

    ret = pthread_mutex_init(&work->lock, NULL);
    if (ret != 0) {
        CVI_TRACE_RGN(CVI_DBG_ERR, "Failed to init mutex: %d\n", ret);
        sem_destroy(&work->sem_work);
        sem_destroy(&work->sem_done);
        return CVI_ERR_RGN_NOMEM;
    }

    work->thread_running = true;

    work->thread = osal_kthread_create(osdc_draw_thread, (void *)work, "osdc_draw", 4096);
    if (!work->thread) {
        CVI_TRACE_RGN(CVI_DBG_ERR, "Failed to create thread: %d\n", ret);
        pthread_mutex_destroy(&work->lock);
        sem_destroy(&work->sem_work);
        sem_destroy(&work->sem_done);
        return CVI_ERR_RGN_NOMEM;
    }
    osal_kthread_set_priority(work->thread, 28);

    CVI_TRACE_RGN(CVI_DBG_INFO, "OSDC draw thread created\n");
    return CVI_SUCCESS;
}

static CVI_S32 deinit_osdc_work_thread(void)
{
    struct osdc_work *work = &g_osdc_work;

    if (work->thread_running) {
        work->thread_running = false;
        sem_post(&work->sem_work);

	osal_kthread_destroy(work->thread, true);
	pthread_mutex_destroy(&work->lock);

        sem_destroy(&work->sem_work);
        sem_destroy(&work->sem_done);

        CVI_TRACE_RGN(CVI_DBG_INFO, "OSDC draw thread stopped\n");
    }
    return CVI_SUCCESS;
}
#endif

static inline CVI_S32 SDK_CTRL_GET_CFG(void *_cfg1, void *_cfg2, int _ioctl, CVI_U32 _handle)
{
	struct rgn_ext_control ec1;
	int ret;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = RGN_IOCTL_SDK_CTRL;
	ec1.sdk_id = _ioctl;
	ec1.handle = _handle;
	ec1.ptr1 = _cfg1;
	ec1.ptr2 = _cfg2;

	ret = driver_rgn_ioctl(RGN_IOC_G_CTRL, (unsigned long)&ec1);
	if (ret < 0) {
		fprintf(stderr, "RGN_SDK_IOC_G_CTRL(%d-%d) - %s NG, %s\n",
			ec1.id, ec1.sdk_id, __func__, strerror(errno));
		return ret;
	}
	return 0;
}

static inline CVI_S32 SDK_CTRL_SET_CFG(void *_cfg1, void *_cfg2, int _ioctl, CVI_U32 _handle)
{
	struct rgn_ext_control ec1;
	int ret;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = RGN_IOCTL_SDK_CTRL;
	ec1.sdk_id = _ioctl;
	ec1.handle = _handle;
	ec1.ptr1 = _cfg1;
	ec1.ptr2 = _cfg2;

	ret = driver_rgn_ioctl(RGN_IOC_S_CTRL, (unsigned long)&ec1);
	if (ret < 0) {
		fprintf(stderr, "RGN_SDK_IOC_S_CTRL(%d-%d) - %s NG, %s\n",
			ec1.id, ec1.sdk_id, __func__, strerror(errno));
		return ret;
	}
	return 0;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 platform_rgn_create(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);

	// Driver control
	s32Ret = SDK_CTRL_SET_CFG((void *)pstRegion, NULL, RGN_SDK_CREATE, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Create RGN fail.\n");
		return s32Ret;
	}
#if CONFIG_OSDC_DUAL_906
	pthread_mutex_lock(&canvas_q_lock);
	pthread_once(&once, rgn_init);
	pthread_mutex_unlock(&canvas_q_lock);

	pthread_mutex_lock(&g_init_mutex);
	if (g_thread_initialized == false) {
		s32Ret = init_osdc_work_thread();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Failed to init OSDC thread\n");
			return s32Ret;
		}
		g_thread_initialized = true;
	}
	g_osdc_work.create_num++;

	pthread_mutex_unlock(&g_init_mutex);
#endif
	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_destroy(RGN_HANDLE Handle)
{
	CVI_S32 s32Ret;

	// Driver control
	s32Ret = SDK_CTRL_SET_CFG(NULL, NULL, RGN_SDK_DESTORY, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Destroy RGN fail.\n");
		return s32Ret;
	}
#if CONFIG_OSDC_DUAL_906
	pthread_mutex_lock(&g_init_mutex);
	if (--g_osdc_work.create_num == 0) {
		s32Ret = deinit_osdc_work_thread();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Failed to init OSDC thread\n");
			return s32Ret;
		}
		g_thread_initialized = false;
	}
	pthread_mutex_unlock(&g_init_mutex);
#endif
	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_getattr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);

	// Driver control
	s32Ret = SDK_CTRL_GET_CFG((void *)pstRegion, NULL, RGN_SDK_GET_ATTR, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setattr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);
	// Driver control
	s32Ret = SDK_CTRL_SET_CFG((void *)pstRegion, NULL, RGN_SDK_SET_ATTR, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setbitmap(RGN_HANDLE Handle, const BITMAP_S *pstBitmap)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstBitmap);
	// Driver control
	s32Ret = SDK_CTRL_SET_CFG((void *)pstBitmap, NULL, RGN_SDK_SET_BIT_MAP, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN Bitmap fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_attachtochn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	// Driver control
	s32Ret = SDK_CTRL_SET_CFG((void *)pstChn, (void *)pstChnAttr, RGN_SDK_ATTACH_TO_CHN, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Attach RGN to channel fail.\n");
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_detachfromchn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);

	// Driver control
	s32Ret = SDK_CTRL_SET_CFG((void *)pstChn, NULL, RGN_SDK_DETACH_FROM_CHN, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Detach RGN from channel fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setdisplayattr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	// Driver control
	s32Ret = SDK_CTRL_SET_CFG((void *)pstChn, (void *)pstChnAttr, RGN_SDK_SET_DISPLAY_ATTR, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set display RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_getdisplayattr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	// Driver control
	s32Ret = SDK_CTRL_GET_CFG((void *)pstChn, pstChnAttr, RGN_SDK_GET_DISPLAY_ATTR, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get display RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_getcanvasinfo(RGN_HANDLE Handle, RGN_CANVAS_INFO_S *pstCanvasInfo)
{
	CVI_S32 s32Ret;
#if CONFIG_OSDC_DUAL_906
	CVI_S32 s32IonLen;
	RGN_ATTR_S stRegion;
	struct rgn_canvas *canvas;
	CVI_U32 bytesperline = 0;
	CVI_U32 u32Stride = 0;
#endif

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstCanvasInfo);

	// Driver control
	s32Ret = SDK_CTRL_GET_CFG((void *)pstCanvasInfo, NULL, RGN_SDK_GET_CANVAS_INFO, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN canvas information fail.\n");
		return s32Ret;
	}

#if CONFIG_OSDC_DUAL_906
	s32Ret = SDK_CTRL_GET_CFG(&stRegion, NULL, RGN_SDK_GET_ATTR, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN attributes fail.\n");
		return s32Ret;
	}

	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_SW) {
		s32IonLen = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	} else if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		s32IonLen = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
	} else {
		PIXEL_FORMAT_E pixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		SIZE_S rgnSize = stRegion.unAttr.stOverlay.stSize;
		rgn_get_bytesperline(pixelFormat, rgnSize.u32Width, &bytesperline);
		u32Stride = ALIGN(bytesperline, 32);
		s32IonLen = u32Stride * rgnSize.u32Height;
	}

	canvas = calloc(sizeof(struct rgn_canvas), 1);
	if (!canvas) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "malloc failed.\n");
		return CVI_ERR_RGN_NOMEM;
	}

	pthread_mutex_lock(&canvas_q_lock);
	canvas->u64PhyAddr = pstCanvasInfo->u64PhyAddr;
	canvas->pu8VirtAddr = (void *)(uintptr_t)pstCanvasInfo->u64PhyAddr;
	canvas->u32Size = s32IonLen;
	canvas->Handle = Handle;
	STAILQ_INSERT_TAIL(&canvas_q, canvas, stailq);
	pthread_mutex_unlock(&canvas_q_lock);
#endif
	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_updatecanvas(RGN_HANDLE Handle)
{
	CVI_S32 s32Ret;
#if CONFIG_OSDC_DUAL_906
	CVI_S32 s32IonLen;
	RGN_ATTR_S stRegion;
	struct rgn_canvas *canvas;
	struct cvi_rgn_bitmap *pstBitmaps;
	CVI_U32 bytesperline = 0, u32Stride = 0, i = 0, j = 0, u32Bpp = 0;
	struct osdc_work *work = &g_osdc_work;
	struct timespec ts;

	s32Ret = SDK_CTRL_GET_CFG(&stRegion, NULL, RGN_SDK_GET_ATTR, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN attributes fail.\n");
		return s32Ret;
	}

	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_SW) {
		s32IonLen = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	} else if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		s32IonLen = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
	} else {
		PIXEL_FORMAT_E pixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		SIZE_S rgnSize = stRegion.unAttr.stOverlay.stSize;
		rgn_get_bytesperline(pixelFormat, rgnSize.u32Width, &bytesperline);
		u32Stride = ALIGN(bytesperline, 32);
		s32IonLen = u32Stride * rgnSize.u32Height;
	}

	pthread_mutex_lock(&canvas_q_lock);
	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(canvas, &canvas_q, stailq) {
			if (canvas->Handle == Handle) {
				break;
			}
		}
	} else {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No corresponding Handle(%d) found.\n", Handle);
		pthread_mutex_unlock(&canvas_q_lock);
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
	pthread_mutex_unlock(&canvas_q_lock);

	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr = NULL;
		RGN_CMPR_OBJ_ATTR_S *pstObjAttr = NULL;
		CVI_S32 status;
		OSDC_Canvas_Attr_S osdc_canvas;
		OSDC_DRAW_OBJ_S *obj_vec;
		CVI_U32 bs_size;

		//cmpr canvas info is stored in ion which needs to be flush before reading
		CVI_SYS_IonInvalidateCache(canvas->u64PhyAddr,
			canvas->pu8VirtAddr, canvas->u32Size);
		pstCanvasCmprAttr = (RGN_CANVAS_CMPR_ATTR_S *)canvas->pu8VirtAddr;
		pstObjAttr = (RGN_CMPR_OBJ_ATTR_S *)(canvas->pu8VirtAddr +
			sizeof(RGN_CANVAS_CMPR_ATTR_S));

		for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
			if (pstObjAttr[i].enObjType == RGN_CMPR_LINE) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG, "start(%d %d) end(%d %d) Thick(%d) Color(0x%x)\n",
					pstObjAttr[i].stLine.stPointStart.s32X,
					pstObjAttr[i].stLine.stPointStart.s32Y,
					pstObjAttr[i].stLine.stPointEnd.s32X,
					pstObjAttr[i].stLine.stPointEnd.s32Y,
					pstObjAttr[i].stLine.u32Thick,
					pstObjAttr[i].stLine.u32Color);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_RECT) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG,
					"xywh(%d %d %d %d) Thick(%d) Color(0x%x) is_fill(%d)\n",
					pstObjAttr[i].stRgnRect.stRect.s32X,
					pstObjAttr[i].stRgnRect.stRect.s32Y,
					pstObjAttr[i].stRgnRect.stRect.u32Width,
					pstObjAttr[i].stRgnRect.stRect.u32Height,
					pstObjAttr[i].stRgnRect.u32Thick,
					pstObjAttr[i].stRgnRect.u32Color,
					pstObjAttr[i].stRgnRect.u32IsFill);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_BIT_MAP) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG, "xywh(%d %d %d %d) u64BitmapPAddr(%"PRIx64")\n",
					pstObjAttr[i].stBitmap.stRect.s32X,
					pstObjAttr[i].stBitmap.stRect.s32Y,
					pstObjAttr[i].stBitmap.stRect.u32Width,
					pstObjAttr[i].stBitmap.stRect.u32Height,
					pstObjAttr[i].stBitmap.u64BitmapPAddr);
			}
		}

		osdc_canvas.width = stRegion.unAttr.stOverlay.stSize.u32Width;
		osdc_canvas.height = stRegion.unAttr.stOverlay.stSize.u32Height;
		osdc_canvas.bg_color_code = stRegion.unAttr.stOverlay.u32BgColor;
		switch (stRegion.unAttr.stOverlay.enPixelFormat) {
		case PIXEL_FORMAT_ARGB_8888:
			osdc_canvas.format = OSD_ARGB8888;
			u32Bpp = 4;
			break;

		case PIXEL_FORMAT_ARGB_4444:
			osdc_canvas.format = OSD_ARGB4444;
			u32Bpp = 2;
			break;

		case PIXEL_FORMAT_ARGB_1555:
			osdc_canvas.format = OSD_ARGB1555;
			u32Bpp = 2;
			break;

		case PIXEL_FORMAT_8BIT_MODE:
			osdc_canvas.format = OSD_LUT8;
			u32Bpp = 1;
			break;

		case PIXEL_FORMAT_4BIT_MODE:
			osdc_canvas.format = OSD_LUT4;
			u32Bpp = 0;
			break;

		default:
			osdc_canvas.format = OSD_ARGB1555;
			u32Bpp = 2;
			break;
		}

		obj_vec = calloc(sizeof(OSDC_DRAW_OBJ_S) * pstCanvasCmprAttr->u32ObjNum, 1);
		if (!obj_vec) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "calloc size (%zu) failed!\n",
							sizeof(OSDC_DRAW_OBJ_S) * pstCanvasCmprAttr->u32ObjNum);
			return CVI_ERR_RGN_NOBUF;
		}
		pstBitmaps = (struct cvi_rgn_bitmap *)calloc(pstCanvasCmprAttr->u32ObjNum,
						sizeof(struct cvi_rgn_bitmap));
		if (!pstBitmaps) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "calloc size (%zu) failed!\n",
							pstCanvasCmprAttr->u32ObjNum * sizeof(struct cvi_rgn_bitmap));
			free(obj_vec);
			return CVI_ERR_RGN_NOBUF;
		}

		for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
			if (pstObjAttr[i].enObjType == RGN_CMPR_LINE) {
				CVI_OSDC_SetLineObjAttr(&osdc_canvas, &obj_vec[i],
				pstObjAttr[i].stLine.u32Color,
					pstObjAttr[i].stLine.stPointStart.s32X,
					pstObjAttr[i].stLine.stPointStart.s32Y,
					pstObjAttr[i].stLine.stPointEnd.s32X,
					pstObjAttr[i].stLine.stPointEnd.s32Y,
					pstObjAttr[i].stLine.u32Thick);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_RECT) {
				CVI_OSDC_SetRectObjAttr(&osdc_canvas, &obj_vec[i],
					pstObjAttr[i].stRgnRect.u32Color,
					pstObjAttr[i].stRgnRect.stRect.s32X,
					pstObjAttr[i].stRgnRect.stRect.s32Y,
					pstObjAttr[i].stRgnRect.stRect.u32Width,
					pstObjAttr[i].stRgnRect.stRect.u32Height,
					pstObjAttr[i].stRgnRect.u32IsFill,
					pstObjAttr[i].stRgnRect.u32Thick);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_BIT_MAP) {
				if (u32Bpp == 0) {
					// 4bit mode: 2 pixels per byte
					pstBitmaps[j].u32BitmapSize = (pstObjAttr[i].stBitmap.stRect.u32Width *
									pstObjAttr[i].stBitmap.stRect.u32Height + 1) / 2;
				} else {
					pstBitmaps[j].u32BitmapSize = pstObjAttr[i].stBitmap.stRect.u32Width *
									pstObjAttr[i].stBitmap.stRect.u32Height * u32Bpp;
				}
				pstBitmaps[j].pBitmapVAddr = CVI_SYS_MmapCache(pstObjAttr[i].stBitmap.u64BitmapPAddr,
								pstBitmaps[j].u32BitmapSize);

				CVI_OSDC_SetBitmapObjAttr(&osdc_canvas, &obj_vec[i],
						pstBitmaps[j++].pBitmapVAddr,
						pstObjAttr[i].stBitmap.stRect.s32X,
						pstObjAttr[i].stBitmap.stRect.s32Y,
						pstObjAttr[i].stBitmap.stRect.u32Width,
						pstObjAttr[i].stBitmap.stRect.u32Height,
						false);
			}
		}

		pthread_mutex_lock(&work->lock);
		work->osdc_canvas = osdc_canvas;
		work->obj_vec = obj_vec;
		work->obj_num = pstCanvasCmprAttr->u32ObjNum;
		work->virt_addr = canvas->pu8VirtAddr;
		work->ion_len = s32IonLen;
		work->bs_size = &bs_size;
		work->handle = Handle;
		work->pstBitmaps = pstBitmaps;

		sem_post(&work->sem_work);

		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += 1;

		s32Ret = sem_timedwait(&work->sem_done, &ts);
		if (s32Ret != 0) {
			if (errno == ETIMEDOUT) {
				CVI_TRACE_RGN(CVI_DBG_WARN, "OSDC draw timeout, handle=%d\n", Handle);
			} else {
				CVI_TRACE_RGN(CVI_DBG_ERR, "Sem wait error: %d\n", errno);
			}
			pthread_mutex_unlock(&work->lock);
		}

		status = work->status;

		if (status != 1) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Region(%d) needs ion size(%d), current size(%d)!\n",
			Handle, bs_size, s32IonLen);
		}
		pthread_mutex_unlock(&work->lock);

		free(pstBitmaps);

		s32Ret = SDK_CTRL_SET_CFG((void *)&bs_size, NULL, RGN_SDK_SET_CMPR_SIZE, Handle);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN compree size fail.\n");
			return s32Ret;
		}

		free(obj_vec);
	}

	CVI_SYS_IonFlushCache(canvas->u64PhyAddr, canvas->pu8VirtAddr, canvas->u32Size);
#endif
	// Driver control
	s32Ret = SDK_CTRL_SET_CFG(NULL, NULL, RGN_SDK_UPDATE_CANVAS, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Update RGN canvas fail.\n");
		return s32Ret;
	}

#if CONFIG_OSDC_DUAL_906
	pthread_mutex_lock(&canvas_q_lock);
	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(canvas, &canvas_q, stailq) {
			if (canvas->Handle == Handle) {
				STAILQ_REMOVE(&canvas_q, canvas, rgn_canvas, stailq);
				free(canvas);
				break;
			}
		}
	}
	pthread_mutex_unlock(&canvas_q_lock);
#endif

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setchnpalette(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_PALETTE_S *pstPalette)
{
	CVI_S32 s32Ret;

	s32Ret = SDK_CTRL_SET_CFG((void *)pstChn, (void *)pstPalette, RGN_SDK_SET_CHN_PALETTE, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN channel palette fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setcmpr_size(RGN_HANDLE Handle, CVI_U32 *bs_size)
{
	CVI_S32 s32Ret;

	s32Ret = SDK_CTRL_SET_CFG((void *)bs_size, NULL, RGN_SDK_SET_CMPR_SIZE, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN compree size fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}
