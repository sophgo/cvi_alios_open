#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc635ai_cmos_ex.h"
#include "sensor_i2c.h"

#define SC635AI_PATTERN_ENABLE	0

static void sc635ai_linear_1800p60_init(VI_PIPE ViPipe);

CVI_U8 sc635ai_i2c_addr = 0x60;        /* I2C Address of SC635AI */
const CVI_U32 sc635ai_addr_byte = 2;
const CVI_U32 sc635ai_data_byte = 1;

int sc635ai_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC635AI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC635AI_AddrInfo[ViPipe].s8I2cAddr);
}

int sc635ai_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC635AI_BusInfo[ViPipe].s8I2cDev);
}

int sc635ai_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC635AI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC635AI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc635ai_addr_byte, sc635ai_data_byte);
}

int sc635ai_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC635AI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC635AI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc635ai_addr_byte, (CVI_U32)data, sc635ai_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc635ai_standby(VI_PIPE ViPipe)
{
	sc635ai_write_register(ViPipe, 0x0100, 0x00);
}

void sc635ai_restart(VI_PIPE ViPipe)
{
	sc635ai_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc635ai_write_register(ViPipe, 0x0100, 0x01);
}

void sc635ai_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC635AI[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc635ai_write_register(ViPipe,
				g_pastSC635AI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC635AI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define SC635AI_CHIP_ID_HI_ADDR		0x3107
#define SC635AI_CHIP_ID_LO_ADDR		0x3108
#define SC635AI_CHIP_ID			0xce7c

void sc635ai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x6;
		break;
	case ISP_SNS_FLIP:
		val |= 0x60;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x66;
		break;
	default:
		return;
	}

	sc635ai_write_register(ViPipe, 0x3221, val);
}

