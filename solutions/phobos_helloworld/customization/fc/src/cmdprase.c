/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:
 * Description:
 *   ....
 */
#include <string.h>
#include <stdlib.h>
#include <aos/kernel.h>
#include "cvi_type.h"
#include "event_def.h"
#include "event_dispatch.h"
#include "uart_communication.h"
#include "module_test.h"
#include "global_sleep_flag.h"


CVI_S32 ModuleResponePack(CVI_U8 *InBuf, CVI_U32 CMD, CVI_U8 *Buf, CVI_U32 BufLength)
{
	memset(InBuf, 0, COMM_PACKET_SIZE + BufLength);
	CVI_U32 ResponeLen = 0;
	H_M_COMM_CMD_PACK_T *PackHead = (H_M_COMM_CMD_PACK_T *)(InBuf);
	CVI_U16 Sum = 0;
	PackHead->FrameHead[0] = COMM_PACKET_HEAD_FIRST;
	PackHead->FrameHead[1] = COMM_PACKET_HEAD_SECOND;
	PackHead->CMD = CMD;
	PackHead->PackIdentification = COMM_PACKET_PID_M2H; //Module标识
	PackHead->Sum = 0;
	PackHead->Length = (CVI_U16)BufLength;
	if (Buf)
		memcpy(InBuf + COMM_PACKET_SIZE, Buf, BufLength);
	Sum = Module_CheckSum(InBuf, BufLength + COMM_PACKET_SIZE);
	PackHead->Sum = Sum;
	ResponeLen = PackHead->Length + COMM_PACKET_SIZE;
	return ResponeLen;
}

CVI_S32 Query_AgreemtVer_Prase()
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 Data[4] = {0};
	CVI_U8 *pPlatformVer = &Data[1];
	CVI_U8 *pMainVer = &Data[2];
	CVI_U8 *pSubVer = &Data[3];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_QUERY_AGREEMT_VER;
	Host2Module_DataCallBack(&ModuleParm);
	*pPlatformVer = ModuleParm.VerInfo.Platform;
	*pMainVer = ModuleParm.VerInfo.Main_Ver;
	*pSubVer = ModuleParm.VerInfo.Sub_Ver;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_QUERY_AGREEMT_VER, Data, sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 ConfigNetCmdAck(CVI_U8 ret)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;

	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_SET_NETCONFIG, &ret, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 ConfigNetCmdPrase(CVI_U8 *PackBuf, CVI_U16 PackBufLen)
{
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_SET_NETCONFIG;
	ModuleParm.NetCfg.ConfigMode = PackBuf[0];
	if (ModuleParm.NetCfg.ConfigMode == 0x01) {
		ModuleParm.Comm_Cmd = COMM_CMD_H2M_SET_NETCONFIG;
		memcpy(ModuleParm.NetCfg.SSID, &PackBuf[1], 32);
		memcpy(ModuleParm.NetCfg.Password, &PackBuf[33], 24);
	}

	Host2Module_DataCallBack(&ModuleParm);
	EVENT_DISPATCH_DEBUG("%s ConfigMode %d SSID %s Paasword %s\n", __func__, ModuleParm.NetCfg.ConfigMode,
	       ModuleParm.NetCfg.SSID, ModuleParm.NetCfg.Password);
	ConfigNetCmdAck(RES_ERRORCODE_CMD_OK);
	return CVI_SUCCESS;
}

CVI_S32 GetNetStatusAck(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 DataBuf[2] = {0};
	CVI_U32 ResponeLen = 0;

	DataBuf[0] = 0; // 错误码 没有定义
	MODULE_PARAM_S ModuleParm = {0};
	ModuleParm.Comm_Cmd = COMM_CMD_H2M_GET_NETCONFIG ;

	Host2Module_DataCallBack(&ModuleParm);
	DataBuf[1] = ModuleParm.NetStatus;
	if (DataBuf[0] == RES_ERRORCODE_CMD_OK)
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_NETCONFIG, DataBuf, sizeof(DataBuf));
	else
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_NETCONFIG, DataBuf, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	EVENT_DISPATCH_DEBUG("%s NetStatus %d\n", __func__, ModuleParm.NetStatus);
	return CVI_SUCCESS;

}

CVI_S32 GetVideoInfoAck(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 Data[1 + sizeof(VIDEO_CONFIG_INFO_S)] = {0};
	CVI_U8 *pErrorCode = &Data[0];
	VIDEO_CONFIG_INFO_S *Vfcg = (VIDEO_CONFIG_INFO_S *)&Data[1];

	Data[0] = 0x00;
	//Note 这里获取主码流视频参数 Note
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_GET_VIDEOINFO;
	*pErrorCode = Host2Module_DataCallBack(&ModuleParm);

	Vfcg->Video_Encode = ModuleParm.VideoParm.Video_Encode;
	Vfcg->Video_Size = ModuleParm.VideoParm.Video_Size;
	Vfcg->Framerate = ModuleParm.VideoParm.Framerate;
	if (pErrorCode == RES_ERRORCODE_CMD_OK) {
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_VIDEOINFO, Data, sizeof(Data));
	} else {
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_VIDEOINFO, Data, 1);
	}
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	EVENT_DISPATCH_DEBUG("%s VideoEncode %d Size %d FrameRate %d", __func__,
	       Vfcg->Video_Encode, Vfcg->Video_Size, Vfcg->Framerate);
	return CVI_SUCCESS;

}

