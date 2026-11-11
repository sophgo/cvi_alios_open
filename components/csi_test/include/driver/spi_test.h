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

#ifndef _SPI_TEST_H_
#define _SPI_TEST_H_
#include <stdint.h>
#include <drv/spi.h>
#include <drv/dma.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include "test_common.h"
#define SPI_TRANSFER_LEN 1024

//Test arguments from AT test commands
typedef struct {
    uint32_t idx;                    // device idx
    csi_spi_mode_t spi_mode;         // master / slave
    csi_spi_frame_len_t frame_len;   // data frame length
    csi_spi_cp_format_t cp_format;   // clock Polarity / Phase
    uint32_t baud;                   // spi work frequency
    bool link_dma;               	 // true if dma to be link
    csi_dma_ch_t *rx_dma;			 // rx channel
    csi_dma_ch_t *tx_dma;			 // tx channel
} test_spi_args_t;


int spi_test_config(csi_spi_t *spi_handler, test_spi_args_t *test_args);

int test_spi_interface(void *args);

int test_spi_sync_send(void *args);
int test_spi_sync_receive(void *args);
int test_spi_sync_send_receive(void *args);

int test_spi_async_send(void *args);
int test_spi_async_receive(void *args);
int test_spi_async_send_receive(void *args);

int test_spi_dma_send(void *args);
int test_spi_dma_receive(void *args);
int test_spi_dma_send_receive(void *args);

extern uint16_t spi_transfer_data[SPI_TRANSFER_LEN + 1];
extern test_spi_args_t spi_args;

#endif
