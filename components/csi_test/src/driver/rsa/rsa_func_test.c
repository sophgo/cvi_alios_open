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


#include "rsa_test.h"
int j=0;
extern uint8_t rsa_temp[RSA_SOURCE_DATA_LEN];
extern test_rsa_args_t rsa_test_param;
volatile uint32_t async_verify_flag = 0;
volatile uint32_t rsa_event_finish = 0;
static void rsa_test_cb(csi_rsa_t *rsa, csi_rsa_event_t event, void *arg)
{

    switch (event) {
        case RSA_EVENT_COMPLETE:
            rsa_event_finish = 0;
            break;

        case RSA_EVENT_VERIFY_SUCCESS:
            async_verify_flag = 0;
            break;

        case RSA_EVENT_VERIFY_FAILED:
            break;
        case RSA_EVENT_ERROR:   
            break;
    }

}

static int get_rsa_test_params(char *args, csi_rsa_context_t *context){
    sscanf(args, "%d,%u,%u,%u,", &rsa_test_param.idx, \
                                    &rsa_test_param.key_bits, \
                                    &rsa_test_param.padding_type, \
                                    &rsa_test_param.hash_type 
                                    );

    context->key_bits = rsa_test_param.key_bits;
    context->padding_type =  rsa_test_param.padding_type; //RSA_PADDING_MODE_PKCS1;
    
    switch (rsa_test_param.key_bits)
    {
    case RSA_KEY_BITS_1024:
        context->d = (void *) rsa_d1024;
        context->e =(void *)  rsa_e1024;
        context->n = (void *) rsa_n1024;
        rsa_test_param.data_len = 128;
        break;
    case RSA_KEY_BITS_2048:
        context->d = (void *) rsa_d2048;
        context->e = (void *) rsa_e2048;
        context->n =(void *)  rsa_n2048;
        rsa_test_param.data_len = 256;
        break;
    default:
        TEST_CASE_ASSERT_QUIT(1==0,"RSA test only support key length 1024 & 2048 now");
        return -1;
        break;
    }
    return 0;
}

//cmd: AT+RSA_SET_KEY=key_type,key_data,length_of_source(units: bytes)
//AT+RSA_SET_KEY=0,eeaabb2266ffeeaabb2266ff,12
//key type:  0- d value,  1- e value,  2- n value
int test_set_rsa_key(void *args){
    int key_type = 0;
    int length_of_source =0;  
    uint8_t * key_buffer_p=NULL;
    sscanf(args, "%d,%[^,],%d",&key_type,rsa_received_data_from_host,&length_of_source);
    switch (key_type)
    {
    case 0:  
        key_buffer_p = rsa_test_key_d;
        TEST_CASE_TIPS("SET for RSA key: d value");
        break;
    case 1:
        key_buffer_p = rsa_test_key_e;
        TEST_CASE_TIPS("SET for RSA key: e value");
        break;
    case 2:
        key_buffer_p = rsa_test_key_n;
        TEST_CASE_TIPS("SET for RSA key: n value");
        break;
    default:
        TEST_CASE_ASSERT(1==0, "KEY TYPE Only support: 0- d value,  1- e value,  2- n value");
        break;
    }
    if (key_buffer_p != NULL){
        hex_to_array(rsa_received_data_from_host, length_of_source, key_buffer_p);
        TEST_PRINT_HEX("KEY VALUE:",key_buffer_p, length_of_source);
    }
    return 0;
}

//cmd: AT+RSA_ENCRYPT_SYNC=idx,key_bits,padding_type,hash_type,source_data,length_of_source(units: bytes)
//AT+RSA_ENCRYPT_SYNC=0,3,1,2,eeaabb2266ffeeaabb2266ff,12
int test_rsa_encrypt_sync(void *args){
    int length_of_source;
    uint32_t output_length = 32;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    csi_rsa_state_t status;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%[^,],%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&length_of_source);
    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);
    TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);
    ret = csi_rsa_encrypt(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
    TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_encrypt should return %d, but returned %d ", CSI_OK, ret);

    while (1) {
       csi_rsa_get_state(&rsa, &status);
       if (status.busy == 0) {
           break;
       }
    }

    TEST_PRINT_HEX("ENCRYPTED DATA:", rsa_target_data, rsa_test_param.data_len);

    // memset(rsa_source_data,0x00,rsa_test_param.data_len);
    ret = csi_rsa_decrypt(&rsa, &context, rsa_target_data, rsa_test_param.data_len, (void *)rsa_temp, &output_length);
    TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_decrypt should return %d, but returned %d ", CSI_OK, ret);
    output_length = length_of_source;
    TEST_PRINT_HEX("DECRYPTED DATA:", rsa_temp, output_length);

    ret = memcmp(rsa_source_data,rsa_temp,output_length);
    TEST_CASE_ASSERT_QUIT(ret == 0,"The result of encrypting is error...\n");
    csi_rsa_uninit(&rsa);
    return 0;
}

