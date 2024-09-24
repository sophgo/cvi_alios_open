/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_errno.h
 * Description:
 *   error code definition.
 */

#ifndef __CVI_ERRNO_H__
#define __CVI_ERRNO_H__

//#include "cvi_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#ifndef CVI_ERR_APPID
#define CVI_ERR_APPID  (0x00000000L)
#endif

typedef enum _ERR_LEVEL_E {
	EN_ERR_LEVEL_DEBUG = 0,  /* debug-level                                  */
	EN_ERR_LEVEL_INFO,       /* informational                                */
	EN_ERR_LEVEL_NOTICE,     /* normal but significant condition             */
	EN_ERR_LEVEL_WARNING,    /* warning conditions                           */
	EN_ERR_LEVEL_ERROR,      /* error conditions                             */
	EN_ERR_LEVEL_CRIT,       /* critical conditions                          */
	EN_ERR_LEVEL_ALERT,      /* action must be taken immediately             */
	EN_ERR_LEVEL_FATAL,      /* just for compatibility with previous version */
	EN_ERR_LEVEL_BUTT
} ERR_LEVEL_E;


/******************************************************************************	*/
/*|----------------------------------------------------------------|*/
/*| 11|   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |*/
/*|----------------------------------------------------------------|*/
/*|<--><--6bits----><----8bits---><--3bits---><------13bits------->|*/
/*******************************************************************************/

#define CVI_DEF_ERR(module, level, errid) \
	((CVI_S32)(0xC0000000L | (CVI_ERR_APPID) | ((module) << 16) | ((level) << 13) | (errid)))

/* NOTE! the following defined all common error code,		*/
/*** all module must reserved 0~63 for their common error code*/

typedef enum _EN_ERR_CODE_E {
	EN_ERR_INVALID_DEVID = 1, /* invalid device ID */
	EN_ERR_INVALID_CHNID = 2, /* invalid channel ID*/
	EN_ERR_ILLEGAL_PARAM = 3,
	/* at least one parameter is illegal*/
	/* eg, an illegal enumeration value */
	EN_ERR_EXIST         = 4, /* resource exists*/
	EN_ERR_UNEXIST       = 5, /* resource unexists */
	EN_ERR_NULL_PTR      = 6, /* using a NULL point*/
	EN_ERR_NOT_CONFIG    = 7,
	/* try to enable or initialize system, device*/
	/* or channel, before configing attribute*/
	EN_ERR_NOT_SUPPORT   = 8,
	/* operation or type is not supported by NOW*/
	EN_ERR_NOT_PERM      = 9,
	/* operation is not permitted*/
	/* eg, try to change static attribute*/
	EN_ERR_INVALID_PIPEID = 10,
	/* invalid pipe ID*/
	EN_ERR_INVALID_GRPID  = 11,
	/* invalid group ID*/
	EN_ERR_NOMEM         = 12,
	/* failure caused by malloc memory*/
	EN_ERR_NOBUF         = 13,
	/* failure caused by malloc buffer*/
	EN_ERR_BUF_EMPTY     = 14,
	/* no data in buffer */
	EN_ERR_BUF_FULL      = 15,
	/* no buffer for new data*/
	EN_ERR_SYS_NOTREADY  = 16,
	/* System is not ready, maybe not initialized or*/
	/* loaded. Returning the error code when opening*/
	/* a device file failed.*/
	EN_ERR_BADADDR       = 17,
	/* bad address,*/
	/* eg. used for copy_from_user & copy_to_user*/
	EN_ERR_BUSY          = 18,
	/* resource is busy,*/
	/* eg. destroy a venc chn without unregister it */
	EN_ERR_SIZE_NOT_ENOUGH = 19,
	/* buffer size is smaller than the actual size required */
	EN_ERR_INVALID_VB    = 20,
	/* invalid VB handle */
	EN_ERR_BUTT          = 63,
	/* maximum code, private error code of all modules*/
	/* must be greater than it */
} EN_ERR_CODE_E;

typedef enum _EN_VI_ERR_CODE_E {
	ERR_VI_FAILED_NOT_ENABLED = 64, /* device or channel not enable*/
	ERR_VI_FAILED_NOT_DISABLED, /* device not disable*/
	ERR_VI_FAILED_CHN_NOT_DISABLED, /* channel not disable*/
	ERR_VI_CFG_TIMEOUT, /* config timeout*/
	ERR_VI_NORM_UNMATCH, /* video norm of ADC and VIU is unmatch*/
	ERR_VI_INVALID_PHYCHNID, /* invalid phychn id*/
	ERR_VI_FAILED_NOTBIND, /* device or channel not bind */
	ERR_VI_FAILED_BINDED, /* device or channel not unbind */
	ERR_VI_DIS_PROCESS_FAIL /* dis process failed */
} EN_VI_ERR_CODE_E;

