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

int test_dma_channelAllocFree(void *args)
{
	csi_dma_ch_t ch_hd;
    csi_error_t ret_sta;
	test_dma_args_t td;


    td.dma_idx = *((uint64_t *)args);
    td.channel_idx = *((uint64_t *)args + 1);
    

    TEST_CASE_TIPS("test DMA idx is %d", td.dma_idx);
    TEST_CASE_TIPS("test channel is %d", td.channel_idx);
    
    

	ret_sta = csi_dma_ch_alloc(&ch_hd, td.channel_idx, td.dma_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d alloc channel %d fail, expected return value is 0, actual return value is %d.", td.dma_idx, td.channel_idx, ret_sta);

	ret_sta = csi_dma_ch_alloc(&ch_hd, td.channel_idx, td.dma_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta != 0, "pwm %d channel %d alloc twice fail, expected return value is 0, actual return value is %d.", td.dma_idx, td.channel_idx, ret_sta);


	csi_dma_ch_free(&ch_hd);

	ret_sta = csi_dma_ch_alloc(&ch_hd, td.channel_idx, td.dma_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "pwm %d alloc channel %d fail, expected return value is 0, actual return value is %d.", td.dma_idx, td.channel_idx, ret_sta);

	csi_dma_ch_free(&ch_hd);

	csi_dma_ch_free(&ch_hd);

	return 0;
}
