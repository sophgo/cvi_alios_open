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

int test_pwm_output(void *args)
{
    csi_error_t ret_sta;
    test_pwm_args_t td;
    csi_pwm_t hd;

    td.dev_idx          = *((uint64_t *)args);
    td.channel          = *((uint64_t *)args + 1);
    td.period           = *((uint64_t *)args + 2);
    td.pulse_width      = *((uint64_t *)args + 3);
    td.output_polarity  = *((uint64_t *)args + 4);
    td.delay_ms         = *((uint64_t *)args + 5);

    TEST_CASE_TIPS("test PWM idx is %d", td.dev_idx);
    TEST_CASE_TIPS("test pwm channel is %d", td.channel);
    TEST_CASE_TIPS("config period is %d us", td.period);
    TEST_CASE_TIPS("config pulse width is %d us", td.pulse_width);
    TEST_CASE_TIPS("config output polarity is %d", td.output_polarity);
    TEST_CASE_TIPS("config output time is %d ms", td.delay_ms);


    ret_sta = csi_pwm_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);


    ret_sta = csi_pwm_out_config(&hd, td.channel, td.period, td.pulse_width, td.output_polarity);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d out config fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    TEST_CASE_READY();

    ret_sta = csi_pwm_out_start(&hd, td.channel);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0,
                          "pwm channel %d output fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);

    tst_mdelay(td.delay_ms);

    csi_pwm_out_stop(&hd, td.channel);


    csi_pwm_uninit(&hd);
    return 0;
}

int test_pwm_output_abnormal_config(void *args)
{
    csi_error_t ret_sta;
    test_pwm_args_t td;
    csi_pwm_t hd;

    td.dev_idx          = *((uint64_t *)args);
    td.channel          = *((uint64_t *)args + 1);
    td.period           = *((uint64_t *)args + 2);
    td.pulse_width      = *((uint64_t *)args + 3);
    td.output_polarity  = *((uint64_t *)args + 4);
    td.delay_ms         = *((uint64_t *)args + 5);

    TEST_CASE_TIPS("test PWM idx is %d", td.dev_idx);
    TEST_CASE_TIPS("test pwm channel is %d", td.channel);
    TEST_CASE_TIPS("config period is %d us", td.period);
    TEST_CASE_TIPS("config pulse width is %d us", td.pulse_width);
    TEST_CASE_TIPS("config output polarity is %d", td.output_polarity);
    TEST_CASE_TIPS("config output time is %d ms", td.delay_ms);

    ret_sta = csi_pwm_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_pwm_out_config(&hd, td.channel, td.period, td.pulse_width, td.output_polarity);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d out config fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    TEST_CASE_READY();

    ret_sta = csi_pwm_out_start(&hd, td.channel);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0,
                          "pwm channel %d output fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);

    tst_mdelay(td.delay_ms);

    ret_sta = csi_pwm_out_config(&hd, td.channel, td.period, td.pulse_width, PWM_POLARITY_HIGH);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d out config fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);
    ret_sta = csi_pwm_out_config(&hd, td.channel, td.period, td.pulse_width, PWM_POLARITY_LOW);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d out config fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    csi_pwm_out_stop(&hd, td.channel);
    csi_pwm_uninit(&hd);
    return 0;
}
