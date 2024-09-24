# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/vi_be_ip_ctrl.c
SRCS_C	+= $(SDIR)/vi_core.c
SRCS_C	+= $(SDIR)/vi_dbg_proc.c
SRCS_C	+= $(SDIR)/vi_drv.c
SRCS_C	+= $(SDIR)/vi_ext.c
SRCS_C	+= $(SDIR)/vi.c
SRCS_C	+= $(SDIR)/vi_fe_ip_ctrl.c
SRCS_C	+= $(SDIR)/vi_ip_test_case.c
SRCS_C	+= $(SDIR)/vi_isp_buf_ctrl.c
SRCS_C	+= $(SDIR)/vi_isp_proc.c
SRCS_C	+= $(SDIR)/vi_perf_chk.c
SRCS_C	+= $(SDIR)/vi_proc.c
SRCS_C	+= $(SDIR)/vi_raw_dump.c
SRCS_C	+= $(SDIR)/vi_raw_ip_ctrl.c
SRCS_C	+= $(SDIR)/vi_rgb_ip_ctrl.c
SRCS_C	+= $(SDIR)/vi_sdk_layer.c
SRCS_C	+= $(SDIR)/vi_snsr_i2c.c
SRCS_C	+= $(SDIR)/vi_subsys_ctrl.c
SRCS_C	+= $(SDIR)/vi_tun_ip_ctrl.c
SRCS_C	+= $(SDIR)/vi_yuv_ip_ctrl.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_osdrv_vi.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
