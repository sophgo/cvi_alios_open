#ifndef __USBD_CDC_UART_DESCRIPTOR_H__
#define __USBD_CDC_UART_DESCRIPTOR_H__

#include <stdint.h>

#include "usbd_cdc_uart.h"

#define CDC_UART_MPS 64


uint8_t *cdc_uart_build_descriptor(struct cdc_uart_device_info *cdc_uart_info, uint32_t *desc_len);
void cdc_uart_destroy_descriptor(uint8_t *desc);

#endif