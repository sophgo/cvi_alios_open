#include <unistd.h>
#include <pthread.h>
#include "custom_videomedia.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_buffer.h"
#include <pinctrl-mars.h>
#include "cvi_vpss.h"
#include "board_config.h"

static CVI_S32 _prepareFrame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
        VB_BLK blk;
        VB_CAL_CONFIG_S stVbCalConfig;

        if (pstVideoFrame == CVI_NULL) {
                printf("Null pointer!\n");
                return CVI_FAILURE;
        }

        COMMON_GetPicBufferConfig(stSize.u32Width, stSize.u32Height, enPixelFormat, DATA_BITWIDTH_8
                , COMPRESS_MODE_NONE, 128, &stVbCalConfig);

        memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
        pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
        pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
        pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
        pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
        pstVideoFrame->stVFrame.u32Width = stSize.u32Width;
        pstVideoFrame->stVFrame.u32Height = stSize.u32Height;
        pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
        pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
        pstVideoFrame->stVFrame.u32TimeRef = 0;
        pstVideoFrame->stVFrame.u64PTS = 0;
        pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

        blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
        if (blk == VB_INVALID_HANDLE) {
                printf("Can't acquire vb block\n");
                return CVI_FAILURE;
        }

        pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
        pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
        pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
        pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
        pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
                + ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
        if (stVbCalConfig.plane_num == 3) {
                pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
                pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
                pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
                        + ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
        }

        return CVI_SUCCESS;
}


void *APP_CustomMedia_Proces(void *args)
{
    SIZE_S stSize = {
        .u32Width = PANEL_WIDTH,
	    .u32Height = PANEL_HEIGHT,
    };
    VIDEO_FRAME_INFO_S stVideoFrame;
    _prepareFrame(stSize, PIXEL_FORMAT_RGB_888_PLANAR, &stVideoFrame);
    while(1) {
        if(CVI_VPSS_SendFrame(CONFIG_MEDIA_VPSS_GRP, &stVideoFrame, 2000) != CVI_SUCCESS) {
            printf("CVI_VPSS_SendFrame err \n");
            continue;
        }
        usleep(30*1000);//30FPS
    }
}

int APP_CustomEventMedia()
{
    pthread_t pthreadId;
    pthread_create(&pthreadId, NULL, APP_CustomMedia_Proces, NULL);
    pthread_setname_np(pthreadId, "CustomMedia");
    return 0;
}