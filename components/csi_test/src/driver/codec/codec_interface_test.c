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

int test_codec_interface(void *args)
{
    csi_error_t ret_status;
    uint32_t ret_size;
    csi_codec_t hd;
    csi_codec_output_t ch_output_hd;
    csi_codec_output_config_t output_config;
    csi_dma_ch_t ch_dma;
    uint8_t data[10];
    csi_state_t state;
    csi_codec_input_t ch_input_hd;
    csi_codec_input_config_t input_config;


    // csi_error_t csi_codec_init(csi_codec_t *codec, uint32_t idx);
    ret_status = csi_codec_init(NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: codec is NULL, csi_codec_init expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_init(&hd, 0xFFFFFFFF);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: idx is 0xFFFFFFFF, csi_codec_init expected return value is -1, actual return value is %d.", ret_status);

    // void csi_codec_uninit(csi_codec_t *codec);
    csi_codec_uninit(NULL);

    // init hd for test
    int ret = csi_codec_init(&hd, 0);
    TEST_CASE_ASSERT_QUIT(ret == CSI_OK, "csi_codec_init failed, skip rest cases");

    // csi_error_t csi_codec_output_open(csi_codec_t *codec, csi_codec_output_t *ch, uint32_t ch_idx);
    ret_status = csi_codec_output_open(NULL, &ch_output_hd, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: codec is NULL, csi_codec_output_open expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_output_open(&hd, NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_open expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_output_open(&hd, &ch_output_hd, 0xFFFFFFFF);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch_idx is 0xFFFFFFFF, csi_codec_output_open expected return value is -1, actual return value is %d.",
                     ret_status);

    // init ch_output_hd for test
    ret_status = csi_codec_output_open(&hd, &ch_output_hd, 0);
    TEST_CASE_ASSERT_QUIT(ret_status == CSI_OK, "csi_codec_output_open failed, skip rest cases");

    // csi_error_t csi_codec_output_config(csi_codec_output_t *ch, csi_codec_output_config_t *config);
    ret_status = csi_codec_output_config(NULL, &output_config);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_config expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_output_config(&ch_output_hd, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: config is NULL, csi_codec_output_config expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_output_attach_callback(csi_codec_output_t *ch, void *callback, void *arg);
    ret_status = csi_codec_output_attach_callback(NULL, NULL, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_attach_callback expected return value is -1, actual return value is %d.",
                     ret_status);

    // void csi_codec_output_detach_callback(csi_codec_output_t *ch);
    csi_codec_output_detach_callback(NULL);

    // void csi_codec_output_close(csi_codec_output_t *ch);
    csi_codec_output_close(NULL);

    // csi_error_t csi_codec_output_link_dma(csi_codec_output_t *ch, csi_dma_ch_t *dma);
    ret_status = csi_codec_output_link_dma(NULL, &ch_dma);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_link_dma expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_output_link_dma(&ch_output_hd, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: dma is NULL, csi_codec_output_link_dma expected return value is -1, actual return value is %d.",
                     ret_status);

    // uint32_t csi_codec_output_write(csi_codec_output_t *ch, const void *data, uint32_t size);
    ret_size = csi_codec_output_write(NULL, (void *)data, sizeof(data));
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_write expected return value is -1, actual return value is %d.", ret_size);
    ret_size = csi_codec_output_write(&ch_output_hd, NULL, sizeof(data));
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: data is NULL, csi_codec_output_write expected return value is -1, actual return value is %d.", ret_size);
    ret_size = csi_codec_output_write(&ch_output_hd, (void *)data, 0);
    TEST_CASE_ASSERT(ret_size == 0,
                     "parameter: size is 0, csi_codec_output_write expected return value is 0, actual return value is %d.", ret_size);

    // uint32_t csi_codec_output_write_async(csi_codec_output_t *ch, const void *data, uint32_t size);
    ret_size = csi_codec_output_write_async(NULL, (void *)data, sizeof(data));
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_write_async expected return value is -1, actual return value is %d.",
                     ret_size);
    ret_size = csi_codec_output_write_async(&ch_output_hd, NULL, sizeof(data));
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: data is NULL, csi_codec_output_write_async expected return value is -1, actual return value is %d.",
                     ret_size);
    ret_size = csi_codec_output_write_async(&ch_output_hd, (void *)data, 0);
    TEST_CASE_ASSERT(ret_size == 0,
                     "parameter: size is 0, csi_codec_output_write_async expected return value is 0, actual return value is %d.", ret_size);

    // csi_error_t csi_codec_output_start(csi_codec_output_t *ch);
    ret_status = csi_codec_output_start(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_start expected return value is -1, actual return value is %d.", ret_status);

    // void csi_codec_output_stop(csi_codec_output_t *ch);
    csi_codec_output_stop(NULL);

    // csi_error_t csi_codec_output_pause(csi_codec_output_t *ch);
    ret_status = csi_codec_output_pause(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_pause expected return value is -1, actual return value is %d.", ret_status);

    // csi_error_t csi_codec_output_resume(csi_codec_output_t *ch);
    ret_status = csi_codec_output_resume(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_resume expected return value is -1, actual return value is %d.", ret_status);

    // uint32_t csi_codec_output_buffer_avail(csi_codec_output_t *ch);
    ret_size = csi_codec_output_buffer_avail(NULL);
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_buffer_avail expected return value is -1, actual return value is %d.",
                     ret_size);

    // uint32_t csi_codec_output_buffer_remain(csi_codec_output_t *ch);
    ret_size = csi_codec_output_buffer_remain(NULL);
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_buffer_remain expected return value is -1, actual return value is %d.",
                     ret_size);

    // csi_error_t csi_codec_output_buffer_reset(csi_codec_output_t *ch);
    ret_status = csi_codec_output_buffer_reset(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_buffer_reset expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_output_mute(csi_codec_output_t *ch, bool enable);
    ret_status = csi_codec_output_mute(NULL, false);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_mute expected return value is -1, actual return value is %d.", ret_status);

    // csi_error_t csi_codec_output_digital_gain(csi_codec_output_t *ch, uint32_t val);
    ret_status = csi_codec_output_digital_gain(NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_digital_gain expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_output_analog_gain(csi_codec_output_t *ch, uint32_t val);
    ret_status = csi_codec_output_analog_gain(NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_analog_gain expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_output_mix_gain(csi_codec_output_t *ch, uint32_t val);
    ret_status = csi_codec_output_mix_gain(NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_mix_gain expected return value is -1, actual return value is %d.", ret_status);

    // csi_error_t csi_codec_output_get_state(csi_codec_output_t *ch, csi_state_t *state);
    ret_status = csi_codec_output_get_state(NULL, &state);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_output_get_state expected return value is -1, actual return value is %d.",
                     ret_status);
    ret_status = csi_codec_output_get_state(&ch_output_hd, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: state is NULL, csi_codec_output_get_state expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_input_open(csi_codec_t *codec, csi_codec_input_t *ch, uint32_t ch_idx);
    ret_status = csi_codec_input_open(NULL, &ch_input_hd, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: codec is NULL, csi_codec_input_open expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_input_open(&hd, NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_open expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_input_open(&hd, &ch_input_hd, 0xFFFFFFFF);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch_idx is 0xFFFFFFFF, csi_codec_input_open expected return value is -1, actual return value is %d.",
                     ret_status);

    // init ch_input_hd for test
    ret_status = csi_codec_input_open(&hd, &ch_input_hd, 0);
    TEST_CASE_ASSERT_QUIT(ret_status == CSI_OK, "csi_codec_input_open failed, skip rest cases");

    // csi_error_t csi_codec_input_config(csi_codec_input_t *ch, csi_codec_input_config_t *config);
    ret_status = csi_codec_input_config(NULL, &input_config);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_config expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_input_config(&ch_input_hd, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: config is NULL, csi_codec_input_config expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_input_attach_callback(csi_codec_input_t *ch, void *callback, void *arg);
    ret_status = csi_codec_input_attach_callback(NULL, NULL, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_attach_callback expected return value is -1, actual return value is %d.",
                     ret_status);

    // void csi_codec_input_detach_callback(csi_codec_input_t *ch);
    csi_codec_input_detach_callback(NULL);

    // void csi_codec_input_close(csi_codec_input_t *ch);
    csi_codec_input_close(NULL);

    // csi_error_t csi_codec_input_link_dma(csi_codec_input_t *ch, csi_dma_ch_t *dma);
    ret_status = csi_codec_input_link_dma(NULL, &ch_dma);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_link_dma expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_input_link_dma(&ch_input_hd, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: dma is NULL, csi_codec_input_link_dma expected return value is -1, actual return value is %d.", ret_status);

    // uint32_t csi_codec_input_read(csi_codec_input_t *ch, void *data, uint32_t size);
    ret_size = csi_codec_input_read(NULL, (void *)data, sizeof(data));
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_read expected return value is -1, actual return value is %d.", ret_size);
    ret_size = csi_codec_input_read(&ch_input_hd, NULL, sizeof(data));
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: data is NULL, csi_codec_input_read expected return value is -1, actual return value is %d.", ret_size);
    ret_size = csi_codec_input_read(&ch_input_hd, (void *)data, 0);
    TEST_CASE_ASSERT(ret_size == 0,
                     "parameter: size is 0, csi_codec_input_read expected return value is 0, actual return value is %d.", ret_size);

    // uint32_t csi_codec_input_read_async(csi_codec_input_t *ch, void *data, uint32_t size);
    ret_size = csi_codec_input_read_async(NULL, (void *)data, sizeof(data));
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_read_async expected return value is -1, actual return value is %d.", ret_size);
    ret_size = csi_codec_input_read_async(&ch_input_hd, NULL, sizeof(data));
    TEST_CASE_ASSERT(ret_size == CSI_ERROR,
                     "parameter: data is NULL, csi_codec_input_read_async expected return value is -1, actual return value is %d.",
                     ret_size);
    ret_size = csi_codec_input_read_async(&ch_input_hd, (void *)data, 0);
    TEST_CASE_ASSERT(ret_size == 0,
                     "parameter: size is 0, csi_codec_input_read_async expected return value is 0, actual return value is %d.", ret_size);

    // csi_error_t csi_codec_input_start(csi_codec_input_t *ch);
    ret_status = csi_codec_input_start(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_start expected return value is -1, actual return value is %d.", ret_status);

    // void csi_codec_input_stop(csi_codec_input_t *ch);
    csi_codec_input_stop(NULL);

    // csi_error_t csi_codec_input_buffer_reset(csi_codec_input_t *ch);
    ret_status = csi_codec_input_buffer_reset(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_buffer_reset expected return value is -1, actual return value is %d.",
                     ret_status);

    // uint32_t csi_codec_input_buffer_avail(csi_codec_input_t *ch);
    ret_status = csi_codec_input_buffer_avail(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_buffer_avail expected return value is -1, actual return value is %d.",
                     ret_status);

    // uint32_t csi_codec_input_buffer_remain(csi_codec_input_t *ch);
    ret_status = csi_codec_input_buffer_remain(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_buffer_remain expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_input_mute(csi_codec_input_t *ch, bool en);
    ret_status = csi_codec_input_mute(NULL, false);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_mute expected return value is -1, actual return value is %d.", ret_status);

    // csi_error_t csi_codec_input_digital_gain(csi_codec_input_t *ch, uint32_t val);
    ret_status = csi_codec_input_digital_gain(NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_digital_gain expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_input_analog_gain(csi_codec_input_t *ch, uint32_t val);
    ret_status = csi_codec_input_analog_gain(NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_analog_gain expected return value is -1, actual return value is %d.",
                     ret_status);

    // csi_error_t csi_codec_input_mix_gain(csi_codec_input_t *ch, uint32_t val);
    ret_status = csi_codec_input_mix_gain(NULL, 0);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_mix_gain expected return value is -1, actual return value is %d.", ret_status);

#ifdef CSI_CODEC_GET_STATE
    // csi_error_t csi_codec_input_get_state(csi_codec_input_t *ch, csi_state_t *state);
    ret_status = csi_codec_input_get_state(NULL, &state);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_input_get_state expected return value is -1, actual return value is %d.", ret_status);
    ret_status = csi_codec_input_get_state(&ch_input_hd, NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: state is NULL, csi_codec_input_get_state expected return value is -1, actual return value is %d.",
                     ret_status);
#endif

#ifdef CONFIG_PM
    // csi_error_t csi_codec_enable_pm(csi_codec_t *codec);
    ret_status = csi_codec_enable_pm(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_enable_pm expected return value is -1, actual return value is %d.", ret_status);

    // void csi_codec_disable_pm(csi_codec_t *codec);
    ret_status = csi_codec_disable_pm(NULL);
    TEST_CASE_ASSERT(ret_status == CSI_ERROR,
                     "parameter: ch is NULL, csi_codec_disable_pm expected return value is -1, actual return value is %d.", ret_status);
#endif

    csi_codec_output_close(&ch_output_hd);
    csi_codec_input_close(&ch_input_hd);
    csi_codec_uninit(&hd);

    return 0;
}
