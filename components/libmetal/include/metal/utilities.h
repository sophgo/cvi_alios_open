/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_UTILITIES__H__
#define __METAL_UTILITIES__H__

#include <stdint.h>
#include <limits.h>
#include <metal/assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MB
#define MB (1024UL << 10UL)
#endif

#ifndef GB
#define GB (MB << 10UL)
#endif

#define metal_unused(x)    do { (x) = (x); } while (0)
#define metal_dim(x)       sizeof(x) / sizeof(x[0])
#define metal_min(x, y)    ((x) < (y) ? (x) : (y))
#define metal_max(x, y)    ((x) > (y) ? (x) : (y))
#define metal_sign(x)      ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

#define metal_align_down(size, align) \
    ((size) & ~((align) - 1))
#define metal_align_up(size, align) \
    metal_align_down((size) + (align) - 1, align)
#define metal_div_round_down(num, den) \
    ((num) / (den))
#define metal_div_round_up(num, den) \
    metal_div_round_down((num) + (den) - 1, (den))

#define metal_ptr_align_down(ptr, align) \
    (void *)(metal_align_down((uintptr_t)(ptr), (uintptr_t)(align)))
#define metal_ptr_align_up(ptr, align) \
    (void *)(metal_align_up((uintptr_t)(ptr), (uintptr_t)(align)))

#define metal_offset_of(structure, member) \
    ((uintptr_t)&(((structure *)0)->member))
#define metal_container_of(ptr, structure, member) \
    (void *)((uintptr_t)(ptr) - metal_offset_of(structure, member))

#define METAL_BITS_PER_ULONG    (CHAR_BIT * sizeof(unsigned long))

#define metal_bit(bit)          (1UL << (bit))
#define metal_bitmap_longs(x)   metal_div_round_up((x), METAL_BITS_PER_ULONG)

static inline void metal_bitmap_set_bit(unsigned long *bitmap, int bit)
{
    bitmap[bit / METAL_BITS_PER_ULONG] |=
        metal_bit(bit & (METAL_BITS_PER_ULONG - 1));
}

static inline int metal_bitmap_is_bit_set(unsigned long *bitmap, int bit)
{
    return ((bitmap[bit / METAL_BITS_PER_ULONG] &
        metal_bit(bit & (METAL_BITS_PER_ULONG - 1))) == 0) ? 0 : 1;
}

static inline void metal_bitmap_clear_bit(unsigned long *bitmap, int bit)
{
    bitmap[bit / METAL_BITS_PER_ULONG] &=
        ~metal_bit(bit & (METAL_BITS_PER_ULONG - 1));
}

static inline int metal_bitmap_is_bit_clear(unsigned long *bitmap, int bit)
{
    return !metal_bitmap_is_bit_set(bitmap, bit);
}

static inline unsigned int
metal_bitmap_next_set_bit(unsigned long *bitmap, unsigned int start,
                          unsigned int max)
{
    unsigned int bit;

    for (bit = start;
         bit < max && !metal_bitmap_is_bit_set(bitmap, bit);
         bit++)
        ;
    return bit;
}

#define metal_bitmap_for_each_set_bit(bitmap, bit, max) \
    for ((bit) = metal_bitmap_next_set_bit((bitmap), 0, (max)); \
         (bit) < (max); \
         (bit) = metal_bitmap_next_set_bit((bitmap), (bit + 1), (max)))

static inline unsigned int
metal_bitmap_next_clear_bit(unsigned long *bitmap, unsigned int start,
                            unsigned int max)
{
    unsigned int bit;

    for (bit = start;
         bit < max + start && !metal_bitmap_is_bit_clear(bitmap, bit % max);
         bit++)
        ;
    return bit % max;
}

#define metal_bitmap_for_each_clear_bit(bitmap, bit, max) \
    for ((bit) = metal_bitmap_next_clear_bit((bitmap), 0, (max)); \
         (bit) < (max); \
         (bit) = metal_bitmap_next_clear_bit((bitmap), (bit + 1), (max)))

static inline unsigned long metal_log2(unsigned long in)
{
    unsigned long result;

    metal_assert((in & (in - 1)) == 0);

    for (result = 0; (1UL << result) < in; result++)
        ;
    return result;
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_UTILITIES__H__ */
