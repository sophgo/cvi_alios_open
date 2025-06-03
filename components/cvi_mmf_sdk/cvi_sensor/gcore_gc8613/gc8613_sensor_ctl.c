#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "gc8613_cmos_ex.h"
#include "sensor_i2c.h"

#define GC8613_CHIP_ID_ADDR_H 0x03f0
#define GC8613_CHIP_ID_ADDR_L 0x03f1
#define GC8613_MIRROR_ADDR	0x0063
#define GC8613_FLIP_ADDR	0x022c

static void gc8613_linear_2m30_init(VI_PIPE ViPipe);
static void gc8613_linear_8m30_init(VI_PIPE ViPipe);
static void gc8613_linear_8m25_init(VI_PIPE ViPipe);
static void gc8613_linear_8m60_30_init(VI_PIPE ViPipe);
static void gc8613_wdr_8m30_2to1_init(VI_PIPE ViPipe);

const CVI_U32 gc8613_addr_byte = 2;
const CVI_U32 gc8613_data_byte = 1;

int gc8613_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunGc8613_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunGc8613_AddrInfo[ViPipe].s8I2cAddr);
}

int gc8613_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunGc8613_BusInfo[ViPipe].s8I2cDev);
}

int gc8613_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunGc8613_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunGc8613_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							gc8613_addr_byte, gc8613_data_byte);
}

int gc8613_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunGc8613_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunGc8613_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							gc8613_addr_byte, (CVI_U32)data, gc8613_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void gc8613_standby(VI_PIPE ViPipe)
{
	gc8613_write_register(ViPipe, 0x0100, 0x00);
	gc8613_write_register(ViPipe, 0x03d1, 0x3d);
	gc8613_write_register(ViPipe, 0x061c, 0x10); // plldg en
	gc8613_write_register(ViPipe, 0x0a34, 0x00); // pllmp en
	gc8613_write_register(ViPipe, 0x031c, 0x8d); // not use pll
	gc8613_write_register(ViPipe, 0x0a38, 0x00); // apwd
	gc8613_write_register(ViPipe, 0x03d1, 0x38);
}

void gc8613_restart(VI_PIPE ViPipe)
{
	gc8613_write_register(ViPipe, 0x0a38, 0x01); // apwd
	gc8613_write_register(ViPipe, 0x061c, 0x50); // plldg en
	gc8613_write_register(ViPipe, 0x0a34, 0x40); // pllmp en
	gc8613_write_register(ViPipe, 0x031c, 0xce); // not use pll
	gc8613_write_register(ViPipe, 0x0100, 0x09);
	delay_ms(20);
}

