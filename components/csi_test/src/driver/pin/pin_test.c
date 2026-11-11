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
#include <drv/pin.h>
#include <pin_test.h>

const test_info_t pin_funcs_map[] = {
    {"PIN_INTERFACE", test_pin_interface, 0},
    {"PIN_CONFIG", test_pin_config, 5},
#ifdef CSI_PIN_GET_DEVIDX
    {"PIN_GET_DEVIDX", test_pin_get_devidx, 3},
#endif
#ifdef CSI_PIN_GET_CHANNEL
    {"PIN_GET_CHANNEL", test_pin_get_channel, 3},
#endif
#ifdef CSI_PIN_GET_PINNAME
    {"PIN_GET_PINNAME", test_pin_get_pinname, 3},
#endif
};

int test_pin_main(char *args)
{
    int ret;
    ret = testcase_jump(args, (void *)pin_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("pin don't supported this command");
    }

    return ret;
}
