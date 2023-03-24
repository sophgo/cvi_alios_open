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

static uint8_t data = 'a';

int test_uart_putChar(void *args)
{
    csi_uart_t       hd;
    test_uart_args_t td;
    csi_error_t      ret_state;
    csi_state_t      uart_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",  td.uart_idx);

    TEST_CASE_READY();

    csi_uart_putc(&hd, data);

    tst_mdelay(100);
    csi_uart_uninit(&hd);
    return 0;
}

int test_uart_getChar(void *args)
{
    uint8_t receive_data;

    csi_uart_t       hd;
    test_uart_args_t td;
    csi_error_t      ret_state;
    csi_state_t      uart_state;

    td.uart_idx          = *((uint64_t *)args);
    td.baudrate          = *((uint64_t *)args + 1);
    td.data_bits         = *((uint64_t *)args + 2);
    td.parity            = *((uint64_t *)args + 3);
    td.stop_bits         = *((uint64_t *)args + 4);

    ret_state = csi_uart_init(&hd, td.uart_idx);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d init fail", td.uart_idx);

    ret_state = csi_uart_baud(&hd, td.baudrate);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config baudrate fail", td.uart_idx);

    ret_state = csi_uart_format(&hd, td.data_bits, td.parity, td.stop_bits);
    TEST_CASE_ASSERT_QUIT(ret_state == CSI_OK, "uart %d config format fail", td.uart_idx);

    ret_state = csi_uart_get_state(&hd, &uart_state);
    TEST_CASE_ASSERT(ret_state == CSI_OK, "uart %d call get state fail",  td.uart_idx);

    TEST_CASE_READY();

    receive_data = csi_uart_getc(&hd);

    if (receive_data != data) {
        TEST_CASE_ASSERT_QUIT(1 == 0,
                              "uart %d char transfer send not equal "
                              "receive,receive_data %x, data %x",
                              td.uart_idx, receive_data, data);
    }

    csi_uart_uninit(&hd);
    return 0;
}
