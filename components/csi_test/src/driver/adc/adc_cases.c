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
volatile uint32_t adc_trigger = 0;
//csi_dma_ch_t adc_dma_ch;
uint32_t adc_read_buf[MAX_CHS];

static void adc_callback(csi_adc_t *adc, csi_adc_event_t event, void *arg)
{
    switch (event) {
    case ADC_EVENT_CONVERT_COMPLETE :
        *((uint32_t *)arg) = 0;
        break;

    case ADC_EVENT_ERROR:
        break;

    default:
        break;
    }
}

static int adc_config(csi_adc_t *adc_handler, test_adc_args_t *args, csi_dma_t *dma, csi_dma_ch_t *dma_ch)
{
    int i;

    csi_error_t status;

    status =  csi_adc_continue_mode(adc_handler, (bool)args->continue_mode);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC set continue_mode failed, quit test...");

    uint32_t freq;
    freq =  csi_adc_freq_div(adc_handler, args->freq_div);
    TEST_CASE_ASSERT_QUIT(freq > 0, "ADC set freq failed, quit test...");

    status =  csi_adc_sampling_time(adc_handler, args->s_time);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC set samplling time failed, quit test...");

    //mode 0: sync, 1: async, 2:dma
    if (args->mode == 1 || args->mode == 2) {
        status = csi_adc_set_buffer(adc_handler, adc_read_buf, args->num_chs);
        TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC set buffer failed, quit test...");
        status = csi_adc_attach_callback(adc_handler, adc_callback, (void *)&adc_trigger);
        TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC attach callback failed, quit test...");
    }
#ifdef CSI_ADC_LINK_DMA
    if (args->mode == 2) {

        status = csi_adc_link_dma(adc_handler, dma_ch);
        TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC link DMA failed, quit test...");

        status = csi_adc_set_buffer(adc_handler, adc_read_buf, args->num_chs);
        TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC set buffer failed, quit test...");

    }
#endif
    for (i = 0; i < args->num_chs; i++) {
        status = csi_adc_channel_enable(adc_handler, *(args->ch + i), true);
        TEST_CASE_ASSERT_QUIT(status == CSI_OK, "Enable ADC channel failed, ch_id: %d", *(args->ch + i));
    }

    return 0;
}


//AT+ADC_SYNC
int test_adc_sync(void *args)
{
    uint32_t adc_value;
    test_adc_args_t test_args;
    int i = 0;
    csi_error_t status;
    csi_adc_t adc_handler;
    uint8_t ch_ids[MAX_CHS];

    test_args.mode    		= 0;
    test_args.idx    		= *((uint64_t *)args);
    test_args.s_time 		= *((uint64_t *)args + 1);
    test_args.continue_mode = *((uint64_t *)args + 2);
    test_args.freq_div 		= *((uint64_t *)args + 3);
    test_args.max 			= *((uint64_t *)args + 4);
    test_args.min 			= *((uint64_t *)args + 5);
    ch_ids[0] 				= *((uint64_t *)args + 6);
    test_args.num_chs 		= 1;
    TEST_CASE_TIPS("%d ADC Channels to be enabled", test_args.num_chs);
    test_args.ch 			= ch_ids;

    TEST_CASE_TIPS("test ADC idx is %d", test_args.idx);
    TEST_CASE_TIPS("test ADC sampling_time is %d", test_args.s_time);
    TEST_CASE_TIPS("test ADC continue_mode is %d", test_args.continue_mode);
    TEST_CASE_TIPS("test ADC freq_div is %d", test_args.freq_div);
    TEST_CASE_TIPS("test ADC expected max is %d", test_args.max);
    TEST_CASE_TIPS("test ADC expected min is %d", test_args.min);
    TEST_CASE_TIPS("test ADC channel is %d", ch_ids[0]);

    status = csi_adc_init(&adc_handler, test_args.idx);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC init failed, quit test...");
    adc_config(&adc_handler, &test_args, NULL, NULL);

    TEST_CASE_READY();

    if ((bool)test_args.continue_mode == true) {
        status = csi_adc_start(&adc_handler);
        TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC start failed, quit test...");
    }

    for (i = 0; i < test_args.num_chs; i++) {
        if ((bool)test_args.continue_mode == false) {
            status = csi_adc_start(&adc_handler);
            TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC start failed, quit test...");
        }

        adc_value = csi_adc_read(&adc_handler);
        TEST_CASE_ASSERT(adc_value >= test_args.min
                         && adc_value <= test_args.max, "ADC read value out of expectation[%d, %d], read value:%d, Channel:%d", test_args.min,
                         test_args.max, adc_value, ch_ids[i]);
        TEST_CASE_TIPS("Channel ID:%d, read value: %d", ch_ids[i], adc_value);
    }

    status = csi_adc_stop(&adc_handler);
    TEST_CASE_ASSERT(status == CSI_OK, "ADC stop failed");
    csi_adc_uninit(&adc_handler);

    return 0;
}


