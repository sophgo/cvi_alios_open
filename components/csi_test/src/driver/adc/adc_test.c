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

#include <adc_test.h>


const test_info_t adc_test_funcs_map[] = {
    {"ADC_INTERFACE", test_adc_interface, 0},
    {"ADC_SYNC", test_adc_sync, 7},
    {"ADC_ASYNC", test_adc_async, 7},
#ifdef CSI_ADC_LINK_DMA
    {"ADC_DMA", test_adc_dma, 7},
#endif
#ifdef CSI_ADC_GET_STATE
    {"ADC_STATUS", test_adc_status, 2},
#endif

};


int test_adc_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)adc_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("ADC don't supported this command,%d", ret);
    }

    return ret;
}