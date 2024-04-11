/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_ive_ioctl.h
 * Description:
 */

#ifndef __CVI_IVE_IOCTL_H__
#define __CVI_IVE_IOCTL_H__

#include "cvi_comm_ive.h"
#include "cvi_errno.h"

struct cvi_ive_test_arg {
	IVE_IMAGE_TYPE_E enType;
	char *pAddr;
#ifdef __arm__
	__u32 padding;
#endif
	CVI_U16 u16Width;
	CVI_U16 u16Height;
};

struct cvi_ive_query_arg {
	IVE_HANDLE pIveHandle;
	CVI_BOOL *pbFinish;
#ifdef __arm__
	__u32 padding;
#endif
	CVI_BOOL bBlock;
};

struct cvi_ive_ioctl_add_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	IVE_ADD_CTRL_S pstCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_and_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_xor_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_or_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_sub_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	IVE_SUB_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_erode_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_ERODE_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_dilate_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_DILATE_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_thresh_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_THRESH_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_match_bgmodel_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stCurImg;
	IVE_DATA_S stBgModel;
	IVE_IMAGE_S stFgFlag;
	IVE_DST_IMAGE_S stDiffFg;
	IVE_DST_MEM_INFO_S stStatData;
	IVE_MATCH_BG_MODEL_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_update_bgmodel_arg {
	IVE_HANDLE pIveHandle;
	IVE_DATA_S stBgModel;
	IVE_IMAGE_S stFgFlag;
	IVE_DST_IMAGE_S stBgImg;
	IVE_DST_IMAGE_S stChgSta;
	IVE_DST_MEM_INFO_S stStatData;
	IVE_UPDATE_BG_MODEL_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_gmm_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stFg;
	IVE_DST_IMAGE_S stBg;
	IVE_MEM_INFO_S stModel;
	IVE_GMM_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_gmm2_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_SRC_IMAGE_S stFactor;
	IVE_DST_IMAGE_S stFg;
	IVE_DST_IMAGE_S stBg;
	IVE_DST_IMAGE_S stInfo;
	IVE_MEM_INFO_S stModel;
	IVE_GMM2_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_dma_arg {
	IVE_HANDLE pIveHandle;
	IVE_DATA_S stSrc;
	IVE_DST_DATA_S stDst;
	IVE_DMA_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_bernsen_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_BERNSEN_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_filter_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_FILTER_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_sobel_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDstH;
	IVE_DST_IMAGE_S stDstV;
	IVE_SOBEL_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_maganang_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDstMag;
	IVE_DST_IMAGE_S stDstAng;
	IVE_MAG_AND_ANG_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_csc_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_CSC_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_hist_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_MEM_INFO_S stDst;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_filter_and_csc_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_FILTER_AND_CSC_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_map_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_SRC_MEM_INFO_S stMap;
	IVE_DST_IMAGE_S stDst;
	IVE_MAP_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_ncc_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_MEM_INFO_S stDst;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_integ_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_MEM_INFO_S stDst;
	IVE_INTEG_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_lbp_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_LBP_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_thresh_s16_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_THRESH_S16_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_thres_su16_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_THRESH_U16_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_16bit_to_8bit_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_16BIT_TO_8BIT_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_ord_stat_filter_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_ORD_STAT_FILTER_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_canny_hys_edge_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_DST_MEM_INFO_S stStack;
	IVE_CANNY_HYS_EDGE_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_norm_grad_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDstH;
	IVE_DST_IMAGE_S stDstV;
	IVE_DST_IMAGE_S stDstHV;
	IVE_NORM_GRAD_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_grad_fg_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stBgDiffFg;
	IVE_SRC_IMAGE_S stCurGrad;
	IVE_SRC_IMAGE_S stBgGrad;
	IVE_DST_IMAGE_S stGradFg;
	IVE_GRAD_FG_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_sad_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stSad;
	IVE_DST_IMAGE_S stThr;
	IVE_SAD_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_resize_arg {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S *astSrc;
#ifdef __arm__
		__u32 padding1;
#endif
	IVE_DST_IMAGE_S *astDst;
#ifdef __arm__
		__u32 padding2;
#endif
	IVE_RESIZE_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_rgbPToYuvToErodeToDilate {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst1;
	IVE_DST_IMAGE_S stDst2;
	IVE_FILTER_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_stcandicorner {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_ST_CANDI_CORNER_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

struct cvi_ive_ioctl_md {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	IVE_FRAME_DIFF_MOTION_CTRL_S stCtrl;
	CVI_BOOL bInstant;
};

#define CVI_IVE_IOC_MAGIC 'v'
#define CVI_IVE_IOC_TEST 0x00
#define CVI_IVE_IOC_DMA 0x01
#define CVI_IVE_IOC_Add 0x02
#define CVI_IVE_IOC_And 0x03
#define CVI_IVE_IOC_Or 0x04
#define CVI_IVE_IOC_Sub 0x05
#define CVI_IVE_IOC_Xor 0x06
#define CVI_IVE_IOC_Thresh 0x07
#define CVI_IVE_IOC_Thresh_S16 0x08
#define CVI_IVE_IOC_Thresh_U16 0x09
#define CVI_IVE_IOC_16BitTo8Bit 0x0a
#define CVI_IVE_IOC_CSC 0x0b
#define CVI_IVE_IOC_GradFg 0x0c
#define CVI_IVE_IOC_NormGrad 0x0d
#define CVI_IVE_IOC_Filter 0x0e
#define CVI_IVE_IOC_FilterAndCSC 0x0f
#define CVI_IVE_IOC_Hist 0x10
#define CVI_IVE_IOC_EqualizeHist 0x11
#define CVI_IVE_IOC_Map 0x12
#define CVI_IVE_IOC_NCC 0x13
#define CVI_IVE_IOC_OrdStatFilter 0x14
#define CVI_IVE_IOC_Resize 0x15
#define CVI_IVE_IOC_CannyHysEdge 0x16
#define CVI_IVE_IOC_CannyEdge 0x17
#define CVI_IVE_IOC_Integ 0x18
#define CVI_IVE_IOC_LBP 0x19
#define CVI_IVE_IOC_MagAndAng 0x1a
#define CVI_IVE_IOC_STCandiCorner 0x1b
#define CVI_IVE_IOC_STCorner 0x1c
#define CVI_IVE_IOC_Sobel 0x1d
#define CVI_IVE_IOC_CCL 0x1e
#define CVI_IVE_IOC_Dilate 0x1f
#define CVI_IVE_IOC_Erode 0x20
#define CVI_IVE_IOC_MatchBgModel 0x21
#define CVI_IVE_IOC_UpdateBgModel 0x22
#define CVI_IVE_IOC_GMM 0x23
#define CVI_IVE_IOC_GMM2 0x24
#define CVI_IVE_IOC_LKOpticalFlowPyr 0x25
#define CVI_IVE_IOC_SAD 0x26
#define CVI_IVE_IOC_Bernsen 0x27
#define CVI_IVE_IOC_imgInToOdma 0x28
#define CVI_IVE_IOC_rgbPToYuvToErodeToDilate 0x29
#define CVI_IVE_IOC_MD 0x2a
#define CVI_IVE_IOC_CMDQ 0x2b
#define CVI_IVE_IOC_RESET 0xF0
#define CVI_IVE_IOC_DUMP 0xF1
#define CVI_IVE_IOC_QUERY 0xF2
#endif /* __CVI_IVE_IOCTL_H__ */
