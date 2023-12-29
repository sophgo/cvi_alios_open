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
#include <pinctrl-mars.h>
#include <soc.h>
#include <mmio.h>
#include <devices/devicelist.h>
#include "board_config.h"

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

csi_gpio_t chip_gpio_handler;
csi_rtc_t rtc_hdl;
csi_dma_t dma_hdl;

void board_uart_init(void) {
  rvm_uart_drv_register(0);
}

static void board_pinmux_config(void)
{
    //BT
}

void board_sound_init(void)
{
    //csi_codec_t codec;
    //csi_codec_init(&codec, 0);
/* speaker enable */
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

void reboot_pre_hook(void)
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
    extern void hi3861_reboot();
    // hi3861_reboot();
#endif
}

void board_wifi_init(void)
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
    wifi_hi3861l_register(NULL);
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
