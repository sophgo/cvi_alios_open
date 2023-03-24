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

#ifndef __PIN_TEST__
#define __PIN_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <drv/pin.h>

typedef enum {
    GPIO         = 0,
    UART,
    IIC,
    SPI,
    I2S,
    PWM,
    ADC
} test_pin_type_t;

typedef struct {
    uint32_t    idx;    //参考pin_name_t
    uint32_t    func;   //参考pin_func_t
    uint32_t    mode;   //参考csi_pin_mode_t
    uint32_t    speed;  //参考csi_pin_speed_t
    uint32_t    drive;  //参考csi_pin_drive_t
} test_pin_args_t;

extern int test_pin_interface(void *args);
extern int test_pin_config(void *args);
extern int test_pin_get_devidx(void *args);
extern int test_pin_get_channel(void *args);
extern int test_pin_get_pinname(void *args);

#endif
