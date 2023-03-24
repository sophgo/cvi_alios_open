/*
* Copyright (C) 2017 C-SKY Microsystems Co., Ltd. All rights reserved.
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

#include <rtc_test.h>


const test_info_t rtc_test_funcs_map[] = {
    {"RTC_INTERFACE", test_rtc_interface, 0},
    {"RTC_TIME", test_rtc_time, 7},
    {"RTC_ALARM", test_rtc_alarm_set, 7},

};


int test_rtc_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)rtc_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("RTC don't supported this command");
    }

    return ret;
}