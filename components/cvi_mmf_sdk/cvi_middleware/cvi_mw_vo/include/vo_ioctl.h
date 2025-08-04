#ifndef __VO_IOCTL_H__
#define __VO_IOCTL_H__

#include "vo_platform.h"
#include "vo_uapi.h"
#include "driver_vo.h"

//self driver_vo_ioctl cmd test
int vo_set_pattern(VO_PATTERN_MODE pattern, unsigned int vodev);
int vo_set_gamma_ctrl(VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev);
int vo_get_gamma_ctrl(VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev);
int vo_set_csc(struct disp_csc_matrix *cfg, unsigned int volayer);

//vo sdk layer apis
int vo_sdk_enable(struct vo_dev_cfg *cfg);
int vo_sdk_disable(struct vo_dev_cfg *cfg);
int vo_sdk_isenable(struct vo_dev_cfg *cfg);
int vo_sdk_get_panelstatue(struct vo_panel_status_cfg *cfg);
int vo_sdk_get_pubattr(struct vo_pub_attr_cfg *cfg);
int vo_sdk_set_pubattr(struct vo_pub_attr_cfg *cfg);
int vo_sdk_set_lvdsparam(struct vo_lvds_param_cfg *cfg);
int vo_sdk_get_lvdsparam(struct vo_lvds_param_cfg *cfg);
int vo_sdk_set_btparam(struct vo_bt_param_cfg *cfg);
int vo_sdk_get_btparam(struct vo_bt_param_cfg *cfg);
int vo_sdk_get_displaybuflen(struct vo_display_buflen_cfg *cfg);
int vo_sdk_set_displaybuflen(struct vo_display_buflen_cfg *cfg);
int vo_sdk_set_videolayerattr(struct vo_video_layer_attr_cfg *cfg);
int vo_sdk_get_videolayerattr(struct vo_video_layer_attr_cfg *cfg);
int vo_sdk_set_layer_proc_amp(struct vo_layer_proc_amp_cfg *cfg);
int vo_sdk_get_layer_proc_amp(struct vo_layer_proc_amp_cfg *cfg);
int vo_sdk_set_layer_csc(struct vo_layer_csc_cfg *cfg);
int vo_sdk_get_layer_csc(struct vo_layer_csc_cfg *cfg);
int vo_sdk_enable_videolayer(struct vo_video_layer_cfg *cfg);
int vo_sdk_disable_videolayer(struct vo_video_layer_cfg *cfg);
int vo_sdk_enable_chn(struct vo_chn_cfg *cfg);
int vo_sdk_disable_chn(struct vo_chn_cfg *cfg);
int vo_sdk_send_frame(struct vo_snd_frm_cfg *cfg);
int vo_sdk_send_logo_fromIon(struct vo_snd_frm_cfg *cfg);
int vo_sdk_clearchnbuf(struct vo_clear_chn_buf_cfg *cfg);
int vo_sdk_set_chnattr(struct vo_chn_attr_cfg *cfg);
int vo_sdk_get_chnattr(struct vo_chn_attr_cfg *cfg);
int vo_sdk_set_chn_frmrate(struct vo_chn_frmrate_cfg *cfg);
int vo_sdk_get_chn_frmrate(struct vo_chn_frmrate_cfg *cfg);
int vo_sdk_get_chn_pts(struct vo_chn_pts_cfg *cfg);
int vo_sdk_get_chn_status(struct vo_chn_status_cfg *cfg);
int vo_sdk_set_chnrotation(struct vo_chn_rotation_cfg *cfg);
int vo_sdk_get_chnrotation(struct vo_chn_rotation_cfg *cfg);
int vo_sdk_showchn(struct vo_chn_cfg *cfg);
int vo_sdk_hidechn(struct vo_chn_cfg *cfg);
int vo_sdk_pausechn(struct vo_chn_cfg *cfg);
int vo_sdk_resumechn(struct vo_chn_cfg *cfg);
int vo_sdk_suspend();
int vo_sdk_resume();

#endif
