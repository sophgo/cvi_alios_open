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

#include <spiflash_test.h>


const test_info_t spiflash_test_funcs_map[] = {
    {"SPIFLASH_INTERFACE", test_spiflash_interface, 0},
#ifdef CSI_SPIFLASH_SPI
    {"SPIFLASH_PROGRAM_READ", test_spiflash_program_read, 5},
#endif
#ifdef CSI_SPIFLASH_QSPI
    {"SPIFLASH_QSPI_PROGRAMREAD", test_spiflash_qspi_programread, 5},
    {"SPIFLASH_WRITE_READ", test_spiflash_qspi_wr, 7},
#endif
#ifdef CSI_SPIFLASH_LOCK
    {"SPIFLASH_LOCK", test_spiflash_lock, 5},
#endif
};


int test_spiflash_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)spiflash_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("SPIFLASH don't supported this command");
    }

    return ret;
}