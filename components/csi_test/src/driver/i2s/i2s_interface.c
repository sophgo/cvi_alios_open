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

int test_i2s_interface(void){
    csi_error_t st;
    int ret= 0 ;
    csi_i2s_t i2s;
    csi_i2s_format_t fmt;

    st = csi_i2s_init(NULL, 0);
    TEST_CASE_ASSERT_QUIT(st ==CSI_ERROR,"I2S init expect return CSI_ERROR, returned:%d", st);

    st = csi_i2s_init(&i2s, 0);
    TEST_CASE_ASSERT_QUIT(st ==CSI_OK,"I2S init failed");

    csi_i2s_enable(NULL, true);

    st = csi_i2s_format(NULL, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"I2S format expect return CSI_ERROR, returned:%d", st);

    st = csi_i2s_format(&i2s, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"I2S format expect return CSI_ERROR, returned:%d", st);

    st = csi_i2s_format(&i2s, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"I2S format expect return CSI_ERROR, returned:%d", st);

    st = csi_i2s_format(NULL, &fmt);
    TEST_CASE_ASSERT(st == CSI_ERROR,"I2S format expect return CSI_ERROR, returned:%d", st);

    st = csi_i2s_rx_select_sound_channel(NULL,I2S_LEFT_RIGHT_CHANNEL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"rx select mono expected CSI_ERROR, returned : %d", st);

    st = csi_i2s_tx_select_sound_channel(NULL,I2S_LEFT_RIGHT_CHANNEL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"tx select mono expected CSI_ERROR, returned : %d", st);    

    st = csi_i2s_rx_link_dma(&i2s, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"rx_link_dma expected CSI_ERROR, returned : %d", st);    

    st = csi_i2s_rx_link_dma(NULL, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"rx_link_dma expected CSI_ERROR, returned : %d", st);    

    st = csi_i2s_tx_link_dma(&i2s, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"rx_link_dma expected CSI_ERROR, returned : %d", st);    

    st = csi_i2s_tx_link_dma(NULL, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"rx_link_dma expected CSI_ERROR, returned : %d", st);  

    csi_i2s_rx_set_buffer(NULL, NULL);
    csi_i2s_tx_set_buffer(NULL, NULL);
    TEST_CASE_TIPS("rx, tx set buffer tested");

    ret = csi_i2s_tx_set_period(&i2s, 0);
    TEST_CASE_ASSERT(ret == -1, "csi_i2s_tx_set_period expected -1, returned: %d", ret);

    ret = csi_i2s_rx_buffer_avail(NULL);
    TEST_CASE_ASSERT(ret == 0, "csi_i2s_rx_buf_avail expected 0, returned: %d", ret);

    ret = csi_i2s_tx_buffer_avail(NULL);
    TEST_CASE_ASSERT(ret == 0, "csi_i2s_tx_buf_avail expected 0, returned: %d", ret);

    st = csi_i2s_tx_buffer_reset(NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"csi_i2s_tx_buf_reset expected CSI_ERROR, returned : %d", st);

    st = csi_i2s_rx_buffer_reset(NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR,"csi_i2s_rx_buf_reset expected CSI_ERROR, returned : %d", st);  

    ret =  csi_i2s_send(NULL, NULL, 0);
    TEST_CASE_ASSERT(ret == -1, "csi_i2s_send expected -1, returned:%d", ret);

    ret = csi_i2s_receive(NULL,  NULL, 0);
    TEST_CASE_ASSERT(ret == -1, "csi_i2s_receive expected -1, returned:%d", ret);

    ret =  csi_i2s_send_async(NULL, NULL, 0);
    TEST_CASE_ASSERT(ret == 0, "csi_i2s_send_async expected 0, returned:%d", ret);

    ret = csi_i2s_receive_async(NULL,  NULL, 0);
    TEST_CASE_ASSERT(ret == 0, "csi_i2s_receive_async expected 0, returned:%d", ret);

    st = csi_i2s_send_pause(NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR, "csi_i2s_send_pause expected CSI_ERROR, returned:%d", st);

    st = csi_i2s_send_resume(NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR, "csi_i2s_send_resume expected CSI_ERROR, returned:%d", st);

    st = csi_i2s_send_start(NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR, "csi_i2s_send_start expected CSI_ERROR, returned:%d", st);

    st = csi_i2s_receive_start(NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR, "csi_i2s_receive_start expected CSI_ERROR, returned:%d", st);

    csi_i2s_send_stop(NULL);
    csi_i2s_receive_stop(NULL);

    st = csi_i2s_attach_callback(NULL, NULL, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR, "csi_i2s_attach_callback expected CSI_ERROR, returned:%d", st);

    csi_i2s_detach_callback(NULL);
    st = csi_i2s_get_state(NULL, NULL);
    TEST_CASE_ASSERT(st == CSI_ERROR, "csi_i2s_get_state expected CSI_ERROR, returned:%d", st);
    return 0;
}