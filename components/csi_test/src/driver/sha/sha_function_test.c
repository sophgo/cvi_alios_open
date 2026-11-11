/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */



#include "sha_test.h" 


extern uint8_t sha_enc_data_sha1[];
extern uint8_t sha_enc_data_sha256[];
extern uint8_t sha_enc_data_sha224[];
extern uint8_t sha_enc_data_sha384[];
extern uint8_t sha_enc_data_sha512[];
extern uint8_t sha_enc_data_sha512_224[];
extern uint8_t sha_enc_data_sha512_256[];

extern uint8_t sha_src_data[SRC_DATA_LEN];
extern uint8_t sha_src_data_sub1[SRC_DATA_SUB1_LEN];
extern uint8_t sha_src_data_sub2[SRC_DATA_SUB2_LEN];
extern uint8_t sha_src_data_sub3[SRC_DATA_SUB3_LEN];

extern uint8_t sha_output_data[SHA_ENC_DATA_LEN_MAX];

static csi_sha_context_t test_sha_context;


int set_arg_by_mode(test_sha_args_t* arg)
{
    switch(arg->mode)
    {
        case SHA_MODE_1:        arg->sha_ref_len = 20; arg->sha_ref_data = sha_enc_data_sha1;       return 0;
        case SHA_MODE_224:	    arg->sha_ref_len = 28; arg->sha_ref_data = sha_enc_data_sha224;     return 0;
        case SHA_MODE_256:	    arg->sha_ref_len = 32; arg->sha_ref_data = sha_enc_data_sha256;     return 0;
        case SHA_MODE_512:	    arg->sha_ref_len = 48; arg->sha_ref_data = sha_enc_data_sha512;     return 0;
        case SHA_MODE_384:	    arg->sha_ref_len = 64; arg->sha_ref_data = sha_enc_data_sha384;     return 0;
        case SHA_MODE_512_256:	arg->sha_ref_len = 28; arg->sha_ref_data = sha_enc_data_sha512_256; return 0;
        case SHA_MODE_512_224:	arg->sha_ref_len = 32; arg->sha_ref_data = sha_enc_data_sha512_224; return 0;
        default:
            return 1;
    }
}


