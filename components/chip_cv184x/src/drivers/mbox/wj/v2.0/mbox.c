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
    volatile uint32_t inter_state;
    uint32_t data_len = 0U;
    uint32_t ack_data = 0U;
    current_cpu_id = soc_get_cpu_id();

    inter_state = wj_read_cpu_interrupt_state(mbox_base, current_cpu_id);
    wj_clear_cpu_ch_interrupt(mbox_base, current_cpu_id, inter_state);

    if (inter_state & (1U << ACK_BIT)) {                                        ///< this is ack signal
        ack_data = wj_read_cpu_ch_info(mbox_base, current_cpu_id, ACK_BIT);
        ack_data = ack_data & 0xffff;

        if (ack_data == MBOX_ACK) {
            if (mbox->callback) {
                mbox->callback(mbox, MBOX_EVENT_SEND_COMPLETE, 0U, 0U, mbox->arg);
            }
        } else {
            if (mbox->callback) {
                mbox->callback(mbox, MBOX_EVENT_ERROR, 0U, 0U, mbox->arg);
            }
        }
    }

    if (inter_state & WJ_GEN_INFO_0) {
        data_len = wj_read_cpu_ch_info(mbox_base, current_cpu_id, ACK_BIT);
        data_len = (data_len >> 16U);

        if (mbox->callback) {
            mbox->callback(mbox, MBOX_EVENT_RECEIVED, 0U, data_len, mbox->arg);
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

        current_cpu_id = soc_get_cpu_id();

        wj_clear_cpu_ch_interrupt(mbox_base, current_cpu_id, 0xFFFFU); ///< clear CPU channel interrupt

        if (current_cpu_id == 0U) {
            memset((void *)((volatile uint32_t *)&mbox_base->MBOX_INFO0), 0U, INFO_NUM * 4U); ///< clear CPU INFO register
            wj_write_cpu_ch_interrupt_mask(mbox_base, current_cpu_id, 0xFFFFU); ///< mask CPU channel 0 interrupt
            wj_write_cpu_ch_interrupt_mask(mbox_base, current_cpu_id, 0U); ///< unmask CPU channel 0 interrupt
        }

        if (current_cpu_id == 1U) {
            memset((void *)((volatile uint32_t *)&mbox_base->MBOX_INFO1), 0U, INFO_NUM * 4U); ///< clear CPU INFO register
            wj_write_cpu_ch_interrupt_mask(mbox_base, current_cpu_id, 0xFFFFU); ///< mask CPU channel 0 interrupt
            wj_write_cpu_ch_interrupt_mask(mbox_base, current_cpu_id, 0U); ///< unmask CPU channel 0 interrupt
        }

        wj_mbox_enable_irq(mbox_base, current_cpu_id);
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
    current_cpu_id = soc_get_cpu_id();

    if (current_cpu_id == 0U) {
        memset((void *)((volatile uint32_t *)&mbox_base->MBOX_INFO0), 0U, INFO_NUM * 4U); ///< clear CPU INFO register
        wj_write_cpu_ch_interrupt_mask(mbox_base, current_cpu_id, 0xFFFFU); ///< mask CPU channel 0 interrupt
    }

    if (current_cpu_id == 1U) {
        memset((void *)((volatile uint32_t *)&mbox_base->MBOX_INFO1), 0U, INFO_NUM * 4U); ///< clear CPU INFO register
        wj_write_cpu_ch_interrupt_mask(mbox_base, current_cpu_id, 0xFFFFU); ///< mask CPU channel 0 interrupt
    }

    for (uint32_t i = 0U; i < INFO_NUM; i++) {
        wj_clear_cpu_ch_interrupt(mbox_base, current_cpu_id, i); ///< clear CPU channel interrupt
    }

    csi_irq_detach(mbox->dev.irq_num);
    csi_irq_disable(mbox->dev.irq_num);
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

    int32_t tx_len = (size > WJ_MBOX_SEND_MAX_MESSAGE_LENGTH) ? WJ_MBOX_SEND_MAX_MESSAGE_LENGTH : size;

    int32_t current_cpu_id = -1;
    current_cpu_id = soc_get_cpu_id();

    uint32_t buf[(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH + 3U) >> 2U] = {0U};
    uint32_t gen_interrupt = 0U;
    memcpy(&buf, data, tx_len);

    for (uint32_t count = ((tx_len + 3U) / 4U); count > 0U; count--) {
        wj_write_cpu_ch_info(mbox_base, current_cpu_id, (count - 1U), buf[count - 1U]);
        gen_interrupt += 1 << (count - 1U);
    }

    /* send tx_len to target channel */
    wj_write_cpu_ch_info(mbox_base, current_cpu_id, ACK_BIT, tx_len << 16U);
    wj_write_gen_cpu_ch_interrupt(mbox_base, current_cpu_id, gen_interrupt);

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
    int32_t rx_len = size;
    int32_t current_cpu_id = -1;

    uint32_t buf[(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH + 3U) >> 2U] = {0U};

    current_cpu_id = soc_get_cpu_id();

    for (uint8_t i = 0U; i < ((size + 3U) >> 2U); i++) {
        buf[i] = wj_read_cpu_ch_info(mbox_base, current_cpu_id, i);
        wj_write_cpu_ch_info(mbox_base, current_cpu_id, i, 0U);
    }

    memcpy(data, buf, size);
    wj_write_cpu_ch_info(mbox_base, current_cpu_id, ACK_BIT, MBOX_ACK);
    wj_write_gen_cpu_ch_interrupt(mbox_base, current_cpu_id, (1U << ACK_BIT));   ///< ack signal
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
    csi_irq_attach(mbox->dev.irq_num, &wj_mbox_irq_handler, &mbox->dev);
    csi_irq_enable(mbox->dev.irq_num);
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
    csi_irq_detach(mbox->dev.irq_num);
    csi_irq_disable(mbox->dev.irq_num);
}
