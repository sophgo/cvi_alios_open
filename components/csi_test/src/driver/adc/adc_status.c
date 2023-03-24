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

#ifdef CSI_ADC_GET_STATE

#include "adc_test.h"


int test_adc_status(void *args)
{
    csi_error_t ret;
    csi_adc_t adc;
    uint32_t freq, freq_read;
    csi_state_t state;
    uint8_t i;
    int32_t data;

    uint8_t ad_idx;
    uint8_t ad_ch;

    ad_idx = *((uint64_t *)args);
    ad_ch  = *((uint64_t *)args + 1);

    TEST_CASE_TIPS("test ADC idx is %d", ad_idx);
    TEST_CASE_TIPS("test channel is %d", ad_ch);

    /*adc init*/
    ret = csi_adc_init(&adc, ad_idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "ADC init failed");

    /*adc set freq div :64*/
    freq =  csi_adc_freq_div(&adc, 64);
    TEST_CASE_ASSERT(freq > 0, "ADC set freq failed");
    freq_read =  csi_adc_get_freq(&adc);
    TEST_CASE_ASSERT(freq > 0, "ADC set freq failed");
    TEST_CASE_ASSERT(freq == freq_read, "ADC set freq err");
    TEST_CASE_TIPS("set freq:%d, read freq:%d", freq, freq_read);

    /*adc set freq div :128*/
    freq =  csi_adc_freq_div(&adc, 128);
    TEST_CASE_ASSERT(freq > 0, "ADC set freq failed");
    freq_read =  csi_adc_get_freq(&adc);
    TEST_CASE_ASSERT(freq > 0, "ADC set freq failed");
    TEST_CASE_ASSERT(freq == freq_read, "ADC set freq err");
    TEST_CASE_TIPS("set freq:%d, read freq:%d", freq, freq_read);

    /*adc set samling time*/
    ret = csi_adc_sampling_time(&adc, 2);
    TEST_CASE_ASSERT(ret == CSI_OK, "ADC set samplling time failed");

    /*adc set mode ： continue*/
    ret = csi_adc_continue_mode(&adc, true);
    TEST_CASE_ASSERT(ret == CSI_OK, "ADC set continue_mode failed");

    /*adc ch enable*/
    ret = csi_adc_channel_enable(&adc, ad_ch, true);
    TEST_CASE_ASSERT(ret == CSI_OK, "Enable ADC channel failed, ch_id: %d", ad_ch);

    /*adc get state :init*/
    ret = csi_adc_get_state(&adc, &state);
    TEST_CASE_ASSERT(ret == CSI_OK, "ADC get state failed");
    TEST_CASE_ASSERT(state.writeable == 1, "ADC get state err");
    TEST_CASE_TIPS("adc init status,adc state :%d", state.writeable);

    ret = csi_adc_start(&adc);
    TEST_CASE_ASSERT(ret == CSI_OK, "ADC start failed");

    /*adc get state :start*/
    ret = csi_adc_get_state(&adc, &state);
    TEST_CASE_ASSERT(ret == CSI_OK, "ADC get state failed");
    TEST_CASE_ASSERT(state.writeable == 0, "ADC get state err");
    TEST_CASE_TIPS("adc start status,adc state :%d", state.writeable);

    /* Read result */
    for (i = 0; i < 5; i++) {
        data = csi_adc_read(&adc);
        TEST_CASE_ASSERT(data >= 0, "ADC read failed");
        TEST_CASE_TIPS("get adc result: %d\n", data);
    }

    /*adc ch disable*/
    ret = csi_adc_channel_enable(&adc, ad_ch, false);
    TEST_CASE_ASSERT(ret == CSI_OK, "Enable ADC channel failed, ch_id: %d", ad_ch);

    /* Read result */
    for (i = 0; i < 5; i++) {
        data = csi_adc_read(&adc);
        TEST_CASE_ASSERT(data < 0, "ADC read failed");
        TEST_CASE_TIPS("get adc result: %d\n", data);
    }

    /*uninit adc*/
    ret = csi_adc_stop(&adc);
    TEST_CASE_ASSERT(ret == CSI_OK, "ADC stop failed");

    /*adc get state :stop*/
    ret = csi_adc_get_state(&adc, &state);
    TEST_CASE_ASSERT(ret == CSI_OK, "ADC get state failed");
    TEST_CASE_ASSERT(state.writeable == 1, "ADC get state err");
    TEST_CASE_TIPS("adc stop status,adc state :%d", state.writeable);

    csi_adc_uninit(&adc);

    return 0;
}

#endif