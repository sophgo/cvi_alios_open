#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "f35_cmos_ex.h"
#include "sensor_i2c.h"

static void f35_wdr_1080p30_2to1_init(VI_PIPE ViPipe);
static void f35_linear_1080p30_init(VI_PIPE ViPipe);

const CVI_U8 f35_i2c_addr = 0x40;        /* I2C Address of F35 */
const CVI_U32 f35_addr_byte = 1;
const CVI_U32 f35_data_byte = 1;

int f35_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunF35_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunF35_AddrInfo[ViPipe].s8I2cAddr);
}

int f35_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunF35_BusInfo[ViPipe].s8I2cDev);
}

int f35_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunF35_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunF35_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							f35_addr_byte, f35_data_byte);
}

int f35_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunF35_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunF35_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							f35_addr_byte, (CVI_U32)data, f35_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void f35_standby(VI_PIPE ViPipe)
{
	f35_write_register(ViPipe, 0x12, 0x40);
}

void f35_restart(VI_PIPE ViPipe)
{
	f35_write_register(ViPipe, 0x12, 0x40);
	delay_ms(20);
	f35_write_register(ViPipe, 0x12, 0x00);
}

void f35_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastF35[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		f35_write_register(ViPipe,
				g_pastF35[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastF35[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void f35_init(VI_PIPE ViPipe)
{
	WDR_MODE_E       enWDRMode;
	CVI_BOOL          bInit;
	CVI_U8            u8ImgMode;

	bInit       = g_pastF35[ViPipe]->bInit;
	enWDRMode   = g_pastF35[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastF35[ViPipe]->u8ImgMode;

	f35_i2c_init(ViPipe);

	/* When sensor first init, config all registers */
	if (bInit == CVI_FALSE) {
		if (enWDRMode == WDR_MODE_2To1_LINE) {
			if (u8ImgMode == F35_MODE_1080P30_WDR) {
				/* F35_MODE_1080P30_WDR */
				f35_wdr_1080p30_2to1_init(ViPipe);
			} else {
			}
		} else {
			f35_linear_1080p30_init(ViPipe);
		}
	}
	/* When sensor switch mode(linear<->WDR or resolution), config different registers(if possible) */
	else {
		if (enWDRMode == WDR_MODE_2To1_LINE) {
			if (u8ImgMode == F35_MODE_1080P30_WDR) {
				/* F35_MODE_1080P30_WDR */
				f35_wdr_1080p30_2to1_init(ViPipe);
			} else {
			}
		} else {
			f35_linear_1080p30_init(ViPipe);
		}
	}
	g_pastF35[ViPipe]->bInit = CVI_TRUE;
}

/* 1080P30 and 1080P25 */
static void f35_linear_1080p30_init(VI_PIPE ViPipe)
{
	f35_write_register(ViPipe, 0x12, 0x40);
	f35_write_register(ViPipe, 0x48, 0x8A);
	f35_write_register(ViPipe, 0x48, 0x0A);
	f35_write_register(ViPipe, 0x0E, 0x11);
	f35_write_register(ViPipe, 0x0F, 0x14);
	f35_write_register(ViPipe, 0x10, 0x20);
	f35_write_register(ViPipe, 0x11, 0x80);
	f35_write_register(ViPipe, 0x0D, 0xF0);
	f35_write_register(ViPipe, 0x5F, 0x42);
	f35_write_register(ViPipe, 0x60, 0x2B);
	f35_write_register(ViPipe, 0x58, 0x18);
	f35_write_register(ViPipe, 0x57, 0x60);
	f35_write_register(ViPipe, 0x64, 0xE0);
	f35_write_register(ViPipe, 0x20, 0x00);
	f35_write_register(ViPipe, 0x21, 0x05);
	f35_write_register(ViPipe, 0x22, 0x65);
	f35_write_register(ViPipe, 0x23, 0x04);
	f35_write_register(ViPipe, 0x24, 0xC0);
	f35_write_register(ViPipe, 0x25, 0x38);
	f35_write_register(ViPipe, 0x26, 0x43);
	f35_write_register(ViPipe, 0x27, 0x0C);
	f35_write_register(ViPipe, 0x28, 0x15);
	f35_write_register(ViPipe, 0x29, 0x02);
	f35_write_register(ViPipe, 0x2A, 0x00);
	f35_write_register(ViPipe, 0x2B, 0x12);
	f35_write_register(ViPipe, 0x2C, 0x00);
	f35_write_register(ViPipe, 0x2D, 0x00);
	f35_write_register(ViPipe, 0x2E, 0x14);
	f35_write_register(ViPipe, 0x2F, 0x44);
	f35_write_register(ViPipe, 0x41, 0xC4);
	f35_write_register(ViPipe, 0x42, 0x13);
	f35_write_register(ViPipe, 0x46, 0x01);
	f35_write_register(ViPipe, 0x76, 0x60);
	f35_write_register(ViPipe, 0x77, 0x09);
	f35_write_register(ViPipe, 0x80, 0x06);
	f35_write_register(ViPipe, 0x1D, 0x00);
	f35_write_register(ViPipe, 0x1E, 0x04);
	f35_write_register(ViPipe, 0x6C, 0x40);
	f35_write_register(ViPipe, 0x68, 0x00);
	f35_write_register(ViPipe, 0x70, 0x6D);
	f35_write_register(ViPipe, 0x71, 0xCD);
	f35_write_register(ViPipe, 0x72, 0x6A);
	f35_write_register(ViPipe, 0x73, 0x36);
	f35_write_register(ViPipe, 0x74, 0x02);
	f35_write_register(ViPipe, 0x78, 0x1E);
	f35_write_register(ViPipe, 0x89, 0x81);
	f35_write_register(ViPipe, 0x6E, 0x2C);
	f35_write_register(ViPipe, 0x32, 0x4F);
	f35_write_register(ViPipe, 0x33, 0x58);
	f35_write_register(ViPipe, 0x34, 0x5F);
	f35_write_register(ViPipe, 0x35, 0x5F);
	f35_write_register(ViPipe, 0x3A, 0xAF);
	f35_write_register(ViPipe, 0x3B, 0x00);
	f35_write_register(ViPipe, 0x3C, 0x70);
	f35_write_register(ViPipe, 0x3D, 0x8F);
	f35_write_register(ViPipe, 0x3E, 0xFF);
	f35_write_register(ViPipe, 0x3F, 0x85);
	f35_write_register(ViPipe, 0x40, 0xFF);
	f35_write_register(ViPipe, 0x56, 0x32);
	f35_write_register(ViPipe, 0x59, 0x67);
	f35_write_register(ViPipe, 0x85, 0x3C);
	f35_write_register(ViPipe, 0x8A, 0x04);
	f35_write_register(ViPipe, 0x91, 0x10);
	f35_write_register(ViPipe, 0x9C, 0xE1);
	f35_write_register(ViPipe, 0x5A, 0x09);
	f35_write_register(ViPipe, 0x5C, 0x4C);
	f35_write_register(ViPipe, 0x5D, 0xF4);
	f35_write_register(ViPipe, 0x5E, 0x1E);
	f35_write_register(ViPipe, 0x62, 0x04);
	f35_write_register(ViPipe, 0x63, 0x0F);
	f35_write_register(ViPipe, 0x66, 0x04);
	f35_write_register(ViPipe, 0x67, 0x30);
	f35_write_register(ViPipe, 0x6A, 0x12);
	f35_write_register(ViPipe, 0x7A, 0xA0);
	f35_write_register(ViPipe, 0x9D, 0x10);
	f35_write_register(ViPipe, 0x4A, 0x05);
	f35_write_register(ViPipe, 0x7E, 0xCD);
	f35_write_register(ViPipe, 0x50, 0x02);
	f35_write_register(ViPipe, 0x49, 0x10);
	f35_write_register(ViPipe, 0x47, 0x02);
	f35_write_register(ViPipe, 0x7B, 0x4A);
	f35_write_register(ViPipe, 0x7C, 0x0C);
	f35_write_register(ViPipe, 0x7F, 0x57);
	f35_write_register(ViPipe, 0x8F, 0x81);
	f35_write_register(ViPipe, 0x90, 0x00);
	f35_write_register(ViPipe, 0x8C, 0xFF);
	f35_write_register(ViPipe, 0x8D, 0xC7);
	f35_write_register(ViPipe, 0x8E, 0x00);
	f35_write_register(ViPipe, 0x8B, 0x01);
	f35_write_register(ViPipe, 0x0C, 0x00);
	f35_write_register(ViPipe, 0x69, 0x74);
	f35_write_register(ViPipe, 0x65, 0x02);
	f35_write_register(ViPipe, 0x81, 0x74);
	f35_write_register(ViPipe, 0x19, 0x20);

	f35_default_reg_init(ViPipe);

	f35_write_register(ViPipe, 0x12, 0x00);

	printf("ViPipe:%d,===F35 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

static void f35_wdr_1080p30_2to1_init(VI_PIPE ViPipe)
{
	f35_write_register(ViPipe, 0x12, 0x48);
	f35_write_register(ViPipe, 0x48, 0x96);
	f35_write_register(ViPipe, 0x48, 0x16);
	f35_write_register(ViPipe, 0x0E, 0x11);
	f35_write_register(ViPipe, 0x0F, 0x14);
	f35_write_register(ViPipe, 0x10, 0x3C);
	f35_write_register(ViPipe, 0x11, 0x80);
	f35_write_register(ViPipe, 0x0D, 0xF0);
	f35_write_register(ViPipe, 0x5F, 0x42);
	f35_write_register(ViPipe, 0x60, 0x2B);
	f35_write_register(ViPipe, 0x58, 0x18);
	f35_write_register(ViPipe, 0x57, 0x60);
	f35_write_register(ViPipe, 0x64, 0xE0);
	f35_write_register(ViPipe, 0x20, 0x58);
	f35_write_register(ViPipe, 0x21, 0x02);
	f35_write_register(ViPipe, 0x22, 0xCA);
	f35_write_register(ViPipe, 0x23, 0x08);
	f35_write_register(ViPipe, 0x24, 0xE0);
	f35_write_register(ViPipe, 0x25, 0x38);
	f35_write_register(ViPipe, 0x26, 0x41);
	f35_write_register(ViPipe, 0x27, 0x07);
	f35_write_register(ViPipe, 0x28, 0x25);
	f35_write_register(ViPipe, 0x29, 0x02);
	f35_write_register(ViPipe, 0x2A, 0x00);
	f35_write_register(ViPipe, 0x2B, 0x12);
	f35_write_register(ViPipe, 0x2C, 0x02);
	f35_write_register(ViPipe, 0x2D, 0x00);
	f35_write_register(ViPipe, 0x2E, 0x14);
	f35_write_register(ViPipe, 0x2F, 0x44);
	f35_write_register(ViPipe, 0x41, 0xC8);
	f35_write_register(ViPipe, 0x42, 0x13);
	f35_write_register(ViPipe, 0x46, 0x05);
	f35_write_register(ViPipe, 0x76, 0x60);
	f35_write_register(ViPipe, 0x77, 0x09);
	f35_write_register(ViPipe, 0x80, 0x06);
	f35_write_register(ViPipe, 0x1D, 0x00);
	f35_write_register(ViPipe, 0x1E, 0x04);
	f35_write_register(ViPipe, 0x6C, 0x40);
	f35_write_register(ViPipe, 0x68, 0x00);
	f35_write_register(ViPipe, 0x70, 0xDD);
	f35_write_register(ViPipe, 0x71, 0xCB);
	f35_write_register(ViPipe, 0x72, 0xD5);
	f35_write_register(ViPipe, 0x73, 0x59);
	f35_write_register(ViPipe, 0x74, 0x02);
	f35_write_register(ViPipe, 0x78, 0x94);
	f35_write_register(ViPipe, 0x89, 0x81);
	f35_write_register(ViPipe, 0x6E, 0x2C);
	f35_write_register(ViPipe, 0x84, 0x20);
	f35_write_register(ViPipe, 0x6B, 0x20);
	f35_write_register(ViPipe, 0x86, 0x40);
	f35_write_register(ViPipe, 0x32, 0x4F);
	f35_write_register(ViPipe, 0x33, 0x58);
	f35_write_register(ViPipe, 0x34, 0x5F);
	f35_write_register(ViPipe, 0x35, 0x5F);
	f35_write_register(ViPipe, 0x3A, 0xAF);
	f35_write_register(ViPipe, 0x3B, 0x00);
	f35_write_register(ViPipe, 0x3C, 0x70);
	f35_write_register(ViPipe, 0x3D, 0x8F);
	f35_write_register(ViPipe, 0x3E, 0xFF);
	f35_write_register(ViPipe, 0x3F, 0x85);
	f35_write_register(ViPipe, 0x40, 0xFF);
	f35_write_register(ViPipe, 0x56, 0x32);
	f35_write_register(ViPipe, 0x59, 0x67);
	f35_write_register(ViPipe, 0x85, 0x3C);
	f35_write_register(ViPipe, 0x8A, 0x04);
	f35_write_register(ViPipe, 0x91, 0x10);
	f35_write_register(ViPipe, 0x9C, 0xE1);
	f35_write_register(ViPipe, 0x5A, 0x09);
	f35_write_register(ViPipe, 0x5C, 0x4C);
	f35_write_register(ViPipe, 0x5D, 0xF4);
	f35_write_register(ViPipe, 0x5E, 0x1E);
	f35_write_register(ViPipe, 0x62, 0x04);
	f35_write_register(ViPipe, 0x63, 0x0F);
	f35_write_register(ViPipe, 0x66, 0x04);
	f35_write_register(ViPipe, 0x67, 0x30);
	f35_write_register(ViPipe, 0x6A, 0x12);
	f35_write_register(ViPipe, 0x7A, 0xA0);
	f35_write_register(ViPipe, 0x9D, 0x10);
	f35_write_register(ViPipe, 0x4A, 0x05);
	f35_write_register(ViPipe, 0x7E, 0xCD);
	f35_write_register(ViPipe, 0x50, 0x02);
	f35_write_register(ViPipe, 0x49, 0x10);
	f35_write_register(ViPipe, 0x47, 0x02);
	f35_write_register(ViPipe, 0x7B, 0x4A);
	f35_write_register(ViPipe, 0x7C, 0x0C);
	f35_write_register(ViPipe, 0x7F, 0x57);
	f35_write_register(ViPipe, 0x8F, 0x81);
	f35_write_register(ViPipe, 0x90, 0x00);
	f35_write_register(ViPipe, 0x8C, 0xFF);
	f35_write_register(ViPipe, 0x8D, 0xC7);
	f35_write_register(ViPipe, 0x8E, 0x00);
	f35_write_register(ViPipe, 0x8B, 0x01);
	f35_write_register(ViPipe, 0x0C, 0x00);
	f35_write_register(ViPipe, 0x69, 0x74);
	f35_write_register(ViPipe, 0x65, 0x02);
	f35_write_register(ViPipe, 0x81, 0x74);
	f35_write_register(ViPipe, 0x19, 0x20);
	f35_write_register(ViPipe, 0xA9, 0x14);
	f35_write_register(ViPipe, 0xAA, 0xFF);	//set l2s to max value

	f35_default_reg_init(ViPipe);

	if (g_au16F35_GainMode[ViPipe] == SNS_GAIN_MODE_ONLY_LEF) {
		f35_write_register(ViPipe, 0x46, 0x01);
	} else {
		f35_write_register(ViPipe, 0x46, 0x05);
	}

	f35_write_register(ViPipe, 0x12, 0x08);

	usleep(33*1000);
	printf("===F35 sensor 1080P30fps 12bit 2to1 WDR(60fps->30fps) init success!=====\n");
}
