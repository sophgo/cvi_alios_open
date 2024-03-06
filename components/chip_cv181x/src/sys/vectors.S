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
#include <riscv_csr.h>

/* FIXME: used for from S->M mode by cause except 2 */
#define OPCODE_FOR_S_M 0x30401073  //csrw    mie, zero

.section .stack
    .align  3
    .global g_trapstackbase
    .global g_top_trapstack
g_trapstackbase:
    .space CONFIG_ARCH_INTERRUPTSTACK
g_top_trapstack:

.text
.global _interrupt_return_address
#if defined(CONFIG_BENGINE_ENABLE) && CONFIG_BENGINE_ENABLE
.global _pagefault_return_address
#endif

    .align  2
    .global Mcoret_Handler
    .global Scoret_Handler
    .weak   Mcoret_Handler
    .type   Mcoret_Handler, %function
Mcoret_Handler:
Scoret_Handler:
    csrw    MODE_PREFIX(scratch), sp
    la      sp, g_top_irqstack
    addi    sp, sp, -(76+76)
    sd      t0, (4+4)(sp)
    sd      t1, (8+8)(sp)
    sd      t2, (12+12)(sp)

    csrr    t0, MODE_PREFIX(epc)
    sd      t0, (68+68)(sp)
    csrr    t0, MODE_PREFIX(status)
    sd      t0, (72+72)(sp)

    sd      ra, (0 +0 )(sp)
    sd      a0, (16+16)(sp)
    sd      a1, (20+20)(sp)
    sd      a2, (24+24)(sp)
    sd      a3, (28+28)(sp)
    sd      a4, (32+32)(sp)
    sd      a5, (36+36)(sp)
    sd      a6, (40+40)(sp)
    sd      a7, (44+44)(sp)
    sd      t3, (48+48)(sp)
    sd      t4, (52+52)(sp)
    sd      t5, (56+56)(sp)
    sd      t6, (60+60)(sp)

    addi    sp,  sp, -160
    fsd     ft0, (0 +0 )(sp)
    fsd     ft1, (4 +4 )(sp)
    fsd     ft2, (8 +8 )(sp)
    fsd     ft3, (12+12)(sp)
    fsd     ft4, (16+16)(sp)
    fsd     ft5, (20+20)(sp)
    fsd     ft6, (24+24)(sp)
    fsd     ft7, (28+28)(sp)
    fsd     fa0, (32+32)(sp)
    fsd     fa1, (36+36)(sp)
    fsd     fa2, (40+40)(sp)
    fsd     fa3, (44+44)(sp)
    fsd     fa4, (48+48)(sp)
    fsd     fa5, (52+52)(sp)
    fsd     fa6, (56+56)(sp)
    fsd     fa7, (60+60)(sp)
    fsd     ft8, (64+64)(sp)
    fsd     ft9, (68+68)(sp)
    fsd     ft10,(72+72)(sp)
    fsd     ft11,(76+76)(sp)
#ifdef __riscv_vector
    addi    sp, sp, -(20+20)
    csrr    t0, vl
    sd      t0,  (0  +0  )(sp)
    csrr    t0, vtype
    sd      t0,  (4  +4  )(sp)
    csrr    t0, vstart
    sd      t0,  (8  +8  )(sp)
    csrr    t0, vxsat
    sd      t0,  (12 +12 )(sp)
    csrr    t0, vxrm
    sd      t0,  (16 +16 )(sp)

    addi    sp, sp, -(256+256)
    vsetvli zero, zero, e8, m8
    vsb.v   v0, (sp)
    addi    sp, sp, 128
    vsb.v   v8, (sp)
    addi    sp, sp, 128
    vsb.v   v16, (sp)
    addi    sp, sp, 128
    vsb.v   v24, (sp)
    addi    sp, sp, -(256+256-128)
#endif
    la      t2, do_irq
    jalr    t2
