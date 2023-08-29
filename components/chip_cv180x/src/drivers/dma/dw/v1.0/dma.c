/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     dw_dma.c
 * @brief    CSI Source File for uart Driver
 * @version  V2.01
 * @date     16. Mar 2020
 * @vendor   csky
 * @name     dw_dma
 * @ip_id    0x111000010
 * @model    dma
 * @tag      DEV_DW_DMA_TAG
 ******************************************************************************/

#include <drv/dma.h>
#include <drv/cvi_irq.h>
#include <csi_core.h>
#include "dw_dma_ll.h"

#define  REGS_OFFSET  0x58
extern uint8_t g_dma_chnum[];
static csi_dma_t *dma_array[2];
static uint32_t inited_ctrl_num = 0U;

static void ch_list_add(csi_dma_t *dma, csi_dma_ch_t *dma_ch)
{
    csi_dma_ch_t *tmp;

    if (dma->ch_list == NULL) {
        dma->ch_list = dma_ch;
    } else {
        tmp = dma->ch_list;

        while (tmp->next != NULL) {
            tmp = tmp->next;
        }

        tmp->next = dma_ch;
    }
}

static void ch_list_delete(csi_dma_t *dma, csi_dma_ch_t *dma_ch)
{
    csi_dma_ch_t *tmp, *tmp_next;

    if (dma->ch_list == dma_ch) {
        dma->ch_list = dma->ch_list->next;
    } else {
        tmp = dma->ch_list;
        tmp_next = dma->ch_list->next;

        do {
            if (tmp_next == dma_ch) {
                tmp->next = tmp_next->next;
                break;
            }

            tmp = tmp_next;
            tmp_next = tmp_next->next;
        } while (tmp_next != NULL);
    }
}

void dw_dma_clear_int_dma(dw_dma_regs_t *dma_addr, uint8_t ch_id, uint32_t  value)
{
    switch (ch_id) {
        case 0U:
            dw_dma_set_cleartfr_dma0(dma_addr, value);
            dw_dma_set_clearblock_dma0(dma_addr, value);
            dw_dma_set_clearsrctran_dma0(dma_addr, value);
            dw_dma_set_cleardsttran_dma0(dma_addr, value);
            dw_dma_set_clearerr_dma0(dma_addr, value);
            break;

        case 1U:
            dw_dma_set_cleartfr_dma1(dma_addr, value);
            dw_dma_set_clearblock_dma1(dma_addr, value);
            dw_dma_set_clearsrctran_dma1(dma_addr, value);
            dw_dma_set_cleardsttran_dma1(dma_addr, value);
            dw_dma_set_clearerr_dma1(dma_addr, value);
            break;
    }

}

void dw_dma_irq_handler(void *arg)
{
    csi_dma_t *dma = (csi_dma_t *)arg;
    dw_dma_regs_t *dma_addr = (dw_dma_regs_t *)dma->dev.reg_base;
    csi_dma_ch_t *dma_ch;
    uint32_t int_status, cb_state, ch_index;
    uint32_t (*func_block)(dw_dma_regs_t *dma);
    uint32_t (*func_err)(dw_dma_regs_t *dma);
    dma_ch = dma->ch_list;

    if (dma_ch->ctrl_id == 0) {

        func_block = dw_dma_get_statusblock_dma0;
        func_err = dw_dma_get_statuserr_dma0;
        ch_index = 0U;
    } else {
        func_block = dw_dma_get_statusblock_dma1;
        func_err = dw_dma_get_statuserr_dma1;
        ch_index = 1U;
    }

    int_status = func_block(dma_addr);
    cb_state = 0;

    if (int_status != 0) {
        cb_state = DMA_EVENT_TRANSFER_DONE;

    } else {
        int_status = func_err(dma_addr);

        if (int_status != 0U) {
            cb_state = DMA_EVENT_TRANSFER_ERROR;
        }
    }

    if (int_status != ~0U) {

        do {
            if (((uint32_t)dma_ch->ch_id + 1U) & int_status) {
                if (dma_ch->callback != NULL) {
                    dw_dma_clear_int_dma(dma_addr, ch_index, ((uint32_t)dma_ch->ch_id + 1U));
                    dma_ch->callback(dma_ch, cb_state, dma_ch->arg);
                }
            }

            dma_ch = dma_ch->next;
        } while (dma_ch != NULL);
    }

}

