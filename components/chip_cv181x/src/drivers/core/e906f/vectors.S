/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */
 /******************************************************************************
 * @file     vectors.S
 * @brief    define default vector handlers. Should use with
 *           GCC for CSKY Embedded Processors
 * @version  V1.0
 * @date     28. Nove 2017
 ******************************************************************************/

#include <csi_config.h>

/* Enable interrupts when returning from the handler */
#define MSTATUS_PRV1 0x1880

.section .bss
    .align  2
    .globl  g_trapstackalloc
    .global g_trapstackbase
    .global g_top_trapstack
g_trapstackalloc:
g_trapstackbase:
    .space 768
g_top_trapstack:

    .align 2
    .globl g_trap_sp
    .type  g_trap_sp, object
g_trap_sp:
    .long 0
    .size g_trap_sp, .-g_trap_sp

irq_nested_level:
.long 0

.text

#ifdef CONFIG_KERNEL_NONE
    .align  2
    .global Default_IRQHandler
    .weak   Default_IRQHandler
    .type   Default_IRQHandler, %function
Default_IRQHandler:
    addi    sp, sp, -72
    sw      t0, 4(sp)
    sw      t1, 8(sp)
    csrr    t0, mepc
    csrr    t1, mcause
    sw      t1, 64(sp)
    sw      t0, 68(sp)
    csrs    mstatus, 8

    sw      ra, 0(sp)
    sw      t2, 12(sp)
    sw      a0, 16(sp)
    sw      a1, 20(sp)
    sw      a2, 24(sp)
    sw      a3, 28(sp)
    sw      a4, 32(sp)
    sw      a5, 36(sp)
    sw      a6, 40(sp)
    sw      a7, 44(sp)
    sw      t3, 48(sp)
    sw      t4, 52(sp)
    sw      t5, 56(sp)
    sw      t6, 60(sp)

    addi    sp,  sp, -80
    fsw     ft0, 0(sp)
    fsw     ft1, 4(sp)
    fsw     ft2, 8(sp)
    fsw     ft3, 12(sp)
    fsw     ft4, 16(sp)
    fsw     ft5, 20(sp)
    fsw     ft6, 24(sp)
    fsw     ft7, 28(sp)
    fsw     fa0, 32(sp)
    fsw     fa1, 36(sp)
    fsw     fa2, 40(sp)
    fsw     fa3, 44(sp)
    fsw     fa4, 48(sp)
    fsw     fa5, 52(sp)
    fsw     fa6, 56(sp)
    fsw     fa7, 60(sp)
    fsw     ft8, 64(sp)
    fsw     ft9, 68(sp)
    fsw     ft10,72(sp)
    fsw     ft11,76(sp)

    andi    t1, t1, 0x3FF
    slli    t1, t1, 2

    la      t0, do_irq
    jalr    t0

    csrc    mstatus, 8

    lw      a1, 144(sp)
    andi    a0, a1, 0x3FF

    /* clear pending */
    li      a2, 0xE000E100
    add     a2, a2, a0
    lb      a3, 0(a2)
    li      a4, 1
    not     a4, a4
    and     a5, a4, a3
    sb      a5, 0(a2)

    li      t0, MSTATUS_PRV1
    csrs    mstatus, t0
    csrw    mcause, a1

    flw     ft0, 0(sp)
    flw     ft1, 4(sp)
    flw     ft2, 8(sp)
    flw     ft3, 12(sp)
    flw     ft4, 16(sp)
    flw     ft5, 20(sp)
    flw     ft6, 24(sp)
    flw     ft7, 28(sp)
    flw     fa0, 32(sp)
    flw     fa1, 36(sp)
    flw     fa2, 40(sp)
    flw     fa3, 44(sp)
    flw     fa4, 48(sp)
    flw     fa5, 52(sp)
    flw     fa6, 56(sp)
    flw     fa7, 60(sp)
    flw     ft8, 64(sp)
    flw     ft9, 68(sp)
    flw     ft10,72(sp)
    flw     ft11,76(sp)

    addi    sp, sp, 80

    lw      t0, 68(sp)
    csrw    mepc, t0
    lw      ra, 0(sp)
    lw      t0, 4(sp)
    lw      t1, 8(sp)
    lw      t2, 12(sp)
    lw      a0, 16(sp)
    lw      a1, 20(sp)
    lw      a2, 24(sp)
    lw      a3, 28(sp)
    lw      a4, 32(sp)
    lw      a5, 36(sp)
    lw      a6, 40(sp)
    lw      a7, 44(sp)
    lw      t3, 48(sp)
    lw      t4, 52(sp)
    lw      t5, 56(sp)
    lw      t6, 60(sp)

    addi    sp, sp, 72
    mret
