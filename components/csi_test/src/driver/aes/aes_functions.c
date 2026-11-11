/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#include "aes_test.h"


extern uint8_t key_128[];
extern uint8_t key_192[];
extern uint8_t key_256[];
extern uint8_t src_data_128[];
extern uint8_t enc_128_ecb[];
extern uint8_t enc_192_ecb[];
extern uint8_t enc_256_ecb[];
extern uint8_t enc_128_cbc[];
extern uint8_t enc_192_cbc[];
extern uint8_t enc_256_cbc[];
extern uint8_t iv_128[];

uint8_t tmp_dec_data[AES_DATA_128];
uint8_t tmp_enc_data[AES_DATA_128];

int assign_key_by_keylen(test_aes_args_t* arg)
{
    switch (arg->key_len) {
        case 16:
            arg->key = key_128;
            arg->key_type = AES_KEY_LEN_BITS_128;
            return 0;

        case 24:
            arg->key = key_192;
            arg->key_type = AES_KEY_LEN_BITS_192;
            return 0;

        case 32:
            arg->key = key_256;
            arg->key_type = AES_KEY_LEN_BITS_256;
            return 0;

        default:
            return 1;
    }
}

int aes_ecb_encrypt(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);


    tst_arg.data_len= AES_DATA_128;
    memset(tmp_enc_data, 0x0, AES_DATA_128);
    tst_arg.src_data = src_data_128;
    tst_arg.enc_data = tmp_enc_data;

    ret = assign_key_by_keylen(&tst_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);

    switch (tst_arg.key_len) {
        case 16:    tst_arg.ref_data = enc_128_ecb; break;
        case 24:    tst_arg.ref_data = enc_192_ecb; break;
        case 32:    tst_arg.ref_data = enc_256_ecb; break;
    }

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_encrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_encrypt_key should return CSI_OK, but return %d", ret);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key data_len=%d", tst_arg.data_len);
    ret = csi_aes_ecb_encrypt(&aes, tst_arg.src_data, tst_arg.enc_data,  tst_arg.data_len);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_ecb_encrypt should return CSI_OK, but return %d", ret);

    for (uint8_t i=0; i<tst_arg.data_len; i++) {
        if (tst_arg.enc_data[i] != tst_arg.ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }

    TEST_PRINT_HEX("ENCRYPTED DATA:", tst_arg.enc_data, tst_arg.data_len);

    csi_aes_uninit(&aes);
    return 0;
}

int aes_ecb_decrypt(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);


    tst_arg.data_len= AES_DATA_128;
    memset(tmp_dec_data, 0x0, AES_DATA_128);
    tst_arg.src_data = tmp_dec_data;
    tst_arg.ref_data = src_data_128;

    ret = assign_key_by_keylen(&tst_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);
    switch (tst_arg.key_len) {
        case 16:    tst_arg.enc_data = enc_128_ecb; break;
        case 24:    tst_arg.enc_data = enc_192_ecb; break;
        case 32:    tst_arg.enc_data = enc_256_ecb; break;
    }

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_decrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_decrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_decrypt_key should return CSI_OK, but return %d", ret);

    TEST_CASE_TIPS("Checking csi_aes_set_decrypt_key data_len=%d", tst_arg.data_len);
    ret = csi_aes_ecb_decrypt(&aes, tst_arg.enc_data, tst_arg.src_data, tst_arg.data_len);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_ecb_decrypt should return CSI_OK, but return %d", ret);

    for (uint8_t i=0; i<tst_arg.data_len; i++) {
        if (tst_arg.src_data[i] != tst_arg.ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }

    TEST_PRINT_HEX("DECRYPTED DATA:", tst_arg.src_data, tst_arg.data_len);

    csi_aes_uninit(&aes);
    return 0;
}


