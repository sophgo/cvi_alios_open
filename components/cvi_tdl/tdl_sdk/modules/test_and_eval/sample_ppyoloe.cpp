#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "core/cvi_tdl_types_mem_internal.h"
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "sys_utils.hpp"

// set preprocess and algorithm param for ppyoloe detection
// if use official model, no need to change param
CVI_S32 init_param(const cvitdl_handle_t tdl_handle, bool assign_out_string) {
  // setup preprocess
  InputPreParam preprocess_cfg = CVI_TDL_GetPreParam(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE);

  for (int i = 0; i < 3; i++) {
    printf("asign val %d \n", i);
    preprocess_cfg.factor[i] = 0.003922;
    preprocess_cfg.mean[i] = 0.0;
  }
  preprocess_cfg.format = PIXEL_FORMAT_RGB_888_PLANAR;

  printf("setup ppyoloe param \n");
  CVI_S32 ret = CVI_TDL_SetPreParam(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, preprocess_cfg);
  if (ret != CVI_SUCCESS) {
    printf("Can not set ppyoloe preprocess parameters %#x\n", ret);
    return ret;
  }

  // setup yolo algorithm preprocess
  cvtdl_det_algo_param_t ppyoloe_param =
      CVI_TDL_GetDetectionAlgoParam(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE);
  ppyoloe_param.cls = 80;

  printf("setup ppyoloe algorithm param \n");
  ret = CVI_TDL_SetDetectionAlgoParam(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, ppyoloe_param);
  if (ret != CVI_SUCCESS) {
    printf("Can not set ppyoloe algorithm parameters %#x\n", ret);
    return ret;
  }
  if (assign_out_string) {
    const char* names_array[] = {"reshape2_1.tmp_0_Reshape",    "reshape2_3.tmp_0_Reshape",
                                 "reshape2_5.tmp_0_Reshape",    "transpose_1.tmp_0_Transpose",
                                 "transpose_3.tmp_0_Transpose", "transpose_5.tmp_0_Transpose"};
    CVI_TDL_Set_Outputlayer_Names(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, names_array, 6);
    if (ret != CVI_SUCCESS) {
      printf("Outputlayer names set failed %#x!\n", ret);
      return ret;
    }
  }

  // set thershold
  CVI_TDL_SetModelThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, 0.5);
  CVI_TDL_SetModelNmsThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, 0.5);

  printf("ppyoloe algorithm parameters setup success!\n");
  return ret;
}

int main(int argc, char* argv[]) {
  int vpssgrp_width = 1920;
  int vpssgrp_height = 1080;
  CVI_S32 ret = MMF_INIT_HELPER2(vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 1,
                                 vpssgrp_width, vpssgrp_height, PIXEL_FORMAT_RGB_888, 1);
  if (ret != CVI_TDL_SUCCESS) {
    printf("Init sys failed with %#x!\n", ret);
    return ret;
  }

  cvitdl_handle_t tdl_handle = NULL;
  ret = CVI_TDL_CreateHandle(&tdl_handle);
  if (ret != CVI_SUCCESS) {
    printf("Create tdl handle failed with %#x!\n", ret);
    return ret;
  }

  std::string model_path = argv[1];
  std::string str_src_dir = argv[2];

  float conf_threshold = 0.5;
  float nms_threshold = 0.5;

  // change param of ppyoloe
  bool assign_out_string = false;
  ret = init_param(tdl_handle, assign_out_string);

  printf("start open cvimodel...\n");
  ret = CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, model_path.c_str());
  if (ret != CVI_SUCCESS) {
    printf("open model failed %#x!\n", ret);
    return ret;
  }
  printf("cvimodel open success!\n");
  // set thershold
  CVI_TDL_SetModelThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, conf_threshold);
  CVI_TDL_SetModelNmsThreshold(tdl_handle, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, nms_threshold);

  std::cout << "model opened:" << model_path << std::endl;

  VIDEO_FRAME_INFO_S fdFrame;
  imgprocess_t img_handle;
  CVI_TDL_Create_ImageProcessor(&img_handle);

  ret = CVI_TDL_ReadImage(img_handle, str_src_dir.c_str(), &fdFrame, PIXEL_FORMAT_RGB_888);
  if (ret != CVI_SUCCESS) {
    std::cout << "Convert out video frame failed with :" << ret << ".file:" << str_src_dir
              << std::endl;
  }

  cvtdl_object_t obj_meta = {0};

  CVI_TDL_Detection(tdl_handle, &fdFrame, CVI_TDL_SUPPORTED_MODEL_PPYOLOE, &obj_meta);

  printf("detect number: %d\n", obj_meta.size);
  for (uint32_t i = 0; i < obj_meta.size; i++) {
    printf("detect res: %f %f %f %f %f %d\n", obj_meta.info[i].bbox.x1, obj_meta.info[i].bbox.y1,
           obj_meta.info[i].bbox.x2, obj_meta.info[i].bbox.y2, obj_meta.info[i].bbox.score,
           obj_meta.info[i].classes);
  }

  CVI_TDL_ReleaseImage(img_handle, &fdFrame);
  CVI_TDL_Free(&obj_meta);
  CVI_TDL_DestroyHandle(tdl_handle);
  CVI_TDL_Destroy_ImageProcessor(img_handle);
  return ret;
}
