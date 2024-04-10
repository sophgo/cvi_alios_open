#ifndef __AUDIO_DETECT_H__
#define __AUDIO_DETECT_H__
#include "cvi_tdl.h"
#include "cvitdl_types.h"
#include "stdint.h"

typedef struct {
	u32 u32AudFormatSize;
	u32 u32Second;
	u32 u32PeriodSize;
	u32 u32SampleRate;
	u32 u32FrameSize;
	u32 u32CaptureSize;
	u32 u32FrameNum;
    u32 u32ChnNum;
	bool bRecord;           // whether record to output
	bool bStaticAudioData;  // Whether use static data for audio detect
} cvitdl_aud_t;

typedef struct {
	c8* pc8ModelPath;      // AI model path
	c8* pc8AudInputPath;   // Audio input path
	c8* pc8AudOutputPath;  // Audio output path
} cvitdl_path_t;

typedef struct {
	u32 u32FuncSelect;     // Audio detect function selection
    cvitdl_handle_t pstHandle;
	cvitdl_path_t stPath;  // File path needed by audio detect
	cvitdl_aud_t stAud;    // Aud configuration
} cvitdl_aud_detect_info_t;

void cvitdl_aud_detect(int32_t argc, char** argv);

#endif /* __AUDIO_DETECT_H__ */