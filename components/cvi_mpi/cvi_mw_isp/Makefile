# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/cvi_isp.c
SRCS_C	+= $(SDIR)/cvi_mipi.c
SRCS_C	+= $(SDIR)/isp_3a.c
SRCS_C	+= $(SDIR)/isp_blc_ctrl.c
SRCS_C	+= $(SDIR)/isp_bnr_ctrl.c
SRCS_C	+= $(SDIR)/isp_ca2_ctrl.c
SRCS_C	+= $(SDIR)/isp_cac_ctrl.c
SRCS_C	+= $(SDIR)/isp_ca_ctrl.c
SRCS_C	+= $(SDIR)/isp_ccm_ctrl.c
SRCS_C	+= $(SDIR)/isp_clut_ctrl.c
SRCS_C	+= $(SDIR)/isp_cnr_ctrl.c
SRCS_C	+= $(SDIR)/isp_control.c
SRCS_C	+= $(SDIR)/isp_crosstalk_ctrl.c
SRCS_C	+= $(SDIR)/isp_csc_ctrl.c
SRCS_C	+= $(SDIR)/isp_dci_ctrl.c
SRCS_C	+= $(SDIR)/isp_debug.c
SRCS_C	+= $(SDIR)/isp_dehaze_ctrl.c
SRCS_C	+= $(SDIR)/isp_demosaic_ctrl.c
SRCS_C	+= $(SDIR)/isp_dis_ctrl.c
SRCS_C	+= $(SDIR)/isp_dpc_ctrl.c
SRCS_C	+= $(SDIR)/isp_drc_ctrl.c
SRCS_C	+= $(SDIR)/isp_feature_ctrl.c
SRCS_C	+= $(SDIR)/isp_flow_ctrl.c
SRCS_C	+= $(SDIR)/isp_freeze.c
SRCS_C	+= $(SDIR)/isp_fswdr_ctrl.c
SRCS_C	+= $(SDIR)/isp_gamma_ctrl.c
SRCS_C	+= $(SDIR)/isp_interpolate.c
SRCS_C	+= $(SDIR)/isp_lcac_ctrl.c
SRCS_C	+= $(SDIR)/isp_ldci_ctrl.c
SRCS_C	+= $(SDIR)/isp_log_ctrl.c
SRCS_C	+= $(SDIR)/isp_lut.c
SRCS_C	+= $(SDIR)/isp_mailbox.c
SRCS_C	+= $(SDIR)/isp_mgr_buf.c
SRCS_C	+= $(SDIR)/isp_mgr.c
SRCS_C	+= $(SDIR)/isp_mlsc_ctrl.c
SRCS_C	+= $(SDIR)/isp_mono_ctrl.c
SRCS_C	+= $(SDIR)/isp_motion_ctrl.c
SRCS_C	+= $(SDIR)/isp_mw_compat.c
SRCS_C	+= $(SDIR)/isp_param_default.c
SRCS_C	+= $(SDIR)/isp_peri_ctrl.c
SRCS_C	+= $(SDIR)/isp_presharpen_ctrl.c
SRCS_C	+= $(SDIR)/isp_proc.c
SRCS_C	+= $(SDIR)/isp_rgbcac_ctrl.c
SRCS_C	+= $(SDIR)/isp_sensor.c
SRCS_C	+= $(SDIR)/isp_sharpen_ctrl.c
SRCS_C	+= $(SDIR)/isp_sts_ctrl.c
SRCS_C	+= $(SDIR)/isp_tnr_ctrl.c
SRCS_C	+= $(SDIR)/isp_tun_buf_ctrl.c
SRCS_C	+= $(SDIR)/isp_wb_ctrl.c
SRCS_C	+= $(SDIR)/isp_ycontrast_ctrl.c
SRCS_C	+= $(SDIR)/isp_ynr_ctrl.c
SRCS_C	+= $(SDIR)/rtos_isp_mgr.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_mw_isp.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
