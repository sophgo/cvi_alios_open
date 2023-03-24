/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#include <sha_test.h>


uint8_t sha_temp_data[SHA_ENC_DATA_LEN_MAX] = {0};

static csi_sha_context_t test_sha_context;

#ifdef CSI_SHA_ASYNC

static void sha_event_cb_func(csi_sha_t *sha, csi_sha_event_t event, void *arg)
{
    TEST_CASE_TIPS("sha_cb_fun event %u", event);
}

#endif

int test_sha_interface(void *args)
{
    csi_error_t ret;
    uint32_t out_sha_len;

    TEST_CASE_READY();


	ret = csi_sha_init(NULL,0); 
	TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_sha_init should return error, but return:%d", ret);

#ifdef CSI_SHA_ASYNC
	ret = csi_sha_attach_callback(NULL, sha_event_cb_func, NULL);
	TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_sha_attach_callback should return error, but return:%d", ret);
#endif

	ret = csi_sha_start(NULL, &test_sha_context, 0);
	TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_sha_start should return error, but return:%d", ret);

    ret = csi_sha_update(NULL, &test_sha_context, sha_temp_data, SHA_ENC_DATA_LEN_MAX);
    TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_sha_update should return error, but return:%d", ret);

#ifdef CSI_SHA_ASYNC
    ret = csi_sha_update_async(NULL, &context, sha_source_data, SHA_ENC_DATA_LEN_MAX);
    TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_sha_update_async should return error, but return:%d", ret);
#endif

	ret = csi_sha_finish(NULL, &test_sha_context, sha_temp_data, &out_sha_len);
	TEST_CASE_ASSERT_QUIT(ret==CSI_ERROR,"csi_sha_finish should return error, but return:%d", ret);


    return 0;
}
