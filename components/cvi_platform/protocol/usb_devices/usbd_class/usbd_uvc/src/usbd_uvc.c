#include <aos/cli.h>
#include <core/core_rv64.h>
#include <stdio.h>
#include "cvi_sys.h"
#include "cvi_venc.h"
#include "cvi_vpss.h"
#include "media_video.h"
#include "usbd_core.h"
#include "usbd_video.h"

#include "usbd_comp.h"
#include "usbd_uvc.h"
#include "usbd_uvc_descriptor.h"

#define WIDTH  (unsigned int)(1920)
#define HEIGHT (unsigned int)(1080)

#define MAX_FRAME_SIZE     (unsigned long)(WIDTH * HEIGHT * 2)
#define DEFAULT_FRAME_SIZE (unsigned long)(WIDTH * HEIGHT * 3 / 2)

/* Embed a format table inline; .formats is derived automatically from the entry count. */
#define UVC_FORMAT_TABLE(...)                                  \
    .format_info = (struct uvc_format_info_st[]){__VA_ARGS__}, \
    .formats =                                                 \
        sizeof((struct uvc_format_info_st[]){__VA_ARGS__}) / sizeof(struct uvc_format_info_st)

#define TIMEOUT_CHECK_CNT 500  // 500 * 1ms

#define MJPEG_FORMAT_INDEX (1)
#define H264_FORMAT_INDEX  (2)
#define H265_FORMAT_INDEX  (3)
#define YUYV_FORMAT_INDEX  (4)
#define NV21_FORMAT_INDEX  (5)

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

static int av_session_init_flag               = CVI_FALSE;
static atomic_t uvc_pause_flags[USBD_UVC_NUM] = {CVI_FALSE};
static atomic_t uvc_pause_dones[USBD_UVC_NUM] = {CVI_FALSE};

static uint8_t enum_speed = USB_SPEED_UNKNOWN;

static struct uvc_frame_info_st yuy2_frame_info[] = {
    {1, 800, 600, 15, 0},
    {2, 640, 360, 15, 0},
    {3, 400, 300, 15, 0},
};

static struct uvc_frame_info_st mjpeg_frame_info[] = {
    {1, 640, 480, 30, 0, 10 * 1024},
    {2, 1280, 720, 30, 0, 20 * 1024},
    {3, 1600, 1200, 30, 0, 30 * 1024},
    {4, 1920, 1080, 30, 0, 30 * 1024},
};

static struct uvc_frame_info_st h264_frame_info[] = {
    {1, 640, 480, 30, 0, 1 * 1024},
    {2, 1280, 720, 30, 0, 2 * 1024},
    {3, 1600, 1200, 30, 0, 3 * 1024},
    {4, 1920, 1080, 30, 0, 3 * 1024},
};

static struct uvc_frame_info_st h265_frame_info[] = {
    {1, 864, 480, 30, 0, 1 * 1024},
    {2, 1280, 720, 30, 0, 2 * 1024},
    {3, 1600, 1200, 30, 0, 3 * 1024},
    {4, 1920, 1080, 30, 0, 3 * 1024},
};

static struct uvc_device_info uvc[USBD_UVC_MAX_NUM] = {
    {
        UVC_FORMAT_TABLE({H265_FORMAT_INDEX, UVC_FORMAT_H265, 1, ARRAY_SIZE(h265_frame_info),
            h265_frame_info}, ),
        .format_index = 1,
        .frame_index  = 1,
        .video        = {0, 0, 0},
    },
    {
        UVC_FORMAT_TABLE({MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info),
             mjpeg_frame_info} ),
        .format_index = 1,
        .frame_index  = 1,
        .video        = {1, 0, 1},
    },
    {
        UVC_FORMAT_TABLE(
            {MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info),
             mjpeg_frame_info},
            {H264_FORMAT_INDEX, UVC_FORMAT_H264, 1, ARRAY_SIZE(h264_frame_info), h264_frame_info},
            {H265_FORMAT_INDEX, UVC_FORMAT_H265, 1, ARRAY_SIZE(h265_frame_info), h265_frame_info},
            {YUYV_FORMAT_INDEX, UVC_FORMAT_YUY2, 1, ARRAY_SIZE(yuy2_frame_info),
             yuy2_frame_info}, ),
        .format_index = 1,
        .frame_index  = 1,
        .video        = {0, 0, 2},
    }};

#if (CONFIG_USBD_UVC_OTHER == 1)
static struct uvc_device_info uvc_other[1] = {
    {
        UVC_FORMAT_TABLE({MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info),
                          mjpeg_frame_info}, ),
        .format_index = 1,
        .frame_index  = 1,
        .video        = {0, 0, 0},
    },
};
#endif

struct uvc_device_info* uvc_container_of_device_id(uint8_t device_id)
{
    return &uvc[device_id];
}

struct uvc_device_info* uvc_container_of_ep(uint8_t ep)
{
    for (int i = 0; i < USBD_UVC_NUM; i++) {
        if (ep == uvc[i].ep)
            return &uvc[i];
    }
    return NULL;
}

struct uvc_device_info* uvc_container_of_vs_intf(uint8_t intf)
{
    for (int i = 0; i < USBD_UVC_NUM; i++) {
        if (intf == uvc[i].vs_intf.intf_num)
            return &uvc[i];
    }
    return NULL;
}

static uint8_t media_buffer[USBD_UVC_NUM][DEFAULT_FRAME_SIZE] __attribute__((aligned(64)));

