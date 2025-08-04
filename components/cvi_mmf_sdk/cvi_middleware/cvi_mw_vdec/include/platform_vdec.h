/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: vdec_ioctl.h
 * Description: Header file for video decoder ioctl functions
 */

#ifndef __VDEC_IOCTL_H__
#define __VDEC_IOCTL_H__

#include "cvi_comm_vdec.h"
#include "vc_uapi.h"

CVI_S32 vdec_create_chn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr);
CVI_S32 vdec_destroy_chn(VDEC_CHN VdChn);
CVI_S32 vdec_get_chn_attr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr);
CVI_S32 vdec_set_chn_attr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr);
CVI_S32 vdec_start_recv_stream(VDEC_CHN VdChn);
CVI_S32 vdec_stop_recv_stream(VDEC_CHN VdChn);
CVI_S32 vdec_query_status(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus);
CVI_S32 vdec_get_fd(VDEC_CHN VdChn);
CVI_S32 vdec_close_fd(VDEC_CHN VdChn);
CVI_S32 vdec_reset_chn(VDEC_CHN VdChn);
CVI_S32 vdec_set_chn_param(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S *pstParam);
CVI_S32 vdec_get_chn_param(VDEC_CHN VdChn, VDEC_CHN_PARAM_S *pstParam);
CVI_S32 vdec_send_stream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream, CVI_S32 s32MilliSec);
CVI_S32 vdec_get_frame(VDEC_CHN VdChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
CVI_S32 vdec_release_frame(VDEC_CHN VdChn, const VIDEO_FRAME_INFO_S *pstFrameInfo);
CVI_S32 vdec_get_user_data(VDEC_CHN VdChn, VDEC_USERDATA_S *pstUserData, CVI_S32 s32MilliSec);
CVI_S32 vdec_release_user_data(VDEC_CHN VdChn, const VDEC_USERDATA_S *pstUserData);
CVI_S32 vdec_set_rotation(VDEC_CHN VdChn, ROTATION_E enRotation);
CVI_S32 vdec_get_rotation(VDEC_CHN VdChn, ROTATION_E *penRotation);
CVI_S32 vdec_attach_vb_pool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool);
CVI_S32 vdec_detach_vb_pool(VDEC_CHN VdChn);
CVI_S32 vdec_set_user_data_attr(VDEC_CHN VdChn, const VDEC_USER_DATA_ATTR_S *pstUserDataAttr);
CVI_S32 vdec_get_user_data_attr(VDEC_CHN VdChn, VDEC_USER_DATA_ATTR_S *pstUserDataAttr);
CVI_S32 vdec_set_mod_param(const VDEC_MOD_PARAM_S *pstModParam);
CVI_S32 vdec_get_mod_param(VDEC_MOD_PARAM_S *pstModParam);
CVI_S32 vdec_get_data_fifo_addr(VDEC_CHN VdChn, CVI_U64 *pu64PhyAddr);
// CVI_S32 vdec_set_data_fifo_len(VDEC_CHN VdChn, CVI_U32 u32Len);

#endif /* __VDEC_IOCTL_H__ */