/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:
 * Description:
 *   ....
 */
#ifndef _EVENT_DISPATCH_H_
#define _EVENT_DISPATCH_H_

#include <stdio.h>
#include "cvi_type.h"
#include "event_def.h"

//#define EVENT_DISPATCH_DEBUG(fmt,...)  printf(fmt,##__VA_ARGS__)
#define EVENT_DISPATCH_DEBUG(fmt,...)

typedef enum _Network_Status_e {
	NET_NOT_CONFIG = 0x00,            //网络未配置
	NET_PLAT_CONNECT_ON = 0x01,       //平台连接成功
	NET_ROUTER_CONNECT_ON = 0x02,    //路由器连接成功
	NET_PLAT_CONNECTING = 0x03,      //正在连接平台
	NET_ROUTER_CONNECTING = 0X04,   //正在连接路由器
	NET_PLAT_CONNECT_OFF = 0x05,    //平台连接断开
	NET_ROUTER_CONNECT_OFF = 0X06 //路由器连接断开
} NET_STATUS_E;

typedef enum _ResponseCode_e {
	RES_ERRORCODE_CMD_OK = 0x00, //命令执行成功
	RES_ERRORCODE_0 = 0x00, //
	RES_ERRORCODE_1 = 0x01, //
	RES_ERRORCODE_2 = 0x02, //
	RES_ERRORCODE_3 = 0x03, //
	RES_ERRORCODE_4 = 0x04, //
	RES_ERRORCODE_5 = 0x05, //
	RES_ERRORCODE_6 = 0x06, //
	RES_ERRORCODE_7 = 0x07, //
	RES_ERRORCODE_8 = 0x08, //
	RES_ERRORCODE_9 = 0x09, //
	RES_ERRORCODE_PARAM_RANGE_WRONG = 0xfc, //参数取值范围错误
	RES_ERRORCODE_PARAM_LEN_WRONG = 0xfd, //参数长度错误
	RES_ERRORCODE_LASTCMD_NOT_FINISH = 0xfe, //前一个命令未完成
	RES_ERRORCODE_CMD_NOT_SUPPORT = 0xff,  //不支持的命令
} HOST_MODULE_COMM_RES_ERRORCODE_E;

typedef struct {
	CVI_U8 ConfigMode;
	CVI_U8 SSID[32];        //Wifi名称,字符串,不足填充'\0'
	CVI_U8 Password[24];    //Wifi密码,字符串,不足填充'\0'
} __attribute__((packed)) NET_CONFIG_S;

typedef enum _VideoEncodeType_e {
	VIDEO_ENCODE_TYPE_H264 = 0,
	VIDEO_ENCODE_TYPE_H265 = 1,
	VIDEO_ENCODE_TYPE_MJPEG = 2,
} VIDEO_ENCODE_TYPE_E;

typedef enum _VideoFrameRate_e {
	VIDEO_FRAME_RATE_15FPS = 0,
	VIDEO_FRAME_RATE_25FPS = 1,
	VIDEO_FRAME_RATE_30FPS = 2,
} VIDEO_FRAME_RATE_E;

typedef enum _VideoSize_e {
	VIDEO_SIZE_1200x1600 = 0,
	VIDEO_SIZE_900x1200 = 1,
	VIDEO_SIZE_720x960 = 2,
	VIDEO_SIZE_600x800 = 3,
	VIDEO_SIZE_360x480 = 4,
	VIDEO_SIZE_300x400 = 5,
	VIDEO_SIZE_320x480 = 6,
	VIDEO_SIZE_720x1280 = 7
} VIDEO_SIZE_E;

typedef struct _VideoCfgInfo_s {
	CVI_U8 Video_Encode;    // 0:H264 1:H265 2:MJPEG
	CVI_U8 Framerate;       // 0:15FPS 1:25FPS 2:30FPS
	CVI_U8 Video_Size;      // 0:1200x1600 1:900x1200 2:720x960 3:600x800 4: 360x480 5 300x400
} __attribute__((packed)) VIDEO_CONFIG_INFO_S;

typedef enum _PictureFormate_e {
	PICTURE_TYPE_JPEG = 0,
	PICTURE_TYPE_YVU420SP = 1,
} PICTURE_FORMATE_E;


typedef struct _PictureCfgInfo_s {
	CVI_U8 Picture_Type;    //0:JPEG 1:YVU420SP
	CVI_U8 Picture_Size;    //0:1200x1600 1:900x1200 2:720x960 3:600x800 4:360x480 5:300x400
} __attribute__((packed)) PICTURE_CONFIG_INFO_E;

typedef enum _AudioEncType_e {
	AUDIO_ENCODE_FORMATE_AAC = 0,
	AUDIO_ENCODE_FORMATE_G711A = 1,
	AUDIO_ENCODE_FORMATE_G711U = 2
} AUDIO_ENCODE_TYPE_E;

