/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef _EFLASH_TEST_H
#define _EFLASH_TEST_H
#include <stdint.h>
#include <drv/eflash.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include "test_common.h"

typedef struct {
    uint32_t eflash_idx;
    uint32_t offset;
    uint32_t size;
    uint32_t repeat_times;
    uint32_t flash_size;
    uint32_t sector_size;
    uint32_t erased_value;
} test_eflash_args_t;


int test_eflash_interface(void *args);
int test_eflash_getInfo(void *args);
int test_eflash_repeatInit(void *args);
int test_eflash_writeRead(void *args);
int test_eflash_eraseRead(void *args);
int test_eflash_writeEraseRead(void *args);
int test_eflash_eraseChip(void *args);

#endif