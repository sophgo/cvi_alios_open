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

int test_iic_memoryTransfer(void *args)
{
    test_iic_args_t td;
    int ret;
    csi_error_t ret_sta;
    csi_iic_t iic_hd;
    int32_t ret_num;

    td.dev_idx = *((uint64_t *)args);
    td.addr_mode = *((uint64_t *)args + 1);
    td.speed = *((uint64_t *)args + 2);
    td.trans_size = *((uint64_t *)args + 3);
    td.slave_addr = *((uint64_t *)args + 4);
    td.timeout = *((uint64_t *)args + 5);
    td.mem_addr = *((uint64_t *)args + 6);
    td.mem_addr_size = *((uint64_t *)args + 7);

    TEST_CASE_TIPS("test iic's idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config iic's mode is %d", IIC_MODE_MASTER);
    TEST_CASE_TIPS("config iic's address mode is %d", td.addr_mode);
    TEST_CASE_TIPS("config iic's sped is %d", td.speed);
    TEST_CASE_TIPS("send size is %d", td.trans_size);
    TEST_CASE_TIPS("select slave address is %d", td.slave_addr);
    TEST_CASE_TIPS("set send timeout is %d", td.timeout);
    TEST_CASE_TIPS("select EEPROM's address is %d", td.mem_addr);
    TEST_CASE_TIPS("set transfer size mode is %d with EEPROM", td.mem_addr_size);

    ret_sta = csi_iic_init(&iic_hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "iic %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_iic_mode(&iic_hd, IIC_MODE_MASTER);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d config mode fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }


    ret_sta = csi_iic_addr_mode(&iic_hd, td.addr_mode);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d config address mode fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    ret_sta = csi_iic_speed(&iic_hd, td.speed);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d config speed fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    char *send_data = NULL, *receive_data = NULL;
    send_data = (char *)malloc(td.trans_size);
    receive_data = (char *)malloc(td.trans_size);

    if ((send_data == NULL) || (receive_data == NULL)) {
        free(send_data);
        free(receive_data);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(send_data, td.trans_size);

    ret_num = csi_iic_mem_send(&iic_hd, td.slave_addr, td.mem_addr, td.mem_addr_size, send_data, td.trans_size, td.timeout);
    TEST_CASE_ASSERT(ret_num == td.trans_size,
                     "iic %d send data to EEPROM fail, expected return value is %d, actual return value is %d", td.dev_idx, td.trans_size,
                     ret_num);

    tst_mdelay(1000);

    ret_num = csi_iic_mem_receive(&iic_hd, td.slave_addr, td.mem_addr, td.mem_addr_size, receive_data, td.trans_size,
                                  td.timeout);
    TEST_CASE_ASSERT(ret_num == td.trans_size,
                     "iic %d receive data to EEPROM fail, expected return value is %d, actual return value is %d", td.dev_idx, td.trans_size,
                     ret_num);

    ret = memcmp((const char *)send_data, (const char *)receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret == 0, "EEPROM read data not equal write data");


    free(send_data);
    free(receive_data);

    csi_iic_uninit(&iic_hd);

    return 0;

}
