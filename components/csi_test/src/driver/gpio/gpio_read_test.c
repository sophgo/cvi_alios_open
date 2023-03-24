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

int test_gpio_pinsRead(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;

    uint32_t read_value;

    uint32_t expec_value, actual_value;

    td.gpio_idx     = *((uint64_t *)args);
    td.pin_mask     = *((uint64_t *)args + 1);
    td.gpio_mode    = *((uint64_t *)args + 2);
    td.pin_value    = *((uint64_t *)args + 3);
    td.dir          = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pins is 0x%x", td.pin_mask);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected read value is %d", td.pin_value);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, td.pin_mask, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, td.pin_mask, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    expec_value = 0xFFFFFFFF & td.pin_mask;


    TEST_CASE_READY();
    read_value = csi_gpio_read(&hd, td.pin_mask);
    actual_value = read_value & td.pin_mask;

    if (td.pin_value) {
        TEST_CASE_ASSERT(actual_value == expec_value, "read gpio error: expected value is %x,actual value is %x", expec_value,
                         actual_value);
    } else {
        TEST_CASE_ASSERT(actual_value == 0, "read gpio error: expected value is 0,actual value is %x",  actual_value);
    }

    csi_gpio_uninit(&hd);

    return 0;
}


#ifdef CSI_GPIO_DEBONCE
int test_gpio_deboncePinsRead(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;
    uint32_t read_value;

    uint32_t expec_value, actual_value;

    td.gpio_idx     = *((uint64_t *)args);
    td.pin_mask     = *((uint64_t *)args + 1);
    td.gpio_mode    = *((uint64_t *)args + 2);
    td.pin_value    = *((uint64_t *)args + 3);
    td.dir          = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pins is 0x%x", td.pin_mask);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected read value is %d", td.pin_value);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, td.pin_mask, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, td.pin_mask, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    expec_value = 0xFFFFFFFF & td.pin_mask;

    ret_status = csi_gpio_debounce(&hd, td.pin_mask, true);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio open debonce error, expected return value is 0, actual return value is %d.", ret_status);

    TEST_CASE_READY();

    read_value = csi_gpio_read(&hd, td.pin_mask);
    actual_value = read_value & td.pin_mask;

    ret_status = csi_gpio_debounce(&hd, td.pin_mask, false);
    TEST_CASE_ASSERT_QUIT(ret_status == -4,
                          "gpio close debonce error, expected return value is 0, actual return value is %d.", ret_status);

    if (td.pin_value) {
        TEST_CASE_ASSERT(actual_value == expec_value, "read gpio error: expected value is %x,actual value is %x", expec_value,
                         actual_value);
    } else {
        TEST_CASE_ASSERT(actual_value == 0, "read gpio error: expected value is 0,actual value is %x",  actual_value);
    }

    csi_gpio_uninit(&hd);
    return 0;
}
#endif



int test_gpio_pinRead(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;

    uint32_t read_value;

    uint32_t expec_value, actual_value;


    td.gpio_idx     = *((uint64_t *)args);
    td.pin          = *((uint64_t *)args + 1);
    td.gpio_mode    = *((uint64_t *)args + 2);
    td.pin_value    = *((uint64_t *)args + 3);
    td.dir          = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pin is %d", td.pin);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected read value is %d", td.pin_value);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, 1 << td.pin, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, 1 << td.pin, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    expec_value = td.pin_value;

    TEST_CASE_READY();

    read_value = csi_gpio_read(&hd, 1 << td.pin);
    actual_value = (read_value & (1 << td.pin)) >> td.pin;

    TEST_CASE_ASSERT(actual_value == expec_value, "read gpio error: expected value is %x,actual value is %x", expec_value,
                     actual_value);

    csi_gpio_uninit(&hd);

    return 0;
}


