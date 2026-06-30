#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <cvi_comm_video.h>
#include "vi_ioctl.h"
#include "vi_uapi.h"
#include "driver_vi.h"

static inline int S_CTRL_VALUE(int cfg, enum VI_IOCTL _ioctl)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.value = cfg;

	rc = driver_vi_ioctl(VI_IOC_S_CTRL, (unsigned long)&ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

static inline int S_CTRL_PTR(void *cfg, int size, enum VI_IOCTL _ioctl)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.ptr = cfg;
	ec1.size = size;

	rc = driver_vi_ioctl(VI_IOC_S_CTRL, (unsigned long)&ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}


static inline int G_CTRL_PTR(void *cfg, int size, enum VI_IOCTL _ioctl)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.ptr = cfg;
	ec1.size = size;

	rc = driver_vi_ioctl(VI_IOC_G_CTRL, (unsigned long)&ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_IOC_G_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

static inline int SDK_CTRL_SET_VALUE(enum VI_IOCTL _ioctl, int dev, int pipe, int chn, int val)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.sdk_cfg.dev = dev;
	ec1.sdk_cfg.pipe = pipe;
	ec1.sdk_cfg.chn = chn;
	ec1.sdk_cfg.value = val;

	rc = driver_vi_ioctl(VI_IOC_SDK_CTRL, (unsigned long)&ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

static inline int SDK_CTRL_GET_VALUE(uint32_t *out, enum VI_IOCTL _ioctl)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.value = 0;

	rc = driver_vi_ioctl(VI_IOC_SDK_CTRL, (unsigned long)&ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_SDK_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	*out = ec1.value;
	return rc;
}

static inline int SDK_CTRL_SET_CFG(void *cfg, int size, enum VI_IOCTL _ioctl, int dev, int pipe, int chn, int val)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.sdk_cfg.dev = dev;
	ec1.sdk_cfg.pipe = pipe;
	ec1.sdk_cfg.chn = chn;
	ec1.sdk_cfg.ptr = cfg;
	ec1.sdk_cfg.size = size;
	ec1.sdk_cfg.reserved[0] = val;

	rc = driver_vi_ioctl(VI_IOC_SDK_CTRL, (unsigned long)&ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_SDK_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

int vi_set_hdr(CVI_BOOL is_hdr_on)
{
	return S_CTRL_VALUE(is_hdr_on, VI_IOCTL_HDR);
}

int vi_get_online2sc(struct sop_isp_sc_online *online)
{
	return G_CTRL_PTR(online, sizeof(struct sop_isp_sc_online), VI_IOCTL_GET_SC_ONLINE);
}

int vi_sdk_get_dev_num(CVI_U32 *devNum)
{
	return SDK_CTRL_GET_VALUE(devNum, VI_SDK_GET_DEV_NUM);
}

int vi_sdk_enable_pagten(int dev)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_ENABLE_PATGEN, dev, -1, -1, -1);
}

int vi_sdk_set_dev_attr(int dev, VI_DEV_ATTR_S *pstDevAttr)
{
	return SDK_CTRL_SET_CFG(pstDevAttr, sizeof(VI_DEV_ATTR_S), VI_SDK_SET_DEV_ATTR, dev, -1, -1, -1);
}

int vi_sdk_get_dev_attr(int dev, VI_DEV_ATTR_S *pstDevAttr)
{
	return SDK_CTRL_SET_CFG(pstDevAttr, sizeof(VI_DEV_ATTR_S), VI_SDK_GET_DEV_ATTR, dev, -1, -1, -1);
}

int vi_sdk_set_dev_attr_ex(int dev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	return SDK_CTRL_SET_CFG(pstDevAttrEx, sizeof(VI_DEV_ATTR_EX_S), VI_SDK_SET_DEV_ATTR_EX, dev, -1, -1, -1);
}

int vi_sdk_get_dev_attr_ex(int dev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	return SDK_CTRL_SET_CFG(pstDevAttrEx, sizeof(VI_DEV_ATTR_EX_S), VI_SDK_GET_DEV_ATTR_EX, dev, -1, -1, -1);
}

int vi_sdk_set_dev_bind_attr(int dev, VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	return SDK_CTRL_SET_CFG(pstDevBindAttr, sizeof(VI_DEV_BIND_PIPE_S), VI_SDK_SET_DEV_BIND_ATTR, dev, -1, -1, -1);
}

int vi_sdk_get_dev_bind_attr(int dev, VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	return SDK_CTRL_SET_CFG(pstDevBindAttr, sizeof(VI_DEV_BIND_PIPE_S), VI_SDK_GET_DEV_BIND_ATTR, dev, -1, -1, -1);
}

int vi_sdk_set_dev_unbind_attr(int dev)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_SET_DEV_UNBIND_ATTR, dev, -1, -1, -1);
}

