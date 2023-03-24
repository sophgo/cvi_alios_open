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

#include <gpio_pin_test.h>

int test_gpio_pin_pinReadAbnormal(void *args)
{
    csi_gpio_pin_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;

    uint32_t read_value;

    // uint32_t expec_value, actual_value;

    td.gpio_idx     = *((uint64_t *)args);
    td.gpio_mode    = *((uint64_t *)args + 1);
    td.pin_value    = *((uint64_t *)args + 2);
    td.dir          = GPIO_DIRECTION_OUTPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected read value is %d", td.pin_value);

    ret_status = csi_gpio_pin_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_pin_dir(&hd, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_pin_mode(&hd, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    TEST_CASE_READY();
    read_value = csi_gpio_pin_read(&hd);

    TEST_CASE_ASSERT_QUIT(read_value == td.pin_value, "read gpio error: expected value is %x,actual value is %x", td.pin_value, read_value);


    csi_gpio_pin_uninit(&hd);

    return 0;
}

int test_gpio_pin_pinWriteAbnormal(void *args)
{
    csi_gpio_pin_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;


    td.gpio_idx     = *((uint64_t *)args);
    td.gpio_mode    = *((uint64_t *)args + 1);
    td.pin_value    = *((uint64_t *)args + 2);
    td.delay_ms     = *((uint64_t *)args + 3);
    td.dir          = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected write value is %d", td.pin_value);
    TEST_CASE_TIPS("output delay %d ms", td.delay_ms);


    ret_status = csi_gpio_pin_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_pin_dir(&hd, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_pin_mode(&hd, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    csi_gpio_pin_state_t value;

    value = td.pin_value;


    csi_gpio_pin_write(&hd, value);

    tst_mdelay(50);

    TEST_CASE_READY();

    tst_mdelay(td.delay_ms);
    csi_gpio_pin_uninit(&hd);
    return 0;
}


int test_gpio_pin_toggleAbnormal(void *args)
{
    csi_gpio_pin_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;


    td.gpio_idx     = *((uint64_t *)args);
    td.gpio_mode    = *((uint64_t *)args + 1);
    td.pin_value    = *((uint64_t *)args + 2);
    td.delay_ms     = *((uint64_t *)args + 3);
    td.dir          = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected write value is %d", td.pin_value);
    TEST_CASE_TIPS("output delay %d ms", td.delay_ms);


    ret_status = csi_gpio_pin_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_pin_dir(&hd, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_pin_mode(&hd, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    csi_gpio_pin_state_t value;

    if (td.pin_value == GPIO_PIN_LOW) {
        value = GPIO_PIN_HIGH;
    } else {
        value = GPIO_PIN_LOW;
    }


    csi_gpio_pin_write(&hd, value);

    tst_mdelay(50);

    csi_gpio_pin_toggle(&hd);

    TEST_CASE_READY();

    tst_mdelay(td.delay_ms);

    csi_gpio_pin_uninit(&hd);
    return 0;
}