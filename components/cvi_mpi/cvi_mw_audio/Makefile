# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/abs.c
SRCS_C	+= $(SDIR)/agc.c
SRCS_C	+= $(SDIR)/agc_init.c
SRCS_C	+= $(SDIR)/agc_subfun.c
SRCS_C	+= $(SDIR)/bessel.c
SRCS_C	+= $(SDIR)/cvi_ai.c
SRCS_C	+= $(SDIR)/cvi_ao.c
SRCS_C	+= $(SDIR)/cviaudio_algo_interface.c
SRCS_C	+= $(SDIR)/cvi_ssp.c
SRCS_C	+= $(SDIR)/cvi_ssp_interface.c
SRCS_C	+= $(SDIR)/db2linear.c
SRCS_C	+= $(SDIR)/dc.c
SRCS_C	+= $(SDIR)/delay.c
SRCS_C	+= $(SDIR)/dg.c
SRCS_C	+= $(SDIR)/dr_detection.c
SRCS_C	+= $(SDIR)/fftcore.c
SRCS_C	+= $(SDIR)/log.c
SRCS_C	+= $(SDIR)/lpaec.c
SRCS_C	+= $(SDIR)/lpaec_subfun.c
SRCS_C	+= $(SDIR)/mmse.c
SRCS_C	+= $(SDIR)/mmse_init.c
SRCS_C	+= $(SDIR)/mmse_rtwutil.c
SRCS_C	+= $(SDIR)/nlpaes.c
SRCS_C	+= $(SDIR)/nlpaes_subfun.c
SRCS_C	+= $(SDIR)/notch.c
SRCS_C	+= $(SDIR)/packfft.c
SRCS_C	+= $(SDIR)/power.c
SRCS_C	+= $(SDIR)/slope_detection.c
SRCS_C	+= $(SDIR)/sum.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_mw_audio.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
