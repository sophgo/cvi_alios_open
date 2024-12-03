# CVI AliOS USB Configuration

## Platform

**Board：**
CV180ZB-WEVB-0008A

**Solution：**
smart_doorbell

**Project：**
turnkey_180xb

**Tech Stack：**
cherryusb

## UVC Configuration

### 分辨率、码率的新增或修改
以修改mjepg为例，H264、H265配置类似

代码路径：components/cvi_platform/protocol/usb_devices/usbd_class/usbd_uvc/src/usbd_uvc.c

```c
static struct uvc_frame_info_st mjpeg_frame_info[] = {
    {1, 800, 480, 15, 0, 10 * 1024},
    {2, 864, 480, 30, 0, 10 * 1024},
    {3, 1920, 1080, 30, 0, 20 * 1024},
    // 新增一组分辨率为1600x1200，帧率为30fps，码率为20480kbps mjpeg格式的图像选项
    {4, 1600, 1200, 30, 0, 20 * 1024},
};
```

> [!NOTE]
> 帧率设置为30fps，实际帧率由UVC前面的模块决定(如：vpss，venc)

### 编码前的像素格式修改

代码路径：components/cvi_platform/protocol/usb_devices/usbd_class/usbd_uvc/src/usbd_uvc.c

```c
static void uvc_media_update(struct uvc_device_info* info){
    /*...*/
    switch (uvc_format_info.format_index) {
    case YUYV_FORMAT_INDEX:
        enPixelFormat = PIXEL_FORMAT_YUYV;
        break;
    case NV21_FORMAT_INDEX:
        enPixelFormat = PIXEL_FORMAT_NV21;
        break;
    default:
        // 像素格式默认为 PIXEL_FORMAT_NV21，如若需要yuv422可修改为 PIXEL_FORMAT_YUV_PLANAR_422
        enPixelFormat = PIXEL_FORMAT_NV21;
        break;
    }
    /*...*/
}

```

> [!NOTE]
> 部分上位机对MJPEG编码FFC0第十个字节位存在要求，其中，YUV420格式时为0x22，YUV422格式时为0x21

### UVC多接口多路流

yaml配置路径：components/cvi_platform/package.yaml

```yaml
  CONFIG_USBD_UVC_NUM: 1    # UVC设备Sensor的数量，最多支持三个
```

不同的UVC接口需对应不同的pipeline配置如下：

代码路径：components/cvi_platform/protocol/usb_devices/usbd_class/usbd_uvc/src/usbd_uvc.c

- 第一个UVC接口的视频流由VPSS grp0的chn0 经由VENC chn0 编码获得
- 第二个UVC接口的视频流由VPSS grp1的chn0 经由VENC chn1 编码获得

```c
static struct uvc_device_info uvc[USBD_UVC_MAX_NUM] = {
    {
        .format_info = uvc_format_info_chna,
        .formats     = ARRAY_SIZE(uvc_format_info_chna),
        /* {venc_channel,vpss_group,vpss_channel} */
        .video       = {0, 0, 0},
    },
#if (USBD_UVC_NUM >= 2)
    {
        .format_info = uvc_format_info_chnb,
        .formats     = ARRAY_SIZE(uvc_format_info_chnb),
        /* {venc_channel,vpss_group,vpss_channel} */
        .video       = {1, 1, 0},
    },
#endif
}


```
> [!NOTE]
> 根据上述多UVC接口的方法，可以使接口1输出mjpeg，接口2输出h264或h265 以实现双码流功能
> <br>
> 若需要支持h264和h265编码，不可在yaml配置中开启下述功能宏:
> <pre>
> CONFIG_DISABLE_VENC_H264: 0
> CONFIG_DISABLE_VENC_H265: 0

### UVC猫眼屏调试指南

调试前需要向猫眼屏原厂获取猫眼屏支持的相关信息，这里以如下信息为例：

| INFO                       | Value          |
|----------------------------|----------------|
| USB能力集报表大小           | 512byte |
| UVC传输模式，如：ISOC、BULK | BULK |
| 数据包的最大尺寸            | 512byte|
| 数据突发大小                | 8|
| 分辨率及图像格式            | 864*480 MJPEG(yuv422) |
| 单帧图像大小                | 480Kb(注意单位) |

**step1**

USB能力集报表是描述USB设备的功能和特性，当前USB能力集大小约为1KB，可通过注释未使用到的图像格式来缩减其大小，如下注释掉h265格式：

```c
// static struct uvc_frame_info_st h265_frame_info[] = {
//     {1, 1280, 720, 15, 0, 1 * 1024},
//     {2, 1600, 1200, 15, 0, 1.5 * 1024},
//     {3, 1920, 1080, 15, 0, 3 * 1024},
// };

static struct uvc_format_info_st uvc_format_info_chna[] = {
    {MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info), mjpeg_frame_info},
    {H264_FORMAT_INDEX, UVC_FORMAT_H264, 1, ARRAY_SIZE(h264_frame_info), h264_frame_info},
    {YUYV_FORMAT_INDEX, UVC_FORMAT_YUY2, 1, ARRAY_SIZE(yuy2_frame_info), yuy2_frame_info},
    // {H265_FORMAT_INDEX, UVC_FORMAT_H265, 1, ARRAY_SIZE(h265_frame_info), h265_frame_info},
};
```

**step2**

传输模式默认为ISOC模式，修改为BULK模式

代码路径：solutions/smart_doorbell/customization/turnkey_180xb/package.turnkey_180xb.yaml

```yaml
  CONFIG_USB_BULK_UVC: 0    # 0:ISOC, 1:BULK
```

**step3**

修改数据包的最大大小及数据突发大小

代码路径：components/cvi_platform/protocol/usb_devices/usbd_class/usbd_uvc/src/usbd_uvc.c

```c
void uvc_get_trans_size(uint32_t* size_pre_trans, uint32_t* trans_pre_microframe,uint32_t* video_packet_size)
{
    uint32_t max_payload_size_pre_transaction = 512;
    uint32_t transaction_pre_microframe       = 1;

    if (usbd_comp_get_speed() == USB_SPEED_HIGH) {
#if CONFIG_USB_BULK_UVC
        max_payload_size_pre_transaction = 512; // BULK模式下 数据包的最大大小
        transaction_pre_microframe       = 8;  // 数据突发大小
#else
    /*...*/
    }
}
```

**step4**

单帧图像大小影响码率故 480kb * 30fps(VENC是以30fps编码) --> 14400kbps，因此码率需要配置为14400kbps

> [!NOTE]
> 分辨率、图像格式、码率的配置前面部分已说明

## UAC Configuration

### 音频采样率修改

代码路径：components/cvi_platform/media/include/media_audio.h

```c
/* AUDIO Class Config */
#define AUDIO_FREQ 8000U    // 默认是8k，可修改至16k
```