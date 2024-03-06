#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aos/aos.h>

#include <aos/cli.h>
#include <ulog/ulog.h>

#include "io.h"
#include "usbd_core.h"
#include "usbd_hid.h"

#include "usbd_descriptor.h"
#include "usbd_hid_keyboard.h"
#include "usbd_comp.h"
#include "usbd_hid_keyboard_descriptor.h"

#define HID_KEYBOARD_REPORT_DESC_SIZE 63
#define HID_INT_EP_SIZE     8
#define HID_INT_EP_INTERVAL 10

#define HID_COPY_DESCRIPTORS(mem, src) \
	do { \
		const struct usb_desc_header * const *__src; \
		for (__src = src; *__src; ++__src) { \
			memcpy(mem, *__src, (*__src)->bLength); \
			mem += (*__src)->bLength; \
		} \
	} while (0)

static struct usb_interface_descriptor hid_keyboard_data_interface_desc = {
	.bLength =		sizeof hid_keyboard_data_interface_desc,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
    .bAlternateSetting = 0,
	.bNumEndpoints =	1,
	.bInterfaceClass =	0x03,	/* bInterfaceClass: HID */
	.bInterfaceSubClass =	1,	/* bInterfaceSubClass : 1=BOOT, 0=no boot */
	.bInterfaceProtocol =	1,	/* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	.iInterface = 0,
};

static struct usb_hid_descriptor hid_keyboard_desc = {
	.bLength			= sizeof hid_keyboard_desc,
	.bDescriptorType		= HID_DESCRIPTOR_TYPE_HID,
	.bcdHID				= 0x1101,
	.bCountryCode			= 0x00,
	.bNumDescriptors		= 0x1,
	.subdesc[0].bDescriptorType = 0x22,
	.subdesc[0].wDescriptorLength = cpu_to_le16(HID_KEYBOARD_REPORT_DESC_SIZE),
};

static struct usb_endpoint_descriptor hid_keyboard_int_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_ENDPOINT,
	/* .bEndpointAddress =	DYNAMIC */
	.bmAttributes =		0x03,
    .bInterval          = HID_INT_EP_INTERVAL,
    .wMaxPacketSize     = cpu_to_le16(HID_INT_EP_SIZE),
};

static const struct usb_desc_header *hid_keyboard_function[] = {
	(struct usb_desc_header *) &hid_keyboard_data_interface_desc,
	(struct usb_desc_header *) &hid_keyboard_desc,
	(struct usb_desc_header *) &hid_keyboard_int_desc,
	NULL,
};

static uint8_t *__hid_keyboard_build_descriptor(uint32_t *len, uint8_t int_ep, uint8_t *interface_total)
{
	uint8_t *desc = NULL;
	void *mem = NULL;
	uint32_t bytes = 0;

	hid_keyboard_data_interface_desc.bInterfaceNumber = *interface_total;
	hid_keyboard_int_desc.bEndpointAddress = int_ep;

	for (uint32_t i = 0; hid_keyboard_function[i] != NULL; i++) {
		bytes += hid_keyboard_function[i]->bLength;
	}

    if (len) {
		*len = bytes;
	}

    // one extra byte to hold NULL pointer
    bytes += sizeof(void *);

    desc = (uint8_t *)malloc(bytes);

	mem = desc;
	HID_COPY_DESCRIPTORS(mem, hid_keyboard_function);

	return desc;
}

uint8_t *hid_keyboard_build_descriptor(struct hid_keyboard_device_info *hid_keyboard_info, uint32_t *desc_len)
{
	uint8_t *desc = NULL;

	desc = __hid_keyboard_build_descriptor(desc_len, hid_keyboard_info->hid_keyboard_int_ep.ep_addr, &hid_keyboard_info->interface_nums);

	return desc;
}

void hid_keyboard_destroy_descriptor(uint8_t *desc)
{
	if (desc) {
		free(desc);
	}
}


