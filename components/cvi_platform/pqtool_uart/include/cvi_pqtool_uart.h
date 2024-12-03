/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: custom_pqtool_uart.h
 * Description:
 */

#ifndef _CUSTOM_PQTOOL_UART_H_
#define _CUSTOM_PQTOOL_UART_H_

void pqtool_uart_deinit(void);
void *pqtool_get_uart(void);
void cli_reg_cmd_pqtool_uart(void);
int write_uart(bool isConnectionByUart, int fd, const void *buf, int size);
int receive_uart(void *data, uint32_t size, uint32_t timeout_ms);
int get_console_need_restart(void);
int set_console_need_restart(int value);

#endif // _CUSTOM_PQTOOL_UART_H_