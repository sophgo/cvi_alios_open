CPRE := @
ifeq ($(V),1)
CPRE :=
VERB := --verbose
endif
HOST_TOOLS := $(CURDIR)/../../host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin
BOARD_PATH:=../../boards/cv181xc_qfn
BOARD_PATH_180X:=../../boards/cv180xb_qfn
BOOT_YAML_PATH:=../../components/chip_cv181x

.PHONY:startup
startup: all

all:
	@cp ${BOOT_YAML_PATH}/package.yaml ${BOOT_YAML_PATH}/package.yaml.bak
	@cp ${BOOT_YAML_PATH}/package.boot.yaml ${BOOT_YAML_PATH}/package.yaml
	$(CPRE) scons $(VERB) -j$(NPROC)
	@echo YoC SDK Done
	@mv ${BOOT_YAML_PATH}/package.yaml.bak ${BOOT_YAML_PATH}/package.yaml

.PHONY:install
install:
	@echo install boot cv181xc_qfn
	$(CPRE) cp boot.bin ${BOARD_PATH}/bootimgs/boot
	$(CPRE) cp boot.elf ${BOARD_PATH}/bootimgs/boot.elf
	@echo install boot cv180xb_qfn
	$(CPRE) cp boot.bin ${BOARD_PATH_180X}/bootimgs/boot
	$(CPRE) cp boot.elf ${BOARD_PATH_180X}/bootimgs/boot.elf

.PHONY:clean
clean:
	$(CPRE) rm yoc_sdk boot.* out -rf
	$(CPRE) find . -name "*.[od]" -delete
	$(CPRE) rm -rf generated
