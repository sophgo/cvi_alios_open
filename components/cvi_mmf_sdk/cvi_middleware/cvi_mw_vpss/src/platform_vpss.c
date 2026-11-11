#include <unistd.h>
#include <aos/kernel.h>
#include <math.h>
#include <pthread.h>
#include <inttypes.h>

#include "cvi_math.h"
#include "cvi_errno.h"
#include "cvi_debug.h"
#include "cvi_buffer.h"
#include "cvi_vo.h"
#include "cvi_vb.h"

#include "platform_vpss.h"
#include "vpss_uapi.h"
#include "driver_vpss.h"

// struct cvi_gdc_mesh mesh[VPSS_MAX_GRP_NUM][VPSS_MAX_CHN_NUM];

// static VPSS_BIN_DATA vpss_bin_data[VPSS_MAX_GRP_NUM];
#define BIT(nr)      (UINT64_C(1) << (nr))

#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

#define GDC_SUPPORT_FMT(fmt)                                                   \
	((fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||           \
	 (fmt == PIXEL_FORMAT_YUV_400))

struct cvi_stitch_ctx {
	CVI_BOOL bUse;
	CVI_BOOL bStart;
	CVI_U8 DropFrame;
	VPSS_GRP VpssGrp;
	VB_POOL VbPool;
	CVI_STITCH_ATTR_S stStitchAttr;
	pthread_t thread;
	pthread_mutex_t lock;
	aos_timer_t timer;
	aos_event_t wait;
};

static struct cvi_stitch_ctx s_StitchCtx[VPSS_MAX_GRP_NUM];

static inline CVI_S32 CHECK_VPSS_GRP_VALID(VPSS_GRP grp)
{
	if ((grp >= VPSS_MAX_GRP_NUM) || (grp < 0)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssGrp(%d) exceeds Max(%d)\n", grp, VPSS_MAX_GRP_NUM);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static inline CVI_S32 CHECK_VPSS_CHN_VALID(VPSS_CHN VpssChn)
{
	if ((VpssChn >= VPSS_MAX_CHN_NUM) || (VpssChn < 0)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Chn(%d) invalid.\n", VpssChn);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static inline CVI_S32 CHECK_VPSS_GDC_FMT(VPSS_GRP grp, VPSS_CHN chn, PIXEL_FORMAT_E fmt)
{
	if (!GDC_SUPPORT_FMT(fmt)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid PixFormat(%d) for GDC.\n",
				grp, chn, fmt);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

/*
static CVI_S32 _vpss_update_rotation_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_S32 fd = get_vpss_fd();
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];
	struct vpss_chn_rot_cfg cfg;

	UNUSED(u32Width);
	UNUSED(u32Height);
	// TODO: dummy settings
	pmesh->paddr = DEFAULT_MESH_PADDR;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enRotation = enRotation;
	return vpss_set_chn_rotation(fd, &cfg);
	return CVI_SUCCESS;
}

static CVI_S32 _vpss_update_ldc_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VPSS_LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U64 paddr = 0;
	CVI_VOID *vaddr;
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];
	CVI_S32 s32Ret;
	char mesh_name[128];

	if (!pstLDCAttr->bEnable) {
		if (enRotation != ROTATION_0)
			return _vpss_update_rotation_mesh(VpssGrp, VpssChn, enRotation,
				u32Width, u32Height);
		else {
			CVI_S32 fd = get_vpss_fd();
			struct vpss_chn_ldc_cfg cfg;

			cfg.VpssGrp = VpssGrp;
			cfg.VpssChn = VpssChn;
			cfg.enRotation = enRotation;
			cfg.stLDCAttr = *pstLDCAttr;
			cfg.meshHandle = paddr;
			return vpss_set_chn_ldc(fd, &cfg);
		}
	}

	snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
	s32Ret = CVI_GDC_GenLDCMesh(u32Width, u32Height, &pstLDCAttr->stAttr,
				mesh_name, &paddr, &vaddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) gen mesh fail\n",
				VpssGrp, VpssChn);
		return s32Ret;
	}

	pthread_mutex_lock(&pmesh->lock);
	pmesh->paddr = paddr;
	pmesh->vaddr = vaddr;
	pthread_mutex_unlock(&pmesh->lock);

	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "Grp(%d) Chn(%d) mesh base(%#"PRIx64") vaddr(%p)\n"
		      , VpssGrp, VpssChn, paddr, vaddr);

	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_ldc_cfg cfg;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enRotation = enRotation;
	cfg.stLDCAttr = *pstLDCAttr;
	cfg.meshHandle = paddr;
	return vpss_set_chn_ldc(fd, &cfg);
} */

static void _vpss_proamp_2_csc(struct vpss_grp_csc_cfg *csc_cfg)
{
	// for grp proc-amp.
	CVI_S32 *proc_amp = csc_cfg->proc_amp;
	float h = (float)(proc_amp[PROC_AMP_HUE] - 50) * PI / 360;
	float b_off = (proc_amp[PROC_AMP_BRIGHTNESS] - 50) * 2.56;
	float C_gain = 1 + (proc_amp[PROC_AMP_CONTRAST] - 50) * 0.02;
	float S = 1 + (proc_amp[PROC_AMP_SATURATION] - 50) * 0.02;
	float A = cos(h) * C_gain * S;
	float B = sin(h) * C_gain * S;
	float C_diff, c_off, tmp;
	CVI_U8 sub_0_l, add_0_l, add_1_l, add_2_l;

	if (proc_amp[PROC_AMP_CONTRAST] > 50)
		C_diff = 256 / C_gain;
	else
		C_diff = 256 * C_gain;
	c_off = 128 - (C_diff/2);

	if (b_off < 0) {
		sub_0_l = ABS(proc_amp[PROC_AMP_BRIGHTNESS] - 50) * 2.56;
		add_0_l = 0;
		add_1_l = 0;
		add_2_l = 0;
	} else {
		sub_0_l = 0;
		if ((C_gain * b_off) > 255) {
			add_0_l = 255;
		} else {
			add_0_l = C_gain * b_off;
		}
		add_1_l = add_0_l;
		add_2_l = add_0_l;
	}

	if (proc_amp[PROC_AMP_CONTRAST] > 50) {
		csc_cfg->sub[0] = sub_0_l + c_off;
		csc_cfg->add[0] = add_0_l;
		csc_cfg->add[1] = add_1_l;
		csc_cfg->add[2] = add_2_l;
	} else {
		csc_cfg->sub[0] = sub_0_l;
		csc_cfg->add[0] = add_0_l + c_off;
		csc_cfg->add[1] = add_1_l + c_off;
		csc_cfg->add[2] = add_2_l + c_off;
	}
	csc_cfg->sub[1] = 128;
	csc_cfg->sub[2] = 128;

	csc_cfg->coef[0][0] = C_gain * BIT(10);
	tmp = B * -1.402;
	csc_cfg->coef[0][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = A * 1.402;
	csc_cfg->coef[0][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	csc_cfg->coef[1][0] = C_gain * BIT(10);
	tmp = A * -0.344 + B * 0.714;
	csc_cfg->coef[1][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = B * -0.344 + A * -0.714;
	csc_cfg->coef[1][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	csc_cfg->coef[2][0] = C_gain * BIT(10);
	tmp = A * 1.772;
	csc_cfg->coef[2][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = B * 1.772;
	csc_cfg->coef[2][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[0][0]: %#4x coef[0][1]: %#4x coef[0][2]: %#4x\n"
		, csc_cfg->coef[0][0], csc_cfg->coef[0][1]
		, csc_cfg->coef[0][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[1][0]: %#4x coef[1][1]: %#4x coef[1][2]: %#4x\n"
		, csc_cfg->coef[1][0], csc_cfg->coef[1][1]
		, csc_cfg->coef[1][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[2][0]: %#4x coef[2][1]: %#4x coef[2][2]: %#4x\n"
		, csc_cfg->coef[2][0], csc_cfg->coef[2][1]
		, csc_cfg->coef[2][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "sub[0]: %3d sub[1]: %3d sub[2]: %3d\n"
		, csc_cfg->sub[0], csc_cfg->sub[1], csc_cfg->sub[2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "add[0]: %3d add[1]: %3d add[2]: %3d\n"
		, csc_cfg->add[0], csc_cfg->add[1], csc_cfg->add[2]);
}

static CVI_VOID _vpss_check_normalize(VPSS_CHN_ATTR_S *pstChnAttr)
{
	if (pstChnAttr->stNormalize.bEnable) {
		for (CVI_U8 i = 0; i < 3; ++i) {
			if (pstChnAttr->stNormalize.factor[i] >= 1.0f) {
				pstChnAttr->stNormalize.factor[i] = 1.0f - 1.0f/8192;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "factor%d replaced with max value 8191/8192\n", i);
			}
			if (pstChnAttr->stNormalize.factor[i] < (1.0f/8192)) {
				pstChnAttr->stNormalize.factor[i] = (1.0f/8192);
				CVI_TRACE_VPSS(CVI_DBG_WARN, "factor%d replaced with min value 1/8192\n", i);
			}
			if (pstChnAttr->stNormalize.mean[i] > 255.0f) {
				pstChnAttr->stNormalize.mean[i] = 255.0f;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "mean%d replaced with max value 255\n", i);
			}
			if (pstChnAttr->stNormalize.mean[i] < 0) {
				pstChnAttr->stNormalize.mean[i] = 0;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "mean%d replaced with min value 0\n", i);
			}
		}
	}
}

static CVI_S32 _set_window_attr(VPSS_GRP VpssGrp, RECT_S *pstDstRect,
			VIDEO_FRAME_INFO_S *cur_frame, CVI_BOOL bFilled)
{
	CVI_S32 s32Ret;
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN_ATTR_S stChnAttr;

	s32Ret = platform_vpss_getgrpattr(VpssGrp, &stGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetGrpAttr failed\n");
		return s32Ret;
	}
	stGrpAttr.u32MaxW = cur_frame->stVFrame.u32Width;
	stGrpAttr.u32MaxH = cur_frame->stVFrame.u32Height;
	stGrpAttr.enPixelFormat = cur_frame->stVFrame.enPixelFormat;
	s32Ret = platform_vpss_setgrpattr(VpssGrp, &stGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetGrpAttr failed\n");
		return s32Ret;
	}

	s32Ret = platform_vpss_getchnattr(VpssGrp, 0, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr failed\n");
		return s32Ret;
	}

	//clean doneq buffers
	if (bFilled) {
		stChnAttr.u32Depth = 0;
		s32Ret = platform_vpss_setchnattr(VpssGrp, 0, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr failed\n");
			return s32Ret;
		}
	}

	stChnAttr.u32Depth = 1;
	stChnAttr.stAspectRatio.enMode	= ASPECT_RATIO_MANUAL;
	stChnAttr.stAspectRatio.stVideoRect.s32X	= pstDstRect->s32X;
	stChnAttr.stAspectRatio.stVideoRect.s32Y	= pstDstRect->s32Y;
	stChnAttr.stAspectRatio.stVideoRect.u32Width  = pstDstRect->u32Width;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = pstDstRect->u32Height;

	if (bFilled) {
		// fill bg color is the there this is the first window
		stChnAttr.stAspectRatio.bEnableBgColor = CVI_TRUE;
		stChnAttr.stAspectRatio.u32BgColor = 0x0;
	} else{
		stChnAttr.stAspectRatio.bEnableBgColor = CVI_FALSE;
		stChnAttr.stAspectRatio.u32BgColor = 0x0;
	}

	s32Ret = platform_vpss_setchnattr(VpssGrp, 0, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr failed\n");
		return s32Ret;
	}
	return CVI_SUCCESS;
}

static CVI_S32 _update_src_frame(CVI_STITCH_ATTR_S *pstStitchAttr, CVI_BOOL isFirst,
		VIDEO_FRAME_INFO_S *pstFrameSrc)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	CVI_STITCH_CHN_S *pstStitchChn;
	VIDEO_FRAME_INFO_S stFrame;

	for (i = 0; i < pstStitchAttr->u8ChnNum; i++) {
		pstStitchChn = &pstStitchAttr->astStitchChn[i];

		//skip
		if (pstStitchChn->stStitchSrc.VpssGrp == -1)
			continue;
		//get source frame
		s32Ret = platform_vpss_getchnframe(pstStitchChn->stStitchSrc.VpssGrp,
			pstStitchChn->stStitchSrc.VpssChn, &stFrame, isFirst ? 1000 : 0);
		if (s32Ret == CVI_SUCCESS) {
			if (!isFirst)
				platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
					pstStitchChn->stStitchSrc.VpssChn, &pstFrameSrc[i]);
			memcpy(&pstFrameSrc[i], &stFrame, sizeof(stFrame));
		} else if (isFirst) {
			break;
		}
	}

	if (isFirst && s32Ret) {
		for (--i; i >= 0; --i) {
			pstStitchChn = &pstStitchAttr->astStitchChn[i];

			platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
				pstStitchChn->stStitchSrc.VpssChn, &pstFrameSrc[i]);
		}
	}

	return isFirst ? s32Ret : CVI_SUCCESS;
}

static CVI_VOID _release_src_frame(CVI_STITCH_ATTR_S *pstStitchAttr,
		VIDEO_FRAME_INFO_S *pstFrameSrc)
{
	CVI_S32 i;
	CVI_STITCH_CHN_S *pstStitchChn;

	for (i = 0; i < pstStitchAttr->u8ChnNum; i++) {
		pstStitchChn = &pstStitchAttr->astStitchChn[i];

		//skip
		if (pstStitchChn->stStitchSrc.VpssGrp == -1)
			continue;
		platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
			pstStitchChn->stStitchSrc.VpssChn, &pstFrameSrc[i]);
	}
}

static CVI_S32 _stitch_proc_frc(struct cvi_stitch_ctx *pStitchCtx,
		VIDEO_FRAME_INFO_S *pstFrameSrc)
{
	CVI_S32 i, s32Ret;
	CVI_STITCH_ATTR_S *pstStitchAttr = &pStitchCtx->stStitchAttr;
	CVI_STITCH_CHN_S *pstStitchChn;
	VPSS_GRP VpssGrp = pStitchCtx->VpssGrp;
	CVI_BOOL bFilled = CVI_TRUE;
	VIDEO_FRAME_INFO_S stFrame_out;

	pthread_mutex_lock(&pStitchCtx->lock);
	for (i = 0; i < pstStitchAttr->u8ChnNum; i++) {
		pstStitchChn = &pstStitchAttr->astStitchChn[i];

		//skip
		if (pstStitchChn->stStitchSrc.VpssGrp == -1)
			continue;

		if (pStitchCtx->DropFrame > 0) {
			pStitchCtx->DropFrame--;
			pthread_mutex_unlock(&pStitchCtx->lock);
			return CVI_SUCCESS;
		}
		//not stitch
		if ((pstStitchAttr->u8ChnNum == 1) &&
			(pstFrameSrc[i].stVFrame.u32Width == pstStitchAttr->stOutSize.u32Width) &&
			(pstFrameSrc[i].stVFrame.u32Height == pstStitchAttr->stOutSize.u32Height)) {
#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
			CVI_VO_SendFrame(0, pstStitchAttr->VoChn, &pstFrameSrc[i], 1000);
#endif
			pthread_mutex_unlock(&pStitchCtx->lock);
			return CVI_SUCCESS;
		}
		if (!bFilled) {
			s32Ret = platform_vpss_sendchnframe(VpssGrp, 0, &stFrame_out, 1000);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SendChnFrame failed\n");
				platform_vpss_releasechnframe(VpssGrp, 0, &stFrame_out);
				break;
			}
			platform_vpss_releasechnframe(VpssGrp, 0, &stFrame_out);
		}

		s32Ret = _set_window_attr(VpssGrp, &pstStitchChn->stDstRect, &pstFrameSrc[i], bFilled);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "set_window_attr failed\n");
			break;
		}

		s32Ret = platform_vpss_sendframe(VpssGrp, &pstFrameSrc[i], 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SendFrame failed\n");
			break;
		}

		s32Ret = platform_vpss_getchnframe(VpssGrp, 0, &stFrame_out, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "i(%d) Grp(%d) Chn(0) GetChnFrame failed\n", i, VpssGrp);
			break;
		}

		bFilled = CVI_FALSE;
	}

	if (!bFilled) {
#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
		if (s32Ret == CVI_SUCCESS)
			CVI_VO_SendFrame(0, pstStitchAttr->VoChn, &stFrame_out, 1000);
#endif
		platform_vpss_releasechnframe(VpssGrp, 0, &stFrame_out);
	}

	pthread_mutex_unlock(&pStitchCtx->lock);
	return s32Ret;
}

static void _wake_up_stitch(void *timer, void *arg)
{
	struct cvi_stitch_ctx *pStitchCtx = (struct cvi_stitch_ctx *)arg;

	aos_event_set(&pStitchCtx->wait, 0x1, AOS_EVENT_OR);
}

static void *_vpss_stitch_handler_frc(void *arg)
{
	CVI_S32 interval_ms;
	CVI_S32 s32Ret;
	unsigned int actl_flags;
	struct cvi_stitch_ctx *pStitchCtx = (struct cvi_stitch_ctx *)arg;
	CVI_VOID *buf;
	VIDEO_FRAME_INFO_S *pstFrameSrc;
	CVI_BOOL isFirst = CVI_TRUE;

	buf = malloc(sizeof(VIDEO_FRAME_INFO_S) * CVI_STITCH_CHN_MAX_NUM);
	if (buf == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "malloc failed.\n");
		return NULL;
	}
	pstFrameSrc = (VIDEO_FRAME_INFO_S *)buf;

	if (aos_event_new(&pStitchCtx->wait, 0)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "aos_event_new failed.\n");
		free(buf);
		return NULL;
	}

	//start timer
	interval_ms = 1000 / pStitchCtx->stStitchAttr.s32OutFps;
	if (aos_timer_new(&pStitchCtx->timer, _wake_up_stitch,
		arg, interval_ms, AOS_TIMER_REPEAT)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "aos_timer_new failed.\n");
		aos_event_free(&pStitchCtx->wait);
		free(buf);
		return NULL;
	}
	if (aos_timer_start(&pStitchCtx->timer)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "aos_timer_start failed.\n");
		aos_timer_free(&pStitchCtx->timer);
		aos_event_free(&pStitchCtx->wait);
		free(buf);
		return NULL;
	}

	while (pStitchCtx->bStart) {
		s32Ret = aos_event_get(&pStitchCtx->wait, 0x1, AOS_EVENT_OR_CLEAR, &actl_flags, 100);
		/* timeout */
		if (s32Ret < 0) {
			continue;
		}
		if (!pStitchCtx->bStart)
			break;

		if (_update_src_frame(&pStitchCtx->stStitchAttr, isFirst, pstFrameSrc))
			continue;

		isFirst = CVI_FALSE;
		_stitch_proc_frc(pStitchCtx, pstFrameSrc);
	}

	if (!isFirst)
		_release_src_frame(&pStitchCtx->stStitchAttr, pstFrameSrc);

	if (aos_timer_is_valid(&pStitchCtx->timer)) {
		aos_timer_stop(&pStitchCtx->timer);
		aos_timer_free(&pStitchCtx->timer);
	}

	aos_event_free(&pStitchCtx->wait);
	free(buf);

	return NULL;
}

