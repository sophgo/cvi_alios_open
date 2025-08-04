#include "cvi_buffer.h"
#include "cvi_debug.h"
#include "cvi_errno.h"

#include "platform_vpss.h"


CVI_S32 CVI_VPSS_SetMode(const VPSS_MODE_S *pstVPSSMode)
{
	return platform_vpss_setmode(pstVPSSMode);
}

CVI_S32 CVI_VPSS_GetMode(VPSS_MODE_S *pstVPSSMode)
{
	return platform_vpss_getmode(pstVPSSMode);
}

CVI_S32 CVI_VPSS_CreateGrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	return platform_vpss_creategrp(VpssGrp, pstGrpAttr);
}

CVI_S32 CVI_VPSS_DestroyGrp(VPSS_GRP VpssGrp)
{
	return platform_vpss_destroygrp(VpssGrp);
}

VPSS_GRP CVI_VPSS_GetAvailableGrp(CVI_VOID)
{
	return platform_vpss_getavailablegrp();
}

CVI_S32 CVI_VPSS_StartGrp(VPSS_GRP VpssGrp)
{
	return platform_vpss_startgrp(VpssGrp);
}

CVI_S32 CVI_VPSS_StopGrp(VPSS_GRP VpssGrp)
{
	return platform_vpss_stopgrp(VpssGrp);
}

CVI_S32 CVI_VPSS_ResetGrp(VPSS_GRP VpssGrp)
{
	return platform_vpss_resetgrp(VpssGrp);
}

CVI_S32 CVI_VPSS_GetGrpAttr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr)
{
	return platform_vpss_getgrpattr(VpssGrp, pstGrpAttr);
}

CVI_S32 CVI_VPSS_SetGrpAttr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	return platform_vpss_setgrpattr(VpssGrp, pstGrpAttr);
}

CVI_S32 CVI_VPSS_GetGrpCrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo)
{
	return platform_vpss_getgrpcrop(VpssGrp, pstCropInfo);
}

CVI_S32 CVI_VPSS_SetGrpCrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo)
{
	return platform_vpss_setgrpcrop(VpssGrp, pstCropInfo);
}

CVI_S32 CVI_VPSS_SendFrame(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	return platform_vpss_sendframe(VpssGrp, pstVideoFrame, s32MilliSec);
}

CVI_S32 CVI_VPSS_GetGrpProcAmpCtrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	return platform_vpss_getgrpprocampctrl(VpssGrp, type, ctrl);
}

CVI_S32 CVI_VPSS_GetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value)
{
	return platform_vpss_getgrpprocamp(VpssGrp, type, value);
}

CVI_S32 CVI_VPSS_SetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 value)
{
	return platform_vpss_setgrpprocamp(VpssGrp, type, value);
}

CVI_S32 CVI_VPSS_GetAllProcAmp(VPSS_ALL_PROC_AMP_S *pstProcAmp)
{
	return platform_vpss_getallprocamp(pstProcAmp);
}

CVI_S32 CVI_VPSS_SetGrpParamfromBin(VPSS_GRP VpssGrp, VPSS_BIN_DATA *bin_data)
{
	return platform_vpss_setgrpparamfrombin(VpssGrp, bin_data);
}

CVI_S32 CVI_VPSS_GetBinScene(VPSS_GRP VpssGrp, CVI_U8 *scene)
{
	return platform_vpss_getbinscene(VpssGrp, scene);
}

CVI_S32 CVI_VPSS_SetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr)
{
	return platform_vpss_setchnattr(VpssGrp, VpssChn, pstChnAttr);
}

CVI_S32 CVI_VPSS_GetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr)
{
	return platform_vpss_getchnattr(VpssGrp, VpssChn, pstChnAttr);
}

CVI_S32 CVI_VPSS_EnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	return platform_vpss_enablechn(VpssGrp, VpssChn);
}

CVI_S32 CVI_VPSS_DisableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	return platform_vpss_disablechn(VpssGrp, VpssChn);
}

CVI_S32 CVI_VPSS_SetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo)
{
	return platform_vpss_setchncrop(VpssGrp, VpssChn, pstCropInfo);
}

CVI_S32 CVI_VPSS_GetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo)
{
	return platform_vpss_getchncrop(VpssGrp, VpssChn, pstCropInfo);
}

CVI_S32 CVI_VPSS_SetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation)
{
	return platform_vpss_setchnrotation(VpssGrp, VpssChn, enRotation);
}

CVI_S32 CVI_VPSS_GetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation)
{
	return platform_vpss_getchnrotation(VpssGrp, VpssChn, penRotation);
}

CVI_S32 CVI_VPSS_SetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr)
{
	return platform_vpss_setchnldcattr(VpssGrp, VpssChn, pstLDCAttr);
}

CVI_S32 CVI_VPSS_GetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr)
{
	return platform_vpss_getchnldcattr(VpssGrp, VpssChn, pstLDCAttr);
}

CVI_S32 CVI_VPSS_SendChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	return platform_vpss_sendchnframe(VpssGrp, VpssChn, pstVideoFrame, s32MilliSec);
}

CVI_S32 CVI_VPSS_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstVideoFrame,
				 CVI_S32 s32MilliSec)
{
	return platform_vpss_getchnframe(VpssGrp, VpssChn, pstVideoFrame, s32MilliSec);
}