#ifdef __riscv_vector
    vsetvli  zero, zero, e8, m8
    vlb.v    v0, (sp)
    addi     sp, sp, 128
    vlb.v    v8, (sp)
    addi     sp, sp, 128
    vlb.v    v16, (sp)
    addi     sp, sp, 128
    vlb.v    v24, (sp)
    addi     sp, sp, 128

    lwu     t0, (0 +0)(sp)
    lwu     t1, (4 +4)(sp)
    lwu     t2, (8 +8)(sp)
    vsetvl  zero, t0, t1
    csrw    vstart, t2
    lwu     t2, (12 +12)(sp)
    csrw    vxsat, t2
    lwu     t2, (16 +16)(sp)
    csrw    vxrm, t2
    addi    sp, sp, (20+20)
#endif
    fld     ft0, (0 +0 )(sp)
    fld     ft1, (4 +4 )(sp)
    fld     ft2, (8 +8 )(sp)
    fld     ft3, (12+12)(sp)
    fld     ft4, (16+16)(sp)
    fld     ft5, (20+20)(sp)
    fld     ft6, (24+24)(sp)
    fld     ft7, (28+28)(sp)
    fld     fa0, (32+32)(sp)
    fld     fa1, (36+36)(sp)
    fld     fa2, (40+40)(sp)
    fld     fa3, (44+44)(sp)
    fld     fa4, (48+48)(sp)
    fld     fa5, (52+52)(sp)
    fld     fa6, (56+56)(sp)
    fld     fa7, (60+60)(sp)
    fld     ft8, (64+64)(sp)
    fld     ft9, (68+68)(sp)
    fld     ft10,(72+72)(sp)
    fld     ft11,(76+76)(sp)

    addi    sp, sp, 160

    ld      t0, (72+72)(sp)
    csrw    MODE_PREFIX(status), t0

    ld      t0, (68+68)(sp)
    csrw    MODE_PREFIX(epc), t0
    ld      ra, (0 +0 )(sp)
    ld      t0, (4 +4 )(sp)
    ld      t1, (8 +8 )(sp)
    ld      t2, (12+12)(sp)
    ld      a0, (16+16)(sp)
    ld      a1, (20+20)(sp)
    ld      a2, (24+24)(sp)
    ld      a3, (28+28)(sp)
    ld      a4, (32+32)(sp)
    ld      a5, (36+36)(sp)
    ld      a6, (40+40)(sp)
    ld      a7, (44+44)(sp)
    ld      t3, (48+48)(sp)
    ld      t4, (52+52)(sp)
    ld      t5, (56+56)(sp)
    ld      t6, (60+60)(sp)

    addi    sp, sp, (76+76)
    csrr    sp, MODE_PREFIX(scratch)
    MODE_PREFIX(ret)



    .align  2
    .global Mirq_Handler
    .global Sirq_Handler
    .weak   Mirq_Handler
    .type   Mirq_Handler, %function
