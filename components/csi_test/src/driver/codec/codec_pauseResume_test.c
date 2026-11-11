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


int test_codec_pauseResume(void *args)
{
    csi_error_t ret_sta;
    csi_codec_t hd;
    csi_codec_output_config_t ch_config;
    csi_codec_output_t ch;
    test_codec_args_t td;
    csi_ringbuf_t output_ring_buffer;

    td.codec_idx = *((uint64_t *)args);
    td.ch_idx = *((uint64_t *)args + 1);
    td.sample_rate = *((uint64_t *)args + 2);
    td.bit_width = *((uint64_t *)args + 3);
    td.sound_channel_num = *((uint64_t *)args + 4);
    td.data_size = *((uint64_t *)args + 5);
    td.buffer_size = *((uint64_t *)args + 6);
    td.mode = *((uint64_t *)args + 7);

    TEST_CASE_TIPS("test codec idx is %d", td.codec_idx);
    TEST_CASE_TIPS("test codec channel is %d", td.ch_idx);
    TEST_CASE_TIPS("config sample rate is %d Hz", td.sample_rate);
    TEST_CASE_TIPS("config converter bit width is %d", td.bit_width);
    TEST_CASE_TIPS("config number of soundtrack per channel is %d", td.sound_channel_num);
    TEST_CASE_TIPS("config output data size is %d bytes", td.data_size);
    TEST_CASE_TIPS("config buffer size is %d", td.buffer_size);
    TEST_CASE_TIPS("config mode is %d", td.mode);

    uint8_t *cache_buffer = NULL;
    cache_buffer = malloc(td.buffer_size);

    if (cache_buffer == NULL) {
        TEST_CASE_WARN_QUIT("cache buffer is too big, malloc fail");
    }

    ret_sta = csi_codec_init(&hd, td.codec_idx);
    if (ret_sta != 0) {
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_init ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "uart %d init fail, expected return value is 0, actual value is %d", td.codec_idx,
                              ret_sta);
    }

    ch_config.sample_rate = td.sample_rate;
    ch_config.bit_width = td.bit_width;
    ch_config.buffer = cache_buffer;
    ch_config.buffer_size = td.buffer_size;
    ch_config.sound_channel_num = td.sound_channel_num;
    ch_config.period = 512;
    ch_config.mode = td.mode;

    ch.ring_buf = &output_ring_buffer;
    ret_sta = csi_codec_output_config(&ch, &ch_config);
    if (ret_sta != 0) {
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_config ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d config fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
    }


    ret_sta = csi_codec_output_open(&hd, &ch, td.ch_idx);
    if (ret_sta != 0) {
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_open ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d open fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
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

    ret_num = csi_codec_output_write(&ch, (void *)0x04, td.data_size);
    TEST_CASE_ASSERT(ret_num == td.data_size,
                     "codec channel %d sync write buffer fail, expected return value is %d, actual value is %d", td.ch_idx, td.data_size,
                     ret_num);

    ret_sta = csi_codec_output_pause(&ch);
    TEST_CASE_ASSERT(ret_sta == 0, "codec channel %d pause fail, expected return value is 0, actual value is %d", td.ch_idx,
                     ret_sta);

    tst_mdelay(10);

    uint32_t free_size1 = 0, free_size2 = 0;
    free_size1 = csi_codec_output_buffer_avail(&ch);
    tst_mdelay(10);
    free_size2 = csi_codec_output_buffer_avail(&ch);
    if ((free_size1 != free_size2) || (free_size2 == td.buffer_size)) {
        TEST_CASE_ASSERT(1 == 0, "codec channel %d pause fail", td.ch_idx);
    }

    ret_sta = csi_codec_output_resume(&ch);
    tst_mdelay(10);
    free_size1 = csi_codec_output_buffer_avail(&ch); 
    if (free_size1 == free_size2) {
        TEST_CASE_ASSERT(1 == 0, "codec channel %d resume fail", td.ch_idx);
    }

    ret_sta = csi_codec_output_link_dma(&ch, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "codec pauseResune unlink dma error");

    csi_codec_output_stop(&ch);
    csi_codec_output_close(&ch);

    csi_codec_uninit(&hd);

    return 0;
}
