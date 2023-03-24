/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:wifi_dispatch.c
 * Description:
 *   ....
 */
#include "wifi_dispatch.h"
#include "wifi_hostmsgpack.h"
#include "wifi_hostmsgprase.h"
#include "wifi_common.h"
#include <stdio.h>
#include "cvi_type.h"
#include "wifi_if.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <aos/kernel.h>
#include <stdlib.h>
#include <string.h>
#include "wifi_if.h"
#include "disk_sd.h"
#include "cJSON.h"
#include <aos/kv.h>
#include "wifi_crc_type.h"
#if (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
#include "cx_func.h"
#include "cx_common.h"
#endif
#if (CONFIG_DEBUG_HOSTWIFI_EMU_SUPPORT == 1)
#include "hostwifi_emulator.h"
#endif
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
#include "media_nightvision.h"
#include "linkkit_client.h"
#endif
#define WIFI_RECVPACKMAXNUM 50
#define WIFI_HEARTCOUNT 8000
typedef struct {
	CVI_U8 *Data;
	CVI_S32 DataLen;
} WIFI_RECVPACKNODE_S;
typedef struct {
	WIFI_RECVPACKNODE_S Node[WIFI_RECVPACKMAXNUM];
	CVI_S32 front;
	CVI_S32 near;
	pthread_mutex_t mutex_lock;
} WIFI_RECVPACK_S;

typedef struct {
	CVI_CHAR wifi_ota_flag;
	CVI_CHAR wifi_packet_data;
	CVI_CHAR wifi_ota_state;
	CVI_CHAR wifi_package_flag;
} WIFI_OTA_INFO_S;

typedef struct {
	CVI_CHAR wifi_version_flag;
	CVI_CHAR wifi_version[9];
} WIFI_VERSION_INFO_S;

static WIFI_RECVPACK_S s_WifiRecvPack_Manager;
static void(*s_WifiRecvManager[Wifi_MsgCbMax])(CVI_U8 *, CVI_U32);
static pthread_t s_WifiDispatch_PthId;
static CVI_S32 s_WifiDispatch_RunStatus;

static WIFI_OTA_INFO_S wifi_ota_info = {
	.wifi_ota_flag = 0,
	.wifi_packet_data = 0,
	.wifi_ota_state = -1,
	.wifi_package_flag = 0,
};

static WIFI_VERSION_INFO_S wifi_version_info = {0};


CVI_S32 Wifi_DispatchSendMsgData(CVI_U8 *Data, CVI_S32 Datalen)
{
	if (!Data)
		return CVI_FAILURE;
#if (CONFIG_DEBUG_HOSTWIFI_EMU_SUPPORT == 1)
	HostWifiEmu_InfoQueuePushBack(Data, Datalen);
#endif
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	Wifi_SendMsgData(Data, Datalen);
#endif
	return CVI_SUCCESS;
}

CVI_S32 WifiRecvPackPush(CVI_U8 *Data, CVI_S32 Datalen)
{
	if (s_WifiDispatch_RunStatus == 0 ||
	    s_WifiRecvPack_Manager.front == (s_WifiRecvPack_Manager.near + 1) % WIFI_RECVPACKMAXNUM)
		return CVI_FAILURE;
	CVI_U8 *tempData = (CVI_U8 *)malloc(Datalen);
	if (!tempData) {
		printf("%s malloc err\n", __func__);
		return CVI_FAILURE;
	}
	pthread_mutex_lock(&s_WifiRecvPack_Manager.mutex_lock);
	if (s_WifiRecvPack_Manager.Node[s_WifiRecvPack_Manager.near].Data) {
		free(s_WifiRecvPack_Manager.Node[s_WifiRecvPack_Manager.near].Data);
		s_WifiRecvPack_Manager.Node[s_WifiRecvPack_Manager.near].Data = NULL;
	}
	memcpy(tempData, Data, Datalen);
	s_WifiRecvPack_Manager.Node[s_WifiRecvPack_Manager.near].Data = tempData;
	s_WifiRecvPack_Manager.Node[s_WifiRecvPack_Manager.near].DataLen = Datalen;
	s_WifiRecvPack_Manager.near = (s_WifiRecvPack_Manager.near + 1) % WIFI_RECVPACKMAXNUM;
	pthread_mutex_unlock(&s_WifiRecvPack_Manager.mutex_lock);
	return CVI_SUCCESS;
}

