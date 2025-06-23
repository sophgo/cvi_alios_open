/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample_common.h
 * Description:
 */

#ifndef __CVI_COMM_VDEC_H__
#define __CVI_COMM_VDEC_H__
#include <cvi_comm_video.h>
#include <cvi_common.h>
#include <cvi_defines.h>
#include <cvi_type.h>
#include <cvi_comm_vb.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define CVI_VDEC_STR_LEN	255
#define CVI_VDEC_MASK_ERR	0x1
#define CVI_VDEC_MASK_WARN	0x2
#define CVI_VDEC_MASK_INFO	0x4
#define CVI_VDEC_MASK_FLOW	0x8
#define CVI_VDEC_MASK_DBG	0x10
#define CVI_VDEC_MASK_MEM	0x80
#define CVI_VDEC_MASK_BS	0x100
#define CVI_VDEC_MASK_SRC	0x200
#define CVI_VDEC_MASK_API	0x400
#define CVI_VDEC_MASK_DISP	0x800
#define CVI_VDEC_MASK_PERF	0x1000
#define CVI_VDEC_MASK_CFG	0x2000
#define CVI_VDEC_MASK_TRACE	0x4000
#define CVI_VDEC_MASK_DUMP_YUV	0x10000
#define CVI_VDEC_MASK_DUMP_BS	0x20000
#define CVI_VDEC_MASK_CURR	(CVI_VDEC_MASK_ERR)


#define CVI_IO_BLOCK CVI_TRUE
#define CVI_IO_NOBLOCK CVI_FALSE

typedef enum _VIDEO_MODE_E {
	VIDEO_MODE_STREAM = 0, /* send by stream */
	VIDEO_MODE_FRAME, /* send by frame  */
	VIDEO_MODE_COMPAT, /* One frame supports multiple packets sending. */
	/* The current frame is considered to end when bEndOfFrame is equal to HI_TRUE */
	VIDEO_MODE_BUTT
} VIDEO_MODE_E;

enum VDEC_BIND_MODE_E {
	VDEC_BIND_DISABLE  = 0,
	VDEC_BIND_VPSS_VO,
	VDEC_BIND_VPSS_VENC,
};

typedef struct _VDEC_ATTR_VIDEO_S {
	CVI_U32 u32RefFrameNum; /* RW, Range: [0, 16]; reference frame num. */
	CVI_BOOL bTemporalMvpEnable; /* RW; */
	/* specifies whether temporal motion vector predictors can be used for inter prediction */
	CVI_U32 u32TmvBufSize; /* RW; tmv buffer size(Byte) */
} VDEC_ATTR_VIDEO_S;

typedef struct _BUFFER_INFO_S {
	CVI_U32 size;
	CVI_U64 phys_addr;
	CVI_U64 virt_addr;
} BUFFER_INFO_S;

typedef struct _VDEC_BUFFER_INFO_S {
	BUFFER_INFO_S* bitstream_buffer;
#ifdef __arm__
	CVI_U32 u32AddrPadding1;
#endif
	BUFFER_INFO_S* frame_buffer;
#ifdef __arm__
	CVI_U32 u32AddrPadding2;
#endif
	BUFFER_INFO_S* Ytable_buffer;
#ifdef __arm__
	CVI_U32 u32AddrPadding3;
#endif
	BUFFER_INFO_S* Ctable_buffer;
#ifdef __arm__
	CVI_U32 u32AddrPadding4;
#endif
	CVI_S32 s32numOfDecFbc;
	CVI_S32 s32numOfDecwtl;
} VDEC_BUFFER_INFO_S;

