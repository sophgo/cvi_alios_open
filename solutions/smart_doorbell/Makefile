CPRE := @
ifeq ($(V),1)
CPRE :=
VERB := --verbose
endif

MK_GENERATED_IMGS_PATH:=generated
ALGORITHMS_PATH:=../../components/chip_d1/d1_flash.elf
STRIP:=riscv64-unknown-elf-strip

HOST_TOOLS := $(CURDIR)/../../host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin
export PATH := $(HOST_TOOLS):$(PATH)
PROJECT := turnkey
CHIP_ARCH := cv180x
TDL_DIR := $(CURDIR)/../../components/cvi_tdl
.PHONY:startup
startup: all

all:
	@echo "build CHIP_ARCH by $(CHIP_ARCH)"
	@echo "Build Solution by $(BOARD) "
	$(CPRE) if [ -e $(CURDIR)/prebuild.sh ]; then bash $(CURDIR)/prebuild.sh $(shell echo $(CHIP_ARCH) | tr A-Z a-z); fi
	$(CPRE) if [ -e package.yaml ]; then rm -f package.yaml; fi
	$(CPRE) ln -sf customization/$(PROJECT)/package.$(PROJECT).yaml package.yaml
	$(CPRE) scons $(VERB) --board=$(BOARD) -j$(NPROC)
	$(CPRE) $(STRIP) --strip-unneeded $(CURDIR)/yoc_sdk/lib/*.a
	@cp yoc.bin $(MK_GENERATED_IMGS_PATH)/data/prim
ifeq ($(YOC_COMPRESS),y)
	@echo YoC Compress start
	$(CPRE) lz4 -f yoc.bin
	$(CPRE) if [ ! -d $(CURDIR)/lz4_out ]; then  mkdir lz4_out; fi
	$(CPRE) if [ -e $(CURDIR)/lz4_out/yoc.bin ]; then rm -rf $(CURDIR)/lz4_out/yoc.bin; fi
	$(CPRE) if [ -e $(CURDIR)/yoc.bin.lz4 ]; then echo ULZ4`stat -c "%s" $(CURDIR)/yoc.bin.lz4`  >> $(CURDIR)/lz4_out/yoc.bin; fi
	$(CPRE) if [ -e $(CURDIR)/lz4_out/yoc.bin ]; then cat  $(CURDIR)/yoc.bin.lz4 >> $(CURDIR)/lz4_out/yoc.bin; fi
	@echo YoC Compress done
endif
	@echo YoC SDK Done
	@echo [INFO] Create bin files

	@echo [INFO] Create imtb bin file
	$(CPRE) product image $(MK_GENERATED_IMGS_PATH)/images.zip -i $(MK_GENERATED_IMGS_PATH)/data -l -p
	$(CPRE) product image $(MK_GENERATED_IMGS_PATH)/images.zip -e $(MK_GENERATED_IMGS_PATH) -x
	$(CPRE) unzip -o generated/images.zip -d generated/images

.PHONY:flashall
flashall:
	$(CPRE) product flash ${MK_GENERATED_IMGS_PATH}/images.zip -a -x gdbinit -f ${ALGORITHMS_PATH}

.PHONY:flash
flash:
	$(CPRE) product flash ${MK_GENERATED_IMGS_PATH}/images.zip -w prim -x gdbinit -f ${ALGORITHMS_PATH}

sdk:
	$(CPRE) yoc sdk

.PHONY:clean
clean:
	$(CPRE) if [ -e package.yaml ]; then rm -f package.yaml; fi
	$(CPRE) if [ ! -e package.yaml ]; then ln -sf customization/$(PROJECT)/package.$(PROJECT).yaml package.yaml; fi
	$(CPRE) scons -c
ifeq ($(OS), Windows_NT)
	$(CPRE) if exist yoc_sdk rmdir /s /q yoc_sdk
	$(CPRE) if exist binary rmdir /s /q binary
	$(CPRE) if exist out rmdir /s /q out
	$(CPRE) if exist generated rmdir /s /q generated
	$(CPRE) if exist yoc.elf del /f /q yoc.elf
	$(CPRE) if exist yoc.map del /f /q yoc.map
else
	$(CPRE) rm -rf yoc_sdk binary out yoc.elf yoc.map generated	yoc.bin.lz4 lz4_out
endif
