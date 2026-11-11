/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: vi_tun_cfg.h
 * Description:
 */

#ifndef _U_VI_TUN_CFG_H_
#define _U_VI_TUN_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vi_reg_fields.h"

#define TUNING_NODE_NUM  2

struct sop_vip_blc_dg_wb_tun_cfg {
	union reg_blc_dg_wb_wbg_le_gain0        wbg_le_gain0;
	union reg_blc_dg_wb_wbg_le_gain1        wbg_le_gain1;
	union reg_blc_dg_wb_blc_le_offset0      blc_le_offset0;
	union reg_blc_dg_wb_blc_le_offset1      blc_le_offset1;
	union reg_blc_dg_wb_blc_le_offset_2nd0  blc_le_offset_2nd0;
	union reg_blc_dg_wb_blc_le_offset_2nd1  blc_le_offset_2nd1;
	union reg_blc_dg_wb_blc_le_gain0        blc_le_gain0;
	union reg_blc_dg_wb_blc_le_gain1        blc_le_gain1;
	union reg_blc_dg_wb_blc_le_normgain0    blc_le_normgain0;
	union reg_blc_dg_wb_blc_le_normgain1    blc_le_normgain1;
	union reg_blc_dg_wb_wbg_se_gain0        wbg_se_gain0;
	union reg_blc_dg_wb_wbg_se_gain1        wbg_se_gain1;
	union reg_blc_dg_wb_blc_se_offset0      blc_se_offset0;
	union reg_blc_dg_wb_blc_se_offset1      blc_se_offset1;
	union reg_blc_dg_wb_blc_se_offset_2nd0  blc_se_offset_2nd0;
	union reg_blc_dg_wb_blc_se_offset_2nd1  blc_se_offset_2nd1;
	union reg_blc_dg_wb_blc_se_gain0        blc_se_gain0;
	union reg_blc_dg_wb_blc_se_gain1        blc_se_gain1;
	union reg_blc_dg_wb_blc_se_normgain0    blc_se_normgain0;
	union reg_blc_dg_wb_blc_se_normgain1    blc_se_normgain1;
};

struct sop_vip_isp_wbg_config {
	__u8  update;
	__u8  inst;
	__u8  bypass;
	__u8  cg_enable;
	__u8  blc_le_enable;
	__u8  blc_se_enable;
	__u8  wbg_le_enable;
	__u8  wbg_se_enable;

	struct sop_vip_blc_dg_wb_tun_cfg burst_cfg;
};

struct sop_vip_isp_ccm_config {
	__u8  update;
	__u8  enable;
	__u8  over_str;
	__u8  over_thr;
	__u16 coef[3][3];
};

struct sop_vip_isp_cacp_config {
	__u8  update;
	__u8  enable;
	__u8  mode;
	__u16 iso_ratio;
	__u16 ca_y_ratio_lut[256];
	__u8  cp_y_lut[256];
	__u8  cp_u_lut[256];
	__u8  cp_v_lut[256];
};

struct sop_vip_isp_ca2_config {
	__u8  update;
	__u8  enable;
	__u16 lut_in[6];
	__u16 lut_out[6];
	__u16 lut_slp[5];
};

struct sop_vip_isp_gamma_config {
	__u8  update;
	__u8  enable;
	__u16 max;
	__u16 lut[256];
};

struct sop_isp_demosiac_tun_cfg {
	union reg_isp_cfa_0c                     reg_0c;
	union reg_isp_cfa_10                     reg_10;
	union reg_isp_cfa_14                     reg_14;
	union reg_isp_cfa_18                     reg_18;
	union reg_isp_cfa_1c                     reg_1c;
};

struct sop_isp_demosiac_tun_1_cfg {
	union reg_isp_cfa_120                   reg_120;
	union reg_isp_cfa_124                   reg_124;
	union reg_isp_cfa_128                   reg_128;
	union reg_isp_cfa_12c                   reg_12c;
	union reg_isp_cfa_130                   reg_130;
	union reg_isp_cfa_134                   reg_134;
	union reg_isp_cfa_138                   reg_138;
	union reg_isp_cfa_13c                   reg_13c;
	union reg_isp_cfa_140                   reg_140;
	union reg_isp_cfa_144                   reg_144;
	union reg_isp_cfa_148                   reg_148;
	union reg_isp_cfa_14c                   reg_14c;
	union reg_isp_cfa_150                   reg_150;
	union reg_isp_cfa_154                   reg_154;
	union reg_isp_cfa_158                   reg_158;
	union reg_isp_cfa_15c                   reg_15c;
	union reg_isp_cfa_160                   reg_160;
	union reg_isp_cfa_164                   reg_164;
	union reg_isp_cfa_168                   reg_168;
	union reg_isp_cfa_16c                   reg_16c;
	union reg_isp_cfa_170                   reg_170;
	union reg_isp_cfa_174                   reg_174;
	union reg_isp_cfa_178                   reg_178;
	union reg_isp_cfa_17c                   reg_17c;
	union reg_isp_cfa_180                   reg_180;
	union reg_isp_cfa_184                   reg_184;
	union reg_isp_cfa_188                   reg_188;
	union reg_isp_cfa_18c                   reg_18c;
	union reg_isp_cfa_190                   reg_190;
};

struct sop_isp_demosiac_tun_2_cfg {
	union reg_isp_cfa_90                    reg_90;
	union reg_isp_cfa_94                    reg_94;
	union reg_isp_cfa_98                    reg_98;
	union reg_isp_cfa_9c                    reg_9c;
	union reg_isp_cfa_a0                    reg_a0;
	union reg_isp_cfa_a4                    reg_a4;
	union reg_isp_cfa_a8                    reg_a8;
};

struct sop_vip_isp_demosiac_config {
	__u8  update;
	__u8  cfa_enable;
	__u16 cfa_edgee_thd2;
	__u8  cfa_out_sel;
	__u8  cfa_force_dir_enable;
	__u8  cfa_force_dir_sel;
	__u16 cfa_rbsig_luma_thd;
	__u8  cfa_ghp_lut[32];
	__u8  cfa_ymoire_enable;
	__u8  cfa_ymoire_dc_w;
	__u8  cfa_ymoire_lpf_w;
	struct sop_isp_demosiac_tun_cfg demosiac_cfg;
	struct sop_isp_demosiac_tun_1_cfg demosiac_1_cfg;
	struct sop_isp_demosiac_tun_2_cfg demosiac_2_cfg;
};

struct sop_vip_isp_lsc_config {
	__u8  update;
	__u8  inst;
	__u8 lsc_enable;
	__u8 lsc_gain_3p9_0_4p8_1;
	__u8 lsc_gain_bicubic_0_bilinear_1;
	__u16 lsc_strength;
	__u16 lsc_xstep;
	__u16 lsc_ystep;
	__u32 lsc_imgx0;
	__u32 lsc_imgy0;
	__u16 lsc_debug;
	__u8 lsc_boundary_interpolation_lf_range;
	__u8 lsc_boundary_interpolation_up_range;
	__u8 lsc_boundary_interpolation_rt_range;
	__u8 lsc_boundary_interpolation_dn_range;
	__u32 lsc_intp_gain_max;
	__u32 lsc_intp_gain_min;
	__u16 blc_offset_r;
	__u16 blc_offset_gr;
	__u16 blc_offset_gb;
	__u16 blc_offset_b;
};

struct sop_vip_isp_bnr_nlm_tun_cfg {
	union reg_isp_bnr_20                    bnr_20;
	union reg_isp_bnr_24                    bnr_24;
	union reg_isp_bnr_28                    bnr_28;
	union reg_isp_bnr_2c                    bnr_2c;
	union reg_isp_bnr_30                    bnr_30;
	union reg_isp_bnr_34                    bnr_34;
	union reg_isp_bnr_38                    bnr_38;
	union reg_isp_bnr_3c                    bnr_3c;
	union reg_isp_bnr_40                    bnr_40;
	union reg_isp_bnr_44                    bnr_44;
	union reg_isp_bnr_48                    bnr_48;
	union reg_isp_bnr_4c                    bnr_4c;
	union reg_isp_bnr_50                    bnr_50;
	union reg_isp_bnr_54                    bnr_54;
	union reg_isp_bnr_58                    bnr_58;
	union reg_isp_bnr_5c                    bnr_5c;
	union reg_isp_bnr_60                    bnr_60;
	union reg_isp_bnr_64                    bnr_64;
	union reg_isp_bnr_68                    bnr_68;
	union reg_isp_bnr_6c                    bnr_6c;
	union reg_isp_bnr_70                    bnr_70;
	union reg_isp_bnr_74                    bnr_74;
	union reg_isp_bnr_78                    bnr_78;
	union reg_isp_bnr_7c                    bnr_7c;
	union reg_isp_bnr_80                    bnr_80;
	union reg_isp_bnr_84                    bnr_84;
	union reg_isp_bnr_88                    bnr_88;
	union reg_isp_bnr_8c                    bnr_8c;
	union reg_isp_bnr_90                    bnr_90;
	union reg_isp_bnr_94                    bnr_94;
	union reg_isp_bnr_98                    bnr_98;
	union reg_isp_bnr_9c                    bnr_9c;
	union reg_isp_bnr_a0                    bnr_a0;
	union reg_isp_bnr_a4                    bnr_a4;
	union reg_isp_bnr_a8                    bnr_a8;
	union reg_isp_bnr_ac                    bnr_ac;
	union reg_isp_bnr_b0                    bnr_b0;
	union reg_isp_bnr_b4                    bnr_b4;
	union reg_isp_bnr_b8                    bnr_b8;
	union reg_isp_bnr_bc                    bnr_bc;
	union reg_isp_bnr_c0                    bnr_c0;
	union reg_isp_bnr_c4                    bnr_c4;
	union reg_isp_bnr_c8                    bnr_c8;
	union reg_isp_bnr_cc                    bnr_cc;
	union reg_isp_bnr_d0                    bnr_d0;
	union reg_isp_bnr_d4                    bnr_d4;
	union reg_isp_bnr_d8                    bnr_d8;
	union reg_isp_bnr_dc                    bnr_dc;
};

struct sop_vip_isp_bnr_bf_tun_cfg {
	union reg_isp_bnr_e0                    bnr_e0;
	union reg_isp_bnr_e4                    bnr_e4;
	union reg_isp_bnr_e8                    bnr_e8;
	union reg_isp_bnr_ec                    bnr_ec;
	union reg_isp_bnr_f0                    bnr_f0;
	union reg_isp_bnr_f4                    bnr_f4;
	union reg_isp_bnr_f8                    bnr_f8;
	union reg_isp_bnr_fc                    bnr_fc;
	union reg_isp_bnr_100                   bnr_100;
	union reg_isp_bnr_104                   bnr_104;
	union reg_isp_bnr_108                   bnr_108;
	union reg_isp_bnr_10c                   bnr_10c;
	union reg_isp_bnr_110                   bnr_110;
	union reg_isp_bnr_114                   bnr_114;
	union reg_isp_bnr_118                   bnr_118;
	union reg_isp_bnr_11c                   bnr_11c;
	union reg_isp_bnr_120                   bnr_120;
	union reg_isp_bnr_124                   bnr_124;
	union reg_isp_bnr_128                   bnr_128;
	union reg_isp_bnr_12c                   bnr_12c;
	union reg_isp_bnr_130                   bnr_130;
	union reg_isp_bnr_134                   bnr_134;
	union reg_isp_bnr_138                   bnr_138;
	union reg_isp_bnr_13c                   bnr_13c;
	union reg_isp_bnr_140                   bnr_140;
	union reg_isp_bnr_144                   bnr_144;
	union reg_isp_bnr_148                   bnr_148;
	union reg_isp_bnr_14c                   bnr_14c;
	union reg_isp_bnr_150                   bnr_150;
	union reg_isp_bnr_154                   bnr_154;
	union reg_isp_bnr_158                   bnr_158;
	union reg_isp_bnr_15c                   bnr_15c;
	union reg_isp_bnr_160                   bnr_160;
	union reg_isp_bnr_164                   bnr_164;
	union reg_isp_bnr_168                   bnr_168;
	union reg_isp_bnr_16c                   bnr_16c;
	union reg_isp_bnr_170                   bnr_170;
	union reg_isp_bnr_174                   bnr_174;
	union reg_isp_bnr_178                   bnr_178;
	union reg_isp_bnr_17c                   bnr_17c;
	union reg_isp_bnr_180                   bnr_180;
	union reg_isp_bnr_184                   bnr_184;
	union reg_isp_bnr_188                   bnr_188;
	union reg_isp_bnr_18c                   bnr_18c;
	union reg_isp_bnr_190                   bnr_190;
	union reg_isp_bnr_194                   bnr_194;
	union reg_isp_bnr_198                   bnr_198;
	union reg_isp_bnr_19c                   bnr_19c;
};

struct sop_vip_isp_bnr_config {
	__u8  update;
	__u8  u1_bnr_enable;
	__u8  u2_bnr_debugmode;
	__u8  u1_bnr_out_enable;
	__u8  u1_bnr_filtmode;
	__u8  u1_bnr_bilat_th_en;
	__u8  u1_bnr_bilat_center_sel;
	__u8  u5_bnr_bilat_blend_w;

	__u16 u10_bnr_bilat_th1;
	__u16 u10_bnr_bilat_th2;

	__u8 u5_bnr_sweight[10];

	struct sop_vip_isp_bnr_nlm_tun_cfg nlm_burst_cfg;
	struct sop_vip_isp_bnr_bf_tun_cfg bf_burst_cfg;
};

struct sop_vip_isp_clut_config {
	__u8  update;
	__u8  enable;
};

struct sop_vip_isp_drc_tun_cfg {
	union reg_isp_drc_hori_lpf_th12         drc_hori_lpf_th12;
	union reg_isp_drc_hori_lpf_th34         drc_hori_lpf_th34;
	union reg_isp_drc_hori_lpf_th56         drc_hori_lpf_th56;
	union reg_isp_drc_q_coeff12             drc_q_coeff12;
	union reg_isp_drc_q_coeff34             drc_q_coeff34;
	union reg_isp_drc_b2q_th                drc_b2q_th;
	union reg_isp_drc_fbc                   drc_fbc;
	union reg_isp_drc_anti_halo             drc_anti_halo;
	union reg_isp_drc_strength              drc_strength;
	union reg_isp_drc_ltm_en                drc_ltm_en;
	union reg_isp_drc_gain                  drc_gain;
	union reg_isp_drc_sat_th                drc_sat_th;
	union reg_isp_drc_sat_delta             drc_sat_delta;
	union reg_isp_drc_yv_bld_lut0_4         drc_yv_bld_lut0_4;
	union reg_isp_drc_yv_bld_lut5_8         drc_yv_bld_lut5_8;
	union reg_isp_drc_input_bld_lut_0_3     drc_input_bld_lut_0_3;
	union reg_isp_drc_input_bld_lut_4_7     drc_input_bld_lut_4_7;
	union reg_isp_drc_input_bld_lut_8_11    drc_input_bld_lut_8_11;
	union reg_isp_drc_input_bld_lut_12_15   drc_input_bld_lut_12_15;
	union reg_isp_drc_input_bld_lut_16      drc_input_bld_lut_16;
	union reg_isp_drc_gtm_l_lut_0_1         drc_gtm_l_lut_0_1;
	union reg_isp_drc_gtm_l_lut_2_3         drc_gtm_l_lut_2_3;
	union reg_isp_drc_gtm_l_lut_4_5         drc_gtm_l_lut_4_5;
	union reg_isp_drc_gtm_l_lut_6_7         drc_gtm_l_lut_6_7;
	union reg_isp_drc_gtm_l_lut_8_9         drc_gtm_l_lut_8_9;
	union reg_isp_drc_gtm_l_lut_10_11       drc_gtm_l_lut_10_11;
	union reg_isp_drc_gtm_l_lut_12_13       drc_gtm_l_lut_12_13;
	union reg_isp_drc_gtm_l_lut_14_15       drc_gtm_l_lut_14_15;
	union reg_isp_drc_gtm_l_lut_16_17       drc_gtm_l_lut_16_17;
	union reg_isp_drc_gtm_l_lut_18_19       drc_gtm_l_lut_18_19;
	union reg_isp_drc_gtm_l_lut_20_21       drc_gtm_l_lut_20_21;
	union reg_isp_drc_gtm_l_lut_22_23       drc_gtm_l_lut_22_23;
	union reg_isp_drc_gtm_l_lut_24_25       drc_gtm_l_lut_24_25;
	union reg_isp_drc_gtm_l_lut_26_27       drc_gtm_l_lut_26_27;
	union reg_isp_drc_gtm_l_lut_28_29       drc_gtm_l_lut_28_29;
	union reg_isp_drc_gtm_l_lut_30_31       drc_gtm_l_lut_30_31;
	union reg_isp_drc_gtm_l_lut_32_33       drc_gtm_l_lut_32_33;
	union reg_isp_drc_gtm_l_lut_34_35       drc_gtm_l_lut_34_35;
	union reg_isp_drc_gtm_l_lut_36_37       drc_gtm_l_lut_36_37;
	union reg_isp_drc_gtm_l_lut_38_39       drc_gtm_l_lut_38_39;
	union reg_isp_drc_gtm_l_lut_40_41       drc_gtm_l_lut_40_41;
	union reg_isp_drc_gtm_l_lut_42_43       drc_gtm_l_lut_42_43;
	union reg_isp_drc_gtm_l_lut_44_45       drc_gtm_l_lut_44_45;
	union reg_isp_drc_gtm_l_lut_46_47       drc_gtm_l_lut_46_47;
	union reg_isp_drc_gtm_l_lut_48_49       drc_gtm_l_lut_48_49;
	union reg_isp_drc_gtm_l_lut_50_51       drc_gtm_l_lut_50_51;
	union reg_isp_drc_gtm_l_lut_52_53       drc_gtm_l_lut_52_53;
	union reg_isp_drc_gtm_l_lut_54_55       drc_gtm_l_lut_54_55;
	union reg_isp_drc_gtm_l_lut_56_57       drc_gtm_l_lut_56_57;
	union reg_isp_drc_gtm_l_lut_58_59       drc_gtm_l_lut_58_59;
	union reg_isp_drc_gtm_l_lut_60_61       drc_gtm_l_lut_60_61;
	union reg_isp_drc_gtm_l_lut_62_63       drc_gtm_l_lut_62_63;
	union reg_isp_drc_gtm_l_lut_64          drc_gtm_l_lut_64;
	union reg_isp_drc_gtm_r_lut_0_1         drc_gtm_r_lut_0_1;
	union reg_isp_drc_gtm_r_lut_2_3         drc_gtm_r_lut_2_3;
	union reg_isp_drc_gtm_r_lut_4_5         drc_gtm_r_lut_4_5;
	union reg_isp_drc_gtm_r_lut_6_7         drc_gtm_r_lut_6_7;
	union reg_isp_drc_gtm_r_lut_8_9         drc_gtm_r_lut_8_9;
	union reg_isp_drc_gtm_r_lut_10_11       drc_gtm_r_lut_10_11;
	union reg_isp_drc_gtm_r_lut_12_13       drc_gtm_r_lut_12_13;
	union reg_isp_drc_gtm_r_lut_14_15       drc_gtm_r_lut_14_15;
	union reg_isp_drc_gtm_r_lut_16          drc_gtm_r_lut_16;
	union reg_isp_drc_luma_prot_lut_0_1     drc_luma_prot_lut_0_1;
	union reg_isp_drc_luma_prot_lut_2_3     drc_luma_prot_lut_2_3;
	union reg_isp_drc_luma_prot_lut_4_5     drc_luma_prot_lut_4_5;
	union reg_isp_drc_luma_prot_lut_6_7     drc_luma_prot_lut_6_7;
	union reg_isp_drc_luma_prot_lut_8_9     drc_luma_prot_lut_8_9;
	union reg_isp_drc_luma_prot_lut_10_11   drc_luma_prot_lut_10_11;
	union reg_isp_drc_luma_prot_lut_12_13   drc_luma_prot_lut_12_13;
	union reg_isp_drc_luma_prot_lut_14_15   drc_luma_prot_lut_14_15;
	union reg_isp_drc_luma_prot_lut_16      drc_luma_prot_lut_16;
	union reg_isp_drc_ltm_l_lut_0_1         drc_ltm_l_lut_0_1;
	union reg_isp_drc_ltm_l_lut_2_3         drc_ltm_l_lut_2_3;
	union reg_isp_drc_ltm_l_lut_4_5         drc_ltm_l_lut_4_5;
	union reg_isp_drc_ltm_l_lut_6_7         drc_ltm_l_lut_6_7;
	union reg_isp_drc_ltm_l_lut_8_9         drc_ltm_l_lut_8_9;
	union reg_isp_drc_ltm_l_lut_10_11       drc_ltm_l_lut_10_11;
	union reg_isp_drc_ltm_l_lut_12_13       drc_ltm_l_lut_12_13;
	union reg_isp_drc_ltm_l_lut_14_15       drc_ltm_l_lut_14_15;
	union reg_isp_drc_ltm_l_lut_16_17       drc_ltm_l_lut_16_17;
	union reg_isp_drc_ltm_l_lut_18_19       drc_ltm_l_lut_18_19;
	union reg_isp_drc_ltm_l_lut_20_21       drc_ltm_l_lut_20_21;
	union reg_isp_drc_ltm_l_lut_22_23       drc_ltm_l_lut_22_23;
	union reg_isp_drc_ltm_l_lut_24_25       drc_ltm_l_lut_24_25;
	union reg_isp_drc_ltm_l_lut_26_27       drc_ltm_l_lut_26_27;
	union reg_isp_drc_ltm_l_lut_28_29       drc_ltm_l_lut_28_29;
	union reg_isp_drc_ltm_l_lut_30_31       drc_ltm_l_lut_30_31;
	union reg_isp_drc_ltm_l_lut_32_33       drc_ltm_l_lut_32_33;
	union reg_isp_drc_ltm_l_lut_34_35       drc_ltm_l_lut_34_35;
	union reg_isp_drc_ltm_l_lut_36_37       drc_ltm_l_lut_36_37;
	union reg_isp_drc_ltm_l_lut_38_39       drc_ltm_l_lut_38_39;
	union reg_isp_drc_ltm_l_lut_40_41       drc_ltm_l_lut_40_41;
	union reg_isp_drc_ltm_l_lut_42_43       drc_ltm_l_lut_42_43;
	union reg_isp_drc_ltm_l_lut_44_45       drc_ltm_l_lut_44_45;
	union reg_isp_drc_ltm_l_lut_46_47       drc_ltm_l_lut_46_47;
	union reg_isp_drc_ltm_l_lut_48_49       drc_ltm_l_lut_48_49;
	union reg_isp_drc_ltm_l_lut_50_51       drc_ltm_l_lut_50_51;
	union reg_isp_drc_ltm_l_lut_52_53       drc_ltm_l_lut_52_53;
	union reg_isp_drc_ltm_l_lut_54_55       drc_ltm_l_lut_54_55;
	union reg_isp_drc_ltm_l_lut_56_57       drc_ltm_l_lut_56_57;
	union reg_isp_drc_ltm_l_lut_58_59       drc_ltm_l_lut_58_59;
	union reg_isp_drc_ltm_l_lut_60_61       drc_ltm_l_lut_60_61;
	union reg_isp_drc_ltm_l_lut_62_63       drc_ltm_l_lut_62_63;
	union reg_isp_drc_ltm_l_lut_64          drc_ltm_l_lut_64;
	union reg_isp_drc_ltm_r_lut_0_1         drc_ltm_r_lut_0_1;
	union reg_isp_drc_ltm_r_lut_2_3         drc_ltm_r_lut_2_3;
	union reg_isp_drc_ltm_r_lut_4_5         drc_ltm_r_lut_4_5;
	union reg_isp_drc_ltm_r_lut_6_7         drc_ltm_r_lut_6_7;
	union reg_isp_drc_ltm_r_lut_8_9         drc_ltm_r_lut_8_9;
	union reg_isp_drc_ltm_r_lut_10_11       drc_ltm_r_lut_10_11;
	union reg_isp_drc_ltm_r_lut_12_13       drc_ltm_r_lut_12_13;
	union reg_isp_drc_ltm_r_lut_14_15       drc_ltm_r_lut_14_15;
	union reg_isp_drc_ltm_r_lut_16          drc_ltm_r_lut_16;
};

