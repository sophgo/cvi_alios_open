#include <sys/prctl.h>
#include <stdio.h>
#include <pthread.h>
#include <aos/cli.h>
#include "cvi_ipcmsg.h"
#include "cvi_msg_server.h"

static CVI_S32 s_s32Id;
static pthread_t s_threadid;

void* rcv_thread(void* arg)
{
	prctl(PR_SET_NAME, "rcv_thread", 0, 0, 0);
	CVI_S32* s32Id = (CVI_S32*)arg;
	printf("receive from %d\n", *s32Id);
	CVI_IPCMSG_Run(*s32Id);
	printf("CVI_IPCMSG_Run end.\n");
	return NULL;
}

void handle_message(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S* msg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR content[32];

	//printf("==============\n");
	printf("receive msg: %s, len: %d\n", (CVI_CHAR*)msg->pBody, msg->u32BodyLen);

	memset(content, 0, 32);
	switch(msg->u32Module) {
		case 1:
			snprintf(content, 32, "module:%d, cmd:%d, have done.", msg->u32Module, msg->u32CMD);
			s32Ret = 0;
			break;
		case 2:
			snprintf(content, 32, "module:%d, cmd:%d, have done.", msg->u32Module, msg->u32CMD);
			s32Ret = 0;
			break;
		case 3:
			return;
		default:
			snprintf(content, 32, "module:%d, cmd:%d, is not found.", msg->u32Module, msg->u32CMD);
			s32Ret = -1;
	}

	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_IPCMSG_CreateRespMessage(msg, s32Ret, content, 32);
	CVI_IPCMSG_SendOnly(s32Id, respMsg);
	CVI_IPCMSG_DestroyMessage(respMsg);
	//printf("==============\n\n");
}

void recv_msg(CVI_S32 argc, CVI_CHAR **argv)
{
	CVI_S32 ret = 0;
	CVI_IPCMSG_CONNECT_S stConnectAttr;
	struct sched_param tsk;
	pthread_attr_t attr;

	stConnectAttr.u32RemoteId = 0;
	stConnectAttr.u32Port = 201;
	stConnectAttr.u32Priority = 0;

	ret = CVI_MSG_Deinit();
	if (ret != CVI_SUCCESS) {
		printf("CVI_MSG_Deinit fail, return err:%x\n", ret);
		return;
	}
	ret = CVI_IPCMSG_AddService("Test", &stConnectAttr);
	if(ret != 0) {
		printf("CVI_IPCMSG_AddService return err:%x\n", ret);
	}

	if (CVI_SUCCESS != CVI_IPCMSG_Connect(&s_s32Id, "Test", handle_message)) {
		printf("Connect fail\n");
		return;
	}

	tsk.sched_priority = 45;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &tsk);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);


	if (0 != pthread_create(&s_threadid, &attr, rcv_thread, &s_s32Id)) {
		printf("pthread_create rcv_thread fail\n");
		return;
	}

}

ALIOS_CLI_CMD_REGISTER(recv_msg, ipcmsg_start, test ipcmsg start);

void recv_msg_stop(CVI_S32 argc, CVI_CHAR **argv)
{

	CVI_IPCMSG_Disconnect(s_s32Id);
	printf("CVI_IPCMSG_Disconnect\n");
	pthread_join(s_threadid, NULL);
	CVI_IPCMSG_DelService("Test");
	printf("quit\n");
}

ALIOS_CLI_CMD_REGISTER(recv_msg_stop, ipcmsg_stop, test ipcmsg stop);

