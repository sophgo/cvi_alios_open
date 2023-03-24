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

#ifdef CSI_PIN_GET_DEVIDX
int test_pin_get_devidx(void *args)
{
	test_pin_args_t pin_arg = {0};
	uint32_t idx = 0;
	uint32_t expect_idx = 0;
	uint32_t type = 0;

	pin_arg.idx = *((uint64_t *)args);
	type = *((uint64_t *)args + 1);
	expect_idx = *((uint64_t *)args + 2);

	switch (type) {
    case GPIO:
        idx = csi_pin_get_gpio_devidx(pin_arg.idx);
		TEST_CASE_TIPS("gpio_idx %u", idx);
		TEST_CASE_ASSERT(idx == expect_idx,
							"csi_pin_get_gpio_devidx expect idx %d, actual idx: %u", expect_idx, idx);
        break;

    case UART:
        idx = csi_pin_get_uart_devidx(pin_arg.idx);
		TEST_CASE_TIPS("uart_idx %u", idx);
		TEST_CASE_ASSERT(idx == expect_idx,
							"csi_pin_get_uart_devidx expect idx %d, actual idx: %u", expect_idx, idx);
        break;

    case IIC:
        idx = csi_pin_get_iic_devidx(pin_arg.idx);
		TEST_CASE_TIPS("iic_idx %u", idx);
		TEST_CASE_ASSERT(idx == expect_idx,
							"csi_pin_get_iic_devidx expect idx %d, actual idx: %u", expect_idx, idx);
        break;

    case SPI:
        idx = csi_pin_get_spi_devidx(pin_arg.idx);
		TEST_CASE_TIPS("spi_idx %u", idx);
		TEST_CASE_ASSERT(idx == expect_idx,
							"csi_pin_get_spi_devidx expect idx %d, actual idx: %u", expect_idx, idx);
        break;

    case I2S:
        idx = csi_pin_get_i2s_devidx(pin_arg.idx);
		TEST_CASE_TIPS("i2s_idx %u", idx);
		TEST_CASE_ASSERT(idx == expect_idx,
							"csi_pin_get_i2s_devidx expect idx %d, actual idx: %u", expect_idx, idx);
        break;

    default:
        break;
    }

	return 0;
}
#endif

#ifdef CSI_PIN_GET_CHANNEL
int test_pin_get_channel(void *args)
{
	test_pin_args_t pin_arg = {0};
	uint32_t channel = 0;
	uint32_t expect_channel = 0;
	uint32_t type = 0;

	pin_arg.idx = *((uint64_t *)args);
	type = *((uint64_t *)args + 1);
	expect_channel = *((uint64_t *)args + 2);

	switch (type) {
    case GPIO:
        channel = csi_pin_get_gpio_channel(pin_arg.idx);
		TEST_CASE_TIPS("gpio_channel %u", channel);
		TEST_CASE_ASSERT(channel == expect_channel,
							"csi_pin_get_gpio_channel expect channel %d, actual channel: %u", expect_channel, channel);
        break;

    case PWM:
        channel = csi_pin_get_pwm_channel(pin_arg.idx);
		TEST_CASE_TIPS("pwm_channel %u", channel);
		TEST_CASE_ASSERT(channel == expect_channel,
							"csi_pin_get_pwm_channel pexpect channel %d, actual channel: %u", expect_channel, channel);
        break;

    case ADC:
        channel = csi_pin_get_adc_channel(pin_arg.idx);
		TEST_CASE_TIPS("adc_channel %u", channel);
		TEST_CASE_ASSERT(channel == expect_channel,
							"csi_pin_get_adc_channel expect channel %d, actual channel: %u", expect_channel, channel);
        break;

    default:
        break;
    }

	return 0;
}
#endif

#ifdef CSI_PIN_GET_PINNAME
int test_pin_get_pinname(void *args)
{
	test_pin_args_t pin_arg = {0};
	uint8_t gpio_idx = 0;
	uint8_t gpio_channel = 0;
	uint32_t expect_pinname = 0;
	pin_name_t pin_name = 0;

	pin_arg.idx = *((uint8_t *)args);
	gpio_channel = *((uint8_t *)args + 1);
	expect_pinname = *((uint64_t *)args + 2);

	pin_name = csi_pin_get_pinname_by_gpio(pin_arg.idx, gpio_channel);
	TEST_CASE_ASSERT(pin_name == expect_pinname,
							"csi_pin_get_pinname_by_gpio expect pin_name %d, actual pin_name: %u", expect_pinname, pin_name);

	return 0;
}
#endif