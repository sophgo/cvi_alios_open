#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "ov9282_cmos_ex.h"
#include "sensor_i2c.h"

static void ov9282_linear_800p15_10BIT_2L_master_init(VI_PIPE ViPipe);
static void ov9282_linear_800p15_10BIT_2L_slave_init(VI_PIPE ViPipe);

const CVI_U32 ov9282_addr_byte = 2;
const CVI_U32 ov9282_data_byte = 1;
ISP_SNS_MIRRORFLIP_TYPE_E g_aeOv9282_MirrorFip_Initial[VI_MAX_PIPE_NUM] = {
	ISP_SNS_MIRROR, ISP_SNS_MIRROR, ISP_SNS_MIRROR, ISP_SNS_MIRROR};

int ov9282_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunOv9282_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv9282_AddrInfo[ViPipe].s8I2cAddr);
}

int ov9282_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunOv9282_BusInfo[ViPipe].s8I2cDev);
}

int ov9282_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunOv9282_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv9282_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							ov9282_addr_byte, ov9282_data_byte);
}

int ov9282_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunOv9282_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv9282_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							ov9282_addr_byte, (CVI_U32)data, ov9282_data_byte);
}
/*
 *static void delay_ms(int ms)
 *{
 *	usleep(ms * 1000);
 *}
 */
void ov9282_standby(VI_PIPE ViPipe)
{
	ov9282_write_register(ViPipe, 0x0100, 0x00); /* STANDBY */
}

void ov9282_restart(VI_PIPE ViPipe)
{
	ov9282_write_register(ViPipe, 0x0100, 0x01); /* resume */
}

void ov9282_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;
	CVI_U32 start = 1;
	CVI_U32 end = g_pastOv9282[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum - 3;

	for (i = start; i < end; i++) {
		ov9282_write_register(ViPipe,
				g_pastOv9282[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOv9282[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		CVI_TRACE_SNS(CVI_DBG_INFO, "i2c_addr:%#x, i2c_data:%#x\n",
			g_pastOv9282[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
			g_pastOv9282[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

static ISP_SNS_MIRRORFLIP_TYPE_E aeSnsMirrorFlipMap[ISP_SNS_BUTT][ISP_SNS_BUTT] = {
	{ISP_SNS_NORMAL, ISP_SNS_MIRROR, ISP_SNS_FLIP, ISP_SNS_MIRROR_FLIP},
	{ISP_SNS_MIRROR, ISP_SNS_NORMAL, ISP_SNS_MIRROR_FLIP, ISP_SNS_FLIP},
	{ISP_SNS_FLIP, ISP_SNS_MIRROR_FLIP, ISP_SNS_NORMAL, ISP_SNS_MIRROR},
	{ISP_SNS_MIRROR_FLIP, ISP_SNS_FLIP, ISP_SNS_MIRROR, ISP_SNS_NORMAL}
};

#define OV9282_FLIP_ADDR (0x3820)
#define OV9282_MIRROR_ADDR (0x3821)
void ov9282_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val_flip = 0;
	CVI_U8 val_mirror = 0;
	CVI_U32 i = 0;

	for (i = 0; i < ISP_SNS_BUTT; i++) {
		if (g_aeOv9282_MirrorFip_Initial[ViPipe] == aeSnsMirrorFlipMap[i][0]) {
			eSnsMirrorFlip = aeSnsMirrorFlipMap[i][eSnsMirrorFlip];
			break;
		}
	}

	val_flip = ov9282_read_register(ViPipe, OV9282_FLIP_ADDR);
	val_mirror = ov9282_read_register(ViPipe, OV9282_MIRROR_ADDR);
	val_flip &= 0x1011;
	val_mirror &= 0x1011;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val_mirror |= 0x1<<2;
		break;
	case ISP_SNS_FLIP:
		val_flip |= 0x1<<2;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val_flip |= 0x1<<2;
		val_mirror |= 0x1<<2;
		break;
	default:
		return;
	}

	ov9282_standby(ViPipe);
	ov9282_write_register(ViPipe, OV9282_FLIP_ADDR, val_flip);
	ov9282_write_register(ViPipe, OV9282_MIRROR_ADDR, val_mirror);
	usleep(1000*100);
	ov9282_restart(ViPipe);
}

#define OV9282_CHIP_ID_ADDR_H		0x300A
#define OV9282_CHIP_ID_ADDR_L		0x300B
#define OV9282_CHIP_ID				0x9281

int ov9282_probe(VI_PIPE ViPipe)
{
	int nVal, nVal2;

	usleep(500);
	if (ov9282_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = ov9282_read_register(ViPipe, OV9282_CHIP_ID_ADDR_H);
	nVal2 = ov9282_read_register(ViPipe, OV9282_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != OV9282_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void ov9282_init(VI_PIPE ViPipe)
{
	WDR_MODE_E		enWDRMode;
	CVI_U8			u8ImgMode;

	enWDRMode   = g_pastOv9282[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastOv9282[ViPipe]->u8ImgMode;

	ov9282_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Current not support WDR mode\n");
	} else {
		if (u8ImgMode == OV9282_MODE_800P15_2L_MASTER_10BIT) {
			ov9282_linear_800p15_10BIT_2L_master_init(ViPipe);
		} else if (u8ImgMode == OV9282_MODE_800P15_2L_SLAVE_10BIT) {
			ov9282_linear_800p15_10BIT_2L_slave_init(ViPipe);
		}
	}
	g_pastOv9282[ViPipe]->bInit = CVI_TRUE;
}

static void ov9282_linear_800p15_10BIT_2L_master_init(VI_PIPE ViPipe)
{
	ov9282_write_register(ViPipe, 0x0103, 0x01);
	ov9282_write_register(ViPipe, 0x0106, 0x00);
	ov9282_write_register(ViPipe, 0x0302, 0x32);
	ov9282_write_register(ViPipe, 0x030d, 0x50);
	ov9282_write_register(ViPipe, 0x030e, 0x02);
	ov9282_write_register(ViPipe, 0x3001, 0x00);
	ov9282_write_register(ViPipe, 0x3004, 0x00);
	ov9282_write_register(ViPipe, 0x3005, 0x00);
	ov9282_write_register(ViPipe, 0x3006, 0x02); // enable FSIN output
	ov9282_write_register(ViPipe, 0x3823, 0x00);
	ov9282_write_register(ViPipe, 0x3011, 0x0a);
	ov9282_write_register(ViPipe, 0x3013, 0x18);
	ov9282_write_register(ViPipe, 0x301c, 0xf0);
	ov9282_write_register(ViPipe, 0x3022, 0x01);
	ov9282_write_register(ViPipe, 0x3030, 0x10);
	ov9282_write_register(ViPipe, 0x3039, 0x32);
	ov9282_write_register(ViPipe, 0x303a, 0x00);
	ov9282_write_register(ViPipe, 0x3500, 0x00);
	ov9282_write_register(ViPipe, 0x3501, 0x2a);
	ov9282_write_register(ViPipe, 0x3502, 0x90);
	ov9282_write_register(ViPipe, 0x3503, 0x08);
	ov9282_write_register(ViPipe, 0x3505, 0x8c);
	ov9282_write_register(ViPipe, 0x3507, 0x03);
	ov9282_write_register(ViPipe, 0x3508, 0x00);
	ov9282_write_register(ViPipe, 0x3509, 0x10);
	ov9282_write_register(ViPipe, 0x3610, 0x80);
	ov9282_write_register(ViPipe, 0x3611, 0xa0);
	ov9282_write_register(ViPipe, 0x3620, 0x6e);
	ov9282_write_register(ViPipe, 0x3632, 0x56);
	ov9282_write_register(ViPipe, 0x3633, 0x78);
	ov9282_write_register(ViPipe, 0x3662, 0x05);
	ov9282_write_register(ViPipe, 0x3666, 0x00);
	ov9282_write_register(ViPipe, 0x366f, 0x5a);
	ov9282_write_register(ViPipe, 0x3680, 0x84);
	ov9282_write_register(ViPipe, 0x3707, 0x56);
	ov9282_write_register(ViPipe, 0x370d, 0x00);
	ov9282_write_register(ViPipe, 0x370e, 0xfa);
	ov9282_write_register(ViPipe, 0x3712, 0x80);
	ov9282_write_register(ViPipe, 0x372d, 0x22);
	ov9282_write_register(ViPipe, 0x3731, 0x80);
	ov9282_write_register(ViPipe, 0x3732, 0x30);
	ov9282_write_register(ViPipe, 0x3778, 0x00);
	ov9282_write_register(ViPipe, 0x377d, 0x22);
	ov9282_write_register(ViPipe, 0x3788, 0x02);
	ov9282_write_register(ViPipe, 0x3789, 0xa4);
	ov9282_write_register(ViPipe, 0x378a, 0x00);
	ov9282_write_register(ViPipe, 0x378b, 0x4a);
	ov9282_write_register(ViPipe, 0x3799, 0x20);
	ov9282_write_register(ViPipe, 0x379c, 0x01);
	ov9282_write_register(ViPipe, 0x3800, 0x00);
	ov9282_write_register(ViPipe, 0x3801, 0x00);
	ov9282_write_register(ViPipe, 0x3802, 0x00);
	ov9282_write_register(ViPipe, 0x3803, 0x00);
	ov9282_write_register(ViPipe, 0x3804, 0x05);
	ov9282_write_register(ViPipe, 0x3805, 0x0f);
	ov9282_write_register(ViPipe, 0x3806, 0x03);
	ov9282_write_register(ViPipe, 0x3807, 0x2f);
	ov9282_write_register(ViPipe, 0x3808, 0x05);
	ov9282_write_register(ViPipe, 0x3809, 0x00);
	ov9282_write_register(ViPipe, 0x380a, 0x03);
	ov9282_write_register(ViPipe, 0x380b, 0x20);
	ov9282_write_register(ViPipe, 0x380c, 0x02);
	ov9282_write_register(ViPipe, 0x380d, 0xd8);
	ov9282_write_register(ViPipe, 0x380e, 0x1c); // 0x03
	ov9282_write_register(ViPipe, 0x380f, 0x9c); // 0x8e
	ov9282_write_register(ViPipe, 0x3810, 0x00);
	ov9282_write_register(ViPipe, 0x3811, 0x08);
	ov9282_write_register(ViPipe, 0x3812, 0x00);
	ov9282_write_register(ViPipe, 0x3813, 0x08);
	ov9282_write_register(ViPipe, 0x3814, 0x11);
	ov9282_write_register(ViPipe, 0x3815, 0x11);
	ov9282_write_register(ViPipe, 0x3820, 0x40);
	ov9282_write_register(ViPipe, 0x3821, 0x00);
	ov9282_write_register(ViPipe, 0x382b, 0x3a);
	ov9282_write_register(ViPipe, 0x382c, 0x06);
	ov9282_write_register(ViPipe, 0x382d, 0xc2);
	ov9282_write_register(ViPipe, 0x389d, 0x00);
	ov9282_write_register(ViPipe, 0x3881, 0x42);
	ov9282_write_register(ViPipe, 0x3882, 0x02);
	ov9282_write_register(ViPipe, 0x3883, 0x12);
	ov9282_write_register(ViPipe, 0x3885, 0x07);
	ov9282_write_register(ViPipe, 0x38a8, 0x02);
	ov9282_write_register(ViPipe, 0x38a9, 0x80);
	ov9282_write_register(ViPipe, 0x38b1, 0x03);
	ov9282_write_register(ViPipe, 0x38b3, 0x07);
	ov9282_write_register(ViPipe, 0x38c4, 0x00);
	ov9282_write_register(ViPipe, 0x38c5, 0xc0);
	ov9282_write_register(ViPipe, 0x38c6, 0x04);
	ov9282_write_register(ViPipe, 0x38c7, 0x80);
	ov9282_write_register(ViPipe, 0x3920, 0xff);
	ov9282_write_register(ViPipe, 0x4003, 0x40);
	ov9282_write_register(ViPipe, 0x4008, 0x04);
	ov9282_write_register(ViPipe, 0x4009, 0x0b);
	ov9282_write_register(ViPipe, 0x400c, 0x01);
	ov9282_write_register(ViPipe, 0x400d, 0x07);
	ov9282_write_register(ViPipe, 0x4010, 0xf0);
	ov9282_write_register(ViPipe, 0x4011, 0x3b);
	ov9282_write_register(ViPipe, 0x4043, 0x40);
	ov9282_write_register(ViPipe, 0x4307, 0x30);
	ov9282_write_register(ViPipe, 0x4317, 0x00);
	ov9282_write_register(ViPipe, 0x4501, 0x00);
	ov9282_write_register(ViPipe, 0x4507, 0x00);
	ov9282_write_register(ViPipe, 0x4509, 0x00);
	ov9282_write_register(ViPipe, 0x450a, 0x08);
	ov9282_write_register(ViPipe, 0x4601, 0x04);
	ov9282_write_register(ViPipe, 0x470f, 0x00);
	ov9282_write_register(ViPipe, 0x4f07, 0x00);
	ov9282_write_register(ViPipe, 0x4800, 0x00);
	ov9282_write_register(ViPipe, 0x4837, 0x14);
	ov9282_write_register(ViPipe, 0x5000, 0x9f);
	ov9282_write_register(ViPipe, 0x5001, 0x00);
	ov9282_write_register(ViPipe, 0x5e00, 0x00);
	ov9282_write_register(ViPipe, 0x5d00, 0x07);
	ov9282_write_register(ViPipe, 0x5d01, 0x00);
	ov9282_write_register(ViPipe, 0x4f00, 0x0c);
	ov9282_write_register(ViPipe, 0x4f10, 0x00);
	ov9282_write_register(ViPipe, 0x4f11, 0x88);
	ov9282_write_register(ViPipe, 0x4f12, 0x0f);
	ov9282_write_register(ViPipe, 0x4f13, 0xc4);

	// Exposure setting
	// ov9282_write_register(ViPipe, 0x3501, 0x37);
	// ov9282_write_register(ViPipe, 0x3502, 0x50);

	ov9282_default_reg_init(ViPipe);
	ov9282_write_register(ViPipe, 0x0100, 0x01);
	usleep(200 * 1000);

	printf("ViPipe:%d,===OV9282 800P 15fps 10bit 2L LINE MASTER Init OK!===\n", ViPipe);
}

static void ov9282_linear_800p15_10BIT_2L_slave_init(VI_PIPE ViPipe)
{
	ov9282_write_register(ViPipe, 0x0103, 0x01);
	ov9282_write_register(ViPipe, 0x0106, 0x00);
	ov9282_write_register(ViPipe, 0x0302, 0x32);
	ov9282_write_register(ViPipe, 0x030d, 0x50);
	ov9282_write_register(ViPipe, 0x030e, 0x02);
	ov9282_write_register(ViPipe, 0x3001, 0x00);
	ov9282_write_register(ViPipe, 0x3004, 0x00);
	ov9282_write_register(ViPipe, 0x3005, 0x00);
	ov9282_write_register(ViPipe, 0x3006, 0x04);
	ov9282_write_register(ViPipe, 0x3011, 0x0a);
	ov9282_write_register(ViPipe, 0x3013, 0x18);
	ov9282_write_register(ViPipe, 0x301c, 0xf0);
	ov9282_write_register(ViPipe, 0x3022, 0x01);
	ov9282_write_register(ViPipe, 0x3030, 0x10);
	ov9282_write_register(ViPipe, 0x3039, 0x32);
	ov9282_write_register(ViPipe, 0x303a, 0x00);
	ov9282_write_register(ViPipe, 0x3500, 0x00);
	ov9282_write_register(ViPipe, 0x3501, 0x2a);
	ov9282_write_register(ViPipe, 0x3502, 0x90);
	ov9282_write_register(ViPipe, 0x3503, 0x08);
	ov9282_write_register(ViPipe, 0x3505, 0x8c);
	ov9282_write_register(ViPipe, 0x3507, 0x03);
	ov9282_write_register(ViPipe, 0x3508, 0x00);
	ov9282_write_register(ViPipe, 0x3509, 0x10);
	ov9282_write_register(ViPipe, 0x3610, 0x80);
	ov9282_write_register(ViPipe, 0x3611, 0xa0);
	ov9282_write_register(ViPipe, 0x3620, 0x6e);
	ov9282_write_register(ViPipe, 0x3632, 0x56);
	ov9282_write_register(ViPipe, 0x3633, 0x78);
	ov9282_write_register(ViPipe, 0x3662, 0x05);
	ov9282_write_register(ViPipe, 0x3666, 0x00);
	ov9282_write_register(ViPipe, 0x366f, 0x5a);
	ov9282_write_register(ViPipe, 0x3680, 0x84);
	ov9282_write_register(ViPipe, 0x3707, 0x56);
	ov9282_write_register(ViPipe, 0x370d, 0x00);
	ov9282_write_register(ViPipe, 0x370e, 0xfa);
	ov9282_write_register(ViPipe, 0x3712, 0x80);
	ov9282_write_register(ViPipe, 0x372d, 0x22);
	ov9282_write_register(ViPipe, 0x3731, 0x80);
	ov9282_write_register(ViPipe, 0x3732, 0x30);
	ov9282_write_register(ViPipe, 0x3778, 0x00);
	ov9282_write_register(ViPipe, 0x377d, 0x22);
	ov9282_write_register(ViPipe, 0x3788, 0x02);
	ov9282_write_register(ViPipe, 0x3789, 0xa4);
	ov9282_write_register(ViPipe, 0x378a, 0x00);
	ov9282_write_register(ViPipe, 0x378b, 0x4a);
	ov9282_write_register(ViPipe, 0x3799, 0x20);
	ov9282_write_register(ViPipe, 0x379c, 0x01);
	ov9282_write_register(ViPipe, 0x3800, 0x00);
	ov9282_write_register(ViPipe, 0x3801, 0x00);
	ov9282_write_register(ViPipe, 0x3802, 0x00);
	ov9282_write_register(ViPipe, 0x3803, 0x00);
	ov9282_write_register(ViPipe, 0x3804, 0x05);
	ov9282_write_register(ViPipe, 0x3805, 0x0f);
	ov9282_write_register(ViPipe, 0x3806, 0x03);
	ov9282_write_register(ViPipe, 0x3807, 0x2f);
	ov9282_write_register(ViPipe, 0x3808, 0x05);
	ov9282_write_register(ViPipe, 0x3809, 0x00);
	ov9282_write_register(ViPipe, 0x380a, 0x03);
	ov9282_write_register(ViPipe, 0x380b, 0x20);
	ov9282_write_register(ViPipe, 0x380c, 0x02);
	ov9282_write_register(ViPipe, 0x380d, 0xd8);
	ov9282_write_register(ViPipe, 0x380e, 0x1c); // 0x03
	ov9282_write_register(ViPipe, 0x380f, 0x9c); // 0x8e
	ov9282_write_register(ViPipe, 0x3810, 0x00);
	ov9282_write_register(ViPipe, 0x3811, 0x08);
	ov9282_write_register(ViPipe, 0x3812, 0x00);
	ov9282_write_register(ViPipe, 0x3813, 0x08);
	ov9282_write_register(ViPipe, 0x3814, 0x11);
	ov9282_write_register(ViPipe, 0x3815, 0x11);
	ov9282_write_register(ViPipe, 0x3820, 0x40);
	ov9282_write_register(ViPipe, 0x3821, 0x00);
	ov9282_write_register(ViPipe, 0x382b, 0x3a);
	ov9282_write_register(ViPipe, 0x382c, 0x06);
	ov9282_write_register(ViPipe, 0x382d, 0xc2);
	ov9282_write_register(ViPipe, 0x389d, 0x00);
	ov9282_write_register(ViPipe, 0x3881, 0x42);
	ov9282_write_register(ViPipe, 0x3882, 0x02);
	ov9282_write_register(ViPipe, 0x3883, 0x12);
	ov9282_write_register(ViPipe, 0x3885, 0x07);
	ov9282_write_register(ViPipe, 0x38a8, 0x02);
	ov9282_write_register(ViPipe, 0x38a9, 0x80);
	ov9282_write_register(ViPipe, 0x38b1, 0x03);
	ov9282_write_register(ViPipe, 0x38b3, 0x07);
	ov9282_write_register(ViPipe, 0x38c4, 0x00);
	ov9282_write_register(ViPipe, 0x38c5, 0xc0);
	ov9282_write_register(ViPipe, 0x38c6, 0x04);
	ov9282_write_register(ViPipe, 0x38c7, 0x80);
	ov9282_write_register(ViPipe, 0x3920, 0xff);
	ov9282_write_register(ViPipe, 0x4003, 0x40);
	ov9282_write_register(ViPipe, 0x4008, 0x04);
	ov9282_write_register(ViPipe, 0x4009, 0x0b);
	ov9282_write_register(ViPipe, 0x400c, 0x01);
	ov9282_write_register(ViPipe, 0x400d, 0x07);
	ov9282_write_register(ViPipe, 0x4010, 0xf0);
	ov9282_write_register(ViPipe, 0x4011, 0x3b);
	ov9282_write_register(ViPipe, 0x4043, 0x40);
	ov9282_write_register(ViPipe, 0x4307, 0x30);
	ov9282_write_register(ViPipe, 0x4317, 0x00);
	ov9282_write_register(ViPipe, 0x4501, 0x00);
	ov9282_write_register(ViPipe, 0x4507, 0x00);
	ov9282_write_register(ViPipe, 0x4509, 0x00);
	ov9282_write_register(ViPipe, 0x450a, 0x08);
	ov9282_write_register(ViPipe, 0x4601, 0x04);
	ov9282_write_register(ViPipe, 0x470f, 0x00);
	ov9282_write_register(ViPipe, 0x4f07, 0x00);
	ov9282_write_register(ViPipe, 0x4800, 0x00);
	ov9282_write_register(ViPipe, 0x4837, 0x14);
	ov9282_write_register(ViPipe, 0x5000, 0x9f);
	ov9282_write_register(ViPipe, 0x5001, 0x00);
	ov9282_write_register(ViPipe, 0x5e00, 0x00);
	ov9282_write_register(ViPipe, 0x5d00, 0x07);
	ov9282_write_register(ViPipe, 0x5d01, 0x00);
	ov9282_write_register(ViPipe, 0x4f00, 0x0c);
	ov9282_write_register(ViPipe, 0x4f10, 0x00);
	ov9282_write_register(ViPipe, 0x4f11, 0x88);
	ov9282_write_register(ViPipe, 0x4f12, 0x0f);
	ov9282_write_register(ViPipe, 0x4f13, 0xc4);

	// Exposure setting
	// ov9282_write_register(ViPipe, 0x3501, 0x37);
	// ov9282_write_register(ViPipe, 0x3502, 0x50);

	// FSIN slave configuration
	ov9282_write_register(ViPipe, 0x3006, 0x04); // disable FSIN output
	ov9282_write_register(ViPipe, 0x3666, 0x00); // [3:0] fsin_i input
	ov9282_write_register(ViPipe, 0x38b3, 0x07);
	ov9282_write_register(ViPipe, 0x3885, 0x07);
	ov9282_write_register(ViPipe, 0x382b, 0x3a);
	ov9282_write_register(ViPipe, 0x3670, 0x68);
	ov9282_write_register(ViPipe, 0x3823, 0x30); // [5] ext_vs_en [4] r_init_man_en
	ov9282_write_register(ViPipe, 0x3824, 0x00); // sclk -> cs counter reset value at vs_ext
	ov9282_write_register(ViPipe, 0x3825, 0x08); // set it to 8
	ov9282_write_register(ViPipe, 0x3826, 0x1c); // sclk -> r counter reset value at vs_ext
	ov9282_write_register(ViPipe, 0x3827, 0x98); // vts = 'h38e as default, set r counter to vts-4
	ov9282_write_register(ViPipe, 0x3740, 0x01); // daclk -> r_dacs_m_st_en = r40[0]
	ov9282_write_register(ViPipe, 0x3741, 0x00); // daclk -> r_dacs_ini_man = {r41,r42}
	ov9282_write_register(ViPipe, 0x3742, 0x08); // daclk -> cs counter reset value at vs_ext

	ov9282_default_reg_init(ViPipe);
	ov9282_write_register(ViPipe, 0x0100, 0x01);
	usleep(200 * 1000);

	printf("ViPipe:%d,===OV9282 800P 15fps 10bit 2L LINE SLAVE Init OK!===\n", ViPipe);
}