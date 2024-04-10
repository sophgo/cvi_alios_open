/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

#include <string.h>
#include <sys_clk.h>
#include <drv/iic.h>
#include <drv/cvi_irq.h>
#include <drv/dma.h>
#include <drv/tick.h>
#include <drv/porting.h>
#include <aos/cli.h>
//#include <timer.h>

#include "dw_iic_ll.h"

extern uint16_t iic_tx_hs_num[];
extern uint16_t iic_rx_hs_num[];
#define VOID_P_DEC(p, val) do{ uint8_t *temp = (uint8_t *)p; temp -= val; p = (void *)temp; }while(0);

static struct iic_drv_priv iic_list[6] = {0};

/**
  \brief       wait_iic_transmit
  \param[in]   iic handle of iic instance
  \return      \ref csi_error_t
*/
static csi_error_t wait_iic_transmit(dw_iic_regs_t *iic_base, uint32_t timeout)
{
    CSI_PARAM_CHK(iic_base, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    do {
        uint32_t timecount = timeout + csi_tick_get_ms();

        while ((dw_iic_get_transmit_fifo_num(iic_base) != 0U) && (ret == CSI_OK)) {
            if (csi_tick_get_ms() >= timecount) {
                ret = CSI_TIMEOUT;
            }
        }

    } while (0);

    return ret;
}

void wait_iic_transmit_fifo_empty(dw_iic_regs_t *iic_base, uint32_t timeout)
{
    CSI_PARAM_CHK_NORETVAL(iic_base);
    csi_error_t ret = CSI_OK;

    do {
        uint32_t timecount = timeout;

        while ((dw_iic_get_transmit_fifo_num(iic_base) != 0U) && (ret == CSI_OK)) {
            timecount --;

            if (timecount <= 0U) {
                ret = CSI_TIMEOUT;
            }
        }

    } while (0);

}


/**
  \brief       wait_iic_receive
  \param[in]   iic handle of iic instance
  \param[in]   wait receive data num
  \return      \ref csi_error_t
*/
static csi_error_t wait_iic_receive(dw_iic_regs_t *iic_base, uint32_t wait_data_num, uint32_t timeout)
{
    CSI_PARAM_CHK(iic_base, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    do {
        uint32_t timecount = timeout + csi_tick_get_ms();

        while ((dw_iic_get_receive_fifo_num(iic_base) < wait_data_num) && (ret == CSI_OK)) {
            if (csi_tick_get_ms() >= timecount) {
                ret = CSI_TIMEOUT;
            }
        }
    } while (0);

    return ret;
}

void wait_iic_receive_fifo_empty(dw_iic_regs_t *iic_base, uint32_t timeout)
{
    CSI_PARAM_CHK_NORETVAL(iic_base);
    csi_error_t ret = CSI_OK;

    do {
        uint32_t timecount = timeout;

        while ((dw_iic_get_receive_fifo_num(iic_base) != 0U) && (ret == CSI_OK)) {
            timecount --;

            if (timecount <= 0U) {
                ret = CSI_TIMEOUT;
            }
        }

    } while (0);

}

#ifdef CONFIG_XIP
#define IIC_CODE_IN_RAM __attribute__((section(".ram.code")))
#else
#define IIC_CODE_IN_RAM
#endif

IIC_CODE_IN_RAM void dw_iic_slave_tx_handler(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    csi_iic_t *iic = (csi_iic_t *)arg;
    dw_iic_regs_t *iic_base;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
    uint32_t intr_state;
    intr_state = dw_iic_read_clear_intrbits(iic_base);

    uint32_t emptyfifo;
    uint32_t transmit_fifo_Level;
    uint32_t i;

    if (intr_state & DW_IIC_INTR_RD_REQ) {

        transmit_fifo_Level = dw_iic_get_transmit_fifo_num(iic_base);
        emptyfifo = (iic->size > (uint32_t)(IIC_MAX_FIFO - transmit_fifo_Level)) ? (uint32_t)(IIC_MAX_FIFO - transmit_fifo_Level) : iic->size;

        for (i = 0U; i < emptyfifo; i++) {
            dw_iic_transmit_data(iic_base, *(iic->data++));
        }

        iic->size -= emptyfifo;
    }

    if (intr_state & DW_IIC_INTR_STOP_DET) {
        if (iic->size == 0U) {
            wait_iic_transmit_fifo_empty(iic_base, 10000U);

            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);

            if (dw_iic_get_transmit_fifo_num(iic_base)) {
                iic->state.error = 1U;

                if (iic->callback) {
                    iic->callback(iic, IIC_EVENT_ERROR, iic->arg);
                }
            } else {
                iic->state.writeable = 1U;

                if (iic->callback) {
                    iic->callback(iic, IIC_EVENT_SEND_COMPLETE, iic->arg);
                }
            }
        }

        if (iic->size != 0U) {
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_disable(iic_base);
            iic->state.error = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_ERROR, iic->arg);
            }
        }
    }

}

IIC_CODE_IN_RAM void dw_iic_tx_handler(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    csi_iic_t *iic = (csi_iic_t *)arg;
    dw_iic_regs_t *iic_base;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    uint32_t intr_state;
    intr_state = dw_iic_read_clear_intrbits(iic_base);

    uint32_t emptyfifo;
    uint32_t transmit_fifo_Level;
    uint32_t i;

    if (intr_state & DW_IIC_INTR_TX_EMPTY) {
        transmit_fifo_Level = dw_iic_get_transmit_fifo_num(iic_base);
        emptyfifo = (iic->size > (uint32_t)(IIC_MAX_FIFO - transmit_fifo_Level)) ? (uint32_t)(IIC_MAX_FIFO - transmit_fifo_Level) : iic->size;

        for (i = 0U; i < emptyfifo; i++) {
            dw_iic_transmit_data(iic_base, *iic->data++);
        }

        iic->size -= emptyfifo;
    }

    /* iic send data is over fifo, data is lost */
    if (intr_state & DW_IIC_INTR_TX_OVER) {
        dw_iic_clear_all_irq(iic_base);
        dw_iic_disable_all_irq(iic_base);
        dw_iic_disable(iic_base);
        iic->state.error = 1U;

        if (iic->callback) {
            iic->callback(iic, IIC_EVENT_ERROR_OVERFLOW, iic->arg);
        }
    }

    if (intr_state & DW_IIC_INTR_STOP_DET) {
        wait_iic_transmit_fifo_empty(iic_base, 10000U);

        if (iic->size != 0U) {
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_disable(iic_base);
            iic->state.error = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_ERROR, iic->arg);
            }
        }

        if (iic->size == 0U) {
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_disable(iic_base);
            iic->state.writeable = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_SEND_COMPLETE, iic->arg);
            }
        }
    }
}

