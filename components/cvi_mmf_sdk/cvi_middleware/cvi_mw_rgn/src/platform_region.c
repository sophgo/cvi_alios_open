#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>

// #include "cvi_base.h"
// #include "cvi_mw_base.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_region.h"
#include "driver_rgn.h"
#include "rgn_uapi.h"

#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

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

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstCanvasInfo);

	// Driver control
	s32Ret = SDK_CTRL_GET_CFG((void *)pstCanvasInfo, NULL, RGN_SDK_GET_CANVAS_INFO, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN canvas information fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_updatecanvas(RGN_HANDLE Handle)
{
	CVI_S32 s32Ret;

	// Driver control
	s32Ret = SDK_CTRL_SET_CFG(NULL, NULL, RGN_SDK_UPDATE_CANVAS, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Update RGN canvas fail.\n");
		return s32Ret;
	}

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
