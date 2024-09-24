# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/cvi_vip_vpss_proc.c
SRCS_C	+= $(SDIR)/dsi_phy.c
SRCS_C	+= $(SDIR)/scaler.c
SRCS_C	+= $(SDIR)/vpss_core.c
SRCS_C	+= $(SDIR)/vpss.c
SRCS_C	+= $(SDIR)/vpss_img.c
SRCS_C	+= $(SDIR)/vpss_os_ioctl.c
SRCS_C	+= $(SDIR)/vpss_rgn_ctrl.c
SRCS_C	+= $(SDIR)/vpss_sc.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_osdrv_vpss.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
