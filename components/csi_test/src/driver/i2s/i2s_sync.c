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

int test_i2s_sync_send(test_i2s_args_t *args)
{
    csi_error_t status;
    uint32_t sent_num = 0;
    csi_i2s_t i2s;
    csi_i2s_format_t fmt;

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

    status = csi_i2s_format(&i2s, &fmt);
    csi_i2s_enable(&i2s, 1);
    TEST_CASE_ASSERT_QUIT(status == CSI_OK, "config I2S format failed.");
	
	uint8_t data_out[i2s_args.size];
    dataset((char *)data_out, i2s_args.size, 0x5A);

    TEST_CASE_READY();
    sent_num = csi_i2s_send(&i2s, data_out, i2s_args.size);
    TEST_CASE_ASSERT(sent_num == i2s_args.size, "I2S send data failed, expected send %d, actual send %d", i2s_args.size, sent_num);
    csi_i2s_enable(&i2s, 0);
    csi_i2s_uninit(&i2s);
    return 0;
}

int test_i2s_sync_receive(test_i2s_args_t *args)
{
    csi_error_t status;
    uint32_t recv_num = 0;
    csi_i2s_t i2s;
    csi_i2s_format_t fmt;

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

    status = csi_i2s_format(&i2s, &fmt);
    csi_i2s_enable(&i2s, 1);

	uint8_t data_in[i2s_args.size];
	uint8_t expect[i2s_args.size];
    memset(data_in, 0, i2s_args.size);

    uint32_t expect_cnt = 0;
    TEST_CASE_READY();
    recv_num = csi_i2s_receive(&i2s, data_in, i2s_args.size);
    TEST_CASE_ASSERT(recv_num == i2s_args.size, "received:%d, expected: %d", recv_num,  i2s_args.size);

    for (uint32_t i = 0; i < i2s_args.size; i++) {
        if (data_in[i] == 0x5a) {
            expect_cnt++;
        }
    }

    if (args->protocol < 2) {
        TEST_CASE_ASSERT(expect_cnt > (i2s_args.size - 800), "recived data has something wrong");
    } else {
        TEST_CASE_ASSERT(expect_cnt > ((i2s_args.size - 800) / 2), "recived data has something wrong");
    }


    csi_i2s_enable(&i2s, 0);
    csi_i2s_uninit(&i2s);
    return 0;

}