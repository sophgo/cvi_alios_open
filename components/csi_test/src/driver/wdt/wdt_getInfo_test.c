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

int test_wdt_getInfo(void *args)
{
    csi_wdt_t hd;
    csi_error_t ret_sta;
    bool ret_bool;
    test_wdt_args_t td;

    td.dev_idx = *((uint64_t *)args);

    TEST_CASE_TIPS("test WDT idx is %d", td.dev_idx);


    ret_sta = csi_wdt_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "wdt %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_wdt_set_timeout(&hd, 2000);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "wdt %d set timeout fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_bool = csi_wdt_is_running(&hd);
    TEST_CASE_ASSERT(ret_bool == false,
                     "wdt %d get running status fail, expected return value is FALSE, actual return value is TRUE.", td.dev_idx);

    ret_sta = csi_wdt_start(&hd);

    if (ret_sta != 0) {
        csi_wdt_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "wdt %d start fail, expected return value is 0, actual return value is %d.", td.dev_idx,
                              ret_sta);
    }

    ret_bool = csi_wdt_is_running(&hd);
    TEST_CASE_ASSERT(ret_bool == true,
                     "wdt %d get running status fail, expected return value is TRUE, actual return value is FALSE.", td.dev_idx);

    csi_wdt_stop(&hd);

    ret_bool = csi_wdt_is_running(&hd);
    TEST_CASE_ASSERT(ret_bool == false,
                     "last wdt %d get running status fail, expected return value is FALSE, actual return value is TRUE.", td.dev_idx);

    csi_wdt_uninit(&hd);

    return 0;
}
