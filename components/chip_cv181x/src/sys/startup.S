/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

 /******************************************************************************
 * @file     startup.S
 * @brief    startup file. Should use with
 *           GCC for RISC-V Embedded Processors
 * @version  V1.0
 * @date     29. July 2019
 ******************************************************************************/

#include <csi_config.h>

.globl  Reset_Handler

#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
.section .vectors
    .align  6
    .globl  __sVectors
    .type   __sVectors, @object
__sVectors:
    j   sDefault_Riscv_Handler				/* 0 */
    j   Stspend_Handler			/* 1 */
    j   sDefault_Riscv_Handler				/* 2 */
    j   Mtspend_Handler			/* 3 */
    j   sDefault_Riscv_Handler				/* 4 */
    j   Scoret_Handler			/* 5 */
    j   sDefault_Riscv_Handler				/* 6 */
    j   Mcoret_Handler			/* 7 */
    j   sDefault_Riscv_Handler				/* 8 */
    j   Sirq_Handler			/* 9 */
    j   sDefault_Riscv_Handler				/* 10 */
    j   Mirq_Handler			/* 11 */
    j   sDefault_Riscv_Handler
    j   sDefault_Riscv_Handler
    j   sDefault_Riscv_Handler
    j   sDefault_Riscv_Handler
#endif

.section .vectors
    .align  6
    .globl  __mVectors
    .type   __mVectors, @object
__mVectors:
    j   mDefault_Riscv_Handler				/* 0 */
    j   Stspend_Handler			/* 1 */
    j   mDefault_Riscv_Handler				/* 2 */
    j   Mtspend_Handler			/* 3 */
    j   mDefault_Riscv_Handler				/* 4 */
    j   Scoret_Handler			/* 5 */
    j   mDefault_Riscv_Handler				/* 6 */
    j   Mcoret_Handler			/* 7 */
    j   mDefault_Riscv_Handler				/* 8 */
    j   Sirq_Handler			/* 9 */
    j   mDefault_Riscv_Handler				/* 10 */
    j   Mirq_Handler			/* 11 */
    j   mDefault_Riscv_Handler
    j   mDefault_Riscv_Handler
    j   mDefault_Riscv_Handler
    j   mDefault_Riscv_Handler


 .text
    .align  2
    j        Reset_Handler
    .align   2
    .long    0x594B5343 /* CSKY ASCII */
    .long    0x594B5343 /* CSKY ASCII */
    .align   2
    .rept    9
    .long    0
    .endr
    .long    Reset_Handler
_start:
    .type   Reset_Handler, %function
Reset_Handler:
.option push
.option norelax
    /* disable ie and clear all interrupts */
    csrw    sie, zero
    csrw    sip, zero
    csrw    mie, zero
    csrw    mip, zero

    /* Disable MIE to avoid triggering interrupts before the first task starts. */
    /* This bit is set when a task recovers context. */
    li      a0, 0x00000008UL        /*  Machine Interrupt Disable */
    csrc    mstatus, a0

    la      gp, __global_pointer$
.option pop
    /*
     * enable thead ISA extension:
     * THEADISAEE = 1, MM = 1,
     */
    csrr    t0, mxstatus
    li      t1, (1 << 22) | (1 << 15)
    or      t1, t1, t0
    csrw    mxstatus, t1

    /* FP: initial state */
    csrr    t0, mstatus
    li      t1, ~0x6000
    and     t0, t0, t1
    li      t1, 0x2000
    or      t0, t0, t1
    csrw    mstatus, t0
    /* csrwi   fcsr, 0 */

    /* RVV: initial state */
    li    t0, ~(3 << 23)
    csrr  t1, mstatus
    and   t2, t1, t0
    li    t0, 1 << 23
    or    t2, t2, t0
    csrw  mstatus, t2

    la      a0, __mVectors
    li	    a1, 0x1
    or	    a0, a0,a1
    csrw    mtvec, a0

#ifdef CONFIG_KERNEL_NONE
    la      sp, g_top_mainstack
#else
    la      sp, g_top_irqstack
#endif

#ifndef __NO_SYSTEM_INIT
    la   a0, SystemInit
    jalr a0
#endif

#ifdef CONFIG_KERNEL_NONE
    /* Enable interrupt */
    li    t0, 1 << 3
    csrr  t1, mstatus
    or    t1, t1, t0
    csrw  mstatus, t1
#endif

#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
    la   a0, smode_init
    jalr a0
#endif

    la   a0, pre_main
    jalr a0

    .size   Reset_Handler, . - Reset_Handler

__exit:
    j      __exit


.section .stack

    .align  3
    .global g_base_irqstack
    .global g_top_irqstack
g_base_irqstack:
    .space CONFIG_ARCH_INTERRUPTSTACK
g_top_irqstack:
#ifdef CONFIG_KERNEL_NONE
    .align  3
    .global g_base_mainstack
    .global g_top_mainstack
g_base_mainstack:
    .space CONFIG_ARCH_MAINSTACK
g_top_mainstack:
#endif