Mirq_Handler:
Sirq_Handler:
    csrw    MODE_PREFIX(scratch), sp
    la      sp, g_top_irqstack
    addi    sp, sp, -(76+76)
    sd      t0, (4+4)(sp)
    sd      t1, (8+8)(sp)
    sd      t2, (12+12)(sp)

    csrr    t0, MODE_PREFIX(epc)
    sd      t0, (68+68)(sp)
    sd      t2, (72+72)(sp)
    sd      ra, (0 +0 )(sp)
    sd      a0, (16+16)(sp)
    sd      a1, (20+20)(sp)
    sd      a2, (24+24)(sp)
    sd      a3, (28+28)(sp)
    sd      a4, (32+32)(sp)
    sd      a5, (36+36)(sp)
    sd      a6, (40+40)(sp)
    sd      a7, (44+44)(sp)
    sd      t3, (48+48)(sp)
    sd      t4, (52+52)(sp)
    sd      t5, (56+56)(sp)
    sd      t6, (60+60)(sp)

    addi    sp,  sp, -160
    fsd     ft0, (0 +0 )(sp)
    fsd     ft1, (4 +4 )(sp)
    fsd     ft2, (8 +8 )(sp)
    fsd     ft3, (12+12)(sp)
    fsd     ft4, (16+16)(sp)
    fsd     ft5, (20+20)(sp)
    fsd     ft6, (24+24)(sp)
    fsd     ft7, (28+28)(sp)
    fsd     fa0, (32+32)(sp)
    fsd     fa1, (36+36)(sp)
    fsd     fa2, (40+40)(sp)
    fsd     fa3, (44+44)(sp)
    fsd     fa4, (48+48)(sp)
    fsd     fa5, (52+52)(sp)
    fsd     fa6, (56+56)(sp)
    fsd     fa7, (60+60)(sp)
    fsd     ft8, (64+64)(sp)
    fsd     ft9, (68+68)(sp)
    fsd     ft10,(72+72)(sp)
    fsd     ft11,(76+76)(sp)

    addi    sp, sp, -(20+20)
    csrr    t0, vl
    sd      t0,  (0  +0  )(sp)
    csrr    t0, vtype
    sd      t0,  (4  +4  )(sp)
    csrr    t0, vstart
    sd      t0,  (8  +8  )(sp)
    csrr    t0, vxsat
    sd      t0,  (12 +12 )(sp)
    csrr    t0, vxrm
    sd      t0,  (16 +16 )(sp)

    addi     sp, sp, -(256+256)
    vsetvli  zero, zero, e8, m8
    vsb.v    v0, (sp)
    addi     sp, sp, 128
    vsb.v    v8, (sp)
    addi     sp, sp, 128
    vsb.v    v16, (sp)
    addi     sp, sp, 128
    vsb.v    v24, (sp)
    addi     sp, sp, -(256+256-128)

    la      t2, do_irq
    jalr    t2
_interrupt_return_address:
    vsetvli  zero, zero, e8, m8
    vlb.v    v0, (sp)
    addi     sp, sp, 128
    vlb.v    v8, (sp)
    addi     sp, sp, 128
    vlb.v    v16, (sp)
    addi     sp, sp, 128
    vlb.v    v24, (sp)
    addi     sp, sp, 128
    
    lwu     t0, (0 +0)(sp)
    lwu     t1, (4 +4)(sp)
    lwu     t2, (8 +8)(sp)
    vsetvl  zero, t0, t1
    csrw    vstart, t2
    lwu     t2, (12 +12)(sp)
    csrw    vxsat, t2
    lwu     t2, (16 +16)(sp)
    csrw    vxrm, t2
    addi    sp, sp, (20+20)

    fld     ft0, (0 +0 )(sp)
    fld     ft1, (4 +4 )(sp)
    fld     ft2, (8 +8 )(sp)
    fld     ft3, (12+12)(sp)
    fld     ft4, (16+16)(sp)
    fld     ft5, (20+20)(sp)
    fld     ft6, (24+24)(sp)
    fld     ft7, (28+28)(sp)
    fld     fa0, (32+32)(sp)
    fld     fa1, (36+36)(sp)
    fld     fa2, (40+40)(sp)
    fld     fa3, (44+44)(sp)
    fld     fa4, (48+48)(sp)
    fld     fa5, (52+52)(sp)
    fld     fa6, (56+56)(sp)
    fld     fa7, (60+60)(sp)
    fld     ft8, (64+64)(sp)
    fld     ft9, (68+68)(sp)
    fld     ft10,(72+72)(sp)
    fld     ft11,(76+76)(sp)

    addi    sp, sp, 160

    ld      t0, (68+68)(sp)
    csrw    MODE_PREFIX(epc), t0
    ld      ra, (0 +0 )(sp)
    ld      t0, (4 +4 )(sp)
    ld      t1, (8 +8 )(sp)
    ld      t2, (12+12)(sp)
    ld      a0, (16+16)(sp)
    ld      a1, (20+20)(sp)
    ld      a2, (24+24)(sp)
    ld      a3, (28+28)(sp)
    ld      a4, (32+32)(sp)
    ld      a5, (36+36)(sp)
    ld      a6, (40+40)(sp)
    ld      a7, (44+44)(sp)
    ld      t3, (48+48)(sp)
    ld      t4, (52+52)(sp)
    ld      t5, (56+56)(sp)
    ld      t6, (60+60)(sp)

    addi    sp, sp, (76+76)
    csrr    sp, MODE_PREFIX(scratch)
    MODE_PREFIX(ret)