static CVI_S32 is_media_info_update(struct uvc_device_info* info)
{
    /* Indices are set by uvc_parse_media_info() on VS_COMMIT from the host.
     * If streaming_on fires before any COMMIT (e.g. alt-interface activated
     * without a prior probe/commit handshake), skip the update rather than
     * accessing format_info[255]. */
    if (info->format_index < 1 || info->frame_index < 1)
        return CVI_FALSE;

    PAYLOAD_TYPE_E enType;
    PIXEL_FORMAT_E enPixelFormat;
    VENC_CHN_ATTR_S stVencChnAttr, *pstVencChnAttr = &stVencChnAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr, *pstVpssChnAttr = &stVpssChnAttr;
    PARAM_VENC_CFG_S* pstVencCfg = PARAM_getVencCtx();
    CVI_U8 u8VencInitStatus =
        pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8InitStatus;

    const struct uvc_format_info_st* uvc_format_info = &info->format_info[info->format_index - 1];
    const struct uvc_frame_info_st* uvc_frame_info =
        &uvc_format_info->frames[info->frame_index - 1];

    CVI_VPSS_GetChnAttr(info->video.vpss_group, info->video.vpss_channel, pstVpssChnAttr);

    switch (uvc_format_info->format_index) {
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

    if (u8VencInitStatus == 0 && enPixelFormat == PIXEL_FORMAT_MAX)
        return CVI_TRUE;

    if ((pstVpssChnAttr->enPixelFormat != enPixelFormat)
        || (pstVpssChnAttr->stFrameRate.s32DstFrameRate != uvc_frame_info->fps)
        || (pstVpssChnAttr->u32Width != uvc_frame_info->width)
        || (pstVpssChnAttr->u32Height != uvc_frame_info->height))
        return CVI_TRUE;

    if (u8VencInitStatus == 0 && enPixelFormat != PIXEL_FORMAT_MAX)
        return CVI_FALSE;

    switch (uvc_format_info->format_index) {
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

    if (u8VencInitStatus == 0 && enType != PT_BUTT)
        return CVI_TRUE;
#if CONFIG_APP_VENC_SUPPORT
    CVI_VENC_GetChnAttr(info->video.venc_channel, pstVencChnAttr);
    if ((pstVencChnAttr->stVencAttr.enType != enType)
        || (pstVencChnAttr->stVencAttr.u32PicWidth != uvc_frame_info->width)
        || (pstVencChnAttr->stVencAttr.u32PicHeight != uvc_frame_info->height))
        return CVI_TRUE;
#endif /* (CONFIG_APP_VENC_SUPPORT) */
    UNUSED(pstVencChnAttr);
    UNUSED(stVencChnAttr);
    return CVI_FALSE;
}

static void uvc_parse_media_info(uint8_t intf, int bFormatIndex, int bFrameIndex)
{
    struct uvc_device_info* dev = uvc_container_of_vs_intf(intf);

    if (!dev) {
        aos_debug_printf("uvc_parse_media_info: unknown intf %d\n", intf);
        return;
    }

    if (bFormatIndex < 0)
        bFormatIndex = dev->formats + bFormatIndex;
    if (bFormatIndex < 1 || bFormatIndex > dev->formats) {
        aos_debug_printf("format_cnt=%d, format %d error!\r\n", dev->formats, bFormatIndex);
        return;
    }

    const struct uvc_format_info_st* format_info = &dev->format_info[bFormatIndex - 1];

    const int nframes = format_info->frame_cnt;
    if (bFrameIndex < 0)
        bFrameIndex = nframes + bFrameIndex;
    if (bFrameIndex < 1 || bFrameIndex > nframes) {
        aos_debug_printf("nframes=%d, frame %d error!\r\n", nframes, bFrameIndex);
        return;
    }

    dev->format_index = (uint8_t)bFormatIndex;
    dev->frame_index  = (uint8_t)bFrameIndex;
}

static void uvc_media_update(struct uvc_device_info* info)
{
    PAYLOAD_TYPE_E enType;
    PIXEL_FORMAT_E enPixelFormat;
    ROTATION_E rotation;
    PARAM_VENC_CFG_S* pstVencCfg = PARAM_getVencCtx();
    VPSS_CHN_ATTR_S stVpssChnAttr;
    CVI_U8 u8VencInitStatus =
        pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8InitStatus;
    VENC_RECV_PIC_PARAM_S stRecvParam = {0};

    const struct uvc_format_info_st* uvc_format_info = &info->format_info[info->format_index - 1];
    const struct uvc_frame_info_st* uvc_frame_info =
        &uvc_format_info->frames[info->frame_index - 1];

    USB_LOG_INFO("media update: ep=0x%02x fmt=%d(%dx%d@%dfps)\n", info->ep,
                 uvc_format_info->format_index, uvc_frame_info->width, uvc_frame_info->height,
                 uvc_frame_info->fps);

    switch (uvc_format_info->format_index) {
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

    switch (uvc_format_info->format_index) {
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

    if (u8VencInitStatus == 1) {
#if CONFIG_APP_VENC_SUPPORT
        if(pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.bIsSBM){
            CVI_VPSS_StopGrp(info->video.vpss_group);
        }
        MEDIA_VIDEO_VencChnDeinit(pstVencCfg, info->video.venc_channel);
        aos_debug_printf("venc chn %d deinit\n", info->video.venc_channel);
#endif
    }

#if CONFIG_UVC_CROP_BEFORE_SCALE
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CROP_INFO_S stCropInfo = {0};
    int crop_ratio              = 0;

    CVI_VPSS_GetGrpAttr(info->video.vpss_group, &stVpssGrpAttr);
    /* `*1000` is to get a more accurate value  */
    crop_ratio                      = MIN(stVpssGrpAttr.u32MaxW * 1000 / uvc_frame_info->width,
                                          stVpssGrpAttr.u32MaxH * 1000 / uvc_frame_info->height);
    stCropInfo.stCropRect.u32Width  = uvc_frame_info->width * crop_ratio / 1000;
    stCropInfo.stCropRect.u32Height = uvc_frame_info->height * crop_ratio / 1000;
    stCropInfo.stCropRect.s32X      = (stVpssGrpAttr.u32MaxW - stCropInfo.stCropRect.u32Width) / 2;
    stCropInfo.stCropRect.s32Y      = (stVpssGrpAttr.u32MaxH - stCropInfo.stCropRect.u32Height) / 2;
    stCropInfo.bEnable              = true;
    CVI_VPSS_SetChnCrop(info->video.vpss_group, info->video.vpss_channel, &stCropInfo);
#endif

    CVI_VPSS_GetChnAttr(info->video.vpss_group, info->video.vpss_channel, &stVpssChnAttr);
    stVpssChnAttr.enPixelFormat = enPixelFormat;
    CVI_VPSS_GetChnRotation(info->video.vpss_group, info->video.vpss_channel, &rotation);
    if (rotation == ROTATION_90 || rotation == ROTATION_270) {
        stVpssChnAttr.u32Width  = uvc_frame_info->height;
        stVpssChnAttr.u32Height = uvc_frame_info->width;
    } else {
        stVpssChnAttr.u32Width  = uvc_frame_info->width;
        stVpssChnAttr.u32Height = uvc_frame_info->height;
    }

    CVI_BOOL bGrpSbm = CVI_FALSE;
    PARAM_VPSS_CFG_S* pstVpssCfg = PARAM_getVpssCtx();
    for (CVI_U8 g = 0; g < pstVpssCfg->u8GrpCnt; g++) {
        PARAM_VPSS_GRP_CFG_S* pstGrp = &pstVpssCfg->pstVpssGrpCfg[g];
        if (pstGrp->VpssGrp != info->video.vpss_group)
            continue;
        for (CVI_U8 c = 0; c < pstGrp->u8ChnCnt; c++) {
            if (pstGrp->pstChnCfg[c].stVpssChnBufWrap.bEnable) {
                bGrpSbm = CVI_TRUE;
                break;
            }
        }
        break;
    }

    if (!bGrpSbm) {
        stVpssChnAttr.stFrameRate.s32SrcFrameRate = 30;
        stVpssChnAttr.stFrameRate.s32DstFrameRate = uvc_frame_info->fps;
    }
    CVI_VPSS_SetChnAttr(info->video.vpss_group, info->video.vpss_channel, &stVpssChnAttr);

    pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16Width = uvc_frame_info->width;
    pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16Height =
        uvc_frame_info->height;
    pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16EnType = enType;

    if (uvc_frame_info->bitrate) {
        pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u32BitRate =
            uvc_frame_info->bitrate;
    } else {
        pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u32BitRate =
            (enType == PT_MJPEG) ? CONFIG_UVC_MJPEG_BITRATE : CONFIG_UVC_H264_H265_BITRATE;
    }

    pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16RcMode =
        (enType == PT_MJPEG) ? VENC_RC_MODE_MJPEGCBR
                             : ((enType == PT_H264) ? VENC_RC_MODE_H264CBR : VENC_RC_MODE_H265VBR);

    if (MJPEG_FORMAT_INDEX == uvc_format_info->format_index
        || H264_FORMAT_INDEX == uvc_format_info->format_index
        || H265_FORMAT_INDEX == uvc_format_info->format_index) {
#if CONFIG_APP_VENC_SUPPORT
        MEDIA_VIDEO_VencChnInit(pstVencCfg, info->video.venc_channel);
        aos_debug_printf("venc chn %d init\n", info->video.venc_channel);
        if(pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.bIsSBM){
            CVI_VPSS_StartGrp(info->video.vpss_group);
        }

#endif
    }
    UNUSED(stRecvParam);
}

static void uvc_streaming_on(uint8_t intf, int is_on)
{
    struct uvc_device_info* uvc = uvc_container_of_vs_intf(intf);
    USB_LOG_INFO("streaming %s: intf=%d ep=0x%02x\n", is_on ? "on" : "off", intf,
                 uvc ? uvc->ep : 0xff);

    if (is_on && is_media_info_update(uvc)) {
        uvc->update_flag = 1;
    }

    uvc->header_flip  = false;
    uvc->xfer_flag    = false;
    uvc->streaming_on = is_on;
    uvc->tx_cnt       = 0;
    uvc->idr_request_flag = false;

    if (is_on && uvc->format_index >= 1) {
        const struct uvc_format_info_st* fmt = &uvc->format_info[uvc->format_index - 1];
        if (fmt->format_index == H264_FORMAT_INDEX || fmt->format_index == H265_FORMAT_INDEX) {
            uvc->idr_request_flag = true;
        }
    }
}

static void uvc_setup_class_control(struct usb_setup_packet* setup, uint8_t** data, uint32_t* len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

static void uvc_setup_class_streaming(struct usb_setup_packet* setup, uint8_t** data, uint32_t* len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

static void uvc_data_out(struct usb_setup_packet* setup, uint8_t** data, uint32_t* len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

#if CONFIG_USB_BULK_UVC
static void uvc_tx_complete(uint8_t ep, uint32_t nbytes)
{
    struct uvc_device_info* uvc = uvc_container_of_ep(ep);
    uvc->tx_frm_idx++;
    uvc->xfer_flag = false;
    uvc->tx_cnt++;
}

static void usbd_video_frame_submmit(void* args)
{
    struct uvc_device_info* uvc = (struct uvc_device_info*)args;
    uint32_t idx                = FRM_BUFFER_GET_IDX(uvc->tx_frm_idx);

    if (!uvc->streaming_on || uvc->xfer_flag) {
        return;
    }

    if (uvc->tx_frm_idx < uvc->rx_frm_idx && uvc->frm_sz[idx] > 0) {
        uvc->xfer_flag = true;
        usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc + idx * uvc->default_frame_size,
                            uvc->frm_sz[idx]);
    }
}
#else
static void uvc_tx_complete(uint8_t ep, uint32_t nbytes)
{
    struct uvc_device_info* uvc = uvc_container_of_ep(ep);
    uint32_t data_len           = 0;

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
        uvc->xfer_flag   = false;
        uvc->xfer_offset = 0;
        uvc->xfer_len    = 0;
        uvc->tx_cnt++;
    }
}
#endif

static uint32_t uvc_payload_fill(struct uvc_device_info* uvc, uint8_t* input, uint32_t input_len,
                                 uint8_t* output, uint32_t* out_len)
{
    uint32_t packets;
    uint32_t last_packet_size;
    uint32_t picture_pos     = 0;
    uint8_t uvc_header[12]   = {0x0c, 0x8d, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint32_t size_uvc_header = sizeof(uvc_header);
    uint32_t size_per_packet = uvc->max_payload_size;
    uint32_t size_payload    = size_per_packet - size_uvc_header;

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
    packets          = (input_len + size_payload - 1) / (size_payload);
    last_packet_size = input_len - ((packets - 1) * size_payload) + size_uvc_header;

    for (size_t i = 0; i < packets; i++) {
        output[size_per_packet * i]     = uvc_header[0];
        output[size_per_packet * i + 1] = uvc_header[1];
        if (i == (packets - 1)) {
            memcpy(&output[size_uvc_header + size_per_packet * i], &input[picture_pos],
                   last_packet_size - size_uvc_header);
            output[size_per_packet * i + 1] |= (1 << 1);
        } else {
            memcpy(&output[size_uvc_header + size_per_packet * i], &input[picture_pos],
                   size_payload);
            picture_pos += size_payload;
        }
    }

    *out_len = (input_len + size_uvc_header * packets);
    return packets;
}

void usbd_video_commit_set_cur(uint8_t intf, struct video_probe_and_commit_controls* commit)
{
    uvc_parse_media_info(intf, commit->bFormatIndex, commit->bFrameIndex);
    aos_debug_printf("commit format idx:%d, frame idx:%d\n", commit->bFormatIndex,
                     commit->bFrameIndex);
}

static uvc_event_callbacks_t uvc_evt_callbks = {
    .uvc_event_stream_on             = uvc_streaming_on,
    .uvc_event_setup_class_control   = uvc_setup_class_control,
    .uvc_event_setup_class_streaming = uvc_setup_class_streaming,
    .uvc_event_data_out              = uvc_data_out,
    .uvc_event_tx_complete           = uvc_tx_complete,
};

static uvc_video_control_callbacks_t uvc_video_control_callbks = {
    .uvc_vc_input_terminal_handler  = usbd_vc_input_terminal_request_handler,
    .uvc_vc_processing_unit_handler = usbd_vc_processing_unit_request_handler,
    .uvc_vc_extension_unit_handler  = usbd_vc_extension_unit_request_handler,
};

static void video_streaming_send(struct uvc_device_info* uvc, int dev_index)
{
    int i, ret = 0;
    uint32_t data_len = 0;
    uint32_t buf_len = 0, buf_len_stride = 0, packets = 0;
    VENC_STREAM_S stStream = {0}, *pstStream = &stStream;
    VENC_PACK_S* ppack;
    VIDEO_FRAME_INFO_S stVideoFrame, *pstVideoFrame = &stVideoFrame;
    VPSS_CHN_ATTR_S stChnAttr, *pstChnAttr          = &stChnAttr;

    if (uvc->update_flag) {
        uvc_media_update(uvc);
        uvc->update_flag = 0;
    }

    if (uvc->idr_request_flag) {
        uvc->idr_request_flag = false;
        if (uvc->format_index >= 1) {
            const struct uvc_format_info_st* fmt = &uvc->format_info[uvc->format_index - 1];
            if (fmt->format_index == H264_FORMAT_INDEX || fmt->format_index == H265_FORMAT_INDEX) {
                CVI_VENC_RequestIDR(uvc->video.venc_channel, CVI_TRUE);
                USB_LOG_INFO("IDR requested for chn %d\n", uvc->video.venc_channel);
            }
        }
    }

    if (enum_speed != USB_SPEED_HIGH && uvc->xfer_flag) {
        aos_msleep(4);
        return;
    }

    if (uvc->format_index < 1 || uvc->frame_index < 1) {
        aos_msleep(1);
        return;
    }

    const uint32_t format_type = uvc->format_info[uvc->format_index - 1].format_index;

    switch (format_type) {
    case H264_FORMAT_INDEX:
    case H265_FORMAT_INDEX:
    case MJPEG_FORMAT_INDEX:
#if CONFIG_APP_VENC_SUPPORT
        ret = MEDIA_VIDEO_VencGetStream(uvc->video.venc_channel, pstStream, 2000);
        if (ret != CVI_SUCCESS) {
            aos_msleep(1);
            return;
        }
        uvc->venc_frame_cnt++;

        for (i = 0; i < pstStream->u32PackCount; ++i) {
            ppack = &pstStream->pstPack[i];
            memcpy(media_buffer[dev_index] + buf_len, ppack->pu8Addr + ppack->u32Offset,
                   ppack->u32Len - ppack->u32Offset);
            buf_len += (ppack->u32Len - ppack->u32Offset);

            if (buf_len > uvc->default_frame_size) {
                printf("venc buf_len oversize\n");
                MEDIA_VIDEO_VencReleaseStream(uvc->video.venc_channel, pstStream);
                return;
            }
        }

        ret = MEDIA_VIDEO_VencReleaseStream(uvc->video.venc_channel, pstStream);
        if (ret != CVI_SUCCESS)
            printf("MEDIA_VIDEO_VencReleaseStream failed\n");
#endif /* (CONFIG_APP_VENC_SUPPORT) */
        UNUSED(ppack);
        UNUSED(pstStream);
        break;
    case YUYV_FORMAT_INDEX:
        ret =
            CVI_VPSS_GetChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame, -1);
        if (ret != CVI_SUCCESS) {
            printf("CVI_VPSS_GetChnFrame failed\n");
            aos_msleep(1);
            return;
        }
        uvc->venc_frame_cnt++;
        CVI_VPSS_GetChnAttr(uvc->video.vpss_group, uvc->video.vpss_channel, pstChnAttr);

        pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t*)pstVideoFrame->stVFrame.u64PhyAddr[0];
        data_len                              = pstChnAttr->u32Width * 2;
        for (i = 0; i < (pstChnAttr->u32Height); ++i) {
            memcpy(media_buffer[dev_index] + buf_len,
                   pstVideoFrame->stVFrame.pu8VirAddr[0] + buf_len_stride, data_len);

            buf_len += pstChnAttr->u32Width * 2;
            buf_len_stride += pstVideoFrame->stVFrame.u32Stride[0];
        }
        pstVideoFrame->stVFrame.pu8VirAddr[0] = NULL;

        ret =
            CVI_VPSS_ReleaseChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame);
        if (ret != CVI_SUCCESS)
            printf("CVI_VPSS_ReleaseChnFrame failed\n");
        break;
    case NV21_FORMAT_INDEX:
        ret =
            CVI_VPSS_GetChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame, -1);
        if (ret != CVI_SUCCESS) {
            printf("CVI_VPSS_GetChnFrame failed\n");
            aos_msleep(1);
            return;
        }
        uvc->venc_frame_cnt++;
        CVI_VPSS_GetChnAttr(uvc->video.vpss_group, uvc->video.vpss_channel, pstChnAttr);

        pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t*)pstVideoFrame->stVFrame.u64PhyAddr[0];
        data_len                              = pstChnAttr->u32Width;
        for (i = 0; i < ((pstChnAttr->u32Height * 3) >> 1); ++i) {
            memcpy(media_buffer[dev_index] + buf_len,
                   pstVideoFrame->stVFrame.pu8VirAddr[0] + buf_len_stride, data_len);
            buf_len += pstChnAttr->u32Width;
            buf_len_stride += pstVideoFrame->stVFrame.u32Stride[0];
        }
        pstVideoFrame->stVFrame.pu8VirAddr[0] = NULL;

        ret =
            CVI_VPSS_ReleaseChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame);
        if (ret != CVI_SUCCESS)
            printf("CVI_VPSS_ReleaseChnFrame failed\n");
        break;
    default:
        break;
    }

#if CONFIG_USB_BULK_UVC
    packets = uvc_payload_fill(
        uvc, media_buffer[dev_index], buf_len,
        uvc->packet_buffer_uvc + FRM_BUFFER_GET_IDX(uvc->rx_frm_idx) * uvc->default_frame_size,
        &data_len);
    uvc->frm_sz[FRM_BUFFER_GET_IDX(uvc->rx_frm_idx)] = data_len;
    uvc->rx_frm_idx++;
#else
    packets =
        uvc_payload_fill(uvc, media_buffer[dev_index], buf_len, uvc->packet_buffer_uvc, &data_len);
#endif
    buf_len        = 0;
    buf_len_stride = 0;

    /* dwc2 must use this method */
    if (uvc->streaming_on && packets > 0) {
        uvc->xfer_offset = 0;
        uvc->xfer_len    = data_len;
#if CONFIG_USB_BULK_UVC
        aos_work_run(&uvc->uvc_workqueue, &uvc->uvc_frame_submmit);
#else
        int check_cnt  = TIMEOUT_CHECK_CNT;
        uvc->xfer_flag = true;
        usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc, uvc->max_payload_size);
        if (enum_speed == USB_SPEED_HIGH) {
            while (uvc->streaming_on && uvc->xfer_flag && check_cnt-- > 0) {
                aos_msleep(1);
            }
        }
#endif
    }
}

