SRCTREE := $(CURDIR)
YOC_FILE := $(SRCTREE)/.yoc
YAML_FILE := $(SRCTREE)/components/chip_cv181x
#NPROC ?= $(shell expr $(shell nproc) - 1)
NPROC := 7
$(info "NPROC = $(NPROC)")
export SRCTREE OBJYOC_FILE NPROC

C906L_DMA_ENABLE := 1

ifeq ($(C906L_DMA_ENABLE), 1)
$(shell sed -i "s,#- src/drivers/dma/cvi/src/cvi_dma.c,- src/drivers/dma/cvi/src/cvi_dma.c,"  $(YAML_FILE)/package.yaml)
$(shell sed -i "s,#- src/drivers/dma/cvi/src/csi_dma.c,- src/drivers/dma/cvi/src/csi_dma.c," $(YAML_FILE)/package.yaml)
$(shell sed -i "s,#- src/drivers/dma/cvi/src/sysdma_remap.c,- src/drivers/dma/cvi/src/sysdma_remap.c," $(YAML_FILE)/package.yaml)
$(shell sed -i "s,#- src/hal/dma/cvi/src/hal_dma.c,- src/hal/dma/cvi/src/hal_dma.c," $(YAML_FILE)/package.yaml)
else
CFLAGS += -DCONFIG_KERNEL_NONE
$(shell sed -i "s,- src/drivers/dma/cvi/src/cvi_dma.c,#- src/drivers/dma/cvi/src/cvi_dma.c,"  $(YAML_FILE)/package.yaml)
$(shell sed -i "s,- src/drivers/dma/cvi/src/csi_dma.c,#- src/drivers/dma/cvi/src/csi_dma.c," $(YAML_FILE)/package.yaml)
$(shell sed -i "s,- src/drivers/dma/cvi/src/sysdma_remap.c,#- src/drivers/dma/cvi/src/sysdma_remap.c," $(YAML_FILE)/package.yaml)
$(shell sed -i "s,- src/hal/dma/cvi/src/hal_dma.c,#- src/hal/dma/cvi/src/hal_dma.c," $(YAML_FILE)/package.yaml)
endif
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
TOPSUBDIRS := $(ALIOS_SOLUTION)
#
.PHONY:all clean install $(TOPSUBDIRS)

#
all: $(TOPSUBDIRS)
#
$(TOPSUBDIRS):
	$(MAKE) -C solutions/$@ $(patsubst $@,,$(MAKECMDGOALS))
#
clean: 
	$(MAKE) -C solutions/$(TOPSUBDIRS) clean
#
install:


