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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <aos/aos.h>
#include <aos/kernel.h>
#include <aos/cli.h>

#include  "pcm_data.h"
#include <codec_test.h>
#include <stdio.h>
#include <drv/codec.h>

/* 句柄空间一般使用静态空间 */    
#define OUTPUT_BUF_SIZE 2048
uint8_t output_buf[OUTPUT_BUF_SIZE];

#define INPUT_BUF_SIZE  2048
uint8_t input_buf[INPUT_BUF_SIZE];


volatile uint8_t cb_output_transfer_flag = 0;
volatile uint8_t cb_input_transfer_flag = 0;

#define input_check(ret)                              \
    do {                                              \
        if (ret < 0) {                                \
            printf("ret:%d, line:%d", ret, __LINE__); \
            while(1);                                 \
        }                                             \
    } while (0)

#define INPUT_SAMPLE_RATE  (16000)
#define INPUT_SAMPLE_BITS  (16)
#define INPUT_CHANNELS     (3)

#define READ_TIME (5000)  //MS
#define INPUT_TIME (400)  //MS
#define PERIOD_TIME (20)  //MS

#define FRAME_SIZE (INPUT_SAMPLE_RATE * INPUT_SAMPLE_BITS / 8 / 1000 * INPUT_CHANNELS)
#define INPUT_BUFFER_SIZE FRAME_SIZE * INPUT_TIME
#define INPUT_PERIOD_SIZE FRAME_SIZE * PERIOD_TIME
#define READ_BUFFER_SIZE FRAME_SIZE * READ_TIME

static csi_codec_t              g_codec;
static csi_codec_input_t        g_input_hdl;
static csi_dma_ch_t             dma_ch_input_handle;
static csi_codec_input_config_t g_input_config;
static uint32_t                 g_input_size;
static aos_sem_t                g_input_sem;
static csi_ringbuf_t            input_ring_buffer;
static uint8_t                  start_run;
static uint8_t                  g_input_buf[INPUT_BUFFER_SIZE];
static uint8_t                  g_read_buffer[READ_BUFFER_SIZE];


#define output_check(ret)                             \
    do {                                              \
        if (ret < 0) {                                \
            printf("ret:%d, line:%d", ret, __LINE__); \
            while(1);                                 \
        }                                             \
    } while (0)
// output
#define HW_OUTBUF_SIZE      (17600*3)
#define OUTPUT_SAMPLE_RATE  (16000)
#define OUTPUT_SAMPLE_BITS  (16)
#define OUTPUT_CHANNELS     (2)

#define OUTPUT_PERIOD_EVENT (0x01)
#define OUTPUT_EMPTY_EVENT  (0x02)

static csi_codec_output_t        g_output_hdl;
static csi_codec_output_config_t g_output_config;
static csi_dma_ch_t              g_dma_hdl;
static csi_ringbuf_t             g_output_ring_buffer;
static uint8_t                   g_hw_outbuf[HW_OUTBUF_SIZE];
static aos_event_t               g_output_evt;
static uint8_t                  *g_audio_outbuf;
static uint32_t                  g_audio_outsize; 
static uint32_t                  g_snd_size;


static void codec_output_event_cb(csi_codec_output_t *output, csi_codec_event_t event, void *arg)
{
    if (!aos_event_is_valid(&g_output_evt)) {
        printf("maybe csi_codec_output_stop err\r\n");
        while(1);
    }
    if (event == CODEC_EVENT_PERIOD_WRITE_COMPLETE) {
        aos_event_set(&g_output_evt, OUTPUT_PERIOD_EVENT, AOS_EVENT_OR);
    }

    if (event == CODEC_EVENT_WRITE_BUFFER_EMPTY) {
        aos_event_set(&g_output_evt, OUTPUT_EMPTY_EVENT, AOS_EVENT_OR);
    }
}


int test_codec_outputBuffer(void *args)
{
    csi_error_t ret_sta;
    csi_codec_t hd;
    csi_codec_output_config_t ch_config;
    csi_codec_output_t ch;
    test_codec_args_t td;
    csi_ringbuf_t output_ring_buffer;

    td.codec_idx = *((uint64_t *)args);
    td.ch_idx = *((uint64_t *)args + 1);
    td.buffer_size = *((uint64_t *)args + 2);

    TEST_CASE_TIPS("test codec idx is %d", td.codec_idx);
    TEST_CASE_TIPS("test codec channel is %d", td.ch_idx);
    TEST_CASE_TIPS("config buffer size is %d", td.buffer_size);

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

    ch.ring_buf = &output_ring_buffer;
    ret_sta = csi_codec_output_open(&hd, &ch, td.ch_idx);
    if (ret_sta != 0) {
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_open ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec output channel %d open fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }

    ch_config.buffer = cache_buffer;
    ch_config.buffer_size = td.buffer_size;
    ch_config.bit_width = 8;
    ch_config.mode = CODEC_OUTPUT_DIFFERENCE;
    ch_config.period = 512;
    ch_config.sample_rate = 48000;

    ret_sta = csi_codec_output_config(&ch, &ch_config);
    if (ret_sta != 0) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_TIPS("csi_codec_output_config ret is %d,assert ERROR", ret_sta);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d config fail, expected return value is 0, actual value is %d", td.ch_idx,
                              ret_sta);
    }

    uint32_t ret_num;

    ret_sta = csi_codec_output_buffer_reset(&ch);
    TEST_CASE_ASSERT(ret_sta == 0,
                     "codec channel %d cache buffer reset fail, expected return value is 0, actual value is %d", td.ch_idx, ret_sta);
    
    ret_num = csi_codec_output_buffer_avail(&ch);
    TEST_CASE_ASSERT(ret_num != td.buffer_size, "the value of free cache size not equal get avali");

    csi_dma_ch_t dma_ch;
    ret_sta = csi_codec_output_link_dma(&ch, &dma_ch);

    if (ret_sta != 0) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d link DMA fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }

    ret_sta = csi_codec_output_start(&ch);

    if (ret_sta != 0) {
        csi_codec_output_close(&ch);
        csi_codec_uninit(&hd);
        free(cache_buffer);
        TEST_CASE_ASSERT_QUIT(1 == 0, "codec channel %d start output fail, expected return value is 0, actual value is %d",
                              td.ch_idx, ret_sta);
    }

    ret_num = csi_codec_output_write(&ch, (void *)0x4, (td.buffer_size / 4));
    TEST_CASE_ASSERT(ret_num == td.buffer_size / 4,
                     "codec channel %d sync write buffer fail, expected return value is %d, actual value is %d", td.ch_idx,
                     td.buffer_size / 4, ret_num);

    ret_num = csi_codec_output_buffer_avail(&ch);
    TEST_CASE_ASSERT(ret_num != td.buffer_size, "the value of free cache size not equal get avali");

    ret_num = csi_codec_output_write_async(&ch, (void *)0x4, td.buffer_size);

    ret_sta = csi_codec_output_buffer_reset(&ch);
    TEST_CASE_ASSERT(ret_sta == 0,
                     "codec channel %d cache buffer reset fail, expected return value is 0, actual value is %d", td.ch_idx, ret_sta);

    ret_num = csi_codec_output_buffer_avail(&ch);
    TEST_CASE_ASSERT(ret_num != td.buffer_size, "the value of free cache size not equal get avali");

    ret_sta = csi_codec_output_link_dma(&ch, NULL);
    TEST_CASE_ASSERT(ret_sta == 0, "codec outputbuffer unlink dma error");

    csi_codec_output_close(&ch);
    csi_codec_uninit(&hd);

    return 0;

}


int test_codec_syncOutput_4K(void *args)
{
    // output
    uint32_t write_len;

    g_snd_size = 0;
    g_audio_outbuf  = (uint8_t*)pcm_16K_16bits_2ch;
    g_audio_outsize = (uint32_t)pcm_16K_16bits_2ch_len;

    aos_event_new(&g_output_evt, 0);
    int ret = csi_codec_init(&g_codec, 0);
    output_check(ret);

    g_output_hdl.ring_buf          = &g_output_ring_buffer;
    ret = csi_codec_output_open(&g_codec, &g_output_hdl, 0);
    output_check(ret);
    ret = csi_codec_output_attach_callback(&g_output_hdl, codec_output_event_cb, NULL);
    output_check(ret);

    g_output_config.bit_width         = 16;
    g_output_config.sample_rate       = 44100;
    g_output_config.buffer            = g_hw_outbuf;
    g_output_config.buffer_size       = HW_OUTBUF_SIZE;
    g_output_config.period            = 2048;
    g_output_config.mode              = CODEC_OUTPUT_SINGLE_ENDED;
    g_output_config.sound_channel_num = 2;

    ret = csi_codec_output_config(&g_output_hdl, &g_output_config);
    output_check(ret);
    ret = csi_codec_output_analog_gain(&g_output_hdl, 0x0);   ///< 设置声音大小0x2f
    output_check(ret);
    ret = csi_codec_output_digital_gain(&g_output_hdl, 0xffad); // 0xffad
    output_check(ret);
    ret = csi_codec_output_link_dma(&g_output_hdl, &g_dma_hdl);
    output_check(ret);
    ret = csi_codec_output_start(&g_output_hdl);
    output_check(ret);

    // output
    write_len = 4096;
    unsigned int flag = 0;
    while (1) {
        ret = csi_codec_output_write_async(&g_output_hdl, (void *)(g_audio_outbuf + g_snd_size), write_len);
        tst_mdelay(10);
        if (ret != write_len) {
            g_snd_size += ret;
            aos_event_get(&g_output_evt, OUTPUT_EMPTY_EVENT|OUTPUT_PERIOD_EVENT , AOS_EVENT_OR_CLEAR, &flag, AOS_WAIT_FOREVER);
        } else {
            g_snd_size += write_len;
        }
        
        if (g_snd_size >= g_audio_outsize) {
            aos_event_get(&g_output_evt, OUTPUT_EMPTY_EVENT, AOS_EVENT_OR_CLEAR, &flag, AOS_WAIT_FOREVER);
        }
        
        if (flag & OUTPUT_EMPTY_EVENT) {
            flag = 0;
            break;
        }
    }

    csi_codec_output_stop(&g_output_hdl);
    aos_event_free(&g_output_evt);
    csi_codec_output_link_dma(&g_output_hdl, NULL);
    csi_codec_output_detach_callback(&g_output_hdl);
    csi_codec_output_close(&g_output_hdl);

    return 0;
}


static void codec_input_event_cb_fun_read(csi_codec_input_t *i2s, csi_codec_event_t event, void *arg)
{
    if (event == CODEC_EVENT_PERIOD_READ_COMPLETE) {
        aos_sem_signal(&g_input_sem);
    }
}

static void input_wait(void) {

    int ret = csi_codec_input_buffer_avail(&g_input_hdl);
    if (ret < INPUT_PERIOD_SIZE) {
        aos_sem_wait(&g_input_sem, AOS_WAIT_FOREVER);
    }
}

int test_codec_syncInput_5s(void *args)
{
    int ret;
    start_run = 1;
    aos_sem_new(&g_input_sem, 0);
    ret = csi_codec_init(&g_codec, 0);

    if (ret != CSI_OK) {
        LOG("csi_codec_init error\n");
        return 0;
    }

    g_input_hdl.ring_buf          = &input_ring_buffer;
    g_input_hdl.sound_channel_num = INPUT_CHANNELS;
    ret = csi_codec_input_open(&g_codec, &g_input_hdl, 0);
    input_check(ret);
    ret = csi_codec_input_attach_callback(&g_input_hdl, codec_input_event_cb_fun_read, NULL);
    input_check(ret);

    /* input ch config */
    g_input_config.bit_width         = INPUT_SAMPLE_BITS;
    g_input_config.sample_rate       = INPUT_SAMPLE_RATE;
    g_input_config.buffer            = g_input_buf;
    g_input_config.buffer_size       = INPUT_BUFFER_SIZE;
    g_input_config.period            = INPUT_PERIOD_SIZE;
    g_input_config.mode              = CODEC_INPUT_DIFFERENCE;
    g_input_config.sound_channel_num = INPUT_CHANNELS;
    ret = csi_codec_input_config(&g_input_hdl, &g_input_config);
    input_check(ret);
    ret = csi_codec_input_analog_gain(&g_input_hdl, 0x8);
    input_check(ret);
    ret = csi_codec_input_digital_gain(&g_input_hdl, 25);
    input_check(ret);
    ret = csi_codec_input_link_dma(&g_input_hdl, &dma_ch_input_handle);
    input_check(ret);
    ret = csi_codec_input_start(&g_input_hdl);
    input_check(ret);

    uint32_t size   = 0;
    uint32_t r_size = 0;
    g_input_size     = 0;
    TEST_CASE_TIPS("input start(%lld)", aos_now_ms());
    while (1) {
        input_wait();
        r_size = (g_input_size + INPUT_PERIOD_SIZE) < READ_BUFFER_SIZE ? INPUT_PERIOD_SIZE : (READ_BUFFER_SIZE-g_input_size);
        size = csi_codec_input_read_async(&g_input_hdl, g_read_buffer + g_input_size, r_size);
        if (size != INPUT_PERIOD_SIZE) {
            TEST_CASE_TIPS("read size = (%u)", size);
            break;
        }
        g_input_size += r_size;
    }
    TEST_CASE_TIPS("input stop, get (%d)ms data (%lld)", READ_TIME, aos_now_ms());
    for(int loop = 0; loop <= 10; loop++)
    {
        TEST_CASE_TIPS("g_read_buffer = (%d)", g_read_buffer[loop]);
    }
    aos_sem_free(&g_input_sem);
    csi_codec_input_stop(&g_input_hdl);
    csi_codec_input_link_dma(&g_input_hdl, NULL);
    csi_codec_input_detach_callback(&g_input_hdl);
    csi_codec_uninit(&g_codec);
    start_run = 0;

    return 0;
}