#ifdef CSI_GPIO_DEBONCE
int test_gpio_deboncePinRead(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;
    uint32_t read_value;

    uint32_t expec_value, actual_value;

    td.gpio_idx     = *((uint64_t *)args);
    td.pin          = *((uint64_t *)args + 1);
    td.gpio_mode    = *((uint64_t *)args + 2);
    td.pin_value    = *((uint64_t *)args + 3);
    td.dir          = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pin is %d", td.pin);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected read value is %d", td.pin_value);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, 1 << td.pin, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, 1 << td.pin, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    expec_value = td.pin_value;

    ret_status = csi_gpio_debounce(&hd, 1 << td.pin, true);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio open debonce error, expected return value is 0, actual return value is %d.", ret_status);

    TEST_CASE_READY();

    read_value = csi_gpio_read(&hd, 1 << td.pin);
    actual_value = (read_value & (1 << td.pin)) >> td.pin;

    ret_status = csi_gpio_debounce(&hd, 1 << td.pin, false);
    TEST_CASE_ASSERT_QUIT(ret_status == -4,
                          "gpio close debonce error, expected return value is 0, actual return value is %d.", ret_status);

    TEST_CASE_ASSERT(actual_value == expec_value, "read gpio error: expected value is %x,actual value is %x", expec_value,
                     actual_value);

    csi_gpio_uninit(&hd);
    return 0;
}
#endif


int test_gpio_pinsToggleRead(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;

    uint32_t read_value;

    uint32_t expec_value, actual_value;

    td.gpio_idx     = *((uint64_t *)args);
    td.pin_mask     = *((uint64_t *)args + 1);
    td.gpio_mode    = *((uint64_t *)args + 2);
    td.pin_value    = *((uint64_t *)args + 3);
    td.dir          = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pins is 0x%x", td.pin_mask);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected read value is %d", td.pin_value);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, td.pin_mask, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, td.pin_mask, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    expec_value = 0xFFFFFFFF & td.pin_mask;


    TEST_CASE_READY();
    read_value = csi_gpio_read(&hd, td.pin_mask);
    actual_value = read_value & td.pin_mask;

    if (td.pin_value) {
        TEST_CASE_ASSERT(actual_value == 0, "read gpio error: expected value is 0,actual value is %x",  actual_value);
    } else {
        TEST_CASE_ASSERT(actual_value == expec_value, "read gpio error: expected value is %x,actual value is %x", expec_value,
                         actual_value);
    }

    csi_gpio_uninit(&hd);

    return 0;
}


#ifdef CSI_GPIO_DEBONCE
int test_gpio_deboncePinsToggleRead(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;
    uint32_t read_value;

    uint32_t expec_value, actual_value;

    td.gpio_idx     = *((uint64_t *)args);
    td.pin_mask     = *((uint64_t *)args + 1);
    td.gpio_mode    = *((uint64_t *)args + 2);
    td.pin_value    = *((uint64_t *)args + 3);
    td.dir          = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pins is 0x%x", td.pin_mask);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("expected read value is %d", td.pin_value);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, td.pin_mask, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, td.pin_mask, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    expec_value = 0xFFFFFFFF & td.pin_mask;

    ret_status = csi_gpio_debounce(&hd, td.pin_mask, true);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio open debonce error, expected return value is 0, actual return value is %d.", ret_status);

    TEST_CASE_READY();

    read_value = csi_gpio_read(&hd, td.pin_mask);
    actual_value = read_value & td.pin_mask;

    ret_status = csi_gpio_debounce(&hd, td.pin_mask, false);
    TEST_CASE_ASSERT_QUIT(ret_status == -4,
                          "gpio close debonce error, expected return value is 0, actual return value is %d.", ret_status);

    if (td.pin_value) {
        TEST_CASE_ASSERT(actual_value == 0, "read gpio error: expected value is 0,actual value is %x",  actual_value);
    } else {
        TEST_CASE_ASSERT(actual_value == expec_value, "read gpio error: expected value is %x,actual value is %x", expec_value,
                         actual_value);
    }

    csi_gpio_uninit(&hd);
    return 0;
}
#endif
