/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:
 * Description:
 *   ....
 */
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "cvi_venc.h"
#include "cvi_vpss.h"
#include "event_dispatch.h"
#include "uart_communication.h"
#include "module_test.h"
#include "event_def.h"
#if CONFIG_DEBUG_HOSTMCU_EMU_SUPPORT
#include "hostmcu_emulator.h"
#endif
static  pthread_t s_RunTask_ThreadId;
static  CVI_S32 s_RunStatus;
static  pthread_mutex_t s_BufVectorMutex;
#define Module_BufSizeMax                2048

typedef struct _ModuleBufVector_s {
	CVI_U8      Buf[Module_BufSizeMax];
	CVI_S32     CurPosition;
} ModuleBufVector_s;

static ModuleBufVector_s s_BufVector = {0};

CVI_S32 Module_SetGetConfigPro(CVI_U8 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	EVENT_DISPATCH_DEBUG("%s CMD %d\n", __func__, CMD);
	switch (CMD) {
	case COMM_CMD_H2M_QUERY_AGREEMT_VER:
		Query_AgreemtVer_Prase();
		break;
	case COMM_CMD_H2M_SET_NETCONFIG:
		ConfigNetCmdPrase(Data, DataLen);
		break;
	case COMM_CMD_H2M_GET_NETCONFIG:
		GetNetStatusAck();
		break;
	case COMM_CMD_H2M_GET_VIDEOINFO:
		GetVideoInfoAck();
		break;
	case COMM_CMD_H2M_SET_VIDEOINFO:
		SetVideoInfoPrase(Data, DataLen);
		break;
	case COMM_CMD_H2M_GET_PICTUREINFO:
		GetPictureInfoAck();
		break;
	case COMM_CMD_H2M_SET_PICTUREINFO:
		SetPictureInfoPrase(Data, DataLen);
		break;
	case COMM_CMD_H2M_GET_AUDIOINFO:
		GetAudioInfoAck();
		break;
	case COMM_CMD_H2M_SET_AUDIOINFO:
		SetAudioInfoPrase(Data, DataLen);
		break;
	case COMM_CMD_H2M_START_STREAM:
		StartStreamPushPrase();
		break;
	case COMM_CMD_H2M_STOP_STREAM:
		StopStreamPushPrase();
		break;
	case COMM_CMD_H2M_GET_STREAM_STATUS:
		GetStreamStausPrase();
		break;
	case COMM_CMD_H2M_START_IPC_TALK:
		StartIpcTalkPrase();
		break;
	case COMM_CMD_H2M_STOP_IPC_TALK:
		StopIpcTalkPrase();
		break;
	case COMM_CMD_H2M_GET_IPC_TALK_STATUS:
		GetIpcTalkStatusPrase();
		break;
	case COMM_CMD_M2H_REPORT_IPC_TALK_STATUS:
		break;
	case COMM_CMD_H2M_PIC_CAPTURE:
		Capture_Picture_Prase(Data, DataLen);
		break;
	case COMM_CMD_H2M_DATA_TRANSFER:
		DataTransferPrase(Data, DataLen);
		break;
	case COMM_CMD_H2M_GET_WIFIMAC:
		GetFaceModuleMacPrase();
		break;
	case COMM_CMD_H2M_GET_ALIYUN_PARAM:
		GetAliYunParmPrase();
		break;
	case COMM_CMD_H2M_SET_ALIYUN_PARAM:
		SetAliYunParmPrase(Data, DataLen);
		break;
	case COMM_CMD_H2M_RAWDATA_TRANSFER:
		TransferRawDataPrase(Data, DataLen);
		break;
	case COMM_CMD_H2M_QUERY_SOFTWARE_VER:
		GetSoftwareVerPrase();
		break;
	case COMM_CMD_H2M_CLEAR_NETCONFIG:
		CleanNetconfigPrase();
		break;
	case COMM_CMD_H2M_FACTORY_RESET:
		FactoryResetPrase();
		break;
	case COMM_CMD_H2M_START_SNAP_PICTURE:
		Snap_Picture();
		break;
	case COMM_CMD_H2M_STOP_SMARTCONFIG:
		Reboot_wifi();
		break;
	case COMM_CMD_H2M_QUERY_CATEYE_VER:
		Get_CatEye_Version();
		break;

	default:
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 Module_BufVectorPushBack(ModuleBufVector_s *p_Vector, CVI_U8 *Buf, CVI_U32 BufLen)
{
	if (p_Vector->CurPosition + BufLen > Module_BufSizeMax)
		p_Vector->CurPosition = 0;
	memcpy(p_Vector->Buf + p_Vector->CurPosition, Buf, BufLen);
	p_Vector->CurPosition += BufLen;
	return CVI_SUCCESS;
}

CVI_S32 Module_BufRemove(ModuleBufVector_s *p_Vector, CVI_S32 Position)
{
	if (Position <= 0 || Position >= Module_BufSizeMax)
		return CVI_FAILURE;
	if (s_BufVector.CurPosition < Position)
		return CVI_FAILURE;
	pthread_mutex_lock(&s_BufVectorMutex);
	memcpy(p_Vector->Buf, &p_Vector->Buf[Position], p_Vector->CurPosition - Position);
	p_Vector->CurPosition = p_Vector->CurPosition - Position;
	pthread_mutex_unlock(&s_BufVectorMutex);
	return CVI_SUCCESS;
}

static CVI_S32 Module_BufVectorClean(ModuleBufVector_s *p_Vector)
{
	pthread_mutex_lock(&s_BufVectorMutex);
	p_Vector->CurPosition = 0;
	memset(p_Vector->Buf, 0, Module_BufSizeMax);
	pthread_mutex_unlock(&s_BufVectorMutex);
	return CVI_SUCCESS;
}

static CVI_S32 Module_FindPackHead(CVI_U8 *Buf, CVI_S32 BufLen)
{
	CVI_S32 i = 0;

	for (i = 0; i < BufLen - 4; i++) {
		if (Buf[i] == (COMM_PACKET_HEAD_FIRST) &&
		    Buf[i + 1] == (COMM_PACKET_HEAD_SECOND) &&
		    Buf[i + 3] == COMM_PACKET_PID_H2M) {
			return i;
		}
	}
	return CVI_FAILURE;
}

static CVI_S32 Module_CheckPackHead(ModuleBufVector_s *p_Vector)
{
	if (p_Vector->CurPosition < COMM_PACKET_SIZE)
		return CVI_FAILURE;
	//寻找标识头
	CVI_S32 HeadPosition = -1;
	HeadPosition = Module_FindPackHead(p_Vector->Buf, p_Vector->CurPosition);
	if (HeadPosition < 0)
		return CVI_FAILURE;
	Module_BufRemove(p_Vector, HeadPosition);
	return CVI_SUCCESS;
}

CVI_U16 Module_CheckSum(CVI_U8 *Buf, CVI_U16 BufLength)
{
	unsigned int sum = 0;
	unsigned short nwords = BufLength >> 1;
	unsigned short *word = (unsigned short *)Buf;
	unsigned short i = 0;
	for (i = 0; i < nwords; ++i) {
		sum += word[i];
	}
	if (BufLength & 1) {
		sum += Buf[i * 2];
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum = ~sum & 0xffff;

	return (unsigned short)sum;

}

static CVI_S32 Module_CheckPackSum(CVI_U8 *Buf, CVI_U32 BufLen)
{
	H_M_COMM_CMD_PACK_T *PackHead = (H_M_COMM_CMD_PACK_T *)Buf;
	CVI_U16 _CheckSum = 0;

	if (BufLen < PackHead->Length + COMM_PACKET_SIZE) {
		return CVI_FAILURE;
	}
	_CheckSum = Module_CheckSum(Buf, PackHead->Length + COMM_PACKET_SIZE);
	if (PackHead->FrameHead[0] != COMM_PACKET_HEAD_FIRST ||
	    PackHead->FrameHead[1] != COMM_PACKET_HEAD_SECOND) {
		EVENT_DISPATCH_DEBUG("%s FrameIdentification Error\n", __func__);
		return CVI_FAILURE;
	}
	if (PackHead->PackIdentification != COMM_PACKET_PID_H2M) {
		EVENT_DISPATCH_DEBUG("%s PackIdentification Error\n", __func__);
		return CVI_FAILURE;
	}
	if (_CheckSum != 0) {
		EVENT_DISPATCH_DEBUG("%s Sum! Error\n", __func__);
		return CVI_FAILURE_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static CVI_S32 Module_MoveToNextPack(ModuleBufVector_s *p_Vector)
{
	CVI_S32 NextPack_Position = 0;

	NextPack_Position = Module_FindPackHead(p_Vector->Buf + 1, p_Vector->CurPosition - 1);
	if (NextPack_Position == -1) {
		Module_BufVectorClean(p_Vector);
		return CVI_FAILURE;
	} else {
		NextPack_Position += 1;
		Module_BufRemove(p_Vector, NextPack_Position);
		return CVI_SUCCESS;
	}
	return CVI_SUCCESS;
}

CVI_S32 Module_CheckPackLength(ModuleBufVector_s *p_Vector, H_M_COMM_CMD_PACK_T *PackHead)
{
	if (p_Vector->CurPosition < COMM_PACKET_SIZE)
		return CVI_FAILURE;
	CVI_S32 NextPack_Position = Module_FindPackHead(p_Vector->Buf + 1, p_Vector->CurPosition - 1);
	if (NextPack_Position != -1)
		return CVI_SUCCESS;
	if ((PackHead->Length + COMM_PACKET_SIZE) > p_Vector->CurPosition)
		return CVI_FAILURE;

	return CVI_SUCCESS;
}

CVI_S32 Module_ReadCallBack(CVI_U8 *Buf, CVI_U32 BufLength)
{
	pthread_mutex_lock(&s_BufVectorMutex);
	Module_BufVectorPushBack(&s_BufVector, Buf, BufLength);
	pthread_mutex_unlock(&s_BufVectorMutex);
	return CVI_SUCCESS;
}

CVI_VOID *Event_dispatch_Task(void *arg)
{
	prctl(PR_SET_NAME, "Event_dispatch_Task");
	while (s_RunStatus) {
		H_M_COMM_CMD_PACK_T *PackHead = NULL;
		if (Module_CheckPackHead(&s_BufVector) != CVI_SUCCESS)
			goto TASK_SLEEP;
		PackHead = (H_M_COMM_CMD_PACK_T *)s_BufVector.Buf;
		if (Module_CheckPackLength(&s_BufVector, PackHead) != CVI_SUCCESS)
			goto TASK_SLEEP;
		if (Module_CheckPackSum(s_BufVector.Buf, s_BufVector.CurPosition) != CVI_SUCCESS) {
			if (Module_MoveToNextPack(&s_BufVector) != CVI_SUCCESS)
				goto TASK_SLEEP;
			continue;
		}
		Module_SetGetConfigPro(PackHead->CMD, s_BufVector.Buf + COMM_PACKET_SIZE, PackHead->Length);
		if (Module_BufRemove(&s_BufVector, PackHead->Length + COMM_PACKET_SIZE) == CVI_SUCCESS)
			continue;
TASK_SLEEP:
		usleep(10 * 1000);
	}
	return NULL;
}

CVI_S32 Event_dispatch_Init()
{
	//初始化
	//注册
	if (s_RunStatus != 0) {
		EVENT_DISPATCH_DEBUG("%s:s_RunStatus :%d\n", __func__, s_RunStatus);
		return CVI_FAILURE;
	}
	s_RunStatus = 1;
	pthread_mutex_init(&s_BufVectorMutex, NULL);
	if (pthread_create(&s_RunTask_ThreadId, NULL, Event_dispatch_Task, NULL) != CVI_SUCCESS) {
		EVENT_DISPATCH_DEBUG("%s:pthread_create err %d\n", __func__, s_RunStatus);
		s_RunStatus = 0;
		return CVI_FAILURE;
	}
	UartComm_RecvCbAttach(UART_LOCKER, &Module_ReadCallBack);
	//初始化串口
	if (UartComm_Init() == CVI_SUCCESS) {
		EVENT_DISPATCH_DEBUG("%s Success\n", __func__);
	} else {
		EVENT_DISPATCH_DEBUG("%s:UartComm_Init err %d\n", __func__, s_RunStatus);
		s_RunStatus = 0;
		pthread_join(s_RunTask_ThreadId, NULL);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 Event_dispatch_Destroy(void)
{
	if (s_RunStatus == 1) {
		s_RunStatus = 0;
		pthread_join(s_RunTask_ThreadId, NULL);
		UartComm_Destroy();
		Module_BufVectorClean(&s_BufVector);
		pthread_mutex_destroy(&s_BufVectorMutex);
		return CVI_SUCCESS;
	} else
		return CVI_FAILURE;

}

CVI_S32 Event_dispath_DataSend(CVI_U8 *Buf, CVI_U32 BufLen, CVI_U32 u32BlockTimeMs)
{
	//需要替换发送方式则将该接口替换掉
#if CONFIG_DEBUG_HOSTMCU_EMU_SUPPORT
	HostMcuEmu_InfoQueuePushBack(Buf, BufLen);
#endif
	return UartComm_Send(Buf, BufLen, u32BlockTimeMs);
}
