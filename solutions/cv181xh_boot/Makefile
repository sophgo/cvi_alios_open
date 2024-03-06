CPRE := @
ifeq ($(V),1)
CPRE :=
VERB := --verbose
endif

BOARD_PATH:=../../boards/cv181xh_evb
BOOT_YAML_PATH:=../../components/cvi_mmf_sdk/chip_cv181x

.PHONY:startup
startup: all


all:
	@cp ${BOOT_YAML_PATH}/package.yaml ${BOOT_YAML_PATH}/package.yaml.bak
	@cp ${BOOT_YAML_PATH}/package.boot.yaml ${BOOT_YAML_PATH}/package.yaml
	$(CPRE) scons $(VERB) -j8
	@echo YoC SDK Done
	@mv ${BOOT_YAML_PATH}/package.yaml.bak ${BOOT_YAML_PATH}/package.yaml


.PHONY:install
install:
	@echo install boot
	$(CPRE) cp boot.bin ${BOARD_PATH}/bootimgs/boot
	$(CPRE) cp boot.elf ${BOARD_PATH}/bootimgs/boot.elf

.PHONY:clean
clean:
	$(CPRE) rm yoc_sdk boot.* out -rf
	$(CPRE) find . -name "*.[od]" -delete
	$(CPRE) rm -rf generated
