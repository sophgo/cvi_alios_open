#ifndef MODULES_VPU_INCLUDE_VI_IOCTL_H_
#define MODULES_VPU_INCLUDE_VI_IOCTL_H_

#include "cvi_comm_vi.h"
#include "vi_isp.h"
#include "vi_uapi.h"

int vi_set_hdr(CVI_BOOL is_hdr_on);
int vi_get_ip_dump_list(struct ip_info *ip_info_list);
int vi_get_online2sc(struct sop_isp_sc_online *online);

int vi_sdk_set_dev_num(CVI_U32 devNum);
int vi_sdk_get_dev_num(CVI_U32 *devNum);
int vi_sdk_enable_pagten(int dev);

int vi_sdk_get_dev_status(int dev, CVI_BOOL *pbStatus);
int vi_sdk_set_dev_attr(int dev, VI_DEV_ATTR_S *pstDevAttr);
int vi_sdk_get_dev_attr(int dev, VI_DEV_ATTR_S *pstDevAttr);
int vi_sdk_set_dev_attr_ex(int dev, VI_DEV_ATTR_EX_S *pstDevAttrEx);
int vi_sdk_get_dev_attr_ex(int dev, VI_DEV_ATTR_EX_S *pstDevAttrEx);
int vi_sdk_set_dev_bind_attr(int dev, VI_DEV_BIND_PIPE_S *pstDevBindAttr);
int vi_sdk_get_dev_bind_attr(int dev, VI_DEV_BIND_PIPE_S *pstDevBindAttr);
int vi_sdk_set_dev_unbind_attr(int dev);
int vi_sdk_get_pipe_status(int pipe, VI_PIPE_STATUS_S *pstStatus);
int vi_sdk_get_chn_status(int pipe, int chn, VI_CHN_STATUS_S *pstStatus);
int vi_sdk_enable_dev(int dev);
int vi_sdk_disable_dev(int dev);
int vi_sdk_create_pipe(int pipe, VI_PIPE_ATTR_S *pstPipeAttr);
int vi_sdk_start_pipe(int pipe);
int vi_sdk_destroy_pipe(int pipe);
int vi_sdk_set_chn_attr(int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr);
int vi_sdk_get_chn_attr(int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr);
int vi_sdk_set_pipe_attr(int pipe, VI_PIPE_ATTR_S *pstPipeAttr);
int vi_sdk_get_pipe_attr(int pipe, VI_PIPE_ATTR_S *pstPipeAttr);
int vi_sdk_get_pipe_dump_attr(int pipe, VI_DUMP_ATTR_S *pstDumpAttr);
int vi_sdk_set_pipe_dump_attr(int pipe, VI_DUMP_ATTR_S *pstDumpAttr);
int vi_sdk_enable_chn(int pipe, int chn);
int vi_sdk_disable_chn(int pipe, int chn);
int vi_sdk_set_motion_lv(struct mlv_info_s *mlv_i);
int vi_sdk_set_bypass_frm(CVI_U32 snr_num, CVI_U8 bypass_num);
int vi_sdk_set_pipe_frm_src(int pipe, VI_PIPE_FRAME_SOURCE_E *source);
int vi_sdk_get_pipe_frm_src(int pipe, VI_PIPE_FRAME_SOURCE_E *source);
int vi_sdk_send_pipe_raw(int pipe, VIDEO_FRAME_INFO_S *sVideoFrm);
int vi_sdk_set_dev_timing_attr(int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr);
int vi_sdk_get_chn_frame(int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
int vi_sdk_get_dev_timing_attr(int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr);
int vi_sdk_release_chn_frame(int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo);
int vi_sdk_set_chn_crop(int pipe, int chn, VI_CROP_INFO_S *pstCropInfo);
int vi_sdk_get_chn_crop(int pipe, int chn, VI_CROP_INFO_S *pstCropInfo);
int vi_sdk_set_pipe_crop(int pipe, CROP_INFO_S *pstCropInfo);
int vi_sdk_get_pipe_crop(int pipe, CROP_INFO_S *pstCropInfo);
int vi_sdk_get_pipe_frame(int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
int vi_sdk_release_pipe_frame(int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo);
int vi_sdk_start_smooth_rawdump(int pipe, struct sop_vip_isp_smooth_raw_param *smooth_raw_param);
int vi_sdk_stop_smooth_rawdump(int pipe, struct sop_vip_isp_smooth_raw_param *smooth_raw_param);
int vi_sdk_get_smooth_rawdump(int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
int vi_sdk_put_smooth_rawdump(int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo);
int vi_sdk_set_chn_rotation(struct vi_chn_rot_cfg *cfg);
int vi_sdk_get_chn_rotation(struct vi_chn_rot_cfg *cfg);
int vi_sdk_set_chn_ldc(struct vi_chn_ldc_cfg *cfg);
int vi_sdk_get_chn_ldc(int pipe, int chn, struct vi_chn_ldc_cfg *cfg);
int vi_sdk_set_chn_flip_mirror(struct vi_chn_flip_mirror_cfg *cfg);
int vi_sdk_get_chn_flip_mirror(struct vi_chn_flip_mirror_cfg *cfg);
int vi_sdk_attach_vbpool(struct vi_vb_pool_cfg *cfg);
int vi_sdk_detach_vbpool(struct vi_vb_pool_cfg *cfg);
int vi_sdk_dump_register(int pipe, struct ip_info *ip_info);
int vi_sdk_set_ai_isp_cfg(VI_AI_ISP_CFG_S *cfg);
int vi_sdk_get_ai_isp_raw(VI_AI_ISP_INFO_WRAP_S *infoWrap);
int vi_sdk_put_ai_isp_raw(VI_AI_ISP_INFO_WRAP_S *infoWrap);
#endif // MODULES_VPU_INCLUDE_VI_IOCTL_H_
