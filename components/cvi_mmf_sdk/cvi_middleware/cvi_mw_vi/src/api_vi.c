#include "platform_vi.h"
#include "cvi_vi.h"
#include "cvi_debug.h"
#include "cvi_defines.h"

#define CHECK_VI_PIPEID_VALID(x)						\
	do {									\
		if ((x) > (VI_MAX_PIPE_NUM - 1)) {				\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid pipe-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_PIPEID;			\
		}								\
	} while (0)

#define CHECK_VI_DEVID_VALID(x)							\
	do {									\
		if ((x) > (VI_MAX_DEV_NUM - 1)) {				\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid dev-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_DEVID;			\
		}								\
	} while (0)

#define CHECK_VI_CHNID_VALID(x)							\
	do {									\
		if ((x) > (VI_MAX_CHN_NUM - 1)) {				\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid chn-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_CHNID;			\
		}								\
	} while (0)

#define CHECK_VI_NULL_PTR(ptr)							\
	do {									\
		if (ptr == NULL) {						\
			CVI_TRACE_VI(CVI_DBG_ERR, " Invalid null pointer\n");	\
			return CVI_ERR_VI_INVALID_NULL_PTR;			\
		}								\
	} while (0)

CVI_S32 CVI_VI_Suspend(void)
{
	return platform_vi_suspend();
}

CVI_S32 CVI_VI_Resume(void)
{
	return platform_vi_resume();
}

CVI_S32 CVI_VI_SetDevNum(CVI_U32 devNum)
{
	return platform_vi_setdevnum(devNum);
}

CVI_S32 CVI_VI_GetDevNum(CVI_U32 *devNum)
{
	CHECK_VI_NULL_PTR(devNum);
	return platform_vi_getdevnum(devNum);
}

CVI_S32 CVI_VI_QueryDevStatus(VI_PIPE ViPipe)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	return platform_vi_querydevstatus(ViPipe);
}

CVI_S32 CVI_VI_EnablePatgen(VI_DEV ViDev)
{
	CHECK_VI_DEVID_VALID(ViDev);
	return platform_vi_enablepatgen(ViDev);
}

CVI_S32 CVI_VI_SetDevAttr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttr);

	// 检查分辨率是否有效
	if (pstDevAttr->stSize.u32Width > VI_DEV_MAX_WIDTH || pstDevAttr->stSize.u32Height > VI_DEV_MAX_HEIGHT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "width(%d)_height(%d) out of range.\n",
					pstDevAttr->stSize.u32Width,
					pstDevAttr->stSize.u32Height);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstDevAttr->stSize.u32Width == 0 ||
		pstDevAttr->stSize.u32Height == 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "width(%d)_height(%d) is invalid.\n",
					pstDevAttr->stSize.u32Width,
					pstDevAttr->stSize.u32Height);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_setdevattr(ViDev, pstDevAttr);
}

CVI_S32 CVI_VI_GetDevAttr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttr);
	return platform_vi_getdevattr(ViDev, pstDevAttr);
}

CVI_S32 CVI_VI_SetDevAttrEx(VI_DEV ViDev, const VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttrEx);

	if (pstDevAttrEx->phyDev > VI_MAX_PHY_DEV_NUM - 1) {
		CVI_TRACE_VI(CVI_DBG_ERR, "phy_dev(%d) is invalid\n", pstDevAttrEx->phyDev);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_setdevattrex(ViDev, pstDevAttrEx);
}

CVI_S32 CVI_VI_GetDevAttrEx(VI_DEV ViDev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttrEx);

	return platform_vi_getdevattrex(ViDev, pstDevAttrEx);
}

CVI_S32 CVI_VI_SetDevBindAttr(VI_DEV ViDev, const VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevBindAttr);

	if (pstDevBindAttr->u32Num > VI_MAX_PIPE_NUM) {
		CVI_TRACE_VI(CVI_DBG_ERR, "pipe_num(%d) out of range.\n", pstDevBindAttr->u32Num);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstDevBindAttr->u32Num == 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "pipe_num(%d) is invalid.\n", pstDevBindAttr->u32Num);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_setdevbindattr(ViDev, pstDevBindAttr);
}

CVI_S32 CVI_VI_GetDevBindAttr(VI_DEV ViDev, VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevBindAttr);
	return platform_vi_getdevbindattr(ViDev, pstDevBindAttr);
}

