/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: metal_io_region operations for bare-metal (no MMU).
 * On AliOS, physical address equals virtual address, so I/O is
 * simple pointer access with optional cache flush/invalidate.
 */

#ifndef METAL_INTERNAL
#define METAL_INTERNAL
#endif

#include <metal/io.h>
#include <metal/sys.h>
#include <metal/cache.h>
#include <string.h>

/* Global libmetal state — defined in metal_sys.c */
extern struct metal_state _metal;

/* Forward declarations for cache ops */
extern void metal_machine_cache_flush(void *addr, unsigned int len);
extern void metal_machine_cache_invalidate(void *addr, unsigned int len);

static uint64_t metal_io_read_default(struct metal_io_region *io,
                                      unsigned long offset,
                                      memory_order order,
                                      int width)
{
    void *ptr = metal_io_virt(io, offset);
    uint64_t value = 0;

    if (!ptr)
        return 0;

    /* Invalidate cache before reading shared memory */
    metal_cache_invalidate(ptr, width);

    switch (width) {
    case 1:
        value = *(volatile uint8_t *)ptr;
        break;
    case 2:
        value = *(volatile uint16_t *)ptr;
        break;
    case 4:
        value = *(volatile uint32_t *)ptr;
        break;
    case 8:
        value = *(volatile uint64_t *)ptr;
        break;
    default:
        metal_assert(0);
    }

    return value;
}

static void metal_io_write_default(struct metal_io_region *io,
                                   unsigned long offset,
                                   uint64_t value,
                                   memory_order order,
                                   int width)
{
    void *ptr = metal_io_virt(io, offset);

    if (!ptr)
        return;

    switch (width) {
    case 1:
        *(volatile uint8_t *)ptr = (uint8_t)value;
        break;
    case 2:
        *(volatile uint16_t *)ptr = (uint16_t)value;
        break;
    case 4:
        *(volatile uint32_t *)ptr = (uint32_t)value;
        break;
    case 8:
        *(volatile uint64_t *)ptr = (uint64_t)value;
        break;
    default:
        metal_assert(0);
    }

    /* Flush cache after writing shared memory */
    metal_cache_flush(ptr, width);
}

static int metal_io_block_read_default(struct metal_io_region *io,
                                       unsigned long offset,
                                       void *restrict dst,
                                       memory_order order,
                                       int len)
{
    void *ptr = metal_io_virt(io, offset);

    if (!ptr || len <= 0)
        return 0;

    /* Invalidate cache before reading */
    metal_cache_invalidate(ptr, len);

    memcpy(dst, ptr, len);
    return len;
}

static int metal_io_block_write_default(struct metal_io_region *io,
                                        unsigned long offset,
                                        const void *restrict src,
                                        memory_order order,
                                        int len)
{
    void *ptr = metal_io_virt(io, offset);

    if (!ptr || len <= 0)
        return 0;

    memcpy(ptr, src, len);

    /* Flush cache after writing */
    metal_cache_flush(ptr, len);

    return len;
}

static void metal_io_block_set_default(struct metal_io_region *io,
                                       unsigned long offset,
                                       unsigned char value,
                                       memory_order order,
                                       int len)
{
    void *ptr = metal_io_virt(io, offset);

    if (!ptr || len <= 0)
        return;

    memset(ptr, value, len);

    /* Flush cache after writing */
    metal_cache_flush(ptr, len);
}

static metal_phys_addr_t
metal_io_offset_to_phys_default(struct metal_io_region *io,
                                unsigned long offset)
{
    /* On bare-metal, phys = virt */
    return (metal_phys_addr_t)((uintptr_t)io->virt + offset);
}

static unsigned long
metal_io_phys_to_offset_default(struct metal_io_region *io,
                                metal_phys_addr_t phys)
{
    /* On bare-metal, offset = phys - virt_base */
    unsigned long offset = (unsigned long)(phys - (metal_phys_addr_t)io->virt);

    if (offset < io->size)
        return offset;

    return METAL_BAD_OFFSET;
}

static const struct metal_io_ops metal_io_default_ops = {
    .read           = metal_io_read_default,
    .write          = metal_io_write_default,
    .block_read     = metal_io_block_read_default,
    .block_write    = metal_io_block_write_default,
    .block_set      = metal_io_block_set_default,
    .close          = NULL,
    .offset_to_phys = metal_io_offset_to_phys_default,
    .phys_to_offset = metal_io_phys_to_offset_default,
};

void metal_io_init(struct metal_io_region *io, void *virt,
                   const metal_phys_addr_t *physmap, size_t size,
                   unsigned int page_shift, unsigned int mem_flags,
                   const struct metal_io_ops *ops)
{
    io->virt = virt;
    io->physmap = physmap;
    io->size = size;
    io->page_shift = page_shift;
    io->page_mask = (metal_phys_addr_t)((1UL << page_shift) - 1);
    io->mem_flags = mem_flags;
    if (ops)
        io->ops = *ops;
    else
        io->ops = metal_io_default_ops;
}

int metal_io_block_read(struct metal_io_region *io, unsigned long offset,
                        void *restrict dst, int len)
{
    if (io->ops.block_read)
        return io->ops.block_read(io, offset, dst, memory_order_seq_cst, len);

    void *ptr = metal_io_virt(io, offset);
    if (!ptr || len <= 0)
        return 0;

    memcpy(dst, ptr, len);
    return len;
}

int metal_io_block_write(struct metal_io_region *io, unsigned long offset,
                         const void *restrict src, int len)
{
    if (io->ops.block_write)
        return io->ops.block_write(io, offset, src, memory_order_seq_cst, len);

    void *ptr = metal_io_virt(io, offset);
    if (!ptr || len <= 0)
        return 0;

    memcpy(ptr, src, len);
    return len;
}

int metal_io_block_set(struct metal_io_region *io, unsigned long offset,
                       unsigned char value, int len)
{
    if (io->ops.block_set)
        io->ops.block_set(io, offset, value, memory_order_seq_cst, len);

    void *ptr = metal_io_virt(io, offset);
    if (!ptr || len <= 0)
        return 0;

    memset(ptr, value, len);
    return len;
}
