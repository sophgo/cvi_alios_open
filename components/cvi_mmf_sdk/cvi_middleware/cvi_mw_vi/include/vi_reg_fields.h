/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name:vi_reg_fields.h
 * Description:HW register description
 */

#ifndef _VI_REG_FIELDS_H_
#define _VI_REG_FIELDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "osal_types.h"

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_ae_hist_ae_hist_status {
	uint32_t raw;
	struct {
		uint32_t ae_hist_status                  : 32;
	} bits;
};

union reg_isp_ae_hist_ae_hist_grace_reset {
	uint32_t raw;
	struct {
		uint32_t ae_hist_grace_reset             : 1;
	} bits;
};

union reg_isp_ae_hist_ae_hist_monitor {
	uint32_t raw;
	struct {
		uint32_t ae_hist_monitor                 : 32;
	} bits;
};

union reg_isp_ae_hist_ae_hist_bypass {
	uint32_t raw;
	struct {
		uint32_t ae_hist_bypass                  : 1;
		uint32_t _rsv_1                          : 19;
		uint32_t hist_zeroing_enable             : 1;
		uint32_t _rsv_21                         : 7;
		uint32_t force_clk_enable                : 1;
	} bits;
};

union reg_isp_ae_hist_ae_kickoff {
	uint32_t raw;
	struct {
		uint32_t ae_zero_ae_sum                  : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t ae_wbgain_apply                 : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t loadshadowreg                   : 1;
		uint32_t _rsv_5                          : 1;
		uint32_t hist_zerohistogram              : 1;
		uint32_t _rsv_7                          : 1;
		uint32_t hist_wbgain_apply               : 1;
		uint32_t _rsv_9                          : 1;
		uint32_t ae_hist_shadow_select           : 1;
		uint32_t _rsv_11                         : 5;
		uint32_t ae_face_enable                  : 4;
	} bits;
};

union reg_isp_ae_hist_sts_ae0_hist_enable {
	uint32_t raw;
	struct {
		uint32_t sts_ae0_hist_enable             : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t ae0_gain_enable                 : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t hist0_enable                    : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t hist0_gain_enable               : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t ir_ae_enable                    : 1;
		uint32_t _rsv_17                         : 3;
		uint32_t ir_ae_gain_enable               : 1;
		uint32_t _rsv_21                         : 3;
		uint32_t ir_hist_enable                  : 1;
		uint32_t _rsv_25                         : 3;
		uint32_t ir_hist_gain_enable             : 1;
	} bits;
};

union reg_isp_ae_hist_sts_ae_offsetx {
	uint32_t raw;
	struct {
		uint32_t sts_ae0_offsetx                 : 13;
	} bits;
};

union reg_isp_ae_hist_sts_ae_offsety {
	uint32_t raw;
	struct {
		uint32_t sts_ae0_offsety                 : 13;
	} bits;
};

union reg_isp_ae_hist_sts_ae_numxm1 {
	uint32_t raw;
	struct {
		uint32_t sts_ae0_numxm1                  : 6;
	} bits;
};

union reg_isp_ae_hist_sts_ae_numym1 {
	uint32_t raw;
	struct {
		uint32_t sts_ae0_numym1                  : 5;
	} bits;
};

union reg_isp_ae_hist_sts_ae_width {
	uint32_t raw;
	struct {
		uint32_t sts_ae0_width                   : 8;
	} bits;
};

union reg_isp_ae_hist_sts_ae_height {
	uint32_t raw;
	struct {
		uint32_t sts_ae0_height                  : 8;
	} bits;
};

union reg_isp_ae_hist_sts_ae_sts_div {
	uint32_t raw;
	struct {
		uint32_t sts_ae0_sts_div                 : 13;
	} bits;
};

union reg_isp_ae_hist_sts_hist_mode {
	uint32_t raw;
	struct {
		uint32_t sts_hist0_mode                  : 2;
	} bits;
};

union reg_isp_ae_hist_shdw_read_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

union reg_isp_ae_hist_ae_hist_monitor_select {
	uint32_t raw;
	struct {
		uint32_t ae_hist_monitor_select          : 32;
	} bits;
};

union reg_isp_ae_hist_ae_hist_location {
	uint32_t raw;
	struct {
		uint32_t ae_hist_location                : 32;
	} bits;
};

union reg_isp_ae_hist_hw_auto_cg_en {
	uint32_t raw;
	struct {
		uint32_t hw_auto_cg_en                   : 1;
	} bits;
};

union reg_isp_ae_hist_sts_ir_ae_offsetx {
	uint32_t raw;
	struct {
		uint32_t sts_ir_ae_offsetx               : 13;
	} bits;
};

union reg_isp_ae_hist_sts_ir_ae_offsety {
	uint32_t raw;
	struct {
		uint32_t sts_ir_ae_offsety               : 13;
	} bits;
};

union reg_isp_ae_hist_sts_ir_ae_numxm1 {
	uint32_t raw;
	struct {
		uint32_t sts_ir_ae_numxm1                : 5;
	} bits;
};

union reg_isp_ae_hist_sts_ir_ae_numym1 {
	uint32_t raw;
	struct {
		uint32_t sts_ir_ae_numym1                : 5;
	} bits;
};

union reg_isp_ae_hist_sts_ir_ae_width {
	uint32_t raw;
	struct {
		uint32_t sts_ir_ae_width                 : 10;
	} bits;
};

union reg_isp_ae_hist_sts_ir_ae_height {
	uint32_t raw;
	struct {
		uint32_t sts_ir_ae_height                : 10;
	} bits;
};

union reg_isp_ae_hist_sts_ir_ae_sts_div {
	uint32_t raw;
	struct {
		uint32_t sts_ir_ae_sts_div               : 3;
	} bits;
};

union reg_isp_ae_hist_ae_hist_bayer_starting {
	uint32_t raw;
	struct {
		uint32_t ae_hist_bayer_starting          : 4;
		uint32_t _rsv_4                          : 12;
		uint32_t force_bayer_enable              : 1;
	} bits;
};

union reg_isp_ae_hist_ae_hist_dummy {
	uint32_t raw;
	struct {
		uint32_t ae_hist_dummy                   : 16;
	} bits;
};

union reg_isp_ae_hist_ae_hist_checksum {
	uint32_t raw;
	struct {
		uint32_t ae_hist_checksum                : 32;
	} bits;
};

union reg_isp_ae_hist_wbg_4 {
	uint32_t raw;
	struct {
		uint32_t ae0_wbg_rgain                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ae0_wbg_ggain                   : 14;
	} bits;
};

union reg_isp_ae_hist_wbg_5 {
	uint32_t raw;
	struct {
		uint32_t ae0_wbg_bgain                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ae1_wbg_bgain                   : 14;
	} bits;
};

union reg_isp_ae_hist_wbg_6 {
	uint32_t raw;
	struct {
		uint32_t ae1_wbg_rgain                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ae1_wbg_ggain                   : 14;
	} bits;
};

union reg_isp_ae_hist_wbg_7 {
	uint32_t raw;
	struct {
		uint32_t ae0_wbg_vgain                   : 14;
	} bits;
};

union reg_isp_ae_hist_dmi_enable {
	uint32_t raw;
	struct {
		uint32_t dmi_enable                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t dmi_qos                         : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t force_dma_disable               : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t ir_dmi_enable                   : 1;
		uint32_t _rsv_17                         : 3;
		uint32_t ir_dmi_qos                      : 1;
		uint32_t _rsv_21                         : 3;
		uint32_t ir_force_dma_disable            : 1;
	} bits;
};

union reg_isp_ae_hist_ae_face0_location {
	uint32_t raw;
	struct {
		uint32_t ae_face0_offset_x               : 13;
		uint32_t ae_face0_offset_y               : 13;
	} bits;
};

union reg_isp_ae_hist_ae_face1_location {
	uint32_t raw;
	struct {
		uint32_t ae_face1_offset_x               : 13;
		uint32_t ae_face1_offset_y               : 13;
	} bits;
};

union reg_isp_ae_hist_ae_face2_location {
	uint32_t raw;
	struct {
		uint32_t ae_face2_offset_x               : 13;
		uint32_t ae_face2_offset_y               : 13;
	} bits;
};

union reg_isp_ae_hist_ae_face3_location {
	uint32_t raw;
	struct {
		uint32_t ae_face3_offset_x               : 13;
		uint32_t ae_face3_offset_y               : 13;
	} bits;
};

union reg_isp_ae_hist_ae_face0_size {
	uint32_t raw;
	struct {
		uint32_t ae_face0_size_minus1_x          : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t ae_face0_size_minus1_y          : 8;
	} bits;
};

union reg_isp_ae_hist_ae_face1_size {
	uint32_t raw;
	struct {
		uint32_t ae_face1_size_minus1_x          : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t ae_face1_size_minus1_y          : 8;
	} bits;
};

union reg_isp_ae_hist_ae_face2_size {
	uint32_t raw;
	struct {
		uint32_t ae_face2_size_minus1_x          : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t ae_face2_size_minus1_y          : 8;
	} bits;
};

union reg_isp_ae_hist_ae_face3_size {
	uint32_t raw;
	struct {
		uint32_t ae_face3_size_minus1_x          : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t ae_face3_size_minus1_y          : 8;
	} bits;
};

union reg_isp_ae_hist_ir_ae_face0_location {
	uint32_t raw;
	struct {
		uint32_t ir_ae_face0_offset_x            : 16;
		uint32_t ir_ae_face0_offset_y            : 16;
	} bits;
};

union reg_isp_ae_hist_ir_ae_face1_location {
	uint32_t raw;
	struct {
		uint32_t ir_ae_face1_offset_x            : 16;
		uint32_t ir_ae_face1_offset_y            : 16;
	} bits;
};

union reg_isp_ae_hist_ir_ae_face2_location {
	uint32_t raw;
	struct {
		uint32_t ir_ae_face2_offset_x            : 16;
		uint32_t ir_ae_face2_offset_y            : 16;
	} bits;
};

union reg_isp_ae_hist_ir_ae_face3_location {
	uint32_t raw;
	struct {
		uint32_t ir_ae_face3_offset_x            : 16;
		uint32_t ir_ae_face3_offset_y            : 16;
	} bits;
};

union reg_isp_ae_hist_ir_ae_face0_size {
	uint32_t raw;
	struct {
		uint32_t ir_ae_face0_size_minus1_x       : 7;
		uint32_t _rsv_7                          : 9;
		uint32_t ir_ae_face0_size_minus1_y       : 7;
	} bits;
};

union reg_isp_ae_hist_ir_ae_face1_size {
	uint32_t raw;
	struct {
		uint32_t ir_ae_face1_size_minus1_x       : 7;
		uint32_t _rsv_7                          : 9;
		uint32_t ir_ae_face1_size_minus1_y       : 7;
	} bits;
};

union reg_isp_ae_hist_ir_ae_face2_size {
	uint32_t raw;
	struct {
		uint32_t ir_ae_face2_size_minus1_x       : 7;
		uint32_t _rsv_7                          : 9;
		uint32_t ir_ae_face2_size_minus1_y       : 7;
	} bits;
};

union reg_isp_ae_hist_ir_ae_face3_size {
	uint32_t raw;
	struct {
		uint32_t ir_ae_face3_size_minus1_x       : 7;
		uint32_t _rsv_7                          : 9;
		uint32_t ir_ae_face3_size_minus1_y       : 7;
	} bits;
};

union reg_isp_ae_hist_ae_face_enable_ctrl {
	uint32_t raw;
	struct {
		uint32_t ae_face0_enable                 : 1;
		uint32_t ae_face1_enable                 : 1;
		uint32_t ae_face2_enable                 : 1;
		uint32_t ae_face3_enable                 : 1;
	} bits;
};

union reg_isp_ae_hist_ae_face0_sts_div {
	uint32_t raw;
	struct {
		uint32_t ae_face0_sts_div                : 13;
	} bits;
};

union reg_isp_ae_hist_ae_face1_sts_div {
	uint32_t raw;
	struct {
		uint32_t ae_face1_sts_div                : 13;
	} bits;
};

union reg_isp_ae_hist_ae_face2_sts_div {
	uint32_t raw;
	struct {
		uint32_t ae_face2_sts_div                : 13;
	} bits;
};

union reg_isp_ae_hist_ae_face3_sts_div {
	uint32_t raw;
	struct {
		uint32_t ae_face3_sts_div                : 13;
	} bits;
};

union reg_isp_ae_hist_sts_enable {
	uint32_t raw;
	struct {
		uint32_t sts_awb_enable                  : 1;
	} bits;
};

union reg_isp_ae_hist_ae_algo_enable {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 1;
		uint32_t ae_algo_enable                  : 1;
	} bits;
};

union reg_isp_ae_hist_ae_hist_low {
	uint32_t raw;
	struct {
		uint32_t ae_hist_low                     : 8;
	} bits;
};

union reg_isp_ae_hist_ae_hist_high {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 8;
		uint32_t ae_hist_high                    : 8;
	} bits;
};

union reg_isp_ae_hist_ae_top {
	uint32_t raw;
	struct {
		uint32_t ae_awb_top                      : 12;
	} bits;
};

union reg_isp_ae_hist_ae_bot {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 12;
		uint32_t ae_awb_bot                      : 12;
	} bits;
};

union reg_isp_ae_hist_ae_overexp_thr {
	uint32_t raw;
	struct {
		uint32_t ae_overexp_thr                  : 10;
	} bits;
};

union reg_isp_ae_hist_ae_num_gapline {
	uint32_t raw;
	struct {
		uint32_t ae_num_gapline                  : 1;
	} bits;
};

union reg_isp_ae_hist_ae_choose_gr_en {
	uint32_t raw;
	struct {
		uint32_t ae_choose_gr_en                 : 1;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_offset_r {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_offset_r              : 12;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_offset_gr {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_offset_gr             : 12;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_offset_gb {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_offset_gb             : 12;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_offset_b {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_offset_b              : 12;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_offset_ir {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_offset_ir             : 12;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_gain_r {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_gain_r                : 16;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_gain_gr {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_gain_gr               : 16;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_gain_gb {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_gain_gb               : 16;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_gain_b {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_gain_b                : 16;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_gain_ir {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_gain_ir               : 16;
	} bits;
};

union reg_isp_ae_hist_se_ae_blc_enable {
	uint32_t raw;
	struct {
		uint32_t se_ae_blc_enable                : 1;
	} bits;
};

union reg_isp_ae_hist_frame_counter_awb {
	uint32_t raw;
	struct {
		uint32_t e_counter_awb_le                : 16;
		uint32_t e_counter_awb_se                : 16;
	} bits;
};

union reg_isp_ae_hist_ae_simple2a_result_luma_se {
	uint32_t raw;
	struct {
		uint32_t simple2a_result_luma_se         : 10;
	} bits;
};

union reg_isp_ae_hist_ae_simple2a_result_rgain_se {
	uint32_t raw;
	struct {
		uint32_t simple2a_result_rgain_se        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_simple2a_result_bgain_se {
	uint32_t raw;
	struct {
		uint32_t simple2a_result_bgain_se        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_simple2a_result_luma_le {
	uint32_t raw;
	struct {
		uint32_t simple2a_result_luma_le         : 10;
	} bits;
};

union reg_isp_ae_hist_ae_simple2a_result_rgain_le {
	uint32_t raw;
	struct {
		uint32_t simple2a_result_rgain_le        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_simple2a_result_bgain_le {
	uint32_t raw;
	struct {
		uint32_t simple2a_result_bgain_le        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_face0_result_se {
	uint32_t raw;
	struct {
		uint32_t ae_face0_result_r_sum_se        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t ae_face0_result_g_sum_se        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t ae_face0_result_b_sum_se        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_face1_result_se {
	uint32_t raw;
	struct {
		uint32_t ae_face1_result_r_sum_se        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t ae_face1_result_g_sum_se        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t ae_face1_result_b_sum_se        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_face2_result_se {
	uint32_t raw;
	struct {
		uint32_t ae_face2_result_r_sum_se        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t ae_face2_result_g_sum_se        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t ae_face2_result_b_sum_se        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_face3_result_se {
	uint32_t raw;
	struct {
		uint32_t ae_face3_result_r_sum_se        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t ae_face3_result_g_sum_se        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t ae_face3_result_b_sum_se        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_face0_result_le {
	uint32_t raw;
	struct {
		uint32_t ae_face0_result_r_sum_le        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t ae_face0_result_g_sum_le        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t ae_face0_result_b_sum_le        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_face1_result_le {
	uint32_t raw;
	struct {
		uint32_t ae_face1_result_r_sum_le        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t ae_face1_result_g_sum_le        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t ae_face1_result_b_sum_le        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_face2_result_le {
	uint32_t raw;
	struct {
		uint32_t ae_face2_result_r_sum_le        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t ae_face2_result_g_sum_le        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t ae_face2_result_b_sum_le        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_face3_result_le {
	uint32_t raw;
	struct {
		uint32_t ae_face3_result_r_sum_le        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t ae_face3_result_g_sum_le        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t ae_face3_result_b_sum_le        : 10;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_00 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_00                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_01 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_01                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_02 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_02                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_03 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_03                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_04 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_04                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_05 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_05                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_06 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_06                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_07 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_07                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_08 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_08                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_09 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_09                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_10 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_10                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_11 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_11                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_12 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_12                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_13 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_13                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_14 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_14                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_15 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_15                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_16 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_16                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_17 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_17                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_18 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_18                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_19 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_19                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_20 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_20                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_21 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_21                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_22 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_22                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_23 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_23                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_24 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_24                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_25 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_25                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_26 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_26                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_27 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_27                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_28 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_28                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_29 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_29                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_30 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_30                       : 32;
	} bits;
};

union reg_isp_ae_hist_ae_wgt_31 {
	uint32_t raw;
	struct {
		uint32_t ae_wgt_31                       : 32;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_offset_r {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_offset_r              : 12;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_offset_gr {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_offset_gr             : 12;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_offset_gb {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_offset_gb             : 12;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_offset_b {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_offset_b              : 12;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_offset_ir {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_offset_ir             : 12;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_gain_r {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_gain_r                : 16;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_gain_gr {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_gain_gr               : 16;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_gain_gb {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_gain_gb               : 16;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_gain_b {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_gain_b                : 16;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_gain_ir {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_gain_ir               : 16;
	} bits;
};

union reg_isp_ae_hist_le_ae_blc_enable {
	uint32_t raw;
	struct {
		uint32_t le_ae_blc_enable                : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_af_status {
	uint32_t raw;
	struct {
		uint32_t af_status                       : 32;
	} bits;
};

union reg_isp_af_grace_reset {
	uint32_t raw;
	struct {
		uint32_t af_grace_reset                  : 1;
	} bits;
};

union reg_isp_af_monitor {
	uint32_t raw;
	struct {
		uint32_t af_monitor                      : 32;
	} bits;
};

union reg_isp_af_bypass {
	uint32_t raw;
	struct {
		uint32_t af_bypass                       : 1;
	} bits;
};

union reg_isp_af_kickoff {
	uint32_t raw;
	struct {
		uint32_t af_enable                       : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t af_wbgain_apply                 : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t af_revert_exposure              : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t af_gain_enable                  : 1;
	} bits;
};

union reg_isp_af_enables {
	uint32_t raw;
	struct {
		uint32_t af_horizon_0_enable             : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t af_horizon_1_enable             : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t af_vertical_0_enable            : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t af_gamma_enable                 : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t af_dpc_enable                   : 1;
		uint32_t _rsv_17                         : 3;
		uint32_t af_hlc_enable                   : 1;
		uint32_t _rsv_21                         : 3;
		uint32_t af_shadow_select                : 1;
		uint32_t _rsv_25                         : 3;
		uint32_t force_clk_enable                : 1;
	} bits;
};

union reg_isp_af_offset_x {
	uint32_t raw;
	struct {
		uint32_t af_offset_x                     : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t af_offset_y                     : 13;
	} bits;
};

union reg_isp_af_mxn_image_width_m1 {
	uint32_t raw;
	struct {
		uint32_t af_mxn_image_width              : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t af_mxn_image_height             : 13;
	} bits;
};

union reg_isp_af_block_width {
	uint32_t raw;
	struct {
		uint32_t af_block_width                  : 8;
	} bits;
};

union reg_isp_af_block_height {
	uint32_t raw;
	struct {
		uint32_t af_block_height                 : 8;
	} bits;
};

union reg_isp_af_block_num_x {
	uint32_t raw;
	struct {
		uint32_t af_block_num_x                  : 5;
	} bits;
};

union reg_isp_af_block_num_y {
	uint32_t raw;
	struct {
		uint32_t af_block_num_y                  : 4;
		uint32_t _rsv_4                          : 12;
		uint32_t force_bayer_enable              : 1;
	} bits;
};

union reg_isp_af_hor_low_pass_value_shift {
	uint32_t raw;
	struct {
		uint32_t af_hor_low_pass_value_shift     : 4;
	} bits;
};

union reg_isp_af_corning_offset_horizontal_0 {
	uint32_t raw;
	struct {
		uint32_t af_corning_offset_horizontal_0  : 8;
	} bits;
};

union reg_isp_af_corning_offset_horizontal_1 {
	uint32_t raw;
	struct {
		uint32_t af_corning_offset_horizontal_1  : 8;
	} bits;
};

union reg_isp_af_corning_offset_vertical {
	uint32_t raw;
	struct {
		uint32_t af_corning_offset_vertical      : 8;
	} bits;
};

union reg_isp_af_high_y_thre {
	uint32_t raw;
	struct {
		uint32_t af_high_y_thre                  : 8;
	} bits;
};

union reg_isp_af_low_pass_horizon {
	uint32_t raw;
	struct {
		uint32_t af_low_pass_horizon_0           : 6;
		uint32_t af_low_pass_horizon_1           : 6;
		uint32_t af_low_pass_horizon_2           : 6;
		uint32_t af_low_pass_horizon_3           : 6;
		uint32_t af_low_pass_horizon_4           : 6;
	} bits;
};

union reg_isp_af_location {
	uint32_t raw;
	struct {
		uint32_t af_location                     : 32;
	} bits;
};

union reg_isp_af_high_pass_horizon_0 {
	uint32_t raw;
	struct {
		uint32_t af_high_pass_horizon_0_0        : 6;
		uint32_t af_high_pass_horizon_0_1        : 6;
		uint32_t af_high_pass_horizon_0_2        : 6;
		uint32_t af_high_pass_horizon_0_3        : 6;
		uint32_t af_high_pass_horizon_0_4        : 6;
	} bits;
};

union reg_isp_af_high_pass_horizon_1 {
	uint32_t raw;
	struct {
		uint32_t af_high_pass_horizon_1_0        : 6;
		uint32_t af_high_pass_horizon_1_1        : 6;
		uint32_t af_high_pass_horizon_1_2        : 6;
		uint32_t af_high_pass_horizon_1_3        : 6;
		uint32_t af_high_pass_horizon_1_4        : 6;
	} bits;
};

union reg_isp_af_high_pass_vertical_0 {
	uint32_t raw;
	struct {
		uint32_t af_high_pass_vertical_0_0       : 6;
		uint32_t af_high_pass_vertical_0_1       : 6;
		uint32_t af_high_pass_vertical_0_2       : 6;
	} bits;
};

union reg_isp_af_mem_sw_mode {
	uint32_t raw;
	struct {
		uint32_t af_mem_sw_mode                  : 1;
		uint32_t af_r_mem_sel                    : 1;
		uint32_t af_g_mem_sel                    : 1;
		uint32_t af_b_mem_sel                    : 1;
		uint32_t af_blk_div_mem_sel              : 1;
		uint32_t af_gamma_g_mem_sel              : 1;
		uint32_t af_magfactor_mem_sel            : 1;
		uint32_t af_blk_div_dff_sel              : 1;
		uint32_t af_gamma_g_dff_sel              : 1;
		uint32_t af_magfactor_dff_sel            : 1;
	} bits;
};

union reg_isp_af_monitor_select {
	uint32_t raw;
	struct {
		uint32_t af_monitor_select               : 32;
	} bits;
};

union reg_isp_af_image_width {
	uint32_t raw;
	struct {
		uint32_t af_image_width                  : 16;
	} bits;
};

union reg_isp_af_dummy {
	uint32_t raw;
	struct {
		uint32_t af_dummy                        : 16;
	} bits;
};

union reg_isp_af_mem_sw_raddr {
	uint32_t raw;
	struct {
		uint32_t af_sw_raddr                     : 7;
	} bits;
};

union reg_isp_af_mem_sw_rdata {
	uint32_t raw;
	struct {
		uint32_t af_rdata                        : 31;
		uint32_t af_sw_read                      : 1;
	} bits;
};

union reg_isp_af_mxn_border {
	uint32_t raw;
	struct {
		uint32_t af_mxn_border                   : 2;
	} bits;
};

union reg_isp_af_th_low    {
	uint32_t raw;
	struct {
		uint32_t af_th_low                       : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t af_th_high                      : 8;
	} bits;
};

union reg_isp_af_gain_low  {
	uint32_t raw;
	struct {
		uint32_t af_gain_low                     : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t af_gain_high                    : 8;
	} bits;
};

union reg_isp_af_slop_low {
	uint32_t raw;
	struct {
		uint32_t af_slop_low                     : 4;
		uint32_t af_slop_high                    : 4;
	} bits;
};

union reg_isp_af_dmi_enable {
	uint32_t raw;
	struct {
		uint32_t dmi_enable                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t dmi_qos                         : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t force_dma_disable               : 1;
	} bits;
};

union reg_isp_af_square_enable {
	uint32_t raw;
	struct {
		uint32_t af_square_enable                : 1;
	} bits;
};

union reg_isp_af_outshift {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 4;
		uint32_t af_outshift                     : 3;
	} bits;
};

union reg_isp_af_num_gapline {
	uint32_t raw;
	struct {
		uint32_t af_num_gapline                  : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_blc_dg_wb_base_config {
	uint32_t raw;
	struct {
		uint32_t shift_mode                      : 1;
		uint32_t blc_le_enable                   : 1;
		uint32_t blc_se_enable                   : 1;
		uint32_t wbg_le_enable                   : 1;
		uint32_t wbg_se_enable                   : 1;
		uint32_t cg_enable                       : 1;
	} bits;
};

union reg_blc_dg_wb_wbg_le_gain0 {
	uint32_t raw;
	struct {
		uint32_t wbg_le_rgain                    : 14;
		uint32_t wbg_le_ggain                    : 14;
	} bits;
};

union reg_blc_dg_wb_wbg_le_gain1 {
	uint32_t raw;
	struct {
		uint32_t wbg_le_bgain                    : 14;
	} bits;
};

union reg_blc_dg_wb_blc_le_offset0 {
	uint32_t raw;
	struct {
		uint32_t blc_le_offset_r                 : 12;
		uint32_t blc_le_offset_gr                : 12;
	} bits;
};

union reg_blc_dg_wb_blc_le_offset1 {
	uint32_t raw;
	struct {
		uint32_t blc_le_offset_gb                : 12;
		uint32_t blc_le_offset_b                 : 12;
	} bits;
};

union reg_blc_dg_wb_blc_le_offset_2nd0 {
	uint32_t raw;
	struct {
		uint32_t blc_le_2ndoffset_r              : 12;
		uint32_t blc_le_2ndoffset_gr             : 12;
	} bits;
};

union reg_blc_dg_wb_blc_le_offset_2nd1 {
	uint32_t raw;
	struct {
		uint32_t blc_le_2ndoffset_gb             : 12;
		uint32_t blc_le_2ndoffset_b              : 12;
	} bits;
};

union reg_blc_dg_wb_blc_le_gain0 {
	uint32_t raw;
	struct {
		uint32_t blc_le_gain_r                   : 16;
		uint32_t blc_le_gain_gr                  : 16;
	} bits;
};

union reg_blc_dg_wb_blc_le_gain1 {
	uint32_t raw;
	struct {
		uint32_t blc_le_gain_gb                  : 16;
		uint32_t blc_le_gain_b                   : 16;
	} bits;
};

union reg_blc_dg_wb_blc_le_normgain0 {
	uint32_t raw;
	struct {
		uint32_t blc_le_normgain_r               : 16;
		uint32_t blc_le_normgain_gr              : 16;
	} bits;
};

union reg_blc_dg_wb_blc_le_normgain1 {
	uint32_t raw;
	struct {
		uint32_t blc_le_normgain_gb              : 16;
		uint32_t blc_le_normgain_b               : 16;
	} bits;
};

union reg_blc_dg_wb_wbg_se_gain0 {
	uint32_t raw;
	struct {
		uint32_t wbg_se_rgain                    : 14;
		uint32_t wbg_se_ggain                    : 14;
	} bits;
};

union reg_blc_dg_wb_wbg_se_gain1 {
	uint32_t raw;
	struct {
		uint32_t wbg_se_bgain                    : 14;
	} bits;
};

union reg_blc_dg_wb_blc_se_offset0 {
	uint32_t raw;
	struct {
		uint32_t blc_se_offset_r                 : 12;
		uint32_t blc_se_offset_gr                : 12;
	} bits;
};

union reg_blc_dg_wb_blc_se_offset1 {
	uint32_t raw;
	struct {
		uint32_t blc_se_offset_gb                : 12;
		uint32_t blc_se_offset_b                 : 12;
	} bits;
};

union reg_blc_dg_wb_blc_se_offset_2nd0 {
	uint32_t raw;
	struct {
		uint32_t blc_se_2ndoffset_r              : 12;
		uint32_t blc_se_2ndoffset_gr             : 12;
	} bits;
};

union reg_blc_dg_wb_blc_se_offset_2nd1 {
	uint32_t raw;
	struct {
		uint32_t blc_se_2ndoffset_gb             : 12;
		uint32_t blc_se_2ndoffset_b              : 12;
	} bits;
};

union reg_blc_dg_wb_blc_se_gain0 {
	uint32_t raw;
	struct {
		uint32_t blc_se_gain_r                   : 16;
		uint32_t blc_se_gain_gr                  : 16;
	} bits;
};

union reg_blc_dg_wb_blc_se_gain1 {
	uint32_t raw;
	struct {
		uint32_t blc_se_gain_gb                  : 16;
		uint32_t blc_se_gain_b                   : 16;
	} bits;
};

union reg_blc_dg_wb_blc_se_normgain0 {
	uint32_t raw;
	struct {
		uint32_t blc_se_normgain_r               : 16;
		uint32_t blc_se_normgain_gr              : 16;
	} bits;
};

union reg_blc_dg_wb_blc_se_normgain1 {
	uint32_t raw;
	struct {
		uint32_t blc_se_normgain_gb              : 16;
		uint32_t blc_se_normgain_b               : 16;
	} bits;
};

union reg_blc_dg_wb_shdw_read_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

union reg_blc_dg_wb_auto_cg_en {
	uint32_t raw;
	struct {
		uint32_t hw_auto_cg_en                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_bnr_00 {
	uint32_t raw;
	struct {
		uint32_t u1_bnr_enable                   : 1;
		uint32_t shd_rd                          : 1;
		uint32_t bypass                          : 1;
		uint32_t soft_clr                        : 1;
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t u2_bnr_debugmode                : 2;
		uint32_t _rsv_10                         : 14;
		uint32_t dummy                           : 8;
	} bits;
};

union reg_isp_bnr_04 {
	uint32_t raw;
	struct {
		uint32_t out_enable                      : 2;
	} bits;
};

union reg_isp_bnr_10 {
	uint32_t raw;
	struct {
		uint32_t u1_bnr_filtmode                 : 1;
		uint32_t u1_bnr_bilat_th_en              : 1;
		uint32_t u1_bnr_bilat_center_sel         : 1;
		uint32_t _rsv_3                          : 5;
		uint32_t u5_bnr_bilat_blend_w            : 5;
	} bits;
};

union reg_isp_bnr_14 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_bilat_th1               : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_bilat_th2               : 10;
	} bits;
};

union reg_isp_bnr_18 {
	uint32_t raw;
	struct {
		uint32_t u5_bnr_sweight_0                : 5;
		uint32_t _rsv_5                          : 1;
		uint32_t u5_bnr_sweight_1                : 5;
		uint32_t _rsv_11                         : 1;
		uint32_t u5_bnr_sweight_2                : 5;
		uint32_t _rsv_17                         : 1;
		uint32_t u5_bnr_sweight_3                : 5;
		uint32_t _rsv_23                         : 1;
		uint32_t u5_bnr_sweight_4                : 5;
	} bits;
};

union reg_isp_bnr_1c {
	uint32_t raw;
	struct {
		uint32_t u5_bnr_sweight_5                : 5;
		uint32_t _rsv_5                          : 1;
		uint32_t u5_bnr_sweight_6                : 5;
		uint32_t _rsv_11                         : 1;
		uint32_t u5_bnr_sweight_7                : 5;
		uint32_t _rsv_17                         : 1;
		uint32_t u5_bnr_sweight_8                : 5;
		uint32_t _rsv_23                         : 1;
		uint32_t u5_bnr_sweight_9                : 5;
	} bits;
};

union reg_isp_bnr_20 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_base_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_base_1     : 10;
	} bits;
};

union reg_isp_bnr_24 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_base_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_base_3     : 10;
	} bits;
};

union reg_isp_bnr_28 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_base_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_base_5     : 10;
	} bits;
};

union reg_isp_bnr_2c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_base_0    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_base_1    : 10;
	} bits;
};

union reg_isp_bnr_30 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_base_2    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_base_3    : 10;
	} bits;
};

union reg_isp_bnr_34 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_base_4    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_base_5    : 10;
	} bits;
};

union reg_isp_bnr_38 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_base_0    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_base_1    : 10;
	} bits;
};

union reg_isp_bnr_3c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_base_2    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_base_3    : 10;
	} bits;
};

union reg_isp_bnr_40 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_base_4    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_base_5    : 10;
	} bits;
};

union reg_isp_bnr_44 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_base_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_base_1     : 10;
	} bits;
};

union reg_isp_bnr_48 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_base_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_base_3     : 10;
	} bits;
};

union reg_isp_bnr_4c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_base_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_base_5     : 10;
	} bits;
};

union reg_isp_bnr_50 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_luma_1     : 10;
	} bits;
};

union reg_isp_bnr_54 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_luma_3     : 10;
	} bits;
};

union reg_isp_bnr_58 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_luma_5     : 10;
	} bits;
};

union reg_isp_bnr_5c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_6     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_luma_7     : 10;
	} bits;
};

union reg_isp_bnr_60 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_8     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_luma_9     : 10;
	} bits;
};

union reg_isp_bnr_64 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_10    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_luma_11    : 10;
	} bits;
};

union reg_isp_bnr_68 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_12    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_luma_13    : 10;
	} bits;
};

union reg_isp_bnr_6c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_14    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_r_lut_luma_15    : 10;
	} bits;
};

union reg_isp_bnr_70 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_r_lut_luma_16    : 10;
	} bits;
};

union reg_isp_bnr_74 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_0    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_luma_1    : 10;
	} bits;
};

union reg_isp_bnr_78 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_2    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_luma_3    : 10;
	} bits;
};

union reg_isp_bnr_7c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_4    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_luma_5    : 10;
	} bits;
};

union reg_isp_bnr_80 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_6    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_luma_7    : 10;
	} bits;
};

