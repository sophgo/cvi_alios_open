#!/usr/bin/env python

sensor_def = "CONFIG_SENSOR_"

chip_def = "CONFIG_CHIP_"

defconfig = [
    "CONFIG_ARCH",
    "CONFIG_CHIP",
    "CONFIG_RTOS_INIT_MEDIA",
    "CONFIG_VCODEC_FIRMWARE_H",
    "CONFIG_RTOS_PARSE_PARAM",
    "CONFIG_RTOS_ANNON_MSG",
    "CONFIG_DW_UART",
    "CONFIG_VIRTUAL_UART",
    "CONFIG_C906L_DMA_ENABLE",
    "CONFIG_AUD_DRV_SEL",
    "CONFIG_H264_FW",
    "CONFIG_H265_FW",
    "CONFIG_CV180ZB_SC2003_SC2003_1l_SWITCH",
    "CONFIG_CV180ZB_SC2331_SC2331_1l_SWITCH",
    "CONFIG_CV180ZB_GC2053",
    "CONFIG_CV1811C_CV2003_1l_TRIPLE",
]


def config_check_and_append(string) -> dict:
    config = {}
    if "CONFIG_CHIP_ARCH_cv180x" in string.split("=")[0]:
        config["ARCH_PHOBOS"] = "1"
        config["__CV180X__"] = "1"

    if "CONFIG_CHIP_ARCH_cv181x" in string.split("=")[0]:
        config["ARCH_MARS"] = "1"
        config["__CV181X__"] = "1"

    if "CONFIG_CHIP_ARCH_cv184x" in string.split("=")[0]:
        config["ARCH_MARS3"] = "1"
        config["__CV184X__"] = "1"

    return config
