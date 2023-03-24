#ifndef WIFI_COMMON_H
#define WIFI_COMMON_H
#include "cvi_type.h"

typedef enum {
	WIFI_NET_SUCCESS = 0,
	WIFI_NET_FAILED,
	WIFI_NET_UPDATE_OK,
	WIFI_NET_UPDATE_FAILE,
	WIFI_NET_ERR_ALREADY_EXIST,
} WIFI_NETCODE_ENUM;

typedef enum {
	H2W_HOST_CMD_GET_MAC, //Host查询3861 macid    0
	H2W_HOST_CMD_GET_IP, //Host查询3861 ip地址
	H2W_HOST_CMD_SET_FILTER,//Host设置默认过滤配置
	H2W_HOST_CMD_GET_RSSI,//Host获取当前wifi的信号值
	H2W_HOST_CMD_CONNECT,//Host发送连接wifi指令
	H2W_HOST_CMD_START_STA,//Host启动sta连接     05
	W2H_HOST_CMD_REPORT_WPA_EVENT,//3861报告连接等状态
	H2W_HOST_CMD_STOP_STA,//Host发送断开sta
	H2W_HOST_CMD_CLEAN_WIFI,//Host发送清除3861wifi 的ssid和pw信息
	H2W_HOST_CMD_CLEAN_DEVICEINFO,//Host发送清除3861 4元组信息
	H2W_HOST_CMD_CLEAN_ALL,//Host发送清除wifi和4元组信息 10
	H2W_IOT_SET_DEVICEINFO,   //Host设置3861的4元组
	H2W_IOT_GET_DEVICEINFO,   //Host获取3861的4元组
	H2W_IOT_REPORT_PROPERTY,  //Host上报属性到云端
	W2H_IOT_SET_PROPERTY,     //云端设置属性到Host
	W2H_IOT_SET_SERVICE,      //云端设置服务到Host        15
	W2H_IOT_REPORT_SERVICE,   //Host上传属性服务
	H2W_IOT_EVENT_NOTIFY,     //Host系统事件上报
	H2W_IOT_USER_NOTIFY_EVENT,//Host用户事件上报
	W2H_IOT_USER_SET_SERVICE,  //Host用户设置服务
	W2H_LINK_VISUAL_SERVICE,//云端传递LV服务       20
	W2H_CMD_CONNECT_WIFI_FIRST,//Wifi端通知cpu第一次连网，需要上报所有属性
	H2W_LINK_VISUAL_RAWDATA,  //暂时未使用
	W2H_LINK_VISUAL_USER,//Lv用户消息
	W2H_LINK_VIUSAL_CONNECT,    //Lv联网消息通知
	W2H_LINK_VIUSAL_DISCONNECT, //Lv断网消息通知        25
	W2H_IOT_DEVICE_CONNECT,    //Iot发连接通知给cpu
	H2W_LINK_VISUAL_PUBLISH,     //Cpu发布消息给Lv
	H2W_LINK_VISUAL_INIT_SUCCESS, //Cpu通知wifi初始化完成
	H2W_HOST_CMD_GET_VERSION,  //Cpu获取wifi版本
	H2W_HOST_CMD_SET_VERSION,   //Cpu设置其他版本给wifi 30
	H2W_HOST_CMD_WANK_UP,  //Cpu通知Wifi唤醒门锁
	H2W_HOST_CMD_WIFI_OTA_START,//Cpu通知wifi进入升级模式
	H2W_HOST_CMD_WIFI_OTA,//Cpu发送升级包给wifi
	W2H_HOST_CMD_ALL_OTA_START,//wifi通知Cpu进入升级模式
	H2W_HOST_CMD_ALL_OTA,        //wifi给Cpu发送升级包        35
	H2W_HOST_CMD_WIFI_SLEEP_TIME,//Cpu设置wifi睡眠时间
	H2W_HOST_TEST_RSSI,     //Cpu请求设定的路由信号值
	H2W_HOST_CMD_HEART,     //Wifi发送心跳给CPU
	W2H_HOST_CMD_UPDATEINFO,//3861给host发送升级信息
	H2W_HOST_CMD_ZEROCONFIG,//host发送零配通知        40
	H2W_HOST_CMD_REBOOT,//host发送重启wifi通知
	H2W_HOST_CMD_UPDATEPROGRESS, //Cpu发送进度给3861
	H2W_HOST_CMD_UPDATEVERSION, // 上报总版本号
} WIFI_MSG_E;

