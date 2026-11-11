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

#include "adc_test.h"

static void adc_callback(csi_adc_t *adc, csi_adc_event_t event, void *arg)
{

}

int test_adc_interface(void *args)
{
    csi_error_t ret;
    csi_adc_t adc;
    int32_t idx = 0;
    uint32_t ret_num = 0;
    csi_state_t state;
    uint32_t adc_read_buf[MAX_CHS];

    ret = csi_adc_init(NULL, idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_ERROR, "csi_adc_init should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_init(&adc, 0xffff);
    TEST_CASE_ASSERT_QUIT(ret == CSI_ERROR, "csi_adc_init should return CSI_ERROR, but returned:%d", ret);

    csi_adc_uninit(NULL);

    ret = csi_adc_set_buffer(NULL, adc_read_buf, 1);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_set_buffer should return CSI_ERROR, but returned:%d", ret);

    csi_adc_init(&adc, 0);

    ret = csi_adc_set_buffer(&adc, NULL, 1);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_set_buffer should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_set_buffer(&adc, adc_read_buf, 0);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_set_buffer should return CSI_ERROR, but returned:%d", ret);

    csi_adc_uninit(&adc);

    ret = csi_adc_start(NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_start should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_start_async(NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_start_async should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_stop(NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_stop should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_stop_async(NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_stop_async should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_channel_enable(NULL, 0, true);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_channel_enable should return CSI_ERROR, but returned:%d", ret);
    
#ifdef CSI_ADC_CHANNEL_SAMPLING_TIME
    ret = csi_adc_channel_sampling_time(NULL, 0, 1);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_channel_sampling_time should return CSI_ERROR, but returned:%d", ret);
#endif

    ret = csi_adc_sampling_time(NULL, 1);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_sampling_time should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_continue_mode(NULL, true);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_continue_mode should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_freq_div(NULL, 0);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_freq_div should return CSI_ERROR, but returned:%d", ret);

    ret_num = csi_adc_read(NULL);
    TEST_CASE_ASSERT(ret_num == CSI_ERROR, "csi_adc_read should return 0, but returned:%d", ret_num);

    ret = csi_adc_get_state(NULL, &state);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_get_state should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_attach_callback(NULL, adc_callback, NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_attach_callback should return CSI_ERROR, but returned:%d", ret);

    ret = csi_adc_attach_callback(&adc, NULL, NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_attach_callback should return CSI_ERROR, but returned:%d", ret);

    csi_adc_detach_callback(NULL);

#ifdef CSI_ADC_LINK_DMA
    ret = csi_adc_link_dma(NULL, NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi_adc_link_dma should return CSI_ERROR, but returned:%d", ret);
#endif
    return 0;
}