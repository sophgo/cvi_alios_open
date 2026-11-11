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

static uint32_t getvalue_timeout_flag = 0;

static void timer_event_cb_reload_fun(csi_timer_t *timer_handle, void *arg)
{
    if (!getvalue_timeout_flag) {
        getvalue_timeout_flag = 1;
    }
}

int test_timer_getLoadvalue(void *args)
{
    csi_error_t ret;
    csi_timer_t timer_handle;
    test_timer_args_t r_timer_args;
    int32_t freq;
    uint32_t load_time;

    r_timer_args.timer_idx = *((uint64_t *)args);
    r_timer_args.timeout = *((uint64_t *)args + 1);
    TEST_CASE_TIPS("test timer idx is %d, timeout %d us", r_timer_args.timer_idx, r_timer_args.timeout);


    ret = csi_timer_init(&timer_handle, r_timer_args.timer_idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_start(&timer_handle, r_timer_args.timeout);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer start error");

    freq = soc_get_timer_freq(r_timer_args.timer_idx);
    load_time = csi_timer_get_load_value(&timer_handle);

    uint64_t tmp_time = (uint64_t)r_timer_args.timeout * (freq / 1000);
    TEST_CASE_ASSERT((uint64_t)load_time * 1000U == tmp_time, "timer get load time not equal set timeout");

    csi_timer_stop(&timer_handle);
    csi_timer_uninit(&timer_handle);

    return 0;
}

int test_timer_getRemainingvalue(void *args)
{
    csi_error_t ret;
    csi_timer_t timer_handle;
    test_timer_args_t r_timer_args;
    int32_t freq;
    uint32_t remaining_value_last;
    uint32_t remaining_value_cur;

    r_timer_args.timer_idx = *((uint64_t *)args);
    r_timer_args.timeout = *((uint64_t *)args + 1);
    TEST_CASE_TIPS("test timer idx is %d, timeout %d us", r_timer_args.timer_idx, r_timer_args.timeout);


    ret = csi_timer_init(&timer_handle, r_timer_args.timer_idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_attach_callback(&timer_handle, timer_event_cb_reload_fun, NULL);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error");

    ret = csi_timer_start(&timer_handle, r_timer_args.timeout);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer start error");

    freq = soc_get_timer_freq(r_timer_args.timer_idx);
    remaining_value_last = csi_timer_get_remaining_value(&timer_handle);

    tst_mdelay(1);

    while (!getvalue_timeout_flag) {
        remaining_value_cur = csi_timer_get_remaining_value(&timer_handle);

        TEST_CASE_ASSERT((remaining_value_cur < remaining_value_last)
                         && ((uint64_t)remaining_value_cur * 1000000U < (uint64_t)r_timer_args.timeout * freq),
                         "timer get remaining_value ,remaining_value is error");
        tst_mdelay(1);
    }

    csi_timer_stop(&timer_handle);
    csi_timer_detach_callback(&timer_handle);
    csi_timer_uninit(&timer_handle);

    getvalue_timeout_flag = 0;

    return 0;
}