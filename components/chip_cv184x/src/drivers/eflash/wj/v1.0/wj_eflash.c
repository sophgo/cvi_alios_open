/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     wj_eflash.c
 * @brief    wj eflash driver
 * @version  V1.0
 * @date     21. May 2020
 ******************************************************************************/

#include <wj_eflash.h>
#include <drv/eflash.h>
#include <sys_clk.h>

static int32_t eflash_program_word(csi_eflash_t *eflash, uint32_t addr, uint32_t *data, uint32_t size)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);

    wj_eflash_regs_t *fbase = (wj_eflash_regs_t *)HANDLE_REG_BASE(eflash);

    for (uint32_t num = 0; num < size; num++) {
        wj_eflash_write_address(fbase, addr);
        wj_edflash_write_data(fbase, *data);
        wj_eflash_write_enbale(fbase);
        addr += 4;
        data++;
    }

    return 0;
}

csi_error_t csi_eflash_init(csi_eflash_t *eflash, int32_t idx, void *arg)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);
#ifdef CONFIG_CHIP_CH2201
    uint16_t prog;
    uint16_t erase;
#endif
    csi_error_t ret = CSI_OK;
    eflash_status_t *eflash_status = NULL;

    target_get(DEV_WJ_EFLASH_TAG, idx, &(eflash->dev));
#ifdef CONFIG_CHIP_CH2201
    drv_get_eflash_params(OSR_8M_CLK_24M, &prog, &erase);
#endif
    eflash_status->busy = 1;
    eflash_status->error = 0;
    eflash->priv = eflash_status;
#ifdef CONFIG_CHIP_CH2201
    eflash->prog = prog;
    eflash->arg = arg;
    eflash->erase = erase;
#endif
    eflash->eflashinfo.erased_value = 0xff;
    eflash->eflashinfo.flash_size = CSKY_EFLASH_SIZE;
    eflash->eflashinfo.sector_size = CSKY_EFLASH_SECTOR_SIZE;

    return ret;
}

void csi_eflash_uninit(csi_eflash_t *eflash)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);
    eflash = NULL;
}

csi_error_t csi_eflash_read(csi_eflash_t *eflash, uint32_t offset, void *data, uint32_t size)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    volatile uint32_t addr = (uint32_t)CSKY_EFLASH_BASE + offset;

    if (EFLASH_ADDR_START > addr || EFLASH_ADDR_END < addr || size < 1 || addr + size - 1 > EFLASH_ADDR_END) {
        return CSI_ERROR;
    }

    eflash_status_t *eflash_status = (eflash_status_t *)eflash->priv;
    csi_error_t ret = CSI_OK;
    volatile uint8_t *src_addr = (uint8_t *)addr;
    uint32_t i;

    if (eflash_status->busy) {
        return CSI_BUSY;
    }

    eflash_status->busy = 1;

    for (i = 0; i < size; i++) {
        *((uint8_t *)data + i) = *((uint8_t *)src_addr + i);
    }

    eflash_status->busy = 0;

    if (i == size) {
        ret = CSI_OK;
    } else {
        ret = CSI_ERROR;
    }

    return ret;;
}

csi_error_t csi_eflash_program(csi_eflash_t *eflash, uint32_t offset, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    volatile uint32_t addr = (uint32_t)CSKY_EFLASH_BASE + offset;

    if (EFLASH_ADDR_START > addr || EFLASH_ADDR_END < addr || size < 1 || addr + size - 1 > EFLASH_ADDR_END) {
        return CSI_ERROR;
    }

    uint32_t cur = 0;
    uint32_t pad_buf = 0;
    eflash_status_t *eflash_status = (eflash_status_t *)eflash->priv;

    if (eflash_status->busy) {
        return CSI_BUSY;
    }

    eflash_status->busy = 1;
    eflash_status->error = 0;
#ifdef CONFIG_CHIP_CH2201
    wj_eflash_regs_t *fbase = (wj_eflash_regs_t *)HANDLE_REG_BASE(eflash);
    fbase->EFLASH_TRCV = eflash->prog;
#endif

    if ((size & 0x3) == 0) {
        cur = size & 0x3;
        eflash_program_word(eflash, addr, (uint32_t *)data, size >> 2);
    } else {
        cur = size & 0x3;
        eflash_program_word(eflash, addr, (uint32_t *)data, size >> 2);
    }

    if (cur > 0) {
        uint8_t *buffer_b = (uint8_t *)data + cur;

        for (int i = 0; i < cur; i++) {
            pad_buf = pad_buf | buffer_b[i] << i * 8;
        }

        eflash_program_word(eflash, addr + size - cur, &pad_buf, 1);
    }

    eflash_status->busy = 0;

    return size;
}

csi_error_t csi_eflash_erase(csi_eflash_t *eflash, uint32_t offset, uint32_t size)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);

    volatile uint32_t addr = (uint32_t)CSKY_EFLASH_BASE + offset;

    if (EFLASH_ADDR_START > addr || EFLASH_ADDR_END < addr || size < 1 || addr + size * EFLASH_SECTOR_SIZE - 1 > EFLASH_ADDR_END) {
        return CSI_ERROR;
    }

    eflash_status_t *eflash_status = (eflash_status_t *)eflash->priv;
    csi_error_t ret = CSI_OK;
    uint32_t i;


    if (addr != EFLASH_ADDR_START && (addr - EFLASH_ADDR_START) % EFLASH_SECTOR_SIZE != 0) {
        return CSI_ERROR;
    }

    wj_eflash_regs_t *fbase = (wj_eflash_regs_t *)HANDLE_REG_BASE(eflash);

    if (eflash_status->busy) {
        return CSI_BUSY;
    }

#ifdef CONFIG_CHIP_CH2201
    fbase->EFLASH_TRCV = eflash->erase;
#endif
    eflash_status->busy = 1;

    for (i = 0; i < size; i++) {
        eflash_status->error = 0;
        wj_eflash_write_address(fbase, addr);
        wj_eflash_erase_enbale(fbase);
        addr = addr + EFLASH_SECTOR_SIZE;
        eflash_status->busy = 0;
    }

    if (i == size) {
        ret = CSI_OK;
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}

csi_error_t csi_eflash_erase_chip(csi_eflash_t *eflash)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);

    return (CSI_UNSUPPORTED);
}

void csi_eflash_info(csi_eflash_t *eflash, csi_eflash_info_t *eflash_info)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);
    *eflash_info = eflash->eflashinfo;
}

csi_error_t csi_eflash_enable_pm(csi_eflash_t *eflash)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);

    return 0;
}

void csi_eflash_disable_pm(csi_eflash_t *eflash)
{
    CSI_PARAM_CHK(eflash, CSI_ERROR);
}


