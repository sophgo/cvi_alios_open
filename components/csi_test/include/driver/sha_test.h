/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef __SHA_TEST__
#define __SHA_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>

#include <drv/sha.h>


#define SRC_DATA_LEN 55
#define SRC_DATA_SUB1_LEN 18
#define SRC_DATA_SUB2_LEN 21
#define SRC_DATA_SUB3_LEN 16
#define SHA_ENC_DATA_LEN_MAX 128


typedef struct {
    uint32_t        idx;
    csi_sha_mode_t  mode;
    uint32_t        sha_ref_len;
    uint8_t*        sha_ref_data;  //reference encrypted data

} test_sha_args_t;

extern int test_sha_interface(void *args);
extern int test_sha_sync(void *args);
extern int test_sha_sync_multi(void *args);
extern int test_sha_async(void *args);
extern int test_sha_async_multi(void *args);
extern int test_sha_sync_perf(void *args);
extern int test_sha_async_perf(void *args);
extern int test_sha_sync_stability(void *args);
extern int test_sha_async_stability(void *args);
#endif
