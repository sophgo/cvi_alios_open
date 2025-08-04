#include "cvi_debug.h"
#include "cvi_errno.h"
#include "cvi_vdec.h"

#include "platform_vdec.h"

//static VENC_DATA_FIFO_CTX_S *pstVencDataFifoCtx[VENC_MAX_CHN_NUM] = {NULL};
CVI_S32 CVI_VDEC_CreateChn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
    return vdec_create_chn(VdChn, pstAttr);
}

CVI_S32 CVI_VDEC_DestroyChn(VDEC_CHN VdChn)
{
    return vdec_destroy_chn(VdChn);
}

CVI_S32 CVI_VDEC_GetChnAttr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr)
{
    return vdec_get_chn_attr(VdChn, pstAttr);
}

CVI_S32 CVI_VDEC_SetChnAttr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
    return vdec_set_chn_attr(VdChn, pstAttr);
}

CVI_S32 CVI_VDEC_StartRecvStream(VDEC_CHN VdChn)
{
    return vdec_start_recv_stream(VdChn);
}

CVI_S32 CVI_VDEC_StopRecvStream(VDEC_CHN VdChn)
{
    return vdec_stop_recv_stream(VdChn);
}

CVI_S32 CVI_VDEC_QueryStatus(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus)
{
    return vdec_query_status(VdChn, pstStatus);
}

CVI_S32 CVI_VDEC_GetFd(VDEC_CHN VdChn)
{
    return vdec_get_fd(VdChn);
}

CVI_S32 CVI_VDEC_CloseFd(VDEC_CHN VdChn)
{
    return vdec_close_fd(VdChn);
}

CVI_S32 CVI_VDEC_ResetChn(VDEC_CHN VdChn)
{
    return vdec_reset_chn(VdChn);
}

CVI_S32 CVI_VDEC_SetChnParam(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S *pstParam)
{
    return vdec_set_chn_param(VdChn, pstParam);
}

CVI_S32 CVI_VDEC_GetChnParam(VDEC_CHN VdChn, VDEC_CHN_PARAM_S *pstParam)
{
    return vdec_get_chn_param(VdChn, pstParam);
}

// CVI_S32 CVI_VDEC_SetProtocolParam(VDEC_CHN VdChn, const VDEC_PRTCL_PARAM_S *pstParam)
// {
//     return vdec_set_protocol_param(VdChn, pstParam);
// }

// CVI_S32 CVI_VDEC_GetProtocolParam(VDEC_CHN VdChn, VDEC_PRTCL_PARAM_S *pstParam)
// {
//     return vdec_get_protocol_param(VdChn, pstParam);
// }

CVI_S32 CVI_VDEC_SendStream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream, CVI_S32 s32MilliSec)
{
    return vdec_send_stream(VdChn, pstStream, s32MilliSec);
}

CVI_S32 CVI_VDEC_GetFrame(VDEC_CHN VdChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
    return vdec_get_frame(VdChn, pstFrameInfo, s32MilliSec);
}

CVI_S32 CVI_VDEC_ReleaseFrame(VDEC_CHN VdChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    return vdec_release_frame(VdChn, pstFrameInfo);
}

CVI_S32 CVI_VDEC_GetUserData(VDEC_CHN VdChn, VDEC_USERDATA_S *pstUserData, CVI_S32 s32MilliSec)
{
    return vdec_get_user_data(VdChn, pstUserData, s32MilliSec);
}

CVI_S32 CVI_VDEC_ReleaseUserData(VDEC_CHN VdChn, const VDEC_USERDATA_S *pstUserData)
{
    return vdec_release_user_data(VdChn, pstUserData);
}

CVI_S32 CVI_VDEC_SetRotation(VDEC_CHN VdChn, ROTATION_E enRotation)
{
    return vdec_set_rotation(VdChn, enRotation);
}

CVI_S32 CVI_VDEC_GetRotation(VDEC_CHN VdChn, ROTATION_E *penRotation)
{
    return vdec_get_rotation(VdChn, penRotation);
}

CVI_S32 CVI_VDEC_AttachVbPool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool)
{
    return vdec_attach_vb_pool(VdChn, pstPool);
}

CVI_S32 CVI_VDEC_DetachVbPool(VDEC_CHN VdChn)
{
    return vdec_detach_vb_pool(VdChn);
}

CVI_S32 CVI_VDEC_SetUserDataAttr(VDEC_CHN VdChn, const VDEC_USER_DATA_ATTR_S *pstUserDataAttr)
{
    return vdec_set_user_data_attr(VdChn, pstUserDataAttr);
}

CVI_S32 CVI_VDEC_GetUserDataAttr(VDEC_CHN VdChn, VDEC_USER_DATA_ATTR_S *pstUserDataAttr)
{
    return vdec_get_user_data_attr(VdChn, pstUserDataAttr);
}

CVI_S32 CVI_VDEC_SetModParam(const VDEC_MOD_PARAM_S *pstModParam)
{
    return vdec_set_mod_param(pstModParam);
}

CVI_S32 CVI_VDEC_GetModParam(VDEC_MOD_PARAM_S *pstModParam)
{
    return vdec_get_mod_param(pstModParam);
}


CVI_S32 CVI_VDEC_GetDataFifoAddr(VDEC_CHN VdChn, CVI_U64 *pu64PhyAddr)
{
	return vdec_get_data_fifo_addr(VdChn, pu64PhyAddr);
}