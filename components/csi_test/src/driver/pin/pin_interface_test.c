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

int test_pin_interface(void *args)
{
    csi_error_t ret_sta;
/*    
    ret_sta = csi_pin_set_mux(NULL, 11);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pin_set_mux interface test fail, expected return value is not 0, actual return value is %d.", ret_sta);

    ret_sta = csi_pin_set_mux(0, NULL);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pin_set_mux interface test fail, expected return value is not 0, actual return value is %d.", ret_sta);

    pin_func_t pin_func;
	pin_func = csi_pin_get_mux(NULL);
    TEST_CASE_ASSERT(pin_func == 0,
                     "csi_pin_get_mux interface test fail, expected return value is 0, actual return value is %d.", pin_func);

	csi_pin_set_mux(0, 11);
    ret_sta = csi_pin_mode(NULL, 0);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pin_mode interface test fail, expected return value is not 0, actual return value is %d.", ret_sta);
*/
    ret_sta = csi_pin_mode(0, -1);
    TEST_CASE_ASSERT(ret_sta == CSI_UNSUPPORTED,
                     "csi_pin_mode interface test fail, expected return value is CSI_UNSUPPORTED, actual return value is %d.", ret_sta);

#ifdef CSI_PIN_SPEED
	ret_sta = csi_pin_speed(NULL, pin_arg.speed);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pin_speed interface test fail, expected return value is not 0, actual return value is %d.", ret_sta);

    ret_sta = csi_pin_speed(0, -1);
    TEST_CASE_ASSERT(ret_sta == CSI_UNSUPPORTED,
                     "csi_pin_speed interface test fail, expected return value is CSI_UNSUPPORTED, actual return value is %d.", ret_sta);
#endif

#ifdef CSI_PIN_DRIVE
	ret_sta = csi_pin_drive(NULL, pin_arg.drive);
	TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pin_drive interface test fail, expected return value is not 0, actual return value is %d.", ret_sta);

    ret_sta = csi_pin_drive(0, -1);
    TEST_CASE_ASSERT(ret_sta == CSI_UNSUPPORTED,
                     "csi_pin_drive interface test fail, expected return value is CSI_UNSUPPORTED, actual return value is %d.", ret_sta);
#endif

#ifdef CSI_PIN_GET_DEVIDX
    uint32_t idx;
    idx = csi_pin_get_gpio_devidx(-1);
    TEST_CASE_ASSERT(idx == 0xFFFFFFFFU,
                     "csi_pin_get_gpio_devidx interface test fail, expected return value is 0xFFFFFFFFU, actual return value is %d.", idx);

    idx = csi_pin_get_uart_devidx(-1);
    TEST_CASE_ASSERT(idx == 0xFFFFFFFFU,
                     "csi_pin_get_uart_devidx interface test fail, expected return value is 0xFFFFFFFFU, actual return value is %d.", idx);

    idx = csi_pin_get_iic_devidx(-1);
    TEST_CASE_ASSERT(idx == 0xFFFFFFFFU,
                     "csi_pin_get_iic_devidx interface test fail, expected return value is 0xFFFFFFFFU, actual return value is %d.", idx);

    idx = csi_pin_get_spi_devidx(-1);
    TEST_CASE_ASSERT(idx == 0xFFFFFFFFU,
                     "csi_pin_get_spi_devidx interface test fail, expected return value is 0xFFFFFFFFU, actual return value is %d.", idx);

    idx = csi_pin_get_i2s_devidx(-1);
    TEST_CASE_ASSERT(idx == 0xFFFFFFFFU,
                     "csi_pin_get_i2s_devidx interface test fail, expected return value is 0xFFFFFFFFU, actual return value is %d.", idx);
#endif

#ifdef CSI_PIN_GET_CHANNEL
    uint32_t channel;
    channel = csi_pin_get_gpio_channel(-1);
    TEST_CASE_ASSERT(channel == 0xFFFFFFFFU,
                     "csi_pin_get_gpio_channel interface test fail, expected return value is 0xFFFFFFFFU, actual return value is %d.", idx);

    channel = csi_pin_get_pwm_channel(-1);
    TEST_CASE_ASSERT(channel == 0xFFFFFFFFU,
                     "csi_pin_get_pwm_channel interface test fail, expected return value is 0xFFFFFFFFU, actual return value is %d.", idx);

    channel = csi_pin_get_adc_channel(-1);
    TEST_CASE_ASSERT(channel == 0xFFFFFFFFU,
                     "csi_pin_get_adc_channel interface test fail, expected return value is 0xFFFFFFFFU, actual return value is %d.", idx);
#endif

    return 0;
}
