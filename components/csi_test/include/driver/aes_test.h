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

#ifndef __AES_TEST__
#define __AES_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <drv/aes.h>

#define AES_DATA_128 16
#define AES_DATA_1024 128

#define AES_KEY_LEN	128
#define AES_IV_LEN	128

typedef struct {
    uint32_t    idx;
    csi_aes_key_bits_t key_type;
    uint32_t    key_len;
    uint32_t    data_len;
    uint8_t*    key;
    uint8_t*    src_data;
    uint8_t*    enc_data;
    uint8_t*    ref_data;
} test_aes_args_t;


extern int test_aes_mode(char *args);
extern int set_aes_key(void *args);
extern int set_aes_iv(void *args);
extern int aes_ecb_encrypt(void *args);
extern int aes_ecb_decrypt(void *args);
extern int aes_cbc_encrypt(void *args);
extern int aes_cbc_decrypt(void *args);
extern int aes_cfb128_encrypt(void *args);
extern int aes_cfb128_decrypt(void *args);
extern int aes_ctr_encrypt(void *args);
extern int aes_ctr_decrypt(void *args);
extern int aes_ecb_encrypt_performance(void *args);
extern int aes_ecb_decrypt_performance(void *args);
extern int aes_cbc_encrypt_performance(void *args);
extern int aes_cbc_decrypt_performance(void *args);
extern int aes_cfb128_encrypt_performance(void *args);
extern int aes_cfb128_decrypt_performance(void *args);
extern int aes_ctr_encrypt_performance(void *args);
extern int aes_ctr_decrypt_performance(void *args);
extern int aes_ecb_encrypt_decrypt_stability(void *args);
extern int aes_cbc_encrypt_decrypt_stability(void *args);
extern int aes_cbc_encrypt_test_aligned(void *args);
extern int aes_cbc_decrypt_test_aligned(void *args);
extern int aes_ecb_encrypt_test_aligned(void *args);
extern int aes_ecb_decrypt_test_aligned(void *args);
#endif
