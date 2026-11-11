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


volatile static uint8_t reading_lock = 0;

static void input_callback(csi_codec_input_t *input, csi_codec_event_t event, void *arg)
{
    if (event == CODEC_EVENT_PERIOD_READ_COMPLETE) {
        *(uint8_t *)arg += 1;
    }

    if (event == CODEC_EVENT_READ_BUFFER_FULL) {
        reading_lock = 0;
        *(uint8_t *)arg += 1;
    }
}

int test_codec_syncInput(void *args)
{
    csi_error_t ret_sta;
    csi_codec_t hd;
    csi_codec_input_config_t ch_config;
    csi_codec_input_t ch;
    test_codec_args_t td;
    csi_ringbuf_t input_ring_buffer;

    uint8_t *voice_data = NULL;

    td.codec_idx = *((uint64_t *)args);
    td.ch_idx = *((uint64_t *)args + 1);
    td.sample_rate = *((uint64_t *)args + 2);
    td.bit_width = *((uint64_t *)args + 3);
    td.sound_channel_num = *((uint64_t *)args + 4);
    td.data_size = *((uint64_t *)args + 5);
    td.buffer_size = *((uint64_t *)args + 6);
    td.digital_gain = *((uint64_t *)args + 7);
    td.analog_gain = *((uint64_t *)args + 8);
    td.mix_gain = *((uint64_t *)args + 9);
    td.period = *((uint64_t *)args + 10);
    td.mode = *((uint64_t *)args + 11);

    TEST_CASE_TIPS("test codec idx is %d", td.codec_idx);
    TEST_CASE_TIPS("test codec channel is %d", td.ch_idx);
    TEST_CASE_TIPS("config sample rate is %d Hz", td.sample_rate);
    TEST_CASE_TIPS("config converter bit width is %d", td.bit_width);
    TEST_CASE_TIPS("config number of soundtrack per channel is %d", td.sound_channel_num);
    TEST_CASE_TIPS("config output data size is %d bytes", td.data_size);
    TEST_CASE_TIPS("config buffer size is %d", td.buffer_size);
    TEST_CASE_TIPS("config digital gain is %d", td.digital_gain);
    TEST_CASE_TIPS("config analog gain is %d", td.analog_gain);
    TEST_CASE_TIPS("config mix gain is %d", td.mix_gain);
    TEST_CASE_TIPS("config priod is %d bytes", td.period);
    TEST_CASE_TIPS("config mode is %d", td.mode);

    if (voice_data == NULL) {
        voice_data = malloc(td.data_size);

        if (voice_data == NULL) {
            TEST_CASE_WARN_QUIT("input voice data size is too big, malloc fail");
        }
    }

    uint8_t *cache_buffer = NULL;
    cache_buffer = malloc(td.buffer_size);

    if (cache_buffer == NULL) {
        free(voice_data);
        TEST_CASE_WARN_QUIT("cache buffer is too big, malloc fail");
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

    volatile uint8_t irq_num = 1;
    //uint8_t excp_num = 0;
    ret_sta = csi_codec_input_attach_callback(&ch, input_callback, (void *)&irq_num);
    if (ret_sta != 0) {
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_attach_callback ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d config fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
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

    reading_lock = 1;
    free_size = td.data_size - use_size;
    ret_num = csi_codec_input_read(&ch, voice_data + 0, free_size);
    if (ret_num != use_size) {
        TEST_CASE_TIPS("csi_codec_input_read ret is %d,assert ERROR", ret_num);
        TEST_CASE_ASSERT(ret_num == free_size, "codec output sync read error");
    }

    while (reading_lock == 1) {
        tst_mdelay(100);
    }

    csi_codec_input_stop(&ch);

    // excp_num = td.data_size / td.period;
    // if (td.data_size % td.period) {
    //     excp_num++;
    // }

    if ((td.data_size / td.period != 0) && (irq_num == 0)) {
        TEST_CASE_ASSERT(1 == 0, "the number of entry callback wrong");
    }

    ret_sta = csi_codec_input_link_dma(&ch, NULL);
    if (ret_sta != 0)
    TEST_CASE_ASSERT(ret_sta == 0, "codec syncinput unlink dma error");
    csi_codec_input_detach_callback(&ch);

    csi_codec_input_close(&ch);
    csi_codec_uninit(&hd);
    free(voice_data);
    return 0;   
}

int test_codec_asyncInput(void *args)
{
    csi_error_t ret_sta;
    csi_codec_t hd;
    csi_codec_input_config_t ch_config;
    csi_codec_input_t ch;
    test_codec_args_t td;
    csi_ringbuf_t input_ring_buffer;

    uint8_t *voice_data = NULL;

    td.codec_idx = *((uint64_t *)args);
    td.ch_idx = *((uint64_t *)args + 1);
    td.sample_rate = *((uint64_t *)args + 2);
    td.bit_width = *((uint64_t *)args + 3);
    td.sound_channel_num = *((uint64_t *)args + 4);
    td.data_size = *((uint64_t *)args + 5);
    td.buffer_size = *((uint64_t *)args + 6);
    td.digital_gain = *((uint64_t *)args + 7);
    td.analog_gain = *((uint64_t *)args + 8);
    td.mix_gain = *((uint64_t *)args + 9);
    td.period = *((uint64_t *)args + 10);
    td.mode = *((uint64_t *)args + 11);

    TEST_CASE_TIPS("test codec idx is %d", td.codec_idx);
    TEST_CASE_TIPS("test codec channel is %d", td.ch_idx);
    TEST_CASE_TIPS("config sample rate is %d Hz", td.sample_rate);
    TEST_CASE_TIPS("config converter bit width is %d", td.bit_width);
    TEST_CASE_TIPS("config number of soundtrack per channel is %d", td.sound_channel_num);
    TEST_CASE_TIPS("config output data size is %d bytes", td.data_size);
    TEST_CASE_TIPS("config buffer size is %d", td.buffer_size);
    TEST_CASE_TIPS("config digital gain is %d", td.digital_gain);
    TEST_CASE_TIPS("config analog gain is %d", td.analog_gain);
    TEST_CASE_TIPS("config mix gain is %d", td.mix_gain);
    TEST_CASE_TIPS("config priod is %d bytes", td.period);
    TEST_CASE_TIPS("config mode is %d", td.mode);
    if (voice_data == NULL) {
        voice_data = malloc(td.data_size);

        if (voice_data == NULL) {
            TEST_CASE_WARN_QUIT("input voice data size is too big, malloc fail");
        }
    }
    uint8_t *cache_buffer = NULL;
    cache_buffer = malloc(td.buffer_size);

    if (cache_buffer == NULL) {
        free(voice_data);
        TEST_CASE_WARN_QUIT("cache buffer is too big, malloc fail");
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

    volatile uint8_t irq_num = 1;
    //uint8_t excp_num = 0;
    ret_sta = csi_codec_input_attach_callback(&ch, input_callback, (void *)&irq_num);
    if (ret_sta != 0) {
        csi_codec_uninit(&hd);
        free(cache_buffer);
        free(voice_data);
        TEST_CASE_TIPS("csi_codec_input_attach_callback ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d config fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
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

    uint32_t free_size = 0, use_size = 0;
    do {
        free_size = td.data_size - use_size;
        reading_lock = 1;
        ret_sta = csi_codec_input_read_async(&ch, voice_data + use_size, free_size);
        use_size += free_size;
    } while (use_size < td.data_size);

    while (reading_lock == 1) {
        tst_mdelay(100);
    }

    csi_codec_input_stop(&ch);

    // excp_num = td.data_size / td.period;
    // if (td.data_size % td.period) {
    //     excp_num++;
    // }

    if ((td.data_size / td.period != 0) && (irq_num == 0)) {
        TEST_CASE_ASSERT(1 == 0, "the number of entry callback wrong");
    }

    ret_sta = csi_codec_input_link_dma(&ch, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "codec asyncinput unlink dma error");
    csi_codec_input_detach_callback(&ch);

    csi_codec_input_close(&ch);

    csi_codec_uninit(&hd);
    free(voice_data);
    return 0;
}

int test_codec_inputMute(void *args)
{
    csi_error_t ret_sta;
    csi_codec_t hd;
    csi_codec_input_config_t ch_config;
    csi_codec_input_t ch;
    test_codec_args_t td;
    csi_ringbuf_t input_ring_buffer;

    uint8_t *voice_data = NULL;

    td.codec_idx = *((uint64_t *)args);
    td.ch_idx = *((uint64_t *)args + 1);
    td.sample_rate = *((uint64_t *)args + 2);
    td.bit_width = *((uint64_t *)args + 3);
    td.sound_channel_num = *((uint64_t *)args + 4);
    td.data_size = *((uint64_t *)args + 5);
    td.buffer_size = *((uint64_t *)args + 6);
    td.digital_gain = *((uint64_t *)args + 7);
    td.analog_gain = *((uint64_t *)args + 8);
    td.mix_gain = *((uint64_t *)args + 9);
    td.period = *((uint64_t *)args + 10);
    td.mode = *((uint64_t *)args + 11);

    TEST_CASE_TIPS("test codec idx is %d", td.codec_idx);
    TEST_CASE_TIPS("test codec channel is %d", td.ch_idx);
    TEST_CASE_TIPS("config sample rate is %d Hz", td.sample_rate);
    TEST_CASE_TIPS("config converter bit width is %d", td.bit_width);
    TEST_CASE_TIPS("config number of soundtrack per channel is %d", td.sound_channel_num);
    TEST_CASE_TIPS("config output data size is %d bytes", td.data_size);
    TEST_CASE_TIPS("config buffer size is %d", td.buffer_size);
    TEST_CASE_TIPS("config digital gain is %d", td.digital_gain);
    TEST_CASE_TIPS("config analog gain is %d", td.analog_gain);
    TEST_CASE_TIPS("config mix gain is %d", td.mix_gain);
    TEST_CASE_TIPS("config priod is %d bytes", td.period);
    TEST_CASE_TIPS("config mode is %d", td.mode);

    if (voice_data == NULL) {
        voice_data = malloc(td.data_size);

        if (voice_data == NULL) {
            TEST_CASE_WARN_QUIT("input voice data size is too big, malloc fail");
        }
    }

    uint8_t *cache_buffer = NULL;
    cache_buffer = malloc(td.buffer_size);

    if (cache_buffer == NULL) {
        free(voice_data);
        TEST_CASE_WARN_QUIT("cache buffer is too big, malloc fail");
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
    
    tst_mdelay(10);
    ret_sta = csi_codec_input_mute(&ch, true);
    TEST_CASE_ASSERT_QUIT(ret_sta == 0, "codec channel %d input mute fail, expected return value is 0, actual value is %d",
                          td.ch_idx, ret_sta);

    uint32_t  free_size = 0, use_size = 0;
    do {
        free_size = td.data_size - use_size;
        ret_sta = csi_codec_input_read_async(&ch, voice_data + use_size, free_size);
        use_size += free_size;
    } while (use_size < td.data_size);

    csi_codec_input_stop(&ch);

    ret_sta = csi_codec_input_link_dma(&ch, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "codec mute input unlink dma error");

    csi_codec_input_detach_callback(&ch);

    csi_codec_input_close(&ch);

    csi_codec_uninit(&hd);
    free(voice_data);

    return 0;
}

