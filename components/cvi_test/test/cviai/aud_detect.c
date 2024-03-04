#include <aos/kernel.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <vfs.h>

#include "aos/cli.h"
#include "cvi_tdl.h"
#include "cvi_tpu_interface.h"
#include "cviaudio_algo_interface.h"
#include "media_audio.h"

#define _GNU_SOURCE
#define AUDIO_FORMAT_SIZE 2
#define SECOND 3
#define CVI_AUDIO_BLOCK_MODE -1
#define PERIOD_SIZE 320
#define SAMPLE_RATE 16000
#define FRAME_SIZE SAMPLE_RATE *AUDIO_FORMAT_SIZE *SECOND  // PCM_FORMAT_S16_LE (2bytes) 3 seconds
#define CAPTURE_SIZE 4096

#define STATIC_AUDIO_DATA 0

// ESC class name
enum Classes { NO_BABY_CRY, BABY_CRY };
static const char *enumStr[] = {"no_baby_cry", "baby_cry"};

bool record = false;                                 // record to output
char outpath[64] = "/mnt/sd/aud_test/dump_aud.raw";  // output file path

// Init cvi_tdl handle.
cvitdl_handle_t tdl_handle = NULL;

/**
 * @brief  Get Video frame and set it to global buffer
 * @note
 * @param  argv: the arguments transfered by call function
 * @retval None
 */
void *uplink_audio(void *argv) {
	CVI_S32 s32Ret;

	int loop = SAMPLE_RATE / PERIOD_SIZE * SECOND;  // 3 seconds
	int size = PERIOD_SIZE * AUDIO_FORMAT_SIZE;     // PCM_FORMAT_S16_LE (2bytes)

	/* Malloc audio capture buffer */
	CVI_U8 *cap_buffer = (CVI_U8 *)malloc(CAPTURE_SIZE);
	if (!cap_buffer) {
		printf("malloc failed\n");
		return NULL;
	}

	/* Set video frame interface */
	CVI_U8 *buffer = (CVI_U8 *)malloc(FRAME_SIZE);  // 3 seconds
	if (!buffer) {
		printf("malloc failed\n");
		return NULL;
	}
	memset(buffer, 0, FRAME_SIZE);
	VIDEO_FRAME_INFO_S Frame;
	Frame.stVFrame.pu8VirAddr[0] = buffer;  // Global buffer
	Frame.stVFrame.u32Height = 1;
	Frame.stVFrame.u32Width = FRAME_SIZE;

	/* Classify the sound result */
	int index = -1;

    /* Audio loop detection */
	while (1) {
        /* Load audio data from file or MIC */
		if (STATIC_AUDIO_DATA == 1) {
			FILE *fd = fopen("/mnt/sd/aud_test/V_2017-04-01+08_06_22=0_30_mp3_21_0.bin", "rb");
			s32Ret = fread(buffer, 1, FRAME_SIZE, fd);
			if (s32Ret == 0) {
				printf("read aud data failed\n");
				break;
			}
			fclose(fd);
		} else {
			for (int i = 0; i < loop; ++i) {
				s32Ret = MEDIA_AUDIO_PcmRead(cap_buffer);
				if (s32Ret > 0) {
					/* Convert two-channel data to one channel */
					memcpy(buffer + i * size, cap_buffer, s32Ret / 2);
				} else {
					printf("MEDIA_AUDIO_PcmRead failed\n");
					continue;
				}
			}
		}

		/* Detect the audio */
		int s32Ret = CVI_TDL_SoundClassification_V2(tdl_handle, &Frame, &index);
		if (s32Ret == CVI_SUCCESS) {
			printf("esc class: %s\n", enumStr[index]);
		} else {
			printf("detect failed\n");
		}

		/* Record the audio */
		if (record) {
			FILE *fp = fopen(outpath, "wb");
            if (fp == NULL) {
                printf("open file %s failed\n", outpath);
            } else {
				printf("to write:%s\n", outpath);
				fwrite((char *)buffer, 1, FRAME_SIZE, fp);
				fclose(fp);
			}
            /* After record the audio, we should break out of the loop */
			break;
		}
	}

	pthread_exit(NULL);
	return NULL;
}

/**
 * @brief Load cvimodel and detect audio
 * @note
 * @param  argc: the number of command line arguments
 * @param  argv: the command line arguments
 * @retval None
 */
void cvitdl_aud_detect(int32_t argc, char **argv) {
	if (argc != 1 && argc != 2) {
		printf(
			"Usage: %s RECORD\n"
			"\t\tRECORD, record input to file, 0: disable 1. enable.\n",
			argv[0]);
		return;
	}
	if (argc == 2) {
		record = atoi(argv[1]) ? true : false;
	}

	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_TDL_CreateHandle3(&tdl_handle);
	if (s32Ret != CVI_SUCCESS) {
		printf("Create tdl handle failed with %#x!\n", s32Ret);
		return;
	}

	cvi_tpu_init();
	aos_msleep(1000);

    /* Open sound classification model */
	char model_path[64] = "/mnt/sd/aud_test/baby_cry_sr16k_v11_cv181x.cvimodel";
	s32Ret =
		CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_SOUNDCLASSIFICATION_V2, model_path);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TDL_DestroyHandle(tdl_handle);
		printf("open model failed with %#x!\n", s32Ret);
		return;
	}

	/* Be used to performance evaluation */
	CVI_TDL_SetPerfEvalInterval(tdl_handle, CVI_TDL_SUPPORTED_MODEL_SOUNDCLASSIFICATION_V2, 10);

    /* Create thread for handling audio detection */
	pthread_attr_t attr;
	pthread_t pcm_output_thread;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192000);
	pthread_create(&pcm_output_thread, &attr, uplink_audio, NULL);
	pthread_setname_np(pcm_output_thread, "cvitdl_aud_detect");
	pthread_join(pcm_output_thread, NULL);

	CVI_TDL_DestroyHandle(tdl_handle);
	return;
}
ALIOS_CLI_CMD_REGISTER(cvitdl_aud_detect, cvitdl_aud_detect, cvitdl_aud_detect);
