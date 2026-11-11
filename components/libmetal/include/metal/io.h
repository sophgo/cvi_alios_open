/*
 * Copyright (c) 2015 - 2017, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_IO__H__
#define __METAL_IO__H__

#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <metal/assert.h>
#include <metal/compiler.h>
#include <metal/atomic.h>
#include <metal/sys.h>
#include <metal/cpu.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __MICROBLAZE__
#define NO_ATOMIC_64_SUPPORT
#endif

struct metal_io_region;

struct metal_io_ops {
    uint64_t    (*read)(struct metal_io_region *io,
                        unsigned long offset,
                        memory_order order,
                        int width);
    void        (*write)(struct metal_io_region *io,
                         unsigned long offset,
                         uint64_t value,
                         memory_order order,
                         int width);
    int         (*block_read)(struct metal_io_region *io,
                              unsigned long offset,
                              void *restrict dst,
                              memory_order order,
                              int len);
    int         (*block_write)(struct metal_io_region *io,
                               unsigned long offset,
                               const void *restrict src,
                               memory_order order,
                               int len);
    void        (*block_set)(struct metal_io_region *io,
                             unsigned long offset,
                             unsigned char value,
                             memory_order order,
                             int len);
    void        (*close)(struct metal_io_region *io);
    metal_phys_addr_t (*offset_to_phys)(struct metal_io_region *io,
                                        unsigned long offset);
    unsigned long   (*phys_to_offset)(struct metal_io_region *io,
                                      metal_phys_addr_t phys);
};

struct metal_io_region {
    void                *virt;
    const metal_phys_addr_t *physmap;
    size_t              size;
    unsigned long       page_shift;
    metal_phys_addr_t   page_mask;
    unsigned int        mem_flags;
    struct metal_io_ops ops;
};

void
metal_io_init(struct metal_io_region *io, void *virt,
              const metal_phys_addr_t *physmap, size_t size,
              unsigned int page_shift, unsigned int mem_flags,
              const struct metal_io_ops *ops);

static inline void metal_io_finish(struct metal_io_region *io)
{
    if (io->ops.close)
        (*io->ops.close)(io);
    memset(io, 0, sizeof(*io));
}

static inline size_t metal_io_region_size(struct metal_io_region *io)
{
    return io->size;
}

static inline void *
metal_io_virt(struct metal_io_region *io, unsigned long offset)
{
    return (io->virt != METAL_BAD_VA && offset < io->size
            ? (void *)((uintptr_t)io->virt + offset)
            : NULL);
}

static inline unsigned long
metal_io_virt_to_offset(struct metal_io_region *io, void *virt)
{
    size_t offset = (uintptr_t)virt - (uintptr_t)io->virt;

    return (offset < io->size ? offset : METAL_BAD_OFFSET);
}

static inline metal_phys_addr_t
metal_io_phys(struct metal_io_region *io, unsigned long offset)
{
    if (!io->ops.offset_to_phys) {
        unsigned long page = (io->page_shift >=
                             sizeof(offset) * CHAR_BIT ?
                             0 : offset >> io->page_shift);
        return (io->physmap && offset < io->size
                ? io->physmap[page] + (offset & io->page_mask)
                : METAL_BAD_PHYS);
    }

    return io->ops.offset_to_phys(io, offset);
}

static inline unsigned long
metal_io_phys_to_offset(struct metal_io_region *io, metal_phys_addr_t phys)
{
    if (!io->ops.phys_to_offset) {
        unsigned long offset =
            (io->page_mask == (metal_phys_addr_t)(-1) ?
            phys - io->physmap[0] :  phys & io->page_mask);
        do {
            if (metal_io_phys(io, offset) == phys)
                return offset;
            offset += io->page_mask + 1;
        } while (offset < io->size);
        return METAL_BAD_OFFSET;
    }

    return (*io->ops.phys_to_offset)(io, phys);
}

static inline void *
metal_io_phys_to_virt(struct metal_io_region *io, metal_phys_addr_t phys)
{
    return metal_io_virt(io, metal_io_phys_to_offset(io, phys));
}

static inline metal_phys_addr_t
metal_io_virt_to_phys(struct metal_io_region *io, void *virt)
{
    return metal_io_phys(io, metal_io_virt_to_offset(io, virt));
}

static inline uint64_t
metal_io_read(struct metal_io_region *io, unsigned long offset,
              memory_order order, int width)
{
    void *ptr = metal_io_virt(io, offset);

    if (io->ops.read)
        return (*io->ops.read)(io, offset, order, width);
    else if (ptr && sizeof(atomic_uchar) == width)
        return atomic_load_explicit((atomic_uchar *)ptr, order);
    else if (ptr && sizeof(atomic_ushort) == width)
        return atomic_load_explicit((atomic_ushort *)ptr, order);
    else if (ptr && sizeof(atomic_uint) == width)
        return atomic_load_explicit((atomic_uint *)ptr, order);
    else if (ptr && sizeof(atomic_ulong) == width)
        return atomic_load_explicit((atomic_ulong *)ptr, order);
#ifndef NO_ATOMIC_64_SUPPORT
    else if (ptr && sizeof(atomic_ullong) == width)
        return atomic_load_explicit((atomic_ullong *)ptr, order);
#endif
    metal_assert(0);
    return 0;
}

static inline void
metal_io_write(struct metal_io_region *io, unsigned long offset,
               uint64_t value, memory_order order, int width)
{
    void *ptr = metal_io_virt(io, offset);

    if (io->ops.write)
        (*io->ops.write)(io, offset, value, order, width);
    else if (ptr && sizeof(atomic_uchar) == width)
        atomic_store_explicit((atomic_uchar *)ptr, (unsigned char)value, order);
    else if (ptr && sizeof(atomic_ushort) == width)
        atomic_store_explicit((atomic_ushort *)ptr, (unsigned short)value, order);
    else if (ptr && sizeof(atomic_uint) == width)
        atomic_store_explicit((atomic_uint *)ptr, (unsigned int)value, order);
    else if (ptr && sizeof(atomic_ulong) == width)
        atomic_store_explicit((atomic_ulong *)ptr, (unsigned long)value, order);
#ifndef NO_ATOMIC_64_SUPPORT
    else if (ptr && sizeof(atomic_ullong) == width)
        atomic_store_explicit((atomic_ullong *)ptr, (unsigned long long)value, order);
#endif
    else
        metal_assert(0);
}

#define metal_io_read8_explicit(_io, _ofs, _order) \
    metal_io_read((_io), (_ofs), (_order), 1)
#define metal_io_read8(_io, _ofs) \
    metal_io_read((_io), (_ofs), memory_order_seq_cst, 1)
#define metal_io_write8_explicit(_io, _ofs, _val, _order) \
    metal_io_write((_io), (_ofs), (_val), (_order), 1)
#define metal_io_write8(_io, _ofs, _val) \
    metal_io_write((_io), (_ofs), (_val), memory_order_seq_cst, 1)

#define metal_io_read16_explicit(_io, _ofs, _order) \
    metal_io_read((_io), (_ofs), (_order), 2)
#define metal_io_read16(_io, _ofs) \
    metal_io_read((_io), (_ofs), memory_order_seq_cst, 2)
#define metal_io_write16_explicit(_io, _ofs, _val, _order) \
    metal_io_write((_io), (_ofs), (_val), (_order), 2)
#define metal_io_write16(_io, _ofs, _val) \
    metal_io_write((_io), (_ofs), (_val), memory_order_seq_cst, 2)

#define metal_io_read32_explicit(_io, _ofs, _order) \
    metal_io_read((_io), (_ofs), (_order), 4)
#define metal_io_read32(_io, _ofs) \
    metal_io_read((_io), (_ofs), memory_order_seq_cst, 4)
#define metal_io_write32_explicit(_io, _ofs, _val, _order) \
    metal_io_write((_io), (_ofs), (_val), (_order), 4)
#define metal_io_write32(_io, _ofs, _val) \
    metal_io_write((_io), (_ofs), (_val), memory_order_seq_cst, 4)

#define metal_io_read64_explicit(_io, _ofs, _order) \
    metal_io_read((_io), (_ofs), (_order), 8)
#define metal_io_read64(_io, _ofs) \
    metal_io_read((_io), (_ofs), memory_order_seq_cst, 8)
#define metal_io_write64_explicit(_io, _ofs, _val, _order) \
    metal_io_write((_io), (_ofs), (_val), (_order), 8)
#define metal_io_write64(_io, _ofs, _val) \
    metal_io_write((_io), (_ofs), (_val), memory_order_seq_cst, 8)

int metal_io_block_read(struct metal_io_region *io, unsigned long offset,
                        void *restrict dst, int len);
int metal_io_block_write(struct metal_io_region *io, unsigned long offset,
                         const void *restrict src, int len);
int metal_io_block_set(struct metal_io_region *io, unsigned long offset,
                       unsigned char value, int len);

#ifdef __cplusplus
}
#endif

#include <metal/system/alios/io.h>

#endif /* __METAL_IO__H__ */
