/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */
#include <soc.h>
#include <csi_core.h>
#include <drv/tick.h>
#include <drv/porting.h>
#include <drv/cvi_irq.h>
#include <drv/dma.h>
#include "riscv_csr.h"

#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
#include <excore/k_mmu.h>
#if defined(CONFIG_BENGINE_ENABLE) && CONFIG_BENGINE_ENABLE
#include "bengine/bestat.h"
#endif
#endif

#if (defined(CONFIG_KERNEL_RHINO) || defined(CONFIG_KERNEL_FREERTOS)) && defined(CONFIG_KERNEL_NONE)
#error "Please check the current system is baremetal or not!!!"
#endif

extern void section_data_copy(void);
extern void section_ram_code_copy(void);
extern void section_bss_clear(void);

extern int pre_main(void);


#define CONFIG_DUMP_RECORD_TIME
#ifdef CONFIG_DUMP_RECORD_TIME

#ifdef __ASSEMBLY__
#define __ASM_STR(x)	x
#else
#define __ASM_STR(x)	#x
#endif

#define SYS_COUNTER_FREQ_IN_SECOND 25000000

#define csr_read(csr)						\
({								\
	register unsigned long __v;				\
	__asm__ __volatile__ ("csrr %0, " __ASM_STR(csr)	\
				: "=r" (__v) :			\
				: "memory");			\
	__v;							\
})

#define CSR_TIME 0xc01

unsigned long long timer_get_boot_us(void)
{
	unsigned long long boot_us = 0;
	boot_us = csr_read(CSR_TIME) / (SYS_COUNTER_FREQ_IN_SECOND / 1000000);

	return boot_us;
}
#endif


#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
extern uint64_t __sVectors;
extern k_mmu_regions_t g_memmaps[];
extern int g_memmaps_size;

void _system_switchto_smode(void)
{
    k_mmu_init(g_memmaps, g_memmaps_size);

    /* go to Supervisor mode*/
    uint64_t m_status = __get_MSTATUS();
    m_status &= ~MSTATUS_TVM_MASK;
    m_status &= ~MSTATUS_MPP_MASK;
    m_status |= MSTATUS_MPP_S;
    __set_MSTATUS(m_status);

    __set_STVEC((uint64_t)(&__sVectors) | (uint64_t)0x1);
    __set_MEPC((uint64_t)pre_main);

    __ASM("mret");
}

void _system_init_for_smode(void)
{
    _system_switchto_smode();
}

extern int32_t aos_debug_printf(const char *fmt, ...);
void _system_switchto_mmode(void)
{
    uint32_t i;

    /* disable irq before */
    for (i = 0; i < 100; i++) {
        csi_irq_disable(i);
    }
    csi_mmu_disable();
    csi_tick_uninit();
    aos_debug_printf("prepare switch to M-mode\r\n");
    /* go to Machine mode*/
    uint64_t m_status = __get_MSTATUS();
    m_status &= ~MSTATUS_TVM_MASK;
    m_status &= ~MSTATUS_MPP_MASK;
    m_status |= MSTATUS_MPP_M;
    __set_MSTATUS(m_status);

    __ASM("mret");
}

void smode_init(void)
{
#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
#if defined(CONFIG_BENGINE_ENABLE) && CONFIG_BENGINE_ENABLE
    bestat_init();
#endif
    /* enable mcounteren for s-mode */
    __set_MCOUNTEREN(0xffffffff);
    __set_MCOUNTERWEN(0xffffffff);

    /* enable delegate, exclude ill inst */
    uint64_t medeleg = __get_MEDELEG();
    medeleg |= 0xffff;
    medeleg &= ~(1U << CAUSE_ILLEGAL_INSTRUCTION);
    __set_MEDELEG(medeleg);
    //__set_MEDELEG(0xffff);

    /* enable interrupt delegate */
    uint64_t mideleg = __get_MIDELEG();
    mideleg |= 0x222;
    __set_MIDELEG(mideleg);

    uint64_t mx_status = __get_MXSTATUS();
    mx_status |= 1 << 17; ///< CLINTEE
    __set_MXSTATUS(mx_status);

    uint64_t s_status = __get_SSTATUS();
    s_status &= ~SSTATUS_SPP_MASK;
    s_status &= ~SSTATUS_SPP_S;
    s_status |= 1 << 13; ///< open fs
    s_status |= 1 << 23; ///< open vs
    __set_SSTATUS(s_status);

    uint64_t s_sxstatus = __get__SXSTATUS();
    s_sxstatus |= 1 << 22; ///< open theadisaee
    __set_SXSTATUS(s_sxstatus);
#endif
    _system_init_for_smode();
}
#endif

