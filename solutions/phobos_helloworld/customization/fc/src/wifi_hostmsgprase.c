/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: wifi_hostmsgprase.c
 * Description:
 *   ....
 */

#include "wifi_hostmsgprase.h"
#include "wifi_dispatch.h"
#include "wifi_common.h"
#include "cvi_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event_dispatch.h"
#include "cJSON.h"
#include <aos/kv.h>
#include "disk_sd.h"
#include "media_video.h"
#include "platform.h"
#include "global_sleep_flag.h"
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
#include "media_nightvision.h"
#include "linkkit_client.h"
#include "link_visual_api.h"
#include "link_visual_enum.h"
#include "exports/iot_export_linkkit.h"
#include "link_visual_struct.h"
#include "iot_export.h"
#include "iot_import.h"
#include "sdk_assistant.h"

#include "global_sleep_flag.h"


#ifdef DUMMY_IPC
#include "dummy_ipc.h"
#else
#include "normal_ipc.h"
#endif
#include "cx_func.h"
static CVI_S32 g_LvConnectStatus = 0;
#endif
#define WIFIMSGPRASEFUN(COMMCMD) WP_##COMMCMD



CVI_S32 WifiMsg_IOT_SET_SERVICEPROPERTY(CVI_CHAR *pServerName, CVI_CHAR *pServerData)
{
	//设置服务解析接口
	if (!pServerName)
		return CVI_FAILURE;
	if (strstr(pServerName, "FormatStorageMedium")) {
		DISK_SdFormat();
		Wifi_ReportSdStatus();
	}

	return CVI_SUCCESS;
}

CVI_S32 WifiMsg_IOT_SET_PROPERTY(CVI_CHAR *pStr)
{
	//设置属性解析接口
	cJSON *pRoot = NULL;
	cJSON *pStreamVideoQuality = NULL;
	cJSON *pDayNightMode = NULL;
	cJSON *pAlarmSwitch = NULL;
	cJSON *pAlarmFrequencyLevel = NULL;
	cJSON *pMotionDetectSensitivity = NULL;
	cJSON *pAlarmPromptSwitch = NULL;
	cJSON *pImageFlipState = NULL;
	pRoot = cJSON_Parse(pStr);
	if (pRoot == NULL)
		goto EXIT;
	//主码流分辨率设置
	pStreamVideoQuality = cJSON_GetObjectItem(pRoot, "StreamVideoQuality");
	if (pStreamVideoQuality) {
		if (pStreamVideoQuality->type == cJSON_Number) {
			EVENT_DISPATCH_DEBUG("pStreamVideoQuality is %d \n", pStreamVideoQuality->valueint);
#if (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
			app_hal_set_video_clarity(CX_STREAM_MAIN, pStreamVideoQuality->valueint);
#endif
#if (CONFIG_SUPPORT_NORFLASH == 1)
			if (aos_kv_setint("main_vq", pStreamVideoQuality->valueint) != 0)
				EVENT_DISPATCH_DEBUG("aos_kv_setint  main_vq %d failed.\n", pStreamVideoQuality->valueint);
#endif
		}
	}
	//DayNightMode 夜视设置
	pDayNightMode = cJSON_GetObjectItem(pRoot, "DayNightMode");
	if (pDayNightMode) {
		if (pDayNightMode->type == cJSON_Number) {
			EVENT_DISPATCH_DEBUG("pDayNightMode is %d \n", pDayNightMode->valueint);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
			APP_NightVision_SetMode(pDayNightMode->valueint, CVI_TRUE);
#endif
		}
	}
	//报警开关 AlarmSwitch
	pAlarmSwitch = cJSON_GetObjectItem(pRoot, "AlarmSwitch");
	if (pAlarmSwitch) {
		if (pAlarmSwitch->type == cJSON_Number) {
			EVENT_DISPATCH_DEBUG("pAlarmSwitch is %d \n", pAlarmSwitch->valueint);
#if (CONFIG_SUPPORT_NORFLASH == 1)
			if (aos_kv_setint("alm_sw", pAlarmSwitch->valueint) != 0)
				EVENT_DISPATCH_DEBUG("aos_kv_setint  alm_sw %d failed.\n", pAlarmSwitch->valueint);
#endif
		}
	}
	//报警频率 AlarmFrequencyLevel
	pAlarmFrequencyLevel = cJSON_GetObjectItem(pRoot, "AlarmFrequencyLevel");
	if (pAlarmFrequencyLevel) {
		if (pAlarmFrequencyLevel->type == cJSON_Number) {
			EVENT_DISPATCH_DEBUG("pAlarmFrequencyLevel is %d \n", pAlarmFrequencyLevel->valueint);
#if (CONFIG_SUPPORT_NORFLASH == 1)
			if (aos_kv_setint("alm_freq", pAlarmFrequencyLevel->valueint) != 0)
				EVENT_DISPATCH_DEBUG("aos_kv_setint  alm_freq %d failed.\n", pAlarmFrequencyLevel->valueint);
#endif
		}
	}
	//移动侦测灵敏度 MotionDetectSensitivity
	pMotionDetectSensitivity = cJSON_GetObjectItem(pRoot, "MotionDetectSensitivity");
	if (pMotionDetectSensitivity) {
		if (pMotionDetectSensitivity->type == cJSON_Number) {
			EVENT_DISPATCH_DEBUG("pMotionDetectSensitivity is %d \n", pMotionDetectSensitivity->valueint);
#if (CONFIG_SUPPORT_NORFLASH == 1)
			if (aos_kv_setint("alm_mdsl", pMotionDetectSensitivity->valueint) != 0)
				EVENT_DISPATCH_DEBUG("aos_kv_setint  alm_mdsl %d failed.\n", pMotionDetectSensitivity->valueint);
#endif
		}
	}
	//报警提示开关 AlarmPromptSwitch
	pAlarmPromptSwitch = cJSON_GetObjectItem(pRoot, "AlarmPromptSwitch");
	if (pAlarmPromptSwitch) {
		if (pAlarmPromptSwitch->type == cJSON_Number) {
			EVENT_DISPATCH_DEBUG("pAlarmPromptSwitch is %d \n", pAlarmPromptSwitch->valueint);
		}
	}
	//画面翻转状态 ImageFlipState 0:正常状态 1：翻转
	pImageFlipState = cJSON_GetObjectItem(pRoot, "ImageFlipState");
	if (pImageFlipState) {
		if (pImageFlipState->type == cJSON_Number) {
			EVENT_DISPATCH_DEBUG("pImageFlipState is %d \n", pImageFlipState->valueint);
#if (CONFIG_SUPPORT_NORFLASH == 1)
			if (aos_kv_setint("flip", pImageFlipState->valueint) != 0)
				EVENT_DISPATCH_DEBUG("aos_kv_setint  flip %d failed.\n", pImageFlipState->valueint);
#endif
//			ISP_SNS_MIRRORFLIP_TYPE_E mirrorFlip = ISP_SNS_NORMAL;
//			if (pImageFlipState->valueint == 1)
//				mirrorFlip = ISP_SNS_FLIP;
//			app_videoSnsFilp(mirrorFlip);
		}
	}
EXIT:
	if (pRoot) {
		cJSON_Delete(pRoot);
	}
	return CVI_SUCCESS;
}

