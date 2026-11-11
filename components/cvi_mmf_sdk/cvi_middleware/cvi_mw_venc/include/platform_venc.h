#ifndef __PLATFORM_VENC_H__
#define __PLATFORM_VENC_H__
#include "cvi_comm_venc.h"
#include "vc_uapi.h"

CVI_S32 venc_create_chn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr);
CVI_S32 venc_destroy_chn(VENC_CHN VeChn);
CVI_S32 venc_reset_chn(VENC_CHN VeChn);
CVI_S32 venc_start_recv_frame(VENC_CHN VeChn,
	const VENC_RECV_PIC_PARAM_S *pstRecvParam);
CVI_S32 venc_stop_recv_frame(VENC_CHN VeChn);
CVI_S32 venc_query_status(VENC_CHN VeChn, VENC_CHN_STATUS_S *pstStatus);
CVI_S32 venc_set_chn_attr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstChnAttr);
CVI_S32 venc_get_chn_attr(VENC_CHN VeChn, VENC_CHN_ATTR_S *pstChnAttr);
CVI_S32 venc_get_stream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, CVI_S32 S32MilliSec);
CVI_S32 venc_release_stream(VENC_CHN VeChn, VENC_STREAM_S *pstStream);
CVI_S32 venc_insert_user_data(VENC_CHN VeChn, CVI_U8 *pu8Data, CVI_U32 u32Len);
CVI_S32 venc_send_frame(VENC_CHN VeChn, const VIDEO_FRAME_INFO_S *pstFrame,
    CVI_S32 s32MilliSec);
CVI_S32 venc_send_frame_ex(VENC_CHN VeChn, const USER_FRAME_INFO_S *pstFrame,
	CVI_S32 s32MilliSec);
CVI_S32 venc_request_idr(VENC_CHN VeChn, CVI_BOOL bInstant);
CVI_S32 venc_enable_idr(VENC_CHN VeChn, CVI_BOOL bInstant);
CVI_S32 venc_set_roi_attr(VENC_CHN VeChn, const VENC_ROI_ATTR_S *pstRoiAttr);
CVI_S32 venc_get_roi_attr(VENC_CHN VeChn, CVI_U32 u32Index, VENC_ROI_ATTR_S *pstRoiAttr);
CVI_S32 venc_set_h264_trans(VENC_CHN VeChn, const VENC_H264_TRANS_S *pstH264Trans);
CVI_S32 venc_get_h264_trans(VENC_CHN VeChn, VENC_H264_TRANS_S *pstH264Trans);
CVI_S32 venc_set_h264_entropy(VENC_CHN VeChn, const VENC_H264_ENTROPY_S *pstH264EntropyEnc);
CVI_S32 venc_get_h264_entropy(VENC_CHN VeChn, VENC_H264_ENTROPY_S *pstH264EntropyEnc);
CVI_S32 venc_set_h264_vui(VENC_CHN VeChn, const VENC_H264_VUI_S *pstH264Vui);
CVI_S32 venc_get_h264_vui(VENC_CHN VeChn, VENC_H264_VUI_S *pstH264Vui);
CVI_S32 venc_set_h265_vui(VENC_CHN VeChn, const VENC_H265_VUI_S *pstH265Vui);
CVI_S32 venc_get_h265_vui(VENC_CHN VeChn, VENC_H265_VUI_S *pstH265Vui);
CVI_S32 venc_set_jpeg_param(VENC_CHN VeChn, const VENC_JPEG_PARAM_S *pstJpegParam);
CVI_S32 venc_get_jpeg_param(VENC_CHN VeChn, VENC_JPEG_PARAM_S *pstJpegParam);
CVI_S32 venc_get_rc_param(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcParam);
CVI_S32 venc_set_mjpeg_param(VENC_CHN VeChn, const VENC_MJPEG_PARAM_S *pstMJpegParam);
CVI_S32 venc_get_mjpeg_param(VENC_CHN VeChn, VENC_MJPEG_PARAM_S *pstMJpegParam);
CVI_S32 venc_set_rc_param(VENC_CHN VeChn, const VENC_RC_PARAM_S *pstRcParam);
CVI_S32 venc_set_ref_param(VENC_CHN VeChn, const VENC_REF_PARAM_S *pstRefParam);
CVI_S32 venc_get_ref_param(VENC_CHN VeChn, VENC_REF_PARAM_S *pstRefParam);
CVI_S32 venc_set_h265_pred_unit(VENC_CHN VeChn, const VENC_H265_PU_S *pstPredUnit);
CVI_S32 venc_get_h265_pred_unit(VENC_CHN VeChn, VENC_H265_PU_S *pstPredUnit);
CVI_S32 venc_set_h265_trans(VENC_CHN VeChn, const VENC_H265_TRANS_S *pstH265Trans);
CVI_S32 venc_get_h265_trans(VENC_CHN VeChn, VENC_H265_TRANS_S *pstH265Trans);
CVI_S32 venc_set_frame_lost_strategy(VENC_CHN VeChn,
	const VENC_FRAMELOST_S *pstFrmLostParam);