int sc635ai_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc635ai_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc635ai_read_register(ViPipe, SC635AI_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc635ai_read_register(ViPipe, SC635AI_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC635AI_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc635ai_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode = g_pastSC635AI[ViPipe]->u8ImgMode;

	sc635ai_i2c_init(ViPipe);

	if (u8ImgMode == SC635AI_MODE_1800P60)
		sc635ai_linear_1800p60_init(ViPipe);

	g_pastSC635AI[ViPipe]->bInit = CVI_TRUE;
}

/* 1800P60 */
static void sc635ai_linear_1800p60_init(VI_PIPE ViPipe)
{
	sc635ai_write_register(ViPipe, 0x3105, 0x32);
	sc635ai_write_register(ViPipe, 0x0103, 0x01);
	sc635ai_write_register(ViPipe, 0x0100, 0x00);
	sc635ai_write_register(ViPipe, 0x302c, 0x0c);
	sc635ai_write_register(ViPipe, 0x302c, 0x00);
	sc635ai_write_register(ViPipe, 0x3105, 0x12);
	sc635ai_write_register(ViPipe, 0x23b0, 0x00);
	sc635ai_write_register(ViPipe, 0x23b1, 0x08);
	sc635ai_write_register(ViPipe, 0x23b2, 0x00);
	sc635ai_write_register(ViPipe, 0x23b3, 0x18);
	sc635ai_write_register(ViPipe, 0x23b4, 0x00);
	sc635ai_write_register(ViPipe, 0x23b5, 0x38);
	sc635ai_write_register(ViPipe, 0x23b6, 0x04);
	sc635ai_write_register(ViPipe, 0x23b7, 0x08);
	sc635ai_write_register(ViPipe, 0x23b8, 0x04);
	sc635ai_write_register(ViPipe, 0x23b9, 0x18);
	sc635ai_write_register(ViPipe, 0x23ba, 0x04);
	sc635ai_write_register(ViPipe, 0x23bb, 0x38);
	sc635ai_write_register(ViPipe, 0x23bc, 0x04);
	sc635ai_write_register(ViPipe, 0x23bd, 0x08);
	sc635ai_write_register(ViPipe, 0x23be, 0x04);
	sc635ai_write_register(ViPipe, 0x23bf, 0x78);
	sc635ai_write_register(ViPipe, 0x23c0, 0x04);
	sc635ai_write_register(ViPipe, 0x23c1, 0x00);
	sc635ai_write_register(ViPipe, 0x23c2, 0x04);
	sc635ai_write_register(ViPipe, 0x23c3, 0x18);
	sc635ai_write_register(ViPipe, 0x23c4, 0x04);
	sc635ai_write_register(ViPipe, 0x23c5, 0x78);
	sc635ai_write_register(ViPipe, 0x23c6, 0x04);
	sc635ai_write_register(ViPipe, 0x23c7, 0x08);
	sc635ai_write_register(ViPipe, 0x23c8, 0x04);
	sc635ai_write_register(ViPipe, 0x23c9, 0x78);
	sc635ai_write_register(ViPipe, 0x3018, 0x7b);
	sc635ai_write_register(ViPipe, 0x301e, 0xf0);
	sc635ai_write_register(ViPipe, 0x301f, 0x01);
	sc635ai_write_register(ViPipe, 0x302c, 0x00);
	sc635ai_write_register(ViPipe, 0x30b0, 0x01);
	sc635ai_write_register(ViPipe, 0x30b8, 0x44);
	sc635ai_write_register(ViPipe, 0x3204, 0x0c);
	sc635ai_write_register(ViPipe, 0x3205, 0x87);
	sc635ai_write_register(ViPipe, 0x3206, 0x07);
	sc635ai_write_register(ViPipe, 0x3207, 0x0f);
	sc635ai_write_register(ViPipe, 0x3208, 0x0c);
	sc635ai_write_register(ViPipe, 0x3209, 0x80);
	sc635ai_write_register(ViPipe, 0x320a, 0x07);
	sc635ai_write_register(ViPipe, 0x320b, 0x08);
	sc635ai_write_register(ViPipe, 0x320c, 0x03);
	sc635ai_write_register(ViPipe, 0x320d, 0xc0);
	sc635ai_write_register(ViPipe, 0x320e, 0x07);
	sc635ai_write_register(ViPipe, 0x320f, 0x53);
	sc635ai_write_register(ViPipe, 0x3211, 0x04);
	sc635ai_write_register(ViPipe, 0x3213, 0x04);
	sc635ai_write_register(ViPipe, 0x3214, 0x11);
	sc635ai_write_register(ViPipe, 0x3215, 0x11);
	sc635ai_write_register(ViPipe, 0x3223, 0xc0);
	sc635ai_write_register(ViPipe, 0x3250, 0x00);
	sc635ai_write_register(ViPipe, 0x3271, 0x10);
	sc635ai_write_register(ViPipe, 0x327f, 0x3f);
	sc635ai_write_register(ViPipe, 0x32e0, 0x00);
	sc635ai_write_register(ViPipe, 0x3301, 0x12);
	sc635ai_write_register(ViPipe, 0x3304, 0x50);
	sc635ai_write_register(ViPipe, 0x3305, 0x00);
	sc635ai_write_register(ViPipe, 0x3306, 0x70);
	sc635ai_write_register(ViPipe, 0x3308, 0x18);
	sc635ai_write_register(ViPipe, 0x3309, 0xb0);
	sc635ai_write_register(ViPipe, 0x330a, 0x01);
	sc635ai_write_register(ViPipe, 0x330b, 0x20);
	sc635ai_write_register(ViPipe, 0x331e, 0x39);
	sc635ai_write_register(ViPipe, 0x331f, 0x99);
	sc635ai_write_register(ViPipe, 0x3333, 0x10);
	sc635ai_write_register(ViPipe, 0x3334, 0x40);
	sc635ai_write_register(ViPipe, 0x3364, 0x5e);
	sc635ai_write_register(ViPipe, 0x338f, 0xa0);
	sc635ai_write_register(ViPipe, 0x3393, 0x18);
	sc635ai_write_register(ViPipe, 0x3394, 0x2c);
	sc635ai_write_register(ViPipe, 0x3395, 0x3c);
	sc635ai_write_register(ViPipe, 0x3399, 0x12);
	sc635ai_write_register(ViPipe, 0x339a, 0x16);
	sc635ai_write_register(ViPipe, 0x339b, 0x1e);
	sc635ai_write_register(ViPipe, 0x339c, 0x2e);
	sc635ai_write_register(ViPipe, 0x33ac, 0x0c);
	sc635ai_write_register(ViPipe, 0x33ad, 0x2c);
	sc635ai_write_register(ViPipe, 0x33ae, 0x30);
	sc635ai_write_register(ViPipe, 0x33af, 0x90);
	sc635ai_write_register(ViPipe, 0x33b0, 0x0f);
	sc635ai_write_register(ViPipe, 0x33b2, 0x24);
	sc635ai_write_register(ViPipe, 0x33b3, 0x10);
	sc635ai_write_register(ViPipe, 0x33f8, 0x00);
	sc635ai_write_register(ViPipe, 0x33f9, 0x70);
	sc635ai_write_register(ViPipe, 0x33fa, 0x00);
	sc635ai_write_register(ViPipe, 0x33fb, 0x70);
	sc635ai_write_register(ViPipe, 0x349f, 0x03);
	sc635ai_write_register(ViPipe, 0x34a8, 0x10);
	sc635ai_write_register(ViPipe, 0x34a9, 0x10);
	sc635ai_write_register(ViPipe, 0x34aa, 0x01);
	sc635ai_write_register(ViPipe, 0x34ab, 0x20);
	sc635ai_write_register(ViPipe, 0x34ac, 0x01);
	sc635ai_write_register(ViPipe, 0x34ad, 0x20);
	sc635ai_write_register(ViPipe, 0x34f9, 0x12);
	sc635ai_write_register(ViPipe, 0x3632, 0x6d);
	sc635ai_write_register(ViPipe, 0x3633, 0x4d);
	sc635ai_write_register(ViPipe, 0x363a, 0x80);
	sc635ai_write_register(ViPipe, 0x363b, 0x57);
	sc635ai_write_register(ViPipe, 0x363c, 0xd8);
	sc635ai_write_register(ViPipe, 0x363d, 0x40);
	sc635ai_write_register(ViPipe, 0x3670, 0x41);
	sc635ai_write_register(ViPipe, 0x3671, 0x31);
	sc635ai_write_register(ViPipe, 0x3672, 0x31);
	sc635ai_write_register(ViPipe, 0x3673, 0x04);
	sc635ai_write_register(ViPipe, 0x3674, 0x08);
	sc635ai_write_register(ViPipe, 0x3675, 0x04);
	sc635ai_write_register(ViPipe, 0x3676, 0x18);
	sc635ai_write_register(ViPipe, 0x367e, 0x69);
	sc635ai_write_register(ViPipe, 0x367f, 0x6d);
	sc635ai_write_register(ViPipe, 0x3680, 0x8d);
	sc635ai_write_register(ViPipe, 0x3681, 0x04);
	sc635ai_write_register(ViPipe, 0x3682, 0x08);
	sc635ai_write_register(ViPipe, 0x3683, 0x04);
	sc635ai_write_register(ViPipe, 0x3684, 0x78);
	sc635ai_write_register(ViPipe, 0x3685, 0x80);
	sc635ai_write_register(ViPipe, 0x3686, 0x80);
	sc635ai_write_register(ViPipe, 0x3687, 0x83);
	sc635ai_write_register(ViPipe, 0x3688, 0x82);
	sc635ai_write_register(ViPipe, 0x3689, 0x85);
	sc635ai_write_register(ViPipe, 0x368a, 0x8b);
	sc635ai_write_register(ViPipe, 0x368b, 0x97);
	sc635ai_write_register(ViPipe, 0x368c, 0xae);
	sc635ai_write_register(ViPipe, 0x368d, 0x00);
	sc635ai_write_register(ViPipe, 0x368e, 0x08);
	sc635ai_write_register(ViPipe, 0x368f, 0x00);
	sc635ai_write_register(ViPipe, 0x3690, 0x18);
	sc635ai_write_register(ViPipe, 0x3691, 0x04);
	sc635ai_write_register(ViPipe, 0x3692, 0x00);
	sc635ai_write_register(ViPipe, 0x3693, 0x04);
	sc635ai_write_register(ViPipe, 0x3694, 0x08);
	sc635ai_write_register(ViPipe, 0x3695, 0x04);
	sc635ai_write_register(ViPipe, 0x3696, 0x18);
	sc635ai_write_register(ViPipe, 0x3697, 0x04);
	sc635ai_write_register(ViPipe, 0x3698, 0x38);
	sc635ai_write_register(ViPipe, 0x3699, 0x04);
	sc635ai_write_register(ViPipe, 0x369a, 0x78);
	sc635ai_write_register(ViPipe, 0x36d0, 0x0d);
	sc635ai_write_register(ViPipe, 0x36ea, 0x14);
	sc635ai_write_register(ViPipe, 0x36eb, 0x45);
	sc635ai_write_register(ViPipe, 0x36ec, 0x4b);
	sc635ai_write_register(ViPipe, 0x36ed, 0x18);
	sc635ai_write_register(ViPipe, 0x370f, 0x13);
	sc635ai_write_register(ViPipe, 0x3721, 0x6c);
	sc635ai_write_register(ViPipe, 0x3722, 0x8b);
	sc635ai_write_register(ViPipe, 0x3724, 0xc1);
	sc635ai_write_register(ViPipe, 0x3726, 0x01);
	sc635ai_write_register(ViPipe, 0x3727, 0x24);
	sc635ai_write_register(ViPipe, 0x3729, 0xb4);
	sc635ai_write_register(ViPipe, 0x37b0, 0x77);
	sc635ai_write_register(ViPipe, 0x37b1, 0x77);
	sc635ai_write_register(ViPipe, 0x37b2, 0x77);
	sc635ai_write_register(ViPipe, 0x37b3, 0x04);
	sc635ai_write_register(ViPipe, 0x37b4, 0x08);
	sc635ai_write_register(ViPipe, 0x37b5, 0x04);
	sc635ai_write_register(ViPipe, 0x37b6, 0x38);
	sc635ai_write_register(ViPipe, 0x37b7, 0x13);
	sc635ai_write_register(ViPipe, 0x37b8, 0x00);
	sc635ai_write_register(ViPipe, 0x37b9, 0x00);
	sc635ai_write_register(ViPipe, 0x37ba, 0xc4);
	sc635ai_write_register(ViPipe, 0x37bb, 0xc4);
	sc635ai_write_register(ViPipe, 0x37bc, 0xc4);
	sc635ai_write_register(ViPipe, 0x37bd, 0x04);
	sc635ai_write_register(ViPipe, 0x37be, 0x08);
	sc635ai_write_register(ViPipe, 0x37bf, 0x04);
	sc635ai_write_register(ViPipe, 0x37c0, 0x38);
	sc635ai_write_register(ViPipe, 0x37c1, 0x04);
	sc635ai_write_register(ViPipe, 0x37c2, 0x08);
	sc635ai_write_register(ViPipe, 0x37c3, 0x04);
	sc635ai_write_register(ViPipe, 0x37c4, 0x38);
	sc635ai_write_register(ViPipe, 0x37fa, 0x18);
	sc635ai_write_register(ViPipe, 0x37fb, 0x55);
	sc635ai_write_register(ViPipe, 0x37fc, 0x19);
	sc635ai_write_register(ViPipe, 0x37fd, 0x1a);
	sc635ai_write_register(ViPipe, 0x3900, 0x05);
	sc635ai_write_register(ViPipe, 0x3903, 0x60);
	sc635ai_write_register(ViPipe, 0x3905, 0x0d);
	sc635ai_write_register(ViPipe, 0x391a, 0x60);
	sc635ai_write_register(ViPipe, 0x391b, 0x40);
	sc635ai_write_register(ViPipe, 0x391c, 0x26);
	sc635ai_write_register(ViPipe, 0x391d, 0x00);
	sc635ai_write_register(ViPipe, 0x3926, 0xe0);
	sc635ai_write_register(ViPipe, 0x3933, 0x80);
	sc635ai_write_register(ViPipe, 0x3934, 0x06);
	sc635ai_write_register(ViPipe, 0x3935, 0x00);
	sc635ai_write_register(ViPipe, 0x3936, 0x72);
	sc635ai_write_register(ViPipe, 0x3937, 0x71);
	sc635ai_write_register(ViPipe, 0x3938, 0x75);
	sc635ai_write_register(ViPipe, 0x3939, 0x0f);
	sc635ai_write_register(ViPipe, 0x393a, 0xf3);
	sc635ai_write_register(ViPipe, 0x393b, 0x0f);
	sc635ai_write_register(ViPipe, 0x393c, 0xd8);
	sc635ai_write_register(ViPipe, 0x393f, 0x80);
	sc635ai_write_register(ViPipe, 0x3940, 0x0b);
	sc635ai_write_register(ViPipe, 0x3941, 0x00);
	sc635ai_write_register(ViPipe, 0x3942, 0x0b);
	sc635ai_write_register(ViPipe, 0x3943, 0x7e);
	sc635ai_write_register(ViPipe, 0x3944, 0x7f);
	sc635ai_write_register(ViPipe, 0x3945, 0x7f);
	sc635ai_write_register(ViPipe, 0x3946, 0x7e);
	sc635ai_write_register(ViPipe, 0x39dd, 0x00);
	sc635ai_write_register(ViPipe, 0x39de, 0x08);
	sc635ai_write_register(ViPipe, 0x39e7, 0x04);
	sc635ai_write_register(ViPipe, 0x39e8, 0x04);
	sc635ai_write_register(ViPipe, 0x39e9, 0x80);
	sc635ai_write_register(ViPipe, 0x3e00, 0x00);
	sc635ai_write_register(ViPipe, 0x3e01, 0x74);
	sc635ai_write_register(ViPipe, 0x3e02, 0xb0);
	sc635ai_write_register(ViPipe, 0x3e03, 0x0b);
	sc635ai_write_register(ViPipe, 0x3e08, 0x00);
	sc635ai_write_register(ViPipe, 0x3e16, 0x01);
	sc635ai_write_register(ViPipe, 0x3e17, 0x54);
	sc635ai_write_register(ViPipe, 0x3e18, 0x01);
	sc635ai_write_register(ViPipe, 0x3e19, 0x54);
	sc635ai_write_register(ViPipe, 0x4402, 0x11);
	sc635ai_write_register(ViPipe, 0x450a, 0x80);
	sc635ai_write_register(ViPipe, 0x450d, 0x0a);
	sc635ai_write_register(ViPipe, 0x4800, 0x24);
	sc635ai_write_register(ViPipe, 0x480f, 0x03);
	sc635ai_write_register(ViPipe, 0x4837, 0x1d);
	sc635ai_write_register(ViPipe, 0x5000, 0x26);
	sc635ai_write_register(ViPipe, 0x5780, 0x76);
	sc635ai_write_register(ViPipe, 0x5784, 0x10);
	sc635ai_write_register(ViPipe, 0x5785, 0x08);
	sc635ai_write_register(ViPipe, 0x5787, 0x0a);
	sc635ai_write_register(ViPipe, 0x5788, 0x0a);
	sc635ai_write_register(ViPipe, 0x5789, 0x08);
	sc635ai_write_register(ViPipe, 0x578a, 0x0a);
	sc635ai_write_register(ViPipe, 0x578b, 0x0a);
	sc635ai_write_register(ViPipe, 0x578c, 0x08);
	sc635ai_write_register(ViPipe, 0x578d, 0x41);
	sc635ai_write_register(ViPipe, 0x5790, 0x08);
	sc635ai_write_register(ViPipe, 0x5791, 0x04);
	sc635ai_write_register(ViPipe, 0x5792, 0x04);
	sc635ai_write_register(ViPipe, 0x5793, 0x08);
	sc635ai_write_register(ViPipe, 0x5794, 0x04);
	sc635ai_write_register(ViPipe, 0x5795, 0x04);
	sc635ai_write_register(ViPipe, 0x5799, 0x46);
	sc635ai_write_register(ViPipe, 0x579a, 0x77);
	sc635ai_write_register(ViPipe, 0x57a1, 0x04);
	sc635ai_write_register(ViPipe, 0x57a8, 0xd2);
	sc635ai_write_register(ViPipe, 0x57aa, 0x2a);
	sc635ai_write_register(ViPipe, 0x57ab, 0x7f);
	sc635ai_write_register(ViPipe, 0x57ac, 0x00);
	sc635ai_write_register(ViPipe, 0x57ad, 0x00);
	sc635ai_write_register(ViPipe, 0x58c0, 0x30);
	sc635ai_write_register(ViPipe, 0x58c1, 0x28);
	sc635ai_write_register(ViPipe, 0x58c2, 0x20);
	sc635ai_write_register(ViPipe, 0x58c3, 0x30);
	sc635ai_write_register(ViPipe, 0x58c4, 0x28);
	sc635ai_write_register(ViPipe, 0x58c5, 0x20);
	sc635ai_write_register(ViPipe, 0x58c6, 0x3c);
	sc635ai_write_register(ViPipe, 0x58c7, 0x30);
	sc635ai_write_register(ViPipe, 0x58c8, 0x28);
	sc635ai_write_register(ViPipe, 0x58c9, 0x3c);
	sc635ai_write_register(ViPipe, 0x58ca, 0x30);
	sc635ai_write_register(ViPipe, 0x58cb, 0x28);
	sc635ai_write_register(ViPipe, 0x36e9, 0x24);
	sc635ai_write_register(ViPipe, 0x37f9, 0x24);

	sc635ai_default_reg_init(ViPipe);
	sc635ai_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(100);

	printf("ViPipe:%d,===SC635AI 1800P 60fps 10bit LINE Init OK!===\n", ViPipe);
}
