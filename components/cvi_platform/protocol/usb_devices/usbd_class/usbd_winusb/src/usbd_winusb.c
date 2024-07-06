/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <aos/cli.h>
#include <aos/kernel.h>
#include "usbd_core.h"
#include "usbd_cdc.h"

#include "usbd_comp.h"
#include "usbd_winusb_descriptor.h"
#include "usbd_winusb.h"

#define DOUBLE_WINUSB 0
#define WINUSB_MAX_RW_LEN 64
#define WINUSB_MPS 512

static struct winusb_device_info winusb_info;
static uint8_t *winusb_descriptor = NULL;

uint8_t read_buffer_test[512];
volatile bool ep_tx_flag = false;

static bool winubs_state = 0;

struct usbd_interface intf0;
struct usb_msosv1_descriptor *msosv1_desc = NULL;

// to do
// static void usbd_event_handler(uint8_t event)
// {
//     switch (event) {
//         case USBD_EVENT_RESET:
//             break;
//         case USBD_EVENT_CONNECTED:
//             break;
//         case USBD_EVENT_DISCONNECTED:
//             break;
//         case USBD_EVENT_RESUME:
//             break;
//         case USBD_EVENT_SUSPEND:
//             break;
//         case USBD_EVENT_CONFIGURED:
//             ep_tx_flag = false;
//             /* setup first out ep read transfer */
//             USB_LOG_RAW("USBD_EVENT_CONFIGURED\r\n");
//             usbd_ep_start_read(WINUSB_OUT_EP, read_buffer_test, 64);
// #if DOUBLE_WINUSB == 1
//             usbd_ep_start_read(WINUSB_OUT_EP2, read_buffer_test, 2048);
// #endif
//         default :
//             break;
//     }
// }

void usbd_winusb_out(uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual out len:%u\r\n", nbytes);
    /* setup next out ep read transfer */
    usbd_ep_start_read(ep, read_buffer_test, WINUSB_MPS);
    winubs_state = 1;
}

void usbd_winusb_in( uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual in len:%d\r\n", nbytes);
    if ((nbytes % WINUSB_MAX_RW_LEN) == 0 && nbytes) {
        /* send zlp */
        ep_tx_flag = false;
    }
}

static void winusb_desc_register_cb()
{
	winusb_destroy_descriptor(winusb_descriptor);
}
static void winusb_configure_done_callback(void)
{
    /* setup first out ep read transfer */
    usbd_ep_start_read(winusb_info.winusb_in_ep.ep_addr, read_buffer_test, WINUSB_MPS);
}

void winusb_init(void)
{
    uint32_t desc_len;

    winusb_info.winusb_in_ep.ep_cb = usbd_winusb_in;
    winusb_info.winusb_in_ep.ep_addr = comp_get_available_ep(1);
    winusb_info.winusb_out_ep.ep_cb = usbd_winusb_out;
    winusb_info.winusb_out_ep.ep_addr = comp_get_available_ep(0);
    winusb_info.interface_nums = comp_get_interfaces_num();
    printf("winusb int ep:%#x\n", winusb_info.winusb_in_ep.ep_addr);
    printf("winusb out ep:%#x\n", winusb_info.winusb_out_ep.ep_addr);
    USB_LOG_INFO("interface_nums:%d\n", winusb_info.interface_nums);

    winusb_descriptor = winusb_build_descriptor(&winusb_info, &desc_len);
    comp_register_descriptors(USB_TYPE_WINUSB, winusb_descriptor, desc_len, 1, winusb_desc_register_cb);
    msosv1_desc = winusb_msosv1_build_descriptor();
    usbd_msosv1_desc_register(msosv1_desc);
    comp_register_cfg_done(USB_TYPE_WINUSB, winusb_configure_done_callback);
    usbd_add_interface(&intf0);
    usbd_add_endpoint(&winusb_info.winusb_in_ep);
    usbd_add_endpoint(&winusb_info.winusb_out_ep);
}

void winusb_deinit()
{

}
void print_buffer(unsigned char *buffer,unsigned int len)
{
    int i = 0;
    for(i = 0; i < len; i++)
    {
        if(i%16 == 0)
        {
            printf("\r\n");
        }
        printf("%02x ", buffer[i]);
    }
    printf("\r\n");
}

const uint8_t sendbuffer[WINUSB_MPS] = "sdfagudrmvkasdvjaioejaksdclmz;koesifje";

void winusb_in_test()
{
    while(1)
    {
        if(!ep_tx_flag)
        {
            usbd_ep_start_write(winusb_info.winusb_in_ep.ep_addr, sendbuffer, WINUSB_MPS);
            ep_tx_flag = true;
        }
        aos_msleep(100);
    }
}

ALIOS_CLI_CMD_REGISTER(winusb_in_test, winusb_in_test, usb winusb_in_test input);

void winusb_out_test()
{
    int ret = usbd_ep_start_read(winusb_info.winusb_out_ep.ep_addr, read_buffer_test, WINUSB_MPS);
    if (ret < 0) {
        return;
    }

    while(1)
    {
        if(winubs_state)
        {
            print_buffer(read_buffer_test,64);
            winubs_state = 0;

        }
        aos_msleep(100);
    }
}
ALIOS_CLI_CMD_REGISTER(winusb_out_test, winusb_out_test, usb winusb_out_test  output);