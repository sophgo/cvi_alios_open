#ifndef __SYS_INTERNAL_H__
#define __SYS_INTERNAL_H__

#include <stdbool.h>
#include <stdint.h>
#include <sys/queue.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>

#include <cvi_common.h>
#include <cvi_comm_sys.h>
#include "cvi_debug.h"


#ifdef CVI_DEBUG
#define VPU_PROFILE
#endif

#define BIT(nr)      (UINT64_C(1) << (nr))

#define IS_VDEV_CLOSED(x) ((x) == VDEV_STATE_CLOSED)
#define IS_VDEV_OPEN(x) ((x) == VDEV_STATE_OPEN)
#define IS_VDEV_RUN(x) ((x) == VDEV_STATE_RUN)
#define IS_VDEV_STOP(x) ((x) == VDEV_STATE_STOP)

#define DEFAULT_MESH_PADDR	0x80000000

#define VPSS_GRP_SUPPORT_FMT(fmt) \
	((fmt == PIXEL_FORMAT_RGB_888_PLANAR) || (fmt == PIXEL_FORMAT_BGR_888_PLANAR) ||	\
	 (fmt == PIXEL_FORMAT_RGB_888) || (fmt == PIXEL_FORMAT_BGR_888) ||			\
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_420) || (fmt == PIXEL_FORMAT_YUV_PLANAR_422) ||	\
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_444) || (fmt == PIXEL_FORMAT_YUV_400) ||		\
	 (fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||				\
	 (fmt == PIXEL_FORMAT_NV16) || (fmt == PIXEL_FORMAT_NV61) ||				\
	 (fmt == PIXEL_FORMAT_YUYV) || (fmt == PIXEL_FORMAT_UYVY) ||				\
	 (fmt == PIXEL_FORMAT_YVYU) || (fmt == PIXEL_FORMAT_VYUY) ||				\
	 (fmt == PIXEL_FORMAT_YUV_444))
#define VPSS_CHN_SUPPORT_FMT(fmt) \
	((fmt == PIXEL_FORMAT_RGB_888_PLANAR) || (fmt == PIXEL_FORMAT_BGR_888_PLANAR) ||	\
	 (fmt == PIXEL_FORMAT_RGB_888) || (fmt == PIXEL_FORMAT_BGR_888) ||			\
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_420) || (fmt == PIXEL_FORMAT_YUV_PLANAR_422) ||	\
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_444) || (fmt == PIXEL_FORMAT_YUV_400) ||		\
	 (fmt == PIXEL_FORMAT_HSV_888) || (fmt == PIXEL_FORMAT_HSV_888_PLANAR) ||		\
	 (fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||				\
	 (fmt == PIXEL_FORMAT_NV16) || (fmt == PIXEL_FORMAT_NV61) ||				\
	 (fmt == PIXEL_FORMAT_YUYV) || (fmt == PIXEL_FORMAT_UYVY) ||				\
	 (fmt == PIXEL_FORMAT_YVYU) || (fmt == PIXEL_FORMAT_VYUY) ||				\
	 (fmt == PIXEL_FORMAT_YUV_444) ||	\
	 (fmt == PIXEL_FORMAT_FP32_C3_PLANAR) || (fmt == PIXEL_FORMAT_FP16_C3_PLANAR) ||	\
	 (fmt == PIXEL_FORMAT_BF16_C3_PLANAR) || (fmt == PIXEL_FORMAT_INT8_C3_PLANAR) ||	\
	 (fmt == PIXEL_FORMAT_UINT8_C3_PLANAR))
#define VO_SUPPORT_FMT(fmt) \
	((fmt == PIXEL_FORMAT_RGB_888_PLANAR) || (fmt == PIXEL_FORMAT_BGR_888_PLANAR) ||	\
	 (fmt == PIXEL_FORMAT_RGB_888) || (fmt == PIXEL_FORMAT_BGR_888) ||			\
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_420) || (fmt == PIXEL_FORMAT_YUV_PLANAR_422) ||	\
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_444) || (fmt == PIXEL_FORMAT_YUV_400) ||		\
	 (fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||				\
	 (fmt == PIXEL_FORMAT_NV16) || (fmt == PIXEL_FORMAT_NV61) ||				\
	 (fmt == PIXEL_FORMAT_YUYV) || (fmt == PIXEL_FORMAT_UYVY) ||				\
	 (fmt == PIXEL_FORMAT_YVYU) || (fmt == PIXEL_FORMAT_VYUY))

#define GDC_SUPPORT_FMT(fmt) \
	((fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||				\
	 (fmt == PIXEL_FORMAT_YUV_400))
#define DWA_SUPPORT_FMT(fmt) \
	((fmt == PIXEL_FORMAT_RGB_888_PLANAR) || (fmt == PIXEL_FORMAT_BGR_888_PLANAR) ||	\
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_444) || (fmt == PIXEL_FORMAT_YUV_PLANAR_420) || (fmt == PIXEL_FORMAT_YUV_400))

CVI_S32 CVI_VI_SetBypassFrm(CVI_U32 snr_num, CVI_U8 bypass_num);
struct cvi_gdc_mesh {
	CVI_U64 paddr;
	CVI_VOID *vaddr;
	CVI_U32 meshSize;
	pthread_mutex_t lock;
};

#endif // __SYS_INTERNAL_H__
