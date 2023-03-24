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

#include "spiflash_test.h"

static csi_gpio_t g_gpio;
static uint64_t tst_gpio_pin_mask;

static void spi_cs_callback(csi_gpio_pin_state_t value)
{
    csi_gpio_write(&g_gpio, 1<<tst_gpio_pin_mask, value);
}


int test_spiflash_program_read(void *args)
{
    csi_error_t ret;
    csi_spiflash_t spiflash_handle;
    test_spiflash_args_t spiflash_args;
    csi_spiflash_info_t info;
    uint8_t *tx_data = NULL;
    uint8_t *rx_data = NULL;

    uint64_t gpio_idx;

    gpio_idx 				= *((uint64_t *)args);
    tst_gpio_pin_mask 		= *((uint64_t *)args + 1);
    spiflash_args.idx  		= *((uint64_t *)args + 2);
    spiflash_args.offset 	= *((uint64_t *)args + 3);
    spiflash_args.size 		= *((uint64_t *)args + 4);
	
    TEST_CASE_TIPS("gpio_idx: %d, gpio_pin_mask: %d", gpio_idx, tst_gpio_pin_mask);
    TEST_CASE_TIPS("idx: %d, offset: %x, size %d", spiflash_args.idx, spiflash_args.offset, spiflash_args.size);

    csi_gpio_init(&g_gpio, gpio_idx);
    csi_gpio_dir(&g_gpio, 1<<tst_gpio_pin_mask, GPIO_DIRECTION_OUTPUT);
   
    ret = csi_spiflash_spi_init(&spiflash_handle, spiflash_args.idx, spi_cs_callback);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi spiflash init error, actual return value is %d", ret);

    ret = csi_spiflash_get_flash_info(&spiflash_handle, &info);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi spiflash spi get flash info error");
    TEST_CASE_TIPS("flash info -- flash name %s", info.flash_name);
    TEST_CASE_TIPS("flash info -- flash id 0x%x", info.flash_id);
    TEST_CASE_TIPS("flash info -- flash size 0x%x", info.flash_size);
    TEST_CASE_TIPS("flash info -- flash pagesize 0x%x", info.page_size);
    TEST_CASE_TIPS("flash info -- flash sectorsize 0x%x", info.sector_size);
    TEST_CASE_TIPS("flash info -- flash xipaddr 0x%x", info.xip_addr);

    ret = csi_spiflash_erase(&spiflash_handle, spiflash_args.offset, info.sector_size);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi spiflash spi erase error");

    tx_data = (uint8_t *)malloc(sizeof(uint8_t) * spiflash_args.size);
    TEST_CASE_ASSERT(tx_data != NULL, "tx_data malloc error");
    memset(tx_data, 0xa, sizeof(uint8_t) * spiflash_args.size);
    ret = csi_spiflash_program(&spiflash_handle, spiflash_args.offset, tx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret >= 0, "csi spiflash spi program error");

    rx_data = (uint8_t *)malloc(sizeof(uint8_t) * spiflash_args.size);
    TEST_CASE_ASSERT(rx_data != NULL, "rx_data malloc error");
    memset(rx_data, 0, sizeof(uint8_t) * spiflash_args.size);
    ret = csi_spiflash_read(&spiflash_handle, spiflash_args.offset, rx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret >= 0, "csi spiflash spi read error");
    ret = memcmp(tx_data, rx_data, spiflash_args.size);
    TEST_CASE_ASSERT(ret == 0, "csi spiflash read and program not equal");


    free(tx_data);
    free(rx_data);

	csi_gpio_uninit(&g_gpio);
    csi_spiflash_spi_uninit(&spiflash_handle);
 
    return 0;
}
