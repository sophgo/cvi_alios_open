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

#include <codec_test.h>
const test_info_t codec_test_funcs_map[] = {
    {"CODEC_INTERFACE", test_codec_interface, 0},
    {"CODEC_SYNC_OUTPUT", test_codec_syncOutput, 12},
    {"CODEC_SYNC_INPUT", test_codec_syncInput, 12},
    {"CODEC_ASYNC_OUTPUT", test_codec_asyncOutput, 12},
    {"CODEC_ASYNC_INPUT", test_codec_asyncInput, 12},
    {"CODEC_OUTPUT_CHANNEL_STATE", test_codec_outputChannelState, 2},
#ifdef CSI_CODEC_GET_STATE
    {"CODEC_INPUT_CHANNEL_STATE", test_codec_inputChannelState, 2},
#endif
    {"CODEC_PAUSE_RESUME", test_codec_pauseResume, 8},
    {"CODEC_OUTPUT_MUTE", test_codec_outputMute, 12},
    {"CODEC_INPUT_MUTE", test_codec_inputMute, 12},
    {"CODEC_OUTPUT_BUFFER", test_codec_outputBuffer, 3},
    {"CODEC_SYNC_OUTPUT_4K", test_codec_syncOutput_4K, 0},
    {"CODEC_SYNC_INPUT_5S", test_codec_syncInput_5s, 0},
    {NULL, NULL, 0}

};

int test_codec_main(char *args)
{
    int ret;
    ret = testcase_jump(args, (void *)codec_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("CODEC don't supported this command");
    }
    
    return ret;
}

//int test_codec_main(char *args)
//{
//	char *case_name[] = {
//		"CODEC_INTERFACE",
//		"CODEC_SYNC_OUTPUT",
//		"CODEC_SYNC_INPUT",
//		"CODEC_ASYNC_OUTPUT",
//		"CODEC_ASYNC_INPUT",
//		"CODEC_OUTPUT_CHANNEL_STATE",
//		"CODEC_INPUT_CHANNEL_STATE",
//		"CODEC_PAUSE_RESUME",
//		"CODEC_OUTPUT_MUTE",
//		"CODEC_INPUT_MUTE",
//		"CODEC_OUTPUT_BUFFER",
//
//	};
//
//	int (*case_func[])(char *args) = {
//		test_codec_interface,
//		test_codec_syncOutput,
//		test_codec_syncInput,
//		test_codec_asyncOutput,
//		test_codec_asyncInput,
//		test_codec_outputChannelState,
//		test_codec_inputChannelState,
//		test_codec_pauseResume,
//		test_codec_outputMute,
//		test_codec_inputMute,
//		test_codec_outputBuffer,
//	};
//
//	uint8_t i = 0;
//
//	for (i = 0; i < sizeof(case_name) / sizeof(char *); i++)
//	{
//		if (!strcmp((const char *)_mc_name, case_name[i]))
//		{
//			case_func[i](args);
//			return 0;
//		}
//	}
//
//	TEST_CASE_TIPS("CODEC don't supported this command");
//	return -1;
//}