CVI_S32 CVI_VI_SetDevUnbindAttr(VI_DEV ViDev)
{
	CHECK_VI_DEVID_VALID(ViDev);
	return platform_vi_setdevunbindattr(ViDev);
}

CVI_S32 CVI_VI_EnableDev(VI_DEV ViDev)
{
	CHECK_VI_DEVID_VALID(ViDev);
	return platform_vi_enabledev(ViDev);
}

CVI_S32 CVI_VI_DisableDev(VI_DEV ViDev)
{
	CHECK_VI_DEVID_VALID(ViDev);
	return platform_vi_disabledev(ViDev);
}

CVI_S32 CVI_VI_SetDevTimingAttr(VI_DEV ViDev, const VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstTimingAttr);
	return platform_vi_setdevtimingattr(ViDev, pstTimingAttr);
}

CVI_S32 CVI_VI_GetDevTimingAttr(VI_DEV ViDev, VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstTimingAttr);
	return platform_vi_getdevtimingattr(ViDev, pstTimingAttr);
}

CVI_S32 CVI_VI_CreatePipe(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	if (pstPipeAttr->u32MaxW > VI_PIPE_OFFLINE_MAX_WIDTH || pstPipeAttr->u32MaxH > VI_PIPE_OFFLINE_MAX_HEIGHT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "width(%d)_height(%d) out of range.\n",
					pstPipeAttr->u32MaxW,
					pstPipeAttr->u32MaxH);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstPipeAttr->u32MaxW == 0 || pstPipeAttr->u32MaxH == 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "width(%d)_height(%d) is invalid.\n",
					pstPipeAttr->u32MaxW,
					pstPipeAttr->u32MaxH);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_createpipe(ViPipe, pstPipeAttr);
}

CVI_S32 CVI_VI_DestroyPipe(VI_PIPE ViPipe)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	return platform_vi_destroypipe(ViPipe);
}

CVI_S32 CVI_VI_SetPipeAttr(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	if (pstPipeAttr->u32MaxW > VI_PIPE_OFFLINE_MAX_WIDTH || pstPipeAttr->u32MaxH > VI_PIPE_OFFLINE_MAX_HEIGHT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "width(%d)_height(%d) out of range.\n",
					pstPipeAttr->u32MaxW,
					pstPipeAttr->u32MaxH);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstPipeAttr->u32MaxW == 0 || pstPipeAttr->u32MaxH == 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "width(%d)_height(%d) is invalid.\n",
					pstPipeAttr->u32MaxW,
					pstPipeAttr->u32MaxH);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_setpipeattr(ViPipe, pstPipeAttr);
}

CVI_S32 CVI_VI_GetPipeAttr(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	return platform_vi_getpipeattr(ViPipe, pstPipeAttr);
}

CVI_S32 CVI_VI_StartPipe(VI_PIPE ViPipe)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	return platform_vi_startpipe(ViPipe);
}

CVI_S32 CVI_VI_StopPipe(VI_PIPE ViPipe)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	return platform_vi_stoppipe(ViPipe);
}

CVI_S32 CVI_VI_SetPipeCrop(VI_PIPE ViPipe, const CROP_INFO_S *pstCropInfo)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	// 检查裁剪参数是否有效
	if (pstCropInfo->stRect.s32X % 2 || pstCropInfo->stRect.s32Y % 2 ||
		pstCropInfo->stRect.u32Width % 2 || pstCropInfo->stRect.u32Height % 2) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d)_w(%d)_h(%d) must be multiple of 2.\n",
					pstCropInfo->stRect.s32X,
					pstCropInfo->stRect.s32Y,
					pstCropInfo->stRect.u32Width,
					pstCropInfo->stRect.u32Height);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstCropInfo->stRect.s32X < 0 || pstCropInfo->stRect.s32Y < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d) is invalid.\n",
					pstCropInfo->stRect.s32X,
					pstCropInfo->stRect.s32Y);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_setpipecrop(ViPipe, pstCropInfo);
}

CVI_S32 CVI_VI_GetPipeCrop(VI_PIPE ViPipe, CROP_INFO_S *pstCropInfo)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);
	return platform_vi_getpipecrop(ViPipe, pstCropInfo);
}

CVI_S32 CVI_VI_SetPipeDumpAttr(VI_PIPE ViPipe, const VI_DUMP_ATTR_S *pstDumpAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstDumpAttr);
	return platform_vi_setpipedumpattr(ViPipe, pstDumpAttr);
}

