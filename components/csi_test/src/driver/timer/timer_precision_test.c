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


static uint32_t timeout_flag = 0;
static uint64_t start_time = 0;
static uint64_t callback_time = 0;


static void timer_event_cb_reload_fun(csi_timer_t *timer_handle, void *arg)
{
    if (!timeout_flag) {
        timeout_flag = 1;
        callback_time = csi_tick_get_us();
    }
}


int test_timer_precision(void *args)
{
    csi_error_t ret;
    csi_timer_t timer_handle;
    test_timer_args_t r_timer_args;
    uint64_t real_time;
    uint32_t diff_time;

    r_timer_args.timer_idx = *((uint64_t *)args);
    r_timer_args.timeout = *((uint64_t *)args + 1);
    TEST_CASE_TIPS("test timer idx is %d, timeout %d us", r_timer_args.timer_idx, r_timer_args.timeout);


    ret = csi_timer_init(&timer_handle, r_timer_args.timer_idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error, timer precision testing fail");

    ret = csi_timer_attach_callback(&timer_handle, timer_event_cb_reload_fun, NULL);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error, csi_timer_attach_callback precision testing fail");

    ret = csi_timer_start(&timer_handle, r_timer_args.timeout);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer start error");

    start_time = csi_tick_get_us();

    while (!timeout_flag) {
        tst_mdelay(1);
    }

    real_time = callback_time - start_time;
    TEST_CASE_TIPS("real_time %lld us", real_time);

    if (real_time > r_timer_args.timeout) {
        diff_time = real_time -  r_timer_args.timeout;
    } else {
        diff_time = r_timer_args.timeout - real_time;
    }

    TEST_CASE_TIPS("diff_time %d us", diff_time);
    TEST_CASE_ASSERT(diff_time < r_timer_args.timeout * 0.1, "timer precision error greater than 10%");

    csi_timer_stop(&timer_handle);
    csi_timer_detach_callback(&timer_handle);
    csi_timer_uninit(&timer_handle);

    timeout_flag = 0;

    return 0;
}