struct sop_vip_isp_drc_config {
	__u8  update;
	__u8 drc_enable;
	__u8 drc_subimg_width;
	__u8 drc_subimg_height;
	__u16 drc_subimg_ratio_hori;
	__u16 drc_subimg_ratio_vert;
	__u16 drc_subimg_ratio_hori_div;
	__u16 drc_subimg_ratio_vert_div;
	__u8 drc_subimg_lpf_mode;
	__u8 drc_hist_enable;
	__u8 drc_hist_mode;
	__u8 drc_hist_step_x;
	__u8 drc_hist_step_y;
	__u8 drc_intensity_mode;
	__u8 drc_tone_curve_enable;

	struct sop_vip_isp_drc_tun_cfg burst_cfg;
};

struct sop_vip_isp_ee_ext_gamma_tun_cfg {
	union reg_ee_ext_ee_ext_gamma_0         ee_ext_gamma_0;
	union reg_ee_ext_ee_ext_gamma_1         ee_ext_gamma_1;
	union reg_ee_ext_ee_ext_gamma_2         ee_ext_gamma_2;
	union reg_ee_ext_ee_ext_gamma_3         ee_ext_gamma_3;
	union reg_ee_ext_ee_ext_gamma_4         ee_ext_gamma_4;
	union reg_ee_ext_ee_ext_gamma_5         ee_ext_gamma_5;
	union reg_ee_ext_ee_ext_gamma_6         ee_ext_gamma_6;
	union reg_ee_ext_ee_ext_gamma_7         ee_ext_gamma_7;
	union reg_ee_ext_ee_ext_gamma_8         ee_ext_gamma_8;
	union reg_ee_ext_ee_ext_gamma_9         ee_ext_gamma_9;
	union reg_ee_ext_ee_ext_gamma_10        ee_ext_gamma_10;
	union reg_ee_ext_ee_ext_gamma_11        ee_ext_gamma_11;
	union reg_ee_ext_ee_ext_gamma_12        ee_ext_gamma_12;
	union reg_ee_ext_ee_ext_gamma_13        ee_ext_gamma_13;
	union reg_ee_ext_ee_ext_gamma_14        ee_ext_gamma_14;
	union reg_ee_ext_ee_ext_gamma_15        ee_ext_gamma_15;
	union reg_ee_ext_ee_ext_gamma_16        ee_ext_gamma_16;
	union reg_ee_ext_ee_ext_gamma_17        ee_ext_gamma_17;
	union reg_ee_ext_ee_ext_gamma_18        ee_ext_gamma_18;
	union reg_ee_ext_ee_ext_gamma_19        ee_ext_gamma_19;
	union reg_ee_ext_ee_ext_gamma_20        ee_ext_gamma_20;
};

struct sop_vip_isp_ee_ext_coef_tun_cfg {
	union reg_ee_ext_ee_ext_e5c_0           ee_ext_e5c_0;
	union reg_ee_ext_ee_ext_e5c_1           ee_ext_e5c_1;
	union reg_ee_ext_ee_ext_e5c_2           ee_ext_e5c_2;
	union reg_ee_ext_ee_ext_e5a_0           ee_ext_e5a_0;
	union reg_ee_ext_ee_ext_e5a_1           ee_ext_e5a_1;
	union reg_ee_ext_ee_ext_e5a_2           ee_ext_e5a_2;
	union reg_ee_ext_ee_ext_e5b_0           ee_ext_e5b_0;
	union reg_ee_ext_ee_ext_e5b_1           ee_ext_e5b_1;
	union reg_ee_ext_ee_ext_e5b_2           ee_ext_e5b_2;
	union reg_ee_ext_ee_ext_e7_0            ee_ext_e7_0;
	union reg_ee_ext_ee_ext_e7_1            ee_ext_e7_1;
	union reg_ee_ext_ee_ext_e7_2            ee_ext_e7_2;
};

struct sop_vip_isp_ee_ext_region_0_tun_cfg {
	union reg_ee_ext_ee_ext_norm            ee_ext_norm;
	union reg_ee_ext_ee_ext_luma_blend      ee_ext_luma_blend;
	union reg_ee_ext_ee_ext_region_0        ee_ext_region_0;
	union reg_ee_ext_ee_ext_region_1        ee_ext_region_1;
};

