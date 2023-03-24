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

int test_uart_syncSend(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    uint32_t    ret_num;
    csi_error_t ret_state;
    csi_state_t uart_state;

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

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",
                     td.uart_idx);

    char *send_data = NULL;
    send_data = (char *)malloc(td.uart_transfer_len);

    if (send_data == NULL) {
        csi_uart_uninit(&hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.uart_transfer_len);

    TEST_CASE_READY();
    ret_num = csi_uart_send(&hd, send_data, td.uart_transfer_len, 1000);

    if (ret_num != td.uart_transfer_len) {
        free(send_data);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "uart %d call sync send fail",
                              td.uart_idx);
    }

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",
                     td.uart_idx);

    tst_mdelay(200);
    free(send_data);
    csi_uart_uninit(&hd);
    return 0;
}

int test_uart_syncReceive(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    uint32_t    ret_num;
    csi_error_t ret_state;
    csi_state_t uart_state;

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

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",
                     td.uart_idx);

    TEST_CASE_READY();
    ret_num = csi_uart_receive(&hd, receive_data, td.uart_transfer_len, 10000);
    TEST_CASE_TIPS("actual received str length: %d", ret_num);
    TEST_CASE_TIPS("expected to receive str length: %d", td.uart_transfer_len);

    TEST_CASE_TIPS("received str: $%s$", receive_data);

    TEST_CASE_TIPS("receive_data is %s", receive_data);
    TEST_CASE_TIPS("send_data is %s", send_data);

    if (ret_num != td.uart_transfer_len) {
        free(send_data);
        free(receive_data);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(
            1 == 0,
            "uart %d, actual received length not equal to expected.",
            td.uart_idx);
    }

    if (memcmp(send_data, receive_data, td.uart_transfer_len)) {
        TEST_CASE_ASSERT(1 == 0,
                         "uart %d sync transfer send not equal receive",
                         td.uart_idx);
    }

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",
                     td.uart_idx);

    free(receive_data);
    free(send_data);

    csi_uart_uninit(&hd);
    return 0;
}

int test_uart_syncSend_frequencyConversion(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    uint32_t    frequencyConversion;
    uint32_t    ret_num;
    csi_error_t ret_state;
    csi_state_t uart_state;

    uint32_t str_length = strlen(source);

    td.uart_idx         = *((uint8_t *)args);
    td.baudrate         = *((uint64_t *)args + 1);
    td.data_bits        = *((uint64_t *)args + 2);
    td.parity           = *((uint64_t *)args + 3);
    td.stop_bits        = *((uint64_t *)args + 4);
    frequencyConversion = *((uint64_t *)args + 5);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    if (frequencyConversion == 0) {
        ret_state = csi_uart_baud(&hd, td.baudrate);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

        ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);
    }

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",
                     td.uart_idx);

    TEST_CASE_READY();
    ret_num = csi_uart_send(&hd, source, str_length, 1000);

    if (ret_num != str_length) {
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "uart %d call sync send fail",
                              td.uart_idx);
    }

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",
                     td.uart_idx);

    tst_mdelay(200);
    csi_uart_uninit(&hd);
    return 0;
}

int test_uart_syncReceive_frequencyConversion(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    uint32_t    frequencyConversion;
    uint32_t    ret_num;
    csi_error_t ret_state;
    csi_state_t uart_state;

    td.uart_idx         = *((uint8_t *)args);
    td.baudrate         = *((uint64_t *)args + 1);
    td.data_bits        = *((uint64_t *)args + 2);
    td.parity           = *((uint64_t *)args + 3);
    td.stop_bits        = *((uint64_t *)args + 4);
    frequencyConversion = *((uint64_t *)args + 5);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    if (frequencyConversion == 0) {
        ret_state = csi_uart_baud(&hd, td.baudrate);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

        ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
        TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);
    }

    uint32_t str_length = strlen(source);
    char    *receive_data;
    receive_data = (char *)malloc(str_length + 1);

    if (receive_data == NULL) {
        TEST_CASE_WARN_QUIT("malloc error");
    }

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",
                     td.uart_idx);

    TEST_CASE_READY();
    ret_num = csi_uart_receive(&hd, receive_data, str_length, 10000);
    TEST_CASE_TIPS("actual received str length: %d", ret_num);
    TEST_CASE_TIPS("expected to receive str length: %d", str_length);
    *(receive_data + str_length) = '\0';
    TEST_CASE_TIPS("received str: $%s$", receive_data);

    if (ret_num != str_length) {
        free(receive_data);
        csi_uart_uninit(&hd);
        TEST_CASE_ASSERT_QUIT(
            1 == 0,
            "uart %d, actual received length not equal to expected.",
            td.uart_idx);
    }

    if (memcmp(source, receive_data, str_length)) {
        TEST_CASE_ASSERT(1 == 0,
                         "uart %d sync transfer send not equal receive",
                         td.uart_idx);
    }

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",
                     td.uart_idx);

    free(receive_data);

    csi_uart_uninit(&hd);
    return 0;
}