typedef struct _VDEC_CHN_ATTR_S {
	PAYLOAD_TYPE_E enType; /* RW; video type to be decoded   */
	VIDEO_MODE_E enMode; /* RW; send by stream or by frame */
	CVI_U32 u32PicWidth; /* RW; max pic width */
	CVI_U32 u32PicHeight; /* RW; max pic height */
	CVI_U32 u32StreamBufSize; /* RW; stream buffer size(Byte) */
	CVI_U32 u32FrameBufSize; /* RW; frame buffer size(Byte) */
	CVI_U32 u32FrameBufCnt;
	COMPRESS_MODE_E enCompressMode; /* RW; compress mode */
	CVI_U8 u8CommandQueueDepth; /* RW; command queue depth [0,4]*/
	CVI_U8 u8ReorderEnable;
	union {
		VDEC_ATTR_VIDEO_S
		stVdecVideoAttr; /* structure with video ( h264/h265) */
	};
	VDEC_BUFFER_INFO_S stBufferInfo;
} VDEC_CHN_ATTR_S;

typedef struct _VDEC_STREAM_S {
	CVI_U32 u32Len; /* W; stream len */
	CVI_U64 u64PTS; /* W; present time stamp */
	CVI_U64 u64DTS; /* W; decoded time stamp */
	CVI_BOOL bEndOfFrame; /* W; is the end of a frame */
	CVI_BOOL bEndOfStream; /* W; is the end of all stream */
	CVI_BOOL bDisplay; /* W; is the current frame displayed. only valid by VIDEO_MODE_FRAME */
	CVI_U8 *pu8Addr; /* W; stream address */
} VDEC_STREAM_S;

typedef struct _VDEC_USERDATA_S {
	CVI_U64 u64PhyAddr; /* R; userdata data phy address */
	CVI_U32 u32Len; /* R; userdata data len */
	CVI_BOOL bValid; /* R; is valid? */
	CVI_U8 *pu8Addr; /* R; userdata data vir address */
} VDEC_USERDATA_S;

typedef struct _VDEC_DECODE_ERROR_S {
	CVI_S32 s32FormatErr; /* R; format error. eg: do not support filed */
	CVI_S32 s32PicSizeErrSet; /* R; picture width or height is larger than channel width or height */
	CVI_S32 s32StreamUnsprt; /* R; unsupport the stream specification */
	CVI_S32 s32PackErr; /* R; stream package error */
	CVI_S32 s32PrtclNumErrSet; /* R; protocol num is not enough. eg: slice, pps, sps */
	CVI_S32 s32RefErrSet; /* R; reference num is not enough */
	CVI_S32 s32PicBufSizeErrSet; /* R; the buffer size of picture is not enough */
	CVI_S32 s32StreamSizeOver; /* R; the stream size is too big and force discard stream */
	CVI_S32 s32VdecStreamNotRelease; /* R; the stream not released for too long time */
} VDEC_DECODE_ERROR_S;

typedef struct {
	CVI_U32 left;   /**< A horizontal pixel offset of top-left corner of rectangle from (0, 0) */
	CVI_U32 top;    /**< A vertical pixel offset of top-left corner of rectangle from (0, 0) */
	CVI_U32 right;  /**< A horizontal pixel offset of bottom-right corner of rectangle from (0, 0) */
	CVI_U32 bottom; /**< A vertical pixel offset of bottom-right corner of rectangle from (0, 0) */
} Rect_S;