int aes_cbc_encrypt(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);


    tst_arg.data_len= AES_DATA_128;
    memset(tmp_enc_data, 0x0, AES_DATA_128);
    tst_arg.src_data = src_data_128;
    tst_arg.enc_data = tmp_enc_data;

    ret = assign_key_by_keylen(&tst_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);
    switch (tst_arg.key_len) {
        case 16:tst_arg.ref_data = enc_128_cbc; break;
        case 24:tst_arg.ref_data = enc_192_cbc; break;
        case 32:tst_arg.ref_data = enc_256_cbc; break;
    }

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_encrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_encrypt_key should return CSI_OK, but return %d", ret);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key data_len=%d", tst_arg.data_len);
    ret = csi_aes_cbc_encrypt(&aes, tst_arg.src_data, tst_arg.enc_data, tst_arg.data_len, iv_128);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_cbc_encrypt should return CSI_OK, but return %d", ret);

    for (uint8_t i=0; i<tst_arg.data_len; i++) {
        if (tst_arg.enc_data[i] != tst_arg.ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }

    TEST_PRINT_HEX("ENCRYPTED DATA:", tst_arg.enc_data, tst_arg.data_len);

    csi_aes_uninit(&aes);
    return 0;
}

int aes_cbc_decrypt(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);


    tst_arg.data_len= AES_DATA_128;
    memset(tmp_dec_data, 0x0, AES_DATA_128);
    tst_arg.src_data = tmp_dec_data;
    tst_arg.ref_data = src_data_128;

    ret = assign_key_by_keylen(&tst_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);
    switch (tst_arg.key_len) {
        case 16:    tst_arg.enc_data = enc_128_cbc; break;
        case 24:    tst_arg.enc_data = enc_192_cbc; break;
        case 32:    tst_arg.enc_data = enc_256_cbc; break;
    }

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_swcrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_decrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_decrypt_key should return CSI_OK, but return %d", ret);

    TEST_CASE_TIPS("Checking csi_aes_set_decrypt_key data_len=%d", tst_arg.data_len);
    ret = csi_aes_cbc_decrypt(&aes, tst_arg.enc_data, tst_arg.src_data, tst_arg.data_len, iv_128);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_cbc_decrypt should return CSI_OK, but return %d", ret);

    for (uint8_t i=0; i<tst_arg.data_len; i++) {
        if (tst_arg.src_data[i] != tst_arg.ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }

    TEST_PRINT_HEX("DECRYPTED DATA:", tst_arg.src_data, tst_arg.data_len);

    csi_aes_uninit(&aes);
    return 0;
}

#ifdef CSI_AES_CFB

extern uint8_t enc_128_cfb[];
extern uint8_t enc_192_cfb[];
extern uint8_t enc_256_cfb[];

int aes_cbf_encrypt(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);


    tst_arg.data_len= AES_DATA_128;
    memset(tmp_enc_data, 0x0, AES_DATA_128);
    tst_arg.src_data = src_data_128;
    tst_arg.enc_data = tmp_enc_data;

    switch (tst_arg.key_len) {
        case 16:
            tst_arg.key = key_128;
            tst_arg.key_type = AES_KEY_LEN_BITS_128;
            tst_arg.ref_data = enc_128_cbf;
            break;

        case 24:
            tst_arg.key = key_192;
            tst_arg.key_type = AES_KEY_LEN_BITS_192;
            tst_arg.ref_data = enc_192_cbf;
            break;

        case 32:
            tst_arg.key = key_256;
            tst_arg.key_type = AES_KEY_LEN_BITS_256;
            tst_arg.ref_data = enc_256_cbf;
            break;

        default:
            TEST_CASE_ASSERT_QUIT(0==1, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);
    }

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_encrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_encrypt_key should return CSI_OK, but return %d", ret);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key data_len=%d", tst_arg.data_len);
    ret = csi_aes_cbf_encrypt(&aes, tst_arg.src_data, tst_arg.enc_data, tst_arg.data_len, iv_128);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_cbf_encrypt should return CSI_OK, but return %d", ret);

    for (uint8_t i=0; i<tst_arg.data_len; i++) {
        if (tst_arg.enc_data[i] != tst_arg.ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }

    TEST_PRINT_HEX("ENCRYPTED DATA:", tst_arg.enc_data, tst_arg.data_len);

    csi_aes_uninit(&aes);
    return 0;
}

