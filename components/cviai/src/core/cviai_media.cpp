#include "core/cviai_media.h"
#include "cviai_log.hpp"

#include "core/utils/vpss_helper.h"
// #include "opencv2/opencv.hpp"

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb/stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb/stb_image_write.h"

// TODO: use memcpy
inline void BufferRGBPackedCopy(const uint8_t *buffer, uint32_t width, uint32_t height,
                                uint32_t stride, VIDEO_FRAME_INFO_S *frame, bool invert) {
  VIDEO_FRAME_S *vFrame = &frame->stVFrame;
  if (invert) {
    for (uint32_t j = 0; j < height; j++) {
      const uint8_t *ptr = buffer + j * stride;
      for (uint32_t i = 0; i < width; i++) {
        uint32_t offset = i * 3 + j * vFrame->u32Stride[0];
        const uint8_t *ptr_pxl = i * 3 + ptr;
        vFrame->pu8VirAddr[0][offset] = ptr_pxl[2];
        vFrame->pu8VirAddr[0][offset + 1] = ptr_pxl[1];
        vFrame->pu8VirAddr[0][offset + 2] = ptr_pxl[0];
      }
    }
  } else {
    for (uint32_t j = 0; j < height; j++) {
      const uint8_t *ptr = buffer + j * stride;
      for (uint32_t i = 0; i < width; i++) {
        uint32_t offset = i * 3 + j * vFrame->u32Stride[0];
        const uint8_t *ptr_pxl = i * 3 + ptr;
        vFrame->pu8VirAddr[0][offset] = ptr_pxl[0];
        vFrame->pu8VirAddr[0][offset + 1] = ptr_pxl[1];
        vFrame->pu8VirAddr[0][offset + 2] = ptr_pxl[2];
      }
    }
  }
}

inline void BufferRGBPacked2PlanarCopy(const uint8_t *buffer, uint32_t width, uint32_t height,
                                       uint32_t stride, VIDEO_FRAME_INFO_S *frame, bool invert) {
  VIDEO_FRAME_S *vFrame = &frame->stVFrame;
  if (invert) {
    for (uint32_t j = 0; j < height; j++) {
      const uint8_t *ptr = buffer + j * stride;
      for (uint32_t i = 0; i < width; i++) {
        const uint8_t *ptr_pxl = i * 3 + ptr;
        vFrame->pu8VirAddr[0][i + j * vFrame->u32Stride[0]] = ptr_pxl[2];
        vFrame->pu8VirAddr[1][i + j * vFrame->u32Stride[1]] = ptr_pxl[1];
        vFrame->pu8VirAddr[2][i + j * vFrame->u32Stride[2]] = ptr_pxl[0];
      }
    }
  } else {
    for (uint32_t j = 0; j < height; j++) {
      const uint8_t *ptr = buffer + j * stride;
      for (uint32_t i = 0; i < width; i++) {
        const uint8_t *ptr_pxl = i * 3 + ptr;
        vFrame->pu8VirAddr[0][i + j * vFrame->u32Stride[0]] = ptr_pxl[0];
        vFrame->pu8VirAddr[1][i + j * vFrame->u32Stride[1]] = ptr_pxl[1];
        vFrame->pu8VirAddr[2][i + j * vFrame->u32Stride[2]] = ptr_pxl[2];
      }
    }
  }
}

template <typename T>
inline void BufferC12C1Copy(const uint8_t *buffer, uint32_t width, uint32_t height, uint32_t stride,
                            VIDEO_FRAME_INFO_S *frame) {
  VIDEO_FRAME_S *vFrame = &frame->stVFrame;
  for (uint32_t j = 0; j < height; j++) {
    const uint8_t *ptr = buffer + j * stride;
    uint8_t *vframec0ptr = vFrame->pu8VirAddr[0] + j * vFrame->u32Stride[0];
    memcpy(vframec0ptr, ptr, width * sizeof(T));
  }
}

CVI_S32 CVI_AI_Buffer2VBFrame(const uint8_t *buffer, uint32_t width, uint32_t height,
                              uint32_t stride, const PIXEL_FORMAT_E inFormat, VB_BLK *blk,
                              VIDEO_FRAME_INFO_S *frame, const PIXEL_FORMAT_E outFormat) {
  if (CREATE_VBFRAME_HELPER(blk, frame, width, height, outFormat) != CVI_SUCCESS) {
    LOGE(AISDK_TAG, "Create VBFrame failed.\n");
    return CVI_FAILURE;
  }

  int ret = CVI_SUCCESS;
  if ((inFormat == PIXEL_FORMAT_RGB_888 && outFormat == PIXEL_FORMAT_BGR_888) ||
      (inFormat == PIXEL_FORMAT_BGR_888 && outFormat == PIXEL_FORMAT_RGB_888)) {
    BufferRGBPackedCopy(buffer, width, height, stride, frame, true);
  } else if ((inFormat == PIXEL_FORMAT_RGB_888 && outFormat == PIXEL_FORMAT_RGB_888) ||
             (inFormat == PIXEL_FORMAT_BGR_888 && outFormat == PIXEL_FORMAT_BGR_888)) {
    BufferRGBPackedCopy(buffer, width, height, stride, frame, false);
  } else if (inFormat == PIXEL_FORMAT_BGR_888 && outFormat == PIXEL_FORMAT_RGB_888_PLANAR) {
    BufferRGBPacked2PlanarCopy(buffer, width, height, stride, frame, true);
  } else if (inFormat == PIXEL_FORMAT_RGB_888 && outFormat == PIXEL_FORMAT_RGB_888_PLANAR) {
    BufferRGBPacked2PlanarCopy(buffer, width, height, stride, frame, false);
  } else if (inFormat == PIXEL_FORMAT_BF16_C1 && outFormat == PIXEL_FORMAT_BF16_C1) {
    BufferC12C1Copy<uint16_t>(buffer, width, height, stride, frame);
  } else if (inFormat == PIXEL_FORMAT_FP32_C1 && outFormat == PIXEL_FORMAT_FP32_C1) {
    BufferC12C1Copy<float>(buffer, width, height, stride, frame);
  } else {
    LOGE(AISDK_TAG, "Unsupported convert format: %u -> %u.\n", inFormat, outFormat);
    ret = CVI_FAILURE;
  }
  // CACHED_VBFRAME_FLUSH_UNMAP(frame);
  return ret;
}

