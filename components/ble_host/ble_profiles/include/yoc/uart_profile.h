#ifndef _UART_PROFILE_H
#define _UART_PROFILE_H

#include <aos/ble.h>
#include <stdio.h>

#define YOC_UART_SERVICE_UUID                                                                                          \
    UUID128_DECLARE(0x7e, 0x31, 0x35, 0xd4, 0x12, 0xf3, 0x11, 0xe9, 0xab, 0x14, 0xd6, 0x63, 0xbd, 0x87, 0x3d, 0x93)
#define YOC_UART_RX_UUID                                                                                               \
    UUID128_DECLARE(0x7e, 0x31, 0x35, 0xd4, 0x12, 0xf3, 0x11, 0xe9, 0xab, 0x14, 0xd6, 0x63, 0xbd, 0x87, 0x3d, 0x94)
#define YOC_UART_TX_UUID                                                                                               \
    UUID128_DECLARE(0x7e, 0x31, 0x35, 0xd4, 0x12, 0xf3, 0x11, 0xe9, 0xab, 0x14, 0xd6, 0x63, 0xbd, 0x87, 0x3d, 0x95)

#define RX_MAX_LEN 244

typedef struct _bt_uart_send_cb {
    void (*start)(int err, void *cb_data);
    void (*end)(int err, void *cb_data);
} bt_uart_send_cb;

// typedef void (*uart_service_init_cb)(int err);
typedef int (*ble_event_cb)(ble_event_en, void *);
typedef int (*uart_rx_data_cb)(const uint8_t *, int);

typedef void *uart_handle_t;

#endif