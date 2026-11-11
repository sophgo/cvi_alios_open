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

#include <trng_test.h>

//#define TRNG_INVALID_VALUE_TEST	/*非法值测试/
#define TRNG_DATA_LENGTH	128
int test_trng_handle(void *args)
{
	uint32_t ret = 0;
	uint32_t ndata1[TRNG_DATA_LENGTH] = {0};
	uint32_t ndata2[TRNG_DATA_LENGTH] = {0};
	uint32_t data1 = 0;
	uint32_t data2 = 0;
	uint32_t i = 0, j = 0;

	TEST_CASE_TIPS("begin test TRNG");
#ifdef TRNG_INVALID_VALUE_TEST
	ret = csi_rng_get_single_word(NULL);
	TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_rng_get_single_word should return error, but return:%d", ret);	
#endif
	ret = csi_rng_get_single_word(&data1);
	TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_rng_get_single_word failed");
	ret = csi_rng_get_single_word(&data2);
	TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_rng_get_single_word failed");
	TEST_CASE_TIPS("data1 %u data2 %u",data1, data2);
	TEST_CASE_ASSERT_QUIT(data1 != data2, "data1 match data2 ,it is error");

#ifdef TRNG_INVALID_VALUE_TEST
	ret = csi_rng_get_multi_word(NULL, 0);
	TEST_CASE_ASSERT(ret==CSI_ERROR,"csi_rng_get_multi_word should return error, but return:%d", ret);
#endif	
	ret = csi_rng_get_multi_word(ndata1, TRNG_DATA_LENGTH);
	TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_rng_get_multi_word failed");
	TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_rng_get_multi_word failed");

	for(i=0; i<TRNG_DATA_LENGTH; i++)
	{
		if(ndata1[i] == ndata2[i])
		{
			TEST_CASE_ASSERT_QUIT(0, "ndata1[%d] match ndata2[%d] ,it is error",i,i);
		}
		for(j=i+1; j<TRNG_DATA_LENGTH; j++)
		{
			if(ndata1[i] == ndata1[j] || ndata2[i] == ndata2[j]) 
			{
				TEST_CASE_ASSERT_QUIT(0, "ndata1 or ndata2 match itself ,it is error");
			}
		}
	}
	TEST_CASE_TIPS("test TRNG finish");
	return 0;
}
