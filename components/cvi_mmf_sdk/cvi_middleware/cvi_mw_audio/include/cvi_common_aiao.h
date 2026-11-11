#ifndef __CVI_COMMON_AIAO_H__
#define __CVI_COMMON_AIAO_H__

#include "cvi_type.h"
#include <alsa/pcm.h>
#include <pthread.h>
#include "cvi_datafifo.h"
typedef struct audio_card_info {
	CVI_U8 dev_id;
	AUDIO_CONFIG_INFO config;
	aos_pcm_t *pcmHandle;
	CVI_DATAFIFO_HANDLE dfHandle;
	CVI_DATAFIFO_PARAMS_S dfParam;
	pthread_t thread_id;
	CVI_BOOL ThreadRunning;
	CVI_S32 volIndex;
	CVI_S32 lvolIndex;
	CVI_S32 rvolIndex;
	CVI_S32 mute;
} AUDIO_CARD_HANDLE;

typedef struct _audio_ssp_handle {
	AUDIO_SSP_CONFIG_INFO sspinfo;
	CVI_DATAFIFO_HANDLE indfHandle;
	CVI_DATAFIFO_HANDLE outdfHandle;
	CVI_DATAFIFO_PARAMS_S indfParam;
	CVI_DATAFIFO_PARAMS_S outdfParam;
	CVI_BOOL ThreadRunning;
	pthread_t thread_id;
} AUDIO_SSP_HANDLE;
#endif
