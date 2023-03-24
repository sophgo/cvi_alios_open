/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     dw_qspi.c
 * @brief
 * @version
 * @date     2020-02-11
 ******************************************************************************/

#include <string.h>

#include <drv/qspi.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>
#include <drv/porting.h>
#include "dw_qspi_ll.h"

#define DW_MAX_BUS_FIFO_WAIT       (100)
#define DW_MAX_SPI_TXFIFO_LV       0x100U
#define DW_MAX_SPI_RXFIFO_LV       0x100U
#define DW_DEFAULT_SPI_TXFIFO_LV   0x80U
#define DW_DEFAULT_SPI_RXFIFO_LV   0x80U
#define DW_MAX_CMD_BUF_LEN         (32)
#define DW_DEFAULT_FRAME_LEN        0x08U

#define DW_DEFAULT_TRANSCATION_TIMEOUT 200U
#define DW_QSPI_DMA_CMD_ADDR_SPACE     (2U)
#define DW_QSPI_DMA_BURST_LEN             (16U)
#define IS_8BIT_FRAME_LEN(qspi_base)   (dw_qspi_get_data_frame_len(qspi_base) <= 8U)
#define IS_16BIT_FRAME_LEN(qspi_base)  (( dw_qspi_get_data_frame_len(qspi_base) > 8U ) && ( dw_qspi_get_data_frame_len(qspi_base) <= 16U ))
#define IS_32BIT_FRAME_LEN(qspi_base)  (( dw_qspi_get_data_frame_len(qspi_base) > 16U ) && ( dw_qspi_get_data_frame_len(qspi_base) <= 32U ))

#define DW_SPI_HS_OFFSET              (2U)

#define DW_SPI_SET_REG_IDX_MASTER(qspi) (qspi->priv = (void *)0U)
#define DW_SPI_SET_REG_IDX_SLAVE(qspi)  (qspi->priv = (void *)1U)
#define DW_SPI_SET_REG_IDX_COMMON(qspi) (qspi->priv = (void *)1U)
#define DW_SPI_GET_REG_IDX(qspi)        ((unsigned long)qspi->priv)
#define IS_DW_SPI_IDX_MASTER(qspi)      ((unsigned long)qspi->priv == 0U)
#define IS_DW_SPI_IDX_SLAVE(qspi)       ((unsigned long)qspi->priv == 1U)

typedef struct{
    uint8_t xfer_pre[DW_MAX_CMD_BUF_LEN];
    uint32_t xfer_pre_len;
    void *xfer_post;
    uint32_t xfer_post_len;
}qspi_xfer_t;    
typedef enum{
    QSPI_XFER_CMD_START,
    QSPI_XFER_CMD_DONE,
    QSPI_XFER_DATA_START,
    QSPI_XFER_DATA_DONE
}qspi_transfer_state_t;

DEFINE_DESC_BUF(link_list_buf,2);
static volatile qspi_transfer_state_t g_qspi_xfer_state = 0;
uint32_t        qspi_dma_pre_buf[2];
extern uint16_t qspi_tx_hs_num[];
extern uint16_t qspi_rx_hs_num[];
static csi_error_t dw_qspi_send_intr(csi_qspi_t *qspi, const void *data, uint32_t size);
static csi_error_t dw_qspi_receive_intr(csi_qspi_t *qspi, void *data, uint32_t size);
static csi_error_t dw_qspi_send_receive_intr(csi_qspi_t *qspi, const void *data_out, void *data_in, uint32_t num);

static dw_qspi_regs_t *dw_get_reg_base(csi_qspi_t *qspi)
{
    dw_qspi_regs_t *qspi_base;
    uint64_t reg_base;

    reg_base = HANDLE_REG_BASE(qspi);
    qspi_base = (dw_qspi_regs_t *)reg_base;
    return qspi_base;
}

static uint16_t dw_qspi_get_hs_num(csi_qspi_t *qspi, uint16_t *hs_num)
{
    return hs_num[( qspi->dev.idx * DW_SPI_HS_OFFSET )];
}
static uint8_t find_max_prime_num(uint32_t num, uint32_t limit)
{
    uint32_t i, min;

    min = (num > limit) ? limit : num;
    i = min;
    while (i > 0U) {
        if (!(num % i)) {
            break;
        }

        i--;
    }

    if (i == 0U) {
        i = min;
    }

    return (uint8_t)i;
}

static uint8_t find_group_len(uint32_t size, uint8_t width)
{
    uint32_t prime_num;
    uint32_t limit;

    limit = 8U;

    do {
        prime_num = find_max_prime_num(size, limit);
        limit = prime_num - 1U;
    } while ((prime_num % width) != 0U);

    return (uint8_t)prime_num;
}


static csi_error_t wait_ready_until_timeout(csi_qspi_t *qspi, uint32_t timeout)
{
    uint32_t timestart = 0U;
    csi_error_t    ret = CSI_OK;
    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);

    timestart = csi_tick_get_ms();

    while (dw_qspi_get_status(qspi_base) & DW_QSPI_SR_BUSY) {
        if ((csi_tick_get_ms() - timestart) > timeout) {
            ret = CSI_TIMEOUT;
            break;
        }
    }

    return ret;
}

static void process_end_transcation(csi_qspi_t *qspi)
{
    uint32_t mode;

    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    mode = dw_qspi_get_transfer_mode(qspi_base);

    /* process end of transmit */
    if ((mode & DW_QSPI_CTRLR0_TMOD_Msk) ==  DW_QSPI_CTRLR0_TMOD_TX) {
        if (qspi->tx_size == 0U) {
            if ((dw_qspi_get_status(qspi_base) & DW_QSPI_SR_BUSY) == 0U) {
                dw_qspi_disable_tx_empty_irq(qspi_base);
                dw_qspi_config_tx_fifo_threshold(qspi_base, 0U);
                qspi->state.writeable = 1U;

                if (qspi->callback) {
                    qspi->callback(qspi,QSPI_EVENT_COMMAND_COMPLETE , qspi->arg);
                }

            }
        }
    }

    /* process end of receive */
    else if ((mode & DW_QSPI_CTRLR0_TMOD_Msk) == DW_QSPI_CTRLR0_TMOD_RX) {
        if (qspi->rx_size == 0U) {
            dw_qspi_disable_rx_fifo_full_irq(qspi_base);
            dw_qspi_config_rx_data_len(qspi_base, 0U);
            dw_qspi_config_rx_fifo_threshold(qspi_base, 0U);

            if (qspi->callback) {
                qspi->callback(qspi, QSPI_EVENT_COMMAND_COMPLETE , qspi->arg);
            }

            qspi->state.readable = 1U;
        }
    }

    /* process end of transmit & receive */
    else if ((mode & DW_QSPI_CTRLR0_TMOD_Msk) == DW_QSPI_CTRLR0_TMOD_TX_RX) {
        if ((qspi->rx_size == 0U) && (qspi->tx_size == 0U)) {
            dw_qspi_disable_tx_empty_irq(qspi_base);
            dw_qspi_disable_rx_fifo_full_irq(qspi_base);
            dw_qspi_config_tx_fifo_threshold(qspi_base, 0U);
            dw_qspi_config_rx_fifo_threshold(qspi_base, 0U);
            qspi->state.readable  = 1U;
            qspi->state.writeable = 1U;

            if (qspi->callback) {
                qspi->callback(qspi, QSPI_EVENT_COMMAND_COMPLETE , qspi->arg);
            }
        } else if (qspi->tx_size == 0U) {
            // reduce interrupt times
            dw_qspi_disable_tx_empty_irq(qspi_base);
        }
    }
}

