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

#include <pwm_test.h>

static volatile csi_pwm_event_t get_event;
static volatile uint8_t get_ch;
static volatile uint32_t get_time;

static void pwm_callback(csi_pwm_t *pwm, csi_pwm_event_t event, uint8_t ch, uint32_t time_us, void *arg)
{
    get_event = event;
    get_ch = ch;
    get_time = time_us;
    (*(uint8_t *)arg) = 0;
}


int test_pwm_capture(void *args)
{
    csi_error_t ret_sta;
    test_pwm_args_t td;
    csi_pwm_t hd;


    td.dev_idx          = *((uint64_t *)args);
    td.channel          = *((uint64_t *)args + 1);
    td.capture_polarity = *((uint64_t *)args + 2);
    td.capture_count    = *((uint64_t *)args + 3);
    td.delay_ms         = *((uint64_t *)args + 4);

    TEST_CASE_TIPS("test PWM idx is %d", td.dev_idx);
    TEST_CASE_TIPS("test pwm channel is %d", td.channel);
    TEST_CASE_TIPS("config capture polarity is %d", td.capture_polarity);
    TEST_CASE_TIPS("config capture count is %d", td.capture_count);
    TEST_CASE_TIPS("config signal received in %d ms", td.delay_ms);


    ret_sta = csi_pwm_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);



    ret_sta = csi_pwm_capture_config(&hd, td.channel, td.capture_polarity, td.capture_count);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture config fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    volatile uint8_t event_lock;

    event_lock = 1;
    get_ch = 0;
    get_time = 0;
    ret_sta = csi_pwm_attach_callback(&hd, pwm_callback, (void *)&event_lock);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d attach callback fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    TEST_CASE_READY();
    ret_sta = csi_pwm_capture_start(&hd, td.channel);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture start fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    while (event_lock) ;


    csi_pwm_capture_stop(&hd, td.channel);


    csi_pwm_detach_callback(&hd);

    TEST_CASE_ASSERT(get_event == td.capture_polarity, "callback get event fail, expected event is %d ,actual is %d",
                     td.capture_polarity, get_event);

    TEST_CASE_ASSERT(get_ch == td.channel, "callback get channel fail, expected channel is %d ,actual is %d", td.channel,
                     get_ch);

    if ((get_time > (td.delay_ms + td.capture_count * 10) * 1000) || (get_time == 0)) {
        TEST_CASE_ASSERT(1 == 0, "callback get capture time error, expected time is %dms, actual is %dms", td.delay_ms,
                         get_time / 1000);
    }

    csi_pwm_uninit(&hd);
    return 0;
}



int test_pwm_capture_abnormal_config(void *args)
{
    csi_error_t ret_sta;
    test_pwm_args_t td;
    csi_pwm_t hd;


    td.dev_idx          = *((uint64_t *)args);
    td.channel          = *((uint64_t *)args + 1);
    td.capture_polarity = *((uint64_t *)args + 2);
    td.capture_count    = *((uint64_t *)args + 3);
    td.delay_ms         = *((uint64_t *)args + 4);

    TEST_CASE_TIPS("test PWM idx is %d", td.dev_idx);
    TEST_CASE_TIPS("test pwm channel is %d", td.channel);
    TEST_CASE_TIPS("config capture polarity is %d", td.capture_polarity);
    TEST_CASE_TIPS("config capture count is %d", td.capture_count);
    TEST_CASE_TIPS("config signal received in %d ms", td.delay_ms);

    ret_sta = csi_pwm_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_pwm_capture_config(&hd, td.channel, td.capture_polarity, td.capture_count);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture config fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    volatile uint8_t event_lock;

    event_lock = 1;
    get_ch = 0;
    get_time = 0;
    ret_sta = csi_pwm_attach_callback(&hd, pwm_callback, (void *)&event_lock);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d attach callback fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    TEST_CASE_READY();
    ret_sta = csi_pwm_capture_start(&hd, td.channel);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture start fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    mdelay(1000);
    TEST_CASE_READY();

    ret_sta = csi_pwm_capture_config(&hd, td.channel, PWM_CAPTURE_POLARITY_POSEDGE, td.capture_count);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture config fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    ret_sta = csi_pwm_capture_config(&hd, td.channel, PWM_CAPTURE_POLARITY_NEGEDGE, td.capture_count);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture config fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    ret_sta = csi_pwm_capture_config(&hd, td.channel, PWM_CAPTURE_POLARITY_BOTHEDGE, td.capture_count);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture config fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    mdelay(1000);

    csi_pwm_capture_stop(&hd, td.channel);
    csi_pwm_detach_callback(&hd);
    csi_pwm_uninit(&hd);
    return 0;
}

int test_pwm_capture_abnormal_uninit(void *args)
{
    csi_error_t ret_sta;
    test_pwm_args_t td;
    csi_pwm_t hd;


    td.dev_idx          = *((uint64_t *)args);
    td.channel          = *((uint64_t *)args + 1);
    td.capture_polarity = *((uint64_t *)args + 2);
    td.capture_count    = *((uint64_t *)args + 3);
    td.delay_ms         = *((uint64_t *)args + 4);

    TEST_CASE_TIPS("test PWM idx is %d", td.dev_idx);
    TEST_CASE_TIPS("test pwm channel is %d", td.channel);
    TEST_CASE_TIPS("config capture polarity is %d", td.capture_polarity);
    TEST_CASE_TIPS("config capture count is %d", td.capture_count);
    TEST_CASE_TIPS("config signal received in %d ms", td.delay_ms);
	
    ret_sta = csi_pwm_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_pwm_capture_config(&hd, td.channel, td.capture_polarity, td.capture_count);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture config fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    volatile uint8_t event_lock;

    event_lock = 1;
    get_ch = 0;
    get_time = 0;
    ret_sta = csi_pwm_attach_callback(&hd, pwm_callback, (void *)&event_lock);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d attach callback fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    TEST_CASE_READY();
    ret_sta = csi_pwm_capture_start(&hd, td.channel);

    if (ret_sta != 0) {
        csi_pwm_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d capture start fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    mdelay(1000);
    TEST_CASE_READY();

    csi_pwm_uninit(&hd);
    mdelay(1000);

    csi_pwm_capture_stop(&hd, td.channel);
    csi_pwm_detach_callback(&hd);
    csi_pwm_uninit(&hd);
    return 0;
}