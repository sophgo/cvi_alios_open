/*
 * Copyright (C) 2020 C-SKY Microsystems Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions an
 * limitations under the License.
 */

#ifndef __SPIFLASH_TEST_H_
#define __SPIFLASH_TEST_H_

#include <stdint.h>
#include <drv/spiflash.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include "drv/tick.h"
#include "stdio.h"
#include <csi_config.h>
#include "soc.h"
#include "board_config.h"
#include "board_init.h"
#include "test_common.h"

typedef struct {
    uint32_t idx;
    uint32_t offset;
    uint32_t size;
} test_spiflash_args_t;

extern int test_spiflash_interface(void *args);
extern int test_spiflash_program_read(void *args);
extern int test_spiflash_qspi_programread(void *argss);
extern int test_spiflash_lock(void *args);
extern int test_spiflash_performance_read(void *args);
int test_spiflash_qspi_wr(void *args);
#endif