static CVI_S32 _stitch_proc(struct cvi_stitch_ctx *pStitchCtx)
{
	CVI_S32 i, s32Ret;
	CVI_STITCH_ATTR_S stStitchAttr;
	CVI_STITCH_ATTR_S *pstStitchAttr = &stStitchAttr;
	CVI_STITCH_CHN_S *pstStitchChn;
	VPSS_GRP VpssGrp = pStitchCtx->VpssGrp;
	CVI_BOOL bFilled = CVI_TRUE;
	VIDEO_FRAME_INFO_S stFrame_src = {0}, stFrame_out  = {0};
	static VPSS_CHN_ATTR_S CurChnAttr[CVI_STITCH_CHN_MAX_NUM] = {0}, PreChnAttr[CVI_STITCH_CHN_MAX_NUM] = {0};
	static uint8_t frame_drop[CVI_STITCH_CHN_MAX_NUM] = {0};

	pthread_mutex_lock(&pStitchCtx->lock);
	memcpy(pstStitchAttr, &pStitchCtx->stStitchAttr, sizeof(stStitchAttr));
	pthread_mutex_unlock(&pStitchCtx->lock);

	for (i = 0; i < pstStitchAttr->u8ChnNum; i++) {
		pstStitchChn = &pstStitchAttr->astStitchChn[i];

		//skip
		if (pstStitchChn->stStitchSrc.VpssGrp == -1)
			continue;
		//get source frame
		s32Ret = platform_vpss_getchnframe(pstStitchChn->stStitchSrc.VpssGrp,
			pstStitchChn->stStitchSrc.VpssChn, &stFrame_src, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "Grp(%d) Chn(%d) GetChnFrame failed\n",
				pstStitchChn->stStitchSrc.VpssGrp,
				pstStitchChn->stStitchSrc.VpssChn);
			break;
		}

		pthread_mutex_lock(&pStitchCtx->lock);
		if (pStitchCtx->DropFrame > 0) {
			platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
				pstStitchChn->stStitchSrc.VpssChn, &stFrame_src);
			pStitchCtx->DropFrame--;
			pthread_mutex_unlock(&pStitchCtx->lock);
			return CVI_SUCCESS;
		}
		pthread_mutex_unlock(&pStitchCtx->lock);

		s32Ret = platform_vpss_getchnattr(pstStitchChn->stStitchSrc.VpssGrp,
			pstStitchChn->stStitchSrc.VpssChn, &CurChnAttr[i]);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "Grp(%d) Chn(%d) GetChnAttr failed\n",
				pstStitchChn->stStitchSrc.VpssGrp,
				pstStitchChn->stStitchSrc.VpssChn);
			break;
		}
		if ((CurChnAttr[i].u32Width != PreChnAttr[i].u32Width) ||
				(CurChnAttr[i].u32Height != PreChnAttr[i].u32Height)) {
			PreChnAttr[i] = CurChnAttr[i];
			if (!bFilled) {
				bFilled = CVI_TRUE;
				platform_vpss_releasechnframe(VpssGrp, 0, &stFrame_out);
			}
			platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
				pstStitchChn->stStitchSrc.VpssChn, &stFrame_src);
			frame_drop[i]++;
			if (frame_drop[i] > 1) {
				frame_drop[i] = 0;
				PreChnAttr[i] = CurChnAttr[i];
			}
			continue;
		}

		//not stitch
		if ((pstStitchAttr->u8ChnNum == 1) &&
			(stFrame_src.stVFrame.u32Width == pstStitchAttr->stOutSize.u32Width) &&
			(stFrame_src.stVFrame.u32Height == pstStitchAttr->stOutSize.u32Height)) {
#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
			CVI_VO_SendFrame(0, pstStitchAttr->VoChn, &stFrame_src, 1000);
#endif
			platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
				pstStitchChn->stStitchSrc.VpssChn, &stFrame_src);
			return CVI_SUCCESS;
		}

		if (!bFilled) {
			s32Ret = platform_vpss_sendchnframe(VpssGrp, 0, &stFrame_out, 1000);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SendChnFrame failed\n");
				platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
					pstStitchChn->stStitchSrc.VpssChn, &stFrame_src);
				platform_vpss_releasechnframe(VpssGrp, 0, &stFrame_out);
				break;
			}
			platform_vpss_releasechnframe(VpssGrp, 0, &stFrame_out);
		}
		s32Ret = _set_window_attr(VpssGrp, &pstStitchChn->stDstRect, &stFrame_src, bFilled);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "set_window_attr failed\n");
			platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
				pstStitchChn->stStitchSrc.VpssChn, &stFrame_src);
			break;
		}
		s32Ret = platform_vpss_sendframe(VpssGrp, &stFrame_src, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VPSS_SendFrame failed\n");
			platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
				pstStitchChn->stStitchSrc.VpssChn, &stFrame_src);
			break;
		}
		platform_vpss_releasechnframe(pstStitchChn->stStitchSrc.VpssGrp,
			pstStitchChn->stStitchSrc.VpssChn, &stFrame_src);

		s32Ret = platform_vpss_getchnframe(VpssGrp, 0, &stFrame_out, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "i(%d) Grp(%d) Chn(0) GetChnFrame failed\n", i, VpssGrp);
			break;
		}

		bFilled = CVI_FALSE;
	}

	if (!bFilled) {
#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
		if (s32Ret == CVI_SUCCESS)
			CVI_VO_SendFrame(0, pstStitchAttr->VoChn, &stFrame_out, 1000);
#endif
		platform_vpss_releasechnframe(VpssGrp, 0, &stFrame_out);
	}

	return s32Ret;
}

