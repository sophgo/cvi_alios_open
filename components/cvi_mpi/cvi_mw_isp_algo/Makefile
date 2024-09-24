# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/dpcm_api.c
SRCS_C	+= $(SDIR)/isp_algo_blc.c
SRCS_C	+= $(SDIR)/isp_algo_bnr.c
SRCS_C	+= $(SDIR)/isp_algo_ca2.c
SRCS_C	+= $(SDIR)/isp_algo_cac.c
SRCS_C	+= $(SDIR)/isp_algo_ca.c
SRCS_C	+= $(SDIR)/isp_algo_ccm.c
SRCS_C	+= $(SDIR)/isp_algo_clut.c
SRCS_C	+= $(SDIR)/isp_algo_cnr.c
SRCS_C	+= $(SDIR)/isp_algo_crosstalk.c
SRCS_C	+= $(SDIR)/isp_algo_csc.c
SRCS_C	+= $(SDIR)/isp_algo_dci.c
SRCS_C	+= $(SDIR)/isp_algo_debug.c
SRCS_C	+= $(SDIR)/isp_algo_dehaze.c
SRCS_C	+= $(SDIR)/isp_algo_demosaic.c
SRCS_C	+= $(SDIR)/isp_algo_dis.c
SRCS_C	+= $(SDIR)/isp_algo_dpc.c
SRCS_C	+= $(SDIR)/isp_algo_drc.c
SRCS_C	+= $(SDIR)/isp_algo_fswdr.c
SRCS_C	+= $(SDIR)/isp_algo_gamma.c
SRCS_C	+= $(SDIR)/isp_algo_lcac.c
SRCS_C	+= $(SDIR)/isp_algo_ldci.c
SRCS_C	+= $(SDIR)/isp_algo_mlsc.c
SRCS_C	+= $(SDIR)/isp_algo_mono.c
SRCS_C	+= $(SDIR)/isp_algo_motion.c
SRCS_C	+= $(SDIR)/isp_algo_presharpen.c
SRCS_C	+= $(SDIR)/isp_algo_rgbcac.c
SRCS_C	+= $(SDIR)/isp_algo_sharpen.c
SRCS_C	+= $(SDIR)/isp_algo_tnr.c
SRCS_C	+= $(SDIR)/isp_algo_utility.c
SRCS_C	+= $(SDIR)/isp_algo_wb.c
SRCS_C	+= $(SDIR)/isp_algo_ycontrast.c
SRCS_C	+= $(SDIR)/isp_algo_ynr.c
SRCS_C	+= $(SDIR)/isp_iir_api.c
SRCS_C	+= $(SDIR)/pchip.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_mw_isp_algo.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