CVI_S32 CVI_VI_GetPipeDumpAttr(VI_PIPE ViPipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstDumpAttr);
	return platform_vi_getpipedumpattr(ViPipe, pstDumpAttr);
}

CVI_S32 CVI_VI_SetPipeFrameSource(VI_PIPE ViPipe, const VI_PIPE_FRAME_SOURCE_E enSource)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	// 检查帧源类型是否有效
	if (enSource < 0 || enSource >= VI_PIPE_FRAME_SOURCE_BUTT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enSource(%d)is invalid\n", enSource);
		return CVI_ERR_VI_INVALID_PARA;
	}
	return platform_vi_setpipeframesource(ViPipe, enSource);
}

CVI_S32 CVI_VI_GetPipeFrameSource(VI_PIPE ViPipe, VI_PIPE_FRAME_SOURCE_E *penSource)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(penSource);
	return platform_vi_getpipeframesource(ViPipe, penSource);
}

CVI_S32 CVI_VI_GetPipeFrame(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstVideoFrame);
	return platform_vi_getpipeframe(ViPipe, pstVideoFrame, s32MilliSec);
}

CVI_S32 CVI_VI_ReleasePipeFrame(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstVideoFrame);
	return platform_vi_releasepipeframe(ViPipe, pstVideoFrame);
}

CVI_S32 CVI_VI_StartSmoothRawDump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_U64 phy_addr = 0;

	CHECK_VI_NULL_PTR(pstDumpInfo);
	// 检查块数量至少为2
	if (pstDumpInfo->u8BlkCnt < 2) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Need two ring buffer at least, now is %d\n", pstDumpInfo->u8BlkCnt);
		return CVI_ERR_VI_INVALID_PARA;
	}

	CHECK_VI_PIPEID_VALID(pstDumpInfo->ViPipe);
	CHECK_VI_NULL_PTR(pstDumpInfo->phy_addr_list);

	// 检查物理地址列表中的地址是否有效
	for (CVI_U8 i = 0; i < pstDumpInfo->u8BlkCnt; i++) {
		phy_addr = *(pstDumpInfo->phy_addr_list + i);
		if (phy_addr == 0) {
			CVI_TRACE_VI(CVI_DBG_ERR, "phy_addr is invalid\n");
			return CVI_ERR_VI_INVALID_PARA;
		}
	}

	return platform_vi_startsmoothrawdump(pstDumpInfo);
}

CVI_S32 CVI_VI_StopSmoothRawDump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CHECK_VI_NULL_PTR(pstDumpInfo);
	CHECK_VI_PIPEID_VALID(pstDumpInfo->ViPipe);
	return platform_vi_stopsmoothrawdump(pstDumpInfo);
}

CVI_S32 CVI_VI_GetSmoothRawDump(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstVideoFrame);
	return platform_vi_getsmoothrawdump(ViPipe, pstVideoFrame, s32MilliSec);
}

CVI_S32 CVI_VI_PutSmoothRawDump(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstVideoFrame);
	return platform_vi_putsmoothrawdump(ViPipe, pstVideoFrame);
}

CVI_S32 CVI_VI_SendPipeRaw(CVI_U32 u32PipeNum, VI_PIPE PipeId[],
			   const VIDEO_FRAME_INFO_S *pstVideoFrame[], CVI_S32 s32MilliSec)
{
	CHECK_VI_NULL_PTR(PipeId);
	CHECK_VI_NULL_PTR(pstVideoFrame);
	if (u32PipeNum > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_VI(CVI_DBG_ERR, "only support %u pipe\n", VI_MAX_PIPE_NUM);
		return CVI_ERR_VI_INVALID_PIPEID;
	}

	for (CVI_U32 i = 0; i < u32PipeNum; ++i) {
		CHECK_VI_PIPEID_VALID(PipeId[i]);
		CHECK_VI_NULL_PTR(pstVideoFrame[i]);
	}

	return platform_vi_sendpiperaw(u32PipeNum, PipeId, pstVideoFrame, s32MilliSec);
}

CVI_S32 CVI_VI_QueryPipeStatus(VI_PIPE ViPipe, VI_PIPE_STATUS_S *pstStatus)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstStatus);
	return platform_vi_querypipestatus(ViPipe, pstStatus);
}

CVI_S32 CVI_VI_GetPipeFd(VI_PIPE ViPipe)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	return platform_vi_getpipefd(ViPipe);
}

