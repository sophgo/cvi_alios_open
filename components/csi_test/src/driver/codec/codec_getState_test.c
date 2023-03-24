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


volatile static uint8_t writeing_lock = 0;

int test_codec_outputChannelState(void *args)
{
    csi_error_t ret_sta;
    csi_codec_t hd;
    csi_codec_output_config_t ch_config;
    csi_codec_output_t ch;
    test_codec_args_t td;
    csi_ringbuf_t output_ring_buffer;

    csi_state_t state;

    td.codec_idx = *((uint64_t *)args);
    td.ch_idx = *((uint64_t *)args + 1);
    td.sample_rate = 48000;
    td.bit_width = 16;
    td.sound_channel_num = 1;
    td.data_size = 2048;
    td.buffer_size = 52800;
    td.digital_gain = 0;
    td.analog_gain = 46;
    td.mix_gain = 0;
    td.period = 512;
    td.mode = 1;
    state.readable = 0;
    state.writeable = 0;


    TEST_CASE_TIPS("test codec idx is %d", td.codec_idx);
    TEST_CASE_TIPS("test codec channel is %d", td.ch_idx);


    uint8_t *cache_buffer = NULL;
    cache_buffer = malloc(td.buffer_size);

    if (cache_buffer == NULL) {
        TEST_CASE_WARN_QUIT("cache buffer malloc fail");
    }

    ret_sta = csi_codec_init(&hd, td.codec_idx);
    if (ret_sta != 0) {
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_init ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec %d init fail, expected return value is 0, actual value is %d", td.codec_idx,
                              ret_sta);
    }

    ch.ring_buf = &output_ring_buffer;
    ret_sta = csi_codec_output_open(&hd, &ch, td.ch_idx);
    if (ret_sta != 0) {
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_open ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d open fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
    }


    ret_sta = csi_codec_output_get_state(&ch, &state);
    if (ret_sta != 0) {
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_get_state ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT(ret_sta == 0, "get state of output channel %d fail, expected return value is 0, actual value is %d",
                     td.ch_idx, ret_sta);
    }
    
    if ((state.readable != 0) || (state.writeable != 0)) {
        TEST_CASE_ASSERT(1 == 0, "when open output channel, get state fail");
    }

    ch_config.sample_rate = td.sample_rate;
    ch_config.bit_width = td.bit_width;
    ch_config.buffer = cache_buffer;
    ch_config.buffer_size = td.buffer_size;
    ch_config.sound_channel_num = td.sound_channel_num;
    ch_config.period = td.period;
    ch_config.mode = td.mode;

    ret_sta = csi_codec_output_config(&ch, &ch_config);
    if (ret_sta != 0) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_config ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d config fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
    }

    ret_sta = csi_codec_output_digital_gain(&ch, td.digital_gain);
    if ((ret_sta != 0) && (ret_sta != CSI_UNSUPPORTED)) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_digital_gain ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0,
                              "codec channel %d config digital gain fail, expected return value is 0, actual value is %d", td.ch_idx, ret_sta);
    }

    ret_sta = csi_codec_output_analog_gain(&ch, td.analog_gain);
    if ((ret_sta != 0) && (ret_sta != CSI_UNSUPPORTED)) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_analog_gain ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0,
                              "codec channel %d config analog gain fail, expected return value is 0, actual value is %d", td.ch_idx, ret_sta);
    }

    ret_sta = csi_codec_output_mix_gain(&ch, td.mix_gain);
    if ((ret_sta != 0) && (ret_sta != CSI_UNSUPPORTED)) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_mix_gain ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d config mix gain fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }

    csi_dma_ch_t dma_ch;

    ret_sta = csi_codec_output_link_dma(&ch, &dma_ch);
    if (ret_sta != 0) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_link_dma ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d link DMA fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }

    ret_sta = csi_codec_output_start(&ch);
    if (ret_sta != 0) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_start ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d start output fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }

    uint32_t ret_num;

    ret_num = csi_codec_output_write(&ch, (const void *)0x4, td.data_size);
    TEST_CASE_ASSERT(ret_num == td.data_size, "codec output sync write error");

    ret_sta = csi_codec_output_get_state(&ch, &state);
    TEST_CASE_ASSERT(ret_sta == 0, "get state of output channel %d fail, expected return value is 0, actual value is %d",
                     td.ch_idx, ret_sta);

    if ((state.readable != 0) || (state.writeable != 0)) {
        TEST_CASE_ASSERT(1 == 0, "when output channel has data, get state fail");
    }

    ret_sta = csi_codec_output_get_state(&ch, &state);
    TEST_CASE_ASSERT(ret_sta == 0, "get state of output channel %d fail, expected return value is 0, actual value is %d",
                     td.ch_idx, ret_sta);

    if ((state.readable != 0) || (state.writeable != 0)) {
        TEST_CASE_ASSERT(1 == 0, "when stop output , get state fail");
    }

    ret_sta = csi_codec_output_link_dma(&ch, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "codec getstate unlink dma error");

    csi_codec_output_stop(&ch);
    csi_codec_output_close(&ch);

    ret_sta = csi_codec_output_get_state(&ch, &state);
    TEST_CASE_ASSERT(ret_sta == 0, "get state of output channel %d fail, expected return value is 0, actual value is %d",
                     td.ch_idx, ret_sta);

    if ((state.readable != 0) || (state.writeable != 0)) {
        TEST_CASE_ASSERT(1 == 0, "when close output channel, get state fail");
    }

    ret_sta = csi_codec_output_link_dma(&ch, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "codec getstate unlink dma CSI_ERROR");

    csi_codec_uninit(&hd);
    return 0;
}