static void* send_to_uvc(void* arg)
{
    int64_t dev_index        = (int64_t)arg;
    atomic_t* uvc_pause_flag = &uvc_pause_flags[dev_index];
    atomic_t* uvc_pause_done = &uvc_pause_dones[dev_index];
    VENC_STREAM_S stStream = {0};
    PARAM_VENC_CFG_S* pstVencCfg = PARAM_getVencCtx();
    static long long s_fps_time[USBD_UVC_MAX_NUM] = {0};
    static uint32_t s_fps_last_venc[USBD_UVC_MAX_NUM] = {0};
    static uint32_t s_fps_last_tx[USBD_UVC_MAX_NUM] = {0};
    long long now;
    uint32_t cur_venc, cur_tx;

    while (av_session_init_flag) {
        if (rhino_atomic_get(uvc_pause_flag)) {
            rhino_atomic_inc(uvc_pause_done);
            while (rhino_atomic_get(uvc_pause_done)) {
                aos_msleep(1);
            }
            uvc[dev_index].update_flag = 1;
        }

        if (uvc[dev_index].streaming_on) {
            video_streaming_send(&uvc[dev_index], dev_index);
        } else if (!uvc[dev_index].streaming_on){
            if (pstVencCfg->pstVencChnCfg[uvc[dev_index].video.venc_channel].stChnParam.bIsSBM) {
                if (MEDIA_VIDEO_VencGetStream(uvc[dev_index].video.venc_channel, &stStream, 200)
                    == CVI_SUCCESS) {
                    MEDIA_VIDEO_VencReleaseStream(uvc[dev_index].video.venc_channel, &stStream);
                }
            }

            aos_msleep(1);
        }

        now = aos_now_ms();
        if (s_fps_time[dev_index] == 0) {
            s_fps_time[dev_index] = now;
            s_fps_last_venc[dev_index] = uvc[dev_index].venc_frame_cnt;
            s_fps_last_tx[dev_index] = uvc[dev_index].tx_cnt;
        }
        if (now - s_fps_time[dev_index] >= 1000) {
            long long elapsed = now - s_fps_time[dev_index];
            cur_venc = uvc[dev_index].venc_frame_cnt;
            cur_tx = uvc[dev_index].tx_cnt;
            uvc[dev_index].venc_fps = (elapsed > 0)
                ? (uint32_t)(((cur_venc - s_fps_last_venc[dev_index]) * 1000ULL) / (uint64_t)elapsed)
                : 0;
            uvc[dev_index].tx_fps = (elapsed > 0)
                ? (uint32_t)(((cur_tx - s_fps_last_tx[dev_index]) * 1000ULL) / (uint64_t)elapsed)
                : 0;
            s_fps_time[dev_index] = now;
            s_fps_last_venc[dev_index] = cur_venc;
            s_fps_last_tx[dev_index] = cur_tx;
        }
    }

    return 0;
}