CVI_S32 CVI_VI_CloseFd(void)
{
	return platform_vi_closefd();
}

CVI_S32 CVI_VI_AttachVbPool(VI_PIPE ViPipe, VI_CHN ViChn, VB_POOL VbPool)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	return platform_vi_attachvbpool(ViPipe, ViChn, VbPool);
}

CVI_S32 CVI_VI_DetachVbPool(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	return platform_vi_detachvbpool(ViPipe, ViChn);
}

CVI_S32 CVI_VI_SetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstChnAttr);

	if (pstChnAttr->stFrameRate.s32SrcFrameRate != pstChnAttr->stFrameRate.s32DstFrameRate)
		CVI_TRACE_VI(CVI_DBG_WARN, "FrameRate ctrl, src(%d) dst(%d), not support yet.\n"
				, pstChnAttr->stFrameRate.s32SrcFrameRate, pstChnAttr->stFrameRate.s32DstFrameRate);

	if (pstChnAttr->enPixelFormat != PIXEL_FORMAT_NV21 && pstChnAttr->enPixelFormat != PIXEL_FORMAT_NV12 &&
		(pstChnAttr->enPixelFormat < PIXEL_FORMAT_YUYV || pstChnAttr->enPixelFormat > PIXEL_FORMAT_VYUY)) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support %d\n", pstChnAttr->enPixelFormat);
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	if (pstChnAttr->stSize.u32Width > VI_PIPE_OFFLINE_MAX_WIDTH ||
		pstChnAttr->stSize.u32Height > VI_PIPE_OFFLINE_MAX_HEIGHT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "width(%d)_height(%d) out of range.\n",
					pstChnAttr->stSize.u32Width,
					pstChnAttr->stSize.u32Height);
	}

	if (pstChnAttr->stSize.u32Width == 0 ||
		pstChnAttr->stSize.u32Height == 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "width(%d)_height(%d) is invalid.\n",
					pstChnAttr->stSize.u32Width,
					pstChnAttr->stSize.u32Height);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_setchnattr(ViPipe, ViChn, pstChnAttr);
}

CVI_S32 CVI_VI_GetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstChnAttr);
	return platform_vi_getchnattr(ViPipe, ViChn, pstChnAttr);
}

CVI_S32 CVI_VI_EnableChn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	return platform_vi_enablechn(ViPipe, ViChn);
}

CVI_S32 CVI_VI_DisableChn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	return platform_vi_disablechn(ViPipe, ViChn);
}

CVI_S32 CVI_VI_SetChnCrop(VI_PIPE ViPipe, VI_CHN ViChn, const VI_CROP_INFO_S *pstCropInfo)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstCropInfo);

	if (pstCropInfo->stCropRect.s32X % 2 || pstCropInfo->stCropRect.s32Y % 2 ||
		pstCropInfo->stCropRect.u32Width % 2 || pstCropInfo->stCropRect.u32Height % 2) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d)_w(%d)_h(%d) must be multiple of 2.\n",
					pstCropInfo->stCropRect.s32X,
					pstCropInfo->stCropRect.s32Y,
					pstCropInfo->stCropRect.u32Width,
					pstCropInfo->stCropRect.u32Height);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstCropInfo->stCropRect.s32X < 0 || pstCropInfo->stCropRect.s32Y < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d) is invalid.\n",
					pstCropInfo->stCropRect.s32X,
					pstCropInfo->stCropRect.s32Y);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_setchncrop(ViPipe, ViChn, pstCropInfo);
}

CVI_S32 CVI_VI_GetChnCrop(VI_PIPE ViPipe, VI_CHN ViChn, VI_CROP_INFO_S *pstCropInfo)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstCropInfo);
	return platform_vi_getchncrop(ViPipe, ViChn, pstCropInfo);
}

CVI_S32 CVI_VI_SetChnRotation(VI_PIPE ViPipe, VI_CHN ViChn, const ROTATION_E enRotation)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	// 检查旋转参数是否有效
	if (enRotation == ROTATION_180) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support rotation(%d).\n", enRotation);
		return CVI_ERR_VI_NOT_SUPPORT;
	} else if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_VI(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_VI_INVALID_PARA;
	}

	return platform_vi_setchnrotation(ViPipe, ViChn, enRotation);
}

