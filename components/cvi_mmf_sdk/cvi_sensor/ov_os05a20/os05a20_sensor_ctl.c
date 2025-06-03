#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "os05a20_cmos_ex.h"
#include "sensor_i2c.h"

static void os05a20_linear_1944p15_12BIT_init(VI_PIPE ViPipe);


const CVI_U32 os05a20_addr_byte = 2;
const CVI_U32 os05a20_data_byte = 1;
ISP_SNS_MIRRORFLIP_TYPE_E g_aeOs05a20_MirrorFip_Initial[VI_MAX_PIPE_NUM] = {
	ISP_SNS_MIRROR, ISP_SNS_MIRROR, ISP_SNS_MIRROR, ISP_SNS_MIRROR};

int os05a20_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunOs05a20_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOs05a20_AddrInfo[ViPipe].s8I2cAddr);
}

int os05a20_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunOs05a20_BusInfo[ViPipe].s8I2cDev);
}

int os05a20_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunOs05a20_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOs05a20_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							os05a20_addr_byte, os05a20_data_byte);
}

int os05a20_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunOs05a20_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunOs05a20_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							os05a20_addr_byte, (CVI_U32)data, os05a20_data_byte);
}
/*
 *static void delay_ms(int ms)
 *{
 *	usleep(ms * 1000);
 *}
 */
void os05a20_standby(VI_PIPE ViPipe)
{
	os05a20_write_register(ViPipe, 0x0100, 0x00); /* STANDBY */
}

void os05a20_restart(VI_PIPE ViPipe)
{
	os05a20_write_register(ViPipe, 0x0100, 0x01); /* resume */
}

void os05a20_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;
	CVI_U32 start = 1;
	CVI_U32 end = g_pastOs05a20[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum - 3;

	for (i = start; i < end; i++) {
		os05a20_write_register(ViPipe,
				g_pastOs05a20[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOs05a20[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		CVI_TRACE_SNS(CVI_DBG_INFO, "i2c_addr:%#x, i2c_data:%#x\n",
			g_pastOs05a20[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
			g_pastOs05a20[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

static ISP_SNS_MIRRORFLIP_TYPE_E aeSnsMirrorFlipMap[ISP_SNS_BUTT][ISP_SNS_BUTT] = {
	{ISP_SNS_NORMAL, ISP_SNS_MIRROR, ISP_SNS_FLIP, ISP_SNS_MIRROR_FLIP},
	{ISP_SNS_MIRROR, ISP_SNS_NORMAL, ISP_SNS_MIRROR_FLIP, ISP_SNS_FLIP},
	{ISP_SNS_FLIP, ISP_SNS_MIRROR_FLIP, ISP_SNS_NORMAL, ISP_SNS_MIRROR},
	{ISP_SNS_MIRROR_FLIP, ISP_SNS_FLIP, ISP_SNS_MIRROR, ISP_SNS_NORMAL}
};

#define OS05A20_ORIEN_ADDR (0x3820)
#define OS05A20_MIRROR_ADDR (0x3821)
void os05a20_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val_flip = 0;
	CVI_U8 val_mirror = 0;
	CVI_U32 i = 0;

	for (i = 0; i < ISP_SNS_BUTT; i++) {
		if (g_aeOs05a20_MirrorFip_Initial[ViPipe] == aeSnsMirrorFlipMap[i][0]) {
			eSnsMirrorFlip = aeSnsMirrorFlipMap[i][eSnsMirrorFlip];
			break;
		}
	}

	val_flip = os05a20_read_register(ViPipe, OS05A20_ORIEN_ADDR);
	val_mirror = os05a20_read_register(ViPipe, OS05A20_MIRROR_ADDR);
	val_flip &= ~(0x1 << 2);
	val_mirror &= ~(0x1 << 2);
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

	os05a20_standby(ViPipe);
	os05a20_write_register(ViPipe, OS05A20_ORIEN_ADDR, val_flip);
	os05a20_write_register(ViPipe, OS05A20_MIRROR_ADDR, val_mirror);
	usleep(1000*100);
	os05a20_restart(ViPipe);
}

#define OS05A20_CHIP_ID_ADDR_H		0x300A
#define OS05A20_CHIP_ID_ADDR_M		0x300B
#define OS05A20_CHIP_ID_ADDR_L		0x300C
#define OS05A20_CHIP_ID			0x530441

int os05a20_probe(VI_PIPE ViPipe)
{
	int nVal, nVal2, nVal3;

	usleep(500);
	if (os05a20_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = os05a20_read_register(ViPipe, OS05A20_CHIP_ID_ADDR_H);
	nVal2 = os05a20_read_register(ViPipe, OS05A20_CHIP_ID_ADDR_M);
	nVal3 = os05a20_read_register(ViPipe, OS05A20_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0 || nVal3 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 16) | ((nVal2 & 0xFF) << 8) | (nVal3 & 0xFF)) != OS05A20_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void os05a20_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	//CVI_U8            u8ImgMode;

	enWDRMode   = g_pastOs05a20[ViPipe]->enWDRMode;
	//u8ImgMode   = g_pastOs05a20[ViPipe]->u8ImgMode;

	os05a20_i2c_init(ViPipe);
	if (enWDRMode == WDR_MODE_2To1_LINE) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Current not support WDR mode\n");
	} else {
			os05a20_linear_1944p15_12BIT_init(ViPipe);
	}
	g_pastOs05a20[ViPipe]->bInit = CVI_TRUE;
}

// void os05a20_exit(VI_PIPE ViPipe)
// {
// 	os05a20_i2c_exit(ViPipe);
// }

static void os05a20_linear_1944p15_12BIT_init(VI_PIPE ViPipe)
{
	os05a20_write_register(ViPipe, 0x0103, 0x01);
	os05a20_write_register(ViPipe, 0x0303, 0x01);
	os05a20_write_register(ViPipe, 0x0305, 0x35);
	os05a20_write_register(ViPipe, 0x0306, 0x00);
	os05a20_write_register(ViPipe, 0x0307, 0x00);
	os05a20_write_register(ViPipe, 0x0308, 0x03);
	os05a20_write_register(ViPipe, 0x0309, 0x04);
	os05a20_write_register(ViPipe, 0x032a, 0x00);
	os05a20_write_register(ViPipe, 0x031e, 0x0a);
	os05a20_write_register(ViPipe, 0x0325, 0x50);
	os05a20_write_register(ViPipe, 0x0328, 0x07);
	os05a20_write_register(ViPipe, 0x300d, 0x11);
	os05a20_write_register(ViPipe, 0x300e, 0x11);
	os05a20_write_register(ViPipe, 0x300f, 0x11);
	os05a20_write_register(ViPipe, 0x3026, 0x00);
	os05a20_write_register(ViPipe, 0x3027, 0x00);
	os05a20_write_register(ViPipe, 0x3010, 0x01);
	os05a20_write_register(ViPipe, 0x3012, 0x21);
	os05a20_write_register(ViPipe, 0x3016, 0xf0);
	os05a20_write_register(ViPipe, 0x3018, 0xf0);
	os05a20_write_register(ViPipe, 0x3028, 0xf0);
	os05a20_write_register(ViPipe, 0x301e, 0x98);
	os05a20_write_register(ViPipe, 0x3010, 0x01);
	os05a20_write_register(ViPipe, 0x3011, 0x04);
	os05a20_write_register(ViPipe, 0x3031, 0xa9);
	os05a20_write_register(ViPipe, 0x3103, 0x48);
	os05a20_write_register(ViPipe, 0x3104, 0x01);
	os05a20_write_register(ViPipe, 0x3106, 0x10);
	os05a20_write_register(ViPipe, 0x3400, 0x04);
	os05a20_write_register(ViPipe, 0x3025, 0x03);
	os05a20_write_register(ViPipe, 0x3425, 0x01);
	os05a20_write_register(ViPipe, 0x3428, 0x01);
	os05a20_write_register(ViPipe, 0x3406, 0x08);
	os05a20_write_register(ViPipe, 0x3408, 0x03);
	os05a20_write_register(ViPipe, 0x3501, 0x09);
	os05a20_write_register(ViPipe, 0x3502, 0xa0);
	os05a20_write_register(ViPipe, 0x3505, 0x83);
	os05a20_write_register(ViPipe, 0x3508, 0x00);
	os05a20_write_register(ViPipe, 0x3509, 0x80);
	os05a20_write_register(ViPipe, 0x350a, 0x04);
	os05a20_write_register(ViPipe, 0x350b, 0x00);
	os05a20_write_register(ViPipe, 0x350c, 0x00);
	os05a20_write_register(ViPipe, 0x350d, 0x80);
	os05a20_write_register(ViPipe, 0x350e, 0x04);
	os05a20_write_register(ViPipe, 0x350f, 0x00);
	os05a20_write_register(ViPipe, 0x3600, 0x00);
	os05a20_write_register(ViPipe, 0x3626, 0xff);
	os05a20_write_register(ViPipe, 0x3605, 0x50);
	os05a20_write_register(ViPipe, 0x3609, 0xdb);
	os05a20_write_register(ViPipe, 0x3610, 0x69);
	os05a20_write_register(ViPipe, 0x360c, 0x01);
	os05a20_write_register(ViPipe, 0x3628, 0xa4);
	os05a20_write_register(ViPipe, 0x3629, 0x6a);
	os05a20_write_register(ViPipe, 0x362d, 0x10);
	os05a20_write_register(ViPipe, 0x3660, 0xd3);
	os05a20_write_register(ViPipe, 0x3661, 0x06);
	os05a20_write_register(ViPipe, 0x3662, 0x00);
	os05a20_write_register(ViPipe, 0x3663, 0x28);
	os05a20_write_register(ViPipe, 0x3664, 0x0d);
	os05a20_write_register(ViPipe, 0x366a, 0x38);
	os05a20_write_register(ViPipe, 0x366b, 0xa0);
	os05a20_write_register(ViPipe, 0x366d, 0x00);
	os05a20_write_register(ViPipe, 0x366e, 0x00);
	os05a20_write_register(ViPipe, 0x3680, 0x00);
	os05a20_write_register(ViPipe, 0x36c0, 0x00);
	os05a20_write_register(ViPipe, 0x3621, 0x81);
	os05a20_write_register(ViPipe, 0x3634, 0x31);
	os05a20_write_register(ViPipe, 0x3620, 0x00);
	os05a20_write_register(ViPipe, 0x3622, 0x00);
	os05a20_write_register(ViPipe, 0x362a, 0xd0);
	os05a20_write_register(ViPipe, 0x362e, 0x8c);
	os05a20_write_register(ViPipe, 0x362f, 0x98);
	os05a20_write_register(ViPipe, 0x3630, 0xb0);
	os05a20_write_register(ViPipe, 0x3631, 0xd7);
	os05a20_write_register(ViPipe, 0x3701, 0x0f);
	os05a20_write_register(ViPipe, 0x3737, 0x02);
	os05a20_write_register(ViPipe, 0x3740, 0x18);
	os05a20_write_register(ViPipe, 0x3741, 0x04);
	os05a20_write_register(ViPipe, 0x373c, 0x0f);
	os05a20_write_register(ViPipe, 0x373b, 0x02);
	os05a20_write_register(ViPipe, 0x3705, 0x00);
	os05a20_write_register(ViPipe, 0x3706, 0xa0);
	os05a20_write_register(ViPipe, 0x370a, 0x01);
	os05a20_write_register(ViPipe, 0x370b, 0xc8);
	os05a20_write_register(ViPipe, 0x3709, 0x4a);
	os05a20_write_register(ViPipe, 0x3714, 0x21);
	os05a20_write_register(ViPipe, 0x371c, 0x00);
	os05a20_write_register(ViPipe, 0x371d, 0x08);
	os05a20_write_register(ViPipe, 0x375e, 0x0e);
	os05a20_write_register(ViPipe, 0x3760, 0x13);
	os05a20_write_register(ViPipe, 0x3776, 0x10);
	os05a20_write_register(ViPipe, 0x3781, 0x02);
	os05a20_write_register(ViPipe, 0x3782, 0x04);
	os05a20_write_register(ViPipe, 0x3783, 0x02);
	os05a20_write_register(ViPipe, 0x3784, 0x08);
	os05a20_write_register(ViPipe, 0x3785, 0x08);
	os05a20_write_register(ViPipe, 0x3788, 0x01);
	os05a20_write_register(ViPipe, 0x3789, 0x01);
	os05a20_write_register(ViPipe, 0x3797, 0x04);
	os05a20_write_register(ViPipe, 0x3798, 0x01);
	os05a20_write_register(ViPipe, 0x3799, 0x00);
	os05a20_write_register(ViPipe, 0x3761, 0x02);
	os05a20_write_register(ViPipe, 0x3762, 0x0d);
	os05a20_write_register(ViPipe, 0x3800, 0x00);
	os05a20_write_register(ViPipe, 0x3801, 0x00);
	os05a20_write_register(ViPipe, 0x3802, 0x00);
	os05a20_write_register(ViPipe, 0x3803, 0x0c);
	os05a20_write_register(ViPipe, 0x3804, 0x0e);
	os05a20_write_register(ViPipe, 0x3805, 0xff);
	os05a20_write_register(ViPipe, 0x3806, 0x08);
	os05a20_write_register(ViPipe, 0x3807, 0x6f);
	os05a20_write_register(ViPipe, 0x3808, 0x0a);
	os05a20_write_register(ViPipe, 0x3809, 0x80);
	os05a20_write_register(ViPipe, 0x380a, 0x07);
	os05a20_write_register(ViPipe, 0x380b, 0x98);
	os05a20_write_register(ViPipe, 0x380c, 0x02);
	os05a20_write_register(ViPipe, 0x380d, 0xd0);
	os05a20_write_register(ViPipe, 0x380e, 0x09);
	os05a20_write_register(ViPipe, 0x380f, 0xc0);
	os05a20_write_register(ViPipe, 0x3811, 0x10);
	os05a20_write_register(ViPipe, 0x3813, 0x04);
	os05a20_write_register(ViPipe, 0x3814, 0x01);
	os05a20_write_register(ViPipe, 0x3815, 0x01);
	os05a20_write_register(ViPipe, 0x3816, 0x01);
	os05a20_write_register(ViPipe, 0x3817, 0x01);
	os05a20_write_register(ViPipe, 0x381c, 0x00);
	os05a20_write_register(ViPipe, 0x3820, 0x00);
	os05a20_write_register(ViPipe, 0x3821, 0x04);
	os05a20_write_register(ViPipe, 0x3822, 0x54);
	os05a20_write_register(ViPipe, 0x3823, 0x18);
	os05a20_write_register(ViPipe, 0x3826, 0x00);
	os05a20_write_register(ViPipe, 0x3827, 0x01);
	os05a20_write_register(ViPipe, 0x3833, 0x00);
	os05a20_write_register(ViPipe, 0x3832, 0x02);
	os05a20_write_register(ViPipe, 0x383c, 0x48);
	os05a20_write_register(ViPipe, 0x383d, 0xff);
	os05a20_write_register(ViPipe, 0x3843, 0x20);
	os05a20_write_register(ViPipe, 0x382d, 0x08);
	os05a20_write_register(ViPipe, 0x3d85, 0x0b);
	os05a20_write_register(ViPipe, 0x3d84, 0x40);
	os05a20_write_register(ViPipe, 0x3d8c, 0x63);
	os05a20_write_register(ViPipe, 0x3d8d, 0x00);
	os05a20_write_register(ViPipe, 0x4000, 0x78);
	os05a20_write_register(ViPipe, 0x4001, 0x2b);
	os05a20_write_register(ViPipe, 0x4004, 0x01);
	os05a20_write_register(ViPipe, 0x4005, 0x00);
	os05a20_write_register(ViPipe, 0x4028, 0x2f);
	os05a20_write_register(ViPipe, 0x400a, 0x01);
	os05a20_write_register(ViPipe, 0x4010, 0x12);
	os05a20_write_register(ViPipe, 0x4008, 0x02);
	os05a20_write_register(ViPipe, 0x4009, 0x0d);
	os05a20_write_register(ViPipe, 0x401a, 0x58);
	os05a20_write_register(ViPipe, 0x4050, 0x00);
	os05a20_write_register(ViPipe, 0x4051, 0x01);
	os05a20_write_register(ViPipe, 0x4052, 0x00);
	os05a20_write_register(ViPipe, 0x4053, 0x80);
	os05a20_write_register(ViPipe, 0x4054, 0x00);
	os05a20_write_register(ViPipe, 0x4055, 0x80);
	os05a20_write_register(ViPipe, 0x4056, 0x00);
	os05a20_write_register(ViPipe, 0x4057, 0x80);
	os05a20_write_register(ViPipe, 0x4058, 0x00);
	os05a20_write_register(ViPipe, 0x4059, 0x80);
	os05a20_write_register(ViPipe, 0x430b, 0xff);
	os05a20_write_register(ViPipe, 0x430c, 0xff);
	os05a20_write_register(ViPipe, 0x430d, 0x00);
	os05a20_write_register(ViPipe, 0x430e, 0x00);
	os05a20_write_register(ViPipe, 0x4501, 0x18);
	os05a20_write_register(ViPipe, 0x4502, 0x00);
	os05a20_write_register(ViPipe, 0x4600, 0x00);
	os05a20_write_register(ViPipe, 0x4601, 0x10);
	os05a20_write_register(ViPipe, 0x4603, 0x01);
	os05a20_write_register(ViPipe, 0x4643, 0x00);
	os05a20_write_register(ViPipe, 0x4640, 0x01);
	os05a20_write_register(ViPipe, 0x4641, 0x04);
	os05a20_write_register(ViPipe, 0x480e, 0x00);
	os05a20_write_register(ViPipe, 0x4813, 0x00);
	os05a20_write_register(ViPipe, 0x4815, 0x2b);
	os05a20_write_register(ViPipe, 0x486e, 0x36);
	os05a20_write_register(ViPipe, 0x486f, 0x84);
	os05a20_write_register(ViPipe, 0x4860, 0x00);
	os05a20_write_register(ViPipe, 0x4861, 0xa0);
	os05a20_write_register(ViPipe, 0x484b, 0x05);
	os05a20_write_register(ViPipe, 0x4850, 0x00);
	os05a20_write_register(ViPipe, 0x4851, 0xaa);
	os05a20_write_register(ViPipe, 0x4852, 0xff);
	os05a20_write_register(ViPipe, 0x4853, 0x8a);
	os05a20_write_register(ViPipe, 0x4854, 0x08);
	os05a20_write_register(ViPipe, 0x4855, 0x30);
	os05a20_write_register(ViPipe, 0x4800, 0x60);
	os05a20_write_register(ViPipe, 0x4837, 0x12);
	os05a20_write_register(ViPipe, 0x484a, 0x3f);
	os05a20_write_register(ViPipe, 0x5000, 0xc9);
	os05a20_write_register(ViPipe, 0x5001, 0x43);
	os05a20_write_register(ViPipe, 0x5002, 0x00);
	os05a20_write_register(ViPipe, 0x5211, 0x03);
	os05a20_write_register(ViPipe, 0x5291, 0x03);
	os05a20_write_register(ViPipe, 0x520d, 0x0f);
	os05a20_write_register(ViPipe, 0x520e, 0xfd);
	os05a20_write_register(ViPipe, 0x520f, 0xa5);
	os05a20_write_register(ViPipe, 0x5210, 0xa5);
	os05a20_write_register(ViPipe, 0x528d, 0x0f);
	os05a20_write_register(ViPipe, 0x528e, 0xfd);
	os05a20_write_register(ViPipe, 0x528f, 0xa5);
	os05a20_write_register(ViPipe, 0x5290, 0xa5);
	os05a20_write_register(ViPipe, 0x5004, 0x40);
	os05a20_write_register(ViPipe, 0x5005, 0x00);
	os05a20_write_register(ViPipe, 0x5180, 0x00);
	os05a20_write_register(ViPipe, 0x5181, 0x10);
	os05a20_write_register(ViPipe, 0x5182, 0x0f);
	os05a20_write_register(ViPipe, 0x5183, 0xff);
	os05a20_write_register(ViPipe, 0x580b, 0x03);
	os05a20_write_register(ViPipe, 0x4d00, 0x03);
	os05a20_write_register(ViPipe, 0x4d01, 0xe9);
	os05a20_write_register(ViPipe, 0x4d02, 0xba);
	os05a20_write_register(ViPipe, 0x4d03, 0x66);
	os05a20_write_register(ViPipe, 0x4d04, 0x46);
	os05a20_write_register(ViPipe, 0x4d05, 0xa5);
	os05a20_write_register(ViPipe, 0x3603, 0x3c);
	os05a20_write_register(ViPipe, 0x3703, 0x26);
	os05a20_write_register(ViPipe, 0x3709, 0x49);
	os05a20_write_register(ViPipe, 0x3708, 0x2d);
	os05a20_write_register(ViPipe, 0x3719, 0x1c);
	os05a20_write_register(ViPipe, 0x371a, 0x06);
	os05a20_write_register(ViPipe, 0x4000, 0x79);
	os05a20_write_register(ViPipe, 0x380c, 0x09);
	os05a20_write_register(ViPipe, 0x380d, 0x3a);
	os05a20_write_register(ViPipe, 0x380e, 0x0d);
	os05a20_write_register(ViPipe, 0x380f, 0x3a);
	os05a20_write_register(ViPipe, 0x3501, 0x0d);
	os05a20_write_register(ViPipe, 0x3502, 0xa5);
	os05a20_write_register(ViPipe, 0x4603, 0x00);
	os05a20_write_register(ViPipe, 0x4028, 0x6f);
	os05a20_write_register(ViPipe, 0x4029, 0x0f);
	os05a20_write_register(ViPipe, 0x402a, 0x3f);
	os05a20_write_register(ViPipe, 0x402b, 0x01);

	os05a20_default_reg_init(ViPipe);
	os05a20_write_register(ViPipe, 0x0100, 0x01);
	os05a20_write_register(ViPipe, 0x0100, 0x01);

	usleep(200 * 1000);

	printf("ViPipe:%d,===OS05A20 1944P 15fps 12bit LINE Init OK!===\n", ViPipe);
}

