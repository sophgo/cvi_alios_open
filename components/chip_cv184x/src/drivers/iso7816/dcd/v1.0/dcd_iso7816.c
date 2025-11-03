/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */


/******************************************************************************
 * @file     dcd_iso7816.c
 * @brief    CSI Source File for ISO7816 Driver
 * @version  V1.0
 * @date     02. Nov 2019
 ******************************************************************************/

#include "dcd_iso7816.h"
#include "drv/cvi_irq.h"
#include "string.h"
#include "stdio.h"

#define DEF_EVENT_INT_MASK (ISO_7816_ERR_EV_IRQ | ISO_7816_BREAK_IRQ)
#define DEFAULT_WT 9600
#define CONFIG_ISO7816_NUM 1

dcd_iso7816_priv_t dcd_iso7816_instance[1];

/*
static void dcd_iso7816_tx_end(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CONTROL |= (1 << 7);
}
*/

static void dcd_iso7816_CRC_LRC_enable(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    if (en) {
        addr->CONTROL |= 1 << 6;
    } else {
        addr->CONTROL &= ~(1 << 6);
    }
}

/*power down*/
static void dcd_iso7816_power_down(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CONTROL |= 1 << 5;
}

typedef enum {
    DCD_ISO7816_TX,
    DCD_ISO7816_RX,
} dcd_iso7816_send_type_t;

static void dcd_iso7816_set_send_type(dcd_iso7816_priv_t *priv, dcd_iso7816_send_type_t type)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    if (type == DCD_ISO7816_TX) {
        addr->CONTROL |= 1 << 4;
        priv->rt = TX_MODE;
    } else {
        addr->CONTROL &= ~(1 << 4);
        priv->rt = RX_MODE;
    }
}

static dcd_iso7816_send_type_t dcd_iso7816_send_type(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    if (addr->CONTROL & (1 << 4)) {
        return DCD_ISO7816_TX;
    }

    return DCD_ISO7816_RX;
}

static void dcd_iso7816_stop(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CONTROL |= 1 << 3;
}

static void dcd_iso7816_clear_error(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CONTROL |= 1 << 2;
}

static void dcd_iso7816_warm_reset(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CONTROL |= 1 << 1;
}

static void dcd_iso7816_start(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CONTROL |= 1;
}

static uint32_t dcd_iso7816_mstatus(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    return addr->MSTATUS_MMASK;
}

static void dcd_iso7816_set_interrupt_mask(dcd_iso7816_priv_t *priv, uint32_t mask, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    uint32_t val = addr->MSTATUS_MMASK;
    val &= ~mask;

    if (en) {
        val |= mask;
    }

    addr->MSTATUS_MMASK |= val;
}

static uint32_t dcd_iso7816_card_status(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    return addr->CSTATUS;
}

static void dcd_iso7816_set_direct_convention(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CSTATUS &= ~(1 << 4);
}

static void dcd_iso7816_set_invert_convention(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CSTATUS |= (1 << 4);
}

static void dcd_iso7816_set_odd_parity(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    if (en) {
        addr->CSTATUS |= (1 << 8);
    } else {
        addr->CSTATUS &= ~(1 << 8);
    }
}

static void dcd_iso7816_interrupt_enable(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    uint32_t val = addr->CONFIG;
    val &= ~(1 << 7);

    if(en) {
        val |= 1 << 7;
    }

    addr->CONFIG = val;
}

/*
static void dcd_iso7816_dma_enable(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    uint32_t val = addr->CONFIG;
    val &= ~(1 << 6);

    if(en) {
        val |= 1 << 6;
    }

    addr->CONFIG = val;
}
*/

static void dcd_iso7816_block_mode_enable(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    uint32_t val = addr->CONFIG;
    val &= ~(1 << 5);

    if(en) {
        val |= 1 << 5;
    }

    addr->CONFIG = val;
}

static void dcd_iso7816_clock_stop(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    uint32_t val = addr->CONFIG;
    val &= ~(1 << 4);

    if(en) {
        val |= 1 << 4;
    }

    addr->CONFIG = val;
}

/**
 * @brief Set/reset card voltage class
 * @param *dsmart Pointer to DSMART device
 * @param vclass Voltage class value
 */
