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

#ifndef __DMA_TEST__
#define __DMA_TEST__

#include <stdint.h>
#include <drv/dma.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>

typedef struct {
    uint8_t     dma_idx;
    uint8_t     channel_idx;
    uint8_t     data_width;
    uint8_t     src_addr_inc;
    uint8_t     dest_addr_inc;
    uint8_t     group_length;
    uint32_t    transfer_length;
} test_dma_args_t;



extern int test_dma_interface(void *args);
extern int test_dma_memoryToMemory(void *args);
extern int test_dma_channelAllocFree(void *args);
extern int test_dma_halfDoneEvent(void *args);

#endif
