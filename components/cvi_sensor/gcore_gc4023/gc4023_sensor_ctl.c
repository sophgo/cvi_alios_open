#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "gc4023_cmos_ex.h"
#define GC4023_CHIP_ID_ADDR_H	0x03f0
#define GC4023_CHIP_ID_ADDR_L	0x03f1
#define GC4023_CHIP_ID		0x4023

#define GC4023_MIRROR_NORMAL

#if defined(GC4023_MIRROR_NORMAL)
#define     MIRROR 0x00
#define OTP_MIRROR 0x60
#elif defined(GC4023_MIRROR_H)
#define     MIRROR 0x01
#define OTP_MIRROR 0x61
#elif defined(GC4023_MIRROR_V)
#define     MIRROR 0x02
#define OTP_MIRROR 0x62
#elif defined(GC4023_MIRROR_HV)
#define     MIRROR 0x03
#define OTP_MIRROR 0x63
#else
#define     MIRROR 0x00
#define OTP_MIRROR 0x60
#endif

static void gc4023_linear_1440p30_init(VI_PIPE ViPipe);

CVI_U8 gc4023_i2c_addr = 0x29;
const CVI_U32 gc4023_addr_byte = 2;
const CVI_U32 gc4023_data_byte = 1;

int gc4023_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc4023_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc4023_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc4023_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int gc4023_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc4023_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc4023_i2c_addr, (CVI_U32)addr, gc4023_addr_byte, gc4023_data_byte);
}

int gc4023_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc4023_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc4023_i2c_addr, (CVI_U32)addr, gc4023_addr_byte,
		(CVI_U32)data, gc4023_data_byte);
}

static void delay_ms(int ms)
{
	udelay(ms * 1000);
}

// void gc4023_prog(VI_PIPE ViPipe, int *rom)
// {
// 	int i = 0;

// 	while (1) {
// 		int lookup = rom[i++];
// 		int addr = (lookup >> 16) & 0xFFFF;
// 		int data = lookup & 0xFFFF;

// 		if (addr == 0xFFFE)
// 			delay_ms(data);
// 		else if (addr != 0xFFFF)
// 			gc4023_write_register(ViPipe, addr, data);
// 	}
// }

void gc4023_standby(VI_PIPE ViPipe)
{
	gc4023_write_register(ViPipe, 0x0100, 0x00);
	gc4023_write_register(ViPipe, 0x0a34, 0x00);
	gc4023_write_register(ViPipe, 0x061c, 0x10);
	gc4023_write_register(ViPipe, 0x031c, 0x01);
	gc4023_write_register(ViPipe, 0x0a38, 0x00);
}

void gc4023_restart(VI_PIPE ViPipe)
{
	gc4023_write_register(ViPipe, 0x0a38, 0x01);
	gc4023_write_register(ViPipe, 0x0a34, 0x40);
	gc4023_write_register(ViPipe, 0x061c, 0x50);
	gc4023_write_register(ViPipe, 0x031c, 0xce);
	gc4023_write_register(ViPipe, 0x0100, 0x09);
}

