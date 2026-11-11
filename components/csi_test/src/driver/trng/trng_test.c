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

#include <trng_test.h>

const test_info_t trng_test_funcs_map[] = {
	{"TRNG_INTERFACE",test_trng_interface, 0},
	{"TRNG_HANDLE",test_trng_handle, 0},
	{"TRNG_MULTI",test_trng_multi, 0},
	{"TRNG_SINGLE",test_trng_single, 0},
	{"TRNG_MULTI_PERFORMANCE",test_trng_multi_performance, 0},
	{"TRNG_SINGLE_PERFORMANCE",test_trng_single_performance, 0},
	{"TRNG_INTERFACE",test_trng_interface, 0},
	{"TRNG_SINGLE_STABILITY",test_trng_single_stability, 0},
	{"TRNG_MULTI_STABILITY",test_trng_multi_stability, 0},
};

int test_trng_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)trng_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("TRNG don't supported this command,%d", ret);
    }

    return ret;

}
