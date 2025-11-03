/*
* Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
*/

/******************************************************************************
 * @file     uart.c
 * @brief    CSI Source File for uart Driver
 * @version  V1.00
 * @date     2022-08-22
 ******************************************************************************/

#include <drv/uart.h>
#include "top_reg.h"
#include "dw_uart_ll.h"

#define UART_SCLK 25000000/* 25MHz */
#define UART_TIMEOUT    0x10000000U

static uint64_t readl(uint64_t addr)
{
    return *(volatile uint64_t*)addr;
}

static void writel(uint64_t value, uint64_t addr)
{
    *(volatile uint64_t*)addr = value;
}

csi_error_t csi_uart_init(csi_uart_t *uart, uint32_t idx)
{
	dw_uart_regs_t *uart_base;

	switch (idx) {
		case UART0:
			uart_base = (dw_uart_regs_t *)UART0_BASE;
			writel(readl(SOFT_RSTN_ADDR) & ~UART0_RSTN_OFFSET, SOFT_RSTN_ADDR);
			udelay(10);
			writel(readl(SOFT_RSTN_ADDR) | UART0_RSTN_OFFSET, SOFT_RSTN_ADDR);
			udelay(10);
			break;
		case UART1:
			uart_base = (dw_uart_regs_t *)UART1_BASE;
			break;
		case UART2:
			uart_base = (dw_uart_regs_t *)UART2_BASE;
			break;
		case UART3:
			uart_base = (dw_uart_regs_t *)UART3_BASE;
			break;
		default:
			return CSI_ERROR;
	}
    uart->dev.reg_base = (unsigned long)uart_base;

	dw_uart_fifo_init(uart_base);
	uart->rx_size = 0U;
	uart->tx_size = 0U;
	uart->rx_data = NULL;
	uart->tx_data = NULL;
	uart->tx_dma  = NULL;
	uart->rx_dma  = NULL;
	dw_uart_disable_trans_irq(uart_base);
	dw_uart_disable_recv_irq(uart_base);

    return CSI_OK;
}

void csi_uart_uninit(csi_uart_t *uart)
{
    dw_uart_regs_t *uart_base = (dw_uart_regs_t *)uart->dev.reg_base;

    uart->priv = NULL;
    uart->rx_size = 0U;
    uart->tx_size = 0U;
    uart->rx_data = NULL;
    uart->tx_data = NULL;

    dw_uart_disable_trans_irq(uart_base);
    dw_uart_disable_recv_irq(uart_base);
}

csi_error_t csi_uart_baud(csi_uart_t *uart, uint32_t baud)
{
    int32_t ret = 0;
    dw_uart_regs_t *uart_base = (dw_uart_regs_t *)uart->dev.reg_base;

    ret = dw_uart_config_baudrate(uart_base, baud, UART_SCLK);
    if (ret == 0)
        return CSI_OK;
    else
        return CSI_ERROR;
}

