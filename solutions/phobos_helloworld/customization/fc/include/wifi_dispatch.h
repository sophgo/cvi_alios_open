#ifndef WIFI_DISPATCH_H
#define WIFI_DISPATCH_H
#include "cvi_type.h"
#include "wifi_common.h"
typedef enum {
	Wifi_Dispatch = 0,
	Wifi_MsgCbMax
} WIFI_MSGRECVCB_E;


CVI_S32 Wifi_RecvMsgData(CVI_U8 *Data, CVI_U32 DataLen);
CVI_S32 Wifi_MsgRecvRegister(WIFI_MSGRECVCB_E Type, void(*func)(CVI_U8 *, CVI_U32));
CVI_S32 Wifi_MsgRecvUnRegister(WIFI_MSGRECVCB_E Type);
CVI_S32 DoorbellModule_WifiDispatchStart();
CVI_S32 Wifi_dispatch_Destroy();
CVI_S32 Wifi_HostMsgUpLoad(WIFI_MSG_E Cmd, const WIFI_MSGINFO_S *Info);
CVI_S32 Wifi_DispatchSendMsgData(CVI_U8 *Data, CVI_S32 Datalen);


CVI_S32 Wifi_AttreibuteInitReport();
CVI_S32 Wifi_ReportAlarmAttribute();
CVI_S32 Wifi_ReportDayNightMode();
CVI_S32 Wifi_ReportStreamVideoQuality();
CVI_S32 Wifi_ReportVideoRotaion();
CVI_S32 Wifi_ReportSdStatus();



CVI_S32 Wifi_Ota_Start(CVI_SL len, CVI_CHAR *version, CVI_CHAR *wifi_statue, CVI_S32 *package_len);
CVI_S32 Wifi_Ota_Start_Info_Back(WIFI_MSGINFO_S *Info);
CVI_S32 Wifi_Get_Version_Back(WIFI_MSGINFO_S *Info);
CVI_S32 Wifi_Get_Version(CVI_CHAR *version);
CVI_S32 Wifi_Report_TotalVersion(CVI_CHAR *version, CVI_U16 VersionLen);
CVI_S32 Wife_Send_Ota_Package(CVI_S32 packet_len, CVI_S32 packet_number, CVI_U8 *packet_data);
CVI_S32 Wifi_Ota_Packet_Back(WIFI_MSGINFO_S *Info);

#endif