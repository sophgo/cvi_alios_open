#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"

#include "cvi_gdc.h"
#include "driver_ldc.h"
#include "ldc_uapi.h"
#include "osal.h"

#define LDC_YUV_BLACK 0x808000
#define LDC_RGB_BLACK 0x0

#define GDC_SUPPORT_FMT(fmt)                                                   \
	((fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||           \
	 (fmt == PIXEL_FORMAT_YUV_400))

#define CHECK_GDC_FORMAT(imgIn, imgOut)                                                                                \
	do {                                                                                                           \
		if (imgIn.stVFrame.enPixelFormat != imgOut.stVFrame.enPixelFormat) {                                   \
			CVI_TRACE_GDC(CVI_DBG_ERR, "in/out pixelformat(%d-%d) mismatch\n",                             \
				      imgIn.stVFrame.enPixelFormat, imgOut.stVFrame.enPixelFormat);                    \
			return CVI_ERR_GDC_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
		if (!GDC_SUPPORT_FMT(imgIn.stVFrame.enPixelFormat)) {                                                  \
			CVI_TRACE_GDC(CVI_DBG_ERR, "pixelformat(%d) unsupported\n", imgIn.stVFrame.enPixelFormat);     \
			return CVI_ERR_GDC_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
	} while (0)

#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

static CVI_S32 gdc_rotation_check_size(ROTATION_E enRotation, const GDC_TASK_ATTR_S *pstTask)
{
	if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	if (enRotation == ROTATION_90 || enRotation == ROTATION_270 || enRotation == ROTATION_XY_FLIP) {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	} else {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 platform_gdc_suspend(void)
{
	return driver_ldc_ioctl(CVI_LDC_SUSPEND, NULL);
}

CVI_S32 platform_gdc_resume(void)
{
	return driver_ldc_ioctl(CVI_LDC_RESUME, NULL);
}

CVI_S32 platform_gdc_init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	// CVI_S32 fd = get_ldc_fd();

	// s32Ret = gdc_init(fd);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_GDC(CVI_DBG_ERR, "init fail\n");
	// 	return s32Ret;
	// }

	return s32Ret;
}

CVI_S32 platform_gdc_deinit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	// CVI_S32 fd = get_ldc_fd();

	// s32Ret = gdc_deinit(fd);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_GDC(CVI_DBG_ERR, "deinit fail\n");
	// 	return s32Ret;
	// }

	// gdc_free_all_tsk_mesh();

	return s32Ret;
}

CVI_S32 platform_gdc_beginjob(GDC_HANDLE *phHandle)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, phHandle);

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	if (driver_ldc_ioctl(CVI_LDC_BEGIN_JOB, &cfg))
		return CVI_FAILURE;

	*phHandle = cfg.handle;

	return CVI_SUCCESS;
}

CVI_S32 platform_gdc_endjob(GDC_HANDLE hHandle)
{
	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.handle = hHandle;
	return driver_ldc_ioctl(CVI_LDC_END_JOB, &cfg);
}

CVI_S32 platform_gdc_canceljob(GDC_HANDLE hHandle)
{
	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.handle = hHandle;
	return driver_ldc_ioctl(CVI_LDC_CANCEL_JOB, &cfg);
}

CVI_S32 platform_gdc_addrotationtask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	UNUSED(hHandle);

	if (gdc_rotation_check_size(enRotation, pstTask) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_rotation_check_size fail\n");
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	struct gdc_task_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	//memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	//attr.reserved = pstTask->reserved;
	attr.enRotation = enRotation;
	return driver_ldc_ioctl(CVI_LDC_ADD_ROT_TASK, &attr);
}

CVI_S32 platform_gdc_addldctask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask
	, const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstLDCAttr);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	UNUSED(hHandle);

	if (enRotation != ROTATION_0) {
		if (gdc_rotation_check_size(enRotation, pstTask) != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_rotation_check_size fail\n");
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	}

	struct gdc_task_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	attr.reserved = pstTask->reserved;
	attr.enRotation = enRotation;
	return driver_ldc_ioctl(CVI_LDC_ADD_LDC_TASK, &attr);
}

CVI_S32 platform_gdc_setjobidentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr)
{
	struct gdc_identity_attr cfg = {0};

	cfg.handle = hHandle;
	memcpy(&cfg.attr, identity_attr, sizeof(*identity_attr));
	return driver_ldc_ioctl(CVI_LDC_SET_JOB_IDENTITY, &cfg);
}

CVI_S32 platform_gdc_getchnframe(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	struct gdc_chn_frm_cfg cfg;
	CVI_S32 s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	memcpy(&cfg.identity.attr, identity, sizeof(*identity));
	cfg.MilliSec = s32MilliSec;

	s32Ret = driver_ldc_ioctl(CVI_LDC_GET_CHN_FRM, &cfg);
	if (s32Ret == CVI_SUCCESS)
		memcpy(pstFrameInfo, &cfg.VideoFrame, sizeof(*pstFrameInfo));

	return s32Ret;
}

CVI_S32 platform_gdc_getworkjob(GDC_HANDLE *phHandle)
{
	struct gdc_handle_data cfg;
	CVI_S32 s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	s32Ret = driver_ldc_ioctl(CVI_LDC_GET_WORK_JOB, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc get work job fail\n");
		return CVI_FAILURE;
	}

	*phHandle = cfg.handle;

	return CVI_SUCCESS;
}

CVI_S32 platform_gdc_attachvbpool(MMF_CHN_S *pChn, VB_POOL u32VbPool)
{
	struct ldc_vb_pool_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.Chn = *pChn;
	cfg.VbPool = u32VbPool;

	return driver_ldc_ioctl(CVI_LDC_ATTACH_VB_POOL, &cfg);
}

CVI_S32 platform_gdc_detachvbpool(MMF_CHN_S *pChn)
{
	struct ldc_vb_pool_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.Chn = *pChn;
	cfg.VbPool = -1;

	return driver_ldc_ioctl(CVI_LDC_DETACH_VB_POOL, &cfg);
}