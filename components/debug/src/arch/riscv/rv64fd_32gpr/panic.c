/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <csi_core.h>
#include <k_arch.h>
//#include "debug_api.h"

#define REG_NAME_WIDTH 8
extern volatile uint32_t g_crash_steps;

typedef struct {
    long X1;
    long X2;
    long X3;
    long X4;
    long X5;
    long X6;
    long X7;
    long X8;
    long X9;
    long X10;
    long X11;
    long X12;
    long X13;
    long X14;
    long X15;
    long X16;
    long X17;
    long X18;
    long X19;
    long X20;
    long X21;
    long X22;
    long X23;
    long X24;
    long X25;
    long X26;
    long X27;
    long X28;
    long X29;
    long X30;
    long X31;
    long MEPC;
    long MSTATUS;
} fault_context_t;

typedef struct
{
    long mcause;
    long mtval;
    long sp;
    long lr;
} FAULT_REGS;

static char *k_ll2str(intptr_t num, char *str)
{
    int i = 15;
    char         index[] = "0123456789ABCDEF";
    uintptr_t usnum   = (uintptr_t)num;

    for(; i >= 0; i--){
        str[i] = index[usnum % 16];
        usnum /= 16;
    }

    return str;
}

void panicGetCtx(void *context, char **pPC, char **pLR, int **pSP)
{
    fault_context_t *rv64_context = (fault_context_t *)context;

    *pSP = (int *)rv64_context->X2;
    *pPC = (char *)rv64_context->MEPC;
    *pLR = (char *)rv64_context->X1;
}

void panicShowRegs(void *context, int (*print_func)(const char *fmt, ...))
{
    int        x;
    long       *regs = (long *)context;
    char       s_panic_regs[REG_NAME_WIDTH + 14 + 8];
    FAULT_REGS stFregs;
    /* fault_context_t */
    char s_panic_ctx[] = "X1(ra)  "
                         "X2(sp)  "
                         "X3(gp)  "
                         "X4(tp)  "
                         "X5(t0)  "
                         "X6(t1)  "
                         "X7(t2)  "
                         "X8(s0)  "
                         "X9(s1)  "
                         "X10(a0) "
                         "X11(a1) "
                         "X12(a2) "
                         "X13(a3) "
                         "X14(a4) "
                         "X15(a5) "
                         "X16(a6) "
                         "X17(a7) "
                         "X18(s2) "
                         "X19(s3) "
                         "X20(s4) "
                         "X21(s5) "
                         "X22(s6) "
                         "X23(s7) "
                         "X24(s8) "
                         "X25(s9) "
                         "X26(s10)"
                         "X27(s11)"
                         "X28(t3) "
                         "X29(t4) "
                         "X30(t5) "
                         "X31(t6) "
                         "MEPC    "
                         "MSTAT   ";

    /* FAULT_REGS */
    char s_panic_reg[] = "MCAUSE  "
                         "MTVAL   "
                         "SP      "
                         "RA      ";

    if (regs == NULL) {
        return;
    }

    print_func("========== Regs info  ==========\r\n");

    /* show fault_context_t */
    for (x = 0; x < sizeof(s_panic_ctx) / REG_NAME_WIDTH; x++) {
        memcpy(&s_panic_regs[0], &s_panic_ctx[x * REG_NAME_WIDTH],
               REG_NAME_WIDTH);
        memcpy(&s_panic_regs[REG_NAME_WIDTH], " 0x", 3);
        k_ll2str(regs[x], &s_panic_regs[REG_NAME_WIDTH + 3]);
        s_panic_regs[REG_NAME_WIDTH + 11 + 8] = '\r';
        s_panic_regs[REG_NAME_WIDTH + 12 + 8] = '\n';
        s_panic_regs[REG_NAME_WIDTH + 13 + 8] = 0;
        print_func(s_panic_regs);
    }

    /* show FAULT_REGS */
    stFregs.mcause = __get_MCAUSE();
    stFregs.mtval  = __get_MTVAL();
    stFregs.sp     = (long)regs[1];
    stFregs.lr     = (long)regs[0];
    for (x = 0; x < sizeof(stFregs) / sizeof(long); x++) {
        memcpy(&s_panic_regs[0], &s_panic_reg[x * REG_NAME_WIDTH],
               REG_NAME_WIDTH);
        memcpy(&s_panic_regs[REG_NAME_WIDTH], " 0x", 3);
        k_ll2str(((long *)(&stFregs))[x], &s_panic_regs[REG_NAME_WIDTH + 3]);
        s_panic_regs[REG_NAME_WIDTH + 11 + 8] = '\r';
        s_panic_regs[REG_NAME_WIDTH + 12 + 8] = '\n';
        s_panic_regs[REG_NAME_WIDTH + 13 + 8] = 0;
        print_func(s_panic_regs);
    }
}

#if defined(RHINO_CONFIG_CLI_AS_NMI) && (RHINO_CONFIG_CLI_AS_NMI > 0)
void panicNmiInputFilter(uint8_t ch)
{
    static int  check_cnt = 0; /* for '$#@!' */

    if ( ch == '$' && check_cnt == 0) {
        check_cnt++;
    }
    else if ( ch == '#' && check_cnt == 1) {
        check_cnt++;
    }
    else if ( ch == '@' && check_cnt == 2) {
        check_cnt++;
    }
    else if ( ch == '!' && check_cnt == 3) {
        panicNmiFlagSet();
        __asm__ __volatile__("udf":::"memory");
    }
    else {
        check_cnt = 0;
    }
}
#else
void panicNmiInputFilter(uint8_t ch){}
#endif

extern void panicHandler(void *context);
extern int32_t aos_debug_printf(const char *fmt, ...);
void exceptionHandler(void *context)
{
    long mstatus, mepc, vec;

    g_crash_steps++;

    vec = __get_MCAUSE() & 0x3FF;
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));

    aos_debug_printf("Exception ++++++++++ MEPC 0x%lx, MSTATUS 0x%lx, CPU Exception: NO.0x%lx\r\n", mepc, mstatus, vec);

    if (g_crash_steps > 1) {
        context = NULL;
    }
    panicHandler(context);
}