static void *_vpss_stitch_handler(void *arg)
{
	struct cvi_stitch_ctx *pStitchCtx = (struct cvi_stitch_ctx *)arg;

	while (pStitchCtx->bStart) {
		_stitch_proc(pStitchCtx);
		usleep(10000);
	}

	return NULL;
}

/************************vpss Settings**********************************/
CVI_S32 platform_vpss_setmode(const VPSS_MODE_S *pstVPSSMode)
{
	return driver_vpss_ioctl(VPSS_SET_MODE, (unsigned long)pstVPSSMode);
}

CVI_S32 platform_vpss_getmode(VPSS_MODE_S *pstVPSSMode)
{
	return driver_vpss_ioctl(VPSS_GET_MODE, (unsigned long)pstVPSSMode);;
}

/************************grp Settings**********************************/
CVI_S32 platform_vpss_creategrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	struct vpss_grp_cfg cfg;

	osal_memset(&cfg, 0, sizeof(struct vpss_grp_cfg));
	cfg.VpssGrp = VpssGrp;
	osal_memcpy(&cfg.stGrpAttr, pstGrpAttr, sizeof(cfg.stGrpAttr));

	return driver_vpss_ioctl(VPSS_CREATE_GROUP, (unsigned long)&cfg);

	// for chn rotation, ldc mesh gen
	// for (CVI_U8 i = 0; i < VPSS_MAX_CHN_NUM; ++i)
	// 	pthread_mutex_init(&mesh[VpssGrp][i].lock, NULL);

	// return CVI_SUCCESS;
}

CVI_S32 platform_vpss_destroygrp(VPSS_GRP VpssGrp)
{
	return driver_vpss_ioctl(VPSS_DESTROY_GROUP, (unsigned long)&VpssGrp);

	// for (CVI_U8 i = 0; i < VPSS_MAX_CHN_NUM; ++i)
	// 	pthread_mutex_destroy(&mesh[VpssGrp][i].lock);

	// return CVI_SUCCESS;
}

VPSS_GRP platform_vpss_getavailablegrp(CVI_VOID)
{
	VPSS_GRP grp = VPSS_INVALID_GRP;

	driver_vpss_ioctl(VPSS_GET_AVAIL_GROUP, (unsigned long)&grp);

	return grp;
}

CVI_S32 platform_vpss_startgrp(VPSS_GRP VpssGrp)
{
	return driver_vpss_ioctl(VPSS_START_GROUP, (unsigned long)&VpssGrp);
}

CVI_S32 platform_vpss_stopgrp(VPSS_GRP VpssGrp)
{
	return driver_vpss_ioctl(VPSS_STOP_GROUP, (unsigned long)&VpssGrp);
}

CVI_S32 platform_vpss_resetgrp(VPSS_GRP VpssGrp)
{
	return driver_vpss_ioctl(VPSS_RESET_GROUP, (unsigned long)&VpssGrp);
}