int aes_cbf_decrypt(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);


    tst_arg.data_len= AES_DATA_128;
    memset(tmp_dec_data, 0x0, AES_DATA_128);
    tst_arg.src_data = tmp_dec_data;
    tst_arg.ref_data = src_data_128;

    switch (tst_arg.key_len) {
        case 16:
            tst_arg.key = key_128;
            tst_arg.key_type = AES_KEY_LEN_BITS_128;
            tst_arg.enc_data = enc_128_cbf;
            break;

        case 24:
            tst_arg.key = key_192;
            tst_arg.key_type = AES_KEY_LEN_BITS_192;
            tst_arg.enc_data = enc_192_cbf;
            break;

        case 32:
            tst_arg.key = key_256;
            tst_arg.key_type = AES_KEY_LEN_BITS_256;
            tst_arg.enc_data = enc_256_cbf;
            break;

        default:
            TEST_CASE_ASSERT_QUIT(0==1, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);
    }

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_decrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_encrypt_key should return CSI_OK, but return %d", ret);

    TEST_CASE_TIPS("Checking csi_aes_set_decrypt_key data_len=%d", tst_arg.data_len);
    ret = csi_aes_cbf_decrypt(&aes, tst_arg.enc_data, tst_arg.src_data, tst_arg.data_len, iv_128);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_cbf_decrypt should return CSI_OK, but return %d", ret);

    for (uint8_t i=0; i<tst_arg.data_len; i++) {
        if (tst_arg.src_data[i] != tst_arg.ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }

    TEST_PRINT_HEX("DECRYPTED DATA:", tst_arg.src_data, tst_arg.data_len);

    csi_aes_uninit(&aes);
    return 0;
}
#endif


#ifdef CSI_AES_CTR

extern uint8_t enc_128_ctr[];
extern uint8_t enc_192_ctr[];
extern uint8_t enc_256_ctr[];

int aes_ctr_encrypt(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);


    tst_arg.data_len= AES_DATA_128;
    memset(tmp_enc_data, 0x0, AES_DATA_128);
    tst_arg.src_data = src_data_128;
    tst_arg.enc_data = tmp_enc_data;

    switch (tst_arg.key_len) {
        case 16:
            tst_arg.key = key_128;
            tst_arg.key_type = AES_KEY_LEN_BITS_128;
            tst_arg.ref_data = enc_128_ctr;
            break;

        case 24:
            tst_arg.key = key_192;
            tst_arg.key_type = AES_KEY_LEN_BITS_192;
            tst_arg.ref_data = enc_192_ctr;
            break;

        case 32:
            tst_arg.key = key_256;
            tst_arg.key_type = AES_KEY_LEN_BITS_256;
            tst_arg.ref_data = enc_256_ctr;
            break;

        default:
            TEST_CASE_ASSERT_QUIT(0==1, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);
    }

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_encrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_encrypt_key should return CSI_OK, but return %d", ret);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key data_len=%d", tst_arg.data_len);
    ret = csi_aes_ctr_encrypt(&aes, tst_arg.src_data, tst_arg.enc_data, tst_arg.data_len, iv_128);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_ctr_encrypt should return CSI_OK, but return %d", ret);

    for (uint8_t i=0; i<tst_arg.data_len; i++) {
        if (tst_arg.enc_data[i] != tst_arg.ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }

    TEST_PRINT_HEX("ENCRYPTED DATA:", tst_arg.enc_data, tst_arg.data_len);

    csi_aes_uninit(&aes);
    return 0;
}

int aes_ctr_decrypt(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);


    tst_arg.data_len= AES_DATA_128;
    memset(tmp_dec_data, 0x0, AES_DATA_128);
    tst_arg.src_data = tmp_dec_data;
    tst_arg.ref_data = src_data_128;

    switch (tst_arg.key_len) {
        case 16:
            tst_arg.key = key_128;
            tst_arg.key_type = AES_KEY_LEN_BITS_128;
            tst_arg.enc_data = enc_128_ctr;
            break;

        case 24:
            tst_arg.key = key_192;
            tst_arg.key_type = AES_KEY_LEN_BITS_192;
            tst_arg.enc_data = enc_192_ctr;
            break;

        case 32:
            tst_arg.key = key_256;
            tst_arg.key_type = AES_KEY_LEN_BITS_256;
            tst_arg.enc_data = enc_256_ctr;
            break;

        default:
            TEST_CASE_ASSERT_QUIT(0==1, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);
    }

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_decrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_encrypt_key should return CSI_OK, but return %d", ret);

    TEST_CASE_TIPS("Checking csi_aes_set_decrypt_key data_len=%d", tst_arg.data_len);
    ret = csi_aes_ctr_decrypt(&aes, tst_arg.enc_data, tst_arg.src_data, tst_arg.data_len, iv_128);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_ctr_decrypt should return CSI_OK, but return %d", ret);

    for (uint8_t i=0; i<tst_arg.data_len; i++) {
        if (tst_arg.src_data[i] != tst_arg.ref_data[i]){
            TEST_CASE_ASSERT( 0 == 1, "The number %d byte of decrypted data is not match with the reference", i);
        }
    }

    TEST_PRINT_HEX("DECRYPTED DATA:", tst_arg.src_data, tst_arg.data_len);

    csi_aes_uninit(&aes);
    return 0;
}
#endif


