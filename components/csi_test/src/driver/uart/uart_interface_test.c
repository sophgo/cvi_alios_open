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
#include <stdlib.h>

void uart_callback_func(csi_uart_t *uart, csi_uart_event_t event, void *arg)
{
    ;
}

int test_uart_interface(void *args)
{
    csi_error_t ret_status;
    uint32_t    ret_num;
    uint32_t    ret_char_num;
    uint8_t     ret_char;
    csi_uart_t  uart_hd;

    TEST_CASE_READY();

    /* Call the csi_uart_init function for interface testing */

    ret_status = csi_uart_init(NULL, 1);
    TEST_CASE_ASSERT(
        ret_status == -1,
        "parameter: uart is NULL, csi_uart_init expects -1 returned, actual %d", ret_status);

    ret_status = csi_uart_init(&uart_hd, 0xFFFFFFFF);
    TEST_CASE_ASSERT(
        ret_status == -1,
        "parameter: idx(0xFFFFFFFF) is out of range, csi_uart_init expects -1 returned, actual %d", ret_status);


    /* Call the csi_uart_uninit function for interface testing */

    csi_uart_uninit(NULL);

    /* Call the csi_uart_config function for interface testing */

    ret_status = csi_uart_init(&uart_hd, 1);
    TEST_CASE_ASSERT_QUIT(ret_status == 0, "uart's init fail");

    ret_status = csi_uart_baud(NULL, 115200);
    TEST_CASE_ASSERT(
        ret_status == -1,
        "parameter: uart is NULL, csi_uart_baud expects -1 returned, actual %d", ret_status);

    ret_status = csi_uart_format(NULL, 3, 0, 0);
    TEST_CASE_ASSERT(
        ret_status == -1,
        "parameter: uart is NULL, csi_uart_format expects -1 returned, actual %d", ret_status);

    ret_status = csi_uart_flowctrl(NULL, 0);
    TEST_CASE_ASSERT(
        ret_status == -1,
        "parameter: uart is NULL, csi_uart_flowctrl expects -1 returned, actual %d", ret_status);


    /* Call the csi_uart_transmit function for interface testing */

    char *str = "abc";

    ret_num   = csi_uart_send(NULL, str, 3, 1);
    TEST_CASE_ASSERT(
        ret_num == -1,
        "parameter: uart is NULL, csi_uart_send expects -1 returned, actual %d", ret_status);
    ret_num   = csi_uart_send(&uart_hd, NULL, 3, 1);
    TEST_CASE_ASSERT(
        ret_num == -1,
        "parameter: date is NULL, csi_uart_send expects -1 returned, actual %d", ret_status);
    ret_num   = csi_uart_send(&uart_hd, str, 0, 1);
    TEST_CASE_ASSERT(
        ret_num == -1,
        "parameter: size is 0, csi_uart_send expects -1 returned, actual %d", ret_status);

    ret_status = csi_uart_send_async(NULL, str, 3);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: uart is NULL, csi_uart_send_async expects -1 returned, actual %d", ret_status);
    ret_status = csi_uart_send_async(&uart_hd, NULL, 3);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: data is NULL, csi_uart_send_async expects -1 returned, actual %d", ret_status);

    // assign the function address to uart_hd.send first, or csi_uart_send_async will jump to an unknow address, and trapped
    // detachment will be done at the end
    ret_status = csi_uart_attach_callback(&uart_hd, uart_callback_func, NULL);
    TEST_CASE_ASSERT(ret_status == 0,
                     "csi_uart_attach_callback fail");
    ret_status = csi_uart_send_async(&uart_hd, str, 0);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: size is 0, csi_uart_send_async expects -1 returned, actual %d", ret_status);


    /* Call the csi_uart_receive_intr function for interface testing */

    char rec_data1[10];

    ret_char_num = csi_uart_receive(NULL, rec_data1, 10, 1);
    TEST_CASE_ASSERT(
        ret_char_num == -1,
        "parameter: uart is NULL, csi_uart_receive expects -1 returned, actual %d", ret_char_num);
    ret_char_num = csi_uart_receive(&uart_hd, NULL, 10, 1);
    TEST_CASE_ASSERT(
        ret_char_num == -1,
        "parameter: data is NULL, csi_uart_receive expects -1 returned, actual %d", ret_char_num);
    ret_char_num = csi_uart_receive(&uart_hd, rec_data1, 0, 1);
    TEST_CASE_ASSERT(
        ret_char_num == 0,
        "parameter: size is 0, csi_uart_receive expects 0 returned, actual %d", ret_char_num);


    ret_status = csi_uart_receive_async(NULL, rec_data1, 10);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: uart is NULL, csi_uart_receive_async expects -1 returned, actual %d", ret_status);
    ret_status = csi_uart_receive_async(&uart_hd, rec_data1, 0);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: size is 0, csi_uart_receive_async expects -1 returned, actual %d", ret_status);
    ret_status = csi_uart_receive_async(&uart_hd, NULL, 10);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: data is NULL, csi_uart_receive_async expects -1 returned, actual %d", ret_status);


    /* Call the csi_uart_getchar function for interface testing */

    ret_char = csi_uart_getc(NULL);
    TEST_CASE_ASSERT(
        ret_char == 0,
        "parameter: uart is NULL, csi_uart_getc expects -1 returned, actual %d", ret_char);


    /* Call the csi_uart_putchar function for interface testing */

    csi_uart_putc(NULL, ret_char);


    /* Call the csi_uart_attach_callback function for interface testing */

    ret_status = csi_uart_attach_callback(NULL, uart_callback_func, NULL);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: uart is NULL, csi_uart_attach_callback expects -1 returned, actual %d", ret_status);

    ret_status = csi_uart_attach_callback(&uart_hd, NULL, NULL);
    TEST_CASE_ASSERT(ret_status == 0,
                     "parameter: callback is NULL, csi_uart_attach_callback expects 0 returned, actual %d", ret_status);

    csi_uart_detach_callback(NULL);


    /* Call the csi_uart_attach_callback function for interface testing */

    ret_status = csi_uart_link_dma(NULL, NULL, NULL);
    TEST_CASE_ASSERT(
        ret_status == -1,
        "parameter: uart is NULL, csi_uart_link_dma expects -1 returned, actual %d", ret_status);

    /* Call the csi_uart_attach_callback function for interface testing */

    csi_state_t state;

    ret_status = csi_uart_get_state(NULL, &state);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: uart is NULL, csi_uart_get_state expects -1 returned, actual %d", ret_status);

    ret_status = csi_uart_get_state(&uart_hd, NULL);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: state is NULL, csi_uart_get_state expects -1 returned, actual %d", ret_status);


#ifdef CONFIG_PM
    csi_uart_disable_pm(NULL);

    ret_status = csi_uart_enable_pm(NULL);
    TEST_CASE_ASSERT(
        ret_status == -1,
        "parameter: state is NULL, csi_uart_enable_pm expects -1 returned, actual %d", ret_status);
#endif
    // detach callback of uart_hd
    csi_uart_detach_callback(&uart_hd);
    csi_uart_uninit(&uart_hd);

    return 0;
}
