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

#include "aes_test.h"

const test_info_t aes_test_funcs_map[] = {
  // {"SET_AES_KEY",set_aes_key, 2},
  // {"SET_AES_IV",set_aes_iv, 1},
  
  {"AES_ECB_ENCRYPT",aes_ecb_encrypt, 2},
  {"AES_ECB_DECRYPT",aes_ecb_decrypt, 2},
  {"AES_CBC_ENCRYPT",aes_cbc_encrypt, 2},
  {"AES_CBC_DECRYPT",aes_cbc_decrypt, 2},

#ifdef CSI_AES_CFB
  {"AES_CFB128_ENCRYPT",aes_cfb128_encrypt, 3},
  {"AES_CFB128_DECRYPT",aes_cfb128_decrypt, 3},
#endif

#ifdef CSI_AES_CTR
  {"AES_CTR_ENCRYPT",aes_ctr_encrypt, 3},
  {"AES_CTR_DECRYPT",aes_ctr_decrypt, 3},
#endif

  {"AES_ECB_ENCRYPT_PERF",aes_ecb_encrypt_performance, 3},
  {"AES_ECB_DECRYPT_PERF",aes_ecb_decrypt_performance, 3},
  {"AES_CBC_ENCRYPT_PERF",aes_cbc_encrypt_performance, 3},
  {"AES_CBC_DECRYPT_PERF",aes_cbc_decrypt_performance, 3},

};

int test_aes_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)aes_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("AES don't supported this command,%d", ret);
    }

    return ret;
}