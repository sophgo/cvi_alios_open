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

#include <efuse_test.h>

const test_info_t efuse_test_funcs_map[] = {
	{"EFUSE_INTERFACE",test_efuse_interface, 0},
	{"EFUSE_READPROGRAM",test_efuse_readprogram, 3},
};


int test_efuse_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)efuse_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("EFUSE don't supported this command,%d", ret);
    }

    return ret;
    
}