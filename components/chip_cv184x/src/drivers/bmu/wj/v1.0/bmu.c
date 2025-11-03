/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     bmu.c
 * @brief    CSI Source File for bmu Driver
 * @version
 * @date     2020-10-28
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <drv/bmu.h>
#include <drv/cvi_irq.h>
#include <drv/common.h>
#include <drv/porting.h>
#include "wj_bmu_ll.h"

static void wj_bmu_irq_handler(void *arg)
{
    drv_bmu_t *bmu = (drv_bmu_t *)arg;
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    uint32_t ir_sta = wj_bmu_apb_int_sta(bmu_base);
    wj_bmu_int_sta_clear(bmu_base);
    drv_bmu_event_t event = 0;

    if (ir_sta & BMU_TIMER_EXPIRED_INT_STA) {
        event |= DRV_BMU_EVENT_PERIOD_EXPIRED;
    }

    if (ir_sta & BMU_TARGET_WDATA_OCCUR_INT_STA) {
        event |= DRV_BMU_EVENT_TARGET_RWDATA_OCCUR;
    }

    if (ir_sta & BMU_WRITE_ERROR_RESP_OCCUR_INT_STA) {
        event |= DRV_BMU_EVENT_RESP_ERROR_HAPPENED;
    }

    if ((ir_sta & BMU_WRITE_DURATION_COUNTER_FULL_INT_STA) ||
        (ir_sta & BMU_READ_DURATION_COUNTER_FULL_INT_STA)) {
        event |= DRV_BMU_EVENT_COUNTER_OVERFLOW;
    }

    if (bmu->callback != NULL) {
        bmu->callback(bmu, event, bmu->arg);
    }

}

csi_error_t drv_bmu_init(drv_bmu_t *bmu, int idx)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ret = target_get(DEV_WJ_BMU_TAG, idx, &bmu->dev);
    return ret;
}

csi_error_t drv_bmu_attach_callback(drv_bmu_t *bmu, void *callback, void *arg)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    bmu->arg = arg;
    bmu->callback = callback;
    csi_irq_attach((uint32_t)(bmu->dev.irq_num), &wj_bmu_irq_handler, &bmu->dev);
    csi_irq_enable((uint32_t)(bmu->dev.irq_num));
    return ret;
}

void drv_bmu_detach_callback(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK_NORETVAL(bmu);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    wj_bmu_bmu_monitor_disable_interrupt(bmu_base);
    bmu->callback = NULL;
    bmu->arg = NULL;
    csi_irq_detach((uint32_t)(bmu->dev.irq_num));
    csi_irq_disable((uint32_t)(bmu->dev.irq_num));
}

csi_error_t drv_bmu_monitor_config(drv_bmu_t *bmu, drv_bmu_monitor_config_t *bmu_monitor_config)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    CSI_PARAM_CHK(bmu_monitor_config, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);

    if (bmu_monitor_config->bmu_monitor_mode == DRV_BMU_SINGLE_MONITOR_MODE) {
        bmu->mode = DRV_BMU_SINGLE_MONITOR_MODE;
        wj_bmu_set_single_monitor_mode(bmu_base);
    } else if (bmu_monitor_config->bmu_monitor_mode == DRV_BMU_PERIOD_MONITOR_MODE) {
        bmu->mode = DRV_BMU_PERIOD_MONITOR_MODE;
        wj_bmu_set_period_monitor_mode(bmu_base);
    } else {
        ret = CSI_ERROR;
    }

    wj_bmu_set_monitor_period(bmu_base, bmu_monitor_config->bmu_monitor_period); ///< set apb clk cycles, apb:100MHZ period: 1000 time: 10us
    wj_bmu_set_monitor_period_div(bmu_base, bmu_monitor_config->bmu_monitor_period_div); ///< set div with apb clk

    wj_bmu_read_cmd_dura_threshold(bmu_base, bmu_monitor_config->bmu_read_dura_th);
    wj_bmu_write_cmd_dura_threshold(bmu_base, bmu_monitor_config->bmu_write_dura_th);

    if (bmu_monitor_config->bmu_ostd_mode == DRV_BUM_OSTD_8192_CYCLE) {
        wj_bmu_ostd_cnt_mode_8192(bmu_base);
    } else if (bmu_monitor_config->bmu_ostd_mode == DRV_BUM_OSTD_4096_CYCLE) {
        wj_bmu_ostd_cnt_mode_4096(bmu_base);
    }

    if (bmu_monitor_config->bmu_monitor_master_id_enable == true) {
        wj_bmu_compare_monitor_id_enable(bmu_base);
        wj_bmu_set_monitor_id(bmu_base, bmu_monitor_config->bmu_monitor_master_id);
    } else {
        wj_bmu_compare_monitor_id_disable(bmu_base);
    }


    return ret;

}