static void dcd_iso7816_set_voltage_class(dcd_iso7816_priv_t *priv, dcd_vclass_t vclass)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    uint32_t val = addr->CONFIG;
    val &= ~(3 << 2);
    val |= vclass << 2;

    addr->CONFIG = val;
}

typedef enum {
    T0_MODE,
    T1_MODE,
} iso7816_protocol_t;

static void dcd_iso7816_set_protocol(dcd_iso7816_priv_t *priv, iso7816_protocol_t protocol)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    uint32_t val = addr->CONFIG;
    val &= ~(1 << 1);

    if (protocol == T1_MODE) {
        val |= 1 << 1;
    }

    addr->CONFIG = val;
}

static void dcd_iso7816_enable_card_detect(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    uint32_t val = addr->CONFIG;
    val &= 0xfe;

    if (en) {
        val |= 1;
    }

    addr->CONFIG = val;
}

/*val: 0~3*/
static void dcd_iso7816_set_tx_threshold(dcd_iso7816_priv_t *priv, uint32_t val)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    uint32_t reg_val = addr->MISC;
    reg_val &= ~(3 << 6);
    reg_val |= val << 6;

    addr->MISC = reg_val;
}

/*val: 0~3*/
static void dcd_iso7816_set_rx_threshold(dcd_iso7816_priv_t *priv, uint32_t val)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    uint32_t reg_val = addr->MISC;
    reg_val &= ~(3 << 4);
    reg_val |= val << 4;

    addr->MISC = reg_val;
}

static void dcd_iso7816_enable_crc(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    uint32_t reg_val = addr->MISC;
    reg_val &= ~1;
    if (en) {
        reg_val |= 1;
    }

    addr->MISC = reg_val;
}

static void dcd_iso7816_set_crc_type(dcd_iso7816_priv_t *priv, crc_type_t type)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    uint32_t reg_val = addr->MISC;
    reg_val &= ~(1 << 1);
    if (type == CRC) {
        reg_val |= 1 << 1;
    }

    addr->MISC = reg_val;
}

static void dcd_iso7816_enable_rx_fifo(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    uint32_t reg_val = addr->MISC;
    reg_val &= ~(1 << 2);
    if (en) {
        reg_val |= 1 << 2;
    }

    addr->MISC = reg_val;
}

static void dcd_iso7816_enable_tx_fifo(dcd_iso7816_priv_t *priv, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    uint32_t reg_val = addr->MISC;
    reg_val &= ~(1 << 3);
    if (en) {
        reg_val |= 1 << 3;
    }

    addr->MISC = reg_val;
}

/*fCCLK= fCLK/{(CCLK_DIV+1)x2}.*/
/*cclkdiv= (Input frequency/(2*cardFrequency)) -1;*/
static void dcd_iso7816_set_clk_div(dcd_iso7816_priv_t *priv, uint32_t val)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->CCLK_DIV = val;
}

/*
static uint8_t dcd_iso7816_lrc(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    return addr->ETU_CRC0_LRC;
}

static uint16_t dcd_iso7816_crc(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    return addr->ETU_CRC0_LRC + (addr->EGT_CRC1 << 8);
}
*/
static void dcd_iso7816_set_egt(dcd_iso7816_priv_t *priv, uint8_t etg)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->EGT_CRC1 = etg;
}

static void dcd_iso7816_set_cwt(dcd_iso7816_priv_t *priv, uint8_t cwt)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->WT1_CWT &= ~(0xf);
    addr->WT1_CWT |= (cwt & 0xf);
}

static void dcd_iso7816_set_bgt(dcd_iso7816_priv_t *priv, uint8_t val)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->WT0_BGT &= ~0x1f;
    addr->WT0_BGT = (0xff & val);
}

static void dcd_iso7816_set_wt(dcd_iso7816_priv_t *priv, uint32_t val)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    addr->WT0_BGT &= ~(0x07 << 5);
    addr->WT0_BGT |= (val & 0x07) << 5;
    addr->WT1_CWT &= ~(0x0f << 4);
    addr->WT1_CWT |= (val & 0x78) << 4;
    addr->WT2 = (val & 0x7f80) >> 7;
    addr->WT3 = (val & 0x7f8000) >> 15;
}

static void dcd_iso7816_set_error_mask(dcd_iso7816_priv_t *priv, uint32_t mask, int en)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    if (en) {
        addr->ERR_MERR |= mask;
    } else {
        addr->ERR_MERR &= ~mask;
    }
}

