/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <hal_timer.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <drv/cvi_irq.h>

static int cvi_timer_alloc_status[DW_NR_TIMERS];

int cvi_timer_init(cvi_timer_t *timer, uint32_t idx, dw_timer_regs_t *timer_base, uint8_t irq_num)
{
	uint32_t irq_flags;

	if(!timer)
		return -EINVAL;

	irq_flags = csi_irq_save();
	if(cvi_timer_alloc_status[idx]){
		csi_irq_restore(irq_flags);
		return -EBUSY;
	}
	cvi_timer_alloc_status[idx] = 1;
	csi_irq_restore(irq_flags);

	timer->irq_num = irq_num;
	timer->timer_base = timer_base;
	timer->timer_id = idx;

	hal_timer_reset_register(timer->timer_base);

	return 0;
}

void cvi_timer_uninit(cvi_timer_t *timer)
{
	if(!timer){
		printf("%s: timer == NULL!!!\r\n", __func__);
		return;
	}

	hal_timer_reset_register(timer->timer_base);

	cvi_timer_alloc_status[timer->timer_id] = 0;
}

int cvi_timer_start(cvi_timer_t *timer, uint32_t timeout_us)
{
	CVI_PARAM_CHK(timer, -EINVAL);
	CVI_PARAM_CHK(timeout_us, -EINVAL);

    uint32_t tmp_freq = TIMER_FREQ / 1000U;
    uint32_t tmp_load = DW_TIMER_GET_RELOAD_VAL(timeout_us, tmp_freq);

    hal_timer_set_mode_load(timer->timer_base);

    //FIXME: no less than 10
    if (tmp_load < 10) {
        tmp_load = 10;
    }

    hal_timer_set_disable(timer->timer_base);

    hal_timer_write_load(timer->timer_base, tmp_load);

    hal_timer_set_enable(timer->timer_base);
    hal_timer_set_unmask(timer->timer_base);

    return 0;
}

void cvi_timer_stop(cvi_timer_t *timer)
{
	if(!timer){
		printf("%s: timer == NULL!!!\r\n", __func__);
		return;
	}

    hal_timer_set_mask(timer->timer_base);
    hal_timer_set_disable(timer->timer_base);
}

uint32_t cvi_timer_get_remaining_value(cvi_timer_t *timer)
{
	if(!timer){
		printf("%s: timer == NULL!!!\r\n", __func__);
		return 0;
	}

    return hal_timer_get_current(timer->timer_base);
}

uint32_t cvi_timer_get_load_value(cvi_timer_t *timer)
{
	if(!timer){
		printf("%s: timer == NULL!!!\r\n", __func__);
		return 0;
	}

	return (hal_timer_read_load(timer->timer_base));
}

bool cvi_timer_is_running(cvi_timer_t *timer)
{
	if(!timer){
		printf("%s: timer == NULL!!!\r\n", __func__);
		return false;
	}

    return (hal_timer_get_enable(timer->timer_base) ? true : false);
}

void cvi_timer_irq_handler(unsigned int irqn, void *arg)
{
    cvi_timer_t *timer = arg;
    dw_timer_regs_t *timer_base = (dw_timer_regs_t *)timer->timer_base;

    if (hal_timer_get_int_status(timer_base)) {
		hal_timer_clear_irq(timer_base);
		hal_timer_set_disable(timer_base);
        if (timer->callback) {
            timer->callback(timer, timer->arg);
        }
    }
}

int cvi_timer_attach_callback(cvi_timer_t *timer, void *callback, void *arg)
{
    CVI_PARAM_CHK(timer, -EINVAL);
    int ret;

    timer->callback = callback;
    timer->arg = arg;

    ret = request_irq((uint32_t)timer->irq_num, cvi_timer_irq_handler, 0, "timer_irq", timer);
	if(ret){
		ret = -ENODEV;
	}

    return 0;
}

void cvi_timer_detach_callback(cvi_timer_t *timer)
{
    if(!timer){
		printf("%s: timer == NULL!!!\r\n", __func__);
		return;
	}

    timer->callback = NULL;
    timer->arg = NULL;
    csi_irq_disable((uint32_t)timer->irq_num);
    csi_irq_detach((uint32_t)timer->irq_num);
}