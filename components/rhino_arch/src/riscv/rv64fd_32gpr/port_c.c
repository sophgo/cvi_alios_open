/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <k_arch.h>
#include <csi_core.h>

void *cpu_task_stack_init(cpu_stack_t *stack_base, size_t stack_size,
                          void *arg, task_entry_t entry)
{
    register int *gp asm("x3");
    cpu_stack_t *stk;
#ifdef __riscv_vector
    int vlenb = csi_vlenb_get_value();
#endif

    /* stack aligned by 8 byte */
    stk = (cpu_stack_t *)((uintptr_t)(stack_base + stack_size) & 0xfffffff0);

    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F31         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F30         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F29         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F28         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F27         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F26         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F25         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F24         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F23         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F22         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F21          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F20          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F19         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F18         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F17         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F16         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F15         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F14         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F13         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F12         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F11         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F10         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F9          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F8          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F7          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F6          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F5          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F4          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F3          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F2          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F1          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* F0          */

#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
    /* FS=0b01 SPP=0b01 MPIE=0b10 */
    *(--stk)  = (cpu_stack_t)SR_FS_INITIAL | SR_SPP_S | SR_SPIE; /* sstatus */
#else
    /* FS=0b01 MPP=0b11 MPIE=0b1 */
    *(--stk)  = (cpu_stack_t)SR_FS_INITIAL | SR_MPP_M | SR_MPIE; /* mstatus */
#endif
    *(--stk)  = (cpu_stack_t)entry;                     /* Entry Point */

    *(--stk)  = (cpu_stack_t)0x3131313131313131L;       /* X31         */
    *(--stk)  = (cpu_stack_t)0x3030303030303030L;       /* X30         */
    *(--stk)  = (cpu_stack_t)0x2929292929292929L;       /* X29         */
    *(--stk)  = (cpu_stack_t)0x2828282828282828L;       /* X28         */
    *(--stk)  = (cpu_stack_t)0x2727272727272727L;       /* X27         */
    *(--stk)  = (cpu_stack_t)0x2626262626262626L;       /* X26         */
    *(--stk)  = (cpu_stack_t)0x2525252525252525L;       /* X25         */
    *(--stk)  = (cpu_stack_t)0x2424242424242424L;       /* X24         */
    *(--stk)  = (cpu_stack_t)0x2323232323232323L;       /* X23         */
    *(--stk)  = (cpu_stack_t)0x2222222222222222L;       /* X22         */
    *(--stk)  = (cpu_stack_t)0x2121212121212121L;       /* X21         */
    *(--stk)  = (cpu_stack_t)0x2020202020202020L;       /* X20         */
    *(--stk)  = (cpu_stack_t)0x1919191919191919L;       /* X19         */
    *(--stk)  = (cpu_stack_t)0x1818181818181818L;       /* X18         */
    *(--stk)  = (cpu_stack_t)0x1717171717171717L;       /* X17         */
    *(--stk)  = (cpu_stack_t)0x1616161616161616L;       /* X16         */
    *(--stk)  = (cpu_stack_t)0x1515151515151515L;       /* X15         */
    *(--stk)  = (cpu_stack_t)0x1414141414141414L;       /* X14         */
    *(--stk)  = (cpu_stack_t)0x1313131313131313L;       /* X13         */
    *(--stk)  = (cpu_stack_t)0x1212121212121212L;       /* X12         */
    *(--stk)  = (cpu_stack_t)0x1111111111111111L;       /* X11         */
    *(--stk)  = (cpu_stack_t)arg;                       /* X10         */
    *(--stk)  = (cpu_stack_t)0x0909090909090909L;       /* X9          */
    *(--stk)  = (cpu_stack_t)0x0808080808080808L;       /* X8          */
    *(--stk)  = (cpu_stack_t)0x0707070707070707L;       /* X7          */
    *(--stk)  = (cpu_stack_t)0x0606060606060606L;       /* X6          */
    *(--stk)  = (cpu_stack_t)0x0505050505050505L;       /* X5          */
    *(--stk)  = (cpu_stack_t)0x0404040404040404L;       /* X4          */
    *(--stk)  = (uint64_t)gp;                           /* X3          */
    *(--stk)  = (cpu_stack_t)krhino_task_deathbed;      /* X1          */
#ifdef __riscv_vector
    *(--stk)  = (cpu_stack_t)0x0L;                      /* VXRM        */
    *(--stk)  = (cpu_stack_t)0x0L;                      /* VXSAT       */
    *(--stk)  = (cpu_stack_t)0x0L;                      /* VSTART      */
    *(--stk)  = (cpu_stack_t)0x0L;                      /* VTYPE       */
    *(--stk)  = (cpu_stack_t)0x0L;                      /* VL          */

    if (vlenb == 16) {
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V31         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V31         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V30         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V30         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V29         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V29         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V28         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V28         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V27         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V27         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V26         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V26         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V25         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V25         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V24         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V24         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V23         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V23         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V22         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V22         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V21          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V21          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V20          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V20          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V19         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V19         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V18         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V18         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V17         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V17         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V16         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V16         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V15         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V15         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V14         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V14         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V13         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V13         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V12         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V12         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V11         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V11         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V10         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V10         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V9          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V9          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V8          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V8          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V7          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V7          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V6          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V6          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V5          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V5          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V4          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V4          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V3          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V3          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V2          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V2          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V1          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V1          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V0          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V0          */
    } else if (vlenb == 32) {
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V31         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V31         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V30         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V30         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V29         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V29         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V28         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V28         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V27         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V27         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V26         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V26         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V25         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V25         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V24         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V24         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V23         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V23         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V22         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V22         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V21          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V21          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V20          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V20          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V19         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V19         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V18         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V18         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V17         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V17         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V16         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V16         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V15         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V15         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V14         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V14         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V13         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V13         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V12         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V12         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V11         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V11         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V10         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V10         */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V9          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V9          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V8          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V8          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V7          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V7          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V6          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V6          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V5          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V5          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V4          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V4          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V3          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V3          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V2          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V2          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V1          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V1          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V0          */
    *(--stk)  = (cpu_stack_t)0x1234567812345678L;  *(--stk)  = (cpu_stack_t)0x1234567812345678L;       /* V0          */ 
    }
#endif
    return (void *)stk;
}