static uint8_t* uvc_descriptor = NULL;

static void uvc_desc_register_cb()
{
    uvc_destroy_descriptor(uvc_descriptor);
}
#if (CONFIG_USBD_UVC_OTHER == 1)
static uint8_t* uvc_descriptor_other = NULL;
static void uvc_desc_register_other_cb()
{
    uvc_destroy_descriptor_other(uvc_descriptor_other);
}
#endif
static void fix_frame_info_fps()
{
    for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
        uvc[i].cam_fps = 30;
        if (usbd_comp_get_speed() != USB_SPEED_HIGH) {
            uvc[i].cam_fps = 15;
        }
        uvc[i].interval = (10000000 / uvc[i].cam_fps);
    }
}

void uvc_get_trans_size(uint32_t* size_per_trans, uint32_t* trans_per_microframe,
                        uint32_t* video_packet_size)
{
    uint32_t max_payload_size_per_transaction = 512;
    uint32_t transaction_per_microframe       = 1;  // the payload number for each URB

    if (usbd_comp_get_speed() == USB_SPEED_HIGH) {
#if CONFIG_USB_BULK_UVC
        max_payload_size_per_transaction = 512;
        transaction_per_microframe       = 8;  // the payload number for each URB
#else
        max_payload_size_per_transaction = 1024;
#if (USBD_UVC_NUM > 1)
        transaction_per_microframe = 2;
#else
        transaction_per_microframe = 3;
#endif
#endif
    } else {
#if CONFIG_USB_BULK_UVC
        max_payload_size_per_transaction = 64;
#else
        max_payload_size_per_transaction = 1023;
#endif
        transaction_per_microframe = 1;
    }

    if (size_per_trans) {
        *size_per_trans = max_payload_size_per_transaction;
    }

    if (trans_per_microframe) {
        *trans_per_microframe = transaction_per_microframe;
    }

    if (video_packet_size) {
        *video_packet_size =
            ((max_payload_size_per_transaction) | ((transaction_per_microframe - 1) << 11));
    }
}

#if (CONFIG_USBD_UVC_OTHER == 1)
void uvc_get_trans_size_other(uint32_t* size_per_trans, uint32_t* trans_per_microframe,
                              uint32_t* video_packet_size)
{
    uint32_t max_payload_size_per_transaction = 512;
    uint32_t transaction_per_microframe       = 1;  // the payload number for each URB

#if CONFIG_USB_BULK_UVC
    max_payload_size_per_transaction = 64;
#else
    max_payload_size_per_transaction = 1023;
#endif

    if (size_per_trans) {
        *size_per_trans = max_payload_size_per_transaction;
    }

    if (trans_per_microframe) {
        *trans_per_microframe = transaction_per_microframe;
    }

    if (video_packet_size) {
        *video_packet_size =
            ((max_payload_size_per_transaction) | ((transaction_per_microframe - 1) << 11));
    }
}

void uvc_desc_other_register()
{
    uint32_t desc_len;
    uint32_t size_per_trans;
    uint32_t trans_per_microframe;

    for (uint8_t i = 0; i < USBD_UVC_NUM_OTHER; i++) {
        uvc_other[i].ep             = comp_get_available_ep(1);
        uvc_other[i].interface_nums = comp_get_interfaces_num_other();
        USB_LOG_INFO("uvc_other[%d].ep:%#x\n", i, uvc_other[i].ep);
        USB_LOG_INFO("uvc_other[%d].interface_nums:%d\n", i, uvc_other[i].interface_nums);
    }

    uvc_descriptor_other = uvc_build_descriptors_other(uvc_other, &desc_len, USBD_UVC_NUM_OTHER);
    comp_register_descriptors_other(USBD_TYPE_UVC, uvc_descriptor_other, desc_len,
                                    2 * USBD_UVC_NUM_OTHER, uvc_desc_register_other_cb);

    uvc_get_trans_size_other(&size_per_trans, &trans_per_microframe, NULL);

    for (uint8_t i = 0; i < USBD_UVC_NUM_OTHER; i++) {
        uvc_other[i].max_payload_size = size_per_trans * trans_per_microframe;
        usbd_add_interface_other(usbd_video_control_init_intf(&uvc_other[i].vc_intf,
                                                              uvc_other[i].interval, MAX_FRAME_SIZE,
                                                              uvc_other[i].max_payload_size));
        usbd_add_interface_other(usbd_video_stream_init_intf(&uvc_other[i].vs_intf,
                                                             uvc_other[i].interval, MAX_FRAME_SIZE,
                                                             uvc_other[i].max_payload_size));
        usbd_add_endpoint(usbd_video_init_ep(&uvc_other[i].video_in_ep, uvc_other[i].ep, NULL));
    }
}
#endif
void uvc_desc_register()
{
    uint32_t desc_len;
    uint32_t size_per_trans;
    uint32_t trans_per_microframe;

#if (CONFIG_USB_HS_FS_ADAPT == 1)
    enum_speed = usbd_comp_get_speed();
    if (enum_speed != USB_SPEED_UNKNOWN) {
        USB_LOG_WRN("enum_speed:%d, rebuild descriptor\n", enum_speed);
        for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
            uvc[i].streaming_on = 0;
        }
    }
