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

void timer_callback_func(csi_timer_t *timer, void *arg)
{
    ;
}

int test_timer_interface(void *args)
{
    csi_error_t ret;
    uint32_t load_time = 0;
    uint32_t remaining_time = 0;
    bool is_running = false;
    csi_timer_t timer_hd;

    // test csi_timer_init
    ret = csi_timer_init(NULL, 0);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "parameter: timer is NULL, csi_timer_init expects -1 returned, actual %d", ret);

    ret = csi_timer_init(&timer_hd, 0xFFFFFFFF);
    TEST_CASE_ASSERT(ret == CSI_ERROR,
                     "parameter: idx(0xFFFFFFFF) is out of range, csi_timer_init expects -1 returned, actual %d", ret);

    // init timer for other parameters test
    csi_timer_init(&timer_hd, 1);

    // test csi_timer_attach_callback
    ret  = csi_timer_attach_callback(NULL, timer_callback_func, NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "parameter: timer is NULL, csi_timer_attach_callback expects -1 returned, actual %d",
                     ret);

    ret  = csi_timer_attach_callback(&timer_hd, NULL, NULL);
    TEST_CASE_ASSERT(ret == CSI_OK, "parameter: callback is NULL, csi_timer_attach_callback expects 0 returned, actual %d",
                     ret);

    // test csi_error_t csi_timer_start(csi_timer_t *timer, uint32_t timeout_us);
    ret = csi_timer_start(NULL, 1000);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "parameter: timer is NULL, csi_timer_start expects -1 returned, actual %d", ret);

    ret = csi_timer_start(&timer_hd, 0);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "parameter: timeout_us is NULL(0), csi_timer_start expects -1 returned, actual %d",
                     ret);

    // test uint32_t csi_timer_get_load_value(csi_timer_t *timer);
    load_time = csi_timer_get_load_value(NULL);
    TEST_CASE_ASSERT(load_time == 0, "parameter: timer is NULL, csi_timer_get_load_value expects 0 returned, actual %d",
                     load_time);

    // test uint32_t csi_timer_get_remaining_value(csi_timer_t *timer);
    remaining_time = csi_timer_get_remaining_value(NULL);
    TEST_CASE_ASSERT(remaining_time == 0,
                     "parameter: timer is NULL, csi_timer_get_remaining_value expects 0 returned, actual %d", remaining_time);

    // test bool csi_timer_is_running(csi_timer_t *timer);
    is_running = csi_timer_is_running(NULL);
    TEST_CASE_ASSERT(is_running == false,
                     "parameter: timer is NULL, csi_timer_is_running expects false returned, actual %d", is_running);

    // test void csi_timer_stop(csi_timer_t *timer);
    csi_timer_stop(NULL);

    // test void csi_timer_detach_callback(csi_timer_t *timer);
    csi_timer_detach_callback(NULL);

    // test void csi_timer_uninit(csi_timer_t *timer);
    csi_timer_uninit(NULL);

    csi_timer_uninit(&timer_hd);
    return 0;
}
