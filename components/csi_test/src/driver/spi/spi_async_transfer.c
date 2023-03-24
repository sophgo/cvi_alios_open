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

static void test_spi_callback(csi_spi_t *spi_handler, csi_spi_event_t event, void *arg)
{
    switch (event) {
    case SPI_EVENT_SEND_COMPLETE:
        *((uint8_t *)arg) = 0;
        break;

    case SPI_EVENT_RECEIVE_COMPLETE:
        *((uint8_t *)arg) = 0;
        break;

    case SPI_EVENT_SEND_RECEIVE_COMPLETE:
        *((uint8_t *)arg) = 0;
        break;

    default:
        break;
    }
}


int test_spi_async_send(void *args)
{
    int ret;
    volatile uint8_t spi_flag = 0;
    csi_error_t spi_ret;
    csi_spi_t spi_handler;

    spi_args.idx = *((uint64_t *)args);
    spi_args.frame_len = *((uint64_t *)args + 1);
    spi_args.cp_format = *((uint64_t *)args + 2);
    spi_args.baud = *((uint64_t *)args + 3);

    spi_ret = csi_spi_init(&spi_handler, spi_args.idx);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "spi %d init fail", spi_args.idx);

    spi_ret = csi_spi_attach_callback(&spi_handler, test_spi_callback, (void *)&spi_flag);
    ret = spi_test_config(&spi_handler, &spi_args);
    TEST_CASE_TIPS("spi idx:%d", spi_args.idx);
    TEST_CASE_TIPS("spi spi mode:%d", spi_args.spi_mode);
    TEST_CASE_TIPS("spi frame_len:%d", spi_args.frame_len);
    TEST_CASE_TIPS("spi cp_format:%d", spi_args.cp_format);
    TEST_CASE_TIPS("spi baud:%d", spi_args.baud);
    TEST_CASE_ASSERT_QUIT(ret == 0, "spi config failed");

    spi_flag = 1;
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "spi attach callback failed, spi idx: %d", spi_args.idx);
    TEST_CASE_READY();
    spi_ret = csi_spi_send_async(&spi_handler, spi_transfer_data, sizeof(spi_transfer_data));
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "spi send async failed, spi idx: %d", spi_args.idx);

    while (spi_flag); //wait for data transfer

#ifdef CSI_SPI_GET_STATE
    //test csi_spi_get_state
    csi_state_t state;
    spi_ret = csi_spi_get_state(&spi_handler, &state);
    TEST_CASE_ASSERT(spi_ret == CSI_OK, "get spi state failed, expected CSI_OK, returned: %d", spi_ret);
    TEST_CASE_ASSERT(state.writeable == 1, "expected spi state.writeable==1, actual: %u", state.writeable);
#endif

#ifdef CSI_SPI_LINK_DMA
    if (spi_args.link_dma) {
        ret = csi_spi_link_dma(&spi_handler, NULL, NULL);
        TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "un-LINK DMA failed for SPI DMA mode(tx_dma)");
    }
#endif

    csi_spi_detach_callback(&spi_handler);
    csi_spi_uninit(&spi_handler);
    return 0;
}


int test_spi_async_receive(void *args)
{
    int ret;
    csi_error_t spi_ret;
    volatile uint8_t spi_flag = 0;
    int data_len = sizeof(spi_transfer_data);
    char *recv_buffer;
    csi_spi_t spi_handler;

    spi_args.idx = *((uint64_t *)args);
    spi_args.frame_len = *((uint64_t *)args + 1);
    spi_args.cp_format = *((uint64_t *)args + 2);
    spi_args.baud = *((uint64_t *)args + 3);

    spi_ret = csi_spi_init(&spi_handler, spi_args.idx);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "spi %d init fail", spi_args.idx);

    spi_ret = csi_spi_attach_callback(&spi_handler, test_spi_callback, (void *)&spi_flag);

    // configurate
    ret = spi_test_config(&spi_handler, &spi_args);
    TEST_CASE_TIPS("spi idx:%d", spi_args.idx);
    TEST_CASE_TIPS("spi spi mode:%d", spi_args.spi_mode);
    TEST_CASE_TIPS("spi frame_len:%d", spi_args.frame_len);
    TEST_CASE_TIPS("spi cp_format:%d", spi_args.cp_format);
    TEST_CASE_TIPS("spi baud:%d", spi_args.baud);
    TEST_CASE_ASSERT_QUIT(ret == 0, "spi config failed");

    spi_flag = 1;
    TEST_CASE_ASSERT(spi_ret == CSI_OK, "spi attach callback failed, spi idx: %d", spi_args.idx);

    recv_buffer = (char *)malloc(data_len + 1);
    TEST_CASE_ASSERT_QUIT(recv_buffer != NULL, "malloc failed");
    *(recv_buffer + data_len) = '\0';

    TEST_CASE_READY();
    spi_ret = csi_spi_receive_async(&spi_handler, recv_buffer, data_len);
    TEST_CASE_ASSERT(spi_ret == CSI_OK, "spi receive async failed, spi idx: %d", spi_args.idx);

    while (spi_flag);

    TEST_CASE_TIPS("spi sync received str: $%d$", recv_buffer[0]);
    TEST_CASE_TIPS("spi sync transfer str: $%d$", spi_transfer_data[0]);
    TEST_CASE_ASSERT_QUIT(memcmp(recv_buffer, spi_transfer_data, data_len) == 0,
                          "spi async received data not equal to expected");
    free(recv_buffer);