void gc8613_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc8613[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc8613_write_register(ViPipe,
				g_pastGc8613[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc8613[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void gc8613_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 u8Flip = 0;
	CVI_U8 u8Mirror = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		u8Mirror = 0;
		u8Flip = 0;
		break;
	case ISP_SNS_MIRROR:
		u8Mirror = 0x05;
		u8Flip = 0;
		break;
	case ISP_SNS_FLIP:
		u8Mirror = 0x02;
		u8Flip = 0x01;
		break;
	case ISP_SNS_MIRROR_FLIP:
		u8Mirror = 0x05;
		u8Flip = 0x01;
		break;
	default:
		return;
	}

	gc8613_write_register(ViPipe, GC8613_MIRROR_ADDR, u8Mirror);
	gc8613_write_register(ViPipe, GC8613_FLIP_ADDR, u8Flip);
}

int gc8613_probe(VI_PIPE ViPipe)
{
	int nVal = 0, nVal2 = 0;

	usleep(100);
	if (gc8613_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = gc8613_read_register(ViPipe, GC8613_CHIP_ID_ADDR_H);
	nVal2 = gc8613_read_register(ViPipe, GC8613_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC8613_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! [0x%4x].\n", (((nVal & 0xFF) << 8) | (nVal2 & 0xFF)));
	}

	return CVI_SUCCESS;
}

void gc8613_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastGc8613[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastGc8613[ViPipe]->u8ImgMode;

	gc8613_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == GC8613_MODE_8M30_WDR)
			gc8613_wdr_8m30_2to1_init(ViPipe);
		else{
		}
	} else {
		if (u8ImgMode == GC8613_MODE_8M30)
			gc8613_linear_8m30_init(ViPipe);
		else if (u8ImgMode == GC8613_MODE_8M25)
			gc8613_linear_8m25_init(ViPipe);
		else if (u8ImgMode == GC8613_MODE_2M30)
			gc8613_linear_2m30_init(ViPipe);
		else if (u8ImgMode == GC8613_MODE_8M60_30)
			gc8613_linear_8m60_30_init(ViPipe);
		else {
		}
	}
	g_pastGc8613[ViPipe]->bInit = CVI_TRUE;
}

static void gc8613_linear_8m60_30_init(VI_PIPE ViPipe)
{
	// 24M 60fps setting changed vts(0x340 0x341) to output 30fps to increase vblanking time(17ms)
	delay_ms(4);
	gc8613_write_register(ViPipe, 0x03fe, 0xf0);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x03fe, 0x10);
	gc8613_write_register(ViPipe, 0x0a38, 0x01);
	gc8613_write_register(ViPipe, 0x0a20, 0x19);
	gc8613_write_register(ViPipe, 0x061b, 0x17);
	gc8613_write_register(ViPipe, 0x061c, 0x44);
	gc8613_write_register(ViPipe, 0x061d, 0x05);
	gc8613_write_register(ViPipe, 0x061e, 0x75);
	gc8613_write_register(ViPipe, 0x061f, 0x04);
	gc8613_write_register(ViPipe, 0x0a21, 0x10);
	gc8613_write_register(ViPipe, 0x0a30, 0x00);
	gc8613_write_register(ViPipe, 0x0a31, 0xf3);
	gc8613_write_register(ViPipe, 0x0a34, 0x40);
	gc8613_write_register(ViPipe, 0x0a35, 0x08);
	gc8613_write_register(ViPipe, 0x0a37, 0x44);
	gc8613_write_register(ViPipe, 0x0314, 0x50);
	gc8613_write_register(ViPipe, 0x031c, 0xce);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0342, 0x02);
	gc8613_write_register(ViPipe, 0x0343, 0x78);
	gc8613_write_register(ViPipe, 0x0259, 0x08);
	gc8613_write_register(ViPipe, 0x025a, 0x96);
	gc8613_write_register(ViPipe, 0x0340, 0x12); // vts 30fps
	gc8613_write_register(ViPipe, 0x0341, 0x24);
	gc8613_write_register(ViPipe, 0x0351, 0x00);
	gc8613_write_register(ViPipe, 0x0345, 0x02);
	gc8613_write_register(ViPipe, 0x0347, 0x02);
	gc8613_write_register(ViPipe, 0x0348, 0x0f);
	gc8613_write_register(ViPipe, 0x0349, 0x18);
	gc8613_write_register(ViPipe, 0x034a, 0x08);
	gc8613_write_register(ViPipe, 0x034b, 0x88);
	gc8613_write_register(ViPipe, 0x034f, 0xf0);
	gc8613_write_register(ViPipe, 0x0094, 0x0f);
	gc8613_write_register(ViPipe, 0x0095, 0x00);
	gc8613_write_register(ViPipe, 0x0096, 0x08);
	gc8613_write_register(ViPipe, 0x0097, 0x70);
	gc8613_write_register(ViPipe, 0x0099, 0x0c);
	gc8613_write_register(ViPipe, 0x009b, 0x0c);
	gc8613_write_register(ViPipe, 0x060c, 0x06);
	gc8613_write_register(ViPipe, 0x060e, 0x20);
	gc8613_write_register(ViPipe, 0x060f, 0x0f);
	gc8613_write_register(ViPipe, 0x070c, 0x06);
	gc8613_write_register(ViPipe, 0x070e, 0x20);
	gc8613_write_register(ViPipe, 0x070f, 0x0f);
	gc8613_write_register(ViPipe, 0x0087, 0x50);
	gc8613_write_register(ViPipe, 0x141b, 0x03);
	gc8613_write_register(ViPipe, 0x0901, 0x0e);
	gc8613_write_register(ViPipe, 0x0907, 0xd5);
	gc8613_write_register(ViPipe, 0x0909, 0x06);
	gc8613_write_register(ViPipe, 0x0902, 0x0b);
	gc8613_write_register(ViPipe, 0x0904, 0x08);
	gc8613_write_register(ViPipe, 0x0908, 0x09);
	gc8613_write_register(ViPipe, 0x0903, 0xc5);
	gc8613_write_register(ViPipe, 0x090c, 0x09);
	gc8613_write_register(ViPipe, 0x0905, 0x10);
	gc8613_write_register(ViPipe, 0x0906, 0x00);
	gc8613_write_register(ViPipe, 0x0724, 0x2b);
	gc8613_write_register(ViPipe, 0x0727, 0x2b);
	gc8613_write_register(ViPipe, 0x072b, 0x1a);
	gc8613_write_register(ViPipe, 0x072a, 0x5e);
	gc8613_write_register(ViPipe, 0x0601, 0x00);
	gc8613_write_register(ViPipe, 0x073e, 0x40);
	gc8613_write_register(ViPipe, 0x0078, 0x88);
	gc8613_write_register(ViPipe, 0x0618, 0x01);
	gc8613_write_register(ViPipe, 0x1466, 0x12);
	gc8613_write_register(ViPipe, 0x1468, 0x07);
	gc8613_write_register(ViPipe, 0x1467, 0x07);
	gc8613_write_register(ViPipe, 0x1478, 0x10);
	gc8613_write_register(ViPipe, 0x1477, 0x10);
	gc8613_write_register(ViPipe, 0x0709, 0x40);
	gc8613_write_register(ViPipe, 0x0719, 0x40);
	gc8613_write_register(ViPipe, 0x1469, 0x80);
	gc8613_write_register(ViPipe, 0x146a, 0x20);
	gc8613_write_register(ViPipe, 0x146b, 0x03);
	gc8613_write_register(ViPipe, 0x1479, 0x80);
	gc8613_write_register(ViPipe, 0x147a, 0x20);
	gc8613_write_register(ViPipe, 0x147b, 0x03);
	gc8613_write_register(ViPipe, 0x1480, 0x02);
	gc8613_write_register(ViPipe, 0x1481, 0x80);
	gc8613_write_register(ViPipe, 0x1482, 0x02);
	gc8613_write_register(ViPipe, 0x1483, 0x80);
	gc8613_write_register(ViPipe, 0x1484, 0x08);
	gc8613_write_register(ViPipe, 0x1485, 0xc0);
	gc8613_write_register(ViPipe, 0x1486, 0x08);
	gc8613_write_register(ViPipe, 0x1487, 0xc0);
	gc8613_write_register(ViPipe, 0x1407, 0x10);
	gc8613_write_register(ViPipe, 0x1408, 0x16);
	gc8613_write_register(ViPipe, 0x1409, 0x03);
	gc8613_write_register(ViPipe, 0x1434, 0x04);
	gc8613_write_register(ViPipe, 0x1447, 0x75);
	gc8613_write_register(ViPipe, 0x140d, 0x04);
	gc8613_write_register(ViPipe, 0x1461, 0x10);
	gc8613_write_register(ViPipe, 0x146c, 0x10);
	gc8613_write_register(ViPipe, 0x146d, 0x10);
	gc8613_write_register(ViPipe, 0x146e, 0x2e);
	gc8613_write_register(ViPipe, 0x146f, 0x30);
	gc8613_write_register(ViPipe, 0x1474, 0x34);
	gc8613_write_register(ViPipe, 0x1470, 0x10);
	gc8613_write_register(ViPipe, 0x1471, 0x13);
	gc8613_write_register(ViPipe, 0x143a, 0x00);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0245, 0xc7);
	gc8613_write_register(ViPipe, 0x025b, 0x07);
	gc8613_write_register(ViPipe, 0x02bb, 0x77);
	gc8613_write_register(ViPipe, 0x0612, 0x01);
	gc8613_write_register(ViPipe, 0x0613, 0x26);
	gc8613_write_register(ViPipe, 0x0243, 0x66);
	gc8613_write_register(ViPipe, 0x0087, 0x53);
	gc8613_write_register(ViPipe, 0x0053, 0x05);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0002, 0xeb);
	gc8613_write_register(ViPipe, 0x005a, 0x0c);
	gc8613_write_register(ViPipe, 0x0040, 0x83);
	gc8613_write_register(ViPipe, 0x0075, 0x58);
	gc8613_write_register(ViPipe, 0x0205, 0x0c);
	gc8613_write_register(ViPipe, 0x0202, 0x06);
	gc8613_write_register(ViPipe, 0x0203, 0x27);
	gc8613_write_register(ViPipe, 0x061a, 0x02);
	gc8613_write_register(ViPipe, 0x0122, 0x12);
	gc8613_write_register(ViPipe, 0x0123, 0x50);
	gc8613_write_register(ViPipe, 0x0126, 0x0f);
	gc8613_write_register(ViPipe, 0x0129, 0x10);
	gc8613_write_register(ViPipe, 0x012a, 0x20);
	gc8613_write_register(ViPipe, 0x012b, 0x10);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x0106, 0x78);
	gc8613_write_register(ViPipe, 0x0136, 0x00);
	gc8613_write_register(ViPipe, 0x0181, 0xf0);
	gc8613_write_register(ViPipe, 0x0185, 0x01);
	gc8613_write_register(ViPipe, 0x0180, 0x46);
	gc8613_write_register(ViPipe, 0x0106, 0x38);
	gc8613_write_register(ViPipe, 0x010d, 0xc0);
	gc8613_write_register(ViPipe, 0x010e, 0x12);
	gc8613_write_register(ViPipe, 0x0113, 0x02);
	gc8613_write_register(ViPipe, 0x0114, 0x03);
	gc8613_write_register(ViPipe, 0x0100, 0x09);
	gc8613_write_register(ViPipe, 0x0619, 0x01);
	gc8613_write_register(ViPipe, 0x023b, 0x58);
	gc8613_write_register(ViPipe, 0x023e, 0x00);
	gc8613_write_register(ViPipe, 0x023f, 0x84);
	gc8613_write_register(ViPipe, 0x0220, 0x80);
	gc8613_write_register(ViPipe, 0x021b, 0x96);
	gc8613_write_register(ViPipe, 0x0004, 0x0f);
	gc8613_write_register(ViPipe, 0x000e, 0x07);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0054, 0x98);
	gc8613_write_register(ViPipe, 0x0076, 0x01);
	gc8613_write_register(ViPipe, 0x0052, 0x02);
	gc8613_write_register(ViPipe, 0x021a, 0x10);
	gc8613_write_register(ViPipe, 0x0430, 0x10);
	gc8613_write_register(ViPipe, 0x0431, 0x10);
	gc8613_write_register(ViPipe, 0x0432, 0x10);
	gc8613_write_register(ViPipe, 0x0433, 0x10);
	gc8613_write_register(ViPipe, 0x0434, 0x6d);
	gc8613_write_register(ViPipe, 0x0435, 0x6d);
	gc8613_write_register(ViPipe, 0x0436, 0x6d);
	gc8613_write_register(ViPipe, 0x0437, 0x6d);
	gc8613_write_register(ViPipe, 0x0704, 0x03);
	gc8613_write_register(ViPipe, 0x0706, 0x02);
	gc8613_write_register(ViPipe, 0x0716, 0x02);
	gc8613_write_register(ViPipe, 0x0708, 0xc8);
	gc8613_write_register(ViPipe, 0x0718, 0xc8);
	gc8613_write_register(ViPipe, 0x071d, 0xdc);
	gc8613_write_register(ViPipe, 0x071e, 0x05);
	//otp autoload
	gc8613_write_register(ViPipe, 0x031f, 0x01);
	gc8613_write_register(ViPipe, 0x031f, 0x00);
	gc8613_write_register(ViPipe, 0x0a67, 0x80);
	gc8613_write_register(ViPipe, 0x0a54, 0x0e);
	gc8613_write_register(ViPipe, 0x0a65, 0x10);
	gc8613_write_register(ViPipe, 0x0a98, 0x04);
	gc8613_write_register(ViPipe, 0x05be, 0x00);
	gc8613_write_register(ViPipe, 0x05a9, 0x01);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0aa0, 0x00);
	gc8613_write_register(ViPipe, 0x0023, 0x00);
	gc8613_write_register(ViPipe, 0x0022, 0x00);
	gc8613_write_register(ViPipe, 0x0025, 0x00);
	gc8613_write_register(ViPipe, 0x0024, 0x00);
	gc8613_write_register(ViPipe, 0x0028, 0x0f);
	gc8613_write_register(ViPipe, 0x0029, 0x18);
	gc8613_write_register(ViPipe, 0x002a, 0x08);
	gc8613_write_register(ViPipe, 0x002b, 0x88);
	gc8613_write_register(ViPipe, 0x0317, 0x1c);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a82, 0x00);
	gc8613_write_register(ViPipe, 0x0a83, 0xe0);
	gc8613_write_register(ViPipe, 0x0a71, 0x00);
	gc8613_write_register(ViPipe, 0x0a72, 0x02);
	gc8613_write_register(ViPipe, 0x0a73, 0x60);
	gc8613_write_register(ViPipe, 0x0a75, 0x41);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a5a, 0x80);
	delay_ms(20);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x05be, 0x01);
	gc8613_write_register(ViPipe, 0x0a70, 0x00);
	gc8613_write_register(ViPipe, 0x0080, 0x02);
	gc8613_write_register(ViPipe, 0x0a67, 0x00);
	gc8613_write_register(ViPipe, 0x0020, 0x01);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0220, 0x80);
	gc8613_write_register(ViPipe, 0x0058, 0x00);
	gc8613_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===GC8613 8M (60->30) linear Init success!===\n", ViPipe);
}

