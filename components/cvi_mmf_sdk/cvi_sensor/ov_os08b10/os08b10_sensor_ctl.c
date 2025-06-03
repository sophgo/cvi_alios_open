#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "os08b10_cmos_ex.h"
#include "sensor_i2c.h"

static void os08b10_wdr_2160p30_2to1_init(VI_PIPE ViPipe);
static void os08b10_linear_2160p30_init(VI_PIPE ViPipe);

#define OS08B10_CHIP_ID_ADDR_H		0x300A
#define OS08B10_CHIP_ID_ADDR_M		0x300B
#define OS08B10_CHIP_ID_ADDR_L		0x300C
#define OS08B10_CHIP_ID				0x530842
#define OS08B10_ORIEN_ADDR			(0x3820)
const CVI_U32 os08b10_addr_byte = 2;
const CVI_U32 os08b10_data_byte = 1;
ISP_SNS_MIRRORFLIP_TYPE_E g_aeOs08b10_MirrorFip_Initial[VI_MAX_PIPE_NUM] = {
	ISP_SNS_MIRROR, ISP_SNS_MIRROR, ISP_SNS_MIRROR, ISP_SNS_MIRROR};

int os08b10_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunOs08b10_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOs08b10_AddrInfo[ViPipe].s8I2cAddr);
}

int os08b10_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunOs08b10_BusInfo[ViPipe].s8I2cDev);
}

int os08b10_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunOs08b10_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOs08b10_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							os08b10_addr_byte, os08b10_data_byte);
}

int os08b10_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunOs08b10_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOs08b10_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							os08b10_addr_byte, (CVI_U32)data, os08b10_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void os08b10_standby(VI_PIPE ViPipe)
{
	os08b10_write_register(ViPipe, 0x0100, 0x00); /* STANDBY */
}

void os08b10_restart(VI_PIPE ViPipe)
{
	os08b10_write_register(ViPipe, 0x0100, 0x01); /* standby */
}

void os08b10_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastOs08b10[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		os08b10_write_register(ViPipe,
				g_pastOs08b10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOs08b10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		CVI_TRACE_SNS(CVI_DBG_INFO, "i2c_addr:%#x, i2c_data:%#x\n",
			g_pastOs08b10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
			g_pastOs08b10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

static ISP_SNS_MIRRORFLIP_TYPE_E aeSnsMirrorFlipMap[ISP_SNS_BUTT][ISP_SNS_BUTT] = {
	{ISP_SNS_NORMAL, ISP_SNS_MIRROR, ISP_SNS_FLIP, ISP_SNS_MIRROR_FLIP},
	{ISP_SNS_MIRROR, ISP_SNS_NORMAL, ISP_SNS_MIRROR_FLIP, ISP_SNS_FLIP},
	{ISP_SNS_FLIP, ISP_SNS_MIRROR_FLIP, ISP_SNS_NORMAL, ISP_SNS_MIRROR},
	{ISP_SNS_MIRROR_FLIP, ISP_SNS_FLIP, ISP_SNS_MIRROR, ISP_SNS_NORMAL}
};

void os08b10_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = 0;
	CVI_U32 i = 0;

	for (i = 0; i < ISP_SNS_BUTT; i++) {
		if (g_aeOs08b10_MirrorFip_Initial[ViPipe] == aeSnsMirrorFlipMap[i][0]) {
			eSnsMirrorFlip = aeSnsMirrorFlipMap[i][eSnsMirrorFlip];
			break;
		}
	}

	val = os08b10_read_register(ViPipe, OS08B10_ORIEN_ADDR);
	val &= ~(0x3 << 1);

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x1<<1;
		break;
	case ISP_SNS_FLIP:
		val |= 0x1<<2;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x1<<1;
		val |= 0x1<<2;
		break;
	default:
		return;
	}

	os08b10_standby(ViPipe);
	os08b10_write_register(ViPipe, OS08B10_ORIEN_ADDR, val);
	usleep(1000*100);
	os08b10_restart(ViPipe);
}

int os08b10_probe(VI_PIPE ViPipe)
{
	int nVal, nVal2, nVal3;

	usleep(500);
	if (os08b10_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = os08b10_read_register(ViPipe, OS08B10_CHIP_ID_ADDR_H);
	nVal2 = os08b10_read_register(ViPipe, OS08B10_CHIP_ID_ADDR_M);
	nVal3 = os08b10_read_register(ViPipe, OS08B10_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0 || nVal3 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 16) | ((nVal2 & 0xFF) << 8) | (nVal3 & 0xFF)) != OS08B10_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void os08b10_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode;
	CVI_U8 u8ImgMode;

	enWDRMode = g_pastOs08b10[ViPipe]->enWDRMode;
	u8ImgMode = g_pastOs08b10[ViPipe]->u8ImgMode;

	os08b10_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == OS08B10_MODE_3840X2160P30_WDR)
			os08b10_wdr_2160p30_2to1_init(ViPipe);
		else {
		}
	} else {
		if (u8ImgMode == OS08B10_MODE_3840X2160P30)
			os08b10_linear_2160p30_init(ViPipe);
		else {
		}
	}
	g_pastOs08b10[ViPipe]->bInit = CVI_TRUE;
}