/******************************************************************************
 * Functions:
 *     void strap(void);
 * default exception handler
 ******************************************************************************/
    .align  2
    .global strap
    .type   strap, %function
strap:
    csrw    sscratch, sp
    la      sp, g_top_trapstack
    addi    sp, sp, -(140+140)
    sd      x1, ( 0 + 0 )(sp)
    sd      x3, ( 8 + 8 )(sp)
    sd      x4, ( 12+ 12)(sp)
    sd      x5, ( 16+ 16)(sp)
    sd      x6, ( 20+ 20)(sp)
    sd      x7, ( 24+ 24)(sp)
    sd      x8, ( 28+ 28)(sp)
    sd      x9, ( 32+ 32)(sp)
    sd      x10,( 36+ 36)(sp)
    sd      x11,( 40+ 40)(sp)
    sd      x12,( 44+ 44)(sp)
    sd      x13,( 48+ 48)(sp)
    sd      x14,( 52+ 52)(sp)
    sd      x15,( 56+ 56)(sp)
    sd      x16,( 60+ 60)(sp)
    sd      x17,( 64+ 64)(sp)
    sd      x18,( 68+ 68)(sp)
    sd      x19,( 72+ 72)(sp)
    sd      x20,( 76+ 76)(sp)
    sd      x21,( 80+ 80)(sp)
    sd      x22,( 84+ 84)(sp)
    sd      x23,( 88+ 88)(sp)
    sd      x24,( 92+ 92)(sp)
    sd      x25,( 96+ 96)(sp)
    sd      x26,(100+100)(sp)
    sd      x27,(104+104)(sp)
    sd      x28,(108+108)(sp)
    sd      x29,(112+112)(sp)
    sd      x30,(116+116)(sp)
    sd      x31,(120+120)(sp)
    csrr    a0, sepc
    sd      a0, (124+124)(sp)
    csrr    a0, sstatus
    sd      a0, (128+128)(sp)
    csrr    a0, scause
    sd      a0, (132+132)(sp)
    csrr    a0, stval
    sd      a0, (136+136)(sp)
    csrr    a0, sscratch
    sd      a0, ( 4 + 4 )(sp)

    mv      a0, sp
    la      a1, exceptionHandler
    jalr    a1

    .align  2
    .global mtrap
    .type   mtrap, %function
mtrap:
    csrw    mscratch, sp
    la      sp, g_top_trapstack
    addi    sp, sp, -(140+140)
    sd      x1, ( 0 + 0 )(sp)
    sd      x3, ( 8 + 8 )(sp)
    sd      x4, ( 12+ 12)(sp)
    sd      x5, ( 16+ 16)(sp)
    sd      x6, ( 20+ 20)(sp)
    sd      x7, ( 24+ 24)(sp)
    sd      x8, ( 28+ 28)(sp)
    sd      x9, ( 32+ 32)(sp)
    sd      x10,( 36+ 36)(sp)
    sd      x11,( 40+ 40)(sp)
    sd      x12,( 44+ 44)(sp)
    sd      x13,( 48+ 48)(sp)
    sd      x14,( 52+ 52)(sp)
    sd      x15,( 56+ 56)(sp)
    sd      x16,( 60+ 60)(sp)
    sd      x17,( 64+ 64)(sp)
    sd      x18,( 68+ 68)(sp)
    sd      x19,( 72+ 72)(sp)
    sd      x20,( 76+ 76)(sp)
    sd      x21,( 80+ 80)(sp)
    sd      x22,( 84+ 84)(sp)
    sd      x23,( 88+ 88)(sp)
    sd      x24,( 92+ 92)(sp)
    sd      x25,( 96+ 96)(sp)
    sd      x26,(100+100)(sp)
    sd      x27,(104+104)(sp)
    sd      x28,(108+108)(sp)
    sd      x29,(112+112)(sp)
    sd      x30,(116+116)(sp)
    sd      x31,(120+120)(sp)
    csrr    a0, mepc
    sd      a0, (124+124)(sp)
    csrr    a0, mstatus
    sd      a0, (128+128)(sp)
    csrr    a0, mcause
    sd      a0, (132+132)(sp)
    csrr    a0, mtval
    sd      a0, (136+136)(sp)
    csrr    a0, mscratch
    sd      a0, ( 4 + 4 )(sp)

    mv      a0, sp
    la      a1, exceptionHandler
    jalr    a1


    .align  6
    .weak   mDefault_Riscv_Handler
    .global mDefault_Riscv_Handler
    .type   mDefault_Riscv_Handler, %function
