#include <aos/kernel.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <vfs.h>

#include "aos/cli.h"
#include "aud_detect.h"
#include "cvi_tdl.h"
#include "cvi_tpu_interface.h"
#include "cviaudio_algo_interface.h"
#include "cvitdl_types.h"
#include "media_audio.h"

// ESC class name
static const c8* enumBabyCryDetectStr[] = {"no_baby_cry", "baby_cry"};
static const c8* enumOrderDetectStr[] = {"无指令", "小爱小爱", "拨打电话", "关闭屏幕", "打开屏幕"};
enum enumAudFunc {
	CVITDL_AUD_CRY_DETECT,
	CVITDL_AUD_ORDER_DETECT,
	CVITDL_AUD_FUNC_LIMIT,
};

/**
 * @brief  Get Video frame, record or hanlde it with AI model
 * @note
 * @param  argv: the arguments transfered by call function
 * @retval None
 */
void* cvitdl_aud_detect_handler(void* argv) {
	cvitdl_aud_detect_info_t* pstInfo = (cvitdl_aud_detect_info_t*)argv;
	s32 s32Ret;
	cvitdl_aud_t* pstAud = &pstInfo->stAud;
	cvitdl_path_t* pstPath = &pstInfo->stPath;
	u32 u32Size = pstAud->u32PeriodSize * pstAud->u32AudFormatSize;

	/* Malloc audio capture buffer */
	u8* pu8CapBuf = (u8*)malloc(pstAud->u32CaptureSize);
	if (!pu8CapBuf) {
		printf("malloc failed\n");
		return NULL;
	}
	printf("malloc capture buffer pass\n");

	/* Set video frame interface */
	u8* pu8Buf = (u8*)malloc(pstAud->u32FrameSize);
	if (!pu8Buf) {
		printf("malloc failed\n");
		return NULL;
	}
	printf("malloc frame buffer pass\n");

	memset(pu8Buf, 0, pstAud->u32FrameSize);
	VIDEO_FRAME_INFO_S stFrame;
	stFrame.stVFrame.pu8VirAddr[0] = pu8Buf;  // Global buffer
	stFrame.stVFrame.u32Height = 1;
	stFrame.stVFrame.u32Width = pstAud->u32FrameSize;

	/* Classify the sound result */
	s32 s32Index = -1;

	/* Audio loop detection */
	while (1) {
		/* Load audio data from file or MIC */
		if (pstAud->bStaticAudioData) {
			FILE* fp = fopen(pstPath->pc8AudInputPath, "rb");

			s32Ret = fread(pu8Buf, 1, pstAud->u32FrameSize, fp);
			if (s32Ret == 0) {
				printf("read aud data failed\n");
				break;
			}
			printf("read aud data pass\n");
			fclose(fp);
		} else {
                memset(pu8Buf, 0, pstAud->u32CaptureSize);
				s32Ret = MEDIA_AUDIO_PcmRead(pu8CapBuf);
				if (s32Ret > 0) {
					/* Slide window for audio update & Convert two-channel data to one channel */
					memmove(pu8Buf, pu8Buf + u32Size, pstAud->u32FrameSize - u32Size);
					memcpy(pu8Buf + pstAud->u32FrameSize - u32Size, pu8CapBuf, s32Ret / 2);
				} else {
					printf("MEDIA_AUDIO_PcmRead failed\n");
					continue;
				}
		}
		/* We can't record and handle audio data at the same time currently */
		if (pstAud->bRecord) {
			/* Record the audio */
            printf("start record audio\n");
			FILE* fp = fopen(pstInfo->stPath.pc8AudOutputPath, "wb");
			if (fp == NULL) {
				printf("open file %s failed\n", pstPath->pc8AudOutputPath);
			} else {
				printf("to write:%s\n", pstPath->pc8AudOutputPath);
				fwrite((c8*)pu8Buf, 1, pstAud->u32FrameSize, fp);
				printf("file write finished\n");
				fclose(fp);
				printf("file close finished\n");
			}
			/* After record the audio, we should break out of the loop */
			break;
		} else {
			/* Detect the audio */
			s32 s32Ret = CVI_TDL_SoundClassification_V2(pstInfo->pstHandle, &stFrame, &s32Index);
			if (s32Ret == CVI_SUCCESS) {
				switch (pstInfo->u32FuncSelect) {
				case CVITDL_AUD_CRY_DETECT:
					printf("esc class: %s\n", enumBabyCryDetectStr[s32Index]);
					break;
				case CVITDL_AUD_ORDER_DETECT:
					printf("esc class: %s\n", enumOrderDetectStr[s32Index]);
					break;
				default:
					printf("Aud Detect don't support func %d currently\n", pstInfo->u32FuncSelect);
					break;
				}
			} else {
				printf("detect failed\n");
			}
		}
	}

	pthread_exit(NULL);
	return NULL;
}

/**
 * @brief Initialize the audio capture
 * @note
 * * We don't support init audio in this func currently,
 * * the parameters should be synced to media_audio.c
 * @param  pstAud: Audio information
 * @param  u32FuncSelect: Decided which attribute configutation to be used
 * @retval None
 */