static void gc8613_linear_8m30_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	gc8613_write_register(ViPipe, 0x03fe, 0xf0);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x03fe, 0x10);
	gc8613_write_register(ViPipe, 0x0a38, 0x01);
	gc8613_write_register(ViPipe, 0x0a20, 0x19);
	gc8613_write_register(ViPipe, 0x061b, 0x17);
	gc8613_write_register(ViPipe, 0x061c, 0x50);
	gc8613_write_register(ViPipe, 0x061d, 0x06);
	gc8613_write_register(ViPipe, 0x061e, 0x78);
	gc8613_write_register(ViPipe, 0x061f, 0x05);
	gc8613_write_register(ViPipe, 0x0a21, 0x10);
	gc8613_write_register(ViPipe, 0x0a31, 0xdf);
	gc8613_write_register(ViPipe, 0x0a34, 0x40);
	gc8613_write_register(ViPipe, 0x0a35, 0x08);
	gc8613_write_register(ViPipe, 0x0a37, 0x46);
	gc8613_write_register(ViPipe, 0x0314, 0x50);
	gc8613_write_register(ViPipe, 0x0315, 0x00);
	gc8613_write_register(ViPipe, 0x031c, 0xce);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0342, 0x03);
	gc8613_write_register(ViPipe, 0x0343, 0x20);
	gc8613_write_register(ViPipe, 0x0259, 0x08);
	gc8613_write_register(ViPipe, 0x025a, 0x96);
	gc8613_write_register(ViPipe, 0x0340, 0x08);
	gc8613_write_register(ViPipe, 0x0341, 0xca);
	gc8613_write_register(ViPipe, 0x0351, 0x00);
	gc8613_write_register(ViPipe, 0x0345, 0x02);
	gc8613_write_register(ViPipe, 0x0347, 0x02);
	gc8613_write_register(ViPipe, 0x0348, 0x0f);
	gc8613_write_register(ViPipe, 0x0349, 0x18);
	gc8613_write_register(ViPipe, 0x034a, 0x08);
	gc8613_write_register(ViPipe, 0x034b, 0x88);
	gc8613_write_register(ViPipe, 0x034f, 0xf0);
	gc8613_write_register(ViPipe, 0x0094, 0x0f);
	gc8613_write_register(ViPipe, 0x0095, 0x00);
	gc8613_write_register(ViPipe, 0x0096, 0x08);
	gc8613_write_register(ViPipe, 0x0097, 0x70);
	gc8613_write_register(ViPipe, 0x0099, 0x0c);
	gc8613_write_register(ViPipe, 0x009b, 0x0c);
	gc8613_write_register(ViPipe, 0x060c, 0x06);
	gc8613_write_register(ViPipe, 0x060e, 0x20);
	gc8613_write_register(ViPipe, 0x060f, 0x0f);
	gc8613_write_register(ViPipe, 0x070c, 0x06);
	gc8613_write_register(ViPipe, 0x070e, 0x20);
	gc8613_write_register(ViPipe, 0x070f, 0x0f);
	gc8613_write_register(ViPipe, 0x0087, 0x50);
	gc8613_write_register(ViPipe, 0x141b, 0x03);
	gc8613_write_register(ViPipe, 0x0907, 0xd5);
	gc8613_write_register(ViPipe, 0x0909, 0x06);
	gc8613_write_register(ViPipe, 0x0901, 0x0e);
	gc8613_write_register(ViPipe, 0x0902, 0x0b);
	gc8613_write_register(ViPipe, 0x0904, 0x08);
	gc8613_write_register(ViPipe, 0x0908, 0x09);
	gc8613_write_register(ViPipe, 0x0903, 0xc5);
	gc8613_write_register(ViPipe, 0x090c, 0x09);
	gc8613_write_register(ViPipe, 0x0905, 0x10);
	gc8613_write_register(ViPipe, 0x0906, 0x00);
	gc8613_write_register(ViPipe, 0x072a, 0x7c);
	gc8613_write_register(ViPipe, 0x0724, 0x2b);
	gc8613_write_register(ViPipe, 0x0727, 0x2b);
	gc8613_write_register(ViPipe, 0x072b, 0x1c);
	gc8613_write_register(ViPipe, 0x073e, 0x40);
	gc8613_write_register(ViPipe, 0x0078, 0x88);
	gc8613_write_register(ViPipe, 0x0618, 0x01);
	gc8613_write_register(ViPipe, 0x1466, 0x12);
	gc8613_write_register(ViPipe, 0x1468, 0x07);
	gc8613_write_register(ViPipe, 0x1467, 0x07);
	gc8613_write_register(ViPipe, 0x0709, 0x40);
	gc8613_write_register(ViPipe, 0x0719, 0x40);
	gc8613_write_register(ViPipe, 0x1469, 0x80);
	gc8613_write_register(ViPipe, 0x146a, 0xc0);
	gc8613_write_register(ViPipe, 0x146b, 0x03);
	gc8613_write_register(ViPipe, 0x1480, 0x02);
	gc8613_write_register(ViPipe, 0x1481, 0x80);
	gc8613_write_register(ViPipe, 0x1484, 0x08);
	gc8613_write_register(ViPipe, 0x1485, 0xc0);
	gc8613_write_register(ViPipe, 0x1430, 0x80);
	gc8613_write_register(ViPipe, 0x1407, 0x10);
	gc8613_write_register(ViPipe, 0x1408, 0x16);
	gc8613_write_register(ViPipe, 0x1409, 0x03);
	gc8613_write_register(ViPipe, 0x1434, 0x04);
	gc8613_write_register(ViPipe, 0x1447, 0x75);
	gc8613_write_register(ViPipe, 0x1470, 0x10);
	gc8613_write_register(ViPipe, 0x1471, 0x13);
	gc8613_write_register(ViPipe, 0x1438, 0x00);
	gc8613_write_register(ViPipe, 0x143a, 0x00);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0245, 0xc7);
	gc8613_write_register(ViPipe, 0x025b, 0x07);
	gc8613_write_register(ViPipe, 0x02bb, 0x77);
	gc8613_write_register(ViPipe, 0x0612, 0x01);
	gc8613_write_register(ViPipe, 0x0613, 0x26);
	gc8613_write_register(ViPipe, 0x0243, 0x66);
	gc8613_write_register(ViPipe, 0x0087, 0x53);
	gc8613_write_register(ViPipe, 0x0053, 0x05);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0002, 0xeb);
	gc8613_write_register(ViPipe, 0x005a, 0x0c);
	gc8613_write_register(ViPipe, 0x0040, 0x83);
	gc8613_write_register(ViPipe, 0x0075, 0x54);
	gc8613_write_register(ViPipe, 0x0205, 0x0c);
	gc8613_write_register(ViPipe, 0x0202, 0x01);
	gc8613_write_register(ViPipe, 0x0203, 0x27);
	gc8613_write_register(ViPipe, 0x061a, 0x02);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x0106, 0x78);
	gc8613_write_register(ViPipe, 0x0136, 0x00);
	gc8613_write_register(ViPipe, 0x0181, 0xf0);
	gc8613_write_register(ViPipe, 0x0185, 0x01);
	gc8613_write_register(ViPipe, 0x0180, 0x46);
	gc8613_write_register(ViPipe, 0x0106, 0x38);
	gc8613_write_register(ViPipe, 0x010d, 0xc0);
	gc8613_write_register(ViPipe, 0x010e, 0x12);
	gc8613_write_register(ViPipe, 0x0113, 0x02);
	gc8613_write_register(ViPipe, 0x0114, 0x03);
	gc8613_write_register(ViPipe, 0x0115, 0x12);
	gc8613_write_register(ViPipe, 0x0100, 0x09);
	gc8613_write_register(ViPipe, 0x0004, 0x0f);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0054, 0x98);
	gc8613_write_register(ViPipe, 0x0076, 0x01);
	gc8613_write_register(ViPipe, 0x0052, 0x02);
	gc8613_write_register(ViPipe, 0x021a, 0x10);
	gc8613_write_register(ViPipe, 0x0430, 0x10);
	gc8613_write_register(ViPipe, 0x0431, 0x10);
	gc8613_write_register(ViPipe, 0x0432, 0x10);
	gc8613_write_register(ViPipe, 0x0433, 0x10);
	gc8613_write_register(ViPipe, 0x0434, 0x6d);
	gc8613_write_register(ViPipe, 0x0435, 0x6d);
	gc8613_write_register(ViPipe, 0x0436, 0x6d);
	gc8613_write_register(ViPipe, 0x0437, 0x6d);
	gc8613_write_register(ViPipe, 0x0704, 0x03);
	gc8613_write_register(ViPipe, 0x0706, 0x02);
	gc8613_write_register(ViPipe, 0x0716, 0x02);
	gc8613_write_register(ViPipe, 0x0708, 0xc8);
	gc8613_write_register(ViPipe, 0x0718, 0xc8);
	gc8613_write_register(ViPipe, 0x071d, 0xdc);
	gc8613_write_register(ViPipe, 0x071e, 0x05);
	//otp   autoload
	gc8613_write_register(ViPipe, 0x031f, 0x01);
	gc8613_write_register(ViPipe, 0x031f, 0x00);
	gc8613_write_register(ViPipe, 0x0a67, 0x80);
	gc8613_write_register(ViPipe, 0x0a54, 0x0e);
	gc8613_write_register(ViPipe, 0x0a65, 0x10);
	gc8613_write_register(ViPipe, 0x0a98, 0x04);
	gc8613_write_register(ViPipe, 0x05be, 0x00);
	gc8613_write_register(ViPipe, 0x05a9, 0x01);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0aa0, 0x00);
	gc8613_write_register(ViPipe, 0x0023, 0x00);
	gc8613_write_register(ViPipe, 0x0022, 0x00);
	gc8613_write_register(ViPipe, 0x0025, 0x00);
	gc8613_write_register(ViPipe, 0x0024, 0x00);
	gc8613_write_register(ViPipe, 0x0028, 0x0f);
	gc8613_write_register(ViPipe, 0x0029, 0x18);
	gc8613_write_register(ViPipe, 0x002a, 0x08);
	gc8613_write_register(ViPipe, 0x002b, 0x88);
	gc8613_write_register(ViPipe, 0x0317, 0x1c);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a82, 0x00);
	gc8613_write_register(ViPipe, 0x0a83, 0xe0);
	gc8613_write_register(ViPipe, 0x0a71, 0x00);
	gc8613_write_register(ViPipe, 0x0a72, 0x02);
	gc8613_write_register(ViPipe, 0x0a73, 0x60);
	gc8613_write_register(ViPipe, 0x0a75, 0x41);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a5a, 0x80);
	gc8613_write_register(ViPipe, 0x0129, 0x0a);
	delay_ms(20);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x05be, 0x01);
	gc8613_write_register(ViPipe, 0x0a70, 0x00);
	gc8613_write_register(ViPipe, 0x0080, 0x02);
	gc8613_write_register(ViPipe, 0x0a67, 0x00);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0220, 0x80);
	gc8613_write_register(ViPipe, 0x0058, 0x00);
	gc8613_write_register(ViPipe, 0x0059, 0x04);
	gc8613_default_reg_init(ViPipe);
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===GC8613 8M30 Init success!===\n", ViPipe);
}