mDefault_Riscv_Handler:
#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
    csrw    mscratch, sp
    la      sp, g_top_trapstack
    addi    sp, sp, -(8+8)
    sd      a0, (0+0)(sp)
    sd      t0, (4+4)(sp)
    csrr    t0, mcause
    li      a0, CAUSE_ILLEGAL_INSTRUCTION
    bne     t0, a0, _default_trap 
_prepare_switch:
    csrr    t0, mtval
    li      a0, OPCODE_FOR_S_M
    beq     t0, a0, _system_switchto_mmode /* no return */
_default_trap:
    ld      a0, (0+0)(sp)
    ld      t0, (4+4)(sp)
    csrr    sp, mscratch
#endif
    j       mtrap


#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
    .align  6
    .weak   sDefault_Riscv_Handler
    .global sDefault_Riscv_Handler
    .type   sDefault_Riscv_Handler, %function
sDefault_Riscv_Handler:
#if defined(CONFIG_BENGINE_ENABLE) && CONFIG_BENGINE_ENABLE
    csrw    sscratch, sp
    la      sp, g_top_trapstack
    addi    sp, sp, -(8+8)
    sd      a0, (0+0)(sp)
    sd      t0, (4+4)(sp)

    li      a0, CAUSE_STORE_PAGE_FAULT
    csrr    t0, scause
    beq     t0, a0, pre_asm_page_fault
    li      a0, CAUSE_LOAD_PAGE_FAULT
    beq     t0, a0, pre_asm_page_fault
    li      a0, CAUSE_FETCH_PAGE_FAULT
    beq     t0, a0, pre_asm_page_fault

pre_strap:
    ld      a0, (0+0)(sp)
    ld      t0, (4+4)(sp)
    csrr    sp, sscratch
    j       strap
_pagefault_return_address:
    j      _pagefault_return_address

pre_asm_page_fault:
    la      a0, g_kernel_elf_post_prepare_ok
    lwu     a0, (a0)
    beqz    a0, pre_strap
    la      a0, g_irq_nested_level
    lwu     a0, (a0)
    bnez    a0, pre_strap

