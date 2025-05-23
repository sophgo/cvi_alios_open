#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <aos/kernel.h>
#include "io.h"

#include "usbd_descriptor.h"
#include "usbd_uvc.h"
#include "usbd_uac.h"
#include "usbd_cdc_uart.h"
#include "usbd_cdc_rndis.h"
#include "usbd_hid_keyboard.h"
#include "usbd_winusb.h"

#define MAX_COMP_DEVICE_NUMS 4
#define MAX_COMP_STRING_NUMS 8
#define MAX_COMP_EP_NUMS 6

#define CVITEK_VENDOR_ID        0x3346    /* cvitek vendor id */
#define CVITEK_PRODUCT_ID       0x0001    /* Webcam A/V gadget */
#define CVITEK_DEVICE_BCD       0x0001    /* 0.01 */
#define USBD_LANGID_STRING 		1033

#define COMP_COPY_DESCRIPTOR(mem, dst, desc) \
    do { \
        memcpy(mem, desc, (desc)->bLength); \
        *(dst)++ = mem; \
        mem += (desc)->bLength; \
    } while (0);

#define COMP_COPY_DESCRIPTORS(mem, dst, src) \
    do { \
        const struct usb_descriptor_header * const *__src; \
        for (__src = src; *__src; ++__src) { \
            memcpy(mem, *__src, (*__src)->bLength); \
            *dst++ = mem; \
            mem += (*__src)->bLength; \
        } \
    } while (0)

static struct usb_device_descriptor comp_device_descriptor = {
    .bLength            = USB_DT_DEVICE_SIZE,
    .bDescriptorType    = USB_DT_DEVICE,
    .bcdUSB             = USB_2_0,
    .bDeviceClass       = USB_CLASS_MISC,
    .bDeviceSubClass    = 0x02,
    .bDeviceProtocol    = 0x01,
    .bMaxPacketSize0    = 0x40,
    .idVendor           = cpu_to_le16(CVITEK_VENDOR_ID),
    .idProduct          = cpu_to_le16(CVITEK_PRODUCT_ID),
    .bcdDevice          = cpu_to_le16(CVITEK_DEVICE_BCD),
    .iManufacturer      = USB_STRING_MFC_INDEX,
    .iProduct           = USB_STRING_PRODUCT_INDEX,
    .iSerialNumber      = USB_STRING_SERIAL_INDEX,
    .bNumConfigurations = 1,
};

static struct usb_config_descriptor comp_config_descriptor = {
    .bLength                = USB_DT_CONFIG_SIZE,
    .bDescriptorType        = USB_DT_CONFIG,
    .wTotalLength           = 0, /* dynamic */
    .bNumInterfaces         = 0, /* dynamic */
    .bConfigurationValue    = 1,
    .iConfiguration         = 0,
    .bmAttributes           = USB_CONFIG_ATT_ONE,
    .bMaxPower              = 50,
};

static struct usb_qualifier_descriptor comp_qual_descriptor = {
    .bLength             = USB_DT_QUALIFIER_SIZE,
    .bDescriptorType     = USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    .bcdUSB              = cpu_to_le16(USB_2_0),
    .bDeviceClass        = 0xEF,
    .bDeviceSubClass     = 0x02,
    .bDeviceProtocol     = 1,
    .bMaxPacketSize0     = 0x40,
    .bNumConfigurations  = 1,
    .bRESERVED           = 0,
};
#if (CONFIG_USBD_UVC_OTHER == 1)
static struct usb_config_descriptor comp_other_speed_config_descriptor = {
    .bLength                = USB_DT_CONFIG_SIZE,
    .bDescriptorType        = USB_DT_OTHER_SPEED_CONFIG,
    .wTotalLength           = 0, /* dynamic */
    .bNumInterfaces         = 0, /* dynamic */
    .bConfigurationValue    = 1,
    .iConfiguration         = 0,
    .bmAttributes           = USB_CONFIG_ATT_ONE,
    .bMaxPower              = 0xFA,
};
#endif
DECLARE_UVC_STRING_DESCRIPTOR(1);
static const struct UVC_STRING_DESCRIPTOR(1) comp_string_descriptor_zero = {
    .bLength            = UVC_STRING_DESCRIPTOR_SIZE(1),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_STRING,
    .wData[0]           = cpu_to_le16(USBD_LANGID_STRING),
};

