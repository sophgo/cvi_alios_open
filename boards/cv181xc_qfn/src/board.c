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
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
#include <hi3861l_devops.h>
#endif

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

csi_gpio_t chip_gpio_handler;
csi_rtc_t rtc_hdl;
csi_dma_t dma_hdl;

const char *g_cx_boardconfig = R"({
"sensors": [
	{
	"tag": "rgb0",
	"width": 1600,
	"height": 1200,
    "onlineMode": 1
	},
	{
	"tag": "ir0",
	"width": 1600,
	"height": 1200,
    "onlineMode": 1
	},
	{
	"tag": "ir1",
	"width": 1600,
	"height": 1200,
	"onlineMode": 1
	}
]
})";

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

#if (CONFIG_CHIP_LOW_POWER_COST ==1)
    //for low power cost
    mmio_write_32(0x03002000, 0xE4007FFE);
    mmio_write_32(0x03002008, 0xFFFF77FF);
    mmio_write_32(0x0300200C, 0xF83FFFFF);
    mmio_write_32(0x03002010, 0xFFFFBFFF);
    mmio_write_32(0x01901008, 0x00030006);
#endif
}

void reboot_pre_hook(void)
{
    extern void hi3861_reboot();
    // hi3861_reboot();
}

void board_wifi_init(void)
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT ==1)
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