struct sop_vip_isp_ee_ext_region_1_tun_cfg {
	union reg_ee_ext_ee_ext_region_2        ee_ext_region_2;
	union reg_ee_ext_ee_ext_region_3        ee_ext_region_3;
	union reg_ee_ext_ee_ext_region_4        ee_ext_region_4;
	union reg_ee_ext_ee_ext_region_5        ee_ext_region_5;
	union reg_ee_ext_ee_ext_region_6        ee_ext_region_6;
};

struct sop_vip_isp_ee_ext_config {
	__u8 update;
	__u8 enable;
	__u8 ee_debug_mode;
	__u8 ee_ch_selection;
	__u8 ee_gamma_selection;
	__u16 gamma_lut[2];

	struct sop_vip_isp_ee_ext_gamma_tun_cfg gamma_burst_cfg;
	struct sop_vip_isp_ee_ext_coef_tun_cfg cpef_burst_cfg;
	struct sop_vip_isp_ee_ext_region_0_tun_cfg reg0_burst_cfg;
	struct sop_vip_isp_ee_ext_region_1_tun_cfg reg1_burst_cfg;
};

struct sop_vip_isp_pfr_tun_cfg_0 {
	union reg_pfr_pfr_luma_level0           pfr_luma_level0;
};

struct sop_vip_isp_pfr_tun_cfg_1 {
	union reg_pfr_pfr_luma_level1           pfr_luma_level1;
	union reg_pfr_pfr_luma_level2           pfr_luma_level2;
	union reg_pfr_pfr_luma_level3           pfr_luma_level3;
	union reg_pfr_pfr_luma_level4           pfr_luma_level4;
	union reg_pfr_pfr_color_uv              pfr_color_uv;
	union reg_pfr_pfr_uv_diff_th            pfr_uv_diff_th;
	union reg_pfr_pfr_uv_diff_lut0          pfr_uv_diff_lut0;
};

struct sop_vip_isp_pfr_tun_cfg_2 {
	union reg_pfr_pfr_uv_diff_lut1          pfr_uv_diff_lut1;
	union reg_pfr_pfr_rb_wet                pfr_rb_wet;
	union reg_pfr_pfr_hueset                pfr_hueset;
};

struct sop_vip_isp_pfr_tun_cfg_3 {
	union reg_pfr_pfr_hue_range             pfr_hue_range;
	union reg_pfr_pfr_hue_th                pfr_hue_th;
	union reg_pfr_pfr_g_diff_th0            pfr_g_diff_th0;
	union reg_pfr_pfr_g_diff_th1            pfr_g_diff_th1;
	union reg_pfr_pfr_edge_th               pfr_edge_th;
};

struct sop_vip_isp_pfr_config {
	__u8 update;
	__u8 pfr_en;
	__u8 pfr_luma_level_en;
	__u8 pfr_uvset_en[2];
	__u8 pfr_hueset_en[2];
	__u8 pfr_luma_level_th;
	__u8 pfr_luma_inivalue;

	struct sop_vip_isp_pfr_tun_cfg_0 burst0_cfg;
	struct sop_vip_isp_pfr_tun_cfg_1 burst1_cfg;
	struct sop_vip_isp_pfr_tun_cfg_2 burst2_cfg;
	struct sop_vip_isp_pfr_tun_cfg_3 burst3_cfg;
};

struct sop_vip_isp_cnr_tun_cfg_0 {
	union reg_cnr_cnr_cmf_ksize             cnr_cmf_ksize;
	union reg_cnr_cnr_ife2_fsize_sel        cnr_ife2_fsize_sel;
	union reg_cnr_cnr_ife2_eksize_sel       cnr_ife2_eksize_sel;
	union reg_cnr_cnr_ife2_y_rcth           cnr_ife2_y_rcth;
	union reg_cnr_cnr_ife2_y_cwt            cnr_ife2_y_cwt;
	union reg_cnr_cnr_ife2_y_rcwt           cnr_ife2_y_rcwt;
	union reg_cnr_cnr_ife2_y_outl_th        cnr_ife2_y_outl_th;
	union reg_cnr_cnr_ife2_uv_rcth          cnr_ife2_uv_rcth;
	union reg_cnr_cnr_ife2_uv_cwt           cnr_ife2_uv_cwt;
	union reg_cnr_cnr_ife2_uv_rcwt          cnr_ife2_uv_rcwt;
	union reg_cnr_cnr_ife2_uv_outl_th       cnr_ife2_uv_outl_th;
	union reg_cnr_cnr_ife2_y_outl_dth       cnr_ife2_y_outl_dth;
	union reg_cnr_cnr_ife2_u_outl_dth       cnr_ife2_u_outl_dth;
	union reg_cnr_cnr_ife2_v_outl_dth       cnr_ife2_v_outl_dth;
	union reg_cnr_cnr_ife2_ed_pn_th         cnr_ife2_ed_pn_th;
	union reg_cnr_cnr_ife2_ed_hv_th         cnr_ife2_ed_hv_th;
	union reg_cnr_cnr_ife2_y_fth            cnr_ife2_y_fth;
	union reg_cnr_cnr_ife2_y_fth_4          cnr_ife2_y_fth_4;
	union reg_cnr_cnr_ife2_y_fwt            cnr_ife2_y_fwt;
	union reg_cnr_cnr_ife2_u_fth            cnr_ife2_u_fth;
	union reg_cnr_cnr_ife2_u_fth_4          cnr_ife2_u_fth_4;
	union reg_cnr_cnr_ife2_u_fwt            cnr_ife2_u_fwt;
	union reg_cnr_cnr_ife2_v_fth            cnr_ife2_v_fth;
	union reg_cnr_cnr_ife2_v_fth_4          cnr_ife2_v_fth_4;
	union reg_cnr_cnr_ife2_v_fwt            cnr_ife2_v_fwt;
	union reg_cnr_cnr_ife2_yftr_en          cnr_ife2_yftr_en;
	union reg_cnr_cnr_ife2_egd_en           cnr_ife2_egd_en;
	union reg_cnr_cnr_ife2_rc_en            cnr_ife2_rc_en;
	union reg_cnr_cnr_chra_refy_wt          cnr_chra_refy_wt;
	union reg_cnr_cnr_chra_refc_wt          cnr_chra_refc_wt;
	union reg_cnr_cnr_chra_out_wt_lut       cnr_chra_out_wt_lut;
	union reg_cnr_cnr_chra_y_rng            cnr_chra_y_rng;
	union reg_cnr_cnr_chra_y_wtprc          cnr_chra_y_wtprc;
	union reg_cnr_cnr_chra_y_th             cnr_chra_y_th;
	union reg_cnr_cnr_chra_y_wts            cnr_chra_y_wts;
	union reg_cnr_cnr_chra_y_wte            cnr_chra_y_wte;
	union reg_cnr_cnr_chra_uv_rng           cnr_chra_uv_rng;
	union reg_cnr_cnr_chra_uv_wtprc         cnr_chra_uv_wtprc;
	union reg_cnr_cnr_chra_uv_th            cnr_chra_uv_th;
	union reg_cnr_cnr_chra_uv_wts           cnr_chra_uv_wts;
	union reg_cnr_cnr_chra_uv_wte           cnr_chra_uv_wte;
	union reg_cnr_cnr_chra_sat_rng          cnr_chra_sat_rng;
	union reg_cnr_cnr_chra_sat_wtprc        cnr_chra_sat_wtprc;
	union reg_cnr_cnr_chra_sat_th           cnr_chra_sat_th;
	union reg_cnr_cnr_chra_sat_wts          cnr_chra_sat_wts;
	union reg_cnr_cnr_chra_sat_wte          cnr_chra_sat_wte;
	union reg_cnr_cnr_chra_sat_outbld_coring  cnr_chra_sat_outbld_coring;
};


struct sop_vip_isp_cnr_config {
	__u8 update;
	__u8 cnr_enable;
	__u8 cnr_scale_shift;

	__u8 cnr_cmf_en;
	__u8 cnr_lca_enable;
	__u8 cnr_ife2_filter_en;
	__u8 cnr_chra_en;
	__u8 cnr_m_chra_bypass;
	__u8 cnr_chra_dbgmode;
	__u8 cnr_chra_sat_outbld_en;

	__u8 cnr_subim_outsel;

	struct sop_vip_isp_cnr_tun_cfg_0 burst0_cfg;
};

struct sop_vip_isp_tnr_sad_tun_cfg {
	union reg_isp_444_422_7                 reg_7;
	union reg_isp_444_422_8                 reg_8;
	union reg_isp_444_422_9                 reg_9;
	union reg_isp_444_422_10                reg_10;
	union reg_isp_444_422_11                reg_11;
	union reg_isp_444_422_12                reg_12;
	union reg_isp_444_422_13                reg_13;
	union reg_isp_444_422_14                reg_14;
	union reg_isp_444_422_15                reg_15;
	union reg_isp_444_422_16                reg_16;
	union reg_isp_444_422_17                reg_17;
	union reg_isp_444_422_18                reg_18;
	union reg_isp_444_422_19                reg_19;
	union reg_isp_444_422_20                reg_20;
	union reg_isp_444_422_21                reg_21;
	union reg_isp_444_422_22                reg_22;
	union reg_isp_444_422_23                reg_23;
	union reg_isp_444_422_24                reg_24;
	union reg_isp_444_422_25                reg_25;
	union reg_isp_444_422_26                reg_26;
	union reg_isp_444_422_27                reg_27;
	union reg_isp_444_422_28                reg_28;
	union reg_isp_444_422_29                reg_29;
	union reg_isp_444_422_30                reg_30;
	union reg_isp_444_422_31                reg_31;
};