typedef struct _SEQ_INITIAL_INFO_S {
	CVI_S32 s32PicWidth; /*R; Horizontal picture size in pixel*/
	CVI_S32 s32PicHeight; /*R; Vertical picture size in pixel*/
	CVI_S32 s32FRateNumerator; /*R; The numerator part of frame rate fraction*/
	CVI_S32 s32FRateDenominator; /*R; The denominator part of frame rate fraction*/
	Rect_S  stPicCropRect; /*R; Picture cropping rectangle information*/
	CVI_S32 s32MinFrameBufferCount; /*R; This is the minimum number of frame buffers required for decoding.*/
	CVI_S32 s32FrameBufDelay; /*R; the maximum display frame buffer delay for buffering decoded picture reorder.*/
	CVI_S32 s32Profile;/*R; H.265/H.264 : profile_idc*/
	CVI_S32 s32Level;/*R;H.265/H.264 : level_idc*/
	CVI_S32 s32Interlace; /*R; progressive or interlace frame */
	CVI_S32 s32MaxNumRefFrmFlag;/*R; one of the SPS syntax elements in H.264.*/
	CVI_S32 s32MaxNumRefFrm;
	/* When avcIsExtSAR is 0, this indicates aspect_ratio_idc[7:0]. When avcIsExtSAR is 1, this indicates sar_width[31:16] and sar_height[15:0].
		If aspect_ratio_info_present_flag = 0, the register returns -1 (0xffffffff).*/
	CVI_S32 s32AspectRateInfo;/**/
	CVI_S32 s32BitRate; /*R; The bitrate value written in bitstream syntax. If there is no bitRate, this reports -1. */
	CVI_S32 s32LumaBitdepth; /*R; bit-depth of luma sample */
	CVI_S32 s32ChromaBitdepth; /*R; bit-depth of chroma sample */
	CVI_U8  u8CoreIdx; /*R, 0:ve_core, 1:vd_core0, 2:vd_core1*/
}SEQ_INITIAL_INFO_S;

typedef struct _VDEC_CHN_STATUS_S {
	PAYLOAD_TYPE_E enType; /* R; video type to be decoded */
	CVI_S32 u32LeftStreamBytes; /* R; left stream bytes waiting for decode */
	CVI_S32 u32LeftStreamFrames; /* R; left frames waiting for decode,only valid for VIDEO_MODE_FRAME */
	CVI_S32 u32LeftPics; /* R; pics waiting for output */
	CVI_BOOL bStartRecvStream; /* R; had started recv stream? */
	CVI_U32 u32RecvStreamFrames; /* R; how many frames of stream has been received. valid when send by frame. */
	CVI_U32 u32DecodeStreamFrames; /* R; how many frames of stream has been decoded. valid when send by frame. */
	VDEC_DECODE_ERROR_S stVdecDecErr; /* R; information about decode error */
	CVI_U32 u32Width; /* R; the width of the currently decoded stream */
	CVI_U32 u32Height; /* R; the height of the currently decoded stream */
	CVI_U8 u8FreeSrcBuffer;
	CVI_U8 u8BusySrcBuffer;
	CVI_U8 u8Status;
	SEQ_INITIAL_INFO_S stSeqinitalInfo;
} VDEC_CHN_STATUS_S;

typedef enum _VIDEO_DEC_MODE_E {
	VIDEO_DEC_MODE_IPB = 0,
	VIDEO_DEC_MODE_IP,
	VIDEO_DEC_MODE_I,
	VIDEO_DEC_MODE_BUTT
} VIDEO_DEC_MODE_E;

typedef enum _VIDEO_OUTPUT_ORDER_E {
	VIDEO_OUTPUT_ORDER_DISP = 0,
	VIDEO_OUTPUT_ORDER_DEC,
	VIDEO_OUTPUT_ORDER_BUTT
} VIDEO_OUTPUT_ORDER_E;

typedef struct _VDEC_PARAM_VIDEO_S {
	CVI_S32 s32ErrThreshold; /* RW, Range: [0, 100]; */
	/* threshold for stream error process, 0: discard with any error, 100 : keep data with any error */
	VIDEO_DEC_MODE_E enDecMode; /* RW; */
	/* decode mode , 0: deocde IPB frames, 1: only decode I frame & P frame , 2: only decode I frame */
	VIDEO_OUTPUT_ORDER_E enOutputOrder; /* RW; */
	/* frames output order ,0: the same with display order , 1: the same width decoder order */
	COMPRESS_MODE_E enCompressMode; /* RW; compress mode */
	VIDEO_FORMAT_E enVideoFormat; /* RW; video format */
} VDEC_PARAM_VIDEO_S;

