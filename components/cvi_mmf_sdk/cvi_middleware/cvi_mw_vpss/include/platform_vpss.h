#ifndef __PLATFORM_VPSS_H__
#define __PLATFORM_VPSS_H__

#include "cvi_comm_vb.h"
#include "cvi_comm_vpss.h"


CVI_S32 platform_vpss_setmode(const VPSS_MODE_S *pstVPSSMode);

CVI_S32 platform_vpss_getmode(VPSS_MODE_S *pstVPSSMode);

CVI_S32 platform_vpss_setmode(const VPSS_MODE_S *pstVPSSMode);

CVI_S32 platform_vpss_getmode(VPSS_MODE_S *pstVPSSMode);

CVI_S32 platform_vpss_creategrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr);

CVI_S32 platform_vpss_destroygrp(VPSS_GRP VpssGrp);

VPSS_GRP platform_vpss_getavailablegrp(CVI_VOID);

CVI_S32 platform_vpss_startgrp(VPSS_GRP VpssGrp);

CVI_S32 platform_vpss_stopgrp(VPSS_GRP VpssGrp);

CVI_S32 platform_vpss_resetgrp(VPSS_GRP VpssGrp);

CVI_S32 platform_vpss_getgrpattr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr);

CVI_S32 platform_vpss_setgrpattr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr);

CVI_S32 platform_vpss_getgrpcrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo);

CVI_S32 platform_vpss_setgrpcrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo);

CVI_S32 platform_vpss_sendframe(VPSS_GRP VpssGrp,
	const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

CVI_S32 platform_vpss_getgrpprocampctrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl);

CVI_S32 platform_vpss_getgrpprocamp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value);

CVI_S32 platform_vpss_setgrpprocamp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 value);

CVI_S32 platform_vpss_getallprocamp(VPSS_ALL_PROC_AMP_S *pstProcAmp);

CVI_S32 platform_vpss_setgrpparamfrombin(VPSS_GRP VpssGrp, VPSS_BIN_DATA *bin_data);

CVI_S32 platform_vpss_getbinscene(VPSS_GRP VpssGrp, CVI_U8 *scene);

CVI_S32 platform_vpss_setchnattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr);

CVI_S32 platform_vpss_getchnattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr);

CVI_S32 platform_vpss_enablechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

CVI_S32 platform_vpss_disablechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

CVI_S32 platform_vpss_setchncrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo);

CVI_S32 platform_vpss_getchncrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo);

CVI_S32 platform_vpss_setchnrotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation);

CVI_S32 platform_vpss_getchnrotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation);

CVI_S32 platform_vpss_setchnldcattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr);

CVI_S32 platform_vpss_setchnldcmeshattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VPSS_LDC_ATTR_S *pstLDCAttr, CVI_U64 Paddr);

CVI_S32 platform_vpss_getchnldcattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr);

CVI_S32 platform_vpss_sendchnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

CVI_S32 platform_vpss_getchnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
	CVI_S32 s32MilliSec);

CVI_S32 platform_vpss_releasechnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 platform_vpss_triggersnapframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32FrameCnt);

CVI_S32 platform_vpss_attachvbpool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool);

CVI_S32 platform_vpss_detachvbpool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

CVI_S32 platform_vpss_setchnalign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align);

CVI_S32 platform_vpss_getchnalign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align);

CVI_S32 platform_vpss_setchnyratio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT YRatio);

CVI_S32 platform_vpss_getchnyratio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio);

CVI_S32 platform_vpss_setchnscalecoeflevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef);

CVI_S32 platform_vpss_getchnscalecoeflevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef);

CVI_S32 platform_vpss_setchndrawrect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_DRAW_RECT_S *pstDrawRect);

CVI_S32 platform_vpss_getchndrawrect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_DRAW_RECT_S *pstDrawRect);

CVI_S32 platform_vpss_setchnconvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CONVERT_S *pstConvert);

CVI_S32 platform_vpss_getchnconvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CONVERT_S *pstConvert);

CVI_S32 platform_vpss_setchnbufwrapattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap);

CVI_S32 platform_vpss_getchnbufwrapattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap);

CVI_S32 platform_vpss_showchn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

CVI_S32 platform_vpss_hidechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

//vpss stitch
CVI_S32 platform_vpss_createstitch(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr);

CVI_S32 platform_vpss_destroystitch(VPSS_GRP VpssGrp);

CVI_S32 platform_vpss_setstitchattr(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr);

CVI_S32 platform_vpss_getstitchattr(VPSS_GRP VpssGrp, CVI_STITCH_ATTR_S *pstStitchAttr);

CVI_S32 platform_vpss_startstitch(VPSS_GRP VpssGrp);

CVI_S32 platform_vpss_stopstitch(VPSS_GRP VpssGrp);

#endif
