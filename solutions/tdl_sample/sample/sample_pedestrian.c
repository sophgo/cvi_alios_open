#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "aos/cli.h"

void pd_main(void *arg) {
  char** argv = (char **)arg;
  int vpssgrp_width = 1920;
  int vpssgrp_height = 1080;


  CVI_S32 ret = MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 2,
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

  char* image = argv[2];

  ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, argv[1]);
  if (ret != CVI_SUCCESS) {
    printf("open model failed with %#x!\n", ret);
    return ret;
  }
  VIDEO_FRAME_INFO_S bg;
  SIZE_S stSize = {
    .u32Width = vpssgrp_width,
    .u32Height = vpssgrp_height,
  };
  ret = SAMPLE_COMM_FRAME_LoadFromFile(image, &bg, &stSize, PIXEL_FORMAT_RGB_888_PLANAR);
  if (ret != CVI_SUCCESS) {
    printf("open img failed with %#x!\n", ret);
    return ret;
  } else {
    printf("image read,width:%d\n", bg.stVFrame.u32Width);
  }

  for (int i = 0; i < 1; i++) {
    cvtdl_object_t obj_meta = {0};
    CVI_TDL_Detection(tdl_handle, &bg, CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, &obj_meta);
    printf("obj_size: %d\n", obj_meta.size);
    CVI_TDL_Free(&obj_meta);
  }

  CVI_VPSS_ReleaseChnFrame(0, 0, &bg);
  CVI_TDL_DestroyHandle(tdl_handle);
  return ret;
}

void sample_pd(int argc, char *argv[]) {
	pthread_attr_t attr;
	pthread_t thread;
  int ret;
  pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 64*1024);
  ret = pthread_create(&thread, &attr, pd_main, (void *)argv);
  if(ret != CVI_SUCCESS) {
    printf("Error create fr thread! \n");
  }
  pthread_join(thread, NULL);
  printf("fr thread finished! \n");
}

ALIOS_CLI_CMD_REGISTER(sample_pd, sample_tdl_pd, cvi_tdl sample pd);
