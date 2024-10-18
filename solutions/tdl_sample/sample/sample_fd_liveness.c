
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include "aos/cli.h"
#include "core/utils/vpss_helper.h"
#include "cvi_ive.h"
#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "cvi_tpu_interface.h"
#include "sample_comm.h"

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

    s32Ret =
        CVI_TDL_FaceDetection(tdl_handle, &stVideoFrame, CVI_TDL_SUPPORTED_MODEL_SCRFDFACE, p_obj);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_TDL_ScrFDFace failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    if (p_obj->size > 0) {
        s32Ret = CVI_TDL_IrLiveness(tdl_handle, &stVideoFrame, p_obj);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_TDL_FaceAttribute failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    } else {
        printf("cannot find faces\n");
    }

    VB_BLK blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);
    s32Ret     = CVI_VB_ReleaseBlock(blk);
    if (s32Ret != CVI_SUCCESS) {
        printf("video frame release failed! \n");
        return CVI_FAILURE;
    }
    return s32Ret;
}

void* liveness_main(void* arg)
{
    char** argv = (char**)arg;

    int vpssgrp_width  = WIDTH;
    int vpssgrp_height = HEIGHT;
    CVI_S32 ret = MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888_PLANAR, 3,
                                   vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888_PLANAR, 1);
    if (ret != CVI_SUCCESS) {
        printf("Init sys failed with %#x!\n", ret);
        pthread_exit(NULL);
    }

    cvitdl_handle_t tdl_handle = NULL;
    ret                        = CVI_TDL_CreateHandle(&tdl_handle);
    if (ret != CVI_SUCCESS) {
        printf("Create ai handle failed with %#x!\n", ret);
        pthread_exit(NULL);
    }

    const char* fd_model = argv[1];  // face detection model path
    const char* ln_model = argv[2];  // liveness model path
    const char* img      = argv[3];  // image path;

    ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_SCRFDFACE, fd_model);
    if (ret != CVI_SUCCESS) {
        printf("open CVI_TDL_SUPPORTED_MODEL_SCRFDFACE model failed with %#x!\n", ret);
        pthread_exit(NULL);
    }

    ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_IRLIVENESS, ln_model);
    if (ret != CVI_SUCCESS) {
        printf("open CVI_TDL_SUPPORTED_MODEL_IRLIVENESS model failed with %#x!\n", ret);
        pthread_exit(NULL);
    }

    cvtdl_face_t obj_meta = {0};
    process_image_file(tdl_handle, img, &obj_meta);

    printf("boxes=[");
    for (uint32_t i = 0; i < obj_meta.size; i++) {
        printf("[ %f, %f, %f, %f ],", obj_meta.info[i].bbox.x1, obj_meta.info[i].bbox.y1,
               obj_meta.info[i].bbox.x2, obj_meta.info[i].bbox.y2);

        if (obj_meta.info[i].liveness_score > 0) {
            printf("liveness score %d: %f  fake person\n", i, obj_meta.info[i].liveness_score);
        } else {
            printf("liveness score %d: %f  real person\n", i, obj_meta.info[i].liveness_score);
        }
    }
    printf("]\n");

    CVI_TDL_Free(&obj_meta);
    CVI_TDL_DestroyHandle(tdl_handle);
    pthread_exit(NULL);
}

void sample_fd_liveness(int argc, char* argv[])
{
    pthread_attr_t attr;
    pthread_t thread;
    int ret;

    cvi_tpu_init();
    aos_msleep(1000);

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 64 * 1024);
    ret = pthread_create(&thread, &attr, liveness_main, (void*)argv);
    if (ret != CVI_SUCCESS) {
        printf("Error create liveness thread! \n");
    }
    pthread_join(thread, NULL);
    printf("liveness thread finished! \n");
    cvi_tpu_deinit();
}

ALIOS_CLI_CMD_REGISTER(sample_fd_liveness, sample_fd_liveness, cvi_tdl sample liveness);