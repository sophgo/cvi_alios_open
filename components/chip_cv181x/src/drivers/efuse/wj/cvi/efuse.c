/*
 * Copyright (C) 2017-2019 cvitek Group Holding Limited
 */

/******************************************************************************
 * @file     efuse.c
 * @brief    CSI Source File for efuse Driver
 * @version  V1.0
 * @date     17. July 2020
 ******************************************************************************/

#include <drv/efuse.h>
#include <sys_clk.h>

#include "drv/common.h"
#include "cvi_efuse.h"

static struct CVI_EFUSE_MAP_S {
    CVI_U32 addr;
    CVI_U32 size;
} cvi_efuse_map[] = {
    { 0x10, 4 }, { 0x12, 4 }, { 0x14, 4 }, { 0x16, 4 }, { 0x18, 4 },
    { 0x1A, 4 }, { 0x1C, 4 }, { 0x1E, 4 }, { 0x20, 4 }, { 0x22, 4 },
    { 0x2A, 4 }, { 0x2B, 4 }, { 0x2C, 4 }, { 0x2D, 4 }, { 0x2E, 4 },
    { 0x2F, 4 }, { 0x30, 4 }, { 0x31, 4 }
};

csi_error_t csi_efuse_init(csi_efuse_t *efuse, int32_t idx)
{
    CSI_PARAM_CHK(efuse, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (cvi_efuse_pwr_on() != CSI_OK) {
        printf("cvi_efuse_pwr_on failed\n");
        return CSI_ERROR;
    }

    efuse->info.start = 0x0;
    efuse->info.end = 0x48;

    return ret;
}

void csi_efuse_uninit(csi_efuse_t *efuse)
{
    CSI_PARAM_CHK_NORETVAL(efuse);

    if (cvi_efuse_pwr_off() != CSI_OK) {
        printf("cvi_efuse_pwr_off failed\n");
        return;
    }

    memset(efuse, 0, sizeof(csi_efuse_t));
}

int32_t csi_efuse_read(csi_efuse_t *efuse, uint32_t addr, void *data, uint32_t size)
{
    CSI_PARAM_CHK(efuse,CSI_ERROR);
    if ((size & 0x3) != 0) return CSI_ERROR;
    if ((addr & 0x3) != 0) return CSI_ERROR;

    int i;

    if (addr < efuse->info.start || addr > efuse->info.end || (addr + size) > efuse->info.end) {
        return CSI_ERROR;
    }

    int map_start_index = (addr >> 2);
    for (i = 0; i < (size >> 2); i++) {
        if (cvi_efuse_read_word_from_shadow(cvi_efuse_map[map_start_index + i].addr, (uint32_t *)data + i) != CSI_OK) {
            printf("cvi_efuse_read_word_from_shadow failed\n");
            return (i << 2);
        }
    }

    return (i << 2);
}

int32_t csi_efuse_program(csi_efuse_t *efuse, uint32_t addr, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(efuse,CSI_ERROR);
    if ((size & 0x3) != 0) return CSI_ERROR;
    if ((addr & 0x3) != 0) return CSI_ERROR;

    int i;
    uint32_t *p = (uint32_t *)data;

    if (addr < efuse->info.start || addr > efuse->info.end || (addr + size) > efuse->info.end) {
        return CSI_ERROR;
    }

    int map_start_index = (addr >> 2);
    for (i = 0; i < (size >> 2); i++) {
        if (cvi_efuse_program_word(cvi_efuse_map[map_start_index + i].addr, p[i]) != CSI_OK) {
            printf("cvi_efuse_program_word failed\n");
            return (i << 2);
        }
    }

    return (i << 2);
}

csi_error_t csi_efuse_get_info(csi_efuse_t *efuse, csi_efuse_info_t *info)
{
    CSI_PARAM_CHK(efuse, CSI_ERROR);
    CSI_PARAM_CHK(info, CSI_ERROR);

    info->start =  efuse->info.start;
    info->end =  efuse->info.end;
    return CSI_OK;
}
