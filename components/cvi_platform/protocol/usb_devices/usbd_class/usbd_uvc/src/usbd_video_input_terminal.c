#include "cvi_vpss.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "drv/pwm.h"

#include "usbd_core.h"
#include "usbd_video.h"
#include "usbd_uvc.h"

#define CONFIG_UVC_PAN_RANGE 10  // [-CONFIG_UVC_PAN_RANGE, CONFIG_UVC_PAN_RANGE]
#define CONFIG_UVC_TILT_RANGE 10 // [-CONFIG_UVC_TILT_RANGE, CONFIG_UVC_TILT_RANGE]
#define PAN_TILT_STEP 3600

static int set_pantile(uint8_t vpss_grp_id, uint8_t vpss_chn_id, int32_t pan, int32_t tilt)
{
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr;
	VPSS_CROP_INFO_S stCropInfo = {0};
	int ret;

	ret = CVI_VPSS_GetGrpAttr(vpss_grp_id, &stVpssGrpAttr);
	if (ret != CVI_SUCCESS) {
		USB_LOG_ERR("get vpss group attr failed with %#x\n", ret);
		return ret;
	}

	ret = CVI_VPSS_GetChnAttr(vpss_grp_id, vpss_chn_id, &stVpssChnAttr);
	if (ret != CVI_SUCCESS) {
		USB_LOG_ERR("get vpss channel attr failed with %#x\n", ret);
		return ret;
	}

	int zero_x = (stVpssGrpAttr.u32MaxW - stVpssChnAttr.u32Width) / 2;
	int zero_y = (stVpssGrpAttr.u32MaxH - stVpssChnAttr.u32Height) / 2;
	int pixel_step_x = CONFIG_UVC_PAN_RANGE == 0 ? 0 : zero_x / CONFIG_UVC_PAN_RANGE;
	int pixel_step_y = CONFIG_UVC_TILT_RANGE == 0 ? 0 : zero_y / CONFIG_UVC_TILT_RANGE;

	stCropInfo.bEnable = true;
	stCropInfo.stCropRect.s32X = zero_x + pan * pixel_step_x;
	stCropInfo.stCropRect.s32Y = zero_y + tilt * pixel_step_y;
	stCropInfo.stCropRect.u32Width = stVpssChnAttr.u32Width;
	stCropInfo.stCropRect.u32Height = stVpssChnAttr.u32Height;
	ret = CVI_VPSS_SetChnCrop(vpss_grp_id, vpss_chn_id, &stCropInfo);
	if (ret != CVI_SUCCESS) {
		USB_LOG_ERR("set vpss channel crop failed with %#x\n", ret);
		return ret;
	}

	return 0;
}

/*
 * accroding to specified grp-chn's crop info, get pan and tilt
*/
static void get_pantilt(uint8_t vpss_grp_id, uint8_t vpss_chn_id, int32_t *pan, int32_t *tilt)
{
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr;
	VPSS_CROP_INFO_S stCropInfo;

	/* if not set crop, this function will return error */
	int ret = CVI_VPSS_GetChnCrop(vpss_grp_id, vpss_chn_id, &stCropInfo);
	if (ret != CVI_SUCCESS || !stCropInfo.bEnable) {
		USB_LOG_ERR("get crop failed or not enable, ret = %#x\n", ret);
		goto fail;
	}

	ret = CVI_VPSS_GetGrpAttr(vpss_grp_id, &stVpssGrpAttr);
	if (ret != CVI_SUCCESS) {
		USB_LOG_ERR("get vpss group attr failed with %#x\n", ret);
		goto fail;
	}

	ret = CVI_VPSS_GetChnAttr(vpss_grp_id, vpss_chn_id, &stVpssChnAttr);
	if (ret != CVI_SUCCESS) {
		USB_LOG_ERR("get vpss channel attr failed with %#x\n", ret);
		goto fail;
	}

	int zero_x = (stVpssGrpAttr.u32MaxW - stVpssChnAttr.u32Width) / 2;
	int zero_y = (stVpssGrpAttr.u32MaxH - stVpssChnAttr.u32Height) / 2;
	int pixel_step_x = CONFIG_UVC_PAN_RANGE == 0 ? 0 : zero_x / CONFIG_UVC_PAN_RANGE;
	int pixel_step_y = CONFIG_UVC_TILT_RANGE == 0 ? 0 : zero_y / CONFIG_UVC_TILT_RANGE;

	*pan = pixel_step_x == 0 ? 0 : (stCropInfo.stCropRect.s32X - zero_x) / pixel_step_x;
	*tilt = pixel_step_y == 0 ? 0 : (stCropInfo.stCropRect.s32Y - zero_y) / pixel_step_y;
	return;
fail:
	*pan = 0;
	*tilt = 0;
}

