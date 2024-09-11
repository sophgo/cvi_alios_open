#ifndef __SHARE_BUFFER_H__
#define __SHARE_BUFFER_H__
#include <stdio.h>
#include <stdint.h>

int share_cyclebuffer_init(uint32_t length, int bClient);
int share_cyclebuffer_client_write(uint32_t index, char *data, uint32_t writeLen, int32_t msecs);
int share_cyclebuffer_server_read(uint32_t index, char  *outbuf, int readLen);
int share_cyclebuffer_client_read(uint32_t index, char  *outbuf, int readLen, int32_t msecs);
int share_cyclebuffer_server_write(uint32_t index, char *data, uint32_t writeLen);
int share_cyclebuffer_frameready_size(int index);
int share_cyclebuffer_clear(int index);
void share_cyclebuffer_destroy(int index);
int share_cyclebuffer_total_buf_size(int index);

#endif //__SHARE_BUFFER_H__
