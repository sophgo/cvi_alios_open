/*
 * Copyright (C) 2018-2020 Alibaba Group Holding Limited
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

// uart config
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

/**
 * @brief  init the board for default: pin mux, etc.
 * re-implement if need.
 * @return
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */

