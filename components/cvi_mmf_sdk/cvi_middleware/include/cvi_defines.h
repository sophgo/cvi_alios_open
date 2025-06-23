/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_defines.h
 * Description:
 *   The common definitions per chip capability.
 */
 /******************************************************************************         */

#ifndef __U_CVI_DEFINES_H__
#define __U_CVI_DEFINES_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CVI_CHIP_NAME "CV184X"

#ifndef __CV184X__
#define __CV184X__
#endif

#define CHIP_TEST  0x0

#define CVIU01 0x1
#define CVIU02 0x2

#define CVI_COLDBOOT 0x1
#define CVI_WDTBOOT 0x2
#define CVI_SUSPENDBOOT 0x3
#define CVI_WARMBOOT 0x4

/* chip ID list */
enum ENUM_CHIP_ID {
	E_CHIPID_CV1822 = 0,	//0
	E_CHIPID_CV1832,		//1
	E_CHIPID_CV1835,		//2
	E_CHIPID_CV1838,		//3
	E_CHIPID_CV1829,		//4
	E_CHIPID_CV1826,		//5
	E_CHIPID_CV1821,		//6
	E_CHIPID_CV1820,		//7
	E_CHIPID_CV1823,		//8
	E_CHIPID_CV1825,		//9
// cv181 chips
	E_CHIPID_CV1820A,		//10
	E_CHIPID_CV1821A,		//11
	E_CHIPID_CV1822A,		//12
	E_CHIPID_CV1823A,		//13
	E_CHIPID_CV1825A,		//14
	E_CHIPID_CV1826A,		//15
	E_CHIPID_CV1810C,		//16
	E_CHIPID_CV1811C,		//17
	E_CHIPID_CV1812C,		//18
	E_CHIPID_CV1811H,		//19
	E_CHIPID_CV1812H,		//20
	E_CHIPID_CV1813H,		//21
// cv180 chips
	E_CHIPID_CV1800B,		//22
	E_CHIPID_CV1801B,		//23
	E_CHIPID_CV1800C,		//24
	E_CHIPID_CV1801C,		//25
// CV184X chips
	E_CHIPID_BM1688,		//26
	E_CHIPID_CV186AH,		//27
//cv184x chip
	E_CHIPID_CV184X,		//28
	E_CHIPID_UNKOWN = 0xFFFFFFFF,   //0xFFFFFFFF
};


#define IS_CHIP_CV183X(x) (((x) == E_CHIPID_CV1829) || ((x) == E_CHIPID_CV1832) \
						|| ((x) == E_CHIPID_CV1835) || ((x) == E_CHIPID_CV1838))
#define IS_CHIP_CV182X(x) (((x) == E_CHIPID_CV1820) || ((x) == E_CHIPID_CV1821) \
						|| ((x) == E_CHIPID_CV1822) || ((x) == E_CHIPID_CV1823) \
						|| ((x) == E_CHIPID_CV1825) || ((x) == E_CHIPID_CV1826))

#define IS_CHIP_CV181X(x) (((x) == E_CHIPID_CV1820A) || ((x) == E_CHIPID_CV1821A) \
						|| ((x) == E_CHIPID_CV1822A) || ((x) == E_CHIPID_CV1823A) \
						|| ((x) == E_CHIPID_CV1825A) || ((x) == E_CHIPID_CV1826A) \
						|| ((x) == E_CHIPID_CV1810C) || ((x) == E_CHIPID_CV1811C) \
						|| ((x) == E_CHIPID_CV1812C) || ((x) == E_CHIPID_CV1811H) \
						|| ((x) == E_CHIPID_CV1812H) || ((x) == E_CHIPID_CV1813H))

#define IS_CHIP_CV180X(x) (((x) == E_CHIPID_CV1800B) || ((x) == E_CHIPID_CV1801B) \
							|| ((x) == E_CHIPID_CV1800C) || ((x) == E_CHIPID_CV1801C))

#define IS_CHIP_PKG_TYPE_QFN(x) (((x) == E_CHIPID_CV1820A) || ((x) == E_CHIPID_CV1821A) \
						|| ((x) == E_CHIPID_CV1822A) || ((x) == E_CHIPID_CV1810C) \
						|| ((x) == E_CHIPID_CV1811C) || ((x) == E_CHIPID_CV1812C))

#define MMF_VER_PRIX "_MMF_V"

#define ALIGN_NUM 4

#define LUMA_PHY_ALIGN               16

#define DEFAULT_ALIGN                64
#define MAX_ALIGN                    1024
#define SEG_CMP_LENGTH               256

/* For VENC */
#define VC_MAX_CHN_NUM               32
#define VENC_MAX_NAME_LEN            16
#define VENC_MAX_CHN_NUM             512
#define VEDU_IP_NUM                  1
#define H264E_MAX_WIDTH              4096
#define H264E_MAX_HEIGHT             4096
#define H264E_MIN_WIDTH              114
#define H264E_MIN_HEIGHT             114
#define H265E_MAX_WIDTH              4096
#define H265E_MAX_HEIGHT             4096
#define H265E_MIN_WIDTH              114
#define H265E_MIN_HEIGHT             114
#define JPEGE_MAX_WIDTH              8192
#define JPEGE_MAX_HEIGHT             8192
#define JPEGE_MIN_WIDTH              32
#define JPEGE_MIN_HEIGHT             32
#define JPGE_MAX_NUM                 1
#define VENC_MAX_ROI_NUM             8
#define H264E_MIN_HW_INDEX           0
#define H264E_MAX_HW_INDEX           11
#define H264E_MIN_VW_INDEX           0
#define H264E_MAX_VW_INDEX           3
#define VENC_QP_HISGRM_NUM           52
#define MAX_TILE_NUM                 1
#define H265E_ADAPTIVE_FRAME_TYPE    4
#define H265E_ADAPTIVE_QP_TYPE       5

#define VENC_MIN_INPUT_FRAME_RATE    1
#define VENC_MAX_INPUT_FRAME_RATE    240

#define VENC_MAX_RECEIVE_SOURCE  4

#define VENC_PIC_RECEIVE_SOURCE0 0
#define VENC_PIC_RECEIVE_SOURCE1 1
#define VENC_PIC_RECEIVE_SOURCE2 2
#define VENC_PIC_RECEIVE_SOURCE3 3

#define VENC_ALIGN_W             32
#define VENC_ALIGN_H             16

/* For RC */
#define RC_TEXTURE_THR_SIZE          16
#define MIN_BITRATE         2
#define MAX_BITRATE         (100*1024)

#define JPEG_MAX_CHN_NUM        512
/* For VDEC */
#define VDEC_MAX_CHN_NUM        512
#define VDH_MAX_CHN_NUM         0
#define VEDU_CHN_START          VDH_MAX_CHN_NUM
#define VEDU_H264D_ERRRATE      10
#define VEDU_H264D_FULLERR      100

#define H264D_ALIGN_W           64
#define H264D_ALIGN_H           64
#define H265D_ALIGN_W           64
#define H265D_ALIGN_H           64
#define JPEGD_ALIGN_W           64
#define JPEGD_ALIGN_H           16
#define JPEGD_RGB_ALIGN         16

#define H264D_ALIGN_FRM          0x1000
#define H265D_ALIGN_FRM          0x1000
#define JPEGD_ALIGN_FRM          0x1000

#define H264D_MAX_SPS           32
#define H264D_MIN_SPS           1
#define H264D_MAX_PPS           256
#define H264D_MIN_PPS           1
#define H264D_MAX_SLICE         300
#define H264D_MIN_SLICE         1

#define H265D_MAX_VPS           16
#define H265D_MIN_VPS           1
#define H265D_MAX_SPS           16
#define H265D_MIN_SPS           1
#define H265D_MAX_PPS           64
#define H265D_MIN_PPS           1
#define H265D_MAX_SLICE         200
#define H265D_MIN_SLICE         1

#define VEDU_H264D_MAX_WIDTH    2880
#define VEDU_H264D_MAX_HEIGHT   1920
#define VEDU_H264D_MIN_WIDTH    114
#define VEDU_H264D_MIN_HEIGHT   114

#define VEDU_H265D_MAX_WIDTH    2880
#define VEDU_H265D_MAX_HEIGHT   1920
#define VEDU_H265D_MIN_WIDTH    114
#define VEDU_H265D_MIN_HEIGHT   114

#define JPEGD_IP_NUM            1
#define JPEGD_MAX_WIDTH         8192
#define JPEGD_MAX_HEIGHT        8192
#define JPEGD_MIN_WIDTH         8
#define JPEGD_MIN_HEIGHT        8

/* For Region */
#define RGN_MIN_WIDTH             2
#define RGN_MIN_HEIGHT            2

#define RGN_COVER_MAX_WIDTH       3840
#define RGN_COVER_MAX_HEIGHT      2160
#define RGN_COVER_MIN_X           0
#define RGN_COVER_MIN_Y           0
#define RGN_COVER_MAX_X           (RGN_COVER_MAX_WIDTH - RGN_MIN_WIDTH)
#define RGN_COVER_MAX_Y           (RGN_COVER_MAX_HEIGHT - RGN_MIN_HEIGHT)

#define RGN_COVEREX_MAX_NUM       4
#define RGN_COVEREX_MAX_WIDTH     3840
#define RGN_COVEREX_MAX_HEIGHT    2160
#define RGN_COVEREX_MIN_X         0
#define RGN_COVEREX_MIN_Y         0
#define RGN_COVEREX_MAX_X         (RGN_COVEREX_MAX_WIDTH - RGN_MIN_WIDTH)
#define RGN_COVEREX_MAX_Y         (RGN_COVEREX_MAX_HEIGHT - RGN_MIN_HEIGHT)

#define RGN_OVERLAY_MAX_WIDTH     3840
#define RGN_OVERLAY_MAX_HEIGHT    2160
#define RGN_OVERLAY_MIN_X         0
#define RGN_OVERLAY_MIN_Y         0
#define RGN_OVERLAY_MAX_X         (RGN_OVERLAY_MAX_WIDTH - RGN_MIN_WIDTH)
#define RGN_OVERLAY_MAX_Y         (RGN_OVERLAY_MAX_HEIGHT - RGN_MIN_HEIGHT)

#define RGN_OVERLAYEX_MAX_WIDTH   3840
#define RGN_OVERLAYEX_MAX_HEIGHT  2160
#define RGN_OVERLAYEX_MIN_X       0
#define RGN_OVERLAYEX_MIN_Y       0
#define RGN_OVERLAYEX_MAX_X       (RGN_OVERLAYEX_MAX_WIDTH - RGN_MIN_WIDTH)
#define RGN_OVERLAYEX_MAX_Y       (RGN_OVERLAYEX_MAX_HEIGHT - RGN_MIN_HEIGHT)

#define RGN_MOSAIC_MAX_NUM        8
#define RGN_MOSAIC_WIDTH_ALIGN    8
#define RGN_MOSAIC_HEIGHT_ALIGN   8

#define RGN_MOSAIC_MIN_WIDTH      8
#define RGN_MOSAIC_MIN_HEIGHT     8
#define RGN_MOSAIC_MAX_WIDTH      3840
#define RGN_MOSAIC_MAX_HEIGHT     2160
#define RGN_MOSAIC_MIN_X          0
#define RGN_MOSAIC_MIN_Y          0
#define RGN_MOSAIC_MAX_X          (RGN_MOSAIC_MAX_WIDTH - RGN_MOSAIC_MIN_WIDTH)
#define RGN_MOSAIC_MAX_Y          (RGN_MOSAIC_MAX_HEIGHT - RGN_MOSAIC_MIN_HEIGHT)

// vpss rgn define
#define RGN_MAX_LAYER_VPSS        2
#define RGN_ODEC_LAYER_VPSS       0
#define RGN_NORMAL_LAYER_VPSS     1
#define RGN_MAX_NUM_VPSS          8
#define RGN_EX_MAX_NUM_VPSS       16
#define RGN_EX_MAX_WIDTH          3840

#define RGN_MAX_BUF_NUM           2
#define RGN_MAX_NUM               108

/*************************************/
#define VENC_MAX_SSE_NUM            8

/* For VI */
/* number of channel and device on video input unit of chip
 * Note! VI_MAX_CHN_NUM is NOT equal to VI_MAX_DEV_NUM
 * multiplied by VI_MAX_CHN_NUM, because all VI devices
 * can't work at mode of 4 channels at the same time.
 */
#define VI_MAX_PHY_DEV_NUM        3
#define VI_MAX_VIR_DEV_NUM        3
#define VI_MAX_DEV_NUM            (VI_MAX_PHY_DEV_NUM + VI_MAX_VIR_DEV_NUM)
#define VI_MAX_DEV_SWITCH_DEPTH   2
#define VI_MAX_DEV_SWITCH_NUM     (1 << VI_MAX_DEV_SWITCH_DEPTH)
#define VI_MAX_PHY_PIPE_NUM       4
#define VI_MAX_VIR_PIPE_NUM       3
#define VI_MAX_PIPE_NUM           (VI_MAX_PHY_PIPE_NUM + VI_MAX_VIR_PIPE_NUM)
#define VI_MAX_WDR_NUM            1

