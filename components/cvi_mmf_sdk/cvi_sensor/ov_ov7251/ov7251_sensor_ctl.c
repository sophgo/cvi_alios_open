#include <unistd.h>
#include "sensor_i2c.h"
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "ov7251_cmos_ex.h"

static void ov7251_linear_480p120_init(VI_PIPE ViPipe);

CVI_U8 ov7251_i2c_addr = 0x60;        /* I2C Address of OV7251 */
const CVI_U32 ov7251_addr_byte = 2;
const CVI_U32 ov7251_data_byte = 1;

int ov7251_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunOv7251_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv7251_AddrInfo[ViPipe].s8I2cAddr);
}

int ov7251_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunOv7251_BusInfo[ViPipe].s8I2cDev);
}

int ov7251_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunOv7251_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv7251_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							ov7251_addr_byte, ov7251_data_byte);
}

int ov7251_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunOv7251_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv7251_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							ov7251_addr_byte, (CVI_U32)data, ov7251_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void ov7251_standby(VI_PIPE ViPipe)
{
	ov7251_write_register(ViPipe, 0x0100, 0x00); /* standby */
}

void ov7251_restart(VI_PIPE ViPipe)
{
	ov7251_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	ov7251_write_register(ViPipe, 0x0100, 0x01); /* restart */
}