static void qspi_intr_tx_fifo_empty(csi_qspi_t *qspi)
{
    uint32_t remain_fifo;
    uint32_t value;
    uint32_t frame_len;
    uint8_t  *tx_data;
    uint32_t tx_size;

    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);

    remain_fifo = DW_MAX_SPI_TXFIFO_LV - dw_qspi_get_tx_fifo_level(qspi_base);
    frame_len   = dw_qspi_get_data_frame_len(qspi_base);

    /* process end of transcation */
    process_end_transcation(qspi);

    /* transfer loop */
    tx_data = qspi->tx_data;
    tx_size = qspi->tx_size;

    if (frame_len <= 8U) {
        while (tx_size && remain_fifo) {
            /* process 4~8bit frame len */
            value = (uint32_t)(*(uint8_t *)tx_data);
            tx_data += sizeof(uint8_t);
            dw_qspi_transmit_data(qspi_base, value);
            remain_fifo--;
            tx_size--;
        }
    } else if ((frame_len > 8U) && (frame_len <= 16U)) {
        while (tx_size && remain_fifo) {
            /* process 8~16bit frame len */
            value = (uint32_t)(*(uint16_t *)tx_data);
            tx_data += sizeof(uint16_t);
            dw_qspi_transmit_data(qspi_base, value);
            remain_fifo--;
            tx_size--;
        }
    }

    qspi->tx_data = tx_data;
    qspi->tx_size = tx_size;
}

static void qspi_intr_rx_fifo_full(csi_qspi_t *qspi)
{
    uint32_t fifo_size;
    uint32_t frame_len;
    uint8_t  *rx_data;
    uint32_t rx_size;

    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    fifo_size = dw_qspi_get_rx_fifo_level(qspi_base);
    frame_len = dw_qspi_get_data_frame_len(qspi_base);
    rx_data = qspi->rx_data;
    rx_size = qspi->rx_size;

    /* transfer loop */
    if (frame_len <= 8U) {
        while (rx_size && fifo_size) {
            *(uint8_t *)rx_data = (uint8_t)dw_qspi_receive_data(qspi_base);
            rx_data += sizeof(uint8_t);
            fifo_size--;
            rx_size--;
        }
    } else if ((frame_len > 8U) && (frame_len <= 16U)) {
        while (rx_size && fifo_size) {
            *(uint16_t *)rx_data = (uint16_t)dw_qspi_receive_data(qspi_base);
            rx_data += sizeof(uint16_t);
            fifo_size--;
            rx_size--;
        }
    }

    /* update rx fifo threshold when remain size less then default threshold*/
    if ((rx_size < (DW_DEFAULT_SPI_RXFIFO_LV + 1U)) && (rx_size > 0U)) {
        dw_qspi_config_rx_fifo_threshold(qspi_base, rx_size - 1U);
    }

    qspi->rx_data = rx_data;
    qspi->rx_size = rx_size;

    /* process end of transcation */
    process_end_transcation(qspi);

}

static __attribute__((unused)) void dw_qspi_irqhandler(void *args)
{
    uint32_t status;
    csi_qspi_t *qspi = (csi_qspi_t *)args;
    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);

    status = dw_qspi_get_interrupt_status(qspi_base);

    /* process receive fifo full interrupt */
    if (status & DW_QSPI_ISR_RXFIS) {
        qspi_intr_rx_fifo_full(qspi);
    }


    /* process transmit fifo empty interrupt */
    if (status & DW_QSPI_ISR_TXEIS) {
        qspi_intr_tx_fifo_empty(qspi);
    }

    /* process Multi-Master contention interrupt */
    if (status & DW_QSPI_ISR_MSTIS) {
        dw_qspi_clr_multi_master_irq(qspi_base);

        if (qspi->callback) {
            qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
        }
    }

    /* process receive fifo overflow interrupt */
    if (status & DW_QSPI_ISR_RXOIS) {
        dw_qspi_clr_rx_fifo_overflow_irq(qspi_base);

        if (qspi->callback) {
            qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
        }
    }

    /* process transmit fifo overflow interrupt */
    if (status & DW_QSPI_ISR_TXOIS) {
        dw_qspi_clr_tx_fifo_overflow_irq(qspi_base);

        if (qspi->callback) {
            qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
        }
    }

    /* process receive fifo underflow interrupt */
    if (status & DW_QSPI_ISR_RXUIS) {
        dw_qspi_clr_rx_fifo_underflow_irq(qspi_base);

        if (qspi->callback) {
            qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
        }
    }
}

static void dw_qspi_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
    dw_qspi_regs_t *qspi_base;
    csi_qspi_t *qspi = (csi_qspi_t *)dma->parent;
    uint32_t mode;
    qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    mode = dw_qspi_get_transfer_mode(qspi_base);

    if (event == DMA_EVENT_TRANSFER_DONE) {
        /* process end of transmit */
        if ((qspi->tx_dma != NULL) && (qspi->tx_dma->ch_id == dma->ch_id)) {
            csi_dma_ch_stop(dma);
            dw_qspi_disable_tx_dma(qspi_base);

            if (wait_ready_until_timeout(qspi, DW_DEFAULT_TRANSCATION_TIMEOUT) == CSI_OK) {

                qspi->state.writeable = 1U;
                qspi->tx_size = 0U;

                if ((mode & DW_QSPI_CTRLR0_TMOD_Msk) == DW_QSPI_CTRLR0_TMOD_TX) {
                    dw_qspi_config_dma_tx_data_level(qspi_base, 0U);

                    if (qspi->callback ) {
                        *((qspi_transfer_state_t*)arg)= *((qspi_transfer_state_t*)arg) + 1;
                        qspi->callback(qspi, QSPI_EVENT_COMMAND_COMPLETE, qspi->arg);
                    }
                } else {
                    if (qspi->state.readable == 1U ) {
                        *((qspi_transfer_state_t*)arg)= *((qspi_transfer_state_t*)arg) + 1;
                        qspi->callback(qspi, QSPI_EVENT_COMMAND_COMPLETE, qspi->arg);
                    }
                }
            }
        } else if ((qspi->rx_dma != NULL) && (qspi->rx_dma->ch_id == dma->ch_id)) {
            csi_dma_ch_stop(dma);
            dw_qspi_disable_rx_dma(qspi_base);
            dw_qspi_config_dma_rx_data_level(qspi_base, 0U);
            dw_qspi_config_rx_data_len(qspi_base, 0U);

            qspi->state.readable = 1U;
            qspi->rx_size = 0U;

            if ((mode & DW_QSPI_CTRLR0_TMOD_Msk) == DW_QSPI_CTRLR0_TMOD_RX) {
                if (qspi->callback ) {
                    *((qspi_transfer_state_t*)arg)= *((qspi_transfer_state_t*)arg) + 1;
                    qspi->callback(qspi, QSPI_EVENT_COMMAND_COMPLETE, qspi->arg);
                }
            } else {
                if (qspi->state.writeable == 1U) {
                    *((qspi_transfer_state_t*)arg)= *((qspi_transfer_state_t*)arg) + 1;
                    qspi->callback(qspi, QSPI_EVENT_COMMAND_COMPLETE, qspi->arg);
                }
            }
        }
    }
}

csi_error_t csi_qspi_set_io_mode(csi_qspi_t *qspi, csi_qspi_bus_width_t io_bus_mode)
{
    dw_qspi_regs_t *qspi_base;
    qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    switch (io_bus_mode)
    {
    case QSPI_CFG_BUS_SINGLE:
        /* code */
        dw_qspi_set_qspi_frame_fmt(qspi_base, DW_QSPI_CTRLR0_SPI_FRF_STD);
        break;
    case QSPI_CFG_BUS_DUAL:
        /* code */
        dw_qspi_set_qspi_frame_fmt(qspi_base, DW_QSPI_CTRLR0_SPI_FRF_DUAL);
        break;
    case QSPI_CFG_BUS_QUAD:
        /* code */
        dw_qspi_set_qspi_frame_fmt(qspi_base, DW_QSPI_CTRLR0_SPI_FRF_QUAD);
        break;
    
    default:
        return CSI_UNSUPPORTED;
        break;
    }

    return CSI_OK;
}

