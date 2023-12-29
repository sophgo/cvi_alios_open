/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     board_config.h
 * @brief    header File for pin definition
 * @version  V1.0
 * @date     02. June 2020
 ******************************************************************************/

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <stdint.h>
#include <soc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONSOLE_UART_IDX                (0)
#define CONSOLE_UART_CLK                (1188000000)
#define CONFIG_CLI_USART_BAUD           (115200)
#define CONFIG_CONSOLE_UART_BUFSIZE     (512)

/* CLK_DIV CRG */
#define DIV_CLK_CAM0_200             (0x30020a8)
#define BIT_DIV_RESET_CONT           (0x1U << 0)
#define BIT_SELT_DIV_REG             (0x1U << 3)
#define BIT_CLK_SRC                  (0x1U << 8)
//Clock Divider Factor[20:16],actual frequency = 1188M / Clock Divider Factor
#define BIT_CLK_DIV_FACT_16          (0x1U << 16)
#define BIT_CLK_DIV_FACT_17          (0x1U << 17)
#define BIT_CLK_DIV_FACT_18          (0x1U << 18)
#define BIT_CLK_DIV_FACT_19          (0x1U << 19)
#define BIT_CLK_DIV_FACT_20          (0x1U << 20)

#define WLAN_ENABLE_PIN     PG12
#define WLAN_POWER_PIN      0xFFFFFFFF

#define LED_PIN             PC1     // LED RGB

//PANEL
#if CONFIG_PANEL_ILI9488
#define PANEL_WIDTH 320
#define PANEL_HEIGHT 480
#elif CONFIG_PANEL_HX8394
#define PANEL_WIDTH 720
#define PANEL_HEIGHT 1280
#elif CONFIG_PANEL_ST7701S
#define PANEL_WIDTH 480
#define PANEL_HEIGHT 480
#elif CONFIG_PANEL_GC9503CV
#define PANEL_WIDTH 480
#define PANEL_HEIGHT 800
#else
#define PANEL_WIDTH 720
#define PANEL_HEIGHT 1280
#endif

#define SENSOR_RGB0_WIDTH 1600
#define SENSOR_RGB0_HEIGHT 1200
#define SENSOR_IR0_WIDTH 1600
#define SENSOR_IR0_HEIGHT 1200
#define SENSOR_IR1_WIDTH 1600
#define SENSOR_IR1_HEIGHT 1200

#define GPIO_SPKEN_GRP 0
#define GPIO_SPKEN_NUM 15

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_CONFIG_H_ */

