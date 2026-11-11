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

#include <wdt_test.h>

int test_wdt_interface(void *args)
{
    csi_error_t ret_sta;
    uint32_t ret_num;
    bool ret_bool;
    csi_wdt_t wdt;

    TEST_CASE_READY();

    ret_sta = csi_wdt_init(NULL, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: wdt is NULL, csi_wdt_init expected return value is -1, actual return value is %d.", ret_sta);

    ret_sta = csi_wdt_init(&wdt, 0xFFFFFFFF);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: idx is 0xFFFFFFFF, csi_wdt_init expected return value is -1, actual return value is %d.", ret_sta);

    csi_wdt_uninit(NULL);

    ret_sta = csi_wdt_set_timeout(NULL, 10);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: wdt is NULL, csi_wdt_set_timeout expected return value is -1, actual return value is %d.", ret_sta);

    ret_sta = csi_wdt_init(&wdt, 0);
    ret_sta = csi_wdt_set_timeout(&wdt, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: ms is NULL(0), csi_wdt_set_timeout expected return value is -1, actual return value is %d.", ret_sta);
    csi_wdt_uninit(&wdt);

    ret_sta = csi_wdt_start(NULL);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: wdt is NULL, csi_wdt_start expected return value is -1, actual return value is %d.", ret_sta);

    csi_wdt_stop(NULL);

    ret_sta = csi_wdt_feed(NULL);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: wdt is NULL, csi_wdt_feed expected return value is -1, actual return value is %d.", ret_sta);

    ret_num = csi_wdt_get_remaining_time(NULL);
    TEST_CASE_ASSERT(ret_num == 0,
                     "parameter: wdt is NULL, csi_wdt_get_remaining_time expected return value is -1, actual return value is %d.", ret_num);

    ret_bool = csi_wdt_is_running(NULL);
    TEST_CASE_ASSERT(ret_bool == false,
                     "parameter: wdt is NULL, csi_wdt_is_running expected return value is 0, actual return value is %d.", ret_bool);

    ret_sta = csi_wdt_attach_callback(NULL, NULL, NULL);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: wdt is NULL, csi_wdt_attach_callback expected return value is -1, actual return value is %d.", ret_sta);

    csi_wdt_detach_callback(NULL);

    return 0;
}
