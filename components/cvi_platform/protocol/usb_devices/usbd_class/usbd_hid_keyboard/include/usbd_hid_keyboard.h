#ifndef __USBD_HID_KEYBOARD_H__
#define __USBD_HID_KEYBOARD_H__

#include <stdint.h>
#include "usbd_core.h"

struct hid_keyboard_device_info {
	struct usbd_endpoint hid_keyboard_int_ep;
	struct usbd_interface hid_keyboard_intf0;
    uint8_t ep;
    uint8_t interface_nums;
};

void hid_keyboard_init(void);
void hid_keyboard_deinit(void);

#endif