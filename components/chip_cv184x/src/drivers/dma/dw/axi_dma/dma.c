/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     dma.c
 * @brief    dw dma axi driver
 * @version  v1.0
 * @date     2020-11-06
 ******************************************************************************/

#include "stdint.h"
#include "math.h"
#include "drv/dma.h"
#include <drv/common.h>
#include <drv/cvi_irq.h>
#include <stdint.h>

#ifdef CONFIG_DW_AXI_DMA_32CH_NUM_CHANNELS
#include "dw_axi_dma_32ch_ll.h"
#else
#include "dw_axi_dma_8ch_ll.h"
#endif

extern const uint8_t g_dma_chnum[];
static dma_rec_t dma_init_record;

static uint32_t get_power(uint32_t base, uint32_t power)
{
    uint32_t ret, i;
    ret =  1U;

    for (i = 0U; i < power; i++) {
        ret *= base;
    }

    return ret;
}

static csi_error_t dw_axi_dma_lli_chain_into_loop(csi_dma_ch_config_t *config)
{
    CSI_PARAM_CHK(config, CSI_ERROR);
    uint8_t *temp_addr;
    dw_dma_axi_lli_t *lli_addr;
    uint32_t group_len = config->group_len;
    temp_addr = (uint8_t *)config->cyclic_config.lli_buf;
    uint32_t period = config->cyclic_config.buffer_bytes / config->cyclic_config.period_bytes;

    /* copy config data into loop data */
    for (uint32_t i = 0U; i < period; i++) {
        lli_addr = (dw_dma_axi_lli_t *)temp_addr;
        lli_addr->LLP = (uint64_t)((uint8_t *)lli_addr + sizeof(dw_dma_axi_lli_t));

        if (config->trans_dir == DMA_MEM2PERH) {
            lli_addr->SAR = ((uint64_t)config->cyclic_config.buffer + (uint64_t)config->cyclic_config.period_bytes * i);
            lli_addr->DAR = (uint64_t)config->cyclic_config.dstaddr;
        } else {
            lli_addr->SAR = (uint64_t)config->cyclic_config.srcaddr;
            lli_addr->DAR = ((uint64_t)config->cyclic_config.buffer + (uint64_t)config->cyclic_config.period_bytes * i);
        }

        uint32_t length = config->cyclic_config.period_bytes;

        uint32_t dma_width_to_bytes = get_power(2U, (uint32_t)(config->src_tw));///< change dma width value to byte

        if (length % dma_width_to_bytes) {
            return CSI_ERROR;
        }

        length /= dma_width_to_bytes;

        if (length <= 0) {
            return CSI_ERROR;
        }

        dw_dma_lli_set_block_ts(lli_addr, length);

        if (config->dst_inc == DMA_ADDR_INC) {
            dw_dma_lli_set_dst_inc(lli_addr);
        } else if (config->dst_inc == DMA_ADDR_CONSTANT) {
            dw_dma_lli_set_dst_no_change(lli_addr);
        }

        if (config->src_inc == DMA_ADDR_INC) {
            dw_dma_lli_set_src_inc(lli_addr);
        } else if (config->src_inc == DMA_ADDR_CONSTANT) {
            dw_dma_lli_set_src_no_change(lli_addr);
        }

        dw_dma_lli_set_src_width(lli_addr, config->src_tw);
        dw_dma_lli_set_dst_width(lli_addr, config->dst_tw);

        group_len /= get_power(2U, (uint32_t)config->dst_tw);

        if (group_len > 1U) {
            dw_dma_lli_enable_arlen(lli_addr);
            dw_dma_lli_set_arlen(lli_addr, group_len);
            dw_dma_lli_enable_awlen(lli_addr);
            dw_dma_lli_set_awlen(lli_addr, group_len);
        } else {
            dw_dma_lli_disable_arlen(lli_addr);
            dw_dma_lli_disable_awlen(lli_addr);
        }

        switch (group_len) {
            case 1U:
                dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_1);
                dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_1);
                break;

            case 4U:
                dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_4);
                dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_4);
                break;

            case 8U:
                dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_8);
                dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_8);
                break;

            case 16U:
                dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_16);
                dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_16);
                break;

            case 32U:
                dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_32);
                dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_32);
                break;

            case 64U:
                dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_64);
                dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_64);
                break;

            case 128U:
                dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_128);
                dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_128);
                break;

            default:
                dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_1);
                dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_1);
                break;
        }

        dw_dma_lli_enable_block_int(lli_addr, true);
        dw_dma_lli_data_is_valid(lli_addr);

        temp_addr += sizeof(dw_dma_axi_lli_t);
    }

    /* chain into loop */
    temp_addr = (uint8_t *)config->cyclic_config.lli_buf;

    for (uint32_t i = 0U; i < (period - 1U); i++) {
        lli_addr = (dw_dma_axi_lli_t *)temp_addr;
        lli_addr->LLP = (uint64_t)((uint8_t *)lli_addr + sizeof(dw_dma_axi_lli_t));
        temp_addr += sizeof(dw_dma_axi_lli_t);
    }

    lli_addr = (dw_dma_axi_lli_t *)temp_addr;
    lli_addr->LLP = (uint64_t)config->cyclic_config.lli_buf;
    return CSI_OK;
}

