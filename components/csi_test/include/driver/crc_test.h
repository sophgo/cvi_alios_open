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

#ifndef __AES_TEST__
#define __AES_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <drv/crc.h>


#define CRC_DATA_LEN 512
typedef struct {
    uint8_t     crc_init_value1;
    uint16_t    crc_init_value2;
    uint32_t    crc_init_value3;
    uint8_t     crc_rand_data;
    uint32_t    size;
} test_crc_args_t;


extern int test_crc_main(char *args);
extern int test_crc7_be_func(char *args);
extern int test_crc8_func(char *args);
extern int test_crc8_maxim_func(char *args);
extern int test_crc16_func(char *args);
extern int test_crc16_ccitt_func(char *args);
extern int test_crc16_dnp_func(char *args);
extern int test_crc16_itu_func(char *args);
extern int test_crc32_le_func(char *args);
extern int test_crc32_be_func(char *args);
extern int test_crc7_be_performance(char *args);
extern int test_crc8_performance(char *args);
extern int test_crc8_maxim_performance(char *args);
extern int test_crc16_performance(char *args);
extern int test_crc16_ccitt_performance(char *args);
extern int test_crc16_dnp_performance(char *args);
extern int test_crc16_itu_performance(char *args);
extern int test_crc32_le_performance(char *args);
extern int test_crc32_be_performance(char *args);
extern int test_crc7_be_stability(char *args);
extern int test_crc8_stability(char *args);
extern int test_crc8_maxim_stability(char *args);
extern int test_crc16_stability(char *args);
extern int test_crc16_ccitt_stability(char *args);
extern int test_crc16_dnp_stability(char *args);
extern int test_crc16_itu_stability(char *args);
extern int test_crc32_le_stability(char *args);
extern int test_crc32_be_stability(char *args);

#endif
