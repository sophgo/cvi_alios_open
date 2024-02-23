#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/kernel.h>
#include "cvi_vo.h"
#include "cviruntime.h"
#include "cvi_tdl.h"
#include "ai_utils.h"
#include "face_detection.h"
#include "fatfs_vfs.h"
#include <math.h>
#include <vfs.h>
#include <cvi_vpss.h>
#include "cvi_vb.h"
#include <stdlib.h>
#include <aos/cli.h>

CVI_S32 cal_cos_sim(cvtdl_feature_t *a, cvtdl_feature_t *b, float *score) {
  if (a->ptr == NULL || b->ptr == NULL || a->size != b->size) {
    printf("cosine distance failed.\n");
    return CVI_FAILURE;
  }
  float A = 0, B = 0, AB = 0;
  for (uint32_t i = 0; i < a->size; i++) {
    A += (int)a->ptr[i] * (int)a->ptr[i];
    B += (int)b->ptr[i] * (int)b->ptr[i];
    AB += (int)a->ptr[i] * (int)b->ptr[i];
  }
  A = sqrt(A);
  B = sqrt(B);
  *score = AB / (A * B);
  return CVI_SUCCESS;
}

// extern void vb_proc_show(int32_t argc, char **argv); //show vb pool
int load_feature_from_file(const char *sz_file,cvtdl_feature_t *p_feat,uint32_t size){
  int fd = aos_open(sz_file, O_RDONLY);
  if(fd <0){
    printf("open file failed,%s\n",sz_file);
    return -1;
  }
  memset(p_feat,0,sizeof(cvtdl_feature_t));
  p_feat->ptr = aos_malloc(size);
  p_feat->size = size;
  int nbytesread = aos_read(fd, p_feat->ptr, size);
  if(nbytesread != size){
    aos_close(fd);
    return -1;
  }
  aos_close(fd);
  return 0;
}
int dump_face_sim_info(const char *sz_file,cvtdl_face_t *p_face_meta,float *p_sim){
  int fd = aos_open(sz_file,  0100 | 02 | 01000);
  CVI_S32 s32Ret = CVI_SUCCESS;
	if (fd <= 0) {
		printf("aos_open dst file failed\n");
		return CVI_FAILURE;
	}
  for(int i = 0; i < p_face_meta->size;i++){
    char szinfo[128];
    sprintf(szinfo,"x1y1x2y2=[%f,%f,%f,%f],score:%f,sim:%f\n",p_face_meta->info[i].bbox.x1,
    p_face_meta->info[i].bbox.y1,p_face_meta->info[i].bbox.x2,p_face_meta->info[i].bbox.y2,
    p_face_meta->info[i].bbox.score,p_sim[i]);
    aos_write(fd,szinfo,strlen(szinfo));
  }
  aos_close(fd);
  return s32Ret;
}

