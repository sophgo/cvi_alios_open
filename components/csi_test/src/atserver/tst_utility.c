/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     tst_utility.c
 * @brief    c file for auto test tools
 * @version  V1.0
 * @date     2020-02-17
 ******************************************************************************/
#include "stdio.h"
#include "stdint.h"
#include "drv/common.h"
#include "drv/tick.h"
#include "drv/uart.h"
#include "drv/ringbuf.h"
#include "csi_config.h"
#include "tst_io.h"
#include "list.h"
#include "board_config.h"
#include "board_init.h"
#include "autotest.h"

extern void mdelay(uint32_t ms);
static uint32_t g_cur_sys_time = 0;
extern  const atserver_cmd_t common_at_cmd[];
/**
 * \brief              init core tick timer
 * \return             none
 */

void tst_timer_init(void)
{
    g_cur_sys_time = 0;
    return;
}

/**
 * \brief              restart timer
 * \return             none
 */

void tst_timer_restart(void)
{
    g_cur_sys_time = csi_tick_get_ms();
    return;
}


/**
 * \brief              get interval from last timer begining
 * \return             interval time unit in mini-second
 */

uint32_t tst_timer_get_interval(void)
{
    uint32_t temp, total;

    /* check if counter reverse */
    temp = csi_tick_get_ms();

    if (temp >= g_cur_sys_time) {
        total = temp - g_cur_sys_time;
    } else {
        total = (0xFFFFFFFF - g_cur_sys_time) + temp;
    }

    return total;
}

/**
 * \brief              do polling delay
 * \param              cnt_ms     time to be delay ,unit in mini-second
 * \return             none
 */

void tst_mdelay(uint32_t cnt_ms)
{
    mdelay(cnt_ms);
    return;
}

/**
 * \brief              init auto test service
 * \param              at_uart        io config
 * \return             err code
 */

csi_error_t tst_init(tst_uart_config_t *at_uart)
{
    csi_error_t ret;

    /* init io device */
    ret = tst_io_init(at_uart);
    CSI_ASSERT(ret == CSI_OK);

    /*init at server */
    ret = csi_test_atserver_init();
    CSI_ASSERT(ret == CSI_OK);

    /*link common AT handler */
    ret = csi_test_atserver_add_command(common_at_cmd);
    CSI_ASSERT(ret == CSI_OK);

    /* init timer */
    tst_timer_init();

    /* send system start up event */
    csi_test_atserver_send("\r\nBOOT_IND\r\n");

    return ret;
}

