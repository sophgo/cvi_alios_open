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

int test_wdt_reset(void *args)
{
    csi_wdt_t hd;
    csi_error_t ret_sta;
    bool ret_status;

    test_wdt_args_t td;


    td.dev_idx = *((uint64_t *)args);
    td.timeout_ms = *((uint64_t *)args + 1);

    TEST_CASE_TIPS("test WDT idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config timeout is %d ms", td.timeout_ms);

    ret_sta = csi_wdt_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == CSI_OK, "wdt %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_wdt_set_timeout(&hd, td.timeout_ms);
    TEST_CASE_ASSERT_QUIT(ret_sta == CSI_OK,
                          "wdt %d set timeout fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);

    ret_status = csi_wdt_is_running(&hd);
    TEST_CASE_ASSERT_QUIT(ret_sta == false, "wdt %d running, expected return value is false, actual return value is %d.",
                          td.dev_idx, ret_status);

    ret_sta = csi_wdt_start(&hd);
    TEST_CASE_ASSERT_QUIT(ret_sta == CSI_OK, "wdt %d start fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_status = csi_wdt_is_running(&hd);
    TEST_CASE_ASSERT_QUIT((csi_wdt_is_running(&hd)),
                          "wdt %d Not running, expected return value is true, actual return value is %d.", td.dev_idx, ret_status);

    mdelay(td.timeout_ms * 2);
    TEST_CASE_ASSERT(1 == 0, "wdt %d timeout reset fail", td.dev_idx);

    csi_wdt_stop(&hd);
    csi_wdt_uninit(&hd);

    return 0;
}
