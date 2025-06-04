/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     irq.c
 * @brief    CSI Source File for IRQ Driver
 * @version  V1.0
 * @date     7. April 2020
 ******************************************************************************/

#include <stdint.h>
#include <soc.h>
#include <csi_core.h>
#include <csi_config.h>
#include <drv/common.h>
#include <drv/cvi_irq.h>
#ifndef CONFIG_KERNEL_NONE
#include <csi_kernel.h>
#endif

void Default_Handler(void) {}
extern uint32_t soc_irq_get_irq_num(void);
extern void soc_irq_end(uint32_t irq_num);

#ifndef CONFIG_KERNEL_NONE
#define CSI_INTRPT_ENTER() csi_kernel_intrpt_enter()
#define CSI_INTRPT_EXIT()  csi_kernel_intrpt_exit()
#else
#define CSI_INTRPT_ENTER()
#define CSI_INTRPT_EXIT()
#endif

uint32_t  g_irq_nested_level;
csi_dev_t *g_irq_table[CONFIG_IRQ_NUM];
csi_dev_t g_irq_dev_table[CONFIG_REAL_IRQ_CNT];

/**
  \brief       register irq handler.
  \param[in]   irq_num Number of IRQ.
  \return      None.
*/
void csi_irq_attach(uint32_t irq_num, void *irq_handler, csi_dev_t *dev)
{
    dev->irq_handler = irq_handler;
    g_irq_table[irq_num] = dev;
}

/**
  \brief       Attach irq handler2 for compatible.
  \param[in]   irq_num      Number of IRQ.
  \param[in]   irq_handler2 IRQ Handler.
  \param[in]   dev          The dev to operate
  \param[in]   arg          user data of irq_handler2
  \return      None.
*/
void csi_irq_attach2(uint32_t irq_num, void *irq_handler2, csi_dev_t *dev, void *arg)
{
    dev->arg             = arg;
    dev->irq_handler2    = irq_handler2;
    g_irq_table[irq_num] = dev;
}

/**
  \brief       unregister irq handler.
  \param[in]   irq_num Number of IRQ.
  \param[in]   irq_handler IRQ Handler.
  \return      None.
*/
void csi_irq_detach(uint32_t irq_num)
{
    g_irq_table[irq_num] = NULL;
}

/**
  \brief       gets whether in irq context
  \return      true or false.
*/
bool csi_irq_context(void)
{
    return ((g_irq_nested_level > 0U) ? true : false);
}

/**
  \brief       dispatching irq handlers
  \return      None.
*/
__WEAK void do_irq(void)
{
    uint32_t irqn;
    uint32_t irqnc;

    g_irq_nested_level++;

    CSI_INTRPT_ENTER();

    irqnc = (__get_MCAUSE() & 0x3FF);
    irqn = irqnc;
    if (irqnc != CORET_IRQn) {                  // FIXME:
        irqn = soc_irq_get_irq_num();
    }

    if (g_irq_table[irqn]) {
        if (g_irq_table[irqn]->irq_handler)
            g_irq_table[irqn]->irq_handler(g_irq_table[irqn]);
        else if (g_irq_table[irqn]->irq_handler2)
            g_irq_table[irqn]->irq_handler2(irqn, g_irq_table[irqn]->arg);
        else
            Default_Handler();
    } else {
        Default_Handler();
    }
    CSI_INTRPT_EXIT();

    g_irq_nested_level--;
    soc_irq_end(irqn);
}


int request_irq(unsigned int irqn, void * handler, unsigned long flags,const char *name, void *priv)
{
    //printf("irqn %d flags :lx\n",irqn,flags);
    csi_irq_attach2(irqn, handler, &g_irq_dev_table[irqn], priv);
    //csi_plic_set_prio(PLIC_BASE, dev.irq_num, 31); //temp
    csi_irq_enable(irqn);

    return 0;
}
