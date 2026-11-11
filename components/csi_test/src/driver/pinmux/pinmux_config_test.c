/*
 * Copyright (C) 2017 C-SKY Microsystems Co., Ltd. All rights reserved.
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
#include <pinmux_test.h>

/**
	\brief		Configure the pins used by the test module.
	\param[in]	pins	pinmux config data,\ref pin_config_t.
	\return		None.

*/
int test_pinmux_config(test_pinmux_args_t *pins)
{
    int32_t ret;

    ret = csi_pin_set_mux(pins->pin, pins->pin_func);
    TEST_CASE_ASSERT_QUIT(ret == 0, "when pin is %d, pin_func is %d; pinmux config error", pins->pin, pins->pin_func);


    return 0;
}