static uint32_t dcd_iso7816_error_status(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    return addr->ERR_MERR;
}

/*dsmart->crc0lrc = ((Fi & 0xF)<<4)|(Di & 0xF); };*/
static void dcd_iso7816_set_etu(dcd_iso7816_priv_t *priv, uint32_t val)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->ETU_CRC0_LRC = val;
}

static void dcd_iso7816_write_byte(dcd_iso7816_priv_t *priv, uint8_t val)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    addr->RW_FIFO = val;
}

static uint8_t dcd_iso7816_read_byte(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    return addr->RW_FIFO;
}

static uint32_t dcd_iso7816_fifo_data_length(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;

    return addr->LENGTH;
}


static uint32_t dcd_iso7816_clear_initial_irq(dcd_iso7816_priv_t *priv)
{
    dcd_iso7816_reg_t *addr =  (dcd_iso7816_reg_t *)priv->base;
    addr->CONFIG = 0;
    addr->MISC = 0;
    addr->MSTATUS_MMASK = 0;
    //addr->CONTROL |= 1 << 3;
    return 0;
}

static void dcd_iso7816_recv_it(dcd_iso7816_priv_t *priv)
{
    uint32_t data_len = dcd_iso7816_fifo_data_length(priv);

    while(data_len && priv->rx_recv_cnt) {
        *priv->rx_buf = dcd_iso7816_read_byte(priv);
        priv->rx_buf++;
        priv->rx_recv_cnt--;
        data_len--;
    }

    if (priv->rx_recv_cnt == 0) {
        dcd_iso7816_set_interrupt_mask(priv, ISO_7816_RX_EV_IRQ, 0);
        if (priv->cb != NULL && priv->card_sta == ACTIVATE) {
            priv->cb(ISO7816_EVENT_READ_COMPLETE, priv->cb_arg);
        }
        return;
    }

    if (priv->rx_recv_cnt >= 7) {
        /*threshold is 7 bytes*/
        dcd_iso7816_set_rx_threshold(priv, 2);
    } else if (priv->rx_recv_cnt >= 3) {
        /*threshold is 3 bytes*/
        dcd_iso7816_set_rx_threshold(priv, 1);
    }  else {
        /*threshold is 1 bytes*/
        dcd_iso7816_set_rx_threshold(priv, 0);
    }
}

static void dcd_iso7816_trf_it(dcd_iso7816_priv_t *priv, uint32_t card_event)
{
    uint32_t fifo_avail = 0;

    if (priv->tx_trf_cnt) {
        fifo_avail = ISO7816_TX_FIFO_SIZE - dcd_iso7816_fifo_data_length(priv);
        while(priv->tx_trf_cnt && fifo_avail) {
            dcd_iso7816_write_byte(priv, *priv->tx_buf);
            priv->tx_buf++;
            priv->tx_trf_cnt--;
            fifo_avail--;
        }

        if (priv->tx_trf_cnt == 0) {
            dcd_iso7816_set_interrupt_mask(priv, ISO_7816_TX_EV_IRQ, 0);
        }
    } else if (card_event == ISO_7816_TX_EM_IRQ && dcd_iso7816_fifo_data_length(priv) == 0) {
        dcd_iso7816_set_interrupt_mask(priv, ISO_7816_TX_EV_IRQ | ISO_7816_TX_EM_IRQ, 0);
        //dcd_iso7816_set_send_type(priv, DCD_ISO7816_RX);
        if (priv->cb != NULL && priv->card_sta == ACTIVATE) {
            priv->cb(ISO7816_EVENT_WRITE_COMPLETE, priv->cb_arg);
        }
    }
}

static void dcd_iso7816_check_trf(dcd_iso7816_priv_t *priv)
{
    if (priv->rx_recv_cnt && dcd_iso7816_send_type(priv) == DCD_ISO7816_TX) {
        if (priv->cb != NULL) {
            priv->cb(ISO7816_EVENT_READ_ERROR, priv->cb_arg);
        }
        priv->rx_recv_cnt = 0;
    }

    if (priv->tx_trf_cnt && dcd_iso7816_send_type(priv) == DCD_ISO7816_RX) {
        if (priv->cb != NULL) {
            priv->cb(ISO7816_EVENT_WRITE_ERROR, priv->cb_arg);
        }
        priv->tx_trf_cnt = 0;
    }
}