static void gc8613_linear_8m25_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	gc8613_write_register(ViPipe, 0x03fe, 0xf0);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x03fe, 0x10);
	gc8613_write_register(ViPipe, 0x0a38, 0x01);
	gc8613_write_register(ViPipe, 0x0a20, 0x19);
	gc8613_write_register(ViPipe, 0x061b, 0x17);
	gc8613_write_register(ViPipe, 0x061c, 0x50);
	gc8613_write_register(ViPipe, 0x061d, 0x06);
	gc8613_write_register(ViPipe, 0x061e, 0x78);
	gc8613_write_register(ViPipe, 0x061f, 0x05);
	gc8613_write_register(ViPipe, 0x0a21, 0x10);
	gc8613_write_register(ViPipe, 0x0a31, 0xdf);
	gc8613_write_register(ViPipe, 0x0a34, 0x40);
	gc8613_write_register(ViPipe, 0x0a35, 0x08);
	gc8613_write_register(ViPipe, 0x0a37, 0x46);
	gc8613_write_register(ViPipe, 0x0314, 0x50);
	gc8613_write_register(ViPipe, 0x0315, 0x00);
	gc8613_write_register(ViPipe, 0x031c, 0xce);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0342, 0x03);
	gc8613_write_register(ViPipe, 0x0343, 0x20);
	gc8613_write_register(ViPipe, 0x0259, 0x08);
	gc8613_write_register(ViPipe, 0x025a, 0x96);
	gc8613_write_register(ViPipe, 0x0340, 0x0A);
	gc8613_write_register(ViPipe, 0x0341, 0x8C);
	gc8613_write_register(ViPipe, 0x0351, 0x00);
	gc8613_write_register(ViPipe, 0x0345, 0x02);
	gc8613_write_register(ViPipe, 0x0347, 0x02);
	gc8613_write_register(ViPipe, 0x0348, 0x0f);
	gc8613_write_register(ViPipe, 0x0349, 0x18);
	gc8613_write_register(ViPipe, 0x034a, 0x08);
	gc8613_write_register(ViPipe, 0x034b, 0x88);
	gc8613_write_register(ViPipe, 0x034f, 0xf0);
	gc8613_write_register(ViPipe, 0x0094, 0x0f);
	gc8613_write_register(ViPipe, 0x0095, 0x00);
	gc8613_write_register(ViPipe, 0x0096, 0x08);
	gc8613_write_register(ViPipe, 0x0097, 0x70);
	gc8613_write_register(ViPipe, 0x0099, 0x0c);
	gc8613_write_register(ViPipe, 0x009b, 0x0c);
	gc8613_write_register(ViPipe, 0x060c, 0x06);
	gc8613_write_register(ViPipe, 0x060e, 0x20);
	gc8613_write_register(ViPipe, 0x060f, 0x0f);
	gc8613_write_register(ViPipe, 0x070c, 0x06);
	gc8613_write_register(ViPipe, 0x070e, 0x20);
	gc8613_write_register(ViPipe, 0x070f, 0x0f);
	gc8613_write_register(ViPipe, 0x0087, 0x50);
	gc8613_write_register(ViPipe, 0x141b, 0x03);
	gc8613_write_register(ViPipe, 0x0907, 0xd5);
	gc8613_write_register(ViPipe, 0x0909, 0x06);
	gc8613_write_register(ViPipe, 0x0901, 0x0e);
	gc8613_write_register(ViPipe, 0x0902, 0x0b);
	gc8613_write_register(ViPipe, 0x0904, 0x08);
	gc8613_write_register(ViPipe, 0x0908, 0x09);
	gc8613_write_register(ViPipe, 0x0903, 0xc5);
	gc8613_write_register(ViPipe, 0x090c, 0x09);
	gc8613_write_register(ViPipe, 0x0905, 0x10);
	gc8613_write_register(ViPipe, 0x0906, 0x00);
	gc8613_write_register(ViPipe, 0x072a, 0x7c);
	gc8613_write_register(ViPipe, 0x0724, 0x2b);
	gc8613_write_register(ViPipe, 0x0727, 0x2b);
	gc8613_write_register(ViPipe, 0x072b, 0x1c);
	gc8613_write_register(ViPipe, 0x073e, 0x40);
	gc8613_write_register(ViPipe, 0x0078, 0x88);
	gc8613_write_register(ViPipe, 0x0618, 0x01);
	gc8613_write_register(ViPipe, 0x1466, 0x12);
	gc8613_write_register(ViPipe, 0x1468, 0x07);
	gc8613_write_register(ViPipe, 0x1467, 0x07);
	gc8613_write_register(ViPipe, 0x0709, 0x40);
	gc8613_write_register(ViPipe, 0x0719, 0x40);
	gc8613_write_register(ViPipe, 0x1469, 0x80);
	gc8613_write_register(ViPipe, 0x146a, 0xc0);
	gc8613_write_register(ViPipe, 0x146b, 0x03);
	gc8613_write_register(ViPipe, 0x1480, 0x02);
	gc8613_write_register(ViPipe, 0x1481, 0x80);
	gc8613_write_register(ViPipe, 0x1484, 0x08);
	gc8613_write_register(ViPipe, 0x1485, 0xc0);
	gc8613_write_register(ViPipe, 0x1430, 0x80);
	gc8613_write_register(ViPipe, 0x1407, 0x10);
	gc8613_write_register(ViPipe, 0x1408, 0x16);
	gc8613_write_register(ViPipe, 0x1409, 0x03);
	gc8613_write_register(ViPipe, 0x1434, 0x04);
	gc8613_write_register(ViPipe, 0x1447, 0x75);
	gc8613_write_register(ViPipe, 0x1470, 0x10);
	gc8613_write_register(ViPipe, 0x1471, 0x13);
	gc8613_write_register(ViPipe, 0x1438, 0x00);
	gc8613_write_register(ViPipe, 0x143a, 0x00);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0245, 0xc7);
	gc8613_write_register(ViPipe, 0x025b, 0x07);
	gc8613_write_register(ViPipe, 0x02bb, 0x77);
	gc8613_write_register(ViPipe, 0x0612, 0x01);
	gc8613_write_register(ViPipe, 0x0613, 0x26);
	gc8613_write_register(ViPipe, 0x0243, 0x66);
	gc8613_write_register(ViPipe, 0x0087, 0x53);
	gc8613_write_register(ViPipe, 0x0053, 0x05);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0002, 0xeb);
	gc8613_write_register(ViPipe, 0x005a, 0x0c);
	gc8613_write_register(ViPipe, 0x0040, 0x83);
	gc8613_write_register(ViPipe, 0x0075, 0x54);
	gc8613_write_register(ViPipe, 0x0205, 0x0c);
	gc8613_write_register(ViPipe, 0x0202, 0x01);
	gc8613_write_register(ViPipe, 0x0203, 0x27);
	gc8613_write_register(ViPipe, 0x061a, 0x02);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x0106, 0x78);
	gc8613_write_register(ViPipe, 0x0136, 0x00);
	gc8613_write_register(ViPipe, 0x0181, 0xf0);
	gc8613_write_register(ViPipe, 0x0185, 0x01);
	gc8613_write_register(ViPipe, 0x0180, 0x46);
	gc8613_write_register(ViPipe, 0x0106, 0x38);
	gc8613_write_register(ViPipe, 0x010d, 0xc0);
	gc8613_write_register(ViPipe, 0x010e, 0x12);
	gc8613_write_register(ViPipe, 0x0113, 0x02);
	gc8613_write_register(ViPipe, 0x0114, 0x03);
	gc8613_write_register(ViPipe, 0x0115, 0x12);
	gc8613_write_register(ViPipe, 0x0100, 0x09);
	gc8613_write_register(ViPipe, 0x0004, 0x0f);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0054, 0x98);
	gc8613_write_register(ViPipe, 0x0076, 0x01);
	gc8613_write_register(ViPipe, 0x0052, 0x02);
	gc8613_write_register(ViPipe, 0x021a, 0x10);
	gc8613_write_register(ViPipe, 0x0430, 0x10);
	gc8613_write_register(ViPipe, 0x0431, 0x10);
	gc8613_write_register(ViPipe, 0x0432, 0x10);
	gc8613_write_register(ViPipe, 0x0433, 0x10);
	gc8613_write_register(ViPipe, 0x0434, 0x6d);
	gc8613_write_register(ViPipe, 0x0435, 0x6d);
	gc8613_write_register(ViPipe, 0x0436, 0x6d);
	gc8613_write_register(ViPipe, 0x0437, 0x6d);
	gc8613_write_register(ViPipe, 0x0704, 0x03);
	gc8613_write_register(ViPipe, 0x0706, 0x02);
	gc8613_write_register(ViPipe, 0x0716, 0x02);
	gc8613_write_register(ViPipe, 0x0708, 0xc8);
	gc8613_write_register(ViPipe, 0x0718, 0xc8);
	gc8613_write_register(ViPipe, 0x071d, 0xdc);
	gc8613_write_register(ViPipe, 0x071e, 0x05);
	//otp   autoload
	gc8613_write_register(ViPipe, 0x031f, 0x01);
	gc8613_write_register(ViPipe, 0x031f, 0x00);
	gc8613_write_register(ViPipe, 0x0a67, 0x80);
	gc8613_write_register(ViPipe, 0x0a54, 0x0e);
	gc8613_write_register(ViPipe, 0x0a65, 0x10);
	gc8613_write_register(ViPipe, 0x0a98, 0x04);
	gc8613_write_register(ViPipe, 0x05be, 0x00);
	gc8613_write_register(ViPipe, 0x05a9, 0x01);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0aa0, 0x00);
	gc8613_write_register(ViPipe, 0x0023, 0x00);
	gc8613_write_register(ViPipe, 0x0022, 0x00);
	gc8613_write_register(ViPipe, 0x0025, 0x00);
	gc8613_write_register(ViPipe, 0x0024, 0x00);
	gc8613_write_register(ViPipe, 0x0028, 0x0f);
	gc8613_write_register(ViPipe, 0x0029, 0x18);
	gc8613_write_register(ViPipe, 0x002a, 0x08);
	gc8613_write_register(ViPipe, 0x002b, 0x88);
	gc8613_write_register(ViPipe, 0x0317, 0x1c);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a82, 0x00);
	gc8613_write_register(ViPipe, 0x0a83, 0xe0);
	gc8613_write_register(ViPipe, 0x0a71, 0x00);
	gc8613_write_register(ViPipe, 0x0a72, 0x02);
	gc8613_write_register(ViPipe, 0x0a73, 0x60);
	gc8613_write_register(ViPipe, 0x0a75, 0x41);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a5a, 0x80);
	gc8613_write_register(ViPipe, 0x0129, 0x0a);
	delay_ms(20);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x05be, 0x01);
	gc8613_write_register(ViPipe, 0x0a70, 0x00);
	gc8613_write_register(ViPipe, 0x0080, 0x02);
	gc8613_write_register(ViPipe, 0x0a67, 0x00);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0220, 0x80);
	gc8613_write_register(ViPipe, 0x0058, 0x00);
	gc8613_write_register(ViPipe, 0x0059, 0x04);
	gc8613_default_reg_init(ViPipe);
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===GC8613 8M25 Init success!===\n", ViPipe);
}