//AT+RSA_DECRYPT_SYNC=0,3,1,2,8fdd213897aa1e4f6b543252bc962111b1527fc7d30086febafaec73a7901da672ee9bcfb21a0184b81f15126da4fa8f547fd07c48d33c2fcba15c9471711888a1ed1223ed42cd55aa25adf3f1a5e9a3993cf30f1308b1f26a8bfa3cfe1a791f709626c6358f0ad88fdcdc3e44d615c2ff409415d06699c4e4200f8dc01f96df,128
int test_rsa_decrypt_sync(void *args){
    int length_of_source;
    uint32_t output_length = 32;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    csi_rsa_state_t status;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%[^,],%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&length_of_source);
    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);
    TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
    
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);

    output_length = length_of_source;
    ret = csi_rsa_decrypt(&rsa, &context, rsa_source_data, length_of_source, (void *)rsa_target_data, &output_length);
    TEST_PRINT_HEX("DECRYPTED DATA:", rsa_target_data, output_length);

    while (1) {
       csi_rsa_get_state(&rsa, &status);
       if (status.busy == 0) {
           break;
       }
    }    

    csi_rsa_uninit(&rsa);
    return 0;
}


//AT+RSA_ENCRYPT_ASYNC=0,3,1,2,eeaabb2266ffeeaabb2266ff,12
int test_rsa_encrypt_async(void *args){
    int length_of_source;
    uint32_t output_length = 32;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%[^,],%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&length_of_source);
    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);
    TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
    
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);

    csi_rsa_attach_callback(&rsa, rsa_test_cb, NULL);
    rsa_event_finish = 1;
    ret = csi_rsa_encrypt_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
    TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_encrypt_async should return %d, but returned %d ", CSI_OK, ret); 
    while (rsa_event_finish) ;

    TEST_PRINT_HEX("ENCRYPTED DATA:", rsa_target_data, rsa_test_param.data_len);

    rsa_event_finish = 1;
    ret = csi_rsa_decrypt_async(&rsa, &context, rsa_target_data, rsa_test_param.data_len, (void *)rsa_temp, &output_length);
    TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_decrypt_async should return %d, but returned %d ", CSI_OK, ret);
    while (rsa_event_finish) ;
    output_length = length_of_source;
    TEST_PRINT_HEX("DECRYPTED DATA:", rsa_temp, output_length);

    ret = memcmp(rsa_source_data,rsa_temp,output_length);
    TEST_CASE_ASSERT_QUIT(ret == 0,"The result of asyn_encrypting is error...\n");

    csi_rsa_detach_callback(&rsa);
    csi_rsa_uninit(&rsa);
    return 0;
}


//AT+RSA_DECRYPT_ASYNC=0,3,1,2,8fdd213897aa1e4f6b543252bc962111b1527fc7d30086febafaec73a7901da672ee9bcfb21a0184b81f15126da4fa8f547fd07c48d33c2fcba15c9471711888a1ed1223ed42cd55aa25adf3f1a5e9a3993cf30f1308b1f26a8bfa3cfe1a791f709626c6358f0ad88fdcdc3e44d615c2ff409415d06699c4e4200f8dc01f96df,128
int test_rsa_decrypt_async(void *args){
    int length_of_source;
    uint32_t output_length = 32;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%[^,],%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&length_of_source);
    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);
    TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
    
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);
    csi_rsa_attach_callback(&rsa, rsa_test_cb, NULL);

    rsa_event_finish = 1;
    ret = csi_rsa_decrypt_async(&rsa, &context, rsa_source_data, length_of_source, (void *)rsa_target_data, &output_length);
    while (rsa_event_finish) ;
    TEST_PRINT_HEX("DECRYPTED DATA:", rsa_target_data, output_length);

    csi_rsa_detach_callback(&rsa);
    csi_rsa_uninit(&rsa);
    return 0;
}



