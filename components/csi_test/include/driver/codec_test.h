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

#ifndef __CODEC_TEST__
#define __CODEC_TEST__

#include <stdint.h>
#include <drv/codec.h>
#include <drv/dma.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>

typedef struct {
    uint32_t codec_idx;
    uint32_t ch_idx;
    uint32_t sample_rate;
    uint32_t bit_width;
    uint32_t period;
    uint32_t sound_channel_num;
    uint32_t data_size;
    uint32_t buffer_size;
    uint32_t digital_gain;
    uint32_t analog_gain;
    uint32_t mix_gain;
    uint32_t mode;
} test_codec_args_t;

extern int test_codec_interface(void *args);
extern int test_codec_syncOutput_4K(void *args);
extern int test_codec_syncInput_5s(void *args);
extern int test_codec_syncOutput(void *args);
extern int test_codec_syncInput(void *args);
extern int test_codec_asyncOutput(void *args);
extern int test_codec_asyncInput(void *args);
extern int test_codec_outputChannelState(void *args);
extern int test_codec_inputChannelState(void *args);
extern int test_codec_pauseResume(void *args);
extern int test_codec_outputMute(void *args);
extern int test_codec_inputMute(void *args);
extern int test_codec_outputBuffer(void *args);

#endif