CVI_VOID WifiMsg_H2L_Adapter_B4(WIFI_MSGINFO_S *Info)
{
	MODULE_UPLOAD_S ModuleInfo = {0};
	ModuleInfo.Comm_Cmd = COMM_CMD_M2H_DATA_TRANSFER;
	EVENT_DISPATCH_DEBUG("ServiceData %s \r\n", Info->IotServerInfo.ServiceData);
	ModuleInfo.RawData.Data = Info->IotServerInfo.ServiceData;
	ModuleInfo.RawData.Datalen = Info->IotServerInfo.ServiceDataLen;
	Module_EventUpLoad(&ModuleInfo);
}

CVI_VOID WifiMsg_H2L_Adapter_BF0F(WIFI_MSGINFO_S *Info)
{
	MODULE_UPLOAD_S ModuleInfo = {0};
	ModuleInfo.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	ModuleInfo.Aliyun_Cmd = M2H_SET_SERVER_COMMOAND;
	CVI_S32 datalen = Info->IotServerInfo.ServiceDataLen + Info->IotServerInfo.ServiceNameLen + 10;
	CVI_CHAR *data = (CVI_CHAR *)malloc(datalen);
	memset(data, 0, datalen);
	sprintf(data, "{\"%s\":%s}", Info->IotServerInfo.ServiceName, Info->IotServerInfo.ServiceData);
	ModuleInfo.RawData.Data = (CVI_U8 *)data;
	ModuleInfo.RawData.Datalen = (CVI_U16)strlen(data) + 1;
	Module_EventUpLoad(&ModuleInfo);
	free(data);
}


CVI_VOID WifdMsg_H2W_Judge_DeviceInfo(WIFI_MSGINFO_S *Info)
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	CVI_U16 ProductKey_Len = Info->DeviceInfo.Product_KeyLen;
	CVI_U16 DeviceName_Len = Info->DeviceInfo.DeviceNameLen;
	CVI_U16 DeviceSecret_Len = Info->DeviceInfo.Device_SecretLen;
	CVI_U16 ProductSecret_Len = Info->DeviceInfo.Product_SecretLen;
	ProductKey_Len = ProductKey_Len > PRODUCT_KEY_MAXLEN - 1 ? PRODUCT_KEY_MAXLEN - 1 : ProductKey_Len;
	DeviceName_Len = DeviceName_Len > DEVICE_NAME_MAXLEN - 1 ? DEVICE_NAME_MAXLEN - 1 : DeviceName_Len;
	DeviceSecret_Len = DeviceSecret_Len > DEVICE_SECRET_MAXLEN - 1 ? DEVICE_SECRET_MAXLEN - 1 : DeviceSecret_Len;
	ProductSecret_Len = ProductSecret_Len > PRODUCT_SECRET_MAXLEN - 1 ? PRODUCT_SECRET_MAXLEN - 1 : ProductSecret_Len;

	char product_key[21] = {0};
	char device_name[33] = {0};
	char device_secret[65] = {0};
	char product_secret[65] = {0};
	CVI_S32 bufferLen = sizeof(device_name);
	aos_kv_get("dn", device_name, &bufferLen);
	if (strncmp(device_name, (CVI_CHAR *)Info->DeviceInfo.DeviceName, DeviceName_Len) != 0) {
		EVENT_DISPATCH_DEBUG("flash_kv dn err reset set\n");
		aos_kv_set("dn", Info->DeviceInfo.DeviceName, DeviceName_Len, 1);
	}
	bufferLen = sizeof(device_secret);
	aos_kv_get("ds", device_secret, &bufferLen);
	if (strncmp(device_secret, (CVI_CHAR *)Info->DeviceInfo.Device_Secret, DeviceSecret_Len) != 0) {
		EVENT_DISPATCH_DEBUG("flash_kv ds err reset set\n");
		aos_kv_set("ds", Info->DeviceInfo.Device_Secret, DeviceSecret_Len, 1);
	}
	bufferLen = sizeof(product_key);
	aos_kv_get("pk", product_key, &bufferLen);
	if (strncmp(product_key, (CVI_CHAR *)Info->DeviceInfo.Product_Key, ProductKey_Len) != 0) {
		EVENT_DISPATCH_DEBUG("flash_kv pk err reset set\n");
		aos_kv_set("pk", Info->DeviceInfo.Product_Key, ProductKey_Len, 1);
	}
	bufferLen = sizeof(product_secret);
	aos_kv_get("ps", product_secret, &bufferLen);
	if (strncmp(product_secret, (CVI_CHAR *)Info->DeviceInfo.Product_Secret, ProductSecret_Len) != 0) {
		EVENT_DISPATCH_DEBUG("flash_kv ps err reset set\n");
		aos_kv_set("ps", Info->DeviceInfo.Product_Secret, ProductSecret_Len, 1);
	}