static void dw_dma_axi_irq_handler(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    csi_dma_t *dma = (csi_dma_t *)arg;
    CSI_PARAM_CHK_NORETVAL(dma);
    dw_dma_axi_reg_t *dma_base = (void *)dma->dev.reg_base;

    uint64_t dma_int_status = dw_dma_get_ch_irq_status(dma_base);

    if (dma_int_status & DW_AXI_DMAC_INTSTATUSREG_COMMON_REG) {
        uint64_t mask = dw_dma_get_error_irq_status(dma_base);
        printf("error mask %lx\n", mask);
        dw_dma_error_irq_clear(dma_base, mask);

        while (1);
    }

    csi_dma_ch_t *dma_ch;
    slist_for_each_entry(&dma->head, dma_ch, csi_dma_ch_t, next) {
        if ((1 << dma_ch->ch_id) & dma_int_status) {
            dw_dma_axi_ch_reg_t *dma_ch_base = (dw_dma_axi_ch_reg_t *)&dma_base->CH[dma_ch->ch_id];
            uint64_t ch_int_status = dw_dma_ch_irq_status(dma_ch_base);
            dw_dma_ch_clear_irq(dma_ch_base, ch_int_status);

            csi_dma_event_t event = DMA_EVENT_TRANSFER_ERROR;

            if (ch_int_status & DW_AXI_DMAC_CH_INT_EN_DMA_TFR_DONE) {
                event = DMA_EVENT_TRANSFER_DONE;

                if (dma_ch->callback != NULL) {
                    dma_ch->callback(dma_ch, event, dma_ch->arg);
                }
            }

            if (ch_int_status & DW_AXI_DMAC_CH_INT_EN_BLOCK_TFR_DONE) {
                event = DMA_EVENT_TRANSFER_BLOCK_DONE;

                if (dma_ch->callback != NULL) {
                    dma_ch->callback(dma_ch, event, dma_ch->arg);
                }
            }

        }
    }
}
csi_error_t csi_dma_init(csi_dma_t *dma, int8_t ctrl_id)
{
    CSI_PARAM_CHK(dma, CSI_ERROR);
    csi_error_t ret = target_get(DEV_DW_DMA_TAG, (uint32_t) ctrl_id, &dma->dev);

    if (ret != CSI_OK) {
        return CSI_ERROR;
    }

    dma_init_record.table[ctrl_id] = dma;
    dma_init_record.init_num++;
    dma->dev.irq_handler = NULL;
    slist_init(&dma->head);
    dma->ch_num = g_dma_chnum[ctrl_id];
    dw_dma_axi_reg_t *axi_dma_base;
    axi_dma_base = (dw_dma_axi_reg_t *)HANDLE_REG_BASE(dma);
    dw_dma_reset(axi_dma_base);
    dw_dma_irq_enable(axi_dma_base);
    dw_dma_enable(axi_dma_base);

    csi_irq_attach((uint32_t)(dma->dev.irq_num), &dw_dma_axi_irq_handler, &dma->dev);
    csi_irq_enable((uint32_t)(dma->dev.irq_num));

    return CSI_OK;
}

