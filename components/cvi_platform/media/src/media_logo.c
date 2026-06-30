#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include <string.h>

#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vdec.h"
#include "cvi_vo.h"
#include "cvi_math.h"
#include "cvi_buffer.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_vdec.h"
#include "cvi_board_memmap.h"
#include "cvi_param.h"
#include "media_logo.h"

#define READ_LEN_MAX (256 * 1024)

typedef struct _VDEC_ATTR {
    PAYLOAD_TYPE_E enType;
    PIXEL_FORMAT_E enPixelFormat;
    VIDEO_MODE_E   enMode;
    CVI_U32 u32Width;
    CVI_U32 u32Height;
    CVI_U32 u32FrameBufCnt;
    CVI_U32 u32DisplayFrameNum;
    CVI_U32 u32VdecChn;
    CVI_U32 vdecFrameNum;
} VDEC_ATTR;

static VDEC_ATTR vdec_attr_logo = {
    .u32VdecChn = 0,
    .enType = PT_JPEG,
    .enPixelFormat = PIXEL_FORMAT_NV21,
    .enMode = VIDEO_MODE_FRAME,
    .u32Width = 720,
    .u32Height = 1280,
    .u32FrameBufCnt = 1,
    .u32DisplayFrameNum = 1,
    .vdecFrameNum = 1,
};

static CVI_S32 _vdec_init(VDEC_ATTR vdecAttr)
{
    VDEC_CHN_ATTR_S stAttr = {0};
    VDEC_CHN_PARAM_S stParam = {0};
    VDEC_MOD_PARAM_S stModParam = {0};
    CVI_S32 s32Ret;

    s32Ret = CVI_VDEC_GetModParam(&stModParam);
    if (s32Ret != CVI_SUCCESS) {
        printf("GetModParam fail, ret:0x%x\n", s32Ret);
    }

    stModParam.enVdecVBSource = VB_SOURCE_COMMON;
    s32Ret = CVI_VDEC_SetModParam(&stModParam);
    if (s32Ret != CVI_SUCCESS) {
        printf("SetModParam fail, ret:0x%x\n", s32Ret);
    }

    stAttr.enType = vdecAttr.enType;
    stAttr.enMode = vdecAttr.enMode;
    stAttr.u32PicWidth = vdecAttr.u32Width;
    stAttr.u32PicHeight = vdecAttr.u32Height;
    stAttr.u32StreamBufSize = ALIGN(vdecAttr.u32Width * vdecAttr.u32Height, 0x4000);
    stAttr.u32FrameBufSize = VDEC_GetPicBufferSize(
                vdecAttr.enType, vdecAttr.u32Width, vdecAttr.u32Height,
                vdecAttr.enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
    stAttr.u32FrameBufCnt = vdecAttr.u32FrameBufCnt;

    s32Ret = CVI_VDEC_CreateChn(vdecAttr.u32VdecChn, &stAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("CreateChn fail, ret:0x%x\n", s32Ret);
    }

    s32Ret = CVI_VDEC_GetChnAttr(vdecAttr.u32VdecChn, &stAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("GetChnAttr fail, ret:0x%x\n", s32Ret);
    }

    s32Ret = CVI_VDEC_SetChnAttr(vdecAttr.u32VdecChn, &stAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("SetChnAttr fail, ret:0x%x\n", s32Ret);
    }

    s32Ret = CVI_VDEC_GetChnParam(vdecAttr.u32VdecChn, &stParam);
    if (s32Ret != CVI_SUCCESS) {
        printf("GetChnParam fail, ret:0x%x\n", s32Ret);
    }

    stParam.enPixelFormat = vdecAttr.enPixelFormat;
    s32Ret = CVI_VDEC_SetChnParam(vdecAttr.u32VdecChn, &stParam);
    if (s32Ret != CVI_SUCCESS) {
        printf("SetChnParam fail, ret:0x%x\n", s32Ret);
    }

    s32Ret = CVI_VDEC_StartRecvStream(vdecAttr.u32VdecChn);
    if (s32Ret != CVI_SUCCESS) {
        printf("StartRecvStream fail, ret:0x%x\n", s32Ret);
    }

    return CVI_SUCCESS;
}

static CVI_S32 _vdec_deinit(VDEC_ATTR vdecAttr)
{
    CVI_VDEC_StopRecvStream(vdecAttr.u32VdecChn);
    CVI_VDEC_ResetChn(vdecAttr.u32VdecChn);
    CVI_VDEC_DestroyChn(vdecAttr.u32VdecChn);

    return CVI_SUCCESS;
}

int CVI_Media_Vdec_Logo(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VDEC_STREAM_S pstStream = {0};
    VIDEO_FRAME_INFO_S pstFrameInfo = {0};
    CVI_U8 *pu8Buf = (CVI_U8 *)CVIMMAP_BOOTLOGO_ADDR;
    CVI_BOOL bFindStart = CVI_FALSE;
    CVI_U32 u32Len = 0, u32Start = 0;
    CVI_U32 s32ReadLen = 0;
    int i = 0;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr = {0};

    s32Ret = CVI_VB_Init();
    if (s32Ret != CVI_SUCCESS) {
        printf("[LOGO] vb init error, ret=0x%x\n", s32Ret);
        return s32Ret;
    }

    // Dynamically parse JPEG header to get width and height (from first image)
    for (CVI_S32 j = 0; j < READ_LEN_MAX - 8; j++) {
        // Find SOF0 marker: 0xFF 0xC0
        if (pu8Buf[j] == 0xFF && pu8Buf[j + 1] == 0xC0 &&
            pu8Buf[j + 2] == 0x00 && pu8Buf[j + 3] == 0x11 &&
            pu8Buf[j + 4] == 0x08) {
            vdec_attr_logo.u32Height = pu8Buf[j + 5] * 256 + pu8Buf[j + 6];
            vdec_attr_logo.u32Width = pu8Buf[j + 7] * 256 + pu8Buf[j + 8];
            break;
        }
    }

    // Loop to play multiple images (animation support)
    while (1) {
        // Initialize VDEC for each frame
        s32Ret = _vdec_init(vdec_attr_logo);
        if (s32Ret != CVI_SUCCESS) {
            printf("[LOGO] vdec init error, ret=0x%x\n", s32Ret);
            CVI_VB_Exit();
            return s32Ret;
        }

        // Find JPEG start marker: 0xFF 0xD8 (from current position i)
        bFindStart = CVI_FALSE;
        for (; i < READ_LEN_MAX - 1; i++) {
            if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD8) {
                u32Start = i;
                bFindStart = CVI_TRUE;
                i = i + 2;
                break;
            }
        }

        // If no JPEG start marker found, exit loop
        if (bFindStart == CVI_FALSE) {
            _vdec_deinit(vdec_attr_logo);
            break;
        }

        // Skip APP markers
        for (; i < READ_LEN_MAX - 3; i++) {
            if ((pu8Buf[i] == 0xFF) && (pu8Buf[i + 1] & 0xF0) == 0xE0) {
                u32Len = (pu8Buf[i + 2] << 8) + pu8Buf[i + 3];
                i += 1 + u32Len;
            } else {
                break;
            }
        }

        // Find JPEG end marker: 0xFF 0xD9
        for (; i < READ_LEN_MAX - 1; i++) {
            if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD9) {
                break;
            }
        }
        s32ReadLen = i + 2 - u32Start;
        i += 2;  // Move past the end marker for next iteration

        // Send JPEG stream to VDEC
        pstStream.pu8Addr = pu8Buf + u32Start;
        pstStream.u32Len = s32ReadLen;
        pstStream.bEndOfFrame = 1;
        pstStream.bEndOfStream = 1;

        s32Ret = CVI_VDEC_SendStream(0, &pstStream, 1000);
        if (s32Ret != CVI_SUCCESS) {
            printf("[LOGO] Send JPEG stream error, ret=0x%x\n", s32Ret);
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

        // Get decoded frame
        s32Ret = CVI_VDEC_GetFrame(0, &pstFrameInfo, 1000);
        if (s32Ret != CVI_SUCCESS) {
            printf("[LOGO] CVI_VDEC_GetFrame error, ret=0x%x\n", s32Ret);
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

        // Get VO layer attribute
        s32Ret = CVI_VO_GetVideoLayerAttr(0, &stLayerAttr);
        if (s32Ret != CVI_SUCCESS) {
            printf("[LOGO] CVI_VO_GetVideoLayerAttr error, ret=0x%x\n", s32Ret);
            CVI_VDEC_ReleaseFrame(0, &pstFrameInfo);
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

        // Center the logo on screen
        if (pstFrameInfo.stVFrame.u32Width > stLayerAttr.stImageSize.u32Width) {
            stLayerAttr.stDispRect.s32X = 0;
        } else {
            stLayerAttr.stDispRect.s32X = (stLayerAttr.stImageSize.u32Width - pstFrameInfo.stVFrame.u32Width) / 2;
        }

        if (pstFrameInfo.stVFrame.u32Height > stLayerAttr.stImageSize.u32Height) {
            stLayerAttr.stDispRect.s32Y = 0;
        } else {
            stLayerAttr.stDispRect.s32Y = (stLayerAttr.stImageSize.u32Height - pstFrameInfo.stVFrame.u32Height) / 2;
        }

        stLayerAttr.stDispRect.u32Width = pstFrameInfo.stVFrame.u32Width;
        stLayerAttr.stDispRect.u32Height = pstFrameInfo.stVFrame.u32Height;
        stLayerAttr.enPixFormat = pstFrameInfo.stVFrame.enPixelFormat;

        s32Ret = CVI_VO_SetVideoLayerAttr(0, &stLayerAttr);
        if (s32Ret != CVI_SUCCESS) {
            printf("[LOGO] CVI_VO_SetVideoLayerAttr error, ret=0x%x\n", s32Ret);
            CVI_VDEC_ReleaseFrame(0, &pstFrameInfo);
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

        // Wait for VO ready
        usleep(80 * 1000);

        // Send frame to VO via ION
        s32Ret = CVI_VO_SendLogoFromIon(0, 0, &pstFrameInfo, 1000);
        if (s32Ret != CVI_SUCCESS) {
            printf("[LOGO] CVI_VO_SendLogoFromIon error, ret=0x%x\n", s32Ret);
            CVI_VDEC_ReleaseFrame(0, &pstFrameInfo);
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

        // Additional delay to ensure display update
        usleep(100 * 1000);

        // Release frame and deinit VDEC for this frame
        CVI_VDEC_ReleaseFrame(0, &pstFrameInfo);
        _vdec_deinit(vdec_attr_logo);
    }

    CVI_VB_Exit();

    return CVI_SUCCESS;
}