CVI_S32 WifiRecvPackPop(CVI_U8 **Data, CVI_S32 *Datalen)
{
	if (s_WifiRecvPack_Manager.front == s_WifiRecvPack_Manager.near || !Data)
		return CVI_FAILURE;
	pthread_mutex_lock(&s_WifiRecvPack_Manager.mutex_lock);
	*Data = s_WifiRecvPack_Manager.Node[s_WifiRecvPack_Manager.front].Data;
	*Datalen = s_WifiRecvPack_Manager.Node[s_WifiRecvPack_Manager.front].DataLen;
	s_WifiRecvPack_Manager.Node[s_WifiRecvPack_Manager.front].Data = NULL;
	s_WifiRecvPack_Manager.front = (s_WifiRecvPack_Manager.front + 1) % WIFI_RECVPACKMAXNUM;
	pthread_mutex_unlock(&s_WifiRecvPack_Manager.mutex_lock);
	return CVI_SUCCESS;
}

void WifiRecv_callback(uint32_t event_id, const void *data, void *context)
{
	if (!data || !context) {
		return ;
	};
	CVI_U32 dataLen = *((CVI_S32 *)context);
	WifiRecvPackPush((CVI_U8 *)data, dataLen);
}

CVI_VOID *Wifi_dispatch_Task(void *argc)
{
	prctl(PR_SET_NAME, "Wifi_dispatch_Task");
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	{
		CVI_U8 Mac[16];
		CVI_U8 IpAddr[16];
		WifiGetMac(Mac);
		WifiGetIp(IpAddr);
		WIFI_MSGINFO_S Info = {0};
		Wifi_HostMsgUpLoad(H2W_IOT_GET_DEVICEINFO, &Info);
	}
#endif
	CVI_U8 *Data = NULL;
	CVI_S32 Datalen = 0;
	while (s_WifiDispatch_RunStatus) {
		if (WifiRecvPackPop(&Data, &Datalen) == CVI_SUCCESS) {
			if (Data) {
				Wifi_RecvMsgData(Data, Datalen);
				free(Data);
				Data = NULL;
			}
		}
		aos_msleep(10);
	}
	s_WifiDispatch_RunStatus = 0;
	return 0;
}

