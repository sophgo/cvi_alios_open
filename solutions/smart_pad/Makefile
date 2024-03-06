CPRE := @
ifeq ($(V),1)
CPRE :=
VERB := --verbose
endif

MK_GENERATED_IMGS_PATH:=generated
ALGORITHMS_PATH:=../../components/chip_d1/d1_flash.elf

HOST_TOOLS := $(CURDIR)/../../host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin
export PATH := $(HOST_TOOLS):$(PATH)
PROJECT := turnkey
.PHONY:startup
startup: all

all:
	@echo "Build Solution by $(BOARD) "
	$(CPRE) if [ -e $(CURDIR)/prebuild.sh ]; then bash $(CURDIR)/prebuild.sh $(shell echo $(CHIP_ARCH) | tr A-Z a-z); fi
	$(CPRE) if [ -e package.yaml ]; then rm -f package.yaml; fi
	$(CPRE) ln -sf customization/$(PROJECT)/package.$(PROJECT).yaml package.yaml
	$(CPRE) scons $(VERB) --board=$(BOARD) -j4
	@cp yoc.bin $(MK_GENERATED_IMGS_PATH)/data/prim
	@cp yoc.bin $(MK_GENERATED_IMGS_PATH)/data/images
	@echo YoC SDK Done
	@echo [INFO] Create bin files
	$(CPRE) product lfs ${MK_GENERATED_IMGS_PATH}/data/lfs -c data -b 4096 -s 0x6D0000
	$(CPRE) cp -arf ${MK_GENERATED_IMGS_PATH}/data/lfs  $(MK_GENERATED_IMGS_PATH)/data/littlefs.bin

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
	$(CPRE) rm -rf yoc_sdk binary out yoc.elf yoc.map generated
endif
