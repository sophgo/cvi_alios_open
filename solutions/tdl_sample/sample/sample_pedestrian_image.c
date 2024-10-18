#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "aos/cli.h"
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_media.h"

#define WIDTH 1280
#define HEIGHT 720

int process_image_file(cvitdl_handle_t tdl_handle, const char* imgf, cvtdl_face_t* p_obj)
{
    VIDEO_FRAME_INFO_S stVideoFrame;
    memset(&stVideoFrame, 0, sizeof(stVideoFrame));
    CVI_S32 s32Ret = CVI_SUCCESS;

    SIZE_S stSize = {
        .u32Width  = WIDTH,
        .u32Height = HEIGHT,
    };
    s32Ret =
        SAMPLE_COMM_FRAME_LoadFromFile(imgf, &stVideoFrame, &stSize, PIXEL_FORMAT_RGB_888_PLANAR);
    if (s32Ret != CVI_SUCCESS) {
        printf("cvi_tdl read image :%s failed.\n", imgf);
        return CVI_TDL_FAILURE;
    }

    s32Ret = CVI_TDL_Detection(tdl_handle, &stVideoFrame,
                               CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, p_obj);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_TDL_ScrFDFace failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    VB_BLK blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);
    s32Ret     = CVI_VB_ReleaseBlock(blk);
    if (s32Ret != CVI_SUCCESS) {
        printf("video frame release failed! \n");
        return CVI_FAILURE;
    }
    return s32Ret;
}

void pd_main(void* arg)
{
    char** argv = (char**)arg;

    int vpssgrp_width  = WIDTH;
    int vpssgrp_height = HEIGHT;

    CVI_S32 s32Ret =
        MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888_PLANAR, 2,
                         vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888_PLANAR, 2);
    if (s32Ret != CVI_SUCCESS) {
        printf("Init sys failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    cvitdl_handle_t tdl_handle = NULL;

    s32Ret = CVI_TDL_CreateHandle(&tdl_handle);
    if (s32Ret != CVI_SUCCESS) {
        printf("Create tdl handle failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    char* image = argv[2];

    s32Ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, argv[1]);
    if (s32Ret != CVI_SUCCESS) {
        printf("open model failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    cvtdl_object_t obj_meta = {0};
    process_image_file(tdl_handle, image, &obj_meta);
    printf("obj_size: %d\n", obj_meta.size);

    CVI_TDL_Free(&obj_meta);
    CVI_TDL_DestroyHandle(tdl_handle);
    return s32Ret;
}

void sample_pd_image(int argc, char* argv[])
{
    pthread_attr_t attr;
    pthread_t thread;
    CVI_S32 s32Ret;

    cvi_tpu_init();
    aos_msleep(1000);

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 64 * 1024);
    s32Ret = pthread_create(&thread, &attr, pd_main, (void*)argv);
    if (s32Ret != CVI_SUCCESS) {
        printf("Error create pd thread! \n");
    }
    pthread_join(thread, NULL);
    printf("pd thread finished! \n");
    cvi_tpu_deinit();
}

ALIOS_CLI_CMD_REGISTER(sample_pd_image, sample_pd_image, cvi_tdl sample pd with image);