union reg_isp_bnr_84 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_8    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_luma_9    : 10;
	} bits;
};

union reg_isp_bnr_88 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_10   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_luma_11   : 10;
	} bits;
};

union reg_isp_bnr_8c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_12   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_luma_13   : 10;
	} bits;
};

union reg_isp_bnr_90 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_14   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gr_lut_luma_15   : 10;
	} bits;
};

union reg_isp_bnr_94 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gr_lut_luma_16   : 10;
	} bits;
};

union reg_isp_bnr_98 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_0    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_luma_1    : 10;
	} bits;
};

union reg_isp_bnr_9c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_2    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_luma_3    : 10;
	} bits;
};

union reg_isp_bnr_a0 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_4    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_luma_5    : 10;
	} bits;
};

union reg_isp_bnr_a4 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_6    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_luma_7    : 10;
	} bits;
};

union reg_isp_bnr_a8 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_8    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_luma_9    : 10;
	} bits;
};

union reg_isp_bnr_ac {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_10   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_luma_11   : 10;
	} bits;
};

union reg_isp_bnr_b0 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_12   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_luma_13   : 10;
	} bits;
};

union reg_isp_bnr_b4 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_14   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_gb_lut_luma_15   : 10;
	} bits;
};

union reg_isp_bnr_b8 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_gb_lut_luma_16   : 10;
	} bits;
};

union reg_isp_bnr_bc {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_luma_1     : 10;
	} bits;
};

union reg_isp_bnr_c0 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_luma_3     : 10;
	} bits;
};

union reg_isp_bnr_c4 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_luma_5     : 10;
	} bits;
};

union reg_isp_bnr_c8 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_6     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_luma_7     : 10;
	} bits;
};

union reg_isp_bnr_cc {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_8     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_luma_9     : 10;
	} bits;
};

union reg_isp_bnr_d0 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_10    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_luma_11    : 10;
	} bits;
};

union reg_isp_bnr_d4 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_12    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_luma_13    : 10;
	} bits;
};

union reg_isp_bnr_d8 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_14    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthnlm_b_lut_luma_15    : 10;
	} bits;
};

union reg_isp_bnr_dc {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthnlm_b_lut_luma_16    : 10;
	} bits;
};

union reg_isp_bnr_e0 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_base_0      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_base_1      : 10;
	} bits;
};

union reg_isp_bnr_e4 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_base_2      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_base_3      : 10;
	} bits;
};

union reg_isp_bnr_e8 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_base_4      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_base_5      : 10;
	} bits;
};

union reg_isp_bnr_ec {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_base_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_base_1     : 10;
	} bits;
};

union reg_isp_bnr_f0 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_base_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_base_3     : 10;
	} bits;
};

union reg_isp_bnr_f4 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_base_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_base_5     : 10;
	} bits;
};

union reg_isp_bnr_f8 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_base_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_base_1     : 10;
	} bits;
};

union reg_isp_bnr_fc {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_base_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_base_3     : 10;
	} bits;
};

union reg_isp_bnr_100 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_base_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_base_5     : 10;
	} bits;
};

union reg_isp_bnr_104 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_base_0      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_base_1      : 10;
	} bits;
};

union reg_isp_bnr_108 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_base_2      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_base_3      : 10;
	} bits;
};

union reg_isp_bnr_10c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_base_4      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_base_5      : 10;
	} bits;
};

union reg_isp_bnr_110 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_0      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_luma_1      : 10;
	} bits;
};

union reg_isp_bnr_114 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_2      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_luma_3      : 10;
	} bits;
};

union reg_isp_bnr_118 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_4      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_luma_5      : 10;
	} bits;
};

union reg_isp_bnr_11c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_6      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_luma_7      : 10;
	} bits;
};

union reg_isp_bnr_120 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_8      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_luma_9      : 10;
	} bits;
};

union reg_isp_bnr_124 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_10     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_luma_11     : 10;
	} bits;
};

union reg_isp_bnr_128 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_12     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_luma_13     : 10;
	} bits;
};

union reg_isp_bnr_12c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_14     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_r_lut_luma_15     : 10;
	} bits;
};

union reg_isp_bnr_130 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_r_lut_luma_16     : 10;
	} bits;
};

union reg_isp_bnr_134 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_luma_1     : 10;
	} bits;
};

union reg_isp_bnr_138 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_luma_3     : 10;
	} bits;
};

union reg_isp_bnr_13c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_luma_5     : 10;
	} bits;
};

union reg_isp_bnr_140 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_6     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_luma_7     : 10;
	} bits;
};

union reg_isp_bnr_144 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_8     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_luma_9     : 10;
	} bits;
};

union reg_isp_bnr_148 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_10    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_luma_11    : 10;
	} bits;
};

union reg_isp_bnr_14c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_12    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_luma_13    : 10;
	} bits;
};

union reg_isp_bnr_150 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_14    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gr_lut_luma_15    : 10;
	} bits;
};

union reg_isp_bnr_154 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gr_lut_luma_16    : 10;
	} bits;
};

union reg_isp_bnr_158 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_luma_1     : 10;
	} bits;
};

union reg_isp_bnr_15c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_luma_3     : 10;
	} bits;
};

union reg_isp_bnr_160 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_luma_5     : 10;
	} bits;
};

union reg_isp_bnr_164 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_6     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_luma_7     : 10;
	} bits;
};

union reg_isp_bnr_168 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_8     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_luma_9     : 10;
	} bits;
};

union reg_isp_bnr_16c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_10    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_luma_11    : 10;
	} bits;
};

union reg_isp_bnr_170 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_12    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_luma_13    : 10;
	} bits;
};

union reg_isp_bnr_174 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_14    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_gb_lut_luma_15    : 10;
	} bits;
};

union reg_isp_bnr_178 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_gb_lut_luma_16    : 10;
	} bits;
};

union reg_isp_bnr_17c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_0      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_luma_1      : 10;
	} bits;
};

union reg_isp_bnr_180 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_2      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_luma_3      : 10;
	} bits;
};

union reg_isp_bnr_184 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_4      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_luma_5      : 10;
	} bits;
};

union reg_isp_bnr_188 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_6      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_luma_7      : 10;
	} bits;
};

union reg_isp_bnr_18c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_8      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_luma_9      : 10;
	} bits;
};

union reg_isp_bnr_190 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_10     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_luma_11     : 10;
	} bits;
};

union reg_isp_bnr_194 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_12     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_luma_13     : 10;
	} bits;
};

union reg_isp_bnr_198 {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_14     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t u10_bnr_rthbf_b_lut_luma_15     : 10;
	} bits;
};

union reg_isp_bnr_19c {
	uint32_t raw;
	struct {
		uint32_t u10_bnr_rthbf_b_lut_luma_16     : 10;
	} bits;
};

union reg_isp_bnr_1a0 {
	uint32_t raw;
	struct {
		uint32_t dummy_shd                       : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_ca_00 {
	uint32_t raw;
	struct {
		uint32_t cacp_enable                     : 1;
		uint32_t cacp_mode                       : 1;
		uint32_t cacp_dbg_mode                   : 1;
		uint32_t cacp_mem_sw_mode                : 1;
		uint32_t cacp_shdw_read_sel              : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t prog_hdk_dis                    : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t cacp_iso_ratio                  : 11;
	} bits;
};

union reg_ca_04 {
	uint32_t raw;
	struct {
		uint32_t cacp_mem_d                      : 24;
		uint32_t _rsv_24                         : 7;
		uint32_t cacp_mem_w                      : 1;
	} bits;
};

union reg_ca_08 {
	uint32_t raw;
	struct {
		uint32_t cacp_mem_st_addr                : 8;
		uint32_t _rsv_8                          : 23;
		uint32_t cacp_mem_st_addr_w              : 1;
	} bits;
};

union reg_ca_0c {
	uint32_t raw;
	struct {
		uint32_t cacp_mem_sw_raddr               : 8;
		uint32_t _rsv_8                          : 23;
		uint32_t cacp_mem_sw_r                   : 1;
	} bits;
};

union reg_ca_10 {
	uint32_t raw;
	struct {
		uint32_t cacp_mem_sw_rdata_r             : 24;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_ca_lite_00 {
	uint32_t raw;
	struct {
		uint32_t ca_lite_enable                  : 1;
		uint32_t ca_lite_shdw_read_sel           : 1;
	} bits;
};

union reg_ca_lite_04 {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_in_0                : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t ca_lite_lut_in_1                : 9;
	} bits;
};

union reg_ca_lite_08 {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_in_2                : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t ca_lite_lut_in_3                : 9;
	} bits;
};

union reg_ca_lite_0c {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_in_4                : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t ca_lite_lut_in_5                : 9;
	} bits;
};

union reg_ca_lite_10 {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_out_0               : 11;
		uint32_t _rsv_11                         : 5;
		uint32_t ca_lite_lut_out_1               : 11;
	} bits;
};

union reg_ca_lite_14 {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_out_2               : 11;
		uint32_t _rsv_11                         : 5;
		uint32_t ca_lite_lut_out_3               : 11;
	} bits;
};

union reg_ca_lite_18 {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_out_4               : 11;
		uint32_t _rsv_11                         : 5;
		uint32_t ca_lite_lut_out_5               : 11;
	} bits;
};

union reg_ca_lite_1c {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_slp_0               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t ca_lite_lut_slp_1               : 12;
	} bits;
};

union reg_ca_lite_20 {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_slp_2               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t ca_lite_lut_slp_3               : 12;
	} bits;
};

union reg_ca_lite_24 {
	uint32_t raw;
	struct {
		uint32_t ca_lite_lut_slp_4               : 12;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_ccm_ccm_00 {
	uint32_t raw;
	struct {
		uint32_t ccm_00                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_01 {
	uint32_t raw;
	struct {
		uint32_t ccm_01                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_02 {
	uint32_t raw;
	struct {
		uint32_t ccm_02                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_10 {
	uint32_t raw;
	struct {
		uint32_t ccm_10                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_11 {
	uint32_t raw;
	struct {
		uint32_t ccm_11                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_12 {
	uint32_t raw;
	struct {
		uint32_t ccm_12                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_20 {
	uint32_t raw;
	struct {
		uint32_t ccm_20                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_21 {
	uint32_t raw;
	struct {
		uint32_t ccm_21                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_22 {
	uint32_t raw;
	struct {
		uint32_t ccm_22                          : 14;
	} bits;
};

union reg_isp_ccm_ccm_ctrl {
	uint32_t raw;
	struct {
		uint32_t ccm_shdw_sel                    : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t ccm_enable                      : 1;
	} bits;
};

union reg_isp_ccm_ccm_dbg {
	uint32_t raw;
	struct {
		uint32_t force_clk_enable                : 1;
		uint32_t softrst                         : 1;
	} bits;
};

union reg_isp_ccm_dmy0 {
	uint32_t raw;
	struct {
		uint32_t dmy_def0                        : 32;
	} bits;
};

union reg_isp_ccm_dmy1 {
	uint32_t raw;
	struct {
		uint32_t dmy_def1                        : 32;
	} bits;
};

union reg_isp_ccm_dmy_r {
	uint32_t raw;
	struct {
		uint32_t dmy_ro                          : 32;
	} bits;
};

union reg_isp_ccm_ccm_overexp_str {
	uint32_t raw;
	struct {
		uint32_t ccm_overexp_str                 : 8;
	} bits;
};

union reg_isp_ccm_ccm_overexp_thr {
	uint32_t raw;
	struct {
		uint32_t ccm_overexp_thr                 : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_cfa_00 {
	uint32_t raw;
	struct {
		uint32_t cfa_shdw_sel                    : 1;
		uint32_t cfa_enable                      : 1;
		uint32_t _rsv_2                          : 1;
		uint32_t cfa_ymoire_enable               : 1;
		uint32_t delay                           : 1;
		uint32_t force_clk_enable                : 1;
		uint32_t cfa_force_dir_enable            : 1;
		uint32_t _rsv_7                          : 1;
		uint32_t cfa_force_dir_sel               : 2;
	} bits;
};

union reg_isp_cfa_04 {
	uint32_t raw;
	struct {
		uint32_t cfa_out_sel                     : 1;
		uint32_t cont_en                         : 1;
		uint32_t _rsv_2                          : 1;
		uint32_t softrst                         : 1;
		uint32_t _rsv_4                          : 1;
		uint32_t dbg_en                          : 1;
		uint32_t _rsv_6                          : 10;
		uint32_t cfa_edgee_thd2                  : 12;
	} bits;
};

union reg_isp_cfa_0c {
	uint32_t raw;
	struct {
		uint32_t cfa_edgee_thd                   : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_sige_thd                    : 12;
	} bits;
};

union reg_isp_cfa_10 {
	uint32_t raw;
	struct {
		uint32_t cfa_gsig_tol                    : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_rbsig_tol                   : 12;
	} bits;
};

union reg_isp_cfa_14 {
	uint32_t raw;
	struct {
		uint32_t cfa_edge_tol                    : 12;
	} bits;
};

union reg_isp_cfa_18 {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_thd                     : 16;
	} bits;
};

union reg_isp_cfa_1c {
	uint32_t raw;
	struct {
		uint32_t cfa_rb_vt_enable                : 1;
	} bits;
};

union reg_isp_cfa_20 {
	uint32_t raw;
	struct {
		uint32_t cfa_rbsig_luma_thd              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_vw_thd                      : 12;
	} bits;
};

union reg_isp_cfa_30 {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_lut00                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t cfa_ghp_lut01                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t cfa_ghp_lut02                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t cfa_ghp_lut03                   : 6;
	} bits;
};

union reg_isp_cfa_34 {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_lut04                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t cfa_ghp_lut05                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t cfa_ghp_lut06                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t cfa_ghp_lut07                   : 6;
	} bits;
};

union reg_isp_cfa_38 {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_lut08                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t cfa_ghp_lut09                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t cfa_ghp_lut10                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t cfa_ghp_lut11                   : 6;
	} bits;
};

union reg_isp_cfa_3c {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_lut12                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t cfa_ghp_lut13                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t cfa_ghp_lut14                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t cfa_ghp_lut15                   : 6;
	} bits;
};

union reg_isp_cfa_40 {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_lut16                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t cfa_ghp_lut17                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t cfa_ghp_lut18                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t cfa_ghp_lut19                   : 6;
	} bits;
};

union reg_isp_cfa_44 {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_lut20                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t cfa_ghp_lut21                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t cfa_ghp_lut22                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t cfa_ghp_lut23                   : 6;
	} bits;
};

union reg_isp_cfa_48 {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_lut24                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t cfa_ghp_lut25                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t cfa_ghp_lut26                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t cfa_ghp_lut27                   : 6;
	} bits;
};

union reg_isp_cfa_4c {
	uint32_t raw;
	struct {
		uint32_t cfa_ghp_lut28                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t cfa_ghp_lut29                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t cfa_ghp_lut30                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t cfa_ghp_lut31                   : 6;
	} bits;
};

union reg_isp_cfa_70 {
	uint32_t raw;
	struct {
		uint32_t dir_readcnt_from_line0          : 5;
	} bits;
};

union reg_isp_cfa_74 {
	uint32_t raw;
	struct {
		uint32_t prob_out_sel                    : 4;
		uint32_t prob_perfmt                     : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t prob_fmt                        : 6;
	} bits;
};

union reg_isp_cfa_78 {
	uint32_t raw;
	struct {
		uint32_t prob_line                       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t prob_pix                        : 12;
	} bits;
};

union reg_isp_cfa_7c {
	uint32_t raw;
	struct {
		uint32_t cfa_dbg0                        : 32;
	} bits;
};

union reg_isp_cfa_80 {
	uint32_t raw;
	struct {
		uint32_t cfa_dbg1                        : 32;
	} bits;
};

union reg_isp_cfa_90 {
	uint32_t raw;
	struct {
		uint32_t cfa_ymoire_ref_maxg_only        : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t cfa_ymoire_np                   : 8;
	} bits;
};

union reg_isp_cfa_94 {
	uint32_t raw;
	struct {
		uint32_t cfa_ymoire_detail_th            : 8;
		uint32_t cfa_ymoire_detail_low           : 9;
	} bits;
};

union reg_isp_cfa_98 {
	uint32_t raw;
	struct {
		uint32_t cfa_ymoire_detail_high          : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t cfa_ymoire_detail_slope         : 15;
	} bits;
};

union reg_isp_cfa_9c {
	uint32_t raw;
	struct {
		uint32_t cfa_ymoire_edge_th              : 8;
		uint32_t cfa_ymoire_edge_low             : 9;
	} bits;
};

union reg_isp_cfa_a0 {
	uint32_t raw;
	struct {
		uint32_t cfa_ymoire_edge_high            : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t cfa_ymoire_edge_slope           : 15;
	} bits;
};

union reg_isp_cfa_a4 {
	uint32_t raw;
	struct {
		uint32_t cfa_ymoire_lut_th               : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t cfa_ymoire_lut_low              : 9;
	} bits;
};

union reg_isp_cfa_a8 {
	uint32_t raw;
	struct {
		uint32_t cfa_ymoire_lut_high             : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t cfa_ymoire_lut_slope            : 15;
	} bits;
};

union reg_isp_cfa_110 {
	uint32_t raw;
	struct {
		uint32_t cfa_ymoire_lpf_w                : 8;
		uint32_t cfa_ymoire_dc_w                 : 8;
	} bits;
};

union reg_isp_cfa_120 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_enable               : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t cfa_cmoire_strth                : 8;
	} bits;
};

union reg_isp_cfa_124 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_sat_x0               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_sat_y0               : 12;
	} bits;
};

union reg_isp_cfa_128 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_sat_slp0             : 18;
	} bits;
};

union reg_isp_cfa_12c {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_sat_x1               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_sat_y1               : 12;
	} bits;
};

union reg_isp_cfa_130 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_ptclr_x0             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_ptclr_y0             : 12;
	} bits;
};

union reg_isp_cfa_134 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_ptclr_slp0           : 18;
	} bits;
};

union reg_isp_cfa_138 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_ptclr_x1             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_ptclr_y1             : 12;
	} bits;
};

union reg_isp_cfa_13c {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_protclr1_enable      : 1;
		uint32_t cfa_cmoire_protclr2_enable      : 1;
		uint32_t cfa_cmoire_protclr3_enable      : 1;
	} bits;
};

union reg_isp_cfa_140 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_protclr1             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_protclr2             : 12;
	} bits;
};

union reg_isp_cfa_144 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_protclr3             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_pd_x0                : 12;
	} bits;
};

union reg_isp_cfa_148 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_pd_y0                : 12;
	} bits;
};

union reg_isp_cfa_14c {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_pd_slp0              : 18;
	} bits;
};

union reg_isp_cfa_150 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_pd_x1                : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_pd_y1                : 12;
	} bits;
};

union reg_isp_cfa_154 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_x0              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_edge_y0              : 12;
	} bits;
};

union reg_isp_cfa_158 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_slp0            : 18;
	} bits;
};

union reg_isp_cfa_15c {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_x1              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t cfa_cmoire_edge_y1              : 12;
	} bits;
};

union reg_isp_cfa_160 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_lumagain_enable      : 1;
	} bits;
};

union reg_isp_cfa_164 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_lumatg               : 12;
	} bits;
};

union reg_isp_cfa_168 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_d0c0            : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t cfa_cmoire_edge_d0c1            : 13;
	} bits;
};

union reg_isp_cfa_16c {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_d0c2            : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t cfa_cmoire_edge_d45c0           : 13;
	} bits;
};

union reg_isp_cfa_170 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_d45c1           : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t cfa_cmoire_edge_d45c2           : 13;
	} bits;
};

union reg_isp_cfa_174 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_d45c3           : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t cfa_cmoire_edge_d45c4           : 13;
	} bits;
};

union reg_isp_cfa_178 {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_d45c5           : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t cfa_cmoire_edge_d45c6           : 13;
	} bits;
};

union reg_isp_cfa_17c {
	uint32_t raw;
	struct {
		uint32_t cfa_cmoire_edge_d45c7           : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t cfa_cmoire_edge_d45c8           : 13;
	} bits;
};

union reg_isp_cfa_180 {
	uint32_t raw;
	struct {
		uint32_t _cfa_shpn_enable                : 1;
		uint32_t _cfa_shpn_pre_proc_enable       : 1;
		uint32_t _rsv_2                          : 6;
		uint32_t _cfa_shpn_min_y                 : 12;
	} bits;
};

union reg_isp_cfa_184 {
	uint32_t raw;
	struct {
		uint32_t _cfa_shpn_min_gain              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t _cfa_shpn_max_gain              : 12;
	} bits;
};

union reg_isp_cfa_188 {
	uint32_t raw;
	struct {
		uint32_t cfa_shpn_mf_core_gain           : 8;
		uint32_t cfa_shpn_hf_blend_wgt           : 8;
		uint32_t cfa_shpn_mf_blend_wgt           : 8;
	} bits;
};

union reg_isp_cfa_18c {
	uint32_t raw;
	struct {
		uint32_t cfa_shpn_core_value             : 12;
	} bits;
};

union reg_isp_cfa_190 {
	uint32_t raw;
	struct {
		uint32_t cfa_shpn_gain_value             : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_clut_ctrl {
	uint32_t raw;
	struct {
		uint32_t clut_enable                     : 1;
		uint32_t clut_shdw_sel                   : 1;
		uint32_t force_clk_enable                : 1;
		uint32_t prog_en                         : 1;
	} bits;
};

union reg_isp_clut_prog_addr {
	uint32_t raw;
	struct {
		uint32_t sram_r_idx                      : 5;
		uint32_t _rsv_5                          : 3;
		uint32_t sram_g_idx                      : 5;
		uint32_t _rsv_13                         : 3;
		uint32_t sram_b_idx                      : 5;
	} bits;
};

union reg_isp_clut_prog_data {
	uint32_t raw;
	struct {
		uint32_t sram_wdata                      : 30;
		uint32_t _rsv_30                         : 1;
		uint32_t sram_wr                         : 1;
	} bits;
};

union reg_isp_clut_prog_rdata {
	uint32_t raw;
	struct {
		uint32_t sram_rdata                      : 30;
		uint32_t _rsv_30                         : 1;
		uint32_t sram_rd                         : 1;
	} bits;
};

union reg_isp_clut_dbg {
	uint32_t raw;
	struct {
		uint32_t prog_hdk_dis                    : 1;
	} bits;
};

union reg_isp_clut_dmy0 {
	uint32_t raw;
	struct {
		uint32_t dmy_def0                        : 32;
	} bits;
};

union reg_isp_clut_dmy1 {
	uint32_t raw;
	struct {
		uint32_t dmy_def1                        : 32;
	} bits;
};

union reg_isp_clut_dmy_r {
	uint32_t raw;
	struct {
		uint32_t dmy_ro                          : 32;
	} bits;
};

union reg_isp_clut_lut_fill_select {
	uint32_t raw;
	struct {
		uint32_t fill_sel_sw                     : 1;
	} bits;
};

union reg_isp_clut_rdma_sw_start_1t {
	uint32_t raw;
	struct {
		uint32_t clut_rdma_sw_start_1t           : 1;
	} bits;
};

union reg_isp_clut_prog_itrp {
	uint32_t raw;
	struct {
		uint32_t clut_prog_itrp                  : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_cnr_cnr_ctrl_hw_only {
	uint32_t raw;
	struct {
		uint32_t cnr_enable                      : 1;
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t tile_mode                       : 2;
		uint32_t _rsv_4                          : 4;
		uint32_t first_frame_reset               : 1;
	} bits;
};

union reg_cnr_mmp_scl_in_size {
	uint32_t raw;
	struct {
		uint32_t mmp_scl_in_imgw                 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t mmp_scl_in_imgh                 : 14;
	} bits;
};

union reg_cnr_scl_down_ctrl {
	uint32_t raw;
	struct {
		uint32_t scl_down_ctrl_en                : 1;
		uint32_t scl_down_mode                   : 2;
	} bits;
};

union reg_cnr_cnr_ctrl_sw_hw {
	uint32_t raw;
	struct {
		uint32_t cnr_cmf_en                      : 1;
		uint32_t cnr_lca_enable                  : 1;
		uint32_t cnr_ife2_filter_en              : 1;
		uint32_t cnr_chra_en                     : 1;
		uint32_t cnr_m_chra_bypass               : 1;
		uint32_t cnr_chra_dbgmode                : 3;
		uint32_t cnr_chra_sat_outbld_en          : 1;
	} bits;
};

union reg_cnr_cnr_subim_outsel {
	uint32_t raw;
	struct {
		uint32_t cnr_subim_outsel                : 1;
	} bits;
};

union reg_cnr_cnr_cmf_ksize {
	uint32_t raw;
	struct {
		uint32_t cnr_cmf_ksize                   : 2;
	} bits;
};

union reg_cnr_cnr_ife2_fsize_sel {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_fsize_sel              : 2;
	} bits;
};

union reg_cnr_cnr_ife2_eksize_sel {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_eksize_sel             : 2;
	} bits;
};

union reg_cnr_cnr_ife2_y_rcth {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_y_rcth_0               : 8;
		uint32_t cnr_ife2_y_rcth_1               : 8;
		uint32_t cnr_ife2_y_rcth_2               : 8;
	} bits;
};

union reg_cnr_cnr_ife2_y_cwt {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_y_cwt                  : 5;
	} bits;
};

union reg_cnr_cnr_ife2_y_rcwt {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_y_rcwt_0               : 4;
		uint32_t cnr_ife2_y_rcwt_1               : 4;
		uint32_t cnr_ife2_y_rcwt_2               : 4;
		uint32_t cnr_ife2_y_rcwt_3               : 4;
	} bits;
};

union reg_cnr_cnr_ife2_y_outl_th {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_y_outl_th              : 3;
	} bits;
};

union reg_cnr_cnr_ife2_uv_rcth {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_uv_rcth_0              : 8;
		uint32_t cnr_ife2_uv_rcth_1              : 8;
		uint32_t cnr_ife2_uv_rcth_2              : 8;
	} bits;
};

union reg_cnr_cnr_ife2_uv_cwt {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_uv_cwt                 : 5;
	} bits;
};

union reg_cnr_cnr_ife2_uv_rcwt {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_uv_rcwt_0              : 4;
		uint32_t cnr_ife2_uv_rcwt_1              : 4;
		uint32_t cnr_ife2_uv_rcwt_2              : 4;
		uint32_t cnr_ife2_uv_rcwt_3              : 4;
	} bits;
};

union reg_cnr_cnr_ife2_uv_outl_th {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_uv_outl_th             : 3;
	} bits;
};

union reg_cnr_cnr_ife2_y_outl_dth {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_y_outl_dth             : 8;
	} bits;
};

union reg_cnr_cnr_ife2_u_outl_dth {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_u_outl_dth             : 8;
	} bits;
};

union reg_cnr_cnr_ife2_v_outl_dth {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_v_outl_dth             : 8;
	} bits;
};

union reg_cnr_cnr_ife2_ed_pn_th {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_ed_pn_th               : 8;
	} bits;
};

union reg_cnr_cnr_ife2_ed_hv_th {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_ed_hv_th               : 8;
	} bits;
};

union reg_cnr_cnr_ife2_y_fth {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_y_fth_0                : 8;
		uint32_t cnr_ife2_y_fth_1                : 8;
		uint32_t cnr_ife2_y_fth_2                : 8;
		uint32_t cnr_ife2_y_fth_3                : 8;
	} bits;
};

union reg_cnr_cnr_ife2_y_fth_4 {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_y_fth_4                : 8;
	} bits;
};

union reg_cnr_cnr_ife2_y_fwt {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_y_fwt_0                : 5;
		uint32_t cnr_ife2_y_fwt_1                : 5;
		uint32_t cnr_ife2_y_fwt_2                : 5;
		uint32_t cnr_ife2_y_fwt_3                : 5;
		uint32_t cnr_ife2_y_fwt_4                : 5;
		uint32_t cnr_ife2_y_fwt_5                : 5;
	} bits;
};

union reg_cnr_cnr_ife2_u_fth {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_u_fth_0                : 8;
		uint32_t cnr_ife2_u_fth_1                : 8;
		uint32_t cnr_ife2_u_fth_2                : 8;
		uint32_t cnr_ife2_u_fth_3                : 8;
	} bits;
};

union reg_cnr_cnr_ife2_u_fth_4 {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_u_fth_4                : 8;
	} bits;
};

union reg_cnr_cnr_ife2_u_fwt {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_u_fwt_0                : 5;
		uint32_t cnr_ife2_u_fwt_1                : 5;
		uint32_t cnr_ife2_u_fwt_2                : 5;
		uint32_t cnr_ife2_u_fwt_3                : 5;
		uint32_t cnr_ife2_u_fwt_4                : 5;
		uint32_t cnr_ife2_u_fwt_5                : 5;
	} bits;
};

union reg_cnr_cnr_ife2_v_fth {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_v_fth_0                : 8;
		uint32_t cnr_ife2_v_fth_1                : 8;
		uint32_t cnr_ife2_v_fth_2                : 8;
		uint32_t cnr_ife2_v_fth_3                : 8;
	} bits;
};

union reg_cnr_cnr_ife2_v_fth_4 {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_v_fth_4                : 8;
	} bits;
};

union reg_cnr_cnr_ife2_v_fwt {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_v_fwt_0                : 5;
		uint32_t cnr_ife2_v_fwt_1                : 5;
		uint32_t cnr_ife2_v_fwt_2                : 5;
		uint32_t cnr_ife2_v_fwt_3                : 5;
		uint32_t cnr_ife2_v_fwt_4                : 5;
		uint32_t cnr_ife2_v_fwt_5                : 5;
	} bits;
};

union reg_cnr_cnr_ife2_yftr_en {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_yftr_en                : 1;
	} bits;
};

union reg_cnr_cnr_ife2_egd_en {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_egd_en                 : 1;
	} bits;
};

union reg_cnr_cnr_ife2_rc_en {
	uint32_t raw;
	struct {
		uint32_t cnr_ife2_rc_en                  : 1;
	} bits;
};

union reg_cnr_cnr_chra_refy_wt {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_refy_wt                : 5;
	} bits;
};

union reg_cnr_cnr_chra_refc_wt {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_refc_wt                : 5;
	} bits;
};

union reg_cnr_cnr_chra_out_wt_lut {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_out_wt_lut_0           : 5;
		uint32_t _rsv_5                          : 3;
		uint32_t cnr_chra_out_wt_lut_1           : 5;
		uint32_t _rsv_13                         : 3;
		uint32_t cnr_chra_out_wt_lut_2           : 5;
	} bits;
};

union reg_cnr_cnr_chra_y_rng {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_y_rng                  : 2;
	} bits;
};

union reg_cnr_cnr_chra_y_wtprc {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_y_wtprc                : 2;
	} bits;
};

union reg_cnr_cnr_chra_y_th {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_y_th                   : 5;
	} bits;
};

union reg_cnr_cnr_chra_y_wts {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_y_wts                  : 6;
	} bits;
};

union reg_cnr_cnr_chra_y_wte {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_y_wte                  : 6;
	} bits;
};

union reg_cnr_cnr_chra_uv_rng {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_uv_rng                 : 3;
	} bits;
};

union reg_cnr_cnr_chra_uv_wtprc {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_uv_wtprc               : 2;
	} bits;
};

union reg_cnr_cnr_chra_uv_th {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_uv_th                  : 6;
	} bits;
};

union reg_cnr_cnr_chra_uv_wts {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_uv_wts                 : 7;
	} bits;
};

union reg_cnr_cnr_chra_uv_wte {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_uv_wte                 : 7;
	} bits;
};

union reg_cnr_cnr_chra_sat_rng {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_sat_rng                : 3;
	} bits;
};

union reg_cnr_cnr_chra_sat_wtprc {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_sat_wtprc              : 2;
	} bits;
};

union reg_cnr_cnr_chra_sat_th {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_sat_th                 : 5;
	} bits;
};

union reg_cnr_cnr_chra_sat_wts {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_sat_wts                : 6;
	} bits;
};

union reg_cnr_cnr_chra_sat_wte {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_sat_wte                : 6;
	} bits;
};

union reg_cnr_cnr_chra_sat_outbld_coring {
	uint32_t raw;
	struct {
		uint32_t cnr_chra_sat_outbld_coring      : 6;
	} bits;
};

union reg_cnr_cnr_lca_h_sfact {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_h_sfact                 : 16;
	} bits;
};

union reg_cnr_cnr_lca_v_sfact {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_v_sfact                 : 16;
	} bits;
};

union reg_cnr_cnr_lca_h_sfact_init_ofs {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_h_sfact_init_ofs        : 17;
	} bits;
};

union reg_cnr_cnr_lca_v_sfact_init_ofs {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_v_sfact_init_ofs        : 17;
	} bits;
};

union reg_cnr_cnr_lca_src_img_size_h {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_src_img_size_h          : 14;
	} bits;
};

union reg_cnr_cnr_lca_src_img_size_v {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_src_img_size_v          : 14;
	} bits;
};

union reg_cnr_cnr_lca_sub_img_size_h {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_sub_img_size_h          : 14;
	} bits;
};

union reg_cnr_cnr_lca_sub_img_size_v {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_sub_img_size_v          : 14;
	} bits;
};

union reg_cnr_cnr_lca_hv_filtmode {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_hv_filtmode             : 1;
	} bits;
};

union reg_cnr_cnr_lca_hv_coef {
	uint32_t raw;
	struct {
		uint32_t cnr_lca_hv_coef                 : 6;
	} bits;
};

union reg_cnr_upscl_h_sfact {
	uint32_t raw;
	struct {
		uint32_t upscl_h_sfact                   : 16;
	} bits;
};

union reg_cnr_upscl_v_sfact {
	uint32_t raw;
	struct {
		uint32_t upscl_v_sfact                   : 16;
	} bits;
};

union reg_cnr_upscl_h_sfact_init_ofs {
	uint32_t raw;
	struct {
		uint32_t upscl_h_sfact_init_ofs          : 17;
	} bits;
};

union reg_cnr_upscl_v_sfact_init_ofs {
	uint32_t raw;
	struct {
		uint32_t upscl_v_sfact_init_ofs          : 17;
	} bits;
};

union reg_cnr_upscl_hv_filtmode {
	uint32_t raw;
	struct {
		uint32_t upscl_hv_filtmode               : 1;
	} bits;
};

union reg_cnr_upscl_hv_coef {
	uint32_t raw;
	struct {
		uint32_t upscl_hv_coef                   : 6;
	} bits;
};

union reg_cnr_shdw_read_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_crop_0 {
	uint32_t raw;
	struct {
		uint32_t crop_enable                     : 1;
		uint32_t dma_enable                      : 1;
		uint32_t shaw_read_sel                   : 1;
		uint32_t dmi_qos                         : 1;
		uint32_t _rsv_4                          : 4;
		uint32_t dpcm_mode                       : 3;
		uint32_t _rsv_11                         : 5;
		uint32_t dpcm_xstr                       : 13;
		uint32_t _rsv_29                         : 2;
		uint32_t dmi16b_en                       : 1;
	} bits;
};

union reg_crop_1 {
	uint32_t raw;
	struct {
		uint32_t crop_start_y                    : 14;
		uint32_t _rsv_13                         : 2;
		uint32_t crop_end_y                      : 14;
	} bits;
};

