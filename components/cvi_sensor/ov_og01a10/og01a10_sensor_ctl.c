#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "og01a10_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>


#define OG01A10_CHIP_ID_ADDR_H   0x300a
#define OG01A10_CHIP_ID_ADDR_M   0x300b
#define OG01A10_CHIP_ID_ADDR_L   0x300c
#define OG01A10_CHIP_ID          0x470141

static void og01a10_linear_1280_1024p30_init(VI_PIPE ViPipe);

CVI_U8 og01a10_i2c_addr = 0x60; // 0x36
const CVI_U32 og01a10_addr_byte = 2;
const CVI_U32 og01a10_data_byte = 1;

int og01a10_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOg01a10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int og01a10_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOg01a10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int og01a10_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOg01a10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, og01a10_i2c_addr, (CVI_U32)addr, og01a10_addr_byte, og01a10_data_byte);
}


int og01a10_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOg01a10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, og01a10_i2c_addr, (CVI_U32)addr, og01a10_addr_byte,
				(CVI_U32)data, og01a10_data_byte);
}

void og01a10_standby(VI_PIPE ViPipe)
{

	printf("og01a10_standby\n");

}

void og01a10_restart(VI_PIPE ViPipe)
{

	printf("og01a10_restart\n");

}

void og01a10_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastOg01a10[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		og01a10_write_register(ViPipe,
				g_pastOg01a10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOg01a10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void og01a10_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 flip_value = 0x40, mirror_value = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		mirror_value = 0x04;
		break;
	case ISP_SNS_FLIP:
		flip_value = 0x44;
		break;
	case ISP_SNS_MIRROR_FLIP:
		flip_value = 0x44;
		mirror_value = 0x04;
		break;
	default:
		return;
	}
	og01a10_write_register(ViPipe, 0x3820, flip_value); // flip
	og01a10_write_register(ViPipe, 0x3821, mirror_value); // mirror
}

int og01a10_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;
	int nVal3;

	usleep(50);
	if (og01a10_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = og01a10_read_register(ViPipe, OG01A10_CHIP_ID_ADDR_H);
	nVal2 = og01a10_read_register(ViPipe, OG01A10_CHIP_ID_ADDR_M);
	nVal3 = og01a10_read_register(ViPipe, OG01A10_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0 || nVal3 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 16) | ((nVal2 & 0xFF) << 8) | (nVal3 & 0xFF)) != OG01A10_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void og01a10_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastOg01a10[ViPipe]->enWDRMode;

	og01a10_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
		og01a10_linear_1280_1024p30_init(ViPipe);
	}
	g_pastOg01a10[ViPipe]->bInit = CVI_TRUE;
}

void og01a10_exit(VI_PIPE ViPipe)
{
	og01a10_i2c_exit(ViPipe);
}

