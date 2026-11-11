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

#include <gpio_test.h>


int test_gpio_toggle(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;

    td.gpio_idx     = *((uint64_t *)args);
    td.pin_mask     = *((uint64_t *)args + 1);
    td.gpio_mode    = *((uint64_t *)args + 2);
    td.pin_value    = *((uint64_t *)args + 3);
    td.delay_ms     = *((uint64_t *)args + 4);
    td.dir          = GPIO_DIRECTION_OUTPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pin is 0x%x", td.pin_mask);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected write value is %d", td.pin_value);
    TEST_CASE_TIPS("output delay %d ms", td.delay_ms);


    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, td.pin_mask, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, td.pin_mask, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    csi_gpio_pin_state_t value;
    value = td.pin_value;


    csi_gpio_write(&hd, td.pin_mask, value);

    tst_mdelay(50);

    csi_gpio_toggle(&hd, td.pin_mask);

    TEST_CASE_READY();

    tst_mdelay(td.delay_ms);

    csi_gpio_uninit(&hd);
    return 0;
}


#ifdef CSI_GPIO_DEBONCE
int test_gpio_debonceToggle(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;

    td.gpio_idx     = *((uint64_t *)args);
    td.pin_mask     = *((uint64_t *)args + 1);
    td.gpio_mode    = *((uint64_t *)args + 2);
    td.pin_value    = *((uint64_t *)args + 3);
    td.delay_ms     = *((uint64_t *)args + 4);
    td.dir          = GPIO_DIRECTION_OUTPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pin is 0x%x", td.pin_mask);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected write value is %d", td.pin_value);
    TEST_CASE_TIPS("output delay %d ms", td.delay_ms);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, td.pin_mask, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, td.pin_mask, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);

    ret_status = csi_gpio_debounce(&hd, td.pin_mask, true);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio open debonce error, expected return value is 0, actual return value is %d.", ret_status);

    csi_gpio_pin_state_t value;

    value = td.pin_value;


    csi_gpio_write(&hd, td.pin_mask, value);
    tst_mdelay(50);

    csi_gpio_toggle(&hd, td.pin_mask);

    TEST_CASE_READY();

    tst_mdelay(td.delay_ms);


    ret_status = csi_gpio_debounce(&hd, td.pin_mask, false);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio close debonce error, expected return value is 0, actual return value is %d.", ret_status);

    csi_gpio_uninit(&hd);
    return 0;
}
#endif
