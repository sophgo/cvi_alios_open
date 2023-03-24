#ifndef WIFI_HOSTMSGPACK_H
#define WIFI_HOSTMSGPACK_H
#include "wifi_common.h"
#include "cvi_type.h"
#include "wifi_dispatch.h"

#define MULTIPLENUMBERMAX 10

typedef struct {
	CVI_U8 Type;
	CVI_U16 Len;
	CVI_U8 *data;
} WIFI_MSGBASETYPE_S;

typedef struct {
	CVI_U8 CMD;
	WIFI_MSGBASETYPE_S MultipleData[MULTIPLENUMBERMAX];
	CVI_U32 MultipleDataNumber;
} WIFI_MSGPACKINFO_S;







#endif