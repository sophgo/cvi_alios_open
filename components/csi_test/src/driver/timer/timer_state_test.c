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

static uint32_t state_timeout_flag = 0;


static void timer_event_cb_reload_fun(csi_timer_t *timer_handle, void *arg)
{
    if (!state_timeout_flag) {
        state_timeout_flag = 1;
    }
}


int test_timer_state(void *args)
{
    csi_error_t ret;
    csi_timer_t timer_handle;
    test_timer_args_t r_timer_args;
    uint32_t delay_time;
    bool state;

    r_timer_args.timer_idx = *((uint64_t *)args);
    r_timer_args.timeout = *((uint64_t *)args + 1);
    TEST_CASE_TIPS("test timer idx is %d, timeout %d us", r_timer_args.timer_idx, r_timer_args.timeout);

    ret = csi_timer_init(&timer_handle, r_timer_args.timer_idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init fail");

    state = csi_timer_is_running(&timer_handle);
    TEST_CASE_ASSERT(state == false, "timer state error should be stop");

    ret = csi_timer_attach_callback(&timer_handle, timer_event_cb_reload_fun, NULL);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error, csi_timer_attach_callback precision testing fail");

    ret = csi_timer_start(&timer_handle, r_timer_args.timeout);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer start fail");
    state = csi_timer_is_running(&timer_handle);
    TEST_CASE_ASSERT(state == true, "timer state error should be run");

    delay_time = r_timer_args.timeout / 1000 / 2;
    tst_mdelay(delay_time);
    TEST_CASE_ASSERT(state_timeout_flag == 0, "timer callback func should not be call");

    csi_timer_stop(&timer_handle);
    state = csi_timer_is_running(&timer_handle);
    TEST_CASE_ASSERT(state == false, "timer state error should be stop");

    ret = csi_timer_start(&timer_handle, r_timer_args.timeout);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer start fail");
    state = csi_timer_is_running(&timer_handle);
    TEST_CASE_ASSERT(state == true, "timer state error should be run");

    delay_time = r_timer_args.timeout / 1000 + 1;
    tst_mdelay(delay_time);
    TEST_CASE_ASSERT(state_timeout_flag == 1, "timer callback func should be call");

    csi_timer_stop(&timer_handle);
    state = csi_timer_is_running(&timer_handle);
    TEST_CASE_ASSERT(state == false, "timer state error should be stop");

    csi_timer_detach_callback(&timer_handle);
    csi_timer_uninit(&timer_handle);

    state_timeout_flag = 0;

    return 0;
}