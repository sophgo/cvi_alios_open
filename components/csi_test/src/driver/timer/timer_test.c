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

#include <timer_test.h>

const test_info_t timer_test_funcs_map[] = {
    {"TIMER_INTERFACE", test_timer_interface, 0},
    {"TIMER_PRECISION", test_timer_precision, 2},
    {"TIMER_STATE", test_timer_state, 2},
    {"TIMER_SETTING", test_timer_setting, 2},
    {"TIMER_GETLOADVALUE", test_timer_getLoadvalue, 2},
    {"TIMER_GETREMAININGVALUE", test_timer_getRemainingvalue, 2},
    {"TIMER_CALLBACK", test_timer_callback, 2}
};

int test_timer_main(char *args)
{
    int ret;
    ret = testcase_jump(args, (void *)timer_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("timer don't supported this command");
    }

    return ret;
}