//AT+ADC_ASYNC
int test_adc_async(void *args)
{
    int i = 0;
    test_adc_args_t test_args;
    csi_error_t status;
    csi_adc_t adc_handler;
    uint8_t ch_ids[MAX_CHS];

    test_args.mode    		= 1;
    test_args.idx    		= *((uint64_t *)args);
    test_args.s_time 		= *((uint64_t *)args + 1);
    test_args.continue_mode = *((uint64_t *)args + 2);
    test_args.freq_div 		= *((uint64_t *)args + 3);
    test_args.max 			= *((uint64_t *)args + 4);
    test_args.min 			= *((uint64_t *)args + 5);
    ch_ids[0] 				= *((uint64_t *)args + 6);
    test_args.num_chs 		= 1;
    TEST_CASE_TIPS("%d ADC Channels to be enabled", test_args.num_chs);
    test_args.ch 			= ch_ids;

    TEST_CASE_TIPS("test ADC idx is %d", test_args.idx);
    TEST_CASE_TIPS("test ADC sampling_time is %d", test_args.s_time);
    TEST_CASE_TIPS("test ADC continue_mode is %d", test_args.continue_mode);
    TEST_CASE_TIPS("test ADC freq_div is %d", test_args.freq_div);
    TEST_CASE_TIPS("test ADC expected max is %d", test_args.max);
    TEST_CASE_TIPS("test ADC expected min is %d", test_args.min);
    TEST_CASE_TIPS("test ADC channel is %d", ch_ids[0]);

    status = csi_adc_init(&adc_handler, test_args.idx);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC init failed, quit test...");
    adc_config(&adc_handler, &test_args, NULL, NULL);

    adc_trigger = 1;
    status = csi_adc_start_async(&adc_handler);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC start failed, quit test...");

    while (adc_trigger);

    for (i = 0; i < test_args.num_chs; i++) {
        TEST_CASE_ASSERT(adc_read_buf[i] >= test_args.min && adc_read_buf[i] <= test_args.max, \
                         "ADC read value out of expectation[%d, %d], read value:%d, CH: %d", test_args.min, test_args.max, adc_read_buf[i],
                         ch_ids[i]);
        TEST_CASE_TIPS("Channel ID:%d, read value: %d", ch_ids[i], adc_read_buf[i]);
    }

    //stop adc
    status = csi_adc_stop_async(&adc_handler);
    TEST_CASE_ASSERT(status == CSI_OK, "ADC async stop failed");

    csi_adc_detach_callback(&adc_handler);
    csi_adc_uninit(&adc_handler);
    return 0;
}

#ifdef CSI_ADC_LINK_DMA
//AT+ADC_DMA
int test_adc_dma(void *args)
{
    int i = 0;
    test_adc_args_t test_args;
    csi_error_t status;
    csi_adc_t adc_handler;
    csi_dma_t dma;
    csi_dma_ch_t dma_ch;
    uint8_t ch_ids[MAX_CHS];

    test_args.mode    		= 2;
    test_args.idx    		= *((uint64_t *)args);
    test_args.s_time 		= *((uint64_t *)args + 1);
    test_args.continue_mode = *((uint64_t *)args + 2);
    test_args.freq_div 		= *((uint64_t *)args + 3);
    test_args.max 			= *((uint64_t *)args + 4);
    test_args.min 			= *((uint64_t *)args + 5);
    ch_ids[0] 				= *((uint64_t *)args + 6);
    test_args.num_chs 		= 1;
    TEST_CASE_TIPS("%d ADC Channels to be enabled", test_args.num_chs);
    test_args.ch 			= ch_ids;

    TEST_CASE_TIPS("test ADC idx is %d", test_args.idx);
    TEST_CASE_TIPS("test ADC sampling_time is %d", test_args.s_time);
    TEST_CASE_TIPS("test ADC continue_mode is %d", test_args.continue_mode);
    TEST_CASE_TIPS("test ADC freq_div is %d", test_args.freq_div);
    TEST_CASE_TIPS("test ADC expected max is %d", test_args.max);
    TEST_CASE_TIPS("test ADC expected min is %d", test_args.min);
    TEST_CASE_TIPS("test ADC channel is %d", ch_ids[0]);

    status = csi_adc_init(&adc_handler, test_args.idx);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC init failed, quit test...");
    adc_config(&adc_handler, &test_args, &dma, &dma_ch);

    adc_trigger = 1;
    status = csi_adc_start_async(&adc_handler);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "ADC start failed, quit test...");

    while (adc_trigger);

    for (i = 0; i < test_args.num_chs; i++) {
        TEST_CASE_ASSERT(adc_read_buf[i] >= test_args.min && adc_read_buf[i] <= test_args.max, \
                         "ADC read value out of expectation[%d, %d], read value:%d, CH: %d", test_args.min, test_args.max, adc_read_buf[i],
                         ch_ids[i]);
        TEST_CASE_TIPS("Channel ID:%d, read value: %d", ch_ids[i], adc_read_buf[i]);
    }

    //stop adc
    status = csi_adc_stop_async(&adc_handler);
    TEST_CASE_ASSERT(status == CSI_OK, "ADC async stop failed");

    status = csi_adc_link_dma(&adc_handler, NULL);
    TEST_CASE_ASSERT(status == CSI_OK, "un-link DMA CH failed");

    csi_adc_detach_callback(&adc_handler);
    csi_adc_uninit(&adc_handler);
    return 0;
}
#endif