CVI_S32 CVI_VPSS_ReleaseChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	return platform_vpss_releasechnframe(VpssGrp, VpssChn, pstVideoFrame);
}

CVI_S32 CVI_VPSS_AttachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool)
{
	return platform_vpss_attachvbpool(VpssGrp, VpssChn, hVbPool);
}

CVI_S32 CVI_VPSS_DetachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	return platform_vpss_detachvbpool(VpssGrp, VpssChn);
}

CVI_S32 CVI_VPSS_SetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align)
{
	return platform_vpss_setchnalign(VpssGrp, VpssChn, u32Align);
}

CVI_S32 CVI_VPSS_GetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align)
{
	return platform_vpss_getchnalign(VpssGrp, VpssChn, pu32Align);
}

CVI_S32 CVI_VPSS_SetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT YRatio)
{
	return platform_vpss_setchnyratio(VpssGrp, VpssChn, YRatio);
}

CVI_S32 CVI_VPSS_GetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio)
{
	return platform_vpss_getchnyratio(VpssGrp, VpssChn, pYRatio);
}

CVI_S32 CVI_VPSS_SetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef)
{
	return platform_vpss_setchnscalecoeflevel(VpssGrp, VpssChn, enCoef);
}

CVI_S32 CVI_VPSS_GetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef)
{
	return platform_vpss_getchnscalecoeflevel(VpssGrp, VpssChn, penCoef);
}

CVI_S32 CVI_VPSS_SetChnDrawRect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_DRAW_RECT_S *pstDrawRect)
{
	return platform_vpss_setchndrawrect(VpssGrp, VpssChn, pstDrawRect);
}

CVI_S32 CVI_VPSS_GetChnDrawRect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_DRAW_RECT_S *pstDrawRect)
{
	return platform_vpss_getchndrawrect(VpssGrp, VpssChn, pstDrawRect);
}

CVI_S32 CVI_VPSS_SetChnConvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CONVERT_S *pstConvert)
{
	return platform_vpss_setchnconvert(VpssGrp, VpssChn, pstConvert);
}

CVI_S32 CVI_VPSS_GetChnConvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CONVERT_S *pstConvert)
{
	return platform_vpss_getchnconvert(VpssGrp, VpssChn, pstConvert);
}

CVI_S32 CVI_VPSS_SetChnBufWrapAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
		const VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	return platform_vpss_setchnbufwrapattr(VpssGrp, VpssChn, pstVpssChnBufWrap);
}

CVI_S32 CVI_VPSS_GetChnBufWrapAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
		VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	return platform_vpss_getchnbufwrapattr(VpssGrp, VpssChn, pstVpssChnBufWrap);
}

CVI_U32 CVI_VPSS_GetWrapBufferSize(CVI_U32 u32Width, CVI_U32 u32Height, PIXEL_FORMAT_E enPixelFormat,
	CVI_U32 u32BufLine, CVI_U32 u32BufDepth)
{
	CVI_U32 u32BufSize;
	VB_CAL_CONFIG_S stCalConfig;

	if (u32Width < 64 || u32Height < 64) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "width(%d) or height(%d) too small\n", u32Width, u32Height);
		return 0;
	}
	if (u32BufLine != 64 && u32BufLine != 128) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "u32BufLine(%d) invalid, only 64 or 128 lines\n",
				u32BufLine);
		return 0;
	}
	if (u32BufDepth < 2 || u32BufDepth > 32) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "u32BufDepth(%d) invalid, 2 ~ 32\n",
				u32BufDepth);
		return 0;
	}

	COMMON_GetPicBufferConfig(u32Width, u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stCalConfig);

	u32BufSize = stCalConfig.u32VBSize / u32Height;
	u32BufSize *= u32BufLine * u32BufDepth;
	CVI_TRACE_VPSS(CVI_DBG_INFO, "width(%d), height(%d), u32BufSize=%d\n",
		   u32Width, u32Height, u32BufSize);

	return u32BufSize;
}

CVI_S32 CVI_VPSS_ShowChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	return platform_vpss_showchn(VpssGrp, VpssChn);
}

CVI_S32 CVI_VPSS_HideChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	return platform_vpss_hidechn(VpssGrp, VpssChn);
}

CVI_S32 CVI_VPSS_CreateStitch(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	return platform_vpss_createstitch(VpssGrp, pstStitchAttr);
}

CVI_S32 CVI_VPSS_DestroyStitch(VPSS_GRP VpssGrp)
{
	return platform_vpss_destroystitch(VpssGrp);
}

CVI_S32 CVI_VPSS_SetStitchAttr(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	return platform_vpss_setstitchattr(VpssGrp, pstStitchAttr);
}

CVI_S32 CVI_VPSS_GetStitchAttr(VPSS_GRP VpssGrp, CVI_STITCH_ATTR_S *pstStitchAttr)
{
	return platform_vpss_getstitchattr(VpssGrp, pstStitchAttr);
}

CVI_S32 CVI_VPSS_StartStitch(VPSS_GRP VpssGrp)
{
	return platform_vpss_startstitch(VpssGrp);
}

CVI_S32 CVI_VPSS_StopStitch(VPSS_GRP VpssGrp)
{
	return platform_vpss_stopstitch(VpssGrp);
}

