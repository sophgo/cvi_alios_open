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

const test_info_t gpio_pin_test_funcs_map[] = {
    {"GPIOPIN_INTERFACE", test_gpio_pin_interface, 0},
    {"GPIOPIN_PIN_WRITE", test_gpio_pin_pinWrite, 4},
    {"GPIOPIN_TOGGLE", test_gpio_pin_toggle, 4},
    {"GPIOPIN_PIN_READ", test_gpio_pin_pinRead, 3},
    {"GPIOPIN_PIN_TOGGLE_READ", test_gpio_pin_pinToggleRead, 3},
    {"GPIOPIN_INTERRUPT_CAPTURE", test_gpio_pin_interruptCapture, 3},
    {"GPIOPIN_INTERRUPT_TRIGGER", test_gpio_pin_interruptTrigger, 4},
    {"GPIOPIN_READ_ABNORMAL", test_gpio_pin_pinReadAbnormal, 3},
    {"GPIOPIN_WRITE_ABNORMAL", test_gpio_pin_pinWriteAbnormal, 4},
    {"GPIOPIN_TOGGLE_ABNORMAL", test_gpio_pin_toggleAbnormal, 4},

#ifdef CSI_GPIO_PIN_DEBONCE
    {"GPIOPIN_PIN_DEBONCE_WRITE", test_gpio_pin_pinDebonceWrite, 4},
    {"GPIOPIN_DEBONCE_TOGGLE", test_gpio_pin_debonceToggle, 4},
    {"GPIOPIN_DEBONCE_PIN_READ", test_gpio_pin_deboncePinRead, 3},
    {"GPIOPIN_DEBONCE_PIN_TOGGLE_READ", test_gpio_pin_deboncePinToggleRead, 3},
#endif
};

int test_gpio_pin_main(char *args)
{
    int ret;
    ret = testcase_jump(args, (void *)gpio_pin_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("GPIO don't supported this command");
    }

    return ret;
}

