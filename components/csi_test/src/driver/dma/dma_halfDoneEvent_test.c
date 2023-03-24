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

#include <dma_test.h>

static volatile uint8_t lock1 = 0, lock2 = 0;
static csi_dma_ch_t ch_hd;

static void dma_ch_callback(csi_dma_ch_t *dma_ch, csi_dma_event_t event, void *arg)
{
    if (event == DMA_EVENT_TRANSFER_HALF_DONE) {
		lock1 = 0;
	}
	
	if (event == DMA_EVENT_TRANSFER_DONE) {
        lock2 = 0;
    }
}


static int data_init(void *addr_src, void *addr_dest, uint32_t data_length)
{
	uint32_t i;

	for (i=0; i<data_length; i++) {
		*((uint8_t *)addr_src+i) = i;
		*((uint8_t *)addr_dest+i) = 0;
	}

	return 0;
}


int test_dma_halfDoneEvent(void *args)
{
    csi_error_t ret_sta;
    void *src, *dest;
    int ret;
    test_dma_args_t td;


    td.dma_idx = *((uint64_t *)args);
    td.channel_idx = *((uint64_t *)args + 1);
    td.transfer_length = *((uint64_t *)args + 2);
    

    TEST_CASE_TIPS("test DMA idx is %d", td.dma_idx);
    TEST_CASE_TIPS("test channel is %d", td.channel_idx);
    TEST_CASE_TIPS("test transfer length is %d", td.transfer_length);

    src = (void *)malloc(td.transfer_length);
    dest = (void *)malloc(td.transfer_length);
    if ((src == NULL) || (dest == NULL)) {
        free(src);
        free(dest);
        TEST_CASE_WARN_QUIT("malloc memory error");
    }

    ret = data_init(src, dest, td.transfer_length);
    if (ret != 0) {
        free(src);
        free(dest);
        TEST_CASE_WARN_QUIT("data init error");
    }   

    ret_sta = csi_dma_ch_alloc(&ch_hd, td.channel_idx, td.dma_idx);
    if (ret_sta != 0) {
        free(src);
        free(dest);
        TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d alloc channel %d fail, expected return value is 0, actual return value is %d.", td.dma_idx, td.channel_idx, ret_sta);
    }

    csi_dma_ch_config_t config;
    config.src_inc = DMA_ADDR_INC;
    config.dst_inc = DMA_ADDR_INC;
    config.src_tw = DMA_DATA_WIDTH_8_BITS;
    config.dst_tw = DMA_DATA_WIDTH_8_BITS;
    config.trans_dir = DMA_MEM2MEM;
    config.group_len = 8;	
	config.half_int_en  = 1;

	ret_sta = csi_dma_ch_config(&ch_hd, &config);
    if (ret_sta != 0) {
        free(src);
        free(dest);
        csi_dma_ch_free(&ch_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d channel %d config fail, expected return value is 0, actual return value is %d.", td.dma_idx, td.channel_idx, ret_sta);
    }


	ret_sta = csi_dma_ch_attach_callback(&ch_hd, dma_ch_callback, NULL);
    if (ret_sta != 0) {
        free(src);
        free(dest);
        csi_dma_ch_free(&ch_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "pwm %d channel %d attach callback fail, expected return value is 0, actual return value is %d.", td.dma_idx, td.channel_idx, ret_sta);
    }

	lock1 = 1;
	lock2 = 1;

	csi_dma_ch_start(&ch_hd, src, dest, td.transfer_length);

	TEST_CASE_TIPS("If the program does not run down, DMA_EVENT_TRANSFER_HALF_DONE interrupt isn't triggered");
	while (lock1) ;


	uint32_t r_counter = 0, i = 0;
	for (i=0; i<td.transfer_length; i++) {
		if ((*(uint8_t *)src) == (*(uint8_t *)dest))
			r_counter++;
	}

    while (lock2) {
        ;
    }

	csi_dma_ch_stop(&ch_hd);


	csi_dma_ch_detach_callback(&ch_hd);

    csi_dma_ch_free(&ch_hd);

	free(src);
	free(dest);
	return 0;
}
