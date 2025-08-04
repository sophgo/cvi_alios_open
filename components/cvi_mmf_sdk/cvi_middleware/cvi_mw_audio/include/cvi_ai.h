#ifndef __CVI_AI_H__
#define __CVI_AI_H__

#include "cvi_type.h"
CVI_S32 cvi_audio_ai_open(CVI_VOID *handle, CVI_U64 *addr);
CVI_S32 cvi_audio_ai_close(CVI_VOID *handle);
CVI_S32 cvi_audio_ai_setvol(CVI_U8 card_id, CVI_S32 volume);
CVI_S32 cvi_audio_ai_getvol(CVI_U8 card_id, CVI_S32 *volume);
CVI_S32 cvi_audio_ai_setstereovol(CVI_U8 card_id, CVI_S32 stereo_id, CVI_S32 volume);
CVI_S32 cvi_audio_ai_getstereovol(CVI_U8 card_id, CVI_S32 stereo_id, CVI_S32 *volume);
CVI_S32 cvi_audio_ai_setmute(CVI_U8 card_id, CVI_S32 enable);
CVI_S32 cvi_audio_ai_getmute(CVI_U8 card_id, CVI_S32 *enable);

#endif