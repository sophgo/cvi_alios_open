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
extern int csi_test_atserver_send(const char *command, ...);
uint32_t ndata[TRNG_DATA_LEN] = {0};

int test_trng_multi(void *args)
{
	uint32_t ret = 0;
    uint32_t number_of_words = 0;
    uint32_t number_of_loop = 0;
    uint32_t params[2];
    int ret_args=0;

    ret_args = args_parsing(args, params, 2);
    TEST_CASE_ASSERT_QUIT(ret_args==0,"number of argtments is wrong.");

    number_of_words = params[0];
    number_of_loop = params[1];

    uint32_t i, j; 
    for(i=1;i<=number_of_loop;i++){
        ret = csi_rng_get_multi_word(ndata, number_of_words);
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_rng_get_single_word failed");

        csi_test_atserver_send("\r\ndata:%d[", i);

        for(j=1;j<=number_of_words;j++){
            csi_test_atserver_send("%u,", ndata[j-1]);
        }
        csi_test_atserver_send("]");
    }

	TEST_CASE_TIPS("test TRNG finish");
	return 0;
}

int test_trng_single(void *args)
{
	uint32_t ret = 0;
	uint32_t data = 0;
    uint32_t number_of_loop = 0;
    uint32_t params[1];
    int ret_args=0;

    ret_args = args_parsing(args, params, 1);
    TEST_CASE_ASSERT_QUIT(ret_args==0,"number of argtments is wrong.");

    number_of_loop = params[0];

    uint32_t i;
    for(i=1;i<=number_of_loop;i++){
        ret = csi_rng_get_single_word(&data);
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_rng_get_single_word failed");

        csi_test_atserver_send("%u,", data);
    }

	TEST_CASE_TIPS("test TRNG finish");
	return 0;
}



int test_trng_single_performance(void *args)
{
	uint32_t data = 0;
    uint32_t number_of_loop = 0;
    uint32_t params[1];
    uint32_t time_ms;
    float performance;
    int ret_args=0;
    ret_args = args_parsing(args, params, 1);
    TEST_CASE_ASSERT_QUIT(ret_args==0,"number of argtments is wrong.");
    number_of_loop = params[0];
    uint32_t i;
    tst_timer_restart();
    for(i=1;i<=number_of_loop;i++){
        csi_rng_get_single_word(&data);
        TEST_CASE_TIPS("number_of_loop:%d\n",i);
    }

    time_ms = tst_timer_get_interval();
    performance = (1.0*number_of_loop)/time_ms;

	TEST_CASE_TIPS("test TRNG Performance value is %f times/ms",performance);

	return 0;
}


int test_trng_multi_performance(void *args)
{
    uint32_t ret = 0;
    uint32_t number_of_words = 0;
    uint32_t number_of_loop = 0;
    uint32_t time_ms;
    float performance;

    uint32_t params[2];
    int ret_args=0;

    ret_args = args_parsing(args, params, 2);
    TEST_CASE_ASSERT_QUIT(ret_args==0,"number of argtments is wrong.");

    number_of_words = params[0];
    number_of_loop = params[1];

    uint32_t i; 
    tst_timer_restart();
    for(i=1;i<=number_of_loop;i++){
        ret = csi_rng_get_multi_word(ndata, number_of_words);
        TEST_CASE_ASSERT_QUIT(ret==0,"number of argtments is wrong in loop %d\n", i);
    }

    time_ms = tst_timer_get_interval();
    performance = (1.0*number_of_loop)/time_ms;

	TEST_CASE_TIPS("test TRNG Performance value is %f times/ms",performance);

	return 0;
}

int test_trng_interface(void *args){
    uint32_t ret = 0;
    uint32_t data = 100;

    ret = csi_rng_get_single_word(NULL);
	TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_rng_get_single_word should return error, but return:%d", ret);

    ret = csi_rng_get_single_word(0);
	TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_rng_get_single_word should return error, but return:%d", ret);

    ret = csi_rng_get_multi_word(NULL, 0);
	TEST_CASE_ASSERT(ret==CSI_ERROR,"csi_rng_get_multi_word should return error, but return:%d", ret);

    ret = csi_rng_get_multi_word(NULL, 100);
	TEST_CASE_ASSERT(ret==CSI_ERROR,"csi_rng_get_multi_word should return error, but return:%d", ret);


    ret = csi_rng_get_multi_word(&data, 0);
	TEST_CASE_ASSERT(ret==CSI_ERROR,"csi_rng_get_multi_word should return error, but return:%d", ret);

    return 0;
}

// /*-------------------------------------------------------------------------------------*/
int test_trng_single_stability(void *args)
{
    uint32_t ret = 0;
	uint32_t data = 0;
    uint32_t number_of_loop = 0;
    uint32_t params[1];
    uint32_t time_ms;
    float performance;
    int ret_args=0;
    ret_args = args_parsing(args, params, 1);
    TEST_CASE_ASSERT_QUIT(ret_args==0,"number of argtments is wrong.");
    number_of_loop = params[0];
    uint32_t i;
    tst_timer_restart();
    for(i=1;i<=number_of_loop;i++){
        ret = csi_rng_get_single_word(&data);
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_rng_get_single_word failed");

        TEST_CASE_TIPS("[loop:%d,trng:%u],",i,data);
    }

    time_ms = tst_timer_get_interval();
    performance = (1.0*number_of_loop)/time_ms;

	TEST_CASE_TIPS("Test trng single stability finished,test TRNG Performance value is %f times/ms",performance);

	return 0;
}

int test_trng_multi_stability(void *args)
{
	uint32_t ret = 0;
    uint32_t number_of_words = 0;
    uint32_t number_of_loop = 0;
    uint32_t time_ms;
    float performance;

    uint32_t params[2];
    int ret_args=0;

    ret_args = args_parsing(args, params, 2);
    TEST_CASE_ASSERT_QUIT(ret_args==0,"number of argtments is wrong.");

    number_of_words = params[0];
    number_of_loop = params[1];

    uint32_t i,j; 
    tst_timer_restart();
    for(i=1;i<=number_of_loop;i++){
        ret = csi_rng_get_multi_word(ndata, number_of_words);
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_rng_get_single_word failed");

        csi_test_atserver_send("\r\ndata:%d[", i);

        for(j=1;j<=number_of_words;j++){
            csi_test_atserver_send("%u,", ndata[j-1]);
        }
        csi_test_atserver_send("]");
    }

    time_ms = tst_timer_get_interval();
    performance = (1.0*number_of_loop)/time_ms;

	TEST_CASE_TIPS("Test trng multi stability finished,test TRNG Performance value is %f times/ms",performance);

	return 0;    
}