DECLARE_UVC_STRING_DESCRIPTOR(6);
static const struct UVC_STRING_DESCRIPTOR(6) comp_string_descriptor_manufacturer = {
    .bLength            = UVC_STRING_DESCRIPTOR_SIZE(6),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_STRING,
    .wData              = {
        cpu_to_le16('C'),
        cpu_to_le16('V'),
        cpu_to_le16('I'),
        cpu_to_le16('T'),
        cpu_to_le16('E'),
        cpu_to_le16('K'),
    },
};

DECLARE_UVC_STRING_DESCRIPTOR(10);
static const struct UVC_STRING_DESCRIPTOR(10) comp_string_descriptor_serial = {
    .bLength            = UVC_STRING_DESCRIPTOR_SIZE(10),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_STRING,
    .wData              = {
        cpu_to_le16('2'),
        cpu_to_le16('0'),
        cpu_to_le16('2'),
        cpu_to_le16('4'),
        cpu_to_le16('0'),
        cpu_to_le16('1'),
        cpu_to_le16('1'),
        cpu_to_le16('9'),
        cpu_to_le16('0'),
        cpu_to_le16('0'),
    },
};


static const struct UVC_STRING_DESCRIPTOR(10) dev_string_descriptor_product = {
    .bLength            = UVC_STRING_DESCRIPTOR_SIZE(10),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_STRING,
    .wData              = {
        cpu_to_le16('C'),
        cpu_to_le16('V'),
        cpu_to_le16('I'),
        cpu_to_le16('T'),
        cpu_to_le16('E'),
        cpu_to_le16('K'),
        cpu_to_le16('-'),
        cpu_to_le16('D'),
        cpu_to_le16('E'),
        cpu_to_le16('V'),
    },
};


/* UVC Camera max name length is 32  */
DECLARE_UVC_STRING_DESCRIPTOR(32);

static const struct UVC_STRING_DESCRIPTOR(32) uvc_string_descriptor_video1_name = {
    .bLength            = UVC_STRING_DESCRIPTOR_SIZE(14), // name length 14
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_STRING,
    .wData              = {
        cpu_to_le16('C'),
        cpu_to_le16('V'),
        cpu_to_le16('I'),
        cpu_to_le16('T'),
        cpu_to_le16('E'),
        cpu_to_le16('K'),
        cpu_to_le16(' '),
        cpu_to_le16('C'),
        cpu_to_le16('A'),
        cpu_to_le16('M'),
        cpu_to_le16('E'),
        cpu_to_le16('R'),
        cpu_to_le16('A'),
        cpu_to_le16('1'),
    },
};

static const struct UVC_STRING_DESCRIPTOR(32) uvc_string_descriptor_video2_name = {
    .bLength            = UVC_STRING_DESCRIPTOR_SIZE(14),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_STRING,
    .wData              = {
        cpu_to_le16('C'),
        cpu_to_le16('V'),
        cpu_to_le16('I'),
        cpu_to_le16('T'),
        cpu_to_le16('E'),
        cpu_to_le16('K'),
        cpu_to_le16(' '),
        cpu_to_le16('C'),
        cpu_to_le16('A'),
        cpu_to_le16('M'),
        cpu_to_le16('E'),
        cpu_to_le16('R'),
        cpu_to_le16('A'),
        cpu_to_le16('2'),
    },
};

static const struct UVC_STRING_DESCRIPTOR(32) uvc_string_descriptor_video3_name = {
    .bLength            = UVC_STRING_DESCRIPTOR_SIZE(14),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_STRING,
    .wData              = {
        cpu_to_le16('C'),
        cpu_to_le16('V'),
        cpu_to_le16('I'),
        cpu_to_le16('T'),
        cpu_to_le16('E'),
        cpu_to_le16('K'),
        cpu_to_le16(' '),
        cpu_to_le16('C'),
        cpu_to_le16('A'),
        cpu_to_le16('M'),
        cpu_to_le16('E'),
        cpu_to_le16('R'),
        cpu_to_le16('A'),
        cpu_to_le16('3'),
    },
};

DECLARE_UVC_STRING_DESCRIPTOR(12);
static const struct UVC_STRING_DESCRIPTOR(12) uac_string_descriptor_audio = {
    .bLength            = UVC_STRING_DESCRIPTOR_SIZE(12),
    .bDescriptorType    = USB_DESCRIPTOR_TYPE_STRING,
    .wData              = {
        cpu_to_le16('C'),
        cpu_to_le16('V'),
        cpu_to_le16('I'),
        cpu_to_le16('T'),
        cpu_to_le16('E'),
        cpu_to_le16('K'),
        cpu_to_le16(' '),
        cpu_to_le16('A'),
        cpu_to_le16('U'),
        cpu_to_le16('D'),
        cpu_to_le16('I'),
        cpu_to_le16('O'),
    },
};