CVI_S32 venc_get_frame_lost_strategy(VENC_CHN VeChn,
	VENC_FRAMELOST_S *pstFrmLostParam);
CVI_S32 venc_set_super_frame_strategy(VENC_CHN VeChn,
	const VENC_SUPERFRAME_CFG_S *pstSuperFrmParam);
CVI_S32 venc_get_super_frame_strategy(VENC_CHN VeChn,
	VENC_SUPERFRAME_CFG_S *pstSuperFrmParam);
CVI_S32 venc_set_chn_param(VENC_CHN VeChn, const VENC_CHN_PARAM_S *pstChnParam);
CVI_S32 venc_get_chn_param(VENC_CHN VeChn, VENC_CHN_PARAM_S *pstChnParam);
CVI_S32 venc_set_mod_param(const VENC_PARAM_MOD_S *pstModParam);
CVI_S32 venc_get_mod_param(VENC_PARAM_MOD_S *pstModParam);
CVI_S32 venc_attach_vb_pool(VENC_CHN VeChn, const VENC_CHN_POOL_S *pstPool);
CVI_S32 venc_detach_vb_pool(VENC_CHN VeChn);
CVI_S32 venc_set_cu_prediction(VENC_CHN VeChn,
	const VENC_CU_PREDICTION_S *pstCuPrediction);
CVI_S32 venc_get_cu_prediction(VENC_CHN VeChn,
	VENC_CU_PREDICTION_S *pstCuPrediction);
CVI_S32 venc_calc_frame_param(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam);
CVI_S32 venc_set_frame_param(VENC_CHN VeChn, const VENC_FRAME_PARAM_S *pstFrameParam);
CVI_S32 venc_get_frame_param(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam);
CVI_S32 venc_set_h264_slice_split(VENC_CHN VeChn,
	const VENC_H264_SLICE_SPLIT_S *pstSliceSplit);
CVI_S32 venc_get_h264_slice_split(VENC_CHN VeChn, VENC_H264_SLICE_SPLIT_S *pstSliceSplit);
CVI_S32 venc_set_h265_slice_split(VENC_CHN VeChn,
	const VENC_H265_SLICE_SPLIT_S *pstSliceSplit);
CVI_S32 venc_get_h265_slice_split(VENC_CHN VeChn,
	VENC_H265_SLICE_SPLIT_S *pstSliceSplit);
CVI_S32 venc_set_h264_dblk(VENC_CHN VeChn, const VENC_H264_DBLK_S *pstH264Dblk);
CVI_S32 venc_get_h264_dblk(VENC_CHN VeChn, VENC_H264_DBLK_S *pstH264Dblk);
CVI_S32 venc_set_h265_dblk(VENC_CHN VeChn, const VENC_H265_DBLK_S *pstH265Dblk);
CVI_S32 venc_get_h265_dblk(VENC_CHN VeChn, VENC_H265_DBLK_S *pstH265Dblk);
CVI_S32 venc_set_h264_intra_pred(VENC_CHN VeChn,
	const VENC_H264_INTRA_PRED_S *pstH264IntraPred);
CVI_S32 venc_get_h264_intra_pred(VENC_CHN VeChn,
	VENC_H264_INTRA_PRED_S *pstH264IntraPred);
CVI_S32 venc_set_h265_sao(VENC_CHN VeChn, const VENC_H265_SAO_S *pstH265Sao);
CVI_S32 venc_get_h265_sao(VENC_CHN VeChn, VENC_H265_SAO_S *pstH265Sao);
CVI_S32 venc_enable_svc(VENC_CHN VeChn, CVI_BOOL enable);
CVI_S32 set_svc_param(VENC_CHN VeChn, const VENC_SVC_PARAM_S *pstSvcParam);
CVI_S32 venc_get_svc_param(VENC_CHN VeChn, VENC_SVC_PARAM_S *pstSvcParam);
CVI_S32 venc_set_data_fifo_len(VENC_CHN VeChn, CVI_U32 u32Len);
CVI_S32 venc_get_data_fifo_len(VENC_CHN VeChn, CVI_U32 *pu32Len);
CVI_S32 venc_get_data_fifo_addr(VENC_CHN VeChn, CVI_U64 *pu64PhyAddr);
CVI_S32 venc_suspend(void);
CVI_S32 venc_resmue(void);
CVI_S32 venc_set_debreath_effect(VENC_CHN VeChn, const VENC_DEBREATHEFFECT_S *pstDebreathEffect);
CVI_S32 venc_get_debreath_effect(VENC_CHN VeChn, VENC_DEBREATHEFFECT_S *pstDebreathEffect);
#endif
