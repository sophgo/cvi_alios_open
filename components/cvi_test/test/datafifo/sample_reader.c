#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <aos/cli.h>
#include "cvi_datafifo.h"

static CVI_DATAFIFO_HANDLE hDataFifo = CVI_DATAFIFO_INVALID_HANDLE;
static const CVI_S32 BLOCK_LEN = 1024;
CVI_U64 phyAddr;
static pthread_t readThread;
static CVI_BOOL s_bStop = CVI_FALSE;
static CVI_BOOL s_bInited = CVI_FALSE;

static int datafifo_read_init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_DATAFIFO_PARAMS_S params = {10, BLOCK_LEN, CVI_TRUE, DATAFIFO_READER};

	s32Ret = CVI_DATAFIFO_OpenByAddr(&hDataFifo, &params, phyAddr);
	if (CVI_SUCCESS != s32Ret) {
		printf("get datafifo error:%x\n", s32Ret);
		return -1;
	}

	printf("datafifo_init finish\n");

	return 0;
}

void *read_more(void *arg)
{
	CVI_CHAR *pBuf = NULL;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 readLen = 0;

	(void)arg;

	while (CVI_FALSE == s_bStop) {
		readLen = 0;
		s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
		if (CVI_SUCCESS != s32Ret) {
			printf("datafifo get available read len failed with:%x\n", s32Ret);
			return NULL;
		}

		if (readLen > 0) {
			s32Ret = CVI_DATAFIFO_Read(hDataFifo, (CVI_VOID **)&pBuf);
			if (CVI_SUCCESS != s32Ret) {
				printf("datafifo read failed with:%x\n", s32Ret);
				return NULL;
			}

			printf("received: %s\n", pBuf);

			s32Ret = CVI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_READ_DONE, pBuf);
			if (CVI_SUCCESS != s32Ret) {
				printf("datafifo read done failed with:%x\n", s32Ret);
				return NULL;
			}
		}
		usleep(40000);
	}

	return NULL;
}

void datafifo_read_deinit(void)
{
	CVI_DATAFIFO_Close(hDataFifo);
	printf("datafifo_deinit finish\n");
}

void read_msg(int32_t argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (s_bInited) return;

	if (argc != 2) {
		printf("Usage: %s PhyAddr\n", argv[0]);
		return;
	}

	sscanf(argv[1],  "%llx", &phyAddr);
	printf("PhyAddr: %llx\n", phyAddr);

	s32Ret = datafifo_read_init();
	if (0 != s32Ret) {
		return;
	}

	s_bInited = CVI_TRUE;
	s_bStop = CVI_FALSE;
	pthread_create(&readThread, NULL, read_more, NULL);
}

ALIOS_CLI_CMD_REGISTER(read_msg, datafifo_read_start, test datafifo start);


void read_msg_stop(int32_t argc, char **argv)
{
	if (!s_bInited) return;

	s_bStop = CVI_TRUE;

	pthread_join(readThread, NULL);

	datafifo_read_deinit();

	s_bInited = CVI_FALSE;
}

ALIOS_CLI_CMD_REGISTER(read_msg_stop, datafifo_read_stop, test datafifo stop);