int aes_ecb_encrypt_performance(void *args)
{
    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);
    uint32_t number_of_loop = *((uint64_t *)args + 2) *1000;
    tst_arg.data_len= AES_DATA_128;

    uint32_t i ;
    uint32_t time_ms;
    float performance;

    ret = assign_key_by_keylen(&tst_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_encrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_encrypt_key should return CSI_OK, but return %d", ret);

    generate_rand_array(tmp_enc_data, 255, tst_arg.data_len);
    generate_rand_array(tmp_dec_data, 255, tst_arg.data_len);

    tst_timer_restart();

    for (i = 1; i <= number_of_loop; i++) {

        ret = csi_aes_ecb_encrypt(&aes, tmp_enc_data, tmp_dec_data, tst_arg.data_len);
        if (ret != 0) {
            TEST_CASE_ASSERT_QUIT(ret == 0, "Aes ecb encrypt failed!\n");
        }

        ret = csi_aes_ecb_encrypt(&aes, tmp_dec_data, tmp_enc_data, tst_arg.data_len);
        if (ret != 0) {
            TEST_CASE_ASSERT_QUIT(ret == 0, "Aes ecb encrypt failed!\n");
        }
    }

    time_ms = tst_timer_get_interval();
    performance = (number_of_loop * 2.0) * 1000 / time_ms;
    TEST_PRINT_HEX("LAST ENC DATA:", tmp_enc_data, tst_arg.data_len);
    TEST_PRINT_HEX("LAST DEC DATA:", tmp_dec_data, tst_arg.data_len);
    TEST_CASE_TIPS("AES ECB encrypt keylen=%d performance is %f encryption cycles/second", tst_arg.key_len ,performance);

    csi_aes_uninit(&aes);
    return 0;
}

int aes_ecb_decrypt_performance(void *args)
{

    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);
    uint32_t number_of_loop = *((uint64_t *)args + 2) * 1000;
    tst_arg.data_len= AES_DATA_128;

    uint32_t i ;
    uint32_t time_ms;
    float performance;

    ret = assign_key_by_keylen(&tst_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_decrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_decrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_decrypt_key should return CSI_OK, but return %d", ret);

    generate_rand_array(tmp_enc_data, 255, tst_arg.data_len);
    generate_rand_array(tmp_dec_data, 255, tst_arg.data_len);

    tst_timer_restart();

    for (i = 1; i <= number_of_loop; i++) {

        ret = csi_aes_ecb_decrypt(&aes, tmp_enc_data, tmp_dec_data, tst_arg.data_len);
        if (ret != 0) {
            TEST_CASE_ASSERT_QUIT(ret == 0, "Aes ecb decrypt failed!\n");
        }

        ret = csi_aes_ecb_decrypt(&aes, tmp_dec_data, tmp_enc_data, tst_arg.data_len);
        if (ret != 0) {
            TEST_CASE_ASSERT_QUIT(ret == 0, "Aes ecb decrypt failed!\n");
        }
    }

    time_ms = tst_timer_get_interval();
    performance = (number_of_loop * 2.0) * 1000 / time_ms;
    TEST_PRINT_HEX("LAST ENC DATA:", tmp_enc_data, tst_arg.data_len);
    TEST_PRINT_HEX("LAST DEC DATA:", tmp_dec_data, tst_arg.data_len);
    TEST_CASE_TIPS("AES ECB decrypt keylen=%d performance is %f decryption cycles/second", tst_arg.key_len ,performance);

    csi_aes_uninit(&aes);
    return 0;
}


