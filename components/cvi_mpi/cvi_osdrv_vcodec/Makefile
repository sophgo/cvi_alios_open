# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/bitstreamfeeder.c
SRCS_C	+= $(SDIR)/bsfeeder_es_in_impl.c
SRCS_C	+= $(SDIR)/cfgParser.c
SRCS_C	+= $(SDIR)/cnm_video_helper.c
SRCS_C	+= $(SDIR)/coda9.c
SRCS_C	+= $(SDIR)/common.c
SRCS_C	+= $(SDIR)/cvi_dec_internal.c
SRCS_C	+= $(SDIR)/cvi_enc_internal.c
SRCS_C	+= $(SDIR)/cvi_enc_rc.c
SRCS_C	+= $(SDIR)/cvi_float_point.c
SRCS_C	+= $(SDIR)/cvi_jpg_dec_internal.c
SRCS_C	+= $(SDIR)/cvi_jpg_enc_internal.c
SRCS_C	+= $(SDIR)/cvi_jpg_interface.c
SRCS_C	+= $(SDIR)/cvi_rc_kernel.c
SRCS_C	+= $(SDIR)/cvi_soft_float.c
SRCS_C	+= $(SDIR)/cvi_vc_ctrl.c
SRCS_C	+= $(SDIR)/cvi_vcodec_lib.c
SRCS_C	+= $(SDIR)/cvi_vdec_api.c
SRCS_C	+= $(SDIR)/datastructure.c
SRCS_C	+= $(SDIR)/debug.c
SRCS_C	+= $(SDIR)/getopt_long.c
SRCS_C	+= $(SDIR)/jdi.c
SRCS_C	+= $(SDIR)/jpuapi.c
SRCS_C	+= $(SDIR)/jpuapifunc.c
SRCS_C	+= $(SDIR)/jpuhelper.c
SRCS_C	+= $(SDIR)/main_enc_cvitest.c
SRCS_C	+= $(SDIR)/main_helper.c
SRCS_C	+= $(SDIR)/mixer.c
SRCS_C	+= $(SDIR)/pbu.c
SRCS_C	+= $(SDIR)/platform.c
SRCS_C	+= $(SDIR)/product.c
SRCS_C	+= $(SDIR)/simplerenderer.c
SRCS_C	+= $(SDIR)/vcodec_debug.c
SRCS_C	+= $(SDIR)/vdi.c
SRCS_C	+= $(SDIR)/vdi_osal.c
SRCS_C	+= $(SDIR)/vpuapi.c
SRCS_C	+= $(SDIR)/vpuapifunc.c
SRCS_C	+= $(SDIR)/vpuhelper.c
SRCS_C	+= $(SDIR)/wave4.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_osdrv_vcodec.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
