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

#include "rtc_test.h"

typedef int (*set_time_func)(csi_rtc_t *rtc, const csi_rtc_time_t *rtctime);

static int test_rtc_time_func(csi_rtc_t *rtc_handler, test_rtc_args *test_args, set_time_func func)
{
    bool rtc_runing;
    csi_error_t rtc_ret;
    csi_rtc_time_t cur_time;
    csi_rtc_time_t expect_time;

    TEST_CASE_TIPS("set RTC  time: %d-%d-%d %d:%d:%d", test_args->tm->tm_year + 1900, test_args->tm->tm_mon + 1,
                   test_args->tm->tm_mday, test_args->tm->tm_hour, test_args->tm->tm_min, test_args->tm->tm_sec);

    calc_expect_rtc(test_args->tm, &expect_time, 2);
    TEST_CASE_TIPS("expect_time: %d-%d-%d %d:%d:%d", expect_time.tm_year + 1900, expect_time.tm_mon + 1,
                   expect_time.tm_mday, expect_time.tm_hour, expect_time.tm_min, expect_time.tm_sec);

    rtc_ret = func(rtc_handler, test_args->tm);

    if (test_args->tm->tm_year < 70 || test_args->tm->tm_year > 199 || \
        test_args->tm->tm_mon < 0 || test_args->tm->tm_mon > 11     || \
        test_args->tm->tm_mday < 1 || test_args->tm->tm_mday > 31   || \
        test_args->tm->tm_hour < 0 || test_args->tm->tm_hour > 23   || \
        test_args->tm->tm_min < 0 || test_args->tm->tm_min > 59   || \
        test_args->tm->tm_sec < 0 || test_args->tm->tm_sec > 59
       ) {
        TEST_CASE_ASSERT_QUIT(1 == 0, "passed an invalid RTC time");
    } else {
        TEST_CASE_ASSERT_QUIT(rtc_ret == CSI_OK, "rtc set time failed(wait), idx:%d", test_args->idx);
        rtc_runing = csi_rtc_is_running(rtc_handler);
        TEST_CASE_ASSERT(rtc_runing == true, "RTC should runing, returned wrong RTC state");
        tst_mdelay(2000);
        rtc_ret = csi_rtc_get_time(rtc_handler, &cur_time);
        rtc_time_assert(&cur_time, &expect_time);
    }

    return 0;
}

int test_rtc_time(void *args)
{
    csi_error_t rtc_ret;
    test_rtc_args rtc_args;
    csi_rtc_t test_rtc_handler;
    csi_rtc_time_t tm_test = {.tm_year = 120,
                              .tm_mon = 11,
                              .tm_mday = 31,
                              .tm_hour = 23,
                              .tm_min = 59,
                              .tm_sec = 59
                             };
    rtc_args.tm = &tm_test;

    rtc_args.idx 	= *((uint64_t *)args);
    tm_test.tm_year = *((uint64_t *)args + 1);
    tm_test.tm_mon  = *((uint64_t *)args + 2);
    tm_test.tm_mday = *((uint64_t *)args + 3);
    tm_test.tm_hour = *((uint64_t *)args + 4);
    tm_test.tm_min  = *((uint64_t *)args + 5);
    tm_test.tm_sec  = *((uint64_t *)args + 6);


    TEST_CASE_TIPS("test RTC idx is %d", rtc_args.idx);
    TEST_CASE_TIPS("test RTC year is %d", tm_test.tm_year);
    TEST_CASE_TIPS("test RTC mon is %d", tm_test.tm_mon);
    TEST_CASE_TIPS("test RTC day is %d", tm_test.tm_mday);
    TEST_CASE_TIPS("test RTC hour is %d", tm_test.tm_hour);
    TEST_CASE_TIPS("test RTC min is %d", tm_test.tm_min);
    TEST_CASE_TIPS("test RTC sec is %d", tm_test.tm_sec);


    TEST_CASE_READY();

    rtc_ret = csi_rtc_init(&test_rtc_handler, rtc_args.idx);
    TEST_CASE_ASSERT_QUIT(rtc_ret == CSI_OK, "rtc init failed, idx:%d", rtc_args.idx);

    TEST_CASE_TIPS("Starting for set time w/ wait...");
    test_rtc_time_func(&test_rtc_handler, &rtc_args, csi_rtc_set_time);

    TEST_CASE_TIPS("Starting for set time w/o wait...");
    test_rtc_time_func(&test_rtc_handler, &rtc_args, csi_rtc_set_time_no_wait);

    csi_rtc_uninit(&test_rtc_handler);
    TEST_CASE_TIPS("RTC time TEST END...");
    return 0;
}

