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

#define CVI_CHIP_TEST  0x0

#ifndef CVI_CHIP_NAME
#define CVI_CHIP_NAME "CV181X"
#endif

#define CVI1822 0x0
#define CVI1832 0x1
#define CVI1835 0x2
#define CVI1838 0x3
#define CVI1829 0x4
#define CVI1826 0x5
#define CVI1821 0x6
#define CVI1820 0x7
#define CVI1823 0x8
#define CVI1825 0x9
#define CV1820A 10
#define CV1821A 11
#define CV1822A 12
#define CV1823A 13
#define CV1825A 14
#define CV1826A 15
#define CV1810C 16
#define CV1811C 17
#define CV1812C 18
#define CV1811H 19
#define CV1812H 20
#define CV1813H 21
#define CV1800B 22
#define CV180ZB 22
#define CV1801B 23
#define CV1800C 24
#define CV1801C 25


#define CVIU01 0x1
#define CVIU02 0x2

#define CVI_COLDBOOT 0x1
#define CVI_WDTBOOT 0x2
#define CVI_SUSPENDBOOT 0x3
#define CVI_WARMBOOT 0x4

#define IS_CHIP_CV183X(x) (((x) == CVI1829) || ((x) == CVI1832) || ((x) == CVI1835) || ((x) == CVI1838))
#define IS_CHIP_CV182X(x) (((x) == CVI1820) || ((x) == CVI1821) || ((x) == CVI1822) \
						|| ((x) == CVI1823) || ((x) == CVI1825) || ((x) == CVI1826))

#define IS_CHIP_CV181X(x) (((x) == CV1820A) || ((x) == CV1821A) || ((x) == CV1822A) \
						|| ((x) == CV1823A) || ((x) == CV1825A) || ((x) == CV1826A) \
						|| ((x) == CV1810C) || ((x) == CV1811C) || ((x) == CV1812C) \
						|| ((x) == CV1811H) || ((x) == CV1812H) || ((x) == CV1813H))

#define IS_CHIP_PKG_TYPE_QFN(x) (((x) == CV1820A) || ((x) == CV1821A) || ((x) == CV1822A) \
						|| ((x) == CV1810C) || ((x) == CV1811C) || ((x) == CV1812C) \
						|| IS_CHIP_CV180X(x))

#define IS_CHIP_CV180X(x) (((x) == CV1800B) || ((x) == CV1801B) \
							|| ((x) == CV1800C) || ((x) == CV1801C))

#define MMF_VER_PRIX "_MMF_V"

#define ALIGN_NUM 4

#define LUMA_PHY_ALIGN               16

#define DEFAULT_ALIGN                64
#define MAX_ALIGN                    1024
#define SEG_CMP_LENGTH               256

/* For VENC */
#define VENC_MAX_NAME_LEN            16
#define VENC_MAX_CHN_NUM             64
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

/* For VDEC */
#define VDEC_MAX_CHN_NUM        64
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
#define JPEGD_MAX_WIDTH         2880
#define JPEGD_MAX_HEIGHT        1920
#define JPEGD_MIN_WIDTH         8
#define JPEGD_MIN_HEIGHT        8

/* For Region */
#define RGN_MIN_WIDTH             2
#define RGN_MIN_HEIGHT            2

#define RGN_COVER_MIN_X           -8192
#define RGN_COVER_MIN_Y           -8192
#define RGN_COVER_MAX_X           8190
#define RGN_COVER_MAX_Y           8190
#define RGN_COVER_MAX_WIDTH       8192
#define RGN_COVER_MAX_HEIGHT      8192

#define RGN_COVEREX_MIN_X         -8192
#define RGN_COVEREX_MIN_Y         -8192
#define RGN_COVEREX_MAX_X         8190
#define RGN_COVEREX_MAX_Y         8190
#define RGN_COVEREX_MAX_WIDTH     8192
#define RGN_COVEREX_MAX_HEIGHT    8192

#define RGN_OVERLAY_MIN_X         0
#define RGN_OVERLAY_MIN_Y         0
#define RGN_OVERLAY_MAX_X         8190
#define RGN_OVERLAY_MAX_Y         8190
#define RGN_OVERLAY_MAX_WIDTH     8192
#define RGN_OVERLAY_MAX_HEIGHT    8192

#define RGN_OVERLAYEX_MIN_X       0
#define RGN_OVERLAYEX_MIN_Y       0
#define RGN_OVERLAYEX_MAX_X       8190
#define RGN_OVERLAYEX_MAX_Y       8190
#define RGN_OVERLAYEX_MAX_WIDTH   8192
#define RGN_OVERLAYEX_MAX_HEIGHT  8192

