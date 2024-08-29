SRCTREE := $(CURDIR)
YOC_FILE := $(SRCTREE)/.yoc
NPROC ?= $(shell nproc)
$(info "NPROC = $(NPROC)")
export SRCTREE YOC_FILE NPROC

ifeq ("$(wildcard $(YOC_FILE))","")
$(error ".yoc is not present, please do 'yoc init' first!!")
endif
#
HOST_TOOLS := $(SRCTREE)/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin
CC := $(HOST_TOOLS)/riscv64-unknown-elf-gcc
CXX := $(HOST_TOOLS)/riscv64-unknown-elf-g++
AR := $(HOST_TOOLS)/riscv64-unknown-elf-ar
STRIP := $(HOST_TOOLS)/riscv64-unknown-elf-strip
export CC CXX AR STRIP
export PATH := $(HOST_TOOLS):$(PATH)
#
TOPSUBDIRS := smart_doorbell autotest smart_pad cv181x_boot peripherals_test usb_cam ipc barcode_scan sophpi
#
.PHONY:all clean install $(TOPSUBDIRS)

TMP = $(foreach f,$(TOPSUBDIRS),$(findstring $f,$(MAKECMDGOALS)))
TMP_SOLUTIONS = $(strip $(TMP))
ifneq ($(TMP_SOLUTIONS), )
	SOLUTIONS = $(TMP_SOLUTIONS)
else
	SOLUTIONS = smart_doorbell
endif
YOC_COMPRESS := n

#
all: $(TOPSUBDIRS)
#
$(TOPSUBDIRS):
	@echo "make $(MAKECMDGOALS) $(MAKEFLAGS)"
	$(MAKE) -C solutions/$@ $(patsubst $@,,$(MAKECMDGOALS))
#
clean:
	$(MAKE) -C solutions/$(SOLUTIONS) clean
	@rm install/ -rf
#
install:

