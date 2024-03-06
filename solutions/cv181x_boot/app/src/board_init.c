/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     board_init.c
 * @brief    CSI Source File for board init
 * @version  V1.0
 * @date     31. June 2018
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <drv/uart.h>
#include <drv/pin.h>
#include <drv/wdt.h>
#include <board.h>
#include <stdint.h>
#include <mmio.h>

static csi_uart_t g_console_handle;

static void uart_init(void)
{
    /* init the console */
    if (csi_uart_init(&g_console_handle, CONSOLE_UART_IDX)) {

    }

    /* config the UART */
    csi_uart_baud(&g_console_handle, CONFIG_CLI_USART_BAUD);
    csi_uart_format(&g_console_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);
}

// static void wdt_init(void)
// {
//     csi_error_t ret;
//     csi_wdt_t wdt;

//     ret = csi_wdt_init(&wdt, 0);
//     if (ret == CSI_OK) {
//         csi_wdt_set_timeout(&wdt, CONFIG_WDT_TIMEOUT);
//         csi_wdt_start(&wdt);
//     } else {
//         printf("wdt init failed.\n");
//     }
// }

void clk_axi4_reconfig(void)
{
// hsperi clock to PLL (FPLL) div by x  = 1500 / x
#if CONFIG_SPINOR_CLK_75M
    mmio_write_32(0x030020B8, 0x00050009);
#elif CONFIG_SPINOR_CLK_93M
    mmio_write_32(0x030020B8, 0x00040009);
#elif CONFIG_SPINOR_CLK_125M
    mmio_write_32(0x030020B8, 0x00030009);
#endif
}

void board_yoc_init(void)
{
    // extern unsigned int csi_tick_get_ms(void);
    // unsigned int cur_ms;
    // cur_ms = csi_tick_get_ms();
    board_init();
    uart_init();
    // printf("\n##cur_ms:%d\n", cur_ms);
    // wdt_init();
}

int fputc(int ch, FILE *stream)
{
    if (ch == '\n') {
        csi_uart_putc(&g_console_handle, '\r');
    }

    csi_uart_putc(&g_console_handle, ch);
    return 0;
}

int uart_putc(int ch)
{
    if (ch == '\n') {
        csi_uart_putc(&g_console_handle, '\r');
    }

    csi_uart_putc(&g_console_handle, ch);
    return 0;
}