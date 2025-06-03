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

#ifndef _I2S_TEST_H_
#define _I2S_TEST_H_
#include <stdint.h>
#include <drv/i2s.h>
#include <drv/dma.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include "test_common.h"

#define I2S_BUF_SIZE      1024    //I2S data buffer
#define I2S_TRANSFER_SIZE 1024   //transfer size for data transfer testing
#define I2S_TRANSFER_PAT  0x5A   //data pattern for transfer test
#define I2S_TX     0
#define I2S_RX     1
#define I2S_DUPLEX 2

typedef struct{
    uint32_t idx;
    csi_i2s_sclk_freq_t sclk_freq;
    csi_i2s_mclk_freq_t mclk_freq;
    csi_i2s_sample_rate_t rate;
    uint32_t period;
    csi_i2s_sample_width_t width;
    csi_i2s_ws_left_polarity_t polarity;
    csi_i2s_protocol_t protocol;
    csi_i2s_mode_t mode;
    uint32_t sound_ch;
    csi_i2s_mclk_freq_t mclk_nfs; 
	uint32_t size;
} test_i2s_args_t;

extern test_i2s_args_t i2s_args;

//extern int test_i2s_config(csi_i2s_t *i2s, test_i2s_args_t *args, csi_i2s_format_t *fmt, uint8_t direction, csi_ringbuf_t *buf, csi_dma_t *dma_handler, csi_dma_ch_t *dma_ch);

extern int test_i2s_interface(void);
//extern int test_i2s_sync_send(test_i2s_args_t *args);
//extern int test_i2s_sync_receive(test_i2s_args_t *args);

extern int test_i2s_async_send(test_i2s_args_t *args);
extern int test_i2s_async_receive(test_i2s_args_t *args);
extern int test_i2s_sync_send(test_i2s_args_t *args);
extern int test_i2s_sync_receive(test_i2s_args_t *args);
extern int test_i2s_dma_send(test_i2s_args_t *args);
extern int test_i2s_dma_receive(test_i2s_args_t *args);
extern int test_i2s_sync_status(char *args);
#endif