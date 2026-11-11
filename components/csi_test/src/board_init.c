/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     main.c
 * @brief    hello world
 * @version  V1.0
 * @date     17. Jan 2018
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <drv/uart.h>
#include "board_config.h"
#include "console/sys_console.h"

sys_console_t console;

extern void __ChipInitHandler(void);

void board_init(void)
{
    __ChipInitHandler();

    console.uart_id = (uint32_t)CONSOLE_IDX;
    console.baudrate = 115200U;
    console.tx.pin = CONSOLE_TXD;
    console.tx.func = CONSOLE_TXD_FUNC;
    console.rx.pin = CONSOLE_RXD;
    console.rx.func = CONSOLE_RXD_FUNC;
    console.uart = NULL;

    csi_test_console_init(&console);
}

