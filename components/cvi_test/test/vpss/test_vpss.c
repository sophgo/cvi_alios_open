#include "board.h"
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/cli.h>
#include "fatfs_vfs.h"
#include "vfs.h"
#include <cvi_base.h>
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_buffer.h"
#include <pinctrl-mars.h>
#include "cvi_vpss.h"

void start_vpss(int32_t argc, char **argv)
{
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 0;
	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;
	CVI_S32 s32Ret;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_NV21;
	stVpssGrpAttr.u32MaxW = 1920;
	stVpssGrpAttr.u32MaxH = 1080;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width = 1280;
	stVpssChnAttr.u32Height = 720;
	stVpssChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat = PIXEL_FORMAT_NV21;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth = 0;
	stVpssChnAttr.bMirror = CVI_FALSE;
	stVpssChnAttr.bFlip = CVI_FALSE;
	stVpssChnAttr.stNormalize.bEnable = CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE;


	CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	CVI_VPSS_StartGrp(VpssGrp);


	stSrcChn.enModId = CVI_ID_VI;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;

	stDestChn.enModId = CVI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;
	stDestChn.s32ChnId = 0;

	s32Ret = CVI_SYS_Bind(&stSrcChn, &stDestChn);
	if(s32Ret == 0) {
		printf("*********VI bind VPSS Sucessful******** \n");
	} else {
		printf("*********VI bind VPSS Failed************\n");
	}
}
ALIOS_CLI_CMD_REGISTER(start_vpss, start_vpss, start vpss);


void GetFmtName(PIXEL_FORMAT_E enPixFmt, CVI_CHAR *szName)
{
	switch (enPixFmt)
	{
		case PIXEL_FORMAT_RGB_888:
			snprintf(szName, 10, "rgb");
			break;
		case PIXEL_FORMAT_BGR_888:
			snprintf(szName, 10, "bgr");
			break;
		case PIXEL_FORMAT_RGB_888_PLANAR:
			snprintf(szName, 10, "rgbm");
			break;
		case PIXEL_FORMAT_BGR_888_PLANAR:
			snprintf(szName, 10, "bgrm");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_422:
			snprintf(szName, 10, "p422");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_420:
			snprintf(szName, 10, "p420");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_444:
			snprintf(szName, 10, "p444");
			break;
		case PIXEL_FORMAT_YUV_400:
			snprintf(szName, 10, "y");
			break;
		case PIXEL_FORMAT_HSV_888:
			snprintf(szName, 10, "hsv");
			break;
		case PIXEL_FORMAT_HSV_888_PLANAR:
			snprintf(szName, 10, "hsvm");
			break;
		case PIXEL_FORMAT_NV12:
			snprintf(szName, 10, "nv12");
			break;
		case PIXEL_FORMAT_NV21:
			snprintf(szName, 10, "nv21");
			break;
		case PIXEL_FORMAT_NV16:
			snprintf(szName, 10, "nv16");
			break;
		case PIXEL_FORMAT_NV61:
			snprintf(szName, 10, "nv61");
			break;
		case PIXEL_FORMAT_YUYV:
			snprintf(szName, 10, "yuyv");
			break;
		case PIXEL_FORMAT_UYVY:
			snprintf(szName, 10, "uyvy");
			break;
		case PIXEL_FORMAT_YVYU:
			snprintf(szName, 10, "yvyu");
			break;
		case PIXEL_FORMAT_VYUY:
			snprintf(szName, 10, "vyuy");
			break;

		default:
			snprintf(szName, 10, "unknown");
			break;
	}

}

