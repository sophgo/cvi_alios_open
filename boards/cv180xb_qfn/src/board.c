/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include <board.h>
#include <drv/pin.h>
#include <drv/gpio.h>
#include <drv/dma.h>
#include <sys_clk.h>
#include <drv/codec.h>
#include <drv/rtc.h>
#include <soc.h>
#include <mmio.h>
#include <devices/devicelist.h>
#include "board_config.h"


csi_gpio_t chip_gpio_handler;
csi_rtc_t rtc_hdl;
csi_dma_t dma_hdl;

void board_uart_init(void) {
  rvm_uart_drv_register(CONSOLE_UART_IDX);
}

static void board_pinmux_config(void)
{
    // BT
    //csi_pin_set_mux(PG6, PG6_UART1_TX);
    //csi_pin_set_mux(PG7, PG7_UART1_RX);

    //csi_pin_set_mux(PG8, PG8_UART1_RTS);
    //csi_pin_set_mux(PG9, PG9_UART1_CTS);

    //csi_pin_set_mux(PG18, PIN_FUNC_GPIO);
}

void board_sound_init(void)
{
    //csi_codec_t codec;
    //csi_codec_init(&codec, 0);
}

void board_clk_init(void)
{
    //soc_clk_init();
    //soc_clk_enable(BUS_UART1_CLK);

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
#else
    //default 170M
    mmio_write_32(DIV_CLK_CAM0_200 , BIT_DIV_RESET_CONT | BIT_SELT_DIV_REG | BIT_CLK_SRC |
	 BIT_CLK_DIV_FACT_16 | BIT_CLK_DIV_FACT_17 | BIT_CLK_DIV_FACT_18);
#endif
}

void board_init(void)
{
    board_clk_init();
#if 0
    csi_rtc_init(&rtc_hdl, 0);
    csi_gpio_init(&chip_gpio_handler, 0);
    csi_dma_init(&dma_hdl, 0);
#else
    csi_dma_init(&dma_hdl, 0);
    board_pinmux_config();
    board_sound_init();
#endif
    board_uart_init();
    board_flash_init();
}
