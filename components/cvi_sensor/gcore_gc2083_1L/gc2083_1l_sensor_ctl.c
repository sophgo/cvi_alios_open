#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "gc2083_1l_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

static void gc2083_1l_linear_1080p30_init(VI_PIPE ViPipe);
static void gc2083_1l_wdr_1080p30_init(VI_PIPE ViPipe);

CVI_U8 gc2083_1l_i2c_addr = 0x37;//0x6e
const CVI_U32 gc2083_1l_addr_byte = 2;
const CVI_U32 gc2083_1l_data_byte = 1;

int gc2083_1l_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc2083_1L_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc2083_1l_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc2083_1L_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int gc2083_1l_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc2083_1L_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc2083_1l_i2c_addr, (CVI_U32)addr, gc2083_1l_addr_byte, gc2083_1l_data_byte);
}

int gc2083_1l_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc2083_1L_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc2083_1l_i2c_addr, (CVI_U32)addr, gc2083_1l_addr_byte,
				(CVI_U32)data, gc2083_1l_data_byte);
}

void gc2083_1l_standby(VI_PIPE ViPipe)
{
	gc2083_1l_write_register(ViPipe, 0x003e, 0x00);
	gc2083_1l_write_register(ViPipe, 0x03f7, 0x00);
	gc2083_1l_write_register(ViPipe, 0x03fc, 0x01);
	gc2083_1l_write_register(ViPipe, 0x03f9, 0x41);

	printf("%s...", __func__);
}

void gc2083_1l_restart(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode;
	CVI_U8     u8ImgMode;

	enWDRMode   = g_pastGc2083_1L[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastGc2083_1L[ViPipe]->u8ImgMode;

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == GC2083_1L_MODE_1920X1080P30_WDR) {
			gc2083_1l_write_register(ViPipe, 0x03f9, 0x40);
			usleep(1);
			gc2083_1l_write_register(ViPipe, 0x03f7, 0x01);
			gc2083_1l_write_register(ViPipe, 0x03fc, 0x8e);
			gc2083_1l_write_register(ViPipe, 0x003e, 0x91);
		}
	} else {
		gc2083_1l_write_register(ViPipe, 0x03f9, 0x42);
		usleep(1);
		gc2083_1l_write_register(ViPipe, 0x03f7, 0x11);
		gc2083_1l_write_register(ViPipe, 0x03fc, 0x8e);
		gc2083_1l_write_register(ViPipe, 0x003e, 0x91);
	}

	printf("%s...", __func__);
}

void gc2083_1l_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc2083_1L[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc2083_1l_write_register(ViPipe,
				g_pastGc2083_1L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc2083_1L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define GC2083_1L_CHIP_ID_ADDR_H	0x03f0
#define GC2083_1L_CHIP_ID_ADDR_L	0x03f1
#define GC2083_1L_CHIP_ID		0x2083

void gc2083_1l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 value = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		value = 0x01;
		break;
	case ISP_SNS_FLIP:
		value = 0x02;
		break;
	case ISP_SNS_MIRROR_FLIP:
		value = 0x03;
		break;
	default:
		return;
	}
	gc2083_1l_write_register(ViPipe, 0x0015, value);
	gc2083_1l_write_register(ViPipe, 0x0d15, value);
}

