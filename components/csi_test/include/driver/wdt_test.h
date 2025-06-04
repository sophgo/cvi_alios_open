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

#ifndef __WDT_TEST__
#define __WDT_TEST__

#include <stdint.h>
#include <drv/wdt.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>

typedef struct {
    uint8_t		dev_idx;
    uint32_t	timeout_ms;
} test_wdt_args_t;



extern int test_wdt_interface(void *args);
extern int test_wdt_feed(void *args);
extern int test_wdt_reset(void *args);
extern int test_wdt_stress(void *args);



extern int test_wdt_startStop(void *args);
extern int test_wdt_getInfo(void *args);
#endif