IIC_CODE_IN_RAM void dw_iic_rx_handler(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    csi_iic_t *iic = (csi_iic_t *)arg;
    dw_iic_regs_t *iic_base;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    uint32_t intr_state;
    intr_state = dw_iic_read_clear_intrbits(iic_base);

    uint32_t count, cmd_flag_read, iic_rx_fifo;

    /* fifo buff is full read from fifo and send read cmd */
    if (intr_state & DW_IIC_INTR_RX_FULL) {
        iic_rx_fifo  = dw_iic_get_receive_fifo_num(iic_base);

        for (count = 0U; count < iic_rx_fifo; count++) {
            *iic->data = dw_iic_receive_data(iic_base);
            iic->data++;
            iic->size--;
        }

        if (iic->size > RXFIFO_IRQ_TH) {
            dw_iic_set_receive_fifo_threshold(iic_base, RXFIFO_IRQ_TH);
        } else {
            dw_iic_set_receive_fifo_threshold(iic_base, iic->size);
        }

        if ((unsigned long)iic->priv > iic_rx_fifo) {
            cmd_flag_read = iic_rx_fifo;
        } else {
            cmd_flag_read = (unsigned long)iic->priv;
        }

        for (count = 0U; count < cmd_flag_read; count++) {
            dw_iic_data_cmd(iic_base);
            VOID_P_DEC(iic->priv, 1U);
        }
    }

    /* iic read data is over fifo ,data is lost */
    if (intr_state & DW_IIC_INTR_RX_OVER) {
        dw_iic_transmit_data(iic_base, 0U);
        dw_iic_disable(iic_base);
        iic->state.error = 1U;

        if (iic->callback) {
            iic->callback(iic, IIC_EVENT_ERROR_OVERFLOW, iic->arg);
        }
    }

    /* stop condition has occurred on the iic interface */
    if (intr_state & DW_IIC_INTR_STOP_DET) {

        wait_iic_receive_fifo_empty(iic_base, 10000U);

        if (iic->size != 0U) {
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_disable(iic_base);
            iic->state.error = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_ERROR, iic->arg);
            }
        }

        if (iic->size == 0U) {
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_disable(iic_base);
            iic->state.readable = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_RECEIVE_COMPLETE, iic->arg);
            }
        }
    }
}

IIC_CODE_IN_RAM void dw_iic_slave_rx_handler(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    csi_iic_t *iic = (csi_iic_t *)arg;
    dw_iic_regs_t *iic_base;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    uint32_t intr_state;
    intr_state = dw_iic_read_clear_intrbits(iic_base);

    uint32_t count, iic_rx_fifo;

    /* fifo buff is full read from fifo and send read cmd */
    if (intr_state & DW_IIC_INTR_RX_FULL) {
        iic_rx_fifo  = dw_iic_get_receive_fifo_num(iic_base);

        for (count = 0U; count < iic_rx_fifo; count++) {
            *iic->data = dw_iic_receive_data(iic_base);
            iic->data++;
            iic->size--;
        }

        if (iic->size > RXFIFO_IRQ_TH) {
            dw_iic_set_receive_fifo_threshold(iic_base, RXFIFO_IRQ_TH);
        } else {
            dw_iic_set_receive_fifo_threshold(iic_base, iic->size);
        }

    }

    /* iic read data is over fifo ,data is lost */
    if (intr_state & DW_IIC_INTR_RX_OVER) {
        dw_iic_transmit_data(iic_base, 0U);
        dw_iic_disable(iic_base);
        iic->state.error = 1U;

        if (iic->callback) {
            iic->callback(iic, IIC_EVENT_ERROR_OVERFLOW, iic->arg);
        }
    }

    /* stop condition has occurred on the iic interface */
    if (intr_state & DW_IIC_INTR_STOP_DET) {

        wait_iic_receive_fifo_empty(iic_base, 10000U);

        if (iic->size != 0U) {
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_disable(iic_base);
            iic->state.error = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_ERROR, iic->arg);
            }
        }

        if (iic->size == 0U) {
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_disable(iic_base);
            iic->state.readable = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_RECEIVE_COMPLETE, iic->arg);
            }
        }
    }
}

/**
  \brief       start slave mode dma send data
  \param[in]   iic handle of iic instance
*/
IIC_CODE_IN_RAM void iic_slave_send_data_with_dma_irq(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    csi_iic_t *iic = (csi_iic_t *)arg;
    dw_iic_regs_t *iic_base;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    uint32_t intr_state;
    intr_state = dw_iic_read_clear_intrbits(iic_base);
    dw_iic_clear_all_irq(iic_base);
    dw_iic_disable_all_irq(iic_base);

    if (intr_state & DW_IIC_INTR_RD_REQ) {
        soc_dcache_clean_invalid_range((unsigned long)iic->data, (iic->size));
        dw_iic_dma_transmit_enable(iic_base);
        dw_iic_dma_transmit_level(iic_base, (uint16_t)dw_iic_find_max_prime_num(iic->size - 8U));
        dw_iic_transmit_data(iic_base, *iic->data++);     ///< start dma must fifo have data
        dw_iic_transmit_data(iic_base, *iic->data++);
        dw_iic_transmit_data(iic_base, *iic->data++);
        dw_iic_transmit_data(iic_base, *iic->data++);
        dw_iic_transmit_data(iic_base, *iic->data++);
        dw_iic_transmit_data(iic_base, *iic->data++);
        dw_iic_transmit_data(iic_base, *iic->data++);
        dw_iic_transmit_data(iic_base, *iic->data++);
        csi_dma_ch_start(iic->tx_dma, iic->data, (uint8_t *) & (iic_base->IC_DATA_CMD), (iic->size - 8U));
    }
}

