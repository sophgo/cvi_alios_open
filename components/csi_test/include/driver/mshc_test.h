/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#ifndef _MSHC_TEST_H
#define _MSHC_TEST_H
#include <stdint.h>
#include <drv/sdhc.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include "test_common.h"

#define BLOCK_SIZE (512)

typedef struct {
    uint32_t mshc_idx;
    uint32_t speed_mode;
    uint32_t vdd;
    uint32_t bus_width;
    uint32_t clk_freq;
    uint32_t rca;
    uint32_t boot_partition;
    uint32_t test_choose;
    uint32_t repeat_times;
    uint32_t block_addr;
    uint32_t block_cnt;

} test_mmc_args_t;


int test_mmc_interface(void *args);
int test_mmc_repeat_init(void *args);
int test_mmc_get_sd_info(void *args);
int test_mmc_get_emmc_info(void *args);
int test_mmc_write_read_emmc(void *args);
int test_mmc_write_read_sd(void *args);
int test_mmc_emmc_speed(void *args);
int test_mmc_sd_speed(void *args);
int test_mmc_emmc_cross_check(void *args);
int test_mmc_sd_cross_check(void *args);

int test_mmc_read(dev_mmc_t *handler, test_mmc_args_t *ts, uint8_t read_value);
int test_mmc_write(dev_mmc_t *handler, test_mmc_args_t *ts, uint8_t write_value);
#endif
