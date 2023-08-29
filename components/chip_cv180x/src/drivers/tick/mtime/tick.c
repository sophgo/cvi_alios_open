/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     tick.c
 * @brief    Source File for tick
 * @version  V1.0
 * @date     3. April 2020
 ******************************************************************************/

#include <csi_config.h>
#include <sys_clk.h>
#include <csi_core.h>
#include <drv/tick.h>
#include <drv/common.h>
#include <drv/porting.h>
#include <drv/cvi_irq.h>
#include <soc.h>

extern void csi_coret_clear_irq(void);
extern void krhino_tick_proc(void);
extern void xPortSysTickHandler(void);
extern void OSTimeTick(void);

static csi_dev_t tick_dev;

static volatile uint32_t csi_tick = 0U;

void csi_tick_increase(void)
{
    csi_tick++;
}


static void tick_irq_handler(void *arg)
{
    csi_tick_increase();
    csi_coret_config(soc_get_coretim_freq() / CONFIG_SYSTICK_HZ, CORET_IRQn);
#if defined(CONFIG_KERNEL_RHINO)
    krhino_tick_proc();
#elif defined(CONFIG_KERNEL_FREERTOS)
    xPortSysTickHandler();
#elif defined(CONFIG_KERNEL_UCOS)
    OSTimeTick();
#endif

}

csi_error_t csi_tick_init(void)
{
    tick_dev.irq_num = CORET_IRQn;

    csi_vic_set_prio(CORET_IRQn, 3U);
    csi_irq_attach(tick_dev.irq_num, &tick_irq_handler, &tick_dev);
    csi_tick = ((csi_tick_get_ms() * CONFIG_SYSTICK_HZ) / 1000U);
    csi_coret_config((soc_get_coretim_freq() / CONFIG_SYSTICK_HZ), CORET_IRQn);
    csi_irq_enable(tick_dev.irq_num);
    return CSI_OK;
}

void csi_tick_uninit(void)
{
    csi_irq_disable(tick_dev.irq_num);
    csi_irq_detach(tick_dev.irq_num);
}

uint32_t csi_tick_get(void)
{
    return csi_tick;
}

uint32_t csi_tick_get_ms(void)
{
    uint32_t time;
    time = ((((uint64_t)csi_coret_get_valueh() << 32U) | csi_coret_get_value())) / (soc_get_cur_cpu_freq() / 1000U);
    return time;
}

uint64_t csi_tick_get_us(void)
{
    uint64_t time;
    time = ((((uint64_t)csi_coret_get_valueh() << 32U) | csi_coret_get_value()) * 1000U) / (soc_get_cur_cpu_freq() / 1000U);
    return time;
}

void _mdelay(void)
{
    unsigned long long start, cur, delta;
    uint32_t startl = csi_coret_get_value();
    uint32_t starth = csi_coret_get_valueh();
    uint32_t curl, curh;
    uint32_t cnt = (soc_get_cur_cpu_freq() / 1000U);
    start = ((unsigned long long)starth << 32U) | startl;

    while (1) {
        curl = csi_coret_get_value();
        curh = csi_coret_get_valueh();
        cur = ((unsigned long long)curh << 32U) | curl;
        delta = cur - start;

        if (delta >= cnt) {
            return;
        }
    }
}

#if defined(CONFIG_KERNEL_RHINO)
#include <k_api.h>
#define RHINO_OS_MS_PERIOD_TICK      (1000 / RHINO_CONFIG_TICKS_PER_SECOND)
#elif defined(CONFIG_KERNEL_FREERTOS)
#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>
#endif

__WEAK void mdelay(uint32_t ms)
{
    while (ms--) {
        _mdelay();
    }
}

__WEAK void msleep(uint32_t ms)
{
#if defined(CONFIG_KERNEL_RHINO)
    extern tick_t     g_tick_count;

    if (g_tick_count > 0) {
        extern kstat_t krhino_task_sleep(tick_t dly);
        uint32_t ms_get = ms;

        if ((ms < RHINO_OS_MS_PERIOD_TICK) && (ms != 0)) {
            ms_get = RHINO_OS_MS_PERIOD_TICK;
        }

        uint64_t ticks = (uint64_t)ms_get / RHINO_OS_MS_PERIOD_TICK;
        krhino_task_sleep(ticks);
    } else {
        while (ms) {
            ms--;
            _500usdelay();
            _500usdelay();
        }
    }

#elif defined(CONFIG_KERNEL_FREERTOS)
    extern UBaseType_t uxTaskGetNumberOfTasks(void);
    uint32_t task_num = (uint32_t)uxTaskGetNumberOfTasks();

    if (task_num > 0) {
        extern void vTaskDelay(const TickType_t xTicksToDelay);
#if ( INCLUDE_vTaskDelay == 1 )
        uint32_t ms_get = ms;

        if ((ms != 0) && (ms < portTICK_PERIOD_MS)) {
            ms_get = portTICK_PERIOD_MS;
        }

        uint64_t ticks = (uint64_t)ms_get / portTICK_PERIOD_MS;
        vTaskDelay(ticks);
    } else {
        while (ms) {
            ms--;
            _mdelay();
        }
    }

#endif
#else

    while (ms--) {
        _mdelay();
    }

#endif
}