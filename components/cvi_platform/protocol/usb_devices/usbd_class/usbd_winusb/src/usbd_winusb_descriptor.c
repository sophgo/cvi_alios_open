#include <aos/aos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <aos/cli.h>
#include <ulog/ulog.h>

#include "io.h"
#include "usbd_core.h"

#include "usbd_comp.h"
#include "usbd_descriptor.h"
#include "usbd_winusb.h"
#include "usbd_winusb_descriptor.h"

#define WINUSB_EP_SIZE 512
#define WCID_VENDOR_CODE 0x17
#define WINUSB_COPY_DESCRIPTORS(mem, src)           \
    do {                                            \
        const struct usb_desc_header* const* __src; \
        for (__src = src; *__src; ++__src) {        \
            memcpy(mem, *__src, (*__src)->bLength); \
            mem += (*__src)->bLength;               \
        }                                           \
    } while (0)

static struct usb_interface_descriptor winusb_data_interface_desc = {
    .bLength = sizeof winusb_data_interface_desc,
    .bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE,
    /* .bInterfaceNumber = DYNAMIC */
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = 0xff,
    .bInterfaceSubClass = 0xff,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
};

static struct usb_endpoint_descriptor winusb_in_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
    /* .bEndpointAddress =	DYNAMIC */
    .bmAttributes = 0x02,
    .wMaxPacketSize = cpu_to_le16(WINUSB_EP_SIZE),
    .bInterval = 0,
};

static struct usb_endpoint_descriptor winusb_out_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
    /* .bEndpointAddress =	DYNAMIC */
    .bmAttributes = 0x02,
    .wMaxPacketSize = cpu_to_le16(WINUSB_EP_SIZE),
    .bInterval = 0,
};

static const struct usb_desc_header* winusb_function[] = {
    (struct usb_desc_header*)&winusb_data_interface_desc,
    (struct usb_desc_header*)&winusb_in_desc,
    (struct usb_desc_header*)&winusb_out_desc,
    NULL,
};


__ALIGN_BEGIN uint8_t WCID_StringDescriptor_MSOS[18] __ALIGN_END = {
    ///////////////////////////////////////
    /// MS OS string descriptor
    ///////////////////////////////////////
    0x12,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    /* MSFT100 */
    'M', 0x00, 'S', 0x00, 'F', 0x00, 'T', 0x00, /* wcChar_7 */
    '1', 0x00, '0', 0x00, '0', 0x00,            /* wcChar_7 */
    WCID_VENDOR_CODE,                           /* bVendorCode */
    0x00,                                       /* bReserved */
};

__ALIGN_BEGIN uint8_t WINUSB_WCIDDescriptor[40] __ALIGN_END = {
    ///////////////////////////////////////
    /// WCID descriptor
    ///////////////////////////////////////
    0x28, 0x00, 0x00, 0x00,                   /* dwLength */
    0x00, 0x01,                               /* bcdVersion */
    0x04, 0x00,                               /* wIndex */
    0x01,                                     /* bCount */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bReserved_7 */

    ///////////////////////////////////////
    /// WCID function descriptor
    ///////////////////////////////////////
    0x00, /* bFirstInterfaceNumber */
    0x01, /* bReserved */
    /* WINUSB */
    'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00, /* cCID_8 */
    /*  */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* cSubCID_8 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             /* bReserved_6 */
};