CVI_S32 SetVideoInfoAck(CVI_U8 RetCode)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;

	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_SET_VIDEOINFO, &RetCode, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 SetVideoInfoPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength)
{
	VIDEO_CONFIG_INFO_S *Vfcg = (VIDEO_CONFIG_INFO_S *)PackBuf;
	CVI_U32 Ret = RES_ERRORCODE_CMD_OK;
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_SET_VIDEOINFO;
	ModuleParm.VideoParm.Video_Encode = Vfcg->Video_Encode;
	ModuleParm.VideoParm.Framerate = Vfcg->Video_Encode;
	ModuleParm.VideoParm.Video_Size = Vfcg->Video_Size;
	Host2Module_DataCallBack(&ModuleParm);
	EVENT_DISPATCH_DEBUG("%s Video_Encode :%d ,Framerate : %d Video_Size %d\n", __func__, Vfcg->Video_Encode,
		   Vfcg->Framerate, Vfcg->Video_Size);
	SetVideoInfoAck(Ret);
	return CVI_SUCCESS;

}

CVI_S32 GetPictureInfoAck(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[3] = {0};
	PICTURE_CONFIG_INFO_E *p_ImgCfg = (PICTURE_CONFIG_INFO_E *)(&DataBuf[1]);
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_GET_PICTUREINFO;
	DataBuf[0] = Host2Module_DataCallBack(&ModuleParm);
	p_ImgCfg->Picture_Type = ModuleParm.picparm.Picture_Type;
	p_ImgCfg->Picture_Size = ModuleParm.picparm.Picture_Size;
	if (DataBuf[0] != RES_ERRORCODE_CMD_OK)
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_PICTUREINFO, DataBuf, 1);
	else
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_PICTUREINFO, DataBuf, sizeof(DataBuf));
	EVENT_DISPATCH_DEBUG("%s Picture_Type is %d Picture_Size is %d\n", __func__,
	       p_ImgCfg->Picture_Type, p_ImgCfg->Picture_Size);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 SetPictureInfoAck(CVI_U8 RetCode)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;

	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_SET_PICTUREINFO, &RetCode, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 SetPictureInfoPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength)
{
	PICTURE_CONFIG_INFO_E *p_Imgcfg = (PICTURE_CONFIG_INFO_E *)PackBuf;
	CVI_U8 RetCode = RES_ERRORCODE_CMD_OK;

	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_SET_PICTUREINFO;
	ModuleParm.picparm.Picture_Type = p_Imgcfg->Picture_Type;
	ModuleParm.picparm.Picture_Size = p_Imgcfg->Picture_Size;
	RetCode = Host2Module_DataCallBack(&ModuleParm);
	EVENT_DISPATCH_DEBUG("%s: Picture_Type :%d ,Picture_Size : %d\n", __func__, p_Imgcfg->Picture_Type,
		   p_Imgcfg->Picture_Size);
	SetPictureInfoAck(RetCode);//0:成功  1:图片格式不支持 2:图像大小错误
	return CVI_SUCCESS;
}

CVI_S32 GetAudioInfoAck(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[3] = {0};
	CVI_U8 *RetCode = &DataBuf[0]; //0:已配置 1:音频参数未配置
	AUDIO_CONFIG_INFO_S *Acfg = (AUDIO_CONFIG_INFO_S *)(&DataBuf[1]);
	*RetCode = RES_ERRORCODE_0; //配置成功
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_GET_AUDIOINFO;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	Acfg->Audio_EncType = ModuleParm.AudioParm.Audio_EncType;
	Acfg->Sample_Rate = ModuleParm.AudioParm.Sample_Rate;

	if (*RetCode == 0)
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_AUDIOINFO, DataBuf, 3);
	else
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_AUDIOINFO, DataBuf, 1);
	EVENT_DISPATCH_DEBUG("%s : RetCode %d Audio_EncType %d Sample_Rate %d\n", __func__,
	       *RetCode, Acfg->Audio_EncType, Acfg->Sample_Rate);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 SetAudioInfoAck(CVI_U8 RetCode)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;

	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_SET_AUDIOINFO, &RetCode, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 SetAudioInfoPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength)
{
	AUDIO_CONFIG_INFO_S *Acfg = (AUDIO_CONFIG_INFO_S *)PackBuf;
	CVI_U8 RetCode = RES_ERRORCODE_0;
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_SET_AUDIOINFO;
	ModuleParm.AudioParm.Audio_EncType = Acfg->Audio_EncType;
	ModuleParm.AudioParm.Sample_Rate = Acfg->Sample_Rate;
	RetCode = Host2Module_DataCallBack(&ModuleParm);
	EVENT_DISPATCH_DEBUG("%s Audio_Enc :%d , Sample_Rate : %d\n", __func__, Acfg->Audio_EncType, Acfg->Sample_Rate);
	SetAudioInfoAck(RetCode);
	return CVI_SUCCESS;
}

CVI_S32 StartStreamPushPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[3] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	CVI_U16 *pDuration = (CVI_U16 *)(&DataBuf[1]);
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_START_STREAM;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	*pDuration = ModuleParm.Duration;
	if (*RetCode == RES_ERRORCODE_CMD_OK)
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_START_STREAM, DataBuf, sizeof(DataBuf));
	else
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_START_STREAM, DataBuf, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	EVENT_DISPATCH_DEBUG("%s Code %d Duration %d\n", __func__,
	       *RetCode, ModuleParm.Duration);
	return CVI_SUCCESS;
}

CVI_S32 StopStreamPushPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[1] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_STOP_STREAM;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_STOP_STREAM, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	EVENT_DISPATCH_DEBUG("%s Code %d\n", __func__, *RetCode);
	return CVI_SUCCESS;
}

