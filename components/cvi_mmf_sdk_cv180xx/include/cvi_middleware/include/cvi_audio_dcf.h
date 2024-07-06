#ifndef __CVITEK_DC_FILTER_H__
#define __CVITEK_DC_FILTER_H__

void *cvitek_dc_init(int channel, float radius);
int cvitek_dc_process(void *handle, float *data, int frames);
void cvitek_dc_deinit(void *handle);
#endif