void gc4023_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc4023[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc4023_write_register(ViPipe,
				g_pastGc4023[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc4023[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}
int gc4023_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (gc4023_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc4023_read_register(ViPipe, GC4023_CHIP_ID_ADDR_H);
	nVal2 = gc4023_read_register(ViPipe, GC4023_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC4023_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc4023_init(VI_PIPE ViPipe)
{
	gc4023_i2c_init(ViPipe);

	gc4023_linear_1440p30_init(ViPipe);

	g_pastGc4023[ViPipe]->bInit = CVI_TRUE;
	g_aeGc4023_MirrorFip[ViPipe] = 0;
}

void gc4023_exit(VI_PIPE ViPipe)
{
	gc4023_i2c_exit(ViPipe);
}

static void gc4023_linear_1440p30_init(VI_PIPE ViPipe)
{
	delay_ms(10);
	gc4023_write_register(ViPipe, 0x03fe, 0xf0);
	gc4023_write_register(ViPipe, 0x03fe, 0x00);
	gc4023_write_register(ViPipe, 0x03fe, 0x10);
	gc4023_write_register(ViPipe, 0x03fe, 0x00);
	gc4023_write_register(ViPipe, 0x0a38, 0x00);
	gc4023_write_register(ViPipe, 0x0a38, 0x01);
	gc4023_write_register(ViPipe, 0x0a20, 0x07);
	gc4023_write_register(ViPipe, 0x061c, 0x50);
	gc4023_write_register(ViPipe, 0x061d, 0x22);
	gc4023_write_register(ViPipe, 0x061e, 0x78);
	gc4023_write_register(ViPipe, 0x061f, 0x06);
	gc4023_write_register(ViPipe, 0x0a21, 0x10);
	gc4023_write_register(ViPipe, 0x0a34, 0x40);
	gc4023_write_register(ViPipe, 0x0a35, 0x01);
	gc4023_write_register(ViPipe, 0x0a36, 0x4e);
	gc4023_write_register(ViPipe, 0x0a37, 0x06);
	gc4023_write_register(ViPipe, 0x0314, 0x50);
	gc4023_write_register(ViPipe, 0x0315, 0x00);
	gc4023_write_register(ViPipe, 0x031c, 0xce);
	gc4023_write_register(ViPipe, 0x0219, 0x47);
	gc4023_write_register(ViPipe, 0x0342, 0x04);
	gc4023_write_register(ViPipe, 0x0343, 0xb0);
	gc4023_write_register(ViPipe, 0x0259, 0x05);
	gc4023_write_register(ViPipe, 0x025a, 0xa0);
	gc4023_write_register(ViPipe, 0x0340, 0x05);
	gc4023_write_register(ViPipe, 0x0341, 0xdc);
	gc4023_write_register(ViPipe, 0x0347, 0x02);
	gc4023_write_register(ViPipe, 0x0348, 0x0a);
	gc4023_write_register(ViPipe, 0x0349, 0x08);
	gc4023_write_register(ViPipe, 0x034a, 0x05);
	gc4023_write_register(ViPipe, 0x034b, 0xa8);
	gc4023_write_register(ViPipe, 0x0094, 0x0a);
	gc4023_write_register(ViPipe, 0x0095, 0x00);
	gc4023_write_register(ViPipe, 0x0096, 0x05);
	gc4023_write_register(ViPipe, 0x0097, 0xa0);
	gc4023_write_register(ViPipe, 0x0099, 0x04);
	gc4023_write_register(ViPipe, 0x009b, 0x04);
	gc4023_write_register(ViPipe, 0x060c, 0x01);
	gc4023_write_register(ViPipe, 0x060e, 0x08);
	gc4023_write_register(ViPipe, 0x060f, 0x05);
	gc4023_write_register(ViPipe, 0x070c, 0x01);
	gc4023_write_register(ViPipe, 0x070e, 0x08);
	gc4023_write_register(ViPipe, 0x070f, 0x05);
	gc4023_write_register(ViPipe, 0x0909, 0x03);
	gc4023_write_register(ViPipe, 0x0902, 0x04);
	gc4023_write_register(ViPipe, 0x0904, 0x0b);
	gc4023_write_register(ViPipe, 0x0907, 0x54);
	gc4023_write_register(ViPipe, 0x0908, 0x06);
	gc4023_write_register(ViPipe, 0x0903, 0x9d);
	gc4023_write_register(ViPipe, 0x072a, 0x18);
	gc4023_write_register(ViPipe, 0x0724, 0x0a);
	gc4023_write_register(ViPipe, 0x0727, 0x0a);
	gc4023_write_register(ViPipe, 0x072a, 0x1c);
	gc4023_write_register(ViPipe, 0x072b, 0x0a);
	gc4023_write_register(ViPipe, 0x1466, 0x10);
	gc4023_write_register(ViPipe, 0x1468, 0x0b);
	gc4023_write_register(ViPipe, 0x1467, 0x13);
	gc4023_write_register(ViPipe, 0x1469, 0x80);
	gc4023_write_register(ViPipe, 0x146a, 0xe8);
	gc4023_write_register(ViPipe, 0x0707, 0x07);
	gc4023_write_register(ViPipe, 0x0737, 0x0f);
	gc4023_write_register(ViPipe, 0x0704, 0x01);
	gc4023_write_register(ViPipe, 0x0706, 0x03);
	gc4023_write_register(ViPipe, 0x0716, 0x03);
	gc4023_write_register(ViPipe, 0x0708, 0xc8);
	gc4023_write_register(ViPipe, 0x0718, 0xc8);
	gc4023_write_register(ViPipe, 0x061a, 0x00);
	gc4023_write_register(ViPipe, 0x1430, 0x80);
	gc4023_write_register(ViPipe, 0x1407, 0x10);
	gc4023_write_register(ViPipe, 0x1408, 0x16);
	gc4023_write_register(ViPipe, 0x1409, 0x03);
	gc4023_write_register(ViPipe, 0x146d, 0x0e);
	gc4023_write_register(ViPipe, 0x146e, 0x42);
	gc4023_write_register(ViPipe, 0x146f, 0x43);
	gc4023_write_register(ViPipe, 0x1470, 0x3c);
	gc4023_write_register(ViPipe, 0x1471, 0x3d);
	gc4023_write_register(ViPipe, 0x1472, 0x3a);
	gc4023_write_register(ViPipe, 0x1473, 0x3a);
	gc4023_write_register(ViPipe, 0x1474, 0x40);
	gc4023_write_register(ViPipe, 0x1475, 0x46);
	gc4023_write_register(ViPipe, 0x1420, 0x14);
	gc4023_write_register(ViPipe, 0x1464, 0x15);
	gc4023_write_register(ViPipe, 0x146c, 0x40);
	gc4023_write_register(ViPipe, 0x146d, 0x40);
	gc4023_write_register(ViPipe, 0x1423, 0x08);
	gc4023_write_register(ViPipe, 0x1428, 0x10);
	gc4023_write_register(ViPipe, 0x1462, 0x18);
	gc4023_write_register(ViPipe, 0x02ce, 0x04);
	gc4023_write_register(ViPipe, 0x143a, 0x0f);
	gc4023_write_register(ViPipe, 0x142b, 0x88);
	gc4023_write_register(ViPipe, 0x0245, 0xc9);
	gc4023_write_register(ViPipe, 0x023a, 0x08);
	gc4023_write_register(ViPipe, 0x02cd, 0x99);
	gc4023_write_register(ViPipe, 0x0612, 0x02);
	gc4023_write_register(ViPipe, 0x0613, 0xc7);
	gc4023_write_register(ViPipe, 0x0243, 0x03);
	gc4023_write_register(ViPipe, 0x021b, 0x09);
	gc4023_write_register(ViPipe, 0x0089, 0x03);
	gc4023_write_register(ViPipe, 0x0040, 0xa3);
	gc4023_write_register(ViPipe, 0x0075, 0x64);
	gc4023_write_register(ViPipe, 0x0004, 0x0f);
	gc4023_write_register(ViPipe, 0x0002, 0xab);
	gc4023_write_register(ViPipe, 0x0053, 0x0a);
	gc4023_write_register(ViPipe, 0x0205, 0x0c);
	gc4023_write_register(ViPipe, 0x0202, 0x06);
	gc4023_write_register(ViPipe, 0x0203, 0x27);
	gc4023_write_register(ViPipe, 0x0614, 0x00);
	gc4023_write_register(ViPipe, 0x0615, 0x00);
	gc4023_write_register(ViPipe, 0x0181, 0x0c);
	gc4023_write_register(ViPipe, 0x0182, 0x05);
	gc4023_write_register(ViPipe, 0x0185, 0x01);
	gc4023_write_register(ViPipe, 0x0180, 0x46);
	gc4023_write_register(ViPipe, 0x0100, 0x08);
	gc4023_write_register(ViPipe, 0x0106, 0x38);
	gc4023_write_register(ViPipe, 0x010d, 0x80);
	gc4023_write_register(ViPipe, 0x010e, 0x0c);
	gc4023_write_register(ViPipe, 0x0113, 0x02);
	gc4023_write_register(ViPipe, 0x0114, 0x01);
	gc4023_write_register(ViPipe, 0x0115, 0x10);
	gc4023_write_register(ViPipe, 0x022c, MIRROR);
	gc4023_write_register(ViPipe, 0x0100, 0x09);

	gc4023_write_register(ViPipe, 0x0a67, 0x80);
	gc4023_write_register(ViPipe, 0x0a54, 0x0e);
	gc4023_write_register(ViPipe, 0x0a65, 0x10);
	gc4023_write_register(ViPipe, 0x0a98, 0x10);
	gc4023_write_register(ViPipe, 0x05be, 0x00);
	gc4023_write_register(ViPipe, 0x05a9, 0x01);
	gc4023_write_register(ViPipe, 0x0029, 0x08);
	gc4023_write_register(ViPipe, 0x002b, 0xa8);
	gc4023_write_register(ViPipe, 0x0a83, 0xe0);
	gc4023_write_register(ViPipe, 0x0a72, 0x02);
	gc4023_write_register(ViPipe, 0x0a73, OTP_MIRROR);
	gc4023_write_register(ViPipe, 0x0a75, 0x41);
	gc4023_write_register(ViPipe, 0x0a70, 0x03);
	gc4023_write_register(ViPipe, 0x0a5a, 0x80);
	delay_ms(20);
	gc4023_write_register(ViPipe, 0x05be, 0x01);
	gc4023_write_register(ViPipe, 0x0a70, 0x00);
	gc4023_write_register(ViPipe, 0x0080, 0x02);
	gc4023_write_register(ViPipe, 0x0a67, 0x00);

	gc4023_default_reg_init(ViPipe);
	delay_ms(10);

	printf("ViPipe:%d,===GC4023 1440P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}