int  gc2083_1l_probe(VI_PIPE ViPipe)
{
#if CONFIG_SENSOR_QUICK_STARTUP
	return 0;
#endif
	int nVal;
	int nVal2;

	// usleep(50);
	if (gc2083_1l_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc2083_1l_read_register(ViPipe, GC2083_1L_CHIP_ID_ADDR_H);
	nVal2 = gc2083_1l_read_register(ViPipe, GC2083_1L_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC2083_1L_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc2083_1l_init(VI_PIPE ViPipe)
{
	WDR_MODE_E       enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastGc2083_1L[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastGc2083_1L[ViPipe]->u8ImgMode;

	gc2083_1l_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == GC2083_1L_MODE_1920X1080P30_WDR) {
			gc2083_1l_wdr_1080p30_init(ViPipe);
		}
	} else {
		gc2083_1l_linear_1080p30_init(ViPipe);
	}


	g_pastGc2083_1L[ViPipe]->bInit = CVI_TRUE;
}

void gc2083_1l_exit(VI_PIPE ViPipe)
{
	gc2083_1l_i2c_exit(ViPipe);
}

static void gc2083_1l_linear_1080p30_init(VI_PIPE ViPipe)
{
	#if CONFIG_SENSOR_QUICK_STARTUP
	return;
#endif
	// delay_ms(10);
	/****system****/

	gc2083_1l_write_register(ViPipe, 0x03fe, 0xf0);
	gc2083_1l_write_register(ViPipe, 0x03fe, 0xf0);
	gc2083_1l_write_register(ViPipe, 0x03fe, 0xf0);
	gc2083_1l_write_register(ViPipe, 0x03fe, 0x00);
	gc2083_1l_write_register(ViPipe, 0x03f2, 0x00);
	gc2083_1l_write_register(ViPipe, 0x03f3, 0x00);
	gc2083_1l_write_register(ViPipe, 0x03f4, 0x36);
	gc2083_1l_write_register(ViPipe, 0x03f5, 0xc0);
	gc2083_1l_write_register(ViPipe, 0x03f6, 0x24);
	gc2083_1l_write_register(ViPipe, 0x03f7, 0x01);
	gc2083_1l_write_register(ViPipe, 0x03f8, 0x37);
	gc2083_1l_write_register(ViPipe, 0x03f9, 0x43);
	gc2083_1l_write_register(ViPipe, 0x03fc, 0xae);
	gc2083_1l_write_register(ViPipe, 0x0381, 0x07);
	gc2083_1l_write_register(ViPipe, 0x00d7, 0x29);
	gc2083_1l_write_register(ViPipe, 0x0d6d, 0x18);
	gc2083_1l_write_register(ViPipe, 0x00d5, 0x03);
	gc2083_1l_write_register(ViPipe, 0x0082, 0x01);
	gc2083_1l_write_register(ViPipe, 0x0db3, 0xd4);
	gc2083_1l_write_register(ViPipe, 0x0db0, 0x0d);
	gc2083_1l_write_register(ViPipe, 0x0db5, 0x96);
	gc2083_1l_write_register(ViPipe, 0x0d03, 0x02);
	gc2083_1l_write_register(ViPipe, 0x0d04, 0x02);
	gc2083_1l_write_register(ViPipe, 0x0d05, 0x07);
	gc2083_1l_write_register(ViPipe, 0x0d06, 0x29);
	gc2083_1l_write_register(ViPipe, 0x0d07, 0x00);
	gc2083_1l_write_register(ViPipe, 0x0d08, 0x11);
	gc2083_1l_write_register(ViPipe, 0x0d09, 0x00);
	gc2083_1l_write_register(ViPipe, 0x0d0a, 0x02);
	gc2083_1l_write_register(ViPipe, 0x000b, 0x00);
	gc2083_1l_write_register(ViPipe, 0x000c, 0x02);
	gc2083_1l_write_register(ViPipe, 0x0d0d, 0x04);
	gc2083_1l_write_register(ViPipe, 0x0d0e, 0x40);
	gc2083_1l_write_register(ViPipe, 0x000f, 0x07);
	gc2083_1l_write_register(ViPipe, 0x0010, 0x90);
	gc2083_1l_write_register(ViPipe, 0x0017, 0x0c);
	gc2083_1l_write_register(ViPipe, 0x0d73, 0x92);
	gc2083_1l_write_register(ViPipe, 0x0076, 0x00);
	gc2083_1l_write_register(ViPipe, 0x0d76, 0x00);
	gc2083_1l_write_register(ViPipe, 0x0d41, 0x04);
	gc2083_1l_write_register(ViPipe, 0x0d42, 0x65);
	gc2083_1l_write_register(ViPipe, 0x0d7a, 0x10);
	gc2083_1l_write_register(ViPipe, 0x0d19, 0x31);
	gc2083_1l_write_register(ViPipe, 0x0d25, 0xcb);
	gc2083_1l_write_register(ViPipe, 0x0d20, 0x60);
	gc2083_1l_write_register(ViPipe, 0x0d27, 0x03);
	gc2083_1l_write_register(ViPipe, 0x0d29, 0x60);
	gc2083_1l_write_register(ViPipe, 0x0d43, 0x10);
	gc2083_1l_write_register(ViPipe, 0x0d49, 0x10);
	gc2083_1l_write_register(ViPipe, 0x0d55, 0x18);
	gc2083_1l_write_register(ViPipe, 0x0dc2, 0x44);
	gc2083_1l_write_register(ViPipe, 0x0058, 0x3c);
	gc2083_1l_write_register(ViPipe, 0x00d8, 0x68);
	gc2083_1l_write_register(ViPipe, 0x00d9, 0x14);
	gc2083_1l_write_register(ViPipe, 0x00da, 0xc1);
	gc2083_1l_write_register(ViPipe, 0x0050, 0x18);
	gc2083_1l_write_register(ViPipe, 0x0db6, 0x3d);
	gc2083_1l_write_register(ViPipe, 0x00d2, 0xbc);
	gc2083_1l_write_register(ViPipe, 0x0d66, 0x42);
	gc2083_1l_write_register(ViPipe, 0x008c, 0x05);
	gc2083_1l_write_register(ViPipe, 0x008d, 0xa8);
	gc2083_1l_write_register(ViPipe, 0x007a, 0x58); //global gain
	gc2083_1l_write_register(ViPipe, 0x00d0, 0x00);
	gc2083_1l_write_register(ViPipe, 0x0dc1, 0x00);
	gc2083_1l_write_register(ViPipe, 0x0102, 0xa9); //89
	gc2083_1l_write_register(ViPipe, 0x0158, 0x00);
	gc2083_1l_write_register(ViPipe, 0x0107, 0xa6);
	gc2083_1l_write_register(ViPipe, 0x0108, 0xa9);
	gc2083_1l_write_register(ViPipe, 0x0109, 0xa8);
	gc2083_1l_write_register(ViPipe, 0x010a, 0xa7);
	gc2083_1l_write_register(ViPipe, 0x010b, 0xff);
	gc2083_1l_write_register(ViPipe, 0x010c, 0xff);
	gc2083_1l_write_register(ViPipe, 0x0428, 0x86);
	gc2083_1l_write_register(ViPipe, 0x0429, 0x86);
	gc2083_1l_write_register(ViPipe, 0x042a, 0x86);
	gc2083_1l_write_register(ViPipe, 0x042b, 0x68);
	gc2083_1l_write_register(ViPipe, 0x042c, 0x68);
	gc2083_1l_write_register(ViPipe, 0x042d, 0x68);
	gc2083_1l_write_register(ViPipe, 0x042e, 0x68);
	gc2083_1l_write_register(ViPipe, 0x042f, 0x68);
	gc2083_1l_write_register(ViPipe, 0x0430, 0x4f);
	gc2083_1l_write_register(ViPipe, 0x0431, 0x68);
	gc2083_1l_write_register(ViPipe, 0x0432, 0x67);
	gc2083_1l_write_register(ViPipe, 0x0433, 0x66);
	gc2083_1l_write_register(ViPipe, 0x0434, 0x66);
	gc2083_1l_write_register(ViPipe, 0x0435, 0x66);
	gc2083_1l_write_register(ViPipe, 0x0436, 0x66);
	gc2083_1l_write_register(ViPipe, 0x0437, 0x66);
	gc2083_1l_write_register(ViPipe, 0x0438, 0x62);
	gc2083_1l_write_register(ViPipe, 0x0439, 0x62);
	gc2083_1l_write_register(ViPipe, 0x043a, 0x62);
	gc2083_1l_write_register(ViPipe, 0x043b, 0x62);
	gc2083_1l_write_register(ViPipe, 0x043c, 0x62);
	gc2083_1l_write_register(ViPipe, 0x043d, 0x62);
	gc2083_1l_write_register(ViPipe, 0x043e, 0x62);
	gc2083_1l_write_register(ViPipe, 0x043f, 0x62);
	gc2083_1l_write_register(ViPipe, 0x0077, 0x01);
	gc2083_1l_write_register(ViPipe, 0x0078, 0x65);
	gc2083_1l_write_register(ViPipe, 0x0079, 0x04);
	gc2083_1l_write_register(ViPipe, 0x0067, 0xa0);
	gc2083_1l_write_register(ViPipe, 0x0054, 0xff);
	gc2083_1l_write_register(ViPipe, 0x0055, 0x02);
	gc2083_1l_write_register(ViPipe, 0x0056, 0x00);
	gc2083_1l_write_register(ViPipe, 0x0057, 0x04);
	gc2083_1l_write_register(ViPipe, 0x005a, 0xff);
	gc2083_1l_write_register(ViPipe, 0x005b, 0x07);
	gc2083_1l_write_register(ViPipe, 0x0026, 0x01);
	gc2083_1l_write_register(ViPipe, 0x0152, 0x02);
	gc2083_1l_write_register(ViPipe, 0x0153, 0x50);
	gc2083_1l_write_register(ViPipe, 0x0155, 0x93);
	gc2083_1l_write_register(ViPipe, 0x0410, 0x16);
	gc2083_1l_write_register(ViPipe, 0x0411, 0x16);
	gc2083_1l_write_register(ViPipe, 0x0412, 0x16);
	gc2083_1l_write_register(ViPipe, 0x0413, 0x16);
	gc2083_1l_write_register(ViPipe, 0x0414, 0x6f);
	gc2083_1l_write_register(ViPipe, 0x0415, 0x6f);
	gc2083_1l_write_register(ViPipe, 0x0416, 0x6f);
	gc2083_1l_write_register(ViPipe, 0x0417, 0x6f);
	gc2083_1l_write_register(ViPipe, 0x04e0, 0x18);
	gc2083_1l_write_register(ViPipe, 0x0192, 0x04);
	gc2083_1l_write_register(ViPipe, 0x0194, 0x04);
	gc2083_1l_write_register(ViPipe, 0x0195, 0x04);
	gc2083_1l_write_register(ViPipe, 0x0196, 0x38);
	gc2083_1l_write_register(ViPipe, 0x0197, 0x07);
	gc2083_1l_write_register(ViPipe, 0x0198, 0x80);
	gc2083_1l_write_register(ViPipe, 0x0201, 0x27);
	gc2083_1l_write_register(ViPipe, 0x0202, 0x53); //0x50
	gc2083_1l_write_register(ViPipe, 0x0203, 0xce); //0xb6//0x8e
	gc2083_1l_write_register(ViPipe, 0x0204, 0x40);
	gc2083_1l_write_register(ViPipe, 0x0212, 0x07);
	gc2083_1l_write_register(ViPipe, 0x0213, 0x80);
	gc2083_1l_write_register(ViPipe, 0x0215, 0x12);
	gc2083_1l_write_register(ViPipe, 0x023e, 0x98);

	gc2083_1l_default_reg_init(ViPipe);
	// delay_ms(80);

	printf("ViPipe:%d,===GC2083_1L 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

static void gc2083_1l_wdr_1080p30_init(VI_PIPE ViPipe)
{
	printf("ViPipe = %d\n", ViPipe);
}
