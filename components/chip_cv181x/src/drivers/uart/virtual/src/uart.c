/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

/******************************************************************************
 * @file     uart.c
 * @brief    CSI Source File for virtual uart Driver
 * @version  V1
 * @date     2023-08-09
 ******************************************************************************/

#include <drv/uart.h>
#include <drv/dma.h>
#include <drv/cvi_irq.h>
#include <drv/gpio.h>
#include <drv/pin.h>
#include <drv/porting.h>
#include <soc.h>
#include <drv/tick.h>
#include <aos/kernel.h>
#include <mmio.h>

#include "board.h"
#include "cvi_board_memmap.h"

#define UART_SHARE_BUFFER       (CVIMMAP_ALIOS_LOG_ADDR)
#define UART_SHARE_BUFFER_SIZE  (CVIMMAP_ALIOS_LOG_SIZE)
#define UART_SHARE_BUFFER_READ_CONFIG    UART_SHARE_BUFFER
#define UART_SHARE_BUFFER_WRITE_CONFIG   (UART_SHARE_BUFFER + UART_SHARE_BUFFER_SIZE - 0x4)

#define UART_BUFFER_BASE        (UART_SHARE_BUFFER + 64)
#define UART_BUFFER_SIZE        (UART_SHARE_BUFFER_SIZE - 128)

#define C906_CACHE_LINE_SIZE    (64)
#define SEGMENT_SIZE            C906_CACHE_LINE_SIZE
#define TOTAL_SEGMENT           (UART_BUFFER_SIZE/SEGMENT_SIZE)

typedef struct virtual_uart {
    uint32_t seg_idx; // index of segment which's in use for write
    uint32_t mirror_times; // mirror times for write in ringbuffer
    uint32_t idx; // buffer index for write
    char *buffer;
} virtual_uart_s;

static inline void renew_write_config_for_cli_end(uint32_t idx)
{
    uint32_t seg_idx;

    // write idx
    mmio_write_32(UART_SHARE_BUFFER_WRITE_CONFIG - 0x8, idx);

    // clean cpu cache
    csi_dcache_clean_range((uint64_t *)((uint64_t)(UART_SHARE_BUFFER + UART_SHARE_BUFFER_SIZE - 64)), 64);

    if (idx & 0x3F) {
        seg_idx = idx >> 6;

        csi_dcache_clean_range((uint64_t *)((uint64_t)UART_BUFFER_BASE + (seg_idx << 6)), 64);
    }
}

static inline void renew_write_config(uint32_t seg_idx)
{
    // write segment id
    mmio_write_32(UART_SHARE_BUFFER_WRITE_CONFIG, seg_idx);

    // clean cpu cache
    csi_dcache_clean_range((uint64_t *)((uint64_t)(UART_SHARE_BUFFER + UART_SHARE_BUFFER_SIZE - 64)), 64);
}

static inline void renew_write_config_after_mirror(virtual_uart_s *vuart, uint32_t seg_idx)
{
    // change mirror times
    vuart->mirror_times++;
    mmio_write_32(UART_SHARE_BUFFER_WRITE_CONFIG - 0x4, vuart->mirror_times);

    renew_write_config(seg_idx - TOTAL_SEGMENT);
}

static uint32_t push_string_to_buffer(csi_uart_t *uart, const uint8_t *data, uint32_t size)
{
    virtual_uart_s *vuart = (virtual_uart_s *)uart->priv;

    uint32_t index = (vuart->idx + size);
    uint32_t seg_idx = index >> 6;

    if (seg_idx != vuart->seg_idx) {
        if (seg_idx >= TOTAL_SEGMENT) {
            renew_write_config_after_mirror(vuart, seg_idx);

            int availed_size = UART_BUFFER_SIZE - vuart->idx;

            if (availed_size > 0) {
                memcpy(vuart->buffer + vuart->idx, data, availed_size);
            }

            csi_dcache_clean_range((uint64_t *)((uint64_t)UART_BUFFER_BASE + (vuart->seg_idx << 6)), (TOTAL_SEGMENT - vuart->seg_idx) << 6);
            memcpy(vuart->buffer, data + availed_size, size - availed_size);
            if ((seg_idx - TOTAL_SEGMENT) > 0) {
                csi_dcache_clean_range((uint64_t *)((uint64_t)UART_BUFFER_BASE), (seg_idx - TOTAL_SEGMENT) << 6);
            }

            vuart->idx = index - UART_BUFFER_SIZE;
            vuart->seg_idx = seg_idx - TOTAL_SEGMENT;
        } else {
            renew_write_config(seg_idx);

            memcpy(vuart->buffer + vuart->idx, data, size);
            csi_dcache_clean_range((uint64_t *)((uint64_t)UART_BUFFER_BASE + (vuart->seg_idx << 6)), (seg_idx - vuart->seg_idx) << 6);

            vuart->idx = index;
            vuart->seg_idx = seg_idx;
        }
    } else {
        memcpy(vuart->buffer + vuart->idx, data, size);
        vuart->idx = index;
    }

    return vuart->idx;
}