union reg_crop_2 {
	uint32_t raw;
	struct {
		uint32_t crop_start_x                    : 14;
		uint32_t _rsv_13                         : 2;
		uint32_t crop_end_x                      : 14;
	} bits;
};

union reg_crop_3 {
	uint32_t raw;
	struct {
		uint32_t in_widthm1                      : 16;
		uint32_t in_heightm1                     : 16;
	} bits;
};

union reg_crop_dummy {
	uint32_t raw;
	struct {
		uint32_t dummy                           : 32;
	} bits;
};

union reg_crop_debug {
	uint32_t raw;
	struct {
		uint32_t force_clk_enable                : 1;
		uint32_t force_dma_disable               : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_csc_0 {
	uint32_t raw;
	struct {
		uint32_t csc_enable                      : 1;
		uint32_t r2y4_shdw_sel                   : 1;
	} bits;
};

union reg_isp_csc_1 {
	uint32_t raw;
	struct {
		uint32_t op_start                        : 1;
		uint32_t cont_en                         : 1;
		uint32_t r2y4_bypass                     : 1;
		uint32_t softrst                         : 1;
		uint32_t auto_update_en                  : 1;
		uint32_t dbg_en                          : 1;
		uint32_t _rsv_6                          : 10;
		uint32_t check_sum                       : 16;
	} bits;
};

union reg_isp_csc_2 {
	uint32_t raw;
	struct {
		uint32_t shdw_update_req                 : 1;
	} bits;
};

union reg_isp_csc_3 {
	uint32_t raw;
	struct {
		uint32_t dmy0                            : 32;
	} bits;
};

union reg_isp_csc_4 {
	uint32_t raw;
	struct {
		uint32_t coeff_00                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t coeff_01                        : 14;
	} bits;
};

union reg_isp_csc_5 {
	uint32_t raw;
	struct {
		uint32_t coeff_02                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t coeff_10                        : 14;
	} bits;
};

union reg_isp_csc_6 {
	uint32_t raw;
	struct {
		uint32_t coeff_11                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t coeff_12                        : 14;
	} bits;
};

union reg_isp_csc_7 {
	uint32_t raw;
	struct {
		uint32_t coeff_20                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t coeff_21                        : 14;
	} bits;
};

union reg_isp_csc_8 {
	uint32_t raw;
	struct {
		uint32_t coeff_22                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t offset_0                        : 11;
	} bits;
};

union reg_isp_csc_9 {
	uint32_t raw;
	struct {
		uint32_t offset_1                        : 11;
		uint32_t _rsv_11                         : 5;
		uint32_t offset_2                        : 11;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_dehaze_dhz_smooth {
	uint32_t raw;
	struct {
		uint32_t dehaze_w                        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_th_smooth                : 10;
	} bits;
};

union reg_isp_dehaze_dhz_skin {
	uint32_t raw;
	struct {
		uint32_t dehaze_skin_cb                  : 8;
		uint32_t dehaze_skin_cr                  : 8;
	} bits;
};

union reg_isp_dehaze_dhz_wgt {
	uint32_t raw;
	struct {
		uint32_t dehaze_a_luma_wgt               : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t dehaze_blend_wgt                : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t dehaze_tmap_scale               : 8;
		uint32_t dehaze_d_wgt                    : 5;
	} bits;
};

union reg_isp_dehaze_dhz_bypass {
	uint32_t raw;
	struct {
		uint32_t dehaze_enable                   : 1;
		uint32_t dehaze_luma_lut_enable          : 1;
		uint32_t dehaze_skin_lut_enable          : 1;
		uint32_t dehaze_shdw_sel                 : 1;
		uint32_t force_clk_enable                : 1;
	} bits;
};

union reg_isp_dehaze_0 {
	uint32_t raw;
	struct {
		uint32_t softrst                         : 1;
		uint32_t _rsv_1                          : 4;
		uint32_t dbg_en                          : 1;
		uint32_t _rsv_6                          : 10;
		uint32_t check_sum                       : 16;
	} bits;
};

union reg_isp_dehaze_1 {
	uint32_t raw;
	struct {
		uint32_t dehaze_cum_th                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t dehaze_hist_th                  : 14;
	} bits;
};

union reg_isp_dehaze_2 {
	uint32_t raw;
	struct {
		uint32_t dehaze_sw_dc_th                 : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t dehaze_sw_aglobal_r             : 12;
		uint32_t dehaze_sw_dc_trig               : 1;
		uint32_t _rsv_29                         : 2;
		uint32_t dehaze_sw_dc_aglobal_trig       : 1;
	} bits;
};

union reg_isp_dehaze_28 {
	uint32_t raw;
	struct {
		uint32_t dehaze_sw_aglobal_g             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t dehaze_sw_aglobal_b             : 12;
	} bits;
};

union reg_isp_dehaze_2c {
	uint32_t raw;
	struct {
		uint32_t dehaze_aglobal_max              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t dehaze_aglobal_min              : 12;
	} bits;
};

union reg_isp_dehaze_3 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_min                 : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t dehaze_tmap_max                 : 13;
	} bits;
};

union reg_isp_dehaze_5 {
	uint32_t raw;
	struct {
		uint32_t fmt_st                          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t fmt_end                         : 12;
		uint32_t tile_nm                         : 4;
	} bits;
};

union reg_isp_dehaze_6 {
	uint32_t raw;
	struct {
		uint32_t dbg_sel                         : 3;
	} bits;
};

union reg_isp_dehaze_7 {
	uint32_t raw;
	struct {
		uint32_t dhz_dbg0                        : 32;
	} bits;
};

union reg_isp_dehaze_8 {
	uint32_t raw;
	struct {
		uint32_t dhz_dbg1                        : 32;
	} bits;
};

union reg_isp_dehaze_9 {
	uint32_t raw;
	struct {
		uint32_t dehaze_luma_lut00               : 8;
		uint32_t dehaze_luma_lut01               : 8;
		uint32_t dehaze_luma_lut02               : 8;
		uint32_t dehaze_luma_lut03               : 8;
	} bits;
};

union reg_isp_dehaze_10 {
	uint32_t raw;
	struct {
		uint32_t dehaze_luma_lut04               : 8;
		uint32_t dehaze_luma_lut05               : 8;
		uint32_t dehaze_luma_lut06               : 8;
		uint32_t dehaze_luma_lut07               : 8;
	} bits;
};

union reg_isp_dehaze_11 {
	uint32_t raw;
	struct {
		uint32_t dehaze_luma_lut08               : 8;
		uint32_t dehaze_luma_lut09               : 8;
		uint32_t dehaze_luma_lut10               : 8;
		uint32_t dehaze_luma_lut11               : 8;
	} bits;
};

union reg_isp_dehaze_12 {
	uint32_t raw;
	struct {
		uint32_t dehaze_luma_lut12               : 8;
		uint32_t dehaze_luma_lut13               : 8;
		uint32_t dehaze_luma_lut14               : 8;
		uint32_t dehaze_luma_lut15               : 8;
	} bits;
};

union reg_isp_dehaze_17 {
	uint32_t raw;
	struct {
		uint32_t dehaze_skin_lut00               : 8;
		uint32_t dehaze_skin_lut01               : 8;
		uint32_t dehaze_skin_lut02               : 8;
		uint32_t dehaze_skin_lut03               : 8;
	} bits;
};

union reg_isp_dehaze_18 {
	uint32_t raw;
	struct {
		uint32_t dehaze_skin_lut04               : 8;
		uint32_t dehaze_skin_lut05               : 8;
		uint32_t dehaze_skin_lut06               : 8;
		uint32_t dehaze_skin_lut07               : 8;
	} bits;
};

union reg_isp_dehaze_19 {
	uint32_t raw;
	struct {
		uint32_t dehaze_skin_lut08               : 8;
		uint32_t dehaze_skin_lut09               : 8;
		uint32_t dehaze_skin_lut10               : 8;
		uint32_t dehaze_skin_lut11               : 8;
	} bits;
};

union reg_isp_dehaze_20 {
	uint32_t raw;
	struct {
		uint32_t dehaze_skin_lut12               : 8;
		uint32_t dehaze_skin_lut13               : 8;
		uint32_t dehaze_skin_lut14               : 8;
		uint32_t dehaze_skin_lut15               : 8;
	} bits;
};

union reg_isp_dehaze_25 {
	uint32_t raw;
	struct {
		uint32_t aglobal_r                       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t aglobal_g                       : 12;
	} bits;
};

union reg_isp_dehaze_26 {
	uint32_t raw;
	struct {
		uint32_t aglobal_b                       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t dc_th                           : 10;
	} bits;
};

union reg_isp_dehaze_tmap_00 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut000         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut001         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut002         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut003         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_01 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut004         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut005         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut006         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut007         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_02 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut008         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut009         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut010         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut011         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_03 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut012         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut013         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut014         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut015         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_04 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut016         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut017         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut018         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut019         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_05 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut020         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut021         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut022         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut023         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_06 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut024         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut025         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut026         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut027         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_07 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut028         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut029         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut030         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut031         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_08 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut032         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut033         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut034         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut035         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_09 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut036         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut037         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut038         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut039         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_10 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut040         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut041         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut042         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut043         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_11 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut044         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut045         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut046         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut047         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_12 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut048         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut049         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut050         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut051         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_13 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut052         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut053         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut054         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut055         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_14 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut056         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut057         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut058         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut059         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_15 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut060         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut061         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut062         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut063         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_16 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut064         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut065         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut066         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut067         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_17 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut068         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut069         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut070         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut071         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_18 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut072         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut073         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut074         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut075         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_19 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut076         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut077         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut078         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut079         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_20 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut080         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut081         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut082         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut083         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_21 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut084         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut085         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut086         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut087         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_22 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut088         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut089         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut090         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut091         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_23 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut092         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut093         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut094         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut095         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_24 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut096         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut097         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut098         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut099         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_25 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut100         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut101         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut102         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut103         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_26 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut104         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut105         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut106         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut107         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_27 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut108         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut109         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut110         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut111         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_28 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut112         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut113         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut114         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut115         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_29 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut116         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut117         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut118         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut119         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_30 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut120         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut121         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut122         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut123         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_31 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut124         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t dehaze_tmap_gain_lut125         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t dehaze_tmap_gain_lut126         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t dehaze_tmap_gain_lut127         : 7;
	} bits;
};

union reg_isp_dehaze_tmap_32 {
	uint32_t raw;
	struct {
		uint32_t dehaze_tmap_gain_lut128         : 7;
	} bits;
};

union reg_isp_dehaze_o_dc_th_up_mode {
	uint32_t raw;
	struct {
		uint32_t o_dc_th_up_mode                 : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_dpc_base_config {
	uint32_t raw;
	struct {
		uint32_t dpc_enable                      : 1;
		uint32_t spc_enable                      : 1;
		uint32_t ge_enable                       : 1;
		uint32_t cg_enable                       : 1;
	} bits;
};

union reg_dpc_dark_threshold0 {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_dark_th0            : 12;
		uint32_t dpc_outlier_dark_th1            : 12;
	} bits;
};

union reg_dpc_dark_threshold1 {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_dark_th2            : 12;
		uint32_t dpc_outlier_dark_th3            : 12;
	} bits;
};

union reg_dpc_dark_threshold2 {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_dark_th4            : 12;
	} bits;
};

union reg_dpc_bright_threshold0 {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_brit_th0            : 12;
		uint32_t dpc_outlier_brit_th1            : 12;
	} bits;
};

union reg_dpc_bright_threshold1 {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_brit_th2            : 12;
		uint32_t dpc_outlier_brit_th3            : 12;
	} bits;
};

union reg_dpc_bright_threshold2 {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_brit_th4            : 12;
	} bits;
};

union reg_dpc_threshold_offset {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_dark_th_ofs         : 12;
		uint32_t dpc_outlier_brit_th_ofs         : 12;
	} bits;
};

union reg_dpc_outlier_cnt {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_cnt1                : 5;
		uint32_t dpc_outlier_cnt2                : 5;
	} bits;
};

union reg_dpc_outlier_config {
	uint32_t raw;
	struct {
		uint32_t dpc_outlier_adv_mode            : 1;
		uint32_t dpc_outlier_avg_mode            : 1;
		uint32_t dpc_outlier_weight              : 8;
	} bits;
};

union reg_dpc_ge_config {
	uint32_t raw;
	struct {
		uint32_t ge_strength                     : 9;
		uint32_t ge_combineweight                : 4;
	} bits;
};

union reg_dpc_ge_threshold0 {
	uint32_t raw;
	struct {
		uint32_t ge_thre1                        : 12;
		uint32_t ge_thre2                        : 12;
	} bits;
};

union reg_dpc_ge_threshold1 {
	uint32_t raw;
	struct {
		uint32_t ge_thre3                        : 12;
		uint32_t ge_thre4                        : 12;
	} bits;
};

union reg_dpc_ge_threshold1_0 {
	uint32_t raw;
	struct {
		uint32_t ge_thre11                       : 12;
		uint32_t ge_thre21                       : 12;
	} bits;
};

union reg_dpc_ge_threshold1_1 {
	uint32_t raw;
	struct {
		uint32_t ge_thre31                       : 12;
		uint32_t ge_thre41                       : 12;
	} bits;
};

union reg_dpc_ge_threshold2_0 {
	uint32_t raw;
	struct {
		uint32_t ge_thre12                       : 12;
		uint32_t ge_thre22                       : 12;
	} bits;
};

union reg_dpc_ge_threshold2_1 {
	uint32_t raw;
	struct {
		uint32_t ge_thre32                       : 12;
		uint32_t ge_thre42                       : 12;
	} bits;
};

union reg_dpc_shdw_read_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

union reg_dpc_spc_config {
	uint32_t raw;
	struct {
		uint32_t dpc_mem0_addr                   : 10;
		uint32_t _rsv_10                         : 21;
		uint32_t dpc_mem_sw_mode                 : 1;
	} bits;
};

union reg_dpc_sw_read_0 {
	uint32_t raw;
	struct {
		uint32_t dpc_rdata_r                     : 26;
		uint32_t _rsv_26                         : 5;
		uint32_t dpc_sw_r                        : 1;
	} bits;
};

union reg_dpc_write_mem {
	uint32_t raw;
	struct {
		uint32_t dpc_bp_mem_d                    : 26;
		uint32_t _rsv_26                         : 5;
		uint32_t dpc_bp_mem_w                    : 1;
	} bits;
};

union reg_dpc_write_mem_st_addr {
	uint32_t raw;
	struct {
		uint32_t dpc_bp_mem_st_addr              : 10;
		uint32_t _rsv_10                         : 2;
		uint32_t dpc_bp_mem_st_addr_w            : 1;
		uint32_t bp_num                          : 13;
	} bits;
};

union reg_dpc_auto_cg_en {
	uint32_t raw;
	struct {
		uint32_t hw_auto_cg_en                   : 1;
	} bits;
};

union reg_dpc_sw_read_1 {
	uint32_t raw;
	struct {
		uint32_t dpc_sw_raddr                    : 10;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_drc_frame_done {
	uint32_t raw;
	struct {
		uint32_t frame_start_w1p                 : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t frame_done                      : 1;
	} bits;
};

union reg_isp_drc_enable {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 1;
		uint32_t drc_enable                      : 1;
	} bits;
};

union reg_isp_drc_blk_num {
	uint32_t raw;
	struct {
		uint32_t drc_subimg_width                : 6;
		uint32_t _rsv_6                          : 10;
		uint32_t drc_subimg_height               : 6;
	} bits;
};

union reg_isp_drc_blk_size {
	uint32_t raw;
	struct {
		uint32_t drc_subimg_ratio_hori           : 16;
		uint32_t drc_subimg_ratio_vert           : 16;
	} bits;
};

union reg_isp_drc_blk_div {
	uint32_t raw;
	struct {
		uint32_t drc_subimg_ratio_hori_div       : 16;
		uint32_t drc_subimg_ratio_vert_div       : 16;
	} bits;
};

union reg_isp_drc_lpf_mode {
	uint32_t raw;
	struct {
		uint32_t drc_subimg_lpf_mode             : 2;
	} bits;
};

union reg_isp_drc_hist {
	uint32_t raw;
	struct {
		uint32_t drc_hist_enable                 : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t drc_hist_mode                   : 1;
	} bits;
};

union reg_isp_drc_hist_step {
	uint32_t raw;
	struct {
		uint32_t drc_hist_step_x                 : 5;
		uint32_t _rsv_5                          : 11;
		uint32_t drc_hist_step_y                 : 5;
	} bits;
};

union reg_isp_drc_intensit_mode {
	uint32_t raw;
	struct {
		uint32_t drc_intensity_mode              : 2;
		uint32_t _rsv_2                          : 14;
		uint32_t drc_tone_curve_enable           : 1;
	} bits;
};

union reg_isp_drc_hori_lpf_th12 {
	uint32_t raw;
	struct {
		uint32_t drc_hori_lpf_th1                : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_hori_lpf_th2                : 12;
	} bits;
};

union reg_isp_drc_hori_lpf_th34 {
	uint32_t raw;
	struct {
		uint32_t drc_hori_lpf_th3                : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_hori_lpf_th4                : 12;
	} bits;
};

union reg_isp_drc_hori_lpf_th56 {
	uint32_t raw;
	struct {
		uint32_t drc_hori_lpf_th5                : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_hori_lpf_th6                : 12;
	} bits;
};

union reg_isp_drc_q_coeff12 {
	uint32_t raw;
	struct {
		uint32_t drc_q_coeff1                    : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t drc_q_coeff2                    : 13;
	} bits;
};

union reg_isp_drc_q_coeff34 {
	uint32_t raw;
	struct {
		uint32_t drc_q_coeff3                    : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t drc_q_coeff4                    : 13;
	} bits;
};

union reg_isp_drc_b2q_th {
	uint32_t raw;
	struct {
		uint32_t drc_b2q_th                      : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_enable            : 1;
	} bits;
};

union reg_isp_drc_fbc {
	uint32_t raw;
	struct {
		uint32_t drc_fbc_enable                  : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t drc_fbc_raito                   : 8;
	} bits;
};

union reg_isp_drc_anti_halo {
	uint32_t raw;
	struct {
		uint32_t drc_anti_halo_mode              : 2;
		uint32_t _rsv_2                          : 6;
		uint32_t drc_anti_halo_ratio             : 8;
		uint32_t drc_anti_halo_slope             : 8;
	} bits;
};

union reg_isp_drc_strength {
	uint32_t raw;
	struct {
		uint32_t drc_strength                    : 8;
	} bits;
};

union reg_isp_drc_ltm_en {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_enable         : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t drc_gain_ctrl_enable            : 1;
	} bits;
};

union reg_isp_drc_gain {
	uint32_t raw;
	struct {
		uint32_t drc_max_gain                    : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t drc_min_gain                    : 8;
	} bits;
};

union reg_isp_drc_sat_th {
	uint32_t raw;
	struct {
		uint32_t drc_gain_prot_str               : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t drc_sat_th                      : 12;
	} bits;
};

union reg_isp_drc_sat_delta {
	uint32_t raw;
	struct {
		uint32_t drc_sat_delta                   : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t drc_sat_wt_low                  : 8;
	} bits;
};

union reg_isp_drc_yv_bld_lut0_4 {
	uint32_t raw;
	struct {
		uint32_t drc_yv_bld_lut_0                : 6;
		uint32_t drc_yv_bld_lut_1                : 6;
		uint32_t drc_yv_bld_lut_2                : 6;
		uint32_t drc_yv_bld_lut_3                : 6;
		uint32_t drc_yv_bld_lut_4                : 6;
	} bits;
};

union reg_isp_drc_yv_bld_lut5_8 {
	uint32_t raw;
	struct {
		uint32_t drc_yv_bld_lut_5                : 6;
		uint32_t drc_yv_bld_lut_6                : 6;
		uint32_t drc_yv_bld_lut_7                : 6;
		uint32_t drc_yv_bld_lut_8                : 6;
	} bits;
};

union reg_isp_drc_input_bld_lut_0_3 {
	uint32_t raw;
	struct {
		uint32_t drc_input_bld_lut_0             : 8;
		uint32_t drc_input_bld_lut_1             : 8;
		uint32_t drc_input_bld_lut_2             : 8;
		uint32_t drc_input_bld_lut_3             : 8;
	} bits;
};

union reg_isp_drc_input_bld_lut_4_7 {
	uint32_t raw;
	struct {
		uint32_t drc_input_bld_lut_4             : 8;
		uint32_t drc_input_bld_lut_5             : 8;
		uint32_t drc_input_bld_lut_6             : 8;
		uint32_t drc_input_bld_lut_7             : 8;
	} bits;
};

union reg_isp_drc_input_bld_lut_8_11 {
	uint32_t raw;
	struct {
		uint32_t drc_input_bld_lut_8             : 8;
		uint32_t drc_input_bld_lut_9             : 8;
		uint32_t drc_input_bld_lut_10            : 8;
		uint32_t drc_input_bld_lut_11            : 8;
	} bits;
};

union reg_isp_drc_input_bld_lut_12_15 {
	uint32_t raw;
	struct {
		uint32_t drc_input_bld_lut_12            : 8;
		uint32_t drc_input_bld_lut_13            : 8;
		uint32_t drc_input_bld_lut_14            : 8;
		uint32_t drc_input_bld_lut_15            : 8;
	} bits;
};

union reg_isp_drc_input_bld_lut_16 {
	uint32_t raw;
	struct {
		uint32_t drc_input_bld_lut_16            : 8;
	} bits;
};

union reg_isp_drc_gtm_l_lut_0_1 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_0          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_1          : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_2_3 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_2          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_3          : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_4_5 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_4          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_5          : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_6_7 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_6          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_7          : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_8_9 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_8          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_9          : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_10_11 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_10         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_11         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_12_13 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_12         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_13         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_14_15 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_14         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_15         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_16_17 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_16         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_17         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_18_19 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_18         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_19         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_20_21 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_20         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_21         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_22_23 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_22         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_23         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_24_25 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_24         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_25         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_26_27 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_26         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_27         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_28_29 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_28         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_29         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_30_31 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_30         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_31         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_32_33 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_32         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_33         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_34_35 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_34         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_35         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_36_37 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_36         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_37         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_38_39 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_38         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_39         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_40_41 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_40         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_41         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_42_43 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_42         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_43         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_44_45 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_44         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_45         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_46_47 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_46         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_47         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_48_49 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_48         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_49         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_50_51 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_50         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_51         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_52_53 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_52         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_53         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_54_55 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_54         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_55         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_56_57 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_56         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_57         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_58_59 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_58         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_59         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_60_61 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_60         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_61         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_62_63 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_62         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_l_lut_63         : 12;
	} bits;
};

union reg_isp_drc_gtm_l_lut_64 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_l_lut_64         : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_0_1 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_0          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_r_lut_1          : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_2_3 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_2          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_r_lut_3          : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_4_5 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_4          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_r_lut_5          : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_6_7 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_6          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_r_lut_7          : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_8_9 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_8          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_r_lut_9          : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_10_11 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_10         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_r_lut_11         : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_12_13 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_12         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_r_lut_13         : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_14_15 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_14         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_tone_curve_r_lut_15         : 12;
	} bits;
};

union reg_isp_drc_gtm_r_lut_16 {
	uint32_t raw;
	struct {
		uint32_t drc_tone_curve_r_lut_16         : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_0_1 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_0             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_lut_1             : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_2_3 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_2             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_lut_3             : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_4_5 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_4             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_lut_5             : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_6_7 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_6             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_lut_7             : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_8_9 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_8             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_lut_9             : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_10_11 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_10            : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_lut_11            : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_12_13 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_12            : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_lut_13            : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_14_15 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_14            : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_luma_prot_lut_15            : 12;
	} bits;
};

union reg_isp_drc_luma_prot_lut_16 {
	uint32_t raw;
	struct {
		uint32_t drc_luma_prot_lut_16            : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_0_1 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_0        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_1        : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_2_3 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_2        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_3        : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_4_5 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_4        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_5        : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_6_7 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_6        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_7        : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_8_9 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_8        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_9        : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_10_11 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_10       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_11       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_12_13 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_12       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_13       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_14_15 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_14       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_15       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_16_17 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_16       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_17       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_18_19 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_18       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_19       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_20_21 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_20       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_21       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_22_23 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_22       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_23       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_24_25 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_24       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_25       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_26_27 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_26       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_27       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_28_29 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_28       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_29       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_30_31 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_30       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_31       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_32_33 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_32       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_33       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_34_35 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_34       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_35       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_36_37 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_36       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_37       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_38_39 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_38       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_39       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_40_41 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_40       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_41       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_42_43 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_42       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_43       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_44_45 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_44       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_45       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_46_47 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_46       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_47       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_48_49 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_48       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_49       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_50_51 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_50       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_51       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_52_53 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_52       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_53       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_54_55 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_54       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_55       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_56_57 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_56       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_57       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_58_59 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_58       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_59       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_60_61 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_60       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_61       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_62_63 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_62       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_l_lut_63       : 12;
	} bits;
};

union reg_isp_drc_ltm_l_lut_64 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_l_lut_64       : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_0_1 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_0        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_r_lut_1        : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_2_3 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_2        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_r_lut_3        : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_4_5 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_4        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_r_lut_5        : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_6_7 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_6        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_r_lut_7        : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_8_9 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_8        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_r_lut_9        : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_10_11 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_10       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_r_lut_11       : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_12_13 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_12       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_r_lut_13       : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_14_15 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_14       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t drc_outbld_curve_r_lut_15       : 12;
	} bits;
};

union reg_isp_drc_ltm_r_lut_16 {
	uint32_t raw;
	struct {
		uint32_t drc_outbld_curve_r_lut_16       : 12;
	} bits;
};

union reg_isp_drc_img_size {
	uint32_t raw;
	struct {
		uint32_t img_width_m1                    : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t img_height_m1                   : 14;
	} bits;
};

union reg_isp_drc_shdw_sel {
	uint32_t raw;
	struct {
		uint32_t drc_shdw_sel                    : 1;
	} bits;
};

union reg_drc_pipectrl_cg {
	uint32_t raw;
	struct {
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t guard_cnt                       : 8;
		uint32_t bypass                          : 1;
	} bits;
};

union reg_isp_drc_bayerid {
	uint32_t raw;
	struct {
		uint32_t pre_img_bayerid                 : 2;
	} bits;
};

union reg_isp_drc_debug {
	uint32_t raw;
	struct {
		uint32_t drc_post_in_dbg_sel             : 4;
		uint32_t drc_post_ltm_dbg_sel            : 1;
		uint32_t drc_post_bld_dbg_sel            : 1;
		uint32_t drc_post_gain_dbg_sel           : 2;
		uint32_t drc_post_out_dbg_sel            : 3;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_ee_ext_ee_ext_reg0 {
	uint32_t raw;
	struct {
		uint32_t ee_enable                       : 1;
		uint32_t ee_debug_mode                   : 4;
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t ee_ch_selection                 : 1;
		uint32_t ee_gamma_selection              : 1;
		uint32_t ee_gamma_lut_u10_0              : 10;
		uint32_t ee_gamma_lut_u10_1              : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_0 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_2              : 10;
		uint32_t ee_gamma_lut_u10_3              : 10;
		uint32_t ee_gamma_lut_u10_4              : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_1 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_5              : 10;
		uint32_t ee_gamma_lut_u10_6              : 10;
		uint32_t ee_gamma_lut_u10_7              : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_2 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_8              : 10;
		uint32_t ee_gamma_lut_u10_9              : 10;
		uint32_t ee_gamma_lut_u10_10             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_3 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_11             : 10;
		uint32_t ee_gamma_lut_u10_12             : 10;
		uint32_t ee_gamma_lut_u10_13             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_4 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_14             : 10;
		uint32_t ee_gamma_lut_u10_15             : 10;
		uint32_t ee_gamma_lut_u10_16             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_5 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_17             : 10;
		uint32_t ee_gamma_lut_u10_18             : 10;
		uint32_t ee_gamma_lut_u10_19             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_6 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_20             : 10;
		uint32_t ee_gamma_lut_u10_21             : 10;
		uint32_t ee_gamma_lut_u10_22             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_7 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_23             : 10;
		uint32_t ee_gamma_lut_u10_24             : 10;
		uint32_t ee_gamma_lut_u10_25             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_8 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_26             : 10;
		uint32_t ee_gamma_lut_u10_27             : 10;
		uint32_t ee_gamma_lut_u10_28             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_9 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_29             : 10;
		uint32_t ee_gamma_lut_u10_30             : 10;
		uint32_t ee_gamma_lut_u10_31             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_10 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_32             : 10;
		uint32_t ee_gamma_lut_u10_33             : 10;
		uint32_t ee_gamma_lut_u10_34             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_11 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_35             : 10;
		uint32_t ee_gamma_lut_u10_36             : 10;
		uint32_t ee_gamma_lut_u10_37             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_12 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_38             : 10;
		uint32_t ee_gamma_lut_u10_39             : 10;
		uint32_t ee_gamma_lut_u10_40             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_13 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_41             : 10;
		uint32_t ee_gamma_lut_u10_42             : 10;
		uint32_t ee_gamma_lut_u10_43             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_14 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_44             : 10;
		uint32_t ee_gamma_lut_u10_45             : 10;
		uint32_t ee_gamma_lut_u10_46             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_15 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_47             : 10;
		uint32_t ee_gamma_lut_u10_48             : 10;
		uint32_t ee_gamma_lut_u10_49             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_16 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_50             : 10;
		uint32_t ee_gamma_lut_u10_51             : 10;
		uint32_t ee_gamma_lut_u10_52             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_17 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_53             : 10;
		uint32_t ee_gamma_lut_u10_54             : 10;
		uint32_t ee_gamma_lut_u10_55             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_18 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_56             : 10;
		uint32_t ee_gamma_lut_u10_57             : 10;
		uint32_t ee_gamma_lut_u10_58             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_19 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_59             : 10;
		uint32_t ee_gamma_lut_u10_60             : 10;
		uint32_t ee_gamma_lut_u10_61             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_gamma_20 {
	uint32_t raw;
	struct {
		uint32_t ee_gamma_lut_u10_62             : 10;
		uint32_t ee_gamma_lut_u10_63             : 10;
		uint32_t ee_gamma_lut_u10_64             : 10;
	} bits;
};

union reg_ee_ext_ee_ext_e5c_0 {
	uint32_t raw;
	struct {
		uint32_t ee_e5c_coef_s11_0               : 11;
		uint32_t ee_e5c_coef_s11_1               : 11;
	} bits;
};

union reg_ee_ext_ee_ext_e5c_1 {
	uint32_t raw;
	struct {
		uint32_t ee_e5c_coef_s11_2               : 11;
		uint32_t ee_e5c_coef_s11_3               : 11;
	} bits;
};

union reg_ee_ext_ee_ext_e5c_2 {
	uint32_t raw;
	struct {
		uint32_t ee_e5c_coef_s11_4               : 11;
		uint32_t ee_e5c_coef_s11_5               : 11;
		uint32_t ee_e5c_enh                      : 10;
	} bits;
};

union reg_ee_ext_ee_ext_e5a_0 {
	uint32_t raw;
	struct {
		uint32_t ee_e5a_coef_s11_0               : 11;
		uint32_t ee_e5a_coef_s11_1               : 11;
	} bits;
};

union reg_ee_ext_ee_ext_e5a_1 {
	uint32_t raw;
	struct {
		uint32_t ee_e5a_coef_s11_2               : 11;
		uint32_t ee_e5a_coef_s11_3               : 11;
	} bits;
};

union reg_ee_ext_ee_ext_e5a_2 {
	uint32_t raw;
	struct {
		uint32_t ee_e5a_coef_s11_4               : 11;
		uint32_t ee_e5a_coef_s11_5               : 11;
		uint32_t ee_e5a_enh                      : 10;
	} bits;
};

union reg_ee_ext_ee_ext_e5b_0 {
	uint32_t raw;
	struct {
		uint32_t ee_e5b_coef_s11_0               : 11;
		uint32_t ee_e5b_coef_s11_1               : 11;
	} bits;
};

union reg_ee_ext_ee_ext_e5b_1 {
	uint32_t raw;
	struct {
		uint32_t ee_e5b_coef_s11_2               : 11;
		uint32_t ee_e5b_coef_s11_3               : 11;
	} bits;
};

union reg_ee_ext_ee_ext_e5b_2 {
	uint32_t raw;
	struct {
		uint32_t ee_e5b_coef_s11_4               : 11;
		uint32_t ee_e5b_coef_s11_5               : 11;
		uint32_t ee_e5b_enh                      : 10;
	} bits;
};

union reg_ee_ext_ee_ext_e7_0 {
	uint32_t raw;
	struct {
		uint32_t ee_e5b2_coef_s11_0              : 11;
		uint32_t ee_e5b2_coef_s11_1              : 11;
	} bits;
};

union reg_ee_ext_ee_ext_e7_1 {
	uint32_t raw;
	struct {
		uint32_t ee_e5b2_coef_s11_2              : 11;
		uint32_t ee_e5b2_coef_s11_3              : 11;
	} bits;
};

union reg_ee_ext_ee_ext_e7_2 {
	uint32_t raw;
	struct {
		uint32_t ee_e5b2_coef_s11_4              : 11;
		uint32_t ee_e5b2_coef_s11_5              : 11;
		uint32_t ee_e5b2_enh                     : 10;
	} bits;
};

union reg_ee_ext_ee_ext_norm {
	uint32_t raw;
	struct {
		uint32_t ee_e5c_norm                     : 4;
		uint32_t ee_e5a_norm                     : 4;
		uint32_t ee_e5b_norm                     : 4;
		uint32_t ee_e5b2_norm                    : 4;
		uint32_t ee_eng_norm                     : 5;
		uint32_t ee_con_norm                     : 5;
		uint32_t ee_con_eng_wt                   : 4;
	} bits;
};

union reg_ee_ext_ee_ext_luma_blend {
	uint32_t raw;
	struct {
		uint32_t ee_blend_th                     : 8;
		uint32_t ee_blend_slope                  : 6;
		uint32_t ee_blend_w1                     : 8;
		uint32_t ee_blend_w2                     : 8;
	} bits;
};

union reg_ee_ext_ee_ext_region_0 {
	uint32_t raw;
	struct {
		uint32_t ee_th_flat                      : 10;
		uint32_t ee_th_edge                      : 10;
		uint32_t ee_th_flat_hld                  : 10;
	} bits;
};

union reg_ee_ext_ee_ext_region_1 {
	uint32_t raw;
	struct {
		uint32_t ee_th_edge_hld                  : 10;
		uint32_t ee_th_lum_hld                   : 10;
	} bits;
};

union reg_ee_ext_ee_ext_region_2 {
	uint32_t raw;
	struct {
		uint32_t ee_thin_wt_for_flat             : 5;
		uint32_t ee_thin_wt_for_edge             : 5;
		uint32_t ee_thin_wt_hld_flat             : 5;
		uint32_t ee_thin_wt_hld_edge             : 5;
		uint32_t ee_region_str_enable            : 1;
	} bits;
};

union reg_ee_ext_ee_ext_region_3 {
	uint32_t raw;
	struct {
		uint32_t ee_str_flat_region              : 8;
		uint32_t ee_str_edge_region              : 8;
		uint32_t ee_thin_enh                     : 8;
		uint32_t ee_robust_enh                   : 8;
	} bits;
};

union reg_ee_ext_ee_ext_region_4 {
	uint32_t raw;
	struct {
		uint32_t ee_slope_con_eng_hld            : 16;
		uint32_t ee_slope_con_eng                : 16;
	} bits;
};

union reg_ee_ext_ee_ext_region_5 {
	uint32_t raw;
	struct {
		uint32_t ee_slope_flat_region            : 16;
		uint32_t ee_slope_edge_region            : 16;
	} bits;
};

union reg_ee_ext_ee_ext_region_6 {
	uint32_t raw;
	struct {
		uint32_t ee_thin_wt_uhf_hf               : 4;
		uint32_t ee_robust_wt_hf_shf             : 4;
		uint32_t ee_thin_wt_against_shf          : 4;
		uint32_t ee_robust_wt_against_mf         : 4;
		uint32_t ee_thin_hld_wt_uhf_hf           : 4;
		uint32_t ee_robust_hld_wt_hf_shf         : 4;
		uint32_t ee_thin_hld_wt_against_shf      : 4;
		uint32_t ee_robust_hld_wt_against_mf     : 4;
	} bits;
};

union reg_ee_ext_shadow_rd_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_ee_00 {
	uint32_t raw;
	struct {
		uint32_t ee_enable                       : 1;
		uint32_t ee_shadow_sel                   : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t ee_debug_mode                   : 4;
		uint32_t ee_coring_th                    : 8;
		uint32_t ee_chroma_adptctrl_en           : 1;
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t cbcr_swap_en                    : 1;
	} bits;
};

union reg_isp_ee_04 {
	uint32_t raw;
	struct {
		uint32_t ee_overshoot_clip_ratio         : 8;
		uint32_t ee_undershoot_clip_ratio        : 8;
	} bits;
};

union reg_isp_ee_0c {
	uint32_t raw;
	struct {
		uint32_t ee_delta_wt_src_opt             : 1;
		uint32_t ee_delta_wt_coring_th           : 8;
		uint32_t ee_delta_wt_gain                : 8;
		uint32_t ee_noise_level                  : 8;
	} bits;
};

union reg_isp_ee_10 {
	uint32_t raw;
	struct {
		uint32_t ee_blend_degamma                : 8;
		uint32_t ee_sharp_str                    : 8;
	} bits;
};

union reg_isp_ee_a4 {
	uint32_t raw;
	struct {
		uint32_t ee_noise_lut_u8_0               : 8;
		uint32_t ee_noise_lut_u8_1               : 8;
		uint32_t ee_noise_lut_u8_2               : 8;
		uint32_t ee_noise_lut_u8_3               : 8;
	} bits;
};

union reg_isp_ee_a8 {
	uint32_t raw;
	struct {
		uint32_t ee_noise_lut_u8_4               : 8;
		uint32_t ee_noise_lut_u8_5               : 8;
		uint32_t ee_noise_lut_u8_6               : 8;
		uint32_t ee_noise_lut_u8_7               : 8;
	} bits;
};

union reg_isp_ee_ac {
	uint32_t raw;
	struct {
		uint32_t ee_noise_lut_u8_8               : 8;
		uint32_t ee_noise_lut_u8_9               : 8;
		uint32_t ee_noise_lut_u8_10              : 8;
		uint32_t ee_noise_lut_u8_11              : 8;
	} bits;
};

union reg_isp_ee_b0 {
	uint32_t raw;
	struct {
		uint32_t ee_noise_lut_u8_12              : 8;
		uint32_t ee_noise_lut_u8_13              : 8;
		uint32_t ee_noise_lut_u8_14              : 8;
		uint32_t ee_noise_lut_u8_15              : 8;
	} bits;
};

union reg_isp_ee_b4 {
	uint32_t raw;
	struct {
		uint32_t ee_noise_lut_u8_16              : 8;
	} bits;
};

union reg_isp_ee_b8 {
	uint32_t raw;
	struct {
		uint32_t ee_con_eng_wt                   : 4;
		uint32_t ee_flat_th                      : 11;
		uint32_t ee_edge_th                      : 11;
	} bits;
};

union reg_isp_ee_bc {
	uint32_t raw;
	struct {
		uint32_t ee_con_eng_slope                : 12;
		uint32_t ee_flat_region_str              : 8;
		uint32_t ee_edge_region_str              : 8;
	} bits;
};

union reg_isp_ee_c0 {
	uint32_t raw;
	struct {
		uint32_t ee_motion_enable                : 1;
		uint32_t ee_motion_delta_wt_str          : 8;
	} bits;
};

union reg_isp_ee_c4 {
	uint32_t raw;
	struct {
		uint32_t ee_static_delta_wt_str          : 8;
		uint32_t ee_trans_delta_wt_str           : 8;
		uint32_t ee_filter_size_opt              : 1;
	} bits;
};

union reg_isp_ee_c8 {
	uint32_t raw;
	struct {
		uint32_t ee_delta_wt_gain_lut_u11_0      : 11;
		uint32_t ee_delta_wt_gain_lut_u11_1      : 11;
	} bits;
};

union reg_isp_ee_hcc {
	uint32_t raw;
	struct {
		uint32_t ee_delta_wt_gain_lut_u11_2      : 11;
		uint32_t ee_delta_wt_gain_lut_u11_3      : 11;
	} bits;
};

union reg_isp_ee_hd0 {
	uint32_t raw;
	struct {
		uint32_t ee_delta_wt_gain_lut_u11_4      : 11;
		uint32_t ee_delta_wt_gain_lut_u11_5      : 11;
	} bits;
};

union reg_isp_ee_hd4 {
	uint32_t raw;
	struct {
		uint32_t ee_delta_wt_gain_lut_u11_6      : 11;
		uint32_t ee_delta_wt_gain_lut_u11_7      : 11;
	} bits;
};

union reg_isp_ee_hd8 {
	uint32_t raw;
	struct {
		uint32_t ee_delta_wt_gain_lut_u11_8      : 11;
	} bits;
};

union reg_isp_ee_178 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_00       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ee_chroma_adptctrl_lut_01       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ee_chroma_adptctrl_lut_02       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ee_chroma_adptctrl_lut_03       : 6;
	} bits;
};

union reg_isp_ee_17c {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_04       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ee_chroma_adptctrl_lut_05       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ee_chroma_adptctrl_lut_06       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ee_chroma_adptctrl_lut_07       : 6;
	} bits;
};

union reg_isp_ee_180 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_08       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ee_chroma_adptctrl_lut_09       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ee_chroma_adptctrl_lut_10       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ee_chroma_adptctrl_lut_11       : 6;
	} bits;
};

