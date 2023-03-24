/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <drv/timer.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>

static csi_timer_t timer_hd;
volatile static uint32_t timer_irq_num;

static void timet_callback(csi_timer_t *timer, void *arg)
{
    *(uint32_t *)arg += 1;
}

int timer_start(uint8_t idx)
{
    csi_error_t ret;

    timer_irq_num = 0;

    ret = csi_timer_init(&timer_hd, idx);
    TEST_CASE_ASSERT_QUIT(ret == 0, "timer init fail");

    ret = csi_timer_attach_callback(&timer_hd, timet_callback, (void *)&timer_irq_num);
    TEST_CASE_ASSERT_QUIT(ret == 0, "timer attach callback fail");

    ret = csi_timer_start(&timer_hd, 100000);
    TEST_CASE_ASSERT_QUIT(ret == 0, "timer start fail");

    return 0;
}

uint32_t timer_stop(void)
{
    uint32_t cur_value = 0, load_value = 0;
    //int32_t sys_freq;
    //uint32_t cnt;
    uint32_t run_tick;

    cur_value = csi_timer_get_remaining_value(&timer_hd);
    load_value = csi_timer_get_load_value(&timer_hd);

    csi_timer_stop(&timer_hd);
    //sys_freq = drv_get_sys_freq();
    run_tick = load_value - cur_value + load_value * timer_irq_num;

    csi_timer_detach_callback(&timer_hd);
    csi_timer_uninit(&timer_hd);

    //cnt = sys_freq / 1000000;
    //return run_tick / cnt;
    return run_tick;
}