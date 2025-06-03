/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

#include <string.h>
#include <soc.h>
#include <drv/mbox.h>
#include <drv/cvi_irq.h>
#include <drv/porting.h>
#include "wj_mbox_ll.h"

/**
 * According to the current CPU, different channel ID correspond to different target CPU
 *     ------------------------------------------------------
 *     |              |       CH0        |       CH1        |
 *     ------------------------------------------------------
 *     |     CPU0     |       CPU1       |       CPU2       |
 *     ------------------------------------------------------
 *     |     CPU1     |       CPU0       |       CPU2       |
 *     ------------------------------------------------------
 *     |     CPU2     |       CPU0       |       CPU1       |
 *     ------------------------------------------------------
*/

#define WJ_MBOX_CPU0                               (0U)
#define WJ_MBOX_CPU1                               (1U)
#define WJ_MBOX_CPU2                               (2U)
#define WJ_MBOX_GET_CURRENT_CPU                    (soc_get_cpu_id())
#define WJ_MBOX_GET_TARGET_CPU(_ID_)               (\
        WJ_MBOX_CPU0 == WJ_MBOX_GET_CURRENT_CPU ? (_ID_ ? WJ_MBOX_CPU2 : WJ_MBOX_CPU1) :\
        WJ_MBOX_CPU1 == WJ_MBOX_GET_CURRENT_CPU ? (_ID_ ? WJ_MBOX_CPU2 : WJ_MBOX_CPU0) :\
        WJ_MBOX_CPU2 == WJ_MBOX_GET_CURRENT_CPU ? (_ID_ ? WJ_MBOX_CPU1 : WJ_MBOX_CPU0) :\
        -1)

#define WJ_MBOX_GET_TARGET_ADDR(_ID_)               (\
        WJ_MBOX_CPU0 == WJ_MBOX_GET_CURRENT_CPU ? (_ID_ ? WJ_MCC1_BASE : WJ_MCC0_BASE) :\
        WJ_MBOX_CPU1 == WJ_MBOX_GET_CURRENT_CPU ? (_ID_ ? WJ_MCC1_BASE : WJ_MCC0_BASE) :\
        WJ_MBOX_CPU2 == WJ_MBOX_GET_CURRENT_CPU ? (WJ_MCC0_BASE) :\
        -1)

#define WJ_MBOX_MCC0_MCC1_OFFSET                   (0x58000000UL)       ///< Different base address about 804 core and 805 core
#define WJ_MBOX_MAX_MESSAGE_LENGTH                 (24U)                ///< Maximum message capacity
#define WJ_MBOX_SEND_MAX_MESSAGE_LENGTH            (20U)                ///< Maximum transportable message capacity
#define WJ_MBOX_ACK                                (0x7975UL)           ///< Transmission data check value
#define UP_ALIGN4(_VAL_)                           (((_VAL_) + 3U) & 0xFFFFFFFCU)


/**
  \brief       mbox_write
  \param[in]   dst address
  \param[in]   src address
  \param[in]   len
*/
static void mbox_write(uint32_t *dst, uint32_t *src, uint32_t len)
{
    for (uint32_t i = 0U; i < len; i++) {
        dst[i] = src[i];
    }
}
/**
  \brief        mbox_get_dst_channel_base_address
  \param[in]    mbox_base    mbox handle of mbox instance
  \param[in]    channel_id   mbox channel id
  \param[out]   dst_address  target address
*/
static void mbox_get_dst_channel_base_address(wj_mbox_regs_t *mbox_base, uint32_t channel_id, uint32_t *dst_address)
{
    CSI_PARAM_CHK_NORETVAL(mbox_base);

    switch (WJ_MBOX_GET_CURRENT_CPU) {
        case WJ_MBOX_CPU0:
            if (channel_id == 0U) {
                *dst_address = (uint32_t)wj_read_cpu_command_0(WJ_MBOX_CPU1, mbox_base);
            } else if (channel_id == 1U) {
                *dst_address = (uint32_t)wj_read_cpu_command_0(WJ_MBOX_CPU2, mbox_base) + WJ_MBOX_MCC0_MCC1_OFFSET;
            }

            break;

        case WJ_MBOX_CPU1:
            if (channel_id == 0U) {
                *dst_address = (uint32_t)wj_read_cpu_command_0(WJ_MBOX_CPU0, mbox_base);
            } else if (channel_id == 1U) {
                *dst_address = (uint32_t)wj_read_cpu_command_6(WJ_MBOX_CPU2, mbox_base) + WJ_MBOX_MCC0_MCC1_OFFSET;
            }

            break;

        case WJ_MBOX_CPU2:
            if (channel_id == 0U) {
                *dst_address = (uint32_t)wj_read_cpu_command_6(WJ_MBOX_CPU0, mbox_base) -  WJ_MBOX_MCC0_MCC1_OFFSET;
            } else if (channel_id == 1U) {
                *dst_address = (uint32_t)wj_read_cpu_command_6(WJ_MBOX_CPU1, mbox_base) -  WJ_MBOX_MCC0_MCC1_OFFSET;
            }

            break;

        default:
            break;
    }
}
/**
  \brief        mbox_get_self_channel_base_address
  \param[in]    mbox_base    mbox handle of mbox instance
  \param[in]    channel_id   mbox channel id
  \param[out]   dst_address  dst_address  target address
*/
static void mbox_get_self_channel_base_address(wj_mbox_regs_t *mbox_base, uint32_t channel_id, uint32_t *dst_address)
{
    CSI_PARAM_CHK_NORETVAL(mbox_base);

    if (channel_id == 0U) {
        *dst_address = (uint32_t)wj_read_cpu_command_0(soc_get_cpu_id(), mbox_base);
    } else if (channel_id == 1U) {
        *dst_address = (uint32_t)wj_read_cpu_command_6(soc_get_cpu_id(), mbox_base);
    }
}

