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

static volatile uint32_t period_time[10] = {0}, high_level_time[10] = {0};
static volatile uint8_t counter = 0;
static volatile uint8_t lock = 0;

static void gpio_callback(csi_gpio_t *gpio, uint32_t pins, void *arg)
{
    lock = 0;

    if ((pins & (1 << (*(uint8_t *)arg))) && (counter < 10)) {
        period_time[counter] = tst_timer_get_interval();
        counter++;
    }

}


int test_pwm_outputCheck(void *args)
{
    csi_gpio_t hd;
    csi_error_t ret_sta;
    uint32_t read_value;
    uint32_t period = 0;
    uint32_t pulse_width = 0;
    test_pwm_args_t td;


    td.dev_idx 			= *((uint64_t *)args);
    td.pin 				= *((uint64_t *)args + 1);
    td.period 			= *((uint64_t *)args + 2);
    td.pulse_width 		= *((uint64_t *)args + 3);
    td.output_polarity  = *((uint64_t *)args + 4);

    TEST_CASE_TIPS("config PWM output check gpio idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config  PWM output check pin is %d", td.pin);
    TEST_CASE_TIPS("config period is %d us", td.period);
    TEST_CASE_TIPS("config pulse width is %d us", td.pulse_width);
    TEST_CASE_TIPS("config output polarity is %d", td.output_polarity);



    ret_sta = csi_gpio_init(&hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_gpio_dir(&hd, 1 << td.pin, GPIO_DIRECTION_INPUT);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "gpio %d config dir fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);


    ret_sta = csi_gpio_mode(&hd, 1 << td.pin, GPIO_MODE_PULLUP);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "gpio %d config mode fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_gpio_irq_mode(&hd, 1 << td.pin, GPIO_IRQ_MODE_RISING_EDGE);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0,
                          "gpio %d config irq mode fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);

    ret_sta = csi_gpio_attach_callback(&hd, gpio_callback, &td.pin);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0,
                          "gpio %d attach callback fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);


    counter = 0;
    lock = 1;
    TEST_CASE_READY();

    tst_timer_restart();
    ret_sta = csi_gpio_irq_enable(&hd, 1 << td.pin, true);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0,
                          "gpio %d pin irq enable fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);

    for (; counter < 10;) {
        if (lock == 0) {
            read_value = csi_gpio_read(&hd, 1 << td.pin);

            if ((read_value & 1 << td.pin) == 0) {
                high_level_time[counter] = tst_timer_get_interval();
                lock = 1;
            }
        }
    }

    ret_sta = csi_gpio_irq_enable(&hd, 1 << td.pin, false);
    TEST_CASE_ASSERT(ret_sta == 0, "gpio %d pin irq disable fail, expected return value is 0, actual return value is %d.",
                     td.dev_idx, ret_sta);

    csi_gpio_detach_callback(&hd);

    csi_gpio_uninit(&hd);

    for (int i = 0; i < 9; i++) {
        period += period_time[i + 1] - period_time[i];
        pulse_width += high_level_time[i + 1] - period_time[i];
    }

    period = period / 9;

    if (((period - 1) > td.period / 1000) || ((period + 1) < td.period / 1000)) {
        TEST_CASE_ASSERT(1 == 0, "the test of period value is %d, set value is %d", period, td.period / 1000);
    }

    pulse_width = pulse_width / 9;

    if (td.output_polarity == PWM_POLARITY_HIGH) {
        if (((pulse_width - 1) > td.pulse_width / 1000) || ((pulse_width + 1) < td.pulse_width / 1000)) {
            TEST_CASE_ASSERT(1 == 0, "the test of pulse_width value is %d, set value is %d", pulse_width, td.pulse_width / 1000);
        }
    } else {
        pulse_width = period - pulse_width;

        if (((pulse_width - 1) > td.pulse_width / 1000) || ((pulse_width + 1) < td.pulse_width / 1000)) {
            TEST_CASE_ASSERT(1 == 0, "the test of pulse_width value is %d, set value is %d", pulse_width, td.pulse_width / 1000);
        }
    }

    return 0;
}