typedef enum _AudioSampleRate_e {
	AUDIO_SAMPLERATE_44_1K = 0, //44.1K
	AUDIO_SAMPLERATE_48K = 1,    //48K
	AUDIO_SAMPLERATE_16K = 3     //16K
} AUDIO_SAMPLE_RATE_E;

typedef struct _AudioCfg_s {
	CVI_U8 Audio_EncType;       //0: AAC 1: G711A 2:G711U
	CVI_U8 Sample_Rate;         //0: 44.1K 1: 48K
} __attribute__((packed)) AUDIO_CONFIG_INFO_S;

typedef struct _AliYunParam_s {
	CVI_U8 Product_Secret_Len;
	CVI_U8 Product_Secret[65];
	CVI_U8 Product_Key_Len;
	CVI_U8 Product_Key[33];
	CVI_U8 Device_Name_Len;
	CVI_U8 Device_Name[33];
	CVI_U8 Device_Secret_Len;
	CVI_U8 Device_Secret[65];
} ALIYUN_PARAM_S;

typedef enum {
	LOCK_KEY_NONE = 0,
	LOCK_KEY_FINGERPRINT = 0x01, //指纹
	LOCK_KEY_PWD = 2, //密码
	LOCK_KEY_CARD = 3, //卡
	LOCK_KEY_MECHANICAL = 4, //机械钥匙
	LOCK_KEY_TMP_PWD = 5, //临时密码
	LOCK_KEY_PHONE = 6, //手机开锁
	LOCK_KEY_MIX = 7, //组合开锁
	LOCK_KEY_FACE = 8, //人脸开锁
	LOCK_KEY_INDOOR = 9, //门内开锁
	LOCK_KEY_FINGERVEIN = 10, //指静脉开锁
	LOCK_KEY_MAX
} LOCK_KEY_TYPE_E;

typedef enum {
	LOCK_KEY_LIMIT_USER = 1, //用户
	LOCK_KEY_LIMIT_ROOT = 2, //管理员
	LOCK_KEY_LIMIT_HIJACK = 3, //劫持用户
} LOCK_KEY_USERLIMIT_E;

typedef struct _TmpPwdInfoNode_s {
	CVI_U8 tempPassWordId;
	CVI_U8 tempPassWordStatus;
	CVI_U8 tempPassWordCntent[6];
} __attribute__((packed)) TMP_PWD_INFO_NODE_S;

typedef struct _TmpPwdInfo_s {
	CVI_U8 tempPassWordNum;
	TMP_PWD_INFO_NODE_S *Node;
} TMP_PWD_INFO_S;

typedef struct _LockKeyInfoNode_s {
	CVI_U16 keyID;
	CVI_U8 lockType;
	CVI_U8 userLimit;
	CVI_U8 KeyAlias[32];
} __attribute__((packed)) LOCK_KEY_INFO_NODE_S;

typedef struct _LockKeyInfo_s {
	CVI_U16 keyNum;
	LOCK_KEY_INFO_NODE_S *keyNode;
} LOCK_KEY_INFO_S;

typedef enum _LOCK_ALARMEVNET_TYPE_E {
	TAMPER_ALARM = 60001,//防拆报警
	DOORBELL_EVENT = 60002,//门铃事件
	STAY_ALARM = 60004,//逗留报警
	OPEN_DOOR_ALARM = 70002,//未关门报警
	FINGERPRINT_ERRO_ALARM = 70006,//指纹试错报警
	PASSWORD_ERROR_ALRAM = 70007,//密码试错报警
} LOCK_ALARMEVNET_TYPE_E;

typedef struct _LockSnapPic_s {
	CVI_U8 CaptureType[4]; //事件类型
	CVI_U16 Datalen;    //抓拍参数长度
	CVI_U8 *Data;       //抓拍参数
} PIC_CAPTURE_DATA_S;

typedef struct _ALIYUN_RAWDATA_S {
	CVI_U16 Datalen;
	CVI_U8 *Data;
} RAWDATA_S;

typedef struct _VERSION_INFO_s {
	CVI_U8 Platform; //平台
	CVI_U8 Main_Ver; //主版本
	CVI_U8 Sub_Ver;  //小版本
} VERSION_INFO_S;

typedef struct _AliyunPackParm_s {
	CVI_U8 packet; //1完整包 0不完整包
	CVI_U16 packet_number; //标识分包数量
	CVI_U16 packet_Seqnumber; //序列号
	CVI_U8 key[20]; //服务关键字
	CVI_U16 len; //PackString 长度
} __attribute__((packed)) ALIYUNPACKPARM_S;

typedef struct _ALIYUNSHADOWDEVICE_S {
	CVI_U8 DeltaUpdate;
	CVI_U16 len;
} ALIYUNSHADOWDEVICE_S;

