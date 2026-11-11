#ifndef __CVI_AO_H__
#define __CVI_AO_H__

#include "cvi_type.h"
CVI_S32 cvi_audio_ao_open(CVI_VOID *handle);
CVI_S32 cvi_audio_ao_close(CVI_VOID *handle);
CVI_S32 cvi_audio_ao_setvol(CVI_U8 card_id, CVI_S32 volume);
CVI_S32 cvi_audio_ao_getvol(CVI_U8 card_id, CVI_S32 *volume);
CVI_S32 cvi_audio_ao_setstereovol(CVI_U8 card_id, CVI_S32 stereo_id, CVI_S32 volume);
CVI_S32 cvi_audio_ao_getstereovol(CVI_U8 card_id, CVI_S32 stereo_id, CVI_S32 *volume);
CVI_S32 cvi_audio_ao_setmute(CVI_U8 card_id, CVI_S32 enable);
CVI_S32 cvi_audio_ao_getmute(CVI_U8 card_id, CVI_S32 *enable);

#endif