union reg_isp_ee_184 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_12       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ee_chroma_adptctrl_lut_13       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ee_chroma_adptctrl_lut_14       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ee_chroma_adptctrl_lut_15       : 6;
	} bits;
};

union reg_isp_ee_188 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_16       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ee_chroma_adptctrl_lut_17       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ee_chroma_adptctrl_lut_18       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ee_chroma_adptctrl_lut_19       : 6;
	} bits;
};

union reg_isp_ee_18c {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_20       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ee_chroma_adptctrl_lut_21       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ee_chroma_adptctrl_lut_22       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ee_chroma_adptctrl_lut_23       : 6;
	} bits;
};

union reg_isp_ee_190 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_24       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ee_chroma_adptctrl_lut_25       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ee_chroma_adptctrl_lut_26       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ee_chroma_adptctrl_lut_27       : 6;
	} bits;
};

union reg_isp_ee_194 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_28       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ee_chroma_adptctrl_lut_29       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ee_chroma_adptctrl_lut_30       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ee_chroma_adptctrl_lut_31       : 6;
	} bits;
};

union reg_isp_ee_198 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_32       : 6;
	} bits;
};

union reg_isp_ee_19c {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_amp_lut_in_0          : 8;
		uint32_t ee_chroma_amp_lut_in_1          : 8;
		uint32_t ee_chroma_amp_lut_in_2          : 8;
		uint32_t ee_chroma_amp_lut_in_3          : 8;
	} bits;
};

union reg_isp_ee_1a0 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_amp_lut_out_0         : 8;
		uint32_t ee_chroma_amp_lut_out_1         : 8;
		uint32_t ee_chroma_amp_lut_out_2         : 8;
		uint32_t ee_chroma_amp_lut_out_3         : 8;
	} bits;
};

union reg_isp_ee_1a4 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_amp_lut_slope_0       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t ee_chroma_amp_lut_slope_1       : 12;
	} bits;
};

union reg_isp_ee_1a8 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_amp_lut_slope_2       : 12;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_ee_add_back_ee_add_b_reg0 {
	uint32_t raw;
	struct {
		uint32_t ee_enable                       : 1;
		uint32_t ee_debug_mode                   : 4;
		uint32_t hw_auto_cg_en                   : 1;
	} bits;
};

union reg_ee_add_back_shadow_rd_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_ee_add_ee_add_reg0 {
	uint32_t raw;
	struct {
		uint32_t ee_enable                       : 1;
		uint32_t ee_debug_mode                   : 4;
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t ee_ext_bypass_en                : 1;
	} bits;
};

union reg_ee_add_ee_add_overshoot_0 {
	uint32_t raw;
	struct {
		uint32_t ee_th_overshoot                 : 8;
		uint32_t ee_th_undershoot_lum            : 8;
		uint32_t ee_th_undershoot_eng            : 8;
		uint32_t ee_th_undershoot                : 8;
	} bits;
};

union reg_ee_add_ee_add_overshoot_1 {
	uint32_t raw;
	struct {
		uint32_t ee_over_under_shoot_enable      : 1;
		uint32_t ee_wt_overshoot                 : 9;
		uint32_t ee_wt_undershoot                : 9;
	} bits;
};

union reg_ee_add_ee_add_overshoot_2 {
	uint32_t raw;
	struct {
		uint32_t ee_slope_overshoot              : 15;
		uint32_t ee_slope_undershoot             : 15;
	} bits;
};

union reg_ee_add_ee_add_overshoot_3 {
	uint32_t raw;
	struct {
		uint32_t ee_slope_undershoot_lum         : 15;
		uint32_t ee_slope_undershoot_eng         : 15;
	} bits;
};

union reg_ee_add_ee_add_overshoot_4 {
	uint32_t raw;
	struct {
		uint32_t ee_ushoot_clampwt_lum           : 8;
		uint32_t ee_ushoot_clampwt_eng           : 8;
		uint32_t ee_ushoot_str_lum_eng           : 8;
		uint32_t ee_ushoot_norm_lum_eng          : 5;
	} bits;
};

union reg_ee_add_ee_add_refine_0 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_lum_th_low         : 10;
		uint32_t ee_refine_by_lum_th_high        : 10;
		uint32_t ee_refine_by_lum_step_low       : 3;
		uint32_t ee_refine_by_lum_step_high      : 3;
	} bits;
};

union reg_ee_add_ee_add_refine_1 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_edge_type_sel         : 2;
		uint32_t ee_refine_by_edge_th_low        : 10;
		uint32_t ee_refine_by_edge_th_high       : 10;
		uint32_t ee_refine_by_edge_step_low      : 3;
		uint32_t ee_refine_by_edge_step_high     : 3;
	} bits;
};

union reg_ee_add_ee_add_refine_2 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_lum_lut_low_u8_0   : 8;
		uint32_t ee_refine_by_lum_lut_low_u8_1   : 8;
		uint32_t ee_refine_by_lum_lut_low_u8_2   : 8;
		uint32_t ee_refine_by_lum_lut_low_u8_3   : 8;
	} bits;
};

union reg_ee_add_ee_add_refine_3 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_lum_lut_low_u8_4   : 8;
		uint32_t ee_refine_by_lum_lut_low_u8_5   : 8;
		uint32_t ee_refine_by_lum_lut_low_u8_6   : 8;
		uint32_t ee_refine_by_lum_lut_low_u8_7   : 8;
	} bits;
};

union reg_ee_add_ee_add_refine_4 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_lum_lut_high_u8_0  : 8;
		uint32_t ee_refine_by_lum_lut_high_u8_1  : 8;
		uint32_t ee_refine_by_lum_lut_high_u8_2  : 8;
		uint32_t ee_refine_by_lum_lut_high_u8_3  : 8;
	} bits;
};

union reg_ee_add_ee_add_refine_5 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_lum_lut_high_u8_4  : 8;
		uint32_t ee_refine_by_lum_lut_high_u8_5  : 8;
		uint32_t ee_refine_by_lum_lut_high_u8_6  : 8;
		uint32_t ee_refine_by_lum_lut_high_u8_7  : 8;
	} bits;
};

union reg_ee_add_ee_add_refine_6 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_edge_lut_low_u8_0  : 8;
		uint32_t ee_refine_by_edge_lut_low_u8_1  : 8;
		uint32_t ee_refine_by_edge_lut_low_u8_2  : 8;
		uint32_t ee_refine_by_edge_lut_low_u8_3  : 8;
	} bits;
};

union reg_ee_add_ee_add_refine_7 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_edge_lut_low_u8_4  : 8;
		uint32_t ee_refine_by_edge_lut_low_u8_5  : 8;
		uint32_t ee_refine_by_edge_lut_low_u8_6  : 8;
		uint32_t ee_refine_by_edge_lut_low_u8_7  : 8;
	} bits;
};

union reg_ee_add_ee_add_refine_8 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_edge_lut_high_u8_0 : 8;
		uint32_t ee_refine_by_edge_lut_high_u8_1 : 8;
		uint32_t ee_refine_by_edge_lut_high_u8_2 : 8;
		uint32_t ee_refine_by_edge_lut_high_u8_3 : 8;
	} bits;
};

union reg_ee_add_ee_add_refine_9 {
	uint32_t raw;
	struct {
		uint32_t ee_refine_by_edge_lut_high_u8_4 : 8;
		uint32_t ee_refine_by_edge_lut_high_u8_5 : 8;
		uint32_t ee_refine_by_edge_lut_high_u8_6 : 8;
		uint32_t ee_refine_by_edge_lut_high_u8_7 : 8;
	} bits;
};

union reg_ee_add_ee_add_chroma_0 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_00       : 6;
		uint32_t ee_chroma_adptctrl_lut_01       : 6;
		uint32_t ee_chroma_adptctrl_lut_02       : 6;
		uint32_t ee_chroma_adptctrl_lut_03       : 6;
		uint32_t ee_chroma_adptctrl_lut_04       : 6;
	} bits;
};

union reg_ee_add_ee_add_chroma_1 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_05       : 6;
		uint32_t ee_chroma_adptctrl_lut_06       : 6;
		uint32_t ee_chroma_adptctrl_lut_07       : 6;
		uint32_t ee_chroma_adptctrl_lut_08       : 6;
		uint32_t ee_chroma_adptctrl_lut_09       : 6;
	} bits;
};

union reg_ee_add_ee_add_chroma_2 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_10       : 6;
		uint32_t ee_chroma_adptctrl_lut_11       : 6;
		uint32_t ee_chroma_adptctrl_lut_12       : 6;
		uint32_t ee_chroma_adptctrl_lut_13       : 6;
		uint32_t ee_chroma_adptctrl_lut_14       : 6;
	} bits;
};

union reg_ee_add_ee_add_chroma_3 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_15       : 6;
		uint32_t ee_chroma_adptctrl_lut_16       : 6;
		uint32_t ee_chroma_adptctrl_lut_17       : 6;
		uint32_t ee_chroma_adptctrl_lut_18       : 6;
		uint32_t ee_chroma_adptctrl_lut_19       : 6;
	} bits;
};

union reg_ee_add_ee_add_chroma_4 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_20       : 6;
		uint32_t ee_chroma_adptctrl_lut_21       : 6;
		uint32_t ee_chroma_adptctrl_lut_22       : 6;
		uint32_t ee_chroma_adptctrl_lut_23       : 6;
		uint32_t ee_chroma_adptctrl_lut_24       : 6;
	} bits;
};

union reg_ee_add_ee_add_chroma_5 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_25       : 6;
		uint32_t ee_chroma_adptctrl_lut_26       : 6;
		uint32_t ee_chroma_adptctrl_lut_27       : 6;
		uint32_t ee_chroma_adptctrl_lut_28       : 6;
		uint32_t ee_chroma_adptctrl_lut_29       : 6;
	} bits;
};

union reg_ee_add_ee_add_chroma_6 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_adptctrl_lut_30       : 6;
		uint32_t ee_chroma_adptctrl_lut_31       : 6;
		uint32_t ee_chroma_adptctrl_en           : 1;
		uint32_t ee_chroma_amp_lut_slope_0       : 12;
	} bits;
};

union reg_ee_add_ee_add_chroma_7 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_amp_lut_slope_1       : 12;
		uint32_t ee_chroma_amp_lut_slope_2       : 12;
	} bits;
};

union reg_ee_add_ee_add_chroma_8 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_amp_lut_in_0          : 8;
		uint32_t ee_chroma_amp_lut_in_1          : 8;
		uint32_t ee_chroma_amp_lut_in_2          : 8;
		uint32_t ee_chroma_amp_lut_in_3          : 8;
	} bits;
};

union reg_ee_add_ee_add_chroma_9 {
	uint32_t raw;
	struct {
		uint32_t ee_chroma_amp_lut_out_0         : 8;
		uint32_t ee_chroma_amp_lut_out_1         : 8;
		uint32_t ee_chroma_amp_lut_out_2         : 8;
		uint32_t ee_chroma_amp_lut_out_3         : 8;
	} bits;
};

union reg_ee_add_shadow_rd_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_fbcd_00 {
	uint32_t raw;
	struct {
		uint32_t fbcd_en                         : 1;
		uint32_t ff_drop                         : 1;
		uint32_t fbcd_auto                       : 1;
		uint32_t force_cke                       : 1;
	} bits;
};

union reg_fbcd_0c {
	uint32_t raw;
	struct {
		uint32_t y_bit_stream_size               : 32;
	} bits;
};

union reg_fbcd_10 {
	uint32_t raw;
	struct {
		uint32_t c_bit_stream_size               : 32;
	} bits;
};

union reg_fbcd_14 {
	uint32_t raw;
	struct {
		uint32_t debug_sel                       : 4;
	} bits;
};

union reg_fbcd_18 {
	uint32_t raw;
	struct {
		uint32_t debug_bus                       : 32;
	} bits;
};

union reg_fbcd_20 {
	uint32_t raw;
	struct {
		uint32_t dummy                           : 8;
		uint32_t shd_rd                          : 1;
	} bits;
};

union reg_fbcd_24 {
	uint32_t raw;
	struct {
		uint32_t y_lossless                      : 1;
		uint32_t y_base_qdpcm_q                  : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t y_base_pcm_bd_minus2            : 3;
		uint32_t _rsv_7                          : 1;
		uint32_t y_default_gr_k                  : 3;
	} bits;
};

union reg_fbcd_28 {
	uint32_t raw;
	struct {
		uint32_t c_lossless                      : 1;
		uint32_t c_base_qdpcm_q                  : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t c_base_pcm_bd_minus2            : 3;
		uint32_t _rsv_7                          : 1;
		uint32_t c_default_gr_k                  : 3;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_fbce_00 {
	uint32_t raw;
	struct {
		uint32_t fbce_en                         : 1;
		uint32_t shd_rd                          : 1;
		uint32_t force_cke                       : 1;
		uint32_t _rsv_3                          : 5;
		uint32_t debug                           : 16;
		uint32_t dummy                           : 8;
	} bits;
};

union reg_fbce_10 {
	uint32_t raw;
	struct {
		uint32_t y_lossless                      : 1;
		uint32_t y_base_qdpcm_q                  : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t y_base_pcm_bd_minus2            : 3;
		uint32_t _rsv_7                          : 1;
		uint32_t y_default_gr_k                  : 3;
		uint32_t _rsv_11                         : 1;
		uint32_t y_cplx_shift                    : 3;
		uint32_t _rsv_15                         : 1;
		uint32_t y_pen_pos_shift                 : 3;
	} bits;
};

union reg_fbce_14 {
	uint32_t raw;
	struct {
		uint32_t y_min_cu_bit                    : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t y_max_cu_bit                    : 7;
	} bits;
};

union reg_fbce_18 {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 6;
		uint32_t y_bit_stream_size               : 22;
	} bits;
};

union reg_fbce_1c {
	uint32_t raw;
	struct {
		uint32_t y_total_line_bit_budget         : 17;
	} bits;
};

union reg_fbce_20 {
	uint32_t raw;
	struct {
		uint32_t c_lossless                      : 1;
		uint32_t c_base_qdpcm_q                  : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t c_base_pcm_bd_minus2            : 3;
		uint32_t _rsv_7                          : 1;
		uint32_t c_default_gr_k                  : 3;
		uint32_t _rsv_11                         : 1;
		uint32_t c_cplx_shift                    : 3;
		uint32_t _rsv_15                         : 1;
		uint32_t c_pen_pos_shift                 : 3;
	} bits;
};

union reg_fbce_24 {
	uint32_t raw;
	struct {
		uint32_t c_min_cu_bit                    : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t c_max_cu_bit                    : 7;
	} bits;
};

union reg_fbce_28 {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 6;
		uint32_t c_bit_stream_size               : 22;
	} bits;
};

union reg_fbce_2c {
	uint32_t raw;
	struct {
		uint32_t c_total_line_bit_budget         : 17;
	} bits;
};

union reg_fbce_30 {
	uint32_t raw;
	struct {
		uint32_t y_total_first_line_bit_budget   : 17;
	} bits;
};

