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

#ifndef __UART_TEST__
#define __UART_TEST__

#include <stdint.h>
#include <drv/uart.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <csi_core.h>

typedef struct {
    uint8_t		uart_idx;
    uint32_t    baudrate;
    uint8_t     data_bits;
    uint8_t     parity;
    uint8_t     stop_bits;
    uint8_t		flowctrl;
    uint32_t    uart_transfer_len;
} test_uart_args_t;



extern  char source[32];
extern const char *source_str;
//extern const char *source;

extern int test_uart_interface(void *args);
extern int test_uart_syncSend(void *args);
extern int test_uart_syncReceive(void *args);
extern int test_uart_asyncSend(void *args);
extern int test_uart_asyncReceive(void *args);
extern int test_uart_putChar(void *args);
extern int test_uart_getChar(void *args);
extern int test_uart_dmaSend(void *args);
extern int test_uart_dmaReceive(void *args);
extern int test_uart_asyncSendReceive(void *args);
extern int test_uart_dmaSendReceive(void *args);

extern int test_uart_asyncSendAll(void *args);
extern int test_uart_asyncReceiveAll(void *args);
extern int test_uart_dmaSendAll(void *args);
extern int test_uart_dmaReceiveAll(void *args);

extern int test_uart_asyncSendAbnormal(void *args);
extern int test_uart_asyncReceiveAbnormal(void *args);
extern int test_uart_dmaSendAbnormal(void *args);
extern int test_uart_dmaReceiveAbnormal(void *args);
extern int test_uart_asyncSendFifo(void *args);
extern int test_uart_asyncReceiveFifo(void *args);
extern int test_uart_asyncSendFlowctrl(void *args);
extern int test_uart_asyncReceiveFlowctrl(void *args);

#endif
