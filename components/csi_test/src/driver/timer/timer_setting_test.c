/*
 * Copyright (C) 2017 C-SKY Microsystems Co., All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "timer_test.h"

#define DELAY_TIME 3*1000 //defalut 3s



int test_timer_setting(void *args)
{
    csi_error_t ret;
    csi_timer_t timer_handle;
    test_timer_args_t r_timer_args;

    r_timer_args.timer_idx = *((uint64_t *)args);
    r_timer_args.timeout = *((uint64_t *)args + 1);
    TEST_CASE_TIPS("test timer idx is %d, timeout %d us", r_timer_args.timer_idx, r_timer_args.timeout);


    ret = csi_timer_init(&timer_handle, r_timer_args.timer_idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_start(&timer_handle, r_timer_args.timeout);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer start error");

    tst_mdelay(DELAY_TIME);

    csi_timer_stop(&timer_handle);
    csi_timer_uninit(&timer_handle);

    return 0;
}