CVI_S32 GetStreamStausPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[2] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	CVI_U8 *PushStreamStatus = &DataBuf[1];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_GET_STREAM_STATUS;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	*PushStreamStatus = ModuleParm.StreamStaus;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_STREAM_STATUS, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	EVENT_DISPATCH_DEBUG("%s Code %d StreamStatus %d\n", __func__, *RetCode, *PushStreamStatus);
	return CVI_SUCCESS;
}

CVI_S32 StartIpcTalkPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[3] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	CVI_U16 *pDuration = (CVI_U16 *)(&DataBuf[1]);
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_START_IPC_TALK;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	*pDuration = ModuleParm.Duration;
	if (*RetCode != RES_ERRORCODE_CMD_OK)
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_START_IPC_TALK, DataBuf, sizeof(DataBuf));
	else
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_START_IPC_TALK, DataBuf, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	EVENT_DISPATCH_DEBUG("%s Code %d Duration %d\n", __func__, *RetCode, *pDuration);
	return CVI_SUCCESS;
}

CVI_S32 StopIpcTalkPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[1] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_STOP_IPC_TALK;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_STOP_IPC_TALK, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	EVENT_DISPATCH_DEBUG("%s Code %d \n", __func__, *RetCode);
	return CVI_SUCCESS;
}

CVI_S32 GetIpcTalkStatusPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[2] = {0};
	CVI_U8 *TalkStatus = &DataBuf[1];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_GET_IPC_TALK_STATUS;
	Host2Module_DataCallBack(&ModuleParm);
	*TalkStatus = ModuleParm.TalkStatus;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_IPC_TALK_STATUS, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	EVENT_DISPATCH_DEBUG("%s TalkStatus %d\n", __func__, *TalkStatus);
	return CVI_SUCCESS;
}

CVI_S32 Snap_Picture()
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[1] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	MODULE_PARAM_S ModuleParm = {0};
	ModuleParm.Comm_Cmd = COMM_CMD_H2M_START_SNAP_PICTURE;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_START_SNAP_PICTURE, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}


CVI_S32 Capture_Picture_Prase(CVI_U8 *PackBuf, CVI_U32 PackBufLength)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[1] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	MODULE_PARAM_S ModuleParm = {0};

	memcpy(ModuleParm.CaptureInfo.CaptureType, PackBuf, 4);
	memcpy(&ModuleParm.CaptureInfo.Datalen, &PackBuf[4], 2);
	ModuleParm.CaptureInfo.Data = &PackBuf[6];
	ModuleParm.Comm_Cmd = COMM_CMD_H2M_PIC_CAPTURE;
	if (1 == get_snap_flag()) {
		*RetCode = 1;
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_PIC_CAPTURE, DataBuf, sizeof(DataBuf));
		Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	} else {
		set_snap_timerwait();
		*RetCode = Host2Module_DataCallBack(&ModuleParm);
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_PIC_CAPTURE, DataBuf, sizeof(DataBuf));
		Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	}

	return CVI_SUCCESS;
}

CVI_S32 DataTransferPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[1] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_DATA_TRANSFER;
	ModuleParm.RawData.Data = PackBuf;
	ModuleParm.RawData.Datalen = PackBufLength;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_DATA_TRANSFER, DataBuf, sizeof(DataBuf));
	EVENT_DISPATCH_DEBUG("%s Code %d\n", __func__, *RetCode);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 GetFaceModuleMacPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[7] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	CVI_U8 *FaceModuleMac = &DataBuf[1];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_GET_WIFIMAC;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	memcpy(FaceModuleMac, ModuleParm.MAC, 6);
	if (*RetCode == RES_ERRORCODE_CMD_OK)
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_WIFIMAC, DataBuf, sizeof(DataBuf));
	else
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_WIFIMAC, DataBuf, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 GetAliYunParmPrase(void)
{
	/*
	CVI_U8 ResponeBuf[512] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8  DataBuf[256] = {0};
	CVI_U8  DataLen = 0;
	*/
	CVI_U8  DataBuf[256] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_GET_ALIYUN_PARAM;
	*RetCode = (CVI_U8)Host2Module_DataCallBack(&ModuleParm);
	/*
	CVI_U8 Product_Secret_Len = (ModuleParm.AliyunParm.Product_Secret_Len + 1) > sizeof(
					    ModuleParm.AliyunParm.Product_Secret) ? sizeof(ModuleParm.AliyunParm.Product_Secret) :
				    ModuleParm.AliyunParm.Product_Secret_Len + 1;
	CVI_U8 Product_Key_Len = (ModuleParm.AliyunParm.Product_Key_Len + 1) > sizeof(ModuleParm.AliyunParm.Product_Key) ?
				 sizeof(ModuleParm.AliyunParm.Product_Key) :
				 ModuleParm.AliyunParm.Product_Key_Len+1;
	CVI_U8 Device_Name_Len = (ModuleParm.AliyunParm.Device_Name_Len + 1) > sizeof(ModuleParm.AliyunParm.Device_Name) ?
				 sizeof(ModuleParm.AliyunParm.Device_Name) :
				 ModuleParm.AliyunParm.Device_Name_Len+1;
	CVI_U8 Device_Secret_Len = (ModuleParm.AliyunParm.Device_Secret_Len + 1) > sizeof(ModuleParm.AliyunParm.Device_Secret) ?
				   sizeof(ModuleParm.AliyunParm.Device_Secret) :
				   ModuleParm.AliyunParm.Device_Secret_Len + 1;

	DataBuf[1] = Product_Secret_Len;
	CVI_U8 *pProduct_Secret = &DataBuf[1 + 1];
	DataBuf[1 + 1 + Product_Secret_Len] = Product_Key_Len;
	CVI_U8 *pProduct_Key = &DataBuf[1 + 1 + Product_Secret_Len + 1];
	DataBuf[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len] = Device_Name_Len;
	CVI_U8 *pDevice_Name = &DataBuf[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1];
	DataBuf[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1 + Device_Name_Len] = Device_Secret_Len;
	CVI_U8 *pDevice_Secret = &DataBuf[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1 + Device_Name_Len + 1];
	memcpy(pProduct_Secret, ModuleParm.AliyunParm.Product_Secret, Product_Secret_Len);
	memcpy(pProduct_Key, ModuleParm.AliyunParm.Product_Key, Product_Key_Len);
	memcpy(pDevice_Name, ModuleParm.AliyunParm.Device_Name, Device_Name_Len);
	memcpy(pDevice_Secret, ModuleParm.AliyunParm.Device_Secret, Device_Secret_Len);
	DataLen = 1 + Product_Secret_Len + 1 + Product_Key_Len + 1 + Device_Name_Len + 1 + Device_Secret_Len+1;
	if (*RetCode == RES_ERRORCODE_CMD_OK)
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_ALIYUN_PARAM, DataBuf, DataLen);
	else
		ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_GET_ALIYUN_PARAM, DataBuf, 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	*/
	EVENT_DISPATCH_DEBUG("%s code %d\n", __func__, *RetCode);
	return CVI_SUCCESS;
}

