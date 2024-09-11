#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_app.h"
#include "cvi_tdl_media.h"
#include "sample_comm.h"
#include "vi_vo_utils.h"
#include "aos/cli.h"

void fr_main(void *arg) {
  // if (argc != 4) {
  //   printf("need fr_model_path, fd_model_path, image_path");
  //   return CVI_FAILURE;
  // }
  char** argv = (char **)arg;
  int vpssgrp_width = 640;
  int vpssgrp_height = 640;
  CVI_S32 ret = MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 1,
                                 vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 1);
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
  cvtdl_face_t p_obj = {0};
  const char *fr_model_path = argv[1];
  const char *fd_model_path = argv[2];
  const char *img_path = argv[3];

  ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, fr_model_path);
  ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_SCRFDFACE, fd_model_path);

  VIDEO_FRAME_INFO_S bg;
  SIZE_S stSize = {
    .u32Width = vpssgrp_width,
    .u32Height = vpssgrp_height,
  };
  ret = SAMPLE_COMM_FRAME_LoadFromFile(img_path, &bg, &stSize, PIXEL_FORMAT_RGB_888_PLANAR);
  if (ret != CVI_SUCCESS) {
    printf("failed to open file\n");
    return ret;
  } else {
    printf("image read,width:%d\n", bg.stVFrame.u32Width);
  }

  // get bbox
  ret = CVI_TDL_FaceDetection(tdl_handle, &bg, CVI_TDL_SUPPORTED_MODEL_SCRFDFACE, &p_obj);
  if (ret != CVI_SUCCESS) {
    printf("failed to run face detection\n");
    return ret;
  }
  // get face feature
  ret |= CVI_TDL_FaceRecognition(tdl_handle, &bg, &p_obj);
  if (ret != CVI_SUCCESS) {
    printf("failed to run face re\n");
    return ret;
  }
  CVI_VPSS_ReleaseChnFrame(0, 0, &bg);
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
  CVI_TDL_DestroyHandle(tdl_handle);
  return true;
}

void sample_fr(int argc, char *argv[]) {
	pthread_attr_t attr;
	pthread_t thread;
  int ret;
  pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 64*1024);
  ret = pthread_create(&thread, &attr, fr_main, (void *)argv);
  if(ret != CVI_SUCCESS) {
    printf("Error create fr thread! \n");
  }
  pthread_join(thread, NULL);
  printf("fr thread finished! \n");
}

ALIOS_CLI_CMD_REGISTER(sample_fr, sample_tdl_fr, cvi_tdl sample fr);