static void gc8613_linear_2m30_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	CVI_TRACE_SNS(CVI_DBG_ERR, "ViPipe:%d,===GC8613 2M30(binning_4l) Init start!===\n", ViPipe);
	gc8613_write_register(ViPipe, 0x03fe, 0xf0);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x03fe, 0x10);
	gc8613_write_register(ViPipe, 0x0a38, 0x01);
	gc8613_write_register(ViPipe, 0x0a20, 0x19);
	gc8613_write_register(ViPipe, 0x061b, 0x17);
	gc8613_write_register(ViPipe, 0x061c, 0x50);
	gc8613_write_register(ViPipe, 0x061d, 0x06);
	gc8613_write_register(ViPipe, 0x061e, 0x78);
	gc8613_write_register(ViPipe, 0x061f, 0x05);
	gc8613_write_register(ViPipe, 0x0a21, 0x10);
	gc8613_write_register(ViPipe, 0x0a31, 0xdf);
	gc8613_write_register(ViPipe, 0x0a34, 0x40);
	gc8613_write_register(ViPipe, 0x0a35, 0x08);
	gc8613_write_register(ViPipe, 0x0a37, 0x46);
	gc8613_write_register(ViPipe, 0x0314, 0x50);
	gc8613_write_register(ViPipe, 0x0315, 0x00);
	gc8613_write_register(ViPipe, 0x031c, 0xce);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0342, 0x03);
	gc8613_write_register(ViPipe, 0x0343, 0x20);
	gc8613_write_register(ViPipe, 0x0259, 0x08);
	gc8613_write_register(ViPipe, 0x025a, 0x96);
	gc8613_write_register(ViPipe, 0x0340, 0x08);
	gc8613_write_register(ViPipe, 0x0341, 0xca);
	gc8613_write_register(ViPipe, 0x0351, 0x00);
	gc8613_write_register(ViPipe, 0x0345, 0x02);
	gc8613_write_register(ViPipe, 0x0347, 0x02);
	gc8613_write_register(ViPipe, 0x0348, 0x0f);
	gc8613_write_register(ViPipe, 0x0349, 0x18);
	gc8613_write_register(ViPipe, 0x034a, 0x08);
	gc8613_write_register(ViPipe, 0x034b, 0x88);
	gc8613_write_register(ViPipe, 0x034f, 0xf0);
	gc8613_write_register(ViPipe, 0x0094, 0x0a);
	gc8613_write_register(ViPipe, 0x0095, 0x00);
	gc8613_write_register(ViPipe, 0x0096, 0x05);
	gc8613_write_register(ViPipe, 0x0097, 0xa0);
	gc8613_write_register(ViPipe, 0x0099, 0x0c);
	gc8613_write_register(ViPipe, 0x009b, 0x0c);
	gc8613_write_register(ViPipe, 0x060c, 0x06);
	gc8613_write_register(ViPipe, 0x060e, 0x20);
	gc8613_write_register(ViPipe, 0x060f, 0x0f);
	gc8613_write_register(ViPipe, 0x070c, 0x06);
	gc8613_write_register(ViPipe, 0x070e, 0x20);
	gc8613_write_register(ViPipe, 0x070f, 0x0f);
	gc8613_write_register(ViPipe, 0x0087, 0x50);
	gc8613_write_register(ViPipe, 0x141b, 0x03);
	gc8613_write_register(ViPipe, 0x0907, 0xd5);
	gc8613_write_register(ViPipe, 0x0909, 0x06);
	gc8613_write_register(ViPipe, 0x0901, 0x0e);
	gc8613_write_register(ViPipe, 0x0902, 0x0b);
	gc8613_write_register(ViPipe, 0x0904, 0x08);
	gc8613_write_register(ViPipe, 0x0908, 0x09);
	gc8613_write_register(ViPipe, 0x0903, 0xc5);
	gc8613_write_register(ViPipe, 0x090c, 0x09);
	gc8613_write_register(ViPipe, 0x0905, 0x10);
	gc8613_write_register(ViPipe, 0x0906, 0x00);
	gc8613_write_register(ViPipe, 0x072a, 0x7c);
	gc8613_write_register(ViPipe, 0x0724, 0x2b);
	gc8613_write_register(ViPipe, 0x0727, 0x2b);
	gc8613_write_register(ViPipe, 0x072b, 0x1c);
	gc8613_write_register(ViPipe, 0x073e, 0x40);
	gc8613_write_register(ViPipe, 0x0078, 0x88);
	gc8613_write_register(ViPipe, 0x0618, 0x01);
	gc8613_write_register(ViPipe, 0x1466, 0x12);
	gc8613_write_register(ViPipe, 0x1468, 0x07);
	gc8613_write_register(ViPipe, 0x1467, 0x07);
	gc8613_write_register(ViPipe, 0x0709, 0x40);
	gc8613_write_register(ViPipe, 0x0719, 0x40);
	gc8613_write_register(ViPipe, 0x1469, 0x80);
	gc8613_write_register(ViPipe, 0x146a, 0xc0);
	gc8613_write_register(ViPipe, 0x146b, 0x03);
	gc8613_write_register(ViPipe, 0x1480, 0x02);
	gc8613_write_register(ViPipe, 0x1481, 0x80);
	gc8613_write_register(ViPipe, 0x1484, 0x08);
	gc8613_write_register(ViPipe, 0x1485, 0xc0);
	gc8613_write_register(ViPipe, 0x1430, 0x80);
	gc8613_write_register(ViPipe, 0x1407, 0x10);
	gc8613_write_register(ViPipe, 0x1408, 0x16);
	gc8613_write_register(ViPipe, 0x1409, 0x03);
	gc8613_write_register(ViPipe, 0x1434, 0x04);
	gc8613_write_register(ViPipe, 0x1447, 0x75);
	gc8613_write_register(ViPipe, 0x1470, 0x10);
	gc8613_write_register(ViPipe, 0x1471, 0x13);
	gc8613_write_register(ViPipe, 0x1438, 0x00);
	gc8613_write_register(ViPipe, 0x143a, 0x00);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0245, 0xc7);
	gc8613_write_register(ViPipe, 0x025b, 0x07);
	gc8613_write_register(ViPipe, 0x02bb, 0x77);
	gc8613_write_register(ViPipe, 0x0612, 0x01);
	gc8613_write_register(ViPipe, 0x0613, 0x26);
	gc8613_write_register(ViPipe, 0x0243, 0x66);
	gc8613_write_register(ViPipe, 0x0087, 0x53);
	gc8613_write_register(ViPipe, 0x0053, 0x05);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0002, 0xeb);
	gc8613_write_register(ViPipe, 0x005a, 0x0c);
	gc8613_write_register(ViPipe, 0x0040, 0x83);
	gc8613_write_register(ViPipe, 0x0075, 0x54);
	gc8613_write_register(ViPipe, 0x0205, 0x0c);
	gc8613_write_register(ViPipe, 0x0202, 0x01);
	gc8613_write_register(ViPipe, 0x0203, 0x27);
	gc8613_write_register(ViPipe, 0x061a, 0x02);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x0106, 0x78);
	gc8613_write_register(ViPipe, 0x0136, 0x00);
	gc8613_write_register(ViPipe, 0x0181, 0xf0);
	gc8613_write_register(ViPipe, 0x0185, 0x01);
	gc8613_write_register(ViPipe, 0x0180, 0x46);
	gc8613_write_register(ViPipe, 0x0106, 0x38);
	gc8613_write_register(ViPipe, 0x010d, 0x80);
	gc8613_write_register(ViPipe, 0x010e, 0x0c);
	gc8613_write_register(ViPipe, 0x0113, 0x02);
	gc8613_write_register(ViPipe, 0x0114, 0x03);
	gc8613_write_register(ViPipe, 0x0100, 0x09);
	gc8613_write_register(ViPipe, 0x0004, 0x0f);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0054, 0x98);
	gc8613_write_register(ViPipe, 0x0076, 0x01);
	gc8613_write_register(ViPipe, 0x0052, 0x02);
	gc8613_write_register(ViPipe, 0x021a, 0x10);
	gc8613_write_register(ViPipe, 0x0430, 0x10);
	gc8613_write_register(ViPipe, 0x0431, 0x10);
	gc8613_write_register(ViPipe, 0x0432, 0x10);
	gc8613_write_register(ViPipe, 0x0433, 0x10);
	gc8613_write_register(ViPipe, 0x0434, 0x6d);
	gc8613_write_register(ViPipe, 0x0435, 0x6d);
	gc8613_write_register(ViPipe, 0x0436, 0x6d);
	gc8613_write_register(ViPipe, 0x0437, 0x6d);
	gc8613_write_register(ViPipe, 0x0704, 0x03);
	gc8613_write_register(ViPipe, 0x0706, 0x02);
	gc8613_write_register(ViPipe, 0x0716, 0x02);
	gc8613_write_register(ViPipe, 0x0708, 0xc8);
	gc8613_write_register(ViPipe, 0x0718, 0xc8);
	gc8613_write_register(ViPipe, 0x071d, 0xdc);
	gc8613_write_register(ViPipe, 0x071e, 0x05);
	//otp   autoload
	gc8613_write_register(ViPipe, 0x031f, 0x01);
	gc8613_write_register(ViPipe, 0x031f, 0x00);
	gc8613_write_register(ViPipe, 0x0a67, 0x80);
	gc8613_write_register(ViPipe, 0x0a54, 0x0e);
	gc8613_write_register(ViPipe, 0x0a65, 0x10);
	gc8613_write_register(ViPipe, 0x0a98, 0x04);
	gc8613_write_register(ViPipe, 0x05be, 0x00);
	gc8613_write_register(ViPipe, 0x05a9, 0x01);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0aa0, 0x00);
	gc8613_write_register(ViPipe, 0x0023, 0x00);
	gc8613_write_register(ViPipe, 0x0022, 0x00);
	gc8613_write_register(ViPipe, 0x0025, 0x00);
	gc8613_write_register(ViPipe, 0x0024, 0x00);
	gc8613_write_register(ViPipe, 0x0028, 0x0f);
	gc8613_write_register(ViPipe, 0x0029, 0x18);
	gc8613_write_register(ViPipe, 0x002a, 0x08);
	gc8613_write_register(ViPipe, 0x002b, 0x88);
	gc8613_write_register(ViPipe, 0x0317, 0x1c);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a82, 0x00);
	gc8613_write_register(ViPipe, 0x0a83, 0xe0);
	gc8613_write_register(ViPipe, 0x0a71, 0x00);
	gc8613_write_register(ViPipe, 0x0a72, 0x02);
	gc8613_write_register(ViPipe, 0x0a73, 0x60);
	gc8613_write_register(ViPipe, 0x0a75, 0x41);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a5a, 0x80);
	delay_ms(20);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0122, 0x05);
	gc8613_write_register(ViPipe, 0x0123, 0x20);
	gc8613_write_register(ViPipe, 0x0124, 0x02);
	gc8613_write_register(ViPipe, 0x0125, 0x20);
	gc8613_write_register(ViPipe, 0x0126, 0x08);
	gc8613_write_register(ViPipe, 0x0121, 0x10);
	gc8613_write_register(ViPipe, 0x0127, 0x10);
	gc8613_write_register(ViPipe, 0x0129, 0x06);
	gc8613_write_register(ViPipe, 0x012a, 0x0a);
	gc8613_write_register(ViPipe, 0x012b, 0x08);
	gc8613_write_register(ViPipe, 0x05be, 0x01);
	gc8613_write_register(ViPipe, 0x0a70, 0x00);
	gc8613_write_register(ViPipe, 0x0080, 0x02);
	gc8613_write_register(ViPipe, 0x0a67, 0x00);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0220, 0x80);
	gc8613_write_register(ViPipe, 0x0058, 0x00);
	gc8613_write_register(ViPipe, 0x0059, 0x04);
	gc8613_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===GC8613 2M30 Init success!===\n", ViPipe);
}

