#include <stdio.h>
#include <aos/cli.h>
#include "usbd_core.h"
#include "usbd_video.h"
#include "media_video.h"
#include "cvi_venc.h"
#include "cvi_vpss.h"
#include "cvi_sys.h"
#include <core/core_rv64.h>

#include "usbd_uvc.h"
#include "usbd_comp.h"
#include "usbd_uvc_descriptor.h"

#define WIDTH  (unsigned int)(1920)
#define HEIGHT (unsigned int)(1080)

#define MAX_FRAME_SIZE (unsigned long)(WIDTH * HEIGHT * 2)
#define DEFAULT_FRAME_SIZE_CHNA (unsigned long)  (WIDTH * HEIGHT * 3 / 2)
#if (USBD_UVC_NUM >= 2)
#define DEFAULT_FRAME_SIZE_CHNB (unsigned long)  (WIDTH * HEIGHT * 3 / 2)
#endif
#if (USBD_UVC_NUM >= 3)
#define DEFAULT_FRAME_SIZE_CHNC (unsigned long)  (WIDTH * HEIGHT * 3 / 2)
#endif
#define TIMEOUT_CHECK_CNT  500 // 500 * 1ms

#define MJPEG_FORMAT_INDEX  (1)
#define H264_FORMAT_INDEX   (2)
#define YUYV_FORMAT_INDEX   (3)
#define NV21_FORMAT_INDEX   (4)
#define H265_FORMAT_INDEX   (5)

static int av_session_init_flag = CVI_FALSE;
static atomic_t uvc_pause_flag = CVI_FALSE;
static atomic_t uvc_pause_done = CVI_FALSE;

static uint8_t enum_speed = USB_SPEED_UNKNOWN;

static struct uvc_frame_info_st yuy2_frame_info[] = {
    {1, 800, 600, 15, 0},
    {2, 640, 360, 15, 0},
    {3, 400, 300, 15, 0},
    {5, 480, 320, 15, 0},
    {6, 480, 360, 15, 0},
    {7, 1280, 720, 15, 0},
};

static struct uvc_frame_info_st mjpeg_frame_info[] = {
	{1, 800, 480, 	15, 0, 10*1024},
	{2, 864, 480, 	30, 0, 10*1024},
	{3, 1920, 1080, 30, 0, 20*1024},
};

static struct uvc_frame_info_st h264_frame_info[] = {
    {1, 1280, 720,  15, 0, 1*1024},
    {2, 1920, 1080, 15, 0, 3*512},
};

//  static struct uvc_frame_info_st nv21_frame_info[] = {
//      {1, 800, 600, 15, 0},
//      {2, 1280, 720, 15, 0},
//      {3, 640, 480, 15, 0},
//  };

static struct uvc_frame_info_st h265_frame_info[] = {
    {1, 1280, 720,  15, 0, 1*1024},
    {2, 1920, 1080, 15, 0, 3*512},
};

static struct uvc_format_info_st uvc_format_info_chna[] = {
    {MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info), mjpeg_frame_info},
    {H264_FORMAT_INDEX, UVC_FORMAT_H264, 1, ARRAY_SIZE(h264_frame_info), h264_frame_info},
    {YUYV_FORMAT_INDEX, UVC_FORMAT_YUY2, 1, ARRAY_SIZE(yuy2_frame_info), yuy2_frame_info},
    // {NV21_FORMAT_INDEX, UVC_FORMAT_NV21, 1, ARRAY_SIZE(nv21_frame_info), nv21_frame_info},
    {H265_FORMAT_INDEX, UVC_FORMAT_H265, 1, ARRAY_SIZE(h265_frame_info), h265_frame_info},
};

#if (USBD_UVC_NUM >= 2)
static struct uvc_format_info_st uvc_format_info_chnb[] = {
    {MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info), mjpeg_frame_info},
    {H264_FORMAT_INDEX, UVC_FORMAT_H264, 1, ARRAY_SIZE(h264_frame_info), h264_frame_info},
    {YUYV_FORMAT_INDEX, UVC_FORMAT_YUY2, 1, ARRAY_SIZE(yuy2_frame_info), yuy2_frame_info},
    // {NV21_FORMAT_INDEX, UVC_FORMAT_NV21, 1, ARRAY_SIZE(nv21_frame_info), nv21_frame_info},
    {H265_FORMAT_INDEX, UVC_FORMAT_H265, 1, ARRAY_SIZE(h265_frame_info), h265_frame_info},
};
#endif

#if (USBD_UVC_NUM >= 3)
static struct uvc_format_info_st uvc_format_info_chnc[] = {
    {MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info), mjpeg_frame_info},
    {H264_FORMAT_INDEX, UVC_FORMAT_H264, 1, ARRAY_SIZE(h264_frame_info), h264_frame_info},
    {YUYV_FORMAT_INDEX, UVC_FORMAT_YUY2, 1, ARRAY_SIZE(yuy2_frame_info), yuy2_frame_info},
    // {NV21_FORMAT_INDEX, UVC_FORMAT_NV21, 1, ARRAY_SIZE(nv21_frame_info), nv21_frame_info},
    {H265_FORMAT_INDEX, UVC_FORMAT_H265, 1, ARRAY_SIZE(h265_frame_info), h265_frame_info},
};
#endif

static struct uvc_device_info uvc[USBD_UVC_MAX_NUM] = {
    {
        // .ep = 0x81,
        .format_info = uvc_format_info_chna,
        .formats = ARRAY_SIZE(uvc_format_info_chna),
        .video = {0, 0, 0},
    },
	#if (USBD_UVC_NUM >= 2)
    {
        // .ep = 0x82,
        .format_info = uvc_format_info_chnb,
        .formats = ARRAY_SIZE(uvc_format_info_chnb),
        .video = {1, 1, 1},
    },
	#endif
	#if (USBD_UVC_NUM >= 3)
    {
        // .ep = 0x83,
        .format_info = uvc_format_info_chnc,
        .formats = ARRAY_SIZE(uvc_format_info_chnc),
        .video = {2, 1, 0},
    }
	#endif
};

struct uvc_device_info *uvc_container_of_device_id(uint8_t device_id)
{
	return &uvc[device_id];
}

static uint8_t media_buffer_chna[DEFAULT_FRAME_SIZE_CHNA] __attribute__((aligned(64)));
#if (USBD_UVC_NUM >= 2)
static uint8_t media_buffer_chnb[DEFAULT_FRAME_SIZE_CHNB] __attribute__((aligned(64)));
#endif
#if (USBD_UVC_NUM >= 3)
static uint8_t media_buffer_chnc[DEFAULT_FRAME_SIZE_CHNC] __attribute__((aligned(64)));
#endif
static uint8_t *media_buffer[] = {
	media_buffer_chna,
	#if (USBD_UVC_NUM >= 2)
	media_buffer_chnb,
	#endif
	#if (USBD_UVC_NUM >= 3)
	media_buffer_chnc,
	#endif
};

static CVI_S32 is_media_info_update(struct uvc_device_info *info){
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	VENC_CHN_ATTR_S stVencChnAttr,*pstVencChnAttr = &stVencChnAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr, *pstVpssChnAttr = &stVpssChnAttr;
	PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
	CVI_U8 u8VencInitStatus = pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8InitStatus;


	struct uvc_format_info_st uvc_format_info;
	struct uvc_frame_info_st uvc_frame_info;

	uvc_get_video_format_info(&uvc_format_info);
	uvc_get_video_frame_info(&uvc_frame_info);

	CVI_VPSS_GetChnAttr(info->video.vpss_group, info->video.vpss_channel, pstVpssChnAttr);

	switch(uvc_format_info.format_index){
	case YUYV_FORMAT_INDEX:
		enPixelFormat = PIXEL_FORMAT_YUYV;
		break;
	case NV21_FORMAT_INDEX:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	default:
		enPixelFormat = PIXEL_FORMAT_MAX;
		break;
	}

	if(u8VencInitStatus == 0 && enPixelFormat == PIXEL_FORMAT_MAX)
		return CVI_TRUE;

	if((pstVpssChnAttr->enPixelFormat != enPixelFormat) ||
		(pstVpssChnAttr->stFrameRate.s32DstFrameRate != uvc_frame_info.fps) ||
		(pstVpssChnAttr->u32Width != uvc_frame_info.width) ||
		(pstVpssChnAttr->u32Height != uvc_frame_info.height))
		return CVI_TRUE;

	if(u8VencInitStatus == 0 && enPixelFormat != PIXEL_FORMAT_MAX)
		return CVI_FALSE;


	switch(uvc_format_info.format_index){
	case MJPEG_FORMAT_INDEX:
		enType = PT_MJPEG;
		break;
	case H264_FORMAT_INDEX:
		enType = PT_H264;
		break;
	case H265_FORMAT_INDEX:
		enType = PT_H265;
		break;
	default:
		enType = PT_BUTT;
		break;
	}

	if(u8VencInitStatus == 0 && enType != PT_BUTT)
		return CVI_TRUE;

	CVI_VENC_GetChnAttr(info->video.venc_channel, pstVencChnAttr);
	if((pstVencChnAttr->stVencAttr.enType != enType)||
		(pstVencChnAttr->stVencAttr.u32PicWidth != uvc_frame_info.width) ||
		(pstVencChnAttr->stVencAttr.u32PicHeight != uvc_frame_info.height))
		return CVI_TRUE;

	return CVI_FALSE;

}


CVI_S32 uvc_index_of_vs_intf(uint8_t intf)
{
    for (int i = 0; i < USBD_UVC_NUM; i++) {
        if(intf == uvc[i].vs_intf.intf_num)
            return i;
    }
    return -1;
}

static void uvc_parse_media_info(uint8_t intf, uint8_t bFormatIndex, uint8_t bFrameIndex)
{
    const struct uvc_format_info_st *format_info = NULL;
	const int uvc_index = uvc_index_of_vs_intf(intf);
    int uvcout_format_cnt;

	switch(uvc_index)
	{
		case 0:
			uvcout_format_cnt = ARRAY_SIZE(uvc_format_info_chna);
			break;
	#if (USBD_UVC_NUM >= 2)
		case 1:
			uvcout_format_cnt = ARRAY_SIZE(uvc_format_info_chnb);
			break;
	#endif
	#if (USBD_UVC_NUM >= 3)
		case 2:
			uvcout_format_cnt = ARRAY_SIZE(uvc_format_info_chnb);
			break;
	#endif
	default:
		aos_debug_printf(" cur uvc_index not suppert!\n");
		break;
	}

    if (bFormatIndex < 0)
        bFormatIndex = uvcout_format_cnt + bFormatIndex;
    if (bFormatIndex < 0 || bFormatIndex > uvcout_format_cnt)
    {
        aos_debug_printf("format_cnt =%d, format %d error!\r\n", uvcout_format_cnt, bFormatIndex);
        return;
    }

	switch(uvc_index)
	{
		case 0:
			format_info = &uvc_format_info_chna[bFormatIndex - 1];
			break;
#if (USBD_UVC_NUM >= 2)
		case 1:
			format_info = &uvc_format_info_chnb[bFormatIndex - 1];
			break;
#endif
#if (USBD_UVC_NUM >= 3)
		case 2:
			format_info = &uvc_format_info_chnc[bFormatIndex - 1];
			break;
#endif
	default:
		aos_debug_printf(" cur uvc_index not suppert!\n");
		break;
	}

    const int nframes = format_info->frame_cnt;
    if (bFrameIndex < 0)
        bFrameIndex = nframes + bFrameIndex;
    if (bFrameIndex < 0 || bFrameIndex > nframes)
    {
        aos_debug_printf("nframes = %d, frame %d error!\r\n", nframes, bFrameIndex);
        return;
    }

	uvc_set_video_format_info(format_info);
	uvc_set_video_frame_info(&format_info->frames[bFrameIndex - 1]);
}

static void uvc_media_update(struct uvc_device_info *info){
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
	VPSS_CHN_ATTR_S stVpssChnAttr;
	CVI_U8 u8VencInitStatus = pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8InitStatus;
	VENC_RECV_PIC_PARAM_S stRecvParam = {0};

	struct uvc_format_info_st uvc_format_info;
	struct uvc_frame_info_st uvc_frame_info;
	uvc_get_video_format_info(&uvc_format_info);
	uvc_get_video_frame_info(&uvc_frame_info);

	info->format_index = uvc_format_info.format_index;
	switch(uvc_format_info.format_index){
	case YUYV_FORMAT_INDEX:
		enPixelFormat = PIXEL_FORMAT_YUYV;
		break;
	case NV21_FORMAT_INDEX:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	default:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	}


	switch(uvc_format_info.format_index){
	case MJPEG_FORMAT_INDEX:
		enType = PT_MJPEG;
		break;
	case H264_FORMAT_INDEX:
		enType = PT_H264;
		break;
	case H265_FORMAT_INDEX:
		enType = PT_H265;
		break;
	default:
		enType = PT_MJPEG;
		break;
	}

	if(u8VencInitStatus == 1) {
		MEDAI_VIDEO_VencChnDeinit(pstVencCfg, info->video.venc_channel);
		printf("venc chn %d deinit\n", info->video.venc_channel);
	}

#if CONFIG_UVC_CROP_BEFORE_SCALE
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CROP_INFO_S stCropInfo = {0};
	int crop_ratio = 0;

	CVI_VPSS_GetGrpAttr(info->video.vpss_group, &stVpssGrpAttr);
	/* `*1000` is to get a more accurate value  */
	crop_ratio = MIN(stVpssGrpAttr.u32MaxW * 1000 / uvc_frame_info.width,
			 stVpssGrpAttr.u32MaxH * 1000 / uvc_frame_info.height);
	stCropInfo.stCropRect.u32Width =  uvc_frame_info.width * crop_ratio / 1000;
	stCropInfo.stCropRect.u32Height = uvc_frame_info.height * crop_ratio / 1000;
	stCropInfo.stCropRect.s32X = (stVpssGrpAttr.u32MaxW - stCropInfo.stCropRect.u32Width) / 2;
	stCropInfo.stCropRect.s32Y = (stVpssGrpAttr.u32MaxH - stCropInfo.stCropRect.u32Height) / 2;
	stCropInfo.bEnable = true;
	CVI_VPSS_SetChnCrop(info->video.vpss_group, info->video.vpss_channel, &stCropInfo);
#endif

	CVI_VPSS_GetChnAttr(info->video.vpss_group, info->video.vpss_channel, &stVpssChnAttr);
	stVpssChnAttr.enPixelFormat = enPixelFormat;
	stVpssChnAttr.u32Width = uvc_frame_info.width;
	stVpssChnAttr.u32Height = uvc_frame_info.height;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = 30;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = uvc_frame_info.fps;
	CVI_VPSS_SetChnAttr(info->video.vpss_group, info->video.vpss_channel, &stVpssChnAttr);

	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16Width = uvc_frame_info.width;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16Height = uvc_frame_info.height;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16EnType = enType;

	if(uvc_frame_info.bitrate){
		pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u32BitRate = uvc_frame_info.bitrate;
	}
	else{
		pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u32BitRate =
			(enType == PT_MJPEG) ? CONFIG_UVC_MJPEG_BITRATE : CONFIG_UVC_H264_H265_BITRATE;
	}

	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u8SrcFrameRate = uvc_frame_info.fps;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16RcMode =
			(enType == PT_MJPEG) ? VENC_RC_MODE_MJPEGCBR:((enType == PT_H264) ? VENC_RC_MODE_H264CBR : VENC_RC_MODE_H265CBR);;
	// pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8ModId = CVI_ID_VPSS;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8DevId = info->video.vpss_group;
   	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8DevChnid = info->video.vpss_channel;

	if(MJPEG_FORMAT_INDEX == uvc_format_info.format_index || H264_FORMAT_INDEX == uvc_format_info.format_index|| H265_FORMAT_INDEX == uvc_format_info.format_index) {
		MEDIA_VIDEO_VencChnInit(pstVencCfg, info->video.venc_channel);
		printf("venc chn %d init\n", info->video.venc_channel);

		stRecvParam.s32RecvPicNum = -1;
		CVI_VENC_StartRecvFrame(info->video.venc_channel, &stRecvParam);
		pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8InitStatus = 1;
    }
}

struct uvc_device_info *uvc_container_of_ep(uint8_t ep)
{
    for (int i = 0; i < USBD_UVC_NUM; i++) {
        if(ep == uvc[i].ep)
            return &uvc[i];
    }
    return NULL;
}

struct uvc_device_info *uvc_container_of_vs_intf(uint8_t intf)
{
    for (int i = 0; i < USBD_UVC_NUM; i++) {
        if(intf == uvc[i].vs_intf.intf_num)
            return &uvc[i];
    }
    return NULL;
}

static void uvc_streaming_on(uint8_t intf, int is_on) {
		struct uvc_device_info *uvc = uvc_container_of_vs_intf(intf);
		USB_LOG_INFO("streaming %s\n", is_on ? "on" : "off");

		if(is_on && is_media_info_update(uvc)) {
			uvc->update_flag = 1;
		}

		uvc->header_flip = false;
		uvc->xfer_flag = false;
		uvc->streaming_on = is_on;
		uvc->tx_cnt = 0;
}

#if 0
void usbd_configure_done_callback(void)
{
    /* no out ep, so do nothing */
}
#endif

static void uvc_setup_class_control(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

static void uvc_setup_class_streaming(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

static void uvc_data_out(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

#if CONFIG_USB_BULK_UVC
static void uvc_tx_complete(uint8_t ep, uint32_t nbytes)
{
	// aos_debug_printf("%d bytes of data sent at ep(%d)\n", nbytes, ep);
	struct uvc_device_info *uvc = uvc_container_of_ep(ep);
	uvc->tx_frm_idx++;
	uvc->xfer_flag = false;
	//aos_work_run(&uvc_workqueue, &uvc_frame_submmit);
}

static void usbd_video_frame_submmit(void *args) {
	struct uvc_device_info *uvc = (struct uvc_device_info *)args;
	uint32_t idx = FRM_BUFFER_GET_IDX(uvc->tx_frm_idx);

	if (!uvc->streaming_on || uvc->xfer_flag) {
		return;
	}

	if(uvc->tx_frm_idx < uvc->rx_frm_idx && uvc->frm_sz[idx]>0) {
		uvc->xfer_flag = true;
		usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc + idx * uvc->default_frame_size, uvc->frm_sz[idx]);
	}
}
#else
static void uvc_tx_complete(uint8_t ep, uint32_t nbytes)
{
    // aos_debug_printf("%d bytes of data sent at ep(%d)\n", nbytes, ep);
    struct uvc_device_info *uvc = uvc_container_of_ep(ep);
	uint32_t data_len = 0;

    uvc->xfer_offset += nbytes;
	if (uvc->xfer_len > nbytes) {
		uvc->xfer_len -= nbytes;
	} else {
		uvc->xfer_len = 0;
	}

	if (uvc->xfer_len > 0) {
		data_len = uvc->xfer_len < uvc->max_payload_size ? uvc->xfer_len : uvc->max_payload_size;
		usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc + uvc->xfer_offset, data_len);
	} else {
		uvc->xfer_flag = false;
		uvc->xfer_offset = 0;
		uvc->xfer_len = 0;
	}
}
#endif

static uint32_t uvc_payload_fill(struct uvc_device_info *uvc, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *out_len)
{
    uint32_t packets;
    uint32_t last_packet_size;
    uint32_t picture_pos = 0;
    uint8_t uvc_header[12] = { 0x0c, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint32_t size_uvc_header = sizeof(uvc_header);
    uint32_t size_per_packet = uvc->max_payload_size;
    uint32_t size_payload = size_per_packet - size_uvc_header;

    if (size_payload > 10240) {
        USB_LOG_ERR("the size of payload is too long!!!!\n");
    }

	if (input_len + size_uvc_header > uvc->default_frame_size) {
		USB_LOG_ERR("input_len + size_uvc_header (%u) > DEFAULT_FRAME_SIZE (%u)\n",
			input_len + size_uvc_header, uvc->default_frame_size);
		return 0;
	}

	if (!uvc->header_flip) {
		uvc_header[1] = 0x8c;
	}
	uvc->header_flip = !uvc->header_flip;

    // The following equals to packets = roundup(input_len / size_payload)
    packets = (input_len + size_payload - 1) / (size_payload);
    last_packet_size = input_len - ((packets - 1) * size_payload) + size_uvc_header;

    for (size_t i = 0; i < packets; i++) {
        output[size_per_packet* i] = uvc_header[0];
        output[size_per_packet * i + 1] = uvc_header[1];
        if (i == (packets - 1)) {
            memcpy(&output[size_uvc_header + size_per_packet * i],
                &input[picture_pos], last_packet_size - size_uvc_header);
            output[size_per_packet * i + 1] |= (1 << 1);
        } else {
            memcpy(&output[size_uvc_header + size_per_packet * i],
                &input[picture_pos], size_payload);
            picture_pos += size_payload;
        }
    }

    *out_len = (input_len + size_uvc_header * packets);
    return packets;
}

void usbd_video_commit_set_cur(uint8_t intf, struct video_probe_and_commit_controls *commit)
{
	uvc_parse_media_info(intf, commit->bFormatIndex, commit->bFrameIndex);
    aos_debug_printf("commit format idx:%d, frame idx:%d\n", commit->bFormatIndex, commit->bFrameIndex);
}

static uvc_event_callbacks_t uvc_evt_callbks = {
    .uvc_event_stream_on = uvc_streaming_on,
    .uvc_event_setup_class_control = uvc_setup_class_control,
    .uvc_event_setup_class_streaming = uvc_setup_class_streaming,
    .uvc_event_data_out = uvc_data_out,
    .uvc_event_tx_complete = uvc_tx_complete,
};

static uvc_video_control_callbacks_t uvc_video_control_callbks = {
	.uvc_vc_input_terminal_handler = usbd_vc_input_terminal_request_handler,
	.uvc_vc_processing_unit_handler = usbd_vc_processing_unit_request_handler,
	.uvc_vc_extension_unit_handler = usbd_vc_extension_unit_request_handler,
};

static void vedio_streaming_send(struct uvc_device_info *uvc, int dev_index)
{
	int i, ret = 0;
	uint32_t data_len = 0;
	uint32_t buf_len = 0, buf_len_stride = 0, packets = 0;
    VENC_STREAM_S stStream = {0},*pstStream= &stStream;
	VENC_PACK_S *ppack;
	VIDEO_FRAME_INFO_S stVideoFrame, *pstVideoFrame=&stVideoFrame;
	VPSS_CHN_ATTR_S stChnAttr,*pstChnAttr = &stChnAttr;

	if(uvc->update_flag){
		uvc_media_update(uvc);
		uvc->update_flag = 0;
	}

	if (enum_speed != USB_SPEED_HIGH && uvc->xfer_flag) {
		aos_msleep(4);
		return;
	}

	switch(uvc->format_index) {
	case H264_FORMAT_INDEX:
	case H265_FORMAT_INDEX:
	case MJPEG_FORMAT_INDEX:
		ret = MEDIA_VIDEO_VencGetStream(uvc->video.venc_channel, pstStream, 2000);
		if(ret != CVI_SUCCESS){
			aos_msleep(1);
			return;
		}

		for (i = 0; i < pstStream->u32PackCount; ++i)
		{
			ppack = &pstStream->pstPack[i];
			memcpy(media_buffer[dev_index] + buf_len,
					ppack->pu8Addr + ppack->u32Offset,
					ppack->u32Len - ppack->u32Offset);
			buf_len += (ppack->u32Len - ppack->u32Offset);

			if (buf_len > uvc->default_frame_size) {
				printf("venc buf_len oversize\n");
				MEDIA_VIDEO_VencReleaseStream(uvc->video.venc_channel, pstStream);
				return;
			}
		}

		ret = MEDIA_VIDEO_VencReleaseStream(uvc->video.venc_channel, pstStream);
		if(ret != CVI_SUCCESS)
			printf("MEDIA_VIDEO_VencReleaseStream failed\n");
		break;
	case YUYV_FORMAT_INDEX:
		ret = CVI_VPSS_GetChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame, -1);
		if(ret != CVI_SUCCESS){
			printf("CVI_VPSS_GetChnFrame failed\n");
			aos_msleep(1);
			return;
		}
		CVI_VPSS_GetChnAttr(uvc->video.vpss_group, uvc->video.vpss_channel, pstChnAttr);

		pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t *)pstVideoFrame->stVFrame.u64PhyAddr[0];
		data_len = pstChnAttr->u32Width * 2;
		for (i = 0;i < (pstChnAttr->u32Height); ++i)
		{
			memcpy(media_buffer[dev_index] + buf_len, pstVideoFrame->stVFrame.pu8VirAddr[0] +
				buf_len_stride, data_len);

			buf_len += pstChnAttr->u32Width * 2;
			buf_len_stride += pstVideoFrame->stVFrame.u32Stride[0];
		}
		pstVideoFrame->stVFrame.pu8VirAddr[0] = NULL;

		ret = CVI_VPSS_ReleaseChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame);
		if(ret != CVI_SUCCESS)
			printf("CVI_VPSS_ReleaseChnFrame failed\n");
		break;
	case NV21_FORMAT_INDEX:
		ret = CVI_VPSS_GetChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame, -1);
		if(ret != CVI_SUCCESS){
			printf("CVI_VPSS_GetChnFrame failed\n");
			aos_msleep(1);
			return;
		}
		CVI_VPSS_GetChnAttr(uvc->video.vpss_group, uvc->video.vpss_channel, pstChnAttr);

		pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t *) pstVideoFrame->stVFrame.u64PhyAddr[0];
		data_len = pstChnAttr->u32Width;
		for (i = 0;i < ((pstChnAttr->u32Height * 3) >>1); ++i)
		{
			memcpy(media_buffer[dev_index] + buf_len, pstVideoFrame->stVFrame.pu8VirAddr[0] +
				buf_len_stride, data_len);
			buf_len += pstChnAttr->u32Width;
			buf_len_stride += pstVideoFrame->stVFrame.u32Stride[0];
		}
		pstVideoFrame->stVFrame.pu8VirAddr[0] = NULL;

		ret = CVI_VPSS_ReleaseChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame);
		if(ret != CVI_SUCCESS)
			printf("CVI_VPSS_ReleaseChnFrame failed\n");
		break;
	default:
		break;
	}

#if CONFIG_USB_BULK_UVC
	packets = uvc_payload_fill(uvc, media_buffer[dev_index], buf_len,
			uvc->packet_buffer_uvc + FRM_BUFFER_GET_IDX(uvc->rx_frm_idx) * uvc->default_frame_size,
			&data_len);
	uvc->frm_sz[FRM_BUFFER_GET_IDX(uvc->rx_frm_idx)] = data_len;
	uvc->rx_frm_idx++;
#else
	packets = uvc_payload_fill(uvc, media_buffer[dev_index], buf_len, uvc->packet_buffer_uvc, &data_len);
#endif
	buf_len = 0;
	buf_len_stride = 0;

    /* dwc2 must use this method */
	if (uvc->streaming_on && packets > 0) {
		uvc->xfer_offset = 0;
		uvc->xfer_len = data_len;
		uvc->tx_cnt++;
	#if CONFIG_USB_BULK_UVC
		aos_work_run(&uvc->uvc_workqueue, &uvc->uvc_frame_submmit);
	#else
		int check_cnt = TIMEOUT_CHECK_CNT;
		uvc->xfer_flag = true;
		usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc, uvc->max_payload_size);
		if (enum_speed == USB_SPEED_HIGH) {
			while(uvc->streaming_on && uvc->xfer_flag && check_cnt-- > 0) {
				aos_msleep(1);
			}
		}
	#endif
	}
}

static void *send_to_uvc(void *arg)
{
    uint32_t i = 0;
    int64_t dev_index = (int64_t)arg;

    while (av_session_init_flag) {
		if (rhino_atomic_get(&uvc_pause_flag)) {
			rhino_atomic_inc(&uvc_pause_done);
			while (rhino_atomic_get(&uvc_pause_done)) {
				aos_msleep(1);
			}

			for (i = 0; i < USBD_UVC_NUM; i++) {
				uvc[i].update_flag = 1;
			}

		}

		if(uvc[dev_index].streaming_on) {
			vedio_streaming_send(&uvc[dev_index], dev_index);
		} else {
			aos_msleep(1);
		}
    }

    return 0;
}

static uint8_t *uvc_descriptor = NULL;

static void uvc_desc_register_cb()
{
	uvc_destroy_descriptor(uvc_descriptor);
}

static void fix_frame_info_fps()
{
	// uint8_t format_cnt = sizeof(uvc_format_info) / sizeof(uvc_format_info[0]);

	for(uint8_t i = 0; i < USBD_UVC_NUM; i++) {
			uvc[i].cam_fps = 30;
			if (usbd_comp_get_speed() != USB_SPEED_HIGH) {
				uvc[i].cam_fps = 15;
			}
			uvc[i].interval = (10000000 / uvc[i].cam_fps);

			// for (uint8_t j = 0; j < format_cnt; j++) {
			// 	for (uint8_t k = 0; j < uvc_format_info[k].frame_cnt; k++) {
			// 		uvc_format_info[j].frames[k].fps = uvc[i].cam_fps;
			// 	}
			// }
	}
}

void uvc_get_trans_size(uint32_t *size_pre_trans, uint32_t *trans_pre_microframe, uint32_t *video_packet_size)
{
    uint32_t max_payload_size_pre_transaction = 512;
    uint32_t transaction_pre_microframe = 1; //the payload number for each URB

    if (usbd_comp_get_speed() == USB_SPEED_HIGH) {
		#if CONFIG_USB_BULK_UVC
				max_payload_size_pre_transaction = 512;
				transaction_pre_microframe = 8;//the payload number for each URB
		#else
				max_payload_size_pre_transaction = 1024;
				#if (USBD_UVC_NUM > 1)
						transaction_pre_microframe = 2;
				#else
						transaction_pre_microframe = 3;
				#endif
		#endif
    }
    else {
		#if CONFIG_USB_BULK_UVC
				max_payload_size_pre_transaction = 64;
		#else
				max_payload_size_pre_transaction = 1023;
		#endif
		transaction_pre_microframe = 1;
    }

    if (size_pre_trans) {
        *size_pre_trans = max_payload_size_pre_transaction;
    }

    if (trans_pre_microframe) {
        *trans_pre_microframe = transaction_pre_microframe;
    }

    if (video_packet_size) {
        *video_packet_size = ((max_payload_size_pre_transaction) | ((transaction_pre_microframe - 1) << 11));
    }
}

void uvc_desc_register()
{
	uint32_t desc_len;
	uint32_t size_pre_trans;
	uint32_t trans_pre_microframe;

	enum_speed = usbd_comp_get_speed();
	if (enum_speed != USB_SPEED_UNKNOWN) {
			USB_LOG_WRN("enum_speed:%d, rebuild descriptor\n", enum_speed);
				for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
					uvc[i].streaming_on = 0;
			}
	}

	fix_frame_info_fps();

	USB_LOG_INFO("multi uvc num:%u\n", USBD_UVC_NUM);

	for(uint8_t i = 0; i < USBD_UVC_NUM; i++) {
		uvc[i].ep = comp_get_available_ep(1);
		uvc[i].interface_nums = comp_get_interfaces_num();
		USB_LOG_INFO("uvc[%d].ep:%#x\n", i, uvc[i].ep);
		USB_LOG_INFO("uvc[%d].interface_nums:%d\n", i, uvc[i].interface_nums);
	}

	uvc_descriptor = uvc_build_descriptors(uvc, &desc_len, USBD_UVC_NUM);
	comp_register_descriptors(USBD_TYPE_UVC, uvc_descriptor, desc_len, 2 * USBD_UVC_NUM, uvc_desc_register_cb);

	uvc_get_trans_size(&size_pre_trans, &trans_pre_microframe, NULL);

	for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
		uvc[i].max_payload_size = size_pre_trans * trans_pre_microframe;
		usbd_add_interface(usbd_video_control_init_intf(&uvc[i].vc_intf, uvc[i].interval, MAX_FRAME_SIZE, uvc[i].max_payload_size));
		usbd_add_interface(usbd_video_stream_init_intf(&uvc[i].vs_intf, uvc[i].interval, MAX_FRAME_SIZE, uvc[i].max_payload_size));
		usbd_add_endpoint(usbd_video_init_ep(&uvc[i].video_in_ep, uvc[i].ep, NULL));
	}
	usbd_video_register_uvc_callbacks(&uvc_evt_callbks);
	usbd_video_register_video_control_callbacks(&uvc_video_control_callbks);
}

int uvc_init(void)
{
	char threadname[64] = {0};
	struct sched_param param;
	pthread_attr_t pthread_attr;
	pthread_t pthreadId[USBD_UVC_NUM] = {0};
	av_session_init_flag = CVI_TRUE;

	for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
		switch(i){
			case 0:
				uvc[i].default_frame_size = DEFAULT_FRAME_SIZE_CHNA;
				break;
			#if (USBD_UVC_NUM >= 2)
			case 1:
				uvc[i].default_frame_size = DEFAULT_FRAME_SIZE_CHNB;
				break;
			#endif
			#if (USBD_UVC_NUM >= 3)
			case 2:
				uvc[i].default_frame_size = DEFAULT_FRAME_SIZE_CHNC;
				break;
			#endif
			default:
				break;
		}
	}

	for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
	#if CONFIG_USB_BULK_UVC
		static char workqueuename[3][16] = {0};
		snprintf(workqueuename[i], sizeof(workqueuename[i]), "uvc_submmit%d", i);
		aos_workqueue_create_ext(&uvc[i].uvc_workqueue, workqueuename[i], 15, 4096);
		aos_work_init(&uvc[i].uvc_frame_submmit, usbd_video_frame_submmit, &uvc[i], 5);
		uvc[i].packet_buffer_uvc = (uint8_t *)usb_iomalloc(uvc[i].default_frame_size * FRM_BUFFER_LEN);
		memset(uvc[i].packet_buffer_uvc, 0, uvc[i].default_frame_size * FRM_BUFFER_LEN);
	#else
		uvc[i].packet_buffer_uvc = (uint8_t *)usb_iomalloc(uvc[i].default_frame_size);
		memset(uvc[i].packet_buffer_uvc, 0, uvc[i].default_frame_size);
	#endif
	}

	param.sched_priority = 31;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
	pthread_attr_setschedparam(&pthread_attr, &param);
	pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&pthread_attr, 6*1024);
	for (int64_t i = 0; i < USBD_UVC_NUM; i++) {
		pthread_create(&pthreadId[i], &pthread_attr, send_to_uvc, (void*)i);
		snprintf(threadname, sizeof(threadname), "uvc_send%ld", i);
		pthread_setname_np(pthreadId[i], threadname);
	}

	return 0;
}

int uvc_deinit(void)
{
	av_session_init_flag = CVI_FALSE;
	aos_msleep(100);

	for (int i = 0; i < USBD_UVC_NUM; i++) {
		if (uvc[i].packet_buffer_uvc) {
			usb_iofree(uvc[i].packet_buffer_uvc);
			uvc[i].packet_buffer_uvc = NULL;
		}
	}

	return 0;
}

void uvc_switch(int argc, char** argv)
{
	if(argc < 5){
		printf("Usage: %s [UVC_ID] [VENC_ID] [VPSS_GrpID] [VPSS_ChnID]\n\n", argv[0]);
		return;
	}

	rhino_atomic_inc(&uvc_pause_flag);
	while (!rhino_atomic_get(&uvc_pause_done)) {
		aos_msleep(1);
	}

	uint32_t id = atoi(argv[1]);
	uvc[id].video.venc_channel = atoi(argv[2]);
	uvc[id].video.vpss_group = atoi(argv[3]);
	uvc[id].video.vpss_channel = atoi(argv[4]);

	rhino_atomic_dec(&uvc_pause_flag);
	rhino_atomic_dec(&uvc_pause_done);
}
ALIOS_CLI_CMD_REGISTER(uvc_switch, uvc_switch, uvc_switch);


#define UVC_DBG_BUF_SIZE 1*1024
void _uvc_dbg_proc_show(int32_t argc, char **argv)
{
	int pos = 0;
	uint32_t size_pre_trans;
	uint32_t trans_pre_microframe;
	uint32_t max_payload_size;
	char *buf = NULL;
	buf = calloc(1, UVC_DBG_BUF_SIZE);
	if (!buf) {
		printf("fail to malloc\n");
		return;
	}

	uvc_get_trans_size(&size_pre_trans, &trans_pre_microframe, NULL);
	max_payload_size = size_pre_trans * trans_pre_microframe;

	pos += sprintf(buf + pos, "[UVC Debug Info]\n");
	pos += sprintf(buf + pos, "UsbdUvcChnNum\t\t:%d\t\tUsbdUvcTransType\t:%s\n",
	USBD_UVC_NUM,CONFIG_USB_BULK_UVC==1?"BULK":"ISOC");
	pos += sprintf(buf + pos, "StreamingEpMPS\t\t:%d\t\tTransferMPS\t\t:%d\n",
	size_pre_trans,max_payload_size);
	pos += sprintf(buf + pos, "[UVC Chn0 Info]\n");
	pos += sprintf(buf + pos, "UvcVencChn\t\t:%d\t\tUvcVpssGrp\t\t:%d\t\tUvcVpssChn\t\t:%d\n",
	uvc[0].video.venc_channel,uvc[0].video.vpss_group, uvc[0].video.vpss_channel);
	pos += sprintf(buf + pos, "UvcEpNum\t\t:0x%x\t\tUvcStreamingOn\t\t:%d\n",
	uvc[0].ep, uvc[0].streaming_on);
	pos += sprintf(buf + pos, "UvcXferFlag\t\t:%d\t\tTxCnt\t\t\t:%d\n",
	uvc[0].xfer_flag, uvc[0].tx_cnt);
	#if USBD_UVC_NUM > 1
	pos += sprintf(buf + pos, "[UVC Chn1 Info]\n");
	pos += sprintf(buf + pos, "UvcVencChn\t\t:%d\t\tUvcVpssGrp\t\t:%d\t\tUvcVpssChn\t\t:%d\n",
	uvc[1].video.venc_channel,uvc[1].video.vpss_group, uvc[1].video.vpss_channel);
	pos += sprintf(buf + pos, "UvcEpNum\t\t:0x%x\t\tUvcStreamingOn\t\t:%d\n",
	uvc[1].ep, uvc[1].streaming_on);
	pos += sprintf(buf + pos, "UvcXferFlag\t\t:%d\t\tTxCnt\t\t\t:%d\n",
	uvc[1].xfer_flag, uvc[1].tx_cnt);
	#endif
	#if USBD_UVC_NUM > 2
	pos += sprintf(buf + pos, "[UVC Chn2 Info]\n");
	pos += sprintf(buf + pos, "UvcVencChn\t\t:%d\t\tUvcVpssGrp\t\t:%d\t\tUvcVpssChn\t\t:%d\n",
	uvc[2].video.venc_channel,uvc[2].video.vpss_group, uvc[2].video.vpss_channel);
	pos += sprintf(buf + pos, "UvcEpNum\t\t:0x%x\t\tUvcStreamingOn\t\t:%d\n",
	uvc[2].ep, uvc[2].streaming_on);
	pos += sprintf(buf + pos, "UvcXferFlag\t\t:%d\t\tTxCnt\t\t\t:%d\n",
	uvc[2].xfer_flag, uvc[2].tx_cnt);
	#endif

	printf(buf);
	free(buf);
}

ALIOS_CLI_CMD_REGISTER(_uvc_dbg_proc_show, proc_uvc, dump uvc debug info);