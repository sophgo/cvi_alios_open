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

int test_pwm_captureInput(void *args)
{
    csi_gpio_t hd;
    csi_error_t ret_sta;
    test_pwm_args_t td;


    td.dev_idx 			= *((uint64_t *)args);
    td.pin 				= *((uint64_t *)args + 1);
    td.capture_polarity = *((uint64_t *)args + 2);
    td.capture_count 	= *((uint64_t *)args + 3);
    td.delay_ms 		= *((uint64_t *)args + 4);

    TEST_CASE_TIPS("config capture input gpio idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config  capture input pin is %d", td.pin);
    TEST_CASE_TIPS("test capture polarity is %d", td.capture_polarity);
    TEST_CASE_TIPS("test capture count is %d", td.capture_count);
    TEST_CASE_TIPS("config delay of %d ms, the single is output", td.delay_ms);


    ret_sta = csi_gpio_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_gpio_dir(&hd, 1 << td.pin, GPIO_DIRECTION_OUTPUT);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "gpio %d config dir fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_gpio_mode(&hd, 1 << td.pin, GPIO_MODE_PULLNONE);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "gpio %d config mode fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);



    if (td.capture_polarity == PWM_CAPTURE_POLARITY_POSEDGE) {
        csi_gpio_write(&hd, 1 << td.pin, 0);
        TEST_CASE_READY();
        tst_mdelay(td.delay_ms);

        for (int i = 0; i < td.capture_count; i++) {
            tst_mdelay(1);
            csi_gpio_write(&hd, 1 << td.pin, 0);
            tst_mdelay(9);
            csi_gpio_write(&hd, 1 << td.pin, 1);
        }
    }

    if (td.capture_polarity == PWM_CAPTURE_POLARITY_NEGEDGE) {
        csi_gpio_write(&hd, 1 << td.pin, 1);
        TEST_CASE_READY();
        tst_mdelay(td.delay_ms);

        for (int i = 0; i < td.capture_count; i++) {
            tst_mdelay(1);
            csi_gpio_write(&hd, 1 << td.pin, 1);
            tst_mdelay(9);
            csi_gpio_write(&hd, 1 << td.pin, 0);
        }
    }


    if (td.capture_polarity == PWM_CAPTURE_POLARITY_BOTHEDGE) {
        csi_gpio_write(&hd, 1 << td.pin, 0);
        TEST_CASE_READY();
        tst_mdelay(td.delay_ms);

        for (int i = 0; i < td.capture_count; i++) {
            tst_mdelay(10);
            csi_gpio_toggle(&hd, 1 << td.pin);
        }
    }




    csi_gpio_uninit(&hd);



    return 0;
}