#endif

    fix_frame_info_fps();

    USB_LOG_INFO("multi uvc num:%u\n", USBD_UVC_NUM);

    for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
        uvc[i].ep             = comp_get_available_ep(1);
        uvc[i].interface_nums = comp_get_interfaces_num();
        USB_LOG_INFO("uvc[%d].ep:%#x\n", i, uvc[i].ep);
        USB_LOG_INFO("uvc[%d].interface_nums:%d\n", i, uvc[i].interface_nums);
    }

    uvc_descriptor = uvc_build_descriptors(uvc, &desc_len, USBD_UVC_NUM);
    comp_register_descriptors(USBD_TYPE_UVC, uvc_descriptor, desc_len, 2 * USBD_UVC_NUM,
                              uvc_desc_register_cb);

    uvc_get_trans_size(&size_per_trans, &trans_per_microframe, NULL);

    for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
        uvc[i].max_payload_size = size_per_trans * trans_per_microframe;
        usbd_add_interface(usbd_video_control_init_intf(&uvc[i].vc_intf, uvc[i].interval,
                                                        MAX_FRAME_SIZE, uvc[i].max_payload_size));
        usbd_add_interface(usbd_video_stream_init_intf(&uvc[i].vs_intf, uvc[i].interval,
                                                       MAX_FRAME_SIZE, uvc[i].max_payload_size));
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
    av_session_init_flag              = CVI_TRUE;

    for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
        uvc[i].default_frame_size = DEFAULT_FRAME_SIZE;
    }

    for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
