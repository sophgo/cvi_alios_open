#ifndef __MEDIA_AUDIO_H__
#define __MEDIA_AUDIO_H__

#define PERIOD_FRAMES_SIZE 160

enum MEDIA_PCM_TYPE {
	MEDIA_PCM_TYPE_MIC,
	MEDIA_PCM_TYPE_SPEAKER,
	MEDIA_PCM_TYPE_BUTT,
};

int MEDIA_AUDIO_PcmRead(unsigned char *buf);
int MEDIA_AUDIO_PcmWrite(const unsigned char *buf,int len);
int MEDIA_AUDIO_GetPcmLen(enum MEDIA_PCM_TYPE type);
int MEDIA_AUDIO_AlgoIsEnable(void);
void* MEDIA_AUDIO_AlgoHandle(void);
int MEDIA_AUDIO_Init();
int MEDIA_AUDIO_DeInit();




#endif