#define VI_MAX_VIR_CHN_NUM          2
#define VI_MAX_PHY_CHN_NUM          6
#define VI_MAX_EXT_CHN_NUM          2
#define VI_MAX_CHN_NUM              1
#define VI_EXT_CHN_START            VI_MAX_CHN_NUM
#define VI_MAX_EXTCHN_BIND_PER_CHN  1

#define VI_MAX_WDR_FRAME_NUM    2
#define VI_MAX_NODE_NUM         3
#define VIPROC_IP_NUM           1
#define VICAP_IP_NUM            1

#define VI_DEV_MIN_WIDTH        128
#define VI_DEV_MIN_HEIGHT       64
#define VI_DEV_MAX_WIDTH        3840
#define VI_DEV_MAX_HEIGHT       2160
#define VI_FPN_MAX_WIDTH        VI_DEV_MAX_WIDTH
#define VI_FPN_MAX_HEIGHT       VI_DEV_MAX_HEIGHT

#define VI_PIPE_OFFLINE_MIN_WIDTH           128
#define VI_PIPE_OFFLINE_MIN_HEIGHT          64
#define VI_PIPE_OFFLINE_MAX_WIDTH           3840
#define VI_PIPE_OFFLINE_MAX_HEIGHT          2160

#define VI_PIPE_ONLINE_MIN_WIDTH            128
#define VI_PIPE_ONLINE_MIN_HEIGHT           64
#define VI_PIPE_ONLINE_MAX_WIDTH            3840
#define VI_PIPE_ONLINE_MAX_HEIGHT           2160

#define VI_PIPE0_MAX_WIDTH                  3840
#define VI_PIPE0_MAX_HEIGHT                 2160
#define VI_PIPE1_MAX_WIDTH                  3840
#define VI_PIPE1_MAX_HEIGHT                 2160
#define VI_PIPE2_MAX_WIDTH                  3840
#define VI_PIPE2_MAX_HEIGHT                 2160
#define VI_PIPE3_MAX_WIDTH                  3840
#define VI_PIPE3_MAX_HEIGHT                 2160

#define VI_PIPE_WDR_FIRST_MAX_WIDTH         VI_PIPE1_MAX_WIDTH
#define VI_PIPE_FUSION_MAX_WIDTH            2880
#define VI_PIPE_FUSION_MAX_HEIGHT           2160

#define VI_PHYCHN_OFFLINE_MIN_WIDTH         128
#define VI_PHYCHN_OFFLINE_MIN_HEIGHT        64
#define VI_PHYCHN_OFFLINE_MAX_WIDTH         3840
#define VI_PHYCHN_OFFLINE_MAX_HEIGHT        2160

#define VI_PHYCHN_ONLINE_MIN_WIDTH          128
#define VI_PHYCHN_ONLINE_MIN_HEIGHT         64
#define VI_PHYCHN_ONLINE_MAX_WIDTH          3840
#define VI_PHYCHN_ONLINE_MAX_HEIGHT         2160

#define VI_CMP_PARAM_SIZE                   152

#define VI_PIXEL_FORMAT                     PIXEL_FORMAT_NV12

#define VI_VPSS_EXTRA_BUF 0

#define VI_CHN_0_BUF                    (2 + VI_VPSS_EXTRA_BUF)
#define VI_CHN_1_BUF                    (2 + VI_VPSS_EXTRA_BUF)
#define VI_CHN_2_BUF                    (2 + VI_VPSS_EXTRA_BUF)
#define VI_CHN_3_BUF                    (2 + VI_VPSS_EXTRA_BUF)
#define VI_BUF                          (VI_CHN_0_BUF + VI_CHN_1_BUF + VI_CHN_2_BUF + VI_CHN_3_BUF)

/* For VO */
#define VO_MAX_DEV_NUM				1
#define VO_MAX_VIDEO_LAYER_IN_DEV		1
#define VO_MAX_GRAPHIC_LAYER_IN_DEV		1
#define VO_MAX_VIDEO_LAYER_NUM			(VO_MAX_DEV_NUM * VO_MAX_VIDEO_LAYER_IN_DEV)
#define VO_MAX_GRAPHIC_LAYER_NUM		(VO_MAX_DEV_NUM * VO_MAX_GRAPHIC_LAYER_IN_DEV)
#define VO_MAX_LAYER_NUM			(VO_MAX_VIDEO_LAYER_NUM + VO_MAX_GRAPHIC_LAYER_NUM)
#define VO_LAYER_V0				0
#define VO_LAYER_G0				1
#define VO_MAX_CHN_NUM				1
#define VO_MIN_CHN_WIDTH			32
#define VO_MIN_CHN_HEIGHT			32
#define VO_CHN_THRESHOLD			2
#define VO_CHN_WORKQ				1