static void dw_iic_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
    CSI_PARAM_CHK_NORETVAL(dma);
    csi_iic_t *iic = (csi_iic_t *)dma->parent;
    dw_iic_regs_t *iic_base = (dw_iic_regs_t *)iic->dev.reg_base;
    uint32_t timeout = 0U;

    if (event == DMA_EVENT_TRANSFER_ERROR) {/* DMA transfer ERROR */
        if (iic->tx_dma->ch_id == dma->ch_id) {
            dw_iic_dma_transmit_disable(iic_base);
            dw_iic_dma_receive_disable(iic_base);
            csi_dma_ch_stop(dma);
            iic->state.error = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_ERROR, iic->arg);
            }
        }
    } else if (event == DMA_EVENT_TRANSFER_DONE) {/* DMA transfer complete */
        if ((iic->tx_dma != NULL) && (iic->tx_dma->ch_id == dma->ch_id)) {
            /* to do tx action */
            dw_iic_dma_transmit_disable(iic_base);
            csi_dma_ch_stop(dma);

            if (iic->mode == IIC_MODE_MASTER) {
                while (!(dw_iic_get_raw_interrupt_state(iic_base) & DW_IIC_RAW_STOP_DET)) {
                    timeout ++;
                }

                if (timeout > DW_IIC_TIMEOUT_DEF_VAL) {
                    iic->state.error = 1U;

                    if (iic->callback) {
                        iic->callback(iic, IIC_EVENT_ERROR, iic->arg);
                    }
                }
            }

            wait_iic_transmit_fifo_empty(iic_base, 10000U);
            iic->state.writeable = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_SEND_COMPLETE, iic->arg);
            }

        } else {
            /* to do rx action */
            dw_iic_dma_receive_disable(iic_base);
            csi_dma_ch_stop(dma);
            iic->state.readable = 1U;

            if (iic->callback) {
                iic->callback(iic, IIC_EVENT_RECEIVE_COMPLETE, iic->arg);
            }


        }
    }
}
/**
  \brief       Init IIC ctrl block
               1. Initializes the resources needed for the IIC instance
               2.registers event callback function and user param for the callback
  \param[in]   iic handle of iic instance
  \param[in]   idx index of instance
  \param[in]   cb  event callback function \ref csi_iic_event_cb_t
  \param[in]   arg user private param  for event callback
  \return      \ref csi_error_t
*/
csi_error_t csi_iic_init(csi_iic_t *iic, uint32_t idx)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    dw_iic_regs_t *iic_base;

    ret = target_get(DEV_DW_IIC_TAG, idx, &iic->dev);
    if (ret)
        return CSI_ERROR;

    if (iic_list[idx].iic) {
        iic_list[idx].references++;
        return CSI_OK;
    } else {
        iic_list[idx].references = 1;
        iic_list[idx].iic = iic;
    }

    ret = aos_mutex_new(&iic_list[idx].tx_mutex);
    if (ret != 0) {
        return CSI_ERROR;
    }
    ret = aos_mutex_new(&iic_list[idx].rx_mutex);
    if (ret != 0) {
        goto rx_mutx_fail;
    }

    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
    iic_base->IC_SAR = 0;
    dw_iic_disable(iic_base);
    dw_iic_clear_all_irq(iic_base);
    dw_iic_disable_all_irq(iic_base);
    //iic_base->IC_FIFO_RST_EN = 0;

    iic->state.writeable = 1U;
    iic->state.readable  = 1U;
    iic->state.error     = 0U;
    iic->send = NULL;
    iic->receive = NULL;
    iic->rx_dma = NULL;
    iic->tx_dma = NULL;
    iic->callback = NULL;

    dw_iic_set_receive_fifo_threshold(iic_base, 0x1);
    dw_iic_set_transmit_fifo_threshold(iic_base, 0x0);
    dw_iic_set_sda_hold_time(iic_base, 0x1e);

    return ret;

rx_mutx_fail:
    aos_mutex_free(&iic_list[idx].tx_mutex);
    return CSI_ERROR;
}

/**
  \brief       De-initialize IIC Instanc
               stops operation and releases the software resources used by the Instance
  \param[in]   iic handle of iic instance
  \return      \ref csi_error_t
*/
void csi_iic_uninit(csi_iic_t *iic)
{
    CSI_PARAM_CHK_NORETVAL(iic);
    uint8_t idx = HANDLE_DEV_IDX(iic);
    dw_iic_regs_t *iic_base;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    if (--iic_list[idx].references != 0)
        return;

    iic->state.error     = 0U;
    iic->state.writeable = 0U;
    iic->state.readable  = 0U;
    iic->send            = NULL;
    iic->receive         = NULL;
    iic->rx_dma          = NULL;
    iic->tx_dma          = NULL;

    iic_list[idx].iic = NULL;
    aos_mutex_free(&iic_list[idx].tx_mutex);
    aos_mutex_free(&iic_list[idx].rx_mutex);

    dw_iic_clear_all_irq(iic_base);
    dw_iic_disable_all_irq(iic_base);
    dw_iic_disable(iic_base);
    csi_irq_disable((uint32_t)iic->dev.irq_num);
    csi_irq_detach((uint32_t)iic->dev.irq_num);
}

/**
  \brief      config iic master or slave mode
  \param[in]  iic    handle of iic instance
  \param[in]  mode   iic mode \ref csi_iic_mode_t
  \return     \ref csi_error_t
*/
csi_error_t csi_iic_mode(csi_iic_t *iic, csi_iic_mode_t mode)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    dw_iic_regs_t *iic_base;
    csi_error_t ret = CSI_ERROR;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    dw_iic_disable(iic_base);

    /* This register can be written only when the I2C is disabled */
    if (mode == IIC_MODE_MASTER) {
        iic->mode = IIC_MODE_MASTER;
        dw_iic_set_master_mode(iic_base);
        dw_iic_enable_restart(iic_base);
        ret = CSI_OK;
    } else if (mode == IIC_MODE_SLAVE) {
        iic->mode = IIC_MODE_SLAVE;
        dw_iic_set_slave_mode(iic_base);
        dw_iic_enable_restart(iic_base);
        ret = CSI_OK;
    }

    return ret;
}

/**
  \brief      config iic addr mode
  \param[in]  iic        handle of iic instance
  \param[in]  addr_mode  iic addr mode
  \return     \ref csi_error_t
*/
csi_error_t csi_iic_addr_mode(csi_iic_t *iic, csi_iic_addr_mode_t addr_mode)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    dw_iic_regs_t *iic_base;
    csi_error_t ret = CSI_ERROR;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    dw_iic_disable(iic_base);

    /* This register can be written only when the I2C is disabled */
    if (addr_mode == IIC_ADDRESS_7BIT) {
        dw_iic_set_master_7bit_addr_mode(iic_base);
        dw_iic_set_slave_7bit_addr_mode(iic_base);
        ret = CSI_OK;
    } else if (addr_mode == IIC_ADDRESS_10BIT) {
        dw_iic_set_master_10bit_addr_mode(iic_base);
        dw_iic_set_slave_10bit_addr_mode(iic_base);
        ret = CSI_OK;
    }

    return ret;
}

