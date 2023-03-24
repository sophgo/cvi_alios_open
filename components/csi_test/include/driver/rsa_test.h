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

#ifndef __RSA_TEST__
#define __RSA_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <drv/rsa.h>

#define RSA_TEST_DATA_LEN  (1024)
#define RSA_SOURCE_DATA_LEN (RSA_TEST_DATA_LEN)
#define RSA_TARGET_DATA_LEN (RSA_TEST_DATA_LEN)
#define RSA_RECV_HOST_DATA_LEN (RSA_SOURCE_DATA_LEN * 2)
#define RSA_TEST_KEY_LEN (RSA_TEST_DATA_LEN)

typedef struct {
    uint32_t    idx;
    csi_rsa_key_bits_t    key_bits;//参考定义csi_rsa_key_bits_t
    csi_rsa_padding_type_t    padding_type;//参考定义csi_rsa_padding_type_t
    csi_rsa_hash_type_t    hash_type;//参考定义csi_rsa_hash_type_t
    uint16_t data_len;
    uint8_t flag_gen_key;
} test_rsa_args_t;


extern uint8_t rsa_temp[RSA_SOURCE_DATA_LEN];
extern uint8_t rsa_source_data[RSA_SOURCE_DATA_LEN];
extern uint8_t rsa_target_data[RSA_TARGET_DATA_LEN];
extern char rsa_received_data_from_host[RSA_RECV_HOST_DATA_LEN];
extern uint8_t rsa_test_key_n[RSA_TEST_KEY_LEN];
extern uint8_t rsa_test_key_e[RSA_TEST_KEY_LEN];
extern uint8_t rsa_test_key_d[RSA_TEST_KEY_LEN];

extern uint8_t rsa_n1024[128];
extern uint8_t rsa_d1024[128];
extern uint8_t rsa_e1024[128];

extern uint8_t rsa_n2048[256];
extern uint8_t rsa_d2048[256];
extern uint8_t rsa_e2048[256];

extern test_rsa_args_t rsa_test_param;
extern int test_rsa_encrypt_sync(void *args);
extern int test_rsa_decrypt_sync(void *args);

extern int test_rsa_encrypt_async(void *args);
extern int test_rsa_decrypt_async(void *args);

extern int test_rsa_encrypt_decrypt_sync_performance(void *args);
extern int test_rsa_encrypt_decrypt_async_performance(void *args);

extern int test_rsa_sign_verify_sync(void *args);
extern int test_rsa_sign_verify_async(void *args);

extern int test_rsa_sign_verify_sync_performance(void *args);
extern int test_rsa_sign_verify_async_performance(void *args);
extern int test_set_rsa_key(void *args);

extern int rsa_encrypt_decrypt_sync_stability(void *args);
extern int rsa_encrypt_decrypt_async_stability(void *args);
extern int rsa_sign_verify_sync_stability(void *args);
extern int rsa_sign_verify_async_stability(void *args);
#endif
