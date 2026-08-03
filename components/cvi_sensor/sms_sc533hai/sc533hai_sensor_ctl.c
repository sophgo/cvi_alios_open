#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "sc533hai_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#define SC533HAI_PATTERN_ENABLE	0

static void sc533hai_linear_1620p30_init(VI_PIPE ViPipe);
static void sc533hai_wdr_1620p30_init(VI_PIPE ViPipe);
static void sc533hai_wdr_1620p30_24m_init(VI_PIPE ViPipe);

CVI_U8 sc533hai_i2c_addr = 0x30;        /* I2C Address of SC533HAI */
const CVI_U32 sc533hai_addr_byte = 2;
const CVI_U32 sc533hai_data_byte = 1;

int sc533hai_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC533HAI_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc533hai_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC533HAI_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc533hai_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC533HAI_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc533hai_i2c_addr, (CVI_U32)addr, sc533hai_addr_byte, sc533hai_data_byte);
}

int sc533hai_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC533HAI_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sc533hai_i2c_addr, (CVI_U32)addr, sc533hai_addr_byte,
				(CVI_U32)data, sc533hai_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc533hai_standby(VI_PIPE ViPipe)
{
	sc533hai_write_register(ViPipe, 0x0100, 0x00);
}

void sc533hai_restart(VI_PIPE ViPipe)
{
	sc533hai_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc533hai_write_register(ViPipe, 0x0100, 0x01);
}

void sc533hai_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC533HAI[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc533hai_write_register(ViPipe,
				g_pastSC533HAI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC533HAI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define SC533HAI_CHIP_ID_HI_ADDR		0x3107
#define SC533HAI_CHIP_ID_LO_ADDR		0x3108
#define SC533HAI_CHIP_ID			0xce7c

void sc533hai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x4;
		break;
	case ISP_SNS_FLIP:
		val |= 0x60;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x64;
		break;
	default:
		return;
	}

	sc533hai_write_register(ViPipe, 0x3221, val);
}

