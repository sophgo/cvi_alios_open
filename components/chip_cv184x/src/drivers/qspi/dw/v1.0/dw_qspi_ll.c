/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     dw_qspi_ll.c
 * @brief
 * @version
 * @date     2020-02-10
 ******************************************************************************/


#include "dw_qspi_ll.h"

void dw_qspi_config_sclk_clock(dw_qspi_regs_t *qspi_base, uint32_t clock_in, uint32_t clock_out)
{
    uint32_t div;

    div = clock_in / clock_out;
    div = (div < 2U) ? 2U : div;
    div = (div > 65534U) ? 65534U : div;
    /* div must be even and round up */
    div =(div+1)&(~0x1);
    udelay(2);
    qspi_base->BAUDR &= DW_QSPI_BAUDR_SCKDV_Msk;
    qspi_base->BAUDR = div;
}

uint32_t dw_qspi_get_sclk_clock_div(dw_qspi_regs_t *qspi_base)
{
    return qspi_base->BAUDR;
}

uint32_t dw_qspi_get_data_frame_len(dw_qspi_regs_t *qspi_base)
{
    uint32_t len = qspi_base->CTRLR0 & DW_QSPI_CTRLR0_DFS32_Msk;
    len >>= DW_QSPI_CTRLR0_DFS32_Pos;
    len++;
    return len;
}

void dw_qspi_config_data_frame_len(dw_qspi_regs_t *qspi_base, uint32_t size)
{
    uint32_t temp;

    if ((size >= 4U) & (size <= 32U)) {
        temp = qspi_base->CTRLR0;
        temp &= ~DW_QSPI_CTRLR0_DFS32_Msk;
        temp |= ((size - 1U) << DW_QSPI_CTRLR0_DFS32_Pos);
        qspi_base->CTRLR0 = temp;
    }
}

void dw_qspi_reset_regs(dw_qspi_regs_t *qspi_base)
{
    qspi_base->CTRLR0   = 7U;
    qspi_base->CTRLR1   = 0U;
    qspi_base->SSIENR   = 0U;
    qspi_base->SER      = 0U;
    qspi_base->BAUDR    = 0U;
    qspi_base->TXFTLR   = 0U;
    qspi_base->RXFTLR   = 0U;
    qspi_base->IMR      = 0U;
}

void dw_qspi_config_enhanced_transfer(dw_qspi_regs_t *qspi_base,dw_qspi_xfer_enhance_cfg_t * cfg)
{
    /* the following operation need disable spi coare */
    
    /* set spi_frf = single|dual|quad|octal */
    dw_qspi_set_qspi_frame_fmt(qspi_base,cfg->spi_prf);

    /* set instruct len */
    dw_qspi_set_inst_len(qspi_base,cfg->inst_len);

    /* set addr len */
    dw_qspi_set_addr_len(qspi_base,cfg->addr_len);

    /* set dummy cycles */
    dw_qspi_set_wait_cycles(qspi_base,cfg->dummy_len);

    /* set enhanced xfer mode */
    dw_qspi_set_trans_type(qspi_base,cfg->mode);

}
