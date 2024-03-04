/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include <board.h>
#include <drv/pin.h>
#include <drv/gpio.h>
#include <drv/dma.h>
#include <sys_clk.h>
#include <mmio.h>

void board_init(void)
{
#if 0
    /* config uart clk */
#if CONSOLE_UART_CLK == 1188000000
    mmio_write_32(DIV_CLK_CAM0_200 , BIT_DIV_RESET_CONT | BIT_SELT_DIV_REG | BIT_CLK_SRC |
	 BIT_CLK_DIV_FACT_16);
#elif CONSOLE_UART_CLK == 594000000
    mmio_write_32(DIV_CLK_CAM0_200 , BIT_DIV_RESET_CONT | BIT_SELT_DIV_REG | BIT_CLK_SRC |
	 BIT_CLK_DIV_FACT_17);
#elif CONSOLE_UART_CLK == 396000000
    mmio_write_32(DIV_CLK_CAM0_200 , BIT_DIV_RESET_CONT | BIT_SELT_DIV_REG | BIT_CLK_SRC |
	 BIT_CLK_DIV_FACT_16 | BIT_CLK_DIV_FACT_17);
#elif CONSOLE_UART_CLK == 297000000
    mmio_write_32(DIV_CLK_CAM0_200 , BIT_DIV_RESET_CONT | BIT_SELT_DIV_REG | BIT_CLK_SRC |
	 BIT_CLK_DIV_FACT_18);
#endif
#endif
}
