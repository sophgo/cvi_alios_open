#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "ov2736_cmos_ex.h"
#include "sensor_i2c.h"

static void ov2736_linear_1080p30_init(VI_PIPE ViPipe);
static void ov2736_wdr_1080p30_init(VI_PIPE ViPipe);

CVI_U8 ov2736_i2c_addr = 0x10;		/* I2C Address of OV2736 */
const CVI_U32 ov2736_addr_byte = 2;
const CVI_U32 ov2736_data_byte = 1;

int ov2736_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunOv2736_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv2736_AddrInfo[ViPipe].s8I2cAddr);
}

int ov2736_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunOv2736_BusInfo[ViPipe].s8I2cDev);
}

int ov2736_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunOv2736_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv2736_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							ov2736_addr_byte, ov2736_data_byte);
}

int ov2736_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunOv2736_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOv2736_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							ov2736_addr_byte, (CVI_U32)data, ov2736_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void ov2736_standby(VI_PIPE ViPipe)
{
	ov2736_write_register(ViPipe, 0x0100, 0x00); /* standby */
}

void ov2736_restart(VI_PIPE ViPipe)
{
	ov2736_write_register(ViPipe, 0x0100, 0x01); /* restart */
}

void ov2736_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastOv2736[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastOv2736[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			ov2736_write_register(ViPipe,
				g_pastOv2736[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOv2736[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void ov2736_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 orien;

	orien = ov2736_read_register(ViPipe, 0x3820);

	orien &= ~(0x3 << 3);

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		orien |= 0x1 << 3;
		break;
	case ISP_SNS_FLIP:
		orien |= 0x1 << 4;
		break;
	case ISP_SNS_MIRROR_FLIP:
		orien |= 0x3 << 3;
		break;
	default:
		return;
	}

	ov2736_write_register(ViPipe, 0x3820, orien);
}

#define OV2736_CHIP_ID_ADDR_H		0x300A
#define OV2736_CHIP_ID_ADDR_M		0x300B
#define OV2736_CHIP_ID_ADDR_L		0x300C
#define OV2736_CHIP_ID			0x2732

int ov2736_probe(VI_PIPE ViPipe)
{
	int nVal, nVal2, nVal3;

	usleep(500);
	if (ov2736_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = ov2736_read_register(ViPipe, OV2736_CHIP_ID_ADDR_H);
	nVal2 = ov2736_read_register(ViPipe, OV2736_CHIP_ID_ADDR_M);
	nVal3 = ov2736_read_register(ViPipe, OV2736_CHIP_ID_ADDR_L);

	if (nVal < 0 || nVal2 < 0 || nVal3 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 16) | ((nVal2 & 0xFF) << 8) | ((nVal3 & 0xFF) << 0)) != OV2736_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void ov2736_init(VI_PIPE ViPipe)
{
	WDR_MODE_E	enWDRMode;

	enWDRMode = g_pastOv2736[ViPipe]->enWDRMode;

	ov2736_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		ov2736_wdr_1080p30_init(ViPipe);
	} else {
		ov2736_linear_1080p30_init(ViPipe);
	}

	g_pastOv2736[ViPipe]->bInit = CVI_TRUE;
}

void ov2736_exit(VI_PIPE ViPipe)
{
	ov2736_i2c_exit(ViPipe);
}

/* 1080P30 */
static void ov2736_linear_1080p30_init(VI_PIPE ViPipe)
{
	ov2736_write_register(ViPipe, 0x0103, 0x01);
	ov2736_write_register(ViPipe, 0x0305, 0x3c);
	ov2736_write_register(ViPipe, 0x0307, 0x00);
	ov2736_write_register(ViPipe, 0x0308, 0x03);
	ov2736_write_register(ViPipe, 0x0309, 0x03);
	ov2736_write_register(ViPipe, 0x0327, 0x07);
	ov2736_write_register(ViPipe, 0x3016, 0x32);
	ov2736_write_register(ViPipe, 0x3000, 0x00);
	ov2736_write_register(ViPipe, 0x3001, 0x00);
	ov2736_write_register(ViPipe, 0x3002, 0x00);
	ov2736_write_register(ViPipe, 0x3013, 0x00);
	ov2736_write_register(ViPipe, 0x301f, 0xf0);
	ov2736_write_register(ViPipe, 0x3023, 0xf0);
	ov2736_write_register(ViPipe, 0x3020, 0x9b);
	ov2736_write_register(ViPipe, 0x3022, 0x51);
	ov2736_write_register(ViPipe, 0x3106, 0x11);
	ov2736_write_register(ViPipe, 0x3107, 0x01);
	ov2736_write_register(ViPipe, 0x3500, 0x00);
	ov2736_write_register(ViPipe, 0x3501, 0x40);
	ov2736_write_register(ViPipe, 0x3502, 0x00);
	ov2736_write_register(ViPipe, 0x3503, 0x88);
	ov2736_write_register(ViPipe, 0x3505, 0x83);
	ov2736_write_register(ViPipe, 0x3508, 0x01);
	ov2736_write_register(ViPipe, 0x3509, 0x80);
	ov2736_write_register(ViPipe, 0x350a, 0x04);
	ov2736_write_register(ViPipe, 0x350b, 0x00);
	ov2736_write_register(ViPipe, 0x350c, 0x00);
	ov2736_write_register(ViPipe, 0x350d, 0x80);
	ov2736_write_register(ViPipe, 0x350e, 0x04);
	ov2736_write_register(ViPipe, 0x350f, 0x00);
	ov2736_write_register(ViPipe, 0x3510, 0x00);
	ov2736_write_register(ViPipe, 0x3511, 0x00);
	ov2736_write_register(ViPipe, 0x3512, 0x20);
	ov2736_write_register(ViPipe, 0x3600, 0x55);
	ov2736_write_register(ViPipe, 0x3601, 0x52);
	ov2736_write_register(ViPipe, 0x3612, 0xb5);
	ov2736_write_register(ViPipe, 0x3613, 0xb3);
	ov2736_write_register(ViPipe, 0x3616, 0x83);
	ov2736_write_register(ViPipe, 0x3621, 0x00);
	ov2736_write_register(ViPipe, 0x3624, 0x06);
	ov2736_write_register(ViPipe, 0x3642, 0x88);
	ov2736_write_register(ViPipe, 0x3660, 0x00);
	ov2736_write_register(ViPipe, 0x3661, 0x00);
	ov2736_write_register(ViPipe, 0x366a, 0x64);
	ov2736_write_register(ViPipe, 0x366c, 0x00);
	ov2736_write_register(ViPipe, 0x366e, 0xff);
	ov2736_write_register(ViPipe, 0x366f, 0xff);
	ov2736_write_register(ViPipe, 0x3677, 0x11);
	ov2736_write_register(ViPipe, 0x3678, 0x11);
	ov2736_write_register(ViPipe, 0x3679, 0x0c);
	ov2736_write_register(ViPipe, 0x3680, 0xff);
	ov2736_write_register(ViPipe, 0x3681, 0x16);
	ov2736_write_register(ViPipe, 0x3682, 0x16);
	ov2736_write_register(ViPipe, 0x3683, 0x90);
	ov2736_write_register(ViPipe, 0x3684, 0x90);
	ov2736_write_register(ViPipe, 0x3768, 0x04);
	ov2736_write_register(ViPipe, 0x3769, 0x20);
	ov2736_write_register(ViPipe, 0x376a, 0x04);
	ov2736_write_register(ViPipe, 0x376b, 0x20);
	ov2736_write_register(ViPipe, 0x3620, 0x80);
	ov2736_write_register(ViPipe, 0x3662, 0x10);
	ov2736_write_register(ViPipe, 0x3663, 0x24);
	ov2736_write_register(ViPipe, 0x3665, 0xa0);
	ov2736_write_register(ViPipe, 0x3667, 0xa6);
	ov2736_write_register(ViPipe, 0x3674, 0x01);
	ov2736_write_register(ViPipe, 0x373d, 0x24);
	ov2736_write_register(ViPipe, 0x3741, 0x28);
	ov2736_write_register(ViPipe, 0x3743, 0x28);
	ov2736_write_register(ViPipe, 0x3745, 0x28);
	ov2736_write_register(ViPipe, 0x3747, 0x28);
	ov2736_write_register(ViPipe, 0x3748, 0x00);
	ov2736_write_register(ViPipe, 0x3749, 0x78);
	ov2736_write_register(ViPipe, 0x374a, 0x00);
	ov2736_write_register(ViPipe, 0x374b, 0x78);
	ov2736_write_register(ViPipe, 0x374c, 0x00);
	ov2736_write_register(ViPipe, 0x374d, 0x78);
	ov2736_write_register(ViPipe, 0x374e, 0x00);
	ov2736_write_register(ViPipe, 0x374f, 0x78);
	ov2736_write_register(ViPipe, 0x3766, 0x12);
	ov2736_write_register(ViPipe, 0x37e0, 0x00);
	ov2736_write_register(ViPipe, 0x37e6, 0x04);
	ov2736_write_register(ViPipe, 0x37e5, 0x04);
	ov2736_write_register(ViPipe, 0x37e1, 0x04);
	ov2736_write_register(ViPipe, 0x3737, 0x04);
	ov2736_write_register(ViPipe, 0x37d0, 0x0a);
	ov2736_write_register(ViPipe, 0x37d8, 0x04);
	ov2736_write_register(ViPipe, 0x37e2, 0x08);
	ov2736_write_register(ViPipe, 0x3739, 0x10);
	ov2736_write_register(ViPipe, 0x37e4, 0x18);
	ov2736_write_register(ViPipe, 0x37e3, 0x04);
	ov2736_write_register(ViPipe, 0x37d9, 0x10);
	ov2736_write_register(ViPipe, 0x4040, 0x04);
	ov2736_write_register(ViPipe, 0x4041, 0x0f);
	ov2736_write_register(ViPipe, 0x4008, 0x00);
	ov2736_write_register(ViPipe, 0x4009, 0x0d);
	ov2736_write_register(ViPipe, 0x37a1, 0x14);
	ov2736_write_register(ViPipe, 0x37a8, 0x16);
	ov2736_write_register(ViPipe, 0x37ab, 0x10);
	ov2736_write_register(ViPipe, 0x37c2, 0x04);
	ov2736_write_register(ViPipe, 0x3705, 0x00);
	ov2736_write_register(ViPipe, 0x3706, 0x28);
	ov2736_write_register(ViPipe, 0x370a, 0x00);
	ov2736_write_register(ViPipe, 0x370b, 0x78);
	ov2736_write_register(ViPipe, 0x3714, 0x24);
	ov2736_write_register(ViPipe, 0x371a, 0x1e);
	ov2736_write_register(ViPipe, 0x372a, 0x03);
	ov2736_write_register(ViPipe, 0x3756, 0x00);
	ov2736_write_register(ViPipe, 0x3757, 0x0e);
	ov2736_write_register(ViPipe, 0x377b, 0x00);
	ov2736_write_register(ViPipe, 0x377c, 0x0c);
	ov2736_write_register(ViPipe, 0x377d, 0x20);
	ov2736_write_register(ViPipe, 0x3790, 0x28);
	ov2736_write_register(ViPipe, 0x3791, 0x78);
	ov2736_write_register(ViPipe, 0x3800, 0x00);
	ov2736_write_register(ViPipe, 0x3801, 0x00);
	ov2736_write_register(ViPipe, 0x3802, 0x00);
	ov2736_write_register(ViPipe, 0x3803, 0x04);
	ov2736_write_register(ViPipe, 0x3804, 0x07);
	ov2736_write_register(ViPipe, 0x3805, 0x8f);
	ov2736_write_register(ViPipe, 0x3806, 0x04);
	ov2736_write_register(ViPipe, 0x3807, 0x43);
	ov2736_write_register(ViPipe, 0x3808, 0x07);
	ov2736_write_register(ViPipe, 0x3809, 0x80);
	ov2736_write_register(ViPipe, 0x380a, 0x04);
	ov2736_write_register(ViPipe, 0x380b, 0x38);
	ov2736_write_register(ViPipe, 0x380c, 0x04);
	ov2736_write_register(ViPipe, 0x380d, 0xf0);
	ov2736_write_register(ViPipe, 0x380e, 0x04);
	ov2736_write_register(ViPipe, 0x380f, 0xa0);
	ov2736_write_register(ViPipe, 0x3811, 0x06);
	ov2736_write_register(ViPipe, 0x3813, 0x04);
	ov2736_write_register(ViPipe, 0x3814, 0x01);
	ov2736_write_register(ViPipe, 0x3815, 0x01);
	ov2736_write_register(ViPipe, 0x3816, 0x01);
	ov2736_write_register(ViPipe, 0x3817, 0x01);
	ov2736_write_register(ViPipe, 0x381d, 0x40);
	ov2736_write_register(ViPipe, 0x381e, 0x02);
	ov2736_write_register(ViPipe, 0x3820, 0x88);
	ov2736_write_register(ViPipe, 0x3821, 0x00);
	ov2736_write_register(ViPipe, 0x3822, 0x04);
	ov2736_write_register(ViPipe, 0x3835, 0x00);
	ov2736_write_register(ViPipe, 0x4303, 0x19);
	ov2736_write_register(ViPipe, 0x4304, 0x19);
	ov2736_write_register(ViPipe, 0x4305, 0x03);
	ov2736_write_register(ViPipe, 0x4306, 0x81);
	ov2736_write_register(ViPipe, 0x4503, 0x00);
	ov2736_write_register(ViPipe, 0x4508, 0x14);
	ov2736_write_register(ViPipe, 0x450a, 0x00);
	ov2736_write_register(ViPipe, 0x450b, 0x40);
	ov2736_write_register(ViPipe, 0x4833, 0x08);
	ov2736_write_register(ViPipe, 0x5001, 0x09);
	ov2736_write_register(ViPipe, 0x3b00, 0x00);
	ov2736_write_register(ViPipe, 0x3b02, 0x00);
	ov2736_write_register(ViPipe, 0x3b03, 0x00);
	ov2736_write_register(ViPipe, 0x3c80, 0x08);
	ov2736_write_register(ViPipe, 0x3c82, 0x00);
	ov2736_write_register(ViPipe, 0x3c83, 0xb1);
	ov2736_write_register(ViPipe, 0x3c87, 0x08);
	ov2736_write_register(ViPipe, 0x3c8c, 0x10);
	ov2736_write_register(ViPipe, 0x3c8d, 0x00);
	ov2736_write_register(ViPipe, 0x3c90, 0x00);
	ov2736_write_register(ViPipe, 0x3c91, 0x00);
	ov2736_write_register(ViPipe, 0x3c92, 0x00);
	ov2736_write_register(ViPipe, 0x3c93, 0x00);
	ov2736_write_register(ViPipe, 0x3c94, 0x00);
	ov2736_write_register(ViPipe, 0x3c95, 0x00);
	ov2736_write_register(ViPipe, 0x3c96, 0x00);
	ov2736_write_register(ViPipe, 0x3c97, 0x00);
	ov2736_write_register(ViPipe, 0x3c98, 0x00);
	ov2736_write_register(ViPipe, 0x4000, 0xf3);
	ov2736_write_register(ViPipe, 0x4001, 0x60);
	ov2736_write_register(ViPipe, 0x4002, 0x00);
	ov2736_write_register(ViPipe, 0x4003, 0x40);
	ov2736_write_register(ViPipe, 0x4090, 0x14);
	ov2736_write_register(ViPipe, 0x4601, 0x10);
	ov2736_write_register(ViPipe, 0x4701, 0x00);
	ov2736_write_register(ViPipe, 0x4708, 0x09);
	ov2736_write_register(ViPipe, 0x470a, 0x00);
	ov2736_write_register(ViPipe, 0x470b, 0x40);
	ov2736_write_register(ViPipe, 0x470c, 0x81);
	ov2736_write_register(ViPipe, 0x480c, 0x12);
	ov2736_write_register(ViPipe, 0x4710, 0x06);
	ov2736_write_register(ViPipe, 0x4711, 0x00);
	ov2736_write_register(ViPipe, 0x4837, 0x12);
	ov2736_write_register(ViPipe, 0x4800, 0x00);
	ov2736_write_register(ViPipe, 0x4c01, 0x00);
	ov2736_write_register(ViPipe, 0x5036, 0x00);
	ov2736_write_register(ViPipe, 0x5037, 0x00);
	ov2736_write_register(ViPipe, 0x580b, 0x0f);
	ov2736_write_register(ViPipe, 0x4903, 0x80);
	ov2736_write_register(ViPipe, 0x484b, 0x05);
	ov2736_write_register(ViPipe, 0x400a, 0x00);
	ov2736_write_register(ViPipe, 0x400b, 0x90);
	ov2736_write_register(ViPipe, 0x5000, 0x81);
	ov2736_write_register(ViPipe, 0x5200, 0x18);
	ov2736_write_register(ViPipe, 0x4837, 0x16);
	ov2736_write_register(ViPipe, 0x380e, 0x04);
	ov2736_write_register(ViPipe, 0x380f, 0xa0);
	ov2736_write_register(ViPipe, 0x3500, 0x00);
	ov2736_write_register(ViPipe, 0x3501, 0x49);
	ov2736_write_register(ViPipe, 0x3502, 0x80);
	ov2736_write_register(ViPipe, 0x3508, 0x02);
	ov2736_write_register(ViPipe, 0x3509, 0x80);
	ov2736_write_register(ViPipe, 0x3d8c, 0x11);
	ov2736_write_register(ViPipe, 0x3d8d, 0xf0);
	ov2736_write_register(ViPipe, 0x5180, 0x00);
	ov2736_write_register(ViPipe, 0x5181, 0x10);
	ov2736_write_register(ViPipe, 0x36a0, 0x16);
	ov2736_write_register(ViPipe, 0x36a1, 0x50);
	ov2736_write_register(ViPipe, 0x36a2, 0x60);
	ov2736_write_register(ViPipe, 0x36a3, 0x80);
	ov2736_write_register(ViPipe, 0x36a4, 0x00);
	ov2736_write_register(ViPipe, 0x36a5, 0xa0);
	ov2736_write_register(ViPipe, 0x36a6, 0x00);
	ov2736_write_register(ViPipe, 0x36a7, 0x50);
	ov2736_write_register(ViPipe, 0x36a8, 0x00);
	ov2736_write_register(ViPipe, 0x36a9, 0x50);
	ov2736_write_register(ViPipe, 0x36aa, 0x00);
	ov2736_write_register(ViPipe, 0x36ab, 0x50);
	ov2736_write_register(ViPipe, 0x36ac, 0x00);
	ov2736_write_register(ViPipe, 0x36ad, 0x50);
	ov2736_write_register(ViPipe, 0x36ae, 0x00);
	ov2736_write_register(ViPipe, 0x36af, 0x50);
	ov2736_write_register(ViPipe, 0x36b0, 0x00);
	ov2736_write_register(ViPipe, 0x36b1, 0x50);
	ov2736_write_register(ViPipe, 0x36b2, 0x00);
	ov2736_write_register(ViPipe, 0x36b3, 0x50);
	ov2736_write_register(ViPipe, 0x36b4, 0x00);
	ov2736_write_register(ViPipe, 0x36b5, 0x50);
	ov2736_write_register(ViPipe, 0x36b9, 0xee);
	ov2736_write_register(ViPipe, 0x36ba, 0xee);
	ov2736_write_register(ViPipe, 0x36bb, 0xee);
	ov2736_write_register(ViPipe, 0x36bc, 0xee);
	ov2736_write_register(ViPipe, 0x36bd, 0x0e);
	ov2736_write_register(ViPipe, 0x36b6, 0x08);
	ov2736_write_register(ViPipe, 0x36b7, 0x08);
	ov2736_write_register(ViPipe, 0x36b8, 0x10);

	ov2736_default_reg_init(ViPipe);

	 ov2736_write_register(ViPipe, 0x0100, 0x01);

	delay_ms(100);

	printf("ViPipe:%d,===OV2736 1080P 30fps 12bit LINE Init OK!===\n", ViPipe);
}

static void ov2736_wdr_1080p30_init(VI_PIPE ViPipe)
{
	ov2736_write_register(ViPipe, 0x0103, 0x01);
	ov2736_write_register(ViPipe, 0x0305, 0x3c);
	ov2736_write_register(ViPipe, 0x0307, 0x00);
	ov2736_write_register(ViPipe, 0x0308, 0x03);
	ov2736_write_register(ViPipe, 0x0309, 0x03);
	ov2736_write_register(ViPipe, 0x0327, 0x07);
	ov2736_write_register(ViPipe, 0x3016, 0x32);
	ov2736_write_register(ViPipe, 0x3000, 0x00);
	ov2736_write_register(ViPipe, 0x3001, 0x00);
	ov2736_write_register(ViPipe, 0x3002, 0x00);
	ov2736_write_register(ViPipe, 0x3013, 0x00);
	ov2736_write_register(ViPipe, 0x301f, 0xf0);
	ov2736_write_register(ViPipe, 0x3023, 0xf0);
	ov2736_write_register(ViPipe, 0x3020, 0x9b);
	ov2736_write_register(ViPipe, 0x3022, 0x51);
	ov2736_write_register(ViPipe, 0x3106, 0x11);
	ov2736_write_register(ViPipe, 0x3107, 0x01);
	ov2736_write_register(ViPipe, 0x3500, 0x00);
	ov2736_write_register(ViPipe, 0x3501, 0x40);
	ov2736_write_register(ViPipe, 0x3502, 0x00);
	ov2736_write_register(ViPipe, 0x3503, 0x88);
	ov2736_write_register(ViPipe, 0x3505, 0x83);
	ov2736_write_register(ViPipe, 0x3508, 0x01);
	ov2736_write_register(ViPipe, 0x3509, 0x80);
	ov2736_write_register(ViPipe, 0x350a, 0x04);
	ov2736_write_register(ViPipe, 0x350b, 0x00);
	ov2736_write_register(ViPipe, 0x350c, 0x00);
	ov2736_write_register(ViPipe, 0x350d, 0x80);
	ov2736_write_register(ViPipe, 0x350e, 0x04);
	ov2736_write_register(ViPipe, 0x350f, 0x00);
	ov2736_write_register(ViPipe, 0x3510, 0x00);
	ov2736_write_register(ViPipe, 0x3511, 0x00);
	ov2736_write_register(ViPipe, 0x3512, 0x20);
	ov2736_write_register(ViPipe, 0x3600, 0x55);
	ov2736_write_register(ViPipe, 0x3601, 0x52);
	ov2736_write_register(ViPipe, 0x3612, 0xb5);
	ov2736_write_register(ViPipe, 0x3613, 0xb3);
	ov2736_write_register(ViPipe, 0x3616, 0x83);
	ov2736_write_register(ViPipe, 0x3621, 0x00);
	ov2736_write_register(ViPipe, 0x3624, 0x06);
	ov2736_write_register(ViPipe, 0x3642, 0x88);
	ov2736_write_register(ViPipe, 0x3660, 0x00);
	ov2736_write_register(ViPipe, 0x3661, 0x00);
	ov2736_write_register(ViPipe, 0x366a, 0x64);
	ov2736_write_register(ViPipe, 0x366c, 0x00);
	ov2736_write_register(ViPipe, 0x366e, 0xff);
	ov2736_write_register(ViPipe, 0x366f, 0xff);
	ov2736_write_register(ViPipe, 0x3677, 0x11);
	ov2736_write_register(ViPipe, 0x3678, 0x11);
	ov2736_write_register(ViPipe, 0x3679, 0x0c);
	ov2736_write_register(ViPipe, 0x3680, 0xff);
	ov2736_write_register(ViPipe, 0x3681, 0x16);
	ov2736_write_register(ViPipe, 0x3682, 0x16);
	ov2736_write_register(ViPipe, 0x3683, 0x90);
	ov2736_write_register(ViPipe, 0x3684, 0x90);
	ov2736_write_register(ViPipe, 0x3768, 0x04);
	ov2736_write_register(ViPipe, 0x3769, 0x20);
	ov2736_write_register(ViPipe, 0x376a, 0x04);
	ov2736_write_register(ViPipe, 0x376b, 0x20);
	ov2736_write_register(ViPipe, 0x3620, 0x80);
	ov2736_write_register(ViPipe, 0x3662, 0x10);
	ov2736_write_register(ViPipe, 0x3663, 0x24);
	ov2736_write_register(ViPipe, 0x3665, 0xa0);
	ov2736_write_register(ViPipe, 0x3667, 0xa6);
	ov2736_write_register(ViPipe, 0x3674, 0x01);
	ov2736_write_register(ViPipe, 0x373d, 0x24);
	ov2736_write_register(ViPipe, 0x3741, 0x28);
	ov2736_write_register(ViPipe, 0x3743, 0x28);
	ov2736_write_register(ViPipe, 0x3745, 0x28);
	ov2736_write_register(ViPipe, 0x3747, 0x28);
	ov2736_write_register(ViPipe, 0x3748, 0x00);
	ov2736_write_register(ViPipe, 0x3749, 0x78);
	ov2736_write_register(ViPipe, 0x374a, 0x00);
	ov2736_write_register(ViPipe, 0x374b, 0x78);
	ov2736_write_register(ViPipe, 0x374c, 0x00);
	ov2736_write_register(ViPipe, 0x374d, 0x78);
	ov2736_write_register(ViPipe, 0x374e, 0x00);
	ov2736_write_register(ViPipe, 0x374f, 0x78);
	ov2736_write_register(ViPipe, 0x3766, 0x12);
	ov2736_write_register(ViPipe, 0x37e0, 0x00);
	ov2736_write_register(ViPipe, 0x37e6, 0x04);
	ov2736_write_register(ViPipe, 0x37e5, 0x04);
	ov2736_write_register(ViPipe, 0x37e1, 0x04);
	ov2736_write_register(ViPipe, 0x3737, 0x04);
	ov2736_write_register(ViPipe, 0x37d0, 0x0a);
	ov2736_write_register(ViPipe, 0x37d8, 0x04);
	ov2736_write_register(ViPipe, 0x37e2, 0x08);
	ov2736_write_register(ViPipe, 0x3739, 0x10);
	ov2736_write_register(ViPipe, 0x37e4, 0x18);
	ov2736_write_register(ViPipe, 0x37e3, 0x04);
	ov2736_write_register(ViPipe, 0x37d9, 0x10);
	ov2736_write_register(ViPipe, 0x4040, 0x04);
	ov2736_write_register(ViPipe, 0x4041, 0x0f);
	ov2736_write_register(ViPipe, 0x4008, 0x00);
	ov2736_write_register(ViPipe, 0x4009, 0x0d);
	ov2736_write_register(ViPipe, 0x37a1, 0x14);
	ov2736_write_register(ViPipe, 0x37a8, 0x16);
	ov2736_write_register(ViPipe, 0x37ab, 0x10);
	ov2736_write_register(ViPipe, 0x37c2, 0x04);
	ov2736_write_register(ViPipe, 0x3705, 0x00);
	ov2736_write_register(ViPipe, 0x3706, 0x28);
	ov2736_write_register(ViPipe, 0x370a, 0x00);
	ov2736_write_register(ViPipe, 0x370b, 0x78);
	ov2736_write_register(ViPipe, 0x3714, 0x24);
	ov2736_write_register(ViPipe, 0x371a, 0x1e);
	ov2736_write_register(ViPipe, 0x372a, 0x03);
	ov2736_write_register(ViPipe, 0x3756, 0x00);
	ov2736_write_register(ViPipe, 0x3757, 0x0e);
	ov2736_write_register(ViPipe, 0x377b, 0x00);
	ov2736_write_register(ViPipe, 0x377c, 0x0c);
	ov2736_write_register(ViPipe, 0x377d, 0x20);
	ov2736_write_register(ViPipe, 0x3790, 0x28);
	ov2736_write_register(ViPipe, 0x3791, 0x78);
	ov2736_write_register(ViPipe, 0x3800, 0x00);
	ov2736_write_register(ViPipe, 0x3801, 0x00);
	ov2736_write_register(ViPipe, 0x3802, 0x00);
	ov2736_write_register(ViPipe, 0x3803, 0x04);
	ov2736_write_register(ViPipe, 0x3804, 0x07);
	ov2736_write_register(ViPipe, 0x3805, 0x8f);
	ov2736_write_register(ViPipe, 0x3806, 0x04);
	ov2736_write_register(ViPipe, 0x3807, 0x43);
	ov2736_write_register(ViPipe, 0x3808, 0x07);
	ov2736_write_register(ViPipe, 0x3809, 0x80);
	ov2736_write_register(ViPipe, 0x380a, 0x04);
	ov2736_write_register(ViPipe, 0x380b, 0x38);
	ov2736_write_register(ViPipe, 0x380c, 0x02);
	ov2736_write_register(ViPipe, 0x380d, 0x78);
	ov2736_write_register(ViPipe, 0x380e, 0x04);
	ov2736_write_register(ViPipe, 0x380f, 0xa0);
	ov2736_write_register(ViPipe, 0x3811, 0x06);
	ov2736_write_register(ViPipe, 0x3813, 0x04);
	ov2736_write_register(ViPipe, 0x3814, 0x01);
	ov2736_write_register(ViPipe, 0x3815, 0x01);
	ov2736_write_register(ViPipe, 0x3816, 0x01);
	ov2736_write_register(ViPipe, 0x3817, 0x01);
	ov2736_write_register(ViPipe, 0x381d, 0x40);
	ov2736_write_register(ViPipe, 0x381e, 0x02);
	ov2736_write_register(ViPipe, 0x3820, 0x88);
	ov2736_write_register(ViPipe, 0x3821, 0x00);
	ov2736_write_register(ViPipe, 0x3822, 0x04);
	ov2736_write_register(ViPipe, 0x3835, 0x00);
	ov2736_write_register(ViPipe, 0x4303, 0x19);
	ov2736_write_register(ViPipe, 0x4304, 0x19);
	ov2736_write_register(ViPipe, 0x4305, 0x03);
	ov2736_write_register(ViPipe, 0x4306, 0x81);
	ov2736_write_register(ViPipe, 0x4503, 0x00);
	ov2736_write_register(ViPipe, 0x4508, 0x14);
	ov2736_write_register(ViPipe, 0x450a, 0x00);
	ov2736_write_register(ViPipe, 0x450b, 0x40);
	ov2736_write_register(ViPipe, 0x4833, 0x08);
	ov2736_write_register(ViPipe, 0x5001, 0x09);
	ov2736_write_register(ViPipe, 0x3b00, 0x00);
	ov2736_write_register(ViPipe, 0x3b02, 0x00);
	ov2736_write_register(ViPipe, 0x3b03, 0x00);
	ov2736_write_register(ViPipe, 0x3c80, 0x08);
	ov2736_write_register(ViPipe, 0x3c82, 0x00);
	ov2736_write_register(ViPipe, 0x3c83, 0xb1);
	ov2736_write_register(ViPipe, 0x3c87, 0x08);
	ov2736_write_register(ViPipe, 0x3c8c, 0x10);
	ov2736_write_register(ViPipe, 0x3c8d, 0x00);
	ov2736_write_register(ViPipe, 0x3c90, 0x00);
	ov2736_write_register(ViPipe, 0x3c91, 0x00);
	ov2736_write_register(ViPipe, 0x3c92, 0x00);
	ov2736_write_register(ViPipe, 0x3c93, 0x00);
	ov2736_write_register(ViPipe, 0x3c94, 0x00);
	ov2736_write_register(ViPipe, 0x3c95, 0x00);
	ov2736_write_register(ViPipe, 0x3c96, 0x00);
	ov2736_write_register(ViPipe, 0x3c97, 0x00);
	ov2736_write_register(ViPipe, 0x3c98, 0x00);
	ov2736_write_register(ViPipe, 0x4000, 0xf3);
	ov2736_write_register(ViPipe, 0x4001, 0x60);
	ov2736_write_register(ViPipe, 0x4002, 0x00);
	ov2736_write_register(ViPipe, 0x4003, 0x40);
	ov2736_write_register(ViPipe, 0x4090, 0x14);
	ov2736_write_register(ViPipe, 0x4601, 0x10);
	ov2736_write_register(ViPipe, 0x4701, 0x00);
	ov2736_write_register(ViPipe, 0x4708, 0x09);
	ov2736_write_register(ViPipe, 0x470a, 0x00);
	ov2736_write_register(ViPipe, 0x470b, 0x40);
	ov2736_write_register(ViPipe, 0x470c, 0x81);
	ov2736_write_register(ViPipe, 0x480c, 0x12);
	ov2736_write_register(ViPipe, 0x4710, 0x06);
	ov2736_write_register(ViPipe, 0x4711, 0x00);
	ov2736_write_register(ViPipe, 0x4837, 0x12);
	ov2736_write_register(ViPipe, 0x4800, 0x00);
	ov2736_write_register(ViPipe, 0x4c01, 0x00);
	ov2736_write_register(ViPipe, 0x5036, 0x00);
	ov2736_write_register(ViPipe, 0x5037, 0x00);
	ov2736_write_register(ViPipe, 0x580b, 0x0f);
	ov2736_write_register(ViPipe, 0x4903, 0x80);
	ov2736_write_register(ViPipe, 0x484b, 0x05);
	ov2736_write_register(ViPipe, 0x400a, 0x00);
	ov2736_write_register(ViPipe, 0x400b, 0x90);
	ov2736_write_register(ViPipe, 0x5000, 0x81);
	ov2736_write_register(ViPipe, 0x5200, 0x18);
	ov2736_write_register(ViPipe, 0x4837, 0x16);
	ov2736_write_register(ViPipe, 0x380e, 0x04);
	ov2736_write_register(ViPipe, 0x380f, 0xa0);
	ov2736_write_register(ViPipe, 0x3500, 0x00);
	ov2736_write_register(ViPipe, 0x3501, 0x49);
	ov2736_write_register(ViPipe, 0x3502, 0x80);
	ov2736_write_register(ViPipe, 0x3508, 0x02);
	ov2736_write_register(ViPipe, 0x3509, 0x80);
	ov2736_write_register(ViPipe, 0x3d8c, 0x11);
	ov2736_write_register(ViPipe, 0x3d8d, 0xf0);
	ov2736_write_register(ViPipe, 0x5180, 0x00);
	ov2736_write_register(ViPipe, 0x5181, 0x10);
	ov2736_write_register(ViPipe, 0x36a0, 0x16);
	ov2736_write_register(ViPipe, 0x36a1, 0x50);
	ov2736_write_register(ViPipe, 0x36a2, 0x60);
	ov2736_write_register(ViPipe, 0x36a3, 0x80);
	ov2736_write_register(ViPipe, 0x36a4, 0x00);
	ov2736_write_register(ViPipe, 0x36a5, 0xa0);
	ov2736_write_register(ViPipe, 0x36a6, 0x00);
	ov2736_write_register(ViPipe, 0x36a7, 0x50);
	ov2736_write_register(ViPipe, 0x36a8, 0x00);
	ov2736_write_register(ViPipe, 0x36a9, 0x50);
	ov2736_write_register(ViPipe, 0x36aa, 0x00);
	ov2736_write_register(ViPipe, 0x36ab, 0x50);
	ov2736_write_register(ViPipe, 0x36ac, 0x00);
	ov2736_write_register(ViPipe, 0x36ad, 0x50);
	ov2736_write_register(ViPipe, 0x36ae, 0x00);
	ov2736_write_register(ViPipe, 0x36af, 0x50);
	ov2736_write_register(ViPipe, 0x36b0, 0x00);
	ov2736_write_register(ViPipe, 0x36b1, 0x50);
	ov2736_write_register(ViPipe, 0x36b2, 0x00);
	ov2736_write_register(ViPipe, 0x36b3, 0x50);
	ov2736_write_register(ViPipe, 0x36b4, 0x00);
	ov2736_write_register(ViPipe, 0x36b5, 0x50);
	ov2736_write_register(ViPipe, 0x36b9, 0xee);
	ov2736_write_register(ViPipe, 0x36ba, 0xee);
	ov2736_write_register(ViPipe, 0x36bb, 0xee);
	ov2736_write_register(ViPipe, 0x36bc, 0xee);
	ov2736_write_register(ViPipe, 0x36bd, 0x0e);
	ov2736_write_register(ViPipe, 0x36b6, 0x08);
	ov2736_write_register(ViPipe, 0x36b7, 0x08);
	ov2736_write_register(ViPipe, 0x36b8, 0x10);


	printf("ViPipe:%d,===set 0x0100 to 0x00 !===\n", ViPipe);
	ov2736_write_register(ViPipe, 0x0100, 0x00);


	ov2736_write_register(ViPipe, 0x3501, 0x20);
	ov2736_write_register(ViPipe, 0x3508, 0x03);
	ov2736_write_register(ViPipe, 0x3509, 0xaa);
	ov2736_write_register(ViPipe, 0x350c, 0x01);
	ov2736_write_register(ViPipe, 0x350d, 0x55);
	ov2736_write_register(ViPipe, 0x3511, 0x08);
	ov2736_write_register(ViPipe, 0x3512, 0x00);
	ov2736_write_register(ViPipe, 0x372a, 0x83);
	ov2736_write_register(ViPipe, 0x3757, 0x00);
	ov2736_write_register(ViPipe, 0x377b, 0x28);
	ov2736_write_register(ViPipe, 0x3821, 0x04);

	ov2736_default_reg_init(ViPipe);
	ov2736_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(100);

	printf("ViPipe:%d,===OV2736 1080P 30fps 12bit WDR Init OK!===\n", ViPipe);
}