union reg_fbce_34 {
	uint32_t raw;
	struct {
		uint32_t c_total_first_line_bit_budget   : 17;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_fusion_ctrl_reg {
	uint32_t raw;
	struct {
		uint32_t u1_fusion_en                    : 1;
		uint32_t u2_fusion_fnum                  : 2;
		uint32_t u2_fusion_mode                  : 2;
		uint32_t u4_fusion_dbg_mode              : 4;
		uint32_t u2_fusion_ysel                  : 2;
		uint32_t u2_fusion_nsel                  : 2;
		uint32_t u2_fusion_dsel                  : 2;
		uint32_t u13_fusion_evratio              : 13;
		uint32_t u4_fusion_ev_fmt                : 4;
	} bits;
};

union reg_fusion_ds1 {
	uint32_t raw;
	struct {
		uint32_t u12_ds1_th                      : 12;
		uint32_t u8_ds1_step                     : 8;
		uint32_t u8_ds1_lb                       : 8;
	} bits;
};

union reg_fusion_ds2 {
	uint32_t raw;
	struct {
		uint32_t u12_ds2_th                      : 12;
		uint32_t u8_ds2_step                     : 8;
		uint32_t u8_ds2_lb                       : 8;
	} bits;
};

union reg_fusion_bcl_range {
	uint32_t raw;
	struct {
		uint32_t u12_bcnl_range                  : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t u12_bcdl_range                  : 12;
	} bits;
};

union reg_fusion_bcl_p0 {
	uint32_t raw;
	struct {
		uint32_t u12_bcnl_p0                     : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t u12_bcdl_p0                     : 12;
	} bits;
};

union reg_fusion_bcl_slop {
	uint32_t raw;
	struct {
		uint32_t u16_bcnl_slope                  : 16;
		uint32_t u16_bcdl_slope                  : 16;
	} bits;
};

union reg_fusion_bcs_range {
	uint32_t raw;
	struct {
		uint32_t u12_bcns_range                  : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t u12_bcds_range                  : 12;
	} bits;
};

union reg_fusion_bcs_p0 {
	uint32_t raw;
	struct {
		uint32_t u12_bcns_p0                     : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t u12_bcds_p0                     : 12;
	} bits;
};

union reg_fusion_bcs_slop {
	uint32_t raw;
	struct {
		uint32_t u16_bcns_slope                  : 16;
		uint32_t u16_bcds_slope                  : 16;
	} bits;
};

union reg_fusion_diff {
	uint32_t raw;
	struct {
		uint32_t u4_diff_lumshftr                : 4;
		uint32_t u12_diff_lumthr                 : 12;
		uint32_t u3_diff_ratio                   : 3;
		uint32_t u5_diff_dw_d                    : 5;
	} bits;
};

union reg_fusion_lut_0 {
	uint32_t raw;
	struct {
		uint32_t u5_diff_dw16_0                  : 5;
		uint32_t u5_diff_dw16_1                  : 5;
		uint32_t u5_diff_dw16_2                  : 5;
		uint32_t u5_diff_dw16_3                  : 5;
		uint32_t u5_diff_dw16_4                  : 5;
		uint32_t u5_diff_dw16_5                  : 5;
	} bits;
};

union reg_fusion_lut_1 {
	uint32_t raw;
	struct {
		uint32_t u5_diff_dw16_6                  : 5;
		uint32_t u5_diff_dw16_7                  : 5;
		uint32_t u5_diff_dw16_8                  : 5;
		uint32_t u5_diff_dw16_9                  : 5;
		uint32_t u5_diff_dw16_10                 : 5;
		uint32_t u5_diff_dw16_11                 : 5;
	} bits;
};

union reg_fusion_lut_2 {
	uint32_t raw;
	struct {
		uint32_t u5_diff_dw16_12                 : 5;
		uint32_t u5_diff_dw16_13                 : 5;
		uint32_t u5_diff_dw16_14                 : 5;
		uint32_t u5_diff_dw16_15                 : 5;
	} bits;
};

union reg_fusion_hw_auto {
	uint32_t raw;
	struct {
		uint32_t hw_auto_cg_en                   : 1;
	} bits;
};

union reg_fusion_shadow_rd_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_gms_gms_status {
	uint32_t raw;
	struct {
		uint32_t gms_status                      : 32;
	} bits;
};

union reg_isp_gms_gms_grace_reset {
	uint32_t raw;
	struct {
		uint32_t gms_grace_reset                 : 1;
	} bits;
};

union reg_isp_gms_gms_monitor {
	uint32_t raw;
	struct {
		uint32_t gms_monitor                     : 32;
	} bits;
};

union reg_isp_gms_gms_enable {
	uint32_t raw;
	struct {
		uint32_t gms_enable                      : 1;
		uint32_t out_shiftbit                    : 3;
		uint32_t _rsv_4                          : 12;
		uint32_t force_bayer_enable              : 1;
		uint32_t _rsv_17                         : 11;
		uint32_t force_clk_enable                : 1;
	} bits;
};

union reg_isp_gms_gms_flow {
	uint32_t raw;
	struct {
		uint32_t gms_zerogmsogram                : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t gms_shadow_select               : 1;
	} bits;
};

union reg_isp_gms_gms_start_x {
	uint32_t raw;
	struct {
		uint32_t gms_start_x                     : 13;
	} bits;
};

union reg_isp_gms_gms_start_y {
	uint32_t raw;
	struct {
		uint32_t gms_start_y                     : 13;
	} bits;
};

union reg_isp_gms_gms_location {
	uint32_t raw;
	struct {
		uint32_t gms_location                    : 32;
	} bits;
};

union reg_isp_gms_gms_x_sizem1 {
	uint32_t raw;
	struct {
		uint32_t gms_x_sizem1                    : 10;
	} bits;
};

union reg_isp_gms_gms_y_sizem1 {
	uint32_t raw;
	struct {
		uint32_t gms_y_sizem1                    : 10;
	} bits;
};

union reg_isp_gms_gms_x_gap {
	uint32_t raw;
	struct {
		uint32_t gms_x_gap                       : 10;
	} bits;
};

union reg_isp_gms_gms_y_gap {
	uint32_t raw;
	struct {
		uint32_t gms_y_gap                       : 10;
	} bits;
};

union reg_isp_gms_gms_dummy {
	uint32_t raw;
	struct {
		uint32_t gms_dummy                       : 16;
	} bits;
};

union reg_isp_gms_gms_mem_sw_mode {
	uint32_t raw;
	struct {
		uint32_t gms_mem_sw_mode                 : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t gms_mem_sel                     : 6;
	} bits;
};

union reg_isp_gms_gms_mem_sw_raddr {
	uint32_t raw;
	struct {
		uint32_t gms_sw_raddr                    : 11;
	} bits;
};

union reg_isp_gms_gms_mem_sw_rdata {
	uint32_t raw;
	struct {
		uint32_t gms_rdata_r                     : 31;
		uint32_t gms_sw_r                        : 1;
	} bits;
};

union reg_isp_gms_gms_monitor_select {
	uint32_t raw;
	struct {
		uint32_t gms_monitor_select              : 32;
	} bits;
};

union reg_isp_gms_dmi_enable {
	uint32_t raw;
	struct {
		uint32_t dmi_enable                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t dmi_qos                         : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t force_dma_disable               : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_csi_bdg_top_ctrl {
	uint32_t raw;
	struct {
		uint32_t csi_mode                        : 2;
		uint32_t csi_in_format                   : 1;
		uint32_t csi_in_yuv_format               : 1;
		uint32_t ch_num                          : 2;
		uint32_t ch0_dma_wr_enable               : 1;
		uint32_t ch1_dma_wr_enable               : 1;
		uint32_t ch2_dma_wr_enable               : 1;
		uint32_t y_only                          : 1;
		uint32_t pxl_data_sel                    : 1;
		uint32_t vs_pol                          : 1;
		uint32_t hs_pol                          : 1;
		uint32_t reset_mode                      : 1;
		uint32_t vs_mode                         : 1;
		uint32_t abort_mode                      : 1;
		uint32_t reset                           : 1;
		uint32_t abort                           : 1;
		uint32_t ch3_dma_wr_enable               : 1;
		uint32_t ch3_dma_420_wr_enable           : 1;
		uint32_t yuv_pack_mode                   : 1;
		uint32_t multi_ch_frame_sync_en          : 1;
		uint32_t ch0_dma_420_wr_enable           : 1;
		uint32_t ch1_dma_420_wr_enable           : 1;
		uint32_t csi_enable                      : 1;
		uint32_t tgen_enable                     : 1;
		uint32_t yuv2bay_enable                  : 1;
		uint32_t ch2_dma_420_wr_enable           : 1;
		uint32_t shdw_read_sel                   : 1;
		uint32_t _rsv_29                         : 2;
		uint32_t csi_up_reg                      : 1;
	} bits;
};

union reg_isp_csi_bdg_interrupt_ctrl {
	uint32_t raw;
	struct {
		uint32_t ch0_vs_int_en                   : 1;
		uint32_t ch0_trig_int_en                 : 1;
		uint32_t ch0_drop_int_en                 : 1;
		uint32_t ch0_size_error_int_en           : 1;
		uint32_t ch1_vs_int_en                   : 1;
		uint32_t ch1_trig_int_en                 : 1;
		uint32_t ch1_drop_int_en                 : 1;
		uint32_t ch1_size_error_int_en           : 1;
		uint32_t ch2_vs_int_en                   : 1;
		uint32_t ch2_trig_int_en                 : 1;
		uint32_t ch2_drop_int_en                 : 1;
		uint32_t ch2_size_error_int_en           : 1;
		uint32_t ch3_vs_int_en                   : 1;
		uint32_t ch3_trig_int_en                 : 1;
		uint32_t ch3_drop_int_en                 : 1;
		uint32_t ch3_size_error_int_en           : 1;
		uint32_t _rsv_16                         : 12;
		uint32_t slice_line_intp_en              : 1;
		uint32_t dma_error_intp_en               : 1;
		uint32_t line_intp_en                    : 1;
		uint32_t fifo_overflow_int_en            : 1;
	} bits;
};

union reg_isp_csi_bdg_dma_dpcm_mode {
	uint32_t raw;
	struct {
		uint32_t dma_st_dpcm_mode                : 3;
		uint32_t _rsv_3                          : 1;
		uint32_t dpcm_mipi_opt                   : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t drop_mode                       : 4;
		uint32_t avg_mode                        : 4;
		uint32_t dpcm_xstr                       : 13;
	} bits;
};

union reg_isp_csi_bdg_dma_ld_dpcm_mode {
	uint32_t raw;
	struct {
		uint32_t dma_ld_dpcm_mode                : 3;
		uint32_t _rsv_3                          : 13;
		uint32_t dpcm_rx_xstr                    : 13;
	} bits;
};

union reg_isp_csi_bdg_ch0_size {
	uint32_t raw;
	struct {
		uint32_t ch0_frame_widthm1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch0_frame_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_ch1_size {
	uint32_t raw;
	struct {
		uint32_t ch1_frame_widthm1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch1_frame_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_ch2_size {
	uint32_t raw;
	struct {
		uint32_t ch2_frame_widthm1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch2_frame_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_ch3_size {
	uint32_t raw;
	struct {
		uint32_t ch3_frame_widthm1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch3_frame_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_ch0_crop_en {
	uint32_t raw;
	struct {
		uint32_t ch0_crop_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_ch0_horz_crop {
	uint32_t raw;
	struct {
		uint32_t ch0_horz_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch0_horz_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_ch0_vert_crop {
	uint32_t raw;
	struct {
		uint32_t ch0_vert_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch0_vert_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_ch0_blc_sum {
	uint32_t raw;
	struct {
		uint32_t ch0_blc_sum                     : 32;
	} bits;
};

union reg_isp_csi_bdg_ch1_crop_en {
	uint32_t raw;
	struct {
		uint32_t ch1_crop_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_ch1_horz_crop {
	uint32_t raw;
	struct {
		uint32_t ch1_horz_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch1_horz_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_ch1_vert_crop {
	uint32_t raw;
	struct {
		uint32_t ch1_vert_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch1_vert_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_ch1_blc_sum {
	uint32_t raw;
	struct {
		uint32_t ch1_blc_sum                     : 32;
	} bits;
};

union reg_isp_csi_bdg_ch2_crop_en {
	uint32_t raw;
	struct {
		uint32_t ch2_crop_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_ch2_horz_crop {
	uint32_t raw;
	struct {
		uint32_t ch2_horz_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch2_horz_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_ch2_vert_crop {
	uint32_t raw;
	struct {
		uint32_t ch2_vert_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch2_vert_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_ch2_blc_sum {
	uint32_t raw;
	struct {
		uint32_t ch2_blc_sum                     : 32;
	} bits;
};

union reg_isp_csi_bdg_ch3_crop_en {
	uint32_t raw;
	struct {
		uint32_t ch3_crop_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_ch3_horz_crop {
	uint32_t raw;
	struct {
		uint32_t ch3_horz_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch3_horz_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_ch3_vert_crop {
	uint32_t raw;
	struct {
		uint32_t ch3_vert_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch3_vert_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_ch3_blc_sum {
	uint32_t raw;
	struct {
		uint32_t ch3_blc_sum                     : 32;
	} bits;
};

union reg_isp_csi_bdg_pat_gen_ctrl {
	uint32_t raw;
	struct {
		uint32_t pat_en                          : 1;
		uint32_t gra_inv                         : 1;
		uint32_t auto_en                         : 1;
		uint32_t dith_en                         : 1;
		uint32_t snow_en                         : 1;
		uint32_t fix_mc                          : 1;
		uint32_t _rsv_6                          : 2;
		uint32_t dith_md                         : 3;
		uint32_t _rsv_11                         : 1;
		uint32_t bayer_id                        : 2;
	} bits;
};

union reg_isp_csi_bdg_pat_idx_ctrl {
	uint32_t raw;
	struct {
		uint32_t pat_prd                         : 8;
		uint32_t pat_idx                         : 5;
	} bits;
};

union reg_isp_csi_bdg_pat_color_0 {
	uint32_t raw;
	struct {
		uint32_t pat_r                           : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t pat_g                           : 12;
	} bits;
};

union reg_isp_csi_bdg_pat_color_1 {
	uint32_t raw;
	struct {
		uint32_t pat_b                           : 12;
	} bits;
};

union reg_isp_csi_bdg_background_color_0 {
	uint32_t raw;
	struct {
		uint32_t fde_r                           : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t fde_g                           : 12;
	} bits;
};

union reg_isp_csi_bdg_background_color_1 {
	uint32_t raw;
	struct {
		uint32_t fde_b                           : 12;
	} bits;
};

union reg_isp_csi_bdg_fix_color_0 {
	uint32_t raw;
	struct {
		uint32_t mde_r                           : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t mde_g                           : 12;
	} bits;
};

union reg_isp_csi_bdg_fix_color_1 {
	uint32_t raw;
	struct {
		uint32_t mde_b                           : 12;
	} bits;
};

union reg_isp_csi_bdg_mde_v_size {
	uint32_t raw;
	struct {
		uint32_t vmde_str                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t vmde_stp                        : 14;
	} bits;
};

union reg_isp_csi_bdg_mde_h_size {
	uint32_t raw;
	struct {
		uint32_t hmde_str                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t hmde_stp                        : 14;
	} bits;
};

union reg_isp_csi_bdg_fde_v_size {
	uint32_t raw;
	struct {
		uint32_t vfde_str                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t vfde_stp                        : 14;
	} bits;
};

union reg_isp_csi_bdg_fde_h_size {
	uint32_t raw;
	struct {
		uint32_t hfde_str                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t hfde_stp                        : 14;
	} bits;
};

union reg_isp_csi_bdg_hsync_ctrl {
	uint32_t raw;
	struct {
		uint32_t hs_str                          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t hs_stp                          : 14;
	} bits;
};

union reg_isp_csi_bdg_vsync_ctrl {
	uint32_t raw;
	struct {
		uint32_t vs_str                          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t vs_stp                          : 14;
	} bits;
};

union reg_isp_csi_bdg_tgen_tt_size {
	uint32_t raw;
	struct {
		uint32_t htt                             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t vtt                             : 14;
	} bits;
};

union reg_isp_csi_bdg_line_intp_height_0 {
	uint32_t raw;
	struct {
		uint32_t ch0_line_intp_heightm1          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch1_line_intp_heightm1          : 14;
	} bits;
};

union reg_isp_csi_bdg_ch0_debug_0 {
	uint32_t raw;
	struct {
		uint32_t ch0_pxl_cnt                     : 32;
	} bits;
};

union reg_isp_csi_bdg_ch0_debug_1 {
	uint32_t raw;
	struct {
		uint32_t ch0_line_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_ch0_debug_2 {
	uint32_t raw;
	struct {
		uint32_t ch0_vs_cnt                      : 16;
		uint32_t ch0_trig_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_ch0_debug_3 {
	uint32_t raw;
	struct {
		uint32_t ch0_tot_blk_idle                : 1;
		uint32_t ch0_tot_dma_idle                : 1;
		uint32_t ch0_bdg_dma_idle                : 1;
	} bits;
};

union reg_isp_csi_bdg_ch1_debug_0 {
	uint32_t raw;
	struct {
		uint32_t ch1_pxl_cnt                     : 32;
	} bits;
};

union reg_isp_csi_bdg_ch1_debug_1 {
	uint32_t raw;
	struct {
		uint32_t ch1_line_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_ch1_debug_2 {
	uint32_t raw;
	struct {
		uint32_t ch1_vs_cnt                      : 16;
		uint32_t ch1_trig_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_ch1_debug_3 {
	uint32_t raw;
	struct {
		uint32_t ch1_tot_blk_idle                : 1;
		uint32_t ch1_tot_dma_idle                : 1;
		uint32_t ch1_bdg_dma_idle                : 1;
	} bits;
};

union reg_isp_csi_bdg_ch2_debug_0 {
	uint32_t raw;
	struct {
		uint32_t ch2_pxl_cnt                     : 32;
	} bits;
};

union reg_isp_csi_bdg_ch2_debug_1 {
	uint32_t raw;
	struct {
		uint32_t ch2_line_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_ch2_debug_2 {
	uint32_t raw;
	struct {
		uint32_t ch2_vs_cnt                      : 16;
		uint32_t ch2_trig_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_ch2_debug_3 {
	uint32_t raw;
	struct {
		uint32_t ch2_tot_blk_idle                : 1;
		uint32_t ch2_tot_dma_idle                : 1;
		uint32_t ch2_bdg_dma_idle                : 1;
	} bits;
};

union reg_isp_csi_bdg_ch3_debug_0 {
	uint32_t raw;
	struct {
		uint32_t ch3_pxl_cnt                     : 32;
	} bits;
};

union reg_isp_csi_bdg_ch3_debug_1 {
	uint32_t raw;
	struct {
		uint32_t ch3_line_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_ch3_debug_2 {
	uint32_t raw;
	struct {
		uint32_t ch3_vs_cnt                      : 16;
		uint32_t ch3_trig_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_ch3_debug_3 {
	uint32_t raw;
	struct {
		uint32_t ch3_tot_blk_idle                : 1;
		uint32_t ch3_tot_dma_idle                : 1;
		uint32_t ch3_bdg_dma_idle                : 1;
	} bits;
};

union reg_isp_csi_bdg_interrupt_status_0 {
	uint32_t raw;
	struct {
		uint32_t ch0_frame_drop_int              : 1;
		uint32_t ch0_vs_int                      : 1;
		uint32_t ch0_trig_int                    : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t ch0_frame_width_gt_int          : 1;
		uint32_t ch0_frame_width_ls_int          : 1;
		uint32_t ch0_frame_height_gt_int         : 1;
		uint32_t ch0_frame_height_ls_int         : 1;
		uint32_t ch1_frame_drop_int              : 1;
		uint32_t ch1_vs_int                      : 1;
		uint32_t ch1_trig_int                    : 1;
		uint32_t _rsv_11                         : 1;
		uint32_t ch1_frame_width_gt_int          : 1;
		uint32_t ch1_frame_width_ls_int          : 1;
		uint32_t ch1_frame_height_gt_int         : 1;
		uint32_t ch1_frame_height_ls_int         : 1;
		uint32_t ch2_frame_drop_int              : 1;
		uint32_t ch2_vs_int                      : 1;
		uint32_t ch2_trig_int                    : 1;
		uint32_t _rsv_19                         : 1;
		uint32_t ch2_frame_width_gt_int          : 1;
		uint32_t ch2_frame_width_ls_int          : 1;
		uint32_t ch2_frame_height_gt_int         : 1;
		uint32_t ch2_frame_height_ls_int         : 1;
		uint32_t ch3_frame_drop_int              : 1;
		uint32_t ch3_vs_int                      : 1;
		uint32_t ch3_trig_int                    : 1;
		uint32_t _rsv_27                         : 1;
		uint32_t ch3_frame_width_gt_int          : 1;
		uint32_t ch3_frame_width_ls_int          : 1;
		uint32_t ch3_frame_height_gt_int         : 1;
		uint32_t ch3_frame_height_ls_int         : 1;
	} bits;
};

union reg_isp_csi_bdg_interrupt_status_1 {
	uint32_t raw;
	struct {
		uint32_t fifo_overflow_int               : 1;
		uint32_t frame_resolution_over_max_int   : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t dma_error_int                   : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t ch0_line_intp_int               : 1;
		uint32_t ch1_line_intp_int               : 1;
		uint32_t ch2_line_intp_int               : 1;
		uint32_t ch3_line_intp_int               : 1;
		uint32_t ch0_slice_line_intp_int         : 1;
		uint32_t ch1_slice_line_intp_int         : 1;
		uint32_t ch2_slice_line_intp_int         : 1;
		uint32_t ch3_slice_line_intp_int         : 1;
		uint32_t ch3_precrop_line_intp_int       : 1;
		uint32_t ch2_precrop_line_intp_int       : 1;
		uint32_t ch1_precrop_line_intp_int       : 1;
		uint32_t ch0_precrop_line_intp_int       : 1;
		uint32_t clsc_se_fetch_done_intp_int     : 1;
		uint32_t clsc_le_fetch_done_intp_int     : 1;
		uint32_t gain_dma_mode_en                : 1;
		uint32_t clsc_rdma_done_intp_int         : 1;
		uint32_t clsc_rdma_done_intp_en          : 1;
		uint32_t sw_clsc_up_reg_1t               : 1;
	} bits;
};

union reg_isp_csi_bdg_bdg_debug {
	uint32_t raw;
	struct {
		uint32_t ring_buff_idle                  : 1;
	} bits;
};

union reg_isp_csi_bdg_out_vsync_line_delay {
	uint32_t raw;
	struct {
		uint32_t out_vsync_line_delay            : 12;
	} bits;
};

union reg_isp_csi_bdg_wr_urgent_ctrl {
	uint32_t raw;
	struct {
		uint32_t wr_near_overflow_threshold      : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t wr_safe_threshold               : 13;
	} bits;
};

union reg_isp_csi_bdg_rd_urgent_ctrl {
	uint32_t raw;
	struct {
		uint32_t rd_near_overflow_threshold      : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t rd_safe_threshold               : 13;
	} bits;
};

union reg_isp_csi_bdg_dummy {
	uint32_t raw;
	struct {
		uint32_t dummy_in                        : 16;
		uint32_t dummy_out                       : 16;
	} bits;
};

union reg_isp_csi_bdg_line_intp_height_1 {
	uint32_t raw;
	struct {
		uint32_t ch2_line_intp_heightm1          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch3_line_intp_heightm1          : 14;
	} bits;
};

union reg_isp_csi_bdg_slice_line_intp_height_0 {
	uint32_t raw;
	struct {
		uint32_t ch0_slice_line_intp_heightm1    : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch1_slice_line_intp_heightm1    : 14;
	} bits;
};

union reg_isp_csi_bdg_slice_line_intp_height_1 {
	uint32_t raw;
	struct {
		uint32_t ch2_slice_line_intp_heightm1    : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch3_slice_line_intp_heightm1    : 14;
	} bits;
};

union reg_isp_csi_bdg_bayer_type_clk_gate_yuv_swap {
	uint32_t raw;
	struct {
		uint32_t bayer_type_post_crop_multich    : 2;
		uint32_t clk_gating_en                   : 1;
		uint32_t yuv_ch_swap_en                  : 1;
		uint32_t hdr_mode_en                     : 1;
		uint32_t clsc_bypass_le                  : 1;
		uint32_t clsc_bypass_se                  : 1;
		uint32_t pre_crop_line_intp_en           : 1;
		uint32_t clsc_fetch_done_intp_en         : 1;
		uint32_t sw_dma_trig_str_1t              : 1;
		uint32_t hw_ff_threshold                 : 10;
		uint32_t ring_fifo_usage                 : 10;
		uint32_t clsc_hw_en_status               : 2;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch0_crop_en {
	uint32_t raw;
	struct {
		uint32_t st_ch0_crop_en                  : 1;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch0_horz_crop {
	uint32_t raw;
	struct {
		uint32_t st_ch0_horz_crop_start          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t st_ch0_horz_crop_end            : 14;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch0_vert_crop {
	uint32_t raw;
	struct {
		uint32_t st_ch0_vert_crop_start          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t st_ch0_vert_crop_end            : 14;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch1_crop_en {
	uint32_t raw;
	struct {
		uint32_t st_ch1_crop_en                  : 1;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch1_horz_crop {
	uint32_t raw;
	struct {
		uint32_t st_ch1_horz_crop_start          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t st_ch1_horz_crop_end            : 14;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch1_vert_crop {
	uint32_t raw;
	struct {
		uint32_t st_ch1_vert_crop_start          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t st_ch1_vert_crop_end            : 14;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch2_crop_en {
	uint32_t raw;
	struct {
		uint32_t st_ch2_crop_en                  : 1;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch2_horz_crop {
	uint32_t raw;
	struct {
		uint32_t st_ch2_horz_crop_start          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t st_ch2_horz_crop_end            : 14;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch2_vert_crop {
	uint32_t raw;
	struct {
		uint32_t st_ch2_vert_crop_start          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t st_ch2_vert_crop_end            : 14;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch3_crop_en {
	uint32_t raw;
	struct {
		uint32_t st_ch3_crop_en                  : 1;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch3_horz_crop {
	uint32_t raw;
	struct {
		uint32_t st_ch3_horz_crop_start          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t st_ch3_horz_crop_end            : 14;
	} bits;
};

union reg_isp_csi_bdg_wdma_ch3_vert_crop {
	uint32_t raw;
	struct {
		uint32_t st_ch3_vert_crop_start          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t st_ch3_vert_crop_end            : 14;
	} bits;
};

union reg_isp_csi_bdg_trig_dly_control_0 {
	uint32_t raw;
	struct {
		uint32_t trig_dly_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_trig_dly_control_1 {
	uint32_t raw;
	struct {
		uint32_t trig_dly_value                  : 32;
	} bits;
};

union reg_isp_csi_bdg_bayer_type {
	uint32_t raw;
	struct {
		uint32_t bayer_type                      : 2;
	} bits;
};

union reg_isp_csi_bdg_ch0_ai_isp_control {
	uint32_t raw;
	struct {
		uint32_t bypass_ch0                      : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t hw_auto_cg_en_ch0               : 1;
		uint32_t guard_cnt_ch0                   : 8;
	} bits;
};

union reg_isp_csi_bdg_ch0_ai_isp_transform {
	uint32_t raw;
	struct {
		uint32_t ai_isp_transform_en_ch0         : 1;
		uint32_t in_format_ch0                   : 1;
		uint32_t out_format_ch0                  : 2;
		uint32_t round_mode_ch0                  : 3;
	} bits;
};

union reg_isp_csi_bdg_ai_isp_debug_status_ch0 {
	uint32_t raw;
	struct {
		uint32_t ai_isp_debug_status_ch0         : 32;
	} bits;
};

union reg_isp_csi_bdg_ch1_ai_isp_control {
	uint32_t raw;
	struct {
		uint32_t bypass_ch1                      : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t hw_auto_cg_en_ch1               : 1;
		uint32_t guard_cnt_ch1                   : 8;
	} bits;
};

union reg_isp_csi_bdg_ch1_ai_isp_transform {
	uint32_t raw;
	struct {
		uint32_t ai_isp_transform_en_ch1         : 1;
		uint32_t in_format_ch1                   : 1;
		uint32_t out_format_ch1                  : 2;
		uint32_t round_mode_ch1                  : 3;
	} bits;
};

union reg_isp_csi_bdg_ai_isp_debug_status_ch1 {
	uint32_t raw;
	struct {
		uint32_t ai_isp_debug_status_ch1         : 32;
	} bits;
};

union reg_isp_csi_bdg_ch2_ai_isp_control {
	uint32_t raw;
	struct {
		uint32_t bypass_ch2                      : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t hw_auto_cg_en_ch2               : 1;
		uint32_t guard_cnt_ch2                   : 8;
	} bits;
};

union reg_isp_csi_bdg_ch2_ai_isp_transform {
	uint32_t raw;
	struct {
		uint32_t ai_isp_transform_en_ch2         : 1;
		uint32_t in_format_ch2                   : 1;
		uint32_t out_format_ch2                  : 2;
		uint32_t round_mode_ch2                  : 3;
	} bits;
};

union reg_isp_csi_bdg_ai_isp_debug_status_ch2 {
	uint32_t raw;
	struct {
		uint32_t ai_isp_debug_status_ch2         : 32;
	} bits;
};

union reg_isp_csi_bdg_ch3_ai_isp_control {
	uint32_t raw;
	struct {
		uint32_t bypass_ch3                      : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t hw_auto_cg_en_ch3               : 1;
		uint32_t guard_cnt_ch3                   : 8;
	} bits;
};

union reg_isp_csi_bdg_ch3_ai_isp_transform {
	uint32_t raw;
	struct {
		uint32_t ai_isp_transform_en_ch3         : 1;
		uint32_t in_format_ch3                   : 1;
		uint32_t out_format_ch3                  : 2;
		uint32_t round_mode_ch3                  : 3;
	} bits;
};

union reg_isp_csi_bdg_ai_isp_debug_status_ch3 {
	uint32_t raw;
	struct {
		uint32_t ai_isp_debug_status_ch3         : 32;
	} bits;
};

union reg_isp_csi_bdg_ch_dma_420_cfg {
	uint32_t raw;
	struct {
		uint32_t ch0_dma_420_cfg                 : 1;
		uint32_t ch1_dma_420_cfg                 : 1;
		uint32_t ch2_dma_420_cfg                 : 1;
		uint32_t ch3_dma_420_cfg                 : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_csi_bdg_lite_bdg_top_ctrl {
	uint32_t raw;
	struct {
		uint32_t csi_mode                        : 2;
		uint32_t _rsv_2                          : 2;
		uint32_t ch_num                          : 2;
		uint32_t ch0_dma_wr_enable               : 1;
		uint32_t ch1_dma_wr_enable               : 1;
		uint32_t ch2_dma_wr_enable               : 1;
		uint32_t y_only                          : 1;
		uint32_t _rsv_10                         : 1;
		uint32_t vs_pol                          : 1;
		uint32_t hs_pol                          : 1;
		uint32_t reset_mode                      : 1;
		uint32_t vs_mode                         : 1;
		uint32_t abort_mode                      : 1;
		uint32_t reset                           : 1;
		uint32_t abort                           : 1;
		uint32_t ch3_dma_wr_enable               : 1;
		uint32_t ch3_dma_420_wr_enable           : 1;
		uint32_t _rsv_20                         : 2;
		uint32_t ch0_dma_420_wr_enable           : 1;
		uint32_t ch1_dma_420_wr_enable           : 1;
		uint32_t mcsi_enable                      : 1;
		uint32_t _rsv_25                         : 2;
		uint32_t ch2_dma_420_wr_enable           : 1;
		uint32_t shdw_read_sel                   : 1;
		uint32_t _rsv_29                         : 2;
		uint32_t csi_up_reg                      : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_bdg_interrupt_ctrl_0 {
	uint32_t raw;
	struct {
		uint32_t ch0_vs_int_en                   : 1;
		uint32_t ch0_trig_int_en                 : 1;
		uint32_t ch0_drop_int_en                 : 1;
		uint32_t ch0_size_error_int_en           : 1;
		uint32_t ch0_frame_done_en               : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t ch1_vs_int_en                   : 1;
		uint32_t ch1_trig_int_en                 : 1;
		uint32_t ch1_drop_int_en                 : 1;
		uint32_t ch1_size_error_int_en           : 1;
		uint32_t ch1_frame_done_en               : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t ch2_vs_int_en                   : 1;
		uint32_t ch2_trig_int_en                 : 1;
		uint32_t ch2_drop_int_en                 : 1;
		uint32_t ch2_size_error_int_en           : 1;
		uint32_t ch2_frame_done_en               : 1;
		uint32_t _rsv_21                         : 3;
		uint32_t ch3_vs_int_en                   : 1;
		uint32_t ch3_trig_int_en                 : 1;
		uint32_t ch3_drop_int_en                 : 1;
		uint32_t ch3_size_error_int_en           : 1;
		uint32_t ch3_frame_done_en               : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_bdg_interrupt_ctrl_1 {
	uint32_t raw;
	struct {
		uint32_t line_intp_en                    : 1;
		uint32_t fifo_overflow_int_en            : 1;
		uint32_t dma_error_intp_en               : 1;
		uint32_t _rsv_3                          : 5;
		uint32_t drop_mode                       : 4;
		uint32_t avg_mode                        : 4;
	} bits;
};

union reg_isp_csi_bdg_lite_frame_vld {
	uint32_t raw;
	struct {
		uint32_t frame_vld_ch0                   : 1;
		uint32_t frame_vld_ch1                   : 1;
		uint32_t frame_vld_ch2                   : 1;
		uint32_t frame_vld_ch3                   : 1;
		uint32_t e_vld_clr_ch0                   : 1;
		uint32_t e_vld_clr_ch1                   : 1;
		uint32_t e_vld_clr_ch2                   : 1;
		uint32_t e_vld_clr_ch3                   : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_ch0_size {
	uint32_t raw;
	struct {
		uint32_t ch0_frame_widthm1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch0_frame_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch1_size {
	uint32_t raw;
	struct {
		uint32_t ch1_frame_widthm1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch1_frame_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch2_size {
	uint32_t raw;
	struct {
		uint32_t ch2_frame_widthm1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch2_frame_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch3_size {
	uint32_t raw;
	struct {
		uint32_t ch3_frame_widthm1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch3_frame_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch0_crop_en {
	uint32_t raw;
	struct {
		uint32_t ch0_crop_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_ch0_horz_crop {
	uint32_t raw;
	struct {
		uint32_t ch0_horz_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch0_horz_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch0_vert_crop {
	uint32_t raw;
	struct {
		uint32_t ch0_vert_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch0_vert_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch1_crop_en {
	uint32_t raw;
	struct {
		uint32_t ch1_crop_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_ch1_horz_crop {
	uint32_t raw;
	struct {
		uint32_t ch1_horz_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch1_horz_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch1_vert_crop {
	uint32_t raw;
	struct {
		uint32_t ch1_vert_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch1_vert_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch2_crop_en {
	uint32_t raw;
	struct {
		uint32_t ch2_crop_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_ch2_horz_crop {
	uint32_t raw;
	struct {
		uint32_t ch2_horz_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch2_horz_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch2_vert_crop {
	uint32_t raw;
	struct {
		uint32_t ch2_vert_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch2_vert_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch3_crop_en {
	uint32_t raw;
	struct {
		uint32_t ch3_crop_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_ch3_horz_crop {
	uint32_t raw;
	struct {
		uint32_t ch3_horz_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch3_horz_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch3_vert_crop {
	uint32_t raw;
	struct {
		uint32_t ch3_vert_crop_start             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ch3_vert_crop_end               : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_line_intp_height {
	uint32_t raw;
	struct {
		uint32_t line_intp_heightm1              : 14;
	} bits;
};

union reg_isp_csi_bdg_lite_ch0_debug_0 {
	uint32_t raw;
	struct {
		uint32_t ch0_pxl_cnt                     : 32;
	} bits;
};

union reg_isp_csi_bdg_lite_ch0_debug_1 {
	uint32_t raw;
	struct {
		uint32_t ch0_line_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_ch0_debug_2 {
	uint32_t raw;
	struct {
		uint32_t ch0_vs_cnt                      : 16;
		uint32_t ch0_trig_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_ch0_debug_3 {
	uint32_t raw;
	struct {
		uint32_t ch0_tot_blk_idle                : 1;
		uint32_t ch0_tot_dma_idle                : 1;
		uint32_t ch0_bdg_dma_idle                : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_ch1_debug_0 {
	uint32_t raw;
	struct {
		uint32_t ch1_pxl_cnt                     : 32;
	} bits;
};

union reg_isp_csi_bdg_lite_ch1_debug_1 {
	uint32_t raw;
	struct {
		uint32_t ch1_line_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_ch1_debug_2 {
	uint32_t raw;
	struct {
		uint32_t ch1_vs_cnt                      : 16;
		uint32_t ch1_trig_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_ch1_debug_3 {
	uint32_t raw;
	struct {
		uint32_t ch1_tot_blk_idle                : 1;
		uint32_t ch1_tot_dma_idle                : 1;
		uint32_t ch1_bdg_dma_idle                : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_ch2_debug_0 {
	uint32_t raw;
	struct {
		uint32_t ch2_pxl_cnt                     : 32;
	} bits;
};

union reg_isp_csi_bdg_lite_ch2_debug_1 {
	uint32_t raw;
	struct {
		uint32_t ch2_line_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_ch2_debug_2 {
	uint32_t raw;
	struct {
		uint32_t ch2_vs_cnt                      : 16;
		uint32_t ch2_trig_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_ch2_debug_3 {
	uint32_t raw;
	struct {
		uint32_t ch2_tot_blk_idle                : 1;
		uint32_t ch2_tot_dma_idle                : 1;
		uint32_t ch2_bdg_dma_idle                : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_ch3_debug_0 {
	uint32_t raw;
	struct {
		uint32_t ch3_pxl_cnt                     : 32;
	} bits;
};

union reg_isp_csi_bdg_lite_ch3_debug_1 {
	uint32_t raw;
	struct {
		uint32_t ch3_line_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_ch3_debug_2 {
	uint32_t raw;
	struct {
		uint32_t ch3_vs_cnt                      : 16;
		uint32_t ch3_trig_cnt                    : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_ch3_debug_3 {
	uint32_t raw;
	struct {
		uint32_t ch3_tot_blk_idle                : 1;
		uint32_t ch3_tot_dma_idle                : 1;
		uint32_t ch3_bdg_dma_idle                : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_interrupt_status_0 {
	uint32_t raw;
	struct {
		uint32_t ch0_frame_drop_int              : 1;
		uint32_t ch0_vs_int                      : 1;
		uint32_t ch0_trig_int                    : 1;
		uint32_t ch0_frame_done                  : 1;
		uint32_t ch0_frame_width_gt_int          : 1;
		uint32_t ch0_frame_width_ls_int          : 1;
		uint32_t ch0_frame_height_gt_int         : 1;
		uint32_t ch0_frame_height_ls_int         : 1;
		uint32_t ch1_frame_drop_int              : 1;
		uint32_t ch1_vs_int                      : 1;
		uint32_t ch1_trig_int                    : 1;
		uint32_t ch1_frame_done                  : 1;
		uint32_t ch1_frame_width_gt_int          : 1;
		uint32_t ch1_frame_width_ls_int          : 1;
		uint32_t ch1_frame_height_gt_int         : 1;
		uint32_t ch1_frame_height_ls_int         : 1;
		uint32_t ch2_frame_drop_int              : 1;
		uint32_t ch2_vs_int                      : 1;
		uint32_t ch2_trig_int                    : 1;
		uint32_t ch2_frame_done                  : 1;
		uint32_t ch2_frame_width_gt_int          : 1;
		uint32_t ch2_frame_width_ls_int          : 1;
		uint32_t ch2_frame_height_gt_int         : 1;
		uint32_t ch2_frame_height_ls_int         : 1;
		uint32_t ch3_frame_drop_int              : 1;
		uint32_t ch3_vs_int                      : 1;
		uint32_t ch3_trig_int                    : 1;
		uint32_t ch3_frame_done                  : 1;
		uint32_t ch3_frame_width_gt_int          : 1;
		uint32_t ch3_frame_width_ls_int          : 1;
		uint32_t ch3_frame_height_gt_int         : 1;
		uint32_t ch3_frame_height_ls_int         : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_interrupt_status_1 {
	uint32_t raw;
	struct {
		uint32_t fifo_overflow_int               : 1;
		uint32_t frame_resolution_over_max_int   : 1;
		uint32_t _rsv_2                          : 1;
		uint32_t line_intp_int                   : 1;
		uint32_t dma_error_int                   : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_bdg_debug {
	uint32_t raw;
	struct {
		uint32_t ring_buff_idle                  : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_wr_urgent_ctrl {
	uint32_t raw;
	struct {
		uint32_t wr_near_overflow_threshold      : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t wr_safe_threshold               : 13;
	} bits;
};

union reg_isp_csi_bdg_lite_rd_urgent_ctrl {
	uint32_t raw;
	struct {
		uint32_t rd_near_overflow_threshold      : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t rd_safe_threshold               : 13;
	} bits;
};

union reg_isp_csi_bdg_lite_dummy {
	uint32_t raw;
	struct {
		uint32_t dummy_in                        : 16;
		uint32_t dummy_out                       : 16;
	} bits;
};

union reg_isp_csi_bdg_lite_trig_dly_control_0 {
	uint32_t raw;
	struct {
		uint32_t trig_dly_en                     : 1;
	} bits;
};

union reg_isp_csi_bdg_lite_trig_dly_control_1 {
	uint32_t raw;
	struct {
		uint32_t trig_dly_value                  : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_dma_ctl_sys_control {
	uint32_t raw;
	struct {
		uint32_t qos_sel                         : 1;
		uint32_t sw_qos                          : 1;
		uint32_t enable_inv                      : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t baseh                           : 16;
		uint32_t base_sel                        : 1;
		uint32_t stride_sel                      : 1;
		uint32_t seglen_sel                      : 1;
		uint32_t segnum_sel                      : 1;
		uint32_t slice_enable                    : 1;
		uint32_t update_base_addr                : 1;
		uint32_t inv_sel                         : 1;
		uint32_t _rsv_27                         : 1;
		uint32_t dbg_sel                         : 3;
	} bits;
};

union reg_isp_dma_ctl_base_addr {
	uint32_t raw;
	struct {
		uint32_t basel                           : 32;
	} bits;
};

union reg_isp_dma_ctl_dma_seglen {
	uint32_t raw;
	struct {
		uint32_t seglen                          : 28;
	} bits;
};

union reg_isp_dma_ctl_dma_stride {
	uint32_t raw;
	struct {
		uint32_t stride                          : 28;
	} bits;
};

union reg_isp_dma_ctl_dma_segnum {
	uint32_t raw;
	struct {
		uint32_t segnum                          : 14;
	} bits;
};

union reg_isp_dma_ctl_dma_status {
	uint32_t raw;
	struct {
		uint32_t status                          : 32;
	} bits;
};

union reg_isp_dma_ctl_dma_slicesize {
	uint32_t raw;
	struct {
		uint32_t slice_size                      : 6;
	} bits;
};

union reg_isp_dma_ctl_dma_dummy {
	uint32_t raw;
	struct {
		uint32_t dummy                           : 16;
		uint32_t perf_patch_enable               : 1;
		uint32_t seglen_less16_enable            : 1;
		uint32_t sync_patch_enable               : 1;
		uint32_t trig_patch_enable               : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_lsc_sc_wrap_0 {
	uint32_t raw;
	struct {
		uint32_t bypass                          : 1;
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t lsc_bayer_starting              : 2;
		uint32_t trig_str_1t                     : 1;
		uint32_t lsc_enable                      : 1;
		uint32_t _rsv_6                          : 10;
		uint32_t lsc_debug                       : 16;
	} bits;
};

union reg_isp_lsc_sc_wrap_1 {
	uint32_t raw;
	struct {
		uint32_t sw_rst                          : 1;
		uint32_t shdw_update_1t                  : 1;
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

union reg_isp_lsc_sc_wrap_2 {
	uint32_t raw;
	struct {
		uint32_t img_width                       : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t img_height                      : 14;
	} bits;
};

union reg_isp_lsc_sc_wrap_3 {
	uint32_t raw;
	struct {
		uint32_t lsc_boundary_interpolation_lf_range: 6;
		uint32_t _rsv_6                          : 2;
		uint32_t lsc_boundary_interpolation_rt_range: 6;
		uint32_t _rsv_14                         : 2;
		uint32_t lsc_boundary_interpolation_up_range: 6;
		uint32_t _rsv_22                         : 2;
		uint32_t lsc_boundary_interpolation_dn_range: 6;
	} bits;
};

union reg_isp_lsc_sc_wrap_4 {
	uint32_t raw;
	struct {
		uint32_t lsc_boundary_interpolation_mode : 1;
		uint32_t lsc_gain_bicubic_0_bilinear_1   : 1;
		uint32_t lsc_gain_3p9_0_4p8_1            : 1;
		uint32_t lsc_overflow_rst                : 1;
		uint32_t _rsv_4                          : 12;
		uint32_t lsc_strength                    : 12;
	} bits;
};

union reg_isp_lsc_sc_wrap_5 {
	uint32_t raw;
	struct {
		uint32_t lsc_imgx0                       : 22;
	} bits;
};

union reg_isp_lsc_sc_wrap_6 {
	uint32_t raw;
	struct {
		uint32_t lsc_imgy0                       : 22;
	} bits;
};

union reg_isp_lsc_sc_wrap_7 {
	uint32_t raw;
	struct {
		uint32_t lsc_xstep                       : 15;
		uint32_t _rsv_15                         : 1;
		uint32_t lsc_ystep                       : 15;
	} bits;
};

union reg_isp_lsc_sc_wrap_8 {
	uint32_t raw;
	struct {
		uint32_t lsc_intp_gain_min               : 26;
	} bits;
};

union reg_isp_lsc_sc_wrap_9 {
	uint32_t raw;
	struct {
		uint32_t lsc_intp_gain_max               : 26;
	} bits;
};

union reg_isp_lsc_sc_wrap_10 {
	uint32_t raw;
	struct {
		uint32_t lsc_src_width                   : 1;
	} bits;
};

union reg_isp_lsc_sc_wrap_11 {
	uint32_t raw;
	struct {
		uint32_t lsc_launch_addr                 : 11;
		uint32_t dma_enable_lsc                  : 1;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_dma_addr_high8              : 8;
	} bits;
};

union reg_isp_lsc_sc_wrap_12 {
	uint32_t raw;
	struct {
		uint32_t lsc_dma_addr_low32              : 32;
	} bits;
};

union reg_isp_lsc_sc_wrap_13 {
	uint32_t raw;
	struct {
		uint32_t blc_offset_r                    : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t blc_offset_gr                   : 12;
	} bits;
};

union reg_isp_lsc_sc_wrap_14 {
	uint32_t raw;
	struct {
		uint32_t blc_offset_gb                   : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t blc_offset_b                    : 12;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_lscr_sc_wrap_0 {
	uint32_t raw;
	struct {
		uint32_t bypass                          : 1;
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t lsc_bayer_starting              : 2;
		uint32_t trig_str_1t                     : 1;
	} bits;
};

union reg_isp_lscr_sc_wrap_1 {
	uint32_t raw;
	struct {
		uint32_t sw_rst                          : 1;
		uint32_t shdw_update_1t                  : 1;
		uint32_t lscr_enable                     : 1;
		uint32_t shdw_read_sel                   : 1;
		uint32_t _rsv_4                          : 4;
		uint32_t lscr_dy_gain                    : 8;
		uint32_t blc_enable                      : 1;
	} bits;
};

union reg_isp_lscr_sc_wrap_2 {
	uint32_t raw;
	struct {
		uint32_t img_width                       : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t img_height                      : 14;
	} bits;
};

union reg_isp_lscr_sc_wrap_3 {
	uint32_t raw;
	struct {
		uint32_t lscr_centerx                    : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t lscr_centery                    : 13;
	} bits;
};

union reg_isp_lscr_sc_wrap_4 {
	uint32_t raw;
	struct {
		uint32_t lscr_norm                       : 15;
		uint32_t _rsv_15                         : 1;
		uint32_t lscr_strnth                     : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_5 {
	uint32_t raw;
	struct {
		uint32_t lscr_nd_thr                     : 8;
		uint32_t lscr_nd_str                     : 8;
	} bits;
};

union reg_isp_lscr_sc_wrap_6 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain0               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain1               : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_7 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain2               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain3               : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_8 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain4               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain5               : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_9 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain6               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain7               : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_10 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain8               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain9               : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_11 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain10              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain11              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_12 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain12              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain13              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_13 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain14              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain15              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_14 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain16              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain17              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_15 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain18              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain19              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_16 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain20              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain21              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_17 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain22              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain23              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_18 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain24              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain25              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_19 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain26              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain27              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_20 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain28              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain29              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_21 {
	uint32_t raw;
	struct {
		uint32_t lsc_radius_ggain30              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t lsc_radius_ggain31              : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_22 {
	uint32_t raw;
	struct {
		uint32_t blc_offset_r                    : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t blc_offset_gr                   : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_23 {
	uint32_t raw;
	struct {
		uint32_t blc_offset_gb                   : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t blc_offset_b                    : 12;
	} bits;
};

union reg_isp_lscr_sc_wrap_24 {
	uint32_t raw;
	struct {
		uint32_t lscr_debug                      : 32;
	} bits;
};

union reg_isp_lscr_sc_wrap_25 {
	uint32_t raw;
	struct {
		uint32_t lscr_dma_enable                 : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t lscr_dma_idle                   : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t lscr_dma_addr_high8             : 8;
	} bits;
};

union reg_isp_lscr_sc_wrap_26 {
	uint32_t raw;
	struct {
		uint32_t lscr_dma_addr_low32             : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_rdma_core_shadow_rd_sel   {
	uint32_t raw;
	struct {
		uint32_t shadow_rd_sel                   : 1;
		uint32_t abort_mode                      : 1;
		uint32_t _rsv_2                          : 6;
		uint32_t max_ostd                        : 8;
		uint32_t ostd_sw_en                      : 1;
	} bits;
};

union reg_rdma_core_ip_disable {
	uint32_t raw;
	struct {
		uint32_t ip_disable                      : 32;
	} bits;
};

union reg_rdma_core_up_ring_base {
	uint32_t raw;
	struct {
		uint32_t up_ring_base                    : 32;
	} bits;
};

union reg_rdma_core_norm_status0 {
	uint32_t raw;
	struct {
		uint32_t abort_done                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t error_axi                       : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t error_id                        : 5;
		uint32_t _rsv_13                         : 3;
		uint32_t dma_version                     : 16;
	} bits;
};

union reg_rdma_core_norm_status1 {
	uint32_t raw;
	struct {
		uint32_t id_idle                         : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_en {
	uint32_t raw;
	struct {
		uint32_t ring_enable                     : 32;
	} bits;
};

union reg_rdma_core_norm_perf  {
	uint32_t raw;
	struct {
		uint32_t bwlwin                          : 10;
		uint32_t bwltxn                          : 6;
	} bits;
};

union reg_rdma_core_ar_priority_sel {
	uint32_t raw;
	struct {
		uint32_t ar_priority_sel                 : 1;
		uint32_t qos_priority_sel                : 1;
		uint32_t arb_hist_disable                : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t arb_usage_th                    : 4;
	} bits;
};

union reg_rdma_core_ring_patch_enable {
	uint32_t raw;
	struct {
		uint32_t ring_patch_enable               : 32;
	} bits;
};

union reg_rdma_core_set_ring_base {
	uint32_t raw;
	struct {
		uint32_t set_ring_base                   : 32;
	} bits;
};

union reg_rdma_core_ring_base_addr_l {
	uint32_t raw;
	struct {
		uint32_t ring_base_l                     : 32;
	} bits;
};

union reg_rdma_core_ring_base_addr_h {
	uint32_t raw;
	struct {
		uint32_t ring_base_h                     : 16;
	} bits;
};

union reg_rdma_core_ring_buffer_size0 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size0                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size1 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size1                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size2 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size2                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size3 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size3                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size4 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size4                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size5 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size5                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size6 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size6                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size7 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size7                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size8 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size8                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size9 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size9                      : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size10 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size10                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size11 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size11                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size12 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size12                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size13 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size13                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size14 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size14                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size15 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size15                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size16 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size16                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size17 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size17                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size18 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size18                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size19 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size19                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size20 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size20                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size21 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size21                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size22 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size22                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size23 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size23                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size24 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size24                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size25 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size25                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size26 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size26                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size27 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size27                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size28 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size28                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size29 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size29                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size30 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size30                     : 32;
	} bits;
};

union reg_rdma_core_ring_buffer_size31 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size31                     : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts0 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr0                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts1 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr1                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts2 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr2                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts3 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr3                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts4 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr4                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts5 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr5                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts6 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr6                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts7 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr7                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts8 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr8                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts9 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr9                  : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts10 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr10                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts11 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr11                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts12 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr12                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts13 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr13                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts14 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr14                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts15 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr15                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts16 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr16                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts17 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr17                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts18 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr18                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts19 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr19                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts20 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr20                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts21 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr21                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts22 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr22                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts23 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr23                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts24 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr24                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts25 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr25                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts26 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr26                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts27 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr27                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts28 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr28                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts29 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr29                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts30 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr30                 : 32;
	} bits;
};

union reg_rdma_core_next_dma_addr_sts31 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr31                 : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_top_int_event0 {
	uint32_t raw;
	struct {
		uint32_t frame_done_fe0                  : 4;
		uint32_t frame_done_fe1                  : 4;
		uint32_t frame_done_fe2                  : 2;
		uint32_t frame_done_be                   : 2;
		uint32_t frame_done_raw                  : 1;
		uint32_t frame_done_rgb                  : 1;
		uint32_t frame_done_yuv                  : 1;
		uint32_t frame_done_post                 : 1;
		uint32_t shaw_done_fe0                   : 4;
		uint32_t shaw_done_fe1                   : 4;
		uint32_t shaw_done_fe2                   : 2;
		uint32_t shaw_done_be                    : 2;
		uint32_t shaw_done_raw                   : 1;
		uint32_t shaw_done_rgb                   : 1;
		uint32_t shaw_done_yuv                   : 1;
		uint32_t shaw_done_post                  : 1;
	} bits;
};

union reg_isp_top_int_event1 {
	uint32_t raw;
	struct {
		uint32_t pq_done_fe0                     : 4;
		uint32_t pq_done_fe1                     : 4;
		uint32_t pq_done_fe2                     : 2;
		uint32_t pq_done_be                      : 2;
		uint32_t pq_done_raw                     : 1;
		uint32_t pq_done_rgb                     : 1;
		uint32_t pq_done_yuv                     : 1;
		uint32_t pq_done_post                    : 1;
	} bits;
};

union reg_isp_top_int_event2 {
	uint32_t raw;
	struct {
		uint32_t frame_start_fe0                 : 4;
		uint32_t frame_start_fe1                 : 4;
		uint32_t frame_start_fe2                 : 2;
		uint32_t frame_err                       : 1;
		uint32_t pchk_err                        : 1;
		uint32_t cmdq_int                        : 1;
		uint32_t line_intp_fe0                   : 1;
		uint32_t line_intp_fe1                   : 1;
		uint32_t line_intp_fe2                   : 1;
		uint32_t line_intp_post                  : 1;
		uint32_t int_bdg0_lite                   : 1;
		uint32_t _rsv_18                         : 1;
		uint32_t int_dma_err                     : 1;
		uint32_t frame_err_line_spliter_fe0      : 1;
		uint32_t frame_err_line_spliter_fe1      : 1;
	} bits;
};

union reg_isp_top_error_sts {
	uint32_t raw;
	struct {
		uint32_t pchk0_err_fe0                   : 1;
		uint32_t pchk0_err_fe1                   : 1;
		uint32_t pchk0_err_fe2                   : 1;
		uint32_t pchk0_err_be                    : 1;
		uint32_t pchk0_err_raw                   : 1;
		uint32_t pchk0_err_rgb                   : 1;
		uint32_t pchk0_err_yuv                   : 1;
		uint32_t pchk1_err_fe0                   : 1;
		uint32_t pchk1_err_fe1                   : 1;
		uint32_t pchk1_err_fe2                   : 1;
		uint32_t pchk1_err_be                    : 1;
		uint32_t pchk1_err_raw                   : 1;
		uint32_t pchk1_err_rgb                   : 1;
		uint32_t pchk1_err_yuv                   : 1;
	} bits;
};

union reg_isp_top_int_event0_en {
	uint32_t raw;
	struct {
		uint32_t frame_done_enable_fe0           : 4;
		uint32_t frame_done_enable_fe1           : 4;
		uint32_t frame_done_enable_fe2           : 2;
		uint32_t frame_done_enable_be            : 2;
		uint32_t frame_done_enable_raw           : 1;
		uint32_t frame_done_enable_rgb           : 1;
		uint32_t frame_done_enable_yuv           : 1;
		uint32_t frame_done_enable_post          : 1;
		uint32_t shaw_done_enable_fe0            : 4;
		uint32_t shaw_done_enable_fe1            : 4;
		uint32_t shaw_done_enable_fe2            : 2;
		uint32_t shaw_done_enable_be             : 2;
		uint32_t shaw_done_enable_raw            : 1;
		uint32_t shaw_done_enable_rgb            : 1;
		uint32_t shaw_done_enable_yuv            : 1;
		uint32_t shaw_done_enable_post           : 1;
	} bits;
};

union reg_isp_top_int_event1_en {
	uint32_t raw;
	struct {
		uint32_t pq_done_enable_fe0              : 4;
		uint32_t pq_done_enable_fe1              : 4;
		uint32_t pq_done_enable_fe2              : 2;
		uint32_t pq_done_enable_be               : 2;
		uint32_t pq_done_enable_raw              : 1;
		uint32_t pq_done_enable_rgb              : 1;
		uint32_t pq_done_enable_yuv              : 1;
		uint32_t pq_done_enable_post             : 1;
	} bits;
};

union reg_isp_top_int_event2_en {
	uint32_t raw;
	struct {
		uint32_t frame_start_enable_fe0          : 4;
		uint32_t frame_start_enable_fe1          : 4;
		uint32_t frame_start_enable_fe2          : 2;
		uint32_t frame_err_enable                : 1;
		uint32_t pchk_err_enable                 : 1;
		uint32_t cmdq_int_enable                 : 1;
		uint32_t line_intp_enable_fe0            : 1;
		uint32_t line_intp_enable_fe1            : 1;
		uint32_t line_intp_enable_fe2            : 1;
		uint32_t line_intp_enable_post           : 1;
		uint32_t int_bdg0_lite_enable            : 1;
		uint32_t _rsv_18                         : 1;
		uint32_t int_dma_err_enable              : 1;
		uint32_t frame_err_line_spliter_enable_fe0: 1;
		uint32_t frame_err_line_spliter_enable_fe1: 1;
	} bits;
};

union reg_isp_top_sw_ctrl_0 {
	uint32_t raw;
	struct {
		uint32_t trig_str_fe0                    : 4;
		uint32_t trig_str_fe1                    : 4;
		uint32_t trig_str_fe2                    : 2;
		uint32_t trig_str_be                     : 2;
		uint32_t trig_str_raw                    : 1;
		uint32_t trig_str_post                   : 1;
		uint32_t _rsv_14                         : 2;
		uint32_t shaw_up_fe0                     : 4;
		uint32_t shaw_up_fe1                     : 4;
		uint32_t shaw_up_fe2                     : 2;
		uint32_t shaw_up_be                      : 2;
		uint32_t shaw_up_raw                     : 1;
		uint32_t shaw_up_post                    : 1;
	} bits;
};

union reg_isp_top_sw_ctrl_1 {
	uint32_t raw;
	struct {
		uint32_t pq_up_fe0                       : 4;
		uint32_t pq_up_fe1                       : 4;
		uint32_t pq_up_fe2                       : 2;
		uint32_t pq_up_be                        : 2;
		uint32_t pq_up_raw                       : 1;
		uint32_t pq_up_post                      : 1;
	} bits;
};

union reg_isp_top_ctrl_mode_sel0 {
	uint32_t raw;
	struct {
		uint32_t trig_str_sel_fe0                : 4;
		uint32_t trig_str_sel_fe1                : 4;
		uint32_t trig_str_sel_fe2                : 2;
		uint32_t trig_str_sel_be                 : 2;
		uint32_t trig_str_sel_raw                : 1;
		uint32_t trig_str_sel_post               : 1;
		uint32_t _rsv_14                         : 2;
		uint32_t shaw_up_sel_fe0                 : 4;
		uint32_t shaw_up_sel_fe1                 : 4;
		uint32_t shaw_up_sel_fe2                 : 2;
		uint32_t shaw_up_sel_be                  : 2;
		uint32_t shaw_up_sel_raw                 : 1;
		uint32_t shaw_up_sel_post                : 1;
	} bits;
};

union reg_isp_top_ctrl_mode_sel1 {
	uint32_t raw;
	struct {
		uint32_t pq_up_sel_fe0                   : 4;
		uint32_t pq_up_sel_fe1                   : 4;
		uint32_t pq_up_sel_fe2                   : 2;
		uint32_t pq_up_sel_be                    : 2;
		uint32_t pq_up_sel_raw                   : 1;
		uint32_t pq_up_sel_post                  : 1;
	} bits;
};

union reg_isp_top_scenarios_ctrl {
	uint32_t raw;
	struct {
		uint32_t dst2sc                          : 1;
		uint32_t dst2dma                         : 1;
		uint32_t pre2be_l_enable                 : 1;
		uint32_t pre2be_s_enable                 : 1;
		uint32_t pre2yuv_422_enable              : 1;
		uint32_t be2raw_l_enable                 : 1;
		uint32_t be2raw_s_enable                 : 1;
		uint32_t be_rdma_l_enable                : 1;
		uint32_t be_rdma_s_enable                : 1;
		uint32_t pre2dram2yuv_422_enable         : 1;
		uint32_t pre2dram2yuv_420_enable         : 1;
		uint32_t multi_sensor_enable             : 1;
		uint32_t ai_isp_enable                   : 1;
		uint32_t bt_lite_enable                  : 1;
		uint32_t _rsv_14                         : 3;
		uint32_t hdr_enable                      : 1;
		uint32_t hw_auto_enable                  : 1;
		uint32_t hw_auto_iso                     : 2;
		uint32_t _rsv_21                         : 1;
		uint32_t fe_dma_share_enable             : 1;
		uint32_t fe2_dma_share_enable            : 1;
		uint32_t be_src_sel                      : 3;
		uint32_t yuv_edge_en                     : 1;
		uint32_t yuv_in_sel_mode                 : 2;
		uint32_t yuv_format                      : 1;
		uint32_t yuv420_replay_enable            : 1;
	} bits;
};

union reg_isp_top_sw_rst {
	uint32_t raw;
	struct {
		uint32_t isp_rst                         : 1;
		uint32_t csi0_rst                        : 1;
		uint32_t csi1_rst                        : 1;
		uint32_t csi_be_rst                      : 1;
		uint32_t csi2_rst                        : 1;
		uint32_t bdg0_lite_rst                   : 1;
		uint32_t _rsv_6                          : 1;
		uint32_t axi_rst                         : 1;
		uint32_t cmdq_rst                        : 1;
		uint32_t apb_rst                         : 1;
		uint32_t raw_rst                         : 1;
	} bits;
};

union reg_isp_top_blk_idle {
	uint32_t raw;
	struct {
		uint32_t fe0_blk_idle                    : 1;
		uint32_t fe1_blk_idle                    : 1;
		uint32_t fe2_blk_idle                    : 1;
		uint32_t be_blk_idle                     : 1;
		uint32_t raw_blk_idle                    : 1;
		uint32_t rgb_blk_idle                    : 1;
		uint32_t yuv_blk_idle                    : 1;
		uint32_t rdma0_idle                      : 1;
		uint32_t wdma0_idle                      : 1;
		uint32_t wdma1_idle                      : 1;
	} bits;
};

union reg_isp_top_blk_idle_enable {
	uint32_t raw;
	struct {
		uint32_t blk_idle_csi0_en                : 1;
		uint32_t blk_idle_csi1_en                : 1;
		uint32_t blk_idle_csi2_en                : 1;
		uint32_t blk_idle_bdg0_lite_en           : 1;
		uint32_t _rsv_4                          : 1;
		uint32_t blk_idle_be_en                  : 1;
		uint32_t blk_idle_post_en                : 1;
		uint32_t blk_idle_apb_en                 : 1;
		uint32_t blk_idle_axi_en                 : 1;
		uint32_t blk_idle_cmdq_en                : 1;
		uint32_t blk_idle_raw_en                 : 1;
		uint32_t blk_idle_rgb_en                 : 1;
		uint32_t blk_idle_yuv_en                 : 1;
	} bits;
};

union reg_isp_top_dbus0 {
	uint32_t raw;
	struct {
		uint32_t dbus0                           : 32;
	} bits;
};

union reg_isp_top_dbus1 {
	uint32_t raw;
	struct {
		uint32_t dbus1                           : 32;
	} bits;
};

union reg_isp_top_dbus2 {
	uint32_t raw;
	struct {
		uint32_t dbus2                           : 32;
	} bits;
};

union reg_isp_top_dbus3 {
	uint32_t raw;
	struct {
		uint32_t dbus3                           : 32;
	} bits;
};

union reg_isp_top_force_int {
	uint32_t raw;
	struct {
		uint32_t force_isp_int                   : 1;
		uint32_t force_isp_int_en                : 1;
	} bits;
};

union reg_isp_top_dummy {
	uint32_t raw;
	struct {
		uint32_t dummy                           : 28;
		uint32_t dbus_sel                        : 4;
	} bits;
};

union reg_isp_top_dbus4 {
	uint32_t raw;
	struct {
		uint32_t dbus4                           : 32;
	} bits;
};

union reg_isp_top_ip_enable1 {
	uint32_t raw;
	struct {
		uint32_t raw_crop_l_enable               : 1;
		uint32_t raw_crop_s_enable               : 1;
		uint32_t raw_bnr_enable                  : 1;
		uint32_t raw_bnr_lite_enable             : 1;
		uint32_t raw_cfa_enable                  : 1;
		uint32_t raw_llsc_enable                 : 1;
		uint32_t raw_clsc_enable                 : 1;
		uint32_t raw_blc_0_enable                : 1;
		uint32_t raw_blc_1_enable                : 1;
		uint32_t raw_fusion_enable               : 1;
		uint32_t raw_map_curve_enable            : 1;
		uint32_t raw_ae_1_enable                 : 1;
		uint32_t raw_af_enable                   : 1;
		uint32_t raw_gms_enable                  : 1;
		uint32_t raw_dpc_enable                  : 1;
		uint32_t raw_drc_enable                  : 1;
		uint32_t _rsv_16                         : 4;
		uint32_t pfr_enable                      : 1;
		uint32_t rgb_ccm_enable                  : 1;
		uint32_t rgb_gamma_enable                : 1;
		uint32_t rgb_dhz_enable                  : 1;
		uint32_t rgb_rgbdither_enable            : 1;
		uint32_t rgb_clut_enable                 : 1;
		uint32_t rgb_r2y4_enable                 : 1;
		uint32_t sram_ee_ext_enable              : 1;
		uint32_t sram_pfr_enable                 : 1;
		uint32_t ee_ext_enable                   : 1;
		uint32_t raw_ae_0_enable                 : 2;
	} bits;
};

union reg_isp_top_ip_enable2 {
	uint32_t raw;
	struct {
		uint32_t yuv_ee_f_enable                 : 1;
		uint32_t yuv_ee_b_enable                 : 1;
		uint32_t yuv_3dnr_enable                 : 1;
		uint32_t yuv_cnr_enable                  : 1;
		uint32_t yuv_postee_enable               : 1;
		uint32_t yuv_ycrop_enable                : 1;
		uint32_t yuv_uvcrop_enable               : 1;
		uint32_t yuv_ycurve_enable               : 1;
		uint32_t yuv_ca_lite_enable              : 1;
		uint32_t yuv_ca_enable                   : 1;
		uint32_t yuv_ldci_enable                 : 1;
		uint32_t resize_enable                   : 1;
	} bits;
};

union reg_isp_top_ip_enable3 {
	uint32_t raw;
	struct {
		uint32_t csi0_ae_l_enable                : 1;
		uint32_t csi0_ae_s_enable                : 1;
		uint32_t csi0_clsc_l_enable              : 1;
		uint32_t csi0_clsc_s_enable              : 1;
		uint32_t csi1_ae_l_enable                : 1;
		uint32_t csi1_ae_s_enable                : 1;
		uint32_t csi1_clsc_l_enable              : 1;
		uint32_t csi1_clsc_s_enable              : 1;
		uint32_t csi2_ae_l_enable                : 1;
		uint32_t csi2_ae_s_enable                : 1;
		uint32_t csi2_clsc_l_enable              : 1;
		uint32_t csi2_clsc_s_enable              : 1;
	} bits;
};

union reg_isp_top_cmdq_ctrl {
	uint32_t raw;
	struct {
		uint32_t cmdq_tsk_en                     : 8;
		uint32_t cmdq_flag_sel                   : 2;
		uint32_t cmdq_task_sel                   : 2;
	} bits;
};

union reg_isp_top_cmdq_trig {
	uint32_t raw;
	struct {
		uint32_t cmdq_tsk_trig                   : 8;
	} bits;
};

union reg_isp_top_trig_cnt {
	uint32_t raw;
	struct {
		uint32_t trig_str_cnt                    : 4;
		uint32_t vsync_delay                     : 4;
	} bits;
};

union reg_isp_top_svn_version {
	uint32_t raw;
	struct {
		uint32_t svn_revision                    : 32;
	} bits;
};

union reg_isp_top_timestamp {
	uint32_t raw;
	struct {
		uint32_t unix_timestamp                  : 32;
	} bits;
};

union reg_isp_top_sclie_enable {
	uint32_t raw;
	struct {
		uint32_t slice_enable_main_lexp          : 1;
		uint32_t slice_enable_main_sexp          : 1;
	} bits;
};

union reg_isp_top_w_slice_thresh_main {
	uint32_t raw;
	struct {
		uint32_t w_slice_thr_main_lexp           : 16;
		uint32_t w_slice_thr_main_sexp           : 16;
	} bits;
};

union reg_isp_top_r_slice_thresh_main {
	uint32_t raw;
	struct {
		uint32_t r_slice_thr_main_lexp           : 16;
		uint32_t r_slice_thr_main_sexp           : 16;
	} bits;
};

union reg_isp_top_vi_sel_frame_valid {
	uint32_t raw;
	struct {
		uint32_t vi_sel_frame_vld_le             : 1;
		uint32_t vi_sel_pq_vld_le                : 1;
		uint32_t vi_sel_frame_vld_se             : 1;
		uint32_t vi_sel_pq_vld_se                : 1;
	} bits;
};

union reg_isp_top_first_frame {
	uint32_t raw;
	struct {
		uint32_t first_frame_sw                  : 4;
		uint32_t first_frame_top                 : 4;
	} bits;
};

union reg_isp_top_int_event0_line_spliter {
	uint32_t raw;
	struct {
		uint32_t line_spliter_dma_done_fe0       : 4;
		uint32_t line_spliter_dma_done_fe1       : 4;
	} bits;
};

union reg_isp_top_int_event0_en_line_spliter {
	uint32_t raw;
	struct {
		uint32_t line_spliter_dma_done_enable_fe0: 4;
		uint32_t line_spliter_dma_done_enable_fe1: 4;
	} bits;
};

union reg_isp_top_blk_idle_1 {
	uint32_t raw;
	struct {
		uint32_t rdma1_idle                      : 1;
		uint32_t wdma2_idle                      : 1;
		uint32_t rdma2_idle                      : 1;
		uint32_t line_spliter_fe0_dma_idle       : 4;
		uint32_t line_spliter_fe1_dma_idle       : 4;
		uint32_t line_spliter_fe0_fifo_overflow_status: 1;
		uint32_t line_spliter_fe1_fifo_overflow_status: 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_wdma_core_shadow_rd_sel   {
	uint32_t raw;
	struct {
		uint32_t shadow_rd_sel                   : 1;
		uint32_t abort_mode                      : 1;
	} bits;
};

union reg_wdma_core_ip_disable {
	uint32_t raw;
	struct {
		uint32_t ip_disable                      : 32;
	} bits;
};

union reg_wdma_core_disable_seglen {
	uint32_t raw;
	struct {
		uint32_t seglen_disable                  : 32;
	} bits;
};

union reg_wdma_core_up_ring_base {
	uint32_t raw;
	struct {
		uint32_t up_ring_base                    : 32;
	} bits;
};

union reg_wdma_core_norm_status0 {
	uint32_t raw;
	struct {
		uint32_t abort_done                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t error_axi                       : 1;
		uint32_t error_dmi                       : 1;
		uint32_t slot_full                       : 1;
		uint32_t _rsv_7                          : 1;
		uint32_t error_id                        : 5;
		uint32_t _rsv_13                         : 3;
		uint32_t dma_version                     : 16;
	} bits;
};

union reg_wdma_core_norm_status1 {
	uint32_t raw;
	struct {
		uint32_t id_idle                         : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_en {
	uint32_t raw;
	struct {
		uint32_t ring_enable                     : 32;
	} bits;
};

union reg_wdma_core_norm_perf  {
	uint32_t raw;
	struct {
		uint32_t bwlwin                          : 10;
		uint32_t bwltxn                          : 6;
		uint32_t qoso_th                         : 4;
		uint32_t qoso_en                         : 1;
	} bits;
};

union reg_wdma_core_ring_patch_enable {
	uint32_t raw;
	struct {
		uint32_t ring_patch_enable               : 32;
	} bits;
};

union reg_wdma_core_set_ring_base {
	uint32_t raw;
	struct {
		uint32_t set_ring_base                   : 32;
	} bits;
};

union reg_wdma_core_ring_base_addr_l {
	uint32_t raw;
	struct {
		uint32_t ring_base_l                     : 32;
	} bits;
};

union reg_wdma_core_ring_base_addr_h {
	uint32_t raw;
	struct {
		uint32_t ring_base_h                     : 16;
	} bits;
};

union reg_wdma_core_ring_buffer_size0 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size0                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size1 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size1                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size2 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size2                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size3 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size3                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size4 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size4                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size5 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size5                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size6 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size6                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size7 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size7                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size8 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size8                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size9 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size9                      : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size10 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size10                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size11 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size11                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size12 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size12                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size13 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size13                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size14 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size14                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size15 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size15                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size16 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size16                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size17 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size17                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size18 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size18                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size19 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size19                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size20 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size20                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size21 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size21                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size22 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size22                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size23 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size23                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size24 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size24                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size25 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size25                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size26 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size26                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size27 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size27                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size28 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size28                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size29 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size29                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size30 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size30                     : 32;
	} bits;
};

union reg_wdma_core_ring_buffer_size31 {
	uint32_t raw;
	struct {
		uint32_t rbuf_size31                     : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts0 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr0                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts1 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr1                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts2 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr2                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts3 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr3                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts4 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr4                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts5 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr5                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts6 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr6                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts7 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr7                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts8 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr8                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts9 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr9                  : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts10 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr10                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts11 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr11                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts12 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr12                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts13 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr13                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts14 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr14                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts15 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr15                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts16 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr16                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts17 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr17                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts18 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr18                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts19 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr19                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts20 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr20                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts21 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr21                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts22 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr22                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts23 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr23                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts24 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr24                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts25 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr25                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts26 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr26                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts27 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr27                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts28 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr28                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts29 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr29                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts30 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr30                 : 32;
	} bits;
};

union reg_wdma_core_next_dma_addr_sts31 {
	uint32_t raw;
	struct {
		uint32_t next_dma_addr31                 : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_ldci_ldci_ctl {
	uint32_t raw;
	struct {
		uint32_t ldci_enable                     : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t dci_enable                      : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t ldci_luma_prot_en               : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t bypass_pipectrl                 : 1;
		uint32_t _rsv_17                         : 7;
		uint32_t ldci_pipe_rst                   : 1;
		uint32_t _rsv_25                         : 3;
		uint32_t ldci_pipe_gclk_en               : 1;
	} bits;
};

union reg_ldci_ldci_reciprocal_0 {
	uint32_t raw;
	struct {
		uint32_t ldci_reciprocal_v               : 16;
		uint32_t ldci_reciprocal_h               : 16;
	} bits;
};

union reg_ldci_ldci_luma_prot_lut0 {
	uint32_t raw;
	struct {
		uint32_t ldci_luma_prot_lut_0            : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ldci_luma_prot_lut_1            : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ldci_luma_prot_lut_2            : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ldci_luma_prot_lut_3            : 6;
	} bits;
};

union reg_ldci_ldci_luma_prot_lut1 {
	uint32_t raw;
	struct {
		uint32_t ldci_luma_prot_lut_4            : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ldci_luma_prot_lut_5            : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ldci_luma_prot_lut_6            : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ldci_luma_prot_lut_7            : 6;
	} bits;
};

union reg_ldci_ldci_luma_prot_lut2 {
	uint32_t raw;
	struct {
		uint32_t ldci_luma_prot_lut_8            : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ldci_luma_prot_lut_9            : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ldci_luma_prot_lut_10           : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ldci_luma_prot_lut_11           : 6;
	} bits;
};

union reg_ldci_ldci_luma_prot_lut3 {
	uint32_t raw;
	struct {
		uint32_t ldci_luma_prot_lut_12           : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ldci_luma_prot_lut_13           : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ldci_luma_prot_lut_14           : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t ldci_luma_prot_lut_15           : 6;
	} bits;
};

union reg_ldci_ldci_luma_prot_lut4 {
	uint32_t raw;
	struct {
		uint32_t ldci_luma_prot_lut_16           : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ldci_luma_prot_cw               : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t ldci_luma_prot_bw               : 4;
	} bits;
};

union reg_ldci_ldci_blk_num {
	uint32_t raw;
	struct {
		uint32_t ldci_blk_num_h                  : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t ldci_blk_num_v                  : 6;
	} bits;
};

union reg_ldci_ldci_blk_size {
	uint32_t raw;
	struct {
		uint32_t ldci_blk_width                  : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t ldci_blk_height                 : 12;
	} bits;
};

union reg_ldci_ldci_reciprocal_1 {
	uint32_t raw;
	struct {
		uint32_t ldci_reciprocal                 : 27;
	} bits;
};

union reg_ldci_ldci_luma_gain_lut0 {
	uint32_t raw;
	struct {
		uint32_t ldci_luma_gain_lut_0            : 8;
		uint32_t ldci_luma_gain_lut_1            : 8;
		uint32_t ldci_luma_gain_lut_2            : 8;
		uint32_t ldci_luma_gain_lut_3            : 8;
	} bits;
};

union reg_ldci_ldci_luma_gain_lut1 {
	uint32_t raw;
	struct {
		uint32_t ldci_luma_gain_lut_4            : 8;
		uint32_t ldci_luma_gain_lut_5            : 8;
		uint32_t ldci_luma_gain_lut_6            : 8;
		uint32_t ldci_luma_gain_lut_7            : 8;
	} bits;
};

union reg_ldci_ldci_luma_gain_lut2 {
	uint32_t raw;
	struct {
		uint32_t ldci_luma_gain_lut_8            : 8;
	} bits;
};

union reg_ldci_ldci_thr {
	uint32_t raw;
	struct {
		uint32_t ldci_threshold1                 : 8;
	} bits;
};

union reg_ldci_ldci_diff_gain_lut_l0 {
	uint32_t raw;
	struct {
		uint32_t ldci_diff_gain_lut_l_0          : 8;
		uint32_t ldci_diff_gain_lut_l_1          : 8;
		uint32_t ldci_diff_gain_lut_l_2          : 8;
		uint32_t ldci_diff_gain_lut_l_3          : 8;
	} bits;
};

union reg_ldci_ldci_diff_gain_lut_l1 {
	uint32_t raw;
	struct {
		uint32_t ldci_diff_gain_lut_l_4          : 8;
	} bits;
};

union reg_ldci_ldci_diff_gain_lut_r0 {
	uint32_t raw;
	struct {
		uint32_t ldci_diff_gain_lut_r_0          : 8;
		uint32_t ldci_diff_gain_lut_r_1          : 8;
		uint32_t ldci_diff_gain_lut_r_2          : 8;
		uint32_t ldci_diff_gain_lut_r_3          : 8;
	} bits;
};

union reg_ldci_ldci_diff_gain_lut_r1 {
	uint32_t raw;
	struct {
		uint32_t ldci_diff_gain_lut_r_4          : 8;
	} bits;
};

union reg_ldci_ldci_roi_ctl {
	uint32_t raw;
	struct {
		uint32_t dci_histsample_step             : 5;
		uint32_t _rsv_5                          : 3;
		uint32_t dci_roi_enable                  : 1;
	} bits;
};

union reg_ldci_ldci_roi_axis_0 {
	uint32_t raw;
	struct {
		uint32_t dci_roi_start_x                 : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t dci_roi_start_y                 : 13;
	} bits;
};

union reg_ldci_ldci_roi_axis_1 {
	uint32_t raw;
	struct {
		uint32_t dci_roi_width                   : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t dci_roi_height                  : 13;
	} bits;
};

union reg_ldci_ldci_hw_ctl {
	uint32_t raw;
	struct {
		uint32_t ldci_force_ck_en                : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t ldci_shdw_sel                   : 1;
	} bits;
};

union reg_ldci_ldci_dma_stat {
	uint32_t raw;
	struct {
		uint32_t ldci_lmap_rdma_idle             : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t ldci_lmap_wdma_idle             : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t ldci_hist_wdma_idle             : 1;
	} bits;
};

union reg_ldci_ldci_crop_width {
	uint32_t raw;
	struct {
		uint32_t ldci_crop_w_str                 : 16;
		uint32_t ldci_crop_w_end                 : 16;
	} bits;
};

union reg_ldci_ldci_crop_height {
	uint32_t raw;
	struct {
		uint32_t ldci_crop_h_str                 : 16;
		uint32_t ldci_crop_h_end                 : 16;
	} bits;
};

union reg_ldci_ldci_crop_enable {
	uint32_t raw;
	struct {
		uint32_t ldci_crop_enable                : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_ldci_map_lut_ldci_map_ctl {
	uint32_t raw;
	struct {
		uint32_t shdw_sel                        : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t map_en                          : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t map_prog_hdk_dis                : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t ck_enable                       : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t lut_prog_en                     : 1;
	} bits;
};

union reg_ldci_map_lut_ldci_rw_ctl {
	uint32_t raw;
	struct {
		uint32_t lut_wsel                        : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t lut_rsel                        : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t lut_w1t                         : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t lut_st_w1t                      : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t lut_wdata                       : 8;
		uint32_t lut_waddr                       : 8;
	} bits;
};

union reg_ldci_map_lut_ldci_map_sw_ctl {
	uint32_t raw;
	struct {
		uint32_t sw_lut_r_w1t                    : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t sw_lut_raddr                    : 8;
		uint32_t sw_lut_rsel                     : 1;
		uint32_t _rsv_17                         : 7;
		uint32_t sw_lut_rdata                    : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_map_curve_01 {
	uint32_t raw;
	struct {
		uint32_t u1_fcurve16_en                  : 1;
		uint32_t u2_fcurve16_ysel                : 2;
		uint32_t _rsv_3                          : 5;
		uint32_t u4_fcurve16_yvwet               : 4;
		uint32_t _rsv_12                         : 4;
		uint32_t u4_ife_f_fcurve_ev_fmt          : 4;
	} bits;
};

union reg_map_curve_fcurve_flumw_lut0 {
	uint32_t raw;
	struct {
		uint32_t u8_fcurve16_flumw_lut17_0       : 8;
		uint32_t u8_fcurve16_flumw_lut17_1       : 8;
		uint32_t u8_fcurve16_flumw_lut17_2       : 8;
		uint32_t u8_fcurve16_flumw_lut17_3       : 8;
	} bits;
};

union reg_map_curve_fcurve_flumw_lut1 {
	uint32_t raw;
	struct {
		uint32_t u8_fcurve16_flumw_lut17_4       : 8;
		uint32_t u8_fcurve16_flumw_lut17_5       : 8;
		uint32_t u8_fcurve16_flumw_lut17_6       : 8;
		uint32_t u8_fcurve16_flumw_lut17_7       : 8;
	} bits;
};

union reg_map_curve_fcurve_flumw_lut2 {
	uint32_t raw;
	struct {
		uint32_t u8_fcurve16_flumw_lut17_8       : 8;
		uint32_t u8_fcurve16_flumw_lut17_9       : 8;
		uint32_t u8_fcurve16_flumw_lut17_10      : 8;
		uint32_t u8_fcurve16_flumw_lut17_11      : 8;
	} bits;
};

union reg_map_curve_fcurve_flumw_lut3 {
	uint32_t raw;
	struct {
		uint32_t u8_fcurve16_flumw_lut17_12      : 8;
		uint32_t u8_fcurve16_flumw_lut17_13      : 8;
		uint32_t u8_fcurve16_flumw_lut17_14      : 8;
		uint32_t u8_fcurve16_flumw_lut17_15      : 8;
	} bits;
};

union reg_map_curve_fcurve_flumw_lut4 {
	uint32_t raw;
	struct {
		uint32_t u8_fcurve16_flumw_lut17_16      : 8;
	} bits;
};

union reg_map_curve_fcurve_l0 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_0            : 24;
	} bits;
};

union reg_map_curve_fcurve_l1 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_1            : 24;
	} bits;
};

union reg_map_curve_fcurve_l2 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_2            : 24;
	} bits;
};

union reg_map_curve_fcurve_l3 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_3            : 24;
	} bits;
};

union reg_map_curve_fcurve_l4 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_4            : 24;
	} bits;
};

union reg_map_curve_fcurve_l5 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_5            : 24;
	} bits;
};

union reg_map_curve_fcurve_l6 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_6            : 24;
	} bits;
};

union reg_map_curve_fcurve_l7 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_7            : 24;
	} bits;
};

union reg_map_curve_fcurve_l8 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_8            : 24;
	} bits;
};

union reg_map_curve_fcurve_l9 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_9            : 24;
	} bits;
};

union reg_map_curve_fcurve_l10 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_10           : 24;
	} bits;
};

union reg_map_curve_fcurve_l11 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_11           : 24;
	} bits;
};

union reg_map_curve_fcurve_l12 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_12           : 24;
	} bits;
};

union reg_map_curve_fcurve_l13 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_13           : 24;
	} bits;
};

union reg_map_curve_fcurve_l14 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_14           : 24;
	} bits;
};

union reg_map_curve_fcurve_l15 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_15           : 24;
	} bits;
};

union reg_map_curve_fcurve_l16 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_16           : 24;
	} bits;
};

union reg_map_curve_fcurve_l17 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_17           : 24;
	} bits;
};

union reg_map_curve_fcurve_l18 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_18           : 24;
	} bits;
};

union reg_map_curve_fcurve_l19 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_19           : 24;
	} bits;
};

union reg_map_curve_fcurve_l20 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_20           : 24;
	} bits;
};

union reg_map_curve_fcurve_l21 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_21           : 24;
	} bits;
};

union reg_map_curve_fcurve_l22 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_22           : 24;
	} bits;
};

union reg_map_curve_fcurve_l23 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_23           : 24;
	} bits;
};

union reg_map_curve_fcurve_l24 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_24           : 24;
	} bits;
};

union reg_map_curve_fcurve_l25 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_25           : 24;
	} bits;
};

union reg_map_curve_fcurve_l26 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_26           : 24;
	} bits;
};

union reg_map_curve_fcurve_l27 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_27           : 24;
	} bits;
};

union reg_map_curve_fcurve_l28 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_28           : 24;
	} bits;
};

union reg_map_curve_fcurve_l29 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_29           : 24;
	} bits;
};

union reg_map_curve_fcurve_l30 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_30           : 24;
	} bits;
};

union reg_map_curve_fcurve_l31 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_31           : 24;
	} bits;
};

union reg_map_curve_fcurve_l32 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_32           : 24;
	} bits;
};

union reg_map_curve_fcurve_l33 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_33           : 24;
	} bits;
};

union reg_map_curve_fcurve_l34 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_34           : 24;
	} bits;
};

union reg_map_curve_fcurve_l35 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_35           : 24;
	} bits;
};

union reg_map_curve_fcurve_l36 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_36           : 24;
	} bits;
};

union reg_map_curve_fcurve_l37 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_37           : 24;
	} bits;
};

union reg_map_curve_fcurve_l38 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_38           : 24;
	} bits;
};

union reg_map_curve_fcurve_l39 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_39           : 24;
	} bits;
};

union reg_map_curve_fcurve_l40 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_40           : 24;
	} bits;
};

union reg_map_curve_fcurve_l41 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_41           : 24;
	} bits;
};

union reg_map_curve_fcurve_l42 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_42           : 24;
	} bits;
};

union reg_map_curve_fcurve_l43 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_43           : 24;
	} bits;
};

union reg_map_curve_fcurve_l44 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_44           : 24;
	} bits;
};

union reg_map_curve_fcurve_l45 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_45           : 24;
	} bits;
};

union reg_map_curve_fcurve_l46 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_46           : 24;
	} bits;
};

union reg_map_curve_fcurve_l47 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_47           : 24;
	} bits;
};

union reg_map_curve_fcurve_l48 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_48           : 24;
	} bits;
};

union reg_map_curve_fcurve_l49 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_49           : 24;
	} bits;
};

union reg_map_curve_fcurve_l50 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_50           : 24;
	} bits;
};

union reg_map_curve_fcurve_l51 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_51           : 24;
	} bits;
};

union reg_map_curve_fcurve_l52 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_52           : 24;
	} bits;
};

union reg_map_curve_fcurve_l53 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_53           : 24;
	} bits;
};

union reg_map_curve_fcurve_l54 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_54           : 24;
	} bits;
};

union reg_map_curve_fcurve_l55 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_55           : 24;
	} bits;
};

union reg_map_curve_fcurve_l56 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_56           : 24;
	} bits;
};

union reg_map_curve_fcurve_l57 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_57           : 24;
	} bits;
};

union reg_map_curve_fcurve_l58 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_58           : 24;
	} bits;
};

union reg_map_curve_fcurve_l59 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_59           : 24;
	} bits;
};

union reg_map_curve_fcurve_l60 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_60           : 24;
	} bits;
};

union reg_map_curve_fcurve_l61 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_61           : 24;
	} bits;
};

union reg_map_curve_fcurve_l62 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_62           : 24;
	} bits;
};

union reg_map_curve_fcurve_l63 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_63           : 24;
	} bits;
};

union reg_map_curve_fcurve_l64 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_l_64           : 24;
	} bits;
};

union reg_map_curve_fcurve_r0 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_0            : 24;
	} bits;
};

union reg_map_curve_fcurve_r1 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_1            : 24;
	} bits;
};

union reg_map_curve_fcurve_r2 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_2            : 24;
	} bits;
};

union reg_map_curve_fcurve_r3 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_3            : 24;
	} bits;
};

union reg_map_curve_fcurve_r4 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_4            : 24;
	} bits;
};

union reg_map_curve_fcurve_r5 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_5            : 24;
	} bits;
};

union reg_map_curve_fcurve_r6 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_6            : 24;
	} bits;
};

union reg_map_curve_fcurve_r7 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_7            : 24;
	} bits;
};

union reg_map_curve_fcurve_r8 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_8            : 24;
	} bits;
};

union reg_map_curve_fcurve_r9 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_9            : 24;
	} bits;
};

union reg_map_curve_fcurve_r10 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_10           : 24;
	} bits;
};

union reg_map_curve_fcurve_r11 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_11           : 24;
	} bits;
};

union reg_map_curve_fcurve_r12 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_12           : 24;
	} bits;
};

union reg_map_curve_fcurve_r13 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_13           : 24;
	} bits;
};

union reg_map_curve_fcurve_r14 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_14           : 24;
	} bits;
};

union reg_map_curve_fcurve_r15 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_15           : 24;
	} bits;
};

union reg_map_curve_fcurve_r16 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_r_16           : 24;
	} bits;
};

union reg_map_curve_fcurve_end0 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_0          : 24;
	} bits;
};

union reg_map_curve_fcurve_end1 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_1          : 24;
	} bits;
};