csi_error_t csi_qspi_init(csi_qspi_t *qspi, uint32_t idx)
{
    CSI_PARAM_CHK(qspi, CSI_ERROR);

    dw_qspi_regs_t *qspi_base;
    csi_error_t ret = CSI_OK;

    if (target_get(DEV_DW_QSPI_TAG, idx, &qspi->dev) != CSI_OK) {
        ret = CSI_ERROR;
    } else {
        qspi->state.writeable = 1U;
        qspi->state.readable  = 1U;
        qspi->state.error     = 0U;
        qspi->send            = NULL;
        qspi->receive         = NULL;
        qspi->send_receive    = NULL;
        qspi->rx_dma          = NULL;
        qspi->tx_dma          = NULL;
        qspi->rx_data         = NULL;
        qspi->tx_data         = NULL;
        qspi->callback        = NULL;
        qspi->arg             = NULL;
        qspi->priv            = NULL;

        qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);

        /* disable qspi core */
        dw_qspi_disable(qspi_base);
        /* disalbe all irq */
        dw_qspi_disable_all_irq(qspi_base);
        /* disable cs toggle */
        dw_qspi_disable_slave_select_toggle(qspi_base);

        /* set ssi format as motorola spi */
        dw_qspi_set_ssi_fmt(qspi_base,DW_QSPI_CTRLR0_FRF_MOTOROLA_SPI);

        /* set standard spi frame format */
        dw_qspi_set_qspi_frame_fmt(qspi_base,DW_QSPI_CTRLR0_SPI_FRF_STD);

        /* set frame len */
        dw_qspi_set_qspi_frame_len(qspi_base,8);

        /* select cs0 as default chip select signal */
        dw_qspi_enable_slave(qspi_base, 0);

        /* delay-sample with one ssi-clk */
        dw_qspi_config_rx_sample_delay(qspi_base,2);
    }

    return ret;
}

void csi_qspi_uninit(csi_qspi_t *qspi)
{
    CSI_PARAM_CHK_NORETVAL(qspi);

    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);

    /* reset all registers */
    qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    dw_qspi_reset_regs(qspi_base);

    /* unregister irq */
    csi_irq_disable((uint32_t)qspi->dev.irq_num);
    csi_irq_detach((uint32_t)qspi->dev.irq_num);
}

csi_error_t csi_qspi_attach_callback(csi_qspi_t *qspi, void *callback, void *arg)
{
    CSI_PARAM_CHK(qspi, CSI_ERROR);
    CSI_PARAM_CHK(callback, CSI_ERROR);

    qspi->callback     = callback;
    qspi->arg          = arg;
    qspi->send         = NULL;
    qspi->receive      = NULL;
    qspi->send_receive = NULL;

    return CSI_OK;
}


void csi_qspi_detach_callback(csi_qspi_t *qspi)
{
    CSI_PARAM_CHK_NORETVAL(qspi);

    qspi->callback     = NULL;
    qspi->arg          = NULL;
    qspi->send         = NULL;
    qspi->receive      = NULL;
    qspi->send_receive = NULL;
}


