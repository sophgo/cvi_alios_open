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


const test_info_t dma_test_funcs_map[] = {
    {"DMA_MEMORY_TO_MEMORY", test_dma_memoryToMemory, 7},
    {"DMA_CHANNEL_ALLOC_FREE", test_dma_channelAllocFree, 2},
    {"DMA_HALF_DONE_EVENT", test_dma_halfDoneEvent, 3},
	{"DMA_INTERFACE", test_dma_interface, 0},
	{NULL, NULL, 0}
};

int test_dma_main(char *args)
{
	int ret;
	ret = testcase_jump(args, (void *)dma_test_funcs_map);
	
	if (ret != 0) {
		TEST_CASE_TIPS("DMA don't supported this command");
	}
	
	return ret;
}

