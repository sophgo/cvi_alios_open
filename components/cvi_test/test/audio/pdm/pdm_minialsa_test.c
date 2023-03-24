#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/cli.h>
#include <posix/timer.h>
#include <drv/i2s.h>
#include <alsa/pcm.h>
#include <unistd.h>
#include "board.h"
#include "dirent.h"
#include "dw_i2s.h"
#include "fatfs_vfs.h"
#include "platform.h"

static int g_capture_running, g_play_running;
static unsigned char * g_test_buf = NULL;
static unsigned char * g_ref_test_buf = NULL;
int g_test_buf_num = 0;
int g_test_ref_num = 0;
#define TEST_BUF_SIZE 1 * 1024 * 1024
#define CAPTURE_SIZE 4096
#define PERIOD_FRAMES_SIZE 320
unsigned int capture_rate = 16000;
unsigned int play_rate = 16000;



static void audio_capture_test_entry(void *arg)
{
	int ret;
	int dir = 0;
	bool bsavemen = 0;
	char save_name[128];
	unsigned char *cap_buf;
	unsigned char *cap_buf_ref;
	aos_pcm_t *capture_handle;
 	aos_pcm_t *capture_handle_ref;
	aos_pcm_hw_params_t *capture_hw_params;
	aos_pcm_hw_params_t *capture_hw_params_ref;
	int fd = 0;
	int fd_ref = 0;
	cap_buf = aos_malloc(CAPTURE_SIZE);
	cap_buf_ref = aos_malloc(CAPTURE_SIZE);
	snprintf(save_name, sizeof(save_name), "%s", (char *)arg);//16k2ch_1.pcm
	if (strstr(save_name, "//mem/")) {
		bsavemen = 1;
		if (g_test_buf == NULL) {
			g_test_buf = aos_malloc(TEST_BUF_SIZE);
		}
		if (g_ref_test_buf == NULL) {
			g_ref_test_buf = aos_malloc(TEST_BUF_SIZE);
		}
		if (g_test_buf == NULL || g_ref_test_buf == NULL) {
			printf(" g_test_buf == NULL || g_ref_test_buf == NULL \r\n");
			g_capture_running = 0;
			return ;
		}
	} else {
		fd = aos_open(save_name, O_CREAT | O_TRUNC | O_RDWR);
		if (fd <= 0) {
			printf("file %s open error\n", save_name);
			g_capture_running = 0;
			return ;
		} else {
			printf("create pcm file:%s\n", save_name);
		}
		snprintf(save_name, sizeof(save_name), "%s.ref", (char *)arg);//16k2ch_1.pcm
		fd_ref = aos_open(save_name, O_CREAT | O_TRUNC | O_RDWR);
		if (fd_ref <= 0) {
			printf("file %s open error\n", save_name);
			g_capture_running = 0;
			return ;
		} else {
			printf("create pcm file:%s\n", save_name);
		}
	}
	if (cap_buf == NULL || cap_buf_ref == NULL ) {
		printf(" cap_buf == NULL || cap_buf_ref == NULL \r\n");
		g_capture_running = 0;
		return ;
	}
	g_test_ref_num = 0;
	g_test_buf_num = 0;
	//init capture(i2s0). clk path: i2s3(master) -> internal_codec -> i2s0(slave)
	dir = 1;
	aos_pcm_open (&capture_handle, "pcmD1", AOS_PCM_STREAM_CAPTURE, 0);//打开设备“pcmC0
	aos_pcm_hw_params_alloca (&capture_hw_params);//申请硬件参数内存空间
	aos_pcm_hw_params_any (capture_handle, capture_hw_params);//初始化硬件参数
	capture_hw_params->period_size = PERIOD_FRAMES_SIZE;
	capture_hw_params->buffer_size = PERIOD_FRAMES_SIZE*4;
	aos_pcm_hw_params_set_access (capture_handle, capture_hw_params, AOS_PCM_ACCESS_RW_INTERLEAVED);// 设置音频数据参数为交错模式
	aos_pcm_hw_params_set_format (capture_handle, capture_hw_params, 16);//设置音频数据参数为小端16bit
	aos_pcm_hw_params_set_rate_near (capture_handle, capture_hw_params, &capture_rate, &dir);//设置音频数据参数采样率为16K
	aos_pcm_hw_params_set_channels (capture_handle, capture_hw_params, 2);//设置音频数据参数为2通道
	aos_pcm_hw_params (capture_handle, capture_hw_params);//设置硬件参数到具体硬件中

	aos_pcm_open (&capture_handle_ref, "pcmC0", AOS_PCM_STREAM_CAPTURE, 0);//打开设备“pcmC0
	aos_pcm_hw_params_alloca (&capture_hw_params_ref);//申请硬件参数内存空间
	aos_pcm_hw_params_any (capture_handle_ref, capture_hw_params_ref);//初始化硬件参数
	capture_hw_params_ref->period_size = PERIOD_FRAMES_SIZE;
	capture_hw_params_ref->buffer_size = PERIOD_FRAMES_SIZE*4;
	aos_pcm_hw_params_set_access (capture_handle_ref, capture_hw_params_ref, AOS_PCM_ACCESS_RW_INTERLEAVED);// 设置音频数据参数为交错模式
	aos_pcm_hw_params_set_format (capture_handle_ref, capture_hw_params_ref, 16);//设置音频数据参数为小端16bit
	aos_pcm_hw_params_set_rate_near (capture_handle_ref, capture_hw_params_ref, &play_rate, &dir);//设置音频数据参数采样率为16K
	aos_pcm_hw_params_set_channels (capture_handle_ref, capture_hw_params_ref, 2);//设置音频数据参数为2通道
	aos_pcm_hw_params (capture_handle_ref, capture_hw_params_ref);//设置硬件参数到具体硬件中
	while(g_capture_running) {
		ret = aos_pcm_readi(capture_handle, cap_buf, PERIOD_FRAMES_SIZE);//接收交错音频数据
		if (ret == 0) {
			usleep(1);
			continue;
		}
		if (fd > 0) {
			aos_write(fd, cap_buf, aos_pcm_frames_to_bytes(capture_handle, ret));
		} else if (bsavemen > 0) {
			if (g_test_buf_num + ret >= TEST_BUF_SIZE) {
				break;
			}
			memcpy(g_test_buf + g_test_buf_num,cap_buf, aos_pcm_frames_to_bytes(capture_handle, ret));
			g_test_buf_num += aos_pcm_frames_to_bytes(capture_handle, ret);
		}
		ret = aos_pcm_readi(capture_handle_ref, cap_buf_ref, PERIOD_FRAMES_SIZE);//接收交错音频数据
		if (ret == 0) {
			usleep(1);
			continue;
		}
		if (fd_ref > 0) {
			aos_write(fd_ref, cap_buf_ref, aos_pcm_frames_to_bytes(capture_handle_ref, ret));
		} else if (bsavemen > 0) {
			if (g_test_ref_num + ret >= TEST_BUF_SIZE) {
				break;
			}
			memcpy(g_ref_test_buf + g_test_ref_num, cap_buf, aos_pcm_frames_to_bytes(capture_handle, ret));
			g_test_ref_num += aos_pcm_frames_to_bytes(capture_handle, ret);
		}
	}
	if (fd > 0) {
		aos_close(fd);
	}
	if (fd_ref > 0) {
		aos_close(fd_ref);
	}
	printf("end pdm_arecord \r\n");
	aos_free(cap_buf);
	aos_free(cap_buf_ref);
	aos_pcm_close(capture_handle); //关闭设备
	aos_pcm_close(capture_handle_ref); //关闭设备
	capture_handle = NULL;
	capture_handle_ref = NULL;
}

