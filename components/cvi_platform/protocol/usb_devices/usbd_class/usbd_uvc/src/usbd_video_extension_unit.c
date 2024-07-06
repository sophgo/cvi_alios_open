#include "aos/kernel.h"

#include "usbd_core.h"
#include "usbd_video.h"

static int usbd_vc_xu_reboot_request_handler(uint8_t bRequest, uint8_t **data, uint32_t *len)
{
	switch (bRequest) {
		case VIDEO_REQUEST_SET_CUR: {
			if (strncmp((char *)(*data), "reboot", 6) == 0) {
				USB_LOG_WRN("receive reboot\n");
				aos_reboot(); /* if need, create a thread to reboot */
			}
		} break;
		case VIDEO_REQUEST_GET_LEN: {
			(*data)[0] = 0x06;
			(*data)[1] = 0x00;
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_INFO: {
			(*data)[0] = 0x02; // only support set
			*len = 1;
		} break;
		case VIDEO_REQUEST_GET_MIN: {
			memset(*data, 0, 6);
			*len = 6;
		} break;
		case VIDEO_REQUEST_GET_MAX: {
			memset(*data, 0xff, 6);
			*len = 6;
		} break;
		case VIDEO_REQUEST_GET_RES: {
			memset(*data, 0, 6);
			(*data)[0] = 0x01;
			*len = 6;
		} break;
		case VIDEO_REQUEST_GET_DEF: {
			memset(*data, 0, 6);
			*len = 6;
		} break;
		default: {
			USB_LOG_WRN("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
			return -1;
		}
	}
	return 0;
}

int usbd_vc_extension_unit_request_handler(struct usbd_video_cfg_priv *usbd_video_cfg,
                                           struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
	uint8_t control_selector = (uint8_t)(setup->wValue >> 8);
	uint8_t bRequest = setup->bRequest;

	switch (control_selector) {
		case VIDEO_XU_REBOOT_CONTROL:
			return usbd_vc_xu_reboot_request_handler(bRequest, data, len);
		default:
			usbd_video_cfg->error_code = 0x06;
			USB_LOG_WRN("Unhandled Video Class extension unit control selector 0x%02x\r\n", control_selector);
			return -1;
	}
	return 0;
}
