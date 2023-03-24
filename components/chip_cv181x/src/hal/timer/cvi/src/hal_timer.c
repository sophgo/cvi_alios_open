/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#include <stdint.h>
#include <dw_timer_ll.h>

uint32_t hal_timer_read_load(dw_timer_regs_t *timer_base)
{
    return (timer_base->TLC);
}
void hal_timer_write_load(dw_timer_regs_t *timer_base, uint32_t value)
{
    timer_base->TLC = value;
}
uint32_t hal_timer_get_current(dw_timer_regs_t *timer_base)
{
    return (timer_base->TCV);
}
void hal_timer_set_enable(dw_timer_regs_t *timer_base)
{
    timer_base->TCR |= (DW_TIMER_CTL_ENABLE_SEL_EN);
}
void hal_timer_set_disable(dw_timer_regs_t *timer_base)
{
    timer_base->TCR &= ~(DW_TIMER_CTL_ENABLE_SEL_EN);
}
uint32_t hal_timer_get_enable(dw_timer_regs_t *timer_base)
{
    return (((timer_base->TCR) & DW_TIMER_CTL_ENABLE_SEL_EN) ? (uint32_t)1 : (uint32_t)0);
}
void hal_timer_set_mode_free(dw_timer_regs_t *timer_base)
{
    timer_base->TCR &= ~(DW_TIMER_CTL_MODE_SEL_EN);
}
void hal_timer_set_mode_load(dw_timer_regs_t *timer_base)
{
    timer_base->TCR |= (DW_TIMER_CTL_MODE_SEL_EN);
}
uint32_t hal_timer_get_model(dw_timer_regs_t *timer_base)
{
    return (((timer_base->TCR) & DW_TIMER_CTL_MODE_SEL_EN) ? (uint32_t)1 : (uint32_t)0);
}
void hal_timer_set_mask(dw_timer_regs_t *timer_base)
{
    timer_base->TCR |= (DW_TIMER_CTL_INT_MAKS_EN);
}
void hal_timer_set_unmask(dw_timer_regs_t *timer_base)
{
    timer_base->TCR &= ~(DW_TIMER_CTL_INT_MAKS_EN);
}
uint32_t hal_timer_get_mask(dw_timer_regs_t *timer_base)
{
    return (((timer_base->TCR) & DW_TIMER_CTL_INT_MAKS_EN) ? (uint32_t)1 : (uint32_t)0);
}
void hal_timer_set_hardtrigger_en(dw_timer_regs_t *timer_base)
{
    timer_base->TCR |= (DW_TIMER_CTL_HARD_TRIG_EN);
}
void hal_timer_set_hardtrigger_dis(dw_timer_regs_t *timer_base)
{
    timer_base->TCR &= ~(DW_TIMER_CTL_HARD_TRIG_EN);
}
uint32_t hal_timer_get_hardtrigger(dw_timer_regs_t *timer_base)
{
    return (((timer_base->TCR) & DW_TIMER_CTL_HARD_TRIG_EN) ? (uint32_t)1 : (uint32_t)0);
}
uint32_t hal_timer_clear_irq(dw_timer_regs_t *timer_base)
{
    return (((timer_base->TEOI) & DW_TIMER_EOI_REG_EN) ? (uint32_t)1 : (uint32_t)0);
}
uint32_t hal_timer_get_int_status(dw_timer_regs_t *timer_base)
{
    return (((timer_base->TIS) & DW_TIMER_INT_STATUS_EN) ? (uint32_t)1 : (uint32_t)0);
}
void hal_timer_reset_register(dw_timer_regs_t *timer_base)
{
    timer_base->TCR = 0U;
    timer_base->TLC = 0U;
}
uint32_t hal_timer_general_active_after_mask(dw_timer_general_regs_t *timer_base)
{
    return ((timer_base->TSIS) & DW_TIMERS_INT_STATUS_EN);
}
uint32_t hal_timer_general_clear_irq(dw_timer_general_regs_t *timer_base)
{
    return ((timer_base->TSEOI) & DW_TIMERS_EOI_REG_EN);
}
uint32_t hal_timer_general_active_prior_mask(dw_timer_general_regs_t *timer_base)
{
    return ((timer_base->TSRIS) & DW_TIMERS_RAW_INT_STA_EN);
}