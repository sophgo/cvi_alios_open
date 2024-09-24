# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/base_context.c
SRCS_C	+= $(SDIR)/base.c
SRCS_C	+= $(SDIR)/cmdq.c
SRCS_C	+= $(SDIR)/cvi_vb_proc.c
SRCS_C	+= $(SDIR)/hashmap.c
SRCS_C	+= $(SDIR)/vb.c
SRCS_C	+= $(SDIR)/vip_sys.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_osdrv_base.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