//cmd: AT+RSA_ENCRYPT_DECRYPT_SYNC_PERFORMANCE=idx,key_bits,padding_type,hash_type,source_data,length_of_source(units: bytes),test_loops
//AT+RSA_ENCRYPT_DECRYPT_SYNC_PERFORMANCE=0,3,1,2,eeaabb2266ffeeaabb2266ff,12,100
int test_rsa_encrypt_decrypt_sync_performance(void *args){
    uint8_t rsa_enc_data[RSA_SOURCE_DATA_LEN];
    int length_of_source;
    uint32_t output_length = 32;
    uint8_t rand_data = 0;
    int test_loops=0;
    float performance = 0.0;
    uint32_t elapsed_ms = 0;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
	csi_rsa_state_t status;
    csi_rsa_context_t context;


    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    // sscanf(args, "%d,%d,%d,%d,%[^,],%d,%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&length_of_source, &test_loops);
    sscanf(args, "%d,%d,%d,%d,%c,%d,%d",&dummy,&dummy,&dummy,&dummy,&rand_data,&length_of_source, &test_loops);

    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    // hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);

    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);
    switch (rand_data)
    {
    case 0:
        generate_rand_array(rsa_source_data,255,length_of_source);
        TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);

        tst_timer_restart();
        for (int i = 0; i<test_loops; i++){
            csi_rsa_encrypt(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
        }

        elapsed_ms =  tst_timer_get_interval() ;
        performance = 1.0 * test_loops/elapsed_ms;
        TEST_CASE_TIPS("Stable src data:RSA sync encrypt performance(csi_rsa_encrypt): %f times/ms", performance );

        while(1){
            csi_rsa_get_state(&rsa, &status);
            if (status.busy == 0) {
                break;
            }
        }

        generate_rand_array(rsa_enc_data,255,length_of_source);
        TEST_PRINT_HEX("SOURCE DATA:", rsa_enc_data, length_of_source);

        tst_timer_restart();
        for (int i=0; i<test_loops; i++){
            csi_rsa_decrypt(&rsa, &context, rsa_enc_data, rsa_test_param.data_len, (void *)rsa_target_data, &output_length);
        }
        elapsed_ms =  tst_timer_get_interval() ;
        performance =  1.0 * test_loops/elapsed_ms;
        TEST_CASE_TIPS("Stable dec data:RSA sync decrypt performance(csi_rsa_decrypt): %f times/ms", performance );
        break;
    case 1:
        tst_timer_restart();
        for (int i = 0; i<test_loops; i++){
            generate_rand_array(rsa_source_data,255,length_of_source);
            TEST_PRINT_HEX("SRC DATA:", rsa_source_data, length_of_source);
            csi_rsa_encrypt(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
        }

        elapsed_ms =  tst_timer_get_interval() ;
        performance = 1.0 * test_loops/elapsed_ms;
        TEST_CASE_TIPS("Changeable src data:RSA sync encrypt performance(csi_rsa_encrypt): %f times/ms", performance );

        while(1){
            csi_rsa_get_state(&rsa, &status);
            if (status.busy == 0) {
                break;
            }
        }

        tst_timer_restart();
        for (int i=0; i<test_loops; i++){
            generate_rand_array(rsa_enc_data,255,length_of_source);
            TEST_PRINT_HEX("DEC DATA:", rsa_enc_data, length_of_source);
            csi_rsa_decrypt(&rsa, &context, rsa_enc_data, rsa_test_param.data_len, (void *)rsa_target_data, &output_length);
        }
        elapsed_ms =  tst_timer_get_interval() ;
        performance =  1.0 * test_loops/elapsed_ms;
        TEST_CASE_TIPS("Changeable dec data:RSA sync decrypt performance(csi_rsa_decrypt): %f times/ms", performance );
        break;
    default:
        TEST_CASE_ASSERT(1 == 0,"RAND_DATA CHOOSE ERROR\n");
        break;
    }

    csi_rsa_uninit(&rsa);
    return 0;
}


//cmd: AT+RSA_ENCRYPT_DECRYPT_ASYNC_PERFORMANCE=idx,key_bits,padding_type,hash_type,source_data,length_of_source(units: bytes),test_loops
//AT+RSA_ENCRYPT_DECRYPT_ASYNC_PERFORMANCE=0,3,1,2,eeaabb2266ffeeaabb2266ff,12,100
int test_rsa_encrypt_decrypt_async_performance(void *args){
    uint8_t rsa_enc_data[RSA_SOURCE_DATA_LEN];
    uint8_t rand_data = 0;
    int length_of_source;
    int test_loops=0;
    csi_rsa_state_t status;
    uint32_t output_length = 32;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    uint32_t elapsed_ms = 0;
    float performance = 0.0;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%c,%d,%d",&dummy,&dummy,&dummy,&dummy,&rand_data,&length_of_source, &test_loops);
    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);
    TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
    
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);

    csi_rsa_attach_callback(&rsa, rsa_test_cb, NULL);
    switch (rand_data)
        {
        case 0:
            generate_rand_array(rsa_source_data,255,length_of_source);
            TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);

            tst_timer_restart();
            for (int i = 0; i<test_loops; i++){
                rsa_event_finish = 1;
                ret = csi_rsa_encrypt_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_encrypt_async should return %d, but returned %d ", CSI_OK, ret); 
                while (rsa_event_finish) ;
            }

            elapsed_ms =  tst_timer_get_interval() ;
            performance = 1.0 * test_loops/elapsed_ms;
            TEST_CASE_TIPS("Stable src data:RSA sync encrypt performance(csi_rsa_encrypt): %f times/ms", performance );

            while(1){
                csi_rsa_get_state(&rsa, &status);
                if (status.busy == 0) {
                    break;
                }
            }

            generate_rand_array(rsa_enc_data,255,length_of_source);
            TEST_PRINT_HEX("SOURCE DATA:", rsa_enc_data, length_of_source);

            tst_timer_restart();
            for (int i=0; i<test_loops; i++){
                rsa_event_finish = 1;
                ret = csi_rsa_decrypt_async(&rsa, &context, rsa_target_data, rsa_test_param.data_len, (void *)rsa_source_data, &output_length);
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_decrypt_async should return %d, but returned %d ", CSI_OK, ret);
                while (rsa_event_finish) ;
            }
            elapsed_ms =  tst_timer_get_interval() ;
            performance =  1.0 * test_loops/elapsed_ms;
            TEST_CASE_TIPS("Stable dec data:RSA sync decrypt performance(csi_rsa_decrypt): %f times/ms", performance );
            break;
        case 1:
            tst_timer_restart();
            for (int i = 0; i<test_loops; i++){
                generate_rand_array(rsa_source_data,255,length_of_source);
                TEST_PRINT_HEX("SRC DATA:", rsa_source_data, length_of_source);
                rsa_event_finish = 1;
                ret = csi_rsa_encrypt_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_encrypt_async should return %d, but returned %d ", CSI_OK, ret); 
                while (rsa_event_finish) ;
            }

            elapsed_ms =  tst_timer_get_interval() ;
            performance = 1.0 * test_loops/elapsed_ms;
            TEST_CASE_TIPS("Changeable src data:RSA sync encrypt performance(csi_rsa_encrypt): %f times/ms", performance );

            while(1){
                csi_rsa_get_state(&rsa, &status);
                if (status.busy == 0) {
                    break;
                }
            }

            tst_timer_restart();
            for (int i=0; i<test_loops; i++){
                generate_rand_array(rsa_enc_data,255,length_of_source);
                TEST_PRINT_HEX("DEC DATA:", rsa_enc_data, length_of_source);
                rsa_event_finish = 1;
                ret = csi_rsa_decrypt_async(&rsa, &context, rsa_target_data, rsa_test_param.data_len, (void *)rsa_source_data, &output_length);
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_decrypt_async should return %d, but returned %d ", CSI_OK, ret);
                while (rsa_event_finish) ;
            }
            elapsed_ms =  tst_timer_get_interval() ;
            performance =  1.0 * test_loops/elapsed_ms;
            TEST_CASE_TIPS("Changeable dec data:RSA sync decrypt performance(csi_rsa_decrypt): %f times/ms", performance );
            break;
        default:
            break;
        }
    csi_rsa_detach_callback(&rsa);
    csi_rsa_uninit(&rsa);
    return 0;
}