//cmd: AT+SHA_SYNC=idx,mode;
int test_sha_sync(void *args){

    uint32_t ret;
    test_sha_args_t sha_arg;
    csi_sha_t test_sha;
    uint32_t out_sha_len;
    sha_arg.idx   = *((uint64_t *)args);
    sha_arg.mode  = *((uint64_t *)args + 1);

    ret = set_arg_by_mode(&sha_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "SHA Mode (%u) error, this mode not supported", sha_arg.mode);

    memset(&test_sha_context,0, sizeof(test_sha_context)); //clean context cache
    memset(sha_output_data,0,SHA_ENC_DATA_LEN_MAX);  //clean output buffer

    TEST_PRINT_HEX("SOURCE DATA:", sha_src_data, SRC_DATA_LEN );
    
    ret = csi_sha_init(&test_sha, sha_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_init failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_start(&test_sha, &test_sha_context, sha_arg.mode);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_start failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_update(&test_sha,  &test_sha_context, sha_src_data, SRC_DATA_LEN);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);
    
    ret = csi_sha_finish(&test_sha, &test_sha_context, sha_output_data, &(out_sha_len));
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_finish failed, should return CSK_OK, but returned %d", ret);
    TEST_CASE_ASSERT_QUIT(sha_arg.sha_ref_len==out_sha_len,"Output SAH length should be %d, but returned %d", sha_arg.sha_ref_len, out_sha_len);

    for (uint8_t i=0; i<sha_arg.sha_ref_len; i++) {
        if (sha_output_data[i] != sha_arg.sha_ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }
    TEST_PRINT_HEX("ENCRYPTED DATA:",sha_output_data,sha_arg.sha_ref_len);

    csi_sha_uninit(&test_sha);
    
    return 0;
}

//cmd: AT+SHA_SYNC_MULTI=idx,mode;
int test_sha_sync_multi(void *args){

    uint32_t ret;
    test_sha_args_t sha_arg;
    csi_sha_t test_sha;
    uint32_t out_sha_len;
    sha_arg.idx   = *((uint64_t *)args);
    sha_arg.mode  = *((uint64_t *)args + 1);

    ret = set_arg_by_mode(&sha_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "SHA Mode (%u) error, this mode not supported", sha_arg.mode);

    memset(&test_sha_context,0, sizeof(test_sha_context)); //clean context cache
    memset(sha_output_data,0,SHA_ENC_DATA_LEN_MAX); //clean output buffer

    TEST_PRINT_HEX("SOURCE DATA:", sha_src_data, SRC_DATA_LEN );
    
    ret = csi_sha_init(&test_sha, sha_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_init failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_start(&test_sha, &test_sha_context, sha_arg.mode);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_start failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_update(&test_sha,  &test_sha_context, sha_src_data_sub1, SRC_DATA_SUB1_LEN);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);
    
    ret = csi_sha_update(&test_sha,  &test_sha_context, sha_src_data_sub2, SRC_DATA_SUB2_LEN);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_update(&test_sha,  &test_sha_context, sha_src_data_sub3, SRC_DATA_SUB3_LEN);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_finish(&test_sha, &test_sha_context, sha_output_data, &(out_sha_len));
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_finish failed, should return CSK_OK, but returned %d", ret);
    TEST_CASE_ASSERT_QUIT(sha_arg.sha_ref_len==out_sha_len,"Output SAH length should be %d, but returned %d", sha_arg.sha_ref_len, out_sha_len);

    for (uint8_t i=0; i<sha_arg.sha_ref_len; i++) {
        if (sha_output_data[i] != sha_arg.sha_ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }
    TEST_PRINT_HEX("ENCRYPTED DATA:",sha_output_data,sha_arg.sha_ref_len);

    csi_sha_uninit(&test_sha);
    
    return 0;
}


//cmd: AT+SHA_SYNC_PERF=idx,mode,loops;
/* performance test will not check the correctness of the output
 * In order to achinve different input data for each SHA calculation around
 * the test will use the output sha from privious round as the input to the next round SHA calculation
 */
int test_sha_sync_perf(void *args){

    uint32_t ret;
    test_sha_args_t sha_arg;
    csi_sha_t test_sha;
    sha_arg.idx   = *((uint64_t *)args);
    sha_arg.mode  = *((uint64_t *)args + 1);
    uint32_t number_of_loop = *((uint64_t *)args + 2) *1000;

    uint32_t i ;
    uint32_t time_ms;
    float performance;

    ret = set_arg_by_mode(&sha_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "SHA Mode (%u) error, this mode not supported", sha_arg.mode);

    memset(&test_sha_context,0, sizeof(test_sha_context)); //clean context cache
    memcpy(sha_arg.sha_ref_data,sha_output_data,sha_arg.sha_ref_len);  //fill output buffer

    TEST_PRINT_HEX("SOURCE DATA:", sha_src_data, SRC_DATA_LEN );

    ret = csi_sha_init(&test_sha, sha_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_init failed, should return CSK_OK, but returned %d", ret);

    tst_timer_restart();
    for (i = 1; i <= number_of_loop; i++) {

        ret = csi_sha_start(&test_sha, &test_sha_context, sha_arg.mode);
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_start failed, should return CSK_OK, but returned %d", ret);

        
        ret = csi_sha_update(&test_sha,  &test_sha_context, sha_output_data, sha_arg.sha_ref_len);
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);


        ret = csi_sha_finish(&test_sha, &test_sha_context, sha_output_data, &(sha_arg.sha_ref_len));
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_finish failed, should return CSK_OK, but returned %d", ret);
    }
    time_ms = tst_timer_get_interval();
    performance = number_of_loop * 1000.0 / time_ms;

    TEST_PRINT_HEX("ENCRYPTED DATA:",sha_output_data,sha_arg.sha_ref_len);
    TEST_CASE_TIPS("SHA performance is %f encryption cycles/second",performance);

    csi_sha_uninit(&test_sha);
    
    return 0;
}


#ifdef CSI_SHA_ASYNC

volatile static uint8_t test_sha_cb_flag = 1;

static void sha_event_cb_func(csi_sha_t *sha, csi_sha_event_t event, void *arg)
{
    if (SHA_EVENT_COMPLETE == event) {
        test_sha_cb_flag--;
    }
    TEST_CASE_TIPS("sha_cb_fun event %u", event);
}


//cmd: AT+SHA_ASYNC=idx,mode;
int test_sha_async(void *args){

    uint32_t ret;
    test_sha_args_t sha_arg;
    csi_sha_t test_sha;
    sha_arg.idx   = *((uint64_t *)args);
    sha_arg.mode  = *((uint64_t *)args + 1);

    ret = set_arg_by_mode(&sha_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "SHA Mode (%u) error, this mode not supported", sha_arg.mode);

    memset(&test_sha_context,0, sizeof(test_sha_context)); //clean context cache
    memset(sha_output_data,0,SHA_ENC_DATA_LEN_MAX);  //clean output buffer

    TEST_PRINT_HEX("SOURCE DATA:", sha_src_data, SRC_DATA_LEN );
    
    ret = csi_sha_init(&test_sha, sha_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_init failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_attach_callback(&test_sha, sha_event_cb_func, NULL);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_attach_callback failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_start(&test_sha, &test_sha_context, sha_arg.mode);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_start failed, should return CSK_OK, but returned %d", ret);

    test_sha_cb_flag ++;
    ret = csi_sha_update_async(&test_sha,  &test_sha_context, sha_src_data, SRC_DATA_LEN);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update_async failed, should return CSK_OK, but returned %d", ret);
    
    ret = csi_sha_finish(&test_sha, &test_sha_context, sha_output_data, &(out_sha_len));
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_finish failed, should return CSK_OK, but returned %d", ret);
    TEST_CASE_ASSERT_QUIT(sha_arg.sha_ref_len==out_sha_len,"Output SAH length should be %d, but returned %d", sha_arg.sha_ref_len, out_sha_len);

    while (test_sha_cb_flag){}; 

    for (uint8_t i=0; i<sha_arg.sha_ref_len; i++) {
        if (sha_output_data[i] != sha_arg.sha_ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }
    TEST_PRINT_HEX("ENCRYPTED DATA:",sha_output_data,sha_arg.sha_ref_len);

    csi_sha_uninit(&test_sha);
    
    return 0;
}

//cmd: AT+SHA_ASYNC_MULTI=idx,mode;
int test_sha_async_multi(void *args){

    uint32_t ret;
    test_sha_args_t sha_arg;
    csi_sha_t test_sha;
    sha_arg.idx   = *((uint64_t *)args);
    sha_arg.mode  = *((uint64_t *)args + 1);

    ret = set_arg_by_mode(&sha_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "SHA Mode (%u) error, this mode not supported", sha_arg.mode);

    memset(&test_sha_context,0, sizeof(test_sha_context)); //clean context cache
    memset(sha_output_data,0,SHA_ENC_DATA_LEN_MAX); //clean output buffer

    TEST_PRINT_HEX("SOURCE DATA:", sha_src_data, SRC_DATA_LEN );
    
    ret = csi_sha_init(&test_sha, sha_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_init failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_attach_callback(&test_sha, sha_event_cb_func, NULL);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_attach_callback failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_start(&test_sha, &test_sha_context, sha_arg.mode);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_start failed, should return CSK_OK, but returned %d", ret);

    test_sha_cb_flag ++;
    ret = csi_sha_update_async(&test_sha,  &test_sha_context, sha_src_data_sub1, SRC_DATA_SUB1_LEN);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);
    
    test_sha_cb_flag ++;
    ret = csi_sha_update_async(&test_sha,  &test_sha_context, sha_src_data_sub2, SRC_DATA_SUB2_LEN);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);

    test_sha_cb_flag ++;
    ret = csi_sha_update_async(&test_sha,  &test_sha_context, sha_src_data_sub3, SRC_DATA_SUB3_LEN);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);

    ret = csi_sha_finish(&test_sha, &test_sha_context, sha_output_data, &(out_sha_len));
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_finish failed, should return CSK_OK, but returned %d", ret);
    TEST_CASE_ASSERT_QUIT(sha_arg.sha_ref_len==out_sha_len,"Output SAH length should be %d, but returned %d", sha_arg.sha_ref_len, out_sha_len);

    while (test_sha_cb_flag){};
    
    for (uint8_t i=0; i<sha_arg.sha_ref_len; i++) {
        if (sha_output_data[i] != sha_arg.sha_ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }
    TEST_PRINT_HEX("ENCRYPTED DATA:",sha_output_data,sha_arg.sha_ref_len);

    csi_sha_uninit(&test_sha);
    
    return 0;
}

//cmd: AT+SHA_ASYNC_PERF=idx,mode,loops;
int test_sha_sync_perf(void *args){

    uint32_t ret;
    test_sha_args_t sha_arg;
    csi_sha_t test_sha;
    sha_arg.idx   = *((uint64_t *)args);
    sha_arg.mode  = *((uint64_t *)args + 1);
    uint32_t number_of_loop = *((uint64_t *)args + 2) *1000;

    uint32_t i ;
    uint32_t time_ms;
    float performance;

    ret = set_arg_by_mode(&sha_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "SHA Mode (%u) error, this mode not supported", sha_arg.mode);

    memset(&test_sha_context,0, sizeof(test_sha_context)); //clean context cache
    memcpy(sha_arg.sha_ref_data,sha_output_data,sha_arg.sha_ref_len);  //fill output buffer

    TEST_PRINT_HEX("SOURCE DATA:", sha_src_data, SRC_DATA_LEN );


    ret = csi_sha_init(&test_sha, sha_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_init failed, should return CSK_OK, but returned %d", ret);

    tst_timer_restart();
    for (i = 1; i <= number_of_loop; i++) {


        ret = csi_sha_start(&test_sha, &test_sha_context, sha_arg.mode);
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_start failed, should return CSK_OK, but returned %d", ret);

        
        ret = csi_sha_update_async(&test_sha,  &test_sha_context, sha_output_data, sha_arg.sha_ref_len);
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_update failed, should return CSK_OK, but returned %d", ret);


        ret = csi_sha_finish(&test_sha, &test_sha_context, sha_output_data, &(sha_arg.sha_ref_len));
        TEST_CASE_ASSERT_QUIT(ret==CSI_OK,"csi_sha_finish failed, should return CSK_OK, but returned %d", ret);
    }
    time_ms = tst_timer_get_interval();
    performance = number_of_loop * 1000.0 / time_ms;

    TEST_PRINT_HEX("ENCRYPTED DATA:",sha_output_data,sha_arg.sha_ref_len);
    TEST_CASE_TIPS("SHA performance is %f encryption cycles/second",performance);

    csi_sha_uninit(&test_sha);
    
    return 0;
}

#endif
