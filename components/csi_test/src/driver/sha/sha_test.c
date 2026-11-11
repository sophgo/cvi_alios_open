/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#include <sha_test.h>

const test_info_t sha_test_funcs_map[] = {
	{"SHA_INTERFACE",test_sha_interface, 0},
	{"SHA_SYNC",test_sha_sync, 2},
	{"SHA_SYNC_MULTI",test_sha_sync_multi, 2},
	{"SHA_SYNC_PERF",test_sha_sync_perf, 3},
#ifdef CSI_SHA_ASYNC
	{"SHA_ASYNC",test_sha_async, 2},
	{"SHA_ASYNC_MULTI",test_sha_async_multi, 2},
	{"SHA_ASYNC_PERF",test_sha_sync_perf, 3},
#endif
	//{"SHA_SYNC_STABILITY",test_sha_sync_stability, 7},
	//{"SHA_ASYNC_STABILITY",test_sha_async_stability}
};


int test_sha_main(char *args)
{
    int ret;

    ret = testcase_jump(args, (void *)sha_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("SHA don't supported this command,%d", ret);
    }

    return ret;
}