union reg_map_curve_fcurve_end2 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_2          : 24;
	} bits;
};

union reg_map_curve_fcurve_end3 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_3          : 24;
	} bits;
};

union reg_map_curve_fcurve_end4 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_4          : 24;
	} bits;
};

union reg_map_curve_fcurve_end5 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_5          : 24;
	} bits;
};

union reg_map_curve_fcurve_end6 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_6          : 24;
	} bits;
};

union reg_map_curve_fcurve_end7 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_7          : 24;
	} bits;
};

union reg_map_curve_fcurve_end8 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_8          : 24;
	} bits;
};

union reg_map_curve_fcurve_end9 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_9          : 24;
	} bits;
};

union reg_map_curve_fcurve_end10 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_10         : 24;
	} bits;
};

union reg_map_curve_fcurve_end11 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_11         : 24;
	} bits;
};

union reg_map_curve_fcurve_end12 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_12         : 24;
	} bits;
};

union reg_map_curve_fcurve_end13 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_13         : 24;
	} bits;
};

union reg_map_curve_fcurve_end14 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_14         : 24;
	} bits;
};

union reg_map_curve_fcurve_end15 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_15         : 24;
	} bits;
};

union reg_map_curve_fcurve_end16 {
	uint32_t raw;
	struct {
		uint32_t u24_ife_f_fcurve_end_16         : 24;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_444_422_0 {
	uint32_t raw;
	struct {
		uint32_t first_frame_reset               : 1;
		uint32_t tdnr_enable                     : 2;
		uint32_t _rsv_3                          : 1;
		uint32_t dma_crop_enable                 : 1;
		uint32_t force_mono_enable               : 1;
		uint32_t debug_status_en                 : 1;
		uint32_t dma_enable                      : 11;
		uint32_t bypass                          : 1;
		uint32_t shdw_read_sel                   : 1;
		uint32_t _rsv_20                         : 1;
		uint32_t hw_auto_cg_en_blend             : 1;
		uint32_t hw_auto_cg_en_all               : 1;
		uint32_t _rsv_23                         : 1;
		uint32_t guard_cnt                       : 8;
	} bits;
};

union reg_isp_444_422_1 {
	uint32_t raw;
	struct {
		uint32_t tdnr_debug_status               : 32;
	} bits;
};

union reg_isp_444_422_2 {
	uint32_t raw;
	struct {
		uint32_t tile_mode_en                    : 1;
		uint32_t dma_sel                         : 1;
		uint32_t bypass_v                        : 1;
		uint32_t bypass_h                        : 1;
		uint32_t _rsv_4                          : 1;
		uint32_t avg_mode_read                   : 1;
		uint32_t avg_mode_write                  : 1;
		uint32_t drop_mode_write                 : 1;
		uint32_t _rsv_8                          : 1;
		uint32_t chroma_off_for_fbcd             : 1;
		uint32_t _rsv_10                         : 1;
		uint32_t reg_3dnr_seed_reset_en          : 1;
		uint32_t _rsv_12                         : 4;
		uint32_t tdnr_debug_cnt                  : 16;
	} bits;
};

union reg_isp_444_422_3 {
	uint32_t raw;
	struct {
		uint32_t img_width_crop                  : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t img_height_crop                 : 14;
	} bits;
};

union reg_isp_444_422_4 {
	uint32_t raw;
	struct {
		uint32_t crop_w_str                      : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t crop_w_end                      : 14;
	} bits;
};

union reg_isp_444_422_5 {
	uint32_t raw;
	struct {
		uint32_t crop_h_str                      : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t crop_h_end                      : 14;
	} bits;
};

union reg_isp_444_422_6 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_debug_mode             : 4;
		uint32_t reg_3dnr_subpixel_enable        : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t reg_3dnr_y_l0_pss_blur          : 8;
		uint32_t _rsv_16                         : 1;
		uint32_t reg_3dnr_luma_jnd_ratio         : 8;
		uint32_t _rsv_25                         : 1;
		uint32_t reg_3dnr_sad_lpf_mode           : 2;
	} bits;
};