int sc533hai_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc533hai_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc533hai_read_register(ViPipe, SC533HAI_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc533hai_read_register(ViPipe, SC533HAI_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC533HAI_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc533hai_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode = g_pastSC533HAI[ViPipe]->u8ImgMode;

	sc533hai_i2c_init(ViPipe);


	if (u8ImgMode == SC533HAI_MODE_1620P30) {
		sc533hai_linear_1620p30_init(ViPipe);
	} else if (u8ImgMode == SC533HAI_MODE_1620P30_WDR_27M) {
		sc533hai_wdr_1620p30_init(ViPipe);
	} else if (u8ImgMode == SC533HAI_MODE_1620P30_WDR_24M) {
		sc533hai_wdr_1620p30_24m_init(ViPipe);
	} else {
		sc533hai_linear_1620p30_init(ViPipe);
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor imagemode[%d] unsupport!, use default mode\n", u8ImgMode);
	}

	g_pastSC533HAI[ViPipe]->bInit = CVI_TRUE;
}

/* 1800P60 */
static void sc533hai_linear_1620p30_init(VI_PIPE ViPipe)
{
	sc533hai_write_register(ViPipe, 0x3105, 0x32);
	sc533hai_write_register(ViPipe, 0x0103, 0x01);
	sc533hai_write_register(ViPipe, 0x0100, 0x00);
	sc533hai_write_register(ViPipe, 0x302c, 0x0c);
	sc533hai_write_register(ViPipe, 0x302c, 0x00);
	sc533hai_write_register(ViPipe, 0x3105, 0x12);
	sc533hai_write_register(ViPipe, 0x2386, 0x8a);
	sc533hai_write_register(ViPipe, 0x23b0, 0x00);
	sc533hai_write_register(ViPipe, 0x23b1, 0x08);
	sc533hai_write_register(ViPipe, 0x23b2, 0x00);
	sc533hai_write_register(ViPipe, 0x23b3, 0x18);
	sc533hai_write_register(ViPipe, 0x23b4, 0x00);
	sc533hai_write_register(ViPipe, 0x23b5, 0x38);
	sc533hai_write_register(ViPipe, 0x23b6, 0x04);
	sc533hai_write_register(ViPipe, 0x23b7, 0x08);
	sc533hai_write_register(ViPipe, 0x23b8, 0x04);
	sc533hai_write_register(ViPipe, 0x23b9, 0x18);
	sc533hai_write_register(ViPipe, 0x23ba, 0x04);
	sc533hai_write_register(ViPipe, 0x23bb, 0x38);
	sc533hai_write_register(ViPipe, 0x23bc, 0x04);
	sc533hai_write_register(ViPipe, 0x23bd, 0x08);
	sc533hai_write_register(ViPipe, 0x23be, 0x04);
	sc533hai_write_register(ViPipe, 0x23bf, 0x78);
	sc533hai_write_register(ViPipe, 0x23c0, 0x04);
	sc533hai_write_register(ViPipe, 0x23c1, 0x00);
	sc533hai_write_register(ViPipe, 0x23c2, 0x04);
	sc533hai_write_register(ViPipe, 0x23c3, 0x18);
	sc533hai_write_register(ViPipe, 0x23c4, 0x04);
	sc533hai_write_register(ViPipe, 0x23c5, 0x78);
	sc533hai_write_register(ViPipe, 0x23c6, 0x04);
	sc533hai_write_register(ViPipe, 0x23c7, 0x08);
	sc533hai_write_register(ViPipe, 0x23c8, 0x04);
	sc533hai_write_register(ViPipe, 0x23c9, 0x78);
	sc533hai_write_register(ViPipe, 0x3018, 0x7b);
	sc533hai_write_register(ViPipe, 0x301e, 0xf0);
	sc533hai_write_register(ViPipe, 0x301f, 0x23);
	sc533hai_write_register(ViPipe, 0x302c, 0x00);
	sc533hai_write_register(ViPipe, 0x30b0, 0x01);
	sc533hai_write_register(ViPipe, 0x30b8, 0x44);
	sc533hai_write_register(ViPipe, 0x3200, 0x00);
	sc533hai_write_register(ViPipe, 0x3201, 0xa0);
	sc533hai_write_register(ViPipe, 0x3202, 0x00);
	sc533hai_write_register(ViPipe, 0x3203, 0x1e);
	sc533hai_write_register(ViPipe, 0x3204, 0x0b);
	sc533hai_write_register(ViPipe, 0x3205, 0xe7);
	sc533hai_write_register(ViPipe, 0x3206, 0x06);
	sc533hai_write_register(ViPipe, 0x3207, 0x77);
	sc533hai_write_register(ViPipe, 0x3208, 0x0b);
	sc533hai_write_register(ViPipe, 0x3209, 0x40);
	sc533hai_write_register(ViPipe, 0x320a, 0x06);
	sc533hai_write_register(ViPipe, 0x320b, 0x54);
	sc533hai_write_register(ViPipe, 0x320c, 0x03);
	sc533hai_write_register(ViPipe, 0x320d, 0xe8);
	sc533hai_write_register(ViPipe, 0x320e, 0x06);
	sc533hai_write_register(ViPipe, 0x320f, 0x72);
	sc533hai_write_register(ViPipe, 0x3210, 0x00);
	sc533hai_write_register(ViPipe, 0x3211, 0x04);
	sc533hai_write_register(ViPipe, 0x3212, 0x00);
	sc533hai_write_register(ViPipe, 0x3213, 0x04);
	sc533hai_write_register(ViPipe, 0x3214, 0x11);
	sc533hai_write_register(ViPipe, 0x3215, 0x11);
	sc533hai_write_register(ViPipe, 0x3223, 0xc0);
	sc533hai_write_register(ViPipe, 0x3250, 0xff);
	sc533hai_write_register(ViPipe, 0x3271, 0x10);
	sc533hai_write_register(ViPipe, 0x327f, 0x3f);
	sc533hai_write_register(ViPipe, 0x32e0, 0x00);
	sc533hai_write_register(ViPipe, 0x3301, 0x0a);
	sc533hai_write_register(ViPipe, 0x3304, 0x50);
	sc533hai_write_register(ViPipe, 0x3305, 0x00);
	sc533hai_write_register(ViPipe, 0x3306, 0x70);
	sc533hai_write_register(ViPipe, 0x3308, 0x18);
	sc533hai_write_register(ViPipe, 0x3309, 0xb0);
	sc533hai_write_register(ViPipe, 0x330a, 0x01);
	sc533hai_write_register(ViPipe, 0x330b, 0x20);
	sc533hai_write_register(ViPipe, 0x331e, 0x39);
	sc533hai_write_register(ViPipe, 0x331f, 0x99);
	sc533hai_write_register(ViPipe, 0x3333, 0x10);
	sc533hai_write_register(ViPipe, 0x3334, 0x40);
	sc533hai_write_register(ViPipe, 0x3364, 0x5e);
	sc533hai_write_register(ViPipe, 0x338f, 0xa0);
	sc533hai_write_register(ViPipe, 0x3393, 0x18);
	sc533hai_write_register(ViPipe, 0x3394, 0x2c);
	sc533hai_write_register(ViPipe, 0x3395, 0x3c);
	sc533hai_write_register(ViPipe, 0x3399, 0x0a);
	sc533hai_write_register(ViPipe, 0x339a, 0x16);
	sc533hai_write_register(ViPipe, 0x339b, 0x1e);
	sc533hai_write_register(ViPipe, 0x339c, 0x3e);
	sc533hai_write_register(ViPipe, 0x33ac, 0x0c);
	sc533hai_write_register(ViPipe, 0x33ad, 0x2c);
	sc533hai_write_register(ViPipe, 0x33ae, 0x30);
	sc533hai_write_register(ViPipe, 0x33af, 0x90);
	sc533hai_write_register(ViPipe, 0x33b0, 0x0f);
	sc533hai_write_register(ViPipe, 0x33b1, 0x80);
	sc533hai_write_register(ViPipe, 0x33b2, 0x24);
	sc533hai_write_register(ViPipe, 0x33b3, 0x10);
	sc533hai_write_register(ViPipe, 0x33f8, 0x00);
	sc533hai_write_register(ViPipe, 0x33f9, 0x70);
	sc533hai_write_register(ViPipe, 0x33fa, 0x00);
	sc533hai_write_register(ViPipe, 0x33fb, 0x70);
	sc533hai_write_register(ViPipe, 0x349f, 0x03);
	sc533hai_write_register(ViPipe, 0x34a8, 0x10);
	sc533hai_write_register(ViPipe, 0x34a9, 0x10);
	sc533hai_write_register(ViPipe, 0x34aa, 0x01);
	sc533hai_write_register(ViPipe, 0x34ab, 0x20);
	sc533hai_write_register(ViPipe, 0x34ac, 0x01);
	sc533hai_write_register(ViPipe, 0x34ad, 0x20);
	sc533hai_write_register(ViPipe, 0x34f9, 0x12);
	sc533hai_write_register(ViPipe, 0x3632, 0x6d);
	sc533hai_write_register(ViPipe, 0x3633, 0x4d);
	sc533hai_write_register(ViPipe, 0x363a, 0x80);
	sc533hai_write_register(ViPipe, 0x363b, 0x57);
	sc533hai_write_register(ViPipe, 0x363c, 0xd8);
	sc533hai_write_register(ViPipe, 0x363d, 0x40);
	sc533hai_write_register(ViPipe, 0x3670, 0x41);
	sc533hai_write_register(ViPipe, 0x3671, 0x31);
	sc533hai_write_register(ViPipe, 0x3672, 0x31);
	sc533hai_write_register(ViPipe, 0x3673, 0x04);
	sc533hai_write_register(ViPipe, 0x3674, 0x08);
	sc533hai_write_register(ViPipe, 0x3675, 0x04);
	sc533hai_write_register(ViPipe, 0x3676, 0x18);
	sc533hai_write_register(ViPipe, 0x367e, 0x69);
	sc533hai_write_register(ViPipe, 0x367f, 0x6d);
	sc533hai_write_register(ViPipe, 0x3680, 0x8d);
	sc533hai_write_register(ViPipe, 0x3681, 0x04);
	sc533hai_write_register(ViPipe, 0x3682, 0x08);
	sc533hai_write_register(ViPipe, 0x3683, 0x04);
	sc533hai_write_register(ViPipe, 0x3684, 0x78);
	sc533hai_write_register(ViPipe, 0x3685, 0x80);
	sc533hai_write_register(ViPipe, 0x3686, 0x80);
	sc533hai_write_register(ViPipe, 0x3687, 0x83);
	sc533hai_write_register(ViPipe, 0x3688, 0x82);
	sc533hai_write_register(ViPipe, 0x3689, 0x85);
	sc533hai_write_register(ViPipe, 0x368a, 0x8b);
	sc533hai_write_register(ViPipe, 0x368b, 0x97);
	sc533hai_write_register(ViPipe, 0x368c, 0xbf);
	sc533hai_write_register(ViPipe, 0x368d, 0x00);
	sc533hai_write_register(ViPipe, 0x368e, 0x08);
	sc533hai_write_register(ViPipe, 0x368f, 0x00);
	sc533hai_write_register(ViPipe, 0x3690, 0x18);
	sc533hai_write_register(ViPipe, 0x3691, 0x04);
	sc533hai_write_register(ViPipe, 0x3692, 0x00);
	sc533hai_write_register(ViPipe, 0x3693, 0x04);
	sc533hai_write_register(ViPipe, 0x3694, 0x08);
	sc533hai_write_register(ViPipe, 0x3695, 0x04);
	sc533hai_write_register(ViPipe, 0x3696, 0x18);
	sc533hai_write_register(ViPipe, 0x3697, 0x04);
	sc533hai_write_register(ViPipe, 0x3698, 0x38);
	sc533hai_write_register(ViPipe, 0x3699, 0x04);
	sc533hai_write_register(ViPipe, 0x369a, 0x78);
	sc533hai_write_register(ViPipe, 0x36d0, 0x0d);
	sc533hai_write_register(ViPipe, 0x36ea, 0x16);
	sc533hai_write_register(ViPipe, 0x36eb, 0x55);
	sc533hai_write_register(ViPipe, 0x36ec, 0x4b);
	sc533hai_write_register(ViPipe, 0x36ed, 0x18);
	sc533hai_write_register(ViPipe, 0x370f, 0x13);
	sc533hai_write_register(ViPipe, 0x3721, 0x6c);
	sc533hai_write_register(ViPipe, 0x3722, 0x8b);
	sc533hai_write_register(ViPipe, 0x3724, 0xc1);
	sc533hai_write_register(ViPipe, 0x3726, 0x01);
	sc533hai_write_register(ViPipe, 0x3727, 0x24);
	sc533hai_write_register(ViPipe, 0x3729, 0xb4);
	sc533hai_write_register(ViPipe, 0x37b0, 0x7f);
	sc533hai_write_register(ViPipe, 0x37b1, 0x7f);
	sc533hai_write_register(ViPipe, 0x37b2, 0x73);
	sc533hai_write_register(ViPipe, 0x37b3, 0x04);
	sc533hai_write_register(ViPipe, 0x37b4, 0x08);
	sc533hai_write_register(ViPipe, 0x37b5, 0x04);
	sc533hai_write_register(ViPipe, 0x37b6, 0x38);
	sc533hai_write_register(ViPipe, 0x37b7, 0x11);
	sc533hai_write_register(ViPipe, 0x37b8, 0x11);
	sc533hai_write_register(ViPipe, 0x37b9, 0x00);
	sc533hai_write_register(ViPipe, 0x37ba, 0x94);
	sc533hai_write_register(ViPipe, 0x37bb, 0xd4);
	sc533hai_write_register(ViPipe, 0x37bc, 0x84);
	sc533hai_write_register(ViPipe, 0x37bd, 0x00);
	sc533hai_write_register(ViPipe, 0x37be, 0x08);
	sc533hai_write_register(ViPipe, 0x37bf, 0x04);
	sc533hai_write_register(ViPipe, 0x37c0, 0x00);
	sc533hai_write_register(ViPipe, 0x37c1, 0x04);
	sc533hai_write_register(ViPipe, 0x37c2, 0x18);
	sc533hai_write_register(ViPipe, 0x37c3, 0x04);
	sc533hai_write_register(ViPipe, 0x37c4, 0x3c);
	sc533hai_write_register(ViPipe, 0x37fa, 0x16);
	sc533hai_write_register(ViPipe, 0x37fb, 0x55);
	sc533hai_write_register(ViPipe, 0x37fc, 0x19);
	sc533hai_write_register(ViPipe, 0x37fd, 0x1a);
	sc533hai_write_register(ViPipe, 0x3900, 0x05);
	sc533hai_write_register(ViPipe, 0x3903, 0x60);
	sc533hai_write_register(ViPipe, 0x3905, 0x0d);
	sc533hai_write_register(ViPipe, 0x391a, 0x60);
	sc533hai_write_register(ViPipe, 0x391b, 0x40);
	sc533hai_write_register(ViPipe, 0x391c, 0x26);
	sc533hai_write_register(ViPipe, 0x391d, 0x00);
	sc533hai_write_register(ViPipe, 0x3926, 0xe0);
	sc533hai_write_register(ViPipe, 0x3933, 0x80);
	sc533hai_write_register(ViPipe, 0x3934, 0x06);
	sc533hai_write_register(ViPipe, 0x3935, 0x00);
	sc533hai_write_register(ViPipe, 0x3936, 0x72);
	sc533hai_write_register(ViPipe, 0x3937, 0x71);
	sc533hai_write_register(ViPipe, 0x3938, 0x75);
	sc533hai_write_register(ViPipe, 0x3939, 0x0f);
	sc533hai_write_register(ViPipe, 0x393a, 0xf3);
	sc533hai_write_register(ViPipe, 0x393b, 0x0f);
	sc533hai_write_register(ViPipe, 0x393c, 0xd8);
	sc533hai_write_register(ViPipe, 0x393f, 0x80);
	sc533hai_write_register(ViPipe, 0x3940, 0x0b);
	sc533hai_write_register(ViPipe, 0x3941, 0x00);
	sc533hai_write_register(ViPipe, 0x3942, 0x0b);
	sc533hai_write_register(ViPipe, 0x3943, 0x7e);
	sc533hai_write_register(ViPipe, 0x3944, 0x7f);
	sc533hai_write_register(ViPipe, 0x3945, 0x7f);
	sc533hai_write_register(ViPipe, 0x3946, 0x7e);
	sc533hai_write_register(ViPipe, 0x39dd, 0x00);
	sc533hai_write_register(ViPipe, 0x39de, 0x08);
	sc533hai_write_register(ViPipe, 0x39e7, 0x04);
	sc533hai_write_register(ViPipe, 0x39e8, 0x04);
	sc533hai_write_register(ViPipe, 0x39e9, 0x80);
	sc533hai_write_register(ViPipe, 0x3e00, 0x00);
	sc533hai_write_register(ViPipe, 0x3e01, 0x66);
	sc533hai_write_register(ViPipe, 0x3e02, 0xa0);
	sc533hai_write_register(ViPipe, 0x3e03, 0x0b);
	sc533hai_write_register(ViPipe, 0x3e08, 0x00);
	sc533hai_write_register(ViPipe, 0x3e16, 0x01);
	sc533hai_write_register(ViPipe, 0x3e17, 0x54);
	sc533hai_write_register(ViPipe, 0x3e18, 0x01);
	sc533hai_write_register(ViPipe, 0x3e19, 0x54);
	sc533hai_write_register(ViPipe, 0x3e1b, 0x29);
	sc533hai_write_register(ViPipe, 0x4402, 0x11);
	sc533hai_write_register(ViPipe, 0x450a, 0x80);
	sc533hai_write_register(ViPipe, 0x450d, 0x0a);
	sc533hai_write_register(ViPipe, 0x4800, 0x24);
	sc533hai_write_register(ViPipe, 0x480f, 0x03);
	sc533hai_write_register(ViPipe, 0x4837, 0x50);
	sc533hai_write_register(ViPipe, 0x5000, 0x26);
	sc533hai_write_register(ViPipe, 0x5780, 0x76);
	sc533hai_write_register(ViPipe, 0x5784, 0x10);
	sc533hai_write_register(ViPipe, 0x5785, 0x08);
	sc533hai_write_register(ViPipe, 0x5787, 0x0a);
	sc533hai_write_register(ViPipe, 0x5788, 0x0a);
	sc533hai_write_register(ViPipe, 0x5789, 0x08);
	sc533hai_write_register(ViPipe, 0x578a, 0x0a);
	sc533hai_write_register(ViPipe, 0x578b, 0x0a);
	sc533hai_write_register(ViPipe, 0x578c, 0x08);
	sc533hai_write_register(ViPipe, 0x578d, 0x41);
	sc533hai_write_register(ViPipe, 0x5790, 0x08);
	sc533hai_write_register(ViPipe, 0x5791, 0x04);
	sc533hai_write_register(ViPipe, 0x5792, 0x04);
	sc533hai_write_register(ViPipe, 0x5793, 0x08);
	sc533hai_write_register(ViPipe, 0x5794, 0x04);
	sc533hai_write_register(ViPipe, 0x5795, 0x04);
	sc533hai_write_register(ViPipe, 0x5799, 0x46);
	sc533hai_write_register(ViPipe, 0x579a, 0x77);
	sc533hai_write_register(ViPipe, 0x57a1, 0x04);
	sc533hai_write_register(ViPipe, 0x57a8, 0xd2);
	sc533hai_write_register(ViPipe, 0x57aa, 0x2a);
	sc533hai_write_register(ViPipe, 0x57ab, 0x7f);
	sc533hai_write_register(ViPipe, 0x57ac, 0x00);
	sc533hai_write_register(ViPipe, 0x57ad, 0x00);
	sc533hai_write_register(ViPipe, 0x58c0, 0x30);
	sc533hai_write_register(ViPipe, 0x58c1, 0x28);
	sc533hai_write_register(ViPipe, 0x58c2, 0x20);
	sc533hai_write_register(ViPipe, 0x58c3, 0x30);
	sc533hai_write_register(ViPipe, 0x58c4, 0x28);
	sc533hai_write_register(ViPipe, 0x58c5, 0x20);
	sc533hai_write_register(ViPipe, 0x58c6, 0x3c);
	sc533hai_write_register(ViPipe, 0x58c7, 0x30);
	sc533hai_write_register(ViPipe, 0x58c8, 0x28);
	sc533hai_write_register(ViPipe, 0x58c9, 0x3c);
	sc533hai_write_register(ViPipe, 0x58ca, 0x30);
	sc533hai_write_register(ViPipe, 0x58cb, 0x28);
	sc533hai_write_register(ViPipe, 0x36e9, 0x44);
	sc533hai_write_register(ViPipe, 0x37f9, 0x20);

	sc533hai_default_reg_init(ViPipe);
	sc533hai_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(100);

	printf("ViPipe:%d,===SC533HAI 1620P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

static void sc533hai_wdr_1620p30_init(VI_PIPE ViPipe)
{
	sc533hai_write_register(ViPipe, 0x3105, 0x32);
	sc533hai_write_register(ViPipe, 0x0103, 0x01);
	sc533hai_write_register(ViPipe, 0x0100, 0x00);
	sc533hai_write_register(ViPipe, 0x302c, 0x0c);
	sc533hai_write_register(ViPipe, 0x302c, 0x00);
	sc533hai_write_register(ViPipe, 0x3105, 0x12);
	sc533hai_write_register(ViPipe, 0x2386, 0x8a);
	sc533hai_write_register(ViPipe, 0x23b0, 0x00);
	sc533hai_write_register(ViPipe, 0x23b1, 0x08);
	sc533hai_write_register(ViPipe, 0x23b2, 0x00);
	sc533hai_write_register(ViPipe, 0x23b3, 0x18);
	sc533hai_write_register(ViPipe, 0x23b4, 0x00);
	sc533hai_write_register(ViPipe, 0x23b5, 0x38);
	sc533hai_write_register(ViPipe, 0x23b6, 0x04);
	sc533hai_write_register(ViPipe, 0x23b7, 0x08);
	sc533hai_write_register(ViPipe, 0x23b8, 0x04);
	sc533hai_write_register(ViPipe, 0x23b9, 0x18);
	sc533hai_write_register(ViPipe, 0x23ba, 0x04);
	sc533hai_write_register(ViPipe, 0x23bb, 0x38);
	sc533hai_write_register(ViPipe, 0x23bc, 0x04);
	sc533hai_write_register(ViPipe, 0x23bd, 0x08);
	sc533hai_write_register(ViPipe, 0x23be, 0x04);
	sc533hai_write_register(ViPipe, 0x23bf, 0x78);
	sc533hai_write_register(ViPipe, 0x23c0, 0x04);
	sc533hai_write_register(ViPipe, 0x23c1, 0x00);
	sc533hai_write_register(ViPipe, 0x23c2, 0x04);
	sc533hai_write_register(ViPipe, 0x23c3, 0x18);
	sc533hai_write_register(ViPipe, 0x23c4, 0x04);
	sc533hai_write_register(ViPipe, 0x23c5, 0x78);
	sc533hai_write_register(ViPipe, 0x23c6, 0x04);
	sc533hai_write_register(ViPipe, 0x23c7, 0x08);
	sc533hai_write_register(ViPipe, 0x23c8, 0x04);
	sc533hai_write_register(ViPipe, 0x23c9, 0x78);
	sc533hai_write_register(ViPipe, 0x3018, 0x7b);
	sc533hai_write_register(ViPipe, 0x301e, 0xf0);
	sc533hai_write_register(ViPipe, 0x301f, 0x08);
	sc533hai_write_register(ViPipe, 0x302c, 0x00);
	sc533hai_write_register(ViPipe, 0x30b0, 0x01);
	sc533hai_write_register(ViPipe, 0x30b8, 0x44);
	sc533hai_write_register(ViPipe, 0x3200, 0x00);
	sc533hai_write_register(ViPipe, 0x3201, 0xa0);
	sc533hai_write_register(ViPipe, 0x3202, 0x00);
	sc533hai_write_register(ViPipe, 0x3203, 0x96);
	sc533hai_write_register(ViPipe, 0x3204, 0x0b);
	sc533hai_write_register(ViPipe, 0x3205, 0xe7);
	sc533hai_write_register(ViPipe, 0x3206, 0x06);
	sc533hai_write_register(ViPipe, 0x3207, 0xf1);
	sc533hai_write_register(ViPipe, 0x3208, 0x0b);
	sc533hai_write_register(ViPipe, 0x3209, 0x40);
	sc533hai_write_register(ViPipe, 0x320a, 0x06);
	sc533hai_write_register(ViPipe, 0x320b, 0x54);
	sc533hai_write_register(ViPipe, 0x320c, 0x03);
	sc533hai_write_register(ViPipe, 0x320d, 0xe8);
	sc533hai_write_register(ViPipe, 0x320e, 0x0c);
	sc533hai_write_register(ViPipe, 0x320f, 0xe4);
	sc533hai_write_register(ViPipe, 0x3210, 0x00);
	sc533hai_write_register(ViPipe, 0x3211, 0x04);
	sc533hai_write_register(ViPipe, 0x3212, 0x00);
	sc533hai_write_register(ViPipe, 0x3213, 0x04);
	sc533hai_write_register(ViPipe, 0x3214, 0x11);
	sc533hai_write_register(ViPipe, 0x3215, 0x11);
	sc533hai_write_register(ViPipe, 0x3223, 0xc0);
	sc533hai_write_register(ViPipe, 0x3250, 0xff);
	sc533hai_write_register(ViPipe, 0x3271, 0x10);
	sc533hai_write_register(ViPipe, 0x327f, 0x3f);
	sc533hai_write_register(ViPipe, 0x3281, 0x01);
	sc533hai_write_register(ViPipe, 0x32e0, 0x00);
	sc533hai_write_register(ViPipe, 0x3301, 0x12);
	sc533hai_write_register(ViPipe, 0x3304, 0x50);
	sc533hai_write_register(ViPipe, 0x3305, 0x00);
	sc533hai_write_register(ViPipe, 0x3306, 0x70);
	sc533hai_write_register(ViPipe, 0x3308, 0x18);
	sc533hai_write_register(ViPipe, 0x3309, 0xb0);
	sc533hai_write_register(ViPipe, 0x330a, 0x01);
	sc533hai_write_register(ViPipe, 0x330b, 0x20);
	sc533hai_write_register(ViPipe, 0x331e, 0x39);
	sc533hai_write_register(ViPipe, 0x331f, 0x99);
	sc533hai_write_register(ViPipe, 0x3333, 0x10);
	sc533hai_write_register(ViPipe, 0x3334, 0x40);
	sc533hai_write_register(ViPipe, 0x3364, 0x5e);
	sc533hai_write_register(ViPipe, 0x338f, 0xa0);
	sc533hai_write_register(ViPipe, 0x3393, 0x18);
	sc533hai_write_register(ViPipe, 0x3394, 0x2c);
	sc533hai_write_register(ViPipe, 0x3395, 0x3c);
	sc533hai_write_register(ViPipe, 0x3399, 0x12);
	sc533hai_write_register(ViPipe, 0x339a, 0x16);
	sc533hai_write_register(ViPipe, 0x339b, 0x1e);
	sc533hai_write_register(ViPipe, 0x339c, 0x3e);
	sc533hai_write_register(ViPipe, 0x33ac, 0x0c);
	sc533hai_write_register(ViPipe, 0x33ad, 0x2c);
	sc533hai_write_register(ViPipe, 0x33ae, 0x30);
	sc533hai_write_register(ViPipe, 0x33af, 0x90);
	sc533hai_write_register(ViPipe, 0x33b0, 0x0f);
	sc533hai_write_register(ViPipe, 0x33b1, 0x80);
	sc533hai_write_register(ViPipe, 0x33b2, 0x24);
	sc533hai_write_register(ViPipe, 0x33b3, 0x10);
	sc533hai_write_register(ViPipe, 0x33f8, 0x00);
	sc533hai_write_register(ViPipe, 0x33f9, 0x70);
	sc533hai_write_register(ViPipe, 0x33fa, 0x00);
	sc533hai_write_register(ViPipe, 0x33fb, 0x70);
	sc533hai_write_register(ViPipe, 0x349f, 0x03);
	sc533hai_write_register(ViPipe, 0x34a8, 0x10);
	sc533hai_write_register(ViPipe, 0x34a9, 0x10);
	sc533hai_write_register(ViPipe, 0x34aa, 0x01);
	sc533hai_write_register(ViPipe, 0x34ab, 0x20);
	sc533hai_write_register(ViPipe, 0x34ac, 0x01);
	sc533hai_write_register(ViPipe, 0x34ad, 0x20);
	sc533hai_write_register(ViPipe, 0x34f9, 0x12);
	sc533hai_write_register(ViPipe, 0x3632, 0x6d);
	sc533hai_write_register(ViPipe, 0x3633, 0x4d);
	sc533hai_write_register(ViPipe, 0x363a, 0x80);
	sc533hai_write_register(ViPipe, 0x363b, 0x57);
	sc533hai_write_register(ViPipe, 0x363c, 0xd8);
	sc533hai_write_register(ViPipe, 0x363d, 0x40);
	sc533hai_write_register(ViPipe, 0x3670, 0x41);
	sc533hai_write_register(ViPipe, 0x3671, 0x31);
	sc533hai_write_register(ViPipe, 0x3672, 0x31);
	sc533hai_write_register(ViPipe, 0x3673, 0x04);
	sc533hai_write_register(ViPipe, 0x3674, 0x08);
	sc533hai_write_register(ViPipe, 0x3675, 0x04);
	sc533hai_write_register(ViPipe, 0x3676, 0x18);
	sc533hai_write_register(ViPipe, 0x367e, 0x69);
	sc533hai_write_register(ViPipe, 0x367f, 0x6d);
	sc533hai_write_register(ViPipe, 0x3680, 0x8d);
	sc533hai_write_register(ViPipe, 0x3681, 0x04);
	sc533hai_write_register(ViPipe, 0x3682, 0x08);
	sc533hai_write_register(ViPipe, 0x3683, 0x04);
	sc533hai_write_register(ViPipe, 0x3684, 0x78);
	sc533hai_write_register(ViPipe, 0x3685, 0x80);
	sc533hai_write_register(ViPipe, 0x3686, 0x80);
	sc533hai_write_register(ViPipe, 0x3687, 0x83);
	sc533hai_write_register(ViPipe, 0x3688, 0x82);
	sc533hai_write_register(ViPipe, 0x3689, 0x85);
	sc533hai_write_register(ViPipe, 0x368a, 0x8b);
	sc533hai_write_register(ViPipe, 0x368b, 0x97);
	sc533hai_write_register(ViPipe, 0x368c, 0xbf);
	sc533hai_write_register(ViPipe, 0x368d, 0x00);
	sc533hai_write_register(ViPipe, 0x368e, 0x08);
	sc533hai_write_register(ViPipe, 0x368f, 0x00);
	sc533hai_write_register(ViPipe, 0x3690, 0x18);
	sc533hai_write_register(ViPipe, 0x3691, 0x04);
	sc533hai_write_register(ViPipe, 0x3692, 0x00);
	sc533hai_write_register(ViPipe, 0x3693, 0x04);
	sc533hai_write_register(ViPipe, 0x3694, 0x08);
	sc533hai_write_register(ViPipe, 0x3695, 0x04);
	sc533hai_write_register(ViPipe, 0x3696, 0x18);
	sc533hai_write_register(ViPipe, 0x3697, 0x04);
	sc533hai_write_register(ViPipe, 0x3698, 0x38);
	sc533hai_write_register(ViPipe, 0x3699, 0x04);
	sc533hai_write_register(ViPipe, 0x369a, 0x78);
	sc533hai_write_register(ViPipe, 0x36d0, 0x0d);
	sc533hai_write_register(ViPipe, 0x36ea, 0x18);
	sc533hai_write_register(ViPipe, 0x36eb, 0x45);
	sc533hai_write_register(ViPipe, 0x36ec, 0x4b);
	sc533hai_write_register(ViPipe, 0x36ed, 0x18);
	sc533hai_write_register(ViPipe, 0x370f, 0x13);
	sc533hai_write_register(ViPipe, 0x3721, 0x6c);
	sc533hai_write_register(ViPipe, 0x3722, 0x8b);
	sc533hai_write_register(ViPipe, 0x3724, 0xc1);
	sc533hai_write_register(ViPipe, 0x3726, 0x01);
	sc533hai_write_register(ViPipe, 0x3727, 0x24);
	sc533hai_write_register(ViPipe, 0x3729, 0xb4);
	sc533hai_write_register(ViPipe, 0x37b0, 0x7f);
	sc533hai_write_register(ViPipe, 0x37b1, 0x7f);
	sc533hai_write_register(ViPipe, 0x37b2, 0x73);
	sc533hai_write_register(ViPipe, 0x37b3, 0x04);
	sc533hai_write_register(ViPipe, 0x37b4, 0x08);
	sc533hai_write_register(ViPipe, 0x37b5, 0x04);
	sc533hai_write_register(ViPipe, 0x37b6, 0x38);
	sc533hai_write_register(ViPipe, 0x37b7, 0x11);
	sc533hai_write_register(ViPipe, 0x37b8, 0x11);
	sc533hai_write_register(ViPipe, 0x37b9, 0x00);
	sc533hai_write_register(ViPipe, 0x37ba, 0x94);
	sc533hai_write_register(ViPipe, 0x37bb, 0xd4);
	sc533hai_write_register(ViPipe, 0x37bc, 0x84);
	sc533hai_write_register(ViPipe, 0x37bd, 0x00);
	sc533hai_write_register(ViPipe, 0x37be, 0x08);
	sc533hai_write_register(ViPipe, 0x37bf, 0x04);
	sc533hai_write_register(ViPipe, 0x37c0, 0x00);
	sc533hai_write_register(ViPipe, 0x37c1, 0x04);
	sc533hai_write_register(ViPipe, 0x37c2, 0x18);
	sc533hai_write_register(ViPipe, 0x37c3, 0x04);
	sc533hai_write_register(ViPipe, 0x37c4, 0x3c);
	sc533hai_write_register(ViPipe, 0x37fa, 0x16);
	sc533hai_write_register(ViPipe, 0x37fb, 0x55);
	sc533hai_write_register(ViPipe, 0x37fc, 0x19);
	sc533hai_write_register(ViPipe, 0x37fd, 0x1a);
	sc533hai_write_register(ViPipe, 0x3900, 0x05);
	sc533hai_write_register(ViPipe, 0x3903, 0x60);
	sc533hai_write_register(ViPipe, 0x3905, 0x0d);
	sc533hai_write_register(ViPipe, 0x391a, 0x60);
	sc533hai_write_register(ViPipe, 0x391b, 0x40);
	sc533hai_write_register(ViPipe, 0x391c, 0x26);
	sc533hai_write_register(ViPipe, 0x391d, 0x00);
	sc533hai_write_register(ViPipe, 0x3926, 0xe0);
	sc533hai_write_register(ViPipe, 0x3933, 0x80);
	sc533hai_write_register(ViPipe, 0x3934, 0x06);
	sc533hai_write_register(ViPipe, 0x3935, 0x00);
	sc533hai_write_register(ViPipe, 0x3936, 0x72);
	sc533hai_write_register(ViPipe, 0x3937, 0x71);
	sc533hai_write_register(ViPipe, 0x3938, 0x75);
	sc533hai_write_register(ViPipe, 0x3939, 0x0f);
	sc533hai_write_register(ViPipe, 0x393a, 0xf3);
	sc533hai_write_register(ViPipe, 0x393b, 0x0f);
	sc533hai_write_register(ViPipe, 0x393c, 0xd8);
	sc533hai_write_register(ViPipe, 0x393f, 0x80);
	sc533hai_write_register(ViPipe, 0x3940, 0x0b);
	sc533hai_write_register(ViPipe, 0x3941, 0x00);
	sc533hai_write_register(ViPipe, 0x3942, 0x0b);
	sc533hai_write_register(ViPipe, 0x3943, 0x7e);
	sc533hai_write_register(ViPipe, 0x3944, 0x7f);
	sc533hai_write_register(ViPipe, 0x3945, 0x7f);
	sc533hai_write_register(ViPipe, 0x3946, 0x7e);
	sc533hai_write_register(ViPipe, 0x39dd, 0x00);
	sc533hai_write_register(ViPipe, 0x39de, 0x08);
	sc533hai_write_register(ViPipe, 0x39e7, 0x04);
	sc533hai_write_register(ViPipe, 0x39e8, 0x04);
	sc533hai_write_register(ViPipe, 0x39e9, 0x80);
	sc533hai_write_register(ViPipe, 0x3e00, 0x00);
	sc533hai_write_register(ViPipe, 0x3e01, 0xbf);
	sc533hai_write_register(ViPipe, 0x3e02, 0x00);
	sc533hai_write_register(ViPipe, 0x3e03, 0x0b);
	sc533hai_write_register(ViPipe, 0x3e04, 0x0b);
	sc533hai_write_register(ViPipe, 0x3e05, 0xf0);
	sc533hai_write_register(ViPipe, 0x3e08, 0x00);
	sc533hai_write_register(ViPipe, 0x3e16, 0x01);
	sc533hai_write_register(ViPipe, 0x3e17, 0x54);
	sc533hai_write_register(ViPipe, 0x3e18, 0x01);
	sc533hai_write_register(ViPipe, 0x3e19, 0x54);
	sc533hai_write_register(ViPipe, 0x3e1b, 0x29);
	sc533hai_write_register(ViPipe, 0x3e23, 0x00);
	sc533hai_write_register(ViPipe, 0x3e24, 0xcc);
	sc533hai_write_register(ViPipe, 0x4402, 0x11);
	sc533hai_write_register(ViPipe, 0x450a, 0x80);
	sc533hai_write_register(ViPipe, 0x450d, 0x0a);
	sc533hai_write_register(ViPipe, 0x4800, 0x24);
	sc533hai_write_register(ViPipe, 0x480f, 0x03);
	sc533hai_write_register(ViPipe, 0x4837, 0x25);
	sc533hai_write_register(ViPipe, 0x5000, 0x26);
	sc533hai_write_register(ViPipe, 0x5780, 0x77);
	sc533hai_write_register(ViPipe, 0x5784, 0x10);
	sc533hai_write_register(ViPipe, 0x5785, 0x08);
	sc533hai_write_register(ViPipe, 0x5787, 0x0a);
	sc533hai_write_register(ViPipe, 0x5788, 0x0a);
	sc533hai_write_register(ViPipe, 0x5789, 0x08);
	sc533hai_write_register(ViPipe, 0x578a, 0x0a);
	sc533hai_write_register(ViPipe, 0x578b, 0x0a);
	sc533hai_write_register(ViPipe, 0x578c, 0x08);
	sc533hai_write_register(ViPipe, 0x578d, 0x41);
	sc533hai_write_register(ViPipe, 0x5790, 0x08);
	sc533hai_write_register(ViPipe, 0x5791, 0x04);
	sc533hai_write_register(ViPipe, 0x5792, 0x04);
	sc533hai_write_register(ViPipe, 0x5793, 0x08);
	sc533hai_write_register(ViPipe, 0x5794, 0x04);
	sc533hai_write_register(ViPipe, 0x5795, 0x04);
	sc533hai_write_register(ViPipe, 0x5799, 0x46);
	sc533hai_write_register(ViPipe, 0x579a, 0x77);
	sc533hai_write_register(ViPipe, 0x57a1, 0x04);
	sc533hai_write_register(ViPipe, 0x57a4, 0x02);
	sc533hai_write_register(ViPipe, 0x57a5, 0x00);
	sc533hai_write_register(ViPipe, 0x57a8, 0xd2);
	sc533hai_write_register(ViPipe, 0x57aa, 0x2a);
	sc533hai_write_register(ViPipe, 0x57ab, 0x7f);
	sc533hai_write_register(ViPipe, 0x57ac, 0x00);
	sc533hai_write_register(ViPipe, 0x57ad, 0x00);
	sc533hai_write_register(ViPipe, 0x58c0, 0x30);
	sc533hai_write_register(ViPipe, 0x58c1, 0x28);
	sc533hai_write_register(ViPipe, 0x58c2, 0x20);
	sc533hai_write_register(ViPipe, 0x58c3, 0x30);
	sc533hai_write_register(ViPipe, 0x58c4, 0x28);
	sc533hai_write_register(ViPipe, 0x58c5, 0x20);
	sc533hai_write_register(ViPipe, 0x58c6, 0x3c);
	sc533hai_write_register(ViPipe, 0x58c7, 0x30);
	sc533hai_write_register(ViPipe, 0x58c8, 0x28);
	sc533hai_write_register(ViPipe, 0x58c9, 0x3c);
	sc533hai_write_register(ViPipe, 0x58ca, 0x30);
	sc533hai_write_register(ViPipe, 0x58cb, 0x28);
	sc533hai_write_register(ViPipe, 0x36e9, 0x44);
	sc533hai_write_register(ViPipe, 0x37f9, 0x24);

	sc533hai_default_reg_init(ViPipe);
	sc533hai_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(100);

	printf("ViPipe:%d,===SC533HAI 1620P 30fps 10bit 2to1 WDR 27M Init OK!===\n", ViPipe);

}

static void sc533hai_wdr_1620p30_24m_init(VI_PIPE ViPipe)
{
	sc533hai_write_register(ViPipe, 0x3105, 0x32);
	sc533hai_write_register(ViPipe, 0x0103, 0x01);
	sc533hai_write_register(ViPipe, 0x0100, 0x00);
	sc533hai_write_register(ViPipe, 0x302c, 0x0c);
	sc533hai_write_register(ViPipe, 0x302c, 0x00);
	sc533hai_write_register(ViPipe, 0x3105, 0x12);
	sc533hai_write_register(ViPipe, 0x2386, 0x8a);
	sc533hai_write_register(ViPipe, 0x23b0, 0x00);
	sc533hai_write_register(ViPipe, 0x23b1, 0x08);
	sc533hai_write_register(ViPipe, 0x23b2, 0x00);
	sc533hai_write_register(ViPipe, 0x23b3, 0x18);
	sc533hai_write_register(ViPipe, 0x23b4, 0x00);
	sc533hai_write_register(ViPipe, 0x23b5, 0x38);
	sc533hai_write_register(ViPipe, 0x23b6, 0x04);
	sc533hai_write_register(ViPipe, 0x23b7, 0x08);
	sc533hai_write_register(ViPipe, 0x23b8, 0x04);
	sc533hai_write_register(ViPipe, 0x23b9, 0x18);
	sc533hai_write_register(ViPipe, 0x23ba, 0x04);
	sc533hai_write_register(ViPipe, 0x23bb, 0x38);
	sc533hai_write_register(ViPipe, 0x23bc, 0x04);
	sc533hai_write_register(ViPipe, 0x23bd, 0x08);
	sc533hai_write_register(ViPipe, 0x23be, 0x04);
	sc533hai_write_register(ViPipe, 0x23bf, 0x78);
	sc533hai_write_register(ViPipe, 0x23c0, 0x04);
	sc533hai_write_register(ViPipe, 0x23c1, 0x00);
	sc533hai_write_register(ViPipe, 0x23c2, 0x04);
	sc533hai_write_register(ViPipe, 0x23c3, 0x18);
	sc533hai_write_register(ViPipe, 0x23c4, 0x04);
	sc533hai_write_register(ViPipe, 0x23c5, 0x78);
	sc533hai_write_register(ViPipe, 0x23c6, 0x04);
	sc533hai_write_register(ViPipe, 0x23c7, 0x08);
	sc533hai_write_register(ViPipe, 0x23c8, 0x04);
	sc533hai_write_register(ViPipe, 0x23c9, 0x78);
	sc533hai_write_register(ViPipe, 0x3018, 0x7b);
	sc533hai_write_register(ViPipe, 0x301e, 0xf0);
	sc533hai_write_register(ViPipe, 0x301f, 0x24);
	sc533hai_write_register(ViPipe, 0x302c, 0x00);
	sc533hai_write_register(ViPipe, 0x30b0, 0x01);
	sc533hai_write_register(ViPipe, 0x30b8, 0x44);
	sc533hai_write_register(ViPipe, 0x3200, 0x00);
	sc533hai_write_register(ViPipe, 0x3201, 0xa0);
	sc533hai_write_register(ViPipe, 0x3202, 0x00);
	sc533hai_write_register(ViPipe, 0x3203, 0x96);
	sc533hai_write_register(ViPipe, 0x3204, 0x0b);
	sc533hai_write_register(ViPipe, 0x3205, 0xe7);
	sc533hai_write_register(ViPipe, 0x3206, 0x06);
	sc533hai_write_register(ViPipe, 0x3207, 0xf1);
	sc533hai_write_register(ViPipe, 0x3208, 0x0b);
	sc533hai_write_register(ViPipe, 0x3209, 0x40);
	sc533hai_write_register(ViPipe, 0x320a, 0x06);
	sc533hai_write_register(ViPipe, 0x320b, 0x54);
	sc533hai_write_register(ViPipe, 0x320c, 0x03);
	sc533hai_write_register(ViPipe, 0x320d, 0xe8);
	sc533hai_write_register(ViPipe, 0x320e, 0x0c);
	sc533hai_write_register(ViPipe, 0x320f, 0xe4);
	sc533hai_write_register(ViPipe, 0x3210, 0x00);
	sc533hai_write_register(ViPipe, 0x3211, 0x04);
	sc533hai_write_register(ViPipe, 0x3212, 0x00);
	sc533hai_write_register(ViPipe, 0x3213, 0x04);
	sc533hai_write_register(ViPipe, 0x3214, 0x11);
	sc533hai_write_register(ViPipe, 0x3215, 0x11);
	sc533hai_write_register(ViPipe, 0x3223, 0xc0);
	sc533hai_write_register(ViPipe, 0x3250, 0xff);
	sc533hai_write_register(ViPipe, 0x3271, 0x10);
	sc533hai_write_register(ViPipe, 0x327f, 0x3f);
	sc533hai_write_register(ViPipe, 0x3281, 0x01);
	sc533hai_write_register(ViPipe, 0x32e0, 0x00);
	sc533hai_write_register(ViPipe, 0x3301, 0x12);
	sc533hai_write_register(ViPipe, 0x3304, 0x50);
	sc533hai_write_register(ViPipe, 0x3305, 0x00);
	sc533hai_write_register(ViPipe, 0x3306, 0x70);
	sc533hai_write_register(ViPipe, 0x3308, 0x18);
	sc533hai_write_register(ViPipe, 0x3309, 0xb0);
	sc533hai_write_register(ViPipe, 0x330a, 0x01);
	sc533hai_write_register(ViPipe, 0x330b, 0x20);
	sc533hai_write_register(ViPipe, 0x331e, 0x39);
	sc533hai_write_register(ViPipe, 0x331f, 0x99);
	sc533hai_write_register(ViPipe, 0x3333, 0x10);
	sc533hai_write_register(ViPipe, 0x3334, 0x40);
	sc533hai_write_register(ViPipe, 0x3364, 0x5e);
	sc533hai_write_register(ViPipe, 0x338f, 0xa0);
	sc533hai_write_register(ViPipe, 0x3393, 0x18);
	sc533hai_write_register(ViPipe, 0x3394, 0x2c);
	sc533hai_write_register(ViPipe, 0x3395, 0x3c);
	sc533hai_write_register(ViPipe, 0x3399, 0x12);
	sc533hai_write_register(ViPipe, 0x339a, 0x16);
	sc533hai_write_register(ViPipe, 0x339b, 0x1e);
	sc533hai_write_register(ViPipe, 0x339c, 0x3e);
	sc533hai_write_register(ViPipe, 0x33ac, 0x0c);
	sc533hai_write_register(ViPipe, 0x33ad, 0x2c);
	sc533hai_write_register(ViPipe, 0x33ae, 0x30);
	sc533hai_write_register(ViPipe, 0x33af, 0x90);
	sc533hai_write_register(ViPipe, 0x33b0, 0x0f);
	sc533hai_write_register(ViPipe, 0x33b1, 0x80);
	sc533hai_write_register(ViPipe, 0x33b2, 0x24);
	sc533hai_write_register(ViPipe, 0x33b3, 0x10);
	sc533hai_write_register(ViPipe, 0x33f8, 0x00);
	sc533hai_write_register(ViPipe, 0x33f9, 0x70);
	sc533hai_write_register(ViPipe, 0x33fa, 0x00);
	sc533hai_write_register(ViPipe, 0x33fb, 0x70);
	sc533hai_write_register(ViPipe, 0x349f, 0x03);
	sc533hai_write_register(ViPipe, 0x34a8, 0x10);
	sc533hai_write_register(ViPipe, 0x34a9, 0x10);
	sc533hai_write_register(ViPipe, 0x34aa, 0x01);
	sc533hai_write_register(ViPipe, 0x34ab, 0x20);
	sc533hai_write_register(ViPipe, 0x34ac, 0x01);
	sc533hai_write_register(ViPipe, 0x34ad, 0x20);
	sc533hai_write_register(ViPipe, 0x34f9, 0x12);
	sc533hai_write_register(ViPipe, 0x3632, 0x6d);
	sc533hai_write_register(ViPipe, 0x3633, 0x4d);
	sc533hai_write_register(ViPipe, 0x363a, 0x80);
	sc533hai_write_register(ViPipe, 0x363b, 0x57);
	sc533hai_write_register(ViPipe, 0x363c, 0xd8);
	sc533hai_write_register(ViPipe, 0x363d, 0x40);
	sc533hai_write_register(ViPipe, 0x3670, 0x41);
	sc533hai_write_register(ViPipe, 0x3671, 0x31);
	sc533hai_write_register(ViPipe, 0x3672, 0x31);
	sc533hai_write_register(ViPipe, 0x3673, 0x04);
	sc533hai_write_register(ViPipe, 0x3674, 0x08);
	sc533hai_write_register(ViPipe, 0x3675, 0x04);
	sc533hai_write_register(ViPipe, 0x3676, 0x18);
	sc533hai_write_register(ViPipe, 0x367e, 0x69);
	sc533hai_write_register(ViPipe, 0x367f, 0x6d);
	sc533hai_write_register(ViPipe, 0x3680, 0x8d);
	sc533hai_write_register(ViPipe, 0x3681, 0x04);
	sc533hai_write_register(ViPipe, 0x3682, 0x08);
	sc533hai_write_register(ViPipe, 0x3683, 0x04);
	sc533hai_write_register(ViPipe, 0x3684, 0x78);
	sc533hai_write_register(ViPipe, 0x3685, 0x80);
	sc533hai_write_register(ViPipe, 0x3686, 0x80);
	sc533hai_write_register(ViPipe, 0x3687, 0x83);
	sc533hai_write_register(ViPipe, 0x3688, 0x82);
	sc533hai_write_register(ViPipe, 0x3689, 0x85);
	sc533hai_write_register(ViPipe, 0x368a, 0x8b);
	sc533hai_write_register(ViPipe, 0x368b, 0x97);
	sc533hai_write_register(ViPipe, 0x368c, 0xbf);
	sc533hai_write_register(ViPipe, 0x368d, 0x00);
	sc533hai_write_register(ViPipe, 0x368e, 0x08);
	sc533hai_write_register(ViPipe, 0x368f, 0x00);
	sc533hai_write_register(ViPipe, 0x3690, 0x18);
	sc533hai_write_register(ViPipe, 0x3691, 0x04);
	sc533hai_write_register(ViPipe, 0x3692, 0x00);
	sc533hai_write_register(ViPipe, 0x3693, 0x04);
	sc533hai_write_register(ViPipe, 0x3694, 0x08);
	sc533hai_write_register(ViPipe, 0x3695, 0x04);
	sc533hai_write_register(ViPipe, 0x3696, 0x18);
	sc533hai_write_register(ViPipe, 0x3697, 0x04);
	sc533hai_write_register(ViPipe, 0x3698, 0x38);
	sc533hai_write_register(ViPipe, 0x3699, 0x04);
	sc533hai_write_register(ViPipe, 0x369a, 0x78);
	sc533hai_write_register(ViPipe, 0x36d0, 0x0d);
	sc533hai_write_register(ViPipe, 0x36ea, 0x18);
	sc533hai_write_register(ViPipe, 0x36eb, 0x45);
	sc533hai_write_register(ViPipe, 0x36ec, 0x4b);
	sc533hai_write_register(ViPipe, 0x36ed, 0x18);
	sc533hai_write_register(ViPipe, 0x370f, 0x13);
	sc533hai_write_register(ViPipe, 0x3721, 0x6c);
	sc533hai_write_register(ViPipe, 0x3722, 0x8b);
	sc533hai_write_register(ViPipe, 0x3724, 0xc1);
	sc533hai_write_register(ViPipe, 0x3726, 0x01);
	sc533hai_write_register(ViPipe, 0x3727, 0x24);
	sc533hai_write_register(ViPipe, 0x3729, 0xb4);
	sc533hai_write_register(ViPipe, 0x37b0, 0x7f);
	sc533hai_write_register(ViPipe, 0x37b1, 0x7f);
	sc533hai_write_register(ViPipe, 0x37b2, 0x73);
	sc533hai_write_register(ViPipe, 0x37b3, 0x04);
	sc533hai_write_register(ViPipe, 0x37b4, 0x08);
	sc533hai_write_register(ViPipe, 0x37b5, 0x04);
	sc533hai_write_register(ViPipe, 0x37b6, 0x38);
	sc533hai_write_register(ViPipe, 0x37b7, 0x11);
	sc533hai_write_register(ViPipe, 0x37b8, 0x11);
	sc533hai_write_register(ViPipe, 0x37b9, 0x00);
	sc533hai_write_register(ViPipe, 0x37ba, 0x94);
	sc533hai_write_register(ViPipe, 0x37bb, 0xd4);
	sc533hai_write_register(ViPipe, 0x37bc, 0x84);
	sc533hai_write_register(ViPipe, 0x37bd, 0x00);
	sc533hai_write_register(ViPipe, 0x37be, 0x08);
	sc533hai_write_register(ViPipe, 0x37bf, 0x04);
	sc533hai_write_register(ViPipe, 0x37c0, 0x00);
	sc533hai_write_register(ViPipe, 0x37c1, 0x04);
	sc533hai_write_register(ViPipe, 0x37c2, 0x18);
	sc533hai_write_register(ViPipe, 0x37c3, 0x04);
	sc533hai_write_register(ViPipe, 0x37c4, 0x3c);
	sc533hai_write_register(ViPipe, 0x37fa, 0x0b);
	sc533hai_write_register(ViPipe, 0x37fb, 0x55);
	sc533hai_write_register(ViPipe, 0x37fc, 0x19);
	sc533hai_write_register(ViPipe, 0x37fd, 0x3a);
	sc533hai_write_register(ViPipe, 0x3900, 0x05);
	sc533hai_write_register(ViPipe, 0x3903, 0x60);
	sc533hai_write_register(ViPipe, 0x3905, 0x0d);
	sc533hai_write_register(ViPipe, 0x391a, 0x60);
	sc533hai_write_register(ViPipe, 0x391b, 0x40);
	sc533hai_write_register(ViPipe, 0x391c, 0x26);
	sc533hai_write_register(ViPipe, 0x391d, 0x00);
	sc533hai_write_register(ViPipe, 0x3926, 0xe0);
	sc533hai_write_register(ViPipe, 0x3933, 0x80);
	sc533hai_write_register(ViPipe, 0x3934, 0x06);
	sc533hai_write_register(ViPipe, 0x3935, 0x00);
	sc533hai_write_register(ViPipe, 0x3936, 0x72);
	sc533hai_write_register(ViPipe, 0x3937, 0x71);
	sc533hai_write_register(ViPipe, 0x3938, 0x75);
	sc533hai_write_register(ViPipe, 0x3939, 0x0f);
	sc533hai_write_register(ViPipe, 0x393a, 0xf3);
	sc533hai_write_register(ViPipe, 0x393b, 0x0f);
	sc533hai_write_register(ViPipe, 0x393c, 0xd8);
	sc533hai_write_register(ViPipe, 0x393f, 0x80);
	sc533hai_write_register(ViPipe, 0x3940, 0x0b);
	sc533hai_write_register(ViPipe, 0x3941, 0x00);
	sc533hai_write_register(ViPipe, 0x3942, 0x0b);
	sc533hai_write_register(ViPipe, 0x3943, 0x7e);
	sc533hai_write_register(ViPipe, 0x3944, 0x7f);
	sc533hai_write_register(ViPipe, 0x3945, 0x7f);
	sc533hai_write_register(ViPipe, 0x3946, 0x7e);
	sc533hai_write_register(ViPipe, 0x39dd, 0x00);
	sc533hai_write_register(ViPipe, 0x39de, 0x08);
	sc533hai_write_register(ViPipe, 0x39e7, 0x04);
	sc533hai_write_register(ViPipe, 0x39e8, 0x04);
	sc533hai_write_register(ViPipe, 0x39e9, 0x80);
	sc533hai_write_register(ViPipe, 0x3e00, 0x00);
	sc533hai_write_register(ViPipe, 0x3e01, 0xbf);
	sc533hai_write_register(ViPipe, 0x3e02, 0x00);
	sc533hai_write_register(ViPipe, 0x3e03, 0x0b);
	sc533hai_write_register(ViPipe, 0x3e04, 0x0b);
	sc533hai_write_register(ViPipe, 0x3e05, 0xf0);
	sc533hai_write_register(ViPipe, 0x3e08, 0x00);
	sc533hai_write_register(ViPipe, 0x3e16, 0x01);
	sc533hai_write_register(ViPipe, 0x3e17, 0x54);
	sc533hai_write_register(ViPipe, 0x3e18, 0x01);
	sc533hai_write_register(ViPipe, 0x3e19, 0x54);
	sc533hai_write_register(ViPipe, 0x3e1b, 0x29);
	sc533hai_write_register(ViPipe, 0x3e23, 0x00);
	sc533hai_write_register(ViPipe, 0x3e24, 0xcc);
	sc533hai_write_register(ViPipe, 0x4402, 0x11);
	sc533hai_write_register(ViPipe, 0x450a, 0x80);
	sc533hai_write_register(ViPipe, 0x450d, 0x0a);
	sc533hai_write_register(ViPipe, 0x4800, 0x24);
	sc533hai_write_register(ViPipe, 0x480f, 0x03);
	sc533hai_write_register(ViPipe, 0x4837, 0x25);
	sc533hai_write_register(ViPipe, 0x5000, 0x26);
	sc533hai_write_register(ViPipe, 0x5780, 0x77);
	sc533hai_write_register(ViPipe, 0x5784, 0x10);
	sc533hai_write_register(ViPipe, 0x5785, 0x08);
	sc533hai_write_register(ViPipe, 0x5787, 0x0a);
	sc533hai_write_register(ViPipe, 0x5788, 0x0a);
	sc533hai_write_register(ViPipe, 0x5789, 0x08);
	sc533hai_write_register(ViPipe, 0x578a, 0x0a);
	sc533hai_write_register(ViPipe, 0x578b, 0x0a);
	sc533hai_write_register(ViPipe, 0x578c, 0x08);
	sc533hai_write_register(ViPipe, 0x578d, 0x41);
	sc533hai_write_register(ViPipe, 0x5790, 0x08);
	sc533hai_write_register(ViPipe, 0x5791, 0x04);
	sc533hai_write_register(ViPipe, 0x5792, 0x04);
	sc533hai_write_register(ViPipe, 0x5793, 0x08);
	sc533hai_write_register(ViPipe, 0x5794, 0x04);
	sc533hai_write_register(ViPipe, 0x5795, 0x04);
	sc533hai_write_register(ViPipe, 0x5799, 0x46);
	sc533hai_write_register(ViPipe, 0x579a, 0x77);
	sc533hai_write_register(ViPipe, 0x57a1, 0x04);
	sc533hai_write_register(ViPipe, 0x57a4, 0x02);
	sc533hai_write_register(ViPipe, 0x57a5, 0x00);
	sc533hai_write_register(ViPipe, 0x57a8, 0xd2);
	sc533hai_write_register(ViPipe, 0x57aa, 0x2a);
	sc533hai_write_register(ViPipe, 0x57ab, 0x7f);
	sc533hai_write_register(ViPipe, 0x57ac, 0x00);
	sc533hai_write_register(ViPipe, 0x57ad, 0x00);
	sc533hai_write_register(ViPipe, 0x58c0, 0x30);
	sc533hai_write_register(ViPipe, 0x58c1, 0x28);
	sc533hai_write_register(ViPipe, 0x58c2, 0x20);
	sc533hai_write_register(ViPipe, 0x58c3, 0x30);
	sc533hai_write_register(ViPipe, 0x58c4, 0x28);
	sc533hai_write_register(ViPipe, 0x58c5, 0x20);
	sc533hai_write_register(ViPipe, 0x58c6, 0x3c);
	sc533hai_write_register(ViPipe, 0x58c7, 0x30);
	sc533hai_write_register(ViPipe, 0x58c8, 0x28);
	sc533hai_write_register(ViPipe, 0x58c9, 0x3c);
	sc533hai_write_register(ViPipe, 0x58ca, 0x30);
	sc533hai_write_register(ViPipe, 0x58cb, 0x28);
	sc533hai_write_register(ViPipe, 0x36e9, 0x53);
	sc533hai_write_register(ViPipe, 0x37f9, 0x23);

	sc533hai_default_reg_init(ViPipe);
	sc533hai_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(100);

	printf("ViPipe:%d,===SC533HAI 1620P 30fps 10bit 2to1 WDR 24M Init OK!===\n", ViPipe);
}
