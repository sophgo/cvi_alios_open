#ifndef __USBD_WINUSB_DESCRIPTOR_H__
#define __USBD_WINUSB_DESCRIPTOR_H__

#include <stdint.h>

#include "usbd_winusb.h"

uint8_t* winusb_build_descriptor(struct winusb_device_info* winusb_info, uint32_t* desc_len);
void winusb_destroy_descriptor(uint8_t* desc);
struct usb_msosv1_descriptor* winusb_msosv1_build_descriptor(void);

#endif