/**
  \brief      config iic speed
  \param[in]  iic    handle of iic instance
  \param[in]  speed  iic speed mode
  \return     \ref csi_error_t
*/
csi_error_t csi_iic_speed(csi_iic_t *iic, csi_iic_speed_t speed)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    dw_iic_regs_t *iic_base;
    csi_error_t ret = CSI_ERROR;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    dw_iic_disable(iic_base);

    /* This register can be written only when the I2C is disabled */
    if (speed == IIC_BUS_SPEED_STANDARD) {
        dw_iic_set_transfer_speed_standard(iic_base);
        dw_iic_set_standard_scl_hcnt(iic_base, (((IC_CLK * 4000U) / 1000U) - 7U));
        dw_iic_set_standard_scl_lcnt(iic_base, (((IC_CLK * 4700) / 1000U) - 1U));
        ret = CSI_OK;
    } else if (speed == IIC_BUS_SPEED_FAST) {
        dw_iic_set_transfer_speed_fast(iic_base);
        dw_iic_set_fast_scl_hcnt(iic_base, (((IC_CLK * 600U) / 1000U) - 7U));
        dw_iic_set_fast_scl_lcnt(iic_base, (((IC_CLK * 1300U) / 1000U) - 1U));
        ret = CSI_OK;
    } else if (speed == IIC_BUS_SPEED_FAST_PLUS) {
        ret = CSI_UNSUPPORTED;
    } else if (speed == IIC_BUS_SPEED_HIGH) {
        dw_iic_set_transfer_speed_high(iic_base);
        dw_iic_set_high_scl_hcnt(iic_base, 6U);
        dw_iic_set_high_scl_lcnt(iic_base, 8U);
        ret = CSI_OK;
    }

    return ret;
}

/**
  \brief      config iic own addr
  \param[in]  iic      handle of iic instance
  \param[in]  own_addr own addr
  \return     \ref csi_error_t
*/
csi_error_t csi_iic_own_addr(csi_iic_t *iic, uint32_t own_addr)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    dw_iic_regs_t *iic_base;
    csi_error_t ret = CSI_OK;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    dw_iic_disable(iic_base);
    /* This register can be written only when the I2C is disabled */
    dw_iic_set_own_address(iic_base, own_addr);
    return ret;
}

/**
  \brief      config iic device addr
  \param[in]  iic      handle of iic instance
  \param[in]  dev_addr addr of device
  \return     \ref csi_error_t
*/
csi_error_t csi_iic_dev_addr(csi_iic_t *iic, uint32_t dev_addr)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    dw_iic_regs_t *iic_base;
    csi_error_t ret = CSI_OK;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    dw_iic_set_target_address(iic_base,dev_addr);
    dw_iic_enable(iic_base);
    return ret;
}
/**
  \brief       Start sending data as IIC Master.
               This function is blocking
  \param[in]   iic            handle of iic instance
  \param[in]   devaddr        addrress of slave device
  \param[in]   data           data to send to IIC Slave
  \param[in]   size           size of data items to send
  \param[in]   timout         unit of time delay
  \return      master send real size and error code
*/
int32_t csi_iic_master_send(csi_iic_t *iic, uint32_t devaddr, const void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    uint32_t timecount;
    int32_t send_count = size;
    uint8_t *send_data = (uint8_t *)data;
    uint8_t iic_idx = HANDLE_DEV_IDX(iic);
    dw_iic_regs_t *iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    aos_mutex_lock(&iic_list[iic_idx].tx_mutex, AOS_WAIT_FOREVER);

    if (!send_data || !size) {
        ret = CSI_ERROR;
        goto SEND_ERROR;
    }

    dw_iic_set_target_address(iic_base, devaddr);
    dw_iic_enable(iic_base);

    timecount = timeout + csi_tick_get_ms();

    while (send_count) {
        if (iic_base->IC_STATUS & DW_IIC_TXFIFO_NOT_FULL_STATE) {
            if (--send_count == 0) {
                dw_iic_transmit_data(iic_base, *send_data++ | DW_IIC_DATA_STOP);
            } else {
                dw_iic_transmit_data(iic_base, *send_data++);
            }
        } else if (csi_tick_get_ms() >= timecount) {
            log("Timeout for waiting ic status TFNF\n");
            ret = CSI_TIMEOUT;
            goto SEND_ERROR;
        }
    }

    if (dw_iic_xfer_finish(iic_base)) {
        ret = CSI_ERROR;
        goto SEND_ERROR;
    }

SEND_ERROR:
    dw_iic_disable(iic_base);
    aos_mutex_unlock(&iic_list[iic_idx].tx_mutex);

    return (ret == CSI_OK) ? size : ret;
}

/**
  \brief       Start receiving data as IIC Master.
               This function is blocking
  \param[in]   iic      handle to operate.
  \param[in]   devaddr  iic addrress of slave device.
  \param[out]  data     Pointer to buffer for data to receive from IIC receiver
  \param[in]   size     size of data items to receive
  \param[in]   timeout  unit of time delay
  \return      master receive real size and error code
*/
int32_t csi_iic_master_receive(csi_iic_t *iic, uint32_t devaddr, void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    uint32_t timecount;
    int32_t read_count = size, active = 0;
    uint8_t *receive_data = (uint8_t *)data;
    uint8_t iic_idx = HANDLE_DEV_IDX(iic);
    dw_iic_regs_t *iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    aos_mutex_lock(&iic_list[iic_idx].rx_mutex, AOS_WAIT_FOREVER);

    if (!receive_data || !size) {
        ret = CSI_ERROR;
        goto RECV_ERROR;
    }

    dw_iic_set_target_address(iic_base, devaddr);
    dw_iic_enable(iic_base);

    timecount = timeout + csi_tick_get_ms();

    while (read_count) {
        if (!active) {
            /*
            * Avoid writing to ic_cmd_data multiple times
            * in case this loop spins too quickly and the
            * ic_status RFNE bit isn't set after the first
            * write. Subsequent writes to ic_cmd_data can
            * trigger spurious i2c transfer.
            */
            dw_iic_transmit_data(iic_base, DW_IIC_DATA_CMD | DW_IIC_DATA_STOP);
            //mmio_write_32((uintptr_t)&i2c_base->ic_cmd_data, (dev <<1) | BIT_I2C_CMD_DATA_READ_BIT | BIT_I2C_CMD_DATA_STOP_BIT);
            active = 1;
        }

        //if (iic_base->IC_RAW_INTR_STAT & DW_IIC_RAW_RX_FULL) {
        if (iic_base->IC_STATUS & DW_IIC_RXFIFO_NOT_EMPTY_STATE) {
        //if (dw_iic_get_receive_fifo_num(iic_base)) {
            *receive_data++ = dw_iic_receive_data(iic_base);
            read_count--;
            active = 0;
        } else if (csi_tick_get_ms() >= timecount) {
            log("Timeout for waiting ic status RFNE\n");
            ret = CSI_TIMEOUT;
            goto RECV_ERROR;
        }
    }

    if (dw_iic_xfer_finish(iic_base)) {
        ret = CSI_ERROR;
    }

RECV_ERROR:
    dw_iic_disable(iic_base);
    aos_mutex_unlock(&iic_list[iic_idx].rx_mutex);

    return (ret == CSI_OK) ? size : ret;
}