struct sop_vip_isp_tnr_misc_tun_cfg {
	union reg_isp_444_422_32                reg_32;
	union reg_isp_444_422_33                reg_33;
	union reg_isp_444_422_34                reg_34;
	union reg_isp_444_422_35                reg_35;
	union reg_isp_444_422_36                reg_36;
	union reg_isp_444_422_37                reg_37;
	union reg_isp_444_422_38                reg_38;
	union reg_isp_444_422_39                reg_39;
	union reg_isp_444_422_40                reg_40;
	union reg_isp_444_422_41                reg_41;
	union reg_isp_444_422_42                reg_42;
	union reg_isp_444_422_43                reg_43;
	union reg_isp_444_422_44                reg_44;
	union reg_isp_444_422_45                reg_45;
};

struct sop_vip_isp_tnr_ych_tun_cfg {
	union reg_isp_444_422_46                reg_46;
	union reg_isp_444_422_47                reg_47;
	union reg_isp_444_422_48                reg_48;
	union reg_isp_444_422_49                reg_49;
	union reg_isp_444_422_50                reg_50;
	union reg_isp_444_422_51                reg_51;
	union reg_isp_444_422_52                reg_52;
	union reg_isp_444_422_53                reg_53;
	union reg_isp_444_422_54                reg_54;
	union reg_isp_444_422_55                reg_55;
	union reg_isp_444_422_56                reg_56;
	union reg_isp_444_422_57                reg_57;
	union reg_isp_444_422_58                reg_58;
	union reg_isp_444_422_59                reg_59;
	union reg_isp_444_422_60                reg_60;
	union reg_isp_444_422_61                reg_61;
	union reg_isp_444_422_62                reg_62;
	union reg_isp_444_422_63                reg_63;
	union reg_isp_444_422_64                reg_64;
	union reg_isp_444_422_65                reg_65;
	union reg_isp_444_422_66                reg_66;
	union reg_isp_444_422_67                reg_67;
	union reg_isp_444_422_68                reg_68;
	union reg_isp_444_422_69                reg_69;
	union reg_isp_444_422_70                reg_70;
	union reg_isp_444_422_71                reg_71;
	union reg_isp_444_422_72                reg_72;
	union reg_isp_444_422_73                reg_73;
	union reg_isp_444_422_74                reg_74;
	union reg_isp_444_422_75                reg_75;
	union reg_isp_444_422_76                reg_76;
	union reg_isp_444_422_77                reg_77;
	union reg_isp_444_422_78                reg_78;
	union reg_isp_444_422_79                reg_79;
	union reg_isp_444_422_80                reg_80;
	union reg_isp_444_422_81                reg_81;
	union reg_isp_444_422_82                reg_82;
	union reg_isp_444_422_83                reg_83;
	union reg_isp_444_422_84                reg_84;
	union reg_isp_444_422_85                reg_85;
	union reg_isp_444_422_86                reg_86;
	union reg_isp_444_422_87                reg_87;
	union reg_isp_444_422_88                reg_88;
	union reg_isp_444_422_89                reg_89;
	union reg_isp_444_422_90                reg_90;
	union reg_isp_444_422_91                reg_91;
	union reg_isp_444_422_92                reg_92;
	union reg_isp_444_422_93                reg_93;
	union reg_isp_444_422_94                reg_94;
	union reg_isp_444_422_95                reg_95;
	union reg_isp_444_422_96                reg_96;
	union reg_isp_444_422_97                reg_97;
	union reg_isp_444_422_98                reg_98;
	union reg_isp_444_422_99                reg_99;
	union reg_isp_444_422_100               reg_100;
	union reg_isp_444_422_101               reg_101;
	union reg_isp_444_422_102               reg_102;
	union reg_isp_444_422_103               reg_103;
	union reg_isp_444_422_104               reg_104;
	union reg_isp_444_422_105               reg_105;
	union reg_isp_444_422_106               reg_106;
	union reg_isp_444_422_107               reg_107;
	union reg_isp_444_422_108               reg_108;
	union reg_isp_444_422_109               reg_109;
	union reg_isp_444_422_110               reg_110;
	union reg_isp_444_422_111               reg_111;
	union reg_isp_444_422_112               reg_112;
	union reg_isp_444_422_113               reg_113;
	union reg_isp_444_422_114               reg_114;
	union reg_isp_444_422_115               reg_115;
	union reg_isp_444_422_reg116            reg_116;
};

struct sop_vip_isp_tnr_config {
	__u8  update;
	__u8  tdnr_enable;
	__u8  reg_3dnr_debug_mode;
	__u8  reg_y_debug_en;
	__u8  reg_3dnr_subpixel_enable;
	__u8  reg_3dnr_y_L0_PSS_blur;
	__u8  reg_3dnr_luma_jnd_ratio;
	__u8  reg_3dnr_sad_lpf_mode;

	__u8 y_bundle_nr_enable;
	__u8 uv_bundle_nr_enable;
	__u8 mono_enable;
	__u8 mono_uv_val;

	struct sop_vip_isp_tnr_sad_tun_cfg sad_burst_cfg;
	struct sop_vip_isp_tnr_misc_tun_cfg misc_burst_cfg;
	struct sop_vip_isp_tnr_ych_tun_cfg ych_burst_cfg;
};

struct sop_vip_isp_ee_nlut_tun_cfg {
	union reg_isp_ee_a4                     reg_a4;
	union reg_isp_ee_a8                     reg_a8;
	union reg_isp_ee_ac                     reg_ac;
	union reg_isp_ee_b0                     reg_b0;
	union reg_isp_ee_b4                     reg_b4;
	union reg_isp_ee_b8                     reg_b8;
	union reg_isp_ee_bc                     reg_bc;
	union reg_isp_ee_c0                     reg_c0;
	union reg_isp_ee_c4                     reg_c4;
	union reg_isp_ee_c8                     reg_c8;
	union reg_isp_ee_hcc                    reg_hcc;
	union reg_isp_ee_hd0                    reg_hd0;
	union reg_isp_ee_hd4                    reg_hd4;
	union reg_isp_ee_hd8                    reg_hd8;
};

struct sop_vip_isp_chroma_lut_tun_cfg {
	union reg_isp_ee_178                    reg_178;
	union reg_isp_ee_17c                    reg_17c;
	union reg_isp_ee_180                    reg_180;
	union reg_isp_ee_184                    reg_184;
	union reg_isp_ee_188                    reg_188;
	union reg_isp_ee_18c                    reg_18c;
	union reg_isp_ee_190                    reg_190;
	union reg_isp_ee_194                    reg_194;
	union reg_isp_ee_198                    reg_198;
	union reg_isp_ee_19c                    reg_19c;
	union reg_isp_ee_1a0                    reg_1a0;
	union reg_isp_ee_1a4                    reg_1a4;
	union reg_isp_ee_1a8                    reg_1a8;
};

struct sop_vip_isp_ee_config {
	__u8  update;
	__u8  ee_enable;
	__u8  ee_debug_mode;
	__u8  ee_coring_th;
	__u8  ee_chroma_adptctrl_en;

	__u8  ee_overshoot_clip_ratio;
	__u8  ee_undershoot_clip_ratio;

	__u8  ee_delta_wt_src_opt;
	__u8  ee_delta_wt_coring_th;
	__u8  ee_delta_wt_gain;
	__u8  ee_noise_level;

	__u8  ee_blend_degamma;
	__u8  ee_sharp_str;

	__u8  ee_con_eng_wt;
	__u8  ee_flat_th;
	__u8  ee_edge_th;
	__u8  ee_con_eng_slope;
	__u8  ee_flat_region_str;
	__u8  ee_edge_region_str;
	__u8  ee_motion_enable;
	__u8  ee_motion_delta_wt_str;
	__u8  ee_static_delta_wt_str;
	__u8  ee_trans_delta_wt_str;
	__u8  ee_filter_size_opt;



	struct sop_vip_isp_ee_nlut_tun_cfg nlut_burst_cfg;
	struct sop_vip_isp_chroma_lut_tun_cfg chra_burst_cfg;
};

struct sop_vip_isp_pre_ee_tun_cfg {
	union reg_ee_add_ee_add_overshoot_0     ee_add_overshoot_0;
	union reg_ee_add_ee_add_overshoot_1     ee_add_overshoot_1;
	union reg_ee_add_ee_add_overshoot_2     ee_add_overshoot_2;
	union reg_ee_add_ee_add_overshoot_3     ee_add_overshoot_3;
	union reg_ee_add_ee_add_overshoot_4     ee_add_overshoot_4;
	union reg_ee_add_ee_add_refine_0        ee_add_refine_0;
	union reg_ee_add_ee_add_refine_1        ee_add_refine_1;
	union reg_ee_add_ee_add_refine_2        ee_add_refine_2;
	union reg_ee_add_ee_add_refine_3        ee_add_refine_3;
	union reg_ee_add_ee_add_refine_4        ee_add_refine_4;
	union reg_ee_add_ee_add_refine_5        ee_add_refine_5;
	union reg_ee_add_ee_add_refine_6        ee_add_refine_6;
	union reg_ee_add_ee_add_refine_7        ee_add_refine_7;
	union reg_ee_add_ee_add_refine_8        ee_add_refine_8;
	union reg_ee_add_ee_add_refine_9        ee_add_refine_9;
	union reg_ee_add_ee_add_chroma_0        ee_add_chroma_0;
	union reg_ee_add_ee_add_chroma_1        ee_add_chroma_1;
	union reg_ee_add_ee_add_chroma_2        ee_add_chroma_2;
	union reg_ee_add_ee_add_chroma_3        ee_add_chroma_3;
	union reg_ee_add_ee_add_chroma_4        ee_add_chroma_4;
	union reg_ee_add_ee_add_chroma_5        ee_add_chroma_5;
	union reg_ee_add_ee_add_chroma_6        ee_add_chroma_6;
	union reg_ee_add_ee_add_chroma_7        ee_add_chroma_7;
	union reg_ee_add_ee_add_chroma_8        ee_add_chroma_8;
	union reg_ee_add_ee_add_chroma_9        ee_add_chroma_9;
};