//detect face in video,and then check similarity with the /mnt/sd/gallery.bin
int  cvitdl_face_recognition(int32_t argc, char **argv) {
  if (argc != 3) {
    printf("usage: cvitdl_fd <inference_num> <gallery_file>\n");
    return CVI_FAILURE;
  }
  int inference_num = atoi(argv[1]);

  LOGI(AI_TAG, "CVIAI Face Detection (inference num: %d)", inference_num);
  CVI_S32 s32Ret = CVI_TDL_SUCCESS;

  VideoSystemContext vs_ctx = {0};
  // SIZE_S aiInputSize = {.u32Width = 1920, .u32Height = 1080};
  SIZE_S aiInputSize = {.u32Width = 1280, .u32Height = 720};
  InitVideoSystem(&vs_ctx, &aiInputSize, VI_PIXEL_FORMAT, 0);

  cvitdl_handle_t ai_handle = NULL;
  s32Ret = CVI_TDL_CreateHandle2(&ai_handle, 1, 0);
  const char *FD_MODEL_PATH = SD_FATFS_MOUNTPOINT"/retinaface_mnet0.25_342_608.cvimodel";
  s32Ret = CVI_TDL_OpenModel(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, FD_MODEL_PATH);
  if (s32Ret != CVI_SUCCESS) {
    printf("Facelib open failed with %#x!\n", s32Ret);
    return s32Ret;
  }
  const char *FR_MODEL_PATH = SD_FATFS_MOUNTPOINT"/cviface-v5-s.cvimodel";
  s32Ret = CVI_TDL_OpenModel(ai_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, FR_MODEL_PATH);
  if (s32Ret != CVI_SUCCESS) {
    printf("Facelib open failed with %#x!\n", s32Ret);
    return s32Ret;
  }
  cvtdl_feature_t gallery;
  s32Ret = load_feature_from_file(argv[2],&gallery,256);
  if(s32Ret !=0){
    aos_free(gallery.ptr);
    printf("load file failed\n");
    return s32Ret;
  }else{
    printf("gallery loaded successfully\n");
  }
  CVI_TDL_SetSkipVpssPreprocess(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, false);
  CVI_TDL_SetSkipVpssPreprocess(ai_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, false);

  VIDEO_FRAME_INFO_S frame;
  /* Initialize VPSS config */
  VPSS_GRP vpssGrp = 0;
  VPSS_CHN vpssChnAI = VPSS_CHN0;

  aos_msleep(1000);

  cvtdl_face_t face_meta;
  memset(&face_meta, 0, sizeof(cvtdl_face_t));
  uint32_t counter = 0;
  while (inference_num == -1 || counter < inference_num) {
    printf("counter %u\n", counter);

    s32Ret = CVI_VPSS_GetChnFrame(vpssGrp, vpssChnAI, &frame, 2000);
    if (s32Ret != CVI_SUCCESS) {
      printf("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
      break;
    }
    printf("frame[%u]: width[%u], height[%u]\n", counter,
           frame.stVFrame.u32Width, frame.stVFrame.u32Height);

    cvtdl_face_t face_meta;
    memset(&face_meta, 0, sizeof(cvtdl_face_t));

    CVI_TDL_RetinaFace(ai_handle, &frame, &face_meta);
    printf("face meta: size[%u], width[%u], height[%u], rescale_type[%d]\n", 
          face_meta.size, face_meta.width, face_meta.height, face_meta.rescale_type);
    for (uint32_t i = 0; i < face_meta.size; ++i) {
      printf("face[%u]: x1[%.2f], x1[%.2f], x2[%.2f], y2[%.2f],score:%.2f\n", face_meta.size,
            face_meta.info[i].bbox.x1, face_meta.info[i].bbox.y1,
            face_meta.info[i].bbox.x2, face_meta.info[i].bbox.y2,face_meta.info[i].bbox.score);
    }

    CVI_TDL_FaceRecognition(ai_handle, &frame, &face_meta);
    for (uint32_t i = 0; i < face_meta.size; ++i){
      float score = 0;
      if(cal_cos_sim(&face_meta.info[i].feature,&gallery,&score)==0){
        printf("face:%u,matched score:%f\n",i,score);
      }
    }
    CVI_TDL_Free(&face_meta);
    s32Ret = CVI_VPSS_ReleaseChnFrame(vpssGrp, vpssChnAI, &frame);
    if (s32Ret != CVI_SUCCESS) {
      printf("CVI_VPSS_ReleaseChnFrame chn0 NG\n");
      break;
    }
    ++counter;
  }
  aos_free(gallery.ptr);
  CVI_TDL_DestroyHandle(ai_handle);

  LOGI(AI_TAG, "CVIAI Face Detection (END)");
  return s32Ret;
}
ALIOS_CLI_CMD_REGISTER(cvitdl_face_recognition, cvitdl_vi_fr, cvitdl_vi_fr);

//check face feature similarity
int cvitdl_read_check_two_face_sim(int argc, char *argv[]) {

  cvitdl_handle_t ai_handle = NULL;

  int ret = CVI_TDL_CreateHandle2(&ai_handle, 1, 0);
  const char *FD_MODEL_PATH = SD_FATFS_MOUNTPOINT"/retinaface_mnet0.25_342_608.cvimodel";
  ret = CVI_TDL_OpenModel(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, FD_MODEL_PATH);
  if (ret != CVI_SUCCESS) {
    printf("Facelib open failed with %#x!\n", ret);
    return ret;
  }
  const char *FR_MODEL_PATH = SD_FATFS_MOUNTPOINT"/cviface-v5-s.cvimodel";
  ret = CVI_TDL_OpenModel(ai_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, FR_MODEL_PATH);
  if (ret != CVI_SUCCESS) {
    printf("Facelib open failed with %#x!\n", ret);
    return ret;
  }

  CVI_TDL_SetSkipVpssPreprocess(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, false);
  CVI_TDL_SetSkipVpssPreprocess(ai_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, false);
  

  VB_BLK blk_fr;
	VIDEO_FRAME_INFO_S frame;
  // if (CVI_SUCCESS != CVI_TDL_ReadImage(argv[1], &blk_fr, &frame, PIXEL_FORMAT_RGB_888)){
  //   LOGE(AI_TAG, "cvitdl read image failed.");
  //   CVI_TDL_DestroyHandle(ai_handle);
  // }
  printf("to read image\n");
  if (CVI_SUCCESS != CVI_TDL_LoadBinImage(argv[1], &blk_fr, &frame, PIXEL_FORMAT_RGB_888_PLANAR)){
    LOGE(AI_TAG, "cvitdl read image failed.");
    CVI_VB_ReleaseBlock(blk_fr);
    CVI_TDL_DestroyHandle(ai_handle);
  }
  cvtdl_face_t face_meta;
  memset(&face_meta, 0, sizeof(cvtdl_face_t));
  printf("image read done,w:%d\n",frame.stVFrame.u32Width);
  CVI_TDL_RetinaFace(ai_handle, &frame, &face_meta);
  printf("face meta: size[%u], width[%u], height[%u], rescale_type[%d]\n", 
         face_meta.size, face_meta.width, face_meta.height, face_meta.rescale_type);
  for (uint32_t i = 0; i < face_meta.size; ++i) {
    printf("face[%u]: x1[%.2f], x1[%.2f], x2[%.2f], y2[%.2f]\n", face_meta.size,
           face_meta.info[i].bbox.x1, face_meta.info[i].bbox.y1,
           face_meta.info[i].bbox.x2, face_meta.info[i].bbox.y2);
  }


  CVI_TDL_FaceRecognition(ai_handle, &frame, &face_meta);
  float score = 0;
  if(face_meta.size == 2){
    if (CVI_SUCCESS != cal_cos_sim(&face_meta.info[0].feature, &face_meta.info[1].feature, &score)) {
            score = -1.;
    }
    printf("similairity[%.4f]\n", score);
  }else{
    printf("only got %u faces,expect 2\n",face_meta.size);
  }
  float *p_sim_info = NULL;
  if(face_meta.size>0){
    p_sim_info = (float*)malloc(face_meta.size*sizeof(float));
    for(int i = 0; i < face_meta.size; i++){
      p_sim_info[i] = score;
    }
  }
  dump_face_sim_info("/mnt/sd/fr_result.txt",&face_meta,p_sim_info);
  if(p_sim_info)free(p_sim_info);
  CVI_TDL_Free(&face_meta);
  CVI_TDL_DestroyHandle(ai_handle);
  return ret;
}
ALIOS_CLI_CMD_REGISTER(cvitdl_read_check_two_face_sim, cvitdl_read_check_fr, cvitdl_read_check_fr);


//extract a face feature to /mnt/sd/gallery.bin
int cvitdl_dump_feature(int argc, char *argv[]) {

  cvitdl_handle_t ai_handle = NULL;

  int ret = CVI_TDL_CreateHandle2(&ai_handle, 1, 0);
  const char *FD_MODEL_PATH = SD_FATFS_MOUNTPOINT"/retinaface_mnet0.25_342_608.cvimodel";
  ret = CVI_TDL_OpenModel(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, FD_MODEL_PATH);
  if (ret != CVI_SUCCESS) {
    printf("Facelib open failed with %#x!\n", ret);
    return ret;
  }
  const char *FR_MODEL_PATH = SD_FATFS_MOUNTPOINT"/cviface-v5-s.cvimodel";
  ret = CVI_TDL_OpenModel(ai_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, FR_MODEL_PATH);
  if (ret != CVI_SUCCESS) {
    printf("Facelib open failed with %#x!\n", ret);
    return ret;
  }

  CVI_TDL_SetSkipVpssPreprocess(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, false);
  CVI_TDL_SetSkipVpssPreprocess(ai_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, false);

  // float threshold = (float)atof(argv[2]);
  // CVI_TDL_SetModelThreshold(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, threshold);
  // CVI_TDL_GetModelThreshold(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, &threshold);
  // printf("CVI_TDL_SUPPORTED_MODEL_RETINAFACE threshold: %f !!!!!!!!!\n", threshold);

  VB_BLK blk_fr;
	VIDEO_FRAME_INFO_S frame;

  if (CVI_SUCCESS != CVI_TDL_LoadBinImage(argv[1], &blk_fr, &frame, PIXEL_FORMAT_RGB_888)){
    LOGE(AI_TAG, "cvitdl read image failed.");
    CVI_VB_ReleaseBlock(blk_fr);
    CVI_TDL_DestroyHandle(ai_handle);
  }
  cvtdl_face_t face_meta;
  memset(&face_meta, 0, sizeof(cvtdl_face_t));
  printf("image read done,w:%d\n",frame.stVFrame.u32Width);
  CVI_TDL_RetinaFace(ai_handle, &frame, &face_meta);
  printf("face meta: size[%u], width[%u], height[%u], rescale_type[%d]\n", 
         face_meta.size, face_meta.width, face_meta.height, face_meta.rescale_type);
  // for (uint32_t i = 0; i < face_meta.size; ++i) {
  
  //   printf("face[%u]: x1[%.2f], y1[%.2f], x2[%.2f], y2[%.2f]\n", i,
  //          face_meta.info[i].bbox.x1, face_meta.info[i].bbox.y1,
  //          face_meta.info[i].bbox.x2, face_meta.info[i].bbox.y2);

  //   printf("pts[%u]: x0[%.2f], y0[%.2f], x1[%.2f], y1[%.2f], x2[%.2f], y2[%.2f], x3[%.2f], y3[%.2f],x4[%.2f], y4[%.2f]\n", i,
  //          face_meta.info[i].pts.x[0], face_meta.info[i].pts.y[0],
  //          face_meta.info[i].pts.x[1], face_meta.info[i].pts.y[1],
  //          face_meta.info[i].pts.x[2], face_meta.info[i].pts.y[2],
  //          face_meta.info[i].pts.x[3], face_meta.info[i].pts.y[3],
  //          face_meta.info[i].pts.x[4], face_meta.info[i].pts.y[4]);
  // }


  CVI_TDL_FaceRecognition(ai_handle, &frame, &face_meta);
  
  if(face_meta.size == 1){
    const uint8_t*ptr = (const uint8_t*)face_meta.info[0].feature.ptr;
    dump_buffer_to_file("/mnt/sd/gallery.bin",ptr,face_meta.info[0].feature.size);
    printf("dump feat,size:%u,type:%d\n",face_meta.info[0].feature.size,face_meta.info[0].feature.type);
  }else{
    printf("only got %u faces,expect 1\n",face_meta.size);
  }

  CVI_TDL_Free(&face_meta);
  CVI_TDL_DestroyHandle(ai_handle);
  return ret;
}
ALIOS_CLI_CMD_REGISTER(cvitdl_dump_feature, cvitdl_dump_feature, cvitdl_dump_feature);


int cvitdl_dump_feature_dir(int argc, char *argv[]) {

  cvitdl_handle_t ai_handle = NULL;

  int ret = CVI_TDL_CreateHandle2(&ai_handle, 1, 0);
  const char *FD_MODEL_PATH = SD_FATFS_MOUNTPOINT"/retinaface_mnet0.25_342_608.cvimodel";
  ret = CVI_TDL_OpenModel(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, FD_MODEL_PATH);
  if (ret != CVI_SUCCESS) {
    printf("Facelib open failed with %#x!\n", ret);
    return ret;
  }
  const char *FR_MODEL_PATH = SD_FATFS_MOUNTPOINT"/cviface-v5-s.cvimodel";
  ret = CVI_TDL_OpenModel(ai_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, FR_MODEL_PATH);
  if (ret != CVI_SUCCESS) {
    printf("Facelib open failed with %#x!\n", ret);
    return ret;
  }

  CVI_TDL_SetSkipVpssPreprocess(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, false);
  CVI_TDL_SetSkipVpssPreprocess(ai_handle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, false);

  // float threshold = (float)atof(argv[2]);
  // CVI_TDL_SetModelThreshold(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, threshold);
  // CVI_TDL_GetModelThreshold(ai_handle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, &threshold);
  // printf("CVI_TDL_SUPPORTED_MODEL_RETINAFACE threshold: %f !!!!!!!!!\n", threshold);


  const char *list_path = SD_FATFS_MOUNTPOINT"/bin_list.txt";

  char strLine[100];

  FILE *fp;
  if ((fp = fopen(list_path, "r")) == NULL)
  {
      printf("Error! opening file");
      return -1;         
  }  

  while (!feof(fp))
  {
    fgets(strLine, 100, fp);

    char pic_path[128];
    sprintf(pic_path,"/mnt/sd/img_bin/%s",strLine);
 
    VB_BLK blk_fr;
	  VIDEO_FRAME_INFO_S frame;
    printf("processing %s ", pic_path);

    if (CVI_SUCCESS != CVI_TDL_LoadBinImage(pic_path, &blk_fr, &frame, PIXEL_FORMAT_RGB_888)){
      LOGE(AI_TAG, "cvitdl read image failed.");
      CVI_VB_ReleaseBlock(blk_fr);
      CVI_TDL_DestroyHandle(ai_handle);
    }

    cvtdl_face_t face_meta;
    memset(&face_meta, 0, sizeof(cvtdl_face_t));
    printf("image read done,w:%d\n",frame.stVFrame.u32Width);
    CVI_TDL_RetinaFace(ai_handle, &frame, &face_meta);
    printf("face meta: size[%u], width[%u], height[%u], rescale_type[%d]\n", 
          face_meta.size, face_meta.width, face_meta.height, face_meta.rescale_type);

    CVI_TDL_FaceRecognition(ai_handle, &frame, &face_meta);
    
    if(face_meta.size == 1){
      const uint8_t*ptr = (const uint8_t*)face_meta.info[0].feature.ptr;
      char save_path[128];
      sprintf(save_path,"/mnt/sd/gallery_bin/%s",strLine);

      dump_buffer_to_file(save_path, ptr,face_meta.info[0].feature.size);
      printf("dump feat,size:%u,type:%d\n",face_meta.info[0].feature.size,face_meta.info[0].feature.type);
    }else{
      printf("only got %u faces,expect 1\n",face_meta.size);
    }

    CVI_TDL_Free(&face_meta); 
    CVI_VB_ReleaseBlock(blk_fr); 
  }

  CVI_TDL_DestroyHandle(ai_handle);
  return ret;

}

ALIOS_CLI_CMD_REGISTER(cvitdl_dump_feature_dir, cvitdl_dump_feature_dir, cvitdl_dump_feature_dir);



int cvitdl_feature_compare(int argc, char *argv[]){

  cvtdl_feature_t gallery;
  CVI_S32 s32Ret = CVI_TDL_SUCCESS;


  s32Ret = load_feature_from_file(argv[1],&gallery,256);
  if(s32Ret !=0){
    aos_free(gallery.ptr);
    printf("load file failed\n");
    return s32Ret;
  }else{
    printf("gallery loaded successfully\n");
  }  

  const char *list_path = SD_FATFS_MOUNTPOINT"/bin_list.txt";

  char strLine[100];

  FILE *fp;
  if ((fp = fopen(list_path, "r")) == NULL)
  {
      printf("Error! opening file");
      return -1;         
  }  

  while (!feof(fp))
  {
    fgets(strLine, 100, fp);
    strLine[strlen(strLine)]='\0';

    char fea_path[128];
    sprintf(fea_path,"/mnt/sd/gallery_bin/%s",strLine);

    float score = 0;
    cvtdl_feature_t feature;
    s32Ret = load_feature_from_file(fea_path,&feature,256);
    if(s32Ret !=0){
      aos_free(feature.ptr);
      printf("load %s failed\n", fea_path);
      return s32Ret;
    }

    if(cal_cos_sim(&feature,&gallery,&score)==0){
      printf("matched score:%f,     bin:%s",score,strLine);
    } 

    aos_free(feature.ptr); 

  }
  fclose(fp);
  aos_free(gallery.ptr); 

  return s32Ret;
} 

ALIOS_CLI_CMD_REGISTER(cvitdl_feature_compare, cvitdl_feature_compare, cvitdl_feature_compare);


