/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     wj_wrap_uart.c
 * @brief    CSI Source File for WRAP UART Driver
 * @version  V1.0
 * @date     19. Feb 2020
 ******************************************************************************/
#include <drv/cvi_irq.h>
#include <drv/uart.h>
#include <wj_usi_ll.h>
#include <drv/usi_usart.h>
#include <drv/tick.h>

/**
  \brief       Initialize UART Interface. 1. Initializes the resources needed for the UART interface 2.registers event callback function
  \param[in]   uart      operate handle.
  \param[in]   idx       the device idx
  \param[in]   cb_event  event call back function \ref uart_event_cb_t
  \param[in]   arg       user can define it by himself
  \return      error code
*/
csi_error_t csi_uart_init(csi_uart_t *uart, uint32_t idx)
{
    return csi_usi_uart_init(uart, idx);
}

/**
  \brief       De-initialize UART Interface. stops operation and releases the software resources used by the interface
  \param[in]   uart  operate handle.
  \return      error code
*/
void csi_uart_uninit(csi_uart_t *uart)
{
    csi_usi_uart_uninit(uart);
}

/**
  \brief       attach the callback handler to UART
  \param[in]   uart  operate handle.
  \param[in]   cb    callback function
  \param[in]   arg   user can define it by himself as callback's param
  \return      error code
*/
csi_error_t csi_uart_attach_callback(csi_uart_t *uart, void *cb, void *arg)
{
    return csi_usi_uart_attach_callback(uart, cb, arg);
}

/**
  \brief       detach the callback handler
  \param[in]   uart  operate handle.
*/
void csi_uart_detach_callback(csi_uart_t *uart)
{
    csi_usi_uart_detach_callback(uart);

}


/**
  \brief       config the baudrate.
  \param[in]   uart  uart handle to operate.
  \param[in]   baud  uart baudrate
  \return      error code
*/
csi_error_t csi_uart_baud(csi_uart_t *uart, uint32_t baud)
{
    return csi_usi_uart_baud(uart,  baud);
}

/**
  \brief       config the uart format.
  \param[in]   uart      uart handle to operate.
  \param[in]   data_bit  uart data bits
  \param[in]   parity    uart data parity
  \param[in]   stop_bit  uart stop bits
  \return      error code
*/
csi_error_t csi_uart_format(csi_uart_t *uart,  csi_uart_data_bits_t data_bits,
                            csi_uart_parity_t parity, csi_uart_stop_bits_t stop_bits)
{
    return csi_usi_uart_format(uart,   data_bits, parity,  stop_bits);
}

/**
  \brief       config the uart flow control.
  \param[in]   uart      uart handle to operate.
  \param[in]   flowctrl  uart flow control
  \return      error code
*/
csi_error_t csi_uart_flowctrl(csi_uart_t *uart,  csi_uart_flowctrl_t flowctrl)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       Start sending data to UART transmitter.
  \param[in]   uart     uart handle to operate.
  \param[in]   data     Pointer to buffer with data to send to UART transmitter. data_type is : uint8_t for 5..8 data bits, uint16_t for 9 data bits
  \param[in]   num      Number of data items to send (byte)
  \param[in]   timeout  is the number of queries, not time
  \return      the num of data witch is send successful
*/
int32_t csi_uart_send(csi_uart_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
    return (int32_t)csi_usi_uart_send(uart,  data,  size,  timeout);
}

/**
  \brief       Start sending data to UART transmitter (interrupt mode).
  \param[in]   uart   uart handle to operate.
  \param[in]   data     Pointer to buffer with data to send to UART transmitter. data_type is : uint8_t for 5..8 data bits, uint16_t for 9 data bits
  \param[in]   num      Number of data items to send
  \return      the status of send func
*/
csi_error_t csi_uart_send_async(csi_uart_t *uart, const void *data, uint32_t size)
{
    return csi_usi_uart_send_async(uart, data,  size);
}

/**
  \brief       Get the num of data in RX_FIFO.
  \param[in]   uart   uart handle to operate.
  \return      the num of data in RX_FIFO
*/
uint32_t csi_uart_get_recvfifo_waiting_num(csi_uart_t *uart)
{
    return csi_usi_uart_get_recvfifo_waiting_num(uart);
}

/**
  \brief       Start receiving data from UART receiver. \n
               This function is non-blocking,\ref uart_event_e is signaled when operation completes or error happens.
               \ref csi_uart_get_status can get operation status.
  \param[in]   uart  uart handle to operate.
  \param[out]  data  Pointer to buffer for data to receive from UART receiver.data_type is : uint8_t for 5..8 data bits, uint16_t for 9 data bits
  \param[in]   num   Number of data items to receive
  \return      error code
*/
csi_error_t csi_uart_receive_async(csi_uart_t *uart, void *data, uint32_t size)
{
    return csi_usi_uart_receive_async(uart, data,  size);
}

/**
  \brief       query data from UART receiver FIFO.
  \param[in]   uart  uart handle to operate.
  \param[out]  data  Pointer to buffer for data to receive from UART receiver
  \param[in]   num   Number of data items to receive
  \param[in]   timeout  is the number of queries, not time
  \return      fifo data num to receive
*/
int32_t csi_uart_receive(csi_uart_t *uart, void *data, uint32_t size, uint32_t timeout)
{
    return csi_usi_uart_receive(uart, data,  size,  timeout);
}

/**
  \brief       get character in query mode.
  \param[in]   uart  uart handle to operate.
  \param[out]  ch the pointer to the received character.
  \return      error code
*/
uint8_t  csi_uart_getchar(csi_uart_t *uart)
{
    return  csi_usi_uart_getchar(uart);
}

/**
  \brief       transmit character in query mode.
  \param[in]   uart  uart handle to operate.
  \param[in]   ch  the input character
  \return      error code
*/
void csi_uart_putc(csi_uart_t *uart, uint8_t ch)
{
    csi_usi_uart_putchar(uart,  ch);
}

csi_error_t csi_uart_link_dma(csi_uart_t *uart, csi_dma_ch_t *tx_dma, csi_dma_ch_t *rx_dma)
{
    return csi_usi_uart_link_dma(uart, tx_dma, rx_dma);
}
/**
  \brief       Get the state of uart device.
  \param[in]   uart   uart handle to operate.
  \param[out]  state  the state of uart device.
  \return      error code.
*/
csi_error_t csi_uart_get_state(csi_uart_t *uart, csi_state_t *state)
{
    return csi_usi_uart_get_state(uart, state);
}

/**
  \brief       Enable uart power manage.
  \param[in]   uart   uart handle to operate.
  \return      error code.
*/
csi_error_t csi_uart_enable_pm(csi_uart_t *uart)
{
    return csi_usi_uart_enable_pm(uart);
}

/**
  \brief       Disable uart power manage.
  \param[in]   uart   uart handle to operate.
*/
void csi_uart_disable_pm(csi_uart_t *uart)
{
    csi_usi_uart_disable_pm(uart);
}