/* For AUDIO */
#define AUD_AI_DEV_MAX_NUM          1
#define AUD_AO_DEV_MIN_NUM          0
#define AUD_AO_DEV_MAX_NUM          2
#define AUD_AIO_MAX_NUM             2
#define AUD_AENC_MAX_CHN_NUM        3
#define AUD_ADEC_MAX_CHN_NUM        3

#define AUD_AI_MAX_CHN_NUM          2
#define AUD_AO_MAX_CHN_NUM          1
#define AUD_AO_SYSCHN_CHNID         (AUD_AO_MAX_CHN_NUM - 1)

#define AUD_AIO_MAX_CHN_NUM         ((AUD_AO_MAX_CHN_NUM > AUD_AI_MAX_CHN_NUM) ? AUD_AO_MAX_CHN_NUM:AUD_AI_MAX_CHN_NUM)

#define AUD_MAX_FRAME_NUM    300       /* max count of audio frame in Buffer */
#define AUD_MAX_VOICE_POINT_NUM    1280      /* max sample per frame for voice encode */
#define AUD_MAX_AUDIO_POINT_NUM    2048     /* max sample per frame for all encoder */
#define AUD_MAX_CHANNEL_NUM      3         /* Maximum number of audio channels supported */
#define AUD_MAX_STREAM_LEN   (4 * 4096)  /* Maximum length of an audio stream in bytes */

#define AUD_MAX_FILE_PATH_LEN          256           /* Maximum length of the file path for an audio file */
#define AUD_MAX_FILE_NAME_LEN          256           /* Maximum length of the file name for an audio file */
#define AUD_MAX_VQE_CUSTOMIZE_NAME     64      /* Maximum length of the name for a custom VQE setting */

/* For VPSS */
#define VPSS_DEVICE_NUM          2
#define VPSS_IP_NUM              4
#define VPSS_MAX_GRP_NUM         16
#define VPSS_ONLINE_NUM          3
#define VPSS_MAX_PHY_CHN_NUM     4
#define VPSS_MAX_CHN_NUM         (VPSS_MAX_PHY_CHN_NUM)
#define VPSS_MIN_IMAGE_WIDTH     16
#define VPSS_MIN_IMAGE_HEIGHT    16
#define VPSS_MAX_IMAGE_WIDTH     3840
#define VPSS_MAX_IMAGE_HEIGHT    2160

#define VPSS_CHN0_MAX_WIDTH      3840
#define VPSS_CHN1_MAX_WIDTH      1920
#define VPSS_CHN2_MAX_WIDTH      1920
#define VPSS_CHN3_MAX_WIDTH      1920

#define VPSS_HW_LIMIT_WIDTH      3840
#define VPSS_HW_LIMIT_HEIGHT     2160

#define VPSS_MAX_ZOOMIN          128
#define VPSS_MAX_ZOOMOUT         128
#define VPSS_RECT_NUM            4


/*For Gdc*/
#define LDC_ALIGN                      64
#define LDC_MIN_IMAGE_WIDTH            640
#define LDC_MIN_IMAGE_HEIGHT           480

#define SPREAD_MIN_IMAGE_WIDTH          640
#define SPREAD_MIN_IMAGE_HEIGHT         480

/* For GDC */
#define GDC_IP_NUM                 2
#define GDC_PROC_JOB_INFO_NUM      (500)
#define GDC_MAX_REGION_NUM         4

/* For DPU */
#define DPU_ALIGN                   16
#define DPU_IP_NUM              	1
#define DPU_MAX_GRP_NUM        		8
#define DPU_MAX_CHN_NUM        		2
#define DPU_PIPE_IN_NUM      		2
#define DPU_PIPE_OUT_NUM     		2
#define DPU_MIN_IMAGE_WIDTH         64
#define DPU_MIN_IMAGE_HEIGHT        64
#define DPU_MAX_IMAGE_WIDTH         1920
#define DPU_MAX_IMAGE_HEIGHT        1080
#define DPU_WAITQ_DEPTH_IN        	2
#define DPU_WORKQ_DEPTH_IN        	1
#define DPU_DONEQ_DEPTH_IN        	1
#define DPU_WAITQ_DEPTH_OUT        	1
#define DPU_WORKQ_DEPTH_OUT        	1
#define DPU_DONEQ_DEPTH_OUT        	2

/* For STITCH*/
#define STITCH_MAX_SRC_NUM         4
#define STITCH_ALIGN                   16

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __U_CVI_DEFINES_H__ */

