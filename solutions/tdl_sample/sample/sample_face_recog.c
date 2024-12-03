#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "aos/cli.h"
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "cvi_tpu_interface.h"
#include "sample_comm.h"

#define WIDTH 1280
#define HEIGHT 720
#define FACE_FEAT_SIZE 512

void save_features(const char *filename, const int8_t *ptr, uint32_t size) {
    FILE *file = fopen(filename, "wb");  // 打开文件以写入二进制模式
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    // 写入数据
    size_t written = fwrite(ptr, sizeof(int8_t), size, file);
    if (written != size) {
        perror("Failed to write all features");
    } else {
        printf("Successfully wrote %u features.\n", size);
    }

    fclose(file);  // 关闭文件
}

CVI_S32 register_gallery_feature(cvitdl_handle_t tdl_handle, const char *sz_feat_file,
                                 cvtdl_service_feature_array_t *p_feat_gallery) {
  FILE *fp = fopen(sz_feat_file, "rb");
  if (fp == NULL) {
    printf("read %s failed\n", sz_feat_file);
    return CVI_TDL_FAILURE;
  }
  fseek(fp, 0, SEEK_END);
  int len = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  int8_t *ptr_feat = (int8_t *)malloc(len);
  fread(ptr_feat, 1, len, fp);
  fclose(fp);
  printf("read %s done,len:%d\n", sz_feat_file, len);
  if (len != FACE_FEAT_SIZE) {
    free(ptr_feat);
    return CVI_TDL_FAILURE;
  }
  size_t src_size = 0;
  int8_t *p_new_feat = NULL;
  if (p_feat_gallery->ptr == 0) {
    p_new_feat = (int8_t *)malloc(FACE_FEAT_SIZE);
    p_feat_gallery->type = TYPE_INT8;
    p_feat_gallery->feature_length = FACE_FEAT_SIZE;
    printf("allocate memory,size:%d\n", (int)p_feat_gallery->feature_length);
  } else {
    src_size = p_feat_gallery->feature_length * p_feat_gallery->data_num;
    p_new_feat = (int8_t *)malloc(src_size + FACE_FEAT_SIZE);
    memcpy(p_new_feat, p_feat_gallery->ptr, src_size);
    free(p_feat_gallery->ptr);
  }
  memcpy(p_new_feat + src_size, ptr_feat, FACE_FEAT_SIZE);
  p_feat_gallery->data_num += 1;
  p_feat_gallery->ptr = p_new_feat;
  free(ptr_feat);
  printf("register face sucess,gallery num:%d\n", (int)p_feat_gallery->data_num);
  return CVI_SUCCESS;
}

CVI_S32 do_face_match(cvitdl_service_handle_t service_handle, cvtdl_face_info_t *p_face,
                      float thresh) {
  if (p_face->feature.size == 0) {
    return CVI_FAILURE;
  }
  uint32_t ind = 0;
  float score = 0;
  uint32_t size;

  int ret = CVI_TDL_Service_FaceInfoMatching(service_handle, p_face, 1, 0.1, &ind, &score, &size);

  if (score > thresh) {
    p_face->recog_score = score;
    sprintf(p_face->name, "%d", ind);
  }
  printf("matchname,trackid:%u,index:%d,score:%f,ret:%d,featlen:%d,name:%s\n",
         (uint32_t)p_face->unique_id, ind, score, ret, (int)p_face->feature.size, p_face->name);
  return ret;
}

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
    cvitdl_service_handle_t service_handle = NULL;

    ret = CVI_TDL_CreateHandle(&tdl_handle);
    ret |= CVI_TDL_Service_CreateHandle(&service_handle, tdl_handle);
    if (ret != CVI_SUCCESS) {
        printf("Create tdl handle failed with %#x!\n", ret);
        return ret;
    }
    cvtdl_face_t p_obj        = {0};
    const char* fd_model_path = argv[1];
    const char* fr_model_path = argv[2];
    const char* img_path      = argv[3];
    const char* register_face = atoi(argv[4]);

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
    for (uint32_t i = 0; i < 1; i++) {
        printf("bbox=[%f,%f,%f,%f]\n", p_obj.info[i].bbox.x1, p_obj.info[i].bbox.y1,
               p_obj.info[i].bbox.x2, p_obj.info[i].bbox.y2);
        printf("feature=[");
        for (uint32_t j = 0; j < p_obj.info[i].feature.size; j++) {
            printf("%d ", p_obj.info[i].feature.ptr[j]);
        }
        printf("]");
        printf("\n");

        if (register_face) {
            char feature_filename[128];
            sprintf(feature_filename, "/mnt/sd/feature/feature_%d.bin", i);
            save_features(feature_filename, p_obj.info[i].feature.ptr, p_obj.info[i].feature.size);
        }
    }

    if (!register_face) {
        cvtdl_service_feature_array_t feat_gallery;
        memset(&feat_gallery, 0, sizeof(feat_gallery));
        printf("to register face gallery\n");
        for(uint32_t i=0; i<1; i++) {
            char szbinf[128];
            sprintf(szbinf, "/mnt/sd/feature/feature_%d.bin", i);
            register_gallery_feature(tdl_handle, szbinf, &feat_gallery);
        }
        ret = CVI_TDL_Service_RegisterFeatureArray(service_handle, feat_gallery, COS_SIMILARITY);
        do_face_match(service_handle, p_obj.info, 0.4);
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