typedef enum _EN_VOU_ERR_CODE_E {
	EN_ERR_VO_DEV_NOT_CONFIG = 0x40,
	EN_ERR_VO_DEV_NOT_ENABLED = 0x41,
	EN_ERR_VO_DEV_HAS_ENABLED = 0x42,

	EN_ERR_VO_LAYER_NOT_ENABLED = 0x45,
	EN_ERR_VO_LAYER_NOT_DISABLED = 0x46,
	EN_ERR_VO_LAYER_NOT_CONFIG = 0x47,

	EN_ERR_VO_CHN_NOT_DISABLED = 0x48,
	EN_ERR_VO_CHN_NOT_ENABLED = 0x49,
	EN_ERR_VO_CHN_NOT_CONFIG = 0x4a,

	EN_ERR_VO_WAIT_TIMEOUT = 0x4e,
	EN_ERR_VO_INVALID_VFRAME = 0x4f,
	EN_ERR_VO_INVALID_RECT_PARA = 0x50,

	EN_ERR_VO_CHN_AREA_OVERLAP = 0x65,

	EN_ERR_VO_INVALID_LAYERID = 0x66,

	/* new added */
	ERR_VO_BUTT
} EN_VOU_ERR_CODE_E;

#define CVI_ERR_SYS_NULL_PTR         CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define CVI_ERR_SYS_NOTREADY         CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define CVI_ERR_SYS_NOT_PERM         CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define CVI_ERR_SYS_NOMEM            CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define CVI_ERR_SYS_ILLEGAL_PARAM    CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define CVI_ERR_SYS_BUSY             CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define CVI_ERR_SYS_NOT_SUPPORT      CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define CVI_ERR_SYS_REMAPPING        CVI_DEF_ERR(CVI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)

#define CVI_ERR_VB_NULL_PTR CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define CVI_ERR_VB_INVALID CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_VB)
#define CVI_ERR_VB_NOMEM CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define CVI_ERR_VB_NOBUF CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
#define CVI_ERR_VB_UNEXIST CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
#define CVI_ERR_VB_ILLEGAL_PARAM CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define CVI_ERR_VB_NOTREADY CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define CVI_ERR_VB_BUSY CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define CVI_ERR_VB_NOT_PERM CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define CVI_ERR_VB_SIZE_NOT_ENOUGH CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_SIZE_NOT_ENOUGH)
#define CVI_ERR_VB_2MPOOLS CVI_DEF_ERR(CVI_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_BUTT + 1)

#define CVI_ERR_VI_INVALID_PARA CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define CVI_ERR_VI_INVALID_DEVID CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
#define CVI_ERR_VI_INVALID_PIPEID CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_PIPEID)
#define CVI_ERR_VI_INVALID_CHNID CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
#define CVI_ERR_VI_INVALID_NULL_PTR CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define CVI_ERR_VI_FAILED_NOTCONFIG CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
#define CVI_ERR_VI_SYS_NOTREADY CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define CVI_ERR_VI_BUF_EMPTY CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
#define CVI_ERR_VI_BUF_FULL CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
#define CVI_ERR_VI_NOMEM CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define CVI_ERR_VI_NOT_SUPPORT CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define CVI_ERR_VI_BUSY CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define CVI_ERR_VI_NOT_PERM CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define CVI_ERR_VI_FAILED_NOT_ENABLED CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, ERR_VI_FAILED_NOT_ENABLED)
#define CVI_ERR_VI_FAILED_NOT_DISABLED CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, ERR_VI_FAILED_NOT_DISABLED)
#define CVI_ERR_VI_FAILED_CHN_NOT_DISABLED CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, ERR_VI_FAILED_CHN_NOT_DISABLED)
#define CVI_ERR_VI_CFG_TIMEOUT CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, ERR_VI_CFG_TIMEOUT)
#define CVI_ERR_VI_NORM_UNMATCH CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, ERR_VI_NORM_UNMATCH)
#define CVI_ERR_VI_INVALID_PHYCHNID CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, ERR_VI_INVALID_PHYCHNID)
#define CVI_ERR_VI_FAILED_NOTBIND CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, ERR_VI_FAILED_NOTBIND)
#define CVI_ERR_VI_FAILED_BINDED CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, ERR_VI_FAILED_BINDED)
#define CVI_ERR_VI_PIPE_EXIST CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
#define CVI_ERR_VI_PIPE_UNEXIST CVI_DEF_ERR(CVI_ID_VI, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)