static int dcd_card_activate(dcd_iso7816_priv_t *priv)
{
    int32_t card_event = dcd_iso7816_mstatus(priv);
    uint32_t card_status = dcd_iso7816_card_status(priv);

    if (ISO_7816_TIMOUT_CARD_STA & card_status) {
        priv->atr.atr_sta = T0;
        return -1;
    }

    if ((card_event & ISO_7816_RX_EV_IRQ) && dcd_iso7816_send_type(priv) == DCD_ISO7816_RX) {
        dcd_iso7816_recv_it(priv);
        priv->atr.atr_sta = atr_next_state(priv, priv->atr.buf[priv->atr.buf_idx]);
        priv->atr.buf_idx++;
        if (priv->atr.atr_sta != TEND_S) {
            csi_iso7816_master_receive_it(priv->idx, &priv->atr.buf[priv->atr.buf_idx], 1);
        }
    } else if (dcd_iso7816_send_type(priv) != DCD_ISO7816_RX) {
        priv->atr.atr_sta = T0;
        return -1;
    }

    if ((card_status & ISO_7816_CRD_ACT_CARD_STA) && priv->atr.atr_sta == TEND_S) {
        if (ISO_7816_CONV_CARD_STA & card_status) {
            priv->atr.is_direct_encode = 0;
            dcd_iso7816_set_invert_convention(priv);
        } else {
            priv->atr.is_direct_encode = 1;
            dcd_iso7816_set_direct_convention(priv);
        }

        atr_parse(priv);
        priv->atr.atr_sta = T0;

        atr_result_t *result;
        result = atr_get_result(priv->idx);
        priv->atr.pps[0] = 0xff;
        priv->atr.pps[1] = 1 << 4 | result->T;
        priv->atr.pps[2] = (result->fi << 4) | (result->di);
        priv->atr.pps[3] = priv->atr.pps[0] ^ priv->atr.pps[1] ^ priv->atr.pps[2];

        csi_iso7816_master_send_it(0, priv->atr.pps, 4);
        return 1;
    }

    return 0;
}

static void cwt_config(dcd_iso7816_priv_t *priv, atr_result_t *result)
{
    uint8_t cwt = 0;
    int i = 1, j = 13;

    if (result->bwi != -1) {
        j = result->bwi;
    }

    while(j) {
        i *= 2;
        j--;
    }

    cwt = 11 + i;
    dcd_iso7816_set_cwt(priv, cwt);
    dcd_iso7816_set_error_mask(priv, ISO_7816_CWT_TIM , 1);
}

static void bwt_config(dcd_iso7816_priv_t *priv, atr_result_t *result)
{
    int j = 4;
    int i = 1;
    uint32_t bwt = 0;
    if (result->bwi != -1) {
        j = result->bwi;
    }

    while(j) {
        i *= 2;
        j--;
    }

    bwt = 11 + i * 960 * result->di;
    /*set bwt, T=1 wt0~wt3 is bwt*/
    dcd_iso7816_set_wt(priv, bwt);
    dcd_iso7816_block_mode_enable(priv, 1);
}

static int protocol_config(dcd_iso7816_priv_t *priv, atr_result_t *result)
{
    uint32_t clk_div = (IHS_VALUE + IHS_VALUE % result->f_max) / result->f_max;
    clk_div = (clk_div + (clk_div % 2)) / 2 -1;
    if (clk_div < 255) {
        dcd_iso7816_set_clk_div(priv, clk_div);
    }

    dcd_iso7816_set_protocol(priv, result->T);

    uint8_t etu = result->fi << 4 | result->di;
    dcd_iso7816_set_etu(priv, etu);
    dcd_iso7816_set_send_type(priv, DCD_ISO7816_TX);

    /*set GT and CGT*/
    dcd_iso7816_set_egt(priv, result->N);

    int atr_fi[] = {372, 372, 558, 744, 1116, 1488, 1860, -1, -1, 512, 768, 1024, 1536, 2048, -1, -1};
    int fi = 372, wi = 10;
    if (result->T == 0) {
        if (result->wi != 0) {
            int fi = atr_fi[result->di];
            if (fi == -1) {
                fi = 372;
            }
            wi = result->wi;
        }
        dcd_iso7816_set_wt(priv, wi * 960 * fi);
        dcd_iso7816_set_error_mask(priv, ISO_7816_REP_ERR , 1);
    } else if (result->T == 1) {
        dcd_iso7816_set_bgt(priv, 22);
        cwt_config(priv, result);
        bwt_config(priv, result);

        if (result->t1_check == CRC) {
            dcd_iso7816_CRC_LRC_enable(priv, 1);
            dcd_iso7816_enable_crc(priv, 1);
            dcd_iso7816_set_crc_type(priv, LRC);
            dcd_iso7816_set_error_mask(priv, ISO_7816_CRC_ERR, 1);
        } else if (result->t1_check == LRC) {
            dcd_iso7816_CRC_LRC_enable(priv, 1);
            dcd_iso7816_enable_crc(priv, 1);
            dcd_iso7816_set_crc_type(priv, CRC);
            dcd_iso7816_set_error_mask(priv, ISO_7816_CRC_ERR, 1);
        }
    }

    return 0;
}

