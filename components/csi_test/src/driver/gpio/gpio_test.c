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

const test_info_t gpio_test_funcs_map[] = {
    {"GPIO_INTERFACE", test_gpio_interface, 0},
    {"GPIO_PIN_WRITE", test_gpio_pinWrite, 5},
    {"GPIO_PINS_WRITE", test_gpio_pinsWrite, 5},
    {"GPIO_TOGGLE", test_gpio_toggle, 5},
    {"GPIO_PIN_READ", test_gpio_pinRead, 4},
    {"GPIO_PINS_READ", test_gpio_pinsRead, 4},
    {"GPIO_PINS_TOGGLE_READ", test_gpio_pinsToggleRead, 4},
    {"GPIO_INTERRUPT_CAPTURE", test_gpio_interruptCapture, 4},
    {"GPIO_INTERRUPT_TRIGGER", test_gpio_interruptTrigger, 5},
    {"GPIO_READ_ABNORMAL", test_gpio_pinsReadAbnormal, 4},
    {"GPIO_WRITE_ABNORMAL", test_gpio_pinsWriteAbnormal, 5},
    {"GPIO_TOGGLE_ABNORMAL", test_gpio_toggleAbnormal, 5},

#ifdef CSI_GPIO_DEBONCE
    {"GPIO_PIN_DEBONCE_WRITE", test_gpio_pinDebonceWrite, 5},
    {"GPIO_PINS_DEBONCE_WRITE", test_gpio_pinsDebonceWrite, 5},
    {"GPIO_DEBONCE_TOGGLE", test_gpio_debonceToggle, 5},
    {"GPIO_DEBONCE_PIN_READ", test_gpio_deboncePinRead, 4},
    {"GPIO_DEBONCE_PINS_READ", test_gpio_deboncePinsRead, 4},
    {"GPIO_DEBONCE_PINS_TOGGLE_READ", test_gpio_deboncePinsToggleRead, 4},
#endif
};

int test_gpio_main(char *args)
{
    int ret;
    ret = testcase_jump(args, (void *)gpio_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("GPIO don't supported this command");
    }

    return ret;
}