CVI_S32 SetAliYunParmPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8  DataBuf[1] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_SET_ALIYUN_PARAM;
	CVI_U8 SecretLen = PackBuf[0];
	CVI_U8 ductKeyLen = PackBuf[1 + SecretLen];
	CVI_U8 DevNameLen = PackBuf[1 + SecretLen + 1 + ductKeyLen];
	CVI_U8 DevSecretLen = PackBuf[1 + SecretLen + 1 + ductKeyLen + 1 + DevNameLen];
	CVI_U8 *pProduct_Secret = &PackBuf[1];
	CVI_U8 *pProduct_Key = &PackBuf[1 + SecretLen + 1];
	CVI_U8 *pDevice_Name = &PackBuf[1 + SecretLen + 1 + ductKeyLen + 1];
	CVI_U8 *pDevice_Secret = &PackBuf[1 + SecretLen + 1 + ductKeyLen + 1 + DevNameLen + 1];
	ModuleParm.AliyunParm.Device_Name_Len = DevNameLen;
	ModuleParm.AliyunParm.Device_Secret_Len = DevSecretLen;
	ModuleParm.AliyunParm.Product_Key_Len = ductKeyLen;
	ModuleParm.AliyunParm.Product_Secret_Len = SecretLen;
	memcpy(ModuleParm.AliyunParm.Product_Secret, pProduct_Secret, SecretLen);
	memcpy(ModuleParm.AliyunParm.Product_Key, pProduct_Key, ductKeyLen);
	memcpy(ModuleParm.AliyunParm.Device_Name, pDevice_Name, DevNameLen);
	memcpy(ModuleParm.AliyunParm.Device_Secret, pDevice_Secret, DevSecretLen);
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_SET_ALIYUN_PARAM, DataBuf, sizeof(DataBuf));
	EVENT_DISPATCH_DEBUG("%s code %d\n", __func__, *RetCode);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

static CVI_S32 Module_ReportPowerOn()
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8  DataBuf[2] = {0};

	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_M2H_REPORT_POWERON, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