int vi_sdk_enable_dev(int dev)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_ENABLE_DEV, dev, -1, -1, -1);
}

int vi_sdk_disable_dev(int dev)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_DISABLE_DEV, dev, -1, -1, -1);
}

int vi_sdk_create_pipe(int pipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	return SDK_CTRL_SET_CFG(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), VI_SDK_CREATE_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_destroy_pipe(int pipe)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_DESTROY_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_attr(int pipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	return SDK_CTRL_SET_CFG(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), VI_SDK_GET_PIPE_ATTR, -1, pipe, -1, -1);
}

int vi_sdk_set_pipe_attr(int pipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	return SDK_CTRL_SET_CFG(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), VI_SDK_SET_PIPE_ATTR, -1, pipe, -1, -1);
}

int vi_sdk_start_pipe(int pipe)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_START_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_stop_pipe(int pipe)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_STOP_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_set_chn_attr(int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr)
{
	return SDK_CTRL_SET_CFG(pstChnAttr, sizeof(VI_CHN_ATTR_S), VI_SDK_SET_CHN_ATTR, -1, pipe, chn, -1);
}

int vi_sdk_get_chn_attr(int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr)
{
	return SDK_CTRL_SET_CFG(pstChnAttr, sizeof(VI_CHN_ATTR_S), VI_SDK_GET_CHN_ATTR, -1, pipe, chn, -1);
}

int vi_sdk_enable_chn(int pipe, int chn)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_ENABLE_CHN, -1, pipe, chn, -1);
}

int vi_sdk_disable_chn(int pipe, int chn)
{
	return SDK_CTRL_SET_CFG(NULL, 0, VI_SDK_DISABLE_CHN, -1, pipe, chn, -1);
}

int vi_sdk_set_motion_lv(struct mlv_info_s *pmlv_i)
{
	return SDK_CTRL_SET_CFG(pmlv_i, sizeof(struct mlv_info_s), VI_SDK_SET_MOTION_LV, -1, -1, -1, -1);
}

int vi_sdk_set_bypass_frm(CVI_U32 snr_num, CVI_U8 bypass_num)
{
	return SDK_CTRL_SET_VALUE(VI_SDK_SET_BYPASS_FRM, -1, snr_num, -1, bypass_num);
}

int vi_sdk_set_pipe_frm_src(int pipe, VI_PIPE_FRAME_SOURCE_E *source)
{
	return SDK_CTRL_SET_CFG(source, sizeof(VI_PIPE_FRAME_SOURCE_E), VI_SDK_SET_PIPE_FRM_SRC, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_frm_src(int pipe, VI_PIPE_FRAME_SOURCE_E *source)
{
	return SDK_CTRL_SET_CFG(source, sizeof(VI_PIPE_FRAME_SOURCE_E), VI_SDK_GET_PIPE_FRM_SRC, -1, pipe, -1, -1);
}

int vi_sdk_set_dev_timing_attr(int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr)
{
	return SDK_CTRL_SET_CFG(pstDevTimingAttr, sizeof(VI_DEV_TIMING_ATTR_S),
				VI_SDK_SET_DEV_TIMING_ATTR, dev, -1, -1, -1);
}

int vi_sdk_get_dev_timing_attr(int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr)
{
	return SDK_CTRL_SET_CFG(pstDevTimingAttr, sizeof(VI_DEV_TIMING_ATTR_S),
				VI_SDK_GET_DEV_TIMING_ATTR, dev, -1, -1, -1);
}

int vi_sdk_send_pipe_raw(int pipe, VIDEO_FRAME_INFO_S *sVideoFrm)
{
	return SDK_CTRL_SET_CFG(sVideoFrm, sizeof(VIDEO_FRAME_INFO_S), VI_SDK_SEND_PIPE_RAW, -1, pipe, -1, -1);
}

int vi_sdk_get_dev_status(int dev, CVI_BOOL *pbStatus)
{
	return SDK_CTRL_SET_CFG(pbStatus, sizeof(CVI_BOOL), VI_SDK_GET_DEV_STATUS, dev, -1, -1, -1);
}

int vi_sdk_get_pipe_status(int pipe, VI_PIPE_STATUS_S *pstStatus)
{
	return SDK_CTRL_SET_CFG(pstStatus, sizeof(VI_PIPE_STATUS_S), VI_SDK_GET_PIPE_STATUS, -1, pipe, -1, -1);
}

int vi_sdk_get_chn_status(int pipe, int chn, VI_CHN_STATUS_S *pstStatus)
{
	return SDK_CTRL_SET_CFG(pstStatus, sizeof(VI_CHN_STATUS_S),
				VI_SDK_GET_CHN_STATUS, -1, pipe, chn, -1);
}

int vi_sdk_get_chn_frame(int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	return SDK_CTRL_SET_CFG(pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S),
				VI_SDK_GET_CHN_FRAME, -1, pipe, chn, s32MilliSec);
}

int vi_sdk_release_chn_frame(int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	return SDK_CTRL_SET_CFG(pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S), VI_SDK_RELEASE_CHN_FRAME, -1, pipe, chn, -1);
}