static const struct usb_descriptor_header * const comp_string_descriptors[] = {
    (struct usb_descriptor_header *) &comp_string_descriptor_zero,
    (struct usb_descriptor_header *) &comp_string_descriptor_manufacturer,
    (struct usb_descriptor_header *) &dev_string_descriptor_product,
    (struct usb_descriptor_header *) &comp_string_descriptor_serial,
    (struct usb_descriptor_header *) &comp_string_descriptor_zero,
    (struct usb_descriptor_header *) &uac_string_descriptor_audio,
    (struct usb_descriptor_header *) &uvc_string_descriptor_video1_name,
    (struct usb_descriptor_header *) &uvc_string_descriptor_video2_name,
    (struct usb_descriptor_header *) &uvc_string_descriptor_video3_name,
    NULL,
};

struct comp_desc_list_t {
    enum USBD_TYPE type;
	uint8_t *desc;
	uint32_t desc_len;
	uint8_t interfaces_num;
    void (*cb)(void);
};

struct comp_cfg_done_cb_list_t {
    void (*cb)(void);
};

struct ep_pool_t {
    uint8_t pool[MAX_COMP_EP_NUMS];
	volatile uint8_t cur_idx;;
};

static struct comp_desc_list_t comp_desc_list[MAX_COMP_DEVICE_NUMS];
static volatile uint8_t comp_desc_list_nums;

static struct comp_cfg_done_cb_list_t comp_cfg_done_cb_list[MAX_COMP_DEVICE_NUMS];
static uint8_t comp_cfg_done_cb_list_nums;

static struct ep_pool_t ep_pool = {
    .pool = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
    .cur_idx = 0,
};

static uint8_t *comp_descriptor = NULL;
static volatile uint32_t comp_descriptor_len = 0;
static volatile uint8_t comp_interfaces_num = 0;
#if (CONFIG_USBD_UVC_OTHER == 1)
static struct comp_desc_list_t comp_desc_list_other[MAX_COMP_DEVICE_NUMS];
static volatile uint8_t comp_desc_list_nums_other;
static volatile uint32_t comp_descriptor_len_other = 0;
static volatile uint8_t comp_interfaces_num_other = 0;
#endif

void usbd_configure_done_callback(void)
{
    for (uint8_t i = 0; i < comp_cfg_done_cb_list_nums; i++) {
        if (comp_cfg_done_cb_list[i].cb) {
            comp_cfg_done_cb_list[i].cb();
        }
    }
}

int32_t comp_register_descriptors(enum USBD_TYPE type, uint8_t *desc, uint32_t desc_len, uint8_t interfaces_num, void (*cb)(void))
{
	if (comp_desc_list_nums >= MAX_COMP_DEVICE_NUMS) {
		return -1;
	}

    USB_LOG_INFO("comp_desc_list_num[%d] tpye:%d\n",comp_desc_list_nums,type);

    comp_desc_list[comp_desc_list_nums].type = type;
	comp_desc_list[comp_desc_list_nums].desc = desc;
	comp_desc_list[comp_desc_list_nums].desc_len = desc_len;
	comp_desc_list[comp_desc_list_nums].interfaces_num = interfaces_num;
    comp_desc_list[comp_desc_list_nums].cb = cb;
	comp_desc_list_nums++;

    comp_interfaces_num += interfaces_num;
    comp_descriptor_len += desc_len;

	return 0;
}

#if (CONFIG_USBD_UVC_OTHER == 1)
int32_t comp_register_descriptors_other(enum USBD_TYPE type, uint8_t *desc, uint32_t desc_len, uint8_t interfaces_num, void (*cb)(void))
{
	if (comp_desc_list_nums_other >= MAX_COMP_DEVICE_NUMS) {
		return -1;
	}

    USB_LOG_INFO("comp_desc_list_num[%d] tpye:%d\n",comp_desc_list_nums_other,type);

    comp_desc_list_other[comp_desc_list_nums_other].type = type;
	comp_desc_list_other[comp_desc_list_nums_other].desc = desc;
	comp_desc_list_other[comp_desc_list_nums_other].desc_len = desc_len;
	comp_desc_list_other[comp_desc_list_nums_other].interfaces_num = interfaces_num;
    comp_desc_list_other[comp_desc_list_nums_other].cb = cb;
	comp_desc_list_nums_other++;

    comp_interfaces_num_other += interfaces_num;
    comp_descriptor_len_other += desc_len;

	return 0;
}
#endif

