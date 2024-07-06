#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/cli.h>
#include <posix/timer.h>
#include <alsa/pcm.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "board.h"
#include "aos/cli.h"
#include "vfs.h"
#include "cvi_comm_aio.h"
#include "cvi_resampler_api.h"

#define DEFAULT_BYTES_PER_SAMPLE 2

static int audio_debug_resample_cmd(int argc, char *argv[])
{

    int iSampleRate = 8000;
    int oSampleRate = 16000;
    int iChannels = 2;
    int s32Readbyte = 0;
    int s32OutResFrameLen = 0;
    printf("***********************************************\n"
           "***Audio Resample TEST[internal test] ***\n"
           "usage	: audio_test_resample <input.raw> <output.raw> <isample rate> <osample rate> <channels> <preiod size>\n"
           "sample	: audio_test_resample input.raw output.raw 8000 16000 2 960\n"
           "sample	: audio_test_resample input.raw output.raw 8000 32000 2 320\n"
           "***********************************************\n");
    if (argc < 7) {
        printf("[Error]Please check the usage\n");
        printf("[Error]Input argument is not enough!!!\n");
        return 1;
    }

    iSampleRate = atoi(argv[3]);
    oSampleRate = atoi(argv[4]);
    iChannels = atoi(argv[5]);
    if (iChannels != 1 && iChannels != 2) {
        printf("iChannels error,not support:%d.\n", iChannels);
        return 1;
    }

    int fdin = aos_open(argv[1], O_RDONLY);
    int fdout = aos_open(argv[2], O_CREAT | O_TRUNC | O_RDWR);
    if (fdin <= 0 || fdout <= 0) {
        printf("can't open file [%d, %d] \n", fdin, fdout);
        return 1;
    }

    int destFrame = atoi(argv[6]);
    int period_ms = destFrame * 1000 / iSampleRate;
    int max_size = 48000 / 1000 * 2 * 2 * period_ms;

    short *dataIn = (short *)malloc(max_size);
    short *dataOut = (short *)malloc(max_size);
    memset(dataIn, 0, max_size);
    memset(dataOut, 0, max_size);

    void *pResHandle = CVI_Resampler_Create(iSampleRate, oSampleRate, iChannels);
    if (pResHandle == CVI_NULL) {
	printf("Create resample func failure..\n");
	goto ERROR;
    }

    int frameLenBytes = destFrame * iChannels * DEFAULT_BYTES_PER_SAMPLE;

    printf("resample start\n");
    while(1){
        s32Readbyte = aos_read(fdin, dataIn, frameLenBytes);
        if(s32Readbyte == 0){
            break;
        }

        s32OutResFrameLen = CVI_Resampler_Process(pResHandle, dataIn, destFrame, dataOut);

        int ret = aos_write(fdout, dataOut, s32OutResFrameLen * iChannels * DEFAULT_BYTES_PER_SAMPLE);
        if(ret < 0){
            printf("write err\r\n");
            goto ERROR;
        }
    }
    CVI_Resampler_Destroy(pResHandle);
    pResHandle = NULL;
    printf("resample end\n");
ERROR:
    free(dataIn);
    free(dataOut);
    aos_close(fdout);
    aos_close(fdin);

    return 0;
}

ALIOS_CLI_CMD_REGISTER(audio_debug_resample_cmd, audio_test_resample, audio resample test);
