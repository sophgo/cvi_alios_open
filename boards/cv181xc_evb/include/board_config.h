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
#define CONFIG_CLI_USART_BAUD           (115200)
#define CONFIG_CONSOLE_UART_BUFSIZE     (512)

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
#elif CONFIG_PANEL_JD9165
#define PANEL_WIDTH 1024
#define PANEL_HEIGHT 600
#else
#define PANEL_WIDTH 720
#define PANEL_HEIGHT 1280
#endif

#define SENSOR_RGB0_WIDTH 1920
#define SENSOR_RGB0_HEIGHT 1080

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_CONFIG_H_ */

