#ifndef __USBD_UVC_H__
#define __USBD_UVC_H__
#include <stdio.h>

#include "usbd_core.h"
#include "usbd_video.h"
#include <aos/kernel.h>
#include <k_atomic.h>

#define USBD_UVC_NUM	CONFIG_USBD_UVC_NUM

#if CONFIG_USB_BULK_UVC
    #define EP_INTERVAL 0x00
#else
    #define EP_INTERVAL 0x01
#endif

#ifndef USBD_UVC_MAX_NUM
#define USBD_UVC_MAX_NUM 3
#endif

// Frame interval in 100 ns units.
#define FRAME_INTERVAL_FPS(N)   (1000 * 1000 * 10 / N)

#if CONFIG_USB_BULK_UVC
    #define FRM_BUFFER_LEN 2
    #define FRM_BUFFER_GET_IDX(idx) (idx&(FRM_BUFFER_LEN-1))
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
    uint32_t tx_cnt;
    uint32_t max_payload_size;
    uint32_t cam_fps;
    uint32_t interval;
    uint32_t default_frame_size;
#if CONFIG_USB_BULK_UVC
    volatile uint32_t rx_frm_idx;
    volatile uint32_t tx_frm_idx;
    volatile uint32_t frm_sz[FRM_BUFFER_LEN];
    aos_workqueue_t uvc_workqueue;
    aos_work_t uvc_frame_submmit;
#endif
	// interval max_frame_size max_payload_size
};

void uvc_desc_register();
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

void uvc_switch(int argc, char** argv);
#endif
