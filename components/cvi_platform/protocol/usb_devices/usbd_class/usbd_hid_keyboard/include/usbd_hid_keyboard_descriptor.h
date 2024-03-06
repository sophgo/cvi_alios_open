#ifndef __USBD_HID_KEYBOARD_DESCRIPTOR_H__
#define __USBD_HID_KEYBOARD_DESCRIPTOR_H__

#include <stdint.h>

#include "usbd_hid_keyboard.h"

#if CONFIG_USB_HS
#define HID_MPS 512
#else
#define HID_MPS 64
#endif


uint8_t *hid_keyboard_build_descriptor(struct hid_keyboard_device_info *hid_keyboard_info, uint32_t *desc_len);
void hid_keyboard_destroy_descriptor(uint8_t *desc);

#endif