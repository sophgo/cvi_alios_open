/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#include <board.h>

#include <stdlib.h>
#include <stdio.h>
#include <devices/blockdev.h>
#include <devices/devicelist.h>

void board_flash_init(void)
{
#if CONFIG_PARTITION_SUPPORT_SPINORFLASH
    rvm_spiflash_drv_register(0);
#endif

#if CONFIG_PARTITION_SUPPORT_SD
    rvm_hal_sd_config_t sd_config;
    sd_config.sdif = CONFIG_SD_SDIF;
    rvm_sd_drv_register(0, &sd_config);
#endif

#if CONFIG_PARTITION_SUPPORT_SPINANDFLASH
	rvm_spinandflash_drv_register(0);
#endif
}
