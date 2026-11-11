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

static volatile uint8_t lock = 0;
static volatile uint32_t irq_pin;

static void gpio_callback(csi_gpio_t *hd, uint32_t pins, void *arg)
{
    lock++;
    irq_pin = pins;

    if (*(uint8_t *)arg != GPIO_IRQ_MODE_BOTH_EDGE) {
        csi_gpio_irq_enable(hd, pins, false);
    }
}

int test_gpio_interruptCapture(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;

    td.gpio_idx      = *((uint64_t *)args);
    td.pin_mask      = *((uint64_t *)args + 1);
    td.gpio_mode     = *((uint64_t *)args + 2);
    td.gpio_irq_mode = *((uint64_t *)args + 3);
    td.dir           = GPIO_DIRECTION_INPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pin is 0x%x", td.pin_mask);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("config irq mode is %d", td.gpio_irq_mode);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, td.pin_mask, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, td.pin_mask, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);

    ret_status = csi_gpio_irq_mode(&hd, td.pin_mask, td.gpio_irq_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio irq mode config error: expected return value is 0,actual return value is %d.", ret_status);



    ret_status = csi_gpio_attach_callback(&hd, gpio_callback, &td.gpio_irq_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio attach callback error, expected return value is 0, actual return value is %d.", ret_status);

    ret_status = csi_gpio_irq_enable(&hd, td.pin_mask, true);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio irq enable error, expected return value is 0, actual return value is %d.",
                          ret_status);

    lock = 0;

    TEST_CASE_READY();

    if (td.gpio_irq_mode == GPIO_IRQ_MODE_BOTH_EDGE) {
        while (lock != 2) {
            csi_gpio_pin_irq_enable(&hd, true);
        }
    } else {
        while (lock != 1) {
            ;
        }
    }

    //	while(1);

    ret_status = csi_gpio_irq_enable(&hd, td.pin_mask, false);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio irq disable error, expected return value is 0, actual return value is %d.",
                          ret_status);

    TEST_CASE_ASSERT(irq_pin == td.pin_mask,
                     "gpio trigger rising interrupt error, expected trigger pis is 0x%x,actual trigger pins is 0x%x", td.pin_mask, irq_pin);

    csi_gpio_detach_callback(&hd);

    csi_gpio_uninit(&hd);
    return 0;
}






int test_gpio_interruptTrigger(void *args)
{
    csi_gpio_t hd;
    test_gpio_args_t td;
    csi_error_t ret_status;


    td.gpio_idx      = *((uint64_t *)args);
    td.pin_mask      = *((uint64_t *)args + 1);
    td.gpio_mode     = *((uint64_t *)args + 2);
    td.gpio_irq_mode = *((uint64_t *)args + 3);
    td.delay_ms      = *((uint64_t *)args + 4);
    td.dir           = GPIO_DIRECTION_OUTPUT;

    TEST_CASE_TIPS("test GPIO idx is %d", td.gpio_idx);
    TEST_CASE_TIPS("config pin is 0x%x", td.pin_mask);
    TEST_CASE_TIPS("config direction is %d", td.dir);
    TEST_CASE_TIPS("config mode is %d", td.gpio_mode);
    TEST_CASE_TIPS("config triggle signle is %d", td.gpio_irq_mode);
    TEST_CASE_TIPS("config trigger delay is %d ms", td.delay_ms);

    ret_status = csi_gpio_init(&hd, td.gpio_idx);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio %d init fail, expected return value is 0, actual return value is %d.",
                          td.gpio_idx, ret_status);

    ret_status = csi_gpio_dir(&hd, td.pin_mask, td.dir);
    TEST_CASE_ASSERT_QUIT(ret_status == 0,
                          "gpio direction config error: expected return value is 0,actual return value is %d.", ret_status);

    ret_status = csi_gpio_mode(&hd, td.pin_mask, td.gpio_mode);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "gpio mode config error: expected return value is 0,actual return value is %d.",
                          ret_status);


    switch (td.gpio_irq_mode) {
    case GPIO_IRQ_MODE_RISING_EDGE:
        csi_gpio_write(&hd, td.pin_mask, false);
        TEST_CASE_READY();
        tst_mdelay(td.delay_ms);
        csi_gpio_write(&hd, td.pin_mask, true);
        break;

    case GPIO_IRQ_MODE_FALLING_EDGE:
        csi_gpio_write(&hd, td.pin_mask, true);
        TEST_CASE_READY();
        tst_mdelay(td.delay_ms);
        csi_gpio_write(&hd, td.pin_mask, false);
        break;

    case GPIO_IRQ_MODE_BOTH_EDGE:
        csi_gpio_write(&hd, td.pin_mask, false);
        TEST_CASE_READY();
        tst_mdelay(td.delay_ms);
        csi_gpio_write(&hd, td.pin_mask, true);
        tst_mdelay(100);
        csi_gpio_write(&hd, td.pin_mask, false);
        break;

    case GPIO_IRQ_MODE_LOW_LEVEL:
        csi_gpio_write(&hd, td.pin_mask, true);
        TEST_CASE_READY();
        tst_mdelay(td.delay_ms);
        csi_gpio_write(&hd, td.pin_mask, false);
        break;

    case GPIO_IRQ_MODE_HIGH_LEVEL:
        csi_gpio_write(&hd, td.pin_mask, false);
        TEST_CASE_READY();
        tst_mdelay(td.delay_ms);
        csi_gpio_write(&hd, td.pin_mask, true);
        break;

    default:
        break;


    }

    tst_mdelay(100);

    csi_gpio_uninit(&hd);
    return 0;
}

