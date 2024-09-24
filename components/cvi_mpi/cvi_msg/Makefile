# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/cvi_msg_audio.c
SRCS_C	+= $(SDIR)/cvi_msg_gdc.c
SRCS_C	+= $(SDIR)/cvi_msg_i80.c
SRCS_C	+= $(SDIR)/cvi_msg_isp.c
SRCS_C	+= $(SDIR)/cvi_msg_mipi_tx.c
SRCS_C	+= $(SDIR)/cvi_msg_rgn.c
SRCS_C	+= $(SDIR)/cvi_msg_sensor.c
SRCS_C	+= $(SDIR)/cvi_msg_server.c
SRCS_C	+= $(SDIR)/cvi_msg_sys.c
SRCS_C	+= $(SDIR)/cvi_msg_vb.c
SRCS_C	+= $(SDIR)/cvi_msg_vdec.c
SRCS_C	+= $(SDIR)/cvi_msg_venc.c
SRCS_C	+= $(SDIR)/cvi_msg_vi.c
SRCS_C	+= $(SDIR)/cvi_msg_vo.c
SRCS_C	+= $(SDIR)/cvi_msg_vpss.c
SRCS_C	+= $(SDIR)/cvi_workq.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_msg.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)