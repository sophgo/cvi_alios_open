#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "aos/cli.h"
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_app.h"
#include "cvi_tdl_media.h"
#include "sample_comm.h"
#include "vi_vo_utils.h"

#define WIDTH 1280
#define HEIGHT 720

void fr_main(void* arg)
{
    char** argv = (char**)arg;

    cvi_tpu_init();
    aos_msleep(1000);

    int vpssgrp_width  = WIDTH;
    int vpssgrp_height = HEIGHT;
    CVI_S32 ret        = MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 2,
                                          vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 2);
    if (ret != CVI_SUCCESS) {
        printf("Init sys failed with %#x!\n", ret);
        return ret;
    }

    cvitdl_handle_t tdl_handle = NULL;

    ret = CVI_TDL_CreateHandle(&tdl_handle);
    if (ret != CVI_SUCCESS) {
        printf("Create tdl handle failed with %#x!\n", ret);
        return ret;
    }
    cvtdl_face_t p_obj        = {0};
    const char* fd_model_path = argv[1];
    const char* fr_model_path = argv[2];
    const char* img_path      = argv[3];

    ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, fd_model_path);
    ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, fr_model_path);

    VIDEO_FRAME_INFO_S stVideoFrame;
    SIZE_S stSize = {
        .u32Width  = WIDTH,
        .u32Height = HEIGHT,
    };
    ret = SAMPLE_COMM_FRAME_LoadFromFile(img_path, &stVideoFrame, &stSize,
                                         PIXEL_FORMAT_RGB_888_PLANAR);
    if (ret != CVI_SUCCESS) {
        printf("failed to open file\n");
        return ret;
    } else {
        printf("image read,width:%d\n", stVideoFrame.stVFrame.u32Width);
    }

    ret = CVI_TDL_FaceDetection(tdl_handle, &stVideoFrame, CVI_TDL_SUPPORTED_MODEL_RETINAFACE,
                                &p_obj);
    if (ret != CVI_SUCCESS) {
        printf("failed to run face detection\n");
        return ret;
    }

    printf("check point ! face count: %d \n", p_obj.size);

    ret = CVI_TDL_FaceRecognition(tdl_handle, &stVideoFrame,
                                  &p_obj);  // need to change k_mm.h:35 MM_ALIGN_BIT from 3 to 6
    if (ret != CVI_SUCCESS) {
        printf("failed to run face re\n");
        return ret;
    }

    VB_BLK blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);
    ret        = CVI_VB_ReleaseBlock(blk);
    if (ret != CVI_SUCCESS) {
        printf("video frame release failed! \n");
        return CVI_FAILURE;
    }
    for (uint32_t i = 0; i < p_obj.size; i++) {
        printf("bbox=[%f,%f,%f,%f]\n", p_obj.info[i].bbox.x1, p_obj.info[i].bbox.y1,
               p_obj.info[i].bbox.x2, p_obj.info[i].bbox.y2);
        printf("feature=[");
        for (uint32_t j = 0; j < p_obj.info[i].feature.size; j++) {
            printf("%d ", p_obj.info[i].feature.ptr[j]);
        }
        printf("]");
        printf("\n");
    }

    CVI_TDL_Free(&p_obj);
    CVI_TDL_DestroyHandle(tdl_handle);
    cvi_tpu_deinit();
    return true;
}

void sample_fr(int argc, char* argv[])
{
    pthread_attr_t attr;
    pthread_t thread;
    int ret;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 64 * 1024);
    ret = pthread_create(&thread, &attr, fr_main, (void*)argv);
    if (ret != CVI_SUCCESS) {
        printf("Error create fr thread! \n");
    }
    pthread_join(thread, NULL);
    printf("fr thread finished! \n");
}

ALIOS_CLI_CMD_REGISTER(sample_fr, sample_tdl_fr, cvi_tdl sample fr);
