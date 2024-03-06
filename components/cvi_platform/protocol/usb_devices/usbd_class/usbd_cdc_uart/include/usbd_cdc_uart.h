#ifndef __USBD_CDC_UART_H__
#define __USBD_CDC_UART_H__

#include <stdint.h>
#include "usbd_core.h"

struct cdc_uart_device_info {
	struct usbd_endpoint cdc_uart_in_ep;
	struct usbd_endpoint cdc_uart_out_ep;
	struct usbd_endpoint cdc_uart_int_ep;
	struct usbd_interface cdc_uart_intf0;
	struct usbd_interface cdc_uart_intf1;
    uint8_t ep;
    uint8_t interface_nums;
};

void cdc_uart_init(void);
void cdc_uart_deinit(void);

#endif