#else
    .align  2
    .global Default_IRQHandler
    .weak   Default_IRQHandler
    .type   Default_IRQHandler, %function
Default_IRQHandler:
    addi    sp, sp, -8
    sw      t0, 0(sp)
    sw      t1, 4(sp)

    la      t0, irq_nested_level
    lw      t1, (t0)
    addi    t1, t1, 1
    sw      t1, (t0)
    li      t0, 1
    bgt     t1, t0, .Lnested1

    lw      t0, 0(sp)
    lw      t1, 4(sp)
    addi    sp, sp, 8

    csrw    mscratch, sp
    la      sp, g_top_irqstack
    j       .Lnested2
.Lnested1:
    lw      t0, 0(sp)
    lw      t1, 4(sp)
    addi    sp, sp, 8
.Lnested2:
    addi    sp, sp, -72
    sw      t0, 4(sp)
    sw      t1, 8(sp)
    csrr    t0, mepc
    csrr    t1, mcause
    sw      t1, 64(sp)
    sw      t0, 68(sp)

    csrs    mstatus, 8

    sw      ra, 0(sp)
    sw      t2, 12(sp)
    sw      a0, 16(sp)
    sw      a1, 20(sp)
    sw      a2, 24(sp)
    sw      a3, 28(sp)
    sw      a4, 32(sp)
    sw      a5, 36(sp)
    sw      a6, 40(sp)
    sw      a7, 44(sp)
    sw      t3, 48(sp)
    sw      t4, 52(sp)
    sw      t5, 56(sp)
    sw      t6, 60(sp)

    addi    sp,  sp, -80
    fsw     ft0, 0(sp)
    fsw     ft1, 4(sp)
    fsw     ft2, 8(sp)
    fsw     ft3, 12(sp)
    fsw     ft4, 16(sp)
    fsw     ft5, 20(sp)
    fsw     ft6, 24(sp)
    fsw     ft7, 28(sp)
    fsw     fa0, 32(sp)
    fsw     fa1, 36(sp)
    fsw     fa2, 40(sp)
    fsw     fa3, 44(sp)
    fsw     fa4, 48(sp)
    fsw     fa5, 52(sp)
    fsw     fa6, 56(sp)
    fsw     fa7, 60(sp)
    fsw     ft8, 64(sp)
    fsw     ft9, 68(sp)
    fsw     ft10,72(sp)
    fsw     ft11,76(sp)

    andi    t1, t1, 0x3FF
    slli    t1, t1, 2

    la      t0, do_irq
    jalr    t0

    csrc    mstatus, 8


    lw      a1, 144(sp)
    andi    a0, a1, 0x3FF

    /* clear pending */
    li      a2, 0xE000E100
    add     a2, a2, a0
    lb      a3, 0(a2)
    li      a4, 1
    not     a4, a4
    and     a5, a4, a3
    sb      a5, 0(a2)

    la      t0, irq_nested_level
    lw      t1, (t0)
    addi    t1, t1, -1
    sw      t1, (t0)
    bgt     t1, zero, .Lnested3

    li      t0, MSTATUS_PRV1
    csrs    mstatus, t0
    csrw    mcause, a1
    
    flw     ft0, 0(sp)
    flw     ft1, 4(sp)
    flw     ft2, 8(sp)
    flw     ft3, 12(sp)
    flw     ft4, 16(sp)
    flw     ft5, 20(sp)
    flw     ft6, 24(sp)
    flw     ft7, 28(sp)
    flw     fa0, 32(sp)
    flw     fa1, 36(sp)
    flw     fa2, 40(sp)
    flw     fa3, 44(sp)
    flw     fa4, 48(sp)
    flw     fa5, 52(sp)
    flw     fa6, 56(sp)
    flw     fa7, 60(sp)
    flw     ft8, 64(sp)
    flw     ft9, 68(sp)
    flw     ft10,72(sp)
    flw     ft11,76(sp)

    addi    sp, sp, 80


    lw      t0, 68(sp)
    csrw    mepc, t0
    lw      ra, 0(sp)
    lw      t0, 4(sp)
    lw      t1, 8(sp)
    lw      t2, 12(sp)
    lw      a0, 16(sp)
    lw      a1, 20(sp)
    lw      a2, 24(sp)
    lw      a3, 28(sp)
    lw      a4, 32(sp)
    lw      a5, 36(sp)
    lw      a6, 40(sp)
    lw      a7, 44(sp)
    lw      t3, 48(sp)
    lw      t4, 52(sp)
    lw      t5, 56(sp)
    lw      t6, 60(sp)

    csrr    sp, mscratch
    mret