static int dcd_card_pps_check(dcd_iso7816_priv_t *priv)
{
    int i = 0;
    while(i < 4) {
        if (priv->atr.rec_pps[i] != priv->atr.pps[i]) {
            return -1;
        }
        i++;
    }

    return 0;
}

static void dcd_error_handle(dcd_iso7816_priv_t *priv)
{
    uint32_t err_mask = dcd_iso7816_error_status(priv);

    while (err_mask) {
        int32_t err_event = -1;
        if (err_mask & ISO_7816_RX_FULL) {
            err_event = ISO7816_EVENT_RX_FULL;
            err_mask &= ~ISO_7816_RX_FULL;
        } else if (err_mask & ISO_7816_CWT_TIM) {
            err_event = ISO7816_EVENT_CWT_TIME_OUT;
            err_mask &= ~ISO_7816_CWT_TIM;
        } else if (err_mask & ISO_7816_RX_OVER) {
            err_event = ISO7816_EVENT_RX_OVER;
            err_mask &= ~ISO_7816_RX_OVER;
        } else if (err_mask & ISO_7816_REP_ERR) {
            if (dcd_iso7816_send_type(priv) == DCD_ISO7816_TX) {
                err_event = ISO7816_EVENT_WRITE_ERROR;
            } else {
                err_event = ISO7816_EVENT_READ_ERROR;
            }
            err_mask &= ~ISO_7816_REP_ERR;
        } else if (err_mask & ISO_7816_CRC_ERR) {
            err_event = ISO7816_EVENT_CRC_ERR;
            err_mask &= ~ISO_7816_REP_ERR;
        } else if (err_mask & ISO_7816_PAR_ERR) {
            err_event = ISO7816_EVENT_PARITY_ERR;
            err_mask &= ~ISO_7816_PAR_ERR;
        }

        priv->tx_trf_cnt = 0;
        priv->rx_recv_cnt = 0;

        if (priv->cb != NULL && err_event!= -1) {
            priv->cb(err_event, priv->cb_arg);
        }
    }

    dcd_iso7816_clear_error(priv);
}

static void dcd_iso7816_card_ev_handle(dcd_iso7816_priv_t *priv)
{
    uint32_t card_status = dcd_iso7816_card_status(priv);
    int32_t event = -1;

    /*if card error*/
    if (card_status & ISO_7816_CRD_ERR_CARD_STA) {
        if (card_status & ISO_7816_SES_END_CARD_STA) {
            event = ISO7816_EVENT_CARD_SESSION_CLOSED;
        } else {
            event = ISO7816_EVENT_CARD_ERROR_DEACTIVATE;
        }

        priv->card_sta = INVCTIVE;
        priv->tx_trf_cnt = 0;
        priv->rx_recv_cnt = 0;
        dcd_iso7816_clear_error(priv);
    } else if ((card_status & ISO_7816_CRD_DET_CARD_STA)){
        event = ISO7816_EVENT_CARD_DETECTED;
    }

    if (priv->cb != NULL && event != -1) {
        priv->cb(event, priv->cb);
    }
}

