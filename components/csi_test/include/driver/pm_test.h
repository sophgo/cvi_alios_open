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

#ifndef __PM_TEST__
#define __PM_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <drv/pm.h>
#include <drv/timer.h>
#include <drv/rtc.h>
#include <drv/adc.h>

typedef struct {
    uint32_t    idx;
    uint32_t    mode;   //csi_pm_mode_t
    uint32_t
    priority;//执行设备回调函数的优先级（值越小，进入低功耗模式前越晚调用/退出低功耗模式后越先执行）
    uint32_t    action;//csi_pm_dev_action_t
    uint32_t    ad_idx;
    uint8_t     ad_ch;
} test_pm_args_t;


extern int test_pm_handle(char *args);
extern int test_deepsleep1(char *args);
extern int test_deepsleep2(char *args);

#endif
