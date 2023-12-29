#include <stdio.h>
#include <aos/kernel.h>
#include <k_atomic.h>
#include <aos/cli.h>
#include "usbd_core.h"
#include "usbd_video.h"
#include "media_video.h"
// #include "video.h"
#include "cvi_venc.h"
#include "cvi_vpss.h"
#include "cvi_sys.h"
#include "av_comp_descriptor.h"
#include "uac.h"
#include "uac_descriptor.h"
#include <core/core_rv64.h>
#include "multi_av_composite.h"
//#include "pic_data.h"

#define VIDEO_IN_EP 0x81

#define WIDTH  (unsigned int)(1920)
#define HEIGHT (unsigned int)(1080)

#define CAM_FPS        (30)
#define INTERVAL       (unsigned long)(10000000 / CAM_FPS)
#define MAX_FRAME_SIZE (unsigned long)(WIDTH * HEIGHT * 2)
#define DEFAULT_FRAME_SIZE (unsigned long)(WIDTH * HEIGHT * 3 / 2)

#define MJPEG_FORMAT_INDEX  (1)
#define H264_FORMAT_INDEX   (2)
#define YUYV_FORMAT_INDEX   (3)
#define NV21_FORMAT_INDEX   (4)



static int av_session_init_flag = CVI_FALSE;
static aos_event_t _gslUvcEvent;

static atomic_t uvc_pause_flag = CVI_FALSE;
static atomic_t uvc_pause_done = CVI_FALSE;

CVI_S32 is_media_info_update(struct uvc_device_info *info);
void uvc_parse_media_info(uint8_t bFormatIndex, uint8_t bFrameIndex);
void uvc_media_update(struct uvc_device_info *info);

static struct usbd_endpoint audio_in_ep = {
    .ep_cb = usbd_audio_in_callback,
    .ep_addr = AUDIO_IN_EP
};

static struct usbd_endpoint audio_out_ep = {
    .ep_cb = usbd_audio_out_callback,
    .ep_addr = AUDIO_OUT_EP
};

static struct uvc_frame_info_st yuy2_frame_info[] = {
    {1, 800, 600, 15, 0},
    {2, 640, 360, 15, 0},
    {3, 400, 300, 15, 0},
    {5, 480, 320, 15, 0},
    {6, 480, 360, 15, 0},
    {7, 1280, 720, 15, 0},
};

static struct uvc_frame_info_st mjpeg_frame_info[] = {
    {1, 240, 320, 30, 0},
    {2, 320, 240, 30, 0},
    {3, 480, 320, 30, 0},
    {4, 800, 480, 30, 0},
    {5, 864, 480, 30, 0},
    {6, 1280, 720, 30, 0},
    {7, 1920, 1080, 30, 0},
	{8, 1600, 1200, 30, 0},
};

static struct uvc_frame_info_st h264_frame_info[] = {
    {1, 800, 600, 30, 0},
    {2, 1280, 720, 30, 0},
    {3, 640, 480, 30, 0},
    {4, 400, 300, 30, 0},
    {5, 1920, 1080, 30, 0},
};

//  static struct uvc_frame_info_st nv21_frame_info[] = {
//      {1, 800, 600, 15, 0},
//      {2, 1280, 720, 15, 0},
//      {3, 640, 480, 15, 0},
//  };

static struct uvc_format_info_st uvc_format_info[] = {
    {MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info), mjpeg_frame_info},
    {H264_FORMAT_INDEX, UVC_FORMAT_H264, 1, ARRAY_SIZE(h264_frame_info), h264_frame_info},
    {YUYV_FORMAT_INDEX, UVC_FORMAT_YUY2, 1, ARRAY_SIZE(yuy2_frame_info), yuy2_frame_info},
    // {NV21_FORMAT_INDEX, UVC_FORMAT_NV21, 1, ARRAY_SIZE(nv21_frame_info), nv21_frame_info},
};

static struct uvc_device_info uvc[CONFIG_MULTI_AV_COMP_UVC_NUM] = {
    {
        .ep = 0x81,
        .format_info = uvc_format_info,
        .formats = ARRAY_SIZE(uvc_format_info),
        .video = {0, 0, 0},
    },
#if CONFIG_MULTI_AV_COMP_UVC_NUM > 1
    {
        .ep = 0x82,
        .format_info = uvc_format_info,
        .formats = ARRAY_SIZE(uvc_format_info),
        .video = {1, 1, 0},
    },
#endif
#if CONFIG_MULTI_AV_COMP_UVC_NUM > 2
    {
        .ep = 0x83,
        .format_info = uvc_format_info,
        .formats = ARRAY_SIZE(uvc_format_info),
        .video = {1, 1, 0},
    }
#endif
};

CVI_S32 is_media_info_update(struct uvc_device_info *info){
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

void uvc_parse_media_info(uint8_t bFormatIndex, uint8_t bFrameIndex)
{
    const struct uvc_format_info_st *format_info;
    const int uvcout_format_cnt = ARRAY_SIZE(uvc_format_info);

    if (bFormatIndex < 0)
        bFormatIndex = uvcout_format_cnt + bFormatIndex;
    if (bFormatIndex < 0 || bFormatIndex > uvcout_format_cnt)
    {
        aos_debug_printf("format_cnt =%d, format %d error!\r\n", uvcout_format_cnt, bFormatIndex);
        return;
    }

    format_info = &uvc_format_info[bFormatIndex - 1];

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

void uvc_media_update(struct uvc_device_info *info){
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
	VPSS_CHN_ATTR_S stVpssChnAttr;
	CVI_U8 u8VencInitStatus = pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8InitStatus;


	struct uvc_format_info_st uvc_format_info;
	struct uvc_frame_info_st uvc_frame_info;
	uvc_get_video_format_info(&uvc_format_info);
	uvc_get_video_frame_info(&uvc_frame_info);

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
	default:
		enType = PT_MJPEG;
		break;
	}

	if(u8VencInitStatus == 1)
		MEDIA_VIDEO_VencDeInit(pstVencCfg);

	CVI_VPSS_GetChnAttr(info->video.vpss_group, info->video.vpss_channel, &stVpssChnAttr);
	stVpssChnAttr.enPixelFormat = enPixelFormat;
	stVpssChnAttr.u32Width = uvc_frame_info.width;
	stVpssChnAttr.u32Height = uvc_frame_info.height;
	CVI_VPSS_SetChnAttr(info->video.vpss_group, info->video.vpss_channel, &stVpssChnAttr);

	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16Width = uvc_frame_info.width;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16Height = uvc_frame_info.height;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16EnType = enType;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16BitRate = (enType == PT_MJPEG)?20480:2048;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16RcMode = (enType == PT_MJPEG)?VENC_RC_MODE_MJPEGCBR:VENC_RC_MODE_H264CBR;

	if(MJPEG_FORMAT_INDEX == uvc_format_info.format_index || H264_FORMAT_INDEX == uvc_format_info.format_index)
		MEDIA_VIDEO_VencInit(pstVencCfg);
}

struct uvc_device_info *uvc_container_of_ep(uint8_t ep)
{
    for (int i = 0; i < CONFIG_MULTI_AV_COMP_UVC_NUM; i++) {
        if(ep == uvc[i].ep)
            return &uvc[i];
    }
    return NULL;
}

struct uvc_device_info *uvc_container_of_vs_intf(uint8_t intf)
{
    for (int i = 0; i < CONFIG_MULTI_AV_COMP_UVC_NUM; i++) {
        if(intf == uvc[i].vs_intf.intf_num)
            return &uvc[i];
    }
    return NULL;
}

void uvc_streaming_on(uint8_t intf, int is_on) {
    struct uvc_device_info *uvc = uvc_container_of_vs_intf(intf);
	USB_LOG_INFO("streaming %s\n", is_on ? "on" : "off");

	if(is_on && is_media_info_update(uvc)) {
		uvc->update_flag = 1;
	}

	uvc->header_flip = false;
	uvc->xfer_flag = false;
	uvc->streaming_on = is_on;
}

void usbd_configure_done_callback(void)
{
    /* no out ep, so do nothing */
}

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
		data_len = uvc->xfer_len < MAX_PAYLOAD_SIZE ? uvc->xfer_len : MAX_PAYLOAD_SIZE;
		usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc + uvc->xfer_offset, data_len);
	} else {
		uvc->xfer_flag = false;
		uvc->xfer_offset = 0;
		uvc->xfer_len = 0;
	}
}

static uint32_t uvc_payload_fill(struct uvc_device_info *uvc, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *out_len)
{
    uint32_t packets;
    uint32_t last_packet_size;
    uint32_t picture_pos = 0;
    uint8_t uvc_header[12] = { 0x0c, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint32_t size_uvc_header = sizeof(uvc_header);
    uint32_t size_per_packet = MAX_PAYLOAD_SIZE;
    uint32_t size_payload = size_per_packet - size_uvc_header;

    if (size_payload > 10240) {
        USB_LOG_ERR("the size of payload is too long!!!!\n");
    }

	if (input_len + size_uvc_header > DEFAULT_FRAME_SIZE) {
		USB_LOG_ERR("input_len + size_uvc_header (%u) > DEFAULT_FRAME_SIZE (%u)\n",
			input_len + size_uvc_header, DEFAULT_FRAME_SIZE);
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


void usbd_video_commit_set_cur(struct video_probe_and_commit_controls *commit)
{
	uvc_parse_media_info(commit->bFormatIndex, commit->bFrameIndex);
    aos_debug_printf("commit format idx:%d, frame idx:%d\n", commit->bFormatIndex, commit->bFrameIndex);
}

static uvc_event_callbacks_t uvc_evt_callbks = {
    .uvc_event_stream_on = uvc_streaming_on,
    .uvc_event_setup_class_control = uvc_setup_class_control,
    .uvc_event_setup_class_streaming = uvc_setup_class_streaming,
    .uvc_event_data_out = uvc_data_out,
    .uvc_event_tx_complete = uvc_tx_complete,
};
static uint8_t media_buffer[DEFAULT_FRAME_SIZE] __attribute__((aligned(64)));

static void vedio_streaming_send(struct uvc_device_info *uvc)
{
	int i, ret = 0;
	uint32_t data_len = 0;
	uint32_t buf_len = 0, buf_len_stride = 0, packets = 0;
	struct uvc_format_info_st uvc_format_info;
    VENC_STREAM_S stStream = {0},*pstStream= &stStream;
	VENC_PACK_S *ppack;
	VIDEO_FRAME_INFO_S stVideoFrame, *pstVideoFrame=&stVideoFrame;
	VPSS_CHN_ATTR_S stChnAttr,*pstChnAttr = &stChnAttr;

	if(uvc->update_flag){
		uvc_media_update(uvc);
		uvc->update_flag = 0;
	}

	uvc_get_video_format_info(&uvc_format_info);
	switch(uvc_format_info.format_index) {
	case H264_FORMAT_INDEX:
	case MJPEG_FORMAT_INDEX:
		ret = MEDIA_VIDEO_VencGetStream(uvc->video.venc_channel, pstStream, 2000);
		if(ret != CVI_SUCCESS){
			aos_msleep(1);
			return;
		}

		for (i = 0; i < pstStream->u32PackCount; ++i)
		{
			ppack = &pstStream->pstPack[i];
			memcpy(media_buffer + buf_len,
					ppack->pu8Addr + ppack->u32Offset,
					ppack->u32Len - ppack->u32Offset);
			buf_len += (ppack->u32Len - ppack->u32Offset);

			if (buf_len > DEFAULT_FRAME_SIZE) {
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
			//printf("CVI_VPSS_GetChnFrame failed\n");
			aos_msleep(1);
			return;
		}
		CVI_VPSS_GetChnAttr(uvc->video.vpss_group, uvc->video.vpss_channel, pstChnAttr);

		pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t *)pstVideoFrame->stVFrame.u64PhyAddr[0];
		data_len = pstChnAttr->u32Width * 2;
		for (i = 0;i < (pstChnAttr->u32Height); ++i)
		{
			memcpy(media_buffer + buf_len, pstVideoFrame->stVFrame.pu8VirAddr[0] +
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
//			printf("CVI_VPSS_GetChnFrame failed\n");
			aos_msleep(1);
			return;
		}
		CVI_VPSS_GetChnAttr(uvc->video.vpss_group, uvc->video.vpss_channel, pstChnAttr);

		pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t *) pstVideoFrame->stVFrame.u64PhyAddr[0];
		data_len = pstChnAttr->u32Width;
		for (i = 0;i < ((pstChnAttr->u32Height * 3) >>1); ++i)
		{
			memcpy(media_buffer + buf_len, pstVideoFrame->stVFrame.pu8VirAddr[0] +
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

	packets = uvc_payload_fill(uvc, media_buffer, buf_len, uvc->packet_buffer_uvc, &data_len);
	buf_len = 0;
	buf_len_stride = 0;

    /* dwc2 must use this method */
	if (uvc->streaming_on && packets > 0) {
		uvc->xfer_offset = 0;
		uvc->xfer_flag = true;
		uvc->xfer_len = data_len;
		usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc, MAX_PAYLOAD_SIZE);
		while(uvc->streaming_on && uvc->xfer_flag) {
			aos_task_yield();
		}
	}
}

static void *send_to_uvc()
{
    uint32_t i = 0;

    while (av_session_init_flag) {
		if (rhino_atomic_get(&uvc_pause_flag)) {
			rhino_atomic_inc(&uvc_pause_done);
			while (rhino_atomic_get(&uvc_pause_done)) {
				aos_msleep(1);
			}

			for (i = 0; i < CONFIG_MULTI_AV_COMP_UVC_NUM; i++) {
				uvc[i].update_flag = 1;
			}

		}

		for (i = 0; i < CONFIG_MULTI_AV_COMP_UVC_NUM; i++) {
			if(uvc[i].streaming_on) {
				vedio_streaming_send(&uvc[i]);
			}
			else {
				aos_msleep(1);
			}
		}
    }

    return 0;
}

static struct usbd_interface uac_intf0;
static struct usbd_interface uac_intf1;
static struct usbd_interface uac_intf2;
static uint8_t *av_comp_descriptor = NULL;

extern uint8_t *multi_av_comp_build_descriptors(struct uvc_device_info *uvc);
void usb_av_comp_init()
{
    av_comp_descriptor = multi_av_comp_build_descriptors((struct uvc_device_info *)uvc);
    usbd_desc_register(av_comp_descriptor);

	printf("multi uvc num:%u\n", CONFIG_MULTI_AV_COMP_UVC_NUM);
    for (int i = 0; i < CONFIG_MULTI_AV_COMP_UVC_NUM; i++) {
        usbd_add_interface(usbd_video_control_init_intf(&uvc[i].vc_intf, INTERVAL, MAX_FRAME_SIZE, MAX_PAYLOAD_SIZE));
        usbd_add_interface(usbd_video_stream_init_intf(&uvc[i].vs_intf, INTERVAL, MAX_FRAME_SIZE, MAX_PAYLOAD_SIZE));
        usbd_add_endpoint(usbd_video_init_ep(&uvc[i].video_in_ep, uvc[i].ep, NULL));
		uvc[i].packet_buffer_uvc = (uint8_t *)usb_iomalloc(DEFAULT_FRAME_SIZE);
    }
    usbd_video_register_uvc_callbacks(&uvc_evt_callbks);

    usbd_add_interface(usbd_audio_init_intf(&uac_intf0));
    usbd_add_interface(usbd_audio_init_intf(&uac_intf1));
    usbd_add_interface(usbd_audio_init_intf(&uac_intf2));
    usbd_add_endpoint(&audio_in_ep);
    usbd_add_endpoint(&audio_out_ep);

    usbd_audio_add_entity(0x02, AUDIO_CONTROL_FEATURE_UNIT);
    usbd_audio_add_entity(0x05, AUDIO_CONTROL_FEATURE_UNIT);

    usbd_initialize();
}

int MEDIA_AV_Init()
{
    char threadname[64] = {0};
	struct sched_param param;
	pthread_attr_t pthread_attr;
	pthread_t pthreadId = 0;

	// csi_dcache_clean_invalid();
	// csi_dcache_disable();
	MEDIA_UAC_Init();

	usb_av_comp_init();

	//packet_buffer_uvc = (uint8_t *)usb_iomalloc(DEFAULT_FRAME_SIZE);

	// Wait until configured
	while (!usb_device_is_configured()) {
		aos_msleep(100);
	}
	av_session_init_flag = CVI_TRUE;
	aos_event_new(&_gslUvcEvent, 0);
	param.sched_priority = 31;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
	pthread_attr_setschedparam(&pthread_attr, &param);
	pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&pthread_attr, 6*1024);
	pthread_create(&pthreadId,&pthread_attr,send_to_uvc,NULL);
	snprintf(threadname,sizeof(threadname),"uvc_send%d",0);
	pthread_setname_np(pthreadId, threadname);

	return 0;
}

int MEDIA_AV_DeInit()
{
	av_session_init_flag = CVI_FALSE;
	aos_msleep(100);
	usbd_deinitialize();
	aos_event_free(&_gslUvcEvent);
	MEDIA_UAC_deInit();
    if (av_comp_descriptor) {
        av_comp_destroy_descriptors(av_comp_descriptor);
    }

    for (int i = 0; i < CONFIG_MULTI_AV_COMP_UVC_NUM; i++) {
	if (uvc[i].packet_buffer_uvc) {
		usb_iofree(uvc[i].packet_buffer_uvc);
		uvc[i].packet_buffer_uvc = NULL;
	}
	}

	return 0;
}

void av_comp_app_init()
{
	MEDIA_AV_Init();
}

void av_comp_app_deinit()
{
	MEDIA_AV_DeInit();
}

ALIOS_CLI_CMD_REGISTER(av_comp_app_init, av_comp_app_init, av_comp_app_init);
ALIOS_CLI_CMD_REGISTER(av_comp_app_deinit, av_comp_app_deinit, av_comp_app_deinit);

void av_comp_app_switch(int argc, char** argv)
{
    uint32_t id = 0;
	if(argc < 5){
		printf("Usage: %s [UVC_ID] [VENC_ID] [VPSS_GrpID] [VPSS_ChnID]\n\n", argv[0]);
		return;
	}

	rhino_atomic_inc(&uvc_pause_flag);
	while (!rhino_atomic_get(&uvc_pause_done)) {
		aos_msleep(1);
	}

    id = atoi(argv[1]);
	uvc[id].video.venc_channel = atoi(argv[2]);
	uvc[id].video.vpss_group = atoi(argv[3]);
	uvc[id].video.vpss_channel = atoi(argv[4]);

	rhino_atomic_dec(&uvc_pause_flag);
	rhino_atomic_dec(&uvc_pause_done);
}
void av_comp_app_get_info(int argc, char** argv)
{
	printf("UVC-0: UVC_VENC_CHN:%d, UVC_VPSS_GRP:%d, UVC_VPSS_CHN:%d\n",
	        uvc[0].video.venc_channel, uvc[0].video.vpss_group, uvc[0].video.vpss_channel);
#if CONFIG_MULTI_AV_COMP_UVC_NUM > 1
	printf("UVC-1: UVC_VENC_CHN:%d, UVC_VPSS_GRP:%d, UVC_VPSS_CHN:%d\n",
	        uvc[1].video.venc_channel, uvc[1].video.vpss_group, uvc[1].video.vpss_channel);
#endif
#if CONFIG_MULTI_AV_COMP_UVC_NUM > 2
	printf("UVC-2: UVC_VENC_CHN:%d, UVC_VPSS_GRP:%d, UVC_VPSS_CHN:%d\n",
	        uvc[2].video.venc_channel, uvc[2].video.vpss_group, uvc[2].video.vpss_channel);
#endif
}

ALIOS_CLI_CMD_REGISTER(av_comp_app_switch, av_comp_app_switch, av_comp_app_switch);
ALIOS_CLI_CMD_REGISTER(av_comp_app_get_info, av_comp_app_get_info, av_comp_app_get_info);