CVI_S32 CVI_AI_ReadImage(const char *filepath, VB_BLK *blk, VIDEO_FRAME_INFO_S *frame,
                         PIXEL_FORMAT_E format) {
  if (format != PIXEL_FORMAT_RGB_888) {
    LOGE(AISDK_TAG, "Pixel format [%d] is not supported.\n", format);
    return CVI_FAILURE;
  }
  return CVI_FAILURE;
  // int width = 0, height = 0, nChannels = 0;
  // stbi_uc *stbi_data = stbi_load(filepath, &width, &height, &nChannels, STBI_rgb);
  // // cv::Mat img = cv::imread(filepath);
  // // if (img.empty()) {
  // if (stbi_data == nullptr) {
  //   LOGE(AISDK_TAG, "Cannot read image %s.\n", filepath);
  //   return CVI_FAILURE;
  // }
  // int stride = width * 3;
  // // stbi_write_png("/mnt/sd/stbi_input_image.png", width, height, STBI_rgb, stbi_data, stride);
  // if (CREATE_VBFRAME_HELPER(blk, frame, width, height, format) != CVI_SUCCESS) {
  //   LOGE(AISDK_TAG, "Create VBFrame failed.\n");
  //   return CVI_FAILURE;
  // }

  // int ret = CVI_SUCCESS;
  // switch (format) {
  //   case PIXEL_FORMAT_RGB_888: {
  //     // BufferRGBPackedCopy(img.data, img.cols, img.rows, img.step, frame, true);
  //     BufferRGBPackedCopy(stbi_data, width, height, stride, frame, false);
  //   } break;
  //   case PIXEL_FORMAT_RGB_888_PLANAR: {
  //     BufferRGBPacked2PlanarCopy(stbi_data, width, height, stride, frame, false);
  //   } break;
  //   default:
  //     LOGE(AISDK_TAG, "Unsupported format: %u.\n", format);
  //     ret = CVI_FAILURE;
  //     break;
  // }
  // CACHED_VBFRAME_FLUSH_UNMAP(frame);
  // return ret;
}


DLL_EXPORT CVI_S32 CVI_AI_LoadBinImage(const char *filepath, VB_BLK *blk, VIDEO_FRAME_INFO_S *frame,
                                    PIXEL_FORMAT_E format){
  // if(frame->stVFrame.enPixelFormat != PIXE)
   if (format != PIXEL_FORMAT_RGB_888_PLANAR ) {
    LOGE(AISDK_TAG, "Pixel format [%d] is not supported.\n", format);
    return CVI_FAILURE;
  }
  
  FILE *fp = fopen(filepath,"rb");
  if(fp == nullptr){
    LOGE(AISDK_TAG, "failed to open: %s.\n", filepath);
    return CVI_FAILURE;
  }
  int width,height;

  fseek(fp,0,SEEK_END);
  long len = ftell(fp);
  fseek(fp,0,SEEK_SET);
  fread(&width,sizeof(uint32_t),1,fp);
  fread(&height,sizeof(uint32_t),1,fp);
  long left_len = len - 8;
  uint8_t *p_data = new uint8_t[left_len];
  fread(p_data,left_len,1,fp);
  fclose(fp);
  LOGI(AISDK_TAG,"load bin image,width:%d,height:%d,imagesize:%ld\n",width,height,left_len);

  // stbi_write_png("/mnt/sd/stbi_input_image.png", width, height, STBI_rgb, stbi_data, stride);
  if (CREATE_VBFRAME_HELPER(blk, frame, width, height, format) != CVI_SUCCESS) {
    LOGE(AISDK_TAG, "Create VBFrame failed.\n");
    delete []p_data;
    return CVI_FAILURE;
  }

  int ret = CVI_SUCCESS;
  int len_per_plane = left_len/3;
  for(int i = 0; i < 3; i ++){
    const uint8_t *p_plane_src = p_data + i*len_per_plane;
    memcpy(frame->stVFrame.pu8VirAddr[i],p_plane_src,len_per_plane);
  }

  return ret;
}
