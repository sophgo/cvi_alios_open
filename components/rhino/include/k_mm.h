/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef K_MM_H
#define K_MM_H


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup aos_rhino mm
 *  Heap memory management.
 *
 *  @{
 */

#if (RHINO_CONFIG_MM_TLF > 0)
#define KMM_ERROR_LOCKED        1
#define KMM_ERROR_UNLOCKED      0

#if (RHINO_CONFIG_MM_DEBUG && (RHINO_CONFIG_MM_TRACE_LVL > 0))
#define KMM_BT_SET_BY_KV        1
#endif

/**
 * Heap useage size statistic
 */
#define K_MM_STATISTIC          1

/**
 * Memory buffer align to MM_ALIGN_SIZE
 */
#define MM_ALIGN_BIT            6
#define MM_ALIGN_SIZE           (1 << MM_ALIGN_BIT)
#define MM_ALIGN_MASK           (MM_ALIGN_SIZE - 1)
#define MM_ALIGN_UP(a)          (((a) + MM_ALIGN_MASK) & ~MM_ALIGN_MASK)
#define MM_ALIGN_DOWN(a)        ((a) & ~MM_ALIGN_MASK)

/**
 * Max size of memory buffer
 */
#define MM_MAX_BIT              RHINO_CONFIG_MM_MAXMSIZEBIT
#define MM_MAX_SIZE             (1 << MM_MAX_BIT)

/**
 * Min size of memory buffer
 */
#define MM_MIN_BIT              RHINO_CONFIG_MM_MINISIZEBIT
#define MM_MIN_SIZE             (1 << (MM_MIN_BIT - 1))

/**
 * Size level of memory buffer
 */
#define MM_BIT_LEVEL            (MM_MAX_BIT - MM_MIN_BIT + 2)

/**
 * Min size of memory heap
 */
#define MM_MIN_HEAP_SIZE        1024

/* magic word for check overwrite or corrupt */
#define MM_DYE_USED             0xFEFE
#define MM_DYE_FREE             0xABAB

#define MM_OWNER_ID_SELF        0xFF

#define MM_CURSTAT_MASK         0x1
#define MM_PRESTAT_MASK         0x2

/* bit 0 */
#define MM_BUFF_FREE            1
#define MM_BUFF_USED            0

/* bit 1 */
#define MM_BUFF_PREV_FREE       2
#define MM_BUFF_PREV_USED       0


/**
 * Buffer head size
 */
#define MMLIST_HEAD_SIZE        (MM_ALIGN_UP(sizeof(k_mm_list_t) - sizeof(free_ptr_t)))
/**
 * Buffer payload size
 */
#define MM_GET_BUF_SIZE(blk)    ((blk)->buf_size & (~MM_ALIGN_MASK))
/**
 * Buffer head + payload size
 */
#define MM_GET_BLK_SIZE(blk)    (MM_GET_BUF_SIZE(blk) + MMLIST_HEAD_SIZE)

/**
 * Get next buffer head addr
 */
#define MM_GET_NEXT_BLK(blk)    ((k_mm_list_t *)((blk)->mbinfo.buffer + MM_GET_BUF_SIZE(blk)))
/**
 * Get this buffer head addr
 */
#define MM_GET_THIS_BLK(buf)    ((k_mm_list_t *)((char *)(buf)-MMLIST_HEAD_SIZE))

#define MM_LAST_BLK_MAGIC       0x11224433

#if (RHINO_CONFIG_MM_REGION_MUTEX == 0)
/**
 * MM critical section strategy:
 * Interrupt mask for single core, and busy-waiting spinlock for multi-core
 */
#define MM_CRITICAL_ENTER(pmmhead,flags_cpsr) krhino_spin_lock_irq_save(&(pmmhead->mm_lock),flags_cpsr);
#define MM_CRITICAL_EXIT(pmmhead,flags_cpsr)  krhino_spin_unlock_irq_restore(&(pmmhead->mm_lock),flags_cpsr);
#else /* (RHINO_CONFIG_MM_REGION_MUTEX != 0) */
/**
 * MM critical section strategy:
 * Task blocked
 */
#define MM_CRITICAL_ENTER(pmmhead,flags_cpsr)                        \
    do {                                                             \
        (void)flags_cpsr;                                            \
        CPSR_ALLOC();                                                \
        RHINO_CRITICAL_ENTER();                                      \
        if (g_intrpt_nested_level[cpu_cur_get()] > 0u) {             \
            k_err_proc(RHINO_NOT_CALLED_BY_INTRPT);                  \
        }                                                            \
        RHINO_CRITICAL_EXIT();                                       \
        krhino_mutex_lock(&(pmmhead->mm_mutex), RHINO_WAIT_FOREVER); \
    } while (0);
#define MM_CRITICAL_EXIT(pmmhead,flags_cpsr)                         \
    do {                                                             \
        (void)flags_cpsr;                                            \
        krhino_mutex_unlock(&(pmmhead->mm_mutex));                   \
    } while (0);
#endif

/**
 * free buffer list
 */
typedef struct free_ptr_struct {
    struct k_mm_list_struct *prev;
    struct k_mm_list_struct *next;
} free_ptr_t;

/**
 * memory buffer head
 */
#define ALIGN_UP_2(a) ((a % 2 == 0) ? a : (a + 1))

typedef struct k_mm_list_struct {
// #if (RHINO_CONFIG_MM_DEBUG > 0)
    uint16_t dye;
    uint8_t owner_id;
    uint8_t trace_id;
    size_t owner;
#if (RHINO_CONFIG_MM_TRACE_LVL > 0)
    void  *trace[ALIGN_UP_2(RHINO_CONFIG_MM_TRACE_LVL)];
#endif
// #endif
    struct k_mm_list_struct *prev;
    /**<
     * buffer payload size, and:
     * bit 0 indicates whether the block is used and
     * bit 1 allows to know whether the previous block is free
     */
    size_t buf_size;
    union {
        free_ptr_t free_ptr;    /**< when buffer is free, add to free list */
        uint8_t    buffer[1];   /**< when buffer is alloced, payload start */
    } mbinfo;
} k_mm_list_t;

/**
 * memory region info
 * Heap can contain multiple regoins
 */
typedef struct k_mm_region_info_struct {
    k_mm_list_t                    *end;
    struct k_mm_region_info_struct *next;
} k_mm_region_info_t;

/**
 * memory heap info
 * heap contains:
 * ---------------------------------------------------------------------------
 * | k_mm_head | k_mm_list_t | k_mm_region_info_t | free space | k_mm_list_t |
 * ---------------------------------------------------------------------------
 */
typedef struct {
#if (RHINO_CONFIG_MM_REGION_MUTEX > 0)
    kmutex_t mm_mutex;
#else
    kspinlock_t mm_lock;
#endif

    k_mm_region_info_t *regioninfo; /**< Heap can contain multiple regoins */

#if (RHINO_CONFIG_MM_BLK > 0)
    void *fix_pool;                 /**< heap can contain one fix pool, deal with little buffer */
#endif

#if (K_MM_STATISTIC > 0)
    size_t used_size;
    size_t maxused_size;
    size_t free_size;
    size_t alloc_times[MM_BIT_LEVEL]; /* number of times for each TLF level */
#endif
    /**< msb (MM_BIT_LEVEL-1) <-> lsb 0, one bit match one freelist */
    uint32_t free_bitmap;
    /**<
     * freelist[N]: contain free blks at level N,
     * 2^(N + MM_MIN_BIT) <= level N buffer size < 2^(1 + N + MM_MIN_BIT)
     */
    k_mm_list_t *freelist[MM_BIT_LEVEL];
} k_mm_head;

/**
 * internal funcs
 */
kstat_t krhino_init_mm_head(k_mm_head **ppmmhead, void *addr, size_t len);
kstat_t krhino_deinit_mm_head(k_mm_head *mmhead);
kstat_t krhino_add_mm_region(k_mm_head *mmhead, void *addr, size_t len);

void *k_mm_alloc(k_mm_head *mmhead, size_t size);
void  k_mm_free(k_mm_head *mmhead, void *ptr);
void *k_mm_realloc(k_mm_head *mmhead, void *oldmem, size_t new_size);

/**
 * Memory buffer allocation.
 *
 * @param[in]  size  size of the mem to malloc
 *
 * @return  buffer address or NULL
 */
void *krhino_mm_alloc(size_t size);

/**
 * Memory buffer free.
 *
 * @param[in]  ptr  buffer address
 *
 * @return  none
 */
void krhino_mm_free(void *ptr);

/**
 * Memory buffer realloc.
 *
 * @param[in]  oldmem   oldmem buffer address
 * @param[in]  newsize  size of the mem to malloc
 *
 * @return  buffer address or NULL
 */
void *krhino_mm_realloc(void *oldmem, size_t newsize);

void *krhino_mm_alloc_resv(size_t size);

void krhino_mm_free_resv(void *ptr);

void *krhino_mm_realloc_resv(void *oldmem, size_t newsize);

/**
 * Get the max free buffer size.
 *
 * @param[in]  NULL
 *
 * @return  the max free buffer size
 */
size_t krhino_mm_max_free_size_get(void);

size_t krhino_mm_max_free_size_get_resv(void);

#else

#include <stdlib.h>

/**
 * do not use os heap management
 */

#define krhino_mm_alloc   malloc
#define krhino_mm_free    free
#define krhino_mm_realloc realloc

#endif /* RHINO_CONFIG_MM_TLF > 0 */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* K_MM_H */