static void gc8613_wdr_8m30_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	// 24M wdr
	gc8613_write_register(ViPipe, 0x03fe, 0xf0);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x03fe, 0x10);
	gc8613_write_register(ViPipe, 0x0a38, 0x01);
	gc8613_write_register(ViPipe, 0x0a20, 0x19);
	gc8613_write_register(ViPipe, 0x061b, 0x17);
	gc8613_write_register(ViPipe, 0x061c, 0x44);
	gc8613_write_register(ViPipe, 0x061d, 0x05);
	gc8613_write_register(ViPipe, 0x061e, 0x75);
	gc8613_write_register(ViPipe, 0x061f, 0x04);
	gc8613_write_register(ViPipe, 0x0a21, 0x10);
	gc8613_write_register(ViPipe, 0x0a30, 0x00);
	gc8613_write_register(ViPipe, 0x0a31, 0xf3);
	gc8613_write_register(ViPipe, 0x0a34, 0x40);
	gc8613_write_register(ViPipe, 0x0a35, 0x08);
	gc8613_write_register(ViPipe, 0x0a37, 0x44);
	gc8613_write_register(ViPipe, 0x0314, 0x50);
	gc8613_write_register(ViPipe, 0x031c, 0xce);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0342, 0x02);
	gc8613_write_register(ViPipe, 0x0343, 0x78);
	gc8613_write_register(ViPipe, 0x0259, 0x08);
	gc8613_write_register(ViPipe, 0x025a, 0x96);
	gc8613_write_register(ViPipe, 0x0340, 0x09);
	gc8613_write_register(ViPipe, 0x0341, 0x0a);
	gc8613_write_register(ViPipe, 0x0351, 0x00);
	gc8613_write_register(ViPipe, 0x0345, 0x02);
	gc8613_write_register(ViPipe, 0x0347, 0x02);
	gc8613_write_register(ViPipe, 0x0348, 0x0f);
	gc8613_write_register(ViPipe, 0x0349, 0x18);
	gc8613_write_register(ViPipe, 0x034a, 0x08);
	gc8613_write_register(ViPipe, 0x034b, 0x88);
	gc8613_write_register(ViPipe, 0x034f, 0xf0);
	gc8613_write_register(ViPipe, 0x0094, 0x0f);
	gc8613_write_register(ViPipe, 0x0095, 0x00);
	gc8613_write_register(ViPipe, 0x0096, 0x08);
	gc8613_write_register(ViPipe, 0x0097, 0x70);
	gc8613_write_register(ViPipe, 0x0099, 0x0c);
	gc8613_write_register(ViPipe, 0x009b, 0x0c);
	gc8613_write_register(ViPipe, 0x060c, 0x06);
	gc8613_write_register(ViPipe, 0x060e, 0x20);
	gc8613_write_register(ViPipe, 0x060f, 0x0f);
	gc8613_write_register(ViPipe, 0x070c, 0x06);
	gc8613_write_register(ViPipe, 0x070e, 0x20);
	gc8613_write_register(ViPipe, 0x070f, 0x0f);
	gc8613_write_register(ViPipe, 0x0087, 0x50);
	gc8613_write_register(ViPipe, 0x141b, 0x03);
	// gc8613_write_register(ViPipe, 0x023b, 0x18); // gain table different
	// gc8613_write_register(ViPipe, 0x0619, 0x00);	// gain table different
	gc8613_write_register(ViPipe, 0x023b, 0x58);	// gain table same
	gc8613_write_register(ViPipe, 0x0619, 0x01);	// gain table same
	gc8613_write_register(ViPipe, 0x0901, 0x0e);
	gc8613_write_register(ViPipe, 0x0907, 0xd5);
	gc8613_write_register(ViPipe, 0x0909, 0x06);
	gc8613_write_register(ViPipe, 0x0902, 0x0b);
	gc8613_write_register(ViPipe, 0x0904, 0x08);
	gc8613_write_register(ViPipe, 0x0908, 0x09);
	gc8613_write_register(ViPipe, 0x0903, 0xc5);
	gc8613_write_register(ViPipe, 0x090c, 0x09);
	gc8613_write_register(ViPipe, 0x0905, 0x10);
	gc8613_write_register(ViPipe, 0x0906, 0x00);
	gc8613_write_register(ViPipe, 0x0724, 0x2b);
	gc8613_write_register(ViPipe, 0x0727, 0x2b);
	gc8613_write_register(ViPipe, 0x072b, 0x1a);
	gc8613_write_register(ViPipe, 0x072a, 0x5e);
	gc8613_write_register(ViPipe, 0x0601, 0x00);
	gc8613_write_register(ViPipe, 0x073e, 0x40);
	gc8613_write_register(ViPipe, 0x0078, 0x88);
	gc8613_write_register(ViPipe, 0x0618, 0x01);
	gc8613_write_register(ViPipe, 0x1466, 0x12);
	gc8613_write_register(ViPipe, 0x1468, 0x10);
	gc8613_write_register(ViPipe, 0x1467, 0x10);
	gc8613_write_register(ViPipe, 0x1478, 0x10);
	gc8613_write_register(ViPipe, 0x1477, 0x10);
	gc8613_write_register(ViPipe, 0x0709, 0x40);
	gc8613_write_register(ViPipe, 0x0719, 0x40);
	gc8613_write_register(ViPipe, 0x1469, 0x80);
	gc8613_write_register(ViPipe, 0x146a, 0x20);
	gc8613_write_register(ViPipe, 0x146b, 0x03);
	gc8613_write_register(ViPipe, 0x1479, 0x80);
	gc8613_write_register(ViPipe, 0x147a, 0x20);
	gc8613_write_register(ViPipe, 0x147b, 0x03);
	gc8613_write_register(ViPipe, 0x1480, 0x02);
	gc8613_write_register(ViPipe, 0x1481, 0x80);
	gc8613_write_register(ViPipe, 0x1482, 0x02);
	gc8613_write_register(ViPipe, 0x1483, 0x80);
	gc8613_write_register(ViPipe, 0x1484, 0x08);
	gc8613_write_register(ViPipe, 0x1485, 0xc0);
	gc8613_write_register(ViPipe, 0x1486, 0x08);
	gc8613_write_register(ViPipe, 0x1487, 0xc0);
	gc8613_write_register(ViPipe, 0x1407, 0x10);
	gc8613_write_register(ViPipe, 0x1408, 0x16);
	gc8613_write_register(ViPipe, 0x1409, 0x03);
	gc8613_write_register(ViPipe, 0x1434, 0x04);
	gc8613_write_register(ViPipe, 0x1447, 0x75);
	gc8613_write_register(ViPipe, 0x140d, 0x04);
	gc8613_write_register(ViPipe, 0x1461, 0x10);
	gc8613_write_register(ViPipe, 0x146c, 0x10);
	gc8613_write_register(ViPipe, 0x146d, 0x10);
	gc8613_write_register(ViPipe, 0x146e, 0x2e);
	gc8613_write_register(ViPipe, 0x146f, 0x30);
	gc8613_write_register(ViPipe, 0x1474, 0x34);
	gc8613_write_register(ViPipe, 0x1470, 0x10);
	gc8613_write_register(ViPipe, 0x1471, 0x13);
	gc8613_write_register(ViPipe, 0x143a, 0x00);
	gc8613_write_register(ViPipe, 0x024a, 0x66);
	gc8613_write_register(ViPipe, 0x024b, 0x02);
	gc8613_write_register(ViPipe, 0x0245, 0xc7);
	gc8613_write_register(ViPipe, 0x025b, 0x07);
	gc8613_write_register(ViPipe, 0x02bb, 0x77);
	gc8613_write_register(ViPipe, 0x0612, 0x01);
	gc8613_write_register(ViPipe, 0x0613, 0x26);
	gc8613_write_register(ViPipe, 0x0243, 0x66);
	gc8613_write_register(ViPipe, 0x0087, 0x53);
	gc8613_write_register(ViPipe, 0x0053, 0x05);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0002, 0xeb);
	gc8613_write_register(ViPipe, 0x005a, 0x0c);
	gc8613_write_register(ViPipe, 0x0040, 0x83);
	gc8613_write_register(ViPipe, 0x0075, 0x58);
	gc8613_write_register(ViPipe, 0x0205, 0x0c);
	gc8613_write_register(ViPipe, 0x0202, 0x06);
	gc8613_write_register(ViPipe, 0x0203, 0x27);
	gc8613_write_register(ViPipe, 0x061a, 0x02);
	gc8613_write_register(ViPipe, 0x0122, 0x12);
	gc8613_write_register(ViPipe, 0x0123, 0x50);
	gc8613_write_register(ViPipe, 0x0126, 0x0f);
	gc8613_write_register(ViPipe, 0x0129, 0x10);
	gc8613_write_register(ViPipe, 0x012a, 0x20);
	gc8613_write_register(ViPipe, 0x012b, 0x10);
	gc8613_write_register(ViPipe, 0x03fe, 0x00);
	gc8613_write_register(ViPipe, 0x0106, 0x78);
	gc8613_write_register(ViPipe, 0x0136, 0x00);
	gc8613_write_register(ViPipe, 0x0181, 0xf0);
	gc8613_write_register(ViPipe, 0x0185, 0x01);
	gc8613_write_register(ViPipe, 0x0180, 0x46);
	gc8613_write_register(ViPipe, 0x0106, 0x38);
	gc8613_write_register(ViPipe, 0x010d, 0xc0);
	gc8613_write_register(ViPipe, 0x010e, 0x12);
	gc8613_write_register(ViPipe, 0x0113, 0x02);
	gc8613_write_register(ViPipe, 0x0114, 0x03);
	gc8613_write_register(ViPipe, 0x0100, 0x09);
	gc8613_write_register(ViPipe, 0x0222, 0x41);
	gc8613_write_register(ViPipe, 0x0107, 0x89);
	gc8613_write_register(ViPipe, 0x023e, 0x00);
	gc8613_write_register(ViPipe, 0x023f, 0x84);
	gc8613_write_register(ViPipe, 0x0220, 0xcf);
	gc8613_write_register(ViPipe, 0x021b, 0x69);
	gc8613_write_register(ViPipe, 0x0004, 0x0f);
	gc8613_write_register(ViPipe, 0x000e, 0x07);
	gc8613_write_register(ViPipe, 0x0219, 0x47);
	gc8613_write_register(ViPipe, 0x0054, 0x98);
	gc8613_write_register(ViPipe, 0x0076, 0x01);
	gc8613_write_register(ViPipe, 0x0052, 0x02);
	gc8613_write_register(ViPipe, 0x021a, 0x10);
	gc8613_write_register(ViPipe, 0x0430, 0x10);
	gc8613_write_register(ViPipe, 0x0431, 0x10);
	gc8613_write_register(ViPipe, 0x0432, 0x10);
	gc8613_write_register(ViPipe, 0x0433, 0x10);
	gc8613_write_register(ViPipe, 0x0434, 0x6d);
	gc8613_write_register(ViPipe, 0x0435, 0x6d);
	gc8613_write_register(ViPipe, 0x0436, 0x6d);
	gc8613_write_register(ViPipe, 0x0437, 0x6d);
	gc8613_write_register(ViPipe, 0x0438, 0x10);
	gc8613_write_register(ViPipe, 0x0439, 0x10);
	gc8613_write_register(ViPipe, 0x043a, 0x10);
	gc8613_write_register(ViPipe, 0x043b, 0x10);
	gc8613_write_register(ViPipe, 0x043c, 0x6d);
	gc8613_write_register(ViPipe, 0x043d, 0x6d);
	gc8613_write_register(ViPipe, 0x043e, 0x6d);
	gc8613_write_register(ViPipe, 0x043f, 0x6d);
	gc8613_write_register(ViPipe, 0x0704, 0x03);
	gc8613_write_register(ViPipe, 0x0706, 0x02);
	gc8613_write_register(ViPipe, 0x0716, 0x02);
	gc8613_write_register(ViPipe, 0x0708, 0xc8);
	gc8613_write_register(ViPipe, 0x0718, 0xc8);
	gc8613_write_register(ViPipe, 0x071d, 0xdc);
	gc8613_write_register(ViPipe, 0x071e, 0x05);
	//otp autoload
	gc8613_write_register(ViPipe, 0x031f, 0x01);
	gc8613_write_register(ViPipe, 0x031f, 0x00);
	gc8613_write_register(ViPipe, 0x0a67, 0x80);
	gc8613_write_register(ViPipe, 0x0a54, 0x0e);
	gc8613_write_register(ViPipe, 0x0a65, 0x10);
	gc8613_write_register(ViPipe, 0x0a98, 0x04);
	gc8613_write_register(ViPipe, 0x05be, 0x00);
	gc8613_write_register(ViPipe, 0x05a9, 0x01);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x0aa0, 0x00);
	gc8613_write_register(ViPipe, 0x0023, 0x00);
	gc8613_write_register(ViPipe, 0x0022, 0x00);
	gc8613_write_register(ViPipe, 0x0025, 0x00);
	gc8613_write_register(ViPipe, 0x0024, 0x00);
	gc8613_write_register(ViPipe, 0x0028, 0x0f);
	gc8613_write_register(ViPipe, 0x0029, 0x18);
	gc8613_write_register(ViPipe, 0x002a, 0x08);
	gc8613_write_register(ViPipe, 0x002b, 0x88);
	gc8613_write_register(ViPipe, 0x0317, 0x1c);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a82, 0x00);
	gc8613_write_register(ViPipe, 0x0a83, 0xe0);
	gc8613_write_register(ViPipe, 0x0a71, 0x00);
	gc8613_write_register(ViPipe, 0x0a72, 0x02);
	gc8613_write_register(ViPipe, 0x0a73, 0x60);
	gc8613_write_register(ViPipe, 0x0a75, 0x41);
	gc8613_write_register(ViPipe, 0x0a70, 0x03);
	gc8613_write_register(ViPipe, 0x0a5a, 0x80);
	delay_ms(20);
	gc8613_write_register(ViPipe, 0x0089, 0x02);
	gc8613_write_register(ViPipe, 0x05be, 0x01);
	gc8613_write_register(ViPipe, 0x0a70, 0x00);
	gc8613_write_register(ViPipe, 0x0080, 0x02);
	gc8613_write_register(ViPipe, 0x0a67, 0x00);
	gc8613_write_register(ViPipe, 0x0020, 0x01);
	gc8613_write_register(ViPipe, 0x0058, 0x00);
	gc8613_write_register(ViPipe, 0x0059, 0x04);
	gc8613_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_ERR, "===Gc8613 sensor 8M30 2to1 WDR (24M) init success!===\n");
}
