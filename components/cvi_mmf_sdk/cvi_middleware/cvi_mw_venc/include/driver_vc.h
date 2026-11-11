#ifndef __DRIVER_VC_H__
#define __DRIVER_VC_H__

int driver_vc_init(void);
void driver_vc_deinit(void);
void driver_venc_release(void);
void driver_vdec_release(void);
int venc_ioctl(int ve_chn, unsigned int cmd,  void *arg);
int vdec_ioctl(int vd_chn, unsigned int cmd,  void *arg);

int venc_clk_enable(void);
int venc_clk_disable(void);
#endif