void cvitdl_aud_init(cvitdl_aud_t* pstAud, u32 u32FuncSelect) {
	if (pstAud == NULL) {
		printf("pstAud is NULL\n");
		return;
	}

	/* Common configuration */
    pstAud->u32ChnNum = 1;
	pstAud->u32AudFormatSize = 2;  // PCM_FORMAT_S16_LE (2bytes)
	pstAud->u32Second = 2;
	pstAud->u32CaptureSize = 4096;
	pstAud->bStaticAudioData = false;

	switch (u32FuncSelect) {
	case CVITDL_AUD_CRY_DETECT:
		pstAud->u32PeriodSize = 320;
		pstAud->u32SampleRate = 16000;
		pstAud->u32FrameNum = 4;
		break;
	case CVITDL_AUD_ORDER_DETECT:
		pstAud->u32PeriodSize = 640;
		pstAud->u32SampleRate = 8000;
		pstAud->u32FrameNum = 10;
		break;
	default:
		printf("Aud Detect don't support func %d currently\n", u32FuncSelect);
		return;
		break;
	}
	pstAud->u32FrameSize = pstAud->u32SampleRate * pstAud->u32AudFormatSize * pstAud->u32Second;
	return;
}

/**
 * @brief  Parse the parameters passed in by user
 * @note
 * @param  s32Argc: the number of command line arguments
 * @param  ppc8Argv: the command line arguments
 * @param  pstInfo: Informations needed by audio detect
 * @retval return CVI_SUCCESS if parse argv successfully
 */
s32 cvitdl_parse_argv(s32 s32Argc, c8** ppc8Argv, cvitdl_aud_detect_info_t* pstInfo) {
	/* Help description */
	if (s32Argc < 2 || s32Argc > 6) {
		printf(
		  "Usage: %s FUNC_SELECT [IS_RECORD] [MODEL_PATH] [INPUT_AUD_PATH] [OUTPUT_AUD_PATH]\n"
		  "\t FUNC_SELECT,      \t select audio detect function.\n"
		  "\t                   \t 0: Baby Cry Detect\n"
		  "\t                   \t 1: Key Word Detect\n"
		  "\t IS_RECORD,        \t record input to file, 0: disable 1: enable.\n"
		  "\t MODEL_PATH,       \t specfic load ai model path.\n"
		  "\t INPUT_AUD_PATH,   \t read audio data from file.\n"
		  "\t OUTPUT_AUD_PATH,  \t write audio data captured from MIC into file.\n",
		  ppc8Argv[0]);
		return CVI_FAILURE;
	}
	cvitdl_path_t* pstPath = &pstInfo->stPath;

	pstPath->pc8ModelPath = "/mnt/sd/aud_test/sound_task1_sr8k_v24_cv181x.cvimodel";
	// pstPath->pc8ModelPath = "/mnt/sd/aud_test/baby_cry_sr16k_v11_cv181x.cvimodel";
	pstPath->pc8AudInputPath = NULL;   // input file path
	pstPath->pc8AudOutputPath = NULL;  // output file
	pstInfo->stAud.bRecord = false;

	/* Load input arguments dynamically */
	for (s32 i = 1; i < s32Argc; ++i) {
		switch (i) {
		case 1:
			pstInfo->u32FuncSelect = atoi(ppc8Argv[i]);
			if (pstInfo->u32FuncSelect >= CVITDL_AUD_FUNC_LIMIT) {
				printf("Aud Detect don't support func %d currently\n", pstInfo->u32FuncSelect);
				return CVI_FAILURE;
			}
			cvitdl_aud_init(&pstInfo->stAud, pstInfo->u32FuncSelect);
			break;
		case 2:
			pstInfo->stAud.bRecord = atoi(ppc8Argv[i]) ? true : false;
			break;
		case 3:
			pstPath->pc8ModelPath = ppc8Argv[i];
			break;
		case 4:
			pstPath->pc8AudInputPath = ppc8Argv[i];
			break;
		case 5:
			pstPath->pc8AudOutputPath = ppc8Argv[i];
			break;
		default:
			break;
		}
	}
	return CVI_SUCCESS;
}

/**
 * @brief Load cvimodel and detect audio
 * @note
 * @param  s32Argc: the number of command line arguments
 * @param  ppc8Argv: the command line arguments
 * @retval None
 */
void cvitdl_aud_detect(s32 s32Argc, c8** ppc8Argv) {
	cvitdl_aud_detect_info_t stInfo;

	if (cvitdl_parse_argv(s32Argc, ppc8Argv, &stInfo) != CVI_SUCCESS) {
		printf("Current input variables are invalid!\n");
		return;
	}

	s32 s32Ret = CVI_SUCCESS;

	stInfo.pstHandle = NULL;
	s32Ret = CVI_TDL_CreateHandle3(&stInfo.pstHandle);
	if (s32Ret != CVI_SUCCESS) {
		printf("Create tdl handle failed with %#x!\n", s32Ret);
		return;
	}

	cvi_tpu_init();
	aos_msleep(1000);

	/* Open sound classification model */
	s32Ret = CVI_TDL_OpenModel(stInfo.pstHandle, CVI_TDL_SUPPORTED_MODEL_SOUNDCLASSIFICATION_V2,
							   stInfo.stPath.pc8ModelPath);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TDL_DestroyHandle(stInfo.pstHandle);
		printf("open model failed with %#x!\n", s32Ret);
		return;
	}

	/* Be used to performance evaluation */
	CVI_TDL_SetPerfEvalInterval(stInfo.pstHandle, CVI_TDL_SUPPORTED_MODEL_SOUNDCLASSIFICATION_V2,
								10);

	/* Create thread for handling audio detection */
	pthread_attr_t attr;
	pthread_t pcm_output_thread;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192000);
	pthread_create(&pcm_output_thread, &attr, cvitdl_aud_detect_handler, (void*)&stInfo);
	pthread_setname_np(pcm_output_thread, "cvitdl_aud_detect_handler");
	pthread_join(pcm_output_thread, NULL);

	CVI_TDL_DestroyHandle(stInfo.pstHandle);

	return;
}
ALIOS_CLI_CMD_REGISTER(cvitdl_aud_detect, cvitdl_aud_detect, cvitdl_aud_detect);