csi_error_t csi_qspi_mode(csi_qspi_t *qspi, csi_qspi_mode_t mode)
{
    CSI_PARAM_CHK(qspi, CSI_ERROR);

    dw_qspi_regs_t *qspi_base;
    csi_error_t   ret = CSI_OK;

    qspi_base = dw_get_reg_base(qspi);

    /* configure qspi format */
    switch (mode) {
        case QSPI_CLOCK_MODE_0 :
            dw_qspi_set_cpol0(qspi_base);
            dw_qspi_set_cpha0(qspi_base);
            break;

        case QSPI_CLOCK_MODE_1:
            dw_qspi_set_cpol0(qspi_base);
            dw_qspi_set_cpha1(qspi_base);
            break;

        case QSPI_CLOCK_MODE_2:
            dw_qspi_set_cpol1(qspi_base);
            dw_qspi_set_cpha0(qspi_base);
            break;

        case QSPI_CLOCK_MODE_3:
            dw_qspi_set_cpol1(qspi_base);
            dw_qspi_set_cpha1(qspi_base);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

uint32_t csi_qspi_frequence(csi_qspi_t *qspi, uint32_t hz)
{
    CSI_PARAM_CHK(qspi,  CSI_ERROR);
    CSI_PARAM_CHK(hz, CSI_ERROR);

    dw_qspi_regs_t *qspi_base;
    uint32_t div;
    uint32_t freq = 0U;

    qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
	dw_qspi_disable(qspi_base);
    dw_qspi_config_sclk_clock(qspi_base, soc_get_qspi_freq((uint32_t)qspi->dev.idx), hz);
    div = dw_qspi_get_sclk_clock_div(qspi_base);
    
    if (div > 0U) {
        freq =  soc_get_qspi_freq((uint32_t)qspi->dev.idx) / div;
    }
    return freq;
}

static void csi_qspi_build_xfer_object(qspi_xfer_t *xfer,csi_qspi_command_t* cmd, const void *data ,uint32_t size)
{
    uint32_t i = 0,j=0;

    /* build cmd transfer portion */
    if(cmd == NULL){
        xfer->xfer_pre_len = 0;
        
    }else{

        /* operation code */
        if(!(cmd->instruction.disabled))
        {
            xfer->xfer_pre[i++]= cmd->instruction.value;
        }

        /* addr */
        if(!(cmd->address.disabled)){
            for(j=0; j < cmd->address.size; j++)
            {
                xfer->xfer_pre[i++] = (cmd->address.value >> (8 *(cmd->address.size -j -1))) & 0xFF;
            }
        }

        /* alternative */
        if(!(cmd->alt.disabled)){
            for(j=0; j < cmd->alt.size; j++)
            {
                xfer->xfer_pre[i++] = (cmd->alt.value >> (8 *(cmd->alt.size -j -1))) & 0xFF;
            }
        }
        
        /* dummy */
        if(cmd->dummy_count){
            memset(&xfer->xfer_pre[i],0xFF,cmd->dummy_count);
            i += cmd->dummy_count;
        }

        xfer->xfer_pre_len = i;
    }

    /* build  data transfer portion */
    if(data == NULL || size == 0){
        xfer->xfer_post_len = 0;
    }else{
        xfer->xfer_post =(void*) data;
        xfer->xfer_post_len = size ;
    }
    
}
static csi_error_t qspi_wait_bus_idle(dw_qspi_regs_t *qspi_base,uint32_t time_start,uint32_t tm_ms)
{
    csi_error_t ret = CSI_OK;

    while ((dw_qspi_get_status(qspi_base) & DW_QSPI_SR_BUSY)) {
        if ((csi_tick_get_ms() - time_start) > tm_ms) {
            ret = CSI_TIMEOUT;
            break;
        }
    }

    return ret;
}
static csi_error_t qspi_wait_txfifo_empty(dw_qspi_regs_t *qspi_base,uint32_t time_start,uint32_t tm_ms)
{
    csi_error_t ret = CSI_OK;

    while (!(dw_qspi_get_status(qspi_base) & DW_QSPI_SR_TFE)) {
        if ((csi_tick_get_ms() - time_start) > tm_ms) {
            ret = CSI_TIMEOUT;
            break;
        }
    }

    return ret;
}
static void qspi_update_rxlen(dw_qspi_regs_t *qspi_base,csi_qspi_command_t *cmd, uint32_t rx_len)
{
    if(rx_len){
        switch (cmd->data.bus_width)
        {
        case QSPI_CFG_BUS_SINGLE:
            dw_qspi_config_rx_data_len(qspi_base,rx_len-1);
            break;
        case QSPI_CFG_BUS_DUAL:
            dw_qspi_config_rx_data_len(qspi_base,(rx_len>>1)-1);
            break;
        
        case QSPI_CFG_BUS_QUAD:
            dw_qspi_config_rx_data_len(qspi_base,(rx_len>>2)-1);
            break;
        
        default:
            break;
        }
    }else{
        dw_qspi_config_rx_data_len(qspi_base,0);
    }

    return; 
}

static void qspi_enhance_config_update(dw_qspi_regs_t *qspi_base,dw_qspi_xfer_enhance_cfg_t *cfg,csi_qspi_command_t *cmd,uint32_t pre_len,uint32_t post_len)
{

    /* init enhanced transfer config */
    if(pre_len){
        cfg->inst_len = (cmd->instruction.disabled == false)?DW_QSPI_SPI_CTRL0_INST_LEN_8:DW_QSPI_SPI_CTRL0_INST_LEN_0;
        cfg->addr_len = (cmd->address.disabled == false)?(cmd->address.size*8)>>2:0;
        cfg->dummy_len = cmd->dummy_count<<3;

    }
    cfg->mode = DW_QSPI_SPI_CTRL0_TRANS_TYPE_ISTD_ASTD;
    if(post_len)
    {
        if(cmd->data.bus_width == QSPI_CFG_BUS_SINGLE){
            cfg->spi_prf = DW_QSPI_CTRLR0_SPI_FRF_STD; 
            dw_qspi_set_qspi_frame_len(qspi_base,8);
        }else if (cmd->data.bus_width == QSPI_CFG_BUS_DUAL)
        {
            cfg->spi_prf = DW_QSPI_CTRLR0_SPI_FRF_DUAL;
            dw_qspi_set_qspi_frame_len(qspi_base,16);
        }else if(cmd->data.bus_width == QSPI_CFG_BUS_QUAD)
        {
            cfg->spi_prf = DW_QSPI_CTRLR0_SPI_FRF_QUAD;
            dw_qspi_set_qspi_frame_len(qspi_base,32);
        }
    }else{
        cfg->spi_prf = DW_QSPI_CTRLR0_SPI_FRF_STD;
        dw_qspi_set_qspi_frame_fmt(qspi_base,cfg->spi_prf);
        dw_qspi_set_qspi_frame_len(qspi_base,8);
    }

    return;
}

static int qspi_read_io_poll(dw_qspi_regs_t *qspi_base, void* rx_buf, uint32_t nbytes_per_frame,uint32_t read_len)
{
    uint32_t total_len,single_max,remain;
    total_len = 0;
    uint32_t timestart = csi_tick_get_ms();

    while(total_len < read_len){
        single_max = dw_qspi_get_rx_fifo_level(qspi_base);
        remain = (read_len-total_len)/nbytes_per_frame;
        single_max = (single_max <= remain) ? single_max:remain;
        while (single_max--)
        {
            if(nbytes_per_frame == 4){
                *(uint32_t *)(rx_buf) = dw_qspi_receive_data(qspi_base);
            }else if (nbytes_per_frame == 2)
            {
                *(uint16_t *)(rx_buf) = (uint16_t)dw_qspi_receive_data(qspi_base);
            }else
            {
                *(uint8_t *)(rx_buf) = (uint8_t)dw_qspi_receive_data(qspi_base);
            }
            rx_buf += nbytes_per_frame; 
            total_len +=nbytes_per_frame;
        }

        if ((csi_tick_get_ms() - timestart) > 100) {
            return CSI_TIMEOUT;
        }
        
    }

    return total_len;
}

static uint32_t qspi_get_nbytes_perframe(uint32_t dw_frf)
{
    uint32_t nbytes = 1;
    switch (dw_frf)
    {
    case DW_QSPI_CTRLR0_SPI_FRF_STD:
        nbytes =1;   
        break;
    case DW_QSPI_CTRLR0_SPI_FRF_DUAL:
        nbytes =2;
        break;
    case DW_QSPI_CTRLR0_SPI_FRF_QUAD:
        nbytes = 4;
        break;
    default:
        break;
    }

    return nbytes;
}

static int qspi_write_io_poll(dw_qspi_regs_t *qspi_base, const void* tx_buf, uint32_t nbytes_per_frame,uint32_t write_len)
{
    uint32_t total_len,single_max,remain;
    total_len = 0;
    //uint32_t timestart = csi_tick_get_ms();

    while(total_len < write_len){
        remain = DW_MAX_SPI_TXFIFO_LV - dw_qspi_get_tx_fifo_level(qspi_base);
        single_max = (write_len-total_len)/nbytes_per_frame;
        single_max = (single_max <= remain) ? single_max:remain;
        while (single_max--)
        {
            if(nbytes_per_frame == 4){
				dw_qspi_transmit_data(qspi_base,*(uint32_t*)tx_buf);
            }else if (nbytes_per_frame == 2)
            {
				dw_qspi_transmit_data(qspi_base,*(uint16_t*)tx_buf);
            }else
            {
				dw_qspi_transmit_data(qspi_base,*(uint8_t*)tx_buf);
            }
            tx_buf += nbytes_per_frame; 
            total_len +=nbytes_per_frame;
        }		
	   /* if call csi_tick_get_ms() dw-qspi tx will failed dueto tx critical timing */
       // if ((csi_tick_get_ms() - timestart) > 100) {
       //     return CSI_TIMEOUT;
       // }
        
    }
    return total_len;
}

int32_t csi_qspi_send(csi_qspi_t *qspi,csi_qspi_command_t *cmd, const void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(qspi,  CSI_ERROR);
    CSI_PARAM_CHK(cmd, CSI_ERROR);

    uint32_t timestart;
    int32_t  ret   = CSI_OK;
    int32_t  count = 0;
    dw_qspi_regs_t *qspi_base;
	uint32_t nbytes;
    qspi_xfer_t    temp_xfer = {0};
    qspi_base  = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    memset(&temp_xfer,0,sizeof(temp_xfer));
    csi_qspi_build_xfer_object(&temp_xfer,cmd,data,size);
    do {
        if ((qspi->state.writeable == 0U) || (qspi->state.readable == 0U)) {
            ret = CSI_BUSY;
            break;
        }

        timestart = csi_tick_get_ms();
        qspi->state.writeable = 0U;
        dw_qspi_xfer_enhance_cfg_t cfg = {0};
        /* set tx mode */
        dw_qspi_disable(qspi_base);
        dw_qspi_disable_slave(qspi_base, 0);
        dw_qspi_set_tx_mode(qspi_base);
        dw_qspi_config_tx_fifo_threshold(qspi_base, DW_MAX_SPI_TXFIFO_LV);
        qspi_enhance_config_update(qspi_base,&cfg,cmd,temp_xfer.xfer_pre_len,temp_xfer.xfer_post_len);        
        dw_qspi_config_enhanced_transfer(qspi_base, &cfg);
		nbytes = qspi_get_nbytes_perframe(cfg.spi_prf);
        dw_qspi_enable(qspi_base);

		   /* standard spi mode */
        if(nbytes == 1){
			dw_qspi_enable_slave(qspi_base,0);
			/* transfer pre portion */
			count = qspi_write_io_poll(qspi_base,temp_xfer.xfer_pre,nbytes,temp_xfer.xfer_pre_len);
			if(count != temp_xfer.xfer_pre_len){
				ret = CSI_ERROR;
				break;
			}
		    /* transfer post portion */
			if(size){
				count = qspi_write_io_poll(qspi_base,data,nbytes,size);
				if(count != size){
				    ret = CSI_ERROR;
					break;
				}
			}	
        }
		   /*non-standard spi mode(dual/quad) */
        else{
               /* send instruction */
            if(!cmd->instruction.disabled){
                
                dw_qspi_transmit_data(qspi_base,cmd->instruction.value);
            }
            /* send addr */
            if( !cmd->address.disabled ){
                dw_qspi_transmit_data(qspi_base,cmd->address.value);
            }
		    /* transfer post portion */
			if(size){
		        count = qspi_write_io_poll(qspi_base,data,nbytes,size);
		        if(count != size){
				    ret = CSI_ERROR;
		           	break;
		        }
			}
			dw_qspi_enable_slave(qspi_base,0);
        }


        /* Check SR.TFE is necessary when tx size = 1, because SR.BUSY has some delay before be vaild */
        timestart = csi_tick_get_ms();
        ret = qspi_wait_txfifo_empty(qspi_base,timestart,DW_MAX_BUS_FIFO_WAIT);
        if(ret){
            break;
        }

        /* wait spi bus idle */
        timestart = csi_tick_get_ms();
        ret = qspi_wait_bus_idle(qspi_base,timestart,DW_MAX_BUS_FIFO_WAIT);
        if(ret){
            break;
        }

    } while (0);

    /* if sent successfully ,return sent bytes */ 
    if (ret >= 0) {
        ret = (int32_t)count;
    }

    /* set tx available */
    dw_qspi_config_tx_fifo_threshold(qspi_base, 0U);
    qspi->state.writeable = 1U;

    return ret;
}

csi_error_t csi_qspi_send_async(csi_qspi_t *qspi,csi_qspi_command_t *cmd, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(qspi,  CSI_ERROR);

    csi_error_t ret = CSI_OK;
    dw_qspi_regs_t *qspi_base = dw_get_reg_base(qspi);

    if ((qspi->state.writeable == 0U) || (qspi->state.readable == 0U)) {
        return CSI_BUSY;
    }

    if (IS_16BIT_FRAME_LEN(qspi_base)) {
        if (size % sizeof(uint16_t)) {
            return CSI_ERROR;
        }
    }

    if (qspi->callback != NULL) {
        if (qspi->send) {
            qspi->state.writeable = 0U;
            ret = qspi->send(qspi,cmd, data, size);
            qspi->state.writeable = 1U;
        } else {
            // qspi->state.writeable = 0U;
            // csi_irq_attach((uint32_t)qspi->dev.irq_num, &dw_qspi_irqhandler, &qspi->dev);
            // csi_irq_enable((uint32_t)qspi->dev.irq_num);
            // ret = dw_qspi_send_intr(qspi, data, size);
            return CSI_UNSUPPORTED;
        }
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}

static __attribute__((unused)) csi_error_t dw_qspi_send_intr(csi_qspi_t *qspi, const void *data, uint32_t size)
{
    csi_error_t ret = CSI_OK;
    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    qspi->tx_data = (uint8_t *)data;

    do {
        // Convert byte to nums
        if (IS_16BIT_FRAME_LEN(qspi_base)) {
            qspi->tx_size = size / 2U;
        } else if (IS_8BIT_FRAME_LEN(qspi_base)) {
            qspi->tx_size = size;
        } else {
            ret = CSI_ERROR;
            break;
        }

        /* set tx mode*/
        dw_qspi_disable(qspi_base);
        dw_qspi_set_tx_mode(qspi_base);
        dw_qspi_config_tx_fifo_threshold(qspi_base, DW_DEFAULT_SPI_TXFIFO_LV);
        dw_qspi_enable(qspi_base);
        dw_qspi_enable_tx_empty_irq(qspi_base);
    } while (0);

    return ret;
}

static csi_error_t dw_qspi_send_dma(csi_qspi_t *qspi, csi_qspi_command_t *cmd, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(qspi,  CSI_ERROR);
    CSI_PARAM_CHK(cmd, CSI_ERROR);

    csi_dma_ch_config_t config;
    dw_qspi_regs_t       *qspi_base;
    csi_dma_ch_t        *dma_ch;
    csi_error_t         ret = CSI_OK;
    qspi_xfer_t         temp_xfer = {0};
    uint32_t            tx_temp_size;
	uint32_t            nbytes,timestart;
	int32_t             count;
	csi_dma_link_list_item_t dma_link_items[2];

    qspi_base = dw_get_reg_base(qspi);
    dma_ch   = (csi_dma_ch_t *)qspi->tx_dma;
    qspi->tx_data = (uint8_t *)data;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    csi_qspi_build_xfer_object(&temp_xfer,cmd,data,size);
    
    do {
        /* init enhanced transfer config */
        dw_qspi_xfer_enhance_cfg_t cfg = {0};
        dw_qspi_disable(qspi_base);
        /* set tx mode*/
        dw_qspi_set_tx_mode(qspi_base);
        qspi_enhance_config_update(qspi_base,&cfg,cmd,temp_xfer.xfer_pre_len,temp_xfer.xfer_post_len);        
        dw_qspi_config_enhanced_transfer(qspi_base, &cfg);
		nbytes = qspi_get_nbytes_perframe(cfg.spi_prf);
        dw_qspi_disable_tx_dma(qspi_base);

		/* standard spi mode */
        if(nbytes == 1){
			/* transfer pre portion */
            dw_qspi_enable(qspi_base);
			count = qspi_write_io_poll(qspi_base,temp_xfer.xfer_pre,nbytes,temp_xfer.xfer_pre_len);
			if(count != temp_xfer.xfer_pre_len){
                qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
				break;
			}
		    /* transfer post portion */
			if(size){
				count = qspi_write_io_poll(qspi_base,data,nbytes,size);
				if(count != temp_xfer.xfer_pre_len){
                    qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
					break;
				}
			}	

            /* Check SR.TFE is necessary when tx size = 1, because SR.BUSY has some delay before be vaild */
            timestart = csi_tick_get_ms();
            ret = qspi_wait_txfifo_empty(qspi_base,timestart,10);
            if(ret){
                qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
                break;
            }
            /* wait spi bus idle */
            timestart = csi_tick_get_ms();
            ret = qspi_wait_bus_idle(qspi_base,timestart,10);
            if(ret){
                qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
                break;
            }
			/* send complete signal */
			if(qspi->callback){
                qspi->callback(qspi, QSPI_EVENT_COMMAND_COMPLETE, qspi->arg);
			}
        }
		/* non-standard spi mode (dual/quad) */
        else{
			dw_qspi_disable_slave(qspi_base,0);
            /* data transfer */
            qspi->tx_size = tx_temp_size = temp_xfer.xfer_post_len;
            qspi->tx_data = temp_xfer.xfer_post;
			/* dma channel config */
            config.dst_tw = DMA_DATA_WIDTH_32_BITS;
            config.src_tw = DMA_DATA_WIDTH_32_BITS;
            config.src_inc = DMA_ADDR_INC;
            config.dst_inc = DMA_ADDR_CONSTANT;
            config.trans_dir = DMA_MEM2PERH;
            config.handshake = dw_qspi_get_hs_num(qspi, qspi_tx_hs_num);
			config.link_list_en = 1;
			config.link_list_config.lli_buf = link_list_buf;
			config.link_list_config.link_list_num = 0;
            csi_dma_ch_config(dma_ch, &config);
		    /* add dma list item */
		    qspi_dma_pre_buf[0] = cmd->instruction.value;
		    qspi_dma_pre_buf[1] = cmd->address.value;
		    dma_link_items[0].srcaddr = qspi_dma_pre_buf;
		    dma_link_items[0].dstaddr = (void *)(&(qspi_base->DR));
		    dma_link_items[0].length  = 8;
			config.group_len = DW_QSPI_DMA_BURST_LEN*4;
		    dma_add_link_list_item(&config,&dma_link_items[0]);
            soc_dcache_clean_invalid_range((unsigned long)qspi_dma_pre_buf,sizeof(qspi_dma_pre_buf));

		    dma_link_items[1].srcaddr = temp_xfer.xfer_post;
		    dma_link_items[1].dstaddr = (void *)(&(qspi_base->DR));
		    dma_link_items[1].length  = temp_xfer.xfer_post_len;
			config.group_len = DW_QSPI_DMA_BURST_LEN*4;
		    dma_add_link_list_item(&config,&dma_link_items[1]);

            soc_dcache_clean_invalid_range((unsigned long)(qspi->tx_data),qspi->tx_size);
            dw_qspi_config_dma_tx_data_level(qspi_base,DW_MAX_SPI_TXFIFO_LV-1);
		    #ifdef SINGLE_BLK
            csi_dma_ch_start(dma_ch, qspi->tx_data, (void *)(&(qspi_base->DR)), qspi->tx_size);
			#else
            csi_dma_ch_start(qspi->tx_dma, NULL, NULL,0);
			#endif
            dw_qspi_enable(qspi_base);
            dw_qspi_enable_tx_dma(qspi_base);
            if(temp_xfer.xfer_post_len){
                g_qspi_xfer_state = QSPI_XFER_DATA_START;        
				timestart = csi_tick_get_ms();
                while(!(g_qspi_xfer_state == QSPI_XFER_DATA_DONE)){
                    if ((csi_tick_get_ms() - timestart) > 1000) {
						ret = CSI_TIMEOUT;
                        break;
                    }

				}
				dw_qspi_enable_slave(qspi_base,0);
                timestart = csi_tick_get_ms();
                ret = qspi_wait_txfifo_empty(qspi_base,timestart,10);
                if(ret){
                    qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
                    break;
                }

				timestart = csi_tick_get_ms();
                while ((dw_qspi_get_status(qspi_base) & DW_QSPI_SR_BUSY)) {
                    if ((csi_tick_get_ms() - timestart) > 100) {
						ret = CSI_TIMEOUT;
                        break;
                    }
                }
            }
        }

    } while (0);
	dw_qspi_disable_tx_dma(qspi_base);	

    return ret;
}

int32_t csi_qspi_receive(csi_qspi_t *qspi, csi_qspi_command_t *cmd, void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(qspi,  CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);
    uint32_t timestart;
    int32_t count = 0U;
    int32_t  ret = CSI_OK;
    qspi_xfer_t    temp_xfer ={0};
    uint32_t       nbytes;
    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    csi_qspi_build_xfer_object(&temp_xfer,cmd,data,size);

    do {
        if ((qspi->state.writeable == 0U) || (qspi->state.readable == 0U)) {
            ret = CSI_BUSY;
            break;
        }

        dw_qspi_disable(qspi_base);
        qspi->state.readable = 0U;
        qspi->rx_data = (uint8_t *)data;
        qspi->tx_data = temp_xfer.xfer_pre;
        dw_qspi_xfer_enhance_cfg_t cfg = {0};

        /* init enhanced transfer config */
        qspi_enhance_config_update(qspi_base,&cfg,cmd,temp_xfer.xfer_pre_len,temp_xfer.xfer_post_len);
        qspi_update_rxlen(qspi_base,cmd,size);
        nbytes = qspi_get_nbytes_perframe(cfg.spi_prf);
        /* set rx mode*/
        if(cmd->data.bus_width != QSPI_CFG_BUS_SINGLE){
            dw_qspi_set_rx_mode(qspi_base);
            dw_qspi_config_enhanced_transfer(qspi_base, &cfg);
        }else{
            dw_qspi_set_tx_mode(qspi_base);
            dw_qspi_set_qspi_frame_fmt(qspi_base,DW_QSPI_CTRLR0_SPI_FRF_STD);
        }

        dw_qspi_config_rx_fifo_threshold(qspi_base, DW_DEFAULT_SPI_RXFIFO_LV);
        dw_qspi_config_tx_fifo_threshold(qspi_base, DW_DEFAULT_SPI_TXFIFO_LV);
        dw_qspi_enable(qspi_base);
        
            
        /*
         * note !!! if both  bus width of pre portion and post portion are single line 
         * we need tx only mode to send pre portion and rx only to receive data 
         */  
        if(nbytes == 1){
		   /*transmit pre-portion */ 
           count = qspi_write_io_poll(qspi_base,temp_xfer.xfer_pre ,nbytes,temp_xfer.xfer_pre_len);
		   if(count != temp_xfer.xfer_pre_len){
			   ret = CSI_ERROR;
			   break;
		   }
		   timestart = csi_tick_get_ms();
		   ret = qspi_wait_txfifo_empty(qspi_base,timestart,DW_MAX_BUS_FIFO_WAIT);
		   if(ret) {
			   break;
		   }
           timestart = csi_tick_get_ms();
           ret = qspi_wait_bus_idle(qspi_base,timestart,DW_MAX_BUS_FIFO_WAIT);
           if(ret){
               break;
           }
		   /* receive post-portion */
            dw_qspi_disable(qspi_base);
            dw_qspi_set_rx_mode(qspi_base);
            dw_qspi_enable(qspi_base);
            dw_qspi_transmit_data(qspi_base,0);
            count = qspi_read_io_poll(qspi_base,data,nbytes,size);
        }
        else{
            /* send instruction */
            if(!cmd->instruction.disabled){
                dw_qspi_transmit_data(qspi_base,cmd->instruction.value);
            }
            /* send addr */
            if( !cmd->address.disabled ){
                dw_qspi_transmit_data(qspi_base,cmd->address.value);
            }
            count = qspi_read_io_poll(qspi_base,data,nbytes,size);
        }
		/* check result */
        if(count < 0){
            /* err code */
            return count;
        }

        /* wait spi bus idle */
        timestart = csi_tick_get_ms();
        ret = qspi_wait_bus_idle(qspi_base,timestart,DW_MAX_BUS_FIFO_WAIT);
        if(ret){
            return ret;
        }
        
    } while (0);
   
    /* close qspi */
    dw_qspi_disable(qspi_base);
    dw_qspi_config_rx_data_len(qspi_base, 0U);
    dw_qspi_config_rx_fifo_threshold(qspi_base, 0U);
    qspi->state.readable = 1U;

    if (ret >= 0) {
        ret = (int32_t)count;
    }
    return ret;
}


csi_error_t csi_qspi_receive_async(csi_qspi_t *qspi,csi_qspi_command_t *cmd, void *data, uint32_t size)
{
    CSI_PARAM_CHK(qspi, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    if ((qspi->state.writeable == 0U) || (qspi->state.readable == 0U)) {
        return CSI_BUSY;
    } else {
        dw_qspi_regs_t *qspi_base = dw_get_reg_base(qspi);

        if (IS_16BIT_FRAME_LEN(qspi_base)) {
            if (size % sizeof(uint16_t)) {
                return CSI_ERROR;
            }
        }

        if (qspi->callback != NULL) {
            if (qspi->receive) {
                qspi->state.readable = 0U;
                ret = qspi->receive(qspi,cmd, data, size);
                qspi->state.readable = 1U;
            } else {
                //spi->state.readable = 0U;
                //csi_irq_attach((uint32_t)spi->dev.irq_num, &dw_spi_irqhandler, &spi->dev);
                //csi_irq_enable((uint32_t)spi->dev.irq_num);
                //ret = dw_spi_receive_intr(spi, data, size);
                return CSI_UNSUPPORTED;
            }
        } else {
            ret = CSI_ERROR;
        }
    }

    return ret;
}

static __attribute__((unused)) csi_error_t dw_qspi_receive_intr(csi_qspi_t *qspi, void *data, uint32_t size)
{
    csi_error_t ret = CSI_OK;
    uint32_t rx_fifo_lv;

    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);

    qspi->rx_data = (uint8_t *)data;

    do {
        // Convert byte to nums
        if (IS_16BIT_FRAME_LEN(qspi_base)) {
            qspi->rx_size = size / 2U;
        } else if (IS_8BIT_FRAME_LEN(qspi_base)) {
            qspi->rx_size = size;
        } else {
            ret = CSI_ERROR;
            break;
        }

        /* set rx mode*/
        dw_qspi_disable(qspi_base);
        dw_qspi_set_rx_mode(qspi_base);
        dw_qspi_config_rx_data_len(qspi_base, qspi->rx_size - 1U);
        rx_fifo_lv = (qspi->rx_size < DW_DEFAULT_SPI_RXFIFO_LV) ? (qspi->rx_size - 1U) : DW_DEFAULT_SPI_RXFIFO_LV;
        dw_qspi_config_rx_fifo_threshold(qspi_base, rx_fifo_lv);
        dw_qspi_enable(qspi_base);
        dw_qspi_enable_rx_fifo_full_irq(qspi_base);
        dw_qspi_transmit_data(qspi_base, 0U);
    } while (0);

    return ret;
}

static csi_error_t dw_qspi_receive_dma(csi_qspi_t *qspi, csi_qspi_command_t *cmd,void *data, uint32_t size)
{
    csi_dma_ch_config_t config;
    dw_qspi_regs_t       *qspi_base;
    csi_error_t         ret = CSI_OK;
    qspi_xfer_t    temp_xfer ={0};
    uint32_t       nbytes;
    uint32_t       timestart;
	int32_t        count;
    dw_qspi_xfer_enhance_cfg_t cfg = {0};
    qspi_base = dw_get_reg_base(qspi);
    qspi->rx_data = (uint8_t *)data;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    csi_qspi_build_xfer_object(&temp_xfer,cmd,data,size);

    do {
        dw_qspi_disable(qspi_base);
        /* init enhanced transfer config */
        qspi_enhance_config_update(qspi_base,&cfg,cmd,temp_xfer.xfer_pre_len,temp_xfer.xfer_post_len);
        qspi_update_rxlen(qspi_base,cmd,size);
        nbytes = qspi_get_nbytes_perframe(cfg.spi_prf);
        if(nbytes == 1){
            dw_qspi_set_tx_mode(qspi_base);
            dw_qspi_set_qspi_frame_fmt(qspi_base,DW_QSPI_CTRLR0_SPI_FRF_STD);
        }else{
            dw_qspi_set_rx_mode(qspi_base);
            dw_qspi_config_enhanced_transfer(qspi_base, &cfg);
        }
        dw_qspi_config_rx_fifo_threshold(qspi_base, 0xff);
        dw_qspi_config_tx_fifo_threshold(qspi_base, 0xff);
        dw_qspi_enable(qspi_base);
        /* set transfer mode */
        if(nbytes == 1){			
		   /*transmit pre-portion */ 
           count = qspi_write_io_poll(qspi_base,temp_xfer.xfer_pre ,nbytes,temp_xfer.xfer_pre_len);
		   if(count != temp_xfer.xfer_pre_len){
               qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
			   return CSI_ERROR;
		   }
           timestart = csi_tick_get_ms();
           ret = qspi_wait_bus_idle(qspi_base,timestart,10);
           if(ret){
               qspi->callback(qspi, QSPI_EVENT_ERROR, qspi->arg);
               return ret;
           }
		   /* receive post-portion */
            dw_qspi_disable(qspi_base);
            dw_qspi_set_rx_mode(qspi_base);
            dw_qspi_enable(qspi_base);
            dw_qspi_transmit_data(qspi_base,0);
            count = qspi_read_io_poll(qspi_base,data,nbytes,size);
			/* send complete signal */
			if(qspi->callback){
                qspi->callback(qspi, QSPI_EVENT_COMMAND_COMPLETE, qspi->arg);
			}
        }
        else{
            qspi->rx_size = size>>2 ;
            config.src_tw = DMA_DATA_WIDTH_32_BITS;
            config.dst_tw = DMA_DATA_WIDTH_32_BITS;

            config.src_inc = DMA_ADDR_CONSTANT;
            config.dst_inc = DMA_ADDR_INC;
            config.group_len = DW_QSPI_DMA_BURST_LEN*4;
            config.trans_dir = DMA_PERH2MEM;
            config.handshake = dw_qspi_get_hs_num(qspi, qspi_rx_hs_num);;
            csi_dma_ch_config(qspi->rx_dma, &config);
            //dw_qspi_set_rx_mode(qspi_base);
            qspi->rx_data = data;
            //dw_qspi_config_enhanced_transfer(qspi_base, &cfg);
            dw_qspi_config_rx_data_len(qspi_base, qspi->rx_size - 1U);
            dw_qspi_config_dma_rx_data_level(qspi_base, DW_QSPI_DMA_BURST_LEN-1);
            dw_qspi_enable_rx_dma(qspi_base);
            soc_dcache_clean_invalid_range((unsigned long)qspi->rx_data, size);
            g_qspi_xfer_state = QSPI_XFER_DATA_START;
            /* send instruction */
            if(!cmd->instruction.disabled){
                dw_qspi_transmit_data(qspi_base,cmd->instruction.value);
            }
            /* send addr */
            if( !cmd->address.disabled ){
                dw_qspi_transmit_data(qspi_base,cmd->address.value);
            }
            csi_dma_ch_start(qspi->rx_dma, (void *)(&(qspi_base->DR)), qspi->rx_data, size);
            dw_qspi_enable(qspi_base);
			timestart = csi_tick_get_ms();
            while(!(g_qspi_xfer_state==QSPI_XFER_DATA_DONE)){
				if((csi_tick_get_ms() - timestart) > 1000){
					ret = CSI_TIMEOUT;
					break;
				}
			}
        }

    } while (0);
	dw_qspi_disable_rx_dma(qspi_base);

    timestart = csi_tick_get_ms();
    ret = qspi_wait_bus_idle(qspi_base,timestart,10);
    if(ret){
        return ret;
    }
    return ret;
}

int32_t csi_qspi_send_receive(csi_qspi_t *qspi,csi_qspi_command_t *cmd, const void *tx_data, void *rx_data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(qspi,      CSI_ERROR);
    CSI_PARAM_CHK(tx_data, CSI_ERROR);
    CSI_PARAM_CHK(rx_data,  CSI_ERROR);
    CSI_PARAM_CHK(size,     CSI_ERROR);

    uint32_t value;
    uint32_t timestart;
    uint32_t count = 0U;
    int32_t  ret   = CSI_OK;
    uint32_t tx_size, rx_size;
    uint8_t  *tx_data1, *rx_data1;
    uint32_t current_size;

    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);

    do {

        if ((qspi->state.writeable == 0U) || (qspi->state.readable == 0U)) {
            ret = CSI_BUSY;
            break;
        }

        if (IS_16BIT_FRAME_LEN(qspi_base)) {
            if (size % sizeof(uint16_t)) {
                ret = CSI_ERROR;
                break;
            }
        }

        timestart = csi_tick_get_ms();
        qspi->state.writeable = 0U;
        qspi->state.readable  = 0U;
        tx_data1 = (uint8_t *)tx_data;

        if (IS_16BIT_FRAME_LEN(qspi_base)) {
            tx_size = size / 2U;
        } else {
            tx_size = size;
        }

        rx_data1 = (uint8_t *)rx_data;

        if (IS_16BIT_FRAME_LEN(qspi_base)) {
            rx_size = size / 2U;
        } else {
            rx_size = size;
        }

        /* set tx rx mode*/
        dw_qspi_disable(qspi_base);
        dw_qspi_set_tx_rx_mode(qspi_base);
        dw_qspi_config_tx_fifo_threshold(qspi_base, DW_DEFAULT_SPI_TXFIFO_LV);
        dw_qspi_config_rx_fifo_threshold(qspi_base, DW_DEFAULT_SPI_RXFIFO_LV);
        dw_qspi_enable(qspi_base);

        /* transfer loop */
        if (IS_8BIT_FRAME_LEN(qspi_base)) {
            while ((tx_size > 0U) || (rx_size > 0U)) {
                /* process tx fifo empty */
                if (tx_size > 0U) {
                    current_size = DW_MAX_SPI_TXFIFO_LV - dw_qspi_get_tx_fifo_level(qspi_base);

                    if (current_size > tx_size) {
                        current_size = tx_size;

                    }

                    while (current_size--) {
                        value = (uint32_t)(*(uint8_t *)tx_data1);
                        dw_qspi_transmit_data(qspi_base, value);
                        tx_data1 += 1;
                        count += 1U;
                        tx_size--;
                    }
                }

                /* process rx fifo not empty */
                if (rx_size > 0U) {
                    current_size = dw_qspi_get_rx_fifo_level(qspi_base);

                    if (current_size > rx_size) {
                        current_size = rx_size;

                    }

                    while (current_size--) {
                        *(uint8_t *)rx_data1 = (uint8_t)dw_qspi_receive_data(qspi_base);
                        rx_data1 += 1;
                        rx_size--;
                    }
                }

                if ((csi_tick_get_ms() - timestart) > timeout) {
                    break;
                }
            }
        } else if (IS_16BIT_FRAME_LEN(qspi_base)) {
            while ((tx_size > 0U) || (rx_size > 0U)) {
                /* process tx fifo empty */
                if (tx_size > 0U) {
                    current_size = DW_MAX_SPI_TXFIFO_LV - dw_qspi_get_tx_fifo_level(qspi_base);

                    if (current_size > tx_size) {
                        current_size = tx_size;

                    }

                    while (current_size--) {
                        value = (uint32_t)(*(uint16_t *)tx_data1);
                        dw_qspi_transmit_data(qspi_base, value);
                        tx_data1 += 2;
                        count += 2U;
                        tx_size--;
                    }
                }

                /* process rx fifo not empty */
                if (rx_size > 0U) {
                    current_size = dw_qspi_get_rx_fifo_level(qspi_base);

                    if (current_size > rx_size) {
                        current_size = rx_size;

                    }

                    while (current_size--) {
                        *(uint16_t *)rx_data1 = (uint16_t)dw_qspi_receive_data(qspi_base);
                        rx_data1 += 2;
                        rx_size--;
                    }
                }

                if ((csi_tick_get_ms() - timestart) > timeout) {
                    break;
                }

            }
        }

        /* wait end of transcation */
        while (dw_qspi_get_status(qspi_base) & DW_QSPI_SR_BUSY) {
            if ((csi_tick_get_ms() - timestart) > timeout) {
                break;
            }
        }
    } while (0);

    /* close qspi */
    qspi->state.writeable = 1U;
    qspi->state.readable  = 1U;

    if (ret >= 0) {
        ret = (int32_t)count;
    }

    return ret;
}


csi_error_t csi_qspi_send_receive_async(csi_qspi_t *qspi, csi_qspi_command_t *cmd, const void *tx_data, void *rx_data, uint32_t size)
{
    CSI_PARAM_CHK(qspi, CSI_ERROR);
    CSI_PARAM_CHK(tx_data, CSI_ERROR);
    CSI_PARAM_CHK(rx_data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);

    return CSI_UNSUPPORTED;
}

static __attribute__((unused))   csi_error_t dw_qspi_send_receive_intr(csi_qspi_t *qspi, const void *data_out, void *data_in, uint32_t size)
{
    csi_error_t ret = CSI_OK;
    uint32_t rx_fifo_lv;

    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);

    qspi->tx_data = (uint8_t *)data_out;
    qspi->rx_data = (uint8_t *)data_in;

    do {
        if (IS_16BIT_FRAME_LEN(qspi_base)) {
            qspi->tx_size = size / 2U;
            qspi->rx_size = size / 2U;
        } else if (IS_8BIT_FRAME_LEN(qspi_base)) {
            qspi->tx_size = size;
            qspi->rx_size = size;
        } else {
            ret = CSI_ERROR;
            break;
        }

        /* set tx rx mode*/
        dw_qspi_disable(qspi_base);
        dw_qspi_set_tx_rx_mode(qspi_base);
        dw_qspi_config_rx_data_len(qspi_base, qspi->rx_size - 1U);
        dw_qspi_config_tx_fifo_threshold(qspi_base, DW_DEFAULT_SPI_TXFIFO_LV);
        rx_fifo_lv = (qspi->rx_size < (DW_DEFAULT_SPI_RXFIFO_LV + 1U)) ? (qspi->rx_size - 1U) : DW_DEFAULT_SPI_RXFIFO_LV;
        dw_qspi_config_rx_fifo_threshold(qspi_base, rx_fifo_lv);
        dw_qspi_enable(qspi_base);
        dw_qspi_enable_rx_fifo_full_irq(qspi_base);
        dw_qspi_enable_tx_empty_irq(qspi_base);

    } while (0);

    return ret;
}


csi_error_t csi_qspi_get_state(csi_qspi_t *qspi, csi_state_t *state)
{
    CSI_PARAM_CHK(qspi, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);

    *state = qspi->state;
    return CSI_OK;
}

csi_error_t csi_qspi_link_dma(csi_qspi_t *qspi, csi_dma_ch_t *tx_dma, csi_dma_ch_t *rx_dma)
{
    CSI_PARAM_CHK(qspi, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    if (tx_dma != NULL) {
        tx_dma->parent = qspi;
        ret = csi_dma_ch_alloc(tx_dma, -1, -1);

        if (ret == CSI_OK) {
            csi_dma_ch_attach_callback(tx_dma, dw_qspi_dma_event_cb, (void*)(&g_qspi_xfer_state));
            qspi->tx_dma = tx_dma;
            qspi->send = dw_qspi_send_dma;
        } else {
            tx_dma->parent = NULL;
        }
    } else {
        if (qspi->tx_dma) {
            csi_dma_ch_detach_callback(qspi->tx_dma);
            csi_dma_ch_free(qspi->tx_dma);
            qspi->tx_dma = NULL;
        }

        qspi->send = NULL;
    }

    if (ret == CSI_OK) {
        if (rx_dma != NULL) {
            rx_dma->parent = qspi;
            ret = csi_dma_ch_alloc(rx_dma, -1, -1);

            if (ret == CSI_OK) {
                csi_dma_ch_attach_callback(rx_dma, dw_qspi_dma_event_cb, (void*)(&g_qspi_xfer_state));
                qspi->rx_dma = rx_dma;
                qspi->receive = dw_qspi_receive_dma;
            } else {
                rx_dma->parent = NULL;
            }
        } else {
            if (qspi->rx_dma) {
                csi_dma_ch_detach_callback(qspi->rx_dma);
                csi_dma_ch_free(qspi->rx_dma);
                qspi->rx_dma = NULL;
            }

            qspi->receive = NULL;
        }
    }


    // if (ret == CSI_OK) {
    //     if ((tx_dma != NULL) && (rx_dma != NULL)) {
    //         spi->send_receive =  dw_spi_send_receive_dma;
    //     } else {
    //         spi->send_receive = NULL;
    //     }
    // }

    return ret;
}

void csi_qspi_select_slave(csi_qspi_t *qspi, uint32_t slave_num)
{
    CSI_PARAM_CHK_NORETVAL(qspi);

    dw_qspi_regs_t *qspi_base = (dw_qspi_regs_t*)(qspi->dev.reg_base);
    dw_qspi_enable_slave(qspi_base, slave_num);
}

#ifdef CONFIG_PM
csi_error_t dw_qspi_pm_action(csi_dev_t *dev, csi_pm_dev_action_t action)
{
    CSI_PARAM_CHK(dev, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    csi_pm_dev_t *pm_dev = &dev->pm_dev;

    switch (action) {
        case PM_DEV_SUSPEND:
            csi_pm_dev_save_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 7U);
            csi_pm_dev_save_regs(pm_dev->reten_mem + 7U, (uint32_t *)(dev->reg_base + 44U), 1U);
            csi_pm_dev_save_regs(pm_dev->reten_mem + 8U, (uint32_t *)(dev->reg_base + 76U), 3U);
            csi_pm_dev_save_regs(pm_dev->reten_mem + 11U, (uint32_t *)(dev->reg_base + 160U), 1U);
            break;

        case PM_DEV_RESUME:
            csi_pm_dev_restore_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 2U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 3U, (uint32_t *)dev->reg_base + 12U, 4U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 7U, (uint32_t *)(dev->reg_base + 44U), 1U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 8U, (uint32_t *)(dev->reg_base + 76U), 3U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 11U, (uint32_t *)(dev->reg_base + 160U), 1U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 2U, (uint32_t *)(dev->reg_base + 8U), 1U);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

csi_error_t csi_qspi_enable_pm(csi_qspi_t *qspi)
{
    return csi_pm_dev_register(&qspi->dev, dw_qspi_pm_action, 36U, 0U);
}

void csi_qspi_disable_pm(csi_qspi_t *qspi)
{
    csi_pm_dev_unregister(&qspi->dev);
}
#endif
