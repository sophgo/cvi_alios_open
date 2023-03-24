/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */


#include <uart_test.h>

volatile uint8_t uart_lock[32] = {0};
int lock = 1;

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

int send_complete()
{
    for (int i = 0; i < 32; i++) {
        if (uart_lock[i]) {
            return 2;
        }
    }

    return 0;
}

int test_uart_asyncSendAll(void *args)
{
    test_uart_args_t td;
    csi_error_t ret_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);

    uint8_t idx = td.uart_idx;

    char *send_data = NULL;
    send_data = (char *)malloc(td.uart_transfer_len);

    if (send_data == NULL) {
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);


    csi_uart_t uart_hd[32];

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_init(&uart_hd[i], i);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", i);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_baud(&uart_hd[i], td.baudrate);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", &uart_hd[i]);

            ret_state = csi_uart_format(&uart_hd[i], td.data_bits, td.parity, td.stop_bits);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", &uart_hd[i]);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_attach_callback(&uart_hd[i], uart_callback, (void *)&uart_lock[i]);

            if (ret_state != 0) {
                free(send_data);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d call attach callback fail", i);
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            uart_lock[i] = 1;
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    TEST_CASE_READY();

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_send_async(&uart_hd[i], send_data, td.uart_transfer_len);

            if (ret_state != 0) {
                free(send_data);
                csi_uart_detach_callback(&uart_hd[i]);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart 1 call async send fail");
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    while (send_complete()) {
        ;
    }

    free(send_data);

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            csi_uart_detach_callback(&uart_hd[i]);
            csi_uart_uninit(&uart_hd[i]);

            td.uart_idx = td.uart_idx >> 1;
        } else if (td.uart_idx == 0) {
            break;
        }
    }

    return 0;
}

int test_uart_asyncReceiveAll(void *args)
{
    test_uart_args_t td;
    csi_error_t      ret_state;


    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);

    uint8_t idx = td.uart_idx;

    char *send_data = NULL;
    char *receive_data[32] = {NULL};

    send_data = (char *)malloc(td.uart_transfer_len);

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            receive_data[i] = (char *)malloc(td.uart_transfer_len);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    if (send_data == NULL) {
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            if (receive_data[i] == NULL) {
                free(send_data);
                TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    csi_uart_t uart_hd[32];

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_init(&uart_hd[i], i);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", i);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_baud(&uart_hd[i], td.baudrate);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", &uart_hd[i]);

            ret_state = csi_uart_format(&uart_hd[i], td.data_bits, td.parity, td.stop_bits);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", &uart_hd[i]);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_attach_callback(&uart_hd[i], uart_callback, (void *)&uart_lock[i]);

            if (ret_state != 0) {
                free(send_data);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d call attach callback fail", i);
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            uart_lock[i] = 1;
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    TEST_CASE_READY();

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_receive_async(&uart_hd[i], receive_data[i], td.uart_transfer_len);

            if (ret_state != 0) {
                free(send_data);
                free(receive_data[i]);
                csi_uart_detach_callback(&uart_hd[i]);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart 1 call async send fail");
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    while (send_complete()) {
        ;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            if (memcmp(send_data, receive_data[i], td.uart_transfer_len)) {
                TEST_CASE_ASSERT(1 == 0,
                                 "uart %d async transfer send not equal "
                                 "receive, soure %x recevie %x",
                                 td.uart_idx, send_data, receive_data[i]);
            }

            td.uart_idx = td.uart_idx >> 1;
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }
    }

    free(send_data);

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            free(receive_data[i]);
            csi_uart_detach_callback(&uart_hd[i]);
            csi_uart_uninit(&uart_hd[i]);

            td.uart_idx = td.uart_idx >> 1;
        } else if (td.uart_idx == 0) {
            break;
        }
    }

    return 0;
}


