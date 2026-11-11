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

static void wdt_callback(csi_wdt_t *wdt,  void *arg)
{
    csi_error_t ret_sta;
    ret_sta = csi_wdt_feed(arg);
    TEST_CASE_ASSERT(ret_sta == CSI_OK, "wdt feed fail");
}

int test_wdt_feed(void *args)
{
    csi_wdt_t hd;
    csi_error_t ret_sta;
    test_wdt_args_t td;

    td.dev_idx = *((uint64_t *)args);
    td.timeout_ms = *((uint64_t *)args + 1);

    TEST_CASE_TIPS("test WDT idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config timeout is %d ms", td.timeout_ms);

    ret_sta = csi_wdt_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "wdt %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_wdt_set_timeout(&hd, td.timeout_ms);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "wdt %d set timeout fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_wdt_attach_callback(&hd, wdt_callback, &hd);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0,
                          "wdt %d attach callback fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);

    ret_sta = csi_wdt_start(&hd);

    if (ret_sta != CSI_OK) {
        csi_wdt_detach_callback(&hd);
        csi_wdt_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "wdt %d start fail, expected return value is 0, actual return value is %d.", td.dev_idx,
                              ret_sta);
    }

    mdelay(csi_wdt_get_remaining_time(&hd));
    TEST_CASE_TIPS("test WDT Feed Success");

    mdelay(td.timeout_ms * 2);
    TEST_CASE_TIPS("test WDT Not Timeout Reset");

    csi_wdt_stop(&hd);
    csi_wdt_detach_callback(&hd);

    csi_wdt_uninit(&hd);

    return 0;
}

