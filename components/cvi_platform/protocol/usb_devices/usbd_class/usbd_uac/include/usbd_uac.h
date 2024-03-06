#ifndef __USBD_UAC_H__
#define __USBD_UAC_H__

#include <stdint.h>
#include "usbd_core.h"

struct uac_device_info {
	struct usbd_endpoint audio_in_ep;
	struct usbd_endpoint audio_out_ep;
	struct usbd_interface uac_intf0;
	struct usbd_interface uac_intf1;
	struct usbd_interface uac_intf2;
    uint8_t ep;
    uint8_t interface_nums;
};

int uac_init(void);
int uac_deinit(void);

#endif