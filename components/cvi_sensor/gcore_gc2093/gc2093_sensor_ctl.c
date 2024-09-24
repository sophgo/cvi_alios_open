#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "gc2093_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

static void gc2093_linear_1080p30_init(VI_PIPE ViPipe);
static void gc2093_wdr_1080p30_init(VI_PIPE ViPipe);

const CVI_U8 gc2093_i2c_addr = 0x37;//0x6e
const CVI_U32 gc2093_addr_byte = 2;
const CVI_U32 gc2093_data_byte = 1;

int gc2093_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc2093_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc2093_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc2093_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int gc2093_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc2093_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc2093_i2c_addr, (CVI_U32)addr, gc2093_addr_byte, gc2093_data_byte);
}

int gc2093_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc2093_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc2093_i2c_addr, (CVI_U32)addr, gc2093_addr_byte,
				(CVI_U32)data, gc2093_data_byte);
}


void gc2093_standby(VI_PIPE ViPipe)
{
	gc2093_write_register(ViPipe, 0x003e, 0x00);
	gc2093_write_register(ViPipe, 0x03f7, 0x00);
	gc2093_write_register(ViPipe, 0x03fc, 0x01);
	gc2093_write_register(ViPipe, 0x03f9, 0x41);

	printf("gc2093_standby\n");
}

void gc2093_restart(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode;
	CVI_U8     u8ImgMode;

	enWDRMode   = g_pastGc2093[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastGc2093[ViPipe]->u8ImgMode;

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == GC2093_MODE_1920X1080P30_WDR) {
			gc2093_write_register(ViPipe, 0x03f9, 0x40);
			usleep(1);
			gc2093_write_register(ViPipe, 0x03f7, 0x01);
			gc2093_write_register(ViPipe, 0x03fc, 0x8e);
			gc2093_write_register(ViPipe, 0x003e, 0x91);
		}
	} else {
		gc2093_write_register(ViPipe, 0x03f9, 0x42);
		usleep(1);
		gc2093_write_register(ViPipe, 0x03f7, 0x11);
		gc2093_write_register(ViPipe, 0x03fc, 0x8e);
		gc2093_write_register(ViPipe, 0x003e, 0x91);
	}

	printf("gc2093_restart\n");
}

void gc2093_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc2093[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc2093_write_register(ViPipe,
				g_pastGc2093[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc2093[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}


void gc2093_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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
	gc2093_write_register(ViPipe, 0xfe, 0x00);
	gc2093_write_register(ViPipe, 0x17, value);
}

#define GC2093_CHIP_ID_ADDR_H	0x03f0
#define GC2093_CHIP_ID_ADDR_L	0x03f1
#define GC2093_CHIP_ID		0x2093

int  gc2093_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (gc2093_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc2093_read_register(ViPipe, GC2093_CHIP_ID_ADDR_H);
	nVal2 = gc2093_read_register(ViPipe, GC2093_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC2093_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc2093_init(VI_PIPE ViPipe)
{
	WDR_MODE_E       enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastGc2093[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastGc2093[ViPipe]->u8ImgMode;

	gc2093_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == GC2093_MODE_1920X1080P30_WDR) {
			gc2093_wdr_1080p30_init(ViPipe);
		}
	} else {
		gc2093_linear_1080p30_init(ViPipe);
	}


	g_pastGc2093[ViPipe]->bInit = CVI_TRUE;
}

void gc2093_exit(VI_PIPE ViPipe)
{
	gc2093_i2c_exit(ViPipe);
}

static void gc2093_linear_1080p30_init(VI_PIPE ViPipe)
{
	usleep(10*1000);
	/****system****/
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0x00);
	gc2093_write_register(ViPipe, 0x03f2, 0x00);
	gc2093_write_register(ViPipe, 0x03f3, 0x00);
	gc2093_write_register(ViPipe, 0x03f4, 0x36);
	gc2093_write_register(ViPipe, 0x03f5, 0xc0);
	gc2093_write_register(ViPipe, 0x03f6, 0x0B);
	gc2093_write_register(ViPipe, 0x03f7, 0x11);
	gc2093_write_register(ViPipe, 0x03f8, 0x2A);
	gc2093_write_register(ViPipe, 0x03f9, 0x42);
	gc2093_write_register(ViPipe, 0x03fc, 0x8e);
	/****CISCTL & ANALOG*/
	gc2093_write_register(ViPipe, 0x0087, 0x18);
	gc2093_write_register(ViPipe, 0x00ee, 0x30);
	gc2093_write_register(ViPipe, 0x00d0, 0xbf);
	gc2093_write_register(ViPipe, 0x01a0, 0x00);
	gc2093_write_register(ViPipe, 0x01a4, 0x40);
	gc2093_write_register(ViPipe, 0x01a5, 0x40);
	gc2093_write_register(ViPipe, 0x01a6, 0x40);
	gc2093_write_register(ViPipe, 0x01af, 0x09);
	gc2093_write_register(ViPipe, 0x0003, 0x04);
	gc2093_write_register(ViPipe, 0x0004, 0x65);
	gc2093_write_register(ViPipe, 0x0005, 0x05);
	gc2093_write_register(ViPipe, 0x0006, 0x78);
	gc2093_write_register(ViPipe, 0x0007, 0x00);
	gc2093_write_register(ViPipe, 0x0008, 0x11);
	gc2093_write_register(ViPipe, 0x0009, 0x00);
	gc2093_write_register(ViPipe, 0x000a, 0x02);
	gc2093_write_register(ViPipe, 0x000b, 0x00);
	gc2093_write_register(ViPipe, 0x000c, 0x04);
	gc2093_write_register(ViPipe, 0x000d, 0x04);
	gc2093_write_register(ViPipe, 0x000e, 0x40);
	gc2093_write_register(ViPipe, 0x000f, 0x07);
	gc2093_write_register(ViPipe, 0x0010, 0x8c);
	gc2093_write_register(ViPipe, 0x0013, 0x15);
	gc2093_write_register(ViPipe, 0x0019, 0x0c);
	gc2093_write_register(ViPipe, 0x0041, 0x04);
	gc2093_write_register(ViPipe, 0x0042, 0x65);
	gc2093_write_register(ViPipe, 0x0053, 0x60);
	gc2093_write_register(ViPipe, 0x008d, 0x92);
	gc2093_write_register(ViPipe, 0x0090, 0x00);
	gc2093_write_register(ViPipe, 0x00c7, 0xe1);
	gc2093_write_register(ViPipe, 0x001b, 0x73);
	gc2093_write_register(ViPipe, 0x0028, 0x0d);
	gc2093_write_register(ViPipe, 0x0029, 0x40);
	gc2093_write_register(ViPipe, 0x002b, 0x04);
	gc2093_write_register(ViPipe, 0x002e, 0x23);
	gc2093_write_register(ViPipe, 0x0037, 0x03);
	gc2093_write_register(ViPipe, 0x0043, 0x04);
	gc2093_write_register(ViPipe, 0x0044, 0x30);
	gc2093_write_register(ViPipe, 0x004a, 0x01);
	gc2093_write_register(ViPipe, 0x004b, 0x28);
	gc2093_write_register(ViPipe, 0x0055, 0x30);
	gc2093_write_register(ViPipe, 0x0066, 0x3f);
	gc2093_write_register(ViPipe, 0x0068, 0x3f);
	gc2093_write_register(ViPipe, 0x006b, 0x44);
	gc2093_write_register(ViPipe, 0x0077, 0x00);
	gc2093_write_register(ViPipe, 0x0078, 0x20);
	gc2093_write_register(ViPipe, 0x007c, 0xa1);
	gc2093_write_register(ViPipe, 0x00ce, 0x7c);
	gc2093_write_register(ViPipe, 0x00d3, 0xd4);
	gc2093_write_register(ViPipe, 0x00e6, 0x50);
	/*gain*/
	gc2093_write_register(ViPipe, 0x00b6, 0xc0);
	gc2093_write_register(ViPipe, 0x00b0, 0x68);
	gc2093_write_register(ViPipe, 0x00b3, 0x00);
	gc2093_write_register(ViPipe, 0x00b8, 0x01);
	gc2093_write_register(ViPipe, 0x00b9, 0x00);
	gc2093_write_register(ViPipe, 0x00b1, 0x01);
	gc2093_write_register(ViPipe, 0x00b2, 0x00);
	/*isp*/
	gc2093_write_register(ViPipe, 0x0101, 0x0c);
	gc2093_write_register(ViPipe, 0x0102, 0x89);
	gc2093_write_register(ViPipe, 0x0104, 0x01);
	gc2093_write_register(ViPipe, 0x0107, 0xa6);
	gc2093_write_register(ViPipe, 0x0108, 0xa9);
	gc2093_write_register(ViPipe, 0x0109, 0xa8);
	gc2093_write_register(ViPipe, 0x010a, 0xa7);
	gc2093_write_register(ViPipe, 0x010b, 0xff);
	gc2093_write_register(ViPipe, 0x010c, 0xff);
	gc2093_write_register(ViPipe, 0x010f, 0x00);
	gc2093_write_register(ViPipe, 0x0158, 0x00);
	gc2093_write_register(ViPipe, 0x0428, 0x86);
	gc2093_write_register(ViPipe, 0x0429, 0x86);
	gc2093_write_register(ViPipe, 0x042a, 0x86);
	gc2093_write_register(ViPipe, 0x042b, 0x68);
	gc2093_write_register(ViPipe, 0x042c, 0x68);
	gc2093_write_register(ViPipe, 0x042d, 0x68);
	gc2093_write_register(ViPipe, 0x042e, 0x68);
	gc2093_write_register(ViPipe, 0x042f, 0x68);
	gc2093_write_register(ViPipe, 0x0430, 0x4f);
	gc2093_write_register(ViPipe, 0x0431, 0x68);
	gc2093_write_register(ViPipe, 0x0432, 0x67);
	gc2093_write_register(ViPipe, 0x0433, 0x66);
	gc2093_write_register(ViPipe, 0x0434, 0x66);
	gc2093_write_register(ViPipe, 0x0435, 0x66);
	gc2093_write_register(ViPipe, 0x0436, 0x66);
	gc2093_write_register(ViPipe, 0x0437, 0x66);
	gc2093_write_register(ViPipe, 0x0438, 0x62);
	gc2093_write_register(ViPipe, 0x0439, 0x62);
	gc2093_write_register(ViPipe, 0x043a, 0x62);
	gc2093_write_register(ViPipe, 0x043b, 0x62);
	gc2093_write_register(ViPipe, 0x043c, 0x62);
	gc2093_write_register(ViPipe, 0x043d, 0x62);
	gc2093_write_register(ViPipe, 0x043e, 0x62);
	gc2093_write_register(ViPipe, 0x043f, 0x62);
	 /*dark sun*/
	gc2093_write_register(ViPipe, 0x0123, 0x08);
	gc2093_write_register(ViPipe, 0x0123, 0x00);
	gc2093_write_register(ViPipe, 0x0120, 0x01);
	gc2093_write_register(ViPipe, 0x0121, 0x04);
	gc2093_write_register(ViPipe, 0x0122, 0x65);
	gc2093_write_register(ViPipe, 0x0124, 0x03);
	gc2093_write_register(ViPipe, 0x0125, 0xff);
	gc2093_write_register(ViPipe, 0x001a, 0x8c);
	gc2093_write_register(ViPipe, 0x00c6, 0xe0);
	/*blk*/
	gc2093_write_register(ViPipe, 0x0026, 0x30);
	gc2093_write_register(ViPipe, 0x0142, 0x00);
	gc2093_write_register(ViPipe, 0x0149, 0x1e);
	gc2093_write_register(ViPipe, 0x014a, 0x0f);
	gc2093_write_register(ViPipe, 0x014b, 0x00);
	gc2093_write_register(ViPipe, 0x0155, 0x07);
	gc2093_write_register(ViPipe, 0x0414, 0x78);
	gc2093_write_register(ViPipe, 0x0415, 0x78);
	gc2093_write_register(ViPipe, 0x0416, 0x78);
	gc2093_write_register(ViPipe, 0x0417, 0x78);
	gc2093_write_register(ViPipe, 0x04e0, 0x18);
	/*window*/
	gc2093_write_register(ViPipe, 0x0192, 0x02);
	gc2093_write_register(ViPipe, 0x0194, 0x03);
	gc2093_write_register(ViPipe, 0x0195, 0x04);
	gc2093_write_register(ViPipe, 0x0196, 0x38);
	gc2093_write_register(ViPipe, 0x0197, 0x07);
	gc2093_write_register(ViPipe, 0x0198, 0x80);
	/****DVP & MIPI****/
	gc2093_write_register(ViPipe, 0x019a, 0x06);
	gc2093_write_register(ViPipe, 0x007b, 0x2a);
	gc2093_write_register(ViPipe, 0x0023, 0x2d);
	gc2093_write_register(ViPipe, 0x0201, 0x27);
	gc2093_write_register(ViPipe, 0x0202, 0x56);
	gc2093_write_register(ViPipe, 0x0203, 0xb6);
	gc2093_write_register(ViPipe, 0x0212, 0x80);
	gc2093_write_register(ViPipe, 0x0213, 0x07);
	gc2093_write_register(ViPipe, 0x0215, 0x10);
	gc2093_write_register(ViPipe, 0x003e, 0x91);

	gc2093_default_reg_init(ViPipe);
	usleep(80*1000);

	printf("ViPipe:%d,===GC2093 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

static void gc2093_wdr_1080p30_init(VI_PIPE ViPipe)
{
	usleep(10*1000);
	/****system**/
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0x00);
	gc2093_write_register(ViPipe, 0x03f2, 0x00);
	gc2093_write_register(ViPipe, 0x03f3, 0x00);
	gc2093_write_register(ViPipe, 0x03f4, 0x36);
	gc2093_write_register(ViPipe, 0x03f5, 0xc0);
	gc2093_write_register(ViPipe, 0x03f6, 0x0B);
	gc2093_write_register(ViPipe, 0x03f7, 0x01);
	gc2093_write_register(ViPipe, 0x03f8, 0x58);
	gc2093_write_register(ViPipe, 0x03f9, 0x40);
	gc2093_write_register(ViPipe, 0x03fc, 0x8e);
	/****CISCTL**/
	gc2093_write_register(ViPipe, 0x0087, 0x18);
	gc2093_write_register(ViPipe, 0x00ee, 0x30);
	gc2093_write_register(ViPipe, 0x00d0, 0xbf);
	gc2093_write_register(ViPipe, 0x01a0, 0x00);
	gc2093_write_register(ViPipe, 0x01a4, 0x40);
	gc2093_write_register(ViPipe, 0x01a5, 0x40);
	gc2093_write_register(ViPipe, 0x01a6, 0x40);
	gc2093_write_register(ViPipe, 0x01af, 0x09);
	gc2093_write_register(ViPipe, 0x0001, 0x00);
	gc2093_write_register(ViPipe, 0x0002, 0x02);
	gc2093_write_register(ViPipe, 0x0003, 0x04);
	gc2093_write_register(ViPipe, 0x0004, 0x02);
	gc2093_write_register(ViPipe, 0x0005, 0x02);
	gc2093_write_register(ViPipe, 0x0006, 0x94);
	gc2093_write_register(ViPipe, 0x0007, 0x00);
	gc2093_write_register(ViPipe, 0x0008, 0x11);
	gc2093_write_register(ViPipe, 0x0009, 0x00);
	gc2093_write_register(ViPipe, 0x000a, 0x02);
	gc2093_write_register(ViPipe, 0x000b, 0x00);
	gc2093_write_register(ViPipe, 0x000c, 0x04);
	gc2093_write_register(ViPipe, 0x000d, 0x04);
	gc2093_write_register(ViPipe, 0x000e, 0x40);
	gc2093_write_register(ViPipe, 0x000f, 0x07);
	gc2093_write_register(ViPipe, 0x0010, 0x8c);
	gc2093_write_register(ViPipe, 0x0013, 0x15);
	gc2093_write_register(ViPipe, 0x0019, 0x0c);
	gc2093_write_register(ViPipe, 0x0041, 0x04);
	gc2093_write_register(ViPipe, 0x0042, 0xE2);
	gc2093_write_register(ViPipe, 0x0053, 0x60);
	gc2093_write_register(ViPipe, 0x008d, 0x92);
	gc2093_write_register(ViPipe, 0x0090, 0x00);
	gc2093_write_register(ViPipe, 0x00c7, 0xe1);
	gc2093_write_register(ViPipe, 0x001b, 0x73);
	gc2093_write_register(ViPipe, 0x0028, 0x0d);
	gc2093_write_register(ViPipe, 0x0029, 0x24);
	gc2093_write_register(ViPipe, 0x002b, 0x04);
	gc2093_write_register(ViPipe, 0x002e, 0x23);
	gc2093_write_register(ViPipe, 0x0037, 0x03);
	gc2093_write_register(ViPipe, 0x0043, 0x04);
	gc2093_write_register(ViPipe, 0x0044, 0x28);
	gc2093_write_register(ViPipe, 0x004a, 0x01);
	gc2093_write_register(ViPipe, 0x004b, 0x20);
	gc2093_write_register(ViPipe, 0x0055, 0x28);
	gc2093_write_register(ViPipe, 0x0066, 0x3f);
	gc2093_write_register(ViPipe, 0x0068, 0x3f);
	gc2093_write_register(ViPipe, 0x006b, 0x44);
	gc2093_write_register(ViPipe, 0x0077, 0x00);
	gc2093_write_register(ViPipe, 0x0078, 0x20);
	gc2093_write_register(ViPipe, 0x007c, 0xa1);
	gc2093_write_register(ViPipe, 0x00ce, 0x7c);
	gc2093_write_register(ViPipe, 0x00d3, 0xd4);
	gc2093_write_register(ViPipe, 0x00e6, 0x50);
	/*gain*/
	gc2093_write_register(ViPipe, 0x00b6, 0xc0);
	gc2093_write_register(ViPipe, 0x00b0, 0x68);
	/*isp*/
	gc2093_write_register(ViPipe, 0x0101, 0x0c);
	gc2093_write_register(ViPipe, 0x0102, 0x89);
	gc2093_write_register(ViPipe, 0x0104, 0x01);
	gc2093_write_register(ViPipe, 0x010e, 0x01);
	gc2093_write_register(ViPipe, 0x010f, 0x00);
	gc2093_write_register(ViPipe, 0x0158, 0x00);
	/*dark sun*/
	gc2093_write_register(ViPipe, 0x0123, 0x08);
	gc2093_write_register(ViPipe, 0x0123, 0x00);
	gc2093_write_register(ViPipe, 0x0120, 0x01);
	gc2093_write_register(ViPipe, 0x0121, 0x04);
	gc2093_write_register(ViPipe, 0x0122, 0xd8);
	gc2093_write_register(ViPipe, 0x0124, 0x03);
	gc2093_write_register(ViPipe, 0x0125, 0xff);
	gc2093_write_register(ViPipe, 0x001a, 0x8c);
	gc2093_write_register(ViPipe, 0x00c6, 0xe0);
	/*blk*/
	gc2093_write_register(ViPipe, 0x0026, 0x30);
	gc2093_write_register(ViPipe, 0x0142, 0x00);
	gc2093_write_register(ViPipe, 0x0149, 0x1e);
	gc2093_write_register(ViPipe, 0x014a, 0x0f);
	gc2093_write_register(ViPipe, 0x014b, 0x00);
	gc2093_write_register(ViPipe, 0x0155, 0x07);
	gc2093_write_register(ViPipe, 0x0414, 0x78);
	gc2093_write_register(ViPipe, 0x0415, 0x78);
	gc2093_write_register(ViPipe, 0x0416, 0x78);
	gc2093_write_register(ViPipe, 0x0417, 0x78);
	gc2093_write_register(ViPipe, 0x0454, 0x78);
	gc2093_write_register(ViPipe, 0x0455, 0x78);
	gc2093_write_register(ViPipe, 0x0456, 0x78);
	gc2093_write_register(ViPipe, 0x0457, 0x78);
	gc2093_write_register(ViPipe, 0x04e0, 0x18);
	/*window*/
	gc2093_write_register(ViPipe, 0x0192, 0x02);
	gc2093_write_register(ViPipe, 0x0194, 0x03);
	gc2093_write_register(ViPipe, 0x0195, 0x04);
	gc2093_write_register(ViPipe, 0x0196, 0x38);
	gc2093_write_register(ViPipe, 0x0197, 0x07);
	gc2093_write_register(ViPipe, 0x0198, 0x80);
	/****DVP**/
	gc2093_write_register(ViPipe, 0x019a, 0x06);
	gc2093_write_register(ViPipe, 0x007b, 0x2a);
	gc2093_write_register(ViPipe, 0x0023, 0x2d);
	gc2093_write_register(ViPipe, 0x0201, 0x27);
	gc2093_write_register(ViPipe, 0x0202, 0x56);
	gc2093_write_register(ViPipe, 0x0203, 0xb6);
	gc2093_write_register(ViPipe, 0x0212, 0x80);
	gc2093_write_register(ViPipe, 0x0213, 0x07);
	gc2093_write_register(ViPipe, 0x0215, 0x10);
	gc2093_write_register(ViPipe, 0x003e, 0x91);
	/****HDR EN**/
	gc2093_write_register(ViPipe, 0x0027, 0x71);
	gc2093_write_register(ViPipe, 0x0215, 0x92);
	gc2093_write_register(ViPipe, 0x024d, 0x01);
	gc2093_default_reg_init(ViPipe);
	usleep(80*1000);

	printf("ViPipe:%d,===GC2093 1080P 30fps 10bit LINE WDR2TO1 OK!===\n", ViPipe);
}
