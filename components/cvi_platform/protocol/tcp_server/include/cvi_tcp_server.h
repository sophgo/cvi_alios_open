#ifndef __CVI_TCP_SERVER_H__
#define __CVI_TCP_SERVER_H__
#include "cvi_type.h"
#include "cvi_comm_venc.h"

typedef enum {
    TCP_IMG_RGB = 1,
    TCP_IMG_IR,
} cvi_tcp_img_type_e;

CVI_S32 cvi_tcp_server_init(const char *ip, int port);
CVI_S32 cvi_tcp_send(cvi_tcp_img_type_e e_img_type, const uint8_t* pu8_buf,
                     const uint32_t u32_len);
CVI_S32 cvi_tcp_server_deinit();

#endif