static csi_error_t iic_master_send_intr(csi_iic_t *iic, uint32_t devaddr, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if ((data == NULL) || (size == 0U)) {
        ret = CSI_ERROR;
    } else {
        if (iic->state.writeable == 0U) {
            ret = CSI_BUSY;
        } else {
            dw_iic_regs_t *iic_base;
            iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

            dw_iic_set_target_address(iic_base, devaddr);

            iic->data            = (uint8_t *)data;
            iic->size            = size;
            iic->state.writeable = 0U;

            dw_iic_disable(iic_base);
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_master_enable_transmit_irq(iic_base);
            dw_iic_set_transmit_fifo_threshold(iic_base, TXFIFO_IRQ_TH);
            dw_iic_enable(iic_base);
        }
    }

    return ret;
}

static csi_error_t iic_master_send_dma(csi_iic_t *iic, uint32_t devaddr, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    dw_iic_regs_t *iic_base = (dw_iic_regs_t *)iic->dev.reg_base;
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)iic->tx_dma;

    iic->data = (void *)data;
    iic->size = size;
    iic->state.writeable = 0U;
    dw_iic_disable(iic_base);
    dw_iic_clear_all_irq(iic_base);
    dw_iic_disable_all_irq(iic_base);
    dw_iic_set_target_address(iic_base, devaddr);
    dw_iic_dma_transmit_enable(iic_base);
    dw_iic_dma_transmit_level(iic_base, (uint16_t)dw_iic_find_max_prime_num(size));
    config.src_inc = DMA_ADDR_INC;
    config.dst_inc = DMA_ADDR_CONSTANT;
    config.src_tw = DMA_DATA_WIDTH_8_BITS;
    config.dst_tw = DMA_DATA_WIDTH_8_BITS;
    /* config for wj_dma */
    config.group_len = (uint16_t)dw_iic_find_max_prime_num(size);
    config.trans_dir = DMA_MEM2PERH;
    /* config for etb */
    config.handshake = iic_tx_hs_num[iic->dev.idx];

    csi_dma_ch_config(dma_ch, &config);


    soc_dcache_clean_invalid_range((unsigned long)iic->data, iic->size);
    csi_dma_ch_start(iic->tx_dma, iic->data, (uint8_t *) & (iic_base->IC_DATA_CMD), iic->size);
    dw_iic_enable(iic_base);

    return ret;
}

/**
  \brief       Start sending data as IIC Master.
               This function is non-blocking,\ref csi_iic_event_e is signaled when transfer completes or error happens.
  \param[in]   iic            handle to operate.
  \param[in]   devaddr        iic addrress of slave device. |_BIT[7:1]devaddr_|_BIT[0]R/W_|
                              eg: BIT[7:0] = 0xA0, devaddr = 0x50.
  \param[in]   data           data to send to IIC Slave
  \param[in]   num            size of data items to send
  \return      \ref csi_error_t
*/
csi_error_t csi_iic_master_send_async(csi_iic_t *iic, uint32_t devaddr, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_iic_master_send_async_t send_func;

    if (iic->send) {
        send_func = iic->send;
        send_func(iic, devaddr, data, size);
    } else {
        csi_irq_attach((uint32_t)iic->dev.irq_num, &dw_iic_tx_handler, &iic->dev);
        csi_irq_enable((uint32_t)iic->dev.irq_num);
        iic_master_send_intr(iic, devaddr, data, size);
    }

    return ret;
}

static csi_error_t iic_master_receive_intr(csi_iic_t  *iic, uint32_t devaddr, void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if ((data == NULL) || (size == 0U)) {
        ret = CSI_ERROR;
    } else {
        if (iic->state.readable == 0U) {
            ret = CSI_BUSY;
        } else {
            uint32_t count = 0U, cmd_flag_read = 0U;
            dw_iic_regs_t *iic_base;
            iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

            iic->data                = (uint8_t *)data;
            iic->size                = size;
            iic->state.readable      = 0U;
            unsigned long temp = size;
            iic->priv = (void *)temp;       ///< iic->priv used to record the transmission value

            dw_iic_disable(iic_base);
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_set_target_address(iic_base, devaddr);
            dw_iic_master_enable_receive_irq(iic_base);

            if (iic->size > RXFIFO_IRQ_TH) {                                    ///< set receive data num and set receive FIFO threshold
                dw_iic_set_receive_fifo_threshold(iic_base, RXFIFO_IRQ_TH);     ///< Set receive FIFO threshold
            } else {
                dw_iic_set_receive_fifo_threshold(iic_base, iic->size);
            }

            if (iic->size > IIC_MAX_FIFO) {
                cmd_flag_read = IIC_MAX_FIFO;
            } else {
                cmd_flag_read = iic->size;
            }

            dw_iic_enable(iic_base);

            for (count = 0U; count < cmd_flag_read; count++) {
                dw_iic_data_cmd(iic_base);
            }

            VOID_P_DEC(iic->priv, cmd_flag_read);
        }
    }

    return ret;
}

/**
  \brief       Start receiving data as IIC Master.
               This function is non-blocking,\ref csi_iic_event_t is signaled when transfer completes or error happens.
               \ref csi_iic_get_status can get operating status.
  \param[in]   iic            handle to operate.
  \param[in]   devaddr        iic addrress of slave device.
  \param[out]  data           pointer to buffer for data to receive from IIC receiver
  \param[in]   size           size of data items to receive
  \return      \ref csi_error_t
*/
csi_error_t csi_iic_master_receive_async(csi_iic_t  *iic, uint32_t devaddr, void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    /*the hardware only support master send with dma and master receive only use interrupt*/
    csi_irq_attach((uint32_t)iic->dev.irq_num, &dw_iic_rx_handler, &iic->dev);
    csi_irq_enable((uint32_t)iic->dev.irq_num);
    ret = iic_master_receive_intr(iic, devaddr, data, size);
    return ret;
}

