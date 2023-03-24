#ifndef WIFI_HOSTMSGPRASE_H
#define WIFI_HOSTMSGPRASE_H
#include "cvi_type.h"





CVI_S32 WifiMsg_Prase(CVI_U8 *Data, CVI_U32 Len);
CVI_S32 WIFI_MSGFindNextDataHeadPosition(CVI_U8 *PackData, CVI_U8 *Data, CVI_U32 PackDataLen);
CVI_S32 WifiMsg_IOT_SET_PROPERTY(CVI_CHAR *pStr);



#endif