void csi_dma_uninit(csi_dma_t *dma)
{
    CSI_PARAM_CHK_NORETVAL(dma);
    dw_dma_axi_reg_t *axi_dma_base;
    axi_dma_base = (dw_dma_axi_reg_t *)HANDLE_REG_BASE(dma);
    slist_init(&dma->head);
    dw_dma_irq_disable(axi_dma_base);
    dw_dma_disable(axi_dma_base);
    csi_irq_detach((uint32_t)dma->dev.irq_num);
    csi_irq_disable((uint32_t)dma->dev.irq_num);
    dma_init_record.table[dma->dev.idx] = NULL;
    dma_init_record.init_num--;
}

csi_error_t csi_dma_ch_alloc(csi_dma_ch_t *dma_ch, int8_t ch_id, int8_t ctrl_id)
{
    CSI_PARAM_CHK(dma_ch, CSI_ERROR);
    csi_dev_t *dma_dev = dma_ch->parent;
    csi_dma_ch_desc_t ch_info;
    csi_error_t ret = CSI_ERROR;

    if (ch_id == -1 && ctrl_id == -1) {
        ret = target_get_optimal_dma_channel(dma_init_record.table,
                                             dma_init_record.init_num,
                                             dma_dev, &ch_info);

        if (ret == CSI_OK) {
            dma_ch->ch_id = ch_info.ch_idx;
            dma_ch->ctrl_id = ch_info.ctrl_idx;
            dma_init_record.table[ch_info.ctrl_idx]->alloc_status |= 1 << ch_info.ch_idx;
            ret = CSI_OK;
        } else {
            ret = CSI_ERROR;
        }
    } else {
        if (dma_init_record.table[ctrl_id]->alloc_status & (1 << ch_id)) {
            ret = CSI_ERROR;
        } else {
            dma_ch->ch_id = ch_id;
            dma_ch->ctrl_id = ctrl_id;
            dma_init_record.table[ctrl_id]->alloc_status |= 1 << ch_id;
            ret = CSI_OK;
        }
    }

    if (ret == CSI_OK) {
        dma_ch->cyclic_state = DMA_CYCLIC_STOP;
        dma_ch->link_list_state = DMA_LINK_LIST_STOP;
        slist_add(&dma_ch->next, &dma_init_record.table[dma_ch->ctrl_id]->head);
    }

    return ret;
}

void csi_dma_ch_free(csi_dma_ch_t *dma_ch)
{
    CSI_PARAM_CHK_NORETVAL(dma_ch);
    uint32_t temp_u32;
    temp_u32 = 1U << (uint8_t)(dma_ch->ch_id);

    if (dma_init_record.table[dma_ch->ctrl_id]->alloc_status & temp_u32) {
        dma_init_record.table[dma_ch->ctrl_id]->alloc_status &= ~(1 << dma_ch->ch_id);
        dw_dma_axi_reg_t *dma_base = (dw_dma_axi_reg_t *)dma_init_record.table[dma_ch->ctrl_id]->dev.reg_base;
        dw_dma_ch_en(dma_base, dma_ch->ctrl_id, false);
        slist_del(&dma_ch->next, &dma_init_record.table[dma_ch->ctrl_id]->head);
    }
}