#define RGN_MOSAIC_X_ALIGN        4
#define RGN_MOSAIC_Y_ALIGN        2
#define RGN_MOSAIC_WIDTH_ALIGN    4
#define RGN_MOSAIC_HEIGHT_ALIGN   4

#define RGN_MOSAIC_MIN_X          0
#define RGN_MOSAIC_MIN_Y          0
#define RGN_MOSAIC_MAX_X          8190
#define RGN_MOSAIC_MAX_Y          8190
#define RGN_MOSAIC_MIN_WIDTH      32
#define RGN_MOSAIC_MIN_HEIGHT     32
#define RGN_MOSAIC_MAX_WIDTH      8192
#define RGN_MOSAIC_MAX_HEIGHT     8192

// vpss rgn define
#define RGN_MAX_LAYER_VPSS        2
#define RGN_ODEC_LAYER_VPSS       0
#if defined(__CV181X__)
#define RGN_NORMAL_LAYER_VPSS     1
#endif

#if defined(__CV180X__)
#define RGN_NORMAL_LAYER_VPSS     0
#endif

#define RGN_MAX_NUM_VPSS          8
#define RGN_EX_MAX_NUM_VPSS       16
#define RGN_EX_MAX_WIDTH          2304

// vo rgn define
#define RGN_MAX_NUM_VO            8

#define RGN_MAX_BUF_NUM           2
#define RGN_MAX_NUM               108

/*************************************/
#define VENC_MAX_SSE_NUM            8
#define CVI_MAX_SENSOR_NUM          2

/* For VI */
/* number of channel and device on video input unit of chip
 * Note! VI_MAX_CHN_NUM is NOT equal to VI_MAX_DEV_NUM
 * multiplied by VI_MAX_CHN_NUM, because all VI devices
 * can't work at mode of 4 channels at the same time.
 */
#define VI_MAX_PHY_DEV_NUM        3
#define VI_MAX_VIR_DEV_NUM        2
#define VI_MAX_DEV_NUM            (VI_MAX_PHY_DEV_NUM + VI_MAX_VIR_DEV_NUM)
#define VI_MAX_PHY_PIPE_NUM       4
#define VI_MAX_VIR_PIPE_NUM       2
#define VI_MAX_PIPE_NUM           (VI_MAX_PHY_PIPE_NUM + VI_MAX_VIR_PIPE_NUM)
#define VI_MAX_WDR_NUM            1

#define VI_MAX_VIR_CHN_NUM          2
#define VI_MAX_PHY_CHN_NUM          3
#define VI_MAX_EXT_CHN_NUM          2
#define VI_MAX_CHN_NUM              (VI_MAX_PHY_CHN_NUM + VI_MAX_VIR_CHN_NUM)
#define VI_EXT_CHN_START            VI_MAX_CHN_NUM
#define VI_MAX_EXTCHN_BIND_PER_CHN  1

#define VI_MAX_WDR_FRAME_NUM    2
#define VI_MAX_NODE_NUM         3
#define VIPROC_IP_NUM           1
#define VICAP_IP_NUM            1

#define VI_MAX_SPLIT_NODE_NUM   3

#define VI_DEV_MIN_WIDTH        120
#define VI_DEV_MIN_HEIGHT       120
#define VI_DEV_MAX_WIDTH        4608
#define VI_DEV_MAX_HEIGHT       4608
#define VI_FPN_MAX_WIDTH        4096
#define VI_FPN_MAX_HEIGHT       VI_DEV_MAX_HEIGHT

#define VI_PIPE_OFFLINE_MIN_WIDTH           120
#define VI_PIPE_OFFLINE_MIN_HEIGHT          120
#define VI_PIPE_OFFLINE_MAX_WIDTH           4096
#define VI_PIPE_OFFLINE_MAX_HEIGHT          2160

#define VI_PIPE_ONLINE_MIN_WIDTH            120
#define VI_PIPE_ONLINE_MIN_HEIGHT           120
#define VI_PIPE_ONLINE_MAX_WIDTH            2688
#define VI_PIPE_ONLINE_MAX_HEIGHT           1944

#define VI_PIPE0_MAX_WIDTH                  4096
#define VI_PIPE0_MAX_HEIGHT                 2160
#define VI_PIPE1_MAX_WIDTH                  4096
#define VI_PIPE1_MAX_HEIGHT                 2160
#define VI_PIPE2_MAX_WIDTH                  2688
#define VI_PIPE2_MAX_HEIGHT                 1944
#define VI_PIPE3_MAX_WIDTH                  2688
#define VI_PIPE3_MAX_HEIGHT                 1944