CVI_S32 Wifi_RecvMsgData(CVI_U8 *Data, CVI_U32 DataLen)
{
	CVI_S32 i = 0;
	for (i = 0; i < Wifi_MsgCbMax; i++) {
		if (s_WifiRecvManager[i]) {
			s_WifiRecvManager[i](Data, DataLen);
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 Wifi_MsgRecvRegister(WIFI_MSGRECVCB_E Type, void (*func)(CVI_U8 *, CVI_U32))
{
	if (Type < 0 || Type > Wifi_MsgCbMax) {
		printf("%s Type err\n", __func__);
		return CVI_FAILURE;
	}
	if (s_WifiRecvManager[Type]) {
		printf("%s Register err\n", __func__);
		return CVI_FAILURE;
	}
	s_WifiRecvManager[Type] = func;
	return CVI_SUCCESS;
}

CVI_S32 Wifi_MsgRecvUnRegister(WIFI_MSGRECVCB_E Type)
{
	if (Type < 0 || Type > Wifi_MsgCbMax) {
		printf("%s Type err\n", __func__);
		return CVI_FAILURE;
	}
	if (s_WifiRecvManager[Type]) {
		s_WifiRecvManager[Type] = NULL;
		return CVI_SUCCESS;
	}
	return CVI_SUCCESS;
}

void wifi_MsgRecv_CallBack(CVI_U8 *Msg, CVI_U32 MsgLen)
{
	WifiMsg_Prase(Msg, MsgLen);
}
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
void Wifi_LinkKit_ClientMqttCb(void *Handle, const char *topic_name, iotx_mqtt_topic_info_pt topic_msg)
{
	WIFI_MSGINFO_S Info = {0};
	Info.LVPublishInfo.Index = &topic_msg->qos;
	Info.LVPublishInfo.IndexLen = 1;
	Info.LVPublishInfo.Topic = (CVI_U8 *)topic_name;
	Info.LVPublishInfo.TopicLen = strlen(topic_name);
	Info.LVPublishInfo.Message = (CVI_U8 *)topic_msg->payload;
	Info.LVPublishInfo.MessaegLen = topic_msg->payload_len;
	Wifi_HostMsgUpLoad(H2W_LINK_VISUAL_PUBLISH, &Info);
}
#endif

CVI_S32 DoorbellModule_WifiDispatchStart()
{
	struct sched_param sch_param;
	pthread_attr_t pthread_attr;
	if (s_WifiDispatch_RunStatus == 0) {
		pthread_mutex_init(&s_WifiRecvPack_Manager.mutex_lock, NULL);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
		Wifi_RecvSubscribe(WifiRecv_callback, NULL);
#endif
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
		iot_msg_callback_register(&Wifi_LinkKit_ClientMqttCb);
#endif
		Wifi_MsgRecvRegister(Wifi_Dispatch, &wifi_MsgRecv_CallBack);
		sch_param.sched_priority = 40;
		pthread_attr_init(&pthread_attr);
		pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
		pthread_attr_setschedparam(&pthread_attr, &sch_param);
		pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
		s_WifiDispatch_RunStatus = 1;
		if (pthread_create(&s_WifiDispatch_PthId, &pthread_attr, Wifi_dispatch_Task, NULL) < 0) {
			printf("%s pthread_create err\n", __func__);
			s_WifiDispatch_RunStatus = 0;
			return CVI_FAILURE;
		}
		printf("%s Success \n", __func__);
		return CVI_SUCCESS;
	} else {
		printf("%s err\n", __func__);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 Wifi_dispatch_Destroy()
{
	if (s_WifiDispatch_RunStatus == 1) {
		s_WifiDispatch_RunStatus = 0;
		pthread_join(s_WifiDispatch_PthId, NULL);
		pthread_mutex_destroy(&s_WifiRecvPack_Manager.mutex_lock);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
		Wifi_RecvUnSubscribe(WifiRecv_callback, NULL);
#endif
		Wifi_MsgRecvUnRegister(Wifi_Dispatch);
	}
	return CVI_SUCCESS;
}

CVI_S32 Wifi_SdStatusToJson(cJSON *pRoot)
{
	double Total = 0;
	double Remain = 0;
	CVI_S32 Status = 0;
	Status = DISK_SdGetStatus(CVI_FAILURE);
	DISK_SdGetCapacity(&Total, &Remain);
	cJSON_AddNumberToObject(pRoot, "StorageTotalCapacity", Total);
	cJSON_AddNumberToObject(pRoot, "StorageRemainCapacity", Remain);
	cJSON_AddNumberToObject(pRoot, "StorageRecordMode", 1);
	cJSON_AddNumberToObject(pRoot, "StorageStatus", Status);
	return CVI_SUCCESS;
}

CVI_S32 Wifi_ReportSdStatus()
{
	cJSON *pRoot = NULL;
	CVI_CHAR *Buf = NULL;
	WIFI_MSGINFO_S Info = {0};

	pRoot = cJSON_CreateObject();
	if (!pRoot)
		goto EXIT;
	Wifi_SdStatusToJson(pRoot);
	Buf = cJSON_Print(pRoot);
	Info.IotReportInfo.Attribute = (CVI_U8 *)Buf;
	Info.IotReportInfo.AttributeLen = strlen(Buf);
	Wifi_HostMsgUpLoad(H2W_IOT_REPORT_PROPERTY, &Info);

EXIT:
	if (pRoot)
		cJSON_Delete(pRoot);
	if (Buf)
		free(Buf);
	return CVI_SUCCESS;
}
CVI_S32 Wifi_VideoQualityToJson(cJSON *pRoot)
{
	if (!pRoot)
		return CVI_FAILURE;
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	CVI_S32 Quality = CX_VIDEO_CLARITY_HD;
#if (CONFIG_SUPPORT_NORFLASH == 1)
	CVI_S32 s32Ret = 0;
	s32Ret = aos_kv_getint("main_vq", &Quality);
	if (s32Ret != 0) {
		Quality = CX_VIDEO_CLARITY_HD;
	}
#endif
	cJSON_AddNumberToObject(pRoot, "StreamVideoQuality", Quality);
#endif
	return CVI_SUCCESS;
}

CVI_S32 Wifi_ReportStreamVideoQuality()
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	cJSON *pRoot = NULL;
	CVI_CHAR *Buf = NULL;
	WIFI_MSGINFO_S Info = {0};

	pRoot = cJSON_CreateObject();
	if (!pRoot)
		goto EXIT;
	Wifi_VideoQualityToJson(pRoot);
	Buf = cJSON_Print(pRoot);
	Info.IotReportInfo.Attribute = (CVI_U8 *)Buf;
	Info.IotReportInfo.AttributeLen = strlen(Buf);
	Wifi_HostMsgUpLoad(H2W_IOT_REPORT_PROPERTY, &Info);
EXIT:
	if (pRoot)
		cJSON_Delete(pRoot);
	if (Buf)
		free(Buf);
#endif
	return CVI_SUCCESS;
}

CVI_S32 Wifi_DayNightModeToJson(cJSON *pRoot)
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	if (!pRoot)
		return CVI_FAILURE;
	CVI_S32 DayNightMode = APP_NIGHTVISION_AUTO_MODE; // 0:白天 1:夜晚 2:自动
	DayNightMode = APP_NightVision_GetMode(CVI_TRUE);
	cJSON_AddNumberToObject(pRoot, "DayNightMode", DayNightMode);
#endif
	return CVI_SUCCESS;
}

CVI_S32 Wifi_ReportDayNightMode()
{
	cJSON *pRoot = NULL;
	CVI_CHAR *Buf = NULL;
	WIFI_MSGINFO_S Info = {0};

	pRoot = cJSON_CreateObject();
	if (!pRoot)
		goto EXIT;
	Wifi_DayNightModeToJson(pRoot);

	Buf = cJSON_Print(pRoot);
	Info.IotReportInfo.Attribute = (CVI_U8 *)Buf;
	Info.IotReportInfo.AttributeLen = strlen(Buf);
	Wifi_HostMsgUpLoad(H2W_IOT_REPORT_PROPERTY, &Info);
EXIT:
	if (pRoot)
		cJSON_Delete(pRoot);
	if (Buf)
		free(Buf);
	return CVI_SUCCESS;
}

CVI_S32 Wifi_AlarmAttributeToJson(cJSON *pRoot)
{
	if (!pRoot)
		return CVI_FAILURE;
	CVI_S32 AlarmSwitch = 0;
	CVI_S32 MotionDetectSensitivity = 0;//0-5 关闭到最高挡
	CVI_S32 AlarmFrequencyLevel = 0;//0-2 低频到高频
#if (CONFIG_SUPPORT_NORFLASH == 1)
	CVI_S32 s32Ret = 0;
	s32Ret = aos_kv_getint("alm_sw", &AlarmSwitch);
	if (s32Ret != 0) {
		AlarmSwitch = 0;
	}
	s32Ret = aos_kv_getint("alm_mdsl", &MotionDetectSensitivity);
	if (s32Ret != 0) {
		MotionDetectSensitivity = 0;
	}
	s32Ret = aos_kv_getint("alm_freq", &AlarmFrequencyLevel);
	if (s32Ret != 0) {
		AlarmFrequencyLevel = 0;
	}
#endif
	cJSON_AddNumberToObject(pRoot, "AlarmSwitch", AlarmSwitch);
	cJSON_AddNumberToObject(pRoot, "MotionDetectSensitivity", MotionDetectSensitivity);
	cJSON_AddNumberToObject(pRoot, "AlarmFrequencyLevel", AlarmFrequencyLevel);
	return CVI_SUCCESS;
}

CVI_S32 Wifi_ReportAlarmAttribute()
{
	cJSON *pRoot = NULL;
	CVI_CHAR *Buf = NULL;
	WIFI_MSGINFO_S Info = {0};

	pRoot = cJSON_CreateObject();
	if (!pRoot)
		goto EXIT;
	Wifi_AlarmAttributeToJson(pRoot);

	Buf = cJSON_Print(pRoot);
	Info.IotReportInfo.Attribute = (CVI_U8 *)Buf;
	Info.IotReportInfo.AttributeLen = strlen(Buf);
	Wifi_HostMsgUpLoad(H2W_IOT_REPORT_PROPERTY, &Info);
EXIT:
	if (pRoot)
		cJSON_Delete(pRoot);
	if (Buf)
		free(Buf);
	return CVI_SUCCESS;
}

CVI_S32 Wifi_VideoRotaionToJson(cJSON *pRoot)
{
	if (!pRoot)
		return CVI_FAILURE;
	CVI_S32 ImageFlipState = 0;

#if (CONFIG_SUPPORT_NORFLASH == 1)
	CVI_S32 s32Ret = 0;
	s32Ret = aos_kv_getint("flip", &ImageFlipState);
	if (s32Ret != 0) {
		ImageFlipState = 0;
	}
#endif
	cJSON_AddNumberToObject(pRoot, "ImageFlipState", ImageFlipState);
	return CVI_SUCCESS;
}

CVI_S32 Wifi_ReportVideoRotaion()
{
	cJSON *pRoot = NULL;
	CVI_CHAR *Buf = NULL;
	WIFI_MSGINFO_S Info = {0};

	pRoot = cJSON_CreateObject();
	if (!pRoot)
		goto EXIT;
	Wifi_VideoRotaionToJson(pRoot);

	Buf = cJSON_Print(pRoot);
	Info.IotReportInfo.Attribute = (CVI_U8 *)Buf;
	Info.IotReportInfo.AttributeLen = strlen(Buf);
	Wifi_HostMsgUpLoad(H2W_IOT_REPORT_PROPERTY, &Info);
EXIT:
	if (pRoot)
		cJSON_Delete(pRoot);
	if (Buf)
		free(Buf);
	return CVI_SUCCESS;
}

CVI_S32 Wifi_AttreibuteInitReport()
{
	cJSON *pRoot = NULL;
	CVI_CHAR *Buf = NULL;
	WIFI_MSGINFO_S Info = {0};

	pRoot = cJSON_CreateObject();
	if (!pRoot)
		goto EXIT;

	Wifi_AlarmAttributeToJson(pRoot);
	Wifi_VideoRotaionToJson(pRoot);
	Wifi_DayNightModeToJson(pRoot);
	DISK_SdGetStatus(CVI_TRUE);
	Wifi_SdStatusToJson(pRoot);
	Wifi_VideoQualityToJson(pRoot);

	Buf = cJSON_Print(pRoot);
	Info.IotReportInfo.Attribute = (CVI_U8 *)Buf;
	Info.IotReportInfo.AttributeLen = strlen(Buf);
	Wifi_HostMsgUpLoad(H2W_IOT_REPORT_PROPERTY, &Info);
EXIT:
	if (pRoot)
		cJSON_Delete(pRoot);
	if (Buf)
		free(Buf);
	return CVI_SUCCESS;
}


CVI_S32 Wifi_Set_Sleep_Time(CVI_U16 second)
{
	WIFI_MSGINFO_S Info = {0};
	Info.TimeInfo.SleepTime = second;
	Info.TimeInfo.SleepTimeLen = 2;
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_WIFI_SLEEP_TIME, &Info);
	return CVI_SUCCESS;
}

CVI_S32 Wifi_Send_Process(CVI_U32 id, CVI_CHAR progress)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 id_data[8] = {0};
	id_data[0] = id % 256;
	id_data[1] = id / 256 % 256;
	id_data[2] = id / 256 / 256 % 256;
	id_data[3] = id / 256 / 256 / 256 % 256;
	Info.UpgradeInfo.UpgradeID = id_data;
	Info.UpgradeInfo.UpgradeIDLen = 4;
	id_data[4] = (CVI_UCHAR) progress;
	Info.UpgradeInfo.Progress = &id_data[4];
	Info.UpgradeInfo.ProgressLen = 1;
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_UPDATEPROGRESS, &Info);
	return CVI_SUCCESS;
}