static void audio_play_test_entry(void *arg)
{
	int fd = 0;
	char save_name[128];
	unsigned char *cap_buf;
	int bsavemem = 0;
	int bplayrefmem = 0;
	int ret = 0;
	int writei_byte = 0;
	aos_pcm_t *playback_handle;
	aos_pcm_hw_params_t *playback_hw_params;
	int dir;
	snprintf(save_name, sizeof(save_name), "%s", (char *)arg);//16k2ch.pcm
	if (strstr(save_name, "//mem/")) {
		bsavemem = 1;
		if(g_test_buf == NULL || g_ref_test_buf == NULL) {
			printf("please input pdm_arecord\r\n");
			return ;
		}
		if(strstr(save_name, "ref")) {
			printf("play ref \r\n");
			bplayrefmem = 1;
		}
	} else {
		fd = aos_open(save_name, O_CREAT | O_TRUNC | O_RDWR);
		if (fd <= 0) {
			printf("file %s open error\n", save_name);
		} else {
			printf("create pcm file:%s\n", save_name);
		}
	}
	dir = 0;
	cap_buf = aos_malloc(CAPTURE_SIZE);
	aos_pcm_open (&playback_handle, "pcmP0", AOS_PCM_STREAM_PLAYBACK, 0); //打开设备“pcmP0”
	aos_pcm_hw_params_alloca(&playback_hw_params); //申请硬件参数内存空间
	aos_pcm_hw_params_any(playback_handle, playback_hw_params); //初始化硬件参数
	playback_hw_params->period_size = PERIOD_FRAMES_SIZE;
	playback_hw_params->buffer_size = PERIOD_FRAMES_SIZE*4;
	aos_pcm_hw_params_set_access(playback_handle, playback_hw_params, AOS_PCM_ACCESS_RW_INTERLEAVED); // 设置音频数据参数为交错模式
	aos_pcm_hw_params_set_format(playback_handle, playback_hw_params, 16); //设置音频数据参数为小端16bit
	aos_pcm_hw_params_set_rate_near(playback_handle, playback_hw_params, &play_rate, &dir); //设置音频数据参数采样率为16K
	aos_pcm_hw_params_set_channels(playback_handle, playback_hw_params, 2); //设置音频数据参数为2通道
	aos_pcm_hw_params(playback_handle, playback_hw_params); //设置硬件参数到具体硬件?
	while (g_play_running) {
		if (bsavemem == 1) {
			unsigned char * tmp = g_test_buf;
			if (bplayrefmem) {
				tmp = g_ref_test_buf;
			}
			memcpy(cap_buf, tmp + ret, aos_pcm_frames_to_bytes(playback_handle, PERIOD_FRAMES_SIZE));
			writei_byte = aos_pcm_frames_to_bytes(playback_handle, PERIOD_FRAMES_SIZE);
		} else {
			if (fd > 0) {
				writei_byte = aos_read(fd, cap_buf, aos_pcm_frames_to_bytes(playback_handle, PERIOD_FRAMES_SIZE));
			}
		}
		ret += aos_pcm_frames_to_bytes(playback_handle, PERIOD_FRAMES_SIZE);
		if (bsavemem == 1) {
			if(bplayrefmem == 1) {
				if(ret >= g_test_ref_num) {
					ret = 0;
					continue;
				}
			} else {
				if(ret >= g_test_buf_num) {
					ret = 0;
					continue;
				}
			}
		}
		aos_pcm_writei(playback_handle, cap_buf, aos_pcm_bytes_to_frames(playback_handle, writei_byte));//发送交错音频数据
	}
	if (fd > 0) {
		aos_close(fd);
	}
	aos_pcm_close(playback_handle); //关闭设备
	playback_handle = NULL;
	aos_free(cap_buf);
}