/* 2160P30 */
static void os08b10_linear_2160p30_init(VI_PIPE ViPipe)
{
	os08b10_write_register(ViPipe, 0x0103, 0x01);
	os08b10_write_register(ViPipe, 0x0109, 0x01);
	os08b10_write_register(ViPipe, 0x0104, 0x03);
	os08b10_write_register(ViPipe, 0x0102, 0x00);
	os08b10_write_register(ViPipe, 0x0303, 0x00);
	os08b10_write_register(ViPipe, 0x0305, 0x2a);
	os08b10_write_register(ViPipe, 0x0306, 0x03);
	os08b10_write_register(ViPipe, 0x0307, 0x00);
	os08b10_write_register(ViPipe, 0x0308, 0x04);
	os08b10_write_register(ViPipe, 0x0309, 0x02);
	os08b10_write_register(ViPipe, 0x030a, 0x01);
	os08b10_write_register(ViPipe, 0x030c, 0x00);
	os08b10_write_register(ViPipe, 0x0317, 0x49);
	os08b10_write_register(ViPipe, 0x0322, 0x01);
	os08b10_write_register(ViPipe, 0x0323, 0x05);
	os08b10_write_register(ViPipe, 0x0324, 0x01);
	os08b10_write_register(ViPipe, 0x0325, 0x80);
	os08b10_write_register(ViPipe, 0x0327, 0x03);
	os08b10_write_register(ViPipe, 0x0328, 0x05);
	os08b10_write_register(ViPipe, 0x0329, 0x02);
	os08b10_write_register(ViPipe, 0x032a, 0x02);
	os08b10_write_register(ViPipe, 0x032c, 0x01);
	os08b10_write_register(ViPipe, 0x032d, 0x00);
	os08b10_write_register(ViPipe, 0x032e, 0x03);
	os08b10_write_register(ViPipe, 0x032f, 0x01);
	os08b10_write_register(ViPipe, 0x3002, 0x00);
	os08b10_write_register(ViPipe, 0x300f, 0x11);
	os08b10_write_register(ViPipe, 0x3012, 0x41);
	os08b10_write_register(ViPipe, 0x301e, 0xb8);
	os08b10_write_register(ViPipe, 0x3026, 0x12);
	os08b10_write_register(ViPipe, 0x3027, 0x08);
	os08b10_write_register(ViPipe, 0x302c, 0x1a);
	os08b10_write_register(ViPipe, 0x3104, 0x00);
	os08b10_write_register(ViPipe, 0x3106, 0x10);
	os08b10_write_register(ViPipe, 0x3400, 0x00);
	os08b10_write_register(ViPipe, 0x3408, 0x05);
	os08b10_write_register(ViPipe, 0x3409, 0x22);
	os08b10_write_register(ViPipe, 0x340a, 0x02);
	os08b10_write_register(ViPipe, 0x340c, 0x08);
	os08b10_write_register(ViPipe, 0x340d, 0xa4);
	os08b10_write_register(ViPipe, 0x3420, 0x00);
	os08b10_write_register(ViPipe, 0x3421, 0x00);
	os08b10_write_register(ViPipe, 0x3422, 0x00);
	os08b10_write_register(ViPipe, 0x3423, 0x00);
	os08b10_write_register(ViPipe, 0x3424, 0x00);
	os08b10_write_register(ViPipe, 0x3425, 0x00);
	os08b10_write_register(ViPipe, 0x3426, 0x55);
	os08b10_write_register(ViPipe, 0x3427, 0x15);
	os08b10_write_register(ViPipe, 0x3428, 0x55);
	os08b10_write_register(ViPipe, 0x3429, 0x15);
	os08b10_write_register(ViPipe, 0x342a, 0x50);
	os08b10_write_register(ViPipe, 0x342b, 0x00);
	os08b10_write_register(ViPipe, 0x342c, 0x01);
	os08b10_write_register(ViPipe, 0x342d, 0x54);
	os08b10_write_register(ViPipe, 0x342e, 0x01);
	os08b10_write_register(ViPipe, 0x342f, 0x04);
	os08b10_write_register(ViPipe, 0x3501, 0x01);
	os08b10_write_register(ViPipe, 0x3502, 0x40);
	os08b10_write_register(ViPipe, 0x3504, 0x08);
	os08b10_write_register(ViPipe, 0x3541, 0x00);
	os08b10_write_register(ViPipe, 0x3542, 0x40);
	os08b10_write_register(ViPipe, 0x3581, 0x00);
	os08b10_write_register(ViPipe, 0x3582, 0x40);
	os08b10_write_register(ViPipe, 0x3603, 0x30);
	os08b10_write_register(ViPipe, 0x3612, 0x96);
	os08b10_write_register(ViPipe, 0x3613, 0xe0);
	os08b10_write_register(ViPipe, 0x3623, 0x00);
	os08b10_write_register(ViPipe, 0x3624, 0xff);
	os08b10_write_register(ViPipe, 0x3625, 0xff);
	os08b10_write_register(ViPipe, 0x362a, 0x12);
	os08b10_write_register(ViPipe, 0x362b, 0x12);
	os08b10_write_register(ViPipe, 0x362c, 0x12);
	os08b10_write_register(ViPipe, 0x362d, 0x12);
	os08b10_write_register(ViPipe, 0x362e, 0x17);
	os08b10_write_register(ViPipe, 0x362f, 0x2d);
	os08b10_write_register(ViPipe, 0x3630, 0x67);
	os08b10_write_register(ViPipe, 0x3631, 0x7f);
	os08b10_write_register(ViPipe, 0x3638, 0x00);
	os08b10_write_register(ViPipe, 0x3639, 0xff);
	os08b10_write_register(ViPipe, 0x363a, 0xff);
	os08b10_write_register(ViPipe, 0x3643, 0x0a);
	os08b10_write_register(ViPipe, 0x3644, 0x00);
	os08b10_write_register(ViPipe, 0x3645, 0x0a);
	os08b10_write_register(ViPipe, 0x3646, 0x0a);
	os08b10_write_register(ViPipe, 0x3647, 0x06);
	os08b10_write_register(ViPipe, 0x3648, 0x00);
	os08b10_write_register(ViPipe, 0x3649, 0x0a);
	os08b10_write_register(ViPipe, 0x364a, 0x0d);
	os08b10_write_register(ViPipe, 0x364b, 0x02);
	os08b10_write_register(ViPipe, 0x364c, 0x12);
	os08b10_write_register(ViPipe, 0x364d, 0x12);
	os08b10_write_register(ViPipe, 0x364e, 0x12);
	os08b10_write_register(ViPipe, 0x364f, 0x12);
	os08b10_write_register(ViPipe, 0x3650, 0xf7);
	os08b10_write_register(ViPipe, 0x3651, 0x77);
	os08b10_write_register(ViPipe, 0x365a, 0xbb);
	os08b10_write_register(ViPipe, 0x365b, 0x9d);
	os08b10_write_register(ViPipe, 0x365c, 0x8e);
	os08b10_write_register(ViPipe, 0x365d, 0x86);
	os08b10_write_register(ViPipe, 0x3661, 0x07);
	os08b10_write_register(ViPipe, 0x3662, 0x0a);
	os08b10_write_register(ViPipe, 0x3667, 0xd4);
	os08b10_write_register(ViPipe, 0x366f, 0x00);
	os08b10_write_register(ViPipe, 0x3671, 0x88);
	os08b10_write_register(ViPipe, 0x3673, 0x2a);
	os08b10_write_register(ViPipe, 0x3678, 0x22);
	os08b10_write_register(ViPipe, 0x3679, 0x00);
	os08b10_write_register(ViPipe, 0x3682, 0xf1);
	os08b10_write_register(ViPipe, 0x3687, 0x02);
	os08b10_write_register(ViPipe, 0x3688, 0x81);
	os08b10_write_register(ViPipe, 0x3689, 0x02);
	os08b10_write_register(ViPipe, 0x3692, 0x0f);
	os08b10_write_register(ViPipe, 0x3701, 0x39);
	os08b10_write_register(ViPipe, 0x3703, 0x32);
	os08b10_write_register(ViPipe, 0x3705, 0x00);
	os08b10_write_register(ViPipe, 0x3706, 0x3a);
	os08b10_write_register(ViPipe, 0x3708, 0x27);
	os08b10_write_register(ViPipe, 0x370a, 0x00);
	os08b10_write_register(ViPipe, 0x370b, 0x8a);
	os08b10_write_register(ViPipe, 0x3712, 0x13);
	os08b10_write_register(ViPipe, 0x3714, 0x02);
	os08b10_write_register(ViPipe, 0x3715, 0x00);
	os08b10_write_register(ViPipe, 0x3716, 0x00);
	os08b10_write_register(ViPipe, 0x3719, 0x23);
	os08b10_write_register(ViPipe, 0x371c, 0x00);
	os08b10_write_register(ViPipe, 0x371d, 0x08);
	os08b10_write_register(ViPipe, 0x373b, 0x50);
	os08b10_write_register(ViPipe, 0x3755, 0x01);
	os08b10_write_register(ViPipe, 0x3756, 0xa9);
	os08b10_write_register(ViPipe, 0x3757, 0xa9);
	os08b10_write_register(ViPipe, 0x376b, 0x02);
	os08b10_write_register(ViPipe, 0x376c, 0x02);
	os08b10_write_register(ViPipe, 0x376d, 0x04);
	os08b10_write_register(ViPipe, 0x376e, 0x04);
	os08b10_write_register(ViPipe, 0x376f, 0x22);
	os08b10_write_register(ViPipe, 0x377a, 0x02);
	os08b10_write_register(ViPipe, 0x377b, 0x01);
	os08b10_write_register(ViPipe, 0x3785, 0x08);
	os08b10_write_register(ViPipe, 0x3790, 0x10);
	os08b10_write_register(ViPipe, 0x3792, 0x00);
	os08b10_write_register(ViPipe, 0x3796, 0x00);
	os08b10_write_register(ViPipe, 0x3797, 0x00);
	os08b10_write_register(ViPipe, 0x3798, 0x00);
	os08b10_write_register(ViPipe, 0x3799, 0x00);
	os08b10_write_register(ViPipe, 0x37bb, 0x88);
	os08b10_write_register(ViPipe, 0x37be, 0x01);
	os08b10_write_register(ViPipe, 0x37bf, 0x01);
	os08b10_write_register(ViPipe, 0x37c0, 0x00);
	os08b10_write_register(ViPipe, 0x37c7, 0x51);
	os08b10_write_register(ViPipe, 0x37c8, 0x22);
	os08b10_write_register(ViPipe, 0x37c9, 0x00);
	os08b10_write_register(ViPipe, 0x37ca, 0xb6);
	os08b10_write_register(ViPipe, 0x37cc, 0x0e);
	os08b10_write_register(ViPipe, 0x37cf, 0x02);
	os08b10_write_register(ViPipe, 0x37d0, 0x00);
	os08b10_write_register(ViPipe, 0x37d1, 0x3a);
	os08b10_write_register(ViPipe, 0x37d2, 0x00);
	os08b10_write_register(ViPipe, 0x37d3, 0x8a);
	os08b10_write_register(ViPipe, 0x37d4, 0x00);
	os08b10_write_register(ViPipe, 0x37d5, 0x3a);
	os08b10_write_register(ViPipe, 0x37d6, 0x00);
	os08b10_write_register(ViPipe, 0x37d7, 0x8a);
	os08b10_write_register(ViPipe, 0x3800, 0x00);
	os08b10_write_register(ViPipe, 0x3801, 0x00);
	os08b10_write_register(ViPipe, 0x3802, 0x00);
	os08b10_write_register(ViPipe, 0x3803, 0x00);
	os08b10_write_register(ViPipe, 0x3804, 0x0f);
	os08b10_write_register(ViPipe, 0x3805, 0x1f);
	os08b10_write_register(ViPipe, 0x3806, 0x08);
	os08b10_write_register(ViPipe, 0x3807, 0x7f);
	os08b10_write_register(ViPipe, 0x3808, 0x0f);
	os08b10_write_register(ViPipe, 0x3809, 0x00);
	os08b10_write_register(ViPipe, 0x380a, 0x08);
	os08b10_write_register(ViPipe, 0x380b, 0x70);
	os08b10_write_register(ViPipe, 0x380c, 0x06);
	os08b10_write_register(ViPipe, 0x380d, 0x58);
	os08b10_write_register(ViPipe, 0x380e, 0x04);
	os08b10_write_register(ViPipe, 0x380f, 0x54);
	os08b10_write_register(ViPipe, 0x3811, 0x10);
	os08b10_write_register(ViPipe, 0x3813, 0x08);
	os08b10_write_register(ViPipe, 0x3814, 0x01);
	os08b10_write_register(ViPipe, 0x3815, 0x01);
	os08b10_write_register(ViPipe, 0x3816, 0x01);
	os08b10_write_register(ViPipe, 0x3817, 0x01);
	os08b10_write_register(ViPipe, 0x381c, 0x00);
	os08b10_write_register(ViPipe, 0x3820, 0x02);
	os08b10_write_register(ViPipe, 0x3821, 0x00);
	os08b10_write_register(ViPipe, 0x3822, 0x04);
	os08b10_write_register(ViPipe, 0x3823, 0x08);
	os08b10_write_register(ViPipe, 0x382b, 0x00);
	os08b10_write_register(ViPipe, 0x382c, 0x00);
	os08b10_write_register(ViPipe, 0x3833, 0x40);
	os08b10_write_register(ViPipe, 0x383e, 0x00);
	os08b10_write_register(ViPipe, 0x384c, 0x02);
	os08b10_write_register(ViPipe, 0x384d, 0xd0);
	os08b10_write_register(ViPipe, 0x3858, 0x3c);
	os08b10_write_register(ViPipe, 0x3850, 0x00);
	os08b10_write_register(ViPipe, 0x3851, 0x00);
	os08b10_write_register(ViPipe, 0x3852, 0x00);
	os08b10_write_register(ViPipe, 0x3853, 0x00);
	os08b10_write_register(ViPipe, 0x3865, 0x02);
	os08b10_write_register(ViPipe, 0x3866, 0x00);
	os08b10_write_register(ViPipe, 0x3867, 0x02);
	os08b10_write_register(ViPipe, 0x3868, 0x00);
	os08b10_write_register(ViPipe, 0x386a, 0x05);
	os08b10_write_register(ViPipe, 0x386b, 0x00);
	os08b10_write_register(ViPipe, 0x386c, 0x05);
	os08b10_write_register(ViPipe, 0x386d, 0x61);
	os08b10_write_register(ViPipe, 0x3881, 0x02);
	os08b10_write_register(ViPipe, 0x3882, 0x00);
	os08b10_write_register(ViPipe, 0x3883, 0x08);
	os08b10_write_register(ViPipe, 0x3884, 0x01);
	os08b10_write_register(ViPipe, 0x3885, 0x00);
	os08b10_write_register(ViPipe, 0x3886, 0x00);
	os08b10_write_register(ViPipe, 0x3887, 0x20);
	os08b10_write_register(ViPipe, 0x3888, 0x40);
	os08b10_write_register(ViPipe, 0x3c37, 0x00);
	os08b10_write_register(ViPipe, 0x3c49, 0x00);
	os08b10_write_register(ViPipe, 0x3c4a, 0x9a);
	os08b10_write_register(ViPipe, 0x3c4c, 0x01);
	os08b10_write_register(ViPipe, 0x3c4d, 0x00);
	os08b10_write_register(ViPipe, 0x3c4e, 0x00);
	os08b10_write_register(ViPipe, 0x3c50, 0x00);
	os08b10_write_register(ViPipe, 0x3c51, 0x00);
	os08b10_write_register(ViPipe, 0x3c52, 0x00);
	os08b10_write_register(ViPipe, 0x3c67, 0x10);
	os08b10_write_register(ViPipe, 0x3c69, 0x10);
	os08b10_write_register(ViPipe, 0x3c6a, 0x00);
	os08b10_write_register(ViPipe, 0x3c6b, 0x52);
	os08b10_write_register(ViPipe, 0x3c6c, 0x00);
	os08b10_write_register(ViPipe, 0x3c6d, 0x52);
	os08b10_write_register(ViPipe, 0x3c70, 0x00);
	os08b10_write_register(ViPipe, 0x3c71, 0x04);
	os08b10_write_register(ViPipe, 0x3c72, 0x00);
	os08b10_write_register(ViPipe, 0x3c73, 0x52);
	os08b10_write_register(ViPipe, 0x3c74, 0x00);
	os08b10_write_register(ViPipe, 0x3c75, 0x52);
	os08b10_write_register(ViPipe, 0x3c76, 0x12);
	os08b10_write_register(ViPipe, 0x3c77, 0x12);
	os08b10_write_register(ViPipe, 0x3c79, 0x00);
	os08b10_write_register(ViPipe, 0x3c7a, 0x00);
	os08b10_write_register(ViPipe, 0x3c7b, 0x00);
	os08b10_write_register(ViPipe, 0x3cb6, 0x41);
	os08b10_write_register(ViPipe, 0x3cb9, 0x00);
	os08b10_write_register(ViPipe, 0x3cc0, 0x90);
	os08b10_write_register(ViPipe, 0x3cc2, 0x90);
	os08b10_write_register(ViPipe, 0x3cc5, 0x00);
	os08b10_write_register(ViPipe, 0x3cc6, 0x98);
	os08b10_write_register(ViPipe, 0x3cc7, 0x00);
	os08b10_write_register(ViPipe, 0x3cc8, 0x98);
	os08b10_write_register(ViPipe, 0x3cc9, 0x00);
	os08b10_write_register(ViPipe, 0x3cca, 0x98);
	os08b10_write_register(ViPipe, 0x3ccc, 0x01);
	os08b10_write_register(ViPipe, 0x3cd9, 0x7c);
	os08b10_write_register(ViPipe, 0x3cda, 0x7c);
	os08b10_write_register(ViPipe, 0x3cdb, 0x7c);
	os08b10_write_register(ViPipe, 0x3cdc, 0x7c);
	os08b10_write_register(ViPipe, 0x3d8c, 0x70);
	os08b10_write_register(ViPipe, 0x3d8d, 0x10);
	os08b10_write_register(ViPipe, 0x4001, 0x2b);
	os08b10_write_register(ViPipe, 0x4004, 0x00);
	os08b10_write_register(ViPipe, 0x4005, 0x40);
	os08b10_write_register(ViPipe, 0x4008, 0x02);
	os08b10_write_register(ViPipe, 0x4009, 0x0d);
	os08b10_write_register(ViPipe, 0x400a, 0x02);
	os08b10_write_register(ViPipe, 0x400b, 0x00);
	os08b10_write_register(ViPipe, 0x400e, 0x40);
	os08b10_write_register(ViPipe, 0x401a, 0x50);
	os08b10_write_register(ViPipe, 0x4028, 0x6f);
	os08b10_write_register(ViPipe, 0x4029, 0x00);
	os08b10_write_register(ViPipe, 0x402a, 0x7f);
	os08b10_write_register(ViPipe, 0x402b, 0x01);
	os08b10_write_register(ViPipe, 0x402e, 0x00);
	os08b10_write_register(ViPipe, 0x402f, 0x40);
	os08b10_write_register(ViPipe, 0x4030, 0x00);
	os08b10_write_register(ViPipe, 0x4031, 0x40);
	os08b10_write_register(ViPipe, 0x4032, 0x1f);
	os08b10_write_register(ViPipe, 0x4050, 0x01);
	os08b10_write_register(ViPipe, 0x4051, 0x06);
	os08b10_write_register(ViPipe, 0x405d, 0x00);
	os08b10_write_register(ViPipe, 0x405e, 0x00);
	os08b10_write_register(ViPipe, 0x4288, 0xcf);
	os08b10_write_register(ViPipe, 0x4289, 0x00);
	os08b10_write_register(ViPipe, 0x428a, 0x40);
	os08b10_write_register(ViPipe, 0x430b, 0x0f);
	os08b10_write_register(ViPipe, 0x430c, 0xfc);
	os08b10_write_register(ViPipe, 0x430d, 0x00);
	os08b10_write_register(ViPipe, 0x430e, 0x00);
	os08b10_write_register(ViPipe, 0x4314, 0x04);
	os08b10_write_register(ViPipe, 0x4500, 0x1a);
	os08b10_write_register(ViPipe, 0x4501, 0x18);
	os08b10_write_register(ViPipe, 0x4504, 0x00);
	os08b10_write_register(ViPipe, 0x4505, 0x00);
	os08b10_write_register(ViPipe, 0x4506, 0x32);
	os08b10_write_register(ViPipe, 0x4507, 0x02);
	os08b10_write_register(ViPipe, 0x4508, 0x1a);
	os08b10_write_register(ViPipe, 0x450a, 0x12);
	os08b10_write_register(ViPipe, 0x450b, 0x28);
	os08b10_write_register(ViPipe, 0x450c, 0x00);
	os08b10_write_register(ViPipe, 0x450f, 0x80);
	os08b10_write_register(ViPipe, 0x4513, 0x00);
	os08b10_write_register(ViPipe, 0x4516, 0x28);
	os08b10_write_register(ViPipe, 0x4603, 0x00);
	os08b10_write_register(ViPipe, 0x460a, 0x50);
	os08b10_write_register(ViPipe, 0x4640, 0x62);
	os08b10_write_register(ViPipe, 0x464d, 0x00);
	os08b10_write_register(ViPipe, 0x4800, 0x04);
	os08b10_write_register(ViPipe, 0x480e, 0x00);
	os08b10_write_register(ViPipe, 0x4813, 0x00);
	os08b10_write_register(ViPipe, 0x4837, 0x12);
	os08b10_write_register(ViPipe, 0x484b, 0x27);
	os08b10_write_register(ViPipe, 0x4850, 0x47);
	os08b10_write_register(ViPipe, 0x4851, 0xaa);
	os08b10_write_register(ViPipe, 0x4852, 0xff);
	os08b10_write_register(ViPipe, 0x4853, 0x20);
	os08b10_write_register(ViPipe, 0x4854, 0x08);
	os08b10_write_register(ViPipe, 0x4855, 0x30);
	os08b10_write_register(ViPipe, 0x4860, 0x00);
	os08b10_write_register(ViPipe, 0x4d00, 0x4d);
	os08b10_write_register(ViPipe, 0x4d01, 0x42);
	os08b10_write_register(ViPipe, 0x4d02, 0xb9);
	os08b10_write_register(ViPipe, 0x4d03, 0x94);
	os08b10_write_register(ViPipe, 0x4d04, 0x95);
	os08b10_write_register(ViPipe, 0x4d05, 0xce);
	os08b10_write_register(ViPipe, 0x4d09, 0x63);
	os08b10_write_register(ViPipe, 0x5000, 0x0f);
	os08b10_write_register(ViPipe, 0x5001, 0x0d);
	os08b10_write_register(ViPipe, 0x5080, 0x00);
	os08b10_write_register(ViPipe, 0x50c0, 0x00);
	os08b10_write_register(ViPipe, 0x5100, 0x00);
	os08b10_write_register(ViPipe, 0x5200, 0x70);
	os08b10_write_register(ViPipe, 0x5201, 0x70);
	os08b10_write_register(ViPipe, 0x5681, 0x00);
	os08b10_write_register(ViPipe, 0x5780, 0x53);
	os08b10_write_register(ViPipe, 0x5781, 0x03);
	os08b10_write_register(ViPipe, 0x5782, 0x18);
	os08b10_write_register(ViPipe, 0x5783, 0x3c);
	os08b10_write_register(ViPipe, 0x5784, 0x00);
	os08b10_write_register(ViPipe, 0x5785, 0x40);
	os08b10_write_register(ViPipe, 0x5786, 0x01);
	os08b10_write_register(ViPipe, 0x5788, 0x18);
	os08b10_write_register(ViPipe, 0x5789, 0x3c);
	os08b10_write_register(ViPipe, 0x5791, 0x08);
	os08b10_write_register(ViPipe, 0x5792, 0x11);
	os08b10_write_register(ViPipe, 0x5793, 0x33);
	os08b10_default_reg_init(ViPipe);
	os08b10_write_register(ViPipe, 0x0100, 0x01); /* standby */
	os08b10_write_register(ViPipe, 0x0100, 0x01); /* standby */

	delay_ms(50);

	printf("ViPipe:%d,===os08b10 2160P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

static void os08b10_wdr_2160p30_2to1_init(VI_PIPE ViPipe)
{
	os08b10_write_register(ViPipe, 0x0103, 0x01);
	os08b10_write_register(ViPipe, 0x0109, 0x01);
	os08b10_write_register(ViPipe, 0x0104, 0x03);
	os08b10_write_register(ViPipe, 0x0102, 0x00);
	os08b10_write_register(ViPipe, 0x0303, 0x00);
	os08b10_write_register(ViPipe, 0x0305, 0x36);
	os08b10_write_register(ViPipe, 0x0306, 0x03);
	os08b10_write_register(ViPipe, 0x0307, 0x00);
	os08b10_write_register(ViPipe, 0x0308, 0x04);
	os08b10_write_register(ViPipe, 0x0309, 0x02);
	os08b10_write_register(ViPipe, 0x030a, 0x01);
	os08b10_write_register(ViPipe, 0x030c, 0x00);
	os08b10_write_register(ViPipe, 0x0317, 0x49);
	os08b10_write_register(ViPipe, 0x0322, 0x01);
	os08b10_write_register(ViPipe, 0x0323, 0x05);
	os08b10_write_register(ViPipe, 0x0324, 0x01);
	os08b10_write_register(ViPipe, 0x0325, 0x80);
	os08b10_write_register(ViPipe, 0x0327, 0x03);
	os08b10_write_register(ViPipe, 0x0328, 0x05);
	os08b10_write_register(ViPipe, 0x0329, 0x02);
	os08b10_write_register(ViPipe, 0x032a, 0x02);
	os08b10_write_register(ViPipe, 0x032c, 0x01);
	os08b10_write_register(ViPipe, 0x032d, 0x00);
	os08b10_write_register(ViPipe, 0x032e, 0x03);
	os08b10_write_register(ViPipe, 0x032f, 0x01);
	os08b10_write_register(ViPipe, 0x3002, 0x00);
	os08b10_write_register(ViPipe, 0x300f, 0x11);
	os08b10_write_register(ViPipe, 0x3012, 0x41);
	os08b10_write_register(ViPipe, 0x301e, 0xb8);
	os08b10_write_register(ViPipe, 0x3026, 0x12);
	os08b10_write_register(ViPipe, 0x3027, 0x08);
	os08b10_write_register(ViPipe, 0x302c, 0x1a);
	os08b10_write_register(ViPipe, 0x3104, 0x00);
	os08b10_write_register(ViPipe, 0x3106, 0x10);
	os08b10_write_register(ViPipe, 0x3400, 0x00);
	os08b10_write_register(ViPipe, 0x3408, 0x05);
	os08b10_write_register(ViPipe, 0x3409, 0x22);
	os08b10_write_register(ViPipe, 0x340a, 0x02);
	os08b10_write_register(ViPipe, 0x340c, 0x08);
	os08b10_write_register(ViPipe, 0x340d, 0xa4);
	os08b10_write_register(ViPipe, 0x3420, 0x00);
	os08b10_write_register(ViPipe, 0x3421, 0x00);
	os08b10_write_register(ViPipe, 0x3422, 0x00);
	os08b10_write_register(ViPipe, 0x3423, 0x00);
	os08b10_write_register(ViPipe, 0x3424, 0x00);
	os08b10_write_register(ViPipe, 0x3425, 0x00);
	os08b10_write_register(ViPipe, 0x3426, 0x55);
	os08b10_write_register(ViPipe, 0x3427, 0x15);
	os08b10_write_register(ViPipe, 0x3428, 0x55);
	os08b10_write_register(ViPipe, 0x3429, 0x15);
	os08b10_write_register(ViPipe, 0x342a, 0x50);
	os08b10_write_register(ViPipe, 0x342b, 0x00);
	os08b10_write_register(ViPipe, 0x342c, 0x01);
	os08b10_write_register(ViPipe, 0x342d, 0x54);
	os08b10_write_register(ViPipe, 0x342e, 0x01);
	os08b10_write_register(ViPipe, 0x342f, 0x04);
	os08b10_write_register(ViPipe, 0x3501, 0x01);
	os08b10_write_register(ViPipe, 0x3502, 0x40);
	os08b10_write_register(ViPipe, 0x3504, 0x08);
	os08b10_write_register(ViPipe, 0x3541, 0x00);
	os08b10_write_register(ViPipe, 0x3542, 0x40);
	os08b10_write_register(ViPipe, 0x3581, 0x00);
	os08b10_write_register(ViPipe, 0x3582, 0x40);
	os08b10_write_register(ViPipe, 0x3603, 0x30);
	os08b10_write_register(ViPipe, 0x3612, 0x96);
	os08b10_write_register(ViPipe, 0x3613, 0xe0);
	os08b10_write_register(ViPipe, 0x3623, 0x00);
	os08b10_write_register(ViPipe, 0x3624, 0xff);
	os08b10_write_register(ViPipe, 0x3625, 0xff);
	os08b10_write_register(ViPipe, 0x362a, 0x12);
	os08b10_write_register(ViPipe, 0x362b, 0x12);
	os08b10_write_register(ViPipe, 0x362c, 0x12);
	os08b10_write_register(ViPipe, 0x362d, 0x12);
	os08b10_write_register(ViPipe, 0x362e, 0x17);
	os08b10_write_register(ViPipe, 0x362f, 0x2d);
	os08b10_write_register(ViPipe, 0x3630, 0x67);
	os08b10_write_register(ViPipe, 0x3631, 0x7f);
	os08b10_write_register(ViPipe, 0x3638, 0x00);
	os08b10_write_register(ViPipe, 0x3639, 0xff);
	os08b10_write_register(ViPipe, 0x363a, 0xff);
	os08b10_write_register(ViPipe, 0x3643, 0x0a);
	os08b10_write_register(ViPipe, 0x3644, 0x00);
	os08b10_write_register(ViPipe, 0x3645, 0x0a);
	os08b10_write_register(ViPipe, 0x3646, 0x0a);
	os08b10_write_register(ViPipe, 0x3647, 0x06);
	os08b10_write_register(ViPipe, 0x3648, 0x00);
	os08b10_write_register(ViPipe, 0x3649, 0x0a);
	os08b10_write_register(ViPipe, 0x364a, 0x0d);
	os08b10_write_register(ViPipe, 0x364b, 0x02);
	os08b10_write_register(ViPipe, 0x364c, 0x12);
	os08b10_write_register(ViPipe, 0x364d, 0x12);
	os08b10_write_register(ViPipe, 0x364e, 0x12);
	os08b10_write_register(ViPipe, 0x364f, 0x12);
	os08b10_write_register(ViPipe, 0x3650, 0xf7);
	os08b10_write_register(ViPipe, 0x3651, 0x77);
	os08b10_write_register(ViPipe, 0x365a, 0xbb);
	os08b10_write_register(ViPipe, 0x365b, 0x9d);
	os08b10_write_register(ViPipe, 0x365c, 0x8e);
	os08b10_write_register(ViPipe, 0x365d, 0x86);
	os08b10_write_register(ViPipe, 0x3661, 0x07);
	os08b10_write_register(ViPipe, 0x3662, 0x0a);
	os08b10_write_register(ViPipe, 0x3667, 0x54);
	os08b10_write_register(ViPipe, 0x366f, 0x00);
	os08b10_write_register(ViPipe, 0x3671, 0x89);
	os08b10_write_register(ViPipe, 0x3673, 0x2a);
	os08b10_write_register(ViPipe, 0x3678, 0x22);
	os08b10_write_register(ViPipe, 0x3679, 0x00);
	os08b10_write_register(ViPipe, 0x3682, 0xf1);
	os08b10_write_register(ViPipe, 0x3687, 0x02);
	os08b10_write_register(ViPipe, 0x3688, 0x81);
	os08b10_write_register(ViPipe, 0x3689, 0x02);
	os08b10_write_register(ViPipe, 0x3692, 0x0f);
	os08b10_write_register(ViPipe, 0x3701, 0x39);
	os08b10_write_register(ViPipe, 0x3703, 0x32);
	os08b10_write_register(ViPipe, 0x3705, 0x00);
	os08b10_write_register(ViPipe, 0x3706, 0x3a);
	os08b10_write_register(ViPipe, 0x3708, 0x27);
	os08b10_write_register(ViPipe, 0x370a, 0x00);
	os08b10_write_register(ViPipe, 0x370b, 0x8a);
	os08b10_write_register(ViPipe, 0x3712, 0x13);
	os08b10_write_register(ViPipe, 0x3714, 0x02);
	os08b10_write_register(ViPipe, 0x3715, 0x00);
	os08b10_write_register(ViPipe, 0x3716, 0x00);
	os08b10_write_register(ViPipe, 0x3719, 0x23);
	os08b10_write_register(ViPipe, 0x371c, 0x00);
	os08b10_write_register(ViPipe, 0x371d, 0x08);
	os08b10_write_register(ViPipe, 0x373b, 0x50);
	os08b10_write_register(ViPipe, 0x3755, 0x01);
	os08b10_write_register(ViPipe, 0x3756, 0xa9);
	os08b10_write_register(ViPipe, 0x3757, 0xa9);
	os08b10_write_register(ViPipe, 0x376b, 0x02);
	os08b10_write_register(ViPipe, 0x376c, 0x02);
	os08b10_write_register(ViPipe, 0x376d, 0x04);
	os08b10_write_register(ViPipe, 0x376e, 0x04);
	os08b10_write_register(ViPipe, 0x376f, 0x22);
	os08b10_write_register(ViPipe, 0x377a, 0x02);
	os08b10_write_register(ViPipe, 0x377b, 0x01);
	os08b10_write_register(ViPipe, 0x3785, 0x08);
	os08b10_write_register(ViPipe, 0x3790, 0x10);
	os08b10_write_register(ViPipe, 0x3792, 0x00);
	os08b10_write_register(ViPipe, 0x3796, 0x00);
	os08b10_write_register(ViPipe, 0x3797, 0x00);
	os08b10_write_register(ViPipe, 0x3798, 0x00);
	os08b10_write_register(ViPipe, 0x3799, 0x00);
	os08b10_write_register(ViPipe, 0x37bb, 0x88);
	os08b10_write_register(ViPipe, 0x37be, 0x01);
	os08b10_write_register(ViPipe, 0x37bf, 0x01);
	os08b10_write_register(ViPipe, 0x37c0, 0x00);
	os08b10_write_register(ViPipe, 0x37c7, 0x51);
	os08b10_write_register(ViPipe, 0x37c8, 0x22);
	os08b10_write_register(ViPipe, 0x37c9, 0x00);
	os08b10_write_register(ViPipe, 0x37ca, 0xb6);
	os08b10_write_register(ViPipe, 0x37cc, 0x0e);
	os08b10_write_register(ViPipe, 0x37cf, 0x02);
	os08b10_write_register(ViPipe, 0x37d0, 0x00);
	os08b10_write_register(ViPipe, 0x37d1, 0x3a);
	os08b10_write_register(ViPipe, 0x37d2, 0x00);
	os08b10_write_register(ViPipe, 0x37d3, 0x8a);
	os08b10_write_register(ViPipe, 0x37d4, 0x00);
	os08b10_write_register(ViPipe, 0x37d5, 0x3a);
	os08b10_write_register(ViPipe, 0x37d6, 0x00);
	os08b10_write_register(ViPipe, 0x37d7, 0x8a);
	os08b10_write_register(ViPipe, 0x3800, 0x00);
	os08b10_write_register(ViPipe, 0x3801, 0x00);
	os08b10_write_register(ViPipe, 0x3802, 0x00);
	os08b10_write_register(ViPipe, 0x3803, 0x00);
	os08b10_write_register(ViPipe, 0x3804, 0x0f);
	os08b10_write_register(ViPipe, 0x3805, 0x1f);
	os08b10_write_register(ViPipe, 0x3806, 0x08);
	os08b10_write_register(ViPipe, 0x3807, 0x7f);
	os08b10_write_register(ViPipe, 0x3808, 0x0f);
	os08b10_write_register(ViPipe, 0x3809, 0x00);
	os08b10_write_register(ViPipe, 0x380a, 0x08);
	os08b10_write_register(ViPipe, 0x380b, 0x70);
	os08b10_write_register(ViPipe, 0x380c, 0x03);
	os08b10_write_register(ViPipe, 0x380d, 0x2d);
	os08b10_write_register(ViPipe, 0x380e, 0x04);
	os08b10_write_register(ViPipe, 0x380f, 0x52);
	os08b10_write_register(ViPipe, 0x3811, 0x10);
	os08b10_write_register(ViPipe, 0x3813, 0x08);
	os08b10_write_register(ViPipe, 0x3814, 0x01);
	os08b10_write_register(ViPipe, 0x3815, 0x01);
	os08b10_write_register(ViPipe, 0x3816, 0x01);
	os08b10_write_register(ViPipe, 0x3817, 0x01);
	os08b10_write_register(ViPipe, 0x381c, 0x08);
	os08b10_write_register(ViPipe, 0x3820, 0x03);
	os08b10_write_register(ViPipe, 0x3821, 0x00);
	os08b10_write_register(ViPipe, 0x3822, 0x04);
	os08b10_write_register(ViPipe, 0x3823, 0x08);
	os08b10_write_register(ViPipe, 0x382b, 0x00);
	os08b10_write_register(ViPipe, 0x382c, 0x00);
	os08b10_write_register(ViPipe, 0x3833, 0x45);
	os08b10_write_register(ViPipe, 0x383e, 0x00);
	os08b10_write_register(ViPipe, 0x384c, 0x03);
	os08b10_write_register(ViPipe, 0x384d, 0x2d);
	os08b10_write_register(ViPipe, 0x3858, 0x7c);
	os08b10_write_register(ViPipe, 0x3850, 0xff);
	os08b10_write_register(ViPipe, 0x3851, 0xff);
	os08b10_write_register(ViPipe, 0x3852, 0xff);
	os08b10_write_register(ViPipe, 0x3853, 0xff);
	os08b10_write_register(ViPipe, 0x3865, 0x02);
	os08b10_write_register(ViPipe, 0x3866, 0x00);
	os08b10_write_register(ViPipe, 0x3867, 0x02);
	os08b10_write_register(ViPipe, 0x3868, 0x00);
	os08b10_write_register(ViPipe, 0x386a, 0x05);
	os08b10_write_register(ViPipe, 0x386b, 0x00);
	os08b10_write_register(ViPipe, 0x386c, 0x05);
	os08b10_write_register(ViPipe, 0x386d, 0x61);
	os08b10_write_register(ViPipe, 0x3881, 0x02);
	os08b10_write_register(ViPipe, 0x3882, 0x00);
	os08b10_write_register(ViPipe, 0x3883, 0x08);
	os08b10_write_register(ViPipe, 0x3884, 0x01);
	os08b10_write_register(ViPipe, 0x3885, 0x00);
	os08b10_write_register(ViPipe, 0x3886, 0x00);
	os08b10_write_register(ViPipe, 0x3887, 0x20);
	os08b10_write_register(ViPipe, 0x3888, 0x40);
	os08b10_write_register(ViPipe, 0x3c37, 0x00);
	os08b10_write_register(ViPipe, 0x3c49, 0x00);
	os08b10_write_register(ViPipe, 0x3c4a, 0x9a);
	os08b10_write_register(ViPipe, 0x3c4c, 0x01);
	os08b10_write_register(ViPipe, 0x3c4d, 0x00);
	os08b10_write_register(ViPipe, 0x3c4e, 0x00);
	os08b10_write_register(ViPipe, 0x3c50, 0x00);
	os08b10_write_register(ViPipe, 0x3c51, 0x00);
	os08b10_write_register(ViPipe, 0x3c52, 0x00);
	os08b10_write_register(ViPipe, 0x3c67, 0x10);
	os08b10_write_register(ViPipe, 0x3c69, 0x10);
	os08b10_write_register(ViPipe, 0x3c6a, 0x00);
	os08b10_write_register(ViPipe, 0x3c6b, 0x52);
	os08b10_write_register(ViPipe, 0x3c6c, 0x00);
	os08b10_write_register(ViPipe, 0x3c6d, 0x52);
	os08b10_write_register(ViPipe, 0x3c70, 0x00);
	os08b10_write_register(ViPipe, 0x3c71, 0x04);
	os08b10_write_register(ViPipe, 0x3c72, 0x00);
	os08b10_write_register(ViPipe, 0x3c73, 0x52);
	os08b10_write_register(ViPipe, 0x3c74, 0x00);
	os08b10_write_register(ViPipe, 0x3c75, 0x52);
	os08b10_write_register(ViPipe, 0x3c76, 0x12);
	os08b10_write_register(ViPipe, 0x3c77, 0x12);
	os08b10_write_register(ViPipe, 0x3c79, 0x00);
	os08b10_write_register(ViPipe, 0x3c7a, 0x00);
	os08b10_write_register(ViPipe, 0x3c7b, 0x00);
	os08b10_write_register(ViPipe, 0x3cb6, 0x41);
	os08b10_write_register(ViPipe, 0x3cb9, 0x00);
	os08b10_write_register(ViPipe, 0x3cc0, 0x90);
	os08b10_write_register(ViPipe, 0x3cc2, 0x90);
	os08b10_write_register(ViPipe, 0x3cc5, 0x00);
	os08b10_write_register(ViPipe, 0x3cc6, 0x98);
	os08b10_write_register(ViPipe, 0x3cc7, 0x00);
	os08b10_write_register(ViPipe, 0x3cc8, 0x98);
	os08b10_write_register(ViPipe, 0x3cc9, 0x00);
	os08b10_write_register(ViPipe, 0x3cca, 0x98);
	os08b10_write_register(ViPipe, 0x3ccc, 0x01);
	os08b10_write_register(ViPipe, 0x3cd9, 0x7c);
	os08b10_write_register(ViPipe, 0x3cda, 0x7c);
	os08b10_write_register(ViPipe, 0x3cdb, 0x7c);
	os08b10_write_register(ViPipe, 0x3cdc, 0x7c);
	os08b10_write_register(ViPipe, 0x3d8c, 0x70);
	os08b10_write_register(ViPipe, 0x3d8d, 0x10);
	os08b10_write_register(ViPipe, 0x4001, 0xeb);
	os08b10_write_register(ViPipe, 0x4004, 0x00);
	os08b10_write_register(ViPipe, 0x4005, 0x40);
	os08b10_write_register(ViPipe, 0x4008, 0x02);
	os08b10_write_register(ViPipe, 0x4009, 0x0d);
	os08b10_write_register(ViPipe, 0x400a, 0x02);
	os08b10_write_register(ViPipe, 0x400b, 0x00);
	os08b10_write_register(ViPipe, 0x400e, 0x40);
	os08b10_write_register(ViPipe, 0x401a, 0x50);
	os08b10_write_register(ViPipe, 0x4028, 0x6f);
	os08b10_write_register(ViPipe, 0x4029, 0x00);
	os08b10_write_register(ViPipe, 0x402a, 0x7f);
	os08b10_write_register(ViPipe, 0x402b, 0x01);
	os08b10_write_register(ViPipe, 0x402e, 0x00);
	os08b10_write_register(ViPipe, 0x402f, 0x40);
	os08b10_write_register(ViPipe, 0x4030, 0x00);
	os08b10_write_register(ViPipe, 0x4031, 0x40);
	os08b10_write_register(ViPipe, 0x4032, 0x1f);
	os08b10_write_register(ViPipe, 0x4050, 0x01);
	os08b10_write_register(ViPipe, 0x4051, 0x06);
	os08b10_write_register(ViPipe, 0x405d, 0x00);
	os08b10_write_register(ViPipe, 0x405e, 0x00);
	os08b10_write_register(ViPipe, 0x4288, 0xce);
	os08b10_write_register(ViPipe, 0x4289, 0x00);
	os08b10_write_register(ViPipe, 0x428a, 0x40);
	os08b10_write_register(ViPipe, 0x430b, 0x0f);
	os08b10_write_register(ViPipe, 0x430c, 0xfc);
	os08b10_write_register(ViPipe, 0x430d, 0x00);
	os08b10_write_register(ViPipe, 0x430e, 0x00);
	os08b10_write_register(ViPipe, 0x4314, 0x04);
	os08b10_write_register(ViPipe, 0x4500, 0x1a);
	os08b10_write_register(ViPipe, 0x4501, 0x18);
	os08b10_write_register(ViPipe, 0x4504, 0x00);
	os08b10_write_register(ViPipe, 0x4505, 0x00);
	os08b10_write_register(ViPipe, 0x4506, 0x32);
	os08b10_write_register(ViPipe, 0x4507, 0x03);
	os08b10_write_register(ViPipe, 0x4508, 0x1a);
	os08b10_write_register(ViPipe, 0x450a, 0x12);
	os08b10_write_register(ViPipe, 0x450b, 0x28);
	os08b10_write_register(ViPipe, 0x450c, 0x00);
	os08b10_write_register(ViPipe, 0x450f, 0x80);
	os08b10_write_register(ViPipe, 0x4513, 0x00);
	os08b10_write_register(ViPipe, 0x4516, 0x28);
	os08b10_write_register(ViPipe, 0x4603, 0x00);
	os08b10_write_register(ViPipe, 0x460a, 0x50);
	os08b10_write_register(ViPipe, 0x4640, 0x62);
	os08b10_write_register(ViPipe, 0x464d, 0x00);
	os08b10_write_register(ViPipe, 0x4800, 0x04);
	os08b10_write_register(ViPipe, 0x480e, 0x04);
	os08b10_write_register(ViPipe, 0x4813, 0x84);
	os08b10_write_register(ViPipe, 0x4837, 0x0a);
	os08b10_write_register(ViPipe, 0x484b, 0x67);
	os08b10_write_register(ViPipe, 0x4850, 0x47);
	os08b10_write_register(ViPipe, 0x4851, 0xaa);
	os08b10_write_register(ViPipe, 0x4852, 0xff);
	os08b10_write_register(ViPipe, 0x4853, 0x20);
	os08b10_write_register(ViPipe, 0x4854, 0x08);
	os08b10_write_register(ViPipe, 0x4855, 0x30);
	os08b10_write_register(ViPipe, 0x4860, 0x00);
	os08b10_write_register(ViPipe, 0x4d00, 0x4d);
	os08b10_write_register(ViPipe, 0x4d01, 0x42);
	os08b10_write_register(ViPipe, 0x4d02, 0xb9);
	os08b10_write_register(ViPipe, 0x4d03, 0x94);
	os08b10_write_register(ViPipe, 0x4d04, 0x95);
	os08b10_write_register(ViPipe, 0x4d05, 0xce);
	os08b10_write_register(ViPipe, 0x4d09, 0x63);
	os08b10_write_register(ViPipe, 0x5000, 0x0f);
	os08b10_write_register(ViPipe, 0x5001, 0x0d);
	os08b10_write_register(ViPipe, 0x5080, 0x00);
	os08b10_write_register(ViPipe, 0x50c0, 0x00);
	os08b10_write_register(ViPipe, 0x5100, 0x00);
	os08b10_write_register(ViPipe, 0x5200, 0x70);
	os08b10_write_register(ViPipe, 0x5201, 0x70);
	os08b10_write_register(ViPipe, 0x5681, 0x02);
	os08b10_write_register(ViPipe, 0x5780, 0x53);
	os08b10_write_register(ViPipe, 0x5781, 0x03);
	os08b10_write_register(ViPipe, 0x5782, 0x18);
	os08b10_write_register(ViPipe, 0x5783, 0x3c);
	os08b10_write_register(ViPipe, 0x5784, 0x00);
	os08b10_write_register(ViPipe, 0x5785, 0x40);
	os08b10_write_register(ViPipe, 0x5786, 0x01);
	os08b10_write_register(ViPipe, 0x5788, 0x18);
	os08b10_write_register(ViPipe, 0x5789, 0x3c);
	os08b10_write_register(ViPipe, 0x5791, 0x08);
	os08b10_write_register(ViPipe, 0x5792, 0x11);
	os08b10_write_register(ViPipe, 0x5793, 0x33);
	os08b10_default_reg_init(ViPipe);
	os08b10_write_register(ViPipe, 0x0100, 0x01);
	os08b10_write_register(ViPipe, 0x0100, 0x01);

	delay_ms(50);

	printf("===os08b10 sensor 2160P30fps 10bit 2to1 WDR(60fps->30fps) init success!=====\n");
}