static CVI_S32 Module_ReportStreamStatus(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8  DataBuf[2] = {0};

	DataBuf[1] = Info->StreamStatus;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_M2H_REPORT_STREAM_STATUS, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

static CVI_S32 Module_ReportTalkStatus(const  MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8  DataBuf[2] = {0};

	DataBuf[1] = Info->TalkStauts;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_M2H_REPORT_IPC_TALK_STATUS, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

static CVI_S32 Module_ReportServerData(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[1024] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[512] = {0};
	CVI_U8 DataLen = 0;
	CVI_U8 *pError = &DataBuf[0];
	*pError = 0;
	DataLen = Info->RawData.Datalen > sizeof(DataBuf) - 1 ? sizeof(DataBuf) - 1 : Info->RawData.Datalen;
	memcpy(&DataBuf[1], Info->RawData.Data, DataLen);
	DataLen += 1;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_M2H_DATA_TRANSFER, DataBuf,
				       DataLen);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_ReportGetKeyInfo(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[5] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 *KeyType = &Data[4];
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;

	*KeyType = Info->KeyType;
	pHead->AliHeadCmd = M2H_QUERY_KEYINFO;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_ReportAddKey(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[6] = {0};
	CVI_U8 *plockType = &Data[4];
	CVI_U8 *puserLimt = &Data[5];
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;

	*plockType = Info->KeyCfg.lockType;
	*puserLimt = Info->KeyCfg.userLimit;
	pHead->AliHeadCmd = M2H_ADD_KEY;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_ReportDeleteKey(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[7] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U16 *pKeyId = (CVI_U16 *)&Data[4];
	CVI_U8 *plockType = &Data[6];
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;

	*pKeyId = Info->KeyCfg.keyID;
	*plockType = Info->KeyCfg.lockType;
	pHead->AliHeadCmd = M2H_DELETE_KEY;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_ReportGetTmpPasswd(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[4] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;

	pHead->AliHeadCmd = M2H_QUERY_TMP_PWD;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_ReportTmpPasswd(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[6] = {0};
	LOCK_TMP_PWD_TCFG_S *pPassST = (LOCK_TMP_PWD_TCFG_S *)&Data[4];
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;

	pHead->AliHeadCmd = M2H_UPDATE_TMP_PWD_STATUS;
	pPassST->tempPassWordId = Info->PassST.tempPassWordId;
	pPassST->tempPassWordStatus = Info->PassST.tempPassWordStatus;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_ReportUpdataGenCfg(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[5] = {0};
	CVI_U8 *pGenCfg = &Data[4];
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;

	pHead->AliHeadCmd = M2H_UPDATE_GENCFG;
	*pGenCfg = Info->GenCfg;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_ReportSetKeyCfg(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[8] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;
	LOCK_KEYCFG_S *pKeyCfg = (LOCK_KEYCFG_S *)&Data[4];

	pHead->AliHeadCmd = M2H_SET_KEY_PER;
	pKeyCfg->keyID = Info->KeyCfg.keyID;
	pKeyCfg->lockType = Info->KeyCfg.lockType;
	pKeyCfg->userLimit = Info->KeyCfg.userLimit;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_ReportUpdataKeyInfo(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[40] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;
	LOCK_KEY_INFO_NODE_S *pNode = (LOCK_KEY_INFO_NODE_S *)&Data[4];
	CVI_U8 *pKeyAlias = &Data[7];

	pHead->AliHeadCmd = M2H_MODIFY_KEY_INFO;
	pNode->keyID = Info->KeyNodeInfo.keyID;
	pNode->lockType = Info->KeyNodeInfo.lockType;
	memcpy(pKeyAlias, Info->KeyNodeInfo.KeyAlias, sizeof(pNode->KeyAlias));
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_SetAttributeSetting(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 *pResponeBuf = NULL;
	CVI_U8 *pData = NULL;
	CVI_U16 *plen = NULL;
	CVI_U16 DataLen = 0;
	CVI_U32 ResponeLen = 0;
	CVI_S32  Ret = CVI_SUCCESS;

	DataLen = Info->RawData.Datalen;
	pResponeBuf = (CVI_U8 *)malloc(COMM_PACKET_SIZE + DataLen + 2 + sizeof(ALIYUN_CMD_PACK_S));
	if (!pResponeBuf) {
		EVENT_DISPATCH_DEBUG("%s:malloc pResponeBuf Err\n", __func__);
		Ret = CVI_FAILURE;
		goto EXIT;
	}
	pData = (CVI_U8 *)malloc(DataLen + 2 + sizeof(ALIYUN_CMD_PACK_S));
	if (!pData) {
		EVENT_DISPATCH_DEBUG("%s:malloc pData Err\n", __func__);
		Ret = CVI_FAILURE;
		goto EXIT;
	}
	memset(pData, 0, DataLen + 2 + sizeof(ALIYUN_CMD_PACK_S));
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)pData;
	pHead->AliHeadCmd = M2H_SET_ATTRIBUTE_SETTING;
	plen = (CVI_U16 *)(&pData[4]);
	*plen = Info->RawData.Datalen;
	memcpy(pData + 2 + sizeof(ALIYUN_CMD_PACK_S), Info->RawData.Data, Info->RawData.Datalen);
	ResponeLen = ModuleResponePack(pResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, pData,
				       DataLen + 2 + sizeof(ALIYUN_CMD_PACK_S));
	Event_dispath_DataSend(pResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
EXIT:
	if (pData)
		free(pData);
	if (pResponeBuf)
		free(pResponeBuf);
	return Ret;
}

CVI_S32 Module_AckUpdateAttibutePack(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[5] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;
	CVI_U8 *pErroCode = &Data[4];

	*pErroCode = Info->ErrorCode;
	pHead->AliHeadCmd = M2H_ACK_UPDATE_ATTRIBUTEPACK;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_SetServerCommand(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 *pResponeBuf = NULL;
	CVI_U8 *pData = NULL;
	CVI_U16 *plen = NULL;
	CVI_U16 DataLen = 0;
	CVI_U32 ResponeLen = 0;
	CVI_S32  Ret = CVI_SUCCESS;

	DataLen = Info->RawData.Datalen;
	pResponeBuf = (CVI_U8 *)malloc(COMM_PACKET_SIZE + DataLen + 2 + sizeof(ALIYUN_CMD_PACK_S));
	if (!pResponeBuf) {
		EVENT_DISPATCH_DEBUG("%s:malloc pResponeBuf Err\n", __func__);
		Ret = CVI_FAILURE;
		goto EXIT;
	}
	pData = (CVI_U8 *)malloc(DataLen + 2 + sizeof(ALIYUN_CMD_PACK_S));
	if (!pData) {
		EVENT_DISPATCH_DEBUG("%s:malloc pData Err\n", __func__);
		Ret = CVI_FAILURE;
		goto EXIT;
	}
	memset(pData, 0, DataLen + 2 + sizeof(ALIYUN_CMD_PACK_S));
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)pData;
	pHead->AliHeadCmd = M2H_SET_SERVER_COMMOAND;
	plen = (CVI_U16 *)(&pData[4]);
	*plen = Info->RawData.Datalen;
	memcpy(pData + 2 + sizeof(ALIYUN_CMD_PACK_S), Info->RawData.Data, Info->RawData.Datalen);
	ResponeLen = ModuleResponePack(pResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, pData,
				       DataLen + 2 + sizeof(ALIYUN_CMD_PACK_S));
	Event_dispath_DataSend(pResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
EXIT:
	if (pData)
		free(pData);
	if (pResponeBuf)
		free(pResponeBuf);
	return Ret;
}

CVI_S32 Module_Ack_Server_Pack(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[5] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;
	CVI_U8 *pErrorCode = &Data[4];

	pHead->AliHeadCmd = M2H_ACK_SERVER_PACK;
	*pErrorCode = Info->ErrorCode;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_AckEvent(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[5] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;
	CVI_U8 *pErrorCode = &Data[4];

	pHead->AliHeadCmd = M2H_ACK_EVENT;
	*pErrorCode = Info->ErrorCode;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Moudle_SET_UPGRADE(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[6] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;
	CVI_U16 *pUpgradePackLen = (CVI_U16 *)&Data[4];

	*pUpgradePackLen = Info->RawData.Datalen;
	pHead->AliHeadCmd = M2H_SET_UPGRADE;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_Update_UpgradePack(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 *pResponeBuf = NULL;
	CVI_U8 *pData = NULL;
	CVI_U16 *pPacknumber = NULL;
	CVI_U16 *pPackSeq = NULL;
	CVI_U16 *plen = NULL;
	CVI_U16 DataLen = 0;
	CVI_U32 ResponeLen = 0;
	CVI_S32  Ret = CVI_SUCCESS;

	DataLen = Info->PackParm.len;
	pResponeBuf = (CVI_U8 *)malloc(COMM_PACKET_SIZE + DataLen + 6 + sizeof(ALIYUN_CMD_PACK_S));
	if (!pResponeBuf) {
		EVENT_DISPATCH_DEBUG("%s:malloc pResponeBuf Err\n", __func__);
		Ret = CVI_FAILURE;
		goto EXIT;
	}
	pData = (CVI_U8 *)malloc(DataLen + 6 + sizeof(ALIYUN_CMD_PACK_S));
	if (!pData) {
		EVENT_DISPATCH_DEBUG("%s:malloc pData Err\n", __func__);
		Ret = CVI_FAILURE;
		goto EXIT;
	}
	memset(pData, 0, DataLen + 6 + sizeof(ALIYUN_CMD_PACK_S));
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)pData;
	pHead->AliHeadCmd = M2H_UPDATE_UPGRADEPACK;
	pPacknumber = (CVI_U16 *)(&pData[4]);
	pPackSeq = (CVI_U16 *)(&pData[6]);
	plen = (CVI_U16 *)(&pData[8]);
	*plen = Info->PackParm.len;
	*pPacknumber = Info->PackParm.packet_number;
	*pPackSeq = Info->PackParm.packet_Seqnumber;
	memcpy(pData + 6 + sizeof(ALIYUN_CMD_PACK_S), Info->PackString, Info->PackParm.len);
	ResponeLen = ModuleResponePack(pResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, pData,
				       DataLen + 6 + sizeof(ALIYUN_CMD_PACK_S));
	Event_dispath_DataSend(pResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
EXIT:
	if (pData)
		free(pData);
	if (pResponeBuf)
		free(pResponeBuf);
	return Ret;
}

CVI_S32 Module_Ack_ShadowDevice(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[5] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;
	CVI_U8 *pErrorCode = &Data[4];

	pHead->AliHeadCmd = M2H_ACK_SHADOWDEVICE;
	*pErrorCode = Info->ErrorCode;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_Ack_ShadowDevice_Update(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U8 Data[5] = {0};
	CVI_U32 ResponeLen = 0;
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)Data;
	CVI_U8 *pErrorCode = &Data[4];

	pHead->AliHeadCmd = M2H_ACK_SHADOWDEVICE_UPDATE;
	*pErrorCode = Info->ErrorCode;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, Data,
				       sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 Module_Set_ShadowDevice_Info(const MODULE_UPLOAD_S *Info)
{
	CVI_U8 *pResponeBuf = NULL;
	CVI_U8 *pData = NULL;
	CVI_U8 *pPacket = NULL;
	CVI_U16 *pPacknumber = NULL;
	CVI_U16 *pPackSeq = NULL;
	CVI_U16 *plen = NULL;
	CVI_U16 DataLen = 0;
	CVI_U32 ResponeLen = 0;
	CVI_S32  Ret = CVI_SUCCESS;

	DataLen = Info->PackParm.len;
	pResponeBuf = (CVI_U8 *)malloc(COMM_PACKET_SIZE + DataLen + 7 + sizeof(ALIYUN_CMD_PACK_S));
	if (!pResponeBuf) {
		EVENT_DISPATCH_DEBUG("%s:malloc pResponeBuf Err\n", __func__);
		Ret = CVI_FAILURE;
		goto EXIT;
	}
	pData = (CVI_U8 *)malloc(DataLen + 7 + sizeof(ALIYUN_CMD_PACK_S));
	if (!pData) {
		EVENT_DISPATCH_DEBUG("%s:malloc pData Err\n", __func__);
		Ret = CVI_FAILURE;
		goto EXIT;
	}
	memset(pData, 0, DataLen + 7 + sizeof(ALIYUN_CMD_PACK_S));
	ALIYUN_CMD_PACK_S *pHead = (ALIYUN_CMD_PACK_S *)pData;
	pHead->AliHeadCmd = M2H_SET_SHADOWDEVICE_INFO;
	pPacket = &pData[4];
	pPacknumber = (CVI_U16 *)(&pData[5]);
	pPackSeq = (CVI_U16 *)(&pData[7]);
	plen = (CVI_U16 *)(&pData[9]);
	*pPacket = Info->PackParm.packet;
	*pPacknumber = Info->PackParm.packet_number;
	*pPackSeq = Info->PackParm.packet_Seqnumber;
	*plen = Info->PackParm.len;
	memcpy(pData + 7 + sizeof(ALIYUN_CMD_PACK_S), Info->PackString, Info->PackParm.len);
	ResponeLen = ModuleResponePack(pResponeBuf, COMM_CMD_H2M_RAWDATA_TRANSFER, pData,
				       DataLen + 7 + sizeof(ALIYUN_CMD_PACK_S));
	Event_dispath_DataSend(pResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
EXIT:
	if (pData)
		free(pData);
	if (pResponeBuf)
		free(pResponeBuf);
	return Ret;
}

CVI_S32 Module_EventUpLoad(const MODULE_UPLOAD_S *Info)
{
	if (Info->Comm_Cmd == COMM_CMD_H2M_RAWDATA_TRANSFER) {
		switch (Info->Aliyun_Cmd) {
		case M2H_QUERY_KEYINFO://查询钥匙信息
			Module_ReportGetKeyInfo(Info);
			break;
		case M2H_ADD_KEY://添加钥匙
			Module_ReportAddKey(Info);
			break;
		case M2H_DELETE_KEY://删除钥匙
			Module_ReportDeleteKey(Info);
			break;
		case M2H_QUERY_TMP_PWD://查询临时密码
			Module_ReportGetTmpPasswd(Info);
			break;
		case M2H_UPDATE_TMP_PWD_STATUS://更新临时密码
			Module_ReportTmpPasswd(Info);
			break;
		case M2H_UPDATE_GENCFG://重新生成临时密码
			Module_ReportUpdataGenCfg(Info);
			break;
		case M2H_SET_KEY_PER://设置钥匙权限
			Module_ReportSetKeyCfg(Info);
			break;
		case M2H_MODIFY_KEY_INFO://修改钥匙信息
			Module_ReportUpdataKeyInfo(Info);
			break;
		case M2H_SET_ATTRIBUTE_SETTING:
			Module_SetAttributeSetting(Info);
			break;
		case M2H_ACK_UPDATE_ATTRIBUTEPACK:
			Module_AckUpdateAttibutePack(Info);
			break;
		case M2H_SET_SERVER_COMMOAND:
			Module_SetServerCommand(Info);
			break;
		case M2H_ACK_SERVER_PACK:
			Module_Ack_Server_Pack(Info);
			break;
		case M2H_ACK_EVENT:
			Module_AckEvent(Info);
			break;
		case M2H_SET_UPGRADE:
			Moudle_SET_UPGRADE(Info);
			break;
		case M2H_UPDATE_UPGRADEPACK:
			Module_Update_UpgradePack(Info);
			break;
		case M2H_ACK_SHADOWDEVICE:
			Module_Ack_ShadowDevice(Info);
			break;
		case M2H_ACK_SHADOWDEVICE_UPDATE:
			Module_Ack_ShadowDevice_Update(Info);
			break;
		case M2H_SET_SHADOWDEVICE_INFO:
			Module_Set_ShadowDevice_Info(Info);
			break;
		default :

			break;
		}
	} else {
		switch (Info->Comm_Cmd) {
		case COMM_CMD_M2H_REPORT_POWERON:
			Module_ReportPowerOn();
			break;
		case COMM_CMD_M2H_REPORT_STREAM_STATUS:
			Module_ReportStreamStatus(Info);
			break;
		case COMM_CMD_M2H_REPORT_IPC_TALK_STATUS:
			Module_ReportTalkStatus(Info);
			break;
		case COMM_CMD_M2H_DATA_TRANSFER:
			Module_ReportServerData(Info);
			break;
		case COMM_CMD_H2M_GET_NETCONFIG:
			GetNetStatusAck();
			break;
		default:

			break;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 LockReportKeyInfoPrase(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	ModuleParm.Aliyun_Cmd = H2M_REPORT_KEYINFO;
	ModuleParm.LockKeyInfo.keyNum = Data[0];
	ModuleParm.LockKeyInfo.keyNode = (LOCK_KEY_INFO_NODE_S *)(&Data[2]);
	Host2Module_DataCallBack(&ModuleParm);
	return CVI_SUCCESS;
}

CVI_S32 LockUpdataTmpPassSTPrase(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S ModuleParm = {0};

	ModuleParm.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	ModuleParm.Aliyun_Cmd = H2M_REPORT_TMP_PWD;
	ModuleParm.TmpPasswdInfo.tempPassWordNum = *((CVI_U16 *)&Data[0]);
	ModuleParm.TmpPasswdInfo.Node = (TMP_PWD_INFO_NODE_S *)(&Data[1]);
	Host2Module_DataCallBack(&ModuleParm);
	return CVI_SUCCESS;
}

CVI_S32 GetSoftwareVerPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	MODULE_PARAM_S Module = {0};

	//获取版本号
	Module.Comm_Cmd = COMM_CMD_H2M_QUERY_SOFTWARE_VER;
	CVI_U8 Data[3] = {0};
	CVI_U8 *pMainVer = &Data[1];
	CVI_U8 *pSubVer = &Data[2];
	Host2Module_DataCallBack(&Module);
	*pMainVer = Module.VerInfo.Main_Ver;
	*pSubVer = Module.VerInfo.Sub_Ver;
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_QUERY_SOFTWARE_VER, Data, sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 CleanNetconfigPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 Data[1] = {0};
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_CLEAR_NETCONFIG;
	Host2Module_DataCallBack(&Module);
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_CLEAR_NETCONFIG, Data, sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 FactoryResetPrase(void)
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 Data[1] = {0};
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_FACTORY_RESET;
	Host2Module_DataCallBack(&Module);
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_FACTORY_RESET, Data, sizeof(Data));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}

CVI_S32 LockAckAttributeSettingPrase(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_ACK_ATTRIBUTE_SETTING;
	Module.ErrCode = Data[0];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockAttributePackPrase(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_REPORT_ATTRIBUTEPACK;
	Module.RawData.Datalen = (*((CVI_U16 *)&Data[0]));
	Module.RawData.Data = &Data[2];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockAckServerCommand(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_ACK_SERVER_COMMOAND;
	Module.ErrCode = Data[0];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockReportServerPack(CVI_U8 *Data, CVI_U32 DataLength)
{

	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_REPORT_SERVER_PACK;
	memcpy(&Module.PackParm, Data, sizeof(ALIYUNPACKPARM_S));
	Module.PackString = &Data[sizeof(ALIYUNPACKPARM_S)];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockReportEvent(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_REPORT_EVENT;
	memcpy(Module.PackParm.key, &Data[0], 20);
	Module.PackParm.len = (*((CVI_U16 *)&Data[20]));
	Module.PackString = &Data[22];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockAckUpgrade(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_ACK_UPGRADE;
	Module.ErrCode = Data[0];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockAckUpgradePack(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_ACK_UPGRADEPACK;
	Module.ErrCode = Data[0];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockReportShadowDevice(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_REPORT_SHADOWDEVICE;
	Module.ShadowDeviceInfo.DeltaUpdate = Data[0];
	Module.ShadowDeviceInfo.len = *(CVI_U16 *)(&Data[1]);
	Module.PackString = &Data[3];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockReportShadowDevice_Update(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_REPORT_SHADOWDEVICE_UPDATE;
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 LockAckShadowDevice_Info(CVI_U8 *Data, CVI_U32 DataLength)
{
	MODULE_PARAM_S Module = {0};

	Module.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Module.Aliyun_Cmd = H2M_ACK_SHADOWDEVICE_INFO;
	Module.ErrCode = Data[0];
	Host2Module_DataCallBack(&Module);
	return CVI_SUCCESS;
}

CVI_S32 TransferRawDataPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength)
{
	CVI_U8 *Data = PackBuf + sizeof(ALIYUN_CMD_PACK_S);
	CVI_U32 DataLen = PackBufLength - sizeof(ALIYUN_CMD_PACK_S);
	ALIYUN_CMD_PACK_S *pAliPackHead = (ALIYUN_CMD_PACK_S *)PackBuf;

	switch (pAliPackHead->AliHeadCmd) {
	case H2M_REPORT_KEYINFO:
		LockReportKeyInfoPrase(Data, DataLen);
		break;
	case H2M_REPORT_TMP_PWD:
		LockUpdataTmpPassSTPrase(Data, DataLen);
		break;
	case H2M_ACK_ATTRIBUTE_SETTING:
		LockAckAttributeSettingPrase(Data, DataLen);
		break;
	case H2M_REPORT_ATTRIBUTEPACK:
		LockAttributePackPrase(Data, DataLen);
		break;
	case H2M_ACK_SERVER_COMMOAND:
		LockAckServerCommand(Data, DataLen);
		break;
	case H2M_REPORT_SERVER_PACK:
		LockReportServerPack(Data, DataLen);
		break;
	case H2M_REPORT_EVENT:
		LockReportEvent(Data, DataLen);
		break;
	case H2M_ACK_UPGRADE:
		LockAckUpgrade(Data, DataLen);
		break;
	case H2M_ACK_UPGRADEPACK:
		LockAckUpgradePack(Data, DataLen);
		break;
	case H2M_REPORT_SHADOWDEVICE:
		LockReportShadowDevice(Data, DataLen);
		break;
	case H2M_REPORT_SHADOWDEVICE_UPDATE:
		LockReportShadowDevice_Update(Data, DataLen);
		break;
	case H2M_ACK_SHADOWDEVICE_INFO:
		LockAckShadowDevice_Info(Data, DataLen);
		break;
	default:
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 Reboot_wifi()
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[1] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	MODULE_PARAM_S ModuleParm = {0};
	ModuleParm.Comm_Cmd = COMM_CMD_H2M_STOP_SMARTCONFIG;
	*RetCode = Host2Module_DataCallBack(&ModuleParm);
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_STOP_SMARTCONFIG, DataBuf, sizeof(DataBuf));
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);
	return CVI_SUCCESS;
}


CVI_S32 Get_CatEye_Version()
{
	CVI_U8 ResponeBuf[64] = {0};
	CVI_U32 ResponeLen = 0;
	CVI_U8 DataBuf[16] = {0};
	CVI_U8 *RetCode = &DataBuf[0];
	CVI_U8 cateye_v[10] = {"1.0.0.1"};  //默认1.0.0版本
	*RetCode = CVI_SUCCESS;
	memcpy(&DataBuf[1], cateye_v, strlen((CVI_CHAR *)cateye_v));
	ResponeLen = ModuleResponePack(ResponeBuf, COMM_CMD_H2M_QUERY_CATEYE_VER, DataBuf, strlen((CVI_CHAR *)cateye_v) + 1);
	Event_dispath_DataSend(ResponeBuf, ResponeLen, AOS_WAIT_FOREVER);

	return CVI_SUCCESS;
}






