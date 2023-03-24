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


#include "i2s_test.h"
static volatile uint8_t i2s_lock;
static uint8_t data_in[I2S_TRANSFER_SIZE];
static uint8_t expect[I2S_TRANSFER_SIZE];
static uint8_t data_out[I2S_TRANSFER_SIZE];

static void i2s_callback_async(csi_i2s_t *i2s, csi_i2s_event_t event, void *arg)
{
    uint8_t tmp;
    tmp = *((uint8_t *)arg);

    switch (event) {
        case I2S_EVENT_SEND_COMPLETE:
            tmp--;
            *((uint8_t *)arg) = tmp;
            break;

        case I2S_EVENT_RECEIVE_COMPLETE:
            tmp--;
            *((uint8_t *)arg) = tmp;
            break;

        default:
            break;
    }

}

int test_i2s_dma_send(test_i2s_args_t *args)
{
    csi_error_t status;
    uint32_t sent_num = 0;
    csi_i2s_t i2s;
    csi_i2s_format_t fmt;
    uint8_t data_buf[1024];
    ringbuffer_t i2s_buf;
    csi_dma_ch_t dma_ch;

    i2s_buf.buffer = data_buf;
    i2s_buf.size = 1024;
	
	i2s_args.idx = *((uint64_t *)args);
    fmt.mode = i2s_args.mode;
    fmt.rate = *((uint64_t *)args + 1);
    fmt.width = *((uint64_t *)args+ 2);
	fmt.protocol = *((uint64_t *)args+ 3);
    fmt.polarity = *((uint64_t *)args+ 4);
    fmt.sclk_nfs = *((uint64_t *)args+ 5);
    fmt.mclk_nfs = *((uint64_t *)args+ 6);
	i2s_args.size= *((uint64_t *)args+ 7);
	
	status =  csi_i2s_init(&i2s, i2s_args.idx);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "i2s init failed");

    status = csi_i2s_attach_callback(&i2s, i2s_callback_async, (void *)&i2s_lock);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "Attach callback failed");

    status = csi_i2s_format(&i2s, &fmt);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "config I2S format failed.");

    status = csi_i2s_tx_link_dma(&i2s, &dma_ch);
    TEST_CASE_ASSERT(status == CSI_OK, "link tx DMA failed");

    csi_i2s_tx_set_buffer(&i2s, &i2s_buf);
    csi_i2s_tx_buffer_reset(&i2s);

    status = csi_i2s_tx_set_period(&i2s, 512); //args->period);
    TEST_CASE_ASSERT(status == CSI_OK, "set I2S tx period failed");

    dataset((char *)data_out, i2s_args.size, 0x5A);

    status = csi_i2s_send_start(&i2s);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "i2s dma send start failed");

    TEST_CASE_READY();
    sent_num = csi_i2s_send(&i2s, data_out, i2s_args.size);
    TEST_CASE_ASSERT(sent_num == i2s_args.size, "i2s dma send data failed");

    csi_i2s_send_stop(&i2s);
    csi_i2s_enable(&i2s, false);

    csi_i2s_tx_link_dma(&i2s, NULL);
    csi_i2s_detach_callback(&i2s);
    csi_i2s_uninit(&i2s);
    return 0;

}


int test_i2s_dma_receive(test_i2s_args_t *args)
{
    csi_error_t status;
    uint32_t recv_num = 0;
    csi_i2s_t i2s;
    csi_i2s_format_t fmt;
    uint8_t data_buf[1024];
    ringbuffer_t i2s_buf;
    csi_dma_ch_t dma_ch;

    i2s_buf.buffer = data_buf;
    i2s_buf.size = 1024;
    
	i2s_args.idx = *((uint64_t *)args);
    fmt.mode = i2s_args.mode;
    fmt.rate = *((uint64_t *)args + 1);
    fmt.width = *((uint64_t *)args+ 2);
	fmt.protocol = *((uint64_t *)args+ 3);
    fmt.polarity = *((uint64_t *)args+ 4);
    fmt.sclk_nfs = *((uint64_t *)args+ 5);
    fmt.mclk_nfs = *((uint64_t *)args+ 6);
	i2s_args.size= *((uint64_t *)args+ 7);
	
	status =  csi_i2s_init(&i2s, i2s_args.idx);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "i2s init failed");

    status = csi_i2s_attach_callback(&i2s, i2s_callback_async, (void *)&i2s_lock);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "Attach callback failed");

    status = csi_i2s_format(&i2s, &fmt);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "config I2S format failed.");

    status = csi_i2s_rx_link_dma(&i2s, &dma_ch);
    TEST_CASE_ASSERT(status == CSI_OK, "link rx DMA failed");

    TEST_CASE_ASSERT(status == CSI_OK, "set I2S rx period failed");
    csi_i2s_rx_set_buffer(&i2s, &i2s_buf);
    csi_i2s_rx_buffer_reset(&i2s);
    status = csi_i2s_rx_set_period(&i2s, 512); // args->period);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "i2s set period failed");
    i2s_lock = i2s_buf.size / i2s.rx_period;

    status = csi_i2s_receive_start(&i2s);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "i2s receive start failed.");

    memset(data_in, 0, i2s_args.size);
    dataset((char *)expect, i2s_args.size, 0x5A);

    TEST_CASE_READY();
    recv_num = csi_i2s_receive(&i2s, data_in, i2s_args.size);

    int i = 0;
    uint32_t expect_cnt = 0;

    for (i = 0; i < I2S_TRANSFER_SIZE; i++) {
        //atserver_send("%2x", data_in[i]);
        if (data_in[i] == 0x5a) {
            expect_cnt++;
        }
    }

    if (args->protocol < 2) {
        TEST_CASE_TIPS("expect data count: %d", expect_cnt);
        TEST_CASE_ASSERT(expect_cnt > (i2s_args.size - 800), "recived data has something wrong");

        TEST_CASE_TIPS("received data: %2x", expect[1]);
        TEST_CASE_ASSERT(recv_num == i2s_args.size, "received size: %d, expected size: %d", recv_num, i2s_args.size);
    } else {
        TEST_CASE_TIPS("expect data count: %d", expect_cnt);
        TEST_CASE_ASSERT(expect_cnt > ((i2s_args.size - 800) / 2), "recived data has something wrong");

        TEST_CASE_TIPS("received data: %2x", expect[1]);
        TEST_CASE_ASSERT(recv_num == i2s_args.size, "received size: %d, expected size: %d", recv_num, i2s_args.size);
    }


    csi_i2s_receive_stop(&i2s);

    csi_i2s_enable(&i2s, false);
    csi_i2s_rx_link_dma(&i2s, NULL);
    csi_i2s_detach_callback(&i2s);
    csi_i2s_uninit(&i2s);

    return 0;
}