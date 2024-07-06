#ifndef __CVI_TCP_SERVER_H__
#define __CVI_TCP_SERVER_H__
#include "cvi_type.h"
#include "cvi_comm_venc.h"

CVI_S32 cvi_tcp_server_init(const char *ip, int port);
CVI_S32 cvi_tcp_send(int client, const char *buf, const int len);
CVI_S32 cvi_tcp_server_deinit();

#endif