CVI_S32 platform_vpss_getgrpattr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr)
{
	struct vpss_grp_cfg cfg;
	CVI_S32 ret;

	osal_memset(&cfg, 0, sizeof(struct vpss_grp_cfg));
	cfg.VpssGrp = VpssGrp;

	ret = driver_vpss_ioctl(VPSS_GET_GRP_ATTR, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get grp attr fail\n", VpssGrp);
		return ret;
	}

	osal_memcpy(pstGrpAttr, &cfg.stGrpAttr, sizeof(*pstGrpAttr));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpattr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	struct vpss_grp_cfg cfg;

	osal_memset(&cfg, 0, sizeof(struct vpss_grp_cfg));
	cfg.VpssGrp = VpssGrp;
	osal_memcpy(&cfg.stGrpAttr, pstGrpAttr, sizeof(cfg.stGrpAttr));

	return driver_vpss_ioctl(VPSS_SET_GRP_ATTR, (unsigned long)&cfg);
}

CVI_S32 platform_vpss_getgrpcrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo)
{
	struct vpss_grp_crop_cfg cfg;
	CVI_S32 ret;

	osal_memset(&cfg, 0, sizeof(struct vpss_grp_crop_cfg));
	cfg.VpssGrp = VpssGrp;

	ret = driver_vpss_ioctl(VPSS_GET_GRP_CROP, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get grp crop fail\n", VpssGrp);
		return ret;
	}

	osal_memcpy(pstCropInfo, &cfg.stCropInfo, sizeof(*pstCropInfo));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpcrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo)
{
	struct vpss_grp_crop_cfg cfg;

	osal_memset(&cfg, 0, sizeof(struct vpss_grp_crop_cfg));
	cfg.VpssGrp = VpssGrp;
	osal_memcpy(&cfg.stCropInfo, pstCropInfo, sizeof(cfg.stCropInfo));

	return driver_vpss_ioctl(VPSS_SET_GRP_CROP, (unsigned long)&cfg);
}

CVI_S32 platform_vpss_sendframe(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	struct vpss_snd_frm_cfg cfg;

	cfg.VpssGrp = VpssGrp;
	osal_memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));
	cfg.s32MilliSec = s32MilliSec;

	return driver_vpss_ioctl(VPSS_SEND_FRAME, (unsigned long)&cfg);
}

CVI_S32 platform_vpss_getgrpprocampctrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	CVI_S32 ret;
	struct vpss_proc_amp_ctrl_cfg cfg = {0};

	cfg.type = type;
	ret = driver_vpss_ioctl(VPSS_GET_AMP_CTRL, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp ctrl fail\n", VpssGrp);
		return ret;
	}
	*ctrl = cfg.ctrl;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpprocamp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value)
{
	struct vpss_proc_amp_cfg cfg = {0};
	CVI_S32 ret;

	cfg.VpssGrp = VpssGrp;
	ret = driver_vpss_ioctl(VPSS_GET_AMP_CFG, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp fail\n", VpssGrp);
		return ret;
	}
	*value = cfg.proc_amp[type];

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpprocamp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 value)
{
	CVI_S32 ret;
	PROC_AMP_CTRL_S ctrl;
	struct vpss_grp_csc_cfg csc_cfg;
	struct vpss_proc_amp_cfg amp_cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) ProcAmp type(%d) invalid.\n", VpssGrp, type);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	platform_vpss_getgrpprocampctrl(VpssGrp, type, &ctrl);
	if ((value > ctrl.maximum) || (value < ctrl.minimum)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) new value(%d) out of range(%d ~ %d).\n"
			, VpssGrp, value, ctrl.minimum, ctrl.maximum);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	amp_cfg.VpssGrp = VpssGrp;
	ret = driver_vpss_ioctl(VPSS_GET_AMP_CFG, (unsigned long)&amp_cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp fail\n", VpssGrp);
		return ret;
	}
	amp_cfg.proc_amp[type] = value;

	osal_memset(&csc_cfg, 0, sizeof(csc_cfg));
	csc_cfg.VpssGrp = VpssGrp;
	osal_memcpy(csc_cfg.proc_amp, amp_cfg.proc_amp, sizeof(csc_cfg.proc_amp));
	_vpss_proamp_2_csc(&csc_cfg);

	ret = driver_vpss_ioctl(VPSS_SET_GRP_CSC_CFG, (unsigned long)&csc_cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set group csc fail\n", VpssGrp);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getallprocamp(VPSS_ALL_PROC_AMP_S *pstProcAmp)
{
	CVI_S32 ret;
	VPSS_ALL_PROC_AMP_S cfg = {0};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstProcAmp);

	ret = driver_vpss_ioctl(VPSS_GET_ALL_AMP, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VPSS get all proc amp fail\n");
		return ret;
	}

	memcpy(pstProcAmp, &cfg, sizeof(*pstProcAmp));

	return CVI_SUCCESS;
}

