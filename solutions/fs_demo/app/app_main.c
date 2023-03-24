/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include "board.h"
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>

#ifdef CONFIG_USB_MSC
#include <usbh_core.h>
#include <usbh_cdc_acm.h>
#include <usbh_hid.h>
#include <usbh_msc.h>
#include <usbh_video.h>
#include <drv_usbh_class.h>
#endif

#define TAG "app"

extern void cxx_system_init(void);
extern void board_yoc_init(void);

int main(int argc, char *argv[])
{
    cxx_system_init();
    board_yoc_init();

#ifdef CONFIG_USB_MSC
    rvm_usb_msc_drv_register(0);
    USBH_REGISTER_MSC_CLASS();
    USBH_REGISTER_CDCACM_CLASS();
    USBH_REGISTER_HID_MOUSE_CLASS();
    usbh_initialize();
#endif

    LOGI(TAG, "app start........\n");
    while (1) {
        aos_msleep(3000);
    };
}