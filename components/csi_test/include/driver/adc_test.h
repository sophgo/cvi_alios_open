/*
* Copyright (C) 2017 C-SKY Microsystems Co., Ltd. All rights reserved.
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

#ifndef _ADC_TEST_H
#define _ADC_TEST_H
#include <stdint.h>
#include <drv/adc.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include "test_common.h"

#define MAX_CHS 16


typedef struct {
    int32_t idx;         //device id
    uint16_t s_time;     //samplling time
    bool continue_mode;  //true to enable the continue mode, or false to disable
    uint32_t freq_div;   //ADC frequence division
    uint8_t mode;        //0: sync, 1:async, 2: dma
    uint8_t *ch;         //channels enabled
    int num_chs;         // num of channels
    uint32_t max;        // expected max value
    uint32_t min;        // expected min value
    uint16_t *stime_array;
} test_adc_args_t;


extern uint8_t ch_ids[MAX_CHS];
extern test_adc_args_t test_args;


extern int test_adc_interface(void *args);
extern int test_adc_status(void *args);
extern int test_adc_async(void *args);
extern int test_adc_sync(void *args);
extern int test_adc_dma(void *args);
extern int test_adc_stress(void);

extern int adc_pins_write(char *args);
#endif