#if CONFIG_USB_BULK_UVC
        static char workqueuename[3][16] = {0};
        snprintf(workqueuename[i], sizeof(workqueuename[i]), "uvc_submmit%d", i);
        aos_workqueue_create_ext(&uvc[i].uvc_workqueue, workqueuename[i], 15, 4096);
        aos_work_init(&uvc[i].uvc_frame_submmit, usbd_video_frame_submmit, &uvc[i], 5);
        uvc[i].packet_buffer_uvc =
            (uint8_t*)usb_iomalloc(uvc[i].default_frame_size * FRM_BUFFER_LEN);
        memset(uvc[i].packet_buffer_uvc, 0, uvc[i].default_frame_size * FRM_BUFFER_LEN);
#else
        uvc[i].packet_buffer_uvc = (uint8_t*)usb_iomalloc(uvc[i].default_frame_size);
        memset(uvc[i].packet_buffer_uvc, 0, uvc[i].default_frame_size);
#endif
    }

    param.sched_priority = 45;
    pthread_attr_init(&pthread_attr);
    pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
    pthread_attr_setschedparam(&pthread_attr, &param);
    pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&pthread_attr, 6 * 1024);
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
    if (argc < 5) {
        printf("Usage: %s [UVC_ID] [VENC_ID] [VPSS_GrpID] [VPSS_ChnID]\n\n", argv[0]);
        return;
    }
    uint32_t id = atoi(argv[1]);
    if (id < 0 || id >= USBD_UVC_NUM) {
        printf("Illegal [UVC_ID], shall 0<=[UVC_ID]<=%d, however input is %d\n\n", USBD_UVC_NUM - 1,
               id);
        return;
    }
    atomic_t* uvc_pause_flag = &uvc_pause_flags[id];
    atomic_t* uvc_pause_done = &uvc_pause_dones[id];

    rhino_atomic_inc(uvc_pause_flag);
    while (!rhino_atomic_get(uvc_pause_done)) {
        aos_msleep(1);
    }

    uvc[id].video.venc_channel = atoi(argv[2]);
    uvc[id].video.vpss_group   = atoi(argv[3]);
    uvc[id].video.vpss_channel = atoi(argv[4]);

    rhino_atomic_dec(uvc_pause_flag);
    rhino_atomic_dec(uvc_pause_done);
}
ALIOS_CLI_CMD_REGISTER(uvc_switch, uvc_switch, uvc_switch);

