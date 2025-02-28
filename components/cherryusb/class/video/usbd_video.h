/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef USBD_VIDEO_H
#define USBD_VIDEO_H

#include "usb_video.h"

#ifdef __cplusplus
extern "C" {
#endif

// size of array `uvc_fs_control_cls` - 1,  in usbd_uvc_descriptor.c
#define VIDEO_ENTITY_NUM 4
#define VIDEO_INTF_NUM 6

struct video_entity_info {
    uint8_t bDescriptorSubtype;
    uint8_t bEntityId;
    uint16_t wTerminalType;
};

struct usbd_video_cfg_priv {
    struct video_probe_and_commit_controls probe;
    struct video_probe_and_commit_controls commit;
    uint8_t power_mode;
    uint8_t error_code;
    struct video_entity_info info[VIDEO_ENTITY_NUM];
    uint8_t alt_setting[VIDEO_INTF_NUM];
};

typedef struct uvc_event_callbacks {
    void (*uvc_event_stream_on)(uint8_t intf, int is_on);
    void (*uvc_event_setup_class_control)(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len);
    void (*uvc_event_setup_class_streaming)(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len);
    void (*uvc_event_data_out)(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len);
    void (*uvc_event_tx_complete)(uint8_t ep, uint32_t nbytesn);
} uvc_event_callbacks_t;

typedef struct uvc_video_control_unit_terminal_callbacks {
    int (*uvc_vc_input_terminal_handler)(struct usbd_video_cfg_priv *usbd_video_cfg, uint16_t terminal_type,
                                         struct usb_setup_packet *setup, uint8_t **data, uint32_t *len);
    int (*uvc_vc_processing_unit_handler)(struct usbd_video_cfg_priv *usbd_video_cfg,
                                          struct usb_setup_packet *setup, uint8_t **data, uint32_t *len);
    int (*uvc_vc_extension_unit_handler)(struct usbd_video_cfg_priv *usbd_video_cfg,
                                         struct usb_setup_packet *setup, uint8_t **data, uint32_t *len);
} uvc_video_control_callbacks_t;

/* Init video interface driver */
struct usbd_interface *usbd_video_init_intf(struct usbd_interface *intf,
                                            uint32_t dwFrameInterval,
                                            uint32_t dwMaxVideoFrameSize,
                                            uint32_t dwMaxPayloadTransferSize);
struct usbd_interface *usbd_video_control_init_intf(struct usbd_interface *intf,
                                            uint32_t dwFrameInterval,
                                            uint32_t dwMaxVideoFrameSize,
                                            uint32_t dwMaxPayloadTransferSize);
struct usbd_interface *usbd_video_stream_init_intf(struct usbd_interface *intf,
                                            uint32_t dwFrameInterval,
                                            uint32_t dwMaxVideoFrameSize,
                                            uint32_t dwMaxPayloadTransferSize);

uint32_t usbd_video_payload_fill(uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *out_len);

struct usbd_endpoint *usbd_video_init_ep(struct usbd_endpoint *ep,
                        uint8_t ep_addr,
                        usbd_endpoint_callback ep_cb);

void usbd_video_register_uvc_callbacks(uvc_event_callbacks_t *cb);
void usbd_video_register_video_control_callbacks(uvc_video_control_callbacks_t *cb);

void usbd_video_commit_set_cur(uint8_t intf, struct video_probe_and_commit_controls *commit);

#ifdef __cplusplus
}
#endif

#endif /* USBD_VIDEO_H */
