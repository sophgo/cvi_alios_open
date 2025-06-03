/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
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
        *((uint32_t *)arg) = *((uint32_t *)arg) & 0b10;
        break;

    case 1:
        *((uint32_t *)arg) = *((uint32_t *)arg) & 0b01;
        break;

    default:
        break;
    }
}

int test_uart_asyncSend(void *args)
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

    ret_state = csi_uart_attach_callback(&hd, uart_callback, (void *)&lock);

    if (ret_state != 0) {
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d call attach callback fail",
                              td.uart_idx);
    }

    char *send_data = NULL;
    send_data = (char *)malloc(td.uart_transfer_len);

    if (send_data == NULL) {
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    TEST_CASE_READY();

    lock      = 1;
    ret_state = csi_uart_send_async(&hd, send_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d call async send fail",
                              td.uart_idx);
    }

    while (lock)
        ;

    free(send_data);
    csi_uart_detach_callback(&hd);

    tst_mdelay(100);
    csi_uart_uninit(&hd);

    return 0;
}

int test_uart_asyncReceive(void *args)
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
        TEST_CASE_ASSERT_QUIT(1 == 0, "uart %d call attach callback fail", td.uart_idx);
    }

    lock = 1;
    TEST_CASE_READY();

    ret_state = csi_uart_receive_async(&hd, receive_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call async receive fail", td.uart_idx);
    }

    while (lock)
        ;

    if (memcmp(send_data, receive_data, td.uart_transfer_len)) {
        TEST_CASE_ASSERT(1 == 0,
                         "uart %d async transfer send not equal "
                         "receive, soure %x recevie %x",
                         td.uart_idx, source, receive_data);
    }

    free(receive_data);
    free(send_data);

    csi_uart_detach_callback(&hd);

    csi_uart_uninit(&hd);

    return 0;
}


int test_uart_asyncSendReceive(void *args)
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

    volatile uint32_t lock = 0;

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

    if (ret_state != CSI_OK) {
        free(send_data);
        free(receive_data);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "uart %d call attach callback fail", td.uart_idx);
    }

    lock = 3;
    TEST_CASE_READY();

    ret_state = csi_uart_receive_async(&hd, receive_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT(ret_state == CSI_OK,
                         "uart %d call async receive fail",
                         td.uart_idx);
    }

    mdelay(2000);

    ret_state = csi_uart_send_async(&hd, send_data, td.uart_transfer_len);

    if (ret_state != CSI_OK) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d call async send fail",
                              td.uart_idx);
    }

    mdelay(1000);

    while (lock)
        ;

    if (memcmp(send_data, receive_data, td.uart_transfer_len)) {
        TEST_CASE_ASSERT(1 == 0,
                         "uart %d async transfer send not equal "
                         "receive, soure %x recevie %x",
                         td.uart_idx, source, receive_data);
    }

    free(receive_data);
    free(send_data);

    csi_uart_detach_callback(&hd);

    csi_uart_uninit(&hd);

    return 0;
}
