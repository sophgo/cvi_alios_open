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

#include "efuse_test.h"

int test_efuse_interface(void *args)
{
	csi_error_t ret;

    ret = csi_efuse_init(NULL, 0);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "para NULL efuse init should be error");

    ret = csi_efuse_read(NULL, 0, NULL, 0);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi efuse read para NULL fucn should be error");

    ret = csi_efuse_program(NULL, 0, NULL, 0);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi efuse program para NULL fucn should be error");

    ret = csi_efuse_get_info(NULL, NULL);
    TEST_CASE_ASSERT(ret == CSI_ERROR, "csi efuse get info para NULL fucn should be error");
       
    csi_efuse_uninit(NULL);

    return 0;
}