.Lnested3:
    li      t0, MSTATUS_PRV1
    csrs    mstatus, t0
    csrw    mcause, a1

    flw     ft0, 0(sp)
    flw     ft1, 4(sp)
    flw     ft2, 8(sp)
    flw     ft3, 12(sp)
    flw     ft4, 16(sp)
    flw     ft5, 20(sp)
    flw     ft6, 24(sp)
    flw     ft7, 28(sp)
    flw     fa0, 32(sp)
    flw     fa1, 36(sp)
    flw     fa2, 40(sp)
    flw     fa3, 44(sp)
    flw     fa4, 48(sp)
    flw     fa5, 52(sp)
    flw     fa6, 56(sp)
    flw     fa7, 60(sp)
    flw     ft8, 64(sp)
    flw     ft9, 68(sp)
    flw     ft10,72(sp)
    flw     ft11,76(sp)

    addi    sp, sp, 80


    lw      t0, 68(sp)
    csrw    mepc, t0
    lw      ra, 0(sp)
    lw      t0, 4(sp)
    lw      t1, 8(sp)
    lw      t2, 12(sp)
    lw      a0, 16(sp)
    lw      a1, 20(sp)
    lw      a2, 24(sp)
    lw      a3, 28(sp)
    lw      a4, 32(sp)
    lw      a5, 36(sp)
    lw      a6, 40(sp)
    lw      a7, 44(sp)
    lw      t3, 48(sp)
    lw      t4, 52(sp)
    lw      t5, 56(sp)
    lw      t6, 60(sp)

    addi    sp, sp, 72
    mret
#endif

/******************************************************************************
 * Functions:
 *     void trap(void);
 * default exception handler
 ******************************************************************************/
    .align  2
    .global trap
    .type   trap, %function
trap:
    /* Check for interrupt */
    addi    sp, sp, -4
    sw      t0, 0x0(sp)
    csrr    t0, mcause

    blt     t0, x0, .Lirq

    addi    sp, sp, 4

    la      t0, g_trap_sp
    addi    t0, t0, -132
    sw      x1, 0(t0)
    sw      x2, 4(t0)
    sw      x3, 8(t0)
    sw      x4, 12(t0)
    sw      x6, 20(t0)
    sw      x7, 24(t0)
    sw      x8, 28(t0)
    sw      x9, 32(t0)
    sw      x10, 36(t0)
    sw      x11, 40(t0)
    sw      x12, 44(t0)
    sw      x13, 48(t0)
    sw      x14, 52(t0)
    sw      x15, 56(t0)
    sw      x16, 60(t0)
    sw      x17, 64(t0)
    sw      x18, 68(t0)
    sw      x19, 72(t0)
    sw      x20, 76(t0)
    sw      x21, 80(t0)
    sw      x22, 84(t0)
    sw      x23, 88(t0)
    sw      x24, 92(t0)
    sw      x25, 96(t0)
    sw      x26, 100(t0)
    sw      x27, 104(t0)
    sw      x28, 108(t0)
    sw      x29, 112(t0)
    sw      x30, 116(t0)
    sw      x31, 120(t0)
    csrr    a0, mepc
    sw      a0, 124(t0)
    csrr    a0, mstatus
    sw      a0, 128(t0)

    mv      a0, t0
    lw      t0, -4(sp)
    mv      sp, a0
    sw      t0, 16(sp)

    jal     trap_c


.Lirq:
    lw      t0, 0x0(sp)
    addi    sp, sp, 4
    j       Default_IRQHandler

    .align  6
    .weak   Default_Handler
    .global Default_Handler
    .type   Default_Handler, %function
Default_Handler:
    j      trap
    .size   Default_Handler, . - Default_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro  def_irq_handler handler_name
    .weak   \handler_name
    .globl  \handler_name
    .set    \handler_name, Default_Handler
    .endm

    def_irq_handler tspend_handler
    def_irq_handler CORET_IRQHandler
    def_irq_handler STIM0_IRQHandler
    def_irq_handler STIM1_IRQHandler
    def_irq_handler STIM2_IRQHandler
    def_irq_handler STIM3_IRQHandler
    def_irq_handler TIM0_IRQHandler
    def_irq_handler TIM1_IRQHandler
    def_irq_handler TIM2_IRQHandler
    def_irq_handler TIM3_IRQHandler
    def_irq_handler USART_IRQHandler
    def_irq_handler GPIO0_IRQHandler
    def_irq_handler GPIO1_IRQHandler
    def_irq_handler GPIO2_IRQHandler
    def_irq_handler GPIO3_IRQHandler
    def_irq_handler GPIO4_IRQHandler
    def_irq_handler GPIO5_IRQHandler
    def_irq_handler GPIO6_IRQHandler
    def_irq_handler GPIO7_IRQHandler
    def_irq_handler PAD_IRQHandler
