#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "platform_venc.h"

extern int venc_ioctl(int ve_chn, unsigned int cmd,  void *arg);
extern int venc_clk_enable(void);
extern int venc_clk_disable(void);
extern int venc_ioctl_set_data_fifo_len(int ve_chn, unsigned int len);
extern int venc_ioctl_get_data_fifo_len(int ve_chn, unsigned int *p_len);
extern int venc_ioctl_get_data_fifo_addr(int ve_chn, unsigned long long *p_phy_addr);
extern uint32_t g_max_venc_chn_num;

#define UNUSED(x)	((void)(x))

typedef struct _VENC_STREAM_EX_S {
	VENC_STREAM_S *pstStream;
	CVI_S32 s32MilliSec;
} VENC_STREAM_EX_S;

typedef struct _VENC_USER_DATA_S {
	CVI_U8 *pu8Data;
	CVI_U32 u32Len;
} VENC_USER_DATA_S;

typedef struct _VIDEO_FRAME_INFO_EX_S {
	const VIDEO_FRAME_INFO_S *pstFrame;
	CVI_S32 s32MilliSec;
} VIDEO_FRAME_INFO_EX_S;

typedef struct _USER_FRAME_INFO_EX_S {
	const USER_FRAME_INFO_S *pstUserFrame;
	CVI_S32 s32MilliSec;
} USER_FRAME_INFO_EX_S;

#define CVI_VENC_NO_INPUT	-10
#define CVI_VENC_INPUT_ERR	-11

CVI_S32 venc_create_chn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_CREATE_CHN, (void *)pstAttr);
}

CVI_S32 venc_destroy_chn(VENC_CHN VeChn)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_DESTROY_CHN, NULL);
}

CVI_S32 venc_reset_chn(VENC_CHN VeChn)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_RESET_CHN, NULL);
}

CVI_S32 venc_start_recv_frame(VENC_CHN VeChn,
	const VENC_RECV_PIC_PARAM_S *pstRecvParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_START_RECV_FRAME, (void *)pstRecvParam);
}

CVI_S32 venc_stop_recv_frame(VENC_CHN VeChn)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_STOP_RECV_FRAME, NULL);
}

CVI_S32 venc_query_status(VENC_CHN VeChn, VENC_CHN_STATUS_S *pstStatus)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_QUERY_STATUS, pstStatus);
}

CVI_S32 venc_set_chn_attr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstChnAttr)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_CHN_ATTR,  (void *)pstChnAttr);
}

CVI_S32 venc_get_chn_attr(VENC_CHN VeChn, VENC_CHN_ATTR_S *pstChnAttr)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_CHN_ATTR, pstChnAttr);
}

CVI_S32 venc_get_stream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, CVI_S32 S32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VENC_STREAM_EX_S stStreamEx, *pstStreamEx = &stStreamEx;
	if (!pstStream) {
		return CVI_ERR_VENC_NULL_PTR;
	}

	pstStreamEx->pstStream = pstStream;
	pstStreamEx->s32MilliSec = S32MilliSec;
	s32Ret = venc_ioctl(VeChn, CVI_VC_VENC_GET_STREAM, pstStreamEx);

	/*if (s32Ret == CVI_SUCCESS) {
		CVI_U32 i = 0;
		VENC_PACK_S *ppack;
		for (i = 0; i < pstStreamEx->pstStream->u32PackCount; i++) {
			ppack = &pstStreamEx->pstStream->pstPack[i];
			if (ppack->u64PhyAddr && ppack->u32Len) {
				pStreamPackArray[VeChn][i] = ppack->pu8Addr;
			}
		}
	}*/
	return s32Ret;
}

CVI_S32 venc_release_stream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{

	/*CVI_U32 i = 0;
	VENC_PACK_S *ppack;

	for (i = 0; i < pstStream->u32PackCount; i++) {
		ppack = &pstStream->pstPack[i];
		ppack->pu8Addr = pStreamPackArray[VeChn][i];
	}*/
	return venc_ioctl(VeChn, CVI_VC_VENC_RELEASE_STREAM, pstStream);
}

CVI_S32 venc_insert_user_data(VENC_CHN VeChn, CVI_U8 *pu8Data, CVI_U32 u32Len)
{

	VENC_USER_DATA_S stUserData, *pstUserData = &stUserData;

	pstUserData->pu8Data = pu8Data;
	pstUserData->u32Len = u32Len;

	return venc_ioctl(VeChn, CVI_VC_VENC_INSERT_USERDATA, pstUserData);
}

