# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/mipi_tx.c
SRCS_C	+= $(SDIR)/vo_core.c
SRCS_C	+= $(SDIR)/vo.c
SRCS_C	+= $(SDIR)/vo_proc.c
SRCS_C	+= $(SDIR)/vo_rgn_ctrl.c
SRCS_C	+= $(SDIR)/vo_sdk_layer.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_osdrv_vo.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