#define UVC_DBG_BUF_SIZE 1 * 1024
void _uvc_dbg_proc_show(int32_t argc, char** argv)
{
    int pos = 0;
    uint32_t size_per_trans;
    uint32_t trans_per_microframe;
    uint32_t max_payload_size;
    char* buf = NULL;
    buf       = calloc(1, UVC_DBG_BUF_SIZE);
    if (!buf) {
        aos_debug_printf("fail to malloc\n");
        return;
    }

    uvc_get_trans_size(&size_per_trans, &trans_per_microframe, NULL);
    max_payload_size = size_per_trans * trans_per_microframe;

    pos += sprintf(buf + pos, "[UVC Debug Info]\r\n");
    pos += sprintf(buf + pos, "%-24s:%-8d  %-24s:%s\r\n",
                   "UsbdUvcChnNum", USBD_UVC_NUM,
                   "UsbdUvcTransType", CONFIG_USB_BULK_UVC == 1 ? "BULK" : "ISOC");
    pos += sprintf(buf + pos, "%-24s:%-8d  %-24s:%-8d\r\n",
                   "StreamingEpMPS", size_per_trans,
                   "TransferMPS", max_payload_size);
    pos += sprintf(buf + pos, "[UVC Chn0 Info]\r\n");
    pos += sprintf(buf + pos, "%-16s:%-6d  %-16s:%-6d  %-16s:%-6d\r\n",
                   "UvcVencChn", uvc[0].video.venc_channel,
                   "UvcVpssGrp", uvc[0].video.vpss_group,
                   "UvcVpssChn", uvc[0].video.vpss_channel);
    pos += sprintf(buf + pos, "%-16s:0x%-4x  %-16s:%-6d\r\n",
                   "UvcEpNum", uvc[0].ep,
                   "UvcStreamingOn", uvc[0].streaming_on);
    pos += sprintf(buf + pos, "%-16s:%-6d  %-16s:%-6d\r\n",
                   "UvcXferFlag", uvc[0].xfer_flag,
                   "TxCnt", uvc[0].tx_cnt);
    pos += sprintf(buf + pos, "%-16s:%-6u  %-16s:%-6u\r\n",
                   "VencFps", uvc[0].venc_fps,
                   "TxFps", uvc[0].tx_fps);
#if USBD_UVC_NUM > 1
    pos += sprintf(buf + pos, "[UVC Chn1 Info]\r\n");
    pos += sprintf(buf + pos, "%-16s:%-6d  %-16s:%-6d  %-16s:%-6d\r\n",
                   "UvcVencChn", uvc[1].video.venc_channel,
                   "UvcVpssGrp", uvc[1].video.vpss_group,
                   "UvcVpssChn", uvc[1].video.vpss_channel);
    pos += sprintf(buf + pos, "%-16s:0x%-4x  %-16s:%-6d\r\n",
                   "UvcEpNum", uvc[1].ep,
                   "UvcStreamingOn", uvc[1].streaming_on);
    pos += sprintf(buf + pos, "%-16s:%-6d  %-16s:%-6d\r\n",
                   "UvcXferFlag", uvc[1].xfer_flag,
                   "TxCnt", uvc[1].tx_cnt);
    pos += sprintf(buf + pos, "%-16s:%-6u  %-16s:%-6u\r\n",
                   "VencFps", uvc[1].venc_fps,
                   "TxFps", uvc[1].tx_fps);
#endif
#if USBD_UVC_NUM > 2
    pos += sprintf(buf + pos, "[UVC Chn2 Info]\r\n");
    pos += sprintf(buf + pos, "%-16s:%-6d  %-16s:%-6d  %-16s:%-6d\r\n",
                   "UvcVencChn", uvc[2].video.venc_channel,
                   "UvcVpssGrp", uvc[2].video.vpss_group,
                   "UvcVpssChn", uvc[2].video.vpss_channel);
    pos += sprintf(buf + pos, "%-16s:0x%-4x  %-16s:%-6d\r\n",
                   "UvcEpNum", uvc[2].ep,
                   "UvcStreamingOn", uvc[2].streaming_on);
    pos += sprintf(buf + pos, "%-16s:%-6d  %-16s:%-6d\r\n",
                   "UvcXferFlag", uvc[2].xfer_flag,
                   "TxCnt", uvc[2].tx_cnt);
    pos += sprintf(buf + pos, "%-16s:%-6u  %-16s:%-6u\r\n",
                   "VencFps", uvc[2].venc_fps,
                   "TxFps", uvc[2].tx_fps);
#endif

    aos_debug_printf(buf);
    free(buf);
}

ALIOS_CLI_CMD_REGISTER(_uvc_dbg_proc_show, proc_uvc, dump uvc debug info);