#endif
}

CVI_VOID WifdMsg_H2W_SendLock_DeviceInfo(WIFI_MSGINFO_S *Info)
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	CVI_U8 ResponeBuf[512] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8  DataBuf[256] = {0};
	CVI_U8  DataLen = 0;
	CVI_U8 *RetCode = &DataBuf[0];
	CVI_U8 Product_Secret_Len = Info->DeviceInfo.Product_SecretLen + 1;
	CVI_U8 Product_Key_Len = Info->DeviceInfo.Product_KeyLen + 1;
	CVI_U8 Device_Name_Len = Info->DeviceInfo.DeviceNameLen + 1;
	CVI_U8 Device_Secret_Len = Info->DeviceInfo.Device_SecretLen + 1;

	DataBuf[1] = Product_Secret_Len;
	CVI_U8 *pProduct_Secret = &DataBuf[1 + 1];
	DataBuf[1 + 1 + Product_Secret_Len] = Product_Key_Len;
	CVI_U8 *pProduct_Key = &DataBuf[1 + 1 + Product_Secret_Len + 1];
	DataBuf[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len] = Device_Name_Len;
	CVI_U8 *pDevice_Name = &DataBuf[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1];
	DataBuf[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1 + Device_Name_Len] = Device_Secret_Len;
	CVI_U8 *pDevice_Secret = &DataBuf[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1 + Device_Name_Len + 1];
	memcpy(pProduct_Secret, Info->DeviceInfo.Product_Secret, Product_Secret_Len - 1);
	memcpy(pProduct_Key, Info->DeviceInfo.Product_Key, Product_Key_Len - 1);
	memcpy(pDevice_Name, Info->DeviceInfo.DeviceName, Device_Name_Len - 1);
	memcpy(pDevice_Secret, Info->DeviceInfo.Device_Secret, Device_Secret_Len - 1);
	DataLen = 1 + Product_Secret_Len + 1 + Product_Key_Len + 1 + Device_Name_Len + 1 + Device_Secret_Len + 1;

	if (Product_Secret_Len > PRODUCT_SECRET_MAXLEN ||
	    Product_Key_Len > PRODUCT_KEY_MAXLEN || Device_Name_Len > DEVICE_NAME_MAXLEN ||  Device_Name_Len > DEVICE_NAME_MAXLEN) {
		*RetCode = 1;
	}

	if (*RetCode == RES_ERRORCODE_CMD_OK)
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_ALIYUN_PARAM, DataBuf, DataLen);
	else
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_ALIYUN_PARAM, DataBuf, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
#endif
}





