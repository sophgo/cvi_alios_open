#include "cvi_vpss.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "drv/pwm.h"

#include "usbd_core.h"
#include "usbd_video.h"
#include "usbd_uvc.h"

// static int usbd_vc_pu_backlight_compensation_request_handler(uint8_t device_id, uint8_t bRequest, uint8_t **data,
//                                                              uint32_t *len)
// {
//     switch (bRequest) {
//         case VIDEO_REQUEST_GET_CUR: {
//             uint16_t wBacklightCompensation = 0x0004;
//             memcpy(*data, (uint8_t *)&wBacklightCompensation, 2);
//             *len = 2;
//         } break;
//         case VIDEO_REQUEST_GET_MIN: {
//             uint16_t wBacklightCompensation = 0;
//             memcpy(*data, (uint8_t *)&wBacklightCompensation, 2);
//             *len = 2;
//         } break;
//         case VIDEO_REQUEST_GET_MAX: {
//             uint16_t wBacklightCompensation = 8;
//             memcpy(*data, (uint8_t *)&wBacklightCompensation, 2);
//             *len = 2;
//         } break;
//         case VIDEO_REQUEST_GET_RES: {
//             uint16_t wBacklightCompensation = 1;
//             memcpy(*data, (uint8_t *)&wBacklightCompensation, 2);
//             *len = 2;
//         } break;
//         case VIDEO_REQUEST_GET_INFO: {
//             (*data)[0] = 0x03; //struct video_camera_capabilities
//             *len = 1;
//             break;
//         }
//         case VIDEO_REQUEST_GET_DEF: {
//             uint16_t wBacklightCompensation = 4;
//             memcpy(*data, (uint8_t *)&wBacklightCompensation, 2);
//             *len = 2;
//         } break;
//         default:
//             USB_LOG_WRN("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
//             return -1;
//     }
//     return 0;
// }