/**
  \brief       Start transmitting data as IIC Master.
               This function is blocking
  \param[in]   iic            handle of iic instance
  \param[in]   devaddr        addrress of slave device
  \param[in]   memaddr        internal addr of device
  \param[in]   memaddr_size   internal addr mode of device
  \param[in]   data           data to send to IIC Slave
  \param[in]   size           size of data items to send
  \param[in]   timout         unit of time delay
  \return      memory send real size and error code
*/
int32_t csi_iic_mem_send(csi_iic_t *iic, uint32_t devaddr, uint16_t memaddr, csi_iic_mem_addr_size_t memaddr_size, const void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    uint32_t timecount;
    uint32_t send_count = size;
    uint8_t *send_data = (uint8_t *)data;
    uint8_t memaddr_len;
    uint8_t iic_idx = HANDLE_DEV_IDX(iic);
    dw_iic_regs_t *iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    if (!size) {
        return CSI_ERROR;
    }

    aos_mutex_lock(&iic_list[iic_idx].tx_mutex, AOS_WAIT_FOREVER);

    switch(memaddr_size) {
    default:
    case IIC_MEM_ADDR_SIZE_8BIT:memaddr_len = 0x1;break;
    case IIC_MEM_ADDR_SIZE_16BIT:memaddr_len = 0x2;break;
    }

    if (dw_iic_xfer_init(iic_base, devaddr, memaddr, memaddr_len)) {
        ret = CSI_ERROR;
        goto SEND_ERROR;
    }

    timecount = timeout + csi_tick_get_ms();

    while (send_count > 0) {
        if (iic_base->IC_STATUS & DW_IIC_TXFIFO_NOT_FULL_STATE) {
            if (--send_count == 0) {
                dw_iic_transmit_data(iic_base, *send_data++ | DW_IIC_DATA_STOP);
                log("send_count = 0\n");
            } else {
                dw_iic_transmit_data(iic_base, *send_data++);
                log("send_count != 0\n");
            }
        } else if (csi_tick_get_ms() >= timecount){
            log("ic status is not TFNF\n");
            ret = CSI_TIMEOUT;
            goto SEND_ERROR;
        }
    }

    if (dw_iic_xfer_finish(iic_base)) {
        ret = CSI_ERROR;
        goto SEND_ERROR;
    }

SEND_ERROR:
    dw_iic_disable(iic_base);
    aos_mutex_unlock(&iic_list[iic_idx].tx_mutex);
    return (ret == CSI_OK) ? size : ret;
}

/**
  \brief       Start receiving data as IIC Master.
               This function is blocking
  \param[in]   iic            handle to operate.
  \param[in]   devaddr        iic addrress of slave device.
  \param[in]   memaddr        internal addr of device
  \param[in]   memaddr_mode   internal addr mode of device
  \param[out]  data           Pointer to buffer for data to receive from IIC receiver
  \param[in]   size           size of data items to receive
  \param[in]   timeout        unit of time delay
  \return      memory receive real size and error code
*/
int32_t csi_iic_mem_receive(csi_iic_t *iic, uint32_t devaddr, uint16_t memaddr, csi_iic_mem_addr_size_t memaddr_size, void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    uint32_t timecount;
    int recv_count = size;
    uint8_t *recv_data = (uint8_t *)data;
    uint8_t memaddr_len;
    uint8_t iic_idx = HANDLE_DEV_IDX(iic);
    //int start = 0;
    dw_iic_regs_t *iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

    if (!data || !size) {
        return CSI_ERROR;
    }

    aos_mutex_lock(&iic_list[iic_idx].rx_mutex, AOS_WAIT_FOREVER);

    switch(memaddr_size) {
    default:
    case IIC_MEM_ADDR_SIZE_8BIT:memaddr_len = 0x1;break;
    case IIC_MEM_ADDR_SIZE_16BIT:memaddr_len = 0x2;break;
    }

    if (dw_iic_xfer_init(iic_base, devaddr, memaddr, memaddr_len)) {
        ret = CSI_ERROR;
        goto RECV_ERROR;
    }
    timecount = timeout + csi_tick_get_ms();
    for (int i = 0 ; i < recv_count; i ++) {
        if(i != (recv_count -1)) {
            dw_iic_transmit_data(iic_base, DW_IIC_DATA_CMD);
        } else {
            dw_iic_transmit_data(iic_base, DW_IIC_DATA_CMD | DW_IIC_DATA_STOP);
        }
    }
    while (recv_count > 0) {
        //if (iic_base->IC_RAW_INTR_STAT & DW_IIC_RAW_RX_FULL) {
        if (iic_base->IC_STATUS & DW_IIC_RXFIFO_NOT_EMPTY_STATE) {
        //if (dw_iic_get_receive_fifo_num(iic_base)) {
            *recv_data++ = dw_iic_receive_data(iic_base);
            --recv_count;
        } else if (csi_tick_get_ms() >= timecount) {
            log("Timed out read ic_cmd_data\n");
            ret = CSI_TIMEOUT;
            goto RECV_ERROR;
        }
    }

    if (dw_iic_xfer_finish(iic_base)) {
        ret = CSI_ERROR;
        goto RECV_ERROR;
    }

RECV_ERROR:
    dw_iic_disable(iic_base);
    aos_mutex_unlock(&iic_list[iic_idx].rx_mutex);
    return (ret == CSI_OK) ? size : ret;
}

/**
  \brief       Start sending data as IIC Slave.
               This function is blocking
  \param[in]   iic      handle to operate.
  \param[in]   data     Pointer to buffer with data to transmit to IIC Master
  \param[in]   size     size of data items to send
  \param[in]   timeout  uint in mini-second
  \return      slave send real size and error code
*/
int32_t csi_iic_slave_send(csi_iic_t *iic, const void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    int32_t send_count = 0;
    uint8_t *send_data = (void *)data;

    if ((send_data == NULL) || (size == 0U)) {
        send_count = (int32_t)CSI_ERROR;
    } else {
        dw_iic_regs_t *iic_base;
        uint32_t intr_state;
        iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
        dw_iic_enable(iic_base);
        uint32_t timecount = csi_tick_get_ms() + timeout;

        while (1) {
            if (csi_tick_get_ms() >= timecount) {
                break;
            }

            intr_state = dw_iic_get_raw_interrupt_state(iic_base);///< read iic irq status

            if (intr_state & DW_IIC_INTR_RD_REQ) {
                dw_iic_transmit_data(iic_base, *(send_data++));

                ret = wait_iic_transmit(iic_base, timeout); ///< wait tx data tx

                if (ret != CSI_OK) {
                    send_count = (int32_t)ret;
                    break;
                }

                send_count ++;

                if (send_count == (int32_t)size) {
                    ret = CSI_OK;
                    ret = wait_iic_transmit(iic_base, timeout); ///< wait tx data tx

                    if (ret != CSI_OK) {
                        send_count = (int32_t)ret;
                        break;
                    }

                    break;
                }
            }
        }
    }

    return send_count;
}