#define CVI_ERR_VPSS_NULL_PTR CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define CVI_ERR_VPSS_NOTREADY CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define CVI_ERR_VPSS_INVALID_DEVID CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
#define CVI_ERR_VPSS_INVALID_CHNID CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
#define CVI_ERR_VPSS_EXIST CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
#define CVI_ERR_VPSS_UNEXIST CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
#define CVI_ERR_VPSS_NOT_SUPPORT CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define CVI_ERR_VPSS_NOT_PERM CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define CVI_ERR_VPSS_NOMEM CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define CVI_ERR_VPSS_NOBUF CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
#define CVI_ERR_VPSS_ILLEGAL_PARAM CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define CVI_ERR_VPSS_BUSY CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define CVI_ERR_VPSS_BUF_EMPTY CVI_DEF_ERR(CVI_ID_VPSS, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)

/* System define error code */
#define CVI_ERR_VO_BUSY CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define CVI_ERR_VO_NO_MEM CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define CVI_ERR_VO_NULL_PTR CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define CVI_ERR_VO_SYS_NOTREADY CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define CVI_ERR_VO_INVALID_DEVID CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
#define CVI_ERR_VO_INVALID_CHNID CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
#define CVI_ERR_VO_ILLEGAL_PARAM CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define CVI_ERR_VO_NOT_SUPPORT CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define CVI_ERR_VO_NOT_PERMIT CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define CVI_ERR_VO_INVALID_LAYERID CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_INVALID_LAYERID)
/* Device relative error code */
#define CVI_ERR_VO_DEV_NOT_CONFIG CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_CONFIG)
#define CVI_ERR_VO_DEV_NOT_ENABLED CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_ENABLED)
#define CVI_ERR_VO_DEV_HAS_ENABLED CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_HAS_ENABLED)
/* Video layer relative error code */
#define CVI_ERR_VO_VIDEO_NOT_ENABLED CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_LAYER_NOT_ENABLED)
#define CVI_ERR_VO_VIDEO_NOT_DISABLED CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_LAYER_NOT_DISABLED)
#define CVI_ERR_VO_VIDEO_NOT_CONFIG CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_LAYER_NOT_CONFIG)
/* Channel Relative error code */
#define CVI_ERR_VO_CHN_NOT_DISABLED CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_CHN_NOT_DISABLED)
#define CVI_ERR_VO_CHN_NOT_ENABLED CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_CHN_NOT_ENABLED)
#define CVI_ERR_VO_CHN_NOT_CONFIG CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_CHN_NOT_CONFIG)
#define CVI_ERR_VO_CHN_AREA_OVERLAP CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_CHN_AREA_OVERLAP)
/* MISCellaneous error code*/
#define CVI_ERR_VO_WAIT_TIMEOUT CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_WAIT_TIMEOUT)
#define CVI_ERR_VO_INVALID_VFRAME CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_INVALID_VFRAME)
#define CVI_ERR_VO_INVALID_RECT_PARA CVI_DEF_ERR(CVI_ID_VO, EN_ERR_LEVEL_ERROR, EN_ERR_VO_INVALID_RECT_PARA)

/* failure caused by malloc buffer */
#define CVI_ERR_GDC_NOBUF CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
#define CVI_ERR_GDC_NOMEM CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define CVI_ERR_GDC_BUF_EMPTY CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
#define CVI_ERR_GDC_NULL_PTR CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define CVI_ERR_GDC_ILLEGAL_PARAM CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define CVI_ERR_GDC_BUF_FULL CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
#define CVI_ERR_GDC_SYS_NOTREADY CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define CVI_ERR_GDC_NOT_SUPPORT CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define CVI_ERR_GDC_NOT_PERMITTED CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define CVI_ERR_GDC_BUSY CVI_DEF_ERR(CVI_ID_GDC, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)

/* System is not ready,maybe not initialized or loaded.
 * Returning the error code when opening a device file failed.
 */
