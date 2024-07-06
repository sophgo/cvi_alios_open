#ifndef __USBD_UVC_H__
#define __USBD_UVC_H__
#include <stdio.h>

#include "usbd_core.h"
#include "usbd_video.h"

#define USBD_UVC_NUM	CONFIG_USBD_UVC_NUM

#if ((CONFIG_USB_BULK_UVC) && (USBD_UVC_NUM > 1))
// TODO
#error "uvc num has been 1 in bulk mode"
#endif

struct video_source {
    int venc_channel;
    int vpss_group;
    int vpss_channel;
};

struct uvc_device_info {
	struct usbd_endpoint video_in_ep;
	struct usbd_interface vc_intf;
	struct usbd_interface vs_intf;
    uint8_t ep;        //assigned in-endpoint
    uint8_t formats;   //number of supported format
    uint8_t interface_nums;
    uint8_t format_index; // uvc_format_info index.
	volatile bool streaming_on;
	volatile bool tx_busy;
    volatile bool xfer_flag;
    volatile uint32_t xfer_len;
    volatile uint32_t xfer_offset;
    volatile bool update_flag;
    struct video_source video;
    struct uvc_format_info_st *format_info;
	uint8_t *packet_buffer_uvc;
    bool header_flip;


	// interval max_frame_size max_payload_size
};

int uvc_init(void);
int uvc_deinit(void);

struct uvc_device_info *uvc_container_of_device_id(uint8_t device_id);

/* usbd video contorl unit/terminal request handler */
int usbd_vc_input_terminal_request_handler(struct usbd_video_cfg_priv *usbd_video_cfg,
                                           uint16_t terminal_type,
                                           struct usb_setup_packet *setup,
                                           uint8_t **data, uint32_t *len);

int usbd_vc_processing_unit_request_handler(struct usbd_video_cfg_priv *usbd_video_cfg,
                                            struct usb_setup_packet *setup,
                                            uint8_t **data, uint32_t *len);

int usbd_vc_extension_unit_request_handler(struct usbd_video_cfg_priv *usbd_video_cfg,
                                           struct usb_setup_packet *setup,
                                           uint8_t **data, uint32_t *len);

#endif