/**
  \brief       wj_mbox_irq_handler
  \param[in]   arg      mbox handle of mbox instance
*/
void wj_mbox_irq_handler(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    csi_mbox_t *mbox = (csi_mbox_t *)arg;
    wj_mbox_regs_t *mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);
    int32_t j;
    uint32_t inter_state;

    inter_state = wj_cpu_command_read_wr_interrupt_state(WJ_MBOX_GET_CURRENT_CPU, mbox_base);

    for (j = 0; j < CONFIG_MAILBOX_CHANNEL_NUM; j++) {
        uint32_t dst_address = 0;
        mbox_get_self_channel_base_address(mbox_base, j, (uint32_t *)&dst_address);

        /* handle the ack irq */
        if (inter_state & (j ? WJ_IUM_WCPU0_IUM_WCPU0_11 : WJ_IUM_WCPU0_IUM_WCPU0_5)) {
            volatile uint32_t ack_data = *(uint32_t *)(dst_address + UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH));
            ack_data = ((ack_data >> 16U) & 0xffff);

            if (ack_data == WJ_MBOX_ACK) {
                if (mbox->callback) {
                    mbox->callback(mbox, MBOX_EVENT_SEND_COMPLETE, j, 0U, mbox->arg);
                }

                /* clean ack data buffer */
                *(uint32_t *)(dst_address + UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH)) = 0U;
            } else {
                uint32_t data_len = 0;
                data_len = *(uint32_t *)(dst_address + UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH));
                data_len = ((data_len >> 4U) & 0xfff);

                if (data_len > 0) {
                    if (mbox->callback) {
                        mbox->callback(mbox, MBOX_EVENT_RECEIVED, j, data_len, mbox->arg);
                    }
                } else {
                    if (mbox->callback) {
                        mbox->callback(mbox, MBOX_EVENT_ERROR, j, 0U, mbox->arg);
                    }
                }
            }
        }
    }

    wj_cpu_command_write_wr_interrupt_clear(WJ_MBOX_GET_CURRENT_CPU, \
                                            mbox_base,  inter_state);
}

/**
  \brief       Initialize MAILBOX Interface.
            1. Initializes the resources needed for the MAILBOX interface
            2.registers event callback function
  \param[in]   mbox    operate handle
  \param[in]   idx     the device idx
  \return      error code
*/
csi_error_t csi_mbox_init(csi_mbox_t *mbox, uint32_t idx)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    csi_error_t ret = target_get(DEV_WJ_MBOX_TAG, idx, &mbox->dev);
    wj_mbox_regs_t *mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);

    wj_cpu_command_write_rd_interrupt_mask(WJ_MBOX_GET_CURRENT_CPU, mbox_base, 0U);                              ///< mask all read interrupt
    wj_cpu_command_write_wr_interrupt_mask(WJ_MBOX_GET_CURRENT_CPU, mbox_base, 0U);                              ///< mask all write interrupt
    memset((void *)((volatile uint32_t *)wj_read_cpu_command_0(WJ_MBOX_GET_CURRENT_CPU, mbox_base)), 0U, 48U);    ///< clear CPU0 command register
    wj_cpu_command_write_rd_interrupt_clear(WJ_MBOX_GET_CURRENT_CPU, mbox_base, 0xfffU);                         ///< clear CPU0 read interrupt
    wj_cpu_command_write_wr_interrupt_clear(WJ_MBOX_GET_CURRENT_CPU, mbox_base, 0xfffU);                         ///< clear CPU0 write interrupt
    wj_cpu_command_write_wr_interrupt_mask(WJ_MBOX_GET_CURRENT_CPU, mbox_base, \
                                           WJ_IUM_WCPU0_IUM_WCPU0_5 |  WJ_IUM_WCPU0_IUM_WCPU0_11); ///< unmask select write interrupt

    csi_irq_attach(mbox->dev.irq_num, &wj_mbox_irq_handler, &mbox->dev);
    csi_irq_enable(mbox->dev.irq_num);

    return ret;
}

