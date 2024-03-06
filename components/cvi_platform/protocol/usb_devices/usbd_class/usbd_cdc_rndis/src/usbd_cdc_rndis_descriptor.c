#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aos/aos.h>

#include <aos/cli.h>
#include <ulog/ulog.h>

#include "io.h"
#include "usbd_core.h"
#include "usb_cdc.h"

#include "usbd_descriptor.h"
#include "usbd_cdc_rndis.h"
#include "usbd_comp.h"
#include "usbd_cdc_rndis_descriptor.h"


#define CDC_COPY_DESCRIPTORS(mem, src) \
	do { \
		const struct usb_desc_header * const *__src; \
		for (__src = src; *__src; ++__src) { \
			memcpy(mem, *__src, (*__src)->bLength); \
			mem += (*__src)->bLength; \
		} \
	} while (0)

static struct usb_interface_association_descriptor
acm_iad_descriptor = {
	.bLength =		sizeof acm_iad_descriptor,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
	/* .bFirstInterface =	DYNAMIC, */
	.bInterfaceCount = 	2,	// control + data
	.bFunctionClass =	USB_DEVICE_CLASS_WIRELESS,
	.bFunctionSubClass =	CDC_DIRECT_LINE_CONTROL_MODEL,
	.bFunctionProtocol =	CDC_COMMON_PROTOCOL_AT_COMMANDS_PCCA_101_AND_ANNEXO,
    .iFunction = 0,
};

static struct usb_interface_descriptor acm_control_interface_desc = {
	.bLength =		sizeof acm_control_interface_desc,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
    .bAlternateSetting = 0,
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_DEVICE_CLASS_WIRELESS,
	.bInterfaceSubClass =	CDC_DIRECT_LINE_CONTROL_MODEL,
	.bInterfaceProtocol =	CDC_COMMON_PROTOCOL_AT_COMMANDS_PCCA_101_AND_ANNEXO,
	.iInterface = 0,
};

static struct cdc_header_descriptor acm_header_desc = {
	.bFunctionLength =		sizeof(acm_header_desc),
	.bDescriptorType =	CDC_CS_INTERFACE,
	.bDescriptorSubtype =	CDC_FUNC_DESC_HEADER,
	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct cdc_call_management_descriptor
acm_call_mgmt_descriptor = {
	.bFunctionLength =		sizeof(acm_call_mgmt_descriptor),
	.bDescriptorType =	CDC_CS_INTERFACE,
	.bDescriptorSubtype =	CDC_FUNC_DESC_CALL_MANAGEMENT,
	.bmCapabilities =	0,
	/* .bDataInterface = DYNAMIC */
};

static struct cdc_abstract_control_management_descriptor acm_descriptor = {
	.bFunctionLength =		sizeof(acm_descriptor),
	.bDescriptorType =	CDC_CS_INTERFACE,
	.bDescriptorSubtype =	CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT,
	.bmCapabilities =	0x0,
};

static struct cdc_union_descriptor acm_union_desc = {
	.bFunctionLength =		sizeof(acm_union_desc),
	.bDescriptorType =	CDC_CS_INTERFACE,
	.bDescriptorSubtype =	CDC_FUNC_DESC_UNION,
	/* .bMasterInterface0 =	DYNAMIC */
	/* .bSlaveInterface0 =	DYNAMIC */
};

static struct usb_endpoint_descriptor acm_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
    /* .bEndpointAddress =	DYNAMIC */
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(8),
	.bInterval =		0x10,
};

static struct usb_interface_descriptor acm_data_interface_desc = {
	.bLength =		sizeof acm_data_interface_desc,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
    .bAlternateSetting = 0,
	.bNumEndpoints =	2,
	.bInterfaceClass =	CDC_DATA_INTERFACE_CLASS,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	.iInterface = 0,
};

static struct usb_endpoint_descriptor acm_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_ENDPOINT,
	/* .bEndpointAddress =	DYNAMIC */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
    .bInterval          = 0,
    .wMaxPacketSize     = cpu_to_le16(CDC_RNDIS_MPS),
};

static struct usb_endpoint_descriptor acm_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DESCRIPTOR_TYPE_ENDPOINT,
	/* .bEndpointAddress =	DYNAMIC */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
    .bInterval          = 0,
    .wMaxPacketSize     = cpu_to_le16(CDC_RNDIS_MPS),
};

static const struct usb_desc_header *acm_function[] = {
	(struct usb_desc_header *) &acm_iad_descriptor,
	(struct usb_desc_header *) &acm_control_interface_desc,
	(struct usb_desc_header *) &acm_header_desc,
	(struct usb_desc_header *) &acm_call_mgmt_descriptor,
	(struct usb_desc_header *) &acm_descriptor,
	(struct usb_desc_header *) &acm_union_desc,
	(struct usb_desc_header *) &acm_notify_desc,
	(struct usb_desc_header *) &acm_data_interface_desc,
    (struct usb_desc_header *) &acm_out_desc,
	(struct usb_desc_header *) &acm_in_desc,
	NULL,
};

static uint8_t *__cdc_rndis_build_descriptor(uint32_t *len, uint8_t in_ep,  uint8_t out_ep, uint8_t int_ep, uint8_t *interface_total)
{
	uint8_t *desc = NULL;
	void *mem = NULL;
	uint32_t bytes = 0;

	acm_iad_descriptor.bFirstInterface = *interface_total;
	acm_control_interface_desc.bInterfaceNumber = *interface_total;
    acm_call_mgmt_descriptor.bDataInterface = *interface_total + 1;
    acm_union_desc.bMasterInterface = *interface_total;
    acm_union_desc.bSlaveInterface0 = *interface_total + 1;
    acm_notify_desc.bEndpointAddress = int_ep;
    acm_data_interface_desc.bInterfaceNumber = *interface_total + 1;
    acm_out_desc.bEndpointAddress = out_ep;
    acm_in_desc.bEndpointAddress = in_ep;

	for (uint32_t i = 0; acm_function[i] != NULL; i++) {
		bytes += acm_function[i]->bLength;
	}

    if (len) {
		*len = bytes;
	}

    // one extra byte to hold NULL pointer
    bytes += sizeof(void *);

    desc = (uint8_t *)malloc(bytes);

	mem = desc;
	CDC_COPY_DESCRIPTORS(mem, acm_function);

	return desc;
}

uint8_t *cdc_rndis_build_descriptor(struct cdc_rndis_device_info *cdc_rndis_info, uint32_t *desc_len)
{
	uint8_t *desc = NULL;

	desc = __cdc_rndis_build_descriptor(desc_len, cdc_rndis_info->cdc_rndis_in_ep.ep_addr,
        cdc_rndis_info->cdc_rndis_out_ep.ep_addr, cdc_rndis_info->cdc_rndis_int_ep.ep_addr, &cdc_rndis_info->interface_nums);

	return desc;
}

void cdc_rndis_destroy_descriptor(uint8_t *desc)
{
	if (desc) {
		free(desc);
	}
}


