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

#ifndef __PMU_TEST__
#define __PMU_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <drv/pm.h>
#include <drv/rtc.h>

typedef struct {
    uint32_t    wakeup_source;
    uint32_t    pmu_mode;   //csi_pm_mode_t
    uint32_t    wakeup_pin;
    uint32_t    wakeup_pin_func;
    uint32_t    wakeup_rtc_idx;
    uint32_t    timeout;
} test_pmu_args_t;

extern int test_pmu_main(char *args);
extern int test_pmu_func_ioctl(char *args);
extern int test_pmu_func_rtc(char *args);

#endif
