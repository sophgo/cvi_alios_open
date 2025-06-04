/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef K_BITMAP_H
#define K_BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup aos_rhino bitmap
 *  Bit operation.
 *
 *  @{
 */

#define BITMAP_UNIT_SIZE    32U
#define BITMAP_UNIT_MASK    (BITMAP_UNIT_SIZE - 1)
#define BITMAP_UNIT_BITS    5U

#define BITMAP_MASK(nr)     (1UL << (BITMAP_UNIT_MASK - ((nr) & BITMAP_UNIT_MASK)))
#define BITMAP_WORD(nr)     ((nr) >> BITMAP_UNIT_BITS)

/**
 * Count Leading Zeros (clz).
 * Counts the number of zero bits preceding the most significant one bit.
 *
 * @param[in]  x  input unsigned int
 *
 * @return  0~32
 */
RHINO_INLINE uint8_t krhino_clz32(uint32_t x)
{
    uint8_t n = 0;

    if (x == 0) {
        return 32;
    }

#ifdef RHINO_BIT_CLZ
    n = RHINO_BIT_CLZ(x);
#else
    if ((x & 0XFFFF0000) == 0) {
        x <<= 16;
        n += 16;
    }
    if ((x & 0XFF000000) == 0) {
        x <<= 8;
        n += 8;
    }
    if ((x & 0XF0000000) == 0) {
        x <<= 4;
        n += 4;
    }
    if ((x & 0XC0000000) == 0) {
        x <<= 2;
        n += 2;
    }
    if ((x & 0X80000000) == 0) {
        n += 1;
    }
#endif

    return n;
}

/**
 * Count Trailing Zeros (ctz).
 * Counts the number of zero bits succeeding the least significant one bit.
 *
 * @param[in]  x  input unsigned int
 *
 * @return  0~32
 */
RHINO_INLINE uint8_t krhino_ctz32(uint32_t x)
{
    uint8_t n = 0;

    if (x == 0) {
        return 32;
    }

#ifdef RHINO_BIT_CTZ
    n = RHINO_BIT_CTZ(x);
#else
    if ((x & 0X0000FFFF) == 0) {
        x >>= 16;
        n += 16;
    }
    if ((x & 0X000000FF) == 0) {
        x >>= 8;
        n += 8;
    }
    if ((x & 0X0000000F) == 0) {
        x >>= 4;
        n += 4;
    }
    if ((x & 0X00000003) == 0) {
        x >>= 2;
        n += 2;
    }
    if ((x & 0X00000001) == 0) {
        n += 1;
    }
#endif

    return n;
}

/**
 * Set a bit of the bitmap for task priority.
 *
 * @param[in]  bitmap  pointer to the bitmap
 * @param[in]  nr      position of the bitmap to set, from msb to lsb
 *
 * @return  no return
 */
RHINO_INLINE void krhino_bitmap_set(uint32_t *bitmap, int32_t nr)
{
    bitmap[BITMAP_WORD(nr)] |= BITMAP_MASK(nr);
}

/**
 * Clear a bit of the bitmap for task priority.
 *
 * @param[in]  bitmap  pointer to the bitmap
 * @param[in]  nr      position of the bitmap to clear
 *
 * @return  no return
 */
RHINO_INLINE void krhino_bitmap_clear(uint32_t *bitmap, int32_t nr)
{
    bitmap[BITMAP_WORD(nr)] &= ~BITMAP_MASK(nr);
}

/**
 * Find the first bit(1) of the bitmap.
 *
 * @param[in]  bitmap  pointer to the bitmap
 *
 * @return  the first bit position
 */
RHINO_INLINE int32_t krhino_bitmap_first(uint32_t *bitmap)
{
    int32_t  nr  = 0;

    while (*bitmap == 0UL) {
        nr += BITMAP_UNIT_SIZE;
        bitmap++;
    }

    nr += krhino_clz32(*bitmap);

    return nr;
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* K_BITMAP_H */