/**
  \brief       Start receiving data as IIC Slave.
               This function is blocking
  \param[in]   iic     handle to operate.
  \param[out]  data    Pointer to buffer for data to receive from IIC Master
  \param[in]   size    size of data items to receive
  \param[in]   timeout uint in mini-second
  \return      slave receive real size and error code
*/
int32_t csi_iic_slave_receive(csi_iic_t *iic, void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    int32_t read_count = 0;
    uint8_t *receive_data = (void *)data;

    if ((receive_data == NULL) || (size == 0U)) {
        read_count = (int32_t)CSI_ERROR;
    } else {
        dw_iic_regs_t *iic_base;
        iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
        dw_iic_enable(iic_base);

        while (1) {
            ret = wait_iic_receive(iic_base, 1U, timeout);

            if (ret != CSI_OK) {
                read_count = (int32_t)ret;
                break;
            }

            *(receive_data++) = dw_iic_receive_data(iic_base);
            read_count ++;

            if (read_count == (int32_t)size) {
                break;
            }
        }
    }

    return read_count;
}

static csi_error_t iic_slave_send_dma(csi_iic_t *iic, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    dw_iic_regs_t *iic_base = (dw_iic_regs_t *)iic->dev.reg_base;
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)iic->tx_dma;

    iic->data = (void *)data;
    iic->size = size;
    iic->state.writeable = 0U;

    dw_iic_disable(iic_base);
    dw_iic_clear_all_irq(iic_base);
    dw_iic_disable_all_irq(iic_base);
    dw_iic_slave_enable_transmit_irq(iic_base);
    csi_irq_attach((uint32_t)iic->dev.irq_num, &iic_slave_send_data_with_dma_irq, &iic->dev); ///< if use slave send dma attach will change
    csi_irq_enable((uint32_t)iic->dev.irq_num);
    config.src_inc = DMA_ADDR_INC;
    config.dst_inc = DMA_ADDR_CONSTANT;
    config.src_tw = DMA_DATA_WIDTH_8_BITS;
    config.dst_tw = DMA_DATA_WIDTH_8_BITS;
    /* config for wj_dma */
    config.group_len = 4U;
    config.trans_dir = DMA_MEM2PERH;
    /* config for etb */
    config.handshake = iic_tx_hs_num[iic->dev.idx];

    csi_dma_ch_config(dma_ch, &config);

    dw_iic_enable(iic_base);

    return ret;
}

static csi_error_t iic_slave_send_intr(csi_iic_t *iic, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if ((data == NULL) || (size == 0U)) {
        ret = CSI_ERROR;
    } else {
        if (iic->state.writeable == 0U) {
            ret = CSI_BUSY;
        } else {
            dw_iic_regs_t *iic_base;
            iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);

            iic->data              = (uint8_t *)data;
            iic->size              = size;
            iic->state.writeable   = 0U;

            dw_iic_disable(iic_base);
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_slave_enable_transmit_irq(iic_base);
            dw_iic_enable(iic_base);
        }
    }

    return ret;
}

/**
\brief       Start transmitting data as IIC Slave.
             This function is non-blocking,\ref csi_iic_event_t is signaled when transfer completes or error happens.
\param[in]   iic       handle to operate.
\param[in]   data      Pointer to buffer with data to transmit to IIC Master
\param[in]   size      size of data items to send
\return      \ref csi_error_t
*/
csi_error_t csi_iic_slave_send_async(csi_iic_t *iic, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_iic_slave_send_async_t send_func;

    if (iic->send) {
        send_func = iic->send;
        csi_irq_attach((uint32_t)iic->dev.irq_num, &iic_slave_send_data_with_dma_irq, &iic->dev);
        csi_irq_enable((uint32_t)iic->dev.irq_num);
        send_func(iic, data, size);
    } else {
        csi_irq_attach((uint32_t)iic->dev.irq_num, &dw_iic_slave_tx_handler, &iic->dev);
        csi_irq_enable((uint32_t)iic->dev.irq_num);
        iic_slave_send_intr(iic, data, size);
    }

    return ret;
}

static csi_error_t iic_slave_receive_intr(csi_iic_t *iic, void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if ((data == NULL) || (size == 0U)) {
        ret =  CSI_ERROR;
    } else {
        if (iic->state.readable == 0U) {
            ret = CSI_BUSY;
        } else {
            dw_iic_regs_t *iic_base;
            iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);


            iic->data            = (uint8_t *)data;
            iic->size            = size;
            iic->state.readable  = 0U;

            dw_iic_disable(iic_base);
            dw_iic_clear_all_irq(iic_base);
            dw_iic_disable_all_irq(iic_base);
            dw_iic_slave_enable_receive_irq(iic_base);

            if (iic->size > RXFIFO_IRQ_TH) {
                dw_iic_set_receive_fifo_threshold(iic_base, RXFIFO_IRQ_TH);     ///< Set receive FIFO threshold
            } else {
                dw_iic_set_receive_fifo_threshold(iic_base, iic->size);
            }

            dw_iic_enable(iic_base);
        }
    }

    return ret;
}

static csi_error_t iic_slave_receive_dma(csi_iic_t *iic, void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    dw_iic_regs_t *iic_base = (dw_iic_regs_t *)iic->dev.reg_base;
    csi_dma_ch_t *dma = (csi_dma_ch_t *)iic->rx_dma;

    iic->data = (uint8_t *)data;
    iic->size = size;
    iic->state.readable = 0U;

    dw_iic_disable(iic_base);
    dw_iic_clear_all_irq(iic_base);
    dw_iic_disable_all_irq(iic_base);
    dw_iic_dma_receive_enable(iic_base);
    dw_iic_dma_receive_level(iic_base, dw_iic_find_max_prime_num(size) - 1U);
    config.src_inc = DMA_ADDR_CONSTANT;
    config.dst_inc = DMA_ADDR_INC;
    config.src_tw = DMA_DATA_WIDTH_8_BITS;
    config.dst_tw = DMA_DATA_WIDTH_8_BITS;
    config.group_len = dw_iic_find_max_prime_num(size);
    config.trans_dir = DMA_PERH2MEM;
    config.handshake = iic_rx_hs_num[iic->dev.idx];

    csi_dma_ch_config(dma, &config);

    soc_dcache_clean_invalid_range((unsigned long)iic->data, iic->size);
    csi_dma_ch_start(iic->rx_dma, (uint8_t *) & (iic_base->IC_DATA_CMD), iic->data, iic->size);
    dw_iic_enable(iic_base);


    return ret;
}

/**
\brief       Start receiving data as IIC Slave.
             This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
\param[in]   handle  iic handle to operate.
\param[out]  data    Pointer to buffer for data to receive from IIC Master
\param[in]   size    size of data items to receive
\return      \ref csi_error_t
*/
csi_error_t csi_iic_slave_receive_async(csi_iic_t *iic, void *data, uint32_t size)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_iic_slave_receive_async_t receive_func;

    if (iic->receive) {
        receive_func = iic->receive;
        receive_func(iic, data, size);
    } else {
        csi_irq_attach((uint32_t)iic->dev.irq_num, &dw_iic_slave_rx_handler, &iic->dev);
        csi_irq_enable((uint32_t)iic->dev.irq_num);
        iic_slave_receive_intr(iic, data, size);
    }

    return ret;
}

