#include <aos/cli.h>
#include <aos/kernel.h>

#include "usbd_core.h"
#include "usbd_hid.h"

#include "usbd_comp.h"
#include "usbd_hid_keyboard_descriptor.h"
#include "usbd_hid_keyboard.h"

#define HID_KEYBOARD_REPORT_DESC_SIZE 63

static const uint8_t hid_keyboard_report_desc[HID_KEYBOARD_REPORT_DESC_SIZE] = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xa1, 0x01, // COLLECTION (Application)
    0x05, 0x07, // USAGE_PAGE (Keyboard)
    0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x01, // LOGICAL_MAXIMUM (1)
    0x75, 0x01, // REPORT_SIZE (1)
    0x95, 0x08, // REPORT_COUNT (8)
    0x81, 0x02, // INPUT (Data,Var,Abs)
    0x95, 0x01, // REPORT_COUNT (1)
    0x75, 0x08, // REPORT_SIZE (8)
    0x81, 0x03, // INPUT (Cnst,Var,Abs)
    0x95, 0x05, // REPORT_COUNT (5)
    0x75, 0x01, // REPORT_SIZE (1)
    0x05, 0x08, // USAGE_PAGE (LEDs)
    0x19, 0x01, // USAGE_MINIMUM (Num Lock)
    0x29, 0x05, // USAGE_MAXIMUM (Kana)
    0x91, 0x02, // OUTPUT (Data,Var,Abs)
    0x95, 0x01, // REPORT_COUNT (1)
    0x75, 0x03, // REPORT_SIZE (3)
    0x91, 0x03, // OUTPUT (Cnst,Var,Abs)
    0x95, 0x06, // REPORT_COUNT (6)
    0x75, 0x08, // REPORT_SIZE (8)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0xFF, // LOGICAL_MAXIMUM (255)
    0x05, 0x07, // USAGE_PAGE (Keyboard)
    0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00, // INPUT (Data,Ary,Abs)
    0xc0        // END_COLLECTION
};

static struct hid_keyboard_device_info hid_keyboard_info;
static uint8_t *hid_keyboard_descriptor = NULL;

static void hid_keyboard_desc_register_cb()
{
	hid_keyboard_destroy_descriptor(hid_keyboard_descriptor);
}


#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1

/*!< hid_keyboard state ! Data can be sent only when state is idle  */
static volatile uint8_t hid_keyboard_state = HID_STATE_IDLE;

void usbd_hid_keyboard_int_callback(uint8_t ep, uint32_t nbytes)
{
    hid_keyboard_state = HID_STATE_IDLE;
}


void hid_keyboard_init()
{

    uint32_t desc_len;

    hid_keyboard_info.hid_keyboard_int_ep.ep_cb = usbd_hid_keyboard_int_callback;
    hid_keyboard_info.hid_keyboard_int_ep.ep_addr = comp_get_available_ep(1);
    hid_keyboard_info.interface_nums = comp_get_interfaces_num();
    USB_LOG_INFO("hid_keyboard int ep:%#x\n", hid_keyboard_info.hid_keyboard_int_ep.ep_addr);
    USB_LOG_INFO("interface_nums:%d\n", hid_keyboard_info.interface_nums);

    hid_keyboard_descriptor = hid_keyboard_build_descriptor(&hid_keyboard_info, &desc_len);
    comp_register_descriptors(USBD_TYPE_HID, hid_keyboard_descriptor, desc_len, 1, hid_keyboard_desc_register_cb);

    usbd_add_interface(usbd_hid_init_intf(&hid_keyboard_info.hid_keyboard_intf0, hid_keyboard_report_desc, HID_KEYBOARD_REPORT_DESC_SIZE));
    usbd_add_endpoint(&hid_keyboard_info.hid_keyboard_int_ep);
}

void hid_keyboard_deinit()
{

}

void hid_keyboard_test()
{
    uint8_t sendbuffer[8] = { 0x00, 0x00, 0x12, 0x0e, 0x00, 0x00, 0x00, 0x00 }; //send ok

    int ret = usbd_ep_start_write(hid_keyboard_info.hid_keyboard_int_ep.ep_addr, sendbuffer, 8);
    if (ret < 0) {
        return;
    }
    hid_keyboard_state = HID_STATE_BUSY;
    while (hid_keyboard_state == HID_STATE_BUSY) {
        aos_msleep(100);
    }
    sendbuffer[2] = 0x00;
    sendbuffer[3] = 0x00;
    usbd_ep_start_write(hid_keyboard_info.hid_keyboard_int_ep.ep_addr, sendbuffer, 8);
}

ALIOS_CLI_CMD_REGISTER(hid_keyboard_test, hid_keyboard_test, usb hid_keyboard keyborad input);
