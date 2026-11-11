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

#include <rsa_test.h>

const test_info_t rsa_test_funcs_map[] = {
	{"RSA_ENCRYPT_SYNC",test_rsa_encrypt_sync, 6},
	{"RSA_DECRYPT_SYNC",test_rsa_decrypt_sync, 6},
	{"RSA_ENCRYPT_ASYNC",test_rsa_encrypt_async, 6},
	{"RSA_DECRYPT_ASYNC",test_rsa_decrypt_async, 6},
	{"RSA_ENCRYPT_DECRYPT_SYNC_PERFORMANCE",test_rsa_encrypt_decrypt_sync_performance, 7},
	{"RSA_ENCRYPT_DECRYPT_ASYNC_PERFORMANCE",test_rsa_encrypt_decrypt_async_performance, 7},
	{"RSA_SIGN_VERIFY_SYNC",test_rsa_sign_verify_sync, 8},
	{"RSA_SIGN_VERIFY_ASYNC",test_rsa_sign_verify_async, 8},
	{"RSA_SIGN_VERIFY_SYNC_PERFORMANCE",test_rsa_sign_verify_sync_performance, 7},
	{"RSA_SIGN_VERIFY_ASYNC_PERFORMANCE",test_rsa_sign_verify_async_performance, 7},
	{"RSA_SIGN_VERIFY_SYNC_STABILITY",rsa_sign_verify_sync_stability, 9},
	{"RSA_SIGN_VERIFY_ASYNC_STABILITY",rsa_sign_verify_async_stability, 9},
	{"RSA_ENCRYPT_DECRYPT_SYNC_STABILITY",rsa_encrypt_decrypt_sync_stability, 9},
	{"RSA_ENCRYPT_DECRYPT_ASYNC_STABILITY",rsa_encrypt_decrypt_async_stability, 9},		
	{"RSA_SET_KEY",test_set_rsa_key, 2}
};


int test_rsa_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)rsa_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("RSA don't supported this command,%d", ret);
    }

    return ret;
}