/* Pan（左右） and tilt（上下） function in one request,
 * pan abs settings put in low 4 bytes and tilt in high 4 bytes
 */
static int usbd_vc_ct_pantilt_absolute_request_handler(uint8_t device_id, uint8_t bRequest, uint8_t **data, uint32_t *len)
{
	struct uvc_device_info *uvc = uvc_container_of_device_id(device_id);
	uint8_t vpss_grp_id = uvc->video.vpss_group;
	uint8_t vpss_chn_id = uvc->video.vpss_channel;

	switch (bRequest) {
		case VIDEO_REQUEST_SET_CUR: {
			int32_t *arr = (int32_t *)(*data);
			int pan = arr[0] / PAN_TILT_STEP;
			int tilt = arr[1] / PAN_TILT_STEP;

			return set_pantile(vpss_grp_id, vpss_chn_id, pan, tilt);
		} break;
		case VIDEO_REQUEST_GET_CUR: {
			int32_t *arr = (int32_t *)(*data);
			int32_t pan = 0, tilt = 0;
			get_pantilt(vpss_grp_id, vpss_chn_id, &pan, &tilt);

			arr[0] = pan * PAN_TILT_STEP;
			arr[1] = tilt * PAN_TILT_STEP;
			*len = 8;
		} break;
		case VIDEO_REQUEST_GET_MIN: {
			int32_t *arr = (int32_t *)(*data);
			arr[0] = -CONFIG_UVC_PAN_RANGE * PAN_TILT_STEP;
			arr[1] = -CONFIG_UVC_TILT_RANGE * PAN_TILT_STEP;
			*len = 8;
		} break;
		case VIDEO_REQUEST_GET_MAX: {
			int32_t *arr = (int32_t *)(*data);
			arr[0] = CONFIG_UVC_PAN_RANGE * PAN_TILT_STEP;
			arr[1] = CONFIG_UVC_TILT_RANGE * PAN_TILT_STEP;
			*len = 8;
		} break;
		case VIDEO_REQUEST_GET_RES: {
			int32_t *arr = (int32_t *)(*data);
			arr[0] = PAN_TILT_STEP;
			arr[1] = PAN_TILT_STEP;
			*len = 8;
		} break;
		case VIDEO_REQUEST_GET_INFO: {
			(*data)[0] = 0x03; // support get and set
			*len = 1;
		} break;
		case VIDEO_REQUEST_GET_DEF: {
			memset(*data, 0, 8); // default value 0
			*len = 8;
		} break;
		default:
			USB_LOG_ERR("Unhandled Video Class bRequest 0x%02x\r\n", bRequest);
			return -1;
	}
	return 0;
}

static int usbd_vc_camera_terminal_request_handler(struct usbd_video_cfg_priv *usbd_video_cfg,
                                                   struct usb_setup_packet *setup,
                                                   uint8_t **data, uint32_t *len)
{
	uint8_t control_selector = (uint8_t)(setup->wValue >> 8);
	uint8_t bRequest = setup->bRequest;
	uint8_t entity_id = (uint8_t)(setup->wIndex >> 8);
	uint8_t device_id = entity_id / VIDEO_ENTITY_NUM;

	switch (control_selector) {
		case VIDEO_CT_PANTILT_ABSOLUTE_CONTROL:
			return usbd_vc_ct_pantilt_absolute_request_handler(device_id, bRequest, data, len);
		default:
			usbd_video_cfg->error_code = 0x06;
			USB_LOG_WRN("Unhandled Video Class camera terminal control selector 0x%02x\r\n", control_selector);
			return -1;
	}

	return 0;
}

int usbd_vc_input_terminal_request_handler(struct usbd_video_cfg_priv *usbd_video_cfg, uint16_t terminal_type,
                                           struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
	switch (terminal_type) {
		case VIDEO_ITT_CAMERA:
			return usbd_vc_camera_terminal_request_handler(usbd_video_cfg, setup, data, len);
		default:
			USB_LOG_WRN("Unhandled Video Class input terminal terminal type 0x%02x\r\n", terminal_type);
			return -2;
	}

	return 0;
};