//cmd: AT+RSA_SIGN_VERIFY_SYNC=idx,key_bits,padding_type,hash_type,source_data,length_of_source(units: bytes)
//AT+RSA_SIGN_VERIFY_SYNC=0,3,1,3,eeaabb2266ffeeaabb2266ff,12
int test_rsa_sign_verify_sync(void *args){
    int length_of_source;
    int dummy;
    uint32_t sign_times = 0;
    uint32_t verify_times = 0;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    csi_rsa_state_t status;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%[^,],%d,%d,%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&sign_times,&verify_times,&length_of_source);
    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);
    TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);

    for(int i=0;i<sign_times;i++){
        ret = csi_rsa_sign(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
        TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_sign should return %d, but returned %d ", CSI_OK, ret);
        TEST_PRINT_HEX("SIGN DATA:", rsa_target_data, rsa_test_param.data_len);
    }
    
    while (1) {
        csi_rsa_get_state(&rsa, &status);
        if (status.busy == 0) {
            break;
        }
    }
    TEST_PRINT_HEX("SIGNED DATA:", rsa_target_data, rsa_test_param.data_len);
    for(int i=0;i<verify_times;i++){
        ret = csi_rsa_verify(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
        TEST_CASE_ASSERT(ret == true, "RSA verify failed.");
        TEST_PRINT_HEX("SIGNATURE DATA:", rsa_target_data, rsa_test_param.data_len);
    }
    
    csi_rsa_uninit(&rsa);
    return 0;
}


//cmd: AT+RSA_SIGN_VERIFY_ASYNC=idx,key_bits,padding_type,hash_type,source_data,length_of_source(units: bytes)
//AT+RSA_SIGN_VERIFY_ASYNC=0,3,1,3,eeaabb2266ffeeaabb2266ff,12
int test_rsa_sign_verify_async(void *args){
    int length_of_source;
    uint32_t sign_times = 0;
    uint32_t verify_times = 0;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    csi_rsa_state_t status;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%[^,],%d,%d,%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&sign_times,&verify_times,&length_of_source);
    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);
    TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);
    csi_rsa_attach_callback(&rsa, rsa_test_cb, NULL);

    for(int i=0;i<sign_times;i++){    
        ret = csi_rsa_sign_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
        TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_sign_async should return %d, but returned %d ", CSI_OK, ret);
    }

    while (1) {
        csi_rsa_get_state(&rsa, &status);
        if (status.busy == 0) {
            break;
        }
    }

    TEST_PRINT_HEX("SIGNED DATA:", rsa_target_data, rsa_test_param.data_len);
    for(int i=0;i<verify_times;i++){   
        ret = csi_rsa_verify_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
        TEST_CASE_ASSERT(ret == true, "RSA verify failed.");
    }
    while (1) {
        csi_rsa_get_state(&rsa, &status);
        if (status.busy == 0) {
            break;
        }
    }
    csi_rsa_detach_callback(&rsa);
    csi_rsa_uninit(&rsa);
    return 0;
}