CVI_S32 venc_send_frame(VENC_CHN VeChn, const VIDEO_FRAME_INFO_S *pstFrame, CVI_S32 s32MilliSec)
{

	VIDEO_FRAME_INFO_EX_S stFrameEx, *pstFrameEx = &stFrameEx;

	pstFrameEx->pstFrame = pstFrame;
	pstFrameEx->s32MilliSec = s32MilliSec;

	return venc_ioctl(VeChn, CVI_VC_VENC_SEND_FRAME, (void *)pstFrameEx);
}

CVI_S32 venc_send_frame_ex(VENC_CHN VeChn, const USER_FRAME_INFO_S *pstFrame,
	CVI_S32 s32MilliSec)
{
	USER_FRAME_INFO_EX_S stUserFrameEx, *pstUserFrameEx = &stUserFrameEx;

	pstUserFrameEx->pstUserFrame = pstFrame;
	pstUserFrameEx->s32MilliSec = s32MilliSec;

	return venc_ioctl(VeChn, CVI_VC_VENC_SEND_FRAMEEX, (void *)pstUserFrameEx);
}

CVI_S32 venc_request_idr(VENC_CHN VeChn, CVI_BOOL bInstant)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_REQUEST_IDR, &bInstant);
}

CVI_S32 venc_enable_idr(VENC_CHN VeChn, CVI_BOOL bInstant)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_ENABLE_IDR, &bInstant);
}

CVI_S32 venc_set_roi_attr(VENC_CHN VeChn, const VENC_ROI_ATTR_S *pstRoiAttr)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_ROI_ATTR, (void *)pstRoiAttr);
}

CVI_S32 venc_get_roi_attr(VENC_CHN VeChn, CVI_U32 u32Index, VENC_ROI_ATTR_S *pstRoiAttr)
{
	pstRoiAttr->u32Index = u32Index;
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_ROI_ATTR, pstRoiAttr);
}

CVI_S32 venc_set_h264_trans(VENC_CHN VeChn, const VENC_H264_TRANS_S *pstH264Trans)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H264_TRANS, (void *)pstH264Trans);
}

CVI_S32 venc_get_h264_trans(VENC_CHN VeChn, VENC_H264_TRANS_S *pstH264Trans)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H264_TRANS, pstH264Trans);
}

CVI_S32 venc_set_h264_entropy(VENC_CHN VeChn, const VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H264_ENTROPY, (void *)pstH264EntropyEnc);
}

CVI_S32 venc_get_h264_entropy(VENC_CHN VeChn, VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H264_ENTROPY, pstH264EntropyEnc);
}

CVI_S32 venc_set_h264_vui(VENC_CHN VeChn, const VENC_H264_VUI_S *pstH264Vui)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H264_VUI, (void *)pstH264Vui);
}

CVI_S32 venc_get_h264_vui(VENC_CHN VeChn, VENC_H264_VUI_S *pstH264Vui)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H264_VUI, pstH264Vui);
}

CVI_S32 venc_set_h265_vui(VENC_CHN VeChn, const VENC_H265_VUI_S *pstH265Vui)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H265_VUI, (void *)pstH265Vui);
}

CVI_S32 venc_get_h265_vui(VENC_CHN VeChn, VENC_H265_VUI_S *pstH265Vui)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H265_VUI, pstH265Vui);
}

CVI_S32 venc_set_jpeg_param(VENC_CHN VeChn, const VENC_JPEG_PARAM_S *pstJpegParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_JPEG_PARAM, (void *)pstJpegParam);
}

CVI_S32 venc_get_jpeg_param(VENC_CHN VeChn, VENC_JPEG_PARAM_S *pstJpegParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_JPEG_PARAM, pstJpegParam);
}

CVI_S32 venc_get_rc_param(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_RC_PARAM, pstRcParam);
}

CVI_S32 venc_set_mjpeg_param(VENC_CHN VeChn, const VENC_MJPEG_PARAM_S *pstMJpegParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_MJPEG_PARAM, (void *)pstMJpegParam);
}

CVI_S32 venc_get_mjpeg_param(VENC_CHN VeChn, VENC_MJPEG_PARAM_S *pstMJpegParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_MJPEG_PARAM, pstMJpegParam);
}

