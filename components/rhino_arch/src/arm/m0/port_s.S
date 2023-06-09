#include <k_config.h>

@******************************************************************************
@                            EXTERN PARAMETERS
@******************************************************************************
.extern g_active_task
.extern g_preferred_ready_task
.extern krhino_stack_ovf_check
.extern krhino_task_sched_stats_get

@******************************************************************************
@                            EXPORT FUNCTIONS
@******************************************************************************
.global cpu_intrpt_save
.global cpu_intrpt_restore
.global cpu_is_irq_enable
.weak cpu_task_switch
.weak cpu_intrpt_switch
.weak cpu_first_task_start

.global PendSV_Handler

@******************************************************************************
@                                 EQUATES
@******************************************************************************
.equ SCB_ICSR,        0xE000ED04 @ Interrupt Control and State Register.
.equ SCB_VTOR,        0xE000ED08 @ Vector Table Offset Register.
.equ ICSR_PENDSVSET,  0x10000000 @ Value to trigger PendSV exception.

.equ SHPR3_PRI_14_15, 0xE000ED20 @ System Handler Priority Register 3 (PendSV + SysTick).
.equ SHPR3_PRI_LVL,   0xC0C00000 @ PendSV + SysTick priority level (lowest).

@******************************************************************************
@                        CODE GENERATION DIRECTIVES
@******************************************************************************
.text
.align 2
.thumb
.syntax unified

@******************************************************************************
@ Functions:
@     size_t cpu_intrpt_save(void);
@     void cpu_intrpt_restore(size_t cpsr);
@******************************************************************************
.thumb_func
cpu_intrpt_save:
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR

.thumb_func
cpu_intrpt_restore:
    MSR     PRIMASK, R0
    BX      LR

.thumb_func
cpu_is_irq_enable:
    MRS     R0, PRIMASK
    MVN     R0, R0
    BX      LR

@******************************************************************************
@ Functions:
@     void cpu_intrpt_switch(void);
@     void cpu_task_switch(void);
@******************************************************************************
.thumb_func
cpu_task_switch:
    LDR     R0, =SCB_ICSR
    LDR     R1, =ICSR_PENDSVSET
    STR     R1, [R0]
    BX      LR

.thumb_func
cpu_intrpt_switch:
    LDR     R0, =SCB_ICSR
    LDR     R1, =ICSR_PENDSVSET
    STR     R1, [R0]
    BX      LR

@******************************************************************************
@ Functions:
@     void cpu_first_task_start(void);
@******************************************************************************
.thumb_func
cpu_first_task_start:
    @set PendSV prority to the lowest
    LDR     R0, =SHPR3_PRI_14_15
    LDR     R2, [R0]
    LDR     R1, =SHPR3_PRI_LVL
    ORRS    R2, R1
    STR     R2, [R0]

    @indicate PendSV_Handler branch to _pendsv_handler_nosave
    MOVS    R0, #0
    MSR     PSP, R0

    @make PendSV exception pending
    LDR     R0, =SCB_ICSR
    LDR     R1, =ICSR_PENDSVSET
    STR     R1, [R0]

    @goto PendSV_Handler
    CPSIE   I
    B       .

@******************************************************************************
@ Functions:
@     void krhino_pendsv_handler(void);
@******************************************************************************
.thumb_func
PendSV_Handler:
    CPSID   I
    MRS     R0, PSP
    //branch if cpu_first_task_start
    CMP     R0, #0
    BEQ     _first_task_restore

    //hardware saved R0~R3,R12,LR,PC,xPSR

    //save context
    SUBS    R0, R0, #0x24
    STMIA   R0!, {R4-R7}
    MOV     R4, R8
    MOV     R5, R9
    MOV     R6, R10
    MOV     R7, R11
    STMIA   R0!, {R4-R7}
	MOV     R4, LR
    STR     R4, [R0]
    SUBS    R0, R0, #0x20

    //g_active_task->task_stack = context region
    LDR     R1, =g_active_task
    LDR     R1, [R1]
    STR     R0, [R1]

//#if (RHINO_CONFIG_TASK_STACK_OVF_CHECK > 0)
//    BL      krhino_stack_ovf_check
//#endif
//#if (RHINO_CONFIG_SYS_STATS > 0)
//    BL      krhino_task_sched_stats_get
//#endif

.thumb_func
_pendsv_handler_nosave:
    LDR     R0, =g_active_task
    LDR     R1, =g_preferred_ready_task
    LDR     R2, [R1]
    STR     R2, [R0]
    //R0 = g_active_task->task_stack = context region
    LDR     R0, [R2]

    //restore context
    MOV     R1, R0
    ADDS    R0, R0, #16
    LDMIA   R0!, {R4-R7}
    MOV     R8,  R4
    MOV     R9,  R5
    MOV     R10, R6
    MOV     R11, R7
    LDMIA   R1!, {R4-R7}
    LDR     R1, [R0]
	MOV     LR, R1
    ADDS    R0, R0, #4

    //return stack = PSP
    MSR     PSP, R0

    CPSIE   I
    //hardware restore R0~R3,R12,LR,PC,xPSR
    BX      LR

.thumb_func
_first_task_restore:
    B       _pendsv_handler_nosave

.end
