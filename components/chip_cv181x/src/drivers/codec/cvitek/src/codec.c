/* SPDX-License-Identifier: GPL-2.0-or-later
 * CV181X CODEC driver on CVITEK CV181X
 *
 * Copyright 2022 CVITEK
 *
 * Author: Ruilong.Chen
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys_clk.h>
#include <drv/i2s.h>
#include <drv/cvi_irq.h>
#include <drv/codec.h>
#include "cv181x_adc_dac.h"
#include "ringbuffer.h"


static csi_i2s_t i2s_codec_output;
static csi_i2s_t i2s_codec_input;


#define INPUT_I2S_IDX       0U
#define OUTPUT_I2S_IDX      3U

__attribute__((section(".ram.code"))) static void cvitek_codec_irq(csi_i2s_t *i2s, csi_i2s_event_t event, void *arg)
{
    csi_codec_t *codec_cb = (csi_codec_t *) arg;

    if (event == I2S_EVENT_RECEIVE_COMPLETE) {
        if (codec_cb->input_chs->callback) {
            codec_cb->input_chs->callback(codec_cb->input_chs, CODEC_EVENT_PERIOD_READ_COMPLETE, codec_cb->input_chs->arg);
        }
    } else if (event == I2S_EVENT_SEND_COMPLETE) {
        if (codec_cb->output_chs->callback) {
            codec_cb->output_chs->callback(codec_cb->output_chs, CODEC_EVENT_PERIOD_WRITE_COMPLETE, codec_cb->output_chs->arg);
        }
    } else if (event == I2S_EVENT_ERROR) {
        if (i2s->dev.idx == OUTPUT_I2S_IDX) {
            if (codec_cb->output_chs->callback) {
                codec_cb->output_chs->callback(codec_cb->output_chs, CODEC_EVENT_ERROR, codec_cb->output_chs->arg);
            }
        } else if (i2s->dev.idx == INPUT_I2S_IDX) {
            if (codec_cb->input_chs->callback) {
                codec_cb->input_chs->callback(codec_cb->input_chs, CODEC_EVENT_ERROR, codec_cb->input_chs->arg);
            }
        }
    } else if (event == I2S_EVENT_TX_BUFFER_EMPTY) {
        if (i2s->dev.idx == OUTPUT_I2S_IDX) {
            if (codec_cb->output_chs->callback) {
                codec_cb->output_chs->callback(codec_cb->output_chs, CODEC_EVENT_WRITE_BUFFER_EMPTY, codec_cb->output_chs->arg);
            }
        }
    } else if (event == I2S_EVENT_RX_BUFFER_FULL) {
        if (i2s->dev.idx == INPUT_I2S_IDX) {
            if (codec_cb->input_chs->callback) {
                codec_cb->input_chs->callback(codec_cb->input_chs, CODEC_EVENT_READ_BUFFER_FULL, codec_cb->input_chs->arg);
            }
        }
    }

}

/**
  \brief  Init the codec according to the specified
  \param[in]   codec codec handle to operate.
  \param[in]   idx   codec interface idx
  \return      error code
*/
csi_error_t csi_codec_init(csi_codec_t *codec, uint32_t idx)
{
    CSI_PARAM_CHK(codec, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    switch(idx) {
    case 0:
        ret = csi_i2s_init(&i2s_codec_input, idx);   ///< init input i2s
        csi_i2s_attach_callback(&i2s_codec_input, cvitek_codec_irq, (void *)codec);
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        ret = csi_i2s_init(&i2s_codec_output, idx);    ///< init output i2s
        csi_i2s_attach_callback(&i2s_codec_output, cvitek_codec_irq, (void *)codec);
        break;

    }
    return ret;
}

/**
  \brief       codec uninit
  \param[in]   codec codec handle to operate.
  \return      none
*/
void csi_codec_uninit(csi_codec_t *codec)
{
    CSI_PARAM_CHK_NORETVAL(codec);
    csi_i2s_uninit(&i2s_codec_input);
    csi_i2s_uninit(&i2s_codec_output);
    return;
}

/**
  \brief  Open a codec output channel
  \param[in]   codec codec handle to operate.
  \param[in]   ch codec output channel handle.
  \param[in]   ch_idx codec output channel idx.
  \return      error code
*/
csi_error_t csi_codec_output_open(csi_codec_t *codec, csi_codec_output_t *ch, uint32_t ch_idx)
{
    CSI_PARAM_CHK(codec, CSI_ERROR);
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    codec->output_chs = ch;
    ch->state.error = 0U;
    ch->state.readable = 0U;
    ch->state.writeable = 0U;
    ch->callback = NULL;
    return ret;
}

/**
  \brief  Config codec output channel
  \param[in]   ch codec output channel handle.
  \param[in]   config codec channel prarm.
  \return      error code
*/
csi_error_t csi_codec_output_config(csi_codec_output_t *ch, csi_codec_output_config_t *config)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(config, CSI_ERROR);
    csi_error_t ret =  CSI_OK;
    csi_i2s_format_t i2s_format;
    struct cvi_vol_ctrl vol;
    vol.vol_ctrl = 0;
    vol.vol_ctrl_mute = 0;

    ch->ring_buf->buffer = config->buffer;
    ch->ring_buf->size = config->buffer_size;
    ch->period = config->period;
    ch->sound_channel_num = config->sound_channel_num;

    if (i2s_codec_output.dev.reg_base == 0U) {
        ret = CSI_ERROR;
    } else {
        i2s_format.mode = I2S_MODE_MASTER;
        i2s_format.protocol = I2S_PROTOCOL_I2S;
        i2s_format.width = (csi_i2s_sample_width_t)config->bit_width;
        i2s_format.rate = (csi_i2s_sample_rate_t)config->sample_rate;
        i2s_format.polarity = I2S_LEFT_POLARITY_LOW;
        csi_i2s_format(&i2s_codec_output, &i2s_format);
        csi_i2s_tx_select_sound_channel(&i2s_codec_output, ch->sound_channel_num == 1 ? I2S_LEFT_CHANNEL : I2S_LEFT_RIGHT_CHANNEL);

        csi_i2s_tx_set_buffer(&i2s_codec_output, ch->ring_buf);
        csi_i2s_tx_buffer_reset(&i2s_codec_output);
        ret = csi_i2s_tx_set_period(&i2s_codec_output, ch->period);
        cv182xdac_init(config->sample_rate, 2);
        cv182xdac_ioctl(ACODEC_SET_OUTPUT_VOL, vol, 12);

    }

    return ret;
}

