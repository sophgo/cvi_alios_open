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

static void iic_receive_callback(csi_iic_t *iic, csi_iic_event_t event, void *arg)
{
    if (event == IIC_EVENT_RECEIVE_COMPLETE) {
        *(uint8_t *)arg = 0;
    }
}



int test_iic_slaveAsyncReceive(void *args)
{
    test_iic_args_t td;
    int ret;
    csi_error_t ret_sta;
    csi_iic_t iic_hd;

    td.dev_idx = *((uint64_t *)args);
    td.addr_mode = *((uint64_t *)args + 1);
    td.speed = *((uint64_t *)args + 2);
    td.own_addr = *((uint64_t *)args + 3);
    td.trans_size = *((uint64_t *)args + 4);

    TEST_CASE_TIPS("test iic's idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config iic's mode is %d", IIC_MODE_SLAVE);
    TEST_CASE_TIPS("config iic's address mode is %d", td.addr_mode);
    TEST_CASE_TIPS("config iic's speed is %d", td.speed);
    TEST_CASE_TIPS("config iic's own address is %d", td.own_addr);
    TEST_CASE_TIPS("receive size is %d", td.trans_size);


    ret_sta = csi_iic_init(&iic_hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "iic %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_iic_mode(&iic_hd, IIC_MODE_SLAVE);

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

    ret_sta = csi_iic_own_addr(&iic_hd, td.own_addr);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d config own address fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    volatile uint8_t lock;
    ret_sta = csi_iic_attach_callback(&iic_hd, iic_receive_callback, (void *)&lock);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d attach callback fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    char *receive_data = NULL, *expect_data = NULL;
    receive_data = aligned_malloc(td.trans_size, 64);
    expect_data = aligned_malloc(td.trans_size, 64);

    if ((receive_data == NULL) || (expect_data == NULL)) {
        aligned_free(receive_data);
        aligned_free(expect_data);
        csi_iic_detach_callback(&iic_hd);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(expect_data, td.trans_size);

    lock = 1;
    TEST_CASE_READY();
    ret_sta = csi_iic_slave_receive_async(&iic_hd, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret_sta == 0,
                     "iic %d slave async receive fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);


    while (lock) ;

    csi_iic_detach_callback(&iic_hd);

    ret = memcmp(expect_data, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret == 0, "the data sent is not equal to the data received");


    aligned_free(receive_data);
    aligned_free(expect_data);

    csi_iic_uninit(&iic_hd);

    return 0;
}


int test_iic_masterAsyncReceive(void *args)
{
    test_iic_args_t td;
    int ret;
    csi_error_t ret_sta;
    csi_iic_t iic_hd;

    td.dev_idx = *((uint64_t *)args);
    td.addr_mode = *((uint64_t *)args + 1);
    td.speed = *((uint64_t *)args + 2);
    td.trans_size = *((uint64_t *)args + 3);
    td.slave_addr = *((uint64_t *)args + 4);

    TEST_CASE_TIPS("test iic's idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config iic's mode is %d", IIC_MODE_MASTER);
    TEST_CASE_TIPS("config iic's address mode is %d", td.addr_mode);
    TEST_CASE_TIPS("config iic's speed is %d", td.speed);
    TEST_CASE_TIPS("receive size is %d", td.trans_size);
    TEST_CASE_TIPS("select slave address is %d", td.slave_addr);

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

    volatile uint8_t lock;
    ret_sta = csi_iic_attach_callback(&iic_hd, iic_receive_callback, (void *)&lock);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d attach callback fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    char *receive_data = NULL, *expect_data = NULL;
    receive_data = aligned_malloc(td.trans_size, 64);
    expect_data = aligned_malloc(td.trans_size, 64);

    if ((receive_data == NULL) || (expect_data == NULL)) {
        aligned_free(receive_data);
        aligned_free(expect_data);
        csi_iic_detach_callback(&iic_hd);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(expect_data, td.trans_size);

    lock = 1;
    TEST_CASE_READY();
    ret_sta = csi_iic_master_receive_async(&iic_hd, td.slave_addr, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret_sta == 0,
                     "iic %d msater async receive fail, expected return value is 0, actual return value is %d.", td.dev_idx, ret_sta);

    while (lock) ;

    csi_iic_detach_callback(&iic_hd);

    ret = memcmp(expect_data, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret == 0, "the data sent is not equal to the data received");

    aligned_free(receive_data);
    aligned_free(expect_data);

    csi_iic_uninit(&iic_hd);

    return 0;
}


int test_iic_masterSyncReceive(void *args)
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

    TEST_CASE_TIPS("test iic's idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config iic's mode is %d", IIC_MODE_MASTER);
    TEST_CASE_TIPS("config iic's address mode is %d", td.addr_mode);
    TEST_CASE_TIPS("config iic's speed is %d", td.speed);
    TEST_CASE_TIPS("receive size is %d", td.trans_size);
    TEST_CASE_TIPS("select slave address is %d", td.slave_addr);
    TEST_CASE_TIPS("set send timeout is %d", td.timeout);


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


    char *receive_data = NULL, *expect_data = NULL;
    receive_data = aligned_malloc(td.trans_size, 64);
    expect_data = aligned_malloc(td.trans_size, 64);

    if ((receive_data == NULL) || (expect_data == NULL)) {
        aligned_free(receive_data);
        aligned_free(expect_data);
        csi_iic_detach_callback(&iic_hd);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(expect_data, td.trans_size);
    TEST_CASE_READY();
    ret_num = csi_iic_master_receive(&iic_hd, td.slave_addr, receive_data, td.trans_size, td.timeout);
    TEST_CASE_ASSERT(ret_num == td.trans_size,
                     "iic %d msater async receive fail, expected return value is %d, actual return value is %d.", td.dev_idx, td.trans_size,
                     ret_num);



    ret = memcmp(expect_data, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret == 0, "the data sent is not equal to the data received");


    aligned_free(receive_data);
    aligned_free(expect_data);

    csi_iic_uninit(&iic_hd);

    return 0;
}



int test_iic_slaveSyncReceive(void *args)
{
    test_iic_args_t td;
    int ret;
    csi_error_t ret_sta;
    csi_iic_t iic_hd;
    int32_t ret_num;

    td.dev_idx = *((uint64_t *)args);
    td.addr_mode = *((uint64_t *)args + 1);
    td.speed = *((uint64_t *)args + 2);
    td.own_addr = *((uint64_t *)args + 3);
    td.trans_size = *((uint64_t *)args + 4);
    td.timeout = *((uint64_t *)args + 5);

    TEST_CASE_TIPS("test iic's idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config iic's mode is %d", IIC_MODE_SLAVE);
    TEST_CASE_TIPS("config iic's address mode is %d", td.addr_mode);
    TEST_CASE_TIPS("config iic's speed is %d", td.speed);
    TEST_CASE_TIPS("config iic's own address is %d", td.own_addr);
    TEST_CASE_TIPS("receive size is %d", td.trans_size);
    TEST_CASE_TIPS("set send timeout is %d", td.timeout);


    ret_sta = csi_iic_init(&iic_hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "iic %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);

    ret_sta = csi_iic_mode(&iic_hd, IIC_MODE_SLAVE);

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

    ret_sta = csi_iic_own_addr(&iic_hd, td.own_addr);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d config own address fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }


    char *receive_data = NULL, *expect_data = NULL;
    receive_data = aligned_malloc(td.trans_size, 64);
    expect_data = aligned_malloc(td.trans_size, 64);

    if ((receive_data == NULL) || (expect_data == NULL)) {
        aligned_free(receive_data);
        aligned_free(expect_data);
        csi_iic_detach_callback(&iic_hd);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(expect_data, td.trans_size);
    TEST_CASE_READY();
    ret_num = csi_iic_slave_receive(&iic_hd, receive_data, td.trans_size, td.timeout);
    TEST_CASE_ASSERT(ret_num == td.trans_size,
                     "iic %d msater async receive fail, expected return value is %d, actual return value is %d.", td.dev_idx, td.trans_size,
                     ret_num);



    ret = memcmp(expect_data, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret == 0, "the data sent is not equal to the data received");


    aligned_free(receive_data);
    aligned_free(expect_data);

    csi_iic_uninit(&iic_hd);

    return 0;
}



int test_iic_slaveDmaReceive(void *args)
{
    test_iic_args_t td;
    int ret;
    csi_error_t ret_sta;
    csi_iic_t iic_hd;

    td.dev_idx = *((uint64_t *)args);
    td.addr_mode = *((uint64_t *)args + 1);
    td.speed = *((uint64_t *)args + 2);
    td.own_addr = *((uint64_t *)args  + 3);
    td.trans_size = *((uint64_t *)args + 4);

    TEST_CASE_TIPS("test iic's idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config iic's mode is %d", IIC_MODE_SLAVE);
    TEST_CASE_TIPS("config iic's address mode is %d", td.addr_mode);
    TEST_CASE_TIPS("config iic's speed is %d", td.speed);
    TEST_CASE_TIPS("config iic's own address is %d", td.own_addr);
    TEST_CASE_TIPS("receive size is %d", td.trans_size);

    ret_sta = csi_iic_init(&iic_hd, td.dev_idx);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "iic %d init fail, expected return value is 0, actual return value is %d.",
                          td.dev_idx, ret_sta);


    ret_sta = csi_iic_mode(&iic_hd, IIC_MODE_SLAVE);

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

    ret_sta = csi_iic_own_addr(&iic_hd, td.own_addr);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d config own address fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    volatile uint8_t lock;
    ret_sta = csi_iic_attach_callback(&iic_hd, iic_receive_callback, (void *)&lock);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d attach callback fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    csi_dma_ch_t rx_dma;
    ret_sta = csi_iic_link_dma(&iic_hd, NULL, &rx_dma);

    if (ret_sta != 0) {
        csi_iic_detach_callback(&iic_hd);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d link DMA channel fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }


    char *receive_data = NULL, *expect_data = NULL;

    receive_data = aligned_malloc(td.trans_size, 64);
    expect_data = aligned_malloc(td.trans_size, 64);

    if ((receive_data == NULL) || (expect_data == NULL)) {
        aligned_free(receive_data);
        aligned_free(expect_data);
        csi_iic_detach_callback(&iic_hd);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(expect_data, td.trans_size);


    lock = 1;
    TEST_CASE_READY();
    ret_sta = csi_iic_slave_receive_async(&iic_hd, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret_sta == 0, "iic %d master async send fail, expected return value is 0, actual return value is %d.",
                     td.dev_idx, ret_sta);

    while (lock) ;

    ret_sta = csi_iic_link_dma(&iic_hd, NULL, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "iic %d unlink DMA channel fail, expected return value is 0, actual return value is %d.",
                     td.dev_idx, ret_sta);

    csi_iic_detach_callback(&iic_hd);

    ret = memcmp(expect_data, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret == 0, "the data sent is not equal to the data received");

    aligned_free(receive_data);
    aligned_free(expect_data);

    csi_iic_uninit(&iic_hd);

    return 0;
}





int test_iic_masterDmaReceive(void *args)
{
    test_iic_args_t td;
    int ret;
    csi_error_t ret_sta;
    csi_iic_t iic_hd;

    td.dev_idx = *((uint64_t *)args);
    td.addr_mode = *((uint64_t *)args + 1);
    td.speed = *((uint64_t *)args + 2);
    td.trans_size = *((uint64_t *)args + 3);
    td.slave_addr = *((uint64_t *)args + 4);

    TEST_CASE_TIPS("test iic's idx is %d", td.dev_idx);
    TEST_CASE_TIPS("config iic's mode is %d", IIC_MODE_MASTER);
    TEST_CASE_TIPS("config iic's address mode is %d", td.addr_mode);
    TEST_CASE_TIPS("config iic's speed is %d", td.speed);
    TEST_CASE_TIPS("receive size is %d", td.trans_size);
    TEST_CASE_TIPS("select slave address is %d", td.slave_addr);


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


    volatile uint8_t lock;
    ret_sta = csi_iic_attach_callback(&iic_hd, iic_receive_callback, (void *)&lock);

    if (ret_sta != 0) {
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d attach callback fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }

    csi_dma_ch_t rx_dma;
    ret_sta = csi_iic_link_dma(&iic_hd, NULL, &rx_dma);

    if (ret_sta != 0) {
        csi_iic_detach_callback(&iic_hd);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_ASSERT_QUIT(1 == 0, "iic %d link DMA channel fail, expected return value is 0, actual return value is %d.",
                              td.dev_idx, ret_sta);
    }


    char *receive_data = NULL, *expect_data = NULL;

    receive_data = aligned_malloc(td.trans_size, 64);
    expect_data = aligned_malloc(td.trans_size, 64);

    if ((receive_data == NULL) || (expect_data == NULL)) {
        aligned_free(receive_data);
        aligned_free(expect_data);
        csi_iic_detach_callback(&iic_hd);
        csi_iic_uninit(&iic_hd);
        TEST_CASE_WARN_QUIT("transfer size too long, malloc error");
    }

    transfer_data(expect_data, td.trans_size);


    lock = 1;
    TEST_CASE_READY();
    ret_sta = csi_iic_master_receive_async(&iic_hd, td.slave_addr, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret_sta == 0, "iic %d master async send fail, expected return value is 0, actual return value is %d.",
                     td.dev_idx, ret_sta);

    while (lock) {
        ;
    }

    ret_sta = csi_iic_link_dma(&iic_hd, NULL, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "iic %d unlink DMA channel fail, expected return value is 0, actual return value is %d.",
                     td.dev_idx, ret_sta);


    csi_iic_detach_callback(&iic_hd);

    ret = memcmp(expect_data, receive_data, td.trans_size);
    TEST_CASE_ASSERT(ret == 0, "the data sent is not equal to the data received");

    aligned_free(receive_data);
    aligned_free(expect_data);

    csi_iic_uninit(&iic_hd);

    return 0;
}