asm_page_fault:
    ld      a0, (0+0)(sp)
    ld      t0, (4+4)(sp)
    csrr    sp, sscratch


    addi     sp, sp, -(128+128)
    fsd      f31, (0  +0  )(sp)
    fsd      f30, (4  +4  )(sp)
    fsd      f29, (8  +8  )(sp)
    fsd      f28, (12 +12 )(sp)
    fsd      f27, (16 +16 )(sp)
    fsd      f26, (20 +20 )(sp)
    fsd      f25, (24 +24 )(sp)
    fsd      f24, (28 +28 )(sp)
    fsd      f23, (32 +32 )(sp)
    fsd      f22, (36 +36 )(sp)
    fsd      f21, (40 +40 )(sp)
    fsd      f20, (44 +44 )(sp)
    fsd      f19, (48 +48 )(sp)
    fsd      f18, (52 +52 )(sp)
    fsd      f17, (56 +56 )(sp)
    fsd      f16, (60 +60 )(sp)
    fsd      f15, (64 +64 )(sp)
    fsd      f14, (68 +68 )(sp)
    fsd      f13, (72 +72 )(sp)
    fsd      f12, (76 +76 )(sp)
    fsd      f11, (80 +80 )(sp)
    fsd      f10, (84 +84 )(sp)
    fsd      f9,  (88 +88 )(sp)
    fsd      f8,  (92 +92 )(sp)
    fsd      f7,  (96 +96 )(sp)
    fsd      f6,  (100+100)(sp)
    fsd      f5,  (104+104)(sp)
    fsd      f4,  (108+108)(sp)
    fsd      f3,  (112+112)(sp)
    fsd      f2,  (116+116)(sp)
    fsd      f1,  (120+120)(sp)
    fsd      f0,  (124+124)(sp)

    addi    sp, sp, -(128+128)

    sd      x1,  (0  +0  )(sp)
    sd      x3,  (4  +4  )(sp)
    sd      x4,  (8  +8  )(sp)
    sd      x5,  (12 +12 )(sp)
    sd      x6,  (16 +16 )(sp)
    sd      x7,  (20 +20 )(sp)
    sd      x8,  (24 +24 )(sp)
    sd      x9,  (28 +28 )(sp)
    sd      x10, (32 +32 )(sp)
    sd      x11, (36 +36 )(sp)
    sd      x12, (40 +40 )(sp)
    sd      x13, (44 +44 )(sp)
    sd      x14, (48 +48 )(sp)
    sd      x15, (52 +52 )(sp)
    sd      x16, (56 +56 )(sp)
    sd      x17, (60 +60 )(sp)
    sd      x18, (64 +64 )(sp)
    sd      x19, (68 +68 )(sp)
    sd      x20, (72 +72 )(sp)
    sd      x21, (76 +76 )(sp)
    sd      x22, (80 +80 )(sp)
    sd      x23, (84 +84 )(sp)
    sd      x24, (88 +88 )(sp)
    sd      x25, (92 +92 )(sp)
    sd      x26, (96 +96 )(sp)
    sd      x27, (100+100)(sp)
    sd      x28, (104+104)(sp)
    sd      x29, (108+108)(sp)
    sd      x30, (112+112)(sp)
    sd      x31, (116+116)(sp)

    csrr    t0, sepc
    sd      t0, (120+120)(sp)
    csrr    t0, sstatus
    sd      t0, (124+124)(sp)
#ifdef __riscv_vector
    addi    sp, sp, -(20+20)
    csrr    t0, vl
    sd      t0,  (0  +0  )(sp)
    csrr    t0, vtype
    sd      t0,  (4  +4  )(sp)
    csrr    t0, vstart
    sd      t0,  (8  +8  )(sp)
    csrr    t0, vxsat
    sd      t0,  (12 +12 )(sp)
    csrr    t0, vxrm
    sd      t0,  (16 +16 )(sp)

    addi     sp, sp, -(256+256)
    vsetvli  zero, zero, e8, m8
    vsb.v    v0, (sp)
    addi     sp, sp, 128
    vsb.v    v8, (sp)
    addi     sp, sp, 128
    vsb.v    v16, (sp)
    addi     sp, sp, 128
    vsb.v    v24, (sp)
    addi     sp, sp, -(256+256-128)
#endif
    la      a1, g_active_task
    lwu     a1, (a1)
    sw      sp, (a1)

    la      sp, g_top_trapstack
    csrr    a0, stval
    la      t0, bengine_dload
    jalr    t0

    la      a0, g_preferred_ready_task
    la      a1, g_active_task
    lwu     a2, (a0)
    sw      a2, (a1)
    lwu     sp, (a2)

#ifdef __riscv_vector
    vsetvli  zero, zero, e8, m8
    vlb.v    v0, (sp)
    addi     sp, sp, 128
    vlb.v    v8, (sp)
    addi     sp, sp, 128
    vlb.v    v16, (sp)
    addi     sp, sp, 128
    vlb.v    v24, (sp)
    addi     sp, sp, 128

    lwu     t0, (0 +0)(sp)
    lwu     t1, (4 +4)(sp)
    lwu     t2, (8 +8)(sp)
    vsetvl  zero, t0, t1
    csrw    vstart, t2
    lwu     t2, (12 +12)(sp)
    csrw    vxsat, t2
    lwu     t2, (16 +16)(sp)
    csrw    vxrm, t2
    addi    sp, sp, (20+20)