int aes_cbc_encrypt_performance(void *args)
{

    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);
    uint32_t number_of_loop = *((uint64_t *)args + 2) *1000;
    tst_arg.data_len= AES_DATA_128;

    uint32_t i ;
    uint32_t time_ms;
    float performance;

    ret = assign_key_by_keylen(&tst_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_encrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_encrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_encrypt_key should return CSI_OK, but return %d", ret);

    generate_rand_array(tmp_enc_data, 255, tst_arg.data_len);
    generate_rand_array(tmp_dec_data, 255, tst_arg.data_len);

    tst_timer_restart();

    for (i = 1; i <= number_of_loop; i++) {

        ret = csi_aes_cbc_encrypt(&aes, tmp_enc_data, tmp_dec_data, tst_arg.data_len, iv_128);
        if (ret != 0) {
            TEST_CASE_ASSERT_QUIT(ret == 0, "Aes cbc encrypt failed!\n");
        }

        ret = csi_aes_cbc_encrypt(&aes, tmp_dec_data, tmp_enc_data, tst_arg.data_len, iv_128);
        if (ret != 0) {
            TEST_CASE_ASSERT_QUIT(ret == 0, "Aes cbc encrypt failed!\n");
        }
    }

    time_ms = tst_timer_get_interval();
    performance = (number_of_loop * 2.0) * 1000 / time_ms;
    TEST_PRINT_HEX("LAST ENC DATA:", tmp_enc_data, tst_arg.data_len);
    TEST_PRINT_HEX("LAST DEC DATA:", tmp_dec_data, tst_arg.data_len);
    TEST_CASE_TIPS("AES ECB encrypt keylen=%d performance is %f encryption cycles/second", tst_arg.key_len ,performance);

    csi_aes_uninit(&aes);
    return 0;
}

int aes_cbc_decrypt_performance(void *args)
{

    uint32_t ret = 0;
    csi_aes_t aes;
    test_aes_args_t tst_arg;

    tst_arg.idx     = *((uint64_t *)args);
    tst_arg.key_len = *((uint64_t *)args + 1);
    uint32_t number_of_loop = *((uint64_t *)args + 2) * 1000;
    tst_arg.data_len= AES_DATA_128;

    uint32_t i ;
    uint32_t time_ms;
    float performance;

    ret = assign_key_by_keylen(&tst_arg);
    TEST_CASE_ASSERT_QUIT(0==ret, "keylen %u is not valid! please only use 16, 24 or 32 Bits, ", tst_arg.key_len);

    TEST_CASE_TIPS("Checking csi_aes_init idx=%d", tst_arg.idx);
    ret = csi_aes_init(&aes, tst_arg.idx);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_aes_init failed with idx: %d", tst_arg.idx);

    TEST_CASE_TIPS("Checking csi_aes_set_decrypt_key key_len=%d", tst_arg.key_len);
    ret = csi_aes_set_decrypt_key(&aes, (void*)tst_arg.key, tst_arg.key_type);
    TEST_CASE_ASSERT(ret == CSI_OK, "csi_aes_set_decrypt_key should return CSI_OK, but return %d", ret);

    generate_rand_array(tmp_enc_data, 255, tst_arg.data_len);
    generate_rand_array(tmp_dec_data, 255, tst_arg.data_len);

    tst_timer_restart();

    for (i = 1; i <= number_of_loop; i++) {

        ret = csi_aes_cbc_decrypt(&aes, tmp_enc_data, tmp_dec_data, tst_arg.data_len, iv_128);
        if (ret != 0) {
            TEST_CASE_ASSERT_QUIT(ret == 0, "Aes cbc decrypt failed!\n");
        }

        ret = csi_aes_cbc_decrypt(&aes, tmp_dec_data, tmp_enc_data, tst_arg.data_len, iv_128);
        if (ret != 0) {
            TEST_CASE_ASSERT_QUIT(ret == 0, "Aes cbc decrypt failed!\n");
        }
    }

    time_ms = tst_timer_get_interval();
    performance = (number_of_loop * 2.0) * 1000 / time_ms;
    TEST_PRINT_HEX("LAST ENC DATA:", tmp_enc_data, tst_arg.data_len);
    TEST_PRINT_HEX("LAST DEC DATA:", tmp_dec_data, tst_arg.data_len);
    TEST_CASE_TIPS("AES ECB decrypt keylen=%d performance is %f decryption cycles/second", tst_arg.key_len ,performance);

    csi_aes_uninit(&aes);
    return 0;
}