int32_t drv_get_cpu_id(void)
{
    return 0;
}

static void pmp_init(void)
{
    long addr;

    addr = 0x90000000UL >> 2;
    /* addr0 used before yoc startup */
    __set_PMPADDR1(addr);
    __set_PMPxCFG(1, 0x8f);
}

static void section_init(void)
{
#ifdef CONFIG_XIP
    section_data_copy();
    section_ram_code_copy();
    csi_dcache_clean();
    csi_icache_invalid();
#endif

    section_bss_clear();
}

static void cache_init(void)
{
    /* enable cache */
    csi_dcache_enable();
    csi_icache_enable();
}

/* clear soft irq before */
static void tspend_clear()
{
#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
    *(volatile int*)(CORET_BASE + 0xC000) = 0;
#else
    *(volatile int*)(CORET_BASE) = 0;
#endif
}

/**
  * @brief  initialize the system
  *         Initialize the psr and vbr.
  * @param  None
  * @return None
  */
#if defined(CONFIG_RISCV_SMODE) && CONFIG_RISCV_SMODE
static void interrupt_init(void)
{
    tspend_clear();
    /*PLIC 权限控制寄存器,PLIC_CTRL*/
    //PLIC->PLIC_PER = 0x1;
    /*机器模式中断响应/完成寄存器*/
    for (int i = 0; i < 1023; i++) {
        PLIC->PLIC_H0_SCLAIM = i;
    }
    /*中断的优先级配置寄存器*/
    for (int i = 0; i < 1023; i++) {
        PLIC->PLIC_PRIO[i] = 31;
    }
    /*清中断等待寄存器*/
    for (int i = 0; i < 32; i++) {
        PLIC->PLIC_IP[i] = 0;
    }

    /*清M&S态中断使能寄存器*/
    for (int i = 0; i < 32; i++) {
        PLIC->PLIC_H0_MIE[i] = 0;
        PLIC->PLIC_H0_SIE[i] = 0;
    }

    /* set hart threshold 0, allow all interrupt */
    PLIC->PLIC_H0_STH = 0;

    uint32_t sie = __get_SIE();
    sie |= (1 << 9 | 1 << 5 | 1 << 1);
    __set_SIE(sie);
}
#else
static void interrupt_init(void)
{
    tspend_clear();
    /*PLIC 权限控制寄存器,PLIC_CTRL*/
    PLIC->PLIC_PER = 0x1;
    /*机器模式中断响应/完成寄存器*/
    for (int i = 0; i < 1023; i++) {
        PLIC->PLIC_H0_MCLAIM = i;
    }
    /*中断的优先级配置寄存器*/
    for (int i = 0; i < 1023; i++) {
        PLIC->PLIC_PRIO[i] = 31;
    }
    /*清中断等待寄存器*/
    for (int i = 0; i < 32; i++) {
        PLIC->PLIC_IP[i] = 0;
    }

    /* set hart threshold 0, allow all interrupt */
    PLIC->PLIC_H0_MTH = 0;

    /* enable msoft interrupt ; Machine_Software_IRQn*/
    uint32_t mie = __get_MIE();
    mie |= (1 << 11 | 1 << 7 | 1 << 3);
    __set_MIE(mie);
}
#endif

#if defined(CONFIG_KERNEL_NONE)
void SystemInit(void)
{
    cache_init();
    section_init();
    interrupt_init();
    csi_tick_init();
}
#else
//volatile int g_debug = 1;
void SystemInit(void)
{
    //while (g_debug);
    cache_init();
    pmp_init();
    section_init();
    interrupt_init();
    csi_tick_init();
}
#endif
