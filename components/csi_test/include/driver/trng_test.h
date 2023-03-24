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

#ifndef __TRNG_TEST__
#define __TRNG_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <drv/rng.h>

#define TRNG_DATA_LEN	1024

typedef struct {
    uint32_t    idx;
} test_trng_args_t;


extern int test_trng_handle(void *args);
extern int test_trng_multi(void *args);
extern int test_trng_single(void *args);
extern int test_trng_multi_performance(void *args);
extern int test_trng_single_performance(void *args);
extern int test_trng_interface(void *args);
extern int test_trng_multi_stability(void *args);
extern int test_trng_single_stability(void *args);
#endif
