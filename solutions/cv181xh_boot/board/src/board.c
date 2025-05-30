/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include <board.h>
#include <drv/pin.h>
#include <drv/gpio.h>
#include <drv/dma.h>
#include <sys_clk.h>
// #include <csi_core.h>

// csi_dma_t dma_hdl;
// void board_dma_init(void)
// {
//     csi_dma_init(&dma_hdl, 0);
// }

csi_dma_t dma_hdl;
void board_dma_init(void)
{
    csi_dma_init(&dma_hdl, 0);
}

void board_init(void)
{
    /* config uart clk */
// mmio_write is not define in boot solution
#if 0  
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
    // board_dma_init();
    board_dma_init();
}
