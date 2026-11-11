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

#include <dma_test.h>

int test_dma_interface(void *args)
{
	csi_error_t ret_status;

	ret_status = csi_dma_init(NULL, 0);
	TEST_CASE_ASSERT(ret_status == CSI_ERROR, "parameter: dma is NULL, csi_dma_init expected return value is -1, actual return value is %d.", ret_status);

	csi_dma_uninit(NULL);

	ret_status = csi_dma_ch_alloc(NULL, 0, 0);
	TEST_CASE_ASSERT(ret_status == CSI_ERROR, "parameter: dma is NULL, csi_dma_ch_alloc expected return value is -1, actual return value is %d.", ret_status);

	csi_dma_ch_free(NULL);

	csi_dma_ch_config_t config;
	ret_status = csi_dma_ch_config(NULL, &config);
	TEST_CASE_ASSERT(ret_status == CSI_ERROR, "parameter: dma is NULL, csi_dma_ch_config expected return value is -1, actual return value is %d.", ret_status);
	
	csi_dma_ch_t dma_ch;
	ret_status = csi_dma_ch_alloc(&dma_ch, 0, 0);
	TEST_CASE_ASSERT(ret_status == 0, "csi_dma_ch_alloc fail, expected return value is 0, actual return value is %d.", ret_status);
	ret_status = csi_dma_ch_config(&dma_ch, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR, "parameter: config is NULL, csi_dma_ch_config expected return value is -1, actual return value is %d.", ret_status);
	csi_dma_ch_free(&dma_ch);
	
	csi_dma_ch_start(NULL,0x0,0x0,100);

	csi_dma_ch_stop(NULL);

	ret_status = csi_dma_ch_attach_callback(NULL, NULL, NULL);
	TEST_CASE_ASSERT(ret_status == CSI_ERROR, "parameter: dma is NULL, csi_dma_ch_attach_callback expected return value is -1, actual return value is %d.", ret_status);

	csi_dma_ch_detach_callback(NULL);

	return 0;
}