CVI_S32 WifiMsgInfo_CallBack(CVI_U32 CMD, WIFI_MSGINFO_S *Info)
{
	//转换后的接口,可以在此接口填充回调
	if (CMD != W2H_HOST_CMD_REPORT_WPA_EVENT)
		EVENT_DISPATCH_DEBUG("WifiMsg CMD :%d\n", CMD);
	switch (CMD) {
	case H2W_HOST_CMD_GET_MAC:
		//EVENT_DISPATCH_DEBUG("%s Get Mac is %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, Info->Mac[0], Info->Mac[1], Info->Mac[2],
		//       Info->Mac[3], Info->Mac[4], Info->Mac[5]);
		break;
	case H2W_HOST_CMD_GET_IP:
		//EVENT_DISPATCH_DEBUG("%s:get ip=%d.%d.%d.%d mask=%d.%d.%d.%d gw=%d.%d.%d.%d\n", __func__,
		//       Info->Net_Data[0] & 0xff, Info->Net_Data[1] & 0xff, Info->Net_Data[2] & 0xff, Info->Net_Data[3] & 0xff,
		//       Info->Net_Data[4] & 0xff, Info->Net_Data[5] & 0xff, Info->Net_Data[6] & 0xff, Info->Net_Data[7] & 0xff,
		//       Info->Net_Data[8] & 0xff, Info->Net_Data[9] & 0xff, Info->Net_Data[10] & 0xff, Info->Net_Data[11] & 0xff);
		break;
	case H2W_HOST_CMD_GET_RSSI:
		//if (Info->rssi)
		//	EVENT_DISPATCH_DEBUG("%s rssi %d\n", __func__, Info->rssi);
		break;
	case W2H_HOST_CMD_REPORT_WPA_EVENT:
		if (Info->EventInfo.EventData)
			//EVENT_DISPATCH_DEBUG("%s event %s\n",__func__,Info->EventInfo.EventData);
			break;
	case H2W_IOT_GET_DEVICEINFO: {
		WifdMsg_H2W_SendLock_DeviceInfo(Info);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
#if (CONFIG_SUPPORT_NORFLASH == 1)
		WifdMsg_H2W_Judge_DeviceInfo(Info);
#endif
#endif
		break;
	}
	case W2H_IOT_SET_PROPERTY:
		if (Info->IotReportInfo.Attribute) {
			EVENT_DISPATCH_DEBUG("%s Attribute %s\n", __func__, Info->IotReportInfo.Attribute);
			WifiMsg_IOT_SET_PROPERTY((CVI_CHAR *)Info->IotReportInfo.Attribute);
		}
		break;
	case W2H_IOT_SET_SERVICE: {
		if (Info->IotServerInfo.ServiceName)
			EVENT_DISPATCH_DEBUG("%s ServiceName is %s\n", __func__, Info->IotServerInfo.ServiceName);

		CVI_U8 user_service[30] = {"lock_control"};
		if (memcmp(Info->IotServerInfo.ServiceName, user_service, strlen((CVI_CHAR *)user_service)) == 0) {
			WifiMsg_H2L_Adapter_B4(Info);
		} else {
			WifiMsg_H2L_Adapter_BF0F(Info);
		}
		WifiMsg_IOT_SET_SERVICEPROPERTY((CVI_CHAR *)Info->IotServerInfo.ServiceName,
						(CVI_CHAR *)Info->IotServerInfo.ServiceData);
	}
	break;
	case W2H_IOT_REPORT_SERVICE:
		if (Info->IotReportInfo.Index)
			EVENT_DISPATCH_DEBUG("%s Index %s\n", __func__, Info->IotReportInfo.Index);
		if (Info->IotReportInfo.Topic)
			EVENT_DISPATCH_DEBUG("%s Topic %s\n", __func__, Info->IotReportInfo.Topic);
		if (Info->IotReportInfo.Message)
			EVENT_DISPATCH_DEBUG("%s Message %s\n", __func__, Info->IotReportInfo.Message);
		break;
	case W2H_IOT_USER_SET_SERVICE:
		if (Info->IotServerInfo.ID)
			EVENT_DISPATCH_DEBUG("%s ID %s\n", __func__, Info->IotServerInfo.ID);
		if (Info->IotServerInfo.Service)
			EVENT_DISPATCH_DEBUG("%s Service %s\n", __func__, Info->IotServerInfo.Service);
		if (Info->IotServerInfo.Request)
			EVENT_DISPATCH_DEBUG("%s Request %s\n", __func__, Info->IotServerInfo.Request);
		break;
	case W2H_LINK_VISUAL_SERVICE:
		//if (Info->IotServerInfo.ID)
		//	EVENT_DISPATCH_DEBUG("%s ID %s\n", __func__, Info->IotServerInfo.ID);
		if (Info->IotServerInfo.Service)
			EVENT_DISPATCH_DEBUG("%s Service %s\n", __func__, Info->IotServerInfo.Service);
		if (Info->IotServerInfo.Request)
			EVENT_DISPATCH_DEBUG("%s Request %s\n", __func__, Info->IotServerInfo.Request);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
		lv_device_auth_s auth = {0};
		if (cx_cloudGetAuth(&auth.product_key, NULL, &auth.device_name, &auth.device_secret) < 0) {
			break;
		}
		if (g_LvConnectStatus == 0) {
			Wifi_AttreibuteInitReport();
			lv_message_adapter_param_s Connectin = {0};
			Connectin.type = LV_MESSAGE_ADAPTER_TYPE_CONNECTED;
			lv_message_adapter(&auth, &Connectin);
			g_LvConnectStatus = 1;
		}
		lv_message_adapter_param_s in = {0};
		in.type = LV_MESSAGE_ADAPTER_TYPE_TSL_SERVICE;
		if (Info->IotServerInfo.ID) {
			in.msg_id = (CVI_CHAR *)Info->IotServerInfo.ID;
			in.msg_id_len = Info->IotServerInfo.IDLen;
		}
		if (Info->IotServerInfo.Service) {
			in.service_name = (CVI_CHAR *)Info->IotServerInfo.Service;
			in.service_name_len = Info->IotServerInfo.ServiceLen;
		}
		if (Info->IotServerInfo.Request) {
			in.request = (CVI_CHAR *)Info->IotServerInfo.Request;
			in.request_len = Info->IotServerInfo.RequestLen;
		}
		if (strncmp(in.service_name, "StartPushStreaming", strlen("StartPushStreaming")) == 0) {
			set_ready_play_flag();
		}
		int ret = lv_message_adapter(&auth, &in);
		if (ret < 0) {
			EVENT_DISPATCH_DEBUG("LinkVisual process service request failed, ret = %d\n", ret);
			break;
		}
#endif
		break;
	case W2H_LINK_VISUAL_USER: {
		if (Info->LVPublishInfo.ID)
			EVENT_DISPATCH_DEBUG("%s ID %s\n", __func__, Info->LVPublishInfo.ID);
		if (Info->LVPublishInfo.Service)
			EVENT_DISPATCH_DEBUG("%s Service %20s\n", __func__, Info->LVPublishInfo.Service);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
		lv_device_auth_s auth = {0};
		if (cx_cloudGetAuth(&auth.product_key, NULL, &auth.device_name, &auth.device_secret) < 0) {
			break;
		}
		lv_message_adapter_param_s in = {0};
		in.type = LV_MESSAGE_ADAPTER_TYPE_LINK_VISUAL;
		in.service_name = (CVI_CHAR *)Info->LVPublishInfo.ID;
		in.service_name_len = Info->LVPublishInfo.IDLen;
		in.request = (CVI_CHAR *)Info->LVPublishInfo.Service;
		in.request_len = Info->LVPublishInfo.ServiceLen;
		lv_message_adapter(&auth, &in);
#endif
	}
	break;
	case W2H_IOT_DEVICE_CONNECT:
		if (Info->IotServerInfo.ID)
			EVENT_DISPATCH_DEBUG("%s ID %s\n", __func__, Info->IotServerInfo.ID);
		if (Info->IotServerInfo.Service)
			EVENT_DISPATCH_DEBUG("%s Service %s\n", __func__, Info->IotServerInfo.Service);
		break;
	case H2W_HOST_CMD_WIFI_OTA_START:
		if (Info->DataInfo.Data)
			EVENT_DISPATCH_DEBUG("%s DataLen %d\n", __func__, Info->DataInfo.DataLen);
		break;
	case H2W_HOST_CMD_WIFI_OTA:
		if (Info->DataInfo.Data)
			EVENT_DISPATCH_DEBUG("%s DataLen %d\n", __func__, Info->DataInfo.DataLen);
		Wifi_Ota_Packet_Back(Info);
		break;
	case W2H_HOST_CMD_ALL_OTA_START:
		if (Info->DataInfo.Data)
			EVENT_DISPATCH_DEBUG("%s DataLen %d\n", __func__, Info->DataInfo.DataLen);
		Wifi_Ota_Start_Info_Back(Info);
		break;
	case H2W_HOST_CMD_ALL_OTA:
		if (Info->DataInfo.Data)
			EVENT_DISPATCH_DEBUG("%s DataLen %d\n", __func__, Info->DataInfo.DataLen);
		break;
	case H2W_HOST_TEST_RSSI:
		if (Info->ConnectInfo.SSID)
			EVENT_DISPATCH_DEBUG("%s SSID %s\n", __func__, Info->ConnectInfo.SSID);
		if (Info->ConnectInfo.RSSI)
			EVENT_DISPATCH_DEBUG("%s RSSI %s\n", __func__, Info->ConnectInfo.RSSI);
		break;
	case H2W_HOST_CMD_HEART:
		//0:允许休眠  1：不允许休眠
	{
		WIFI_MSGINFO_S _Info = {0};
		//CVI_U8 HeartEnable = Wifi_HeartEnableGet();
		//_Info.HeartInfo.SleepEnable = &HeartEnable;
		CVI_S32 ret   =  get_living_flag();
		CVI_U8  data  = (CVI_U8)ret;
		if(data == 0) {
			PLATFORM_PowerOff();
		}
		_Info.HeartInfo.SleepEnable = &data;
		_Info.HeartInfo.SleepEnableLen = 1;
		//EVENT_DISPATCH_DEBUG("H2W_HOST_CMD_HEART HeadEnable is %d \n",HeartEnable);
		Wifi_HostMsgUpLoad(H2W_HOST_CMD_HEART, &_Info);
	}
	break;
	case W2H_HOST_CMD_UPDATEINFO:
		if (Info->DataInfo.Data)
			EVENT_DISPATCH_DEBUG("%s DataLen %d\n", __func__, Info->DataInfo.DataLen);
		break;

	case W2H_LINK_VIUSAL_CONNECT: {
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
		lv_device_auth_s auth = {0};
		if (cx_cloudGetAuth(&auth.product_key, NULL, &auth.device_name, &auth.device_secret) < 0) {
			break;
		}
		lv_message_adapter_param_s in = {0};
		in.type = LV_MESSAGE_ADAPTER_TYPE_CONNECTED;
		lv_message_adapter(&auth, &in);
		g_LvConnectStatus = 1;
		Wifi_AttreibuteInitReport();
		EVENT_DISPATCH_DEBUG("LV_MESSAGE_ADAPTER_TYPE_CONNECTED connect\n");
#endif
		Module_NetStatusSet(NET_ROUTER_CONNECTING);
		MODULE_UPLOAD_S Info = {0};
		Info.Comm_Cmd = COMM_CMD_H2M_GET_NETCONFIG;
		Module_EventUpLoad(&Info);
	}
	break;
	case W2H_LINK_VIUSAL_DISCONNECT: {
		Module_NetStatusSet(NET_ROUTER_CONNECT_OFF);
		MODULE_UPLOAD_S Info = {0};
		Info.Comm_Cmd = COMM_CMD_H2M_GET_NETCONFIG;
		Module_EventUpLoad(&Info);
	}
	break;
	case W2H_CMD_CONNECT_WIFI_FIRST: {
		Module_NetStatusSet(NET_ROUTER_CONNECTING);
		MODULE_UPLOAD_S Info = {0};
		Info.Comm_Cmd = COMM_CMD_H2M_GET_NETCONFIG;
		Module_EventUpLoad(&Info);
	}
	break;
	case H2W_LINK_VISUAL_RAWDATA:
	case H2W_LINK_VISUAL_PUBLISH:
		EVENT_DISPATCH_DEBUG("%s CMD %d not data\n", __func__, CMD);
		break;
	case H2W_HOST_CMD_GET_VERSION:
		Wifi_Get_Version_Back(Info);
		break;
	default:
		EVENT_DISPATCH_DEBUG("%s err cmd %d\n", __func__, CMD);
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 WIFI_MSGFindNextDataHeadPosition(CVI_U8 *PackData, CVI_U8 *Data, CVI_U32 PackDataLen)
{
	CVI_S32 Position = 0;
	//Type(1B) Len(2B) Data(n) ...
	CVI_U16 *Len = (CVI_U16 *)&Data[1];
	if (((Data + (*Len) + 3) - PackData) >= PackDataLen) {
		return CVI_FAILURE;
	}
	Position = (Data + (*Len) + 3) - PackData;
	return Position;
}

CVI_S32 WP_H2W_HOST_CMD_GET_MAC(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	if (DataLen < 6) {
		EVENT_DISPATCH_DEBUG("%s DataLen err\n", __func__);
		return CVI_FAILURE;
	}
	//Type(1B) Len(2B) MAC[6]
	memcpy(Info.Mac, &Data[3], sizeof(Info.Mac));
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_H2W_HOST_CMD_GET_IP(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	if (DataLen < 13) {
		EVENT_DISPATCH_DEBUG("%s err\n", __func__);
		return CVI_FAILURE;
	}
	memcpy(Info.Net_Data, &Data[3], sizeof(Info.Net_Data));
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_H2W_HOST_CMD_GET_RSSI(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	if (Data) {
		Info.rssi = Data[0];
	}
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_W2H_HOST_CMD_REPORT_WPA_EVENT(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	if (DataLen < 3) {
		EVENT_DISPATCH_DEBUG("%s err\n", __func__);
		return CVI_FAILURE;
	}
	Info.EventInfo.EventData = &Data[3];
	Info.EventInfo.EventDataLen = (*((CVI_U16 *)&Data[1]));
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_H2W_IOT_GET_DEVICEINFO(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;
	CVI_S32 Position = 0;
	char DeviceName[64] = {0};
	Info.DeviceInfo.DeviceName = &RawData[3];
	Info.DeviceInfo.DeviceNameLen = *((CVI_U16 *)&RawData[1]);
	memcpy(DeviceName, &RawData[3], Info.DeviceInfo.DeviceNameLen);
	EVENT_DISPATCH_DEBUG("DeviceName: %s \n", DeviceName);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.DeviceInfo.Device_Secret = &RawData[3];
	Info.DeviceInfo.Device_SecretLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.DeviceInfo.Product_Key = &RawData[3];
	Info.DeviceInfo.Product_KeyLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.DeviceInfo.Product_Secret = &RawData[3];
	Info.DeviceInfo.Product_SecretLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
ErrExit :
	EVENT_DISPATCH_DEBUG("%s ErrExit\n", __func__);
	return CVI_FAILURE;
}

CVI_S32 WP_W2H_IOT_SET_PROPERTY(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.IotReportInfo.Attribute = &Data[3];
	Info.IotReportInfo.AttributeLen = *((CVI_U16 *)&Data[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_W2H_IOT_SET_SERVICE(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;
	CVI_S32 Position = 0;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.IotServerInfo.ID = &RawData[3];
	Info.IotServerInfo.IDLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotServerInfo.ServiceName = &RawData[3];
	Info.IotServerInfo.ServiceNameLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotServerInfo.ServiceData = &RawData[3];
	Info.IotServerInfo.ServiceDataLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
ErrExit :
	EVENT_DISPATCH_DEBUG("%s ErrExit\n", __func__);
	return CVI_FAILURE;
}

CVI_S32 WP_W2H_IOT_REPORT_SERVICE(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;
	CVI_S32 Position = 0;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.IotReportInfo.Index = &RawData[3];
	Info.IotReportInfo.IndexLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotReportInfo.Topic = &RawData[3];
	Info.IotReportInfo.TopicLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotReportInfo.Message = &RawData[3];
	Info.IotReportInfo.MessaegLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
ErrExit :
	EVENT_DISPATCH_DEBUG("%s ErrExit\n", __func__);
	return CVI_FAILURE;
}

CVI_S32 WP_W2H_IOT_USER_SET_SERVICE(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;
	CVI_S32 Position = 0;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.IotServerInfo.ID = &RawData[3];
	Info.IotServerInfo.IDLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotServerInfo.Service = &RawData[3];
	Info.IotServerInfo.ServiceLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotServerInfo.Request = &RawData[3];
	Info.IotServerInfo.RequestLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
ErrExit :
	EVENT_DISPATCH_DEBUG("%s ErrExit\n", __func__);
	return CVI_FAILURE;
}

CVI_S32 WP_W2H_LINK_VISUAL_SERVICE(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;
	CVI_S32 Position = 0;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.IotServerInfo.ID = &RawData[3];
	Info.IotServerInfo.IDLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotServerInfo.Service = &RawData[3];
	Info.IotServerInfo.ServiceLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotServerInfo.Request = &RawData[3];
	Info.IotServerInfo.RequestLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
ErrExit :
	EVENT_DISPATCH_DEBUG("%s ErrExit\n", __func__);
	return CVI_FAILURE;
}

CVI_S32 WP_W2H_CMD_CONNECT_WIFI_FIRST(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};

	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_H2W_LINK_VISUAL_RAWDATA(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};

	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_W2H_LINK_VISUAL_USER(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;
	CVI_S32 Position = 0;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.LVPublishInfo.ID = &RawData[3];
	Info.LVPublishInfo.IDLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.LVPublishInfo.Service = &RawData[3];
	Info.LVPublishInfo.ServiceLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
ErrExit :
	EVENT_DISPATCH_DEBUG("%s ErrExit\n", __func__);
	return CVI_FAILURE;
}

CVI_S32 WP_W2H_LINK_VIUSAL_CONNECT(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};

	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_W2H_LINK_VIUSAL_DISCONNECT(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};

	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_W2H_IOT_DEVICE_CONNECT(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;
	CVI_S32 Position = 0;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.IotServerInfo.ID = &RawData[3];
	Info.IotServerInfo.IDLen = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.IotServerInfo.Service = &RawData[3];
	Info.IotServerInfo.ServiceLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
ErrExit :
	EVENT_DISPATCH_DEBUG("%s ErrExit\n", __func__);
	return CVI_FAILURE;
}

CVI_S32 WP_H2W_HOST_CMD_WIFI_OTA_START(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.DataInfo.Data = &RawData[3];
	Info.DataInfo.DataLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_H2W_HOST_CMD_WIFI_OTA(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.DataInfo.Data = &RawData[3];
	Info.DataInfo.DataLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_W2H_HOST_CMD_ALL_OTA_START(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.DataInfo.Data = &RawData[3];
	Info.DataInfo.DataLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_H2W_HOST_CMD_ALL_OTA(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.DataInfo.Data = &RawData[3];
	Info.DataInfo.DataLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_H2W_HOST_TEST_RSSI(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;
	CVI_S32 Position = 0;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.ConnectInfo.SSID = &RawData[3];
	Info.ConnectInfo.SSIDLEN = *((CVI_U16 *)&RawData[1]);
	Position = WIFI_MSGFindNextDataHeadPosition(Data, RawData, DataLen);
	if (Position == CVI_FAILURE)
		goto ErrExit;
	RawData = Data + Position;
	Info.ConnectInfo.RSSI = &RawData[3];
	Info.ConnectInfo.RSSILEN = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
ErrExit :
	EVENT_DISPATCH_DEBUG("%s ErrExit\n", __func__);
	return CVI_FAILURE;
}

CVI_S32 WP_H2W_HOST_CMD_HEART(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};

	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}

CVI_S32 WP_W2H_HOST_CMD_UPDATEINFO(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.DataInfo.Data = &RawData[3];
	Info.DataInfo.DataLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}


CVI_S32 WP_H2W_HOST_CMD_GET_VERSION(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	WIFI_MSGINFO_S Info = {0};
	CVI_U8 *RawData = Data;

	if (!Data) {
		EVENT_DISPATCH_DEBUG("%s data null\n", __func__);
		return CVI_FAILURE;
	}
	Info.DataInfo.Data = &RawData[3];
	Info.DataInfo.DataLen = *((CVI_U16 *)&RawData[1]);
	WifiMsgInfo_CallBack(CMD, &Info);
	return CVI_SUCCESS;
}


CVI_S32 WifiMsg_PackPro(CVI_U32 CMD, CVI_U8 *Data, CVI_U32 DataLen)
{
	//无填充内容代表 该函数无响应
	switch (CMD) {
	case H2W_HOST_CMD_GET_MAC:
		WIFIMSGPRASEFUN(H2W_HOST_CMD_GET_MAC)(CMD, Data, DataLen);
		break;
	case H2W_HOST_CMD_GET_IP:
		WIFIMSGPRASEFUN(H2W_HOST_CMD_GET_IP)(CMD, Data, DataLen);
		break;
	case H2W_HOST_CMD_GET_RSSI:
		WIFIMSGPRASEFUN(H2W_HOST_CMD_GET_RSSI)(CMD, Data, DataLen);
		break;
	case W2H_HOST_CMD_REPORT_WPA_EVENT:
		WIFIMSGPRASEFUN(W2H_HOST_CMD_REPORT_WPA_EVENT)(CMD, Data, DataLen);
		break;
	case H2W_IOT_GET_DEVICEINFO:
		WIFIMSGPRASEFUN(H2W_IOT_GET_DEVICEINFO)(CMD, Data, DataLen);
		break;
	case W2H_IOT_SET_PROPERTY:
		WIFIMSGPRASEFUN(W2H_IOT_SET_PROPERTY)(CMD, Data, DataLen);
		break;
	case W2H_IOT_SET_SERVICE:
		WIFIMSGPRASEFUN(W2H_IOT_SET_SERVICE)(CMD, Data, DataLen);
		break;
	case W2H_IOT_REPORT_SERVICE:
		WIFIMSGPRASEFUN(W2H_IOT_REPORT_SERVICE)(CMD, Data, DataLen);
		break;
	case W2H_IOT_USER_SET_SERVICE:
		WIFIMSGPRASEFUN(W2H_IOT_USER_SET_SERVICE)(CMD, Data, DataLen);
		break;
	case W2H_LINK_VISUAL_SERVICE:
		WIFIMSGPRASEFUN(W2H_LINK_VISUAL_SERVICE)(CMD, Data, DataLen);
		break;
	case W2H_CMD_CONNECT_WIFI_FIRST:
		WIFIMSGPRASEFUN(W2H_CMD_CONNECT_WIFI_FIRST)(CMD, Data, DataLen);
		break;
	case H2W_LINK_VISUAL_RAWDATA:
		WIFIMSGPRASEFUN(H2W_LINK_VISUAL_RAWDATA)(CMD, Data, DataLen);
		break;
	case W2H_LINK_VISUAL_USER:
		WIFIMSGPRASEFUN(W2H_LINK_VISUAL_USER)(CMD, Data, DataLen);
		break;
	case W2H_LINK_VIUSAL_CONNECT:
		WIFIMSGPRASEFUN(W2H_LINK_VIUSAL_CONNECT)(CMD, Data, DataLen);
		break;
	case W2H_LINK_VIUSAL_DISCONNECT:
		WIFIMSGPRASEFUN(W2H_LINK_VIUSAL_DISCONNECT)(CMD, Data, DataLen);
		break;
	case W2H_IOT_DEVICE_CONNECT:
		WIFIMSGPRASEFUN(W2H_IOT_DEVICE_CONNECT)(CMD, Data, DataLen);
		break;
	case H2W_HOST_CMD_WIFI_OTA_START:
		WIFIMSGPRASEFUN(H2W_HOST_CMD_WIFI_OTA_START)(CMD, Data, DataLen);
		break;
	case H2W_HOST_CMD_WIFI_OTA:
		WIFIMSGPRASEFUN(H2W_HOST_CMD_WIFI_OTA)(CMD, Data, DataLen);
		break;
	case W2H_HOST_CMD_ALL_OTA_START:
		WIFIMSGPRASEFUN(W2H_HOST_CMD_ALL_OTA_START)(CMD, Data, DataLen);
		break;
	case H2W_HOST_CMD_ALL_OTA:
		WIFIMSGPRASEFUN(H2W_HOST_CMD_ALL_OTA)(CMD, Data, DataLen);
		break;
	case H2W_HOST_TEST_RSSI:
		WIFIMSGPRASEFUN(H2W_HOST_TEST_RSSI)(CMD, Data, DataLen);
		break;
	case H2W_HOST_CMD_HEART:
		WIFIMSGPRASEFUN(H2W_HOST_CMD_HEART)(CMD, Data, DataLen);
		break;
	case W2H_HOST_CMD_UPDATEINFO:
		WIFIMSGPRASEFUN(W2H_HOST_CMD_UPDATEINFO)(CMD, Data, DataLen);
		break;
	case H2W_HOST_CMD_GET_VERSION:
		WIFIMSGPRASEFUN(H2W_HOST_CMD_GET_VERSION)(CMD, Data, DataLen);
		break;
	case H2W_HOST_CMD_STOP_STA:
	case H2W_HOST_CMD_START_STA:
	case H2W_HOST_CMD_CLEAN_WIFI:
	case H2W_HOST_CMD_CLEAN_DEVICEINFO:
	case H2W_HOST_CMD_CLEAN_ALL:
	case H2W_IOT_SET_DEVICEINFO:
	case H2W_IOT_REPORT_PROPERTY:
	case H2W_IOT_EVENT_NOTIFY:
	case H2W_IOT_USER_NOTIFY_EVENT:
	case H2W_LINK_VISUAL_INIT_SUCCESS:
	case H2W_HOST_CMD_SET_VERSION:
	case H2W_HOST_CMD_WANK_UP:
	case H2W_HOST_CMD_WIFI_SLEEP_TIME:
	case H2W_HOST_CMD_ZEROCONFIG:
	case H2W_HOST_CMD_REBOOT:
	case H2W_HOST_CMD_UPDATEPROGRESS:
	case H2W_HOST_CMD_SET_FILTER:
	case H2W_HOST_CMD_CONNECT:
	case H2W_LINK_VISUAL_PUBLISH:
	default:
		EVENT_DISPATCH_DEBUG("%s err cmd %d\n", __func__, CMD);
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 WifiMsg_Prase(CVI_U8 *Data, CVI_U32 Len)
{
	if (!Data || Len == 0) {
		EVENT_DISPATCH_DEBUG("%s err\n", __func__);
		return CVI_FAILURE;
	}
	CVI_U32 CMD = Data[0];
	CVI_U8 *PackData = NULL;
	if (Len > 1) {
		PackData = &Data[1];
	}
	WifiMsg_PackPro(CMD, PackData, Len - 1);
	return CVI_SUCCESS;
}
