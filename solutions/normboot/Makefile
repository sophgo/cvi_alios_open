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
	@echo "Build Solution by $(ALIOS_BOARD) "
	$(CPRE) if [ -e $(CURDIR)/prebuild.sh ]; then bash $(CURDIR)/prebuild.sh; fi
	$(CPRE) if [ -e package.yaml ]; then rm -f package.yaml; fi
ifeq ($(RELEASE_FLAG),1)
	$(CPRE) cp -f package_yamls/package.yaml.$(PROJECT).release package.yaml
else
	$(CPRE) cp -f package_yamls/package.yaml.$(PROJECT) package.yaml
endif
	@/usr/bin/env python ../../scripts/defconfig/defconfig.py
	@/usr/bin/env python ../../scripts/yaml_update.py
ifeq ($(RELEASE_FLAG),1)
	@make -C ../../components/cvi_mpi CHIP_ARCH=$(CHIP_ARCH)
endif
	$(CPRE) scons $(VERB) --board=$(ALIOS_BOARD) -j4
	@cp yoc.bin $(MK_GENERATED_IMGS_PATH)/data/prim
	@echo YoC SDK Done
	@echo [INFO] Create bin files

	@echo [INFO] Create imtb bin file
	$(CPRE) product image $(MK_GENERATED_IMGS_PATH)/images.zip -i $(MK_GENERATED_IMGS_PATH)/data -l -p
	$(CPRE) product image $(MK_GENERATED_IMGS_PATH)/images.zip -e $(MK_GENERATED_IMGS_PATH) -x

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
ifeq ($(RELEASE_FLAG),1)
	$(CPRE) cp -f package_yamls/package.yaml.$(PROJECT).release package.yaml
else
	$(CPRE) cp -f package_yamls/package.yaml.$(PROJECT) package.yaml
endif
	@/usr/bin/env python ../../scripts/defconfig/defconfig.py
	@/usr/bin/env python ../../scripts/yaml_update.py
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

install:
	$(CPRE) yoc sdk