/**
  \brief       De-initialize MAILBOX Interface. stops operation and releases the software resources used by the interface
  \param[in]   mbox    operate handle
*/
void csi_mbox_uninit(csi_mbox_t *mbox)
{
    CSI_PARAM_CHK_NORETVAL(mbox);
    wj_mbox_regs_t *mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);

    wj_cpu_command_write_wr_interrupt_mask(WJ_MBOX_GET_CURRENT_CPU, mbox_base,   0U);        ///< mask all read interrupt
    wj_cpu_command_write_rd_interrupt_mask(WJ_MBOX_GET_CURRENT_CPU, mbox_base,  0U);         ///< mask all write interrupt
    wj_cpu_command_write_wr_interrupt_clear(WJ_MBOX_GET_CURRENT_CPU, mbox_base,  0xfffU);    ///< clear CPU0 read interrupt
    wj_cpu_command_write_rd_interrupt_clear(WJ_MBOX_GET_CURRENT_CPU, mbox_base, 0xfffU);     ///< clear CPU0 write interrupt

    csi_irq_detach(mbox->dev.irq_num);
}

/**
  \brief       Start sending data to MAILBOX transmitter,(received data is ignored).
               This function is non-blocking,\ref mailbox_event_e is signaled when operation completes or error happens.
               \ref csi_mailbox_get_status can get operation status.
  \param[in]   mbox       operate handle
  \param[in]   channel_id index of channel
  \param[in]   data       Pointer to buffer with data to send to MAILBOX transmitter.
  \param[in]   size       size of data items to send
  \return      sent number of data or error code
*/
int32_t csi_mbox_send(csi_mbox_t *mbox, uint32_t channel_id, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    wj_mbox_regs_t *mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);
    int32_t ret = 0;
    uint32_t dst_address = 0U;
    uint32_t tx_len = (size > WJ_MBOX_SEND_MAX_MESSAGE_LENGTH) ? WJ_MBOX_SEND_MAX_MESSAGE_LENGTH : size;
    uint32_t buf[UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH) >> 2] = {0U};

    mbox_get_dst_channel_base_address(mbox_base, channel_id, (uint32_t *)&dst_address);

    volatile uint32_t timeout_5s = 5000U;
    volatile uint32_t mbox_mask = wj_cpu_command_read_wr_interrupt_mask(WJ_MBOX_GET_TARGET_CPU(channel_id), (wj_mbox_regs_t *)WJ_MBOX_GET_TARGET_ADDR(channel_id));
    volatile uint32_t mbox_busy = *(uint32_t *)(dst_address + UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH));

    while ((mbox_busy & 0x01U) ||
           (0U == (mbox_mask && (1U << (channel_id * 6U))))) {

        mbox_mask = wj_cpu_command_read_wr_interrupt_mask(WJ_MBOX_GET_TARGET_CPU(channel_id), (wj_mbox_regs_t *)WJ_MBOX_GET_TARGET_ADDR(channel_id));
        mbox_busy = *(uint32_t *)(dst_address + UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH));

        if (0U == timeout_5s--) {
            ret = -1;
            break;
        }

        mdelay(1);
    }

    if (0 == ret) {
        memcpy(&buf, data, tx_len);

        mbox_write((uint32_t *)dst_address, buf, UP_ALIGN4(tx_len) >> 2);
        uint32_t ack_data = (tx_len & 0xfffU) << 4U | 1U;
        mbox_write((uint32_t *)(dst_address + UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH)), &ack_data, 1U);  ///< write tx_len to ack bit

        ret = tx_len;
    }

    return ret;
}

/**
  \brief       Start Receiving data from Mailbox receiver.
  \param[in]   mbox         operate handle.
  \param[in]   channel_id   index of channel
  \param[out]  data         Pointer to buffer with data to receive from mailbox.
  \param[in]   size         Number of data items to receive
  \return      received number or  error code
*/
int32_t csi_mbox_receive(csi_mbox_t *mbox, uint32_t channel_id, void *data, uint32_t size)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    wj_mbox_regs_t *mbox_base = (wj_mbox_regs_t *)HANDLE_REG_BASE(mbox);
    int32_t rx_len = -1;
    uint32_t dst_address = 0U;

    mbox_get_self_channel_base_address(mbox_base, channel_id, (uint32_t *)&dst_address);

    if (channel_id < 2U) {
        uint32_t ack_data = 0;
        memcpy(data, (void *)dst_address, UP_ALIGN4(size));

        rx_len = size;
        mbox_write((uint32_t *)(dst_address + UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH)), &ack_data, 1U);
        mbox_get_dst_channel_base_address(mbox_base, channel_id, (uint32_t *)&dst_address);
        ack_data = (WJ_MBOX_ACK << 16U) + 1;
        mbox_write((uint32_t *)(dst_address + UP_ALIGN4(WJ_MBOX_SEND_MAX_MESSAGE_LENGTH)), &ack_data, 1U);
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

    mbox->callback = callback;
    mbox->arg = arg;

    return CSI_OK;
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
}
