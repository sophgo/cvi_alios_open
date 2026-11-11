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

#include <uart_test.h>

static void uart_callback(csi_uart_t *uart, csi_uart_event_t event, void *arg)
{
    switch (event) {
    case 0:
        *((uint8_t *)arg) = 0;
        break;

    case 1:
        *((uint8_t *)arg) = 0;
        break;

    default:
        break;
    }
}

static void uart_callback_sendReceive(csi_uart_t *uart, csi_uart_event_t event, void *arg)
{
    switch (event) {
    case 0:
        *((uint8_t *)arg) = *((uint8_t *)arg) & 0b10;
        break;

    case 1:
        *((uint8_t *)arg) = *((uint8_t *)arg) & 0b01;
        break;

    default:
        break;
    }
}

int test_uart_dmaSend(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    csi_error_t ret_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    volatile uint8_t lock = 0;

    char *send_data = NULL;
    send_data = (char *)malloc(td.uart_transfer_len);

    if (send_data == NULL) {
        csi_uart_uninit(&hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    ret_state = csi_uart_attach_callback(&hd, uart_callback, (void *)&lock);

    if (ret_state != 0) {
        free(send_data);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d call attach callback fail", td.uart_idx);
    }

    csi_dma_ch_t send_ch;
    ret_state = csi_uart_link_dma(&hd, &send_ch, NULL);

    if (ret_state != 0) {
        free(send_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d link dma channel fail", td.uart_idx);
    }

    TEST_CASE_READY();

    lock      = 1;
    ret_state = csi_uart_send_async(&hd, send_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        csi_uart_detach_callback(&hd);
        csi_uart_link_dma(&hd, NULL, NULL);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d dma send fail", td.uart_idx);
    }

    while (lock)
        ;

    ret_state = csi_uart_link_dma(&hd, NULL, NULL);

    if (ret_state != 0) {
        TEST_CASE_ASSERT(ret_state == CSI_OK,  "uart %d unlink dma channel fail",  td.uart_idx);
    }

    free(send_data);
    csi_uart_detach_callback(&hd);
    csi_uart_uninit(&hd);

    return 0;
}

int test_uart_dmaReceive(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    csi_error_t      ret_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    volatile uint8_t lock = 0;

    char *send_data = NULL;
    char *receive_data = NULL;

    send_data = (char *)malloc(td.uart_transfer_len);
    receive_data = (char *)malloc(td.uart_transfer_len);

    if ((send_data == NULL) || (receive_data == NULL)) {
        free(send_data);
        free(receive_data);
        csi_uart_uninit(&hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    ret_state = csi_uart_attach_callback(&hd, uart_callback, (void *)&lock);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d call attach callback fail", td.uart_idx);
    }

    csi_dma_ch_t rv_ch;
    ret_state = csi_uart_link_dma(&hd, NULL, &rv_ch);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d link dma channel fail", td.uart_idx);
    }

    lock = 1;
    TEST_CASE_READY();

    ret_state = csi_uart_receive_async(&hd, receive_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_link_dma(&hd, NULL, NULL);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d dma receive fail",
                         td.uart_idx);
    }

    while (lock)
        ;

    if (memcmp(send_data, receive_data, td.uart_transfer_len)) {
        TEST_CASE_ASSERT(1 == 0, "uart %d dma transfer send not equal receive", td.uart_idx);
    }

    free(receive_data);
    free(send_data);

    ret_state = csi_uart_link_dma(&hd, NULL, NULL);

    if (ret_state != 0) {
        TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d unlink dma channel fail", td.uart_idx);
    }

    csi_uart_detach_callback(&hd);

    csi_uart_uninit(&hd);

    return 0;
}



int test_uart_dmaSendReceive(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    csi_error_t      ret_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    volatile uint8_t lock = 0;

    char *send_data = NULL;
    char *receive_data = NULL;

    send_data = (char *)malloc(td.uart_transfer_len);
    receive_data = (char *)malloc(td.uart_transfer_len);

    if ((send_data == NULL) || (receive_data == NULL)) {
        free(send_data);
        free(receive_data);
        csi_uart_uninit(&hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    ret_state = csi_uart_attach_callback(&hd, uart_callback_sendReceive, (void *)&lock);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(
            1 == CSI_OK, "uart %d call attach callback fail", td.uart_idx);
    }

    csi_dma_ch_t rv_ch;
    ret_state = csi_uart_link_dma(&hd, NULL, &rv_ch);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d link dma channel fail",
                              td.uart_idx);
    }

    lock = 3;
    TEST_CASE_READY();

    ret_state = csi_uart_receive_async(&hd, receive_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_link_dma(&hd, NULL, NULL);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d dma receive fail",
                         td.uart_idx);
    }

    mdelay(2000);

    ret_state = csi_uart_send_async(&hd, send_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_link_dma(&hd, NULL, NULL);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d dma send fail",
                              td.uart_idx);
    }

    mdelay(1000);

    while (lock)
        ;

    if (memcmp(send_data, receive_data, td.uart_transfer_len)) {
        TEST_CASE_ASSERT(1 == CSI_OK,
                         "uart %d async transfer send not equal "
                         "receive, send %x recevie %x",
                         td.uart_idx, send_data, receive_data);
    }

    free(receive_data);
    free(send_data);

    ret_state = csi_uart_link_dma(&hd, NULL, NULL);

    if (ret_state != 0) {
        TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d unlink dma channel fail", td.uart_idx);
    }

    csi_uart_detach_callback(&hd);

    csi_uart_uninit(&hd);

    return 0;
}