struct sop_vip_isp_pre_ee_config {
	__u8  update;
	__u8 ee_enable;
	__u8 ee_debug_mode;

	struct sop_vip_isp_pre_ee_tun_cfg burst_cfg;
};

struct sop_vip_isp_fusion_tun_cfg {
	union reg_fusion_ds1             fusion_ds1;
	union reg_fusion_ds2             fusion_ds2;
	union reg_fusion_bcl_range       fusion_bcl_range;
	union reg_fusion_bcl_p0          fusion_bcl_p0;
	union reg_fusion_bcl_slop        fusion_bcl_slop;
	union reg_fusion_bcs_range       fusion_bcs_range;
	union reg_fusion_bcs_p0          fusion_bcs_p0;
	union reg_fusion_bcs_slop        fusion_bcs_slop;
	union reg_fusion_diff            fusion_diff;
};

struct sop_vip_isp_fusion_config {
	__u8  update;
	__u8  enable;
	__u8 u1_fusion_en;
	__u8 u2_fusion_mode;
	__u8 u4_fusion_dbg_mode;
	__u16 u13_fusion_evratio;
	__u8 u4_fusion_ev_fmt;
	__u8 u2_fusion_fnum;
	__u8 u2_fusion_ysel;
	__u8 u2_fusion_nsel;
	__u8 u2_fusion_dsel;

	__u8 u5_diff_dw16[16];

	struct sop_vip_isp_fusion_tun_cfg burst_cfg;
};

struct sop_vip_isp_map_curve_tun_cfg {
	union reg_map_curve_fcurve_flumw_lut0   fcurve_flumw_lut0;
	union reg_map_curve_fcurve_flumw_lut1   fcurve_flumw_lut1;
	union reg_map_curve_fcurve_flumw_lut2   fcurve_flumw_lut2;
	union reg_map_curve_fcurve_flumw_lut3   fcurve_flumw_lut3;
	union reg_map_curve_fcurve_flumw_lut4   fcurve_flumw_lut4;
	union reg_map_curve_fcurve_l0           fcurve_l0;
	union reg_map_curve_fcurve_l1           fcurve_l1;
	union reg_map_curve_fcurve_l2           fcurve_l2;
	union reg_map_curve_fcurve_l3           fcurve_l3;
	union reg_map_curve_fcurve_l4           fcurve_l4;
	union reg_map_curve_fcurve_l5           fcurve_l5;
	union reg_map_curve_fcurve_l6           fcurve_l6;
	union reg_map_curve_fcurve_l7           fcurve_l7;
	union reg_map_curve_fcurve_l8           fcurve_l8;
	union reg_map_curve_fcurve_l9           fcurve_l9;
	union reg_map_curve_fcurve_l10          fcurve_l10;
	union reg_map_curve_fcurve_l11          fcurve_l11;
	union reg_map_curve_fcurve_l12          fcurve_l12;
	union reg_map_curve_fcurve_l13          fcurve_l13;
	union reg_map_curve_fcurve_l14          fcurve_l14;
	union reg_map_curve_fcurve_l15          fcurve_l15;
	union reg_map_curve_fcurve_l16          fcurve_l16;
	union reg_map_curve_fcurve_l17          fcurve_l17;
	union reg_map_curve_fcurve_l18          fcurve_l18;
	union reg_map_curve_fcurve_l19          fcurve_l19;
	union reg_map_curve_fcurve_l20          fcurve_l20;
	union reg_map_curve_fcurve_l21          fcurve_l21;
	union reg_map_curve_fcurve_l22          fcurve_l22;
	union reg_map_curve_fcurve_l23          fcurve_l23;
	union reg_map_curve_fcurve_l24          fcurve_l24;
	union reg_map_curve_fcurve_l25          fcurve_l25;
	union reg_map_curve_fcurve_l26          fcurve_l26;
	union reg_map_curve_fcurve_l27          fcurve_l27;
	union reg_map_curve_fcurve_l28          fcurve_l28;
	union reg_map_curve_fcurve_l29          fcurve_l29;
	union reg_map_curve_fcurve_l30          fcurve_l30;
	union reg_map_curve_fcurve_l31          fcurve_l31;
	union reg_map_curve_fcurve_l32          fcurve_l32;
	union reg_map_curve_fcurve_l33          fcurve_l33;
	union reg_map_curve_fcurve_l34          fcurve_l34;
	union reg_map_curve_fcurve_l35          fcurve_l35;
	union reg_map_curve_fcurve_l36          fcurve_l36;
	union reg_map_curve_fcurve_l37          fcurve_l37;
	union reg_map_curve_fcurve_l38          fcurve_l38;
	union reg_map_curve_fcurve_l39          fcurve_l39;
	union reg_map_curve_fcurve_l40          fcurve_l40;
	union reg_map_curve_fcurve_l41          fcurve_l41;
	union reg_map_curve_fcurve_l42          fcurve_l42;
	union reg_map_curve_fcurve_l43          fcurve_l43;
	union reg_map_curve_fcurve_l44          fcurve_l44;
	union reg_map_curve_fcurve_l45          fcurve_l45;
	union reg_map_curve_fcurve_l46          fcurve_l46;
	union reg_map_curve_fcurve_l47          fcurve_l47;
	union reg_map_curve_fcurve_l48          fcurve_l48;
	union reg_map_curve_fcurve_l49          fcurve_l49;
	union reg_map_curve_fcurve_l50          fcurve_l50;
	union reg_map_curve_fcurve_l51          fcurve_l51;
	union reg_map_curve_fcurve_l52          fcurve_l52;
	union reg_map_curve_fcurve_l53          fcurve_l53;
	union reg_map_curve_fcurve_l54          fcurve_l54;
	union reg_map_curve_fcurve_l55          fcurve_l55;
	union reg_map_curve_fcurve_l56          fcurve_l56;
	union reg_map_curve_fcurve_l57          fcurve_l57;
	union reg_map_curve_fcurve_l58          fcurve_l58;
	union reg_map_curve_fcurve_l59          fcurve_l59;
	union reg_map_curve_fcurve_l60          fcurve_l60;
	union reg_map_curve_fcurve_l61          fcurve_l61;
	union reg_map_curve_fcurve_l62          fcurve_l62;
	union reg_map_curve_fcurve_l63          fcurve_l63;
	union reg_map_curve_fcurve_l64          fcurve_l64;
	union reg_map_curve_fcurve_r0           fcurve_r0;
	union reg_map_curve_fcurve_r1           fcurve_r1;
	union reg_map_curve_fcurve_r2           fcurve_r2;
	union reg_map_curve_fcurve_r3           fcurve_r3;
	union reg_map_curve_fcurve_r4           fcurve_r4;
	union reg_map_curve_fcurve_r5           fcurve_r5;
	union reg_map_curve_fcurve_r6           fcurve_r6;
	union reg_map_curve_fcurve_r7           fcurve_r7;
	union reg_map_curve_fcurve_r8           fcurve_r8;
	union reg_map_curve_fcurve_r9           fcurve_r9;
	union reg_map_curve_fcurve_r10          fcurve_r10;
	union reg_map_curve_fcurve_r11          fcurve_r11;
	union reg_map_curve_fcurve_r12          fcurve_r12;
	union reg_map_curve_fcurve_r13          fcurve_r13;
	union reg_map_curve_fcurve_r14          fcurve_r14;
	union reg_map_curve_fcurve_r15          fcurve_r15;
	union reg_map_curve_fcurve_r16          fcurve_r16;
	union reg_map_curve_fcurve_end0         fcurve_end0;
	union reg_map_curve_fcurve_end1         fcurve_end1;
	union reg_map_curve_fcurve_end2         fcurve_end2;
	union reg_map_curve_fcurve_end3         fcurve_end3;
	union reg_map_curve_fcurve_end4         fcurve_end4;
	union reg_map_curve_fcurve_end5         fcurve_end5;
	union reg_map_curve_fcurve_end6         fcurve_end6;
	union reg_map_curve_fcurve_end7         fcurve_end7;
	union reg_map_curve_fcurve_end8         fcurve_end8;
	union reg_map_curve_fcurve_end9         fcurve_end9;
	union reg_map_curve_fcurve_end10        fcurve_end10;
	union reg_map_curve_fcurve_end11        fcurve_end11;
	union reg_map_curve_fcurve_end12        fcurve_end12;
	union reg_map_curve_fcurve_end13        fcurve_end13;
	union reg_map_curve_fcurve_end14        fcurve_end14;
	union reg_map_curve_fcurve_end15        fcurve_end15;
	union reg_map_curve_fcurve_end16        fcurve_end16;
};
struct sop_vip_isp_map_curve_config {
	__u8 update;
	__u8 u1_fcurve16_en;
	__u8 u2_fcurve16_ysel;
	__u8 u4_fcurve16_yvwet;
	__u8 u4_ife_f_fcurve_ev_fmt;

	struct sop_vip_isp_map_curve_tun_cfg burst_cfg;
};

struct sop_vip_isp_ycur_config {
	__u8  update;
	__u8  enable;
	__u8  lut[64];
	__u16 lut_256;
};

struct sop_vip_isp_dci_config {
	__u8  update;
	__u8  dci_enable;
	__u8  dci_histsample_step;
	__u8  dci_roi_enable;
	__u16  dci_roi_start_x;
	__u16  dci_roi_start_y;
	__u16  dci_roi_width;
	__u16  dci_roi_height;
	__u16  dci_ygamma_curve[256];
};