void ov7251_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastOv7251[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastOv7251[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			ov7251_write_register(ViPipe,
				g_pastOv7251[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOv7251[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void ov7251_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 flip, mirror;

	flip = ov7251_read_register(ViPipe, 0x3820);
	mirror = ov7251_read_register(ViPipe, 0x3821);

	flip &= ~(0x1 << 2);
	mirror &= ~(0x1 << 2);

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		mirror |= 0x1 << 2;
		break;
	case ISP_SNS_FLIP:
		flip |= 0x1 << 2;
		break;
	case ISP_SNS_MIRROR_FLIP:
		flip |= 0x1 << 2;
		mirror |= 0x1 << 2;
		break;
	default:
		return;
	}

	ov7251_write_register(ViPipe, 0x3820, flip);
	ov7251_write_register(ViPipe, 0x3821, mirror);
}

#define OV7251_CHIP_ID_ADDR_H		0x300A
#define OV7251_CHIP_ID_ADDR_L		0x300B
#define OV7251_CHIP_ID			0x7750

int ov7251_probe(VI_PIPE ViPipe)
{
	int nVal, nVal2;

	if (ov7251_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	delay_ms(5);

	nVal  = ov7251_read_register(ViPipe, OV7251_CHIP_ID_ADDR_H);
	nVal2 = ov7251_read_register(ViPipe, OV7251_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | ((nVal2 & 0xFF) << 0)) != OV7251_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void ov7251_init(VI_PIPE ViPipe)
{
	ov7251_i2c_init(ViPipe);

	ov7251_linear_480p120_init(ViPipe);

	g_pastOv7251[ViPipe]->bInit = CVI_TRUE;
}

/* 1944P30 and 1944P25 */
static void ov7251_linear_480p120_init(VI_PIPE ViPipe)
{
	ov7251_write_register(ViPipe, 0x0103, 0x01);
	ov7251_write_register(ViPipe, 0x0100, 0x00);
	ov7251_write_register(ViPipe, 0x3012, 0xc0);
	ov7251_write_register(ViPipe, 0x3013, 0xd2);
	ov7251_write_register(ViPipe, 0x3016, 0x10);
	ov7251_write_register(ViPipe, 0x3017, 0x00);
	ov7251_write_register(ViPipe, 0x3018, 0x00);
	ov7251_write_register(ViPipe, 0x301a, 0x00);
	ov7251_write_register(ViPipe, 0x301b, 0x00);
	ov7251_write_register(ViPipe, 0x301c, 0x00);
	ov7251_write_register(ViPipe, 0x3023, 0x05);
	ov7251_write_register(ViPipe, 0x3099, 0x32);
	ov7251_write_register(ViPipe, 0x30b3, 0x64);
	ov7251_write_register(ViPipe, 0x30b4, 0x03);
	ov7251_write_register(ViPipe, 0x30b5, 0x05);
	ov7251_write_register(ViPipe, 0x3106, 0xda);
	ov7251_write_register(ViPipe, 0x3501, 0x1f);
	ov7251_write_register(ViPipe, 0x3502, 0x80);
	ov7251_write_register(ViPipe, 0x3503, 0x07);
	ov7251_write_register(ViPipe, 0x3600, 0x1c);
	ov7251_write_register(ViPipe, 0x3602, 0x62);
	ov7251_write_register(ViPipe, 0x3620, 0xb7);
	ov7251_write_register(ViPipe, 0x3622, 0x04);
	ov7251_write_register(ViPipe, 0x3626, 0x21);
	ov7251_write_register(ViPipe, 0x3627, 0x30);
	ov7251_write_register(ViPipe, 0x3630, 0x44);
	ov7251_write_register(ViPipe, 0x3631, 0x35);
	ov7251_write_register(ViPipe, 0x3634, 0x60);
	ov7251_write_register(ViPipe, 0x3663, 0x70);
	ov7251_write_register(ViPipe, 0x3669, 0x1a);
	ov7251_write_register(ViPipe, 0x3673, 0x01);
	ov7251_write_register(ViPipe, 0x3674, 0xef);
	ov7251_write_register(ViPipe, 0x3675, 0x03);
	ov7251_write_register(ViPipe, 0x3705, 0xc1);
	ov7251_write_register(ViPipe, 0x3757, 0xb3);
	ov7251_write_register(ViPipe, 0x37a8, 0x01);
	ov7251_write_register(ViPipe, 0x37a9, 0xc0);
	ov7251_write_register(ViPipe, 0x380d, 0xa0);
	ov7251_write_register(ViPipe, 0x380f, 0x1a);
	ov7251_write_register(ViPipe, 0x3811, 0x04);
	ov7251_write_register(ViPipe, 0x3813, 0x05);
	ov7251_write_register(ViPipe, 0x3820, 0x40);
	ov7251_write_register(ViPipe, 0x382f, 0x0e);
	ov7251_write_register(ViPipe, 0x3835, 0x0c);
	ov7251_write_register(ViPipe, 0x3b80, 0x00);
	ov7251_write_register(ViPipe, 0x3c01, 0x63);
	ov7251_write_register(ViPipe, 0x3c07, 0x06);
	ov7251_write_register(ViPipe, 0x3c0c, 0x01);
	ov7251_write_register(ViPipe, 0x3c0d, 0xd0);
	ov7251_write_register(ViPipe, 0x3c0e, 0x02);
	ov7251_write_register(ViPipe, 0x3c0f, 0x0a);
	ov7251_write_register(ViPipe, 0x4001, 0x42);
	ov7251_write_register(ViPipe, 0x404e, 0x01);
	ov7251_write_register(ViPipe, 0x4501, 0x48);
	ov7251_write_register(ViPipe, 0x4601, 0x4e);
	ov7251_write_register(ViPipe, 0x4801, 0x0f);
	ov7251_write_register(ViPipe, 0x4819, 0xaa);
	ov7251_write_register(ViPipe, 0x4823, 0x3e);
	ov7251_write_register(ViPipe, 0x4a47, 0x7f);
	ov7251_write_register(ViPipe, 0x4a49, 0xf0);
	ov7251_write_register(ViPipe, 0x4a4b, 0x30);
	ov7251_write_register(ViPipe, 0x5001, 0x80);

	ov7251_default_reg_init(ViPipe);

	ov7251_write_register(ViPipe, 0x0100, 0x01);

	delay_ms(100);

	printf("ViPipe:%d,===OV7251 480P 120fps 10bit LINE Init OK!===\n", ViPipe);
}