csi_error_t csi_uart_format(csi_uart_t *uart,  csi_uart_data_bits_t data_bits,
                            csi_uart_parity_t parity, csi_uart_stop_bits_t stop_bits)
{
    int32_t ret = 0;
    csi_error_t csi_ret = CSI_OK;
    dw_uart_regs_t *uart_base = (dw_uart_regs_t *)uart->dev.reg_base;

    switch (data_bits) {
        case UART_DATA_BITS_5:
            ret = dw_uart_config_data_bits(uart_base, 5U);
            break;

        case UART_DATA_BITS_6:
            ret = dw_uart_config_data_bits(uart_base, 6U);
            break;

        case UART_DATA_BITS_7:
            ret = dw_uart_config_data_bits(uart_base, 7U);
            break;

        case UART_DATA_BITS_8:
            ret = dw_uart_config_data_bits(uart_base, 8U);
            break;

        default:
            ret = -1;
            break;
    }

    if (ret == 0) {
        switch (parity) {
            case UART_PARITY_NONE:
                ret = dw_uart_config_parity_none(uart_base);
                break;

            case UART_PARITY_ODD:
                ret = dw_uart_config_parity_odd(uart_base);
                break;

            case UART_PARITY_EVEN:
                ret = dw_uart_config_parity_even(uart_base);
                break;

            default:
                ret = -1;
                break;
        }

        if (ret == 0) {
            switch (stop_bits) {
                case UART_STOP_BITS_1:
                    ret = dw_uart_config_stop_bits(uart_base, 1U);
                    break;

                case UART_STOP_BITS_2:
                    ret = dw_uart_config_stop_bits(uart_base, 2U);
                    break;

                case UART_STOP_BITS_1_5:
                    if (data_bits == UART_DATA_BITS_5) {
                        ret = dw_uart_config_stop_bits(uart_base, 2U);
                        break;
                    }

                default:
                    ret = -1;
                    break;
            }

            if (ret != 0) {
                csi_ret = CSI_ERROR;
            }

        } else {
            csi_ret = CSI_ERROR;
        }

    } else {
        csi_ret = CSI_ERROR;
    }

    return csi_ret;
}

csi_error_t csi_uart_flowctrl(csi_uart_t *uart,  csi_uart_flowctrl_t flowctrl)
{
    csi_error_t csi_ret = CSI_OK;
    dw_uart_regs_t *uart_base = (dw_uart_regs_t *)uart->dev.reg_base;

    switch (flowctrl) {
        case UART_FLOWCTRL_CTS:
            dw_uart_wait_idle(uart_base);
            break;

        case UART_FLOWCTRL_RTS_CTS:
            dw_uart_wait_idle(uart_base);
            break;

        case UART_FLOWCTRL_NONE:
            dw_uart_wait_idle(uart_base);
            break;

        case UART_FLOWCTRL_RTS:
        default:
            csi_ret = CSI_UNSUPPORTED;
            break;
    }

    return csi_ret;
}

void csi_uart_putc(csi_uart_t *uart, uint8_t ch)
{
	uint32_t timeout = UART_TIMEOUT;
    dw_uart_regs_t *uart_base = (dw_uart_regs_t *)uart->dev.reg_base;

    while (!dw_uart_putready(uart_base) && timeout--);

    if (timeout) {
        dw_uart_putchar(uart_base, ch);
    }
}

uint8_t csi_uart_getc(csi_uart_t *uart)
{
    dw_uart_regs_t *uart_base = (dw_uart_regs_t *)uart->dev.reg_base;

    while (!dw_uart_getready(uart_base));

    return dw_uart_getchar(uart_base);
}

int32_t csi_uart_receive(csi_uart_t *uart, void *data, uint32_t size, uint32_t timeout)
{
    int rc = 0;
    uint8_t *pdata = data;

    while (rc < size) {
        pdata[rc] = csi_uart_getc(uart);
        rc++;
    }

    return rc;
}

csi_error_t csi_uart_receive_async(csi_uart_t *uart, void *data, uint32_t size)
{
    //TODO:
    return CSI_ERROR;
}

int32_t csi_uart_send(csi_uart_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
    int rc = 0;
    uint8_t *pdata = (uint8_t *)data;

    while (rc < size) {
        csi_uart_putc(uart, pdata[rc]);
        rc++;
    }

    return rc;
}

csi_error_t csi_uart_send_async(csi_uart_t *uart, const void *data, uint32_t size)
{
    //TODO:
    return CSI_ERROR;
}

csi_error_t csi_uart_attach_callback(csi_uart_t *uart, void  *callback, void *arg)
{
    //TODO:
    return CSI_OK;
}

void csi_uart_detach_callback(csi_uart_t *uart)
{
    //TODO:
}

csi_error_t csi_uart_get_state(csi_uart_t *uart, csi_state_t *state)
{
    //TODO:
    return CSI_OK;
}