static int usbd_vc_pu_brightness_request_handler(uint8_t device_id, uint8_t bRequest, uint8_t **data, uint32_t *len)
{
	int ret;
	struct uvc_device_info *uvc = uvc_container_of_device_id(device_id);
	int group_id = uvc->video.vpss_group;
	uint16_t brightness;

	switch (bRequest) {
		case VIDEO_REQUEST_SET_CUR: {
			brightness = (uint16_t)(*data)[1] << 8 | (uint16_t)(*data)[0];
			if(brightness > 100 || brightness < 0)
				return -1;
			ret = CVI_VPSS_SetGrpProcAmp(group_id, PROC_AMP_BRIGHTNESS, brightness);
			if (ret != 0) {
				USB_LOG_WRN("set vpss grp amp(%d) failed with %#x", PROC_AMP_BRIGHTNESS, ret);
				return -1;
			}
			USB_LOG_DBG("Video set brightness:%d\r\n", brightness);
		} break;
		case VIDEO_REQUEST_GET_CUR: {
			int32_t value = 0;
			ret = CVI_VPSS_GetGrpProcAmp(group_id, PROC_AMP_BRIGHTNESS, &value);
			if (ret != 0) {
				USB_LOG_WRN("get vpss grp amp(%d) failed with %#x", PROC_AMP_BRIGHTNESS, ret);
				return -1;
			}
			brightness = value;
			memcpy(*data, (uint8_t *)&brightness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MIN: {
			brightness = 0;
			memcpy(*data, (uint8_t *)&brightness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MAX: {
			brightness = 100;
			memcpy(*data, (uint8_t *)&brightness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_RES: {
			brightness = 1;
			memcpy(*data, (uint8_t *)&brightness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_INFO: {
			(*data)[0] = 0x03; // support set and get
			*len = 1;
		} break;
		case VIDEO_REQUEST_GET_DEF: {
			brightness = 50;
			memcpy(*data, (uint8_t *)&brightness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_LEN: {
			(*data)[0] = 0x02;
			*len = 2;
		} break;
		default:
			USB_LOG_WRN("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
			return -1;
	}
	return 0;
}

static int usbd_vc_pu_contrast_request_handler(uint8_t device_id, uint8_t bRequest, uint8_t **data, uint32_t *len)
{
	int ret;
	struct uvc_device_info *uvc = uvc_container_of_device_id(device_id);
	int group_id = uvc->video.vpss_group;
	uint16_t wContrast;

	switch (bRequest) {
		case VIDEO_REQUEST_SET_CUR: {
			wContrast = (uint16_t)(*data)[1] << 8 | (uint16_t)(*data)[0];
			if(wContrast > 100 || wContrast < 0)
				return -1;
			ret = CVI_VPSS_SetGrpProcAmp(group_id, PROC_AMP_CONTRAST, wContrast);
			if (ret != 0) {
				USB_LOG_WRN("set vpss grp amp(%d) failed with %#x", PROC_AMP_CONTRAST, ret);
				return -1;
			}
			USB_LOG_DBG("Video set Contrast:%d\r\n", wContrast);
		} break;
		case VIDEO_REQUEST_GET_CUR: {
			int32_t value = 0;
			ret = CVI_VPSS_GetGrpProcAmp(group_id, PROC_AMP_CONTRAST, &value);
			if (ret != 0) {
				USB_LOG_WRN("get vpss grp amp(%d) failed with %#x", PROC_AMP_CONTRAST, ret);
				return -1;
			}
			wContrast = value;
			memcpy(*data, (uint8_t *)&wContrast, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MIN: {
			wContrast = 0;
			memcpy(*data, (uint8_t *)&wContrast, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MAX: {
			wContrast = 100;
			memcpy(*data, (uint8_t *)&wContrast, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_RES: {
			wContrast = 1;
			memcpy(*data, (uint8_t *)&wContrast, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_INFO: {
			(*data)[0] = 0x03; // support set and get
			*len = 1;
		} break;
		case VIDEO_REQUEST_GET_DEF: {
			wContrast = 50;
			memcpy(*data, (uint8_t *)&wContrast, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_LEN: {
			(*data)[1] = 0x02;
			*len = 2;
		} break;
		default:
			USB_LOG_WRN("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
			return -1;
	}
	return 0;
}

static int usbd_vc_pu_hue_request_handler(uint8_t device_id, uint8_t bRequest, uint8_t **data, uint32_t *len)
{
	int ret;
	struct uvc_device_info *uvc = uvc_container_of_device_id(device_id);
	int group_id = uvc->video.vpss_group;
	int16_t wHue;

	switch (bRequest) {
		case VIDEO_REQUEST_SET_CUR: {
			wHue = (uint16_t)(*data)[1] << 8 | (uint16_t)(*data)[0];
			if(wHue >50 || wHue < -50)
				return -1;

			wHue = 50 - wHue;
			ret = CVI_VPSS_SetGrpProcAmp(group_id, PROC_AMP_HUE, wHue);
			if (ret != 0) {
				USB_LOG_WRN("set vpss grp amp(%d) failed with %#x", PROC_AMP_HUE, ret);
			}
			USB_LOG_DBG("Video set Hue:%d\r\n", wHue);
		} break;
		case VIDEO_REQUEST_GET_CUR: {
			int32_t value = 0;
			ret = CVI_VPSS_GetGrpProcAmp(group_id, PROC_AMP_HUE, &value);
			if (ret != 0) {
				USB_LOG_WRN("get vpss grp amp(%d) failed with %#x", PROC_AMP_HUE, ret);
				return -1;
			}
			wHue = 50 - value;
			memcpy(*data, (uint8_t *)&wHue, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MIN: {
			wHue = -50;
			memcpy(*data, (uint8_t *)&wHue, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MAX: {
			wHue = 50;
			memcpy(*data, (uint8_t *)&wHue, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_RES: {
			wHue = 0x0001;
			memcpy(*data, (uint8_t *)&wHue, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_INFO: {
			(*data)[0] = 0x03; //struct video_camera_capabilities
			*len = 1;
		} break;
		case VIDEO_REQUEST_GET_DEF: {
			wHue = 0;
			memcpy(*data, (uint8_t *)&wHue, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_LEN: {
			(*data)[0] = 0x02;
			*len = 2;
		} break;
		default:
			USB_LOG_WRN("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
			return -1;
	}
	return 0;
}

static int usbd_vc_pu_saturation_request_handler(uint8_t device_id, uint8_t bRequest, uint8_t **data, uint32_t *len)
{
	int ret;
	struct uvc_device_info *uvc = uvc_container_of_device_id(device_id);
	int group_id = uvc->video.vpss_group;
	uint16_t wSaturation;

	switch (bRequest) {
		case VIDEO_REQUEST_SET_CUR: {
			wSaturation = (uint16_t)(*data)[1] << 8 | (uint16_t)(*data)[0];
			if(wSaturation > 100 || wSaturation < 0)
				return -1;
			ret = CVI_VPSS_SetGrpProcAmp(group_id, PROC_AMP_SATURATION, wSaturation);
			if (ret != 0) {
				USB_LOG_WRN("set vpss grp amp(%d) failed with %#x", PROC_AMP_SATURATION, ret);
				return -1;
			}
			USB_LOG_DBG("Video set saturation:%d\r\n", wSaturation);
		} break;
		case VIDEO_REQUEST_GET_CUR: {
			int32_t value = 0;
			ret = CVI_VPSS_GetGrpProcAmp(group_id, PROC_AMP_SATURATION, &value);
			if (ret != 0) {
				USB_LOG_WRN("get vpss grp amp(%d) failed with %#x", PROC_AMP_SATURATION, ret);
				break;
			}
			wSaturation = value;
			memcpy(*data, (uint8_t *)&wSaturation, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MIN: {
			wSaturation = 0;
			memcpy(*data, (uint8_t *)&wSaturation, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MAX: {
			wSaturation = 100;
			memcpy(*data, (uint8_t *)&wSaturation, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_RES: {
			wSaturation = 0x0001;
			memcpy(*data, (uint8_t *)&wSaturation, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_INFO: {
			(*data)[0] = 0x03; // support set and get
			*len = 1;
		} break;
		case VIDEO_REQUEST_GET_DEF: {
			wSaturation = 50;
			memcpy(*data, (uint8_t *)&wSaturation, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_LEN: {
			(*data)[0] = 0x02;
			*len = 2;
		} break;
		default:
			USB_LOG_WRN("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
			return -1;
	}
	return 0;
}


static int usbd_vc_pu_sharpness_request_handler(uint8_t device_id, uint8_t bRequest, uint8_t **data, uint32_t *len)
{
	int ret;
	int vi_pipe = device_id;
	uint16_t wSharpness = 0;
	static ISP_SHARPEN_ATTR_S stDRCAttr;
	uint8_t cnt = 0;

	if(!cnt)
	{
		ret = CVI_ISP_GetSharpenAttr(vi_pipe, &stDRCAttr);
		if (ret != 0) {
			USB_LOG_WRN("CVI_ISP_GetSharpenAttr failed\n");
		}
		cnt = 1;
	}

	switch (bRequest) {
		case VIDEO_REQUEST_SET_CUR: {
			wSharpness = (uint16_t)(*data)[1] << 8 | (uint16_t)(*data)[0];
			if(wSharpness > 0xFF || wSharpness < 0)
				return -1;
			if(wSharpness<0 || wSharpness>0xff)
				return -1;
			stDRCAttr.Enable = CVI_TRUE;
			stDRCAttr.enOpType = OP_TYPE_MANUAL;
			stDRCAttr.stManual.GlobalGain = (CVI_U8)wSharpness;
			USB_LOG_DBG("Video set Sharpness:%d\r\n", wSharpness);
			ret = CVI_ISP_SetSharpenAttr(vi_pipe, &stDRCAttr);
			if (ret != 0) {
				USB_LOG_WRN("CVI_ISP_SetSharpenAttr failed\n");
				return -1;
			}
		} break;
		case VIDEO_REQUEST_GET_CUR: {
			ret = CVI_ISP_GetSharpenAttr(vi_pipe, &stDRCAttr);
			if (ret != 0) {
				USB_LOG_WRN("CVI_ISP_GetSharpenAttr failed\n");
				return -1;
			}
			wSharpness = stDRCAttr.stManual.GlobalGain;
			memcpy(*data, (uint8_t *)&wSharpness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MIN: {
			wSharpness = 0;
			memcpy(*data, (uint8_t *)&wSharpness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_MAX: {
			wSharpness = 0xFF;
			memcpy(*data, (uint8_t *)&wSharpness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_RES: {
			wSharpness = 0x0001;
			memcpy(*data, (uint8_t *)&wSharpness, 2);
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_INFO: {
			(*data)[0] = 0x03; // support set and get
			*len = 1;
		} break;
		case VIDEO_REQUEST_GET_LEN: {
			(*data)[0] = 0x02;
			*len = 2;
		} break;
		case VIDEO_REQUEST_GET_DEF: {
			wSharpness = 0x80;
			memcpy(*data, (uint8_t *)&wSharpness, 2);
			*len = 2;
		} break;
		default:
			USB_LOG_WRN("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
			return -1;
	}
	return 0;
}

// static int usbd_vc_pu_gain_request_handler(uint8_t device_id, uint8_t bRequest, uint8_t **data, uint32_t *len)
// {
//     int ret;
//     int vi_pipe = device_id;

//     switch (bRequest) {
//         case VIDEO_REQUEST_SET_CUR: {
//             ISP_EXPOSURE_ATTR_S pstExpAttr;
//             ret = CVI_ISP_GetExposureAttr(vi_pipe, &pstExpAttr);
//             if (ret != 0) {
//                 USB_LOG_WRN("CVI_ISP_GetExposureAttr failed\n");
//                 break;
//             }
//             uint16_t wGain = (uint16_t)(*data)[1] << 8 | (uint16_t)(*data)[0];
//             wGain = wGain * 1024;
//             pstExpAttr.stAuto.u16EVBias = (CVI_U16)wGain;
//             ret = CVI_ISP_SetExposureAttr(vi_pipe, &pstExpAttr);
//             if (ret != 0) {
//                 USB_LOG_WRN("CVI_ISP_SetExposureAttr failed\n");
//                 break;
//             }
//             USB_LOG_DBG("Video set wGain:%d\r\n", wGain);
//         } break;
//         case VIDEO_REQUEST_GET_CUR: {
//             ISP_EXPOSURE_ATTR_S pstExpAttr;
//             int32_t wGain = 0;
//             ret = CVI_ISP_GetExposureAttr(vi_pipe, &pstExpAttr);
//             if (ret != 0) {
//                 USB_LOG_WRN("CVI_ISP_GetExposureAttr failed\n");
//                 break;
//             }
//             wGain = (int32_t)pstExpAttr.stAuto.u16EVBias;
//             wGain = wGain / 1024;
//             memcpy(*data, (uint8_t *)&wGain, 2);
//             *len = 2;
//         } break;
//         case VIDEO_REQUEST_GET_MIN: {
//             uint16_t wGain = 0;
//             memcpy(*data, (uint8_t *)&wGain, 2);
//             *len = 2;
//         } break;
//         case VIDEO_REQUEST_GET_MAX: {
//             uint16_t wGain = 3;
//             memcpy(*data, (uint8_t *)&wGain, 2);
//             *len = 2;
//         } break;
//         case VIDEO_REQUEST_GET_RES: {
//             uint16_t wGain = 1;
//             memcpy(*data, (uint8_t *)&wGain, 2);
//             *len = 2;
//         } break;
//         case VIDEO_REQUEST_GET_INFO: {
//             (*data)[0] = 0x03; // support set and get
//             *len = 1;
//         } break;
//         case VIDEO_REQUEST_GET_DEF: {
//             uint16_t wGain = 1;
//             memcpy(*data, (uint8_t *)&wGain, 2);
//             *len = 2;
//         } break;
//         default:
//             USB_LOG_WRN("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
//             return -1;
//     }
//     return 0;
// }

int usbd_vc_processing_unit_request_handler(struct usbd_video_cfg_priv *usbd_video_cfg, struct usb_setup_packet *setup,
                                            uint8_t **data, uint32_t *len)
{
	uint8_t control_selector = (uint8_t)(setup->wValue >> 8);
	uint8_t bRequest = setup->bRequest;
	uint8_t entity_id = (uint8_t)(setup->wIndex >> 8);
	uint8_t device_id = entity_id / VIDEO_ENTITY_NUM;

	switch (control_selector) {
		// case VIDEO_PU_BACKLIGHT_COMPENSATION_CONTROL:
		//     return usbd_vc_pu_backlight_compensation_request_handler(device_id, bRequest, data, len);
		case VIDEO_PU_BRIGHTNESS_CONTROL:
			return usbd_vc_pu_brightness_request_handler(device_id, bRequest, data, len);
		case VIDEO_PU_CONTRAST_CONTROL:
			return usbd_vc_pu_contrast_request_handler(device_id, bRequest, data, len);
		case VIDEO_PU_HUE_CONTROL:
			return usbd_vc_pu_hue_request_handler(device_id, bRequest, data, len);
		case VIDEO_PU_SATURATION_CONTROL:
			return usbd_vc_pu_saturation_request_handler(device_id, bRequest, data, len);
		case VIDEO_PU_SHARPNESS_CONTROL:
			return usbd_vc_pu_sharpness_request_handler(device_id, bRequest, data, len);
		// case VIDEO_PU_GAIN_CONTROL:
		//     return usbd_vc_pu_gain_request_handler(device_id, bRequest, data, len);
		default:
			usbd_video_cfg->error_code = 0x06;
			USB_LOG_WRN("Unhandled Video Class processing unit control selector 0x%02x\r\n", control_selector);
			return -1;
	}
	return 0;
}
