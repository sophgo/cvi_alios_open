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

#ifdef CSI_SPIFLASH_QSPI

#include "spiflash_test.h"


int test_spiflash_qspi_programread(void *args)
{
    csi_error_t ret;
    csi_spiflash_t qspiflash_handle;
    test_spiflash_args_t spiflash_args;
    csi_spiflash_info_t info;
    uint8_t *tx_data = NULL;
    uint8_t *rx_data = NULL;
    uint32_t frequence;

    csi_spiflash_data_line_t line_mode;


    spiflash_args.idx 		= *((uint64_t *)args);
    spiflash_args.offset 	= *((uint64_t *)args + 1);
    spiflash_args.size 		= *((uint64_t *)args + 2);
    line_mode 				= *((uint64_t *)args + 3);
    frequence 				= *((uint64_t *)args + 4);

    TEST_CASE_TIPS("idx: %d, offset: 0x%x, size: %d, line mode: %d, frequency: %d", spiflash_args.idx, spiflash_args.offset,
                   spiflash_args.size, line_mode, frequence);

    ret = csi_spiflash_qspi_init(&qspiflash_handle, spiflash_args.idx, NULL);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi spiflash spi init error");

    ret = csi_spiflash_config_data_line(&qspiflash_handle, line_mode);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi spiflash qspi config data line error");

    int ret_freq;
    ret_freq = csi_spiflash_frequence(&qspiflash_handle, frequence);
    TEST_CASE_TIPS("csi spiflash qspi config frequence is %d", ret_freq);

    ret = csi_spiflash_get_flash_info(&qspiflash_handle, &info);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi spiflash spi get flash info error");
    TEST_CASE_TIPS("flash name %s", info.flash_name);
    TEST_CASE_TIPS("flash id 0x%x", info.flash_id);
    TEST_CASE_TIPS("flash size 0x%x", info.flash_size);
    TEST_CASE_TIPS("flash pagesize 0x%x", info.page_size);
    TEST_CASE_TIPS("flash sectorsize 0x%x", info.sector_size);
    TEST_CASE_TIPS("flash xipaddr 0x%x", info.xip_addr);

    ret = csi_spiflash_erase(&qspiflash_handle, spiflash_args.offset, spiflash_args.size);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi spiflash spi erase error");

    tx_data = (uint8_t *)malloc(sizeof(uint8_t) * spiflash_args.size);
    TEST_CASE_ASSERT(tx_data != NULL, "tx_data malloc error");
    memset(tx_data, 0xa, sizeof(uint8_t) * spiflash_args.size);
    ret = csi_spiflash_program(&qspiflash_handle, spiflash_args.offset, tx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret >= 0, "csi spiflash spi program error");

    rx_data = (uint8_t *)malloc(sizeof(uint8_t) * spiflash_args.size);
    memset(rx_data, 0, sizeof(uint8_t) * spiflash_args.size);
    ret = csi_spiflash_read(&qspiflash_handle, spiflash_args.offset, rx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret >= 0, "csi spiflash spi read error");
    ret = memcmp(tx_data, rx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret == 0, "csi spiflash read and program not equal");

    free(tx_data);
    free(rx_data);

	csi_spiflash_qspi_uninit(&qspiflash_handle);
    return 0;
}


static csi_gpio_t g_gpio;
static uint64_t tst_gpio_pin_mask;

static void spi_cs_callback(csi_gpio_pin_state_t value)
{
    csi_gpio_write(&g_gpio, 1 << tst_gpio_pin_mask, value);
}

/**
 * description: 测试QSPI控制器与FLASH的读写，软件使用GPIO来控制FLASH的CS功能脚
 * param {args} AT指令传入的参数
 * return {int} 测试用例执行完成返回0，否则返回1
 */
