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

int test_rtc_interface(void *args)
{
    uint32_t rt = 0;
    csi_error_t rtc_ret;
    csi_rtc_time_t rtctime;
    csi_rtc_t test_rtc_handler;

    rtc_ret = csi_rtc_init(NULL, 0);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_init], pass rtc as NULL , expect:%d, returned:%d", CSI_ERROR, rtc_ret);

    rtc_ret = csi_rtc_init(&test_rtc_handler, 0);
    TEST_CASE_ASSERT(rtc_ret == CSI_OK, "[csi_rtc_init], pass rtc as NULL , expect:%d, returned:%d", CSI_OK, rtc_ret);

    rtc_ret = csi_rtc_set_time(NULL, &rtctime);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_set_time], pass rtc as NULL , expect:%d, returned:%d", CSI_ERROR,
                     rtc_ret);

    rtc_ret = csi_rtc_set_time(&test_rtc_handler, NULL);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_set_time], pass rtc as NULL , expect:%d, returned:%d", CSI_ERROR,
                     rtc_ret);

    rtc_ret = csi_rtc_set_time_no_wait(NULL, &rtctime);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_set_time_no_wait], pass rtc as NULL , expect:%d, returned:%d",
                     CSI_ERROR, rtc_ret);

    rtc_ret = csi_rtc_set_time_no_wait(&test_rtc_handler, NULL);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_set_time_no_wait], pass rtc as NULL , expect:%d, returned:%d",
                     CSI_ERROR, rtc_ret);

    rtc_ret = csi_rtc_get_time(NULL, &rtctime);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_get_time], pass rtc as NULL , expect:%d, returned:%d", CSI_ERROR,
                     rtc_ret);

    rtc_ret = csi_rtc_get_time(&test_rtc_handler, NULL);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_get_time], pass rtc as NULL , expect:%d, returned:%d", CSI_ERROR,
                     rtc_ret);

    rt = csi_rtc_get_alarm_remaining_time(NULL);
    TEST_CASE_ASSERT(rt == 0, "[csi_rtc_get_alarm_remaining_time], pass rtc as NULL , expect:%d, returned:%d", 0, rt);

    rtc_ret = csi_rtc_set_alarm(NULL, &rtctime, NULL, NULL);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_set_alarm], pass rtc as NULL , expect:%d, returned:%d", CSI_ERROR,
                     rtc_ret);

    rtc_ret = csi_rtc_set_alarm(&test_rtc_handler, NULL, NULL, NULL);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_set_alarm], pass rtc as NULL , expect:%d, returned:%d", CSI_ERROR,
                     rtc_ret);

    rtc_ret = csi_rtc_cancel_alarm(NULL);
    TEST_CASE_ASSERT(rtc_ret == CSI_ERROR, "[csi_rtc_cancel_alarm], pass rtc as NULL , expect:%d, returned:%d", CSI_ERROR,
                     rtc_ret);

    TEST_CASE_ASSERT(csi_rtc_is_running(NULL) == false, "[csi_rtc_is_running], pass rtc as NULL , expect:%d, returned:%d",
                     false, true);

    csi_rtc_uninit(NULL);

    csi_rtc_uninit(&test_rtc_handler);
    TEST_CASE_TIPS("RTC interface test end.");
    return 0;
}