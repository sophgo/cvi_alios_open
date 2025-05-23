
#ifndef __USBD_COMP_H__
#define __USBD_COMP_H__

#include <stdint.h>

#ifndef CONFIG_USBD_UVC_OTHER
#define CONFIG_USBD_UVC_OTHER 0
#endif

enum USBD_TYPE {
    USBD_TYPE_UVC,
    USBD_TYPE_UAC,
    USBD_TYPE_CDC_UART,
    USBD_TYPE_CDC_RNDIS,
    USBD_TYPE_HID,
    USB_TYPE_WINUSB,
};

uint8_t usbd_comp_get_speed(void);
int32_t comp_register_descriptors(enum USBD_TYPE type, uint8_t* desc, uint32_t desc_len,
                                  uint8_t interfaces_num, void (*cb)(void));
int32_t comp_register_cfg_done(enum USBD_TYPE type, void (*cb)(void));
uint8_t comp_get_available_ep(uint8_t b_in);
uint8_t comp_get_interfaces_num(void);
int32_t comp_register_descriptors_other(enum USBD_TYPE type, uint8_t* desc, uint32_t desc_len,
                                  uint8_t interfaces_num, void (*cb)(void));
uint8_t comp_get_interfaces_num_other(void);
uint32_t usbd_comp_init();
void usbd_comp_deinit();

#endif