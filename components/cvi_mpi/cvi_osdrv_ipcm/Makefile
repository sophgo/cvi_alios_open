# Usage:
# riscv64 newlib: make CHIP_ARCH=CV181X
# CROSS_COMPILE=${ALIOS_PATH}/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
##
SDIR	:= $(CURDIR)/src
INCS	+= -I$(CURDIR)/include
#
SRCS_C	:= $(SDIR)/cache.c
SRCS_C	+= $(SDIR)/cvi_ipcm.c
SRCS_C	+= $(SDIR)/cvi_spinlock.c
SRCS_C	+= $(SDIR)/ipcm_alios_port_common.c
SRCS_C	+= $(SDIR)/ipcm_anonymous.c
SRCS_C	+= $(SDIR)/ipcm_common.c
SRCS_C	+= $(SDIR)/ipcm.c
SRCS_C	+= $(SDIR)/ipcm_message.c
SRCS_C	+= $(SDIR)/ipcm_msg_proc.c
SRCS_C	+= $(SDIR)/ipcm_pool.c
SRCS_C	+= $(SDIR)/ipcm_port_common.c
SRCS_C	+= $(SDIR)/ipcm_system.c
SRCS_C	+= $(SDIR)/ipcm_test_anon.c
SRCS_C	+= $(SDIR)/ipcm_test_common_anon.c
SRCS_C	+= $(SDIR)/ipcm_test_common.c
SRCS_C	+= $(SDIR)/ipcm_test_msg.c
SRCS_C	+= $(SDIR)/ipcm_test_sys.c
SRCS_C	+= $(SDIR)/mailbox.c
SRCS_C	+= $(SDIR)/msg_queue.c
SRCS_C	+= $(SDIR)/ring.c
SRCS_C	+= $(SDIR)/test.c
##
LIB_DIR		?= lib
TARGET_DIR	:= $(CURDIR)/$(LIB_DIR)
TARGET_A	:= $(TARGET_DIR)/libcvi_osdrv_ipcm.a
#
PARAM_FILE:=../mpi_param.mk
include $(PARAM_FILE)
