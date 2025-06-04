/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#ifndef K_COMPILER_H
#define K_COMPILER_H

// #include <time.h>
// size_t strftime(char *s, size_t max, const char *format,
//                        const struct tm *tm);
// struct tm *gmtime (const time_t *_timer);
// struct tm* localtime(const time_t* t);

#if defined(__CC_ARM)

#define RHINO_INLINE                static __inline

/* get the return address of the current function
   unsigned int __return_address(void) */
#define RHINO_GET_RA()              (void *)__return_address()

/* get the  the value of the stack pointer
   unsigned int __current_sp(void) */
#define RHINO_GET_SP()              (void *)__current_sp()

/* Returns the number of leading 0-bits in x,
   starting at the most signifi cant bit position. */
#define RHINO_BIT_CLZ(x)            __builtin_clz(x)

/* Returns the number of trailing 0-bits in x,
   starting at the least signifi cant bit position. */
#define RHINO_BIT_CTZ(x)            __builtin_ctz(x)

#ifndef RHINO_WEAK
#define RHINO_WEAK                  __weak
#endif

#ifndef RHINO_ASM
#define RHINO_ASM                   __asm
#endif

/* Instruction Synchronization Barrier */
#define OS_ISB()                    __isb(15)   /* Full system  Any-Any */

/* Data Memory Barrier */
#define OS_DMB()                    __dmb(15)   /* Full system  Any-Any */

/* Data Synchronization Barrier */
#define OS_DSB()                    __dsb(15)   /* Full system  Any-Any */

#elif defined(__ICCARM__)
#include "intrinsics.h"
#define RHINO_INLINE                static inline
/* get the return address of the current function
   unsigned int __get_LR(void) */
#define RHINO_GET_RA()              (void *)__get_LR()

/* get the  the value of the stack pointer
   unsigned int __get_SP(void) */
#define RHINO_GET_SP()              (void *)__get_SP()

/* Returns the number of leading 0-bits in x,
   starting at the most signifi cant bit position. */
#define RHINO_BIT_CLZ(x)            __CLZ(x)

//#define RHINO_BIT_CTZ(x)

#ifndef RHINO_WEAK
#define RHINO_WEAK                  __weak
#endif

#ifndef RHINO_ASM
#define RHINO_ASM                   asm
#endif

/* Instruction Synchronization Barrier */
#define OS_ISB()                    __isb(15)   /* Full system  Any-Any */

/* Data Memory Barrier */
#define OS_DMB()                    __dmb(15)   /* Full system  Any-Any */

/* Data Synchronization Barrier */
#define OS_DSB()                    __dsb(15)   /* Full system  Any-Any */

#elif defined(__GNUC__)
#define RHINO_INLINE                static inline
/* get the return address of the current function
   void * __builtin_return_address (unsigned int level) */
#define RHINO_GET_RA()              __builtin_return_address(0)

/* get the return address of the current function */
__attribute__((always_inline)) RHINO_INLINE void *RHINO_GET_SP(void)
{
    void *sp;
    __asm__ volatile("mv %0, sp\n":"=r"(sp));
    return sp;
}

/* get the return address of the current function */
__attribute__((always_inline)) RHINO_INLINE void *RHINO_GET_PC(void)
{
    void *pc;
    __asm__ volatile("auipc %0, 0\n":"=r"(pc));
    return pc;
}

/* get the return address of the current function */
__attribute__((always_inline)) RHINO_INLINE void *RHINO_GET_LR(void)
{
    void *lr;
    __asm__ volatile("mv %0, ra\n":"=r"(lr));
    return lr;
}
/* Returns the number of leading 0-bits in x,
   starting at the most signifi cant bit position. */
#define RHINO_BIT_CLZ(x)            __builtin_clz(x)

/* Returns the number of trailing 0-bits in x,
   starting at the least signifi cant bit position. */
#define RHINO_BIT_CTZ(x)            __builtin_ctz(x)

#ifndef RHINO_WEAK
#define RHINO_WEAK                  __attribute__((weak))
#endif

#ifndef RHINO_ASM
#define RHINO_ASM                   __asm__
#endif

/* Instruction Synchronization Barrier */
//#define OS_ISB()                    __asm__ volatile ("isb sy":::"memory")

/* Data Memory Barrier */
//#define OS_DMB()                    __asm__ volatile ("dmb sy":::"memory")

/* Data Synchronization Barrier */
//#define OS_DSB()                    __asm__ volatile ("dsb sy":::"memory")

#else
#error "Unsupported compiler"
#endif

#endif /* K_COMPILER_H */