CVI_S32 Wifi_Ota_Start(CVI_SL len, CVI_CHAR *version, CVI_CHAR *wifi_statue, CVI_S32 *package_len)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 id_data[32] = {0};
	id_data[0] = len % 256;
	id_data[1] = len / 256 % 256;
	id_data[2] = len / 256 / 256 % 256;
	id_data[3] = len / 256 / 256 / 256 % 256;
	memcpy(&id_data[4], (CVI_U8 *)version, 8);
	Info.DataInfo.Data    =  id_data;
	Info.DataInfo.DataLen =  12;
	wifi_ota_info.wifi_ota_flag = 0;
	Wifi_HostMsgUpLoad(W2H_HOST_CMD_ALL_OTA_START, &Info);
	aos_msleep(150);
	if (1 != wifi_ota_info.wifi_ota_flag) {
		printf("wait for time out \r\n");
		return -2;
	}
	*wifi_statue = wifi_ota_info.wifi_ota_state;
	*package_len = wifi_ota_info.wifi_packet_data;
	wifi_ota_info.wifi_ota_flag = 0;
	return CVI_SUCCESS;

}


CVI_S32 Wifi_Ota_Start_Info_Back(WIFI_MSGINFO_S *Info)
{

	if (Info->DataInfo.Data[0] == 0) {
		wifi_ota_info.wifi_ota_state = 0;
		wifi_ota_info.wifi_packet_data = Info->DataInfo.Data[1] + Info->DataInfo.Data[2] * 256;

	} else if (24 == Info->DataInfo.Data[0]) {
		wifi_ota_info.wifi_ota_state = 1;
	} else {
		wifi_ota_info.wifi_ota_state = -1;
	}

	wifi_ota_info.wifi_ota_flag = 1;
	return 0;
}


