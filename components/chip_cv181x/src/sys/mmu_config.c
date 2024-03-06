/*
 * Copyright (C) 2018-2022 Alibaba Group Holding Limited
 */

#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
#include <soc.h>
#include <csi_core.h>
#include <drv/tick.h>
#include <drv/porting.h>
#include <excore/k_mmu.h>

extern void *_mmu_text_start;
extern void *_mmu_text_len;
extern void *_mmu_rodata_start;
extern void *_mmu_rodata_len;
extern void *_mmu_data_start;
extern void *_mmu_data_len;
extern void *_mmu_bss_start;
extern void *_mmu_bss_len;
extern void *_mmu_heap_start;
extern void *_mmu_heap_len;

__attribute__((weak)) k_mmu_regions_t g_memmaps[]  = {
    {
        .vaddr    = 0x0UL,
        .paddr    = 0x0UL,
        .len      = 0x80000000UL, /* space of io-device */
        .mem_attr = MMU_ATTR_SO
    },
    {
        .vaddr    = (unsigned long)&_mmu_text_start,
        .paddr    = (unsigned long)&_mmu_text_start,
        .len      = (size_t)&_mmu_text_len,
        .mem_attr = MMU_ATTR_DATA_RO
    },
    {
        .vaddr    = (unsigned long)&_mmu_rodata_start,
        .paddr    = (unsigned long)&_mmu_rodata_start,
        .len      = (size_t)&_mmu_rodata_len,
        .mem_attr = MMU_ATTR_DATA_RO
    },
    {
        .vaddr    = (unsigned long)&_mmu_data_start,
        .paddr    = (unsigned long)&_mmu_data_start,
        .len      = (size_t)&_mmu_data_len,
        .mem_attr = MMU_ATTR_DATA
    },
    {
        .vaddr    = (unsigned long)&_mmu_bss_start,
        .paddr    = (unsigned long)&_mmu_bss_start,
        .len      = (size_t)&_mmu_bss_len,
        .mem_attr = MMU_ATTR_DATA
    },
    {
        .vaddr    = (unsigned long)&_mmu_heap_start,
        .paddr    = (unsigned long)&_mmu_heap_start,
        .len      = (size_t)&_mmu_heap_len,
        .mem_attr = MMU_ATTR_DATA
    },
};

int g_memmaps_size = ARRAY_SIZE(g_memmaps);
#endif