__ALIGN_BEGIN uint8_t WINUSB_IF0_WCIDProperties [142] __ALIGN_END = {
  ///////////////////////////////////////
  /// WCID property descriptor
  ///////////////////////////////////////
  0x8e, 0x00, 0x00, 0x00,                           /* dwLength */
  0x00, 0x01,                                       /* bcdVersion */
  0x05, 0x00,                                       /* wIndex */
  0x01, 0x00,                                       /* wCount */

  ///////////////////////////////////////
  /// registry propter descriptor
  ///////////////////////////////////////
  0x84, 0x00, 0x00, 0x00,                           /* dwSize */
  0x01, 0x00, 0x00, 0x00,                           /* dwPropertyDataType */
  0x28, 0x00,                                       /* wPropertyNameLength */
  /* DeviceInterfaceGUID */
  'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00,       /* wcName_20 */
  'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00,       /* wcName_20 */
  't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00,       /* wcName_20 */
  'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00,       /* wcName_20 */
  'U', 0x00, 'I', 0x00, 'D', 0x00, 0x00, 0x00,      /* wcName_20 */
  0x4e, 0x00, 0x00, 0x00,                           /* dwPropertyDataLength */
  /* {1D4B2365-4749-48EA-B38A-7C6FDDDD7E26} */
  '{', 0x00, '1', 0x00, 'D', 0x00, '4', 0x00,       /* wcData_39 */
  'B', 0x00, '2', 0x00, '3', 0x00, '6', 0x00,       /* wcData_39 */
  '5', 0x00, '-', 0x00, '4', 0x00, '7', 0x00,       /* wcData_39 */
  '4', 0x00, '9', 0x00, '-', 0x00, '4', 0x00,       /* wcData_39 */
  '8', 0x00, 'E', 0x00, 'A', 0x00, '-', 0x00,       /* wcData_39 */
  'B', 0x00, '3', 0x00, '8', 0x00, 'A', 0x00,       /* wcData_39 */
  '-', 0x00, '7', 0x00, 'C', 0x00, '6', 0x00,       /* wcData_39 */
  'F', 0x00, 'D', 0x00, 'D', 0x00, 'D', 0x00,       /* wcData_39 */
  'D', 0x00, '7', 0x00, 'E', 0x00, '2', 0x00,       /* wcData_39 */
  '6', 0x00, '}', 0x00, 0x00, 0x00,                 /* wcData_39 */
};
#define  WINUSB_IF1_WCID_PROPERTIES_SIZE  (142)
__ALIGN_BEGIN const uint8_t WINUSB_IF1_WCIDProperties [142] __ALIGN_END = {
  ///////////////////////////////////////
  /// WCID property descriptor
  ///////////////////////////////////////
  0x8e, 0x00, 0x00, 0x00,                           /* dwLength */
  0x00, 0x01,                                       /* bcdVersion */
  0x05, 0x00,                                       /* wIndex */
  0x01, 0x00,                                       /* wCount */

  ///////////////////////////////////////
  /// registry propter descriptor
  ///////////////////////////////////////
  0x84, 0x00, 0x00, 0x00,                           /* dwSize */
  0x01, 0x00, 0x00, 0x00,                           /* dwPropertyDataType */
  0x28, 0x00,                                       /* wPropertyNameLength */
  /* DeviceInterfaceGUID */
  'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00,       /* wcName_20 */
  'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00,       /* wcName_20 */
  't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00,       /* wcName_20 */
  'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00,       /* wcName_20 */
  'U', 0x00, 'I', 0x00, 'D', 0x00, 0x00, 0x00,      /* wcName_20 */
  0x4e, 0x00, 0x00, 0x00,                           /* dwPropertyDataLength */
  /* {1D4B2365-4749-48EA-B38A-7C6FDDDD7E26} */
  '{', 0x00, '1', 0x00, 'D', 0x00, '4', 0x00,       /* wcData_39 */
  'B', 0x00, '2', 0x00, '3', 0x00, '6', 0x00,       /* wcData_39 */
  '5', 0x00, '-', 0x00, '4', 0x00, '7', 0x00,       /* wcData_39 */
  '4', 0x00, '9', 0x00, '-', 0x00, '4', 0x00,       /* wcData_39 */
  '8', 0x00, 'E', 0x00, 'A', 0x00, '-', 0x00,       /* wcData_39 */
  'B', 0x00, '3', 0x00, '8', 0x00, 'A', 0x00,       /* wcData_39 */
  '-', 0x00, '7', 0x00, 'C', 0x00, '6', 0x00,       /* wcData_39 */
  'F', 0x00, 'D', 0x00, 'D', 0x00, 'D', 0x00,       /* wcData_39 */
  'D', 0x00, '7', 0x00, 'E', 0x00, '2', 0x00,       /* wcData_39 */
  '6', 0x00, '}', 0x00, 0x00, 0x00,                 /* wcData_39 */
};


const uint8_t* WINUSB_IFx_WCIDProperties[] = {
    WINUSB_IF0_WCIDProperties,
    WINUSB_IF1_WCIDProperties,
};

struct usb_msosv1_descriptor msosv1_desc_ = {
    .string = WCID_StringDescriptor_MSOS,
    .vendor_code = WCID_VENDOR_CODE,
    .string_len = 0x12,
    .compat_id = WINUSB_WCIDDescriptor,
    .compat_id_len = 0x28,
    .comp_id_property = WINUSB_IF0_WCIDProperties,
    .comp_id_property_len = 0x8e,
};

static uint8_t* __winusb_build_descriptor(uint32_t* len, uint8_t in_ep, uint8_t out_ep,
                                          uint8_t* interface_total)
{
    uint8_t* desc = NULL;
    void* mem = NULL;
    uint32_t bytes = 0;

    winusb_data_interface_desc.bInterfaceNumber = *interface_total;
    winusb_in_desc.bEndpointAddress = in_ep;
    winusb_out_desc.bEndpointAddress = out_ep;
    for (uint32_t i = 0; winusb_function[i] != NULL; i++) {
        bytes += winusb_function[i]->bLength;
    }

    if (len) {
        *len = bytes;
    }

    // one extra byte to hold NULL pointer
    bytes += sizeof(void*);

    desc = (uint8_t*)malloc(bytes);

    mem = desc;
    WINUSB_COPY_DESCRIPTORS(mem, winusb_function);

    return desc;
}

uint8_t* winusb_build_descriptor(struct winusb_device_info* winusb_info, uint32_t* desc_len)
{
    uint8_t* desc = NULL;

    desc = __winusb_build_descriptor(desc_len,
                                     winusb_info->winusb_in_ep.ep_addr,
                                     winusb_info->winusb_out_ep.ep_addr,
                                     &winusb_info->interface_nums);

    return desc;
}

struct usb_msosv1_descriptor* winusb_msosv1_build_descriptor(void)
{
    return &msosv1_desc_;
}

void winusb_destroy_descriptor(uint8_t* desc)
{
    if (desc) {
        free(desc);
        desc = NULL;
    }
}