union reg_isp_444_422_7 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_std_u14_0          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_jnd_std_u14_1          : 14;
	} bits;
};

union reg_isp_444_422_8 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_std_u14_2          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_jnd_std_u14_3          : 14;
	} bits;
};

union reg_isp_444_422_9 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_std_u14_4          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_jnd_std_u14_5          : 14;
	} bits;
};

union reg_isp_444_422_10 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_std_u14_6          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_jnd_std_u14_7          : 14;
	} bits;
};

union reg_isp_444_422_11 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_edge_k_u6_0        : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t reg_3dnr_jnd_edge_k_u6_1        : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t reg_3dnr_jnd_edge_k_u6_2        : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t reg_3dnr_jnd_edge_k_u6_3        : 6;
	} bits;
};

union reg_isp_444_422_12 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_edge_k_u6_4        : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t reg_3dnr_jnd_edge_k_u6_5        : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t reg_3dnr_jnd_edge_k_u6_6        : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t reg_3dnr_jnd_edge_k_u6_7        : 6;
	} bits;
};

union reg_isp_444_422_13 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_b_u14_0            : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_jnd_b_u14_1            : 14;
	} bits;
};

union reg_isp_444_422_14 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_b_u14_2            : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_jnd_b_u14_3            : 14;
	} bits;
};

union reg_isp_444_422_15 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_b_u14_4            : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_jnd_b_u14_5            : 14;
	} bits;
};

union reg_isp_444_422_16 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_b_u14_6            : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_jnd_b_u14_7            : 14;
	} bits;
};

union reg_isp_444_422_17 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_std_u14_0         : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_std_u14_1         : 14;
	} bits;
};

union reg_isp_444_422_18 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_std_u14_2         : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_std_u14_3         : 14;
	} bits;
};

union reg_isp_444_422_19 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_std_u14_4         : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_std_u14_5         : 14;
	} bits;
};

union reg_isp_444_422_20 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_std_u14_6         : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_std_u14_7         : 14;
	} bits;
};

union reg_isp_444_422_21 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_b0_u14_0          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_b0_u14_1          : 14;
	} bits;
};

union reg_isp_444_422_22 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_b0_u14_2          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_b0_u14_3          : 14;
	} bits;
};

union reg_isp_444_422_23 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_b0_u14_4          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_b0_u14_5          : 14;
	} bits;
};

union reg_isp_444_422_24 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_b0_u14_6          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_b0_u14_7          : 14;
	} bits;
};

union reg_isp_444_422_25 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_edge_k_u6_0       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t reg_3dnr_warp_edge_k_u6_1       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t reg_3dnr_warp_edge_k_u6_2       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t reg_3dnr_warp_edge_k_u6_3       : 6;
	} bits;
};

union reg_isp_444_422_26 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_edge_k_u6_4       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t reg_3dnr_warp_edge_k_u6_5       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t reg_3dnr_warp_edge_k_u6_6       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t reg_3dnr_warp_edge_k_u6_7       : 6;
	} bits;
};

union reg_isp_444_422_27 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_b_u14_0           : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_b_u14_1           : 14;
	} bits;
};

union reg_isp_444_422_28 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_b_u14_2           : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_b_u14_3           : 14;
	} bits;
};

union reg_isp_444_422_29 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_b_u14_4           : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_b_u14_5           : 14;
	} bits;
};

union reg_isp_444_422_30 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_warp_b_u14_6           : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t reg_3dnr_warp_b_u14_7           : 14;
	} bits;
};

union reg_isp_444_422_31 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_acbd_sad_adj_u10_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t reg_3dnr_acbd_sad_adj_u10_1     : 10;
	} bits;
};

union reg_isp_444_422_32 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_acbd_sad_adj_u10_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t reg_3dnr_acbd_sad_adj_u10_3     : 10;
	} bits;
};

union reg_isp_444_422_33 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_luma_jnd_th_u8_0       : 8;
		uint32_t reg_3dnr_luma_jnd_th_u8_1       : 8;
		uint32_t reg_3dnr_luma_jnd_th_u8_2       : 8;
		uint32_t reg_3dnr_luma_jnd_th_u8_3       : 8;
	} bits;
};

union reg_isp_444_422_34 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_luma_jnd_th_u8_4       : 8;
		uint32_t reg_3dnr_luma_jnd_th_u8_5       : 8;
		uint32_t reg_3dnr_luma_jnd_th_u8_6       : 8;
		uint32_t reg_3dnr_luma_jnd_th_u8_7       : 8;
	} bits;
};

union reg_isp_444_422_35 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_jnd_sad_wt             : 4;
		uint32_t reg_3dnr_edge_th                : 14;
		uint32_t reg_3dnr_a_mutual_th            : 14;
	} bits;
};

union reg_isp_444_422_36 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_acbd_edge_adj_u4_0     : 4;
		uint32_t reg_3dnr_acbd_edge_adj_u4_1     : 4;
		uint32_t reg_3dnr_acbd_edge_adj_u4_2     : 4;
		uint32_t reg_3dnr_acbd_edge_adj_u4_3     : 4;
		uint32_t _rsv_16                         : 1;
		uint32_t reg_3dnr_me_boundary_set        : 1;
		uint32_t reg_3dnr_rand_search_mode       : 1;
		uint32_t _rsv_19                         : 1;
		uint32_t reg_3dnr_me_rand_bit_y          : 3;
		uint32_t reg_3dnr_me_rand_bit_x          : 3;
		uint32_t _rsv_26                         : 1;
		uint32_t reg_3dnr_dbg_random0            : 1;
	} bits;
};

union reg_isp_444_422_37 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_stable_intensity_u1    : 8;
		uint32_t reg_3dnr_flow_jnd_mode          : 2;
		uint32_t reg_3dnr_flow_norm_mode         : 2;
		uint32_t _rsv_12                         : 1;
		uint32_t reg_3dnr_dynamic_blur_enable    : 1;
		uint32_t _rsv_14                         : 1;
		uint32_t reg_3dnr_dynamic_blur_jnd_th    : 5;
		uint32_t _rsv_20                         : 2;
		uint32_t reg_3dnr_dynamic_blur_st        : 4;
		uint32_t _rsv_26                         : 2;
		uint32_t reg_3dnr_dynamic_blur_step      : 4;
	} bits;
};

union reg_isp_444_422_38 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l0_st_jnd_sigma_u8_0 : 8;
		uint32_t reg_3dnr_y_l0_st_jnd_sigma_u8_1 : 8;
		uint32_t reg_3dnr_y_l0_st_jnd_sigma_u8_2 : 8;
	} bits;
};

union reg_isp_444_422_39 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l0_ss_jnd_blur_u8_0  : 8;
		uint32_t reg_3dnr_y_l0_ss_jnd_blur_u8_1  : 8;
		uint32_t reg_3dnr_y_l0_ss_jnd_blur_u8_2  : 8;
	} bits;
};

union reg_isp_444_422_40 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l0_ss_grad_blur_th   : 16;
		uint32_t reg_3dnr_y_l0_ss_grad_blur_str  : 8;
		uint32_t reg_3dnr_y_l0_st_center_zero_en : 1;
	} bits;
};

union reg_isp_444_422_41 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_sad_neighbor_th        : 14;
		uint32_t reg_3dnr_neighbor_ratio_mode    : 1;
		uint32_t reg_3dnr_neighbor_blend_ratio_u8_0: 8;
		uint32_t reg_3dnr_neighbor_blend_ratio_u8_1: 8;
	} bits;
};

union reg_isp_444_422_42 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_flow_neighbor_th_u14_0 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t reg_3dnr_flow_neighbor_th_u14_1 : 14;
	} bits;
};

union reg_isp_444_422_43 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_ratio_slope_u16_0      : 16;
		uint32_t reg_3dnr_ratio_slope_u16_1      : 16;
	} bits;
};

union reg_isp_444_422_44 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_still_region_th_u14_0  : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t reg_3dnr_still_region_th_u14_1  : 14;
	} bits;
};

union reg_isp_444_422_45 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_still_mix_base         : 8;
		uint32_t reg_3dnr_still_mix_slope        : 16;
		uint32_t _rsv_24                         : 1;
		uint32_t reg_3dnr_null_flow_mode         : 1;
	} bits;
};

union reg_isp_444_422_46 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_fs_global_str     : 16;
		uint32_t reg_3dnr_y_l1_ft_global_str     : 16;
	} bits;
};

union reg_isp_444_422_47 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_luma_adj_u8_0     : 8;
		uint32_t reg_3dnr_y_l1_luma_adj_u8_1     : 8;
		uint32_t reg_3dnr_y_l1_luma_adj_u8_2     : 8;
		uint32_t reg_3dnr_y_l1_luma_adj_u8_3     : 8;
	} bits;
};

union reg_isp_444_422_48 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_luma_adj_u8_4     : 8;
		uint32_t reg_3dnr_y_l1_luma_adj_u8_5     : 8;
		uint32_t reg_3dnr_y_l1_luma_adj_u8_6     : 8;
		uint32_t reg_3dnr_y_l1_luma_adj_u8_7     : 8;
	} bits;
};

union reg_isp_444_422_49 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_fs_jnd_adj_u8_0   : 8;
		uint32_t reg_3dnr_y_l1_fs_jnd_adj_u8_1   : 8;
		uint32_t reg_3dnr_y_l1_fs_jnd_adj_u8_2   : 8;
	} bits;
};

union reg_isp_444_422_50 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_ft_jnd_adj_u8_0   : 8;
		uint32_t reg_3dnr_y_l1_ft_jnd_adj_u8_1   : 8;
		uint32_t reg_3dnr_y_l1_ft_jnd_adj_u8_2   : 8;
		uint32_t _rsv_24                         : 4;
		uint32_t reg_3dnr_dynamic_blur_uv_wt     : 4;
	} bits;
};

union reg_isp_444_422_51 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_uv_l0_st_jnd_sigma_u8_0: 8;
		uint32_t reg_3dnr_uv_l0_st_jnd_sigma_u8_1: 8;
		uint32_t reg_3dnr_uv_l0_st_jnd_sigma_u8_2: 8;
		uint32_t _rsv_24                         : 1;
		uint32_t reg_3dnr_dc_enable              : 1;
	} bits;
};

union reg_isp_444_422_52 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_fs_freq_adj_u8_0  : 8;
		uint32_t reg_3dnr_y_l1_fs_freq_adj_u8_1  : 8;
		uint32_t reg_3dnr_y_l1_fs_freq_adj_u8_2  : 8;
		uint32_t reg_3dnr_y_l1_fs_freq_adj_u8_3  : 8;
	} bits;
};

union reg_isp_444_422_53 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_ft_freq_adj_u8_0  : 8;
		uint32_t reg_3dnr_y_l1_ft_freq_adj_u8_1  : 8;
		uint32_t reg_3dnr_y_l1_ft_freq_adj_u8_2  : 8;
		uint32_t reg_3dnr_y_l1_ft_freq_adj_u8_3  : 8;
	} bits;
};

union reg_isp_444_422_54 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_0        : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_1        : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_2        : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_3        : 8;
	} bits;
};

union reg_isp_444_422_55 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_4        : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_5        : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_6        : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_7        : 8;
	} bits;
};

union reg_isp_444_422_56 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_8        : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_9        : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_10       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_11       : 8;
	} bits;
};

union reg_isp_444_422_57 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_12       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_13       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_14       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_15       : 8;
	} bits;
};

union reg_isp_444_422_58 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_16       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_17       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_18       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_19       : 8;
	} bits;
};

union reg_isp_444_422_59 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_20       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_21       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_22       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_23       : 8;
	} bits;
};

union reg_isp_444_422_60 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_24       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_25       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_26       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_27       : 8;
	} bits;
};

union reg_isp_444_422_61 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_28       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_29       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_30       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_31       : 8;
	} bits;
};

union reg_isp_444_422_62 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_32       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_33       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_34       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_35       : 8;
	} bits;
};

union reg_isp_444_422_63 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_36       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_37       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_38       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_39       : 8;
	} bits;
};

union reg_isp_444_422_64 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_40       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_41       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_42       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_43       : 8;
	} bits;
};

union reg_isp_444_422_65 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_44       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_45       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_46       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_47       : 8;
	} bits;
};

union reg_isp_444_422_66 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_48       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_49       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_50       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_51       : 8;
	} bits;
};

union reg_isp_444_422_67 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_52       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_53       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_54       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_55       : 8;
	} bits;
};

union reg_isp_444_422_68 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_56       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_57       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_58       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_59       : 8;
	} bits;
};

union reg_isp_444_422_69 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_s_u8_60       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_61       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_62       : 8;
		uint32_t reg_3dnr_y_l1_pft_s_u8_63       : 8;
	} bits;
};

union reg_isp_444_422_70 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_0        : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_1        : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_2        : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_3        : 8;
	} bits;
};

union reg_isp_444_422_71 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_4        : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_5        : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_6        : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_7        : 8;
	} bits;
};

union reg_isp_444_422_72 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_8        : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_9        : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_10       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_11       : 8;
	} bits;
};

union reg_isp_444_422_73 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_12       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_13       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_14       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_15       : 8;
	} bits;
};

union reg_isp_444_422_74 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_16       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_17       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_18       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_19       : 8;
	} bits;
};

union reg_isp_444_422_75 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_20       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_21       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_22       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_23       : 8;
	} bits;
};

union reg_isp_444_422_76 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_24       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_25       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_26       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_27       : 8;
	} bits;
};

union reg_isp_444_422_77 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_28       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_29       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_30       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_31       : 8;
	} bits;
};

union reg_isp_444_422_78 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_32       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_33       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_34       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_35       : 8;
	} bits;
};

union reg_isp_444_422_79 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_36       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_37       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_38       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_39       : 8;
	} bits;
};

union reg_isp_444_422_80 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_40       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_41       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_42       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_43       : 8;
	} bits;
};

union reg_isp_444_422_81 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_44       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_45       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_46       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_47       : 8;
	} bits;
};

union reg_isp_444_422_82 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_48       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_49       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_50       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_51       : 8;
	} bits;
};

union reg_isp_444_422_83 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_52       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_53       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_54       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_55       : 8;
	} bits;
};

union reg_isp_444_422_84 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_56       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_57       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_58       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_59       : 8;
	} bits;
};

union reg_isp_444_422_85 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_t_u8_60       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_61       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_62       : 8;
		uint32_t reg_3dnr_y_l1_pft_t_u8_63       : 8;
	} bits;
};

union reg_isp_444_422_86 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_0        : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_1        : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_2        : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_3        : 8;
	} bits;
};

union reg_isp_444_422_87 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_4        : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_5        : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_6        : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_7        : 8;
	} bits;
};

union reg_isp_444_422_88 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_8        : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_9        : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_10       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_11       : 8;
	} bits;
};

union reg_isp_444_422_89 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_12       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_13       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_14       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_15       : 8;
	} bits;
};

union reg_isp_444_422_90 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_16       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_17       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_18       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_19       : 8;
	} bits;
};

union reg_isp_444_422_91 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_20       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_21       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_22       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_23       : 8;
	} bits;
};

union reg_isp_444_422_92 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_24       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_25       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_26       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_27       : 8;
	} bits;
};

union reg_isp_444_422_93 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_28       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_29       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_30       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_31       : 8;
	} bits;
};

union reg_isp_444_422_94 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_32       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_33       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_34       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_35       : 8;
	} bits;
};

union reg_isp_444_422_95 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_36       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_37       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_38       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_39       : 8;
	} bits;
};

union reg_isp_444_422_96 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_40       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_41       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_42       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_43       : 8;
	} bits;
};

union reg_isp_444_422_97 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_44       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_45       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_46       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_47       : 8;
	} bits;
};

union reg_isp_444_422_98 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_48       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_49       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_50       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_51       : 8;
	} bits;
};

union reg_isp_444_422_99 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_52       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_53       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_54       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_55       : 8;
	} bits;
};

union reg_isp_444_422_100 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_56       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_57       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_58       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_59       : 8;
	} bits;
};

union reg_isp_444_422_101 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l1_pft_m_u8_60       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_61       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_62       : 8;
		uint32_t reg_3dnr_y_l1_pft_m_u8_63       : 8;
	} bits;
};

union reg_isp_444_422_102 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l2_s_lut_u7_0        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_y_l2_s_lut_u7_1        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_y_l2_s_lut_u7_2        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_y_l2_s_lut_u7_3        : 7;
	} bits;
};

union reg_isp_444_422_103 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l2_s_lut_u7_4        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_y_l2_s_lut_u7_5        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_y_l2_s_lut_u7_6        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_y_l2_s_lut_u7_7        : 7;
	} bits;
};

union reg_isp_444_422_104 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l2_t_lut_u7_0        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_y_l2_t_lut_u7_1        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_y_l2_t_lut_u7_2        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_y_l2_t_lut_u7_3        : 7;
	} bits;
};

union reg_isp_444_422_105 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l2_t_lut_u7_4        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_y_l2_t_lut_u7_5        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_y_l2_t_lut_u7_6        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_y_l2_t_lut_u7_7        : 7;
	} bits;
};

union reg_isp_444_422_106 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l2_m_lut_u7_0        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_y_l2_m_lut_u7_1        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_y_l2_m_lut_u7_2        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_y_l2_m_lut_u7_3        : 7;
	} bits;
};

union reg_isp_444_422_107 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_l2_m_lut_u7_4        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_y_l2_m_lut_u7_5        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_y_l2_m_lut_u7_6        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_y_l2_m_lut_u7_7        : 7;
	} bits;
};

union reg_isp_444_422_108 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_dynamic_blur_y_jnd_wt_u4_0: 4;
		uint32_t reg_3dnr_dynamic_blur_y_jnd_wt_u4_1: 4;
		uint32_t reg_3dnr_dynamic_blur_y_jnd_wt_u4_2: 4;
		uint32_t reg_3dnr_y_l1_ft_luma_str       : 8;
	} bits;
};

union reg_isp_444_422_109 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_uv_l2_st_delta_damp_u7_0: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_u7_1: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_u7_2: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_u7_3: 7;
	} bits;
};

union reg_isp_444_422_110 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_uv_l2_st_delta_damp_u7_4: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_u7_5: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_u7_6: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_u7_7: 7;
	} bits;
};

union reg_isp_444_422_111 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_uv_l2_st_delta_damp_m_u7_0: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_m_u7_1: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_m_u7_2: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_m_u7_3: 7;
	} bits;
};

union reg_isp_444_422_112 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_uv_l2_st_delta_damp_m_u7_4: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_m_u7_5: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_m_u7_6: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_m_u7_7: 7;
	} bits;
};

union reg_isp_444_422_113 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_uv_l2_st_delta_damp_t_u7_0: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_t_u7_1: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_t_u7_2: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_t_u7_3: 7;
	} bits;
};

union reg_isp_444_422_114 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_uv_l2_st_delta_damp_t_u7_4: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_t_u7_5: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_t_u7_6: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t reg_3dnr_uv_l2_st_delta_damp_t_u7_7: 7;
	} bits;
};

union reg_isp_444_422_115 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_sad_debias_enable      : 1;
		uint32_t reg_3dnr_warp_std_range_u6_0    : 6;
		uint32_t reg_3dnr_warp_std_range_u6_1    : 6;
		uint32_t reg_3dnr_jnd_std_range_u6_0     : 6;
		uint32_t reg_3dnr_jnd_std_range_u6_1     : 6;
		uint32_t reg_3dnr_flow_check             : 1;
		uint32_t reg_3dnr_flow_jnd_th            : 6;
	} bits;
};

union reg_isp_444_422_reg116 {
	uint32_t raw;
	struct {
		uint32_t reg_3dnr_y_bundle_nr_enable     : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t reg_3dnr_uv_bundle_nr_enable    : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t reg_3dnr_uv_mono_enable         : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t reg_3dnr_uv_mono_val            : 8;
	} bits;
};

union reg_isp_444_422_reg117 {
	uint32_t raw;
	struct {
		uint32_t mmp_still_cnt                   : 20;
	} bits;
};

union reg_isp_444_422_reg118 {
	uint32_t raw;
	struct {
		uint32_t mmp_trans_cnt                   : 20;
	} bits;
};

union reg_isp_444_422_reg119 {
	uint32_t raw;
	struct {
		uint32_t mmp_motion_cnt                  : 20;
	} bits;
};

