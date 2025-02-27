#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <aos/cli.h>

#include "cvi_datafifo.h"


static CVI_S32 g_s32Index = 0;
static CVI_DATAFIFO_HANDLE hDataFifo = CVI_DATAFIFO_INVALID_HANDLE;
static const CVI_S32 BLOCK_LEN = 1024;
static pthread_t sendThread;
static CVI_BOOL s_bStop = CVI_FALSE;
static CVI_BOOL s_bInited = CVI_FALSE;

static void release(void* pStream)
{
	printf("release %p\n", pStream);
}

static int datafifo_init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U64 phyAddr = 0;
	CVI_DATAFIFO_PARAMS_S params = {10, BLOCK_LEN, CVI_TRUE, DATAFIFO_WRITER};

	s32Ret = CVI_DATAFIFO_Open(&hDataFifo, &params);
	if (CVI_SUCCESS != s32Ret) {
		printf("open datafifo error:%x\n", s32Ret);
		return -1;
	}

	s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_GET_PHY_ADDR, &phyAddr);
	if (CVI_SUCCESS != s32Ret) {
		printf("get datafifo phy addr error:%x\n", s32Ret);
		return -1;
	}

	printf("PhyAddr: %llx\n", phyAddr);

	s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, release);
	if (CVI_SUCCESS != s32Ret) {
		printf("set release func callback error:%x\n", s32Ret);
		return -1;
	}

	printf("datafifo_init finish\n");

	return 0;
}

void* send_more(void* arg)
{
	CVI_CHAR buf[BLOCK_LEN];
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 availWriteLen = 0;

	while (CVI_FALSE == s_bStop) {

		// call write NULL to flush
		s32Ret = CVI_DATAFIFO_Write(hDataFifo, NULL);
		if (CVI_SUCCESS != s32Ret) {
			printf("write error:%x\n", s32Ret);
		}

		s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
		if (CVI_SUCCESS != s32Ret) {
			printf("get available write len error:%x\n", s32Ret);
			break;
		}

		if (availWriteLen > 0) {
			memset(buf, 0, BLOCK_LEN);
			snprintf(buf, BLOCK_LEN, "========%d========", g_s32Index);
			s32Ret = CVI_DATAFIFO_Write(hDataFifo, buf);
			if (CVI_SUCCESS != s32Ret) {
				printf("write error:%x\n", s32Ret);
				break;
			}

			printf("send: %s\n", buf);

			s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_WRITE_DONE, NULL);
			if (CVI_SUCCESS != s32Ret) {
				printf("write done error:%x\n", s32Ret);
				break;
			}

			g_s32Index++;
		}

		usleep(40000);
	}

	return NULL;
}

void datafifo_deinit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	// call write NULL to flush and release stream buffer.
	s32Ret = CVI_DATAFIFO_Write(hDataFifo, NULL);
	if (CVI_SUCCESS != s32Ret) {
		printf("write error:%x\n", s32Ret);
	}

	CVI_DATAFIFO_Close(hDataFifo);
	printf("datafifo_deinit finish\n");
}

void write_msg(int32_t argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (s_bInited) return;

	s32Ret = datafifo_init();
	if (0 != s32Ret) {
		return;
	}

	s_bInited = CVI_TRUE;
	s_bStop = CVI_FALSE;
	pthread_create(&sendThread, NULL, send_more, NULL);
}

ALIOS_CLI_CMD_REGISTER(write_msg, datafifo_write_start, test datafifo start);


void write_msg_stop(int32_t argc, char **argv)
{
	if (!s_bInited) return;

	s_bStop = CVI_TRUE;

	pthread_join(sendThread, NULL);

	datafifo_deinit();

	s_bInited = CVI_FALSE;
}

ALIOS_CLI_CMD_REGISTER(write_msg_stop, datafifo_write_stop, test datafifo stop);

