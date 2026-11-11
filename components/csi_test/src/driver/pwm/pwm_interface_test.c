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

int test_pwm_interface(void *args)
{
    csi_error_t ret_sta;
    csi_pwm_t td;


    ret_sta = csi_pwm_init(NULL, 0);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_init interface test fail, expected return value is 0, actual return value is %d.", ret_sta);

    ret_sta = csi_pwm_init(&td, 0xffff);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_init interface test fail, expected return value is 0, actual return value is %d.", ret_sta);

    csi_pwm_uninit(NULL);

    ret_sta = csi_pwm_out_config(NULL, 0, 10, 5, 0);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_out_config interface test fail, expected return value is not 0, actual return value is %d.", ret_sta);
    csi_pwm_init(&td, 0);
    ret_sta = csi_pwm_out_config(&td, 0, 0, 5, 0);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_out_config interface test fail, expected return value is not 0, actual return value is %d.", ret_sta);
    ret_sta = csi_pwm_out_config(&td, 0, 10, 0, 0);
    TEST_CASE_ASSERT(ret_sta == 0,
                     "csi_pwm_out_config interface test fail, expected return value is 0, actual return value is %d.", ret_sta);
    csi_pwm_uninit(&td);

    ret_sta = csi_pwm_out_start(NULL, 0);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_out_start interface test fail, expected return value is 0, actual return value is %d.", ret_sta);


    csi_pwm_out_stop(NULL, 0);


    ret_sta = csi_pwm_capture_config(NULL, 0, 0, 1);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_capture_config interface test fail, expected return value is 0, actual return value is %d.", ret_sta);
    ret_sta = csi_pwm_capture_config(&td, 0, 0, 0);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_capture_config interface test fail, expected return value is 0, actual return value is %d.", ret_sta);


    ret_sta = csi_pwm_capture_start(NULL, 1);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_capture_start interface test fail, expected return value is 0, actual return value is %d.", ret_sta);


    csi_pwm_capture_stop(NULL, 1);

    ret_sta = csi_pwm_attach_callback(NULL, test_pwm_output, NULL);
    TEST_CASE_ASSERT(ret_sta != 0,
                     "csi_pwm_attach_callback interface test fail, expected return value is 0, actual return value is %d.", ret_sta);
    ret_sta = csi_pwm_attach_callback(&td, NULL, NULL);

    csi_pwm_detach_callback(NULL);


#ifdef CONFIG_PM
    csi_pwm_enable_pm(NULL);

    ret_status = csi_pwm_disable_pm(NULL);
    TEST_CASE_ASSERT(ret_status == -1, "uart parameter is NULL, csi_pwm_disable_pm interface testing fail");
#endif

    return 0;
}
