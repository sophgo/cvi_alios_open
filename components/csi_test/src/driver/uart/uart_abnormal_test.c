#include <uart_test.h>
#include "timer_test.h"


csi_uart_t       hd;

static void uart_send_callback(csi_uart_t *uart, csi_uart_event_t event, void *arg)
{
    switch (event) {
    case 0:
        *((uint8_t *)arg) = 0;
        break;

    default:
        break;
    }
}

static void uart_Receive_callback(csi_uart_t *uart, csi_uart_event_t event, void *arg)
{
    switch (event) {
    case 1:
        *((uint8_t *)arg) = 0;
        break;

    default:
        break;
    }
}

static void timer_callback(csi_timer_t *timer, void *arg)
{
    switch (*((uint8_t *)arg)) {
    case 0:
        csi_uart_baud(&hd, 9600);
        break;

    case 1:
        csi_uart_format(&hd, 0, 0, 0);
        break;

    case 2:
        csi_uart_flowctrl(&hd, 1);
        break;

    case 3:
        csi_uart_uninit(&hd);
        break;

    case 4:
        csi_uart_detach_callback(&hd);
        break;

    default:
        break;
    }
}

static void timer_callback_fifo(csi_uart_t *uart_hd, void *arg)
{
    tst_mdelay(1000);
}



int test_uart_asyncSendAbnormal(void *args)
{

    test_uart_args_t td;
    int              ret;

    csi_timer_t timer_handle;
    uint32_t callback_config;

    csi_error_t ret_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);
    callback_config      = *((uint64_t *)args + 6);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    volatile uint8_t lock = 0;

    ret_state = csi_uart_attach_callback(&hd, uart_send_callback, (void *)&lock);

    if (ret_state != 0) {
        csi_uart_detach_callback(&hd);
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

    csi_vic_set_prio(DW_TIMER1_IRQn, 3U);

    ret = csi_timer_init(&timer_handle, 1);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_attach_callback(&timer_handle, timer_callback, (void *)&callback_config);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error");

    ret = csi_timer_start(&timer_handle, 20000);

    ret_state = csi_uart_send_async(&hd, send_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d call async send fail",
                              td.uart_idx);
    }

    tst_mdelay(20);

    csi_timer_detach_callback(&timer_handle);
    csi_timer_stop(&timer_handle);
    csi_timer_uninit(&timer_handle);

    while (lock)
        ;

    free(send_data);

    csi_uart_detach_callback(&hd);
    csi_uart_uninit(&hd);

    return 0;
}

int test_uart_asyncReceiveAbnormal(void *args)
{
    test_uart_args_t td;
    int              ret;
    csi_error_t      ret_state;

    csi_timer_t timer_handle;
    uint32_t callback_config;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);
    callback_config      = *((uint64_t *)args + 6);

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

    ret_state = csi_uart_attach_callback(&hd, uart_Receive_callback, (void *)&lock);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(
            1 == 0, "uart %d call attach callback fail", td.uart_idx);
    }

    tst_mdelay(500);

    lock = 1;
    TEST_CASE_READY();

    ret = csi_timer_init(&timer_handle, 1);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_attach_callback(&timer_handle, timer_callback, (void *)&callback_config);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error");

    ret = csi_timer_start(&timer_handle, 100000);

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

    tst_mdelay(1000);

    csi_timer_detach_callback(&timer_handle);
    csi_timer_stop(&timer_handle);
    csi_timer_uninit(&timer_handle);

    tst_mdelay(1000);

    free(send_data);
    free(receive_data);
    csi_uart_detach_callback(&hd);
    csi_uart_uninit(&hd);

    return 0;
}

int test_uart_dmaSendAbnormal(void *args)
{
    test_uart_args_t td;
    int              ret;

    csi_timer_t timer_handle;
    uint32_t callback_config;

    csi_error_t ret_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);
    callback_config      = *((uint64_t *)args + 6);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    volatile uint8_t lock = 0;

    ret_state = csi_uart_attach_callback(&hd, uart_send_callback, (void *)&lock);

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

    csi_dma_ch_t send_ch;
    ret_state = csi_uart_link_dma(&hd, &send_ch, NULL);

    if (ret_state != 0) {
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d link dma channel fail",
                              td.uart_idx);
    }

    TEST_CASE_READY();

    lock      = 1;

    ret = csi_timer_init(&timer_handle, 1);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_attach_callback(&timer_handle, timer_callback, (void *)&callback_config);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error");

    ret = csi_timer_start(&timer_handle, 20000);

    ret_state = csi_uart_send_async(&hd, send_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        csi_uart_detach_callback(&hd);
        csi_uart_link_dma(&hd, NULL, NULL);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d call async send fail",
                              td.uart_idx);
    }

    tst_mdelay(20);

    csi_timer_detach_callback(&timer_handle);
    csi_timer_stop(&timer_handle);
    csi_timer_uninit(&timer_handle);

    while (lock)
        ;

    free(send_data);
    csi_uart_detach_callback(&hd);
    csi_uart_link_dma(&hd, NULL, NULL);

    tst_mdelay(100);
    csi_uart_uninit(&hd);

    return 0;
}