#define CVI_ERR_RGN_SYS_NOTREADY CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
/* invalid device ID */
#define CVI_ERR_RGN_INVALID_DEVID CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
/* invalid channel ID */
#define CVI_ERR_RGN_INVALID_CHNID CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
/* at least one parameter is illegal ,eg, an illegal enumeration value	*/
#define CVI_ERR_RGN_ILLEGAL_PARAM CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
/* channel exists */
#define CVI_ERR_RGN_EXIST CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
/* region unexist */
#define CVI_ERR_RGN_UNEXIST CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
/* using a NULL point */
#define CVI_ERR_RGN_NULL_PTR CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
/* try to enable or initialize system, device or channel, before config attribute */
#define CVI_ERR_RGN_NOT_CONFIG CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define CVI_ERR_RGN_NOT_SUPPORT CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change static attribute */
#define CVI_ERR_RGN_NOT_PERM CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define CVI_ERR_RGN_NOMEM CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
/* failure caused by malloc buffer */
#define CVI_ERR_RGN_NOBUF CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
/* no data in buffer */
#define CVI_ERR_RGN_BUF_EMPTY CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
/* no buffer for new data */
#define CVI_ERR_RGN_BUF_FULL CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
/* bad address, eg. used for copy_from_user & copy_to_user */
#define CVI_ERR_RGN_BADADDR CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_BADADDR)
/* resource is busy, eg. destroy a venc chn without unregistering it */
#define CVI_ERR_RGN_BUSY CVI_DEF_ERR(CVI_ID_RGN, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)


/* invalid device ID */
#define CVI_ERR_ADEC_INVALID_DEVID     0xA3000001
/* invalid channel ID */
#define CVI_ERR_ADEC_INVALID_CHNID     0xA3000002
/* at least one parameter is illegal ,eg, an illegal enumeration value  */
#define CVI_ERR_ADEC_ILLEGAL_PARAM     0xA3000003
/* channel exists */
#define CVI_ERR_ADEC_EXIST            0xA3000004
/* channel unexists */
#define CVI_ERR_ADEC_UNEXIST           0xA3000005
/* using a NULL point */
#define CVI_ERR_ADEC_NULL_PTR          0xA3000006
/* try to enable or initialize system,device or channel, before configing attribute */
#define CVI_ERR_ADEC_NOT_CONFIG        0xA3000007
/* operation is not supported by NOW */
#define CVI_ERR_ADEC_NOT_SUPPORT       0xA3000008
/* operation is not permitted ,eg, try to change stati attribute */
#define CVI_ERR_ADEC_NOT_PERM          0xA3000009
/* failure caused by malloc memory */
#define CVI_ERR_ADEC_NOMEM             0xA300000A
/* failure caused by malloc buffer */
#define CVI_ERR_ADEC_NOBUF             0xA300000B
/* no data in buffer */
#define CVI_ERR_ADEC_BUF_EMPTY         0xA300000C
/* no buffer for new data */
#define CVI_ERR_ADEC_BUF_FULL          0xA300000D
/* system is not ready,had not initialized or loaded*/
#define CVI_ERR_ADEC_SYS_NOTREADY      0xA300000E
/* decoder internal err */
#define CVI_ERR_ADEC_DECODER_ERR       0xA300000F
/* input buffer not enough to decode one frame */
#define CVI_ERR_ADEC_BUF_LACK          0xA3000010


/* invalid device ID */
#define CVI_ERR_AENC_INVALID_DEVID     0xA2000001
/* invalid channel ID */
#define CVI_ERR_AENC_INVALID_CHNID     0xA2000002
/* at least one parameter is illegal ,eg, an illegal enumeration value  */
#define CVI_ERR_AENC_ILLEGAL_PARAM     0xA2000003
/* channel exists */
#define CVI_ERR_AENC_EXIST             0xA2000004
/* channel unexists */
#define CVI_ERR_AENC_UNEXIST           0xA2000005
/* using a NULL point */
#define CVI_ERR_AENC_NULL_PTR         0xA2000006
/* try to enable or initialize system,device or channel, before configing attribute */
#define CVI_ERR_AENC_NOT_CONFIG        0xA2000007
/* operation is not supported by NOW */
#define CVI_ERR_AENC_NOT_SUPPORT       0xA2000008
/* operation is not permitted ,eg, try to change static attribute */
#define CVI_ERR_AENC_NOT_PERM          0xA2000009
/* failure caused by malloc memory */
#define CVI_ERR_AENC_NOMEM            0xA200000A
/* failure caused by malloc buffer */
#define CVI_ERR_AENC_NOBUF             0xA200000B
/* no data in buffer */
#define CVI_ERR_AENC_BUF_EMPTY         0xA200000C
/* no buffer for new data */
#define CVI_ERR_AENC_BUF_FULL          0xA200000D
/* system is not ready,had not initialized or loaded*/
#define CVI_ERR_AENC_SYS_NOTREADY      0xA200000E
/* encoder internal err */
#define CVI_ERR_AENC_ENCODER_ERR       0xA200000F
/* vqe internal err */
#define CVI_ERR_AENC_VQE_ERR       0xA2000010