#define VI_PIPE_WDR_FIRST_MAX_WIDTH         VI_PIPE1_MAX_WIDTH
#define VI_PIPE_FUSION_MAX_WIDTH            2688
#define VI_PIPE_FUSION_MAX_HEIGHT           1944

#define VI_PHYCHN_OFFLINE_MIN_WIDTH         120
#define VI_PHYCHN_OFFLINE_MIN_HEIGHT        120
#define VI_PHYCHN_OFFLINE_MAX_WIDTH         4608
#define VI_PHYCHN_OFFLINE_MAX_HEIGHT        4608

#define VI_PHYCHN_ONLINE_MIN_WIDTH          120
#define VI_PHYCHN_ONLINE_MIN_HEIGHT         120
#define VI_PHYCHN_ONLINE_MAX_WIDTH          2048
#define VI_PHYCHN_ONLINE_MAX_HEIGHT         4608

#define VI_CMP_PARAM_SIZE                   152

#define VI_PIXEL_FORMAT                     PIXEL_FORMAT_NV21

#define CVI_VI_VPSS_EXTRA_BUF 0

#define CVI_VI_CHN_0_BUF                    (2 + CVI_VI_VPSS_EXTRA_BUF)
#define CVI_VI_CHN_1_BUF                    (2 + CVI_VI_VPSS_EXTRA_BUF)
#define CVI_VI_CHN_2_BUF                    (2 + CVI_VI_VPSS_EXTRA_BUF)
#define CVI_VI_CHN_3_BUF                    (2 + CVI_VI_VPSS_EXTRA_BUF)
#define CVI_VI_BUF                          (CVI_VI_CHN_0_BUF + CVI_VI_CHN_1_BUF + CVI_VI_CHN_2_BUF + CVI_VI_CHN_3_BUF)

/* For VO */
#define VO_MIN_CHN_WIDTH        32      /* channel minimal width */
#define VO_MIN_CHN_HEIGHT       32      /* channel minimal height */
#define VO_MAX_DEV_NUM          1       /* max dev num */
#define VO_MAX_LAYER_NUM        1       /* max layer num */
#define VO_MAX_CHN_NUM          1       /* max chn num */

/* For AUDIO */
#define AI_DEV_MAX_NUM          1
#define AO_DEV_MIN_NUM          0
#define AO_DEV_MAX_NUM          2
#define AIO_MAX_NUM             2
#define AENC_MAX_CHN_NUM        3
#define ADEC_MAX_CHN_NUM        3

#define AI_MAX_CHN_NUM          2
#define AO_MAX_CHN_NUM          1
#define AO_SYSCHN_CHNID         (AO_MAX_CHN_NUM - 1)

#define AIO_MAX_CHN_NUM         ((AO_MAX_CHN_NUM > AI_MAX_CHN_NUM) ? AO_MAX_CHN_NUM:AI_MAX_CHN_NUM)

/* For VPSS */
#define VPSS_IP_NUM              2
#define VPSS_DEV_0               0
#define VPSS_DEV_1               1
#define VPSS_MAX_GRP_NUM         16
#define VPSS_ONLINE_NUM          5
#define VPSS_ONLINE_GRP_0        0
#define VPSS_ONLINE_GRP_1        1
#define VPSS_ONLINE_GRP_2        2
#ifdef __CV181X__
#define VPSS_MAX_PHY_CHN_NUM     4	/* sc_d, sc_v1, sc_v2, sc_v3 */
#else
#define VPSS_MAX_PHY_CHN_NUM     3	/* sc_d, sc_v1, sc_v2 */
#endif
#define VPSS_MAX_CHN_NUM         (VPSS_MAX_PHY_CHN_NUM)
#define VPSS_MIN_IMAGE_WIDTH     32
#define VPSS_MAX_IMAGE_WIDTH            2880
#define VPSS_MAX_IMAGE_HEIGHT           2880
#define VPSS_MAX_ZOOMIN                 32
#define VPSS_MAX_ZOOMOUT                32

/* For GDC */
#define LDC_ALIGN                       64 /* ldc image size alignment constraints*/

#define GDC_IP_NUM                    1 /* number of hardware IPs of gdc*/
#define GDC_PROC_JOB_INFO_NUM         (500) /* maximum number of job to be processed*/
#define GDC_MIN_IMAGE_WIDTH            64 /* minimum width of the GDC image */
#define GDC_MIN_IMAGE_HEIGHT           64 /* minimum height of the GDC image */
#define GDC_MAX_IMAGE_WIDTH            4032 /* maximum width of the GDC image */
#define GDC_MAX_IMAGE_HEIGHT           4032 /* maximum height of the GDC image */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __U_CVI_DEFINES_H__ */

