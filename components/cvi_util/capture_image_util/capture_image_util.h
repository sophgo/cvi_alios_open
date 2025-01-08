#ifndef __CVI_CAPTURE_IMAGE_UTIL_H__
#define __CVI_CAPTURE_IMAGE_UTIL_H__
#include "cvi_tcp_server.h"

void cvi_capture_image_set_num(uint8_t u8_rgb_num, uint8_t u8_ir_num);
int cvi_capture_image_init();
int32_t cvi_capture_image_set_buf(cvi_tcp_img_type_e e_img_type, const uint8_t* pu8_buf,
                                  const uint32_t u32_len);

#endif