csi_error_t csi_dma_init(csi_dma_t *dma, int8_t ctrl_id)
{
    CSI_PARAM_CHK(dma, CSI_ERROR);
    int32_t ret;
    dw_dma_regs_t *dma_addr;
    ret = target_get(DEV_DW_DMA_TAG, ctrl_id, &dma->dev);

    if (ret != 0U) {
        return CSI_ERROR;
    }

    dma->ch_list = NULL;
    dma->alloc_status = 0U;
    dma_array[ctrl_id] = dma;
    dma_array[ctrl_id]->ch_num = g_dma_chnum[ctrl_id];
    dma_addr = (dw_dma_regs_t *)(dma_array[ctrl_id]->dev.reg_base);

    csi_irq_attach((uint32_t)dma->dev.irq_num, &dw_dma_irq_handler, &dma->dev);
    csi_irq_enable((uint32_t)dma->dev.irq_num);

    //enable dma module
    dw_dma_en(dma_addr);
    uint32_t result = csi_irq_save();
    inited_ctrl_num++;
    csi_irq_restore(result);
    return CSI_OK;
}

void csi_dma_uninit(csi_dma_t *dma)
{
    CSI_PARAM_CHK_NORETVAL(dma);
    dw_dma_regs_t *dma_addr;
    dma->ch_list = NULL;
    dma->alloc_status = 0U;
    dma_addr = (dw_dma_regs_t *)(dma_array[dma->dev.idx]->dev.reg_base);
    dma_array[dma->dev.idx] = NULL;
    //dma_set_en(dma_addr, 0U);
    dw_dma_dis(dma_addr);
    csi_irq_disable((uint32_t)dma->dev.irq_num);
    csi_irq_detach((uint32_t)dma->dev.irq_num);
    uint32_t result = csi_irq_save();
    inited_ctrl_num--;
    csi_irq_restore(result);
}