//liang.wang, 猫眼模块无人脸识别功能， 20220314.
typedef struct _ModuleSetParm_s {
	HOST_MODULE_COMM_CMD_E Comm_Cmd;
	ALIYUN_CMD_TYPE_E      Aliyun_Cmd;
	union {
		NET_CONFIG_S NetCfg;                    //网络配置
		VIDEO_CONFIG_INFO_S   VideoParm;        //视频配置
		PICTURE_CONFIG_INFO_E picparm;          //图片配置
		AUDIO_CONFIG_INFO_S   AudioParm;        //音频配置
		ALIYUN_PARAM_S AliyunParm;              //阿里云对接参数
		CVI_U8 MAC[6];                          //锁端MAC地址或人脸MAC地址
		CVI_U8 StreamStaus;                     //推流状态  0:当前没有推流 1:正在推流
		CVI_U8 TalkStatus;                      //对讲状态  0：当前没有推流 1：正在推流
		CVI_U8 NetStatus;                       //网络状态
		CVI_U8 ErrCode;
		CVI_U16 Duration;                       //耗时时长
		TMP_PWD_INFO_S TmpPasswdInfo;	        //临时密码上报设置
		LOCK_KEY_INFO_S       LockKeyInfo;      //钥匙信息
		PIC_CAPTURE_DATA_S CaptureInfo;         //抓图参数信息
		RAWDATA_S    RawData;                   //透传数据
		VERSION_INFO_S VerInfo;                 //版本信息
		ALIYUNPACKPARM_S PackParm;              //打包参数
		ALIYUNSHADOWDEVICE_S ShadowDeviceInfo;  //影子设备上传更新信息体
	};
	CVI_U8 *PackString;                         //阿里透传PackParm打包使用
} MODULE_PARAM_S;

typedef enum _ModuleUploadCMD_e {
	REPORT_STREAMSTATUS = 0,     //上报推流状态
	REPORT_TALKSTATUS = 1,       //上报对讲状态
	REPORT_SERVERDATA = 2,      //接受服务器数据透给门锁
	REPORT_GETKEYINFO = 3,         //查询钥匙信息
	REPORT_ADDKEY = 4,             //添加钥匙
	REPORT_DELETEKEY = 5,          //删除钥匙
	REPORT_GETTMPPASSWD = 6,       //查询临时密码
	REPORT_TMPPASSWDSTATUS = 7,    //更新临时密码状态
	REPORT_UPDATAGENCFG = 8,       //重新生成临时密码
	REPORT_SETKEYCFG = 9,          //设置钥匙权限
	REPORT_UPDATAKEYINFO = 10,     //修改钥匙信息
	MODELE_UPLOAD_CMDMax,           //保留
} MODELE_UPLOAD_CMD_E;

typedef struct _LockKeyCfg_s {
	CVI_U16 keyID;
	CVI_U8 lockType;
	CVI_U8 userLimit; //钥匙权限
} LOCK_KEYCFG_S;

typedef enum {
	LOCK_STATUS_NORMAL = 0, //正常状态
	Lock_STATUS_USED = 1, //已使用
	Lock_STATUS_OPEN = 2, //已开锁
} LOCK_PASSWD_STATUS_E;

typedef struct _LockPassSTCfg_s {
	CVI_U8 tempPassWordId;      //临时密码ID
	CVI_U8 tempPassWordStatus; //临时密码状态
} LOCK_TMP_PWD_TCFG_S;

typedef struct ModuleUpload_s {
	HOST_MODULE_COMM_CMD_E Comm_Cmd;
	ALIYUN_CMD_TYPE_E      Aliyun_Cmd;
	union {
		CVI_U8 StreamStatus;            //上传推流状态
		CVI_U8 TalkStauts;              //上传对讲状态
		CVI_U8 KeyType;
		CVI_U8 LockGenCfg;
		CVI_U8  GenCfg;                 //请求重新生成的临时密码个数
		LOCK_KEYCFG_S KeyCfg;
		LOCK_KEY_INFO_S KeyInfo;
		LOCK_TMP_PWD_TCFG_S PassST;
		LOCK_KEY_INFO_NODE_S KeyNodeInfo;
		RAWDATA_S RawData;   //Data数据
		ALIYUNPACKPARM_S PackParm;              //打包参数
		CVI_U8 ErrorCode;
	};
	CVI_U8 *PackString;                         //阿里透传PackParm打包使用

} MODULE_UPLOAD_S;

CVI_S32 Module_EventUpLoad(const MODULE_UPLOAD_S *Info);
CVI_S32 Event_dispatch_Init();
CVI_S32 Event_dispatch_Destroy(void);
CVI_S32 Event_dispath_DataSend(CVI_U8 *Buf, CVI_U32 BufLen, CVI_U32 u32BlockTimeMs);
CVI_U16 Module_CheckSum(CVI_U8 *Buf, CVI_U16 BufLength);
CVI_S32 Module_ReadCallBack(CVI_U8 *Buf, CVI_U32 BufLength);
void Module_NetStatusSet(NET_STATUS_E bSet);
void DoorbellModule_EventDispatchStart(void);
#endif



