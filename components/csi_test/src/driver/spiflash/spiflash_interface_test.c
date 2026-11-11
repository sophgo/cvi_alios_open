/*
 * Copyright (C) 2017 C-SKY Microsystems Co., All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "spiflash_test.h"


static void spi_cs_callback(csi_gpio_pin_state_t value)
{

}

int test_spiflash_interface(void *args)
{
    csi_error_t ret;
    csi_spiflash_t spiflash_handle;
    csi_spiflash_info_t info;
    uint8_t *rx_data = NULL;
#ifdef CSI_SPIFLASH_SPI
    /*
     *	Call the csi_spiflash_spi_init function for interface testing
     */
    ret = csi_spiflash_spi_init(NULL, 0, spi_cs_callback);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_spi_init interface testing fail");

    ret = csi_spiflash_spi_init(&spiflash_handle, 0xffff, spi_cs_callback);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_spi_init interface testing fail");

    ret = csi_spiflash_spi_init(&spiflash_handle, 0, NULL);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_spi_init interface testing fail");
    /*	done */

    csi_spiflash_spi_init(&spiflash_handle, 0, spi_cs_callback);

    /*
     *	Call the csi_spiflash_get_flash_info function for interface testing
     */
    ret = csi_spiflash_get_flash_info(NULL, &info);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_get_flash_info interface testing fail");

    ret = csi_spiflash_get_flash_info(&spiflash_handle, NULL);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_get_flash_info interface testing fail");
    /*	done */

    /*
     *	Call the csi_spiflash_uninit function for interface testing
     */
    csi_spiflash_spi_uninit(NULL);
    csi_spiflash_spi_uninit(&spiflash_handle);
    /*	done */

#endif

#ifdef CSI_SPIFLASH_QSPI
    /*
     *	Call the csi_spiflash_qspi_init function for interface testing
     */
    ret = csi_spiflash_qspi_init(NULL, 0, NULL);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_qspi_init interface testing fail");

    csi_spiflash_qspi_uninit(NULL);

    /*	done */
#endif   

    /*
     *	Call the csi_spiflash_read function for interface testing
     */
    rx_data = (uint8_t *)malloc(sizeof(uint8_t) * 2);
    TEST_CASE_ASSERT(rx_data != NULL, "rx_data malloc error");

    ret = csi_spiflash_read(NULL, 0, rx_data, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_read interface testing fail");

    ret = csi_spiflash_read(&spiflash_handle, 0, NULL, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_read interface testing fail");

    ret = csi_spiflash_read(&spiflash_handle, 0, rx_data, 0);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_read interface testing fail");

    /*	done */

    /*
     *	Call the csi_spiflash_program function for interface testing
     */
    ret = csi_spiflash_program(NULL, 0, rx_data, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_program interface testing fail");

    ret = csi_spiflash_program(&spiflash_handle, 0, NULL, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_program interface testing fail");

    ret = csi_spiflash_program(&spiflash_handle, 0, rx_data, 0);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_program interface testing fail");

    /*	done */

    /*
     *	Call the csi_spiflash_erase function for interface testing
     */
    ret = csi_spiflash_erase(NULL, 0, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_erase interface testing fail");

    ret = csi_spiflash_erase(&spiflash_handle, 0, 0);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_erase interface testing fail");

    /*	done */

#ifdef CSI_SPIFLASH_READ_REG
    /*
     *	Call the csi_spiflash_read_reg function for interface testing
     */
    ret = csi_spiflash_read_reg(NULL, 0, rx_data, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_read_reg interface testing fail");

    ret = csi_spiflash_read_reg(&spiflash_handle, 0, NULL, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_read_reg interface testing fail");

    ret = csi_spiflash_read_reg(&spiflash_handle, 0, rx_data, 0);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_read_reg interface testing fail");

    /*	done */
#endif

    /*
     *	Call the csi_spiflash_write_reg function for interface testing
     */
    ret = csi_spiflash_write_reg(NULL, 0, rx_data, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_write_reg interface testing fail");

    ret = csi_spiflash_write_reg(&spiflash_handle, 0, NULL, 2);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_write_reg interface testing fail");

    ret = csi_spiflash_write_reg(&spiflash_handle, 0, rx_data, 0);
    TEST_CASE_ASSERT(ret == -1, "spiflash parameter is NULL, csi_spiflash_write_reg interface testing fail");

    /*	done */


    return 0;
}