/* 1080P30 */
static void og01a10_linear_1280_1024p30_init(VI_PIPE ViPipe)
{
	og01a10_write_register(ViPipe, 0x0103, 0x01);
	og01a10_write_register(ViPipe, 0x0100, 0x00);
	og01a10_write_register(ViPipe, 0x0300, 0x0a);
	og01a10_write_register(ViPipe, 0x0301, 0x29);
	og01a10_write_register(ViPipe, 0x0302, 0x31);
	og01a10_write_register(ViPipe, 0x0303, 0x04);
	og01a10_write_register(ViPipe, 0x0304, 0x00);
	og01a10_write_register(ViPipe, 0x0305, 0xfa);
	og01a10_write_register(ViPipe, 0x0306, 0x00);
	og01a10_write_register(ViPipe, 0x0307, 0x01);
	og01a10_write_register(ViPipe, 0x0308, 0x02);
	og01a10_write_register(ViPipe, 0x0309, 0x00);
	og01a10_write_register(ViPipe, 0x0310, 0x00);
	og01a10_write_register(ViPipe, 0x0311, 0x00);
	og01a10_write_register(ViPipe, 0x0312, 0x07);
	og01a10_write_register(ViPipe, 0x0313, 0x00);
	og01a10_write_register(ViPipe, 0x0314, 0x00);
	og01a10_write_register(ViPipe, 0x0315, 0x00);
	og01a10_write_register(ViPipe, 0x0320, 0x02);
	og01a10_write_register(ViPipe, 0x0321, 0x01);
	og01a10_write_register(ViPipe, 0x0322, 0x01);
	og01a10_write_register(ViPipe, 0x0323, 0x04);
	og01a10_write_register(ViPipe, 0x0324, 0x01);
	og01a10_write_register(ViPipe, 0x0325, 0xc2);
	og01a10_write_register(ViPipe, 0x0326, 0xce);
	og01a10_write_register(ViPipe, 0x0327, 0x04);
	og01a10_write_register(ViPipe, 0x0329, 0x02);
	og01a10_write_register(ViPipe, 0x032a, 0x04);
	og01a10_write_register(ViPipe, 0x032b, 0x04);
	og01a10_write_register(ViPipe, 0x032c, 0x02);
	og01a10_write_register(ViPipe, 0x032d, 0x01);
	og01a10_write_register(ViPipe, 0x032e, 0x00);
	og01a10_write_register(ViPipe, 0x300d, 0x02);
	og01a10_write_register(ViPipe, 0x300e, 0x04);
	og01a10_write_register(ViPipe, 0x3021, 0x08);
	og01a10_write_register(ViPipe, 0x301e, 0x03);
	og01a10_write_register(ViPipe, 0x3103, 0x00);
	og01a10_write_register(ViPipe, 0x3106, 0x08);
	og01a10_write_register(ViPipe, 0x3107, 0x40);
	og01a10_write_register(ViPipe, 0x3216, 0x01);
	og01a10_write_register(ViPipe, 0x3217, 0x00);
	og01a10_write_register(ViPipe, 0x3218, 0xc0);
	og01a10_write_register(ViPipe, 0x3219, 0x55);
	og01a10_write_register(ViPipe, 0x3500, 0x00);
	og01a10_write_register(ViPipe, 0x3501, 0x04);
	og01a10_write_register(ViPipe, 0x3502, 0x8a);
	og01a10_write_register(ViPipe, 0x3506, 0x01);
	og01a10_write_register(ViPipe, 0x3507, 0x72);
	og01a10_write_register(ViPipe, 0x3508, 0x01);
	og01a10_write_register(ViPipe, 0x3509, 0x00);
	og01a10_write_register(ViPipe, 0x350a, 0x01);
	og01a10_write_register(ViPipe, 0x350b, 0x00);
	og01a10_write_register(ViPipe, 0x350c, 0x00);
	og01a10_write_register(ViPipe, 0x3541, 0x00);
	og01a10_write_register(ViPipe, 0x3542, 0x40);
	og01a10_write_register(ViPipe, 0x3605, 0xe0);
	og01a10_write_register(ViPipe, 0x3606, 0x41);
	og01a10_write_register(ViPipe, 0x3614, 0x20);
	og01a10_write_register(ViPipe, 0x3620, 0x0b);
	og01a10_write_register(ViPipe, 0x3630, 0x07);
	og01a10_write_register(ViPipe, 0x3636, 0xa0);
	og01a10_write_register(ViPipe, 0x3637, 0xf9);
	og01a10_write_register(ViPipe, 0x3638, 0x09);
	og01a10_write_register(ViPipe, 0x3639, 0x38);
	og01a10_write_register(ViPipe, 0x363f, 0x09);
	og01a10_write_register(ViPipe, 0x3640, 0x17);
	og01a10_write_register(ViPipe, 0x3662, 0x04);
	og01a10_write_register(ViPipe, 0x3665, 0x80);
	og01a10_write_register(ViPipe, 0x3670, 0x68);
	og01a10_write_register(ViPipe, 0x3674, 0x00);
	og01a10_write_register(ViPipe, 0x3677, 0x3f);
	og01a10_write_register(ViPipe, 0x3679, 0x00);
	og01a10_write_register(ViPipe, 0x369f, 0x19);
	og01a10_write_register(ViPipe, 0x36a0, 0x03);
	og01a10_write_register(ViPipe, 0x36a2, 0x19);
	og01a10_write_register(ViPipe, 0x36a3, 0x03);
	og01a10_write_register(ViPipe, 0x370d, 0x66);
	og01a10_write_register(ViPipe, 0x370f, 0x00);
	og01a10_write_register(ViPipe, 0x3710, 0x03);
	og01a10_write_register(ViPipe, 0x3715, 0x03);
	og01a10_write_register(ViPipe, 0x3716, 0x03);
	og01a10_write_register(ViPipe, 0x3717, 0x06);
	og01a10_write_register(ViPipe, 0x3733, 0x00);
	og01a10_write_register(ViPipe, 0x3778, 0x00);
	og01a10_write_register(ViPipe, 0x37a8, 0x0f);
	og01a10_write_register(ViPipe, 0x37a9, 0x01);
	og01a10_write_register(ViPipe, 0x37aa, 0x07);
	og01a10_write_register(ViPipe, 0x37bd, 0x1c);
	og01a10_write_register(ViPipe, 0x37c1, 0x2f);
	og01a10_write_register(ViPipe, 0x37c3, 0x09);
	og01a10_write_register(ViPipe, 0x37c8, 0x1d);
	og01a10_write_register(ViPipe, 0x37ca, 0x30);
	og01a10_write_register(ViPipe, 0x37df, 0x00);
	og01a10_write_register(ViPipe, 0x3800, 0x00);
	og01a10_write_register(ViPipe, 0x3801, 0x00);
	og01a10_write_register(ViPipe, 0x3802, 0x00);
	og01a10_write_register(ViPipe, 0x3803, 0x00);
	og01a10_write_register(ViPipe, 0x3804, 0x05);
	og01a10_write_register(ViPipe, 0x3805, 0x0f);
	og01a10_write_register(ViPipe, 0x3806, 0x04);
	og01a10_write_register(ViPipe, 0x3807, 0x0f);
	og01a10_write_register(ViPipe, 0x3808, 0x05);
	og01a10_write_register(ViPipe, 0x3809, 0x00);
	og01a10_write_register(ViPipe, 0x380a, 0x04);
	og01a10_write_register(ViPipe, 0x380b, 0x00);
	og01a10_write_register(ViPipe, 0x380c, 0x07);
	og01a10_write_register(ViPipe, 0x380d, 0xF4);
	og01a10_write_register(ViPipe, 0x380e, 0x09);
	og01a10_write_register(ViPipe, 0x380f, 0x30);
	og01a10_write_register(ViPipe, 0x3810, 0x00);
	og01a10_write_register(ViPipe, 0x3811, 0x08);
	og01a10_write_register(ViPipe, 0x3812, 0x00);
	og01a10_write_register(ViPipe, 0x3813, 0x08);
	og01a10_write_register(ViPipe, 0x3814, 0x11);
	og01a10_write_register(ViPipe, 0x3815, 0x11);
	og01a10_write_register(ViPipe, 0x3820, 0x40);
	og01a10_write_register(ViPipe, 0x3821, 0x04);
	og01a10_write_register(ViPipe, 0x3826, 0x00);
	og01a10_write_register(ViPipe, 0x3827, 0x00);
	og01a10_write_register(ViPipe, 0x382a, 0x08);
	og01a10_write_register(ViPipe, 0x382b, 0x52);
	og01a10_write_register(ViPipe, 0x382d, 0xba);
	og01a10_write_register(ViPipe, 0x383d, 0x14);
	og01a10_write_register(ViPipe, 0x384a, 0xa2);
	og01a10_write_register(ViPipe, 0x3866, 0x0e);
	og01a10_write_register(ViPipe, 0x3867, 0x07);
	og01a10_write_register(ViPipe, 0x3884, 0x00);
	og01a10_write_register(ViPipe, 0x3885, 0x08);
	og01a10_write_register(ViPipe, 0x3893, 0x68);
	og01a10_write_register(ViPipe, 0x3894, 0x2a);
	og01a10_write_register(ViPipe, 0x3898, 0x00);
	og01a10_write_register(ViPipe, 0x3899, 0x31);
	og01a10_write_register(ViPipe, 0x389a, 0x04);
	og01a10_write_register(ViPipe, 0x389b, 0x00);
	og01a10_write_register(ViPipe, 0x389c, 0x0b);
	og01a10_write_register(ViPipe, 0x389d, 0xad);
	og01a10_write_register(ViPipe, 0x389f, 0x08);
	og01a10_write_register(ViPipe, 0x38a0, 0x00);
	og01a10_write_register(ViPipe, 0x38a1, 0x00);
	og01a10_write_register(ViPipe, 0x38a8, 0x70);
	og01a10_write_register(ViPipe, 0x38ac, 0xea);
	og01a10_write_register(ViPipe, 0x38b2, 0x00);
	og01a10_write_register(ViPipe, 0x38b3, 0x08);
	og01a10_write_register(ViPipe, 0x38bc, 0x20);
	og01a10_write_register(ViPipe, 0x38c4, 0x0c);
	og01a10_write_register(ViPipe, 0x38c5, 0x3a);
	og01a10_write_register(ViPipe, 0x38c7, 0x3a);
	og01a10_write_register(ViPipe, 0x38e1, 0xc0);
	og01a10_write_register(ViPipe, 0x38ec, 0x3c);
	og01a10_write_register(ViPipe, 0x38f0, 0x09);
	og01a10_write_register(ViPipe, 0x38f1, 0x6f);
	og01a10_write_register(ViPipe, 0x38fe, 0x3c);
	og01a10_write_register(ViPipe, 0x391e, 0x00);
	og01a10_write_register(ViPipe, 0x391f, 0x00);
	og01a10_write_register(ViPipe, 0x3920, 0xa5);
	og01a10_write_register(ViPipe, 0x3921, 0x00);
	og01a10_write_register(ViPipe, 0x3922, 0x00);
	og01a10_write_register(ViPipe, 0x3923, 0x00);
	og01a10_write_register(ViPipe, 0x3924, 0x05);
	og01a10_write_register(ViPipe, 0x3925, 0x00);
	og01a10_write_register(ViPipe, 0x3926, 0x00);
	og01a10_write_register(ViPipe, 0x3927, 0x00);
	og01a10_write_register(ViPipe, 0x3928, 0x1a);
	og01a10_write_register(ViPipe, 0x3929, 0x01);
	og01a10_write_register(ViPipe, 0x392a, 0xb4);
	og01a10_write_register(ViPipe, 0x392b, 0x00);
	og01a10_write_register(ViPipe, 0x392c, 0x10);
	og01a10_write_register(ViPipe, 0x392f, 0x40);
	og01a10_write_register(ViPipe, 0x4000, 0xcf);
	og01a10_write_register(ViPipe, 0x4003, 0x40);
	og01a10_write_register(ViPipe, 0x4008, 0x00);
	og01a10_write_register(ViPipe, 0x4009, 0x07);
	og01a10_write_register(ViPipe, 0x400a, 0x02);
	og01a10_write_register(ViPipe, 0x400b, 0x54);
	og01a10_write_register(ViPipe, 0x400c, 0x00);
	og01a10_write_register(ViPipe, 0x400d, 0x07);
	og01a10_write_register(ViPipe, 0x4010, 0xc0);
	og01a10_write_register(ViPipe, 0x4012, 0x02);
	og01a10_write_register(ViPipe, 0x4014, 0x04);
	og01a10_write_register(ViPipe, 0x4015, 0x04);
	og01a10_write_register(ViPipe, 0x4017, 0x02);
	og01a10_write_register(ViPipe, 0x4042, 0x01);
	og01a10_write_register(ViPipe, 0x4306, 0x04);
	og01a10_write_register(ViPipe, 0x4307, 0x12);
	og01a10_write_register(ViPipe, 0x4509, 0x00);
	og01a10_write_register(ViPipe, 0x450b, 0x83);
	og01a10_write_register(ViPipe, 0x4604, 0x68);
	og01a10_write_register(ViPipe, 0x4608, 0x0a);
	og01a10_write_register(ViPipe, 0x4700, 0x06);
	og01a10_write_register(ViPipe, 0x4800, 0x64);
	og01a10_write_register(ViPipe, 0x481b, 0x3c);
	og01a10_write_register(ViPipe, 0x4825, 0x32);
	og01a10_write_register(ViPipe, 0x4833, 0x18);
	og01a10_write_register(ViPipe, 0x4837, 0x10);
	og01a10_write_register(ViPipe, 0x4850, 0x40);
	og01a10_write_register(ViPipe, 0x4860, 0x00);
	og01a10_write_register(ViPipe, 0x4861, 0xec);
	og01a10_write_register(ViPipe, 0x4864, 0x00);
	og01a10_write_register(ViPipe, 0x4883, 0x00);
	og01a10_write_register(ViPipe, 0x4888, 0x90);
	og01a10_write_register(ViPipe, 0x4889, 0x05);
	og01a10_write_register(ViPipe, 0x488b, 0x04);
	og01a10_write_register(ViPipe, 0x4f00, 0x04);
	og01a10_write_register(ViPipe, 0x4f10, 0x04);
	og01a10_write_register(ViPipe, 0x4f21, 0x01);
	og01a10_write_register(ViPipe, 0x4f22, 0x40);
	og01a10_write_register(ViPipe, 0x4f23, 0x44);
	og01a10_write_register(ViPipe, 0x4f24, 0x51);
	og01a10_write_register(ViPipe, 0x4f25, 0x41);
	og01a10_write_register(ViPipe, 0x5000, 0x1f);
	og01a10_write_register(ViPipe, 0x500a, 0x00);
	og01a10_write_register(ViPipe, 0x5100, 0x00);
	og01a10_write_register(ViPipe, 0x5111, 0x20);

	og01a10_write_register(ViPipe, 0x3020, 0x20);
	og01a10_write_register(ViPipe, 0x3613, 0x3 );
	og01a10_write_register(ViPipe, 0x38c9, 0x02);
	og01a10_write_register(ViPipe, 0x5304, 0x01);
	og01a10_write_register(ViPipe, 0x3620, 0x08);
	og01a10_write_register(ViPipe, 0x3639, 0x58);
	og01a10_write_register(ViPipe, 0x363a, 0x10);
	og01a10_write_register(ViPipe, 0x3674, 0x4 );
	og01a10_write_register(ViPipe, 0x3780, 0xff);
	og01a10_write_register(ViPipe, 0x3781, 0xff);
	og01a10_write_register(ViPipe, 0x3782, 0x0 );
	og01a10_write_register(ViPipe, 0x3783, 0x1 );
	og01a10_write_register(ViPipe, 0x3798, 0xa3);
	og01a10_write_register(ViPipe, 0x37aa, 0x10);
	og01a10_write_register(ViPipe, 0x38a8, 0xf0);
	og01a10_write_register(ViPipe, 0x38c4, 0x9 );
	og01a10_write_register(ViPipe, 0x38c5, 0xb0);
	og01a10_write_register(ViPipe, 0x38df, 0x80);
	og01a10_write_register(ViPipe, 0x38ff, 0x5 );
	og01a10_write_register(ViPipe, 0x4010, 0xf1);
	og01a10_write_register(ViPipe, 0x4011, 0x70);

	og01a10_write_register(ViPipe, 0x3667, 0x80);
	og01a10_write_register(ViPipe, 0x4d00, 0x4a);
	og01a10_write_register(ViPipe, 0x4d01, 0x18);
	og01a10_write_register(ViPipe, 0x4d02, 0xbb);
	og01a10_write_register(ViPipe, 0x4d03, 0xde);
	og01a10_write_register(ViPipe, 0x4d04, 0x93);
	og01a10_write_register(ViPipe, 0x4d05, 0xff);
	og01a10_write_register(ViPipe, 0x4d09, 0x0a);

	og01a10_write_register(ViPipe, 0x4f22, 0x00);

	og01a10_write_register(ViPipe, 0x37aa, 0x16);
	og01a10_write_register(ViPipe, 0x3606, 0x42);
	og01a10_write_register(ViPipe, 0x3605, 0x00);
	og01a10_write_register(ViPipe, 0x36a2, 0x17);
	og01a10_write_register(ViPipe, 0x300d, 0x0a);
	og01a10_write_register(ViPipe, 0x4d00, 0x4d);
	og01a10_write_register(ViPipe, 0x4d01, 0x95);
	og01a10_write_register(ViPipe, 0x3d8C, 0x70);
	og01a10_write_register(ViPipe, 0x3d8d, 0xE9);
	og01a10_write_register(ViPipe, 0x5300, 0x00);
	og01a10_write_register(ViPipe, 0x5301, 0x10);
	og01a10_write_register(ViPipe, 0x5302, 0x00);
	og01a10_write_register(ViPipe, 0x5303, 0xE3);
	og01a10_write_register(ViPipe, 0x3d88, 0x00);
	og01a10_write_register(ViPipe, 0x3d89, 0x10);
	og01a10_write_register(ViPipe, 0x3d8a, 0x00);
	og01a10_write_register(ViPipe, 0x3d8b, 0xE3);

	og01a10_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===OG01A10 1280x1024 30fps 10bit LINEAR Init OK!===\n", ViPipe);
	return;
}