csi_error_t drv_bmu_variable_counter_filter_config(drv_bmu_t *bmu, drv_bmu_variable_counter_filter_config_t *bmu_filter_config, uint32_t counter_idx)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    CSI_PARAM_CHK(bmu_filter_config, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);

    if (counter_idx == 0) {
        wj_bmu_counter0_alignment_filter_enable(bmu_base, bmu_filter_config->bmu_filter_align_enable);
        wj_bmu_counter0_size_filter_enable(bmu_base, bmu_filter_config->bmu_filter_size_enable);
        wj_bmu_counter0_addr_range_filter_enable(bmu_base,  bmu_filter_config->bmu_filter_address_enable);
        wj_bmu_counter0_length_filter_enable(bmu_base,  bmu_filter_config->bmu_filter_length_enable);

        if (bmu_filter_config->bmu_filter_align_enable == true) {
            wj_bmu_counter0_addr_align_filter(bmu_base, bmu_filter_config->bmu_filter_align);
        }

        if (bmu_filter_config->bmu_filter_size_enable == true) {
            wj_bmu_counter0_size_filter(bmu_base, bmu_filter_config->bmu_filter_size);
        }

        if (bmu_filter_config->bmu_filter_length_enable == true) {
            wj_bmu_counter0_length_filter(bmu_base, bmu_filter_config->bmu_filter_length);
        }

        if (bmu_filter_config->bmu_filter_address_enable == true) {
            wj_bmu_counter0_addr_filter(bmu_base, bmu_filter_config->bmu_filter_lower_address, bmu_filter_config->bmu_filter_upper_address);
        }
    } else if (counter_idx == 1) {
        wj_bmu_counter1_alignment_filter_enable(bmu_base, bmu_filter_config->bmu_filter_align_enable);
        wj_bmu_counter1_size_filter_enable(bmu_base, bmu_filter_config->bmu_filter_size_enable);
        wj_bmu_counter1_addr_range_filter_enable(bmu_base,  bmu_filter_config->bmu_filter_address_enable);
        wj_bmu_counter1_length_filter_enable(bmu_base,  bmu_filter_config->bmu_filter_length_enable);

        if (bmu_filter_config->bmu_filter_align_enable == true) {
            wj_bmu_counter1_addr_align_filter(bmu_base, bmu_filter_config->bmu_filter_align);
        }

        if (bmu_filter_config->bmu_filter_size_enable == true) {
            wj_bmu_counter1_size_filter(bmu_base, bmu_filter_config->bmu_filter_size);
        }

        if (bmu_filter_config->bmu_filter_length_enable == true) {
            wj_bmu_counter1_length_filter(bmu_base, bmu_filter_config->bmu_filter_length);
        }

        if (bmu_filter_config->bmu_filter_address_enable == true) {
            wj_bmu_counter1_addr_filter(bmu_base, bmu_filter_config->bmu_filter_lower_address, bmu_filter_config->bmu_filter_upper_address);
        }

    } else {
        ret = CSI_ERROR;
    }

    return ret;
}

csi_error_t drv_bmu_start_monitor(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    wj_bmu_bmu_monitor_reset(bmu_base);
    wj_bmu_bmu_monitor_interrupt(bmu_base, 0x1fU);

    if (bmu->mode == DRV_BMU_SINGLE_MONITOR_MODE) {
        wj_bmu_bmu_monitor_enable(bmu_base);
        wj_bmu_bmu_monitor_disable(bmu_base);
    } else {
        wj_bmu_bmu_monitor_enable(bmu_base);
    }

    return ret;
}

void drv_bmu_stop_monitor(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK_NORETVAL(bmu);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    wj_bmu_bmu_monitor_disable(bmu_base);
}

csi_error_t drv_bmu_show_hardware_info(drv_bmu_t *bmu, drv_bmu_hardware_info_t *info)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    CSI_PARAM_CHK(info, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    uint32_t info0 = wj_bmu_read_version0(bmu_base);
    uint32_t info1 = wj_bmu_read_version1(bmu_base);

    info->address_width = (uint8_t)info0;
    info->id_width      = (uint8_t)(info0 >> 8U);
    info->monitor_read_duration_fifo_depth  = (uint8_t)(info0 >> 16U);
    info->monitor_write_duration_fifo_depth = (uint8_t)(info0 >> 24U);

    info->axi_length_width = (uint8_t)info1;
    info->variable_counter_number = (uint8_t)(info1 >> 8U);
    info->axi_data_width = (uint16_t)(info1 >> 16U);
    return ret;
}

void drv_bmu_uninit(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK_NORETVAL(bmu);
    csi_irq_disable((uint32_t)bmu->dev.irq_num);
    csi_irq_detach((uint32_t)bmu->dev.irq_num);
}

uint32_t drv_bmu_get_ostd_status(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_OSTD_STS;
}

uint32_t drv_bmu_get_total_read_duration_counter(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_RD_STS0;
}

uint32_t drv_bmu_get_total_read_transaction_number(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_RD_STS1;
}

uint32_t drv_bmu_get_total_read_byte_number(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_RD_STS2;
}

uint32_t drv_bmu_get_over_dura_threshod_read_cmd_number(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_RD_STS3;
}

uint32_t drv_bmu_get_total_write_duration_counter(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_WD_STS0;
}

uint32_t drv_bmu_get_total_write_transaction_number(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_WD_STS1;
}

uint32_t drv_bmu_get_total_write_byte_number(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_WD_STS2;
}

uint32_t drv_bmu_get_over_dura_threshod_write_cmd_number(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_WD_STS3;
}

uint32_t drv_bmu_get_variable_read_counter0(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_VRD_STS0;
}

uint32_t drv_bmu_get_variable_read_counter1(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_VRD_STS1;
}

uint32_t drv_bmu_get_variable_write_counter0(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_VWR_STS0;
}

uint32_t drv_bmu_get_variable_write_counter1(drv_bmu_t *bmu)
{
    CSI_PARAM_CHK(bmu, CSI_ERROR);
    wj_bmu_regs_t *bmu_base = (wj_bmu_regs_t *)HANDLE_REG_BASE(bmu);
    return bmu_base->BMU_VWR_STS1;
}