static void dcd_pps_handle(dcd_iso7816_priv_t *priv)
{
    uint32_t card_event = dcd_iso7816_mstatus(priv);

    /*pps data send end*/
    if(priv->card_sta == PSS_TRF && (card_event & ISO_7816_TX_EM_IRQ)
       && dcd_iso7816_fifo_data_length(priv) == 0) {
        csi_iso7816_master_receive_it(priv->idx, priv->atr.rec_pps, 4);
        priv->card_sta = PSS_RECV;
    } else if (priv->card_sta == PSS_RECV && priv->rx_recv_cnt == 0) {
        int activate_event = -1;
        if (dcd_card_pps_check(priv) == 0) {
            protocol_config(priv, atr_get_result(priv->idx));
            priv->card_sta = ACTIVATE;
            activate_event = ISO7816_EVENT_ACTIVATE_SUCCESS;
        } else {
            priv->card_sta = INVCTIVE;
            activate_event = ISO7816_EVENT_ACTIVATE_FAILED;
        }

        if (priv->cb != NULL) {
            priv->cb(activate_event, priv->cb_arg);
        }
    }
}

void dcd_iso7816_irq_handler(csi_dev_t *dev)
{
    int idx = dev->idx;
    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    uint32_t card_event = dcd_iso7816_mstatus(priv);

    if (card_event & ISO_7816_CRD_EV_IRQ) {
        dcd_iso7816_card_ev_handle(priv);
    }

    if (priv->card_sta == ACTIVATEING) {
        int ret = dcd_card_activate(priv);
        int activate_event = -1;

        if (ret == -1) {
            activate_event = ISO7816_EVENT_ACTIVATE_FAILED;
            priv->card_sta = INVCTIVE;
        } else if (ret == 1) {
            priv->card_sta = PSS_TRF;
        }

        if (activate_event != -1 && priv->cb != NULL) {
            priv->cb(activate_event, priv->cb_arg);
        }

        return;
    }

    if (card_event & ISO_7816_RX_EV_IRQ && dcd_iso7816_send_type(priv) == DCD_ISO7816_RX) {
        dcd_iso7816_recv_it(priv);
    }

    if (card_event & (ISO_7816_TX_EV_IRQ) && dcd_iso7816_send_type(priv) == DCD_ISO7816_TX) {
        dcd_iso7816_trf_it(priv, ISO_7816_TX_EV_IRQ);
    } else if ((card_event & ISO_7816_TX_EM_IRQ) && dcd_iso7816_send_type(priv) == DCD_ISO7816_TX) {
        dcd_iso7816_trf_it(priv, ISO_7816_TX_EM_IRQ);
    }

    dcd_pps_handle(priv);

    if (card_event & ISO_7816_BREAK_IRQ) {
        dcd_iso7816_set_send_type(priv, DCD_ISO7816_RX);
        dcd_iso7816_clear_error(priv);
        if (priv->cb != NULL && dcd_iso7816_send_type(priv) == DCD_ISO7816_TX) {
            priv->cb(ISO7816_EVENT_CARD_ERROR_DEACTIVATE, priv->cb_arg);
        }
    }

    /*error event*/
    if (card_event & ISO_7816_ERR_EV_IRQ) {
        dcd_error_handle(priv);
    }

    dcd_iso7816_check_trf(priv);
}

int32_t csi_iso7816_master_init(int idx, iso7816_event_cb_t cb, void *cb_arg)
{
    int ret = -1;

    if (idx >= CONFIG_ISO7816_NUM || idx < 0) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_clear_initial_irq(priv);

    priv->cb = cb;
    priv->cb_arg = cb_arg;
    priv->card_sta = INVCTIVE;
    priv->rt = RX_MODE;

    dcd_iso7816_set_interrupt_mask(priv, 0xff, 0);
    uint32_t mask = DEF_EVENT_INT_MASK;
    dcd_iso7816_set_interrupt_mask(priv, mask, 1);
    mask = ISO_7816_RX_FULL | ISO_7816_TX_FULL | ISO_7816_RX_OVER | ISO_7816_PAR_ERR;
    dcd_iso7816_set_error_mask(priv, mask, 1);
    dcd_iso7816_enable_rx_fifo(priv, 0);
    dcd_iso7816_enable_tx_fifo(priv, 0);
    dcd_iso7816_interrupt_enable(priv, 1);

    ret = target_get(DEV_DCD_ISO7816_TAG, idx, &priv->dev);

    if (ret == CSI_OK) {
        priv->base = priv->dev.reg_base;
        priv->gsk_irq = priv->dev.irq_num;
        priv->dev.idx = idx;
        priv->idx = idx;
        csi_irq_attach((uint32_t)(priv->dev.irq_num), &dcd_iso7816_irq_handler, &priv->dev);
        csi_irq_enable((uint32_t)(priv->dev.irq_num));
        return 0;
    }

    return -1;
}

int32_t csi_iso7816_master_uninit(int idx)
{
    if (idx >= CONFIG_ISO7816_NUM || idx < 0) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_interrupt_enable(priv, 0);
    csi_irq_detach((uint32_t)(priv->dev.irq_num));
    csi_irq_disable((uint32_t)(priv->dev.irq_num));
    /*to do reset the iso7816 module*/
    return 0;
}

int32_t csi_iso7816_master_config(int idx, iso7816_config_t *config)
{
    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    if (config == NULL || idx >= 1) {
        return -1;
    }

    dcd_iso7816_set_clk_div(priv, config->clk_div);
    dcd_iso7816_set_voltage_class(priv, config->vclass);

    if (config->card_detected_en) {
        dcd_iso7816_enable_card_detect(priv, 1);
        dcd_iso7816_set_interrupt_mask(priv, ISO_7816_CRD_EV_IRQ, 1);
    } else {
        dcd_iso7816_enable_card_detect(priv, 0);
        dcd_iso7816_set_interrupt_mask(priv, ISO_7816_CRD_EV_IRQ, 0);
    }

    dcd_iso7816_set_odd_parity(priv, 0);

    return 0;
}

int32_t csi_iso7816_master_receive(int idx, uint8_t *buf, uint32_t len, uint32_t time_out)
{
    if(idx >= CONFIG_ISO7816_NUM || len == 0 || buf == NULL) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_set_send_type(priv, DCD_ISO7816_RX);
    dcd_iso7816_enable_rx_fifo(priv, 1);
    while(len && dcd_iso7816_send_type(priv) == DCD_ISO7816_RX) {
        uint32_t data_len = dcd_iso7816_fifo_data_length(priv);
        while(data_len && dcd_iso7816_send_type(priv) == DCD_ISO7816_RX) {
            *buf = dcd_iso7816_read_byte(priv);
            len--;
            data_len--;
        }

        if ((dcd_iso7816_card_status(priv) & ISO_7816_CRD_ACT_CARD_STA) == 0) {
            return -1;
        }
    }

    dcd_iso7816_enable_rx_fifo(priv, 0);

    if (len) {
        return -1;
    }

    return 0;
}

int32_t csi_iso7816_master_receive_it(int idx, uint8_t *buf, uint32_t len)
{
    if(idx >= CONFIG_ISO7816_NUM || len == 0 || buf == NULL) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_set_send_type(priv, DCD_ISO7816_RX);
    dcd_iso7816_enable_rx_fifo(priv, 1);

    priv->rx_buf = buf;
    priv->rx_recv_cnt = len;

    if (len >= 7) {
        /*threshold is 7 bytes*/
        dcd_iso7816_set_rx_threshold(priv, 2);
    } else if (len >= 3) {
        /*threshold is 3 bytes*/
        dcd_iso7816_set_rx_threshold(priv, 1);
    }  else {
        /*threshold is 1 bytes*/
        dcd_iso7816_set_rx_threshold(priv, 0);
    }

    dcd_iso7816_set_interrupt_mask(priv, ISO_7816_RX_EV_IRQ, 1);
    return 0;
}

int32_t csi_iso7816_master_send(int idx, uint8_t *buf, uint32_t len, uint32_t time_out)
{
    if(idx >= CONFIG_ISO7816_NUM || len == 0 || buf == NULL) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_set_send_type(priv, DCD_ISO7816_TX);
    dcd_iso7816_enable_tx_fifo(priv, 1);

    uint32_t fifo_avail = 0;

    while(len && dcd_iso7816_send_type(priv) == DCD_ISO7816_TX) {
        while(fifo_avail == 0 && dcd_iso7816_send_type(priv) == DCD_ISO7816_TX) {
            fifo_avail = ISO7816_TX_FIFO_SIZE - dcd_iso7816_fifo_data_length(priv);
        }

        while(fifo_avail && len && dcd_iso7816_send_type(priv) == DCD_ISO7816_TX) {
            dcd_iso7816_write_byte(priv, *buf);
            buf++;
            len--;
            fifo_avail--;
        }

        if ((dcd_iso7816_card_status(priv) & ISO_7816_CRD_ACT_CARD_STA) == 0) {
            return -1;
        }
    }

    while((dcd_iso7816_mstatus(priv) & ISO_7816_TX_EM_IRQ) == 0);

    //dcd_iso7816_set_send_type(priv, DCD_ISO7816_RX);
    dcd_iso7816_enable_tx_fifo(priv, 0);

    if (len != 0) {
        return -1;
    }

    return 0;
}

