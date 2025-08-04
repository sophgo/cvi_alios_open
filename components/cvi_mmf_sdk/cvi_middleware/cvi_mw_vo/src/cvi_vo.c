#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>

#include "cvi_debug.h"
#include "cvi_defines.h"
#include "cvi_math.h"
#include "cvi_comm_vo.h"
#include "cvi_errno.h"
#include "vo_uapi.h"
#include "vo_ioctl.h"

#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

static inline int check_vo_dev_valid(int VoDev)
{
	if ((VoDev >= VO_MAX_DEV_NUM) || (VoDev < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) invalid.\n", VoDev);
		return CVI_ERR_VO_INVALID_DEVID;
	}
	return 0;
}

static inline int check_vo_layer_valid(int VoLayer)
{
	if ((VoLayer >= VO_MAX_VIDEO_LAYER_NUM) || (VoLayer < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);
		return CVI_ERR_VO_INVALID_LAYERID;
	}
	return 0;
}

static inline int check_vo_chn_valid(int VoLayer, int VoChn)
{
	if ((VoLayer >= VO_MAX_VIDEO_LAYER_NUM) || (VoLayer < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);
		return CVI_ERR_VO_INVALID_LAYERID;
	}
	if ((VoChn >= VO_MAX_CHN_NUM) || (VoChn < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoChn(%d) invalid.\n", VoChn);
		return CVI_ERR_VO_INVALID_CHNID;
	}
	return 0;
}

struct vo_pm_s {
	VO_PM_OPS_S stOps;
	CVI_VOID	*pvData;
};

static CVI_BOOL bVoOpen = CVI_FALSE;
static pthread_once_t once = PTHREAD_ONCE_INIT;

static struct vo_pm_s apstVoPm[VO_MAX_DEV_NUM] = { 0 };

static CVI_S32 vo_dev_open(CVI_VOID)
{
	if (!bVoOpen) {
		bVoOpen = CVI_TRUE;
		vo_open();
	}

	return CVI_SUCCESS;
}

static CVI_S32 vo_dev_close(CVI_VOID)
{
	if (bVoOpen) {
		bVoOpen = CVI_FALSE;
		vo_release();
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/
#define VO_BIN_GUARDMAGIC 0x12345678
static VO_BIN_INFO_S vo_bin_info[VO_MAX_DEV_NUM] = {
	{
		.gamma_info = {
			.s32VoDev = 0,
			.enable = CVI_FALSE,
			.osd_apply = CVI_FALSE,
			.value = {
				0,   3,   7,   11,  15,  19,  23,  27,
				31,  35,  39,  43,  47,  51,  55,  59,
				63,  67,  71,  75,  79,  83,  87,  91,
				95,  99,  103, 107, 111, 115, 119, 123,
				127, 131, 135, 139, 143, 147, 151, 155,
				159, 163, 167, 171, 175, 179, 183, 187,
				191, 195, 199, 203, 207, 211, 215, 219,
				223, 227, 231, 235, 239, 243, 247, 251,
				255
			}
		},
		.guard_magic = VO_BIN_GUARDMAGIC
	},
};

static PROC_AMP_CTRL_S vp_proc_amp_ctrls[PROC_AMP_MAX] = {
	{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
	{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
	{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
	{ .minimum = 0, .maximum = 359, .step = 1, .default_value = 0 },
};

VO_BIN_INFO_S *get_vo_bin_info_addr(void)
{
	return vo_bin_info;
}

CVI_U32 get_vo_bin_guardmagic_code(void)
{
	return VO_BIN_GUARDMAGIC;
}

void vo_layer_init(void)
{
	CVI_S32 s32Ret;
	struct vo_layer_proc_amp_cfg cfg;
	VO_LAYER VoLayer = 0;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	for (CVI_U8 i = PROC_AMP_BRIGHTNESS; i < PROC_AMP_MAX; ++i)
		cfg.proc_amp[i] = vp_proc_amp_ctrls[i].default_value;

	s32Ret = vo_sdk_set_layer_proc_amp(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Layer proc_amp fail\n", VoLayer);
		return;
	}
}

CVI_S32 CVI_VO_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = -1;

	vo_dev_open();

	s32Ret = vo_sdk_suspend(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "vo sdk suspend fail\n");
		return s32Ret;
	}

	for (VO_DEV VoDev = 0; VoDev < VO_MAX_DEV_NUM; ++VoDev) {
		if (apstVoPm[VoDev].stOps.pfnPanelSuspend) {
			s32Ret = apstVoPm[VoDev].stOps.pfnPanelSuspend(apstVoPm[VoDev].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VO(CVI_DBG_ERR, "Panel[%d] suspend failed with %#x!\n", VoDev, s32Ret);
				return s32Ret;
			}
		}
	}

	CVI_TRACE_VO(CVI_DBG_DEBUG, "-\n");
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = -1;

	vo_dev_open();

	for (VO_DEV VoDev = 0; VoDev < VO_MAX_DEV_NUM; ++VoDev) {
		if (apstVoPm[VoDev].stOps.pfnPanelResume) {
			s32Ret = apstVoPm[VoDev].stOps.pfnPanelResume(apstVoPm[VoDev].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VO(CVI_DBG_ERR, "Panel[%d] resume failed with %#x!\n", VoDev, s32Ret);
				return s32Ret;
			}
		}
	}

	s32Ret = vo_sdk_resume(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "vo sdk resume fail\n");
		return s32Ret;
	}

	CVI_TRACE_VO(CVI_DBG_DEBUG, "-\n");
	return CVI_SUCCESS;
}
/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr)
{
	CVI_S32 s32Ret;
	struct vo_pub_attr_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPubAttr);

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoDev = VoDev;
	CVI_TRACE_VO(CVI_DBG_DEBUG, "VoDev(%d) INTF type(0x%x) Sync(%d)\n", VoDev
		    , pstPubAttr->enIntfType, pstPubAttr->enIntfSync);

	memcpy(&cfg.stPubAttr, pstPubAttr, sizeof(VO_PUB_ATTR_S));

	s32Ret = vo_sdk_set_pubattr(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Set Pub Attr fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	CVI_S32 s32Ret;
	struct vo_pub_attr_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPubAttr);

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoDev = VoDev;
	s32Ret = vo_sdk_get_pubattr(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get Pub Attr fail\n", VoDev);
		return s32Ret;
	}

	memcpy(pstPubAttr, &cfg.stPubAttr, sizeof(VO_PUB_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetLVDSParam(VO_DEV VoDev, const VO_LVDS_ATTR_S *pstLVDSParam)
{
	CVI_S32 s32Ret;
	struct vo_lvds_param_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLVDSParam);

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoDev = VoDev;
	memcpy(&cfg.stLVDSParam, pstLVDSParam, sizeof(VO_LVDS_ATTR_S));

	s32Ret = vo_sdk_set_lvdsparam(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Set LVDS param fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLVDSParam(VO_DEV VoDev, VO_LVDS_ATTR_S *pstLVDSParam)
{
	CVI_S32 s32Ret;
	struct vo_lvds_param_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLVDSParam);

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoDev = VoDev;

	s32Ret = vo_sdk_get_lvdsparam(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get LVDS param fail\n", VoDev);
		return s32Ret;
	}

	memcpy(pstLVDSParam, &cfg.stLVDSParam, sizeof(VO_LVDS_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetBTParam(VO_DEV VoDev, const VO_BT_ATTR_S *pstBTParam)
{
	CVI_S32 s32Ret;
	struct vo_bt_param_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstBTParam);

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoDev = VoDev;
	memcpy(&cfg.stBTParam, pstBTParam, sizeof(VO_BT_ATTR_S));

	s32Ret = vo_sdk_set_btparam(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get BT param fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetBTParam(VO_DEV VoDev, VO_BT_ATTR_S *pstBTParam)
{
	CVI_S32 s32Ret;
	struct vo_bt_param_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstBTParam);

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoDev = VoDev;

	s32Ret = vo_sdk_get_btparam(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get BT param fail\n", VoDev);
		return s32Ret;
	}

	memcpy(pstBTParam, &cfg.stBTParam, sizeof(VO_BT_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Enable(VO_DEV VoDev)
{
	CVI_S32 s32Ret;
	struct vo_dev_cfg cfg;

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoDev = VoDev;
	s32Ret = vo_sdk_enable(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Enable fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Disable(VO_DEV VoDev)
{
	CVI_S32 s32Ret;
	struct vo_dev_cfg cfg;

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoDev = VoDev;
	s32Ret = vo_sdk_disable(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Disable fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_EnableVideoLayer(VO_LAYER VoLayer)
{
	CVI_S32 s32Ret;
	struct vo_video_layer_cfg cfg;

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_enable_videolayer(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Enable Video Layer fail\n", VoLayer);
		return s32Ret;
	}

	pthread_once(&once, vo_layer_init);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_DisableVideoLayer(VO_LAYER VoLayer)
{
	CVI_S32 s32Ret;
	struct vo_video_layer_cfg cfg;

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_disable_videolayer(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Disable Video Layer fail\n", VoLayer);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	CVI_S32 s32Ret;
	struct vo_video_layer_attr_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLayerAttr);

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	memcpy(&cfg.stLayerAttr, pstLayerAttr, sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = vo_sdk_set_videolayerattr(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Video Layer Attr fail\n", VoLayer);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	CVI_S32 s32Ret;
	struct vo_video_layer_attr_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLayerAttr);

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_videolayerattr(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Video Layer Attr fail\n", VoLayer);
		return s32Ret;
	}
	memcpy(pstLayerAttr, &cfg.stLayerAttr, sizeof(VO_VIDEO_LAYER_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLayerProcAmpCtrl(VO_LAYER VoLayer, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, ctrl);

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) ProcAmp type(%d) invalid.\n", VoLayer, type);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	*ctrl = vp_proc_amp_ctrls[type];

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 *value)
{
	CVI_S32 s32Ret;
	struct vo_layer_proc_amp_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, value);

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) ProcAmp type(%d) invalid.\n", VoLayer, type);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_layer_proc_amp(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Layer proc_amp fail\n", VoLayer);
		return s32Ret;
	}

	*value = cfg.proc_amp[type];

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 value)
{
	struct vo_layer_proc_amp_cfg cfg;
	struct vo_layer_csc_cfg layer_csc_cfg;
	CVI_S32 s32Ret;

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) ProcAmp type(%d) invalid.\n", VoLayer, type);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	vo_dev_open();

	if ((value > vp_proc_amp_ctrls[type].maximum) || (value < vp_proc_amp_ctrls[type].minimum)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) new value(%d) out of range(%d ~ %d).\n"
			, VoLayer, value, vp_proc_amp_ctrls[type].minimum, vp_proc_amp_ctrls[type].maximum);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_layer_proc_amp(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Layer proc_amp fail\n", VoLayer);
		return s32Ret;
	}

	cfg.proc_amp[type] = value;
	s32Ret = vo_sdk_set_layer_proc_amp(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Layer proc_amp fail\n", VoLayer);
		return s32Ret;
	}

	layer_csc_cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_layer_csc(&layer_csc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Layer csc fail\n", VoLayer);
		return s32Ret;
	}

	struct disp_csc_matrix csc_cfg;
	CVI_S32 b = (cfg.proc_amp[PROC_AMP_BRIGHTNESS] >> 1) - 64;
	float c = (float)cfg.proc_amp[PROC_AMP_CONTRAST] / 128;
	float s = (float)cfg.proc_amp[PROC_AMP_SATURATION] / 128;
	float h = (float)cfg.proc_amp[PROC_AMP_HUE] * 2 * PI / 360;
	float A = cos(h) * c * s;
	float B = sin(h) * c * s;
	float tmp;

	if (layer_csc_cfg.stVideoCSC.enCscMatrix == VO_CSC_MATRIX_601_LIMIT_YUV2RGB) {
		if (b > 0) {
			csc_cfg.sub[0] = 16;
			csc_cfg.add[0] = b;
			csc_cfg.add[1] = csc_cfg.add[0];
			csc_cfg.add[2] = csc_cfg.add[0];
		} else {
			csc_cfg.sub[0] = 16 + abs(b);
			csc_cfg.add[0] = 0;
			csc_cfg.add[1] = 0;
			csc_cfg.add[2] = 0;
		}
		csc_cfg.sub[1] = 128;
		csc_cfg.sub[2] = 128;

		csc_cfg.coef[0][0] = c * 1192;
		tmp = B * -1.596;
		csc_cfg.coef[0][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = A * 1.596;
		csc_cfg.coef[0][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		csc_cfg.coef[1][0] = c * 1192;
		tmp = A * -0.392 + B * 0.812;
		csc_cfg.coef[1][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = B * -0.392 + A * -0.812;
		csc_cfg.coef[1][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		csc_cfg.coef[2][0] = c * 1192;
		tmp = A * 2.016;
		csc_cfg.coef[2][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = B * 2.016;
		csc_cfg.coef[2][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
	} else if (layer_csc_cfg.stVideoCSC.enCscMatrix == VO_CSC_MATRIX_601_FULL_YUV2RGB) {
		if (b > 0) {
			csc_cfg.sub[0] = 0;
			csc_cfg.add[0] = b;
			csc_cfg.add[1] = csc_cfg.add[0];
			csc_cfg.add[2] = csc_cfg.add[0];
		} else {
			csc_cfg.sub[0] = abs(b);
			csc_cfg.add[0] = 0;
			csc_cfg.add[1] = 0;
			csc_cfg.add[2] = 0;
		}
		csc_cfg.sub[1] = 128;
		csc_cfg.sub[2] = 128;

		csc_cfg.coef[0][0] = c * BIT(10);
		tmp = B * -1.4075;
		csc_cfg.coef[0][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = A * 1.4075;
		csc_cfg.coef[0][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		csc_cfg.coef[1][0] = c * BIT(10);
		tmp = A * -0.3455 + B * 0.7169;
		csc_cfg.coef[1][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = B * -0.3455 + A * -0.7169;
		csc_cfg.coef[1][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		csc_cfg.coef[2][0] = c * BIT(10);
		tmp = A * 1.779;
		csc_cfg.coef[2][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = B * 1.779;
		csc_cfg.coef[2][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	} else if (layer_csc_cfg.stVideoCSC.enCscMatrix == VO_CSC_MATRIX_709_LIMIT_YUV2RGB) {
		if (b > 0) {
			csc_cfg.sub[0] = 16;
			csc_cfg.add[0] = b;
			csc_cfg.add[1] = csc_cfg.add[0];
			csc_cfg.add[2] = csc_cfg.add[0];
		} else {
			csc_cfg.sub[0] = 16 + abs(b);
			csc_cfg.add[0] = 0;
			csc_cfg.add[1] = 0;
			csc_cfg.add[2] = 0;
		}
		csc_cfg.sub[1] = 128;
		csc_cfg.sub[2] = 128;

		csc_cfg.coef[0][0] = c * 1192;
		tmp = B * -1.792;
		csc_cfg.coef[0][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = A * 1.792;
		csc_cfg.coef[0][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		csc_cfg.coef[1][0] = c * 1192;
		tmp = A * -0.213 + B * 0.534;
		csc_cfg.coef[1][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = B * -0.213 + A * -0.534;
		csc_cfg.coef[1][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		csc_cfg.coef[2][0] = c * 1192;
		tmp = A * 2.114;
		csc_cfg.coef[2][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = B * 2.114;
		csc_cfg.coef[2][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
	} else if (layer_csc_cfg.stVideoCSC.enCscMatrix == VO_CSC_MATRIX_709_FULL_YUV2RGB) {
		if (b > 0) {
			csc_cfg.sub[0] = 0;
			csc_cfg.add[0] = b;
			csc_cfg.add[1] = csc_cfg.add[0];
			csc_cfg.add[2] = csc_cfg.add[0];
		} else {
			csc_cfg.sub[0] = abs(b);
			csc_cfg.add[0] = 0;
			csc_cfg.add[1] = 0;
			csc_cfg.add[2] = 0;
		}
		csc_cfg.sub[1] = 128;
		csc_cfg.sub[2] = 128;

		csc_cfg.coef[0][0] = c * BIT(10);
		tmp = B * -1.5748;
		csc_cfg.coef[0][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = A * 1.5748;
		csc_cfg.coef[0][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		csc_cfg.coef[1][0] = c * BIT(10);
		tmp = A * -0.1868 + B * 0.468;
		csc_cfg.coef[1][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = B * -0.1868 + A * -0.468;
		csc_cfg.coef[1][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		csc_cfg.coef[2][0] = c * BIT(10);
		tmp = A * 1.856;
		csc_cfg.coef[2][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = B * 1.856;
		csc_cfg.coef[2][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	}

	vo_set_csc(&csc_cfg, VoLayer);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "coef[0][0]: %#4x coef[0][1]: %#4x coef[0][2]: %#4x\n"
		, csc_cfg.coef[0][0], csc_cfg.coef[0][1]
		, csc_cfg.coef[0][2]);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "coef[1][0]: %#4x coef[1][1]: %#4x coef[1][2]: %#4x\n"
		, csc_cfg.coef[1][0], csc_cfg.coef[1][1]
		, csc_cfg.coef[1][2]);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "coef[2][0]: %#4x coef[2][1]: %#4x coef[2][2]: %#4x\n"
		, csc_cfg.coef[2][0], csc_cfg.coef[2][1]
		, csc_cfg.coef[2][2]);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "sub[0]: %3d sub[1]: %3d sub[2]: %3d\n"
		, csc_cfg.sub[0], csc_cfg.sub[1], csc_cfg.sub[2]);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "add[0]: %3d add[1]: %3d add[2]: %3d\n"
		, csc_cfg.add[0], csc_cfg.add[1], csc_cfg.add[2]);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	struct vo_chn_attr_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnAttr);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	memcpy(&cfg.stChnAttr, pstChnAttr, sizeof(VO_CHN_ATTR_S));
	s32Ret = vo_sdk_set_chnattr(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn Attr fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	struct vo_chn_attr_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnAttr);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chnattr(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn Attr fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(pstChnAttr, &cfg.stChnAttr, sizeof(VO_CHN_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 s32ChnFrmRate)
{
	CVI_S32 s32Ret;
	struct vo_chn_frmrate_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.u32FrameRate = s32ChnFrmRate;
	s32Ret = vo_sdk_set_chn_frmrate(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn FrmRate fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 *ps32ChnFrmRate)
{
	CVI_S32 s32Ret;
	struct vo_chn_frmrate_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, ps32ChnFrmRate);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chn_frmrate(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn FrmRate fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	*ps32ChnFrmRate = cfg.u32FrameRate;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnPTS(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U64 *pu64ChnPTS)
{
	CVI_S32 s32Ret;
	struct vo_chn_pts_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pu64ChnPTS);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chn_pts(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) get chn pts fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	*pu64ChnPTS = cfg.u64ChnPTS;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_QueryChnStatus(VO_LAYER VoLayer, VO_CHN VoChn, VO_QUERY_STATUS_S *pstStatus)
{
	CVI_S32 s32Ret;
	struct vo_chn_status_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstStatus);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chn_status(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) get chn status fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(pstStatus, &cfg.stStatus, sizeof(VO_QUERY_STATUS_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 u32BufLen)
{
	CVI_S32 s32Ret;
	struct vo_display_buflen_cfg cfg;

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.u32BufLen = u32BufLen;
	s32Ret = vo_sdk_set_displaybuflen(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Display BufLen (%d)fail\n", VoLayer, u32BufLen);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 *pu32BufLen)
{
	CVI_S32 s32Ret;
	struct vo_display_buflen_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pu32BufLen);

	s32Ret = check_vo_layer_valid(VoLayer);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_displaybuflen(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Display BufLen (%d)fail\n", VoLayer, cfg.u32BufLen);
		return s32Ret;
	}

	*pu32BufLen = cfg.u32BufLen;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CVI_S32 s32Ret;
	struct vo_chn_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_enable_chn(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Enable Chn(%d) fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CVI_S32 s32Ret;
	struct vo_chn_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_disable_chn(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Disable Chn(%d) fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E enRotation)
{
	CVI_S32 s32Ret;
	struct vo_chn_rotation_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.enRotation = enRotation;
	if (enRotation == ROTATION_180) {
		CVI_TRACE_VO(CVI_DBG_ERR, "not support rotation(%d).\n", enRotation);
		return CVI_ERR_VO_NOT_SUPPORT;
	} else if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}
	s32Ret = vo_sdk_set_chnrotation(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Chn(%d) Rotation fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E *penRotation)
{
	CVI_S32 s32Ret;
	struct vo_chn_rotation_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, penRotation);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_get_chnrotation(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Chn(%d) Rotation fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	*penRotation = cfg.enRotation;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	struct vo_snd_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.s32MilliSec = s32MilliSec;

	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(VIDEO_FRAME_INFO_S));
	s32Ret = vo_sdk_send_frame(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Chn(%d) send frame fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SendLogoFromIon(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	struct vo_snd_frm_cfg cfg;
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.s32MilliSec = s32MilliSec;

	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

	s32Ret = vo_sdk_send_logo_fromIon(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Chn(%d) send frame fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ShowPattern(VO_DEV VoDev, VO_PATTERN_MODE PatternId)
{
	CVI_S32 s32Ret;

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	s32Ret = vo_set_pattern(PatternId, VoDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) set Pattern failed.\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, CVI_BOOL bClrAll)
{
	CVI_S32 s32Ret;
	struct vo_clear_chn_buf_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.bClrAll = bClrAll;
	s32Ret = vo_sdk_clearchnbuf(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Chn(%d) clean buf fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CVI_S32 s32Ret;
	struct vo_chn_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_showchn(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) show chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CVI_S32 s32Ret;
	struct vo_chn_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_hidechn(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) hide chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_CloseFd(void)
{
	vo_dev_close();
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_PauseChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CVI_S32 s32Ret;
	struct vo_chn_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_pausechn(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) pause chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ResumeChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CVI_S32 s32Ret;
	struct vo_chn_cfg cfg;

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_resumechn(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) resume chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Get_Panel_Status(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *is_init)
{
	CVI_S32 s32Ret;
	struct vo_panel_status_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, is_init);

	s32Ret = check_vo_chn_valid(VoLayer, VoChn);
	if (s32Ret)
		return s32Ret;

	vo_dev_open();

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_get_panelstatue(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Chn(%d) get panel status fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(&is_init, &cfg.is_init, sizeof(is_init));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_RegPmCallBack(VO_DEV VoDev, VO_PM_OPS_S *pstPmOps, void *pvData)
{
	CVI_S32 s32Ret;

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPmOps);

	apstVoPm[VoDev].stOps = *pstPmOps;
	apstVoPm[VoDev].pvData = pvData;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_UnRegPmCallBack(VO_DEV VoDev)
{
	CVI_S32 s32Ret;

	s32Ret = check_vo_dev_valid(VoDev);
	if (s32Ret)
		return s32Ret;

	memset(&apstVoPm[VoDev].stOps, 0, sizeof(apstVoPm[VoDev].stOps));
	apstVoPm[VoDev].pvData = NULL;
	return CVI_SUCCESS;
}

CVI_BOOL CVI_VO_IsEnabled(VO_DEV VoDev)
{
	CVI_S32 s32Ret;
	struct vo_dev_cfg cfg;

	if ((VoDev >= VO_MAX_DEV_NUM) || (VoDev < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) invalid.\n", VoDev);
		return CVI_FALSE;
	}

	vo_dev_open();

	cfg.VoDev = VoDev;
	s32Ret = vo_sdk_isenable(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) get vo dev status fail\n", VoDev);
		return CVI_FALSE;
	}

	return (CVI_BOOL)cfg.isEnable;
}

CVI_S32 CVI_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pinfo);

	vo_dev_open();

	if (!CVI_VO_IsEnabled(pinfo->s32VoDev)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) not ready yet!\n", pinfo->s32VoDev);
		return CVI_ERR_VO_SYS_NOTREADY;
	}

	s32Ret = vo_set_gamma_ctrl(pinfo, pinfo->s32VoDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Device(%d) set gamma fail\n", pinfo->s32VoDev);
		return s32Ret;
	}

	memcpy(&vo_bin_info[pinfo->s32VoDev].gamma_info, pinfo, sizeof(VO_GAMMA_INFO_S));
	vo_bin_info[pinfo->s32VoDev].guard_magic = VO_BIN_GUARDMAGIC;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pinfo);

	vo_dev_open();

	if (!CVI_VO_IsEnabled(pinfo->s32VoDev)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) not ready yet!\n", pinfo->s32VoDev);
		return CVI_ERR_VO_SYS_NOTREADY;
	}

	//calling HW
	s32Ret = vo_get_gamma_ctrl(pinfo, pinfo->s32VoDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Device(%d) get gamma fail\n", pinfo->s32VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}