CVI_S32 CVI_VI_GetChnRotation(VI_PIPE ViPipe, VI_CHN ViChn, ROTATION_E *penRotation)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(penRotation);
	return platform_vi_getchnrotation(ViPipe, ViChn, penRotation);
}

CVI_S32 CVI_VI_SetChnLDCAttr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstLDCAttr);
	return platform_vi_setchnldcattr(ViPipe, ViChn, pstLDCAttr);
}

CVI_S32 CVI_VI_GetChnLDCAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_LDC_ATTR_S *pstLDCAttr)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstLDCAttr);
	return platform_vi_getchnldcattr(ViPipe, ViChn, pstLDCAttr);
}

CVI_S32 CVI_VI_GetChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstFrameInfo);
	return platform_vi_getchnframe(ViPipe, ViChn, pstFrameInfo, s32MilliSec);
}

CVI_S32 CVI_VI_ReleaseChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstFrameInfo);
	return platform_vi_releasechnframe(ViPipe, ViChn, pstFrameInfo);
}

CVI_S32 CVI_VI_QueryChnStatus(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_STATUS_S *pstChnStatus)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstChnStatus);
	return platform_vi_querychnstatus(ViPipe, ViChn, pstChnStatus);
}

CVI_S32 CVI_VI_RegChnFlipMirrorCallBack(VI_PIPE ViPipe, VI_DEV ViDev, void *pvData)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pvData);
	return platform_vi_regchnflipmirrorcallback(ViPipe, ViDev, pvData);
}

CVI_S32 CVI_VI_UnRegChnFlipMirrorCallBack(VI_PIPE ViPipe, VI_DEV ViDev)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_DEVID_VALID(ViDev);
	return platform_vi_unregchnflipmirrorcallback(ViPipe, ViDev);
}

CVI_S32 CVI_VI_SetChnFlipMirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL bFlip, CVI_BOOL bMirror)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	return platform_vi_setchnflipmirror(ViPipe, ViChn, bFlip, bMirror);
}

CVI_S32 CVI_VI_GetChnFlipMirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL *pbFlip, CVI_BOOL *pbMirror)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pbFlip);
	CHECK_VI_NULL_PTR(pbMirror);
	return platform_vi_getchnflipmirror(ViPipe, ViChn, pbFlip, pbMirror);
}

CVI_S32 CVI_VI_DumpHwRegisterToFile(VI_PIPE ViPipe, FILE *fp, VI_DUMP_REGISTER_TABLE_S *pstRegTbl)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(fp);
	CHECK_VI_NULL_PTR(pstRegTbl);
	return platform_vi_dumphwregistertofile(ViPipe, fp, pstRegTbl);
}

CVI_S32 CVI_VI_RegPmCallBack(VI_DEV ViDev, VI_PM_OPS_S *pstPmOps, void *pvData)
{
	return platform_vi_regpmcallback(ViDev, pstPmOps, pvData);
}

CVI_S32 CVI_VI_UnRegPmCallBack(VI_DEV ViDev)
{
	return platform_vi_unregpmcallback(ViDev);
}

CVI_S32 CVI_VI_Trig_AHD(VI_PIPE ViPipe, CVI_U8 u8AHDSignal)
{
	return platform_vi_trig_ahd(ViPipe, u8AHDSignal);
}

CVI_S32 CVI_VI_SetExtChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_EXT_CHN_ATTR_S *pstExtChnAttr)
{
	return platform_vi_setextchnattr(ViPipe, ViChn, pstExtChnAttr);
}

CVI_S32 CVI_VI_GetExtChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_EXT_CHN_ATTR_S *pstExtChnAttr)
{
	return platform_vi_getextchnattr(ViPipe, ViChn, pstExtChnAttr);
}

CVI_S32 CVI_VI_SetBypassFrm(VI_PIPE ViPipe, CVI_U8 bypass_num)
{
	return platform_vi_setbypassfrm(ViPipe, bypass_num);
}

CVI_S32 CVI_VI_AiIspCfg(VI_AI_ISP_CFG_S *pstAiIspCfg)
{
	CHECK_VI_NULL_PTR(pstAiIspCfg);
	return platform_vi_aiispcfg(pstAiIspCfg);
}

CVI_S32 CVI_VI_AiIspInfo(VI_AI_ISP_INFO_WRAP_S *pstAiIspInfoWrap)
{
	CHECK_VI_NULL_PTR(pstAiIspInfoWrap);
	return platform_vi_aiispinfo(pstAiIspInfoWrap);
}	