union reg_isp_444_422_reg120 {
	uint32_t raw;
	struct {
		uint32_t y_debug_en                      : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t y_debug_value                   : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_pfr_pfr_reg0 {
	uint32_t raw;
	struct {
		uint32_t pfr_en                          : 1;
		uint32_t hw_auto_cg_en                   : 1;
		uint32_t pfr_luma_level_en               : 1;
		uint32_t pfr_uvset_en_0                  : 1;
		uint32_t pfr_uvset_en_1                  : 1;
		uint32_t pfr_hueset_en_0                 : 1;
		uint32_t pfr_hueset_en_1                 : 1;
	} bits;
};

union reg_pfr_pfr_luma_set0 {
	uint32_t raw;
	struct {
		uint32_t pfr_luma_level_th               : 8;
		uint32_t pfr_luma_inivalue               : 8;
	} bits;
};

union reg_pfr_pfr_luma_level0 {
	uint32_t raw;
	struct {
		uint32_t pfr_luma_level_0                : 8;
		uint32_t pfr_luma_level_1                : 8;
		uint32_t pfr_luma_level_2                : 8;
		uint32_t pfr_luma_level_3                : 8;
	} bits;
};

union reg_pfr_pfr_luma_level1 {
	uint32_t raw;
	struct {
		uint32_t pfr_luma_level_4                : 8;
		uint32_t pfr_luma_level_5                : 8;
		uint32_t pfr_luma_level_6                : 8;
		uint32_t pfr_luma_level_7                : 8;
	} bits;
};

union reg_pfr_pfr_luma_level2 {
	uint32_t raw;
	struct {
		uint32_t pfr_luma_level_8                : 8;
		uint32_t pfr_luma_level_9                : 8;
		uint32_t pfr_luma_level_10               : 8;
		uint32_t pfr_luma_level_11               : 8;
	} bits;
};

union reg_pfr_pfr_luma_level3 {
	uint32_t raw;
	struct {
		uint32_t pfr_luma_level_12               : 8;
		uint32_t pfr_luma_level_13               : 8;
		uint32_t pfr_luma_level_14               : 8;
		uint32_t pfr_luma_level_15               : 8;
	} bits;
};

union reg_pfr_pfr_luma_level4 {
	uint32_t raw;
	struct {
		uint32_t pfr_luma_level_16               : 8;
	} bits;
};

union reg_pfr_pfr_color_uv {
	uint32_t raw;
	struct {
		uint32_t pfr_color_u_0                   : 8;
		uint32_t pfr_color_u_1                   : 8;
		uint32_t pfr_color_v_0                   : 8;
		uint32_t pfr_color_v_1                   : 8;
	} bits;
};

union reg_pfr_pfr_uv_diff_th {
	uint32_t raw;
	struct {
		uint32_t pfr_color_diff_th_set_0         : 8;
		uint32_t pfr_color_diff_th_set_1         : 8;
		uint32_t pfr_color_diff_lut_set0_0       : 8;
		uint32_t pfr_color_diff_lut_set0_1       : 8;
	} bits;
};

union reg_pfr_pfr_uv_diff_lut0 {
	uint32_t raw;
	struct {
		uint32_t pfr_color_diff_lut_set0_2       : 8;
		uint32_t pfr_color_diff_lut_set0_3       : 8;
		uint32_t pfr_color_diff_lut_set0_4       : 8;
		uint32_t pfr_color_diff_lut_set1_0       : 8;
	} bits;
};

union reg_pfr_pfr_uv_diff_lut1 {
	uint32_t raw;
	struct {
		uint32_t pfr_color_diff_lut_set1_1       : 8;
		uint32_t pfr_color_diff_lut_set1_2       : 8;
		uint32_t pfr_color_diff_lut_set1_3       : 8;
		uint32_t pfr_color_diff_lut_set1_4       : 8;
	} bits;
};

union reg_pfr_pfr_rb_wet {
	uint32_t raw;
	struct {
		uint32_t pfr_r_wet_0                     : 8;
		uint32_t pfr_r_wet_1                     : 8;
		uint32_t pfr_b_wet_0                     : 8;
		uint32_t pfr_b_wet_1                     : 8;
	} bits;
};

union reg_pfr_pfr_hueset {
	uint32_t raw;
	struct {
		uint32_t pfr_hue_0                       : 9;
		uint32_t pfr_hue_1                       : 9;
	} bits;
};

union reg_pfr_pfr_hue_range {
	uint32_t raw;
	struct {
		uint32_t pfr_hue_range_0                 : 8;
		uint32_t pfr_hue_range_1                 : 8;
	} bits;
};

union reg_pfr_pfr_hue_th {
	uint32_t raw;
	struct {
		uint32_t pfr_hue_th                      : 8;
	} bits;
};

union reg_pfr_pfr_g_diff_th0 {
	uint32_t raw;
	struct {
		uint32_t pfr_g_diff_th_set_0             : 8;
		uint32_t pfr_g_diff_th_set_1             : 8;
		uint32_t pfr_g_diff_th_set_2             : 8;
		uint32_t pfr_g_diff_th_set_3             : 8;
	} bits;
};

union reg_pfr_pfr_g_diff_th1 {
	uint32_t raw;
	struct {
		uint32_t pfr_g_diff_th_set_4             : 8;
	} bits;
};

union reg_pfr_pfr_edge_th {
	uint32_t raw;
	struct {
		uint32_t pfr_edge_th_0                   : 8;
		uint32_t pfr_edge_th_1                   : 8;
		uint32_t pfr_edge_th_2                   : 8;
		uint32_t pfr_edge_th_3                   : 8;
	} bits;
};

union reg_pfr_shadow_rd_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_pre_raw_fe_pre_raw_ctrl {
	uint32_t raw;
	struct {
		uint32_t bayer_type_le                   : 4;
		uint32_t bayer_type_se                   : 4;
		uint32_t _rsv_8                          : 10;
		uint32_t up_pq_en                        : 1;
		uint32_t _rsv_19                         : 12;
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

union reg_pre_raw_fe_pre_raw_frame_size {
	uint32_t raw;
	struct {
		uint32_t frame_widthm1                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t frame_heightm1                  : 14;
	} bits;
};

union reg_pre_raw_fe_pre_raw_post_no_rspd_cyc {
	uint32_t raw;
	struct {
		uint32_t post_no_rspd_cyc                : 32;
	} bits;
};

union reg_pre_raw_fe_pre_raw_frame_vld {
	uint32_t raw;
	struct {
		uint32_t fe_frame_vld_ch0                : 1;
		uint32_t fe_frame_vld_ch1                : 1;
		uint32_t fe_frame_vld_ch2                : 1;
		uint32_t fe_frame_vld_ch3                : 1;
		uint32_t fe_pq_vld_ch0                   : 1;
		uint32_t fe_pq_vld_ch1                   : 1;
		uint32_t fe_pq_vld_ch2                   : 1;
		uint32_t fe_pq_vld_ch3                   : 1;
		uint32_t _rsv_8                          : 8;
		uint32_t post_raw_idle                   : 1;
	} bits;
};

union reg_pre_raw_fe_pre_raw_debug_state {
	uint32_t raw;
	struct {
		uint32_t pre_raw_fe_idle                 : 32;
	} bits;
};

union reg_pre_raw_fe_pre_raw_dummy {
	uint32_t raw;
	struct {
		uint32_t dummy_rw                        : 16;
		uint32_t dummy_ro                        : 16;
	} bits;
};

union reg_pre_raw_fe_pre_raw_debug_info {
	uint32_t raw;
	struct {
		uint32_t ip_frame_done_sts               : 32;
	} bits;
};

union reg_pre_raw_fe_fe_idle_info {
	uint32_t raw;
	struct {
		uint32_t ip_dma_idle                     : 32;
	} bits;
};

union reg_pre_raw_fe_fe_check_sum {
	uint32_t raw;
	struct {
		uint32_t lexp_chksum_enable              : 1;
		uint32_t sexp_chksum_enable              : 1;
	} bits;
};

union reg_pre_raw_fe_fe_check_sum_value {
	uint32_t raw;
	struct {
		uint32_t lexp_chksum_value               : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t sexp_chksum_value               : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_pre_raw_vi_sel_0 {
	uint32_t raw;
	struct {
		uint32_t ring_buff_reset                 : 1;
		uint32_t ring_buff_monitor_en            : 1;
		uint32_t dma_ld_dpcm_mode                : 3;
		uint32_t csi_in_format                   : 1;
		uint32_t yuv_swap_en                     : 1;
		uint32_t _rsv_7                          : 9;
		uint32_t dpcm_rx_xstr                    : 13;
	} bits;
};

union reg_pre_raw_vi_sel_1 {
	uint32_t raw;
	struct {
		uint32_t frame_widthm1                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t frame_heightm1                  : 14;
	} bits;
};

union reg_pre_raw_vi_sel_2 {
	uint32_t raw;
	struct {
		uint32_t ring_buff_threshold_0           : 16;
		uint32_t ring_buff_threshold_1           : 16;
	} bits;
};

union reg_pre_raw_vi_sel_3 {
	uint32_t raw;
	struct {
		uint32_t ring_buff_threshold_2           : 16;
		uint32_t ring_buff_threshold_3           : 16;
	} bits;
};

union reg_pre_raw_vi_sel_4 {
	uint32_t raw;
	struct {
		uint32_t ring_buff_above_th_0            : 32;
	} bits;
};

union reg_pre_raw_vi_sel_5 {
	uint32_t raw;
	struct {
		uint32_t ring_buff_above_th_1            : 32;
	} bits;
};

union reg_pre_raw_vi_sel_6 {
	uint32_t raw;
	struct {
		uint32_t ring_buff_above_th_2            : 32;
	} bits;
};

union reg_pre_raw_vi_sel_7 {
	uint32_t raw;
	struct {
		uint32_t ring_buff_above_th_3            : 32;
	} bits;
};

union reg_pre_raw_vi_sel_8 {
	uint32_t raw;
	struct {
		uint32_t ip_dma_idle                     : 32;
	} bits;
};

union reg_pre_raw_vi_sel_9 {
	uint32_t raw;
	struct {
		uint32_t crop_enable_le                  : 1;
		uint32_t crop_enable_se                  : 1;
		uint32_t crop_enable_yuv                 : 1;
	} bits;
};

union reg_pre_raw_vi_sel_10 {
	uint32_t raw;
	struct {
		uint32_t vi_sel_shadow_sel               : 1;
	} bits;
};

union reg_pre_raw_vi_sel_11 {
	uint32_t raw;
	struct {
		uint32_t ai_isp_transform_en             : 1;
		uint32_t bayer_type                      : 2;
		uint32_t bypass_le                       : 1;
		uint32_t bypass_se                       : 1;
		uint32_t hw_auto_cg_en_le                : 1;
		uint32_t hw_auto_cg_en_se                : 1;
		uint32_t in_format_le                    : 2;
		uint32_t in_format_se                    : 2;
		uint32_t round_mode_le                   : 3;
		uint32_t round_mode_se                   : 3;
	} bits;
};

union reg_pre_raw_vi_sel_12 {
	uint32_t raw;
	struct {
		uint32_t guard_cnt_le                    : 8;
		uint32_t guard_cnt_se                    : 8;
	} bits;
};

union reg_pre_raw_vi_sel_13 {
	uint32_t raw;
	struct {
		uint32_t ai_isp_debug_status_le          : 32;
	} bits;
};

union reg_pre_raw_vi_sel_14 {
	uint32_t raw;
	struct {
		uint32_t ai_isp_debug_status_se          : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_raw_top0_raw_top_read_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel_2x                : 1;
	} bits;
};

union reg_raw_top0_chk_sum_en {
	uint32_t raw;
	struct {
		uint32_t up_pq_en_2x                     : 1;
	} bits;
};

union reg_raw_top0_raw_2 {
	uint32_t raw;
	struct {
		uint32_t img_widthm_1_2x                 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t img_heightm_1_2x                : 14;
	} bits;
};

union reg_raw_top0_raw_3 {
	uint32_t raw;
	struct {
		uint32_t img_widthm_0                    : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t img_heightm_0                   : 14;
	} bits;
};

union reg_raw_top0_raw_bayer_type_topleft {
	uint32_t raw;
	struct {
		uint32_t bayer_type_precrop              : 2;
		uint32_t bayer_type_2x                   : 2;
	} bits;
};

union reg_raw_top0_pass_sel {
	uint32_t raw;
	struct {
		uint32_t fifo_reverse_2x                 : 1;

		uint32_t pass_sel                        : 1;

		uint32_t pass_cnt_m1                     : 8;
		uint32_t pass_sel_bnr                    : 1;
		uint32_t pass_cnt_m1_bnr                 : 8;
	} bits;
};

union reg_raw_top0_ip_cfg_gclk_en {
	uint32_t raw;
	struct {
		uint32_t cfg_gclk_en_clsc                : 1;
	} bits;
};

union reg_raw_top0_ip_bypass {
	uint32_t raw;
	struct {
		uint32_t bypass_se_bnr                   : 1;
		uint32_t _rsv_1                          : 2;
		uint32_t bypass_clsc_le                  : 1;
		uint32_t bypass_clsc_se                  : 1;
	} bits;
};

union reg_raw_top0_pg_cfg0 {
	uint32_t raw;
	struct {
		uint32_t pg_enable                       : 1;
		uint32_t curser_en                       : 1;
		uint32_t x_curser                        : 14;
		uint32_t y_curser                        : 14;
	} bits;
};

union reg_raw_top0_pg_cfg1 {
	uint32_t raw;
	struct {
		uint32_t curser_value                    : 16;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_raw_top1_raw_top_read_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel_1x                : 1;
	} bits;
};

union reg_raw_top1_chk_sum_en {
	uint32_t raw;
	struct {
		uint32_t up_pq_en_1x                     : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t chk_sum_en                      : 1;
	} bits;
};

union reg_raw_top1_raw_2 {
	uint32_t raw;
	struct {
		uint32_t img_widthm_1_1x                 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t img_heightm_1_1x                : 14;
	} bits;
};

union reg_raw_top1_raw_4 {
	uint32_t raw;
	struct {
		uint32_t yuv_path                        : 1;
		uint32_t frame_done_sel_tail0_all1       : 1;
	} bits;
};

union reg_raw_top1_raw_top_status {
	uint32_t raw;
	struct {
		uint32_t raw_top_status                  : 32;
	} bits;
};

union reg_raw_top1_raw_top_debug {
	uint32_t raw;
	struct {
		uint32_t raw_top_debug                   : 32;
	} bits;
};

union reg_raw_top1_raw_bayer_type {
	uint32_t raw;
	struct {
		uint32_t bayer_type_1x                   : 2;
	} bits;
};

union reg_raw_top1_fifo_reverse {
	uint32_t raw;
	struct {
		uint32_t fifo_reverse_1x                 : 1;
	} bits;
};

union reg_raw_top1_raw_top_dma_idle {
	uint32_t raw;
	struct {
		uint32_t raw_top_dma_idle                : 32;
	} bits;
};

union reg_raw_top1_raw_top_checksum {
	uint32_t raw;
	struct {
		uint32_t raw_top_checksum                : 32;
	} bits;
};

union reg_raw_top1_ip_cfg_gclk_en {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 1;
		uint32_t cfg_gclk_en_llsc                : 1;
		uint32_t cfg_gclk_en_drc                 : 1;
	} bits;
};

union reg_raw_top1_ip_bypass {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 1;
		uint32_t bypass_map_curve                : 1;
		uint32_t bypass_llsc                     : 1;
	} bits;
};

union reg_raw_top1_af_blc_cfg0 {
	uint32_t raw;
	struct {
		uint32_t af_blc_offset_b                 : 12;
		uint32_t af_blc_offset_gb                : 12;
	} bits;
};

union reg_raw_top1_af_blc_cfg1 {
	uint32_t raw;
	struct {
		uint32_t af_blc_offset_gr                : 12;
		uint32_t af_blc_offset_r                 : 12;
	} bits;
};

union reg_raw_top1_af_blc_cfg2 {
	uint32_t raw;
	struct {
		uint32_t af_blc_gain_b                   : 16;
		uint32_t af_blc_gain_gb                  : 16;
	} bits;
};

union reg_raw_top1_af_blc_cfg3 {
	uint32_t raw;
	struct {
		uint32_t af_blc_gain_gr                  : 16;
		uint32_t af_blc_gain_r                   : 16;
	} bits;
};

union reg_raw_top1_af_blc_cfg4 {
	uint32_t raw;
	struct {
		uint32_t af_blc_enable                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_rgb_dither_rgb_dither {
	uint32_t raw;
	struct {
		uint32_t rgb_dither_enable               : 1;
		uint32_t rgb_dither_mod_en               : 1;
		uint32_t rgb_dither_histidx_en           : 1;
		uint32_t rgb_dither_fmnum_en             : 1;
		uint32_t rgb_dither_shdw_sel             : 1;
		uint32_t rgb_dither_softrst              : 1;
		uint32_t _rsv_6                          : 2;
		uint32_t crop_widthm1                    : 12;
		uint32_t crop_heightm1                   : 12;
	} bits;
};

union reg_isp_rgb_dither_rgb_dither_debug0 {
	uint32_t raw;
	struct {
		uint32_t rgb_dither_debug0               : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_rgb_top_patgen_01 {
	uint32_t raw;
	struct {
		uint32_t pg_enable                       : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t curser_en                       : 1;
		uint32_t _rsv_5                          : 11;
		uint32_t curser_value                    : 16;
	} bits;
};

union reg_rgb_top_patgen_02 {
	uint32_t raw;
	struct {
		uint32_t x_curser                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t y_curser                        : 14;
	} bits;
};

union reg_rgb_top_patgen_03 {
	uint32_t raw;
	struct {
		uint32_t value_report                    : 32;
	} bits;
};

union reg_rgb_top_patgen_04 {
	uint32_t raw;
	struct {
		uint32_t xcnt_rpt                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ycnt_rpt                        : 14;
	} bits;
};

union reg_rgb_top_dummy {
	uint32_t raw;
	struct {
		uint32_t dummy                           : 32;
	} bits;
};

union reg_rgb_top_patgen_05 {
	uint32_t raw;
	struct {
		uint32_t rgbtop_imgh                     : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t rgbtop_imgw                     : 14;
		uint32_t _rsv_30                         : 1;
		uint32_t pfr_bypass_rgbtop               : 1;
	} bits;
};

union reg_rgb_top_slice_soft_reset {
	uint32_t raw;
	struct {
		uint32_t rgb_top_slice_softrst           : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_gamma_ctrl {
	uint32_t raw;
	struct {
		uint32_t gamma_enable                    : 1;
		uint32_t gamma_shdw_sel                  : 1;
		uint32_t force_clk_enable                : 1;
	} bits;
};

union reg_isp_gamma_prog_ctrl {
	uint32_t raw;
	struct {
		uint32_t gamma_wsel                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t gamma_rsel                      : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t gamma_prog_en                   : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t gamma_prog_1to3_en              : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t gamma_prog_mode                 : 2;
	} bits;
};

union reg_isp_gamma_prog_st_addr {
	uint32_t raw;
	struct {
		uint32_t gamma_st_addr                   : 8;
		uint32_t _rsv_8                          : 23;
		uint32_t gamma_st_w                      : 1;
	} bits;
};

union reg_isp_gamma_prog_data {
	uint32_t raw;
	struct {
		uint32_t gamma_data_e                    : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t gamma_data_o                    : 12;
		uint32_t _rsv_28                         : 3;
		uint32_t gamma_w                         : 1;
	} bits;
};

union reg_isp_gamma_prog_max {
	uint32_t raw;
	struct {
		uint32_t gamma_max                       : 13;
	} bits;
};

union reg_isp_gamma_mem_sw_raddr {
	uint32_t raw;
	struct {
		uint32_t gamma_sw_raddr                  : 8;
		uint32_t _rsv_8                          : 4;
		uint32_t gamma_sw_r_mem_sel              : 1;
	} bits;
};

union reg_isp_gamma_mem_sw_rdata {
	uint32_t raw;
	struct {
		uint32_t gamma_rdata_r                   : 12;
		uint32_t _rsv_12                         : 19;
		uint32_t gamma_sw_r                      : 1;
	} bits;
};

union reg_isp_gamma_mem_sw_rdata_bg {
	uint32_t raw;
	struct {
		uint32_t gamma_rdata_g                   : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t gamma_rdata_b                   : 12;
	} bits;
};

union reg_isp_gamma_dbg {
	uint32_t raw;
	struct {
		uint32_t prog_hdk_dis                    : 1;
		uint32_t softrst                         : 1;
	} bits;
};

union reg_isp_gamma_dmy0 {
	uint32_t raw;
	struct {
		uint32_t dmy_def0                        : 32;
	} bits;
};

union reg_isp_gamma_dmy1 {
	uint32_t raw;
	struct {
		uint32_t dmy_def1                        : 32;
	} bits;
};

union reg_isp_gamma_dmy_r {
	uint32_t raw;
	struct {
		uint32_t dmy_ro                          : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_se_bnr_u1_se_bnr_enable {
	uint32_t raw;
	struct {
		uint32_t u1_se_bnr_enable                : 1;
	} bits;
};

union reg_isp_se_bnr_u4_se_bnr_spatial_str {
	uint32_t raw;
	struct {
		uint32_t u4_se_bnr_spatial_str           : 4;
	} bits;
};

union reg_isp_se_bnr_u10_se_bnr_intensity_str_lut_0 {
	uint32_t raw;
	struct {
		uint32_t u10_se_bnr_intensity_str_lut_0  : 10;
	} bits;
};

union reg_isp_se_bnr_u10_se_bnr_intensity_str_lut_1 {
	uint32_t raw;
	struct {
		uint32_t u10_se_bnr_intensity_str_lut_1  : 10;
	} bits;
};

union reg_isp_se_bnr_u10_se_bnr_intensity_str_lut_2 {
	uint32_t raw;
	struct {
		uint32_t u10_se_bnr_intensity_str_lut_2  : 10;
	} bits;
};

union reg_isp_se_bnr_u10_se_bnr_intensity_str_lut_3 {
	uint32_t raw;
	struct {
		uint32_t u10_se_bnr_intensity_str_lut_3  : 10;
	} bits;
};

union reg_isp_se_bnr_u10_se_bnr_intensity_str_lut_4 {
	uint32_t raw;
	struct {
		uint32_t u10_se_bnr_intensity_str_lut_4  : 10;
	} bits;
};

union reg_isp_se_bnr_u10_se_bnr_intensity_str_lut_5 {
	uint32_t raw;
	struct {
		uint32_t u10_se_bnr_intensity_str_lut_5  : 10;
	} bits;
};

union reg_isp_se_bnr_shdw_read_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

union reg_isp_se_bnr_hw_auto_cg_en {
	uint32_t raw;
	struct {
		uint32_t hw_auto_cg_en                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_twode_src_surface_reg0 {
	uint32_t raw;
	struct {
		uint32_t twode_enable                    : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t twode_mode                      : 3;
	} bits;
};

union reg_twode_src_surface_reg1 {
	uint32_t raw;
	struct {
		uint32_t src_surface_addr_l              : 32;
	} bits;
};

union reg_twode_src_surface_reg2 {
	uint32_t raw;
	struct {
		uint32_t src_surface_addr_h              : 8;
	} bits;
};

union reg_twode_src_surface_reg3 {
	uint32_t raw;
	struct {
		uint32_t pixel_4b                        : 1;
	} bits;
};

union reg_twode_src_surface_reg4 {
	uint32_t raw;
	struct {
		uint32_t pixel_2b                        : 1;
	} bits;
};

union reg_twode_src_surface_reg5 {
	uint32_t raw;
	struct {
		uint32_t src_surface_stride              : 18;
	} bits;
};

union reg_twode_src_surface_reg6 {
	uint32_t raw;
	struct {
		uint32_t src_surface_width               : 16;
	} bits;
};

union reg_twode_src_surface_reg7 {
	uint32_t raw;
	struct {
		uint32_t src_surface_height              : 16;
	} bits;
};

union reg_twode_dst_surface_cfg0 {
	uint32_t raw;
	struct {
		uint32_t dst_surface_addr_l              : 32;
	} bits;
};

union reg_twode_dst_surface_cfg1 {
	uint32_t raw;
	struct {
		uint32_t dst_surface_addr_h              : 8;
	} bits;
};

union reg_twode_dst_surface_cfg2 {
	uint32_t raw;
	struct {
		uint32_t dst_surface_width               : 16;
	} bits;
};

union reg_twode_dst_surface_cfg3 {
	uint32_t raw;
	struct {
		uint32_t dst_surface_height              : 16;
	} bits;
};

union reg_twode_dst_surface_cfg4 {
	uint32_t raw;
	struct {
		uint32_t dst_surface_stride              : 18;
	} bits;
};

union reg_twode_draw_line_ctrl0 {
	uint32_t raw;
	struct {
		uint32_t dl_sx                           : 16;
	} bits;
};

union reg_twode_draw_line_ctrl1 {
	uint32_t raw;
	struct {
		uint32_t dl_sy                           : 16;
	} bits;
};

union reg_twode_draw_line_ctrl2 {
	uint32_t raw;
	struct {
		uint32_t dl_ex                           : 16;
	} bits;
};

union reg_twode_draw_line_ctrl3 {
	uint32_t raw;
	struct {
		uint32_t dl_ey                           : 16;
	} bits;
};

union reg_twode_draw_line_ctrl4 {
	uint32_t raw;
	struct {
		uint32_t dl_pix_val                      : 32;
	} bits;
};

union reg_twode_draw_line_ctrl5 {
	uint32_t raw;
	struct {
		uint32_t dl_thick                        : 8;
	} bits;
};

union reg_twode_shadow_rd_sel {
	uint32_t raw;
	struct {
		uint32_t shdw_read_sel                   : 1;
	} bits;
};

union reg_twode_operator_start_ctrl {
	uint32_t raw;
	struct {
		uint32_t twode_start                     : 1;
	} bits;
};

union reg_twode_dbg_bus {
	uint32_t raw;
	struct {
		uint32_t dbg_bus                         : 32;
	} bits;
};

union reg_twode_clk_gate {
	uint32_t raw;
	struct {
		uint32_t reg_2de_cg_en                   : 1;
	} bits;
};

union reg_twode_intr_status {
	uint32_t raw;
	struct {
		uint32_t _status                         : 1;
	} bits;
};

union reg_twode_intr_clear {
	uint32_t raw;
	struct {
		uint32_t intr_clr_w1p                    : 1;
	} bits;
};

union reg_twode_bw_limit_rd {
	uint32_t raw;
	struct {
		uint32_t cnt_limit_rdma                  : 8;
	} bits;
};

union reg_twode_bw_limit_wr {
	uint32_t raw;
	struct {
		uint32_t cnt_limit_wdma                  : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_ycurv_ycur_ctrl {
	uint32_t raw;
	struct {
		uint32_t ycur_enable                     : 1;
		uint32_t ycur_shdw_sel                   : 1;
		uint32_t force_clk_enable                : 1;
	} bits;
};

union reg_isp_ycurv_ycur_prog_ctrl {
	uint32_t raw;
	struct {
		uint32_t ycur_wsel                       : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t ycur_rsel                       : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t ycur_prog_en                    : 1;
	} bits;
};

union reg_isp_ycurv_ycur_prog_st_addr {
	uint32_t raw;
	struct {
		uint32_t ycur_st_addr                    : 6;
		uint32_t _rsv_6                          : 25;
		uint32_t ycur_st_w                       : 1;
	} bits;
};

union reg_isp_ycurv_ycur_prog_data {
	uint32_t raw;
	struct {
		uint32_t ycur_data_e                     : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t ycur_data_o                     : 8;
		uint32_t _rsv_24                         : 7;
		uint32_t ycur_w                          : 1;
	} bits;
};

union reg_isp_ycurv_ycur_prog_max {
	uint32_t raw;
	struct {
		uint32_t ycur_max                        : 9;
	} bits;
};

union reg_isp_ycurv_ycur_mem_sw_mode {
	uint32_t raw;
	struct {
		uint32_t ycur_sw_raddr                   : 6;
		uint32_t _rsv_6                          : 6;
		uint32_t ycur_sw_r_mem_sel               : 1;
	} bits;
};

union reg_isp_ycurv_ycur_mem_sw_rdata {
	uint32_t raw;
	struct {
		uint32_t ycur_rdata_r                    : 8;
		uint32_t _rsv_8                          : 23;
		uint32_t ycur_sw_r                       : 1;
	} bits;
};

union reg_isp_ycurv_ycur_dbg {
	uint32_t raw;
	struct {
		uint32_t prog_hdk_dis                    : 1;
		uint32_t softrst                         : 1;
	} bits;
};

union reg_isp_ycurv_ycur_dmy0 {
	uint32_t raw;
	struct {
		uint32_t dmy_def0                        : 32;
	} bits;
};

union reg_isp_ycurv_ycur_dmy1 {
	uint32_t raw;
	struct {
		uint32_t dmy_def1                        : 32;
	} bits;
};

union reg_isp_ycurv_ycur_dmy_r {
	uint32_t raw;
	struct {
		uint32_t dmy_ro                          : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_isp_yuv_dither_y_dither {
	uint32_t raw;
	struct {
		uint32_t y_dither_enable                 : 1;
		uint32_t y_dither_mod_enable             : 1;
		uint32_t y_dither_histidx_enable         : 1;
		uint32_t y_dither_fmnum_enable           : 1;
		uint32_t y_dither_shdw_sel               : 1;
		uint32_t y_dither_softrst                : 1;
		uint32_t _rsv_6                          : 2;
		uint32_t y_dither_heightm1               : 12;
		uint32_t y_dither_widthm1                : 12;
	} bits;
};

union reg_isp_yuv_dither_uv_dither {
	uint32_t raw;
	struct {
		uint32_t uv_dither_enable                : 1;
		uint32_t uv_dither_mod_enable            : 1;
		uint32_t uv_dither_histidx_enable        : 1;
		uint32_t uv_dither_fmnum_enable          : 1;
		uint32_t _rsv_4                          : 4;
		uint32_t uv_dither_heightm1              : 12;
		uint32_t uv_dither_widthm1               : 12;
	} bits;
};

union reg_isp_yuv_dither_debug_00 {
	uint32_t raw;
	struct {
		uint32_t uv_dither_debug0                : 32;
	} bits;
};

union reg_isp_yuv_dither_debug_01 {
	uint32_t raw;
	struct {
		uint32_t y_dither_debug0                 : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union reg_yuv_top_yuv_0 {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 8;
		uint32_t yuv_top_sel                     : 1;
	} bits;
};

union reg_yuv_top_yuv_2 {
	uint32_t raw;
	struct {
		uint32_t fd_int                          : 1;
		uint32_t dma_int                         : 1;
		uint32_t frame_overflow                  : 1;
	} bits;
};

union reg_yuv_top_yuv_3 {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 2;
		uint32_t yonly_en                        : 1;
	} bits;
};

union reg_yuv_top_yuv_debug_0 {
	uint32_t raw;
	struct {
		uint32_t debug_bus                       : 32;
	} bits;
};

union reg_yuv_top_yuv_4 {
	uint32_t raw;
	struct {
		uint32_t dummy                           : 32;
	} bits;
};

union reg_yuv_top_yuv_debug_state {
	uint32_t raw;
	struct {
		uint32_t ma_idle                         : 24;
		uint32_t _rsv_24                         : 7;
		uint32_t idle                            : 1;
	} bits;
};

union reg_yuv_top_yuv_5 {
	uint32_t raw;
	struct {
		uint32_t dis_uv2dram                     : 1;
		uint32_t line_thres_en                   : 1;
		uint32_t _rsv_2                          : 6;
		uint32_t line_thres                      : 14;
		uint32_t _rsv_22                         : 2;
		uint32_t pg2_enable                      : 1;
	} bits;
};

union reg_yuv_top_yuv_ctrl {
	uint32_t raw;
	struct {
		uint32_t checksum_enable                 : 1;
		uint32_t sc_dma_switch                   : 1;
		uint32_t _rsv_2                          : 5;
		uint32_t curser2_en                      : 1;
		uint32_t guard_cnt                       : 8;
	} bits;
};

union reg_yuv_top_imgw_m1 {
	uint32_t raw;
	struct {
		uint32_t yuv_top_imgw_m1                 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t yuv_top_imgh_m1                 : 14;
	} bits;
};

union reg_yuv_top_stvalid_status {
	uint32_t raw;
	struct {
		uint32_t stvalid_status                  : 32;
	} bits;
};

union reg_yuv_top_stready_status {
	uint32_t raw;
	struct {
		uint32_t stready_status                  : 32;
	} bits;
};

union reg_yuv_top_patgen1 {
	uint32_t raw;
	struct {
		uint32_t x_curser                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t y_curser                        : 14;
		uint32_t curser_en                       : 1;
		uint32_t pg_enable                       : 1;
	} bits;
};

union reg_yuv_top_patgen2 {
	uint32_t raw;
	struct {
		uint32_t curser_value                    : 16;
	} bits;
};

union reg_yuv_top_patgen3 {
	uint32_t raw;
	struct {
		uint32_t value_report                    : 32;
	} bits;
};

union reg_yuv_top_patgen4 {
	uint32_t raw;
	struct {
		uint32_t xcnt_rpt                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ycnt_rpt                        : 14;
	} bits;
};

union reg_yuv_top_check_sum {
	uint32_t raw;
	struct {
		uint32_t k_sum                           : 32;
	} bits;
};

union reg_yuv_top_ai_isp_rdma_ctrl {
	uint32_t raw;
	struct {
		uint32_t ai_isp_rdma_enable              : 3;
		uint32_t ai_isp_crop_enable              : 1;
		uint32_t ai_isp_enable                   : 1;
		uint32_t ai_isp_mask                     : 1;
	} bits;
};

union reg_yuv_top_ai_isp_img_size_y {
	uint32_t raw;
	struct {
		uint32_t ai_isp_img_width_crop_0         : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ai_isp_img_height_crop_0        : 14;
	} bits;
};

union reg_yuv_top_ai_isp_w_crop_y {
	uint32_t raw;
	struct {
		uint32_t ai_isp_crop_w_str_0             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ai_isp_crop_w_end_0             : 14;
	} bits;
};

union reg_yuv_top_ai_isp_h_crop_y {
	uint32_t raw;
	struct {
		uint32_t ai_isp_crop_h_str_0             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ai_isp_crop_h_end_0             : 14;
	} bits;
};

union reg_yuv_top_ai_isp_img_size_uv {
	uint32_t raw;
	struct {
		uint32_t ai_isp_img_width_crop_1         : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ai_isp_img_height_crop_1        : 14;
	} bits;
};

union reg_yuv_top_ai_isp_w_crop_uv {
	uint32_t raw;
	struct {
		uint32_t ai_isp_crop_w_str_1             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ai_isp_crop_w_end_1             : 14;
	} bits;
};

union reg_yuv_top_ai_isp_h_crop_uv {
	uint32_t raw;
	struct {
		uint32_t ai_isp_crop_h_str_1             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ai_isp_crop_h_end_1             : 14;
	} bits;
};

union reg_yuv_top_yuv_ctrl_mars3 {
	uint32_t raw;
	struct {
		uint32_t bypass_h_0                      : 1;
		uint32_t avg_mode_0                      : 1;
		uint32_t nodelay_duplicate_mode_0        : 1;
		uint32_t scale_disable_0                 : 1;
		uint32_t reg_422_444_en_0                : 1;
		uint32_t swap_en_0                       : 2;
		uint32_t avg_mode_5                      : 1;
		uint32_t bypass_h_3                      : 1;
		uint32_t nodelay_duplicate_mode_4        : 1;
		uint32_t scale_disable_4                 : 1;
		uint32_t bypass_h_1                      : 1;
		uint32_t avg_mode_2                      : 1;
		uint32_t nodelay_duplicate_mode_2        : 1;
		uint32_t scale_disable_2                 : 1;
		uint32_t bypass_v_1                      : 1;
		uint32_t avg_mode_3                      : 1;
		uint32_t drop_mode                       : 1;
		uint32_t bypass_h_2                      : 1;
		uint32_t bypass_v_2                      : 1;
		uint32_t avg_mode_4                      : 1;
		uint32_t nodelay_duplicate_mode_3        : 1;
		uint32_t scale_disable_3                 : 1;
		uint32_t reg_422_444_dither              : 1;
		uint32_t yv_swap_en                      : 1;
		uint32_t uv_swap_en                      : 1;
		uint32_t yu_swap_en                      : 1;
		uint32_t mctf_uv_swap_en                 : 1;
		uint32_t hw_auto_cg_en                   : 1;
	} bits;
};

#ifdef __cplusplus
}
#endif

#endif /* _VI_REG_FIELDS_H_ */
