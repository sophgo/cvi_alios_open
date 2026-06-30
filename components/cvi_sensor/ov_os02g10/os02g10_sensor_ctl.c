#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "os02g10_cmos_ex.h"

static void os02g10_linear_1080p30_init(VI_PIPE ViPipe);

CVI_U8 os02g10_i2c_addr = 0x3d;        /* I2C Address of OS02G10 */
const CVI_U32 os02g10_addr_byte = 1;
const CVI_U32 os02g10_data_byte = 1;

int os02g10_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOs02g10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int os02g10_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOs02g10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int os02g10_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOs02g10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, os02g10_i2c_addr, (CVI_U32)addr, os02g10_addr_byte, os02g10_data_byte);

}

int os02g10_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOs02g10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, os02g10_i2c_addr, (CVI_U32)addr, os02g10_addr_byte,
				(CVI_U32)data, os02g10_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void os02g10_standby(VI_PIPE ViPipe)
{
	printf("os02g10 standby.\n");
}

void os02g10_restart(VI_PIPE ViPipe)
{
	printf("os02g10 restart.\n");
}

void os02g10_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;
	for (i = 0; i < g_pastOs02g10[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastOs02g10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			os02g10_write_register(ViPipe,
				g_pastOs02g10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOs02g10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

#define OS02G10_CHIP_ID_ADDR_0		0x02
#define OS02G10_CHIP_ID_ADDR_1		0x03
#define OS02G10_CHIP_ID_ADDR_2		0x04
#define OS02G10_CHIP_ID_ADDR_3		0x05
#define OS02G10_CHIP_ID			0x56024700

int os02g10_probe(VI_PIPE ViPipe)
{
	int nVal0, nVal1, nVal2, nVal3;
	CVI_U32 chip_id;

	usleep(1000);
	if (os02g10_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal0  = os02g10_read_register(ViPipe, OS02G10_CHIP_ID_ADDR_0);
	nVal1 = os02g10_read_register(ViPipe, OS02G10_CHIP_ID_ADDR_1);
	nVal2  = os02g10_read_register(ViPipe, OS02G10_CHIP_ID_ADDR_2);
	nVal3 = os02g10_read_register(ViPipe, OS02G10_CHIP_ID_ADDR_3);
	if (nVal0 < 0 || nVal1 < 0 || nVal2 < 0 || nVal3 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return CVI_FAILURE;
	}
	chip_id = (nVal0 << 24) | (nVal1 << 16) | (nVal2 << 8) | nVal3;
	if (chip_id != OS02G10_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

void os02g10_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastOs02g10[ViPipe]->enWDRMode;

	os02g10_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
		os02g10_linear_1080p30_init(ViPipe);
	}
	g_pastOs02g10[ViPipe]->bInit = CVI_TRUE;
}

void os02g10_exit(VI_PIPE ViPipe)
{
	os02g10_i2c_exit(ViPipe);
}

/* 1080P30 */
static void os02g10_linear_1080p30_init(VI_PIPE ViPipe)
{
	os02g10_write_register(ViPipe, 0xfd, 0x00);
	os02g10_write_register(ViPipe, 0x36, 0x01);
	os02g10_write_register(ViPipe, 0xfd, 0x00);
	os02g10_write_register(ViPipe, 0x36, 0x00);
	os02g10_write_register(ViPipe, 0xfd, 0x00);
	os02g10_write_register(ViPipe, 0x20, 0x00);
	delay_ms(5); //delay 5ms
	os02g10_write_register(ViPipe, 0xfd, 0x00);
	os02g10_write_register(ViPipe, 0xfd, 0x00);
	os02g10_write_register(ViPipe, 0x30, 0x0a);
	os02g10_write_register(ViPipe, 0x35, 0x04);
	os02g10_write_register(ViPipe, 0x38, 0x11);
	os02g10_write_register(ViPipe, 0x41, 0x06);
	os02g10_write_register(ViPipe, 0x44, 0x20);
	os02g10_write_register(ViPipe, 0xfd, 0x01);
	os02g10_write_register(ViPipe, 0x03, 0x04);
	os02g10_write_register(ViPipe, 0x04, 0x4c);
	os02g10_write_register(ViPipe, 0x06, 0x8a);
	os02g10_write_register(ViPipe, 0x24, 0x30);
	os02g10_write_register(ViPipe, 0x01, 0x01);
	os02g10_write_register(ViPipe, 0x19, 0x50);
	os02g10_write_register(ViPipe, 0x1a, 0x0c);
	os02g10_write_register(ViPipe, 0x1b, 0x0d);
	os02g10_write_register(ViPipe, 0x1c, 0x00);
	os02g10_write_register(ViPipe, 0x1d, 0x75);
	os02g10_write_register(ViPipe, 0x1e, 0x52);
	os02g10_write_register(ViPipe, 0x22, 0x14);
	os02g10_write_register(ViPipe, 0x25, 0x44);
	os02g10_write_register(ViPipe, 0x26, 0x0f);
	os02g10_write_register(ViPipe, 0x3c, 0xca);
	os02g10_write_register(ViPipe, 0x3d, 0x4a);
	os02g10_write_register(ViPipe, 0x40, 0x0f);
	os02g10_write_register(ViPipe, 0x43, 0x38);
	os02g10_write_register(ViPipe, 0x46, 0x00);
	os02g10_write_register(ViPipe, 0x47, 0x00);
	os02g10_write_register(ViPipe, 0x49, 0x32);
	os02g10_write_register(ViPipe, 0x50, 0x01);
	os02g10_write_register(ViPipe, 0x51, 0x28);
	os02g10_write_register(ViPipe, 0x52, 0x20);
	os02g10_write_register(ViPipe, 0x53, 0x03);
	os02g10_write_register(ViPipe, 0x57, 0x16);
	os02g10_write_register(ViPipe, 0x59, 0x01);
	os02g10_write_register(ViPipe, 0x5a, 0x01);
	os02g10_write_register(ViPipe, 0x5d, 0x04);
	os02g10_write_register(ViPipe, 0x6a, 0x04);
	os02g10_write_register(ViPipe, 0x6b, 0x03);
	os02g10_write_register(ViPipe, 0x6e, 0x28);
	os02g10_write_register(ViPipe, 0x71, 0xbe);
	os02g10_write_register(ViPipe, 0x72, 0x06);
	os02g10_write_register(ViPipe, 0x73, 0x38);
	os02g10_write_register(ViPipe, 0x74, 0x06);
	os02g10_write_register(ViPipe, 0x79, 0x00);
	os02g10_write_register(ViPipe, 0x7a, 0xb2);
	os02g10_write_register(ViPipe, 0x7b, 0x10);
	os02g10_write_register(ViPipe, 0x8f, 0x80);
	os02g10_write_register(ViPipe, 0x91, 0x38);
	os02g10_write_register(ViPipe, 0x92, 0x02);
	os02g10_write_register(ViPipe, 0x9d, 0x03);
	os02g10_write_register(ViPipe, 0x9e, 0x55);
	os02g10_write_register(ViPipe, 0xb8, 0x70);
	os02g10_write_register(ViPipe, 0xb9, 0x70);
	os02g10_write_register(ViPipe, 0xba, 0x70);
	os02g10_write_register(ViPipe, 0xbb, 0x70);
	os02g10_write_register(ViPipe, 0xbc, 0x00);
	os02g10_write_register(ViPipe, 0xc0, 0x00);
	os02g10_write_register(ViPipe, 0xc1, 0x00);
	os02g10_write_register(ViPipe, 0xc2, 0x00);
	os02g10_write_register(ViPipe, 0xc3, 0x00);
	os02g10_write_register(ViPipe, 0xc4, 0x6e);
	os02g10_write_register(ViPipe, 0xc5, 0x6e);
	os02g10_write_register(ViPipe, 0xc6, 0x6b);
	os02g10_write_register(ViPipe, 0xc7, 0x6b);
	os02g10_write_register(ViPipe, 0xcc, 0x11);
	os02g10_write_register(ViPipe, 0xcd, 0xe0);
	os02g10_write_register(ViPipe, 0xd0, 0x1b);
	os02g10_write_register(ViPipe, 0xd2, 0x76);
	os02g10_write_register(ViPipe, 0xd3, 0x68);
	os02g10_write_register(ViPipe, 0xd4, 0x68);
	os02g10_write_register(ViPipe, 0xd5, 0x73);
	os02g10_write_register(ViPipe, 0xd6, 0x73);
	os02g10_write_register(ViPipe, 0xe8, 0x55);
	os02g10_write_register(ViPipe, 0xf0, 0x40);
	os02g10_write_register(ViPipe, 0xf1, 0x40);
	os02g10_write_register(ViPipe, 0xf2, 0x40);
	os02g10_write_register(ViPipe, 0xf3, 0x40);
	os02g10_write_register(ViPipe, 0xf4, 0x00);
	os02g10_write_register(ViPipe, 0xfa, 0x1c);
	os02g10_write_register(ViPipe, 0xfb, 0x33);
	os02g10_write_register(ViPipe, 0xfc, 0xff);
	os02g10_write_register(ViPipe, 0xfe, 0x01);
	os02g10_write_register(ViPipe, 0xfd, 0x03);
	os02g10_write_register(ViPipe, 0x03, 0x67);
	os02g10_write_register(ViPipe, 0x00, 0x59);
	os02g10_write_register(ViPipe, 0x04, 0x11);
	os02g10_write_register(ViPipe, 0x05, 0x04);
	os02g10_write_register(ViPipe, 0x06, 0x0c);
	os02g10_write_register(ViPipe, 0x07, 0x08);
	os02g10_write_register(ViPipe, 0x08, 0x08);
	os02g10_write_register(ViPipe, 0x09, 0x4f);
	os02g10_write_register(ViPipe, 0x0b, 0x08);
	os02g10_write_register(ViPipe, 0x0d, 0x26);
	os02g10_write_register(ViPipe, 0x0f, 0x00);
	os02g10_write_register(ViPipe, 0xfd, 0x02);
	os02g10_write_register(ViPipe, 0x34, 0xfe);
	os02g10_write_register(ViPipe, 0x5e, 0x22);
	os02g10_write_register(ViPipe, 0xa1, 0x06);
	os02g10_write_register(ViPipe, 0xa3, 0x38);
	os02g10_write_register(ViPipe, 0xa5, 0x02);
	os02g10_write_register(ViPipe, 0xa7, 0x80);
	os02g10_write_register(ViPipe, 0xfd, 0x01);
	os02g10_write_register(ViPipe, 0xa1, 0x05);
	os02g10_write_register(ViPipe, 0x94, 0x44);
	os02g10_write_register(ViPipe, 0x95, 0x44);
	os02g10_write_register(ViPipe, 0x96, 0x09);
	os02g10_write_register(ViPipe, 0x98, 0x44);
	os02g10_write_register(ViPipe, 0x9c, 0x0e);
	os02g10_write_register(ViPipe, 0xb1, 0x01);
	os02g10_write_register(ViPipe, 0xfd, 0x01);
	os02g10_write_register(ViPipe, 0xb1, 0x03);

	os02g10_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===OS02G10 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}