typedef struct _VDEC_PARAM_PICTURE_S {
	CVI_U32 u32Alpha; /* RW, Range: [0, 255]; value 0 is transparent. */
	/* [0 ,127]   is deemed to transparent when enPixelFormat is ARGB1555 or
	 * ABGR1555 [128 ,256] is deemed to non-transparent when enPixelFormat is
	 * ARGB1555 or ABGR1555
	 */
	CVI_U32 u32HDownSampling;
	CVI_U32 u32VDownSampling;
	CVI_S32 s32ROIEnable;
	CVI_S32 s32ROIOffsetX;
	CVI_S32 s32ROIOffsetY;
	CVI_S32 s32ROIOffset;
	CVI_S32 s32ROIWidth;
	CVI_S32 s32ROIHeight;
	CVI_S32 s32RotAngle;
	CVI_S32 s32MirDir;
} VDEC_PARAM_PICTURE_S;

typedef struct _VDEC_CHN_PARAM_S {
	PAYLOAD_TYPE_E enType; /* RW; video type to be decoded   */
	PIXEL_FORMAT_E enPixelFormat; /* RW; out put pixel format */
	CVI_U32 u32DisplayFrameNum; /* RW, Range: [0, 16]; display frame num */
	union {
		VDEC_PARAM_VIDEO_S
		stVdecVideoParam; /* structure with video ( h265/h264) */
		VDEC_PARAM_PICTURE_S
		stVdecPictureParam; /* structure with picture (jpeg/mjpeg ) */
	};
} VDEC_CHN_PARAM_S;

typedef struct _H264_PRTCL_PARAM_S {
	CVI_S32 s32MaxSliceNum; /* RW; max slice num support */
	CVI_S32 s32MaxSpsNum; /* RW; max sps num support */
	CVI_S32 s32MaxPpsNum; /* RW; max pps num support */
} H264_PRTCL_PARAM_S;

typedef struct _H265_PRTCL_PARAM_S {
	CVI_S32 s32MaxSliceSegmentNum; /* RW; max slice segmnet num support */
	CVI_S32 s32MaxVpsNum; /* RW; max vps num support */
	CVI_S32 s32MaxSpsNum; /* RW; max sps num support */
	CVI_S32 s32MaxPpsNum; /* RW; max pps num support */
} H265_PRTCL_PARAM_S;

typedef struct _VDEC_PRTCL_PARAM_S {
	PAYLOAD_TYPE_E
	enType; /* RW; video type to be decoded, only h264 and h265 supported */
	union {
		H264_PRTCL_PARAM_S
		stH264PrtclParam; /* protocol param structure for h264 */
		H265_PRTCL_PARAM_S
		stH265PrtclParam; /* protocol param structure for h265 */
	};
} VDEC_PRTCL_PARAM_S;

typedef struct _VDEC_CHN_POOL_S {
	VB_POOL hPicVbPool; /* RW;  vb pool id for pic buffer */
	VB_POOL hTmvVbPool; /* RW;  vb pool id for tmv buffer */
} VDEC_CHN_POOL_S;

typedef enum _VDEC_EVNT_E {
	VDEC_EVNT_STREAM_ERR = 1,
	VDEC_EVNT_UNSUPPORT,
	VDEC_EVNT_OVER_REFTHR,
	VDEC_EVNT_REF_NUM_OVER,
	VDEC_EVNT_SLICE_NUM_OVER,
	VDEC_EVNT_SPS_NUM_OVER,
	VDEC_EVNT_PPS_NUM_OVER,
	VDEC_EVNT_PICBUF_SIZE_ERR,
	VDEC_EVNT_SIZE_OVER,
	VDEC_EVNT_IMG_SIZE_CHANGE,
	VDEC_EVNT_VPS_NUM_OVER,
	VDEC_EVNT_BUTT
} VDEC_EVNT_E;

typedef enum _VDEC_CAPACITY_STRATEGY_E {
	VDEC_CAPACITY_STRATEGY_BY_MOD = 0,
	VDEC_CAPACITY_STRATEGY_BY_CHN = 1,
	VDEC_CAPACITY_STRATEGY_BUTT
} VDEC_CAPACITY_STRATEGY_E;

