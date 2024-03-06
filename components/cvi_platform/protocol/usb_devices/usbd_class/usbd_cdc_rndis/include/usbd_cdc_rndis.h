#ifndef __USBD_CDC_RNDIS_H__
#define __USBD_CDC_RNDIS_H__

#include <stdint.h>
#include "usbd_core.h"

struct cdc_rndis_device_info {
	struct usbd_endpoint cdc_rndis_in_ep;
	struct usbd_endpoint cdc_rndis_out_ep;
	struct usbd_endpoint cdc_rndis_int_ep;
	struct usbd_interface cdc_rndis_intf0;
	struct usbd_interface cdc_rndis_intf1;
    uint8_t ep;
    uint8_t interface_nums;
};

void cdc_rndis_init(void);
void cdc_rndis_deinit(void);

void rndis_device_lwip_init(void);
void rndis_device_init(void);
void rndis_device_deinit(void);

#endif