csi_error_t csi_dma_ch_config(csi_dma_ch_t *dma_ch, csi_dma_ch_config_t *config)
{
    CSI_PARAM_CHK(dma_ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    uint32_t group_len = config->group_len;
    dw_dma_axi_reg_t *dma_base = (dw_dma_axi_reg_t *)dma_init_record.table[dma_ch->ctrl_id]->dev.reg_base;
    dw_dma_axi_ch_reg_t *dma_ch_base = (dw_dma_axi_ch_reg_t *)&dma_base->CH[dma_ch->ch_id];

    if (config->dst_inc == DMA_ADDR_DEC || config->src_inc == DMA_ADDR_DEC) {
        return CSI_UNSUPPORTED;
    }

    if (config->dst_inc == DMA_ADDR_INC) {
        dw_dma_ch_set_dst_inc(dma_ch_base);
    } else if (config->dst_inc == DMA_ADDR_CONSTANT) {
        dw_dma_ch_set_dst_no_change(dma_ch_base);
    }

    if (config->src_inc == DMA_ADDR_INC) {
        dw_dma_ch_set_src_inc(dma_ch_base);
    } else if (config->src_inc == DMA_ADDR_CONSTANT) {
        dw_dma_ch_set_src_no_change(dma_ch_base);
    }

    dw_dma_ch_set_src_width(dma_ch_base, config->src_tw);
    dw_dma_ch_set_dst_width(dma_ch_base, config->dst_tw);

    group_len /= get_power(2U, (uint32_t)config->dst_tw);

    if (group_len > 1U) {
        dw_dma_ch_enable_arlen(dma_ch_base);
        dw_dma_ch_set_arlen(dma_ch_base, group_len);
        dw_dma_ch_enable_awlen(dma_ch_base);
        dw_dma_ch_set_awlen(dma_ch_base, group_len);
    } else {
        dw_dma_ch_disable_arlen(dma_ch_base);
        dw_dma_ch_disable_awlen(dma_ch_base);
    }

    switch (group_len) {
        case 1U:
            dw_dma_ch_set_dst_msize(dma_ch_base, DATA_ITEMS_1);
            dw_dma_ch_set_src_msize(dma_ch_base, DATA_ITEMS_1);
            break;

        case 4U:
            dw_dma_ch_set_dst_msize(dma_ch_base, DATA_ITEMS_4);
            dw_dma_ch_set_src_msize(dma_ch_base, DATA_ITEMS_4);
            break;

        case 8U:
            dw_dma_ch_set_dst_msize(dma_ch_base, DATA_ITEMS_8);
            dw_dma_ch_set_src_msize(dma_ch_base, DATA_ITEMS_8);
            break;

        case 16U:
            dw_dma_ch_set_dst_msize(dma_ch_base, DATA_ITEMS_16);
            dw_dma_ch_set_src_msize(dma_ch_base, DATA_ITEMS_16);
            break;

        case 32U:
            dw_dma_ch_set_dst_msize(dma_ch_base, DATA_ITEMS_32);
            dw_dma_ch_set_src_msize(dma_ch_base, DATA_ITEMS_32);
            break;

        case 64U:
            dw_dma_ch_set_dst_msize(dma_ch_base, DATA_ITEMS_64);
            dw_dma_ch_set_src_msize(dma_ch_base, DATA_ITEMS_64);
            break;

        case 128U:
            dw_dma_ch_set_dst_msize(dma_ch_base, DATA_ITEMS_128);
            dw_dma_ch_set_src_msize(dma_ch_base, DATA_ITEMS_128);
            break;

        default:
            dw_dma_ch_set_dst_msize(dma_ch_base, DATA_ITEMS_1);
            dw_dma_ch_set_src_msize(dma_ch_base, DATA_ITEMS_1);
            break;
    }

    /* the dma default type is contiguous */
    dw_dma_ch_set_dst_multblk_type_contiguous(dma_ch_base);
    dw_dma_ch_set_src_multblk_type_contiguous(dma_ch_base);
    dw_dma_ch_set_prior(dma_ch_base, 0U); ///< set all ch prior is the lowest

    if (config->dst_reload_en) {
        dw_dma_ch_set_dst_multblk_type_reload(dma_ch_base);
    }

    if (config->src_reload_en) {
        dw_dma_ch_set_src_multblk_type_reload(dma_ch_base);
    }

    switch (config->trans_dir) {
        case DMA_MEM2MEM:
            dw_dma_ch_set_transfer_type_m2m_fc_dma(dma_ch_base);
            dw_dma_ch_sel_dst_softshake(dma_ch_base);
            dw_dma_ch_sel_src_softshake(dma_ch_base);
            dw_dma_ch_set_src_handshake(dma_ch_base, 0);
            dw_dma_ch_set_dst_handshake(dma_ch_base, 0);
            break;

        case DMA_MEM2PERH:
            dw_dma_ch_set_transfer_type_m2p_fc_dma(dma_ch_base);
            dw_dma_ch_sel_src_softshake(dma_ch_base);
            dw_dma_ch_sel_dst_handshake(dma_ch_base);
            dw_dma_ch_set_src_handshake(dma_ch_base, 0);
            dw_dma_ch_set_dst_handshake(dma_ch_base, config->handshake);
            break;

        case DMA_PERH2MEM:
            dw_dma_ch_set_transfer_type_p2m_fc_dma(dma_ch_base);
            dw_dma_ch_sel_dst_softshake(dma_ch_base);
            dw_dma_ch_sel_src_handshake(dma_ch_base);
            dw_dma_ch_set_dst_handshake(dma_ch_base, 0);
            dw_dma_ch_set_src_handshake(dma_ch_base, config->handshake);
            break;

        default:
            ret = CSI_ERROR;
            return ret;
    }

    if (config->cyclic_en) {
        dma_ch->cyclic_state = 0;
        dw_dma_ch_set_dst_multblk_type_llp(dma_ch_base);
        dw_dma_ch_set_src_multblk_type_llp(dma_ch_base);
        dw_axi_dma_lli_chain_into_loop(config);
        dw_dma_ch_set_llp_address(dma_ch_base, (uint64_t)config->cyclic_config.lli_buf);
        soc_dcache_clean_invalid_range((unsigned long)(config->cyclic_config.lli_buf), sizeof(dw_dma_axi_lli_t) * config->cyclic_config.buffer_bytes / config->cyclic_config.period_bytes);
        dma_ch->cyclic_state = DMA_CYCLIC_READY;
    }

    if (config->link_list_en) {
        dma_ch->link_list_state = 0;
        dw_dma_ch_set_dst_multblk_type_llp(dma_ch_base);
        dw_dma_ch_set_src_multblk_type_llp(dma_ch_base);
        dw_dma_ch_set_llp_address(dma_ch_base, (uint64_t)config->link_list_config.lli_buf);
        dma_ch->link_list_state = DMA_LINK_LIST_READY;
    }

    return ret;
}

void csi_dma_ch_start(csi_dma_ch_t *dma_ch, void *srcaddr, void *dstaddr, uint32_t length)
{
    CSI_PARAM_CHK_NORETVAL(dma_ch);
    dw_dma_axi_reg_t *dma_base = (dw_dma_axi_reg_t *)dma_init_record.table[dma_ch->ctrl_id]->dev.reg_base;
    dw_dma_axi_ch_reg_t *dma_ch_base = (dw_dma_axi_ch_reg_t *)&dma_base->CH[dma_ch->ch_id];

    if (dma_ch->cyclic_state == DMA_CYCLIC_READY) {
        dma_ch->cyclic_state = DMA_CYCLIC_RUNNING;
        dw_dma_ch_enable_irq(dma_ch_base, DW_AXI_DMAC_CH_INT_EN_DMA_TFR_DONE | DW_AXI_DMAC_CH_INT_EN_BLOCK_TFR_DONE);
        dw_dma_ch_enable_intsignal(dma_ch_base, DW_AXI_DMAC_CH_INTSIGNAL_EN_DMA_TFR_DONE | DW_AXI_DMAC_CH_INTSIGNAL_EN_BLOCK_TFR_DONE);
        dw_dma_ch_suspend(dma_base, dma_ch->ch_id, false);
        dw_dma_ch_en(dma_base, dma_ch->ch_id, true);
    } else if (dma_ch->link_list_state == DMA_LINK_LIST_READY) {
        dma_ch->link_list_state = DMA_LINK_LIST_RUNNING;
        dw_dma_ch_enable_irq(dma_ch_base, DW_AXI_DMAC_CH_INT_EN_DMA_TFR_DONE | DW_AXI_DMAC_CH_INT_EN_BLOCK_TFR_DONE);
        dw_dma_ch_enable_intsignal(dma_ch_base, DW_AXI_DMAC_CH_INTSIGNAL_EN_DMA_TFR_DONE | DW_AXI_DMAC_CH_INTSIGNAL_EN_BLOCK_TFR_DONE);
        dw_dma_ch_suspend(dma_base, dma_ch->ch_id, false);
        dw_dma_ch_en(dma_base, dma_ch->ch_id, true);
    } else {
        dw_dma_ch_set_src(dma_ch_base, soc_virt_to_phys(srcaddr));
        dw_dma_ch_set_dst(dma_ch_base, soc_virt_to_phys(dstaddr));
        uint32_t dma_width_to_bytes = dw_dma_ch_get_src_width(dma_ch_base) / 8U;

        if (length % dma_width_to_bytes) {
            return;
        }

        length /= dma_width_to_bytes;

        if (length <= 0) {
            return;
        }

        dw_dma_ch_set_block_ts(dma_ch_base, length);
        dw_dma_ch_enable_irq(dma_ch_base, DW_AXI_DMAC_CH_INT_EN_DMA_TFR_DONE);
        dw_dma_ch_enable_intsignal(dma_ch_base, DW_AXI_DMAC_CH_INTSIGNAL_EN_DMA_TFR_DONE);
        dw_dma_ch_suspend(dma_base, dma_ch->ch_id, false);
        dw_dma_ch_en(dma_base, dma_ch->ch_id, true);
    }

}

void csi_dma_ch_stop(csi_dma_ch_t *dma_ch)
{
    CSI_PARAM_CHK_NORETVAL(dma_ch);

    dw_dma_axi_reg_t *dma_base = (dw_dma_axi_reg_t *)dma_init_record.table[dma_ch->ctrl_id]->dev.reg_base;

    if (dma_ch->cyclic_state == DMA_CYCLIC_RUNNING) {
        dma_ch->cyclic_state = DMA_CYCLIC_STOP;
    }

    dw_dma_ch_en(dma_base, dma_ch->ch_id, false);
}

void dma_ch_pause(csi_dma_ch_t *dma_ch)
{
    CSI_PARAM_CHK_NORETVAL(dma_ch);
    dw_dma_axi_reg_t *dma_base = (dw_dma_axi_reg_t *)dma_init_record.table[dma_ch->ctrl_id]->dev.reg_base;
    dw_dma_ch_suspend(dma_base, dma_ch->ch_id, true);
}

void dma_ch_resume(csi_dma_ch_t *dma_ch)
{
    CSI_PARAM_CHK_NORETVAL(dma_ch);
    dw_dma_axi_reg_t *dma_base = (dw_dma_axi_reg_t *)dma_init_record.table[dma_ch->ctrl_id]->dev.reg_base;
    dw_dma_ch_suspend(dma_base, dma_ch->ch_id, false);
}

csi_error_t csi_dma_ch_attach_callback(csi_dma_ch_t *dma_ch, void *callback, void *arg)
{
    CSI_PARAM_CHK(dma_ch, CSI_ERROR);
    dma_ch->callback = callback;
    dma_ch->arg = arg;
    return CSI_OK;
}

void csi_dma_ch_detach_callback(csi_dma_ch_t *dma_ch)
{
    CSI_PARAM_CHK_NORETVAL(dma_ch);
    dma_ch->callback = NULL;
    dma_ch->arg = NULL;
}

csi_error_t dma_add_link_list_item(csi_dma_ch_config_t *config, csi_dma_link_list_item_t *item)
{
    CSI_PARAM_CHK(config, CSI_ERROR);
    CSI_PARAM_CHK(item, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    uint8_t *temp_addr;
    dw_dma_axi_lli_t *lli_addr;
    temp_addr = (uint8_t *)config->link_list_config.lli_buf + sizeof(dw_dma_axi_lli_t) * config->link_list_config.link_list_num;
    lli_addr = (dw_dma_axi_lli_t *)temp_addr;
    uint32_t length = item->length;

    uint32_t dma_width_to_bytes = get_power(2U, (uint32_t)(config->src_tw));///< change dma width value to byte

    if (length % dma_width_to_bytes) {
        return CSI_ERROR;
    }

    length /= dma_width_to_bytes;

    if (length <= 0) {
        return CSI_ERROR;
    }

    dw_dma_lli_set_block_ts(lli_addr, length);
    lli_addr->SAR = (uint64_t)item->srcaddr;
    lli_addr->DAR = (uint64_t)item->dstaddr;

    if (config->dst_inc == DMA_ADDR_INC) {
        dw_dma_lli_set_dst_inc(lli_addr);
    } else if (config->dst_inc == DMA_ADDR_CONSTANT) {
        dw_dma_lli_set_dst_no_change(lli_addr);
    }

    if (config->src_inc == DMA_ADDR_INC) {
        dw_dma_lli_set_src_inc(lli_addr);
    } else if (config->src_inc == DMA_ADDR_CONSTANT) {
        dw_dma_lli_set_src_no_change(lli_addr);
    }

    dw_dma_lli_set_src_width(lli_addr, config->src_tw);
    dw_dma_lli_set_dst_width(lli_addr, config->dst_tw);

    uint16_t group_len = config->group_len;
    group_len /= get_power(2U, (uint32_t)config->dst_tw);

    switch (group_len) {
        case 1U:
            dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_1);
            dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_1);
            break;

        case 4U:
            dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_4);
            dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_4);
            break;

        case 8U:
            dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_8);
            dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_8);
            break;

        case 16U:
            dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_16);
            dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_16);
            break;

        case 32U:
            dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_32);
            dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_32);
            break;

        case 64U:
            dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_64);
            dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_64);
            break;

        case 128U:
            dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_128);
            dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_128);
            break;

        default:
            dw_dma_lli_set_dst_msize(lli_addr, DATA_ITEMS_1);
            dw_dma_lli_set_src_msize(lli_addr, DATA_ITEMS_1);
            break;
    }

    dw_dma_lli_enable_block_int(lli_addr, true);

    dw_dma_lli_data_is_valid(lli_addr);
    dw_dma_lli_data_is_last(lli_addr);

    if (config->link_list_config.link_list_num >= 1) {
        uint8_t *pre_addr;
        dw_dma_axi_lli_t *lli_pre_addr;
        pre_addr = (uint8_t *)config->link_list_config.lli_buf + sizeof(dw_dma_axi_lli_t) * (config->link_list_config.link_list_num - 1U);
        lli_pre_addr = (dw_dma_axi_lli_t *)pre_addr;
        dw_dma_lli_data_is_not_last(lli_pre_addr);
        lli_pre_addr->LLP = (uint64_t)lli_addr;
    }

    config->link_list_config.link_list_num ++;

    soc_dcache_clean_invalid_range((unsigned long)(config->link_list_config.lli_buf), sizeof(dw_dma_axi_lli_t) *config->link_list_config.link_list_num);
    return ret;
}