CVI_S32 venc_set_rc_param(VENC_CHN VeChn, const VENC_RC_PARAM_S *pstRcParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_RC_PARAM, (void *)pstRcParam);
}

CVI_S32 venc_set_ref_param(VENC_CHN VeChn, const VENC_REF_PARAM_S *pstRefParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_REF_PARAM, (void *)pstRefParam);
}

CVI_S32 venc_get_ref_param(VENC_CHN VeChn, VENC_REF_PARAM_S *pstRefParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_REF_PARAM, pstRefParam);
}

CVI_S32 venc_set_h265_pred_unit(VENC_CHN VeChn, const VENC_H265_PU_S *pstPredUnit)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H265_PRED_UNIT, (void *)pstPredUnit);

}

CVI_S32 venc_get_h265_pred_unit(VENC_CHN VeChn, VENC_H265_PU_S *pstPredUnit)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H265_PRED_UNIT, pstPredUnit);
}


CVI_S32 venc_set_h265_trans(VENC_CHN VeChn, const VENC_H265_TRANS_S *pstH265Trans)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H265_TRANS, (void *)pstH265Trans);
}

CVI_S32 venc_get_h265_trans(VENC_CHN VeChn, VENC_H265_TRANS_S *pstH265Trans)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H265_TRANS, pstH265Trans);
}

CVI_S32 venc_set_frame_lost_strategy(VENC_CHN VeChn,
	const VENC_FRAMELOST_S *pstFrmLostParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_FRAMELOST_STRATEGY, (void *)pstFrmLostParam);
}

CVI_S32 venc_get_frame_lost_strategy(VENC_CHN VeChn,
	VENC_FRAMELOST_S *pstFrmLostParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_FRAMELOST_STRATEGY, pstFrmLostParam);
}

CVI_S32 venc_set_super_frame_strategy(VENC_CHN VeChn,
	const VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_SUPERFRAME_STRATEGY, (void *)pstSuperFrmParam);
}

CVI_S32 venc_get_super_frame_strategy(VENC_CHN VeChn,
	VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_SUPERFRAME_STRATEGY, pstSuperFrmParam);
}

CVI_S32 venc_set_chn_param(VENC_CHN VeChn, const VENC_CHN_PARAM_S *pstChnParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_CHN_PARAM,  (void *)pstChnParam);
}

CVI_S32 venc_get_chn_param(VENC_CHN VeChn, VENC_CHN_PARAM_S *pstChnParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_CHN_PARAM, pstChnParam);
}

CVI_S32 venc_set_mod_param(const VENC_PARAM_MOD_S *pstModParam)
{
	VENC_CHN VeChn = 0;	// default hard-code

	return venc_ioctl(VeChn, CVI_VC_VENC_SET_MOD_PARAM,  (void *)pstModParam);
}

CVI_S32 venc_get_mod_param(VENC_PARAM_MOD_S *pstModParam)
{
	VENC_CHN VeChn = 0;	// default hard-code

	return venc_ioctl(VeChn, CVI_VC_VENC_GET_MOD_PARAM, pstModParam);
}

CVI_S32 venc_attach_vb_pool(VENC_CHN VeChn, const VENC_CHN_POOL_S *pstPool)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_ATTACH_VBPOOL,  (void *)pstPool);
}

CVI_S32 venc_detach_vb_pool(VENC_CHN VeChn)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_DETACH_VBPOOL, NULL);
}

CVI_S32 venc_set_cu_prediction(VENC_CHN VeChn,
		const VENC_CU_PREDICTION_S *pstCuPrediction)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_CUPREDICTION,  (void *)pstCuPrediction);
}

CVI_S32 venc_get_cu_prediction(VENC_CHN VeChn,
	VENC_CU_PREDICTION_S *pstCuPrediction)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_CUPREDICTION, pstCuPrediction);
}

CVI_S32 venc_calc_frame_param(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_CALC_FRAME_PARAM, pstFrameParam);
}

CVI_S32 venc_set_frame_param(VENC_CHN VeChn, const VENC_FRAME_PARAM_S *pstFrameParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_FRAME_PARAM,  (void *)pstFrameParam);
}

CVI_S32 venc_get_frame_param(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_FRAME_PARAM, pstFrameParam);
}

