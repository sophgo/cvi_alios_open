# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/awbalgo.c
SRCS_C	+= $(SDIR)/awb_buf.c
SRCS_C	+= $(SDIR)/awb_debug.c
SRCS_C	+= $(SDIR)/cvi_awb.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_mw_isp_awb.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
