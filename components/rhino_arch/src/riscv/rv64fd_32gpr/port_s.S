#include <k_config.h>
#include <k_arch.h>

#ifdef CONFIG_VIC_TSPDR
#define VIC_TSPDR CONFIG_VIC_TSPDR
#else
#define VIC_TSPDR 0xE4000000
#endif

/******************************************************************************
@                            EXPORT FUNCTIONS
@******************************************************************************/
.extern krhino_task_sched_stats_get
.extern krhino_stack_ovf_check

.global cpu_intrpt_save
.global cpu_intrpt_restore
.global cpu_is_irq_enable
.global cpu_task_switch
.global cpu_intrpt_switch
.global cpu_first_task_start
.global Mtspend_Handler
.global Stspend_Handler

.extern task_restore
/******************************************************************************
@                                 EQUATES
@******************************************************************************/
.equ RISCV_MSTATUS_MIE,        (1<<3)       /*machine-level interrupt bit*/
.equ RISCV_SSTATUS_SIE,        (1<<1)       /*machine-level interrupt bit*/

#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
.equ RISCV_VIC_TSPDR,          (VIC_TSPDR + 0xC000)       /*soft irq register*/
#else
.equ RISCV_VIC_TSPDR,          (VIC_TSPDR)                /*soft irq register*/
#endif

/******************************************************************************
@                        CODE GENERATION DIRECTIVES
@*******************************************************************************/
.text
.align 3

/******************************************************************************
@ Functions:
@     size_t cpu_intrpt_save(void);
@     void cpu_intrpt_restore(size_t cpsr);
@******************************************************************************/
#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
cpu_intrpt_save:
    csrrci a0, MODE_PREFIX(status), RISCV_SSTATUS_SIE
    ret

cpu_is_irq_enable:
    csrr a0, MODE_PREFIX(status)
    andi a0, a0, RISCV_SSTATUS_SIE
    ret
#else
cpu_intrpt_save:
    csrrci a0, MODE_PREFIX(status), RISCV_MSTATUS_MIE
    ret

cpu_is_irq_enable:
    csrr a0, MODE_PREFIX(status)
    andi a0, a0, RISCV_MSTATUS_MIE
    ret
#endif

cpu_intrpt_restore:
    csrw MODE_PREFIX(status), a0
    ret


/******************************************************************************
@ Functions:
@     void cpu_intrpt_switch(void);
@     void cpu_task_switch(void);
@******************************************************************************/
cpu_task_switch:
    li      t0, RISCV_VIC_TSPDR
    li      t2, 0x1
    sw      t2, 0(t0)

__loop:
    lw      t2, 0(t0)
    seqz    t1, t2
#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
    csrr    t2, sip
    andi    t2, t2, 0x2
#else
    csrr    t2, mip
    andi    t2, t2, 0x8
#endif
    snez    t3, t2
    add     t2, t3, t1
    beqz    t2, __loop
    ret

cpu_intrpt_switch:
    j cpu_task_switch

/******************************************************************************
@ Functions:
@     void cpu_first_task_start(void);
@******************************************************************************/
.align 8
cpu_first_task_start:
    j       __task_switch_nosave

/*-----------------------------------------------------------*/

