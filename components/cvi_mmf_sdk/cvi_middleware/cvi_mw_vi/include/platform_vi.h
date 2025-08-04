#ifndef __PLATFORM_VI_H__
#define __PLATFORM_VI_H__

#include <stdio.h>
#include <cvi_comm_vi.h>
#include <cvi_comm_vb.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

CVI_S32 platform_vi_suspend(void);
CVI_S32 platform_vi_resume(void);
CVI_S32 platform_vi_setdevnum(CVI_U32 devNum);
CVI_S32 platform_vi_getdevnum(CVI_U32 *devNum);
CVI_S32 platform_vi_querydevstatus(VI_PIPE ViPipe);
CVI_S32 platform_vi_enablepatgen(VI_DEV Videv);
CVI_S32 platform_vi_setdevattr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr);
CVI_S32 platform_vi_getdevattr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr);
CVI_S32 platform_vi_setdevattrex(VI_DEV ViDev, const VI_DEV_ATTR_EX_S *pstDevAttrEx);
CVI_S32 platform_vi_getdevattrex(VI_DEV ViDev, VI_DEV_ATTR_EX_S *pstDevAttrEx);
CVI_S32 platform_vi_setdevbindattr(VI_DEV ViDev, const VI_DEV_BIND_PIPE_S *pstDevBindAttr);
CVI_S32 platform_vi_getdevbindattr(VI_DEV ViDev, VI_DEV_BIND_PIPE_S *pstDevBindAttr);
CVI_S32 platform_vi_setdevunbindattr(VI_DEV ViDev);
CVI_S32 platform_vi_enabledev(VI_DEV ViDev);
CVI_S32 platform_vi_disabledev(VI_DEV ViDev);
CVI_S32 platform_vi_setdevtimingattr(VI_DEV ViDev, const VI_DEV_TIMING_ATTR_S *pstTimingAttr);
CVI_S32 platform_vi_getdevtimingattr(VI_DEV ViDev, VI_DEV_TIMING_ATTR_S *pstTimingAttr);
CVI_S32 platform_vi_createpipe(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr);
CVI_S32 platform_vi_destroypipe(VI_PIPE ViPipe);
CVI_S32 platform_vi_setpipeattr(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr);
CVI_S32 platform_vi_getpipeattr(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr);
CVI_S32 platform_vi_startpipe(VI_PIPE ViPipe);
CVI_S32 platform_vi_stoppipe(VI_PIPE ViPipe);
CVI_S32 platform_vi_setpipecrop(VI_PIPE ViPipe, const CROP_INFO_S *pstCropInfo);
CVI_S32 platform_vi_getpipecrop(VI_PIPE ViPipe, CROP_INFO_S *pstCropInfo);
CVI_S32 platform_vi_setpipedumpattr(VI_PIPE ViPipe, const VI_DUMP_ATTR_S *pstDumpAttr);
CVI_S32 platform_vi_getpipedumpattr(VI_PIPE ViPipe, VI_DUMP_ATTR_S *pstDumpAttr);
CVI_S32 platform_vi_setpipeframesource(VI_PIPE ViPipe, const VI_PIPE_FRAME_SOURCE_E enSource);
CVI_S32 platform_vi_getpipeframesource(VI_PIPE ViPipe, VI_PIPE_FRAME_SOURCE_E *penSource);
CVI_S32 platform_vi_getpipeframe(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);
CVI_S32 platform_vi_releasepipeframe(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 platform_vi_startsmoothrawdump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo);
CVI_S32 platform_vi_stopsmoothrawdump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo);
CVI_S32 platform_vi_getsmoothrawdump(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);
CVI_S32 platform_vi_putsmoothrawdump(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 platform_vi_sendpiperaw(CVI_U32 u32PipeNum, VI_PIPE PipeId[],
				const VIDEO_FRAME_INFO_S *pstVideoFrame[], CVI_S32 s32MilliSec);
CVI_S32 platform_vi_querypipestatus(VI_PIPE ViPipe, VI_PIPE_STATUS_S *pstStatus);
CVI_S32 platform_vi_getpipefd(VI_PIPE ViPipe);
CVI_S32 platform_vi_dumphwregistertofile(VI_PIPE ViPipe, FILE *fp, VI_DUMP_REGISTER_TABLE_S *pstRegTbl);
CVI_S32 platform_vi_attachvbpool(VI_PIPE ViPipe, VI_CHN ViChn, VB_POOL VbPool);
CVI_S32 platform_vi_detachvbpool(VI_PIPE ViPipe, VI_CHN ViChn);
CVI_S32 platform_vi_setchnattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr);
CVI_S32 platform_vi_getchnattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr);
CVI_S32 platform_vi_enablechn(VI_PIPE ViPipe, VI_CHN ViChn);
CVI_S32 platform_vi_disablechn(VI_PIPE ViPipe, VI_CHN ViChn);
CVI_S32 platform_vi_setchncrop(VI_PIPE ViPipe, VI_CHN ViChn, const VI_CROP_INFO_S *pstCropInfo);
CVI_S32 platform_vi_getchncrop(VI_PIPE ViPipe, VI_CHN ViChn, VI_CROP_INFO_S *pstCropInfo);
CVI_S32 platform_vi_setchnrotation(VI_PIPE ViPipe, VI_CHN ViChn, const ROTATION_E enRotation);
CVI_S32 platform_vi_getchnrotation(VI_PIPE ViPipe, VI_CHN ViChn, ROTATION_E *penRotation);
CVI_S32 platform_vi_setchnldcattr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr);
CVI_S32 platform_vi_setchnldcmeshattr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr, CVI_U64 Paddr);
CVI_S32 platform_vi_getchnldcattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_LDC_ATTR_S *pstLDCAttr);
CVI_S32 platform_vi_getchnframe(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
CVI_S32 platform_vi_releasechnframe(VI_PIPE ViPipe, VI_CHN ViChn, const VIDEO_FRAME_INFO_S *pstFrameInfo);
CVI_S32 platform_vi_querychnstatus(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_STATUS_S *pstChnStatus);
CVI_S32 platform_vi_closefd(void);
CVI_S32 platform_vi_regchnflipmirrorcallback(VI_PIPE ViPipe, VI_DEV ViDev, void *pvData);
CVI_S32 platform_vi_unregchnflipmirrorcallback(VI_PIPE ViPipe, VI_DEV ViDev);
CVI_S32 platform_vi_setchnflipmirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL bFlip, CVI_BOOL bMirror);
CVI_S32 platform_vi_getchnflipmirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL *pbFlip, CVI_BOOL *pbMirror);
CVI_S32 platform_vi_regpmcallback(VI_DEV ViDev, VI_PM_OPS_S *pstPmOps, void *pvData);
CVI_S32 platform_vi_unregpmcallback(VI_DEV ViDev);
CVI_S32 platform_vi_trig_ahd(VI_PIPE ViPipe, CVI_U8 u8AHDSignal);
CVI_S32 platform_vi_setextchnattr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_EXT_CHN_ATTR_S *pstExtChnAttr);
CVI_S32 platform_vi_getextchnattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_EXT_CHN_ATTR_S *pstExtChnAttr);
CVI_S32 platform_vi_setbypassfrm(CVI_U32 snr_num, CVI_U8 bypass_num);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__CVI_VI_H__ */
