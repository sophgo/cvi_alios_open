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
#include "cvi_audio_pp_interface.h"

static void dump_audiodata(char *filename, char *buf, unsigned int len)
{
	FILE *fp;

	if (filename == NULL) {
		return;
	}

	fp = fopen(filename, "ab+");
	fwrite(buf, 1, len, fp);
	fclose(fp);
}

void _check_and_dump(const char *filename, char *buf, unsigned int sizebytes)
{
	if (access(filename, F_OK) == 0) {
		char newfilename[128] = {0};

		snprintf(newfilename, 128, "%s.pcm", filename);

		dump_audiodata((char *)newfilename,
			       (char *)buf,
			       (unsigned int)sizebytes);
	}
}

static int audio_debug_pp_cmd(int argc, char *argv[])
{

    int sampleRate = 48000;
    int iChannels = 2;
    int s32Readbyte = 0;

    printf("***********************************************\n"
           "***Audio PP TEST[internal test] ***\n"
           "usage	: <input.raw> <output.raw> <sample rate> <channels> <preiod size>\n"
           "sample	: audio_test_pp input.raw output.raw 48000 2 960\n"
           "sample	: audio_test_pp input.raw output.raw 8000 2 320\n"
           "***********************************************\n");
    if (argc < 6) {
        printf("[Error]Please check the usage\n");
        printf("[Error]Input argument is not enough!!!\n");
        return 1;
    }

    sampleRate = atoi(argv[3]);//eq only support 48k
    iChannels = atoi(argv[4]);
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

    int destFrame = atoi(argv[5]);
    int period_ms = destFrame * 1000 / sampleRate;
    int max_size = 48000 / 1000 * 2 * 2 * period_ms;

    short *dataIn = (short *)malloc(max_size);
    short *dataOut = (short *)malloc(max_size);
    memset(dataIn, 0, max_size);
    memset(dataOut, 0, max_size);

    void *handle = cvi_audio_pp_init(sampleRate, iChannels);
    if(handle == NULL){
        printf("Create post process func failure..\n");
        goto ERROR;
    }

    int frameLenBytes = destFrame * iChannels * 2;

    printf("post process start\n");
    while(1){
        s32Readbyte = aos_read(fdin, dataIn, frameLenBytes);
        if(s32Readbyte == 0){
            break;
        }
        _check_and_dump("/mnt/sd/dump_before_pp", (char *)dataIn, frameLenBytes);

        cvi_audio_pp_process(handle, dataIn, dataOut, destFrame);

        _check_and_dump("/mnt/sd/dump_after_pp", (char *)dataOut, frameLenBytes);
        int ret = aos_write(fdout, dataOut, frameLenBytes);
        if(ret < 0){
            printf("write err\r\n");
            goto ERROR;
        }
    }
    cvi_audio_pp_deinit(handle);
    handle = NULL;
    printf("post process end\n");
ERROR:
    free(dataIn);
    free(dataOut);
    aos_close(fdout);
    aos_close(fdin);

    return 0;
}

ALIOS_CLI_CMD_REGISTER(audio_debug_pp_cmd, audio_test_pp, audio pp test);