CVI_VOID test_VpssDump(VPSS_GRP Grp, VPSS_CHN Chn, CVI_U32 u32FrameCnt)
{
	CVI_S32 s32MilliSec = 1000;
	CVI_U32 u32Cnt = u32FrameCnt;
	CVI_CHAR szFrameName[128], szPixFrm[10];
	CVI_BOOL bFlag = CVI_TRUE;
	int fd = 0;
	CVI_S32 i;
	CVI_U32 u32DataLen;
	VIDEO_FRAME_INFO_S stFrameInfo;


	/* get frame  */
	while (u32Cnt--) {
		if (CVI_VPSS_GetChnFrame(Grp, Chn, &stFrameInfo, s32MilliSec) != CVI_SUCCESS) {
			printf("Get frame fail \n");
			usleep(1000);
			continue;
		}
		if (bFlag) {
			/* make file name */
			GetFmtName(stFrameInfo.stVFrame.enPixelFormat, szPixFrm);
			snprintf(szFrameName, 128, SD_FATFS_MOUNTPOINT"/vpss_grp%d_chn%d_%dx%d_%s_%d.yuv", Grp, Chn,
					 stFrameInfo.stVFrame.u32Width, stFrameInfo.stVFrame.u32Height,
					 szPixFrm, u32FrameCnt);
			printf("Dump frame of vpss chn %d to file: \"%s\"\n", Chn, szFrameName);

			fd = aos_open(szFrameName, O_CREAT | O_RDWR | O_TRUNC);
			if (fd <= 0) {
				printf("aos_open dst file failed\n");
				CVI_VPSS_ReleaseChnFrame(Grp, Chn, &stFrameInfo);
				return;
			}

			bFlag = CVI_FALSE;
		}

		for (i = 0; i < 3; ++i) {
			u32DataLen = stFrameInfo.stVFrame.u32Stride[i] * stFrameInfo.stVFrame.u32Height;
			if (u32DataLen == 0)
				continue;
			if (i > 0 && ((stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
				(stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
				(stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
				u32DataLen >>= 1;

			printf("plane(%d): paddr(%lx) vaddr(%p) stride(%d)\n",
				   i, stFrameInfo.stVFrame.u64PhyAddr[i],
				   stFrameInfo.stVFrame.pu8VirAddr[i],
				   stFrameInfo.stVFrame.u32Stride[i]);
			printf(" data_len(%d) plane_len(%d)\n",
					  u32DataLen, stFrameInfo.stVFrame.u32Length[i]);
			aos_write(fd, (CVI_U8 *)stFrameInfo.stVFrame.u64PhyAddr[i], u32DataLen);
		}

		if (CVI_VPSS_ReleaseChnFrame(Grp, Chn, &stFrameInfo) != CVI_SUCCESS)
			printf("CVI_VPSS_ReleaseChnFrame fail\n");
	}
	if (fd) {
		aos_sync(fd);
		aos_close(fd);
	}
}

void dump_vpss_frame(int32_t argc, char **argv)
{
	CVI_U32  VpssGrp = 0;
	CVI_U32  VpssChn = 0;
	CVI_S32  s32Cnt = 1;

	if (argc != 4) {
		printf("invailed param\n usage: %s [grp] [chn] [count]\n", argv[0]);
		return;
	}

	VpssGrp = atoi(argv[1]);
	VpssChn = atoi(argv[2]);
	s32Cnt = atoi(argv[3]);

	test_VpssDump(VpssGrp, VpssChn, s32Cnt);
}
ALIOS_CLI_CMD_REGISTER(dump_vpss_frame, dump_vpss_frame, dump vpss frame function);

typedef struct _VPSS_PARAM {
	VPSS_GRP VpssGrp;
	SIZE_S stSize;
	SIZE_S stSizeOut;
	PIXEL_FORMAT_E pixelFormat;
	PIXEL_FORMAT_E pixelFormatOut;
	VPSS_CROP_INFO_S stCropInfo;
	VPSS_NORMALIZE_S stNormalize;
	ROTATION_E enRotation;
	CVI_FLOAT yRatio;
	VPSS_LDC_ATTR_S stLDCAttr;
	CVI_BOOL bAttachVb;
	CVI_BOOL bManualAspectRatio;
	VPSS_SCALE_COEF_E enCoef;
	CVI_BOOL bChnScaleCoef;
	CVI_U32 u32LoopCount;
	CVI_BOOL bShowProc;
	CVI_BOOL bSendOnly;
	CVI_BOOL bMirror;
	CVI_BOOL bFlip;
	char fileName[256];
	char fileNameOut[256];
	char fileNameOutRef[256];
} VPSS_PARAM;

CVI_S32 vpss_send_one_frame(VPSS_GRP VpssGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	int fp;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	stVideoFrame.stVFrame.enPixelFormat = enPixelFormat;
	stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVideoFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
	stVideoFrame.stVFrame.u32Width = stSize->u32Width;
	stVideoFrame.stVFrame.u32Height = stSize->u32Height;
	stVideoFrame.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	stVideoFrame.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		printf("SAMPLE_COMM_VPSS_SendFrame: Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = aos_open(filename, O_RDWR | O_TRUNC);
	if (fp == CVI_NULL) {
		printf("open data file error\n");
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	stVideoFrame.stVFrame.u64PhyAddr[1] = stVideoFrame.stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		stVideoFrame.stVFrame.u64PhyAddr[2] = stVideoFrame.stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}
	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		stVideoFrame.stVFrame.pu8VirAddr[i] = (CVI_U8 *)stVideoFrame.stVFrame.u64PhyAddr[i];
		u32len = aos_read(fp, stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
		if (u32len <= 0) {
			printf("vpss send frame: fread plane%d error\n", i);
			aos_close(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
	}

	printf("length of buffer(%d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2]);

	aos_close(fp);

	printf("read file done and send out frame.\n");
	CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, -1);
	CVI_VB_ReleaseBlock(blk);
	return CVI_SUCCESS;
}

CVI_S32 frame_save_to_file(const CVI_CHAR * filename, VIDEO_FRAME_INFO_S * pstVideoFrame)
{
	int fp;
	CVI_U32 u32len, u32DataLen;

	fp = aos_open(filename, O_CREAT | O_RDWR | O_TRUNC);
	if (fp == CVI_NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "open data file error\n");
		return CVI_FAILURE;
	}
	if (pstVideoFrame == NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "video frame is NULL\n");
		return CVI_FAILURE;
	}

	// Print_Frame_Info(pstVideoFrame);
	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		printf("plane(%d): paddr(%#lx) vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		printf("data_len(%d) plane_len(%d)\n",
			    u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);

		u32len = aos_write(fp, (void *)pstVideoFrame->stVFrame.u64PhyAddr[i], u32DataLen);
		if (u32len <= 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "aos_write data(%d) error\n", i);
			break;
		}
	}

	aos_close(fp);
	return CVI_SUCCESS;
}

void ldc_one_frame(int32_t argc, char **argv)
{
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr;
	VPSS_GRP VpssGrp = -1;
	VPSS_CHN VpssChn = 0;
	CVI_S32 s32Ret;
	VPSS_MODE_S VpssMode;
	VPSS_PARAM param;
	VPSS_LDC_ATTR_S *pstLDCAttr;
	VPSS_LDC_ATTR_S stLDCAttrOut;
	VB_POOL_CONFIG_S cfg;
	VB_POOL PoolID;
	CVI_U32 u32BlkSize, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame;

	if (argc != 14) {
		printf("Usage: ldc_one_frame src_width src_height dst_width dst_height input_file output_file\n");
		printf("       aspectRatio[1(Y)/0(N)] Ratio[0-100] XRatio[0-100] YRatio[0-100]\n");
		printf("       XOffset[-511 ~ 511] YOffset[-511 ~ 511] DistortionRatio[-300 ~ 500]\n");
		return;
	}

	s32Ret = CVI_SYS_GetVPSSModeEx(&VpssMode);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_SYS_GetVPSSModeEx failed with %d\n", s32Ret);
		return;
	}

	if (VpssMode.enMode == VPSS_MODE_SINGLE && VpssMode.aenInput[1] == VPSS_INPUT_ISP) {
		printf("ldc_one_frame not supported in vpss online\n");
		return;
	}

	memset(&param, 0, sizeof(param));
	param.stSize.u32Width = atoi(argv[1]);
	param.stSize.u32Height = atoi(argv[2]);
	param.stSizeOut.u32Width = atoi(argv[3]);
	param.stSizeOut.u32Height = atoi(argv[4]);
	param.pixelFormat = PIXEL_FORMAT_NV21;
	param.pixelFormatOut = PIXEL_FORMAT_NV21;
	param.enRotation = ROTATION_0;
	param.yRatio = 0.0f;
	param.bManualAspectRatio = CVI_FALSE;
	snprintf(param.fileName, sizeof(param.fileName)-1, "%s", argv[5]);
	snprintf(param.fileNameOut, sizeof(param.fileNameOut)-1, "%s", argv[6]);

	memset(&cfg, 0, sizeof(VB_POOL_CONFIG_S));
	u32BlkSize = COMMON_GetPicBufferSize(param.stSize.u32Width, param.stSize.u32Height,
		param.pixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(param.stSizeOut.u32Width, param.stSizeOut.u32Height,
		param.pixelFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	cfg.u32BlkSize = u32BlkSize > u32BlkSizeOut ? u32BlkSize : u32BlkSizeOut;
	cfg.u32BlkCnt = 2;
	printf("Create VB pool cnt(%d) blksize(0x%x)\n",
			cfg.u32BlkCnt, cfg.u32BlkSize);
	PoolID = CVI_VB_CreatePool(&cfg);
	if (PoolID == VB_INVALID_POOLID) {
		printf("create vb pool failed\n");
		goto VB_EXIT;
	}

	pstLDCAttr = &param.stLDCAttr;
	pstLDCAttr->bEnable = 1;

	pstLDCAttr->stAttr.bAspect = atoi(argv[7]);
	printf("Keep AspectRatio 1(Y)/0(N): %d\n", pstLDCAttr->stAttr.bAspect);
	if (pstLDCAttr->stAttr.bAspect) {
		pstLDCAttr->stAttr.s32XYRatio = atoi(argv[8]);
		printf("Ratio (0 ~ 100): %d\n", pstLDCAttr->stAttr.s32XYRatio);
	} else {
		pstLDCAttr->stAttr.s32XRatio = atoi(argv[9]);
		printf("XRatio (0 ~ 100): %d\n",pstLDCAttr->stAttr.s32XRatio);
		pstLDCAttr->stAttr.s32XRatio = atoi(argv[10]);
		printf("YRatio (0 ~ 100): %d\n",pstLDCAttr->stAttr.s32YRatio);
	}
	pstLDCAttr->stAttr.s32CenterXOffset = atoi(argv[11]);
	printf("XOffset (-511 ~ 511): %d\n",pstLDCAttr->stAttr.s32CenterXOffset);
	pstLDCAttr->stAttr.s32CenterYOffset = atoi(argv[12]);
	printf("YOffset (-511 ~ 511): %d\n",pstLDCAttr->stAttr.s32CenterXOffset);
	pstLDCAttr->stAttr.s32DistortionRatio = atoi(argv[13]);
	printf("DistortionRatio (-300 ~ 500): %d\n",pstLDCAttr->stAttr.s32DistortionRatio);

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = param.pixelFormat;
	stVpssGrpAttr.u32MaxW = param.stSize.u32Width;
	stVpssGrpAttr.u32MaxH = param.stSize.u32Height;
	if (VpssMode.enMode == VPSS_MODE_DUAL) {
		stVpssGrpAttr.u8VpssDev = VpssMode.aenInput[0] == VPSS_INPUT_MEM ? 0 : 1;
	}

	stVpssChnAttr.u32Width = param.stSizeOut.u32Width;
	stVpssChnAttr.u32Height = param.stSizeOut.u32Height;
	stVpssChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat = param.pixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth = 0;
	stVpssChnAttr.bMirror = CVI_FALSE;
	stVpssChnAttr.bFlip = CVI_FALSE;
	stVpssChnAttr.stNormalize.bEnable = CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE;

	VpssGrp = CVI_VPSS_GetAvailableGrp();
	CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	CVI_VPSS_StartGrp(VpssGrp);

	s32Ret = CVI_VPSS_SetChnLDCAttr(VpssGrp, VpssChn, pstLDCAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("set vpss chn ldc attr failed. s32Ret: 0x%x !\n", s32Ret);
		goto VPSS_EXIT;
	}

	s32Ret = CVI_VPSS_GetChnLDCAttr(VpssGrp, VpssChn, &stLDCAttrOut);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_GetChnLDCAttr NG with %d\n", s32Ret);
		goto VPSS_EXIT;
	}
	if (pstLDCAttr->bEnable) {
		if (pstLDCAttr->bEnable != stLDCAttrOut.bEnable ||
			pstLDCAttr->stAttr.bAspect != stLDCAttrOut.stAttr.bAspect ||
			pstLDCAttr->stAttr.s32XYRatio != stLDCAttrOut.stAttr.s32XYRatio ||
			pstLDCAttr->stAttr.s32CenterXOffset != stLDCAttrOut.stAttr.s32CenterXOffset ||
			pstLDCAttr->stAttr.s32CenterYOffset != stLDCAttrOut.stAttr.s32CenterYOffset ||
			pstLDCAttr->stAttr.s32DistortionRatio != stLDCAttrOut.stAttr.s32DistortionRatio) {
			printf("CVI_VPSS_GetChnLDCAttr NG - invalid ldc\n");
			printf("bEnable=%d, apect=%d, xyratio=%d, xoffset=%d, yoffset=%d, ratio=%d\n",
				stLDCAttrOut.bEnable, stLDCAttrOut.stAttr.bAspect,
				stLDCAttrOut.stAttr.s32XYRatio, stLDCAttrOut.stAttr.s32CenterXOffset,
				stLDCAttrOut.stAttr.s32CenterYOffset, stLDCAttrOut.stAttr.s32DistortionRatio);
			goto VPSS_EXIT;
		}
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, PoolID);
	if (s32Ret != CVI_SUCCESS) {
		printf("vpss attach vb pool failed. s32Ret: 0x%x !\n", s32Ret);
		goto VPSS_EXIT;
	}

	s32Ret = vpss_send_one_frame(VpssGrp, &param.stSize, param.pixelFormat, param.fileName);
	if (s32Ret != CVI_SUCCESS) {
		printf("vpss_send_one_frame for grp%d chn%d. failed with s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
		goto VPSS_EXIT;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, -1);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_GetChnFrame for grp%d chn%d. failed with s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
		goto VPSS_EXIT;
	}

	s32Ret = frame_save_to_file(param.fileNameOut, &stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		printf("frame_save_to_file for grp%d chn%d. failed with s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
		goto VPSS_EXIT;
	}

	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_ReleaseChnFrame for grp%d chn%d. failed with s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
		goto VPSS_EXIT;
	}

VPSS_EXIT:
	s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		printf("Vpss stop Grp %d channel %d failed! Please check param\n",
		VpssGrp, VpssChn);
		goto VB_EXIT;
	}
	s32Ret = CVI_VPSS_StopGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		printf("Vpss Stop Grp %d failed! Please check param\n", VpssGrp);
		goto VB_EXIT;
	}

	s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		printf("Vpss Destroy Grp %d failed! Please check\n", VpssGrp);
		goto VB_EXIT;
	}
VB_EXIT:
	s32Ret = CVI_VB_DestroyPool(PoolID);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VB_DestroyPool for pool %d failed with %d\n", PoolID, s32Ret);
		goto VB_EXIT;
	}
}
ALIOS_CLI_CMD_REGISTER(ldc_one_frame, ldc_one_frame, do ldc for one frame);