CVI_S32 venc_set_h264_slice_split(VENC_CHN VeChn,
	const VENC_H264_SLICE_SPLIT_S *pstSliceSplit)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H264_SLICE_SPLIT,  (void *)pstSliceSplit);
}

CVI_S32 venc_get_h264_slice_split(VENC_CHN VeChn, VENC_H264_SLICE_SPLIT_S *pstSliceSplit)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H264_SLICE_SPLIT, pstSliceSplit);
}

CVI_S32 venc_set_h265_slice_split(VENC_CHN VeChn,
	const VENC_H265_SLICE_SPLIT_S *pstSliceSplit)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H265_SLICE_SPLIT,  (void *)pstSliceSplit);
}

CVI_S32 venc_get_h265_slice_split(VENC_CHN VeChn, VENC_H265_SLICE_SPLIT_S *pstSliceSplit)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H265_SLICE_SPLIT, pstSliceSplit);
}

CVI_S32 venc_set_h264_dblk(VENC_CHN VeChn, const VENC_H264_DBLK_S *pstH264Dblk)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H264_DBLK,  (void *)pstH264Dblk);
}

CVI_S32 venc_get_h264_dblk(VENC_CHN VeChn, VENC_H264_DBLK_S *pstH264Dblk)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H264_DBLK, pstH264Dblk);
}

CVI_S32 venc_set_h265_dblk(VENC_CHN VeChn, const VENC_H265_DBLK_S *pstH265Dblk)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H265_DBLK,  (void *)pstH265Dblk);
}

CVI_S32 venc_get_h265_dblk(VENC_CHN VeChn, VENC_H265_DBLK_S *pstH265Dblk)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H265_DBLK, pstH265Dblk);
}

CVI_S32 venc_set_h264_intra_pred(VENC_CHN VeChn,
	const VENC_H264_INTRA_PRED_S *pstH264IntraPred)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H264_INTRA_PRED,  (void *)pstH264IntraPred);
}

CVI_S32 venc_get_h264_intra_pred(VENC_CHN VeChn,
	VENC_H264_INTRA_PRED_S *pstH264IntraPred)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H264_INTRA_PRED, pstH264IntraPred);
}

CVI_S32 venc_set_h265_sao(VENC_CHN VeChn, const VENC_H265_SAO_S *pstH265Sao)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_H265_SAO,  (void *)pstH265Sao);
}

CVI_S32 venc_get_h265_sao(VENC_CHN VeChn, VENC_H265_SAO_S *pstH265Sao)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_H265_SAO, pstH265Sao);
}

CVI_S32 venc_enable_svc(VENC_CHN VeChn, CVI_BOOL enable)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_ENABLE_SVC,  &enable);
}

CVI_S32 set_svc_param(VENC_CHN VeChn, const VENC_SVC_PARAM_S *pstSvcParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_SVC_PARAM,  (void *)pstSvcParam);
}

CVI_S32 venc_get_svc_param(VENC_CHN VeChn, VENC_SVC_PARAM_S *pstSvcParam)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_SVC_PARAM, pstSvcParam);
}

CVI_S32 venc_set_data_fifo_len(VENC_CHN VeChn, CVI_U32 u32Len)
{
	return venc_ioctl_set_data_fifo_len(VeChn, u32Len);
}

CVI_S32 venc_get_data_fifo_len(VENC_CHN VeChn, CVI_U32 *pu32Len)
{
	return venc_ioctl_get_data_fifo_len(VeChn, pu32Len);
}

CVI_S32 venc_get_data_fifo_addr(VENC_CHN VeChn, CVI_U64 *pu64PhyAddr)
{
	return venc_ioctl_get_data_fifo_addr(VeChn, (unsigned long long *)pu64PhyAddr);
}

CVI_S32 venc_suspend(void)
{
	return venc_clk_disable();
}

CVI_S32 venc_resmue(void)
{
	return venc_clk_enable();
}

CVI_S32 venc_set_debreath_effect(VENC_CHN VeChn, const VENC_DEBREATHEFFECT_S *pstDebreathEffect)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_SET_DEBREATH_EFFECT, (void *)pstDebreathEffect);
}

CVI_S32 venc_get_debreath_effect(VENC_CHN VeChn, VENC_DEBREATHEFFECT_S *pstDebreathEffect)
{
	return venc_ioctl(VeChn, CVI_VC_VENC_GET_DEBREATH_EFFECT, pstDebreathEffect);
}