csi_error_t csi_dma_ch_alloc(csi_dma_ch_t *dma_ch, int8_t ch_id, int8_t ctrl_id)
{
    CSI_PARAM_CHK(dma_ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_dma_ch_desc_t ch_info;
    csi_dev_t *dev_info;
    dw_dma_regs_t *dma_addr;
    dev_info = (csi_dev_t *)(dma_ch->parent);
    uint32_t result = csi_irq_save();

    if ((ctrl_id == -1U) && (ch_id == -1U)) {
        ret = target_get_optimal_dma_channel(dma_array, inited_ctrl_num, dev_info, &ch_info);

        if (ret == CSI_OK) {
            dma_array[ch_info.ctrl_idx]->alloc_status |= (1U << ch_info.ch_idx);

            dma_ch->ch_id = ch_info.ch_idx;
            dma_ch->ctrl_id = ch_info.ctrl_idx;
        } else {
            ret = CSI_ERROR;
        }
    } else if ((ctrl_id >= 0U) && (ch_id >= 0U)) {
        if (dma_array[ctrl_id]->alloc_status & (1U << ch_id)) {
            ret = CSI_ERROR;
        } else {
            dma_array[ctrl_id]->alloc_status |= (1U << ch_id);
            dma_ch->ch_id = ch_id;
            dma_ch->ctrl_id = ctrl_id;
        }
    } else {
        ret = CSI_ERROR;
    }

    if (ret != CSI_ERROR) {
        dma_ch->next = NULL;
        ch_list_add(dma_array[dma_ch->ctrl_id], dma_ch);

        //disable the channel
        dma_addr = (dw_dma_regs_t *)(dma_array[dma_ch->ctrl_id]->dev.reg_base);
        dw_dma_reset_channel_en(dma_addr, (DW_DMA_ChEnReg_CH_EN_En_CH0 + dma_ch->ch_id));
    }

    csi_irq_restore(result);

    return ret;
}

void csi_dma_ch_free(csi_dma_ch_t *dma_ch)
{
    CSI_PARAM_CHK_NORETVAL(dma_ch);
    uint32_t result = csi_irq_save();
    uint32_t temp_u32;
    temp_u32 = 1U << (uint8_t)(dma_ch->ch_id);

    if (dma_array[dma_ch->ctrl_id]->alloc_status & temp_u32) {
        dma_array[dma_ch->ctrl_id]->alloc_status &= ~(1U << dma_ch->ch_id);
        ch_list_delete(dma_array[dma_ch->ctrl_id], dma_ch);
    }

    csi_irq_restore(result);
}

csi_error_t csi_dma_ch_config(csi_dma_ch_t *dma_ch, csi_dma_ch_config_t *config)
{
    CSI_PARAM_CHK(dma_ch, CSI_ERROR);
    int32_t ret = CSI_OK;//, etb_ch;
    // csi_etb_config_t etb_config;
    dw_dma_ch_regs_t *dma_ch_addr = (dw_dma_ch_regs_t *)((dma_ch->ch_id * REGS_OFFSET) + dma_array[dma_ch->ctrl_id]->dev.reg_base);

    /* Initializes corresponding channel registers */
    //dw_dma_set_transferwidth(dma_ch_addr, config->src_tw, config->dst_tw);
    switch (config->dst_tw) {
        case DMA_DATA_WIDTH_8_BITS:
            dw_dma_set_dst_transfer_width(dma_ch_addr, DW_DMA_CTL_DST_TR_WIDTH_8);
            break;

        case DMA_DATA_WIDTH_16_BITS:
            dw_dma_set_dst_transfer_width(dma_ch_addr, DW_DMA_CTL_DST_TR_WIDTH_16);
            break;

        case DMA_DATA_WIDTH_32_BITS:
            dw_dma_set_dst_transfer_width(dma_ch_addr, DW_DMA_CTL_DST_TR_WIDTH_32);
            break;

        default:
            break;
    }

    switch (config->src_tw) {
        case DMA_DATA_WIDTH_8_BITS:
            dw_dma_set_src_transfer_width(dma_ch_addr, DW_DMA_CTL_SRT_TR_WIDTH_8);
            break;

        case DMA_DATA_WIDTH_16_BITS:
            dw_dma_set_src_transfer_width(dma_ch_addr, DW_DMA_CTL_SRT_TR_WIDTH_16);
            break;

        case DMA_DATA_WIDTH_32_BITS:
            dw_dma_set_src_transfer_width(dma_ch_addr, DW_DMA_CTL_SRT_TR_WIDTH_32);
            break;

        default:
            break;
    }

    dw_dma_set_sinc(dma_ch_addr, config->src_inc);
    dw_dma_set_dinc(dma_ch_addr, config->dst_inc);
    // config->group_len
    dw_dma_set_dest_burst_size(dma_ch_addr, DW_DMA_CTL_DEST_MSIZE_1);
    dw_dma_set_src_burst_size(dma_ch_addr, DW_DMA_CTL_SRC_MSIZE_1);
    /* set little endian */
    //not support
    // dw_dma_set_addr_endian(dma_ch_addr, 0U, 0U);

    switch (config->trans_dir) {
        case DMA_MEM2MEM:
            dw_dma_set_transfer_type(dma_ch_addr, DW_DMA_CTL_TT_FC_M2M);
            dw_dma_dst_hs_sft(dma_ch_addr);
            dw_dma_dst_hs_sft(dma_ch_addr);
            break;

        case DMA_MEM2PERH:
            dw_dma_set_transfer_type(dma_ch_addr, DW_DMA_CTL_TT_FC_M2P);
            dw_dma_dst_hs_hw(dma_ch_addr);
            dw_dma_dst_hs_sft(dma_ch_addr);
            dw_dma_set_src_per(dma_ch_addr, 10);
            break;

        case DMA_PERH2MEM:
            dw_dma_set_transfer_type(dma_ch_addr, DW_DMA_CTL_TT_FC_P2M);
            break;

        default:
            break;
    }

    //Don't suspend dma
    dw_dma_dis_ch_susp(dma_ch_addr);

    return ret;
}

csi_error_t csi_dma_ch_attach_callback(csi_dma_ch_t *dma_ch, void *callback, void *arg)
{
    csi_error_t ret = CSI_OK;
    CSI_PARAM_CHK(dma_ch, CSI_ERROR);
    dw_dma_ch_regs_t *dma_ch_addr = (dw_dma_ch_regs_t *)((dma_ch->ch_id * REGS_OFFSET) + dma_array[dma_ch->ctrl_id]->dev.reg_base);
    dw_dma_regs_t *dma_addr;
    dma_ch->callback = callback;
    dma_ch->arg = arg;

    if (dma_ch->ctrl_id == 0U) {
        dma_addr = (dw_dma_regs_t *)(dma_array[dma_ch->ctrl_id]->dev.reg_base);

        //open mask
        dw_dma_set_masktfr_dma0(dma_addr, (DW_DMA_MaskTfr_DMA0_En_CH0 << dma_ch->ctrl_id));
        dw_dma_set_maskblock_dma0(dma_addr, (DW_DMA_MaskBlock_DMA0_En_CH0 << dma_ch->ctrl_id));
        dw_dma_set_masksrctran_dma0(dma_addr, (DW_DMA_MaskSrcTran_DMA0_En_CH0 << dma_ch->ctrl_id));
        dw_dma_set_maskdsttran_dma0(dma_addr, (DW_DMA_MaskDstTran_DMA0_En_CH0 << dma_ch->ctrl_id));
        dw_dma_set_maskerr_dma0(dma_addr, (DW_DMA_MaskErr_DMA0_En_CH0 << dma_ch->ctrl_id));

        //clear interrupt flag
        dw_dma_clear_int_dma(dma_addr, 0U, (DW_DMA_MaskTfr_DMA0_En_CH0 << dma_ch->ctrl_id));

    } else if (dma_ch->ctrl_id == 1U) {
        dma_addr = (dw_dma_regs_t *)(dma_array[dma_ch->ctrl_id]->dev.reg_base);
        //open mask
        dw_dma_set_masktfr_dma1(dma_addr, (DW_DMA_MaskTfr_DMA1_En_CH0 << dma_ch->ctrl_id));
        dw_dma_set_maskblock_dma1(dma_addr, (DW_DMA_MaskBlock_DMA1_En_CH0 << dma_ch->ctrl_id));
        dw_dma_set_masksrctran_dma1(dma_addr, (DW_DMA_MaskSrcTran_DMA1_En_CH0 << dma_ch->ctrl_id));
        dw_dma_set_maskdsttran_dma1(dma_addr, (DW_DMA_MaskDstTran_DMA1_En_CH0 << dma_ch->ctrl_id));
        dw_dma_set_maskerr_dma1(dma_addr, (DW_DMA_MaskErr_DMA1_En_CH0 << dma_ch->ctrl_id));

        //clear interrupt flag
        dw_dma_clear_int_dma(dma_addr, 1U, (DW_DMA_MaskTfr_DMA1_En_CH0 << dma_ch->ctrl_id));

    } else {
        ret = CSI_ERROR;
    }

    if (ret == CSI_OK) {
        dw_dma_en_int(dma_ch_addr);
    }

    return ret;
}

void csi_dma_ch_detach_callback(csi_dma_ch_t *dma_ch)
{
    dw_dma_regs_t *dma_addr;
    CSI_PARAM_CHK_NORETVAL(dma_ch);
    dw_dma_ch_regs_t *dma_ch_addr = (dw_dma_ch_regs_t *)((dma_ch->ch_id * REGS_OFFSET) + dma_array[dma_ch->ctrl_id]->dev.reg_base);
    dma_ch->callback = NULL;
    dma_ch->arg = NULL;

    if (dma_ch->ctrl_id == 0U) {
        dma_addr = (dw_dma_regs_t *)(dma_array[dma_ch->ctrl_id]->dev.reg_base);
        //open mask
        dw_dma_reset_masktfr_dma0(dma_addr, (DW_DMA_MaskTfr_DMA0_En_CH0 << dma_ch->ctrl_id));
        dw_dma_reset_maskblock_dma0(dma_addr, (DW_DMA_MaskBlock_DMA0_En_CH0 << dma_ch->ctrl_id));
        dw_dma_reset_maskerr_dma0(dma_addr, (DW_DMA_MaskErr_DMA0_En_CH0 << dma_ch->ctrl_id));

        //clear interrupt flag
        dw_dma_clear_int_dma(dma_addr, 0U, (DW_DMA_MaskTfr_DMA0_En_CH0));
    } else if (dma_ch->ctrl_id == 1U) {
        dma_addr = (dw_dma_regs_t *)(dma_array[dma_ch->ctrl_id]->dev.reg_base);
        //open mask
        dw_dma_reset_masktfr_dma1(dma_addr, (DW_DMA_MaskTfr_DMA1_En_CH0 << dma_ch->ctrl_id));
        dw_dma_reset_maskblock_dma1(dma_addr, (DW_DMA_MaskBlock_DMA1_En_CH0 << dma_ch->ctrl_id));
        dw_dma_reset_maskerr_dma1(dma_addr, (DW_DMA_MaskErr_DMA1_En_CH0 << dma_ch->ctrl_id));

        //clear interrupt flag
        dw_dma_clear_int_dma(dma_addr, 1U, (DW_DMA_MaskTfr_DMA1_En_CH0 << dma_ch->ctrl_id));

    }

    dw_dma_dis_int(dma_ch_addr);
}
void delay_x(uint32_t value)
{
    for (uint32_t i = 0U; i < value; i++) {
        printf("%d", i);
    }
}

void csi_dma_ch_start(csi_dma_ch_t *dma_ch, void *srcaddr, void *dstaddr, uint32_t length)
{
    dw_dma_regs_t *dma_addr;
    CSI_PARAM_CHK_NORETVAL(dma_ch);

    dw_dma_ch_regs_t *dma_ch_addr = (dw_dma_ch_regs_t *)((dma_ch->ch_id * REGS_OFFSET) + dma_array[dma_ch->ctrl_id]->dev.reg_base);
    dma_addr = (dw_dma_regs_t *)(dma_array[dma_ch->ctrl_id]->dev.reg_base);

    dw_dma_set_transfer_size(dma_ch_addr, length);



    if (srcaddr != NULL) {
        dw_dma_set_sar(dma_ch_addr, (uint32_t)srcaddr);
        csi_dcache_invalid_range((uint32_t *)srcaddr, length);
    }

    if (dstaddr != NULL) {
        dw_dma_set_dar(dma_ch_addr, (uint32_t)dstaddr);
        csi_dcache_invalid_range((uint32_t *)dstaddr, length);
    }

    //enable channel
    dw_dma_set_channel_en(dma_addr, (DW_DMA_ChEnReg_CH_EN_En_CH0 << dma_ch->ch_id));

}

void csi_dma_ch_stop(csi_dma_ch_t *dma_ch)
{
    dw_dma_regs_t *dma_addr;
    CSI_PARAM_CHK_NORETVAL(dma_ch);
    dma_addr = (dw_dma_regs_t *)(dma_array[dma_ch->ctrl_id]->dev.reg_base);
    dw_dma_reset_channel_en(dma_addr, DW_DMA_ChEnReg_CH_EN_En_CH0 << dma_ch->ch_id);

}