/**
  \brief       Attach the callback handler to codec output
  \param[in]   ch codec output channel handle.
  \param[in]   cb    callback function
  \param[in]   arg   user can define it by himself as callback's param
  \return      error code
*/
csi_error_t csi_codec_output_attach_callback(csi_codec_output_t *ch, void *callback, void *arg)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ch->callback = callback;
    ch->arg = arg;
    return ret;
}

/**
  \brief       detach the callback handler
  \param[in]   ch codec output channel handle.
  \return      none
*/
void csi_codec_output_detach_callback(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    ch->callback = NULL;
    ch->arg = NULL;
    csi_i2s_detach_callback(&i2s_codec_output);
}

/**
  \brief  Close a CODEC output channel
  \param[in]   ch codec output channel handle.
  \return      error code
*/
void csi_codec_output_close(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    ch->state.writeable = 0U;
    ch->state.readable  = 0U;
    ch->state.error     = 0U;
    //csi_i2s_uninit(&i2s_codec_output);
}

/**
  \brief       link DMA channel to codec output channel
  \param[in]   ch codec output channel handle.
  \param[in]   dma dma channel info.
  \return      error code
*/
csi_error_t csi_codec_output_link_dma(csi_codec_output_t *ch, csi_dma_ch_t *dma)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ch->dma = dma;
    ret = csi_i2s_tx_link_dma(&i2s_codec_output, dma);
    return ret;
}

/**
  \brief       write an amount of data to cache in blocking mode.
  \param[in]   ch    the codec output channel
  \param[in]   data  send data.
  \param[in]   size  receive data byte_size.
  \return      The num of data witch is send successful
*/
uint32_t csi_codec_output_write(csi_codec_output_t *ch, const void *data, uint32_t size)
{
    return csi_i2s_send(&i2s_codec_output, data, size);
}

/**
  \brief Write data to the cache.
  \With asynchronous sending,
  \the data is first written to the cache and then output through the codec interface.
  \This function does not block, and the return value is the number
  \of data that was successfully written to the cache.
  \param[in]   ch    the codec output channel
  \param[in]   data  send data.
  \param[in]   size  receive data size.
  \return      The data size that write to cache
*/
uint32_t csi_codec_output_write_async(csi_codec_output_t *ch, const void *data, uint32_t size)
{
    return csi_i2s_send_async(&i2s_codec_output, data, size);
}

/**
  \brief       Start sending data from the cache
  \param[in]   ch codec output channel handle.
  \return      error code
*/
csi_error_t csi_codec_output_start(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    ch->state.writeable = 1U;
    return csi_i2s_send_start(&i2s_codec_output);
}

/**
  \brief       Stop sending data from the cache
  \param[in]   ch codec output channel handle.
  \return      error code
*/
void csi_codec_output_stop(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    ch->state.writeable = 0U;
    csi_i2s_send_stop(&i2s_codec_output);
}

/**
  \brief       Pause sending data from the cache
  \param[in]   ch codec output channel handle.
  \return      error code
*/
csi_error_t csi_codec_output_pause(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    ch->state.writeable = 0U;
    return csi_i2s_send_pause(&i2s_codec_output);
}

/**
  \brief       Resume sending data from the cache
  \param[in]   ch codec output channel handle.
  \return      error code
*/
csi_error_t csi_codec_output_resume(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    ch->state.writeable = 1U;
    return csi_i2s_send_resume(&i2s_codec_output);
}

/**
  \brief  Get cache free space
  \param[in]   ch codec output channel handle.
  \return buffer free space (bytes)
*/
uint32_t csi_codec_output_buffer_avail(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    return ringbuffer_avail(ch->ring_buf);
}

/**
  \brief       Get buffer data num
  \param[in]   ch    Codec output channel handle
  \return      Buffer free space (bytes)
*/
uint32_t csi_codec_output_buffer_remain(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    return ringbuffer_len(ch->ring_buf);
}

/**
  \brief  Reset the buf, discard all data in the cache
  \param[in]   ch codec output channel handle.
  \return      error code
*/
csi_error_t csi_codec_output_buffer_reset(csi_codec_output_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    ringbuffer_reset(ch->ring_buf);

    if (ch->ring_buf->buffer != NULL) {
        memset(ch->ring_buf->buffer, 0, ch->ring_buf->size);
    }

    if ((ch->ring_buf->read == 0U) && (ch->ring_buf->write == 0U)) {
        ret = CSI_OK;
    }

    return ret;
}

/**
  \brief       mute codec ouput channel
  \param[in]   ch codec output channel handle.
  \param[in]   en true codec mute.
  \return      error code
*/
csi_error_t csi_codec_output_mute(csi_codec_output_t *ch, bool enable)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    struct cvi_vol_ctrl vol;
    if(enable){
      vol.vol_ctrl = 0;
      vol.vol_ctrl_mute = 1;

      //_/ es8156_set_software_mute(&es8156_dev, true);
      cv182xdac_ioctl(ACODEC_SET_DACR_MUTE, vol, 1);
      cv182xdac_ioctl(ACODEC_SET_DACL_MUTE, vol, 1);
    }
    return ret;
}

/**
  \brief       Set codec ouput channel digital gain.
  \param[in]   ch codec output channel handle.
  \param[in]   val gain val.
  \return      error code
*/
csi_error_t csi_codec_output_digital_gain(csi_codec_output_t *ch, uint32_t val)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_UNSUPPORTED;
    struct cvi_vol_ctrl vol;
    vol.vol_ctrl = 0;
    vol.vol_ctrl_mute = 0;

    if(val == 0)
        vol.vol_ctrl_mute = 1;

    cv182xdac_ioctl(ACODEC_SET_OUTPUT_VOL, vol, val);
    return ret;
}

/**
  \brief       Set codec ouput channel analog gain.
  \param[in]   ch codec output channel handle.
  \param[in]   val gain val.
  \return      error code
*/
csi_error_t csi_codec_output_analog_gain(csi_codec_output_t *ch, uint32_t val)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    struct cvi_vol_ctrl vol;
    vol.vol_ctrl = 0;
    vol.vol_ctrl_mute = 0;

    if(val)
        vol.vol_ctrl_mute = 0;
    else
        vol.vol_ctrl_mute = 1;
    cv182xdac_ioctl(ACODEC_SET_OUTPUT_VOL, vol, val);
    return ret;
}

/**
  \brief       Set codec ouput channel mix gain.
  \param[in]   ch codec output channel handle.
  \param[in]   val gain val.
  \return      error code
*/
csi_error_t csi_codec_output_mix_gain(csi_codec_output_t *ch, uint32_t val)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_UNSUPPORTED;
    return ret;
}

/**
  \brief       Get codec output channel state.
  \param[in]   ch codec output channel handle.
  \param[out]  state channel state.
  \return      channel state
*/
csi_error_t csi_codec_output_get_state(csi_codec_output_t *ch, csi_state_t *state)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    memcpy(state, &(ch->state), sizeof(csi_state_t));
    return ret;
}

/**
  \brief  Open a CODEC input channel
  \param[in]   codec codec handle to operate.
  \param[in]   ch codec input channel handle.
  \param[in]   ch_idx codec input channel idx.
  \return      error code
*/
csi_error_t csi_codec_input_open(csi_codec_t *codec, csi_codec_input_t *ch, uint32_t ch_idx)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(codec, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    codec->input_chs = ch;
    ch->state.error = 0U;
    ch->state.readable = 0U;
    ch->state.writeable = 0U;
    ch->callback = NULL;
    return ret;
}

/**
  \brief  Config codec input channel
  \param[in]   ch codec input channel handle.
  \param[in]   config codec channel prarm.
  \return      error code
*/
csi_error_t csi_codec_input_config(csi_codec_input_t *ch, csi_codec_input_config_t *config)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(config, CSI_ERROR);
    csi_error_t ret =  CSI_OK;
    csi_i2s_format_t i2s_format;
    struct cvi_vol_ctrl vol;
    vol.vol_ctrl = 0;
    vol.vol_ctrl_mute = 0;

    ch->ring_buf->buffer = config->buffer;
    ch->ring_buf->size = config->buffer_size;
    ch->period = config->period;
    ch->sound_channel_num = config->sound_channel_num;

    if (i2s_codec_input.dev.reg_base == 0U) {
        ret = CSI_ERROR;
    } else {
        i2s_format.mode = I2S_MODE_SLAVE;
        i2s_format.protocol = I2S_PROTOCOL_I2S;
        i2s_format.width = (csi_i2s_sample_width_t)config->bit_width;
        i2s_format.rate = (csi_i2s_sample_rate_t)config->sample_rate;
        i2s_format.polarity = I2S_LEFT_POLARITY_LOW;
        csi_i2s_format(&i2s_codec_input, &i2s_format);
        csi_i2s_rx_select_sound_channel(&i2s_codec_input, ch->sound_channel_num == 1 ? I2S_LEFT_CHANNEL : I2S_LEFT_RIGHT_CHANNEL);
        csi_i2s_rx_set_buffer(&i2s_codec_input, ch->ring_buf);
        csi_i2s_rx_buffer_reset(&i2s_codec_input);
        ret = csi_i2s_rx_set_period(&i2s_codec_input, ch->period);
        cv182xadc_init(config->sample_rate);
        cv182xadc_ioctl(ACODEC_SET_ADCL_VOL, vol, 8);
    }
    return ret;
}

/**
  \brief       Attach the callback handler to codec output
  \param[in]   ch codec input channel handle.
  \param[in]   cb    callback function
  \param[in]   arg   user can define it by himself as callback's param
  \return      error code
*/
csi_error_t csi_codec_input_attach_callback(csi_codec_input_t *ch, void *callback, void *arg)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ch->callback = callback;
    ch->arg = arg;
    return ret;
}

/**
  \brief       detach the callback handler
  \param[in]   ch codec input channel handle.
  \return      none
*/
void csi_codec_input_detach_callback(csi_codec_input_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    ch->callback = NULL;
    ch->arg = NULL;
    csi_i2s_detach_callback(&i2s_codec_input);
}

/**
  \brief  Close a CODEC input channel
  \param[in]   ch codec input channel handle..
  \return      error code
*/
void csi_codec_input_close(csi_codec_input_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    ch->state.writeable = 0U;
    ch->state.readable  = 0U;
    ch->state.error     = 0U;
    //csi_i2s_uninit(&i2s_codec_input);
}

/**
  \brief       link DMA channel to codec input channel
  \param[in]   ch codec input channel handle.
  \param[in]   dma dma channel info.
  \return      error code
*/
csi_error_t csi_codec_input_link_dma(csi_codec_input_t *ch, csi_dma_ch_t *dma)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ch->dma = dma;
    ret = csi_i2s_rx_link_dma(&i2s_codec_input, dma);
    return ret;
}

/**
  \brief  read an amount of data in blocking mode.
  \param[in]   ch codec input channel handle.
  \param[in]   data  save receive data.
  \param[in]   size  receive data size.
  \return      The size of data receive successfully
*/
uint32_t csi_codec_input_read(csi_codec_input_t *ch, void *data, uint32_t size)
{
    return csi_i2s_receive(&i2s_codec_input, data, size);
}

/**
  \brief Read data from the cache.
  \Using asynchronous receive.
  \This function read data from the cache, returns the number of successful reads,
  \and returns 0 if there is no data in the cache.
  \param[in]   ch codec input channel handle.
  \param[in]   data  the buf save receive data.
  \param[in]   size  receive data size.
  \return      The size of data read successfully
*/
uint32_t csi_codec_input_read_async(csi_codec_input_t *ch, void *data, uint32_t size)
{
    return csi_i2s_receive_async(&i2s_codec_input, data, size);
}

/**
  \brief       Start receive data, writes the recording data to the cache
  \param[in]   ch codec input channel handle.
  \return      error code
*/
csi_error_t csi_codec_input_start(csi_codec_input_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    ch->state.readable = 1U;
    return csi_i2s_receive_start(&i2s_codec_input);
}

/**
  \brief       Stop receive data.
  \param[in]   ch codec input channel handle.
  \return      error code
*/
void csi_codec_input_stop(csi_codec_input_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    ch->state.readable = 0U;
    csi_i2s_receive_stop(&i2s_codec_input);
}

/**
  \brief       Get buffer free space
  \param[in]   ch    Codec input channel handle
  \return      Buffer free space (bytes)
*/
uint32_t csi_codec_input_buffer_avail(csi_codec_input_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    return ringbuffer_avail(ch->ring_buf);
}

/**
  \brief       Get buffer data num
  \param[in]   ch    Codec input channel handle
  \return      Buffer free space (bytes)
*/
uint32_t csi_codec_input_buffer_remain(csi_codec_input_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    return ringbuffer_len(ch->ring_buf);
}

/**
  \brief  Reset the buf, discard all data in the cache
  \param[in]   ch codec input channel handle.
  \return      error code
*/
csi_error_t csi_codec_input_buffer_reset(csi_codec_input_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    ringbuffer_reset(ch->ring_buf);

    if (ch->ring_buf->buffer != NULL) {
        memset(ch->ring_buf->buffer, 0, ch->ring_buf->size);
    }

    if ((ch->ring_buf->read == 0U) && (ch->ring_buf->write == 0U)) {
        ret = CSI_OK;
    }

    return ret;
}

/**
  \brief       mute codec input channel
  \param[in]   ch codec input channel handle.
  \param[in]   en true codec mute.
  \return      error code
*/
csi_error_t csi_codec_input_mute(csi_codec_input_t *ch, bool en)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    struct cvi_vol_ctrl vol;

    if(en){
      vol.vol_ctrl = 0;
      vol.vol_ctrl_mute = 1;

      cv182xadc_ioctl(ACODEC_SET_MICL_MUTE, vol, 1);
      cv182xadc_ioctl(ACODEC_SET_MICR_MUTE, vol, 1);
    }
    return ret;
}

/**
  \brief       Set codec input channel digital gain.
  \param[in]   ch codec input channel handle.
  \param[in]   val gain val.
  \return      error code
*/
csi_error_t csi_codec_input_digital_gain(csi_codec_input_t *ch, uint32_t val)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_UNSUPPORTED;
    return ret;
}

/**
  \brief       Set codec input channel analog gain.
  \param[in]   ch codec input channel handle.
  \param[in]   val gain val.
  \return      error code
*/
csi_error_t csi_codec_input_analog_gain(csi_codec_input_t *ch, uint32_t val)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    struct cvi_vol_ctrl vol;
    vol.vol_ctrl = 0;
    vol.vol_ctrl_mute = 0;

    if(val)
        vol.vol_ctrl_mute = 0;
    else
        vol.vol_ctrl_mute = 1;
    cv182xadc_ioctl(ACODEC_SET_ADCL_VOL, vol, val);
#if defined (CONFIG_CHIP_cv1811h) || defined (CONFIG_CHIP_cv1812h) || (CONFIG_CHIP_cv1811ha) || (CONFIG_CHIP_cv1812ha) || (CONFIG_CHIP_cv1813h)
    cv182xadc_ioctl(ACODEC_SET_ADCR_VOL, vol, val);
#endif
    return ret;
}

/**
  \brief       Set codec input channel mix gain.
  \param[in]   ch codec input channel handle.
  \param[in]   val gain val.
  \return      error code
*/
csi_error_t csi_codec_input_mix_gain(csi_codec_input_t *ch, uint32_t val)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_UNSUPPORTED;
    return ret;
}

/**
  \brief       Get codec input channel state.
  \param[in]   ch codec input channel handle.
  \param[out]  state channel state.
  \return      channel state
*/
csi_error_t csi_codec_input_get_state(csi_codec_input_t *ch, csi_state_t *state)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    memcpy(state, &(ch->state), sizeof(csi_state_t));
    return ret;
}