struct sop_vip_isp_ldci_tun_0_cfg {
	union reg_ldci_ldci_reciprocal_0        ldci_reciprocal_0;
	union reg_ldci_ldci_luma_prot_lut0      ldci_luma_prot_lut0;
	union reg_ldci_ldci_luma_prot_lut1      ldci_luma_prot_lut1;
	union reg_ldci_ldci_luma_prot_lut2      ldci_luma_prot_lut2;
	union reg_ldci_ldci_luma_prot_lut3      ldci_luma_prot_lut3;
	union reg_ldci_ldci_luma_prot_lut4      ldci_luma_prot_lut4;
	union reg_ldci_ldci_blk_num             ldci_blk_num;
	union reg_ldci_ldci_blk_size            ldci_blk_size;
	union reg_ldci_ldci_reciprocal_1        ldci_reciprocal_1;
};

struct sop_vip_isp_ldci_tun_1_cfg {
	union reg_ldci_ldci_luma_gain_lut0      ldci_luma_gain_lut0;
	union reg_ldci_ldci_luma_gain_lut1      ldci_luma_gain_lut1;
	union reg_ldci_ldci_luma_gain_lut2      ldci_luma_gain_lut2;
	union reg_ldci_ldci_thr                 ldci_thr;
	union reg_ldci_ldci_diff_gain_lut_l0    ldci_diff_gain_lut_l0;
	union reg_ldci_ldci_diff_gain_lut_l1    ldci_diff_gain_lut_l1;
	union reg_ldci_ldci_diff_gain_lut_r0    ldci_diff_gain_lut_r0;
	union reg_ldci_ldci_diff_gain_lut_r1    ldci_diff_gain_lut_r1;
};

struct sop_vip_isp_ldci_config {
	__u8 update;
	__u8 ldci_enable;
	__u8 ldci_luma_prot_en;

	struct sop_vip_isp_ldci_tun_0_cfg burst_0_cfg;
	struct sop_vip_isp_ldci_tun_1_cfg burst_1_cfg;
};

struct sop_vip_isp_csc_config {
	__u8  update;
	__u8  enable;
	__s16 coeff[9];
	__s16 offset[3];
};

struct sop_vip_isp_dpc_tun_cfg {
	union reg_dpc_dark_threshold0           dark_threshold0;
	union reg_dpc_dark_threshold1           dark_threshold1;
	union reg_dpc_dark_threshold2           dark_threshold2;
	union reg_dpc_bright_threshold0         bright_threshold0;
	union reg_dpc_bright_threshold1         bright_threshold1;
	union reg_dpc_bright_threshold2         bright_threshold2;
	union reg_dpc_threshold_offset          threshold_offset;
	union reg_dpc_outlier_cnt               outlier_cnt;
	union reg_dpc_outlier_config            outlier_config;
};
struct sop_vip_isp_dpc_config {
	__u8  update;
	__u8  dpc_enable;
	__u8  spc_enable;
	__u32  spc_defect_lut[4096];
	__u32  bp_cnt;

	struct sop_vip_isp_dpc_tun_cfg burst_cfg;
};

struct sop_isp_ae_tun_cfg {
	union reg_isp_ae_hist_ae_face_enable_ctrl  ae_face_enable_ctrl;
	union reg_isp_ae_hist_ae_face0_sts_div  ae_face0_sts_div;
	union reg_isp_ae_hist_ae_face1_sts_div  ae_face1_sts_div;
	union reg_isp_ae_hist_ae_face2_sts_div  ae_face2_sts_div;
	union reg_isp_ae_hist_ae_face3_sts_div  ae_face3_sts_div;
	union reg_isp_ae_hist_sts_enable        sts_enable;
	union reg_isp_ae_hist_ae_algo_enable    ae_algo_enable;
	union reg_isp_ae_hist_ae_hist_low       ae_hist_low;
	union reg_isp_ae_hist_ae_hist_high      ae_hist_high;
	union reg_isp_ae_hist_ae_top            ae_top;
	union reg_isp_ae_hist_ae_bot            ae_bot;
	union reg_isp_ae_hist_ae_overexp_thr    ae_overexp_thr;
	union reg_isp_ae_hist_ae_num_gapline    ae_num_gapline;
	union reg_isp_ae_hist_ae_choose_gr_en   ae_choose_gr_en;
};

struct sop_isp_ae_1_tun_cfg {
	union reg_isp_ae_hist_se_ae_blc_offset_r  se_ae_blc_offset_r;
	union reg_isp_ae_hist_se_ae_blc_offset_gr  se_ae_blc_offset_gr;
	union reg_isp_ae_hist_se_ae_blc_offset_gb  se_ae_blc_offset_gb;
	union reg_isp_ae_hist_se_ae_blc_offset_b  se_ae_blc_offset_b;
	union reg_isp_ae_hist_se_ae_blc_offset_ir  se_ae_blc_offset_ir;
	union reg_isp_ae_hist_se_ae_blc_gain_r  se_ae_blc_gain_r;
	union reg_isp_ae_hist_se_ae_blc_gain_gr  se_ae_blc_gain_gr;
	union reg_isp_ae_hist_se_ae_blc_gain_gb  se_ae_blc_gain_gb;
	union reg_isp_ae_hist_se_ae_blc_gain_b  se_ae_blc_gain_b;
	union reg_isp_ae_hist_se_ae_blc_gain_ir  se_ae_blc_gain_ir;
	union reg_isp_ae_hist_se_ae_blc_enable  se_ae_blc_enable;
};

struct sop_isp_ae_2_tun_cfg {
	union reg_isp_ae_hist_ae_wgt_00         ae_wgt_00;
	union reg_isp_ae_hist_ae_wgt_01         ae_wgt_01;
	union reg_isp_ae_hist_ae_wgt_02         ae_wgt_02;
	union reg_isp_ae_hist_ae_wgt_03         ae_wgt_03;
	union reg_isp_ae_hist_ae_wgt_04         ae_wgt_04;
	union reg_isp_ae_hist_ae_wgt_05         ae_wgt_05;
	union reg_isp_ae_hist_ae_wgt_06         ae_wgt_06;
	union reg_isp_ae_hist_ae_wgt_07         ae_wgt_07;
	union reg_isp_ae_hist_ae_wgt_08         ae_wgt_08;
	union reg_isp_ae_hist_ae_wgt_09         ae_wgt_09;
	union reg_isp_ae_hist_ae_wgt_10         ae_wgt_10;
	union reg_isp_ae_hist_ae_wgt_11         ae_wgt_11;
	union reg_isp_ae_hist_ae_wgt_12         ae_wgt_12;
	union reg_isp_ae_hist_ae_wgt_13         ae_wgt_13;
	union reg_isp_ae_hist_ae_wgt_14         ae_wgt_14;
	union reg_isp_ae_hist_ae_wgt_15         ae_wgt_15;
	union reg_isp_ae_hist_ae_wgt_16         ae_wgt_16;
	union reg_isp_ae_hist_ae_wgt_17         ae_wgt_17;
	union reg_isp_ae_hist_ae_wgt_18         ae_wgt_18;
	union reg_isp_ae_hist_ae_wgt_19         ae_wgt_19;
	union reg_isp_ae_hist_ae_wgt_20         ae_wgt_20;
	union reg_isp_ae_hist_ae_wgt_21         ae_wgt_21;
	union reg_isp_ae_hist_ae_wgt_22         ae_wgt_22;
	union reg_isp_ae_hist_ae_wgt_23         ae_wgt_23;
	union reg_isp_ae_hist_ae_wgt_24         ae_wgt_24;
	union reg_isp_ae_hist_ae_wgt_25         ae_wgt_25;
	union reg_isp_ae_hist_ae_wgt_26         ae_wgt_26;
	union reg_isp_ae_hist_ae_wgt_27         ae_wgt_27;
	union reg_isp_ae_hist_ae_wgt_28         ae_wgt_28;
	union reg_isp_ae_hist_ae_wgt_29         ae_wgt_29;
	union reg_isp_ae_hist_ae_wgt_30         ae_wgt_30;
	union reg_isp_ae_hist_ae_wgt_31         ae_wgt_31;
	union reg_isp_ae_hist_le_ae_blc_offset_r  le_ae_blc_offset_r;
	union reg_isp_ae_hist_le_ae_blc_offset_gr  le_ae_blc_offset_gr;
	union reg_isp_ae_hist_le_ae_blc_offset_gb  le_ae_blc_offset_gb;
	union reg_isp_ae_hist_le_ae_blc_offset_b  le_ae_blc_offset_b;
	union reg_isp_ae_hist_le_ae_blc_offset_ir  le_ae_blc_offset_ir;
	union reg_isp_ae_hist_le_ae_blc_gain_r  le_ae_blc_gain_r;
	union reg_isp_ae_hist_le_ae_blc_gain_gr  le_ae_blc_gain_gr;
	union reg_isp_ae_hist_le_ae_blc_gain_gb  le_ae_blc_gain_gb;
	union reg_isp_ae_hist_le_ae_blc_gain_b  le_ae_blc_gain_b;
	union reg_isp_ae_hist_le_ae_blc_gain_ir  le_ae_blc_gain_ir;
	union reg_isp_ae_hist_le_ae_blc_enable  le_ae_blc_enable;
};

struct sop_vip_isp_ae_config {
	__u8  update;
	__u8  inst;
	__u8  ae_enable;
	__u16 ae_offsetx;
	__u16 ae_offsety;
	__u8  ae_numx;
	__u8  ae_numy;
	__u16 ae_width;
	__u16 ae_height;
	__u16 ae_sts_div;

