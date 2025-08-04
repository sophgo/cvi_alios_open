#ifndef __CVI_PARAM_H__
#define __CVI_PARAM_H__

#include "custom_param.h"

typedef struct _PARTITION_CHECK_HAED_S
{
    unsigned int magic_number;
    unsigned int crc;
    unsigned int header_ver;
    unsigned int length;
    unsigned int reserved;
    unsigned int package_number;
    unsigned int package_length[10];
}PARTITION_CHECK_HAED_S;

//多级别结构 确定使用的多级数组加number作标定 (多级限定场景 串行结构 不允许存在同一级别有多个多级存在) 大结构必须精简尽量使用指针

CVI_S32 PARAM_LoadCfg(CVI_VOID);
PARAM_MODULE_CFG_S *PARAM_getModuleCtx(void);
PARAM_SYS_CFG_S *PARAM_getSysCtx(void);
#if CONFIG_SUPPORT_VI
PARAM_VI_CFG_S *PARAM_getViCtx(void);
#endif
PARAM_VPSS_CFG_S *PARAM_getVpssCtx(void);
PARAM_VENC_CFG_S *PARAM_getVencCtx(void);
PARAM_VO_CFG_S *PARAM_getVoCtx(void);
void PARAM_setPipeline(int pipeline);
int PARAM_getPipeline(void);
void PARAM_setSceneMode(int mode);
int PARAM_getSceneMode();
int PARAM_Reinit_RawReplay(void);
int _param_check_head(unsigned char * buffer);

#endif
