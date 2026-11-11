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

int test_pin_config(void *args)
{
	uint32_t ret = 0;
	test_pin_args_t pin_arg;
	pin_func_t pin_func = 0;

	pin_arg.idx = *((uint64_t *)args);
	pin_arg.func = *((uint64_t *)args + 1);
	pin_arg.mode = *((uint64_t *)args + 2);
	pin_arg.speed = *((uint64_t *)args + 3);
	pin_arg.drive = *((uint64_t *)args + 4);

	TEST_CASE_TIPS("begin test pin idx %u, func %u, mode %u, speed %u, drive %u ",
				   pin_arg.idx, pin_arg.func, pin_arg.mode);

	ret = csi_pin_set_mux(pin_arg.idx, pin_arg.func);
	TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_pin_set_mux failed with idx: %d", pin_arg.idx);

	pin_func = csi_pin_get_mux(pin_arg.idx);
	TEST_CASE_ASSERT_QUIT(pin_func == pin_arg.func, "pin_func no match pin_arg.func");

	ret = csi_pin_mode(pin_arg.idx, pin_arg.mode);
	TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_pin_mode failed with idx: %d", pin_arg.idx);

#ifdef CSI_PIN_SPEED
	ret = csi_pin_speed(pin_arg.idx, pin_arg.speed);
	TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_pin_speed failed with idx: %d", pin_arg.idx);
#endif

#ifdef CSI_PIN_DRIVE
	ret = csi_pin_drive(pin_arg.idx, pin_arg.drive);
	TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_pin_drive failed with idx: %d", pin_arg.idx);
#endif

	return 0;
}
