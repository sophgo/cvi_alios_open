SHELL = /bin/bash

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=$(MW_PATH)/Makefile.param
	include $(PARAM_FILE)
endif
include sensor.mk
export KBUILD_DEFINES

define MAKE_SENSOR
    pushd $(1) && \
    $(MAKE) all _USER_CFLAGS="$(KBUILD_DEFINES)" && \
    popd
endef

.PHONY : prepare clean $(sensor-y)
all: prepare $(sensor-y) all_sensor

prepare:
	@echo "#################################################"
	@echo "#"
	@echo "# Compiling 'component libs' Configs as below..."
	@echo "# SENSOR_LIST=$(sensor-y)"
	@echo "#"
	@echo "#################################################"

sensor_i2c:
	$(call MAKE_SENSOR, ${@})

sensor_cfg:
	$(call MAKE_SENSOR, ${@})

brigates_bg0808:
	$(call MAKE_SENSOR, ${@})

cvsens_cv2003:
	$(call MAKE_SENSOR, ${@})

gcore_gc02m1:
	$(call MAKE_SENSOR, ${@})

gcore_gc1054:
	$(call MAKE_SENSOR, ${@})

gcore_gc2053:
	$(call MAKE_SENSOR, ${@})

gcore_gc2053_slave:
	$(call MAKE_SENSOR, ${@})

gcore_gc2053_1L:
	$(call MAKE_SENSOR, ${@})

gcore_gc2093:
	$(call MAKE_SENSOR, ${@})

gcore_gc2145:
	$(call MAKE_SENSOR, ${@})

gcore_gc4023:
	$(call MAKE_SENSOR, ${@})

gcore_gc4653:
	$(call MAKE_SENSOR, ${@})

gcore_gc8613:
	$(call MAKE_SENSOR, ${@})

nextchip_n5:
	$(call MAKE_SENSOR, ${@})

nextchip_n6:
	$(call MAKE_SENSOR, ${@})

nuc_nc021:
	$(call MAKE_SENSOR, ${@})

onsemi_ar2020:
	$(call MAKE_SENSOR, ${@})

ov_os04a10:
	$(call MAKE_SENSOR, ${@})

ov_os04c10:
	$(call MAKE_SENSOR, ${@})

ov_os05a20:
	$(call MAKE_SENSOR, ${@})

ov_os08a20:
	$(call MAKE_SENSOR, ${@})

ov_os08b10:
	$(call MAKE_SENSOR, ${@})

ov_os04e10:
	$(call MAKE_SENSOR, ${@})

ov_ov2736:
	$(call MAKE_SENSOR, ${@})

ov_ov4689:
	$(call MAKE_SENSOR, ${@})

ov_ov6211:
	$(call MAKE_SENSOR, ${@})

ov_ov7251:
	$(call MAKE_SENSOR, ${@})

ov_ov9282:
	$(call MAKE_SENSOR, ${@})

pixelplus_pr2020:
	$(call MAKE_SENSOR, ${@})

pixelplus_pr2100:
	$(call MAKE_SENSOR, ${@})

sms_sc020hgs:
	$(call MAKE_SENSOR, ${@})

sms_sc035gs:
	$(call MAKE_SENSOR, ${@})

sms_sc035gs_1L:
	$(call MAKE_SENSOR, ${@})

sms_sc035hgs:
	$(call MAKE_SENSOR, ${@})

sms_sc200ai:
	$(call MAKE_SENSOR, ${@})

sms_sc301iot:
	$(call MAKE_SENSOR, ${@})

sms_sc233hgs:
	$(call MAKE_SENSOR, ${@})

sms_sc401ai:
	$(call MAKE_SENSOR, ${@})

sms_sc438ai:
	$(call MAKE_SENSOR, ${@})

sms_sc500ai:
	$(call MAKE_SENSOR, ${@})

sms_sc501ai_2L:
	$(call MAKE_SENSOR, ${@})

sms_sc3332:
	$(call MAKE_SENSOR, ${@})

sms_sc531ai_2L:
	$(call MAKE_SENSOR, ${@})

sms_sc1330:
	$(call MAKE_SENSOR, ${@})

sms_sc3335:
	$(call MAKE_SENSOR, ${@})

sms_sc3336:
	$(call MAKE_SENSOR, ${@})

sms_sc2335:
	$(call MAKE_SENSOR, ${@})

sms_sc4336:
	$(call MAKE_SENSOR, ${@})

sms_sc4336p:
	$(call MAKE_SENSOR, ${@})

sms_sc4336p_slave:
	$(call MAKE_SENSOR, ${@})

soi_f23:
	$(call MAKE_SENSOR, ${@})

soi_f35:
	$(call MAKE_SENSOR, ${@})

soi_f37p:
	$(call MAKE_SENSOR, ${@})

soi_q03:
	$(call MAKE_SENSOR, ${@})

soi_k06:
	$(call MAKE_SENSOR, ${@})

sony_imx307:
	$(call MAKE_SENSOR, ${@})

sony_imx307_slave:
	$(call MAKE_SENSOR, ${@})

sony_imx307_2L:
	$(call MAKE_SENSOR, ${@})

sony_imx327:
	$(call MAKE_SENSOR, ${@})

sony_imx327_slave:
	$(call MAKE_SENSOR, ${@})

sony_imx327_2L:
	$(call MAKE_SENSOR, ${@})

sony_imx327_fpga:
	$(call MAKE_SENSOR, ${@})

sony_imx327_sublvds:
	$(call MAKE_SENSOR, ${@})

sony_imx335:
	$(call MAKE_SENSOR, ${@})

sony_imx412:
	$(call MAKE_SENSOR, ${@})

sony_imx415:
	$(call MAKE_SENSOR, ${@})

sony_imx585:
	$(call MAKE_SENSOR, ${@})

sony_imx675:
	$(call MAKE_SENSOR, ${@})

sony_imx900:
	$(call MAKE_SENSOR, ${@})

techpoint_tp2860:
	$(call MAKE_SENSOR, ${@})

lontium_lt6911:
	$(call MAKE_SENSOR, ${@})

all_sensor:
	@$(MAKE) -f Makefile_full || exit 1;

clean:
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	    do cd `dirname $$x`; if [ $$? ]; then $(MAKE) clean; cd -; fi; done
	@echo "#"
	@$(MAKE) clean -f Makefile_full || exit 1;