typedef struct {
	union {
		CVI_U8 RawData[32];
		struct {
			CVI_U8 *SSID;
			CVI_U16 SSIDLEN;
			CVI_U8 *ENC;
			CVI_U16 ENCLEN;
			CVI_U8 *KEY;
			CVI_U16 KEYLEN;
			CVI_U8 *BSSID;
			CVI_U16 BSSIDLEN;
			CVI_U8 *RSSI;
			CVI_U16 RSSILEN;
		} ConnectInfo;
		struct {
			CVI_U8 *EventID;
			CVI_U16 EventIDLen;
			CVI_U8 *EventData;
			CVI_U16 EventDataLen;
		} EventInfo;
		struct {
			CVI_U8 *Attribute;
			CVI_U16 AttributeLen;
			CVI_U8 *Index;
			CVI_U16 IndexLen;
			CVI_U8 *Topic;
			CVI_U16 TopicLen;
			CVI_U8 *Message;
			CVI_U16 MessaegLen;
		} IotReportInfo;
		struct {
			CVI_U8 *ID;
			CVI_U16 IDLen;
			CVI_U8 *Service;
			CVI_U16 ServiceLen;
			CVI_U8 *Request;
			CVI_U16 RequestLen;
			CVI_U8 *ServiceName;
			CVI_U16 ServiceNameLen;
			CVI_U8 *ServiceData;
			CVI_U16 ServiceDataLen;
		} IotServerInfo;
		struct {
			CVI_U8 *Index;
			CVI_U16 IndexLen;
			CVI_U8 *Topic;
			CVI_U16 TopicLen;
			CVI_U8 *Message;
			CVI_U16 MessaegLen;
			CVI_U8 *ID;
			CVI_U16 IDLen;
			CVI_U8 *Service;
			CVI_U16 ServiceLen;
		} LVPublishInfo;
		struct {
			CVI_U8 *WifiMac;
			CVI_U16 WifiMacLen;
		} WifiInfo;
		struct {
			CVI_U8 *WifiVer;  //前锁版本
			CVI_U16 WifiVerLen;
			CVI_U8 *LockVer;  //前锁版本
			CVI_U16 LockVerLen;
			CVI_U8 *InLockVer;//后锁版本
			CVI_U16 InLockVerLen;
			CVI_U8 *HostVer;
			CVI_U16 HostVerLen;
			CVI_U8 *OtaVer;
			CVI_U16 OtaVerLen;
		} VersionInfo;
		struct {
			CVI_U8 *Data;
			CVI_U16 DataLen;
		} DataInfo;
		struct {
			CVI_U16 SleepTime;
			CVI_U16 SleepTimeLen;
		} TimeInfo;
		struct {
			CVI_U8 *SleepEnable;
			CVI_U16 SleepEnableLen;
		} HeartInfo;
		struct {
			CVI_U8 *UpgradeID;
			CVI_U16 UpgradeIDLen;
			CVI_U8 *Progress;
			CVI_U16 ProgressLen;
		} UpgradeInfo;
		struct {
			CVI_U8 *DeviceName;
			CVI_U16 DeviceNameLen;
			CVI_U8 *Device_Secret;
			CVI_U16 Device_SecretLen;
			CVI_U8 *Product_Key;
			CVI_U16 Product_KeyLen;
			CVI_U8 *Product_Secret;
			CVI_U16 Product_SecretLen;
		} DeviceInfo;
		struct {
			CVI_U8  *OtaSize;
			CVI_U16  OtaSizeLen;
			CVI_U8  *OtaVersion;
			CVI_U16  OtaVersionLen;
		} OtaStartInfo;
		CVI_U8 Net_Data[13];
		CVI_U8 Mac[6];
		CVI_U8 rssi;
	};
} WIFI_MSGINFO_S;



#endif