/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

#include <string.h>
#include <soc.h>
#include <drv/mbox.h>
#include <drv/cvi_irq.h>
#include <drv/porting.h>
#include <wj_mbox_ll.h>

/**
  \brief       wj_mbox_get_target_cpu_channel
  \param[in]   current_cpu
  \param[in]   current_channel
  \param[in]   target_cpu
  \param[in]   target_channel
*/
static csi_error_t wj_mbox_get_target_cpu_channel(uint32_t current_cpu, uint32_t current_channel, uint32_t *target_cpu, uint32_t *target_channel)
{
    csi_error_t ret = CSI_OK;

    switch (current_cpu) {
        case WJ_CPU0:
            if (current_channel == 0U) {
                *target_cpu = WJ_CPU1;
                *target_channel = 0U;
            } else if (current_channel == 1U) {
                *target_cpu = WJ_CPU2;
                *target_channel = 1U;
            } else if (current_channel == 2U) {
                *target_cpu = WJ_CPU3;
                *target_channel = 0U;
            }

            break;

        case WJ_CPU1:
            if (current_channel == 0U) {
                *target_cpu = WJ_CPU0;
                *target_channel = 0U;
            } else if (current_channel == 1U) {
                *target_cpu = WJ_CPU2;
                *target_channel = 0U;
            } else if (current_channel == 2U) {
                *target_cpu = WJ_CPU3;
                *target_channel = 2U;
            }

            break;

        case WJ_CPU2:
            if (current_channel == 0U) {
                *target_cpu = WJ_CPU1;
                *target_channel = 1U;
            } else if (current_channel == 1U) {
                *target_cpu = WJ_CPU0;
                *target_channel = 1U;
            } else if (current_channel == 2U) {
                *target_cpu = WJ_CPU3;
                *target_channel = 1U;
            }

            break;

        case WJ_CPU3:
            if (current_channel == 0U) {
                *target_cpu = WJ_CPU0;
                *target_channel = 2U;
            } else if (current_channel == 1U) {
                *target_cpu = WJ_CPU2;
                *target_channel = 2U;
            } else if (current_channel == 2U) {
                *target_cpu = WJ_CPU1;
                *target_channel = 2U;
            }

            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

/**
  \brief       wj_box_irq_handler
  \param[in]   mbox handle of mbox instance
*/
static void wj_mbox_irq_handler(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    csi_mbox_t *mbox = (csi_mbox_t *)arg;
    wj_mbox_regs_t *mbox_base;
    mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);
    int32_t current_cpu_id = -1;
    uint32_t inter_state;
    uint32_t info_state = 0U;
    uint32_t data_len = 0U;
    uint32_t ack_data = 0U;
    current_cpu_id = (int32_t)soc_get_cpu_id();

    uint32_t target_cpu = 0U;
    uint32_t target_channel = 0U;

    inter_state = wj_read_cpu_interrupt_state(mbox_base, (uint32_t)current_cpu_id);

    for (uint32_t i = 0U; i < CHN_NUM; i++) {
        if (inter_state & (uint32_t)((uint32_t)1U << i)) {
            wj_mbox_get_target_cpu_channel((uint32_t)current_cpu_id, i, &target_cpu, &target_channel);
            wj_clear_cpu_ch_interrupt(mbox_base, (uint32_t)current_cpu_id, i);
            info_state = wj_read_gen_cpu_ch_interrupt(mbox_base, (uint32_t)current_cpu_id, i); ///< read info irq state
            wj_write_gen_cpu_ch_interrupt(mbox_base, (uint32_t)current_cpu_id, i, 0U);         ///< clear info state

            if (info_state & WJ_GEN_INFO_7) {                                        ///< this is ack signal
                ack_data = wj_read_cpu_ch_info(mbox_base, (uint32_t)current_cpu_id, i, ACK_BIT);
                ack_data = ((ack_data >> 16U) & 0xffffU);
                wj_write_cpu_ch_info(mbox_base, (uint32_t)current_cpu_id, target_channel, ACK_BIT, 0U);   ///< clear ack

                if (ack_data == MBOX_ACK) {
                    if (mbox->callback) {
                        mbox->callback(mbox, MBOX_EVENT_SEND_COMPLETE, i, 0U, mbox->arg);
                    }
                } else {
                    if (mbox->callback) {
                        mbox->callback(mbox, MBOX_EVENT_ERROR, i, 0U, mbox->arg);
                    }
                }

            } else if (info_state & WJ_GEN_INFO_0) {
                data_len = wj_read_cpu_ch_info(mbox_base, (uint32_t)current_cpu_id, i, ACK_BIT);
                data_len = (uint32_t)((data_len >> 4U) & 0xfffU);
                wj_write_cpu_ch_info(mbox_base, (uint32_t)current_cpu_id, i, ACK_BIT, 0U);   ///< clear ack

                if (mbox->callback) {
                    mbox->callback(mbox, MBOX_EVENT_RECEIVED, i, data_len, mbox->arg);
                }
            }
        }
    }
}

/**
  \brief       Initialize MAILBOX Interface. 1. Initializes the resources needed for the MAILBOX interface 2.registers event callback function
  \param[in]   mbox    operate handle
  \param[in]   idx     the device idx
  \return      error code
*/
csi_error_t csi_mbox_init(csi_mbox_t *mbox, uint32_t idx)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    int32_t current_cpu_id = -1;
    ret = target_get(DEV_WJ_MBOX_TAG, idx, &mbox->dev);

    if (ret == CSI_OK) {
        wj_mbox_regs_t *mbox_base;
        mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);

        current_cpu_id = (int32_t)soc_get_cpu_id();

        for (uint32_t i = 0U; i < CHN_NUM; i++) {
            memset((void *)((volatile uint32_t *)&mbox_base->MBOX_CPU[current_cpu_id].MBOX_CH[i].MBOX_INFO), 0, INFO_NUM * 4U); ///< clear CPU0 INFO register
            wj_write_cpu_ch_interrupt_mask(mbox_base, (uint32_t)current_cpu_id, i, 0U); ///< mask CPU0 all channel interrupt
        }

        for (uint32_t i = 0U; i < CHN_NUM; i++) {
            wj_write_cpu_ch_interrupt_mask(mbox_base, (uint32_t)current_cpu_id, i, 1U); ///< unmask CPU0 channel 0 interrupt
            wj_clear_cpu_ch_interrupt(mbox_base, (uint32_t)current_cpu_id, i); ///< clear CPU0 channel 0 interrupt
        }
    }

    return ret;
}

/**
  \brief       De-initialize MAILBOX Interface. stops operation and releases the software resources used by the interface
  \param[in]   mbox    operate handle
*/
void csi_mbox_uninit(csi_mbox_t *mbox)
{
    CSI_PARAM_CHK_NORETVAL(mbox);
    wj_mbox_regs_t *mbox_base;
    mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);
    int32_t current_cpu_id = -1;
    current_cpu_id = (int32_t)soc_get_cpu_id();

    for (uint32_t i = 0U; i < CHN_NUM; i++) {
        wj_write_cpu_ch_interrupt_mask(mbox_base, (uint32_t)current_cpu_id, i, 0U); ///< mask CPU0 all read interrupt
        wj_clear_cpu_ch_interrupt(mbox_base, (uint32_t)current_cpu_id, i); ///< clear CPU0 interrupt
    }

    csi_irq_detach((uint32_t)mbox->dev.irq_num);
    csi_irq_disable((uint32_t)mbox->dev.irq_num);
}

/**
  \brief       Start sending data to MAILBOX transmitter,(received data is ignored).
               This function is non-blocking,\ref mailbox_event_e is signaled when operation completes or error happens.
               \ref csi_mailbox_get_status can get operation status.
  \param[in]   mbox       operate handle
  \param[in]   channel_id index of channel
  \param[in]   data       Pointer to buffer with data to send to MAILBOX transmitter.
  \param[in]   size       size of data items to send
  \return      sent number of data
*/
int32_t csi_mbox_send(csi_mbox_t *mbox, uint32_t channel_id, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret;
    wj_mbox_regs_t *mbox_base;
    mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);

    int32_t tx_len = (size > WJ_MBOX_SEND_MAX_MESSAGE_LENGTH) ? (int32_t)WJ_MBOX_SEND_MAX_MESSAGE_LENGTH : (int32_t)size;

    int32_t current_cpu_id = -1;
    current_cpu_id = (int32_t)soc_get_cpu_id();

    uint32_t target_cpu = 0U;
    uint32_t target_channel = 0U;

    ret = wj_mbox_get_target_cpu_channel((uint32_t)current_cpu_id, channel_id, &target_cpu, &target_channel);

    if (ret == CSI_OK) {
        uint32_t mbox_busy = wj_read_cpu_ch_info(mbox_base, target_cpu, target_channel, ACK_BIT);

        if (mbox_busy & 0x1U) {
            tx_len = 0;
        } else {
            uint32_t buf[(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH + 3U) >> 2U] = {0U};
            uint32_t gen_interrupt = 0U;
            memcpy(&buf, data, (uint32_t)tx_len);

            for (uint32_t count = (uint32_t)(((uint32_t)tx_len + 3U) / 4U); count > 0U; count--) {
                wj_write_cpu_ch_info(mbox_base, target_cpu, target_channel, (count - 1U), buf[count - 1U]);
                gen_interrupt += (uint32_t)((uint32_t)1U << (count - 1U));
            }

            /* send tx_len to target channel */
            wj_write_cpu_ch_info(mbox_base, target_cpu, target_channel, ACK_BIT, (uint32_t)(((uint32_t)tx_len & 0xfffU) << 4U));

            wj_write_gen_cpu_ch_interrupt(mbox_base, target_cpu, target_channel, gen_interrupt);
        }



    } else {
        tx_len = -1;
    }

    return tx_len;
}

