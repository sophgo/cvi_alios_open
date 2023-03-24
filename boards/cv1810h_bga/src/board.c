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
#include "board_config.h"

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

csi_gpio_t chip_gpio_handler;
csi_rtc_t rtc_hdl;
csi_dma_t dma_hdl;

#include <board.h>
#include <devices/devicelist.h>

//#if defined(BOARD_UART_NUM) && BOARD_UART_NUM > 0
#include <stdlib.h>
#include <stdio.h>

void board_uart_init(void) {
  rvm_uart_drv_register(0);
}
//#endif
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

    /* adjust uart clock source to 170MHz */
    mmio_write_32(0x30020a8, 0x70109);
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
    
#else
    csi_dma_init(&dma_hdl, 0);
    board_uart_init();
    board_pinmux_config();
    board_sound_init();
#endif
    board_flash_init();
}