csi_error_t csi_uart_init(csi_uart_t *uart, uint32_t idx)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    virtual_uart_s *vuart = NULL;

    vuart = (virtual_uart_s *)malloc(sizeof(virtual_uart_s));
    if (NULL == vuart) {
        return CSI_ERROR;
    }

    vuart->seg_idx = 0;
    vuart->mirror_times = 0;
    vuart->idx = 0;
    vuart->buffer = (char *)UART_BUFFER_BASE;

    uart->priv = vuart;
    uart->rx_size = 0U;
    uart->tx_size = 0U;
    uart->rx_data = NULL;
    uart->tx_data = NULL;
    uart->tx_dma  = NULL;
    uart->rx_dma  = NULL;

    return ret;
}

void csi_uart_uninit(csi_uart_t *uart)
{
    CSI_PARAM_CHK_NORETVAL(uart);

    free(uart->priv);

    uart->priv = NULL;
    uart->rx_size = 0U;
    uart->tx_size = 0U;
    uart->rx_data = NULL;
    uart->tx_data = NULL;
}

csi_error_t csi_uart_baud(csi_uart_t *uart, uint32_t baud)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);

    return CSI_OK;
}

csi_error_t csi_uart_format(csi_uart_t *uart,  csi_uart_data_bits_t data_bits,
                            csi_uart_parity_t parity, csi_uart_stop_bits_t stop_bits)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);

    return CSI_OK;
}

csi_error_t csi_uart_flowctrl(csi_uart_t *uart,  csi_uart_flowctrl_t flowctrl)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);

    return CSI_OK;
}

void csi_uart_putc(csi_uart_t *uart, uint8_t ch)
{
    CSI_PARAM_CHK_NORETVAL(uart);

    push_string_to_buffer(uart, &ch, 1);
}

uint8_t csi_uart_getc(csi_uart_t *uart)
{
    CSI_PARAM_CHK(uart, 0U);

    return 0;
}

int32_t csi_uart_receive(csi_uart_t *uart, void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);

    return 0;
}

csi_error_t virtual_uart_receive_intr(csi_uart_t *uart, void *data, uint32_t num)
{
    return CSI_OK;
}

csi_error_t csi_uart_receive_async(csi_uart_t *uart, void *data, uint32_t size)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(uart->callback, CSI_ERROR);
    CSI_PARAM_CHK(uart->receive, CSI_ERROR);

    csi_error_t ret;

    ret = uart->receive(uart, data, size);

    if (ret == CSI_OK) {
        uart->state.readable = 0U;
    }

    return ret;
}

int32_t csi_uart_send(csi_uart_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
    /* check data and uart */
    CSI_PARAM_CHK(uart, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);

    return 0;
}

csi_error_t virtual_uart_send_intr(csi_uart_t *uart, const void *data, uint32_t size)
{
    push_string_to_buffer(uart, (uint8_t*)data, size);

    return CSI_OK;
}

csi_error_t csi_uart_send_async(csi_uart_t *uart, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(uart->callback, CSI_ERROR);
    CSI_PARAM_CHK(uart->send, CSI_ERROR);

    csi_error_t ret;
    ret = uart->send(uart, data, size);

    if (ret == CSI_OK) {
        uart->state.writeable = 0U;
    }

    return ret;
}

csi_error_t csi_uart_attach_callback(csi_uart_t *uart, void  *callback, void *arg)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);

    uart->callback = callback;
    uart->arg = arg;
    uart->send = virtual_uart_send_intr;
    uart->receive = virtual_uart_receive_intr;

    return CSI_OK;
}

void csi_uart_detach_callback(csi_uart_t *uart)
{
    CSI_PARAM_CHK_NORETVAL(uart);

    uart->callback  = NULL;
    uart->arg = NULL;
    uart->send = NULL;
    uart->receive = NULL;
}

csi_error_t csi_uart_get_state(csi_uart_t *uart, csi_state_t *state)
{
    CSI_PARAM_CHK(uart, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);

    *state = uart->state;

    return CSI_OK;
}

csi_error_t dw_uart_send_dma(csi_uart_t *uart, const void *data, uint32_t num)
{
    return CSI_OK;
}

csi_error_t dw_uart_receive_dma(csi_uart_t *uart, void *data, uint32_t num)
{
    return CSI_OK;
}

csi_error_t csi_uart_link_dma(csi_uart_t *uart, csi_dma_ch_t *tx_dma, csi_dma_ch_t *rx_dma)
{
    return CSI_OK;
}

void csi_uart_flush_cache(csi_uart_t *uart)
{
    virtual_uart_s *vuart = (virtual_uart_s *)uart->priv;

    renew_write_config_for_cli_end(vuart->idx);
}

