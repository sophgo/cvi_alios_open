#include <unistd.h>

#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "sp2509_multi_cmos_ex.h"

#define SP2509_MULTI_CHIP_ID_ADDR_H 0x02
#define SP2509_MULTI_CHIP_ID_ADDR_L 0x03
#define SP2509_MULTI_CHIP_ID 0x2509

#define SP2509_MULTI_MIRROR_NORMAL 1
#define SP2509_MULTI_MIRROR_H 0
#define SP2509_MULTI_MIRROR_V 0
#define SP2509_MULTI_MIRROR_HV 0

#if SP2509_MULTI_MIRROR_NORMAL
#define SP2509_MULTI_MIRROR 0x0
#elif SP2509_MULTI_MIRROR_H
#define SP2509_MULTI_MIRROR 0x1
#elif SP2509_MULTI_MIRROR_V
#define SP2509_MULTI_MIRROR 0x2
#elif SP2509_MULTI_MIRROR_HV
#define SP2509_MULTI_MIRROR 0x3
#else
#define SP2509_MULTI_MIRROR 0x0
#endif

extern CVI_U16 g_au16Sp2509_multi_UseHwSync[VI_MAX_PIPE_NUM];
static void sp2509_multi_linear_1200p30_init(VI_PIPE ViPipe);
static void sp2509_multi_linear_600p45_init(VI_PIPE ViPipe);

CVI_U8 sp2509_multi_i2c_addr = 0x3d;
const CVI_U32 sp2509_multi_addr_byte = 1;
const CVI_U32 sp2509_multi_data_byte = 1;

int sp2509_multi_i2c_init(VI_PIPE ViPipe) {
	CVI_U8 i2c_id = (CVI_U8)g_aunSp2509_multi_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sp2509_multi_i2c_exit(VI_PIPE ViPipe) {
	CVI_U8 i2c_id = (CVI_U8)g_aunSp2509_multi_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sp2509_multi_read_register(VI_PIPE ViPipe, int addr) {
	CVI_U8 i2c_id = (CVI_U8)g_aunSp2509_multi_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sp2509_multi_i2c_addr, (CVI_U32)addr, sp2509_multi_addr_byte,
						   sp2509_multi_data_byte);
}

int sp2509_multi_write_register(VI_PIPE ViPipe, int addr, int data) {
	CVI_U8 i2c_id = (CVI_U8)g_aunSp2509_multi_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sp2509_multi_i2c_addr, (CVI_U32)addr, sp2509_multi_addr_byte, (CVI_U32)data,
							sp2509_multi_data_byte);
}

void sp2509_multi_standby(VI_PIPE ViPipe) {
    sp2509_multi_write_register(ViPipe, 0xac, 0x00);  // mipi disable
}

void sp2509_multi_restart(VI_PIPE ViPipe) {
    sp2509_multi_write_register(ViPipe, 0xac, 0x01);  // mipi enable
}

void sp2509_multi_default_reg_init(VI_PIPE ViPipe) {
	CVI_U32 i;

	for (i = 0; i < g_pastSp2509_multi[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sp2509_multi_write_register(ViPipe,
							  g_pastSp2509_multi[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
							  g_pastSp2509_multi[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int sp2509_multi_probe(VI_PIPE ViPipe) {
	int nVal;
	int nVal2;

	udelay(50);
	if (sp2509_multi_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sp2509_multi_read_register(ViPipe, SP2509_MULTI_CHIP_ID_ADDR_H);
	nVal2 = sp2509_multi_read_register(ViPipe, SP2509_MULTI_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != SP2509_MULTI_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sp2509_multi_init(VI_PIPE ViPipe) {
	CVI_U8 u8ImgMode = g_pastSp2509_multi[ViPipe]->u8ImgMode;
	WDR_MODE_E enWDRMode = g_pastSp2509_multi[ViPipe]->enWDRMode;
	sp2509_multi_i2c_init(ViPipe);

	printf("sp2509 Img mode: %d\n", u8ImgMode);
	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
		if (u8ImgMode == SP2509_MULTI_MODE_1600X1200P30)
			sp2509_multi_linear_1200p30_init(ViPipe);
		else if (u8ImgMode == SP2509_MULTI_MODE_800X600P45)
			sp2509_multi_linear_600p45_init(ViPipe);
		else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this ImgMode[%d]!\n", u8ImgMode);
			return;
		}
	}
	g_pastSp2509_multi[ViPipe]->bInit = CVI_TRUE;
}

void sp2509_multi_exit(VI_PIPE ViPipe) { sp2509_multi_i2c_exit(ViPipe); }

static void sp2509_multi_linear_1200p30_init(VI_PIPE ViPipe) {
	sp2509_multi_write_register(ViPipe, 0xfd, 0x00);
	sp2509_multi_write_register(ViPipe, 0x2f, 0x29);
	sp2509_multi_write_register(ViPipe, 0x34, 0x00);
	sp2509_multi_write_register(ViPipe, 0x35, 0x21);
	sp2509_multi_write_register(ViPipe, 0x30, 0x15);
	sp2509_multi_write_register(ViPipe, 0x33, 0x01);
	sp2509_multi_write_register(ViPipe, 0xfd, 0x01);
	sp2509_multi_write_register(ViPipe, 0x44, 0x00);
	sp2509_multi_write_register(ViPipe, 0x2a, 0x4c);
	sp2509_multi_write_register(ViPipe, 0x2b, 0x1e);
	sp2509_multi_write_register(ViPipe, 0x2c, 0x60);
	sp2509_multi_write_register(ViPipe, 0x25, 0x11);
	sp2509_multi_write_register(ViPipe, 0x03, 0x04);
	sp2509_multi_write_register(ViPipe, 0x04, 0x74);
	sp2509_multi_write_register(ViPipe, 0x09, 0x00);
	sp2509_multi_write_register(ViPipe, 0x0a, 0x02);
	sp2509_multi_write_register(ViPipe, 0x06, 0x2a);
	sp2509_multi_write_register(ViPipe, 0x31, 0x00);
	sp2509_multi_write_register(ViPipe, 0x24, 0xff);
	sp2509_multi_write_register(ViPipe, 0x01, 0x01);
	sp2509_multi_write_register(ViPipe, 0xfb, 0x73);
    sp2509_multi_write_register(ViPipe, 0xfd, 0x01);
	sp2509_multi_write_register(ViPipe, 0x16, 0x04);
	sp2509_multi_write_register(ViPipe, 0x1c, 0x09);
	sp2509_multi_write_register(ViPipe, 0x21, 0x42);
	sp2509_multi_write_register(ViPipe, 0x12, 0x04);
	sp2509_multi_write_register(ViPipe, 0x13, 0x10);
	sp2509_multi_write_register(ViPipe, 0x11, 0x40);
	sp2509_multi_write_register(ViPipe, 0x33, 0x40);
	sp2509_multi_write_register(ViPipe, 0xd0, 0x00);
	sp2509_multi_write_register(ViPipe, 0xd1, 0x01);
	sp2509_multi_write_register(ViPipe, 0xd2, 0x00);
	sp2509_multi_write_register(ViPipe, 0x50, 0x00);
	sp2509_multi_write_register(ViPipe, 0x51, 0x23);
	sp2509_multi_write_register(ViPipe, 0x52, 0x20);
	sp2509_multi_write_register(ViPipe, 0x53, 0x00);
	sp2509_multi_write_register(ViPipe, 0x54, 0x02);
	sp2509_multi_write_register(ViPipe, 0x55, 0x20);
	sp2509_multi_write_register(ViPipe, 0x56, 0x02);
	sp2509_multi_write_register(ViPipe, 0x58, 0x48);
	sp2509_multi_write_register(ViPipe, 0x5d, 0x15);
	sp2509_multi_write_register(ViPipe, 0x5e, 0x05);
	sp2509_multi_write_register(ViPipe, 0x66, 0xa6);
	sp2509_multi_write_register(ViPipe, 0x68, 0xa8);
	sp2509_multi_write_register(ViPipe, 0x6b, 0x00);
	sp2509_multi_write_register(ViPipe, 0x6c, 0x00);
	sp2509_multi_write_register(ViPipe, 0x6f, 0x40);
	sp2509_multi_write_register(ViPipe, 0x70, 0x40);
	sp2509_multi_write_register(ViPipe, 0x71, 0x10);
	sp2509_multi_write_register(ViPipe, 0x72, 0xf0);
	sp2509_multi_write_register(ViPipe, 0x73, 0x80);
	sp2509_multi_write_register(ViPipe, 0x75, 0xb9);
	sp2509_multi_write_register(ViPipe, 0x76, 0x80);
	sp2509_multi_write_register(ViPipe, 0x84, 0x00);
	sp2509_multi_write_register(ViPipe, 0x85, 0x10);
	sp2509_multi_write_register(ViPipe, 0x86, 0x10);
	sp2509_multi_write_register(ViPipe, 0x87, 0x00);
	sp2509_multi_write_register(ViPipe, 0x8a, 0x22);
	sp2509_multi_write_register(ViPipe, 0x8b, 0x22);
	sp2509_multi_write_register(ViPipe, 0x19, 0xf1);
	sp2509_multi_write_register(ViPipe, 0x29, 0x01);
	sp2509_multi_write_register(ViPipe, 0xfd, 0x01);
	sp2509_multi_write_register(ViPipe, 0x9d, 0xea);
	sp2509_multi_write_register(ViPipe, 0xa0, 0x29);
	sp2509_multi_write_register(ViPipe, 0xa1, 0x04);
	sp2509_multi_write_register(ViPipe, 0xad, 0x62);
	sp2509_multi_write_register(ViPipe, 0xae, 0x00);
	sp2509_multi_write_register(ViPipe, 0xaf, 0x85);
	sp2509_multi_write_register(ViPipe, 0xb1, 0x01);
	sp2509_multi_write_register(ViPipe, 0xf0, 0x40);
	sp2509_multi_write_register(ViPipe, 0xf1, 0x40);
	sp2509_multi_write_register(ViPipe, 0xf2, 0x40);
	sp2509_multi_write_register(ViPipe, 0xf3, 0x40);
	sp2509_multi_write_register(ViPipe, 0xac, 0x01);
	sp2509_multi_write_register(ViPipe, 0xfd, 0x01);
	sp2509_multi_write_register(ViPipe, 0x8e, 0x06);
	sp2509_multi_write_register(ViPipe, 0x8f, 0x40);
	sp2509_multi_write_register(ViPipe, 0x90, 0x04);
	sp2509_multi_write_register(ViPipe, 0x91, 0xb0);
	sp2509_multi_write_register(ViPipe, 0x45, 0x01);
	sp2509_multi_write_register(ViPipe, 0x46, 0x00);
	sp2509_multi_write_register(ViPipe, 0x47, 0x6c);
	sp2509_multi_write_register(ViPipe, 0x48, 0x03);
	sp2509_multi_write_register(ViPipe, 0x49, 0x8b);
	sp2509_multi_write_register(ViPipe, 0x4a, 0x00);
	sp2509_multi_write_register(ViPipe, 0x4b, 0x07);
	sp2509_multi_write_register(ViPipe, 0x4c, 0x04);
	sp2509_multi_write_register(ViPipe, 0x4d, 0xb7);
	sp2509_multi_write_register(ViPipe, 0xfd, 0x01);

	sp2509_multi_default_reg_init(ViPipe);
    sp2509_multi_standby(ViPipe);
	printf("ViPipe:%d,===SP2509 1200P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}

static void sp2509_multi_linear_600p45_init(VI_PIPE ViPipe) {
    sp2509_multi_write_register(ViPipe, 0xfd, 0x00);
    sp2509_multi_write_register(ViPipe, 0x2f, 0x08);
    sp2509_multi_write_register(ViPipe, 0x34, 0x00);
    sp2509_multi_write_register(ViPipe, 0x35, 0x21);
    sp2509_multi_write_register(ViPipe, 0x30, 0x1d);
    sp2509_multi_write_register(ViPipe, 0x33, 0x00);
    sp2509_multi_write_register(ViPipe, 0xfd, 0x01);
    sp2509_multi_write_register(ViPipe, 0x44, 0x00);
    sp2509_multi_write_register(ViPipe, 0x2a, 0x4c);
    sp2509_multi_write_register(ViPipe, 0x2b, 0x1e);
    sp2509_multi_write_register(ViPipe, 0x2c, 0x60);
    sp2509_multi_write_register(ViPipe, 0x25, 0x11);
    sp2509_multi_write_register(ViPipe, 0x30, 0x01);
    sp2509_multi_write_register(ViPipe, 0x31, 0x04);
    sp2509_multi_write_register(ViPipe, 0x03, 0x01);
    sp2509_multi_write_register(ViPipe, 0x04, 0x68);
    sp2509_multi_write_register(ViPipe, 0x09, 0x00);
    sp2509_multi_write_register(ViPipe, 0x0a, 0x02);
    sp2509_multi_write_register(ViPipe, 0x06, 0x0a);
    sp2509_multi_write_register(ViPipe, 0x24, 0xf0);
    sp2509_multi_write_register(ViPipe, 0x01, 0x01);
    sp2509_multi_write_register(ViPipe, 0xfb, 0x73);
    sp2509_multi_write_register(ViPipe, 0xfd, 0x01);
    sp2509_multi_write_register(ViPipe, 0x16, 0x04);
    sp2509_multi_write_register(ViPipe, 0x1a, 0x13);
    sp2509_multi_write_register(ViPipe, 0x1c, 0x09);
    sp2509_multi_write_register(ViPipe, 0x21, 0x46);
    sp2509_multi_write_register(ViPipe, 0x6c, 0x00);
    sp2509_multi_write_register(ViPipe, 0x6b, 0x00);
    sp2509_multi_write_register(ViPipe, 0x84, 0x00);
    sp2509_multi_write_register(ViPipe, 0x85, 0x10);
    sp2509_multi_write_register(ViPipe, 0x86, 0x10);
    sp2509_multi_write_register(ViPipe, 0x51, 0x14);
    sp2509_multi_write_register(ViPipe, 0x52, 0x10);
    sp2509_multi_write_register(ViPipe, 0x55, 0x20);
    sp2509_multi_write_register(ViPipe, 0x66, 0x56);
    sp2509_multi_write_register(ViPipe, 0x68, 0x58);
    sp2509_multi_write_register(ViPipe, 0x72, 0x60);
    sp2509_multi_write_register(ViPipe, 0x58, 0x10);
    sp2509_multi_write_register(ViPipe, 0x71, 0x10);
    sp2509_multi_write_register(ViPipe, 0x6f, 0x30);
    sp2509_multi_write_register(ViPipe, 0x75, 0x40);
    sp2509_multi_write_register(ViPipe, 0x76, 0x10);
    sp2509_multi_write_register(ViPipe, 0x8a, 0x22);
    sp2509_multi_write_register(ViPipe, 0x8b, 0x22);
    sp2509_multi_write_register(ViPipe, 0x19, 0xf1);
    sp2509_multi_write_register(ViPipe, 0x29, 0x01);
    sp2509_multi_write_register(ViPipe, 0xc9, 0x70);
    sp2509_multi_write_register(ViPipe, 0xd6, 0x40);
    sp2509_multi_write_register(ViPipe, 0xd8, 0xc0);
    sp2509_multi_write_register(ViPipe, 0xd9, 0x02);
    sp2509_multi_write_register(ViPipe, 0xda, 0x58);
    sp2509_multi_write_register(ViPipe, 0xdc, 0x60);
    sp2509_multi_write_register(ViPipe, 0xdd, 0x01);
    sp2509_multi_write_register(ViPipe, 0xde, 0x58);
    sp2509_multi_write_register(ViPipe, 0xea, 0x60);
    sp2509_multi_write_register(ViPipe, 0xeb, 0x01);
    sp2509_multi_write_register(ViPipe, 0xfd, 0x01);
    sp2509_multi_write_register(ViPipe, 0x8e, 0x03);
    sp2509_multi_write_register(ViPipe, 0x8f, 0x28);
    sp2509_multi_write_register(ViPipe, 0x90, 0x02);
    sp2509_multi_write_register(ViPipe, 0x91, 0x60);
    sp2509_multi_write_register(ViPipe, 0x9d, 0xea);
    sp2509_multi_write_register(ViPipe, 0xa0, 0x29);
    sp2509_multi_write_register(ViPipe, 0xa1, 0x04);
    sp2509_multi_write_register(ViPipe, 0xad, 0x62);
    sp2509_multi_write_register(ViPipe, 0xae, 0x00);
    sp2509_multi_write_register(ViPipe, 0xaf, 0x85);
    sp2509_multi_write_register(ViPipe, 0xb1, 0x01);
    sp2509_multi_write_register(ViPipe, 0xac, 0x01);
    sp2509_multi_write_register(ViPipe, 0xfd, 0x01);
    sp2509_multi_write_register(ViPipe, 0xfc, 0x10);
    sp2509_multi_write_register(ViPipe, 0xfe, 0x10);
    sp2509_multi_write_register(ViPipe, 0xf9, 0x00);
    sp2509_multi_write_register(ViPipe, 0xfa, 0x00);
    sp2509_multi_write_register(ViPipe, 0xfd, 0x02);
    sp2509_multi_write_register(ViPipe, 0x10, 0xff);
    sp2509_multi_write_register(ViPipe, 0x11, 0xff);
    sp2509_multi_write_register(ViPipe, 0x13, 0xff);
    sp2509_multi_write_register(ViPipe, 0x14, 0xff);
    sp2509_multi_write_register(ViPipe, 0xfd, 0x01);

    sp2509_multi_default_reg_init(ViPipe);
    sp2509_multi_standby(ViPipe);
    printf("ViPipe:%d,===SP2509 600P 45fps 10bit LINEAR Init OK!===\n", ViPipe);
}