int test_uart_dmaReceiveAbnormal(void *args)
{
    test_uart_args_t td;
    int              ret;
    csi_error_t      ret_state;

    csi_timer_t timer_handle;
    uint32_t callback_config;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);
    callback_config      = *((uint64_t *)args + 6);

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

    ret_state = csi_uart_attach_callback(&hd, uart_Receive_callback, (void *)&lock);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_link_dma(&hd, NULL, NULL);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(
            1 == 0, "uart %d call attach callback fail", td.uart_idx);
    }

    tst_mdelay(500);

    csi_dma_ch_t send_ch;
    ret_state = csi_uart_link_dma(&hd, &send_ch, NULL);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d link dma channel fail",
                              td.uart_idx);
    }

    lock = 1;
    TEST_CASE_READY();

    ret = csi_timer_init(&timer_handle, 1);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_attach_callback(&timer_handle, timer_callback, (void *)&callback_config);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error");

    ret = csi_timer_start(&timer_handle, 100000);

    ret_state = csi_uart_receive_async(&hd, receive_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_detach_callback(&hd);
        csi_uart_link_dma(&hd, NULL, NULL);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT(ret_state == CSI_OK,
                         "uart %d call async receive fail",
                         td.uart_idx);
    }

    tst_mdelay(100);

    csi_timer_detach_callback(&timer_handle);
    csi_timer_stop(&timer_handle);
    csi_timer_uninit(&timer_handle);

    tst_mdelay(1000);

    free(send_data);
    free(receive_data);
    csi_uart_detach_callback(&hd);
    csi_uart_link_dma(&hd, NULL, NULL);

    csi_uart_uninit(&hd);

    return 0;
}



int test_uart_asyncSendFifo(void *args)
{
    test_uart_args_t td;
    int              ret;

    csi_timer_t timer_handle;

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

    ret_state = csi_uart_attach_callback(&hd, uart_send_callback, (void *)&lock);

    if (ret_state != 0) {
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d call attach callback fail",
                              td.uart_idx);
    }

    char *send_data = NULL;
    send_data = (char *)malloc(td.uart_transfer_len);

    if (send_data == NULL) {
        free(send_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    TEST_CASE_READY();

    lock      = 1;

    ret = csi_timer_init(&timer_handle, 1);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_attach_callback(&timer_handle, timer_callback_fifo, NULL);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error");

    ret = csi_timer_start(&timer_handle, 1100000);



    ret_state = csi_uart_send_async(&hd, send_data, td.uart_transfer_len);

    if (ret_state != 0) {
        free(send_data);
        csi_uart_detach_callback(&hd);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK,
                              "uart %d call async send fail",
                              td.uart_idx);
    }

    tst_mdelay(1100);

    csi_timer_detach_callback(&timer_handle);
    csi_timer_stop(&timer_handle);
    csi_timer_uninit(&timer_handle);

    while (lock)
        ;

    free(send_data);
    csi_uart_detach_callback(&hd);

    tst_mdelay(100);
    csi_uart_uninit(&hd);

    return 0;
}



int test_uart_asyncReceiveFifo(void *args)
{
    test_uart_args_t td;
    int              ret;
    csi_error_t      ret_state;

    csi_timer_t timer_handle;

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

    ret_state = csi_uart_attach_callback(&hd, uart_Receive_callback, (void *)&lock);

    if (ret_state != 0) {
        free(send_data);
        free(receive_data);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(
            1 == 0, "uart %d call attach callback fail", td.uart_idx);
    }

    tst_mdelay(600);

    lock = 1;
    TEST_CASE_READY();

    ret = csi_timer_init(&timer_handle, 1);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "timer init error");

    ret = csi_timer_attach_callback(&timer_handle, timer_callback_fifo, NULL);
    TEST_CASE_ASSERT(ret == CSI_OK, "timer attach callback error");

    ret = csi_timer_start(&timer_handle, 1100000);

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

    tst_mdelay(1100);

    csi_timer_detach_callback(&timer_handle);
    csi_timer_stop(&timer_handle);
    csi_timer_uninit(&timer_handle);

    while (lock)
        ;

    if (memcmp(send_data, receive_data, td.uart_transfer_len)) {
        TEST_CASE_ASSERT(1 == 0,
                         "uart %d async transfer send not equal "
                         "receive, soure %x recevie %x",
                         td.uart_idx, source, receive_data);
    }

    free(send_data);
    free(receive_data);
    csi_uart_detach_callback(&hd);
    csi_uart_uninit(&hd);

    return 0;
}