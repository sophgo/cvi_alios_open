# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/cvi_bin.c
SRCS_C	+= $(SDIR)/md5.c
SRCS_C	+= $(SDIR)/rw_json.c
SRCS_C	+= $(SDIR)/vo_bin.c
SRCS_C	+= $(SDIR)/vo_json_struct.c
SRCS_C	+= $(SDIR)/vpss_bin.c
SRCS_C	+= $(SDIR)/vpss_json_struct.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_mw_bin.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