int vi_sdk_set_chn_crop(int pipe, int chn, VI_CROP_INFO_S *pstCropInfo)
{
	return SDK_CTRL_SET_CFG(pstCropInfo, sizeof(VI_CROP_INFO_S), VI_SDK_SET_CHN_CROP, -1, pipe, chn, -1);
}

int vi_sdk_get_chn_crop(int pipe, int chn, VI_CROP_INFO_S *pstCropInfo)
{
	return SDK_CTRL_SET_CFG(pstCropInfo, sizeof(VI_CROP_INFO_S), VI_SDK_GET_CHN_CROP, -1, pipe, chn, -1);
}

int vi_sdk_set_pipe_crop(int pipe, CROP_INFO_S *pstCropInfo)
{
	return SDK_CTRL_SET_CFG(pstCropInfo, sizeof(CROP_INFO_S), VI_SDK_SET_PIPE_CROP, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_crop(int pipe, CROP_INFO_S *pstCropInfo)
{
	return SDK_CTRL_SET_CFG(pstCropInfo, sizeof(CROP_INFO_S), VI_SDK_GET_PIPE_CROP, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_frame(int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	return SDK_CTRL_SET_CFG(pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S) * 2,
				VI_SDK_GET_PIPE_FRAME, -1, pipe, -1, s32MilliSec);
}

int vi_sdk_release_pipe_frame(int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	return SDK_CTRL_SET_CFG(pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S) * 2,
				VI_SDK_RELEASE_PIPE_FRAME, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_dump_attr(int pipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	return SDK_CTRL_SET_CFG(pstDumpAttr, sizeof(VI_DUMP_ATTR_S), VI_SDK_GET_PIPE_DUMP_ATTR, -1, pipe, -1, -1);
}

int vi_sdk_set_pipe_dump_attr(int pipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	return SDK_CTRL_SET_CFG(pstDumpAttr, sizeof(VI_DUMP_ATTR_S), VI_SDK_SET_PIPE_DUMP_ATTR, -1, pipe, -1, -1);
}

int vi_sdk_start_smooth_rawdump(int pipe, struct sop_vip_isp_smooth_raw_param *smooth_raw_param)
{
	return SDK_CTRL_SET_CFG(smooth_raw_param, sizeof(struct sop_vip_isp_smooth_raw_param),
				VI_SDK_START_SMOOTH_RAWDUMP, -1, pipe, -1, -1);
}

int vi_sdk_stop_smooth_rawdump(int pipe, struct sop_vip_isp_smooth_raw_param *smooth_raw_param)
{
	return SDK_CTRL_SET_CFG(smooth_raw_param, sizeof(struct sop_vip_isp_smooth_raw_param),
				VI_SDK_STOP_SMOOTH_RAWDUMP, -1, pipe, -1, -1);
}

int vi_sdk_get_smooth_rawdump(int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	return SDK_CTRL_SET_CFG(pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S) * 2,
				VI_SDK_GET_SMOOTH_RAWDUMP, -1, pipe, -1, s32MilliSec);
}

int vi_sdk_put_smooth_rawdump(int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	return SDK_CTRL_SET_CFG(pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S) * 2,
				VI_SDK_PUT_SMOOTH_RAWDUMP, -1, pipe, -1, -1);
}

int vi_sdk_set_chn_rotation(struct vi_chn_rot_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(cfg, sizeof(struct vi_chn_rot_cfg),
				VI_SDK_SET_CHN_ROTATION, -1, cfg->ViPipe, cfg->ViChn, -1);
}

int vi_sdk_get_chn_rotation(struct vi_chn_rot_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(cfg, sizeof(struct vi_chn_rot_cfg),
				VI_SDK_GET_CHN_ROTATION, -1, cfg->ViPipe, cfg->ViChn, -1);
}

int vi_sdk_set_chn_ldc(struct vi_chn_ldc_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(cfg, sizeof(struct vi_chn_ldc_cfg), VI_SDK_SET_CHN_LDC, -1, cfg->ViPipe, cfg->ViChn, -1);
}

int vi_sdk_get_chn_ldc(int pipe, int chn, struct vi_chn_ldc_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(cfg, sizeof(struct vi_chn_ldc_cfg), VI_SDK_GET_CHN_LDC, -1, pipe, chn, -1);
}

int vi_sdk_set_chn_flip_mirror(struct vi_chn_flip_mirror_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(cfg, sizeof(struct vi_chn_flip_mirror_cfg),
				VI_SDK_SET_CHN_FLIP_MIRROR, -1, cfg->ViPipe, cfg->ViChn, -1);
}

int vi_sdk_get_chn_flip_mirror(struct vi_chn_flip_mirror_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(cfg, sizeof(struct vi_chn_flip_mirror_cfg),
				VI_SDK_GET_CHN_FLIP_MIRROR, -1, cfg->ViPipe, cfg->ViChn, -1);
}

int vi_sdk_attach_vbpool(struct vi_vb_pool_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(cfg, sizeof(struct vi_vb_pool_cfg), VI_SDK_ATTACH_VB_POOL, -1, -1, cfg->ViChn, -1);
}

int vi_sdk_detach_vbpool(struct vi_vb_pool_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(cfg, sizeof(struct vi_vb_pool_cfg), VI_SDK_DETACH_VB_POOL, -1, -1, cfg->ViChn, -1);
}

int vi_sdk_dump_register(int pipe, struct ip_info *ip_info)
{
	return SDK_CTRL_SET_CFG(ip_info, sizeof(struct ip_info), VI_SDK_DUMP_REGISTER, -1, pipe, -1, -1);
}

int vi_sdk_set_ai_isp_cfg(VI_AI_ISP_CFG_S *pstAiIspCfg)
{
	struct vi_ai_isp_cfg cfg;

	cfg.ViPipe = pstAiIspCfg->viPipe;
	cfg.ViAiISPType = (CVI_U8)pstAiIspCfg->enAiIspType;
	cfg.Reserved[0] = pstAiIspCfg->reserved[0];
	cfg.Reserved[1] = pstAiIspCfg->reserved[1];

	return S_CTRL_PTR(&cfg, sizeof(struct vi_ai_isp_cfg), VI_IOCTL_AI_ISP_CFG);
}

int vi_sdk_get_ai_isp_raw(VI_AI_ISP_INFO_WRAP_S *infoWrap)
{
	int ret = 0;
	struct vi_ai_isp_info info;

	info.ViPipe = infoWrap->stIspInfo.viPipe;
	info.InputAddr[0] = infoWrap->stIspInfo.inputAddr[0];
	info.InputAddr[1] = infoWrap->stIspInfo.inputAddr[1];
	info.OutputAddr[0] = infoWrap->stIspInfo.outputAddr[0];
	info.OutputAddr[1] = infoWrap->stIspInfo.outputAddr[1];
	info.Size = infoWrap->stIspInfo.size;
	info.Reserved[0] = infoWrap->stIspInfo.reserved[0];

	ret = G_CTRL_PTR(&info, sizeof(struct vi_ai_isp_info), VI_IOCTL_GET_AI_ISP_RAW);
	if (ret != CVI_SUCCESS) {
		fprintf(stderr, "VI_IOCTL_GET_AI_ISP_RAW - %s NG, %s\n", __func__, strerror(errno));
		return ret;
	}

	infoWrap->stIspInfo.inputAddr[0] = info.InputAddr[0];
	infoWrap->stIspInfo.inputAddr[1] = info.InputAddr[1];
	infoWrap->stIspInfo.outputAddr[0] = info.OutputAddr[0];
	infoWrap->stIspInfo.outputAddr[1] = info.OutputAddr[1];
	infoWrap->stIspInfo.size = info.Size;
	infoWrap->stIspInfo.reserved[0] = info.Reserved[0];

	return ret;
}

int vi_sdk_put_ai_isp_raw(VI_AI_ISP_INFO_WRAP_S *infoWrap)
{
	struct vi_ai_isp_info info;

	info.ViPipe = infoWrap->stIspInfo.viPipe;
	info.InputAddr[0] = infoWrap->stIspInfo.inputAddr[0];
	info.InputAddr[1] = infoWrap->stIspInfo.inputAddr[1];
	info.OutputAddr[0] = infoWrap->stIspInfo.outputAddr[0];
	info.OutputAddr[1] = infoWrap->stIspInfo.outputAddr[1];
	info.Size = infoWrap->stIspInfo.size;
	info.Reserved[0] = infoWrap->stIspInfo.reserved[0];

	return S_CTRL_PTR(&info, sizeof(struct vi_ai_isp_info), VI_IOCTL_PUT_AI_ISP_RAW);
}