CVI_S32 Wifi_Get_Version_Back(WIFI_MSGINFO_S *Info)
{
	if (Info->DataInfo.Data == NULL) {
		return 0;
	}
	memcpy(wifi_version_info.wifi_version, Info->DataInfo.Data, 8);
	wifi_version_info.wifi_version_flag = 1;

	return 0;
}



CVI_S32 Wifi_Get_Version(CVI_CHAR *version)
{
	CVI_S32 version_len = 8;
	if (1 == wifi_version_info.wifi_version_flag) {
		//version =wifi_version;
		memcpy(version, wifi_version_info.wifi_version, version_len);
		return 0;
	}

	WIFI_MSGINFO_S Info = {0};
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_GET_VERSION, &Info);

	//hcc_send_base_cmd(HOST_CMD_GET_VERSION);

	//wait for 150ms
	aos_msleep(150);
	if (1 != wifi_version_info.wifi_version_flag) {
		printf("wait for version timeout \r\n");
		return -1;
	}
	memcpy(version, wifi_version_info.wifi_version, version_len);
	wifi_version_info.wifi_version_flag = 0;

	return 0;
}

CVI_S32 Wifi_Report_TotalVersion(CVI_CHAR *version, CVI_U16 VersionLen)
{

	WIFI_MSGINFO_S Info = {0};
	Info.DataInfo.Data    = (CVI_U8 *)version;
	Info.DataInfo.DataLen =  VersionLen;
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_UPDATEVERSION, &Info);

	return CVI_SUCCESS;

}