int test_uart_dmaSendAll(void *args)
{
    test_uart_args_t td;
    csi_error_t ret_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);

    uint8_t idx = td.uart_idx;

    char *send_data = NULL;
    send_data = (char *)malloc(td.uart_transfer_len);

    if (send_data == NULL) {
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    csi_uart_t uart_hd[32];

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_init(&uart_hd[i], i);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", i);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_baud(&uart_hd[i], td.baudrate);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", &uart_hd[i]);

            ret_state = csi_uart_format(&uart_hd[i], td.data_bits, td.parity, td.stop_bits);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", &uart_hd[i]);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_attach_callback(&uart_hd[i], uart_callback, (void *)&uart_lock[i]);

            if (ret_state != 0) {
                free(send_data);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d call attach callback fail", i);
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    csi_dma_ch_t uart_rv_ch[32];

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_link_dma(&uart_hd[i], NULL, &uart_rv_ch[i]);

            if (ret_state != 0) {
                free(send_data);
                csi_uart_detach_callback(&uart_hd[i]);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart 1 link dma channel fail");
            }

            td.uart_idx = td.uart_idx >> 1;
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            uart_lock[i] = 1;
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    TEST_CASE_READY();

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_send_async(&uart_hd[i], send_data, td.uart_transfer_len);

            if (ret_state != 0) {
                free(send_data);
                csi_uart_detach_callback(&uart_hd[i]);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart 1 call async send fail");
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    while (send_complete()) {
        ;
    }

    free(send_data);

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_link_dma(&uart_hd[i], NULL, NULL);

            if (ret_state != 0) {
                TEST_CASE_ASSERT(ret_state == CSI_OK, "uart 1 unlink dma channel fail");
            }

            csi_uart_detach_callback(&uart_hd[i]);
            csi_uart_uninit(&uart_hd[i]);

            td.uart_idx = td.uart_idx >> 1;
        } else if (td.uart_idx == 0) {
            break;
        }
    }

    return 0;
}

int test_uart_dmaReceiveAll(void *args)
{
    test_uart_args_t td;
    csi_error_t      ret_state;


    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);
    td.uart_transfer_len = *((uint64_t *)args + 5);

    uint8_t idx = td.uart_idx;

    char *send_data = NULL;
    char *receive_data[32] = {NULL};

    send_data = (char *)malloc(td.uart_transfer_len);

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            receive_data[i] = (char *)malloc(td.uart_transfer_len);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    if (send_data == NULL) {
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            if (receive_data[i] == NULL) {
                free(send_data);
                TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    csi_uart_t uart_hd[32];

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_init(&uart_hd[i], i);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", i);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_baud(&uart_hd[i], td.baudrate);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", &uart_hd[i]);

            ret_state = csi_uart_format(&uart_hd[i], td.data_bits, td.parity, td.stop_bits);
            TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", &uart_hd[i]);
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_attach_callback(&uart_hd[i], uart_callback, (void *)&uart_lock[i]);

            if (ret_state != 0) {
                free(send_data);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d call attach callback fail", i);
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    csi_dma_ch_t uart_rv_ch[32];

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_link_dma(&uart_hd[i], NULL, &uart_rv_ch[i]);

            if (ret_state != 0) {
                free(send_data);
                csi_uart_detach_callback(&uart_hd[i]);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart 1 link dma channel fail");
            }

            td.uart_idx = td.uart_idx >> 1;
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            uart_lock[i] = 1;
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    TEST_CASE_READY();

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            ret_state = csi_uart_receive_async(&uart_hd[i], receive_data[i], td.uart_transfer_len);

            if (ret_state != 0) {
                free(send_data);
                free(receive_data[i]);
                csi_uart_detach_callback(&uart_hd[i]);
                csi_uart_uninit(&uart_hd[i]);
                TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart 1 call async send fail");
            }
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }

        td.uart_idx = td.uart_idx >> 1;
    }

    while (send_complete()) {
        ;
    }

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            if (memcmp(send_data, receive_data[i], td.uart_transfer_len)) {
                TEST_CASE_ASSERT(1 == 0,
                                 "uart %d async transfer send not equal "
                                 "receive, soure %x recevie %x",
                                 td.uart_idx, send_data, receive_data[i]);
            }

            td.uart_idx = td.uart_idx >> 1;
        } else if (td.uart_idx == 0) {
            td.uart_idx = idx;
            break;
        }
    }

    free(send_data);

    for (int i = 0; i < 32; i++) {
        if (td.uart_idx & 0x1) {
            free(receive_data[i]);
            ret_state = csi_uart_link_dma(&uart_hd[i], NULL, NULL);

            if (ret_state != 0) {
                TEST_CASE_ASSERT(ret_state == CSI_OK, "uart 1 unlink dma channel fail");
            }

            csi_uart_detach_callback(&uart_hd[i]);
            csi_uart_uninit(&uart_hd[i]);

            td.uart_idx = td.uart_idx >> 1;
        } else if (td.uart_idx == 0) {
            break;
        }
    }

    return 0;
}