/**
\brief       Attach callback to the iic
\param[in]   iic          iic handle to operate.
\param[in]   cb           event callback function \ref csi_iic_callback_t
\param[in]   arg          user private param  for event callback
\return      \ref csi_error_t
*/
csi_error_t csi_iic_attach_callback(csi_iic_t *iic, void *callback, void *arg)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    iic->callback = callback;
    iic->arg = arg;
    return ret;
}

/**
\brief       detach callback from the iic
\param[in]   iic          iic handle to operate.
*/
void csi_iic_detach_callback(csi_iic_t *iic)
{
    CSI_PARAM_CHK_NORETVAL(iic);
    iic->callback = NULL;
    iic->arg = NULL;
    iic->send = NULL;
    iic->receive = NULL;
    csi_irq_disable((uint32_t)iic->dev.irq_num);
    csi_irq_detach((uint32_t)iic->dev.irq_num);
}

/**
\brief       config iic stop to generate
\param[in]   iic        iic handle to operate.
\param[in]   enable     Transfer operation is pending - Stop condition will not be generated.
\return      \ref csi_error_t
*/
csi_error_t csi_iic_xfer_pending(csi_iic_t *iic, bool enable)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    csi_error_t ret = CSI_UNSUPPORTED;
    return ret;
}

/**
  \brief       link DMA channel to iic device
  \param[in]   iic handle to operate.
  \param[in]   tx_dma the DMA channel handle for send, when it is NULL means to unlink the channel
  \param[in]   rx_dma the DMA channel handle for receive, when it is NULL means to unlink the channel
  \return      error code
*/
csi_error_t csi_iic_link_dma(csi_iic_t *iic, csi_dma_ch_t *tx_dma, csi_dma_ch_t *rx_dma)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;

    if (iic->callback != NULL) {
        if (iic->mode == IIC_MODE_MASTER) {
            /*the hardware only support master send with dma and master receive only use interrupt*/
            if (tx_dma != NULL) {
                tx_dma->parent = iic;
                ret = csi_dma_ch_alloc(tx_dma, -1, -1);

                if (ret == CSI_OK) {
                    csi_dma_ch_attach_callback(tx_dma, dw_iic_dma_event_cb, NULL);
                    iic->tx_dma = tx_dma;
                    iic->send = iic_master_send_dma;
                } else {
                    tx_dma->parent = NULL;
                }
            } else {
                if (iic->tx_dma) {
                    csi_dma_ch_detach_callback(iic->tx_dma);
                    csi_dma_ch_free(iic->tx_dma);
                    iic->send = iic_master_send_intr;
                    iic->tx_dma = NULL;
                }

                ret = CSI_OK;
            }
        } else if (iic->mode == IIC_MODE_SLAVE) {
            /*the hardware only support slave receive with dma and slave send only use interrupt*/
            if (tx_dma != NULL) {
                tx_dma->parent = iic;
                ret = csi_dma_ch_alloc(tx_dma, -1, -1);

                if (ret == CSI_OK) {
                    csi_dma_ch_attach_callback(tx_dma, dw_iic_dma_event_cb, NULL);
                    iic->tx_dma = tx_dma;
                    iic->send = iic_slave_send_dma;
                } else {
                    tx_dma->parent = NULL;
                }
            } else {
                if (iic->tx_dma) {
                    csi_dma_ch_detach_callback(iic->tx_dma);
                    csi_dma_ch_free(iic->tx_dma);
                    iic->send = iic_slave_send_intr;
                    iic->tx_dma = NULL;
                }

                ret = CSI_OK;
            }

            if (rx_dma != NULL) {
                rx_dma->parent = iic;
                ret = csi_dma_ch_alloc(rx_dma, -1, -1);

                if (ret == CSI_OK) {
                    iic->rx_dma = rx_dma;
                    csi_dma_ch_attach_callback(rx_dma, dw_iic_dma_event_cb, NULL);
                    iic->receive = iic_slave_receive_dma;
                } else {
                    rx_dma->parent = NULL;
                }
            } else {
                if (iic->rx_dma) {
                    csi_dma_ch_detach_callback(iic->rx_dma);
                    csi_dma_ch_free(iic->rx_dma);
                    iic->receive = iic_slave_receive_intr;
                    iic->rx_dma = NULL;
                }

                ret = CSI_OK;
            }
        }
    }

    return ret;
}

/**
\brief       start receiving data as IIC Slave.
             This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
\param[in]   iic   handle to operate.
\param[out]  state iic state \ref csi_state_t
\return      \ref csi_error_t
*/
csi_error_t csi_iic_get_state(csi_iic_t *iic, csi_state_t *state)
{
    CSI_PARAM_CHK(iic, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);
    *state = iic->state;
    return CSI_OK;
}

#ifdef CONFIG_PM
static csi_error_t dw_iic_pm_action(csi_dev_t *dev, csi_pm_dev_action_t action)
{
    CSI_PARAM_CHK(dev, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    csi_pm_dev_t *pm_dev = &dev->pm_dev;

    switch (action) {
        case PM_DEV_SUSPEND:
            csi_pm_dev_save_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 4U);                 ///< save iic control registers
            csi_pm_dev_save_regs(pm_dev->reten_mem + 4U, (uint32_t *)(dev->reg_base + 20U),   6U);  ///< save iic speed config registers
            csi_pm_dev_save_regs(pm_dev->reten_mem + 10U, (uint32_t *)(dev->reg_base + 48U),  1U);  ///< save iic interrupt mask registers
            csi_pm_dev_save_regs(pm_dev->reten_mem + 11U, (uint32_t *)(dev->reg_base + 56U),  2U);  ///< save iic fifo config registers
            csi_pm_dev_save_regs(pm_dev->reten_mem + 13U, (uint32_t *)(dev->reg_base + 128U), 12U); ///< save iic dma config registers
            break;

        case PM_DEV_RESUME:
            csi_pm_dev_restore_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 4U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 4U, (uint32_t *)(dev->reg_base + 20U),  6U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 10U, (uint32_t *)(dev->reg_base + 48U),  1U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 11U, (uint32_t *)(dev->reg_base + 56U),  2U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 13U, (uint32_t *)(dev->reg_base + 128U), 12U);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

csi_error_t csi_iic_enable_pm(csi_iic_t *iic)
{
    return csi_pm_dev_register(&iic->dev, dw_iic_pm_action, 100U, 0U);
}

void csi_iic_disable_pm(csi_iic_t *iic)
{
    csi_pm_dev_unregister(&iic->dev);
}
#endif

