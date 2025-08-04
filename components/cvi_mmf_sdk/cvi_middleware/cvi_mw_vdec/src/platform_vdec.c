#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "platform_vdec.h"

extern int vdec_ioctl(int vd_chn, unsigned int cmd,  void *arg);
extern int vdec_ioctl_get_data_fifo_addr(int vd_chn, unsigned long long *p_phy_addr);
#define UNUSED(x)	((void)(x))

typedef struct _VIDEO_FRAME_INFO_EX_S {
	const VIDEO_FRAME_INFO_S *pstFrame;
	CVI_S32 s32MilliSec;
} VIDEO_FRAME_INFO_EX_S;

typedef struct _VDEC_STREAM_EX_S {
	const VDEC_STREAM_S *pstStream;
	CVI_S32 s32MilliSec;
} VDEC_STREAM_EX_S;



CVI_S32 vdec_create_chn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_CREATE_CHN, (void *)pstAttr);
}

CVI_S32 vdec_destroy_chn(VDEC_CHN VdChn)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_DESTROY_CHN, NULL);
}

CVI_S32 vdec_get_chn_attr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_GET_CHN_ATTR, (void *)pstAttr);
}

CVI_S32 vdec_set_chn_attr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_SET_CHN_ATTR, (void *)pstAttr);
}

CVI_S32 vdec_start_recv_stream(VDEC_CHN VdChn)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_START_RECV_STREAM, NULL);
}

CVI_S32 vdec_stop_recv_stream(VDEC_CHN VdChn)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_STOP_RECV_STREAM, NULL);
}

CVI_S32 vdec_query_status(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_QUERY_STATUS, (void *)pstStatus);
}

CVI_S32 vdec_reset_chn(VDEC_CHN VdChn)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_RESET_CHN, NULL);
}

CVI_S32 vdec_set_chn_param(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S *pstParam)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_SET_CHN_PARAM, (void *)pstParam);
}

CVI_S32 vdec_get_chn_param(VDEC_CHN VdChn, VDEC_CHN_PARAM_S *pstParam)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_GET_CHN_PARAM, (void *)pstParam);
}

CVI_S32 vdec_send_stream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream, CVI_S32 s32MilliSec)
{
    VDEC_STREAM_EX_S stStreamEx;

    stStreamEx.pstStream = (VDEC_STREAM_S *)pstStream;
    stStreamEx.s32MilliSec = s32MilliSec;

    return vdec_ioctl(VdChn, CVI_VC_VDEC_SEND_STREAM, (void *)&stStreamEx);
}

CVI_S32 vdec_get_frame(VDEC_CHN VdChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
    VIDEO_FRAME_INFO_EX_S stFrameEx;

    stFrameEx.pstFrame = pstFrameInfo;
    stFrameEx.s32MilliSec = s32MilliSec;

    return vdec_ioctl(VdChn, CVI_VC_VDEC_GET_FRAME, &stFrameEx);
}

CVI_S32 vdec_release_frame(VDEC_CHN VdChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_RELEASE_FRAME, (void *)pstFrameInfo);
}

#if 0
CVI_S32 vdec_get_user_data(VDEC_CHN VdChn, VDEC_USERDATA_S *pstUserData, CVI_S32 s32MilliSec)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_GET_USERDATA, pstUserData);
}

CVI_S32 vdec_release_user_data(VDEC_CHN VdChn, const VDEC_USERDATA_S *pstUserData)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_RELEASE_USERDATA, (void *)pstUserData);
}

CVI_S32 vdec_set_rotation(VDEC_CHN VdChn, ROTATION_E enRotation)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_SET_ROTATION, &enRotation);
}

CVI_S32 vdec_get_rotation(VDEC_CHN VdChn, ROTATION_E *penRotation)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_GET_ROTATION, penRotation);
}

CVI_S32 vdec_set_user_data_attr(VDEC_CHN VdChn, const VDEC_USER_DATA_ATTR_S *pstUserDataAttr)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_SET_USERDATA_ATTR, (void *)pstUserDataAttr);
}

CVI_S32 vdec_get_user_data_attr(VDEC_CHN VdChn, VDEC_USER_DATA_ATTR_S *pstUserDataAttr)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_GET_USERDATA_ATTR, pstUserDataAttr);
}
#endif
CVI_S32 vdec_attach_vb_pool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_ATTACH_VBPOOL, (void *)pstPool);
}

CVI_S32 vdec_detach_vb_pool(VDEC_CHN VdChn)
{
    return vdec_ioctl(VdChn, CVI_VC_VDEC_DETACH_VBPOOL, NULL);
}

CVI_S32 vdec_set_mod_param(const VDEC_MOD_PARAM_S *pstModParam)
{
    VDEC_CHN VdChn = 0; // default channel
    return vdec_ioctl(VdChn, CVI_VC_VDEC_SET_MOD_PARAM, (void *)pstModParam);
}

CVI_S32 vdec_get_mod_param(VDEC_MOD_PARAM_S *pstModParam)
{
    VDEC_CHN VdChn = 0; // default channel
    return vdec_ioctl(VdChn, CVI_VC_VDEC_GET_MOD_PARAM, (void *)pstModParam);
}

CVI_S32 vdec_get_data_fifo_addr(VDEC_CHN VdChn, CVI_U64 *pu64PhyAddr)
{
	return vdec_ioctl_get_data_fifo_addr(VdChn, (unsigned long long *)pu64PhyAddr);
}
// CVI_S32 vdec_set_data_fifo_len(VDEC_CHN VdChn, CVI_U32 u32Len)
// {
// 	// return vdec_ioctl_set_data_fifo_len(VeChn, u32Len);
// }