int32_t comp_register_cfg_done(enum USBD_TYPE type, void (*cb)(void))
{
    (void)type;
    if (comp_cfg_done_cb_list_nums >= MAX_COMP_DEVICE_NUMS) {
		return -1;
	}

    comp_cfg_done_cb_list[comp_cfg_done_cb_list_nums].cb = cb;
    comp_cfg_done_cb_list_nums++;

    return 0;
}

uint8_t comp_get_available_ep(uint8_t b_in)
{
    if (ep_pool.cur_idx >= MAX_COMP_EP_NUMS) {
        USB_LOG_ERR("usb ep overflow, %d\n", ep_pool.cur_idx);
        return 0;
    }

    uint8_t ep = ep_pool.pool[ep_pool.cur_idx];
    if (b_in) {
        ep |= (1 << 7);
    }

    ep_pool.cur_idx++;

    return ep;
}

uint8_t comp_get_interfaces_num(void)
{
    return comp_interfaces_num;
}
#if (CONFIG_USBD_UVC_OTHER == 1)
uint8_t comp_get_interfaces_num_other(void)
{
    return comp_interfaces_num_other;
}
#endif
uint8_t usbd_comp_get_speed(void)
{
#if(CONFIG_USB_HS_FS_ADAPT == 1)
    return usbd_get_port_speed(0);
#else
    return USB_SPEED_HIGH;
#endif
}

static uint8_t *comp_build_descriptors(void)
{
	uint8_t *comp_desc = NULL;
    uint32_t comp_desc_len = 0;
#if (CONFIG_USBD_UVC_OTHER == 1)
    uint32_t comp_desc_len_other = 0;
#endif
	uint32_t bytes = 0;
	uint32_t n_desc = 15;
	void *mem = NULL;
	const struct usb_descriptor_header * const *src;
    struct usb_descriptor_header **dst;
    struct usb_config_descriptor *comp_config_header;
#if (CONFIG_USBD_UVC_OTHER == 1)
    struct usb_config_descriptor *comp_other_speed_config_header;
#endif
    comp_config_descriptor.bNumInterfaces = comp_interfaces_num;

    comp_desc_len = comp_descriptor_len;
	comp_desc_len += comp_config_descriptor.bLength;

	bytes += comp_desc_len;
	bytes += comp_device_descriptor.bLength;

    for (src = comp_string_descriptors; *src; ++src) {
        bytes += (*src)->bLength;
    }

#if (CONFIG_USBD_UVC_OTHER == 1)
    for (src = comp_string_descriptors; *src; ++src) {
        bytes += (*src)->bLength;
    }
#endif

    if (usbd_comp_get_speed() == USB_SPEED_HIGH) {
        bytes += comp_qual_descriptor.bLength;
    }

#if (CONFIG_USBD_UVC_OTHER == 1)
    comp_desc_len_other = comp_descriptor_len_other;
    comp_other_speed_config_descriptor.bNumInterfaces = comp_interfaces_num_other;
    comp_desc_len_other += comp_other_speed_config_descriptor.bLength;
    bytes += comp_desc_len_other;
#endif
	comp_desc = malloc(bytes + 1 + n_desc * sizeof(*src));
	mem = comp_desc;
	dst = mem + bytes + 1;

	// Copy the descriptors
    COMP_COPY_DESCRIPTOR(mem, dst, &comp_device_descriptor);

	comp_config_header = mem;
	COMP_COPY_DESCRIPTOR(mem, dst, &comp_config_descriptor);
    comp_config_header->wTotalLength = cpu_to_le16(comp_desc_len);

	// Copy iad
    for(uint8_t i = 0; i < comp_desc_list_nums; i ++) {
        memcpy(mem, comp_desc_list[i].desc, comp_desc_list[i].desc_len);
        mem += comp_desc_list[i].desc_len;
    }

    // Copy string
    COMP_COPY_DESCRIPTORS(mem, dst, comp_string_descriptors);

    if (usbd_comp_get_speed() == USB_SPEED_HIGH) {
        COMP_COPY_DESCRIPTOR(mem, dst, &comp_qual_descriptor);
    }
#if (CONFIG_USBD_UVC_OTHER == 1)
    comp_other_speed_config_header = mem;
    COMP_COPY_DESCRIPTOR(mem, dst, &comp_other_speed_config_descriptor);
    comp_other_speed_config_header->wTotalLength = cpu_to_le16(comp_desc_len_other);//这里的长度已经算上了设备描述符

    for(uint8_t i = 0; i < comp_desc_list_nums_other; i ++) {
        memcpy(mem, comp_desc_list_other[i].desc, comp_desc_list_other[i].desc_len);
        mem += comp_desc_list_other[i].desc_len;
    }
#endif
    ((uint8_t *)mem)[0] = 0x00;

    for (uint8_t i = 0; i < comp_desc_list_nums; i++) {
        if (comp_desc_list[i].cb) {
            USB_LOG_INFO("comp_desc_list_num[%d/%d] tpye:%d\n",i,comp_desc_list_nums,comp_desc_list[i].type);
            comp_desc_list[i].cb();
        }
	}
#if (CONFIG_USBD_UVC_OTHER == 1)
    for (uint8_t i = 0; i < comp_desc_list_nums_other; i++) {
        if (comp_desc_list_other[i].cb) {
            USB_LOG_INFO("comp_desc_list_nums_other[%d/%d] tpye:%d\n",i,comp_desc_list_nums_other,comp_desc_list_other[i].type);
            comp_desc_list_other[i].cb();
        }
	}
#endif

    return comp_desc;
}