#ifdef CSI_CODEC_GET_STATE
int test_codec_inputChannelState(void *args)
{
    csi_error_t ret_sta;
    csi_codec_t hd;
    csi_codec_input_config_t ch_config;
    csi_codec_input_t ch;
    test_codec_args_t td;
    csi_ringbuf_t input_ring_buffer;
    csi_state_t state;

    uint8_t *voice_data = NULL;
    td.codec_idx = *((uint64_t *)args);
    td.ch_idx = *((uint64_t *)args + 1);
    td.sample_rate = 48000;
    td.bit_width = 16;
    td.sound_channel_num = 1;
    td.data_size = 1024;
    td.buffer_size = 52800;
    td.digital_gain = 0;
    td.analog_gain = 46;
    td.mix_gain = 0;
    td.period = 256;
    td.mode = 1;
    state.readable = 0;
    state.writeable = 0;

    TEST_CASE_TIPS("test codec idx is %d", td.codec_idx);
    TEST_CASE_TIPS("test codec channel is %d", td.ch_idx);


    if (voice_data == NULL) {
        voice_data = malloc(td.data_size);

        if (voice_data == NULL) {
            TEST_CASE_WARN_QUIT("malloc fail");
        }
    }

    uint8_t *cache_buffer = NULL;
    cache_buffer = malloc(td.buffer_size);

    if (cache_buffer == NULL) {
        free(voice_data);
        TEST_CASE_WARN_QUIT("cache buffer malloc fail");
    }

    ret_sta = csi_codec_init(&hd, td.codec_idx);
    if (ret_sta != 0) {
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_init ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "uart %d init fail, expected return value is 0, actual value is %d", td.codec_idx,
                              ret_sta);
    }

    ch.ring_buf = &input_ring_buffer;
    ret_sta = csi_codec_input_open(&hd, &ch, td.ch_idx);
    if (ret_sta != 0) {
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_open ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d open fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
    }

    ret_sta = csi_codec_input_get_state(&ch, &state);
    TEST_CASE_ASSERT(ret_sta == 0, "get state of input channel %d fail, expected return value is 0, actual value is %d",
                     td.ch_idx, ret_sta);

    if ((state.readable != 0) || (state.writeable != 0)) {
        TEST_CASE_ASSERT(1 == 0, "when open input channel, get state fail");
    }

    ch_config.sample_rate = td.sample_rate;
    ch_config.bit_width = td.bit_width;
    ch_config.buffer = cache_buffer;
    ch_config.buffer_size = td.buffer_size;
    ch_config.sound_channel_num = td.sound_channel_num;
    ch_config.period = td.period;
    ch_config.mode = td.mode;


    ret_sta = csi_codec_input_config(&ch, &ch_config);
    if (ret_sta != 0) {
        csi_codec_input_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_config ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d config fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
    }

    ret_sta = csi_codec_input_digital_gain(&ch, td.digital_gain);
    if ((ret_sta != 0) && (ret_sta != CSI_UNSUPPORTED)) {
        csi_codec_input_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_digital_gain ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0,
                              "codec channel %d config digital gain fail, expected return value is 0, actual value is %d", td.ch_idx, ret_sta);
    }

    ret_sta = csi_codec_input_analog_gain(&ch, td.analog_gain);
    if ((ret_sta != 0) && (ret_sta != CSI_UNSUPPORTED)) {
        csi_codec_input_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_analog_gain ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0,
                              "codec channel %d config analog gain fail, expected return value is 0, actual value is %d", td.ch_idx, ret_sta);
    }

    ret_sta = csi_codec_input_mix_gain(&ch, td.mix_gain);
    if ((ret_sta != 0) && (ret_sta != CSI_UNSUPPORTED)) {
        csi_codec_input_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_mix_gain ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d config mix gain fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }


    csi_dma_ch_t dma_ch;

    ret_sta = csi_codec_input_link_dma(&ch, &dma_ch);
    if (ret_sta != 0) {
        csi_codec_input_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_link_dma ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d link DMA fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }

    ret_sta = csi_codec_input_start(&ch);
    if (ret_sta != 0) {
        csi_codec_input_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_start ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d start input fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }

    uint32_t ret_num, free_size = 0, use_size = 0;

    free_size = td.data_size - use_size;
    ret_num = csi_codec_input_read(&ch, voice_data + use_size, free_size);
    TEST_CASE_ASSERT(ret_num == 0, "codec input state read error");

    ret_sta = csi_codec_input_get_state(&ch, &state);
    TEST_CASE_ASSERT(ret_sta == 0, "get state of input channel %d fail, expected return value is 0, actual value is %d",
                     td.ch_idx, ret_sta);

    if ((state.readable != 0) || (state.writeable != 0)) {
        TEST_CASE_ASSERT(1 == 0, "when input channel has data, get state fail");
    }

    ret_sta = csi_codec_input_link_dma(&ch, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "codec getstate input unlink dma error");

    ret_sta = csi_codec_input_get_state(&ch, &state);
    TEST_CASE_ASSERT(ret_sta == 0, "get state of input channel %d fail, expected return value is 0, actual value is %d",
                     td.ch_idx, ret_sta);

    if ((state.readable != 0) || (state.writeable != 0)) {
        TEST_CASE_ASSERT(1 == 0, "when stop input channel, get state fail");
    }

    csi_codec_input_stop(&ch);
    csi_codec_input_close(&ch);

    ret_sta = csi_codec_input_link_dma(&ch, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "codec getstate input unlink dma error");


    csi_codec_uninit(&hd);
    free(voice_data);
    return 0;
}

#endif





