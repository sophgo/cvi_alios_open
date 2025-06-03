/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <sys_clk.h>
#include <drv/i2s.h>
#include <drv/cvi_irq.h>
#include <drv/codec.h>
#include <wj_i2s_ll.h>
#include "ipgoal_codec_ll.h"

extern uint16_t i2s_tx_hs_num[];
extern uint16_t i2s_rx_hs_num[];

static csi_i2s_t i2s_codec_output[2];
static csi_i2s_t i2s_codec_input[2];

const uint8_t codec_output_idx[2] = {3, 5};
const uint8_t codec_input_idx[2] = {2, 4};


static void ipgoal_codec_irq(csi_i2s_t *i2s, csi_i2s_event_t event, void *arg)
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
        if ((i2s->dev.idx == codec_output_idx[0U]) || (i2s->dev.idx == codec_output_idx[1U])) {
            if (codec_cb->output_chs->callback) {
                codec_cb->output_chs->callback(codec_cb->output_chs, CODEC_EVENT_ERROR, codec_cb->output_chs->arg);
            }
        } else if ((i2s->dev.idx == codec_input_idx[0U]) || (i2s->dev.idx == codec_input_idx[1U])) {
            if (codec_cb->input_chs->callback) {
                codec_cb->input_chs->callback(codec_cb->input_chs, CODEC_EVENT_ERROR, codec_cb->input_chs->arg);
            }
        }
    } else if (event == I2S_EVENT_TX_BUFFER_EMPTY) {
        if ((i2s->dev.idx == codec_output_idx[0U]) || (i2s->dev.idx == codec_output_idx[1U])) {
            if (codec_cb->output_chs->callback) {
                codec_cb->output_chs->callback(codec_cb->output_chs, CODEC_EVENT_WRITE_BUFFER_EMPTY, codec_cb->output_chs->arg);
            }
        }
    } else if (event == I2S_EVENT_RX_BUFFER_FULL) {
            if (codec_cb->input_chs->callback) {
                codec_cb->input_chs->callback(codec_cb->input_chs, CODEC_EVENT_READ_BUFFER_FULL, codec_cb->input_chs->arg);
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
    ret = target_get(DEV_WJ_CODEC_TAG, idx, &codec->dev);

    wj_codec_regs_t *codec_base;
    codec_base = (wj_codec_regs_t *)HANDLE_REG_BASE(codec);

    ipgoal_set_fc_reg01(codec_base, 0x01U);
    ipgoal_set_fc_reg02(codec_base, 0x00U);
    ipgoal_set_fc_reg03(codec_base, 0x4CU);
    ipgoal_set_fc_reg04(codec_base, 0x44U);
    ipgoal_set_fc_reg05(codec_base, 0xE5U);
    ipgoal_set_fc_reg06(codec_base, 0x32U);
    ipgoal_set_fc_reg07(codec_base, 0x40U);
    ipgoal_set_fc_reg08(codec_base, 0x7FU);
    ipgoal_set_fc_reg09(codec_base, 0x20U);
    ipgoal_set_fc_reg00(codec_base, 0x06U);

    csi_i2s_format_t i2s_format;

    ret = csi_i2s_init(&i2s_codec_output[codec->dev.idx], codec_output_idx[codec->dev.idx]);    ///< init output i2s
    csi_i2s_attach_callback(&i2s_codec_output[codec->dev.idx], ipgoal_codec_irq, (void *)codec);
    csi_i2s_enable(&i2s_codec_output[codec->dev.idx], false);
    wj_i2s_set_transmit_mode((wj_i2s_regs_t *)i2s_codec_output[codec->dev.idx].dev.reg_base, 1U);
    i2s_format.mode = I2S_MODE_MASTER;
    i2s_format.protocol = I2S_PROTOCOL_MSB_JUSTIFIED;
    i2s_format.width = I2S_SAMPLE_WIDTH_16BIT;
    i2s_format.rate = I2S_SAMPLE_RATE_48000;
    i2s_format.polarity = I2S_LEFT_POLARITY_HIGH;
    i2s_format.sclk_nfs = I2S_SCLK_64FS;
    i2s_format.mclk_nfs = I2S_MCLK_256FS;
    csi_i2s_format(&i2s_codec_output[codec->dev.idx], &i2s_format);
    csi_i2s_enable(&i2s_codec_output[codec->dev.idx], true);

    ret = csi_i2s_init(&i2s_codec_input[codec->dev.idx], codec_input_idx[codec->dev.idx]);   ///< init input i2s
    csi_i2s_attach_callback(&i2s_codec_input[codec->dev.idx], ipgoal_codec_irq, (void *)codec);
    csi_i2s_enable(&i2s_codec_input[codec->dev.idx], false);
    i2s_format.mode = I2S_MODE_SLAVE;
    i2s_format.protocol = I2S_PROTOCOL_MSB_JUSTIFIED;
    i2s_format.width = I2S_SAMPLE_WIDTH_16BIT;
    i2s_format.rate = I2S_SAMPLE_RATE_48000;
    i2s_format.polarity = I2S_LEFT_POLARITY_HIGH;
    i2s_format.sclk_nfs = I2S_SCLK_64FS;
    i2s_format.mclk_nfs = I2S_MCLK_256FS;
    csi_i2s_format(&i2s_codec_input[codec->dev.idx], &i2s_format);

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
    ch->codec = codec;
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

    ch->ring_buf->buffer = config->buffer;
    ch->ring_buf->size = config->buffer_size;
    ch->period = config->period;
    ch->sound_channel_num = config->sound_channel_num;

    if (i2s_codec_output[ch->codec->dev.idx].dev.reg_base == 0U) {
        ret = CSI_ERROR;
    } else {
        csi_i2s_enable(&i2s_codec_output[ch->codec->dev.idx], false);
        i2s_format.mode = I2S_MODE_MASTER;
        i2s_format.protocol = I2S_PROTOCOL_MSB_JUSTIFIED;
        i2s_format.width = (csi_i2s_sample_width_t)config->bit_width;
        i2s_format.rate = (csi_i2s_sample_rate_t)config->sample_rate;
        i2s_format.polarity = I2S_LEFT_POLARITY_HIGH;
        i2s_format.sclk_nfs = I2S_SCLK_64FS;
        i2s_format.mclk_nfs = I2S_MCLK_256FS;
        csi_i2s_format(&i2s_codec_output[ch->codec->dev.idx], &i2s_format);

        csi_i2s_tx_set_buffer(&i2s_codec_output[ch->codec->dev.idx], ch->ring_buf);
        csi_i2s_tx_buffer_reset(&i2s_codec_output[ch->codec->dev.idx]);
        ret = csi_i2s_tx_set_period(&i2s_codec_output[ch->codec->dev.idx], ch->period);
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
    csi_i2s_detach_callback(&i2s_codec_output[ch->codec->dev.idx]);
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
    csi_i2s_uninit(&i2s_codec_output[ch->codec->dev.idx]);
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
    ret = csi_i2s_tx_link_dma(&i2s_codec_output[ch->codec->dev.idx], dma);
    return ret;
}

/**
  \brief       write an amount of data to cache in blocking mode.
  \param[in]   ch    the codec output channel
  \param[in]   data  send data.
  \param[in]   size  receive data size.
  \return      The num of data witch is send successful
*/
uint32_t csi_codec_output_write(csi_codec_output_t *ch, const void *data, uint32_t size)
{
    return csi_i2s_send(&i2s_codec_output[ch->codec->dev.idx], data, size);
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
    return csi_i2s_send_async(&i2s_codec_output[ch->codec->dev.idx], data, size);
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
    return csi_i2s_send_start(&i2s_codec_output[ch->codec->dev.idx]);
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
    csi_i2s_send_stop(&i2s_codec_output[ch->codec->dev.idx]);
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
    return csi_i2s_send_pause(&i2s_codec_output[ch->codec->dev.idx]);
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
    return csi_i2s_send_resume(&i2s_codec_output[ch->codec->dev.idx]);
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
    csi_error_t ret = CSI_ERROR;

    /*
      range0 ~ 0x3f, step:0.75DB
      0（-35.25DB）... 0x2f(0DB) ... 0x3f(12DB)
    */
    if (val <= (uint8_t)0x3f) {
        ret = CSI_OK;
    }

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
    ch->codec = codec;
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

    ch->ring_buf->buffer = config->buffer;
    ch->ring_buf->size = config->buffer_size;
    ch->period = config->period;
    ch->sound_channel_num = config->sound_channel_num;

    if (i2s_codec_input[ch->codec->dev.idx].dev.reg_base == 0U) {
        ret = CSI_ERROR;
    } else {
        csi_i2s_enable(&i2s_codec_input[ch->codec->dev.idx], false);

        i2s_format.mode = I2S_MODE_SLAVE;
        i2s_format.protocol = I2S_PROTOCOL_MSB_JUSTIFIED;
        i2s_format.width = (csi_i2s_sample_width_t)config->bit_width;
        i2s_format.rate = (csi_i2s_sample_rate_t)config->sample_rate;
        i2s_format.polarity = I2S_LEFT_POLARITY_HIGH;
        i2s_format.sclk_nfs = I2S_SCLK_64FS;
        i2s_format.mclk_nfs = I2S_MCLK_256FS;
        csi_i2s_format(&i2s_codec_input[ch->codec->dev.idx], &i2s_format);

        csi_i2s_rx_set_buffer(&i2s_codec_input[ch->codec->dev.idx], ch->ring_buf);
        csi_i2s_rx_buffer_reset(&i2s_codec_input[ch->codec->dev.idx]);
        ret = csi_i2s_rx_set_period(&i2s_codec_input[ch->codec->dev.idx], ch->period);
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
    csi_i2s_detach_callback(&i2s_codec_input[ch->codec->dev.idx]);
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
    csi_i2s_uninit(&i2s_codec_input[ch->codec->dev.idx]);
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
    ret = csi_i2s_rx_link_dma(&i2s_codec_input[ch->codec->dev.idx], dma);
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
    return csi_i2s_receive(&i2s_codec_input[ch->codec->dev.idx], data, size);
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
    return csi_i2s_receive_async(&i2s_codec_input[ch->codec->dev.idx], data, size);
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
    return csi_i2s_receive_start(&i2s_codec_input[ch->codec->dev.idx]);
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
    csi_i2s_receive_stop(&i2s_codec_input[ch->codec->dev.idx]);
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
    csi_error_t ret = CSI_ERROR;

    /*
      range0 ~ 0x3f, step:0.75DB
      0（8DB）... 0x10(20DB) ... 0x3f(55.25DB)
    */
    if (val <= (uint8_t)0x3f) {
        ret = CSI_OK;
    }

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

#ifdef CONFIG_PM
csi_error_t csi_codec_enable_pm(csi_codec_t *codec)
{
    return CSI_UNSUPPORTED;
}

void csi_codec_disable_pm(csi_codec_t *codec)
{

}
#endif



