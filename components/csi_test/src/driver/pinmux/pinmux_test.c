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

#include <pinmux_test.h>

test_pinmux_args_t pinmux_config;

int test_pinmux_main(char *args)
{
    test_pinmux_args_t td;
    int ret;

    //	uint32_t get_data[3];
    uint32_t get_data[2];

    //	ret = args_parsing(args, get_data, 3);
    ret = args_parsing(args, get_data, 2);
    TEST_CASE_ASSERT_QUIT(ret == 0, "the number of parameters passed in does not match");

    //    td.dev_idx = (uint8_t)get_data[0];
    //    td.pin = (uint8_t)get_data[1];
    //    td.pin_func = (uint8_t)get_data[2];

    td.pin = (uint8_t)get_data[0];
    td.pin_func = (uint8_t)get_data[1];

    //	TEST_CASE_TIPS("dev idx is %d",td.dev_idx);
    TEST_CASE_TIPS("pin idx is %d", td.pin);
    TEST_CASE_TIPS("pin_func idx is %d", td.pin_func);



    if ((!strcmp((const char *)_mc_name, "PINMUX_CONFIG")) || (!strcmp((const char *)_mc_name, "PINMUX_CONFIG_ASSIST"))) {
        test_pinmux_config(&td);
        return 0;
    }

    TEST_CASE_ASSERT(1 == 0, "PINMUX don't supported this command");
    return -1;

}


