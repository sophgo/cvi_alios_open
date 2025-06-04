/**
 * @file uart.h
 * @copyright Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup hal_uart UART
 *  uart hal API.
 *
 *  @{
 */

#include <stdint.h>

#define HAL_SHADOW_PORT_MASK 0x80

/* Define the wait forever timeout macro */
#define HAL_WAIT_FOREVER 0xFFFFFFFFU

/*
 * UART data width
 */
typedef enum {
    DATA_WIDTH_5BIT,
    DATA_WIDTH_6BIT,
    DATA_WIDTH_7BIT,
    DATA_WIDTH_8BIT,
    DATA_WIDTH_9BIT
} hal_uart_data_width_t;

/*
 * UART stop bits
 */
typedef enum {
    STOP_BITS_1,
    STOP_BITS_2
} hal_uart_stop_bits_t;

/*
 * UART flow control
 */
typedef enum {
    FLOW_CONTROL_DISABLED,  /**< Flow control disabled */
    FLOW_CONTROL_CTS,       /**< Clear to send, yet to send data */
    FLOW_CONTROL_RTS,       /**< Require to send, yet to receive data */
    FLOW_CONTROL_CTS_RTS    /**< Both CTS and RTS flow control */
} hal_uart_flow_control_t;

/*
 * UART parity
 */
typedef enum {
    NO_PARITY,      /**< No parity check */
    ODD_PARITY,     /**< Odd parity check */
    EVEN_PARITY     /**< Even parity check */
} hal_uart_parity_t;

/*
 * UART mode
 */
typedef enum {
    MODE_TX,        /**< Uart in send mode */
    MODE_RX,        /**< Uart in receive mode */
    MODE_TX_RX      /**< Uart in send and receive mode */
} hal_uart_mode_t;

/*
 * UART configuration
 */
typedef struct {
    uint32_t                baud_rate;      /**< Uart baud rate */
    hal_uart_data_width_t   data_width;     /**< Uart data width */
    hal_uart_parity_t       parity;         /**< Uart parity check mode */
    hal_uart_stop_bits_t    stop_bits;      /**< Uart stop bit mode */
    hal_uart_flow_control_t flow_control;   /**< Uart flow control mode */
    hal_uart_mode_t         mode;           /**< Uart send/receive mode */
} uart_config_t;

/*
 * UART dev handle
 */
typedef struct {
    uint8_t        port;   /**< uart port */
    uart_config_t  config; /**< uart config */
    void          *priv;   /**< priv data */
} uart_dev_t;

typedef int32_t (*uart_rx_cb)(uart_dev_t *uart);

/**
 * Initialises a UART interface
 *
 *
 * @param[in]  uart  the interface which should be initialised
 *
 * @return  0 : on success,  otherwise is error
 */
int32_t hal_uart_init(uart_dev_t *uart);

/**
 * Transmit data on a UART interface
 *
 * @param[in]  uart     the UART interface
 * @param[in]  data     pointer to the start of data
 * @param[in]  size     number of bytes to transmit
 * @param[in]  timeout  timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                      if you want to wait forever
 *
 * @return  0 : on success,  otherwise is error
 */
int32_t hal_uart_send(uart_dev_t *uart, const void *data, uint32_t size, uint32_t timeout);

int32_t hal_uart_send_irq(uart_dev_t *uart, const void *data, uint32_t size);

int32_t hal_uart_flush_ringbuffer(uart_dev_t *uart);

/**
 * Transmit data on a UART interface with polling
 *
 * @param[in]  uart     the UART interface
 * @param[in]  data     pointer to the start of data
 * @param[in]  size     number of bytes to transmit
 *
 * @return  0 : on success,  otherwise is error
 */
int32_t hal_uart_send_poll(uart_dev_t *uart, const void *data, uint32_t size);

/**
 * Receive data on a UART interface
 *
 * @param[in]   uart         the UART interface
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[in]   timeout      timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success,  otherwise is error
 */
int32_t hal_uart_recv(uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t timeout);


/**
 * Receive data on a UART interface with polling
 *
 * @param[in]   uart         the UART interface
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[in]   timeout      timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success,  otherwise is error
 */
int32_t hal_uart_recv_poll(uart_dev_t *uart, void *data, uint32_t expect_size);


/**
 * Receive data on a UART interface
 *
 * @param[in]   uart         the UART interface
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[out]  recv_size    number of bytes trully received
 * @param[in]   timeout      timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success,  otherwise is error
 */
int32_t hal_uart_recv_II(uart_dev_t *uart, void *data, uint32_t expect_size,
                         uint32_t *recv_size, uint32_t timeout);

/*
 *
 * @param [in]   uart         the UART interface
 * @param [in]   rx_cb        Non-zero pointer is the rx callback handler;
 *                            NULL pointer for rx_cb unregister operation
 *                            uart in rx_cb must be the same pointer with uart pointer passed to hal_uart_recv_cb_reg
 *                            driver must notify upper layer by calling rx_cb if data is available in UART's hw or rx buffer
 * @return 0: on success, negative no.: if an error occured with any step
 */
int32_t hal_uart_recv_cb_reg(uart_dev_t *uart, uart_rx_cb cb);

/**
 * Deinitialises a UART interface
 *
 * @param[in]  uart  the interface which should be deinitialised
 *
 * @return  0 : on success,  otherwise is error
 */
int32_t hal_uart_finalize(uart_dev_t *uart);

/**
 *  push data to uart recv ringbuffer
 *  virtual uart use it
*/
int32_t hal_uart_push_data_to_ringbuffer(uart_dev_t *uart, const void *data, uint32_t size);

/**
 *  flush virtual uart share memory last segment cpu cache
 *  virtual uart use it
*/
int32_t hal_uart_flush_cache(uart_dev_t *uart);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* HAL_UART_H */