/* platform_vpss_setgrpparamfrombin: Apply the settings of scene from bin
 *
 * @param VpssGrp: the vpss grp to apply
 * @param scene: the scene of settings stored in bin to use
 * @return: result of the API
 */
CVI_S32 platform_vpss_setgrpparamfrombin(VPSS_GRP VpssGrp, VPSS_BIN_DATA *bin_data)
{
	CVI_S32 ret;
	struct vpss_grp_csc_cfg csc_cfg = {0};

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	csc_cfg.VpssGrp = VpssGrp;
	memcpy(csc_cfg.proc_amp, bin_data->proc_amp, sizeof(csc_cfg.proc_amp));
	_vpss_proamp_2_csc(&csc_cfg);

	ret = driver_vpss_ioctl(VPSS_SET_GRP_CSC_CFG, (unsigned long)&csc_cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set group csc fail\n", VpssGrp);
		return ret;
	}

	CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is exist, vpss grp param use pqbin value !!\n");

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getbinscene(VPSS_GRP VpssGrp, CVI_U8 *scene)
{


	return CVI_SUCCESS;
}

CVI_VOID _vpss_pack_fixed_point_norm(VPSS_NORMALIZE_S *pstNormalize)
{
	CVI_U16 sc_frac[3];
	CVI_U8 sub[3];
	CVI_U16 sub_frac[3];
	CVI_DOUBLE sub_int;
	struct vpss_int_normalize *int_norm;

	if (!pstNormalize->bEnable)
		return;

	for (CVI_U8 i = 0; i < 3; ++i) {
		sc_frac[i] = pstNormalize->factor[i] * 8192;
		sub_frac[i]
			= modf(pstNormalize->mean[i], &sub_int) * 1024;
		sub[i] = sub_int;
	}

	int_norm = (struct vpss_int_normalize *)pstNormalize;
	int_norm->enable = 1;
	int_norm->rounding = pstNormalize->rounding;

	for (CVI_U8 i = 0; i < 3; ++i) {
		int_norm->sc_frac[i] = sc_frac[i];
		int_norm->sub[i] = sub[i];
		int_norm->sub_frac[i] = sub_frac[i];
	}
}

/* Chn Settings */
CVI_S32 platform_vpss_setchnattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 ret;
	struct vpss_chn_cfg attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memcpy(&attr.stChnAttr,  pstChnAttr, sizeof(attr.stChnAttr));
	// Handle float poing in user space
	_vpss_check_normalize(&attr.stChnAttr);

	if (sizeof(struct vpss_int_normalize) > sizeof(VPSS_NORMALIZE_S)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Cannnot use float\n");
		return CVI_FAILURE;
	}

	_vpss_pack_fixed_point_norm(&attr.stChnAttr.stNormalize);

	ret = driver_vpss_ioctl(VPSS_SET_CHN_ATTR, (unsigned long)&attr);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn attr fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 ret;
	struct vpss_chn_cfg attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_ATTR, (unsigned long)&attr);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*pstChnAttr = attr.stChnAttr;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_enablechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 ret;
	struct vpss_en_chn_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = driver_vpss_ioctl(VPSS_ENABLE_CHN, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) enable fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_disablechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 ret;
	struct vpss_en_chn_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = driver_vpss_ioctl(VPSS_DISABLE_CHN, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) disable fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchncrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 ret;
	struct vpss_chn_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stCropInfo = *pstCropInfo;

	ret = driver_vpss_ioctl(VPSS_SET_CHN_CROP, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn crop fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchncrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 ret;
	struct vpss_chn_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_CROP, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn crop fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*pstCropInfo = cfg.stCropInfo;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnrotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation)
{
	CVI_S32 ret;
	struct vpss_chn_rot_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enRotation = enRotation;

	ret = driver_vpss_ioctl(VPSS_SET_CHN_ROTATION, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn rotation fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnrotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation)
{
	CVI_S32 ret;
	struct vpss_chn_rot_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penRotation);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_ROTATION, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn rotation fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*penRotation = cfg.enRotation;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnldcattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	return s32Ret;
}

CVI_S32 platform_vpss_setchnldcmeshattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VPSS_LDC_ATTR_S *pstLDCAttr, CVI_U64 Paddr)
{
	CVI_S32 s32Ret;
	struct vpss_chn_rot_cfg rot_cfg;
	struct vpss_chn_ldc_cfg ldc_cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	osal_memset(&rot_cfg, 0, sizeof(rot_cfg));
	rot_cfg.VpssGrp = VpssGrp;
	rot_cfg.VpssChn = VpssChn;

	s32Ret = driver_vpss_ioctl(VPSS_GET_CHN_ROTATION, (unsigned long)&rot_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn rotation fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	osal_memset(&ldc_cfg, 0, sizeof(ldc_cfg));
	ldc_cfg.VpssGrp = VpssGrp;
	ldc_cfg.VpssChn = VpssChn;
	ldc_cfg.enRotation = rot_cfg.enRotation;
	ldc_cfg.stLDCAttr = *(VPSS_LDC_ATTR_S *)pstLDCAttr;
	ldc_cfg.meshHandle = Paddr;
	s32Ret = driver_vpss_ioctl(VPSS_SET_CHN_LDC, (unsigned long)&ldc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn ldc fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vpss_getchnldcattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 ret;
	struct vpss_chn_ldc_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_LDC, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn LDC attr fail\n", VpssGrp, VpssChn);
		return ret;
	}

	memcpy(pstLDCAttr, &cfg.stLDCAttr, sizeof(*pstLDCAttr));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_sendchnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	osal_memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));
	cfg.s32MilliSec = s32MilliSec;

	ret = driver_vpss_ioctl(VPSS_SEND_CHN_FRAME, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) send chn frame fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
			CVI_S32 s32MilliSec)
{
	CVI_S32 ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstFrameInfo);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.s32MilliSec = s32MilliSec;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_FRAME, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn frame fail\n", VpssGrp, VpssChn);
		return ret;
	}
	osal_memcpy(pstFrameInfo, &cfg.stVideoFrame, sizeof(*pstFrameInfo));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_releasechnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	osal_memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));

	ret = driver_vpss_ioctl(VPSS_RELEASE_CHN_FRAME, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) release chn frame fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_triggersnapframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32FrameCnt)
{
	CVI_S32 ret;
	struct vpss_snap_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn, .frame_cnt = u32FrameCnt};

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = driver_vpss_ioctl(VPSS_TRIGGER_SNAP_FRAME, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) Trigger Snap Frame fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_attachvbpool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool)
{
	CVI_S32 ret;
	struct vpss_vb_pool_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.hVbPool = hVbPool;
	return driver_vpss_ioctl(VPSS_ATTACH_VB_POOL, (unsigned long)&cfg);
}

CVI_S32 platform_vpss_detachvbpool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 ret;
	struct vpss_vb_pool_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	return driver_vpss_ioctl(VPSS_DETACH_VB_POOL, (unsigned long)&cfg);
}

CVI_S32 platform_vpss_setchnalign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align)
{
	CVI_S32 ret;
	struct vpss_chn_align_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.u32Align = u32Align;

	ret = driver_vpss_ioctl(VPSS_SET_CHN_ALIGN, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn align fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnalign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align)
{
	CVI_S32 ret;
	struct vpss_chn_align_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_ALIGN, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn align fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*pu32Align = cfg.u32Align;

	return CVI_SUCCESS;
}

/* platform_vpss_SetChnYRatio: Modify the y ratio of chn output. Only work for yuv format.
 *
 * @param VpssGrp: The Vpss Grp to work.
 * @param VpssChn: The Vpss Chn to work.
 * @param YRatio: Output's Y will be sacled by this ratio.
 * @return: CVI_SUCCESS if OK.
 */
CVI_S32 platform_vpss_setchnyratio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT YRatio)
{
	CVI_S32 ret;
	struct vpss_chn_yratio_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.YRatio = (CVI_U32)(YRatio * 100);

	ret = driver_vpss_ioctl(VPSS_SET_CHN_YRATIO, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn Y Ratio fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnyratio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio)
{
	CVI_S32 ret;
	struct vpss_chn_yratio_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pYRatio);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_YRATIO, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn Y Ratio fail\n", VpssGrp, VpssChn);
		return ret;
	}
	*pYRatio = (1.0f * cfg.YRatio) / 100.0;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnscalecoeflevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef)
{

	CVI_S32 ret;
	struct vpss_chn_coef_level_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enCoef = enCoef;

	ret = driver_vpss_ioctl(VPSS_SET_CHN_SCALE_COEFF_LEVEL, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn ScaleCoefLevel fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnscalecoeflevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef)
{
	CVI_S32 ret;
	struct vpss_chn_coef_level_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penCoef);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_SCALE_COEFF_LEVEL, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn ScaleCoefLevel fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*penCoef = cfg.enCoef;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchndrawrect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_DRAW_RECT_S *pstDrawRect)
{

	CVI_S32 ret;
	struct vpss_chn_draw_rect_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stDrawRect = *pstDrawRect;

	ret = driver_vpss_ioctl(VPSS_SET_CHN_DRAW_RECT, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set draw rect fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchndrawrect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_DRAW_RECT_S *pstDrawRect)
{
	CVI_S32 ret;
	struct vpss_chn_draw_rect_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_DRAW_RECT, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get draw rect fail\n", VpssGrp, VpssChn);
		return ret;
	}
	*pstDrawRect = cfg.stDrawRect;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnconvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CONVERT_S *pstConvert)
{
	CVI_S32 ret;
	struct vpss_chn_convert_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stConvert = *pstConvert;

	ret = driver_vpss_ioctl(VPSS_SET_CHN_CONVERT, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set convert fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnconvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CONVERT_S *pstConvert)
{
	CVI_S32 ret;
	struct vpss_chn_convert_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_GET_CHN_CONVERT, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get convert fail\n", VpssGrp, VpssChn);
		return ret;
	}
	*pstConvert = cfg.stConvert;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnbufwrapattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
		const VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	CVI_S32 s32Ret;
	struct vpss_chn_wrap_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.wrap = *pstVpssChnBufWrap;

	s32Ret = driver_vpss_ioctl(VPSS_SET_CHN_WRAP, (unsigned long)&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn wrap fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnbufwrapattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
		VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	CVI_S32 s32Ret;
	struct vpss_chn_wrap_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = driver_vpss_ioctl(VPSS_GET_CHN_WRAP, (unsigned long)&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn Wrap Attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}
	osal_memcpy(pstVpssChnBufWrap, &cfg.wrap, sizeof(*pstVpssChnBufWrap));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_showchn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{

	CVI_S32 ret;
	struct vpss_en_chn_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_SHOW_CHN, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) show chn fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_hidechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 ret;
	struct vpss_en_chn_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = driver_vpss_ioctl(VPSS_HIDE_CHN, (unsigned long)&cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) hide chn fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_createstitch(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	CVI_S32 s32Ret;
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN_ATTR_S stChnAttr;

	if (!pstStitchAttr) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "NULL pointer\n");
		return CVI_FAILURE;
	}

	if (s_StitchCtx[VpssGrp].bUse)
		return CVI_SUCCESS;
	if (!pstStitchAttr->s32OutFps) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "s32OutFps cannot be 0\n");
		return CVI_FAILURE;
	}
	memset(&s_StitchCtx[VpssGrp], 0, sizeof(struct cvi_stitch_ctx));

	stGrpAttr.u32MaxW = pstStitchAttr->stOutSize.u32Width; //invalid
	stGrpAttr.u32MaxH = pstStitchAttr->stOutSize.u32Height; //invalid
	stGrpAttr.enPixelFormat = pstStitchAttr->enOutPixelFormat; //invalid
	stGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stGrpAttr.u8VpssDev = 0;
	s32Ret = platform_vpss_creategrp(VpssGrp, &stGrpAttr);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	stChnAttr.u32Width = pstStitchAttr->stOutSize.u32Width;
	stChnAttr.u32Height = pstStitchAttr->stOutSize.u32Height;
	stChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stChnAttr.enPixelFormat = pstStitchAttr->enOutPixelFormat;
	stChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stChnAttr.stFrameRate.s32DstFrameRate = -1;
	stChnAttr.bMirror = CVI_FALSE;
	stChnAttr.bFlip = CVI_FALSE;
	stChnAttr.u32Depth = 1;
	stChnAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stChnAttr.stNormalize.bEnable = CVI_FALSE;
	s32Ret = platform_vpss_setchnattr(VpssGrp, 0, &stChnAttr);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = platform_vpss_enablechn(VpssGrp, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = platform_vpss_startgrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (pstStitchAttr->hVbPool == VB_INVALID_POOLID) {
		VB_POOL_CONFIG_S stVbPoolCfg;
		VB_POOL chnVbPool;
		CVI_U32 u32BlkSize = 0;

		u32BlkSize = COMMON_GetPicBufferSize(stChnAttr.u32Width, stChnAttr.u32Height, stChnAttr.enPixelFormat,
			DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

		memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
		stVbPoolCfg.u32BlkSize	= u32BlkSize;
		stVbPoolCfg.u32BlkCnt	= 1;
		stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
		chnVbPool = CVI_VB_CreatePool(&stVbPoolCfg);
		if (chnVbPool == VB_INVALID_POOLID) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_VB_CreatePool failed.\n");
		} else {
			platform_vpss_attachvbpool(VpssGrp, 0, chnVbPool);
			s_StitchCtx[VpssGrp].VbPool = chnVbPool;
		}
	} else {
		platform_vpss_attachvbpool(VpssGrp, 0, pstStitchAttr->hVbPool);
	}

	pthread_mutex_init(&s_StitchCtx[VpssGrp].lock, NULL);
	s_StitchCtx[VpssGrp].stStitchAttr = *pstStitchAttr;
	s_StitchCtx[VpssGrp].VpssGrp = VpssGrp;
	s_StitchCtx[VpssGrp].bUse = CVI_TRUE;

	return CVI_SUCCESS;

}

