/*
* Copyright (C) 2017 C-SKY Microsystems Co., Ltd. All rights reserved.
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

#include "spi_test.h"

void spi_callback_func(csi_spi_t *spi, csi_spi_event_t event, void *arg)
{
    ;
}

int test_spi_interface(void *args)
{

    char data_buffer[8];
    char *data_out = "12345678";
    int recv_num = 0;
    csi_error_t spi_ret;
    csi_spi_t spi_handler;

    // test csi_spi_init
    spi_ret = csi_spi_init(NULL, 0);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_init expects -1 returned, actual %d", spi_ret);

    spi_ret = csi_spi_init(&spi_handler, 0xFFFFFFFF);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR,
                     "parameter: idx(0xFFFFFFFF) is out of range, csi_spi_init expects -1 returned, actual %d", spi_ret);

    // test csi_spi_uninit
    csi_spi_uninit(NULL);

    // init spi for other parameter test
    csi_spi_init(&spi_handler, 0);

    // test csi_spi_attach_callback
    spi_ret = csi_spi_attach_callback(NULL, spi_callback_func, NULL);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_attach_callback expects -1 returned, actual %d",
                     spi_ret);

    spi_ret = csi_spi_attach_callback(&spi_handler, NULL, NULL);
    TEST_CASE_ASSERT(spi_ret == CSI_OK,
                     "parameter: callback is NULL, csi_spi_attach_callback expects 0 returned, actual %d", spi_ret);

    // test csi_spi_detach_callback
    csi_spi_detach_callback(NULL);

    // test csi_spi_mode
    spi_ret = csi_spi_mode(NULL, SPI_MASTER);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_mode expects -1 returned, actual %d", spi_ret);

    // test csi_spi_cp_format
    spi_ret = csi_spi_cp_format(NULL, SPI_FORMAT_CPOL0_CPHA0);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_cp_format expects -1 returned, actual %d",
                     spi_ret);

    // test csi_spi_frame_len
    spi_ret = csi_spi_frame_len(NULL, SPI_FRAME_LEN_16);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_frame_len expects -1 returned, actual %d",
                     spi_ret);

    // test csi_spi_baud
    spi_ret = csi_spi_baud(NULL, 0x1000000);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_baud expects -1 returned, actual %d", spi_ret);

    spi_ret = csi_spi_baud(&spi_handler, 0);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: baud is NULL(0), csi_spi_baud expects -1 returned, actual %d",
                     spi_ret);

    // test csi_spi_send
    spi_ret = csi_spi_send(NULL, data_buffer, sizeof(data_buffer), 1);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_send expects -1 returned, actual %d", spi_ret);

    spi_ret = csi_spi_send(&spi_handler, NULL, sizeof(data_buffer), 1);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: data is NULL, csi_spi_send expects -1 returned, actual %d", spi_ret);

    spi_ret = csi_spi_send(&spi_handler, data_buffer, 0, 1);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: size is NULL(0), csi_spi_send expects -1 returned, actual %d",
                     spi_ret);

    // test csi_spi_send_async
    spi_ret = csi_spi_send_async(NULL, data_buffer, sizeof(data_buffer));
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_send_async expects -1 returned, actual %d",
                     spi_ret);

    spi_ret = csi_spi_send_async(&spi_handler, NULL, sizeof(data_buffer));
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: data is NULL, csi_spi_send_async expects -1 returned, actual %d",
                     spi_ret);

    spi_ret = csi_spi_send_async(&spi_handler, data_buffer, 0);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: size is NULL(0), csi_spi_send_async expects -1 returned, actual %d",
                     spi_ret);

    // test csi_spi_receive
    recv_num = csi_spi_receive(NULL, data_buffer, sizeof(data_buffer), 1);
    TEST_CASE_ASSERT(recv_num == CSI_ERROR, "parameter: spi is NULL, csi_spi_receive expects -1 returned, actual %d",
                     spi_ret);

    recv_num = csi_spi_receive(&spi_handler, NULL, sizeof(data_buffer), 1);
    TEST_CASE_ASSERT(recv_num == CSI_ERROR, "parameter: data is NULL, csi_spi_receive expects -1 returned, actual %d",
                     spi_ret);

    recv_num = csi_spi_receive(&spi_handler, data_buffer, 0, 1);
    TEST_CASE_ASSERT(recv_num == CSI_ERROR, "parameter: size is NULL(0), csi_spi_receive expects -1 returned, actual %d",
                     spi_ret);

    // test csi_spi_receive_async
    spi_ret = csi_spi_receive_async(NULL, data_buffer, sizeof(data_buffer));
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_receive_async expects -1 returned, actual %d",
                     spi_ret);

    spi_ret = csi_spi_receive_async(&spi_handler, NULL, sizeof(data_buffer));
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: data is NULL, csi_spi_receive_async expects -1 returned, actual %d",
                     spi_ret);

    spi_ret = csi_spi_receive_async(&spi_handler, data_buffer, 0);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR,
                     "parameter: size is NULL(0), csi_spi_receive_async expects -1 returned, actual %d", spi_ret);

    // test csi_spi_send_receive_async
    spi_ret = csi_spi_send_receive_async(NULL, data_out, data_buffer, sizeof(data_buffer));
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR,
                     "parameter: spi is NULL, csi_spi_send_receive_async expects -1 returned, actual %d", spi_ret);

    spi_ret = csi_spi_send_receive_async(&spi_handler, NULL, data_buffer, sizeof(data_buffer));
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR,
                     "parameter: data_out is NULL, csi_spi_send_receive_async expects -1 returned, actual %d", spi_ret);

    spi_ret = csi_spi_send_receive_async(&spi_handler, data_out, NULL, sizeof(data_buffer));
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR,
                     "parameter: data_in is NULL, csi_spi_send_receive_async expects -1 returned, actual %d", spi_ret);

    spi_ret = csi_spi_send_receive_async(&spi_handler, data_out, data_buffer, 0);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR,
                     "parameter: size is NULL(0), csi_spi_send_receive_async expects -1 returned, actual %d", spi_ret);
#ifdef CSI_SPI_LINK_DMA
    // test csi_spi_link_dam
    spi_ret = csi_spi_link_dma(NULL, NULL, NULL);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_link_dma expects -1 returned, actual %d",
                     spi_ret);
#endif

#ifdef CSI_SPI_GET_STATE
    // test csi_spi_get_state
    csi_state_t spi_state;
    spi_ret = csi_spi_get_state(NULL, &spi_state);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: spi is NULL, csi_spi_get_state expects -1 returned, actual %d",
                     spi_ret);

    spi_ret = csi_spi_get_state(&spi_handler, NULL);
    TEST_CASE_ASSERT(spi_ret == CSI_ERROR, "parameter: state is NULL, csi_spi_get_state expects -1 returned, actual %d",
                     spi_ret);
#endif

#ifdef CSI_SPI_SELECT_SLAVE 
    // test csi_spi_select_slave
    csi_spi_select_slave(NULL, 0);
#endif
    csi_spi_uninit(&spi_handler);
    return 0;

}