static void pdm_audio_debug_capture_cmd(int32_t argc, char **argv)
{
	if (argc == 3) {
		if (strcmp(argv[2], "1") == 0) {
			if(g_capture_running) {
				printf("capture device is busy pleae input arecord 0 to stop.\n");
				return;
			}
			g_capture_running = 1;
			aos_task_new("audio_capture_test", audio_capture_test_entry, argv[1], 4096);
			return;
		} else if (strcmp(argv[2], "0") == 0) {
			printf("audio_arecord stop\n");
			g_capture_running = 0;
			return;
		}
	} else if (argc == 2) {
		if (strcmp(argv[1], "0") == 0) {
			printf("audio_arecord stop\n");
			g_capture_running = 0;
			return;
		}
	}
	printf("invalid cmd params.\n");
	printf("usage:%s filepath 1/0\n",argv[0]);
	printf("usage:%s 0\n",argv[0]);
}

static void pdm_audio_debug_play_cmd(int32_t argc, char **argv)
{
	if (argc == 3) {
		if (strcmp(argv[2], "1") == 0) {
			//if (access(argv[1], F_OK) != 0) {
			//	printf("audio_play stop can't find file \n");
			//	return ;
			//}
			if (g_play_running) {
				printf("play device is busy pleae input aplay 0 to stop.\n");
				return;
			}
			g_play_running = 1;
			PLATFORM_SpkMute(1);
			aos_task_new("audio_play_test", audio_play_test_entry, argv[1], 4096);
			return;
		} else if (strcmp(argv[2], "0") == 0) {
			printf("audio_play stop\n");
			g_play_running = 0;
			return;
		}
	} else if (argc == 2) {
		if (strcmp(argv[1], "0") == 0) {
			printf("audio_play stop \n");
			PLATFORM_SpkMute(0);
			g_play_running = 0;
			return;
		}
	}
	printf("invalid cmd params.\n");
	printf("usage:%s filepath 1/0\n",argv[0]);
	printf("usage:%s 0\n",argv[0]);
}

ALIOS_CLI_CMD_REGISTER(pdm_audio_debug_play_cmd, pdm_aplay, pdm play audio);
ALIOS_CLI_CMD_REGISTER(pdm_audio_debug_capture_cmd, pdm_arecord, pdm capture audio);
