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

const test_info_t wdt_test_funcs_map[] = {
    {"WDT_INTERFACE", test_wdt_interface, 0},
    {"WDT_FEED", test_wdt_feed, 2},
    {"WDT_RESET", test_wdt_reset, 2},
    {"WDT_STRESS", test_wdt_stress, 3},
    {"WDT_START_STOP", test_wdt_startStop, 1},
    {"WDT_GET_INFO", test_wdt_getInfo, 1},
    {NULL, NULL, 0}
};

int test_wdt_main(char *args)
{
    int ret;
    ret = testcase_jump(args, (void *)wdt_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("WDT don't supported this command");
    }

    return ret;
}
