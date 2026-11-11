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

#include <iic_test.h>

void iic_callback_func(csi_iic_t *iic, csi_iic_event_t event, void *arg)
{
    ;
}

int test_iic_interface(void *args)
{
    csi_error_t ret_sta;
    csi_iic_t	iic_hd;

    // test csi_iic_init
    ret_sta = csi_iic_init(NULL, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_init expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_init(&iic_hd, 0xFFFFFFFF);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: idx(0xFFFFFFFF) is out of range, csi_iic_init expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_uninit
    csi_iic_uninit(NULL);

    // init iic_hd for testing other parameters
    csi_iic_init(&iic_hd, 0);

    // test csi_iic_mode
    ret_sta = csi_iic_mode(NULL, IIC_MODE_MASTER);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_mode expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_addr_mode
    ret_sta = csi_iic_addr_mode(NULL, IIC_ADDRESS_7BIT);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_addr_mode expects -1 returned, actual %d.",
                     ret_sta);

    // test csi_iic_speed
    ret_sta = csi_iic_speed(NULL, IIC_BUS_SPEED_STANDARD);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_speed expects -1 returned, actual %d.",
                     ret_sta);

    // test csi_iic_own_addr
    ret_sta = csi_iic_own_addr(NULL, 0x0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_own_addr expects -1 returned, actual %d.",
                     ret_sta);

    // test csi_iic_master_send
    const char *data_sta = "123";
    ret_sta = csi_iic_master_send(NULL, 0x0, data_sta, 3, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_master_send expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_master_send(&iic_hd, 0x0, NULL, 3, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: data is NULL, csi_iic_master_send expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_master_send(&iic_hd, 0x0, data_sta, 0, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: size is NULL(0), csi_iic_master_send expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_master_receive
    char data_rece[5];
    ret_sta = csi_iic_master_receive(NULL, 0x0, data_rece, 5, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_master_receive expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_master_receive(&iic_hd, 0x0, NULL, 5, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: data is NULL, csi_iic_master_receive expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_master_receive(&iic_hd, 0x0, data_rece, 0, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: size is NULL(0), csi_iic_master_receive expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_master_send_async
    ret_sta = csi_iic_master_send_async(NULL, 0x0, data_sta, 3);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: iic is NULL, csi_iic_master_send_async expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_master_send_async(&iic_hd, 0x0, NULL, 3);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: data is NULL, csi_iic_master_send_async expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_master_send_async(&iic_hd, 0x0, data_sta, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: size is NULL(0), csi_iic_master_send_async expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_master_receive_async
    ret_sta = csi_iic_master_receive_async(NULL, 0x0, data_rece, 5);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: iic is NULL, csi_iic_master_receive_async expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_master_receive_async(&iic_hd, 0x0, NULL, 5);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: data is NULL, csi_iic_master_receive_async expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_master_receive_async(&iic_hd, 0x0, data_rece, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: size is NULL(0), csi_iic_master_receive_async expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_mem_send
    ret_sta = csi_iic_mem_send(NULL, 0x0, 0x0, IIC_MEM_ADDR_SIZE_8BIT, data_sta, 3, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_mem_send expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_mem_send(&iic_hd, 0x0, 0x0, IIC_MEM_ADDR_SIZE_8BIT, NULL, 3, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: data is NULL, csi_iic_mem_send expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_mem_send(&iic_hd, 0x0, 0x0, IIC_MEM_ADDR_SIZE_8BIT, data_sta, 0, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: size is NULL(0), csi_iic_mem_send expects -1 returned, actual %d.",
                     ret_sta);

    // test csi_iic_mem_receive
    ret_sta = csi_iic_mem_receive(NULL, 0x0, 0x0, IIC_MEM_ADDR_SIZE_8BIT, data_rece, 5, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_mem_receive expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_mem_receive(&iic_hd, 0x0, 0x0, IIC_MEM_ADDR_SIZE_8BIT, NULL, 5, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: data is NULL, csi_iic_mem_receive expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_mem_receive(&iic_hd, 0x0, 0x0, IIC_MEM_ADDR_SIZE_8BIT, data_rece, 0, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: size is NULL(0), csi_iic_mem_receive expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_slave_send
    ret_sta = csi_iic_slave_send(NULL, data_sta, 3, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_slave_send expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_slave_send(&iic_hd, NULL, 3, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: data is NULL, csi_iic_slave_send expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_slave_send(&iic_hd, data_sta, 0, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: size is NULL(0), csi_iic_slave_send expects -1 returned, actual %d.",
                     ret_sta);

    // test csi_iic_slave_receive
    ret_sta = csi_iic_slave_receive(NULL, data_rece, 5, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_slave_receive expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_slave_receive(&iic_hd, NULL, 5, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: data is NULL, csi_iic_slave_receiveexpects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_slave_receive(&iic_hd, data_rece, 0, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: size is NULL(0), csi_iic_slave_receiveexpects -1 returned, actual %d.", ret_sta);

    // test csi_iic_slave_send_async
    ret_sta = csi_iic_slave_send_async(NULL, data_sta, 3);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: iic is NULL, csi_iic_slave_send_async expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_slave_send_async(&iic_hd, NULL, 3);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: data is NULL, csi_iic_slave_send_async expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_slave_send_async(&iic_hd, data_sta, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: szie is NULL(0), csi_iic_slave_send_async expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_slave_receive_async
    ret_sta = csi_iic_slave_receive_async(NULL, data_rece, 5);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: iic is NULL, csi_iic_slave_receive_async expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_slave_receive_async(&iic_hd, NULL, 5);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: data is NULL, csi_iic_slave_receive_async expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_slave_receive_async(&iic_hd, data_rece, 0);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: size is NULL, csi_iic_slave_receive_async expects -1 returned, actual %d.", ret_sta);

    // test csi_iic_attach_callback
    ret_sta = csi_iic_attach_callback(NULL, iic_callback_func, NULL);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR,
                     "parameter: iic is NULL, csi_iic_attach_callback expects -1 returned, actual %d.", ret_sta);

    ret_sta = csi_iic_attach_callback(&iic_hd, NULL, NULL);
    TEST_CASE_ASSERT(ret_sta == CSI_OK,
                     "parameter: callback is NULL, csi_iic_attach_callback expects 0 returned, actual %d.", ret_sta);

    // test csi_iic_detach_callback
    csi_iic_detach_callback(NULL);

    // test csi_iic_xfer_pending
    ret_sta = csi_iic_xfer_pending(NULL, false);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_xfer_pending expects -1 returned, actual %d.",
                     ret_sta);

    // test csi_iic_link_dma
    csi_dma_ch_t rx_dma, tx_dma;
    ret_sta = csi_iic_link_dma(NULL, &tx_dma, &rx_dma);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_link_dma expects -1 returned, actual %d.",
                     ret_sta);

    // test csi_iic_get_state
    csi_state_t state;
    ret_sta = csi_iic_get_state(NULL, &state);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: iic is NULL, csi_iic_get_state expects -1 returned, actual %d.",
                     ret_sta);

    ret_sta = csi_iic_get_state(&iic_hd, NULL);
    TEST_CASE_ASSERT(ret_sta == CSI_ERROR, "parameter: state is NULL, csi_iic_get_state expects -1 returned, actual %d.",
                     ret_sta);

    // uninit iic_hd
    csi_iic_uninit(&iic_hd);

    return 0;
}