CVI_S32 Wifi_Ota_Packet_Back(WIFI_MSGINFO_S *Info)
{
	if (Info->DataInfo.Data[0] == 0 || Info->DataInfo.Data[0] == 22) {
		wifi_ota_info.wifi_package_flag = 1;
		return 0;
	} else if (24 == Info->DataInfo.Data[0]) {
		return 0;
	} else {
		//wifi_ota_state = -1;
		return 0;
	}

	wifi_ota_info.wifi_package_flag = 1;
	return 0;
}




CVI_S32 Wife_Send_Ota_Package(CVI_S32 packet_len, CVI_S32 packet_number, CVI_U8 *packet_data)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_S32 datalen = 0;
	CVI_U8 *cmd  = (CVI_U8 *)aos_malloc(packet_len + 10);
	memset(cmd, 0, packet_len + 10);
	cmd[0] = packet_number % 256;
	cmd[1] = packet_number / 256;
	datalen += 2;
	memcpy(&cmd[3], packet_data, packet_len);
	CVI_U32 crc_16 = CRC16(packet_data, packet_len);
	cmd[3 + packet_len] = crc_16 % 256;
	cmd[4 + packet_len] = crc_16 / 256;
	wifi_ota_info.wifi_package_flag = 0;

	Info.DataInfo.Data = cmd;
	Info.DataInfo.DataLen = packet_len + 4;
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_WIFI_OTA, &Info);

	CVI_S32 i = 0;
	while (1) {
		if (i > 300) {
			printf("wait time out \r\n");
			aos_free(cmd);
			return -2;
		}
		if (1 == wifi_ota_info.wifi_package_flag) {
			wifi_ota_info.wifi_package_flag = 0;
			break;
		}
		aos_msleep(10);
		i++;
	}
	aos_free(cmd);
	return CVI_SUCCESS;

}