#ifdef CSI_SPI_GET_STATE
    //test csi_spi_get_state
    csi_state_t state;
    spi_ret = csi_spi_get_state(&spi_handler, &state);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "get spi state failed, expected CSI_OK, returned: %d", spi_ret);
    TEST_CASE_ASSERT(state.readable == 1, "expected spi state.readable==1, actual: %u", state.readable);
#endif

    if (spi_args.link_dma) {
        ret = csi_spi_link_dma(&spi_handler, NULL, NULL);
        TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "un-LINK DMA failed for SPI DMA mode(tx_dma)");
    }

    csi_spi_detach_callback(&spi_handler);

    csi_spi_uninit(&spi_handler);
    return 0;

}

int test_spi_dma_send(void *args)
{
    spi_args.link_dma = true;
    spi_args.tx_dma = (csi_dma_ch_t *)malloc(sizeof(csi_dma_ch_t));
    test_spi_async_send(args);
    free(spi_args.tx_dma);
    return 0;
}

int test_spi_dma_receive(void *args)
{
    spi_args.link_dma = true;
    spi_args.rx_dma = (csi_dma_ch_t *)malloc(sizeof(csi_dma_ch_t));
    test_spi_async_receive(args);
    free(spi_args.rx_dma);
    return 0;
}

int test_spi_async_send_receive(void *args)
{
    int ret;
    csi_error_t spi_ret;
    volatile uint8_t spi_flag = 0;
    int data_len = sizeof(spi_transfer_data);
    char *recv_buffer;
    csi_spi_t spi_handler;

    spi_args.idx = *((uint64_t *)args);
    spi_args.frame_len = *((uint64_t *)args + 1);
    spi_args.cp_format = *((uint64_t *)args + 2);
    spi_args.baud = *((uint64_t *)args + 3);

    spi_ret = csi_spi_init(&spi_handler, spi_args.idx);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "spi %d init fail", spi_args.idx);
    spi_ret = csi_spi_attach_callback(&spi_handler, test_spi_callback, (void *)&spi_flag);

    ret = spi_test_config(&spi_handler, &spi_args);
    TEST_CASE_TIPS("spi idx:%d", spi_args.idx);
    TEST_CASE_TIPS("spi spi mode:%d", spi_args.spi_mode);
    TEST_CASE_TIPS("spi frame_len:%d", spi_args.frame_len);
    TEST_CASE_TIPS("spi cp_format:%d", spi_args.cp_format);
    TEST_CASE_TIPS("spi baud:%d", spi_args.baud);
    TEST_CASE_ASSERT_QUIT(ret == 0, "spi config failed");

    spi_flag = 1;
    TEST_CASE_ASSERT(spi_ret == CSI_OK, "spi attach callback failed, spi idx: %d", spi_args.idx);

    recv_buffer = (char *)malloc(data_len + 1);
    TEST_CASE_ASSERT_QUIT(recv_buffer != NULL, "malloc failed");
    *(recv_buffer + data_len) = '\0';

    TEST_CASE_READY();
    spi_ret = csi_spi_send_receive_async(&spi_handler, spi_transfer_data, recv_buffer, data_len);
    TEST_CASE_ASSERT(spi_ret == CSI_OK, "spi receive async failed, spi idx: %d", spi_args.idx);

    while (spi_flag);

    TEST_CASE_ASSERT_QUIT(memcmp(recv_buffer, spi_transfer_data, data_len) == 0,
                          "spi async send received data not equal to expected");
    free(recv_buffer);

#ifdef CSI_SPI_GET_STATE
    //test csi_spi_get_state
    csi_state_t state;
    spi_ret = csi_spi_get_state(&spi_handler, &state);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "get spi state failed, expected CSI_OK, returned: %d", spi_ret);
    TEST_CASE_ASSERT(state.readable == 1, "expected spi state.readable==1, actual: %u", state.readable);
#endif

    if (spi_args.link_dma) {
        ret = csi_spi_link_dma(&spi_handler, NULL, NULL);
        TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "un-LINK DMA failed for SPI DMA mode(tx_dma)");
    }

    csi_spi_detach_callback(&spi_handler);

    csi_spi_uninit(&spi_handler);
    return 0;

}

int test_spi_dma_send_receive(void *args)
{
    spi_args.link_dma = true;
    spi_args.rx_dma = (csi_dma_ch_t *)malloc(sizeof(csi_dma_ch_t));
    spi_args.tx_dma = (csi_dma_ch_t *)malloc(sizeof(csi_dma_ch_t));
    TEST_CASE_TIPS("test_spi_dma_send_receive...");
    test_spi_async_send_receive(args);
    free(spi_args.rx_dma);
    free(spi_args.tx_dma);
    return 0;
}