int32_t csi_iso7816_master_send_it(int idx, uint8_t *buf, uint32_t len)
{
    if(idx >= CONFIG_ISO7816_NUM || len == 0 || buf == NULL) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];

    priv->tx_buf = buf;
    priv->tx_trf_cnt = len;
    dcd_iso7816_set_send_type(priv, DCD_ISO7816_TX);
    dcd_iso7816_enable_tx_fifo(priv, 1);

    if (len >= 8) {
        dcd_iso7816_set_tx_threshold(priv, 2);
    } else if (len >= 4) {
        dcd_iso7816_set_tx_threshold(priv, 1);
    } else {
        dcd_iso7816_set_tx_threshold(priv, 0);
    }

    dcd_iso7816_set_interrupt_mask(priv, ISO_7816_TX_EV_IRQ | ISO_7816_TX_EM_IRQ, 1);

    return 0;
}

int32_t csi_iso7816_master_card_activate(int idx)
{
    if(idx >= CONFIG_ISO7816_NUM) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_enable_rx_fifo(priv, 1);
    dcd_iso7816_set_send_type(priv, DCD_ISO7816_RX);
    dcd_iso7816_set_protocol(priv, T0_MODE);
    dcd_iso7816_set_wt(priv, DEFAULT_WT);
    atr_result_t *result = atr_get_result(priv->idx);
    memset(result, 0, sizeof(atr_result_t));
    result->cwi = -1;
    result->bwi = -1;
    result->t1_check = -1;

    uint32_t mask = ISO_7816_CWT_TIM | ISO_7816_REP_ERR | ISO_7816_CRC_ERR;
    dcd_iso7816_set_error_mask(priv, mask, 0);
    priv->atr.atr_sta = T0;
    priv->card_sta = ACTIVATEING;

    dcd_iso7816_start(priv);
    csi_iso7816_master_receive_it(idx, priv->atr.buf, 1);

    return 0;
}

int32_t csi_iso7816_master_card_deactivate(int idx)
{
    if(idx >= CONFIG_ISO7816_NUM) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_stop(priv);
    return 0;
}

iso7816_card_sta_t csi_iso7816_master_card_status(int idx)
{
    if(idx >= CONFIG_ISO7816_NUM) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    uint32_t card_status = dcd_iso7816_card_status(priv);

    if ((card_status & ISO_7816_CRD_ACT_CARD_STA) == 0) {
        priv->card_sta = INVCTIVE;
    }

    return priv->card_sta;
}

int32_t csi_iso7816_master_card_warm_reset(int idx)
{
    if(idx >= CONFIG_ISO7816_NUM) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_warm_reset(priv);

    return 0;
}

int32_t csi_iso7816_master_card_clk_stop_enable(int idx, int en)
{
    if(idx >= CONFIG_ISO7816_NUM) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_clock_stop(priv, en);

    return 0;
}

int32_t csi_iso7816_master_power_down(int idx)
{
    if(idx >= CONFIG_ISO7816_NUM) {
        return -1;
    }

    dcd_iso7816_priv_t *priv = &dcd_iso7816_instance[idx];
    dcd_iso7816_power_down(priv);

    return 0;
}

int32_t csi_iso7816_master_atr_info(int idx, iso7816_atr_info_t *info)
{
    if(idx >= CONFIG_ISO7816_NUM) {
        return -1;
    }

    atr_result_t *result = atr_get_result(idx);
    info->clk_stop_is_support = result->clk_stop_type ? 1 : 0;
    info->support_voltage_class = result->support_voltage_class;
    info->history_byte_num = result->history_byte_num;
    info->proto_t = result->T;

    memcpy(info->history_data, result->history_data, 15);

    return 0;
}
