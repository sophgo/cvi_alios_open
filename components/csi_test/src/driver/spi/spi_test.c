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

#include "spi_test.h"

// parameters
test_spi_args_t spi_args = {.rx_dma = NULL, .tx_dma = NULL, .link_dma = false};

// transfer data
uint16_t spi_transfer_data[SPI_TRANSFER_LEN + 1];


const test_info_t spi_test_funcs_map[] = {
    {"SPI_INTERFACE", test_spi_interface, 0},
    {"SPI_SYNC_SEND_M", test_spi_sync_send, 4},
    {"SPI_SYNC_SEND_S", test_spi_sync_send, 4},
    {"SPI_SYNC_RECEIVE_M", test_spi_sync_receive, 4},
    {"SPI_SYNC_RECEIVE_S", test_spi_sync_receive, 4},
    {"SPI_SYNC_SENDRECEIVE_M", test_spi_sync_send_receive, 4},
    {"SPI_SYNC_SENDRECEIVE_S", test_spi_sync_send_receive, 4},
    {"SPI_ASYNC_SEND_M", test_spi_async_send, 4},
    {"SPI_ASYNC_SEND_S", test_spi_async_send, 4},
    {"SPI_ASYNC_RECEIVE_M", test_spi_async_receive, 4},
    {"SPI_ASYNC_RECEIVE_S", test_spi_async_receive, 4},
    {"SPI_ASYNC_SENDRECEIVE_M", test_spi_async_send_receive, 4},
    {"SPI_ASYNC_SENDRECEIVE_S", test_spi_async_send_receive, 4},
    {"SPI_DMA_SEND_M", test_spi_dma_send, 4},
    {"SPI_DMA_SEND_S", test_spi_dma_send, 4},
    {"SPI_DMA_RECEIVE_M", test_spi_dma_receive, 4},
    {"SPI_DMA_RECEIVE_S", test_spi_dma_receive, 4},
    {"SPI_DMA_SENDRECEIVE_M", test_spi_dma_send_receive, 4},
    {"SPI_DMA_SENDRECEIVE_S", test_spi_dma_send_receive, 4}
};

int test_spi_main(char *args)
{
    int mc_len = strlen((const char *)_mc_name);

    if (!strcmp((const char *)_mc_name + mc_len - 2, "_M")) {
        spi_args.spi_mode = SPI_MASTER;
    } else {
        spi_args.spi_mode = SPI_SLAVE;
    }

    int ret;
    ret = testcase_jump(args, (void *)spi_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("SPI don't supported this command");
    }

    return ret;
}

int spi_test_config(csi_spi_t *spi_handler, test_spi_args_t *test_args)
{
    csi_error_t ret;
    TEST_CASE_TIPS("SPI config startting...");
    ret = csi_spi_mode(spi_handler, test_args->spi_mode);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "spi config mode fail, spi mode: %d", test_args->spi_mode);

    ret = csi_spi_cp_format(spi_handler, test_args->cp_format);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "spi config cp_format((Clock Polarity/Phase)) fail, cp_format: %d",
                          test_args->cp_format);

    ret = csi_spi_frame_len(spi_handler, test_args->frame_len);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "spi config frame length failed, frame_len: %d", test_args->frame_len);
    uint32_t baud_set = 0;

    if (test_args->spi_mode == SPI_MASTER) {
        baud_set = csi_spi_baud(spi_handler, test_args->baud);
        TEST_CASE_ASSERT_QUIT(baud_set > 0, "spi config baud failed, baud: %d", test_args->baud);
    }

    csi_spi_select_slave(spi_handler, 0);

    if (test_args->link_dma) {
        ret = csi_spi_link_dma(spi_handler, test_args->tx_dma, test_args->rx_dma);
        TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "LINK DMA failed for SPI DMA mode(tx_dma)");
    }

    TEST_CASE_TIPS("SPI config finished.");
    return 0;
}