int test_spiflash_qspi_wr(void *args)
{
    csi_error_t ret;
    csi_spiflash_t qspiflash_handle;
    test_spiflash_args_t spiflash_args;
    csi_spiflash_info_t info;
    uint8_t *tx_data = NULL;
    uint8_t *rx_data = NULL;
    uint64_t gpio_idx, frequence;
    int ret_freq;
    csi_spiflash_data_line_t line_mode;
    uint8_t write_size;

    
	gpio_idx 				= *((uint64_t *)args);
    tst_gpio_pin_mask 		= *((uint64_t *)args + 1);
	spiflash_args.idx  		= *((uint64_t *)args + 2);
    spiflash_args.offset 	= *((uint64_t *)args + 3);
    spiflash_args.size 		= *((uint64_t *)args + 4);
    frequence 				= *((uint64_t *)args + 5);
    line_mode 				= *((uint64_t *)args + 6);

    TEST_CASE_TIPS("gpio idx: %d, gpio pin: 0x%x, gpio idx: %d, offset: %d, size %d, frequence: %d, line mode: %d",
                   gpio_idx,tst_gpio_pin_mask,spiflash_args.idx, \
                   spiflash_args.offset, spiflash_args.size, frequence, line_mode);


    csi_gpio_init(&g_gpio, gpio_idx);
    csi_gpio_dir(&g_gpio, 1 << tst_gpio_pin_mask, GPIO_DIRECTION_OUTPUT);

    ret = csi_spiflash_qspi_init(&qspiflash_handle, spiflash_args.idx, spi_cs_callback);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi spiflash qspi init error");

    ret = csi_spiflash_config_data_line(&qspiflash_handle, line_mode);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi spiflash qspi config data line error");

    ret_freq = csi_spiflash_frequence(&qspiflash_handle, frequence);
    TEST_CASE_TIPS("csi spiflash qspi config frequence is %d", ret_freq);

    ret = csi_spiflash_get_flash_info(&qspiflash_handle, &info);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi spiflash spi get flash info error");
    TEST_CASE_TIPS("flash name %s", info.flash_name);
    TEST_CASE_TIPS("flash id 0x%x", info.flash_id);
    TEST_CASE_TIPS("flash size 0x%x", info.flash_size);
    TEST_CASE_TIPS("flash pagesize 0x%x", info.page_size);
    TEST_CASE_TIPS("flash sectorsize 0x%x", info.sector_size);
    TEST_CASE_TIPS("flash xipaddr 0x%x", info.xip_addr);

    uint32_t erase_page_cnt = 0;

    if (spiflash_args.size % 0x1000) {
        erase_page_cnt = spiflash_args.size / 0x1000 + 1;
    } else {
        erase_page_cnt = spiflash_args.size / 0x1000;
    }

    if (spiflash_args.offset % 0x1000) {
        erase_page_cnt += 1;
    }


    ret = csi_spiflash_erase(&qspiflash_handle, spiflash_args.offset & 0xFFFFF000, erase_page_cnt * 0x1000);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi spiflash spi erase error");

    tx_data = (uint8_t *)malloc(sizeof(uint8_t) * spiflash_args.size);
    TEST_CASE_ASSERT(tx_data != NULL, "tx_data malloc error");

    write_size = rand();
    memset(tx_data, write_size, sizeof(uint8_t) * spiflash_args.size);
    ret = csi_spiflash_program(&qspiflash_handle, spiflash_args.offset, tx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret >= 0, "csi spiflash spi program error");

    rx_data = (uint8_t *)malloc(sizeof(uint8_t) * spiflash_args.size);
    memset(rx_data, 0, sizeof(uint8_t) * spiflash_args.size);
    ret = csi_spiflash_read(&qspiflash_handle, spiflash_args.offset, rx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret >= 0, "csi spiflash spi read error");
    ret = memcmp(tx_data, rx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret == 0, "csi spiflash read and program not equal");

    free(tx_data);
    free(rx_data);

	csi_gpio_uninit(&g_gpio);
	csi_spiflash_qspi_uninit(&qspiflash_handle);
    return 0;
}
#endif
