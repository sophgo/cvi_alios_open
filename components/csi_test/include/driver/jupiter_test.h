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

#ifndef __JUPITER_TEST__
#define __JUPITER_TEST__

#include <stdint.h>
#include <drv/pin.h>
#include <drv/spiflash.h>
#include <drv/uart.h>
#include <drv/gpio.h>
#include <drv/rtc.h>
#include <drv/pm.h>
#include <drv/wdt.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>

#ifdef CONFIG_CHIP_JUPITER_LC_MPW_2
#include <wj_pmu_jupiter_lc_mpw_2.h>
#else
#include <wj_pmu_jupiter_bc_mpw_2.h>
#endif

typedef struct {
    uint8_t     gpio_idx;
    uint8_t		pin;
    uint8_t		dir;
    uint8_t	    gpio_mode;
    uint8_t	    pin_value;
} test_jupiter_gpio_t;

typedef struct {
    uint8_t     wakeup_pin;
    uint8_t     wakeup_pin_func;
    uint8_t     cpu_testing;
    bool        main_pwr_setting;
    bool        rf_pwr_setting;
    uint8_t     dcdc_or_ldo;
    uint32_t    value;
} test_jupiter_pmu_args_t;

typedef struct {
    uint8_t     spiflash_idx;
    uint32_t	size;
    uint32_t    offset;
    uint32_t    line_mode;
    uint32_t    frequence;
} test_jupiter_qspi_t;

extern int test_jupiter_pmu_main(char *args);
extern int test_jupiter_core_main(char *args);
extern int pmu_io_dt_check(char *args);
extern int pmu_io_latch_gpio_input_read_check(char *args);
extern int pmu_io_latch_gpio_input_write_check(char *args);
extern int pmu_io_latch_gpio_output_check(char *args);
extern int pmu_io_latch_qspi_read_program_check(char *args);
int pmu_mexstatus_control(char *args);
int pmu_reset_mode_test_bksram_data(char *args);
extern int pmu_retention_switch_test(char *args);
int set_sram_delay(char *args);
int benchmark_test(char *args);
#endif
