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


int test_spi_sync_send(void *args)
{
    int ret;
    csi_error_t spi_ret;
    csi_spi_t spi_handler;
    uint32_t send_num = 0;

    spi_args.idx = *((uint64_t *)args);
    spi_args.frame_len = *((uint64_t *)args + 1);
    spi_args.cp_format = *((uint64_t *)args + 2);
    spi_args.baud = *((uint64_t *)args + 3);

    spi_ret = csi_spi_init(&spi_handler, spi_args.idx);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "spi %d init fail", spi_args.idx);

    ret = spi_test_config(&spi_handler, &spi_args);
    TEST_CASE_TIPS("spi idx:%d", spi_args.idx);
    TEST_CASE_TIPS("spi spi mode:%d", spi_args.spi_mode);
    TEST_CASE_TIPS("spi frame_len:%d", spi_args.frame_len);
    TEST_CASE_TIPS("spi cp_format:%d", spi_args.cp_format);
    TEST_CASE_TIPS("spi baud:%d", spi_args.baud);
    TEST_CASE_ASSERT_QUIT(ret == 0, "spi config failed, spi idx: %d", spi_args.idx);

    TEST_CASE_READY();
    send_num = csi_spi_send(&spi_handler, spi_transfer_data, sizeof(spi_transfer_data), 30000);
    TEST_CASE_ASSERT_QUIT(send_num > 0, "spi send data failed, send_num: %d", send_num);

#ifdef CSI_SPI_GET_STATE
    //test csi_spi_get_state
    csi_state_t state;
    spi_ret = csi_spi_get_state(&spi_handler, &state);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "get spi state failed, expected CSI_OK, returned: %d", spi_ret);
    TEST_CASE_ASSERT(state.writeable == 1, "expected spi state.writeable==1, actual: %u", state.writeable);
#endif

    csi_spi_uninit(&spi_handler);
    return 0;
}


int test_spi_sync_receive(void *args)
{
    int ret;
    int data_len = sizeof(spi_transfer_data);
    char *recv_buffer = NULL;

    int received_num = 0 ;
    csi_error_t spi_ret;
    csi_spi_t spi_handler;

    spi_args.idx = *((uint64_t *)args);
    spi_args.frame_len = *((uint64_t *)args + 1);
    spi_args.cp_format = *((uint64_t *)args + 2);
    spi_args.baud = *((uint64_t *)args + 3);

    spi_ret = csi_spi_init(&spi_handler, spi_args.idx);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "spi %d init fail", spi_args.idx);

    ret = spi_test_config(&spi_handler, &spi_args);
    TEST_CASE_TIPS("spi idx:%d", spi_args.idx);
    TEST_CASE_TIPS("spi spi mode:%d", spi_args.spi_mode);
    TEST_CASE_TIPS("spi frame_len:%d", spi_args.frame_len);
    TEST_CASE_TIPS("spi cp_format:%d", spi_args.cp_format);
    TEST_CASE_TIPS("spi baud:%d", spi_args.baud);
    TEST_CASE_ASSERT_QUIT(ret == 0, "spi_test_config failed.");

    recv_buffer = (char *)malloc(data_len + 1);
    TEST_CASE_ASSERT_QUIT(recv_buffer != NULL, "malloc failed.");
    *(recv_buffer + data_len) = '\0';

    TEST_CASE_READY();
    received_num = csi_spi_receive(&spi_handler, recv_buffer, data_len, 10000);
    TEST_CASE_TIPS("actual received str length: %d", received_num);
    TEST_CASE_TIPS("expected to receive str length: %d", data_len);

    TEST_CASE_ASSERT(received_num == data_len, "spi actual receive data len not equal to expected: %d", data_len);
    TEST_CASE_ASSERT(memcmp(recv_buffer, spi_transfer_data, data_len) == 0, "actual received data not equal to expected");

    free(recv_buffer);
#ifdef CSI_SPI_GET_STATE
    //test csi_spi_get_state
    csi_state_t spi_state;
    spi_ret = csi_spi_get_state(&spi_handler, &spi_state);
    TEST_CASE_ASSERT(spi_ret == CSI_OK, "get spi state failed, expected CSI_OK, returned: %d", spi_ret);
    TEST_CASE_ASSERT(spi_state.readable == 1, "expected spi state.readable==1, actual: %u", spi_state.readable);
#endif
    csi_spi_uninit(&spi_handler);

    return 0;

}

int test_spi_sync_send_receive(void *args)
{
    int ret;
    csi_error_t spi_ret;
    csi_spi_t spi_handler;
    int data_len = sizeof(spi_transfer_data);
    char *recv_buf;

    spi_args.idx = *((uint64_t *)args);
    spi_args.frame_len = *((uint64_t *)args + 1);
    spi_args.cp_format = *((uint64_t *)args + 2);
    spi_args.baud = *((uint64_t *)args + 3);

    spi_ret = csi_spi_init(&spi_handler, spi_args.idx);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "spi %d init fail", spi_args.idx);

    ret = spi_test_config(&spi_handler, &spi_args);
    TEST_CASE_TIPS("spi idx:%d", spi_args.idx);
    TEST_CASE_TIPS("spi spi mode:%d", spi_args.spi_mode);
    TEST_CASE_TIPS("spi frame_len:%d", spi_args.frame_len);
    TEST_CASE_TIPS("spi cp_format:%d", spi_args.cp_format);
    TEST_CASE_TIPS("spi baud:%d", spi_args.baud);
    TEST_CASE_ASSERT_QUIT(ret == 0, "spi config failed, spi idx: %d", spi_args.idx);

    recv_buf = (char *)malloc(data_len + 1);
    TEST_CASE_ASSERT_QUIT(recv_buf != NULL, "malloc failed.");
    *(recv_buf + data_len) = '\0';

    TEST_CASE_READY();

    uint32_t recv_num;
    recv_num = csi_spi_send_receive(&spi_handler, spi_transfer_data, recv_buf, data_len, 10000);
    TEST_CASE_ASSERT_QUIT(recv_num == data_len, "spi send receive data failed.");
    TEST_CASE_ASSERT(memcmp(spi_transfer_data, recv_buf, data_len) == 0, "received data not equal to expected");
    free(recv_buf);

#ifdef CSI_SPI_GET_STATE
    //test csi_spi_get_state
    csi_state_t state;
    spi_ret = csi_spi_get_state(&spi_handler, &state);
    TEST_CASE_ASSERT_QUIT(spi_ret == CSI_OK, "get spi state failed, expected CSI_OK, returned: %d", spi_ret);
    TEST_CASE_ASSERT(state.writeable == 1, "expected spi state.writeable==1, actual: %u", state.writeable);
#endif
    csi_spi_uninit(&spi_handler);
    return 0;
}