/**
  \brief       Start Receiving data from Mailbox receiver.
  \param[in]   mbox   operate handle.
  \param[in]   channel_id index of channel
  \param[out]  data   Pointer to buffer with data to receive from mailbox.
  \param[in]   size   Number of data items to receive
  \return      received number or  error code
*/
int32_t csi_mbox_receive(csi_mbox_t *mbox, uint32_t channel_id, void *data, uint32_t size)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    wj_mbox_regs_t *mbox_base;
    mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);
    int32_t rx_len = (int32_t)size;
    int32_t current_cpu_id = -1;

    uint32_t buf[(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH + 3U) >> 2U] = {0U};

    current_cpu_id = (int32_t)soc_get_cpu_id();
    uint32_t target_cpu = 0U;
    uint32_t target_channel = 0U;
    wj_mbox_get_target_cpu_channel((uint32_t)current_cpu_id, channel_id, &target_cpu, &target_channel);

    if (channel_id < CHN_NUM) {
        for (uint32_t i = 0U; i < ((size + 3U) >> 2U); i++) {
            buf[i] = wj_read_cpu_ch_info(mbox_base, (uint32_t)current_cpu_id, channel_id, i);
            wj_write_cpu_ch_info(mbox_base, (uint32_t)current_cpu_id, channel_id, (uint32_t)i, 0U);
        }

        wj_write_gen_cpu_ch_interrupt(mbox_base, target_cpu, target_channel, WJ_GEN_INFO_7);   /// clear gen
        memcpy(data, buf, size);

        wj_write_cpu_ch_info(mbox_base, target_cpu, target_channel, ACK_BIT, (MBOX_ACK << 16U) | 1U); ///< the LSB bit is meaning info is busy
        wj_write_gen_cpu_ch_interrupt(mbox_base, target_cpu, target_channel, WJ_GEN_INFO_7);   ///< ack signal
    } else {
        rx_len = -1;
    }


    return rx_len;
}

/**
\brief       Attach callback to the mbox
\param[in]   mbox    operate handle.
\param[in]   cb      event callback function \ref csi_mbox_callback_t
\param[in]   arg     user private param  for event callback
\return      error code
*/
csi_error_t csi_mbox_attach_callback(csi_mbox_t *mbox, void *callback, void *arg)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    CSI_PARAM_CHK(callback, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    mbox->callback = callback;
    mbox->arg = arg;
    csi_irq_attach((uint32_t)mbox->dev.irq_num, &wj_mbox_irq_handler, &mbox->dev);
    csi_irq_enable((uint32_t)mbox->dev.irq_num);
    return ret;
}

/**
\brief       detach callback from the mbox
\param[in]   mbox   operate handle.
*/
void csi_mbox_detach_callback(csi_mbox_t *mbox)
{
    CSI_PARAM_CHK_NORETVAL(mbox);
    mbox->callback  = NULL;
    mbox->arg = NULL;
    csi_irq_detach((uint32_t)mbox->dev.irq_num);
    csi_irq_disable((uint32_t)mbox->dev.irq_num);
}
