/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef _SPINAND_TEST_H
#define _SPINAND_TEST_H
#include <stdint.h>
#include <drv/spinand.h>
#include <drv/gpio.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include "test_common.h"

typedef struct {
    uint32_t qspi_idx;
    uint32_t gpio_idx;
    uint32_t pin;
    uint32_t frequence;
    uint32_t repeat_times;
    uint64_t addr;
    uint32_t offset;
    uint64_t size;
    uint32_t test_mode; //1: 只测读; 2: 只测写; other: 读写一起测试
} test_spinand_args_t;

int spinand_gpio_as_cs_init(uint32_t port_idx, uint32_t pin);
void spinand_gpio_as_cs_uninit(void);
void spinand_cs_operate(csi_gpio_pin_state_t val);
int test_nandflash_write(csi_spinand_t *spinand, uint64_t addr, uint64_t length, uint8_t write_value);
int test_nandflash_read(csi_spinand_t *spinand, uint64_t addr, uint64_t length, uint8_t read_value);


int test_spinand_interface(void *args);
int test_spinand_device_reset(void *args);
int test_spinand_device_info(void *args);
int test_spinand_repeat_init(void *args);
int test_spinand_bad_block_check(void *args);
int test_spinand_bad_block_mark(void *args);
int test_spinand_device_erase(void *args);
int test_spinand_device_write_read(void *args);
int test_spinand_device_spare_write_read(void *args);
int test_spinand_device_cross_check(void *args);
int test_spinand_device_speed(void *args);
#endif
