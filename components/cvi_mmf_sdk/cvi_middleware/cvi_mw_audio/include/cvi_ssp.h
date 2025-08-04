#ifndef __CVI_SSP_H__
#define __CVI_SSP_H__

#include "cvi_type.h"
CVI_S32 cvi_audio_ssp_open(CVI_VOID * handle, CVI_U64 * addr);
CVI_S32 cvi_audio_ssp_close(CVI_VOID *handle);

#endif