CVI_S32 platform_vpss_destroystitch(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;

	if (!s_StitchCtx[VpssGrp].bUse)
		return CVI_SUCCESS;

	s32Ret = platform_vpss_disablechn(VpssGrp, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = platform_vpss_stopgrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = platform_vpss_destroygrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (s_StitchCtx[VpssGrp].stStitchAttr.hVbPool == VB_INVALID_POOLID) {
		CVI_VB_DestroyPool(s_StitchCtx[VpssGrp].VbPool);
	}

	pthread_mutex_destroy(&s_StitchCtx[VpssGrp].lock);
	s_StitchCtx[VpssGrp].bUse = CVI_FALSE;

	return CVI_SUCCESS;

}

CVI_S32 platform_vpss_setstitchattr(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	if (!pstStitchAttr) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "NULL pointer\n");
		return CVI_FAILURE;
	}

	if (!s_StitchCtx[VpssGrp].bUse) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "vpss(%d), The stitch not created\n", VpssGrp);
		return CVI_FAILURE;
	}
	if (!pstStitchAttr->s32OutFps) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "s32OutFps cannot be 0\n");
		return CVI_FAILURE;
	}

	pthread_mutex_lock(&s_StitchCtx[VpssGrp].lock);
	s_StitchCtx[VpssGrp].stStitchAttr = *pstStitchAttr;
	pthread_mutex_unlock(&s_StitchCtx[VpssGrp].lock);

	return CVI_SUCCESS;

}

CVI_S32 platform_vpss_getstitchattr(VPSS_GRP VpssGrp, CVI_STITCH_ATTR_S *pstStitchAttr)
{
	if (!pstStitchAttr) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "NULL pointer\n");
		return CVI_FAILURE;
	}

	if (!s_StitchCtx[VpssGrp].bUse) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "vpss(%d), The stitch not created\n", VpssGrp);
		return CVI_FAILURE;
	}
	*pstStitchAttr = s_StitchCtx[VpssGrp].stStitchAttr;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_startstitch(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	struct sched_param tsk;
	pthread_attr_t attr;
	char thread_name[32];
	struct cvi_stitch_ctx *pStitchCtx;

	if (!s_StitchCtx[VpssGrp].bUse) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "vpss(%d), The stitch not created\n", VpssGrp);
		return CVI_FAILURE;
	}
	pStitchCtx = &s_StitchCtx[VpssGrp];
	if (pStitchCtx->bStart)
		return CVI_SUCCESS;

	//drop 2 frame
	pStitchCtx->DropFrame = 2;
	pStitchCtx->bStart = CVI_TRUE;

	//set vpss thread attr
	tsk.sched_priority = 37;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	pthread_attr_setschedparam(&attr, &tsk);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	s32Ret = pthread_create(&pStitchCtx->thread, &attr,
		pStitchCtx->stStitchAttr.s32OutFps < 0 ? _vpss_stitch_handler : _vpss_stitch_handler_frc,
		(void *)pStitchCtx);
	if (s32Ret != 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "failed to create stitch pthread.\n");
		pStitchCtx->bStart = CVI_FALSE;
		return CVI_FAILURE;
	}
	snprintf(thread_name, 31, "stitch-%d", VpssGrp);
	pthread_setname_np(pStitchCtx->thread, thread_name);

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_stopstitch(VPSS_GRP VpssGrp)
{
	struct cvi_stitch_ctx *pStitchCtx;

	if (!s_StitchCtx[VpssGrp].bUse) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "vpss(%d), The stitch not created\n", VpssGrp);
		return CVI_FAILURE;
	}
	pStitchCtx = &s_StitchCtx[VpssGrp];
	if (!pStitchCtx->bStart)
		return CVI_SUCCESS;

	pStitchCtx->bStart = CVI_FALSE;
	if ((pStitchCtx->stStitchAttr.s32OutFps > 0) &&
		aos_event_is_valid(&pStitchCtx->wait))
		aos_event_set(&pStitchCtx->wait, 0x1, AOS_EVENT_OR);
	pthread_join(pStitchCtx->thread, NULL);
	pStitchCtx->thread = NULL;

	return CVI_SUCCESS;
}
