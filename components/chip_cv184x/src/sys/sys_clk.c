/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     sys_clk.c
 * @brief    source file for setting system frequency.
 * @version  V1.0
 * @date     7. April 2020
 ******************************************************************************/

#include <stdint.h>
#include <soc.h>
#include <sys_clk.h>

uint32_t soc_get_cpu_freq(uint32_t idx)
{
    return 0U;
}

uint32_t soc_get_usi_freq(uint32_t idx)
{
    return 0U;
}

uint32_t soc_get_uart_freq(uint32_t idx)
{
    return 0U;
}

uint32_t soc_get_pwm_freq(uint32_t idx)
{
    return 0U;
}

uint32_t soc_get_i2s_freq(uint32_t idx)
{
    return 0U;
}

uint32_t soc_get_timer_freq(uint32_t idx)
{
    return 0U;
}

uint32_t soc_get_rtc_freq(uint32_t idx)
{
    return 0U;
}

uint32_t soc_get_apb_freq(uint32_t idx)
{
    return 0U;
}

uint32_t soc_get_coretim_freq(void)
{
    return 25 * 1000000;
}

uint32_t soc_get_coretim_us_freq(void)
{
    return 25;
}

void soc_set_sys_freq(uint32_t val)
{
}

void soc_clk_enable(int32_t module)
{
}

void soc_clk_disable(int32_t module)
{
}
