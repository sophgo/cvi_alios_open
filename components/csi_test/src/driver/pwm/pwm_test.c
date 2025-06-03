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

#include <pwm_test.h>


const test_info_t pwm_test_funcs_map[] = {
    {"PWM_INTERFACE", test_pwm_interface, 0},
    {"PWM_OUTPUT", test_pwm_output, 6},
    {"PWM_OUTPUT_CHECK", test_pwm_outputCheck, 5},
    {"PWM_CAPTURE", test_pwm_capture, 5},
    {"PWM_CAPTURE_INPUT", test_pwm_captureInput, 5},

};


int test_pwm_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)pwm_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("PWM don't supported this command");
    }

    return ret;
}