typedef struct _VDEC_VIDEO_MOD_PARAM_S {
	CVI_U32 u32MaxPicWidth;
	CVI_U32 u32MaxPicHeight;
	CVI_U32 u32MaxSliceNum;
	CVI_U32 u32VdhMsgNum;
	CVI_U32 u32VdhBinSize;
	CVI_U32 u32VdhExtMemLevel;
} VDEC_VIDEO_MOD_PARAM_S;

typedef struct _VDEC_PICTURE_MOD_PARAM_S {
	CVI_U32 u32MaxPicWidth;
	CVI_U32 u32MaxPicHeight;
	CVI_BOOL bSupportProgressive;
	CVI_BOOL bDynamicAllocate;
	VDEC_CAPACITY_STRATEGY_E enCapStrategy;
} VDEC_PICTURE_MOD_PARAM_S;

typedef struct _VDEC_MOD_PARAM_S {
	VB_SOURCE_E enVdecVBSource; /* RW, Range: [1, 3];  frame buffer mode  */
	CVI_U32 u32MiniBufMode; /* RW, Range: [0, 1];  stream buffer mode */
	CVI_U32 u32ParallelMode; /* RW, Range: [0, 1];  VDH working mode   */
	VDEC_VIDEO_MOD_PARAM_S stVideoModParam;
	VDEC_PICTURE_MOD_PARAM_S stPictureModParam;
} VDEC_MOD_PARAM_S;

typedef struct _VDEC_USER_DATA_ATTR_S {
	CVI_BOOL bEnable;
	CVI_U32 u32MaxUserDataLen;
} VDEC_USER_DATA_ATTR_S;

// TODO: refinememt for hardcode
#define ENUM_ERR_VDEC_INVALID_CHNID \
	((CVI_S32)(0xC0000000L | ((CVI_ID_VDEC) << 16) | ((4) << 13) | (2)))
#define ENUM_ERR_VDEC_ERR_INIT \
	((CVI_S32)(0xC0000000L | ((CVI_ID_VDEC) << 16) | ((4) << 13) | (64)))

typedef enum {
	CVI_ERR_VDEC_INVALID_CHNID = ENUM_ERR_VDEC_INVALID_CHNID,
	CVI_ERR_VDEC_ILLEGAL_PARAM,
	CVI_ERR_VDEC_EXIST,
	CVI_ERR_VDEC_UNEXIST,
	CVI_ERR_VDEC_NULL_PTR,
	CVI_ERR_VDEC_NOT_CONFIG,
	CVI_ERR_VDEC_NOT_SUPPORT,
	CVI_ERR_VDEC_NOT_PERM,
	CVI_ERR_VDEC_INVALID_PIPEID,
	CVI_ERR_VDEC_INVALID_GRPID,
	CVI_ERR_VDEC_NOMEM,
	CVI_ERR_VDEC_NOBUF,
	CVI_ERR_VDEC_BUF_EMPTY,
	CVI_ERR_VDEC_BUF_FULL,
	CVI_ERR_VDEC_SYS_NOTREADY,
	CVI_ERR_VDEC_BADADDR,
	CVI_ERR_VDEC_BUSY,
	CVI_ERR_VDEC_SIZE_NOT_ENOUGH,
	CVI_ERR_VDEC_INVALID_VB,
    ///========//
	CVI_ERR_VDEC_ERR_INIT = ENUM_ERR_VDEC_ERR_INIT,
	CVI_ERR_VDEC_ERR_INVALID_RET,
	CVI_ERR_VDEC_ERR_SEQ_OPER,
	CVI_ERR_VDEC_ERR_VDEC_MUTEX,
	CVI_ERR_VDEC_ERR_SEND_FAILED,
	CVI_ERR_VDEC_ERR_GET_FAILED,
	CVI_ERR_VDEC_BUTT
} VDEC_RECODE_E_ERRTYPE;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  __CVI_COMM_VDEC_H__ */