#endif
    lwu     t0, (124 +124)(sp)
    csrs    sstatus, t0

    lwu     t0, (120 +120)(sp)
    csrw    sepc, t0

    ld     x1,  (0  +0  )(sp)
    ld     x3,  (4  +4  )(sp)
    ld     x4,  (8  +8  )(sp)
    ld     x5,  (12 +12 )(sp)
    ld     x6,  (16 +16 )(sp)
    ld     x7,  (20 +20 )(sp)
    ld     x8,  (24 +24 )(sp)
    ld     x9,  (28 +28 )(sp)
    ld     x10, (32 +32 )(sp)
    ld     x11, (36 +36 )(sp)
    ld     x12, (40 +40 )(sp)
    ld     x13, (44 +44 )(sp)
    ld     x14, (48 +48 )(sp)
    ld     x15, (52 +52 )(sp)
    ld     x16, (56 +56 )(sp)
    ld     x17, (60 +60 )(sp)
    ld     x18, (64 +64 )(sp)
    ld     x19, (68 +68 )(sp)
    ld     x20, (72 +72 )(sp)
    ld     x21, (76 +76 )(sp)
    ld     x22, (80 +80 )(sp)
    ld     x23, (84 +84 )(sp)
    ld     x24, (88 +88 )(sp)
    ld     x25, (92 +92 )(sp)
    ld     x26, (96 +96 )(sp)
    ld     x27, (100+100)(sp)
    ld     x28, (104+104)(sp)
    ld     x29, (108+108)(sp)
    ld     x30, (112+112)(sp)
    ld     x31, (116+116)(sp)

    addi    sp, sp, (128+128)

    fld      f31,( 0 + 0 )(sp)
    fld      f30,( 4 + 4 )(sp)
    fld      f29,( 8 + 8 )(sp)
    fld      f28,( 12+ 12)(sp)
    fld      f27,( 16+ 16)(sp)
    fld      f26,( 20+ 20)(sp)
    fld      f25,( 24+ 24)(sp)
    fld      f24,( 28+ 28)(sp)
    fld      f23,( 32+ 32)(sp)
    fld      f22,( 36+ 36)(sp)
    fld      f21,( 40+ 40)(sp)
    fld      f20,( 44+ 44)(sp)
    fld      f19,( 48+ 48)(sp)
    fld      f18,( 52+ 52)(sp)
    fld      f17,( 56+ 56)(sp)
    fld      f16,( 60+ 60)(sp)
    fld      f15,( 64+ 64)(sp)
    fld      f14,( 68+ 68)(sp)
    fld      f13,( 72+ 72)(sp)
    fld      f12,( 76+ 76)(sp)
    fld      f11,( 80+ 80)(sp)
    fld      f10,( 84+ 84)(sp)
    fld      f9, ( 88+ 88)(sp)
    fld      f8, ( 92+ 92)(sp)
    fld      f7, ( 96+ 96)(sp)
    fld      f6, (100+100)(sp)
    fld      f5, (104+104)(sp)
    fld      f4, (108+108)(sp)
    fld      f3, (112+112)(sp)
    fld      f2, (116+116)(sp)
    fld      f1, (120+120)(sp)
    fld      f0, (124+124)(sp)

    addi    sp, sp, (128+128)

    sret
#else
    j       strap
#endif

    .size   sDefault_Riscv_Handler, . - sDefault_Riscv_Handler
#endif

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro  def_irq_handler handler_name
    .weak   \handler_name
    .globl  \handler_name
    .set    \handler_name, mDefault_Riscv_Handler
    .endm

    def_irq_handler Stspend_Handler
    def_irq_handler Mtspend_Handler