/*
 * Unlike other ports pxPortInitialiseStack() is written in assembly code as it
 * needs access to the portasmADDITIONAL_CONTEXT_SIZE constant.  The prototype
 * for the function is as per the other ports:
 * StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters );
 *
 * As per the standard RISC-V ABI pxTopcOfStack is passed in in a0, pxCode in
 * a1, and pvParameters in a2.  The new top of stack is passed out in a0.
 *
 * RISC-V maps registers to ABI names as follows (X1 to X31 integer registers
 * for the 'I' profile, X1 to X15 for the 'E' profile, currently I assumed).
 *
 * Register		ABI Name	Description						Saver
 * x0			zero		Hard-wired zero					-
 * x1			ra			Return address					Caller
 * x2			sp			Stack pointer					Callee
 * x3			gp			Global pointer					-
 * x4			tp			Thread pointer					-
 * x5-7			t0-2		Temporaries						Caller
 * x8			s0/fp		Saved register/Frame pointer	Callee
 * x9			s1			Saved register					Callee
 * x10-11		a0-1		Function Arguments/return values Caller
 * x12-17		a2-7		Function arguments				Caller
 * x18-27		s2-11		Saved registers					Callee
 * x28-31		t3-6		Temporaries						Caller
 *
 * The RISC-V context is saved t FreeRTOS tasks in the following stack frame,
 * where the global and thread pointers are currently assumed to be constant so
 * are not saved:
 *
 * MODE_PREFIX(status)
 * x31
 * x30
 * x29
 * x28
 * x27
 * x26
 * x25
 * x24
 * x23
 * x22
 * x21
 * x20
 * x19
 * x18
 * x17
 * x16
 * x15
 * x14
 * x13
 * x12
 * x11
 * pvParameters
 * x9
 * x8
 * x7
 * x6
 * x5
 * portTASK_RETURN_ADDRESS
 * [chip specific registers go here]
 * pxCode
 */

/*-----------------------------------------------------------*/

.align 8
Mtspend_Handler:
Stspend_Handler:
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

    csrr    t0, MODE_PREFIX(epc)
    sd      t0, (120+120)(sp)
    csrr    t0, MODE_PREFIX(status)
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

    csrr     t0, vlenb
    slli     t0, t0, 3
    slli     t1, t0, 2
    sub      sp, sp, t1
    vsetvli  zero, zero, e8, m8
#if (__riscv_v == 7000)
    vsb.v    v0, (sp)
    add      sp, sp, t0
    vsb.v    v8, (sp)
    add      sp, sp, t0
    vsb.v    v16, (sp)
    add      sp, sp, t0
    vsb.v    v24, (sp)
#elif (__riscv_v == 1000000)
    vs8r.v   v0, (sp)
    add      sp, sp, t0
    vs8r.v   v8, (sp)
    add      sp, sp, t0
    vs8r.v   v16, (sp)
    add      sp, sp, t0
    vs8r.v   v24, (sp)
#endif
    sub      t0, t1, t0
    sub      sp, sp, t0
#endif /*__riscv_vector*/

    la      a1, g_active_task
    lwu     a1, (a1)
    sw      sp, (a1)

#if (RHINO_CONFIG_TASK_STACK_OVF_CHECK > 0)
    call     krhino_stack_ovf_check
#endif

#if (RHINO_CONFIG_SYS_STATS > 0)
    call     krhino_task_sched_stats_get
#endif

__task_switch_nosave:
    la      a0, g_preferred_ready_task
    la      a1, g_active_task
    lwu     a2, (a0)
    sw      a2, (a1)

    lwu     sp, (a2)

    li      t0, RISCV_VIC_TSPDR
    li      t2, 0x0
    sw      t2, 0(t0)

#ifdef __riscv_vector
    csrr     t0, vlenb
    slli     t0, t0, 3
    vsetvli  zero, zero, e8, m8
#if (__riscv_v == 7000)
    vlb.v    v0, (sp)
    add      sp, sp, t0
    vlb.v    v8, (sp)
    add      sp, sp, t0
    vlb.v    v16, (sp)
    add      sp, sp, t0
    vlb.v    v24, (sp)
    add      sp, sp, t0
#elif (__riscv_v == 1000000)
    vl8r.v   v0, (sp)
    add      sp, sp, t0
    vl8r.v   v8, (sp)
    add      sp, sp, t0
    vl8r.v   v16, (sp)
    add      sp, sp, t0
    vl8r.v   v24, (sp)
    add      sp, sp, t0
#endif
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
#endif /*__riscv_vector*/

    lwu     t0, (124 +124)(sp)
    csrs    MODE_PREFIX(status), t0

    lwu     t0, (120 +120)(sp)
    csrw    MODE_PREFIX(epc), t0

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

    MODE_PREFIX(ret)
