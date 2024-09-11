#ifndef __USBD_WINUSB_H__
#define __USBD_WINUSB_H__

#include <stdint.h>
#include "usbd_core.h"

struct winusb_device_info {
	struct usbd_endpoint winusb_in_ep;
    struct usbd_endpoint winusb_out_ep;
	struct usbd_interface winusb_intf0;
    uint8_t ep;
    uint8_t interface_nums;
};

void winusb_desc_register(void);
void winusb_init(void);
void winusb_deinit(void);

#endif