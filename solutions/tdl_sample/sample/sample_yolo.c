
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include "aos/cli.h"
#include "core/utils/vpss_helper.h"
#include "cvi_ive.h"
#include "cvi_ive_interface.h"
#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "cvi_tpu_interface.h"
#include "sample_comm.h"
#include "sample_utils.h"

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

    s32Ret = CVI_TDL_Detection(tdl_handle, &stVideoFrame, CVI_TDL_SUPPORTED_MODEL_YOLO, p_obj);
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

void* yolo_main(void* arg)
{
    char** argv = (char**)arg;

    cvi_tpu_init();
    aos_msleep(1000);

    int vpssgrp_width  = WIDTH;
    int vpssgrp_height = HEIGHT;
    CVI_S32 ret = MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 3,
                                   vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 1);
    if (ret != CVI_SUCCESS) {
        printf("Init sys failed with %#x!\n", ret);
        pthread_exit(NULL);
    }

    cvitdl_handle_t tdl_handle = NULL;

    ret = CVI_TDL_CreateHandle(&tdl_handle);
    if (ret != CVI_SUCCESS) {
        printf("Create tdl handle failed with %#x!\n", ret);
        pthread_exit(NULL);
    }

    const char* model = argv[1];  // model path
    const char* img   = argv[2];  // image path;

    ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLO, model);
    if (ret != CVI_SUCCESS) {
        printf("open model failed with %#x!\n", ret);
        pthread_exit(NULL);
    }

    // set thershold
    CVI_TDL_SetModelThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLO, 0.5);
    CVI_TDL_SetModelNmsThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_YOLO, 0.5);

    cvtdl_object_t stObjMeta = {0};
    memset(&stObjMeta, 0, sizeof(cvtdl_face_t));
    process_image_file(tdl_handle, img, &stObjMeta);

    for (uint32_t i = 0; i < stObjMeta.size; i++) {
        printf("bbox: [%f, %f, %f, %f]; score: [%f]; class: [%d]\n", stObjMeta.info[i].bbox.x1, stObjMeta.info[i].bbox.y1,
            stObjMeta.info[i].bbox.x2, stObjMeta.info[i].bbox.y2, stObjMeta.info[i].bbox.score,
            stObjMeta.info[i].classes);
    }

    CVI_TDL_Free(&stObjMeta);
    CVI_TDL_DestroyHandle(tdl_handle);
    cvi_tpu_deinit();
    pthread_exit(NULL);
}

void sample_yolo(int argc, char* argv[])
{
    pthread_attr_t attr;
    pthread_t thread;
    int ret;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 64 * 1024);
    ret = pthread_create(&thread, &attr, yolo_main, (void*)argv);
    if (ret != CVI_SUCCESS) {
        printf("Error create od thread! \n");
    }
    pthread_join(thread, NULL);
    printf("od thread finished! \n");
}

ALIOS_CLI_CMD_REGISTER(sample_yolo, sample_yolo, cvi_tdl sample yolo);