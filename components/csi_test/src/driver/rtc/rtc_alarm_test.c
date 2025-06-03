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

static void rtc_callback(csi_rtc_t *rtc, void *arg)
{
    *((uint8_t *)arg) = 1;
}

int test_rtc_alarm_func(csi_rtc_t *rtc_handler, test_rtc_args *test_args, int cancel_flag)
{
    csi_error_t rtc_ret;
    csi_rtc_time_t alarm_time;
    uint32_t remain_time = 0;
    volatile uint8_t rtc_flag = 0;

    calc_expect_rtc(test_args->tm, &alarm_time, 5);
    TEST_CASE_TIPS("set RTC  time: %d-%d-%d %d:%d:%d", test_args->tm->tm_year + 1900, test_args->tm->tm_mon + 1,
                   test_args->tm->tm_mday, test_args->tm->tm_hour, test_args->tm->tm_min, test_args->tm->tm_sec);
    TEST_CASE_TIPS("alarm    time: %d-%d-%d %d:%d:%d", alarm_time.tm_year + 1900, alarm_time.tm_mon + 1, alarm_time.tm_mday,
                   alarm_time.tm_hour, alarm_time.tm_min, alarm_time.tm_sec);

    rtc_ret = csi_rtc_init(rtc_handler, test_args->idx);
    TEST_CASE_ASSERT_QUIT(rtc_ret == CSI_OK, "rtc init failed, idx:%d", test_args->idx);

    rtc_ret = csi_rtc_set_time(rtc_handler, test_args->tm);
    TEST_CASE_ASSERT_QUIT(rtc_ret == CSI_OK, "rtc set time failed(wait), idx:%d", test_args->idx);

    rtc_flag = 0;
    rtc_ret = csi_rtc_set_alarm(rtc_handler, &alarm_time, rtc_callback, (void *)&rtc_flag);

    tst_mdelay(1000);
    remain_time = csi_rtc_get_alarm_remaining_time(rtc_handler);
    TEST_CASE_ASSERT(remain_time > 0 && remain_time < 5, "get wrong remaining time: %d", remain_time);

    if (cancel_flag == 1) {
        rtc_ret =  csi_rtc_cancel_alarm(rtc_handler);
        TEST_CASE_TIPS("RTC Alarm canceled...");
        TEST_CASE_ASSERT(rtc_ret == CSI_OK, "csi_rtc_cancel_alarm failed. returned:%d ", rtc_ret);
    }

    tst_mdelay(2000);
    TEST_CASE_ASSERT(rtc_flag == 0, "Alarm occured before expect time ");

    tst_mdelay(4000);

    if (cancel_flag == 0) {
        TEST_CASE_ASSERT(rtc_flag == 1, "Alarm not occured as expected ");
    } else {
        TEST_CASE_ASSERT(rtc_flag == 0, "Alarm still occured after canceled alarm! ");
    }

    csi_rtc_uninit(rtc_handler);
    return 0;
}

int test_rtc_alarm_set(void *args)
{
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

    //test for no alarm cancelling
    test_rtc_alarm_func(&test_rtc_handler, &rtc_args, 0);

    //test for alarm cancelling
    test_rtc_alarm_func(&test_rtc_handler, &rtc_args, 1);
    return 0;
}