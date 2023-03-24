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

const test_info_t uart_test_funcs_map[] = {
    {"UART_INTERFACE", test_uart_interface, 0},
    {"UART_SYNC_SEND", test_uart_syncSend, 6},
    {"UART_SYNC_RECEIVE", test_uart_syncReceive, 6},
    {"UART_ASYNC_SEND", test_uart_asyncSend, 6},
    {"UART_ASYNC_RECEIVE", test_uart_asyncReceive, 6},
    {"UART_PUT_CHAR", test_uart_putChar, 5},
    {"UART_GET_CHAR", test_uart_getChar, 5},
    {"UART_DMA_SEND", test_uart_dmaSend, 6},
    {"UART_DMA_RECEIVE", test_uart_dmaReceive, 6},
    {"UART_ASYNC_SENDRECEIVE", test_uart_asyncSendReceive, 6},
    {"UART_DMA_SENDRECEIVE", test_uart_dmaSendReceive, 6},
    {"UART_ASYNC_SEND_ALL", test_uart_asyncSendAll, 6},
    {"UART_ASYNC_RECEIVE_ALL", test_uart_asyncReceiveAll, 6},
    {"UART_DMA_SEND_ALL", test_uart_dmaSendAll, 6},
    {"UART_DMA_RECEIVE_ALL", test_uart_dmaReceiveAll, 6},
    {"UART_ASYNC_SEND_ABNORMAL", test_uart_asyncSendAbnormal, 7},
    {"UART_ASYNC_RECEIVE_ABNORMAL", test_uart_asyncReceiveAbnormal, 7},
    {"UART_DMA_SEND_ABNORMAL", test_uart_dmaSendAbnormal, 7},
    {"UART_DMA_RECEIVE_ABNORMAL", test_uart_dmaReceiveAbnormal, 7},
    {"UART_ASYNC_SEND_FIFO", test_uart_asyncSendFifo, 6},
    {"UART_ASYNC_RECEIVE_FIFO", test_uart_asyncReceiveFifo, 6},
    {"UART_ASYNC_SEND_FLOWCTRL", test_uart_asyncSendFlowctrl, 7},
    {"UART_ASYNC_RECEIVE_FLOWCTRL", test_uart_asyncReceiveFlowctrl, 7},

};

int test_uart_main(char *args)
{
    int ret;

    memcpy(source, source_str, 27);
    ret = testcase_jump(args, (void *)uart_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("UART don't supported this command");
    }

    return ret;
}