static void comp_destroy_descriptors(void)
{
    if (comp_descriptor) {
        free(comp_descriptor);
        comp_descriptor = NULL;
    }
    comp_descriptor_len = 0;
    comp_interfaces_num = 0;
    comp_desc_list_nums = 0;
#if (CONFIG_USBD_UVC_OTHER == 1)
    comp_descriptor_len_other = 0;
    comp_interfaces_num_other = 0;
    comp_desc_list_nums_other = 0;
#endif
    ep_pool.cur_idx = 0;
}

void usbd_comp_desc_register()
{
    if(comp_descriptor){
        comp_destroy_descriptors();
    }

#if CONFIG_USBD_CDC_RNDIS
    // Rndis must be the first IDA on windows.
    cdc_rndis_desc_register();
#endif
#if CONFIG_USBD_UVC
    uvc_desc_register();
#endif
#if CONFIG_USBD_UAC
    uac_desc_register();
#endif
#if CONFIG_USBD_CDC_UART
    cdc_uart_desc_register();
#endif
#if CONFIG_USBD_HID_KEYBOARD
    hid_keyboard_desc_register();
#endif
#if CONFIG_USBD_WINUSB
    // Other composite interfaces must be disabled to use winusb
    winusb_desc_register();
#endif
#if (CONFIG_USBD_UVC_OTHER == 1)
    uvc_desc_other_register();
#endif
    comp_descriptor = comp_build_descriptors();
    usbd_desc_register(comp_descriptor);
}

uint32_t usbd_comp_init()
{
#if(CONFIG_USB_HS_FS_ADAPT == 1)
    usb_dc_register_enum_cb(usbd_comp_desc_register);
#else
    usbd_comp_desc_register();
#endif
    usbd_initialize();

#if CONFIG_USBD_CDC_RNDIS
    // Rndis must be the first IDA on windows.
    cdc_rndis_init();
#endif
#if CONFIG_USBD_UVC
    uvc_init();
#endif
#if CONFIG_USBD_UAC
    uac_init();
#endif
#if CONFIG_USBD_CDC_UART
    cdc_uart_init();
#endif
#if CONFIG_USBD_HID_KEYBOARD
    hid_keyboard_init();
#endif
#if CONFIG_USBD_WINUSB
    // Other composite interfaces must be disabled to use winusb
    winusb_init();
#endif

    return 0;
}

void usbd_comp_deinit()
{
#if CONFIG_USBD_CDC_RNDIS
    cdc_rndis_deinit();
#endif
#if CONFIG_USBD_CDC_UART
    cdc_uart_deinit();
#endif
#if CONFIG_USBD_UAC
    uac_deinit();
#endif
#if CONFIG_USBD_UVC
    uvc_deinit();
#endif

    usbd_deinitialize();
    comp_destroy_descriptors();
}
