
#include "object_detection.h"

#include <aos/kernel.h>
#include <stdio.h>
#include <vfs.h>

#include "ai_utils.h"
#include "base_ctx.h"
#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tpu_interface.h"
#include "cvi_vo.h"
#include "cviruntime.h"
#include "fatfs_vfs.h"
#include "param_vpss.h"
#include "vpss_uapi.h"

#define USE_OUTSIDE_VPSS 1

/**
 * @brief  Save videoframe to the file
 * @note
 *
 * @param  filename: the file to save data
 * @param  frame_header: the pointer indicate to the frame header
 * @param  frame_len: the length of the frame header
 * @param  pstVideoFrame: the videoframe whose data will be saved to file
 *
 * @retval CVI_SUCCESS if no problem
 */
CVI_S32 FRAME_SaveToFile(const CVI_CHAR *filename, uint8_t *frame_header, int frame_len,
                         VIDEO_FRAME_INFO_S *pstVideoFrame) {
  FILE *fp;
  CVI_U32 u32len, u32DataLen;

  fp = fopen(filename, "w");
  if (fp == CVI_NULL) {
    printf("open data file error\n");
    return CVI_FAILURE;
  }
  if (pstVideoFrame == NULL) {
    printf("video frame is NULL\n");
    return CVI_FAILURE;
  }

  if (frame_header != NULL) {
    u32len = fwrite(frame_header, frame_len, 1, fp);
  }

  for (int i = 0; i < 3; ++i) {
    u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
    if (u32DataLen == 0)
      continue;
    if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
                  (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
                  (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
      u32DataLen >>= 1;

    printf("plane(%d): paddr(%#lx) vaddr(%p) stride(%d)\n", i,
           pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.pu8VirAddr[i],
           pstVideoFrame->stVFrame.u32Stride[i]);
    printf("data_len(%d) plane_len(%d)\n", u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);

    u32len = fwrite((void *)pstVideoFrame->stVFrame.u64PhyAddr[i], u32DataLen, 1, fp);
    if (u32len <= 0) {
      printf("fwrite data(%d) error\n", i);
      break;
    }
  }

  fclose(fp);
  return CVI_SUCCESS;
}

/**
 * @brief  write cvi_tdl model test results into file
 * @note
 *
 * @param  *sz_file: the path of file
 * @param  *p_obj_meta: cvi_tdl result object
 *
 * @retval return CVI_SUCCESS if record result successfully
 */
int dump_object_info(const char *sz_file, cvtdl_object_t *p_obj_meta) {
  int fd = aos_open(sz_file, 0100 | 02 | 01000);
  CVI_S32 ret = CVI_SUCCESS;
  if (fd <= 0) {
    printf("aos_open dst file failed\n");
    return CVI_FAILURE;
  }
  for (int i = 0; i < p_obj_meta->size; i++) {
    char szinfo[128];
    sprintf(szinfo, "x1y1x2y2=[%f,%f,%f,%f],score:%f,class:%d\n", p_obj_meta->info[i].bbox.x1,
            p_obj_meta->info[i].bbox.y1, p_obj_meta->info[i].bbox.x2, p_obj_meta->info[i].bbox.y2,
            p_obj_meta->info[i].bbox.score, p_obj_meta->info[i].classes);
    aos_write(fd, szinfo, strlen(szinfo));
  }
  aos_close(fd);
  return ret;
}

/**
 * @brief Load frame from pipeline into cvi_tdl model for object detection
 * @note
 *
 * @param  argc: the number of command line arguments
 * @param  argv: the command line arguments
 *
 * @retval None
 */
void cvitdl_object_detection(int32_t argc, char **argv) {
  if (argc != 2) {
    printf("usage: cvitdl_fd <inference_num>\n");
    return;
  }

  int inference_num = atoi(argv[1]);
  if (inference_num <= 0) {
    printf("inference_num must be larger than zero.\n");
    return;
  }

  printf("CVITDL Object Detection (BEGIN)\n");
  CVI_S32 ret = CVI_TDL_SUCCESS;

  cvi_tpu_init();
  aos_msleep(1000);

  /* Get model index & .cvimodel path */
  int OD_MODEL_INDEX = CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN;
  char *OD_MODEL_PATH = SD_FATFS_MOUNTPOINT "/mobiledetv2-pedestrian-d0-ls-640.cvimodel";
  printf("OD MODEL PATH = %s\n", OD_MODEL_PATH);

  cvitdl_handle_t ai_handle = NULL;
  ret = CVI_TDL_CreateHandle2(&ai_handle, 1, 0);
  printf("CVITDL CreateHandle Success.\n");

  ret = CVI_TDL_OpenModel(ai_handle, OD_MODEL_INDEX, OD_MODEL_PATH);
  printf("CVITDL OpenModel Success.\n");

  /* Initialize VPSS config */
  VPSS_GRP vpssGrp = 0;
  VPSS_CHN vpssChnAI = VPSS_CHN0;
  int width = 1280;
  int height = 720;
  if (USE_OUTSIDE_VPSS == 1) {
    cvtdl_vpssconfig_t vpssconfig;
    CVI_TDL_GetVpssChnConfig(ai_handle, OD_MODEL_INDEX, width, height, vpssChnAI, &vpssconfig);
    CVI_VPSS_SetChnAttr(vpssGrp, vpssChnAI, &vpssconfig.chn_attr);
    CVI_TDL_SetSkipVpssPreprocess(ai_handle, OD_MODEL_INDEX, true);
    printf("CVITDL SetSkipVpssPreprocess Success.\n");
  } else {
    CVI_TDL_SetSkipVpssPreprocess(ai_handle, OD_MODEL_INDEX, false);
  }
  aos_msleep(1000);

  cvtdl_object_t obj_meta;
  memset(&obj_meta, 0, sizeof(cvtdl_object_t));
  int counter = 0;
  VIDEO_FRAME_INFO_S stVideoFrame;

  /* Get frame from vpss & Start object detection */
  while (counter < inference_num) {
    printf("counter %u\n", counter);

    ret = CVI_VPSS_GetChnFrame(vpssGrp, vpssChnAI, &stVideoFrame, 2000);
    if (ret != CVI_SUCCESS) {
      printf("CVI_VPSS_GetChnFrame failed with %#x\n", ret);
      break;
    }
    printf("frame[%u]: height[%u], width[%u]\n", counter, stVideoFrame.stVFrame.u32Height,
           stVideoFrame.stVFrame.u32Width);

    if (counter > 0) {
      switch (OD_MODEL_INDEX) {
      case CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN:
        CVI_TDL_MobileDetV2_Pedestrian(ai_handle, &stVideoFrame, &obj_meta);
        break;
      default:
        printf("Model %d is not supported yet\n", OD_MODEL_INDEX);
        break;
      }
    } else {
      printf("ignore counter 0 (workaround)\n");
    }

    printf("[OD] obj count: %u\n", obj_meta.size);
    for (uint32_t i = 0; i < obj_meta.size; i++) {
      printf("obj[%u]: class[%d], score[%.2f]\n", i, obj_meta.info[i].classes,
             obj_meta.info[i].bbox.score);
      printf("pedistrian[%u]: x1y1x2y2[%.2f,%.2f,%.2f,%.2f],class:%d\n", obj_meta.size,
             obj_meta.info[i].bbox.x1, obj_meta.info[i].bbox.y1, obj_meta.info[i].bbox.x2,
             obj_meta.info[i].bbox.y2, obj_meta.info[i].classes);
    }

    ret = CVI_VPSS_ReleaseChnFrame(vpssGrp, vpssChnAI, &stVideoFrame);
    if (ret != CVI_SUCCESS) {
      printf("CVI_VPSS_ReleaseChnFrame grp%d chn%d NG\n", vpssGrp, vpssChnAI);
      break;
    }

    CVI_TDL_Free(&obj_meta);
    ++counter;
  }

  CVI_TDL_DestroyHandle(ai_handle);

  printf("CVITDL Object Detection (END)\n");
}
ALIOS_CLI_CMD_REGISTER(cvitdl_object_detection, cvitdl_od, cvitdl object detection);

/**
 * @brief Load .bin image into cvi_tdl model for object detection
 * @note
 *
 * @param  argc: the number of command line arguments
 * @param  argv: the command line arguments
 *
 * @retval None
 */
void cvitdl_read_od_bin(int32_t argc, char **argv) {
  if (argc != 2) {
    printf("usage: cvitdl_read_pd_bin <image_name>\n");
    return;
  }

  CVI_U32 ret = CVI_SUCCESS;

  VIDEO_FRAME_INFO_S frame_in;
  memset(&frame_in, 0, sizeof(frame_in));
  cvtdl_object_t obj_meta;
  memset(&obj_meta, 0, sizeof(cvtdl_object_t));
  cvtdl_vpssconfig_t vpssConfig;
  memset(&vpssConfig, 0, sizeof(vpssConfig));

  char image_path[64];
  snprintf(image_path, 64, "%s/%s", SD_FATFS_MOUNTPOINT, argv[1]);
  printf("CVITDL Pedestrian Detection (read image: %s)\n", image_path);

  int OD_PIXEL_FORMAT = PIXEL_FORMAT_RGB_888_PLANAR;
  int OD_MODEL_INDEX = CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN;
  char *OD_MODEL_PATH = SD_FATFS_MOUNTPOINT "/mobiledetv2-pedestrian-d0-ls-640.cvimodel";
  printf("OD MODEL PATH = %s\n", OD_MODEL_PATH);

  cvi_tpu_init();
  aos_msleep(1000);

  cvitdl_handle_t ai_handle = NULL;
  CVI_TDL_CreateHandle2(&ai_handle, 1, 0);
  printf("CVITDL CreateHandle Success.\n");

  CVI_TDL_OpenModel(ai_handle, OD_MODEL_INDEX, OD_MODEL_PATH);
  printf("CVITDL OpenModel Success.\n");

  /* Initialize VPSS config */
  VPSS_GRP vpssGrp = (USE_OUTSIDE_VPSS == 1) ? 0 : 1;
  VPSS_CHN vpssChnAI = VPSS_CHN0;

  /* Initialize Channel Attribute */
  ret = CVI_TDL_GetVpssChnConfig(ai_handle, OD_MODEL_INDEX, 640, 384, 0, &vpssConfig);
  if (ret != CVI_SUCCESS) {
    printf("CVI_TDL_GetVpssChnConfig failed!\n");
    return;
  }
  vpssConfig.chn_attr.u32Depth = 1;
  CVI_VPSS_SetChnAttr(vpssGrp, vpssChnAI, &vpssConfig.chn_attr);

  bool bSkipVpssPreprocess = (USE_OUTSIDE_VPSS == 1) ? true : false;
  CVI_TDL_SetSkipVpssPreprocess(ai_handle, OD_MODEL_INDEX, bSkipVpssPreprocess);

  VB_BLK blk_fr;
  if (CVI_SUCCESS != CVI_TDL_LoadBinImage(image_path, &blk_fr, &frame_in, OD_PIXEL_FORMAT)) {
    printf("cvitdl read image failed.\n");
    CVI_VB_ReleaseBlock(blk_fr);
    CVI_TDL_DestroyHandle(ai_handle);
  }

  /* Handle .bin image for object detection */
  if (USE_OUTSIDE_VPSS == 1) {
    CVI_VPSS_SendFrame(vpssGrp, &frame_in, 2000);
    printf("CVI_VPSS_SendChnFrame pass\n");

    VIDEO_FRAME_INFO_S frame;
    printf("Get vpssGrp %d, Chn %d\n", vpssGrp, vpssChnAI);
    ret = CVI_VPSS_GetChnFrame(vpssGrp, vpssChnAI, &frame, 2000);
    if (ret != CVI_SUCCESS) {
      printf("CVI_VPSS_GetChnFrame failed with %#x\n", ret);
      return;
    }
    CVI_TDL_MobileDetV2_Pedestrian(ai_handle, &frame, &obj_meta);
    CVI_VPSS_ReleaseChnFrame(vpssGrp, vpssChnAI, &frame);
  } else {
    CVI_TDL_MobileDetV2_Pedestrian(ai_handle, &frame_in, &obj_meta);
  }

  printf("obj meta: size[%u], width[%u], height[%u], rescale_type[%d]\n", obj_meta.size,
         obj_meta.width, obj_meta.height, obj_meta.rescale_type);
  for (uint32_t i = 0; i < obj_meta.size; ++i) {
    printf("obj[%u]: x1y1x2y2[%.2f,%.2f,%.2f,%.2f],class:%d\n", obj_meta.size,
           obj_meta.info[i].bbox.x1, obj_meta.info[i].bbox.y1, obj_meta.info[i].bbox.x2,
           obj_meta.info[i].bbox.y2, obj_meta.info[i].classes);
  }
  
  dump_object_info("/mnt/sd/od_result.txt", &obj_meta);

  CVI_TDL_Free(&obj_meta);
  CVI_VB_ReleaseBlock(blk_fr);
  CVI_TDL_DestroyHandle(ai_handle);

  printf("CVITDL Object Detection (END)\n");
}

ALIOS_CLI_CMD_REGISTER(cvitdl_read_od_bin, cvitdl_od_bin, cvitdl_read_od_bin);