//cmd: AT+RSA_SIGN_VERIFY_SYNC_PERFORMANCE=idx,key_bits,padding_type,hash_type,source_data,length_of_source(units: bytes),test_loops
//AT+RSA_SIGN_VERIFY_SYNC_PERFORMANCE=0,3,1,3,eeaabb2266ffeeaabb2266ff,12,100
int test_rsa_sign_verify_sync_performance(void *args){
    int length_of_source;
    uint32_t sign_times = 0;
    uint32_t verify_times = 0;
    uint8_t rand_data = 0;
    int test_loops=0;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    bool result;
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    csi_rsa_state_t status;
    uint32_t elapsed_ms = 0;
    float performance;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    // memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    // sscanf(args, "%d,%d,%d,%d,%[^,],%d,%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&length_of_source, &test_loops);
    sscanf(args, "%d,%d,%d,%d,%c,%d,%d,%d,%d",&dummy,&dummy,&dummy,&dummy,&rand_data,&sign_times,&verify_times,&length_of_source, &test_loops);
    TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    // hex_to_array(rsa_received_data_from_host, length_of_source, rsa_source_data);
    // TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);

    switch(rand_data){
        case 0:      
            generate_rand_array(rsa_source_data,255,length_of_source);
            TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
            for(j=0;j<test_loops;j++){
                tst_timer_restart();
                for (int i=0; i < sign_times; i++){
                    ret = csi_rsa_sign(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
                }
                elapsed_ms = tst_timer_get_interval();
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_sign should return %d, but returned %d ", CSI_OK, ret);
                performance = 1.0 * sign_times/elapsed_ms;
                TEST_CASE_TIPS("RSA sync sign performance(csi_rsa_sign): %f times/ms", performance );

                while (1) {
                    csi_rsa_get_state(&rsa, &status);
                    if (status.busy == 0) {
                        break;
                    }
                }

                // TEST_PRINT_HEX("SIGNED DATA:", rsa_target_data, rsa_test_param.data_len);

                tst_timer_restart();
                for (int i=0; i < verify_times; i++){
                    // memset(rsa_target_data, 0x00,rsa_test_param.data_len);
                    result = csi_rsa_verify(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
                    // TEST_PRINT_HEX("SIGNATURE DATA:", rsa_target_data, rsa_test_param.data_len);
                    TEST_CASE_ASSERT_QUIT(result == true, "RSA verify failed.");
                }
                elapsed_ms = tst_timer_get_interval();
                performance = 1.0 * verify_times/elapsed_ms;
                TEST_CASE_TIPS("RSA sync verify performance(csi_rsa_verify): %f times/ms", performance );
            }
            break;

        case 1:     
            for(j=0;j<test_loops;j++){
                for (int i=0; i < sign_times; i++){
                    memset(rsa_source_data,0x00,length_of_source);
                    generate_rand_array(rsa_source_data,255,length_of_source);
                    TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source); 
                    ret = csi_rsa_sign(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
                }
                elapsed_ms = tst_timer_get_interval();
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_sign should return %d, but returned %d ", CSI_OK, ret);
                
                performance = 1.0 * sign_times/elapsed_ms;
                TEST_CASE_TIPS("RSA sync sign performance(csi_rsa_sign): %f times/ms", performance );

                while (1) {
                    csi_rsa_get_state(&rsa, &status);
                    if (status.busy == 0) {
                        break;
                    }
                }

                // TEST_PRINT_HEX("SIGNED DATA:", rsa_target_data, rsa_test_param.data_len);

                tst_timer_restart();
                for (int i=0; i < verify_times; i++){
                    result = csi_rsa_verify(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
                    // TEST_PRINT_HEX("SIGNATURE DATA:", rsa_target_data, rsa_test_param.data_len);
                    TEST_CASE_ASSERT_QUIT(result == true, "RSA verify failed.");
                }
                elapsed_ms = tst_timer_get_interval();
                performance = 1.0 * verify_times/elapsed_ms;
                TEST_CASE_TIPS("RSA sync verify performance(csi_rsa_verify): %f times/ms", performance );
            }
            break;
        default:
            break;
    }

    csi_rsa_uninit(&rsa);
    return 0;
}

//cmd: AT+RSA_SIGN_VERIFY_ASYNC_PERFORMANCE=idx,key_bits,padding_type,hash_type,source_data,length_of_source(units: bytes),test_loops
//AT+RSA_SIGN_VERIFY_ASYNC_PERFORMANCE=0,3,1,3,eeaabb2266ffeeaabb2266ff,12,100
int test_rsa_sign_verify_async_performance(void *args){
    int length_of_source;
    csi_rsa_state_t status;
    int test_loops;
    uint32_t sign_times = 0;
    uint32_t verify_times = 0;
    uint8_t rand_data = 0;
    int dummy;
	int i,j;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    uint32_t elapsed_ms = 0;
    float performance;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    // memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    // sscanf(args, "%d,%d,%d,%d,%[^,],%d,%d",&dummy,&dummy,&dummy,&dummy,rsa_received_data_from_host,&length_of_source,&test_loops);
    sscanf(args, "%d,%d,%d,%d,%c,%d,%d,%d,%d",&dummy,&dummy,&dummy,&dummy,&rand_data,&sign_times,&verify_times,&length_of_source,&test_loops);
    // TEST_CASE_TIPS("received source data: %s\n", rsa_received_data_from_host);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);
    csi_rsa_attach_callback(&rsa, rsa_test_cb, NULL);

    switch (rand_data)
    {
    case 0:
        generate_rand_array(rsa_source_data,255,length_of_source);
        TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
        for(j=0;j<test_loops;j++){
            rsa_event_finish=1;
            tst_timer_restart();
            for (i =0;i<sign_times;i++){
                ret = csi_rsa_sign_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
                while (rsa_event_finish);
                while (1) {
                    csi_rsa_get_state(&rsa, &status);
                    if (status.busy == 0) {
                        break;
                    }
                }
            }
            elapsed_ms = tst_timer_get_interval();
            TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_sign_async should return %d, but returned %d ", CSI_OK, ret);
            performance = 1.0 * sign_times/elapsed_ms;
            TEST_CASE_TIPS("RSA async sign performance(csi_rsa_sign_async): %f times/ms", performance ); 

            rsa_event_finish=1;
            async_verify_flag = 1;
            tst_timer_restart();
            for (i =0;i<verify_times;i++){
                ret = csi_rsa_verify_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
                TEST_CASE_ASSERT(ret == CSI_OK, "RSA verify failed.");
                while(async_verify_flag);
                while (1) {
                    csi_rsa_get_state(&rsa, &status);
                    if (status.busy == 0) {
                        break;
                    }
                }
            }
            
            elapsed_ms = tst_timer_get_interval();
            performance = 1.0 * verify_times/elapsed_ms;
            TEST_CASE_TIPS("RSA async verify performance(csi_rsa_verify_async): %f times/ms", performance );
        }
        break;
    case 1:
        for( j=0;j<test_loops;j++){
            rsa_event_finish=1;
            tst_timer_restart();
            generate_rand_array(rsa_source_data,255,length_of_source);
            TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
            for (i =0;i<sign_times;i++){
                ret = csi_rsa_sign_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
                while (rsa_event_finish);
                while (1) {
                    csi_rsa_get_state(&rsa, &status);
                    if (status.busy == 0) {
                        break;
                    }
                }
            }
            elapsed_ms = tst_timer_get_interval();
            TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_sign_async should return %d, but returned %d ", CSI_OK, ret);
            performance = 1.0 * sign_times/elapsed_ms;
            TEST_CASE_TIPS("RSA async sign performance(csi_rsa_sign_async): %f times/ms", performance ); 

            rsa_event_finish=1;
            tst_timer_restart();
            for (i =0;i<verify_times;i++){
            	async_verify_flag = 1;
                ret = csi_rsa_verify_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
                TEST_CASE_ASSERT(ret == CSI_OK, "RSA verify failed.");
                while(async_verify_flag);
                while (1) {
                    csi_rsa_get_state(&rsa, &status);
                    if (status.busy == 0) {
                        break;
                    }
                }
            }
            
            elapsed_ms = tst_timer_get_interval();
            performance = 1.0 * verify_times/elapsed_ms;
            TEST_CASE_TIPS("RSA async verify performance(csi_rsa_verify_async): %f times/ms", performance );
        }
        break;
    default:
        TEST_CASE_ASSERT(1 == 0,"RAND_DATA CHOOSE ERROR\n");
        break;
    }
    csi_rsa_detach_callback(&rsa);
    csi_rsa_uninit(&rsa);
    return 0;
}



int rsa_encrypt_decrypt_sync_stability(void *args)
{
    int length_of_source;
    uint32_t output_length = 32;
    int test_loops=0;
    int dummy;
    csi_rsa_state_t status;
    uint32_t encrypt_times = 0;
    uint32_t decrypt_times = 0;
    uint8_t rand_data = 0;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    uint8_t rsa_dec_data[RSA_SOURCE_DATA_LEN];

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%c,%d,%d,%d,%d",&dummy,&dummy,&dummy,&dummy,&rand_data,&encrypt_times,&decrypt_times,&length_of_source, &test_loops);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);

    switch (rand_data)
    {
    case 0:
        generate_rand_array(rsa_source_data,255,length_of_source);
        TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);

        for (j = 0; j<test_loops; j++){
            for(int i=0; i<encrypt_times;i++){
                ret = csi_rsa_encrypt(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_encrypt_sync should return %d, but returned %d ", CSI_OK, ret);
                TEST_PRINT_HEX("ENCRYPTED DATA:", rsa_target_data, rsa_test_param.data_len);
            }

            while(1){
                csi_rsa_get_state(&rsa, &status);
                if (status.busy == 0) {
                    break;
                }
            }

            for(int i=0;i<decrypt_times;i++){
                ret = csi_rsa_decrypt(&rsa, &context, rsa_target_data, rsa_test_param.data_len, rsa_dec_data, &output_length);
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_decrypt_sync should return %d, but returned %d ", CSI_OK, ret);
                TEST_PRINT_HEX("DECRYPTED DATA:", rsa_dec_data, output_length);
            }

            ret = memcmp(rsa_source_data,rsa_dec_data,output_length);
            TEST_CASE_ASSERT_QUIT(ret==0,"The result of this time is not equal to last time\n");
            TEST_CASE_TIPS("Test loop:%d finished!\n",j);
        }
        break;
    case 1:
        for (j = 0; j<test_loops; j++){
            generate_rand_array(rsa_source_data,255,length_of_source);
            TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
            for(int i=0; i<encrypt_times;i++){
                ret = csi_rsa_encrypt(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_encrypt_sync should return %d, but returned %d ", CSI_OK, ret);
                TEST_PRINT_HEX("ENCRYPTED DATA:", rsa_target_data, rsa_test_param.data_len);
            }
            while(1){
                csi_rsa_get_state(&rsa, &status);
                if (status.busy == 0) {
                    break;
                }
            }

            for(int i=0;i<decrypt_times;i++){
                ret = csi_rsa_decrypt(&rsa, &context, rsa_target_data, rsa_test_param.data_len, rsa_dec_data, &output_length);
                TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_decrypt_sync should return %d, but returned %d ", CSI_OK, ret);
                TEST_PRINT_HEX("DECRYPTED DATA:", rsa_dec_data, output_length);
            }

            ret = memcmp(rsa_source_data,rsa_dec_data,output_length);
            TEST_CASE_ASSERT_QUIT(ret==0,"The result of this time is not equal to last time\n");
            TEST_CASE_TIPS("Test loop:%d finished!\n",j);
            memset(rsa_source_data,0x00,length_of_source);
        }
        break;
    default:
        TEST_CASE_ASSERT(1 == 0,"RAND_DATA CHOOSE ERROR\n");
        break;
    }

    csi_rsa_uninit(&rsa);
    return 0;
}

int rsa_encrypt_decrypt_async_stability(void *args)
{
    int length_of_source;
    int test_loops=0;
    uint32_t output_length = 32;
    uint32_t encrypt_times = 0;
    uint32_t decrypt_times = 0;
    uint8_t rand_data = 0;
    csi_rsa_state_t status;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    uint8_t rsa_dec_data[RSA_SOURCE_DATA_LEN];

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_received_data_from_host,0, RSA_RECV_HOST_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%c,%d,%d,%d,%d",&dummy,&dummy,&dummy,&dummy,&rand_data,&encrypt_times,&decrypt_times,&length_of_source, &test_loops);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);
   
    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);
    csi_rsa_attach_callback(&rsa, rsa_test_cb, NULL);

    switch(rand_data){
        case 0:
            generate_rand_array(rsa_source_data,255,length_of_source);
            TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
            for (j = 0; j<test_loops; j++){
                for(int i=0; i<encrypt_times;i++){
                    rsa_event_finish = 1;
                    ret = csi_rsa_encrypt_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
                    TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_encrypt_async should return %d, but returned %d ", CSI_OK, ret);
                    while(rsa_event_finish);
                    TEST_PRINT_HEX("ENCRYPTED DATA:", rsa_target_data, rsa_test_param.data_len);
                }

                while(1){
                    csi_rsa_get_state(&rsa, &status);
                    if (status.busy == 0) {
                        break;
                    }
                }

                for(int i=0;i<decrypt_times;i++){
                    rsa_event_finish = 1;
                    ret = csi_rsa_decrypt_async(&rsa, &context, rsa_target_data, rsa_test_param.data_len,rsa_dec_data, &output_length);
                    while (rsa_event_finish);
                    TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_decrypt_async should return %d, but returned %d ", CSI_OK, ret);
                    TEST_PRINT_HEX("DECRYPTED DATA:", rsa_dec_data, output_length);
                }

                ret = memcmp(rsa_source_data,rsa_dec_data,output_length);
                TEST_CASE_ASSERT_QUIT(ret==0,"The result of this time is not equal to last time\n");
                TEST_CASE_TIPS("Test loop:%d finished!\n",j);
                memset(rsa_dec_data,0x00,output_length);
                memset(rsa_target_data,0x00,output_length);
            }
            break;
        case 1:
            for (j = 0; j<test_loops; j++){
                generate_rand_array(rsa_source_data,255,length_of_source);
                TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
                for(int i=0; i<encrypt_times;i++){
                    rsa_event_finish = 1;
                    ret = csi_rsa_encrypt_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data);
                    TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_encrypt_async should return %d, but returned %d ", CSI_OK, ret);
                    while(rsa_event_finish);
                    TEST_PRINT_HEX("ENCRYPTED DATA:", rsa_target_data, rsa_test_param.data_len);
                }

                while(1){
                    csi_rsa_get_state(&rsa, &status);
                    if (status.busy == 0) {
                        break;
                    }
                }

                for(int i=0;i<decrypt_times;i++){
                    rsa_event_finish = 1;
                    ret = csi_rsa_decrypt_async(&rsa, &context, rsa_target_data, rsa_test_param.data_len,rsa_dec_data, &output_length);
                    while (rsa_event_finish);
                    TEST_CASE_ASSERT(ret == CSI_OK,"csi_rsa_decrypt_async should return %d, but returned %d ", CSI_OK, ret);
                    TEST_PRINT_HEX("DECRYPTED DATA:", rsa_dec_data, output_length);
                }

                ret = memcmp(rsa_source_data,rsa_dec_data,output_length);
                TEST_CASE_ASSERT_QUIT(ret==0,"The result of this time is not equal to last time\n");
                TEST_CASE_TIPS("Test loop:%d finished!\n",j);
                memset(rsa_dec_data,0x00,output_length);
                memset(rsa_target_data,0x00,output_length);
            }
            break;
        default:
            break;
    }
    csi_rsa_detach_callback(&rsa);
    csi_rsa_uninit(&rsa);
    return 0;
}



int rsa_sign_verify_sync_stability(void *args)
{
    int length_of_source;
    bool result;
    int test_loops=0;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    csi_rsa_state_t status;
    uint32_t sign_times = 0;
    uint32_t verify_times = 0;
    uint8_t rand_data = 0;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%c,%d,%d,%d,%d",&dummy,&dummy,&dummy,&dummy,&rand_data,&sign_times,&verify_times,&length_of_source, &test_loops);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);

    switch (rand_data)
    {
    case 0:
        generate_rand_array(rsa_source_data,255,length_of_source);
        TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
        for (j = 0; j<test_loops; j++){
            for(int i=0; i<sign_times;i++){
                ret = csi_rsa_sign(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
                TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"csi_rsa_sign should return %d, but returned %d ", CSI_OK, ret);
                TEST_PRINT_HEX("SIGNED DATA:", rsa_target_data, rsa_test_param.data_len);
            }

            while(1){
                csi_rsa_get_state(&rsa, &status);
                if (status.busy == 0) {
                    break;
                }
            }

            for(int i=0;i<verify_times;i++){
                result = csi_rsa_verify(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
                TEST_CASE_ASSERT_QUIT(result == true,"csi_rsa_verify should return %d, but returned %d ", CSI_OK, ret);
            }

            TEST_CASE_TIPS("Test loop:%d finished!\n",j);
        }
        memset(rsa_source_data,0x00,length_of_source);
        break;
    case 1:
        for (j = 0; j<test_loops; j++){
            generate_rand_array(rsa_source_data,255,length_of_source);
            TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
            for(int i=0; i<sign_times;i++){
                ret = csi_rsa_sign(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
                TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"csi_rsa_sign should return %d, but returned %d ", CSI_OK, ret);
                TEST_PRINT_HEX("SIGNED DATA:", rsa_target_data, rsa_test_param.data_len);
            }

            while(1){
                csi_rsa_get_state(&rsa, &status);
                if (status.busy == 0) {
                    break;
                }
            }

            for(int i=0;i<verify_times;i++){
                result = csi_rsa_verify(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
                TEST_CASE_ASSERT_QUIT(result == true,"csi_rsa_verify should return %d, but returned %d ", CSI_OK, ret);
            }

            TEST_CASE_TIPS("Test loop:%d finished!\n",j);
            memset(rsa_source_data,0x00,length_of_source);
        }
        break;
    default:
        TEST_CASE_ASSERT(1 == 0,"RAND_DATA CHOOSE ERROR\n");
        break;
    }
    csi_rsa_uninit(&rsa);
    return 0;
}



int rsa_sign_verify_async_stability(void *args)
{
    int length_of_source;
    int test_loops=0;
    int dummy;
    char *p = args ;
    csi_error_t ret; 
    csi_rsa_t rsa;
    csi_rsa_context_t context;
    csi_rsa_state_t status;
    uint32_t sign_times = 0;
    uint32_t verify_times = 0;
    uint8_t rand_data = 0;

    get_rsa_test_params(p, &context);
    memset(rsa_source_data,0,RSA_SOURCE_DATA_LEN);
    memset(rsa_target_data,0x00,rsa_test_param.data_len);
    sscanf(args, "%d,%d,%d,%d,%c,%d,%d,%d,%d",&dummy,&dummy,&dummy,&dummy,&rand_data,&sign_times,&verify_times,&length_of_source, &test_loops);
    TEST_CASE_TIPS("key bits: %d", context.key_bits);
    TEST_CASE_TIPS("padding_type: %d", context.padding_type);
    TEST_CASE_TIPS("hash_type: %d", rsa_test_param.hash_type );
    TEST_CASE_TIPS("length_of_source: %d", length_of_source);
    TEST_CASE_TIPS("rsa_test_param.data_len: %d", rsa_test_param.data_len);

    ret = csi_rsa_init(&rsa, rsa_test_param.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"rsa init failed, it returned: %d", ret);
    csi_rsa_attach_callback(&rsa, rsa_test_cb, NULL);
    
    switch (rand_data)
    {
    case 0:
        generate_rand_array(rsa_source_data,255,length_of_source);
        TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
        for (j = 0; j<test_loops; j++){
            for(int i=0; i<sign_times;i++){
                ret = csi_rsa_sign_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
                TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"csi_rsa_sign should return %d, but returned %d ", CSI_OK, ret);
                TEST_PRINT_HEX("SIGNED DATA:", rsa_target_data, rsa_test_param.data_len);
            }

            while(1){
                csi_rsa_get_state(&rsa, &status);
                if (status.busy == 0) {
                    break;
                }
            }

            for(int i=0;i<verify_times;i++){
                ret = csi_rsa_verify_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
                TEST_CASE_ASSERT(ret == CSI_OK, "RSA verify failed.");
            }

            TEST_CASE_TIPS("Test loop:%d finished!\n",j);
        }
        memset(rsa_source_data,0x00,length_of_source);
        break;
    case 1:
        for (j = 0; j<test_loops; j++){
            generate_rand_array(rsa_source_data,255,length_of_source);
            TEST_PRINT_HEX("SOURCE DATA:", rsa_source_data, length_of_source);
            for(int i=0; i<sign_times;i++){
                rsa_event_finish=1;
                ret = csi_rsa_sign_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.hash_type);
                TEST_CASE_ASSERT_QUIT(ret == CSI_OK,"csi_rsa_sign should return %d, but returned %d ", CSI_OK, ret);
                while(rsa_event_finish);
                TEST_PRINT_HEX("SIGNED DATA:", rsa_target_data, rsa_test_param.data_len);
            }

            while(1){
                csi_rsa_get_state(&rsa, &status);
                if (status.busy == 0) {
                    break;                
                }
            }

            for(int i=0;i<verify_times;i++){
                async_verify_flag= 1;
                ret = csi_rsa_verify_async(&rsa, &context, (void *)rsa_source_data, length_of_source, (void *)rsa_target_data, rsa_test_param.data_len, rsa_test_param.hash_type);
                TEST_CASE_ASSERT(ret == CSI_OK, "RSA verify failed.");
                while (async_verify_flag);
            }

            TEST_CASE_TIPS("Test loop:%d finished!\n",j);
            memset(rsa_source_data,0x00,length_of_source);
        }
        break;
    default:
        TEST_CASE_ASSERT(1 == 0,"RAND_DATA CHOOSE ERROR\n");
        break;
    }

    csi_rsa_detach_callback(&rsa);
    csi_rsa_uninit(&rsa);
    return 0;   
}