/* at least one parameter is illegal ,eg, an illegal enumeration value  */
#define CVI_ERR_AIO_ILLEGAL_PARAM     0xAA000001
/* using a NULL point */
#define CVI_ERR_AIO_NULL_PTR          0xAA000002
/* operation is not supported by NOW */
#define CVI_ERR_AIO_NOT_PERM          0xAA000003
/* vqe  err */
#define CVI_ERR_AIO_REGISTER_ERR      0xAA000004

/* invalid device ID */
#define CVI_ERR_AI_INVALID_DEVID     0xA0000005
/* invalid channel ID */
#define CVI_ERR_AI_INVALID_CHNID     0xA0000006
/* at least one parameter is illegal ,eg, an illegal enumeration value  */
#define CVI_ERR_AI_ILLEGAL_PARAM     0xA0000001
/* using a NULL point */
#define CVI_ERR_AI_NULL_PTR          0xA0000002
/* try to enable or initialize system,device or channel, before configing attribute */
#define CVI_ERR_AI_NOT_CONFIG        0xA0000007
/* operation is not supported by NOW */
#define CVI_ERR_AI_NOT_SUPPORT       0xA0000008
/* operation is not permitted ,eg, try to change stati attribute */
#define CVI_ERR_AI_NOT_PERM         0xA0000003
/* the devide is not enabled  */
#define CVI_ERR_AI_NOT_ENABLED       0xA0000009
/* failure caused by malloc memory */
#define CVI_ERR_AI_NOMEM             0xA000000A
/* failure caused by malloc buffer */
#define CVI_ERR_AI_NOBUF             0xA000000B
/* no data in buffer */
#define CVI_ERR_AI_BUF_EMPTY         0xA000000C
/* no buffer for new data */
#define CVI_ERR_AI_BUF_FULL          0xA000000D
/* system is not ready,had not initialized or loaded*/
#define CVI_ERR_AI_SYS_NOTREADY      0xA000000E

/* AI dev busy */
#define CVI_ERR_AI_BUSY              0xA000000F
/* vqe  err */
#define CVI_ERR_AI_VQE_ERR       0xA0000010
/* vqe buffer full */
#define CVI_ERR_AI_VQE_BUF_FULL       0xA0000011
/* vqe file unexist */
#define CVI_ERR_AI_VQE_FILE_UNEXIST       0xA0000012
/*invalid card ID*/
#define CVI_ERR_AI_INVALID_CARDID    0xA100013

/* invalid device ID */
#define CVI_ERR_AO_INVALID_DEVID     0xA1000001
/* invalid channel ID */
#define CVI_ERR_AO_INVALID_CHNID     0xA1000002
/* at least one parameter is illegal ,eg, an illegal enumeration value  */
#define CVI_ERR_AO_ILLEGAL_PARAM     0xA1000003
/* using a NULL point */
#define CVI_ERR_AO_NULL_PTR          0xA1000004
/* try to enable or initialize system,device or channel, before configing attribute */
#define CVI_ERR_AO_NOT_CONFIG        0xA1000005
/* operation is not supported by NOW */
#define CVI_ERR_AO_NOT_SUPPORT       0xA1000006
/* operation is not permitted ,eg, try to change stati attribute */
#define CVI_ERR_AO_NOT_PERM          0xA1000007
/* the devide is not enabled  */
#define CVI_ERR_AO_NOT_ENABLED       0xA1000008
/* failure caused by malloc memory */
#define CVI_ERR_AO_NOMEM             0xA1000009
/* failure caused by malloc buffer */
#define CVI_ERR_AO_NOBUF             0xA100000A
/* no data in buffer */
#define CVI_ERR_AO_BUF_EMPTY         0xA100000B
/* no buffer for new data */
#define CVI_ERR_AO_BUF_FULL          0xA100000C
/* system is not ready,had not initialized or loaded*/
#define CVI_ERR_AO_SYS_NOTREADY      0xA100000D

/* AO dev busy */
#define CVI_ERR_AO_BUSY              0xA100000E
/* vqe  err */
#define CVI_ERR_AO_VQE_ERR       0xA100000F
/*invalid card ID*/
#define CVI_ERR_AO_INVALID_CARDID    0xA100010


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __CVI_ERRNO_H__ */