	__u16 ae_face_offset_x[4];
	__u16 ae_face_offset_y[4];
	__u8  ae_face_size_minus1_x[4];
	__u8  ae_face_size_minus1_y[4];
	__u8  se_ae_blc_enable;
	__u8  le_ae_blc_enable;
	__u16 se_ae_blc_offset_r;
	__u16 se_ae_blc_offset_gr;
	__u16 se_ae_blc_offset_gb;
	__u16 se_ae_blc_offset_b;
	__u16 se_ae_blc_gain_r;
	__u16 se_ae_blc_gain_gr;
	__u16 se_ae_blc_gain_gb;
	__u16 se_ae_blc_gain_b;
	__u16 le_ae_blc_offset_r;
	__u16 le_ae_blc_offset_gr;
	__u16 le_ae_blc_offset_gb;
	__u16 le_ae_blc_offset_b;
	__u16 le_ae_blc_gain_r;
	__u16 le_ae_blc_gain_gr;
	__u16 le_ae_blc_gain_gb;
	__u16 le_ae_blc_gain_b;

	struct sop_isp_ae_tun_cfg ae_cfg;
	struct sop_isp_ae_1_tun_cfg ae_1_cfg;
	struct sop_isp_ae_2_tun_cfg ae_2_cfg;
};

struct sop_vip_isp_ge_tun_cfg {
	union reg_dpc_ge_config                 ge_config;
	union reg_dpc_ge_threshold0             ge_threshold0;
	union reg_dpc_ge_threshold1             ge_threshold1;
	union reg_dpc_ge_threshold1_0           ge_threshold1_0;
	union reg_dpc_ge_threshold1_1           ge_threshold1_1;
	union reg_dpc_ge_threshold2_0           ge_threshold2_0;
	union reg_dpc_ge_threshold2_1           ge_threshold2_1;
};

struct sop_vip_isp_ge_config {
	__u8  update;
	__u8  ge_enable;

	struct sop_vip_isp_ge_tun_cfg burst_cfg;
};

struct sop_vip_isp_af_config {
	__u8  update;
	__u8  enable;
	__u8  dpc_enable;
	__u8  hlc_enable;
	__u8  af_blc_enable;
	__u8  square_enable;
	__u8  outshift;
	__u8  num_gapline;
	__u16 offsetx;
	__u16 offsety;
	__u16 block_width;
	__u16 block_height;
	__u8  block_numx;
	__u8  block_numy;
	__u8  h_low_pass_value_shift;
	__u32 h_corning_offset_0;
	__u32 h_corning_offset_1;
	__u16 v_corning_offset;
	__u16 high_luma_threshold;
	__u8  h_low_pass_coef[5];
	__u8  h_high_pass_coef_0[5];
	__u8  h_high_pass_coef_1[5];
	__u8  v_high_pass_coef[3];
	__u8  th_low;
	__u8  th_high;
	__u8  gain_low;
	__u8  gain_high;
	__u8  slop_low;
	__u8  slop_high;
	__u16 af_blc_offset_r;
	__u16 af_blc_offset_gr;
	__u16 af_blc_offset_gb;
	__u16 af_blc_offset_b;
	__u16 af_blc_gain_r;
	__u16 af_blc_gain_gr;
	__u16 af_blc_gain_gb;
	__u16 af_blc_gain_b;
};

struct sop_vip_isp_3dlut_config {
	__u8  update;
	__u8  enable;
	__u8  h_clamp_wrap_opt;
	__u16 h_lut[3276];
	__u16 s_lut[3276];
	__u16 v_lut[3276];
};

struct sop_vip_isp_lscr_tun_cfg {
	union reg_isp_lscr_sc_wrap_6            lscr_sc_wrap_6;
	union reg_isp_lscr_sc_wrap_7            lscr_sc_wrap_7;
	union reg_isp_lscr_sc_wrap_8            lscr_sc_wrap_8;
	union reg_isp_lscr_sc_wrap_9            lscr_sc_wrap_9;
	union reg_isp_lscr_sc_wrap_10           lscr_sc_wrap_10;
	union reg_isp_lscr_sc_wrap_11           lscr_sc_wrap_11;
	union reg_isp_lscr_sc_wrap_12           lscr_sc_wrap_12;
	union reg_isp_lscr_sc_wrap_13           lscr_sc_wrap_13;
	union reg_isp_lscr_sc_wrap_14           lscr_sc_wrap_14;
	union reg_isp_lscr_sc_wrap_15           lscr_sc_wrap_15;
	union reg_isp_lscr_sc_wrap_16           lscr_sc_wrap_16;
	union reg_isp_lscr_sc_wrap_17           lscr_sc_wrap_17;
	union reg_isp_lscr_sc_wrap_18           lscr_sc_wrap_18;
	union reg_isp_lscr_sc_wrap_19           lscr_sc_wrap_19;
	union reg_isp_lscr_sc_wrap_20           lscr_sc_wrap_20;
	union reg_isp_lscr_sc_wrap_21           lscr_sc_wrap_21;
};

struct sop_vip_isp_lscr_config {
	__u8  update;
	__u8  lscr_enable;
	__u8  blc_enable;

	__u16 centerx;
	__u16 centery;

	__u16 norm;
	__u16 strength;

	__u16 lscr_nd_thr;
	__u16 lscr_nd_str;

	__u16 blc_offset_r;
	__u16 blc_offset_gr;
	__u16 blc_offset_gb;
	__u16 blc_offset_b;

	struct sop_vip_isp_lscr_tun_cfg burst_cfg;
};

struct sop_vip_isp_awb_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u8  bayer_start;
	__u16 awb_offsetx;
	__u16 awb_offsety;
	__u16 awb_sub_win_w;
	__u16 awb_sub_win_h;
	__u8  awb_numx;
	__u8  awb_numy;
	__u8  corner_avg_en;
	__u8  corner_size;
	__u8  awb_sts_div;
	__u16 r_lower_bound;
	__u16 r_upper_bound;
	__u16 g_lower_bound;
	__u16 g_upper_bound;
	__u16 b_lower_bound;
	__u16 b_upper_bound;
};

struct sop_vip_isp_hsv_config {
	__u8  update;
	__u8  enable;
	__u8  htune_enable;
	__u8  stune_enable;
	__u8  hsgain_enable;
	__u8  hvgain_enable;
	__u16 h_lut[769];
	__u16 s_lut[513];
	__u16 sgain_lut[769];
	__u16 vgain_lut[769];
};

struct sop_vip_isp_preproc_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__s16 r_ir_ratio[128];
	__s16 g_ir_ratio[128];
	__s16 b_ir_ratio[128];
	__u8  w_lut[128];
};

struct sop_vip_isp_fe_tun_cfg {
	struct sop_vip_isp_lsc_config	lsc_cfg[2]; // A2 change
	struct sop_vip_isp_ae_config	ae_cfg; // A2 change
};

struct sop_vip_isp_post_tun_cfg {
	struct sop_vip_isp_wbg_config		wbg_cfg; // A2 change
	struct sop_vip_isp_fusion_config	fusion_cfg; // New
	struct sop_vip_isp_map_curve_config	map_curve_cfg; // New
	struct sop_vip_isp_af_config		af_cfg; // A2 change
	struct sop_vip_isp_dpc_config		dpc_cfg; // New
	struct sop_vip_isp_ge_config		ge_cfg; // New
	struct sop_vip_isp_bnr_config		bnr_cfg; // New
	struct sop_vip_isp_lscr_config		lscr_cfg; // A2 change
	struct sop_vip_isp_drc_config		drc_cfg; // New
	struct sop_vip_isp_demosiac_config	demosiac_cfg; // A2

	struct sop_vip_isp_ee_ext_config	ee_ext_cfg; // New
	struct sop_vip_isp_pfr_config		pfr_cfg; // New
	struct sop_vip_isp_ccm_config		ccm_cfg; // A2 change
	struct sop_vip_isp_gamma_config		gamma_cfg; // A2 change
	struct sop_vip_isp_clut_config		clut_cfg; // A2
	struct sop_vip_isp_csc_config		csc_cfg; // A2

	struct sop_vip_isp_pre_ee_config	pre_ee_cfg; // New
	struct sop_vip_isp_ldci_config		ldci_cfg; // New
	struct sop_vip_isp_dci_config		dci_cfg; // New
	struct sop_vip_isp_tnr_config		tnr_cfg; // New
	struct sop_vip_isp_cnr_config		cnr_cfg; // New
	struct sop_vip_isp_ee_config		ee_cfg; // New
	struct sop_vip_isp_cacp_config		cacp_cfg; // A2
	struct sop_vip_isp_ca2_config		ca2_cfg; // A2
	struct sop_vip_isp_ycur_config		ycur_cfg; // A2
};

struct sop_vip_isp_fe_cfg {
	uint8_t tun_update[TUNING_NODE_NUM];
	uint8_t tun_idx;
	struct sop_vip_isp_fe_tun_cfg tun_cfg[TUNING_NODE_NUM];
};

struct sop_vip_isp_post_cfg {
	uint8_t tun_update[TUNING_NODE_NUM];
	uint8_t tun_idx;
	struct sop_vip_isp_post_tun_cfg tun_cfg[TUNING_NODE_NUM];
};

struct isp_tuning_cfg {
	uint64_t  fe_addr[VI_MAX_PIPE_NUM];
	void	  *fe_vir[VI_MAX_PIPE_NUM];
#ifdef __arm__
	__u32	  fe_padding[VI_MAX_PIPE_NUM];
#endif
	uint64_t  be_addr[VI_MAX_PIPE_NUM];
	void	  *be_vir[VI_MAX_PIPE_NUM];
#ifdef __arm__
	__u32	  be_padding[VI_MAX_PIPE_NUM];
#endif
	uint64_t  post_addr[VI_MAX_PIPE_NUM];
	void      *post_vir[VI_MAX_PIPE_NUM];
#ifdef __arm__
	__u32	  post_padding[VI_MAX_PIPE_NUM];
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* _U_VI_TUN_CFG_H_ */
