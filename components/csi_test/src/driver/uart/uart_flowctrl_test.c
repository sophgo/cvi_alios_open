
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

int test_uart_asyncSendFlowctrl(void *args)
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
    td.flowctrl          = *((uint64_t *)args + 6);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    ret_state = csi_uart_flowctrl(&hd, td.flowctrl);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config flowctrl fail", td.flowctrl);

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
    transfer_data(send_data, td.uart_transfer_len);

    if (send_data == NULL) {
        csi_uart_uninit(&hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

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


int test_uart_asyncReceiveFlowctrl(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    csi_error_t      ret_state;
    uint8_t receive_data;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);
    td.flowctrl          = *((uint64_t *)args + 6);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    ret_state = csi_uart_flowctrl(&hd, td.flowctrl);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config flowctrl fail", td.flowctrl);

    TEST_CASE_READY();

    volatile uint32_t i;
    char addr;

    for (i = 0; i < td.uart_transfer_len; i++) {
        receive_data = csi_uart_getc(&hd);

        addr = i % 32;

        if (receive_data != addr) {
            TEST_CASE_ASSERT_QUIT(1 == 0,
                                  "uart %d char transfer send not equal "
                                  "receive,receive_data %x, data %x",
                                  td.uart_idx, receive_data, addr);
        }

        mdelay(100);
    }

    csi_uart_uninit(&hd);

    return 0;
}
