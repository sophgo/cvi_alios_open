#include "cvi_sns_ctrl.h"

#include "cvi_comm_video.h"
#include "sc285sl_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#define SC285SL_CHIP_ID_ADDR_H 0x3107
#define SC285SL_CHIP_ID_ADDR_L 0x3108
#define SC285SL_CHIP_ID 0xbb98

CVI_U8 sc285sl_i2c_addr = 0x30;
const CVI_U32 sc285sl_addr_byte = 2;
const CVI_U32 sc285sl_data_byte = 1;

static void sc285sl_linear_1080p30_init(VI_PIPE ViPipe);
static void sc285sl_wdr_1080p30_2to1_init(VI_PIPE ViPipe);

int sc285sl_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc285sl_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc285sl_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc285sl_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc285sl_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc285sl_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc285sl_i2c_addr, (CVI_U32)addr, sc285sl_addr_byte,
						   sc285sl_data_byte);
}

int sc285sl_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc285sl_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sc285sl_i2c_addr, (CVI_U32)addr, sc285sl_addr_byte,
							(CVI_U32)data, sc285sl_data_byte);
}

void sc285sl_standby(VI_PIPE ViPipe)
{
	sc285sl_write_register(ViPipe, 0x3019, 0xff);
	sc285sl_write_register(ViPipe, 0x0100, 0x00);

	printf("%s i2c_addr:%x, i2c_dev:%d\n", __func__, sc285sl_i2c_addr,
		   g_aunSc285sl_BusInfo[ViPipe].s8I2cDev);
}

void sc285sl_restart(VI_PIPE ViPipe)
{
	sc285sl_write_register(ViPipe, 0x3019, 0xfe);
	sc285sl_write_register(ViPipe, 0x0100, 0x01);

	printf("%s i2c_addr:%x, i2c_dev:%d\n", __func__, sc285sl_i2c_addr,
		   g_aunSc285sl_BusInfo[ViPipe].s8I2cDev);
}

void sc285sl_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSc285sl[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc285sl_write_register(
			ViPipe, g_pastSc285sl[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
			g_pastSc285sl[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc285sl_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	int reg_val = sc285sl_read_register(ViPipe, 0x3221);
	CVI_U8 val;

	if (reg_val < 0)
		return;

	val = (CVI_U8)reg_val;
	val &= ~0x66;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x06;
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

	sc285sl_write_register(ViPipe, 0x3221, val);
}

int sc285sl_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	if (sc285sl_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc285sl_read_register(ViPipe, SC285SL_CHIP_ID_ADDR_H);
	nVal2 = sc285sl_read_register(ViPipe, SC285SL_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != SC285SL_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		CVI_TRACE_SNS(CVI_DBG_ERR, "nVal:%#x, nVal2:%#x\n", nVal, nVal2);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sc285sl_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastSc285sl[ViPipe]->enWDRMode;
	CVI_U8 u8ImgMode = g_pastSc285sl[ViPipe]->u8ImgMode;

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == SC285SL_MODE_1920X1080P30_WDR) {
			sc285sl_wdr_1080p30_2to1_init(ViPipe);
		}
	} else {
		sc285sl_linear_1080p30_init(ViPipe);
	}
	g_pastSc285sl[ViPipe]->bInit = CVI_TRUE;
}

void sc285sl_exit(VI_PIPE ViPipe)
{
	sc285sl_i2c_exit(ViPipe);
}

static void sc285sl_linear_1080p30_init(VI_PIPE ViPipe)
{
	sc285sl_write_register(ViPipe, 0x0103, 0x01);
	sc285sl_write_register(ViPipe, 0x0100, 0x00);
	sc285sl_write_register(ViPipe, 0x36e9, 0x80);
	sc285sl_write_register(ViPipe, 0x37f9, 0x80);
	sc285sl_write_register(ViPipe, 0x23b0, 0x00);
	sc285sl_write_register(ViPipe, 0x23b1, 0x08);
	sc285sl_write_register(ViPipe, 0x23b2, 0x00);
	sc285sl_write_register(ViPipe, 0x23b3, 0x18);
	sc285sl_write_register(ViPipe, 0x23b4, 0x00);
	sc285sl_write_register(ViPipe, 0x23b5, 0x38);
	sc285sl_write_register(ViPipe, 0x23b6, 0x04);
	sc285sl_write_register(ViPipe, 0x23b7, 0x08);
	sc285sl_write_register(ViPipe, 0x23b8, 0x04);
	sc285sl_write_register(ViPipe, 0x23b9, 0x18);
	sc285sl_write_register(ViPipe, 0x23ba, 0x04);
	sc285sl_write_register(ViPipe, 0x23bb, 0x38);
	sc285sl_write_register(ViPipe, 0x23bc, 0x04);
	sc285sl_write_register(ViPipe, 0x23bd, 0x08);
	sc285sl_write_register(ViPipe, 0x23be, 0x04);
	sc285sl_write_register(ViPipe, 0x23bf, 0x78);
	sc285sl_write_register(ViPipe, 0x23c0, 0x04);
	sc285sl_write_register(ViPipe, 0x23c1, 0x00);
	sc285sl_write_register(ViPipe, 0x23c2, 0x04);
	sc285sl_write_register(ViPipe, 0x23c3, 0x18);
	sc285sl_write_register(ViPipe, 0x23c4, 0x04);
	sc285sl_write_register(ViPipe, 0x23c5, 0x38);
	sc285sl_write_register(ViPipe, 0x23c6, 0x04);
	sc285sl_write_register(ViPipe, 0x23c7, 0x08);
	sc285sl_write_register(ViPipe, 0x23c8, 0x04);
	sc285sl_write_register(ViPipe, 0x23c9, 0x78);
	sc285sl_write_register(ViPipe, 0x3018, 0x3a);
	sc285sl_write_register(ViPipe, 0x3019, 0x0c);
	sc285sl_write_register(ViPipe, 0x301c, 0x70);
	sc285sl_write_register(ViPipe, 0x301e, 0xf0);
	sc285sl_write_register(ViPipe, 0x301f, 0x1b);
	sc285sl_write_register(ViPipe, 0x302b, 0x6f);
	sc285sl_write_register(ViPipe, 0x3037, 0x00);
	sc285sl_write_register(ViPipe, 0x3058, 0x21);
	sc285sl_write_register(ViPipe, 0x3059, 0x43);
	sc285sl_write_register(ViPipe, 0x30b0, 0x00);
	sc285sl_write_register(ViPipe, 0x30b8, 0x44);
	sc285sl_write_register(ViPipe, 0x3107, 0xbb);
	sc285sl_write_register(ViPipe, 0x3108, 0x98);
	sc285sl_write_register(ViPipe, 0x3204, 0x07);
	sc285sl_write_register(ViPipe, 0x3205, 0x8f);
	sc285sl_write_register(ViPipe, 0x3206, 0x04);
	sc285sl_write_register(ViPipe, 0x3207, 0x47);
	sc285sl_write_register(ViPipe, 0x3208, 0x07);
	sc285sl_write_register(ViPipe, 0x3209, 0x80);
	sc285sl_write_register(ViPipe, 0x320a, 0x04);
	sc285sl_write_register(ViPipe, 0x320b, 0x38);
	sc285sl_write_register(ViPipe, 0x320c, 0x0a);
	sc285sl_write_register(ViPipe, 0x320d, 0x8c);
	sc285sl_write_register(ViPipe, 0x320e, 0x04);
	sc285sl_write_register(ViPipe, 0x320f, 0x65);
	sc285sl_write_register(ViPipe, 0x3210, 0x00);
	sc285sl_write_register(ViPipe, 0x3211, 0x08);
	sc285sl_write_register(ViPipe, 0x3212, 0x00);
	sc285sl_write_register(ViPipe, 0x3213, 0x08);
	sc285sl_write_register(ViPipe, 0x3214, 0x11);
	sc285sl_write_register(ViPipe, 0x3215, 0x11);
	sc285sl_write_register(ViPipe, 0x3220, 0x00);
	sc285sl_write_register(ViPipe, 0x3223, 0xc0);
	sc285sl_write_register(ViPipe, 0x3250, 0x00);
	sc285sl_write_register(ViPipe, 0x325f, 0xa0);
	sc285sl_write_register(ViPipe, 0x3271, 0x10);
	sc285sl_write_register(ViPipe, 0x3273, 0x13);
	sc285sl_write_register(ViPipe, 0x32d1, 0x90);
	sc285sl_write_register(ViPipe, 0x32e0, 0x00);
	sc285sl_write_register(ViPipe, 0x3301, 0x08);
	sc285sl_write_register(ViPipe, 0x3304, 0x80);
	sc285sl_write_register(ViPipe, 0x3305, 0x00);
	sc285sl_write_register(ViPipe, 0x3306, 0x60);
	sc285sl_write_register(ViPipe, 0x3309, 0xa0);
	sc285sl_write_register(ViPipe, 0x330a, 0x00);
	sc285sl_write_register(ViPipe, 0x330b, 0xc0);
	sc285sl_write_register(ViPipe, 0x331c, 0x01);
	sc285sl_write_register(ViPipe, 0x331e, 0x69);
	sc285sl_write_register(ViPipe, 0x331f, 0x89);
	sc285sl_write_register(ViPipe, 0x3320, 0x0c);
	sc285sl_write_register(ViPipe, 0x3326, 0x14);
	sc285sl_write_register(ViPipe, 0x3328, 0x04);
	sc285sl_write_register(ViPipe, 0x3329, 0x08);
	sc285sl_write_register(ViPipe, 0x3353, 0x14);
	sc285sl_write_register(ViPipe, 0x3364, 0x5e);
	sc285sl_write_register(ViPipe, 0x3366, 0x08);
	sc285sl_write_register(ViPipe, 0x338f, 0xa0);
	sc285sl_write_register(ViPipe, 0x3393, 0x0c);
	sc285sl_write_register(ViPipe, 0x3394, 0x12);
	sc285sl_write_register(ViPipe, 0x3395, 0x3c);
	sc285sl_write_register(ViPipe, 0x3399, 0x08);
	sc285sl_write_register(ViPipe, 0x339a, 0x10);
	sc285sl_write_register(ViPipe, 0x339b, 0x12);
	sc285sl_write_register(ViPipe, 0x339c, 0x50);
	sc285sl_write_register(ViPipe, 0x33ac, 0x18);
	sc285sl_write_register(ViPipe, 0x33ad, 0x2c);
	sc285sl_write_register(ViPipe, 0x33ae, 0x28);
	sc285sl_write_register(ViPipe, 0x33af, 0x60);
	sc285sl_write_register(ViPipe, 0x33b0, 0x0f);
	sc285sl_write_register(ViPipe, 0x33b2, 0x54);
	sc285sl_write_register(ViPipe, 0x33b3, 0x18);
	sc285sl_write_register(ViPipe, 0x33f8, 0x00);
	sc285sl_write_register(ViPipe, 0x33f9, 0x80);
	sc285sl_write_register(ViPipe, 0x33fa, 0x00);
	sc285sl_write_register(ViPipe, 0x33fb, 0xa0);
	sc285sl_write_register(ViPipe, 0x3428, 0x04);
	sc285sl_write_register(ViPipe, 0x3429, 0x04);
	sc285sl_write_register(ViPipe, 0x342a, 0x04);
	sc285sl_write_register(ViPipe, 0x3447, 0x11);
	sc285sl_write_register(ViPipe, 0x349f, 0x03);
	sc285sl_write_register(ViPipe, 0x34a5, 0x01);
	sc285sl_write_register(ViPipe, 0x34a8, 0x10);
	sc285sl_write_register(ViPipe, 0x34a9, 0x0c);
	sc285sl_write_register(ViPipe, 0x34aa, 0x00);
	sc285sl_write_register(ViPipe, 0x34ab, 0xe0);
	sc285sl_write_register(ViPipe, 0x34ac, 0x01);
	sc285sl_write_register(ViPipe, 0x34ad, 0x00);
	sc285sl_write_register(ViPipe, 0x34ba, 0x04);
	sc285sl_write_register(ViPipe, 0x34bb, 0x0f);
	sc285sl_write_register(ViPipe, 0x34f9, 0x04);
	sc285sl_write_register(ViPipe, 0x3622, 0x04);
	sc285sl_write_register(ViPipe, 0x3623, 0x38);
	sc285sl_write_register(ViPipe, 0x3624, 0x04);
	sc285sl_write_register(ViPipe, 0x3625, 0x78);
	sc285sl_write_register(ViPipe, 0x3626, 0x04);
	sc285sl_write_register(ViPipe, 0x3627, 0x38);
	sc285sl_write_register(ViPipe, 0x362f, 0x10);
	sc285sl_write_register(ViPipe, 0x3632, 0x64);
	sc285sl_write_register(ViPipe, 0x3633, 0x44);
	sc285sl_write_register(ViPipe, 0x3634, 0x24);
	sc285sl_write_register(ViPipe, 0x3637, 0x5c);
	sc285sl_write_register(ViPipe, 0x363d, 0x24);
	sc285sl_write_register(ViPipe, 0x3670, 0x41);
	sc285sl_write_register(ViPipe, 0x3671, 0x42);
	sc285sl_write_register(ViPipe, 0x3672, 0x33);
	sc285sl_write_register(ViPipe, 0x3673, 0x04);
	sc285sl_write_register(ViPipe, 0x3674, 0x08);
	sc285sl_write_register(ViPipe, 0x3675, 0x04);
	sc285sl_write_register(ViPipe, 0x3676, 0x18);
	sc285sl_write_register(ViPipe, 0x367e, 0x44);
	sc285sl_write_register(ViPipe, 0x367f, 0x44);
	sc285sl_write_register(ViPipe, 0x3680, 0x34);
	sc285sl_write_register(ViPipe, 0x3681, 0x04);
	sc285sl_write_register(ViPipe, 0x3682, 0x08);
	sc285sl_write_register(ViPipe, 0x3683, 0x04);
	sc285sl_write_register(ViPipe, 0x3684, 0x18);
	sc285sl_write_register(ViPipe, 0x3685, 0x80);
	sc285sl_write_register(ViPipe, 0x3686, 0x80);
	sc285sl_write_register(ViPipe, 0x3687, 0x80);
	sc285sl_write_register(ViPipe, 0x3688, 0x80);
	sc285sl_write_register(ViPipe, 0x3689, 0x80);
	sc285sl_write_register(ViPipe, 0x368a, 0x80);
	sc285sl_write_register(ViPipe, 0x368b, 0x80);
	sc285sl_write_register(ViPipe, 0x368c, 0x80);
	sc285sl_write_register(ViPipe, 0x368d, 0x00);
	sc285sl_write_register(ViPipe, 0x368e, 0x08);
	sc285sl_write_register(ViPipe, 0x368f, 0x00);
	sc285sl_write_register(ViPipe, 0x3690, 0x18);
	sc285sl_write_register(ViPipe, 0x3691, 0x04);
	sc285sl_write_register(ViPipe, 0x3692, 0x00);
	sc285sl_write_register(ViPipe, 0x3693, 0x04);
	sc285sl_write_register(ViPipe, 0x3694, 0x08);
	sc285sl_write_register(ViPipe, 0x3695, 0x04);
	sc285sl_write_register(ViPipe, 0x3696, 0x18);
	sc285sl_write_register(ViPipe, 0x3697, 0x04);
	sc285sl_write_register(ViPipe, 0x3698, 0x38);
	sc285sl_write_register(ViPipe, 0x3699, 0x04);
	sc285sl_write_register(ViPipe, 0x369a, 0x78);
	sc285sl_write_register(ViPipe, 0x36b9, 0x33);
	sc285sl_write_register(ViPipe, 0x36ba, 0x33);
	sc285sl_write_register(ViPipe, 0x36bb, 0x23);
	sc285sl_write_register(ViPipe, 0x36bc, 0x04);
	sc285sl_write_register(ViPipe, 0x36bd, 0x08);
	sc285sl_write_register(ViPipe, 0x36be, 0x04);
	sc285sl_write_register(ViPipe, 0x36bf, 0x38);
	sc285sl_write_register(ViPipe, 0x36d0, 0x09);
	sc285sl_write_register(ViPipe, 0x36d1, 0x30);
	sc285sl_write_register(ViPipe, 0x36d2, 0x33);
	sc285sl_write_register(ViPipe, 0x36d3, 0x33);
	sc285sl_write_register(ViPipe, 0x36d4, 0x33);
	sc285sl_write_register(ViPipe, 0x36d5, 0x04);
	sc285sl_write_register(ViPipe, 0x36d6, 0x00);
	sc285sl_write_register(ViPipe, 0x36d7, 0x04);
	sc285sl_write_register(ViPipe, 0x36d8, 0x18);
	sc285sl_write_register(ViPipe, 0x36e0, 0x33);
	sc285sl_write_register(ViPipe, 0x36e1, 0x53);
	sc285sl_write_register(ViPipe, 0x36e2, 0x34);
	sc285sl_write_register(ViPipe, 0x36ea, 0x16);
	sc285sl_write_register(ViPipe, 0x36eb, 0x45);
	sc285sl_write_register(ViPipe, 0x36ec, 0x53);
	sc285sl_write_register(ViPipe, 0x36ed, 0x28);
	sc285sl_write_register(ViPipe, 0x370f, 0x71);
	sc285sl_write_register(ViPipe, 0x3722, 0x03);
	sc285sl_write_register(ViPipe, 0x3724, 0xb0);
	sc285sl_write_register(ViPipe, 0x3727, 0x20);
	sc285sl_write_register(ViPipe, 0x3729, 0x82);
	sc285sl_write_register(ViPipe, 0x37b0, 0x03);
	sc285sl_write_register(ViPipe, 0x37b1, 0x03);
	sc285sl_write_register(ViPipe, 0x37b2, 0x33);
	sc285sl_write_register(ViPipe, 0x37b3, 0x04);
	sc285sl_write_register(ViPipe, 0x37b4, 0x08);
	sc285sl_write_register(ViPipe, 0x37b5, 0x04);
	sc285sl_write_register(ViPipe, 0x37b6, 0x78);
	sc285sl_write_register(ViPipe, 0x37b7, 0x82);
	sc285sl_write_register(ViPipe, 0x37b8, 0x82);
	sc285sl_write_register(ViPipe, 0x37b9, 0x82);
	sc285sl_write_register(ViPipe, 0x37ba, 0x80);
	sc285sl_write_register(ViPipe, 0x37bb, 0x80);
	sc285sl_write_register(ViPipe, 0x37bc, 0x80);
	sc285sl_write_register(ViPipe, 0x37bd, 0x04);
	sc285sl_write_register(ViPipe, 0x37be, 0x08);
	sc285sl_write_register(ViPipe, 0x37bf, 0x04);
	sc285sl_write_register(ViPipe, 0x37c0, 0x18);
	sc285sl_write_register(ViPipe, 0x37c1, 0x00);
	sc285sl_write_register(ViPipe, 0x37c2, 0x08);
	sc285sl_write_register(ViPipe, 0x37c3, 0x00);
	sc285sl_write_register(ViPipe, 0x37c4, 0x18);
	sc285sl_write_register(ViPipe, 0x37c5, 0x00);
	sc285sl_write_register(ViPipe, 0x37c6, 0x08);
	sc285sl_write_register(ViPipe, 0x37c7, 0x00);
	sc285sl_write_register(ViPipe, 0x37c8, 0x18);
	sc285sl_write_register(ViPipe, 0x37c9, 0x00);
	sc285sl_write_register(ViPipe, 0x37ca, 0x00);
	sc285sl_write_register(ViPipe, 0x37cb, 0x00);
	sc285sl_write_register(ViPipe, 0x37cc, 0x80);
	sc285sl_write_register(ViPipe, 0x37cd, 0x80);
	sc285sl_write_register(ViPipe, 0x37ce, 0x80);
	sc285sl_write_register(ViPipe, 0x37cf, 0x80);
	sc285sl_write_register(ViPipe, 0x37d6, 0x80);
	sc285sl_write_register(ViPipe, 0x37d7, 0x04);
	sc285sl_write_register(ViPipe, 0x37d8, 0x00);
	sc285sl_write_register(ViPipe, 0x37d9, 0x04);
	sc285sl_write_register(ViPipe, 0x37da, 0x08);
	sc285sl_write_register(ViPipe, 0x37db, 0x04);
	sc285sl_write_register(ViPipe, 0x37dc, 0x18);
	sc285sl_write_register(ViPipe, 0x37dd, 0x04);
	sc285sl_write_register(ViPipe, 0x37de, 0x38);
	sc285sl_write_register(ViPipe, 0x37df, 0x04);
	sc285sl_write_register(ViPipe, 0x37e0, 0x78);
	sc285sl_write_register(ViPipe, 0x37e1, 0x00);
	sc285sl_write_register(ViPipe, 0x37e2, 0x00);
	sc285sl_write_register(ViPipe, 0x37e3, 0x00);
	sc285sl_write_register(ViPipe, 0x37e4, 0x00);
	sc285sl_write_register(ViPipe, 0x37e5, 0x00);
	sc285sl_write_register(ViPipe, 0x37e6, 0x04);
	sc285sl_write_register(ViPipe, 0x37e7, 0x00);
	sc285sl_write_register(ViPipe, 0x37e8, 0x04);
	sc285sl_write_register(ViPipe, 0x37e9, 0x08);
	sc285sl_write_register(ViPipe, 0x37ea, 0x04);
	sc285sl_write_register(ViPipe, 0x37eb, 0x18);
	sc285sl_write_register(ViPipe, 0x37ec, 0x04);
	sc285sl_write_register(ViPipe, 0x37ed, 0x38);
	sc285sl_write_register(ViPipe, 0x37ee, 0x04);
	sc285sl_write_register(ViPipe, 0x37ef, 0x78);
	sc285sl_write_register(ViPipe, 0x37fa, 0x1b);
	sc285sl_write_register(ViPipe, 0x37fb, 0x65);
	sc285sl_write_register(ViPipe, 0x37fc, 0x18);
	sc285sl_write_register(ViPipe, 0x37fd, 0x35);
	sc285sl_write_register(ViPipe, 0x3900, 0x05);
	sc285sl_write_register(ViPipe, 0x3903, 0x40);
	sc285sl_write_register(ViPipe, 0x3907, 0x01);
	sc285sl_write_register(ViPipe, 0x3908, 0x00);
	sc285sl_write_register(ViPipe, 0x391a, 0x80);
	sc285sl_write_register(ViPipe, 0x391b, 0x60);
	sc285sl_write_register(ViPipe, 0x391c, 0x33);
	sc285sl_write_register(ViPipe, 0x391d, 0x00);
	sc285sl_write_register(ViPipe, 0x391f, 0x61);
	sc285sl_write_register(ViPipe, 0x3926, 0xe2);
	sc285sl_write_register(ViPipe, 0x3933, 0x80);
	sc285sl_write_register(ViPipe, 0x3934, 0x02);
	sc285sl_write_register(ViPipe, 0x3935, 0x01);
	sc285sl_write_register(ViPipe, 0x3936, 0x10);
	sc285sl_write_register(ViPipe, 0x3937, 0x75);
	sc285sl_write_register(ViPipe, 0x3938, 0x73);
	sc285sl_write_register(ViPipe, 0x3939, 0x00);
	sc285sl_write_register(ViPipe, 0x393a, 0x00);
	sc285sl_write_register(ViPipe, 0x393b, 0x00);
	sc285sl_write_register(ViPipe, 0x393c, 0x20);
	sc285sl_write_register(ViPipe, 0x39c9, 0x30);
	sc285sl_write_register(ViPipe, 0x39dd, 0x00);
	sc285sl_write_register(ViPipe, 0x39de, 0x0c);
	sc285sl_write_register(ViPipe, 0x39e7, 0x08);
	sc285sl_write_register(ViPipe, 0x39e8, 0x08);
	sc285sl_write_register(ViPipe, 0x39e9, 0x80);
	sc285sl_write_register(ViPipe, 0x3e00, 0x00);
	sc285sl_write_register(ViPipe, 0x3e01, 0x45);
	sc285sl_write_register(ViPipe, 0x3e02, 0xd0);
	sc285sl_write_register(ViPipe, 0x3e03, 0x0b);
	sc285sl_write_register(ViPipe, 0x3e06, 0x00);
	sc285sl_write_register(ViPipe, 0x3e07, 0x80);
	sc285sl_write_register(ViPipe, 0x3e08, 0x00);
	sc285sl_write_register(ViPipe, 0x3e09, 0x20);
	sc285sl_write_register(ViPipe, 0x3e16, 0x02);
	sc285sl_write_register(ViPipe, 0x3e17, 0xb3);
	sc285sl_write_register(ViPipe, 0x3e18, 0x01);
	sc285sl_write_register(ViPipe, 0x3e19, 0x59);
	sc285sl_write_register(ViPipe, 0x4402, 0x03);
	sc285sl_write_register(ViPipe, 0x4403, 0x0c);
	sc285sl_write_register(ViPipe, 0x4404, 0x22);
	sc285sl_write_register(ViPipe, 0x4405, 0x2c);
	sc285sl_write_register(ViPipe, 0x440c, 0x38);
	sc285sl_write_register(ViPipe, 0x440d, 0x38);
	sc285sl_write_register(ViPipe, 0x440e, 0x2a);
	sc285sl_write_register(ViPipe, 0x440f, 0x47);
	sc285sl_write_register(ViPipe, 0x4412, 0x01);
	sc285sl_write_register(ViPipe, 0x4424, 0x01);
	sc285sl_write_register(ViPipe, 0x4509, 0x1e);
	sc285sl_write_register(ViPipe, 0x450d, 0x0c);
	sc285sl_write_register(ViPipe, 0x4800, 0x24);
	sc285sl_write_register(ViPipe, 0x480f, 0x03);
	sc285sl_write_register(ViPipe, 0x4837, 0x2b);
	sc285sl_write_register(ViPipe, 0x5000, 0x2e);
	sc285sl_write_register(ViPipe, 0x5011, 0x01);
	sc285sl_write_register(ViPipe, 0x530d, 0x08);
	sc285sl_write_register(ViPipe, 0x5310, 0x01);
	sc285sl_write_register(ViPipe, 0x5311, 0x10);
	sc285sl_write_register(ViPipe, 0x5312, 0x04);
	sc285sl_write_register(ViPipe, 0x5314, 0x07);
	sc285sl_write_register(ViPipe, 0x5315, 0x10);
	sc285sl_write_register(ViPipe, 0x5316, 0x0b);
	sc285sl_write_register(ViPipe, 0x5318, 0x0e);
	sc285sl_write_register(ViPipe, 0x531a, 0x11);
	sc285sl_write_register(ViPipe, 0x531c, 0x14);
	sc285sl_write_register(ViPipe, 0x531d, 0x10);
	sc285sl_write_register(ViPipe, 0x531e, 0x1a);
	sc285sl_write_register(ViPipe, 0x531f, 0x10);
	sc285sl_write_register(ViPipe, 0x5400, 0x01);
	sc285sl_write_register(ViPipe, 0x5404, 0x0c);
	sc285sl_write_register(ViPipe, 0x5405, 0x10);
	sc285sl_write_register(ViPipe, 0x5784, 0x10);
	sc285sl_write_register(ViPipe, 0x5785, 0x08);
	sc285sl_write_register(ViPipe, 0x5787, 0x0a);
	sc285sl_write_register(ViPipe, 0x5788, 0x0a);
	sc285sl_write_register(ViPipe, 0x5789, 0x08);
	sc285sl_write_register(ViPipe, 0x578a, 0x0a);
	sc285sl_write_register(ViPipe, 0x578b, 0x0a);
	sc285sl_write_register(ViPipe, 0x578c, 0x08);
	sc285sl_write_register(ViPipe, 0x5790, 0x08);
	sc285sl_write_register(ViPipe, 0x5791, 0x04);
	sc285sl_write_register(ViPipe, 0x5792, 0x04);
	sc285sl_write_register(ViPipe, 0x5793, 0x08);
	sc285sl_write_register(ViPipe, 0x5794, 0x04);
	sc285sl_write_register(ViPipe, 0x5795, 0x04);
	sc285sl_write_register(ViPipe, 0x57ac, 0x00);
	sc285sl_write_register(ViPipe, 0x57ad, 0x00);
	sc285sl_write_register(ViPipe, 0x36e9, 0x50);
	sc285sl_write_register(ViPipe, 0x37f9, 0x20);
	sc285sl_write_register(ViPipe, 0x4402, 0x02);
	sc285sl_write_register(ViPipe, 0x4403, 0x0a);
	sc285sl_write_register(ViPipe, 0x4404, 0x1c);
	sc285sl_write_register(ViPipe, 0x4405, 0x24);
	sc285sl_write_register(ViPipe, 0x440c, 0x2e);
	sc285sl_write_register(ViPipe, 0x440d, 0x2e);
	sc285sl_write_register(ViPipe, 0x440e, 0x23);
	sc285sl_write_register(ViPipe, 0x440f, 0x3a);
	sc285sl_write_register(ViPipe, 0x4412, 0x01);
	sc285sl_write_register(ViPipe, 0x4406, 0x00);
	sc285sl_write_register(ViPipe, 0x4407, 0x7e);
	sc285sl_write_register(ViPipe, 0x4424, 0x01);
	sc285sl_write_register(ViPipe, 0x0100, 0x01);

	sc285sl_default_reg_init(ViPipe);

	printf("ViPipe:%d,===SC285SL 1080P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}

static void sc285sl_wdr_1080p30_2to1_init(VI_PIPE ViPipe)
{
	sc285sl_write_register(ViPipe, 0x0103, 0x01);
	sc285sl_write_register(ViPipe, 0x0100, 0x00);
	sc285sl_write_register(ViPipe, 0x36e9, 0x80);
	sc285sl_write_register(ViPipe, 0x37f9, 0x80);
	sc285sl_write_register(ViPipe, 0x23b0, 0x00);
	sc285sl_write_register(ViPipe, 0x23b1, 0x08);
	sc285sl_write_register(ViPipe, 0x23b2, 0x00);
	sc285sl_write_register(ViPipe, 0x23b3, 0x18);
	sc285sl_write_register(ViPipe, 0x23b4, 0x00);
	sc285sl_write_register(ViPipe, 0x23b5, 0x38);
	sc285sl_write_register(ViPipe, 0x23b6, 0x04);
	sc285sl_write_register(ViPipe, 0x23b7, 0x08);
	sc285sl_write_register(ViPipe, 0x23b8, 0x04);
	sc285sl_write_register(ViPipe, 0x23b9, 0x18);
	sc285sl_write_register(ViPipe, 0x23ba, 0x04);
	sc285sl_write_register(ViPipe, 0x23bb, 0x38);
	sc285sl_write_register(ViPipe, 0x23bc, 0x04);
	sc285sl_write_register(ViPipe, 0x23bd, 0x08);
	sc285sl_write_register(ViPipe, 0x23be, 0x04);
	sc285sl_write_register(ViPipe, 0x23bf, 0x78);
	sc285sl_write_register(ViPipe, 0x23c0, 0x04);
	sc285sl_write_register(ViPipe, 0x23c1, 0x00);
	sc285sl_write_register(ViPipe, 0x23c2, 0x04);
	sc285sl_write_register(ViPipe, 0x23c3, 0x18);
	sc285sl_write_register(ViPipe, 0x23c4, 0x04);
	sc285sl_write_register(ViPipe, 0x23c5, 0x38);
	sc285sl_write_register(ViPipe, 0x23c6, 0x04);
	sc285sl_write_register(ViPipe, 0x23c7, 0x08);
	sc285sl_write_register(ViPipe, 0x23c8, 0x04);
	sc285sl_write_register(ViPipe, 0x23c9, 0x78);
	sc285sl_write_register(ViPipe, 0x3018, 0x3a);
	sc285sl_write_register(ViPipe, 0x3019, 0x0c);
	sc285sl_write_register(ViPipe, 0x301c, 0x70);
	sc285sl_write_register(ViPipe, 0x301e, 0xf0);
	sc285sl_write_register(ViPipe, 0x301f, 0x4f);
	sc285sl_write_register(ViPipe, 0x302b, 0x6f);
	sc285sl_write_register(ViPipe, 0x3037, 0x00);
	sc285sl_write_register(ViPipe, 0x3058, 0x21);
	sc285sl_write_register(ViPipe, 0x3059, 0x43);
	sc285sl_write_register(ViPipe, 0x30b0, 0x00);
	sc285sl_write_register(ViPipe, 0x30b8, 0x44);
	sc285sl_write_register(ViPipe, 0x3107, 0xbb);
	sc285sl_write_register(ViPipe, 0x3108, 0x98);
	sc285sl_write_register(ViPipe, 0x3204, 0x07);
	sc285sl_write_register(ViPipe, 0x3205, 0x8f);
	sc285sl_write_register(ViPipe, 0x3206, 0x04);
	sc285sl_write_register(ViPipe, 0x3207, 0x47);
	sc285sl_write_register(ViPipe, 0x3208, 0x07);
	sc285sl_write_register(ViPipe, 0x3209, 0x80);
	sc285sl_write_register(ViPipe, 0x320a, 0x04);
	sc285sl_write_register(ViPipe, 0x320b, 0x38);
	sc285sl_write_register(ViPipe, 0x320c, 0x05);
	sc285sl_write_register(ViPipe, 0x320d, 0x46);
	sc285sl_write_register(ViPipe, 0x320e, 0x0a);
	sc285sl_write_register(ViPipe, 0x320f, 0xbe);
	sc285sl_write_register(ViPipe, 0x3210, 0x00);
	sc285sl_write_register(ViPipe, 0x3211, 0x08);
	sc285sl_write_register(ViPipe, 0x3212, 0x00);
	sc285sl_write_register(ViPipe, 0x3213, 0x08);
	sc285sl_write_register(ViPipe, 0x3214, 0x11);
	sc285sl_write_register(ViPipe, 0x3215, 0x11);
	sc285sl_write_register(ViPipe, 0x3220, 0x00);
	sc285sl_write_register(ViPipe, 0x3223, 0xc0);
	sc285sl_write_register(ViPipe, 0x3250, 0xff);
	sc285sl_write_register(ViPipe, 0x325f, 0xa0);
	sc285sl_write_register(ViPipe, 0x3271, 0x10);
	sc285sl_write_register(ViPipe, 0x3273, 0x13);
	sc285sl_write_register(ViPipe, 0x3281, 0x01);
	sc285sl_write_register(ViPipe, 0x32d1, 0x90);
	sc285sl_write_register(ViPipe, 0x32e0, 0x00);
	sc285sl_write_register(ViPipe, 0x3301, 0x08);
	sc285sl_write_register(ViPipe, 0x3304, 0x80);
	sc285sl_write_register(ViPipe, 0x3305, 0x00);
	sc285sl_write_register(ViPipe, 0x3306, 0x60);
	sc285sl_write_register(ViPipe, 0x3309, 0xa0);
	sc285sl_write_register(ViPipe, 0x330a, 0x00);
	sc285sl_write_register(ViPipe, 0x330b, 0xc0);
	sc285sl_write_register(ViPipe, 0x331c, 0x01);
	sc285sl_write_register(ViPipe, 0x331e, 0x69);
	sc285sl_write_register(ViPipe, 0x331f, 0x89);
	sc285sl_write_register(ViPipe, 0x3320, 0x0c);
	sc285sl_write_register(ViPipe, 0x3326, 0x14);
	sc285sl_write_register(ViPipe, 0x3328, 0x04);
	sc285sl_write_register(ViPipe, 0x3329, 0x08);
	sc285sl_write_register(ViPipe, 0x3353, 0x14);
	sc285sl_write_register(ViPipe, 0x3364, 0x5e);
	sc285sl_write_register(ViPipe, 0x3366, 0x08);
	sc285sl_write_register(ViPipe, 0x338f, 0xa0);
	sc285sl_write_register(ViPipe, 0x3393, 0x0c);
	sc285sl_write_register(ViPipe, 0x3394, 0x12);
	sc285sl_write_register(ViPipe, 0x3395, 0x3c);
	sc285sl_write_register(ViPipe, 0x3399, 0x08);
	sc285sl_write_register(ViPipe, 0x339a, 0x10);
	sc285sl_write_register(ViPipe, 0x339b, 0x12);
	sc285sl_write_register(ViPipe, 0x339c, 0x50);
	sc285sl_write_register(ViPipe, 0x33ac, 0x18);
	sc285sl_write_register(ViPipe, 0x33ad, 0x2c);
	sc285sl_write_register(ViPipe, 0x33ae, 0x28);
	sc285sl_write_register(ViPipe, 0x33af, 0x60);
	sc285sl_write_register(ViPipe, 0x33b0, 0x0f);
	sc285sl_write_register(ViPipe, 0x33b2, 0x54);
	sc285sl_write_register(ViPipe, 0x33b3, 0x18);
	sc285sl_write_register(ViPipe, 0x33f8, 0x00);
	sc285sl_write_register(ViPipe, 0x33f9, 0x80);
	sc285sl_write_register(ViPipe, 0x33fa, 0x00);
	sc285sl_write_register(ViPipe, 0x33fb, 0xa0);
	sc285sl_write_register(ViPipe, 0x3428, 0x04);
	sc285sl_write_register(ViPipe, 0x3429, 0x04);
	sc285sl_write_register(ViPipe, 0x342a, 0x04);
	sc285sl_write_register(ViPipe, 0x3432, 0x72);
	sc285sl_write_register(ViPipe, 0x3447, 0x11);
	sc285sl_write_register(ViPipe, 0x349f, 0x03);
	sc285sl_write_register(ViPipe, 0x34a5, 0x01);
	sc285sl_write_register(ViPipe, 0x34a8, 0x10);
	sc285sl_write_register(ViPipe, 0x34a9, 0x0c);
	sc285sl_write_register(ViPipe, 0x34aa, 0x00);
	sc285sl_write_register(ViPipe, 0x34ab, 0xe0);
	sc285sl_write_register(ViPipe, 0x34ac, 0x01);
	sc285sl_write_register(ViPipe, 0x34ad, 0x00);
	sc285sl_write_register(ViPipe, 0x34ba, 0x04);
	sc285sl_write_register(ViPipe, 0x34bb, 0x0f);
	sc285sl_write_register(ViPipe, 0x34f9, 0x04);
	sc285sl_write_register(ViPipe, 0x3622, 0x04);
	sc285sl_write_register(ViPipe, 0x3623, 0x38);
	sc285sl_write_register(ViPipe, 0x3624, 0x04);
	sc285sl_write_register(ViPipe, 0x3625, 0x78);
	sc285sl_write_register(ViPipe, 0x3626, 0x04);
	sc285sl_write_register(ViPipe, 0x3627, 0x38);
	sc285sl_write_register(ViPipe, 0x362f, 0x10);
	sc285sl_write_register(ViPipe, 0x3632, 0x64);
	sc285sl_write_register(ViPipe, 0x3633, 0x44);
	sc285sl_write_register(ViPipe, 0x3634, 0x24);
	sc285sl_write_register(ViPipe, 0x3637, 0x5c);
	sc285sl_write_register(ViPipe, 0x363d, 0x24);
	sc285sl_write_register(ViPipe, 0x3670, 0x41);
	sc285sl_write_register(ViPipe, 0x3671, 0x42);
	sc285sl_write_register(ViPipe, 0x3672, 0x33);
	sc285sl_write_register(ViPipe, 0x3673, 0x04);
	sc285sl_write_register(ViPipe, 0x3674, 0x08);
	sc285sl_write_register(ViPipe, 0x3675, 0x04);
	sc285sl_write_register(ViPipe, 0x3676, 0x18);
	sc285sl_write_register(ViPipe, 0x367e, 0x44);
	sc285sl_write_register(ViPipe, 0x367f, 0x44);
	sc285sl_write_register(ViPipe, 0x3680, 0x34);
	sc285sl_write_register(ViPipe, 0x3681, 0x04);
	sc285sl_write_register(ViPipe, 0x3682, 0x08);
	sc285sl_write_register(ViPipe, 0x3683, 0x04);
	sc285sl_write_register(ViPipe, 0x3684, 0x18);
	sc285sl_write_register(ViPipe, 0x3685, 0x80);
	sc285sl_write_register(ViPipe, 0x3686, 0x80);
	sc285sl_write_register(ViPipe, 0x3687, 0x80);
	sc285sl_write_register(ViPipe, 0x3688, 0x80);
	sc285sl_write_register(ViPipe, 0x3689, 0x80);
	sc285sl_write_register(ViPipe, 0x368a, 0x80);
	sc285sl_write_register(ViPipe, 0x368b, 0x80);
	sc285sl_write_register(ViPipe, 0x368c, 0x80);
	sc285sl_write_register(ViPipe, 0x368d, 0x00);
	sc285sl_write_register(ViPipe, 0x368e, 0x08);
	sc285sl_write_register(ViPipe, 0x368f, 0x00);
	sc285sl_write_register(ViPipe, 0x3690, 0x18);
	sc285sl_write_register(ViPipe, 0x3691, 0x04);
	sc285sl_write_register(ViPipe, 0x3692, 0x00);
	sc285sl_write_register(ViPipe, 0x3693, 0x04);
	sc285sl_write_register(ViPipe, 0x3694, 0x08);
	sc285sl_write_register(ViPipe, 0x3695, 0x04);
	sc285sl_write_register(ViPipe, 0x3696, 0x18);
	sc285sl_write_register(ViPipe, 0x3697, 0x04);
	sc285sl_write_register(ViPipe, 0x3698, 0x38);
	sc285sl_write_register(ViPipe, 0x3699, 0x04);
	sc285sl_write_register(ViPipe, 0x369a, 0x78);
	sc285sl_write_register(ViPipe, 0x36b9, 0x33);
	sc285sl_write_register(ViPipe, 0x36ba, 0x33);
	sc285sl_write_register(ViPipe, 0x36bb, 0x23);
	sc285sl_write_register(ViPipe, 0x36bc, 0x04);
	sc285sl_write_register(ViPipe, 0x36bd, 0x08);
	sc285sl_write_register(ViPipe, 0x36be, 0x04);
	sc285sl_write_register(ViPipe, 0x36bf, 0x38);
	sc285sl_write_register(ViPipe, 0x36d0, 0x09);
	sc285sl_write_register(ViPipe, 0x36d1, 0x30);
	sc285sl_write_register(ViPipe, 0x36d2, 0x33);
	sc285sl_write_register(ViPipe, 0x36d3, 0x33);
	sc285sl_write_register(ViPipe, 0x36d4, 0x33);
	sc285sl_write_register(ViPipe, 0x36d5, 0x04);
	sc285sl_write_register(ViPipe, 0x36d6, 0x00);
	sc285sl_write_register(ViPipe, 0x36d7, 0x04);
	sc285sl_write_register(ViPipe, 0x36d8, 0x18);
	sc285sl_write_register(ViPipe, 0x36e0, 0x33);
	sc285sl_write_register(ViPipe, 0x36e1, 0x53);
	sc285sl_write_register(ViPipe, 0x36e2, 0x34);
	sc285sl_write_register(ViPipe, 0x36ea, 0x14);
	sc285sl_write_register(ViPipe, 0x36eb, 0x45);
	sc285sl_write_register(ViPipe, 0x36ec, 0x43);
	sc285sl_write_register(ViPipe, 0x36ed, 0x28);
	sc285sl_write_register(ViPipe, 0x370f, 0x71);
	sc285sl_write_register(ViPipe, 0x3722, 0x03);
	sc285sl_write_register(ViPipe, 0x3724, 0xb0);
	sc285sl_write_register(ViPipe, 0x3727, 0x20);
	sc285sl_write_register(ViPipe, 0x3729, 0x82);
	sc285sl_write_register(ViPipe, 0x37b0, 0x03);
	sc285sl_write_register(ViPipe, 0x37b1, 0x03);
	sc285sl_write_register(ViPipe, 0x37b2, 0x33);
	sc285sl_write_register(ViPipe, 0x37b3, 0x04);
	sc285sl_write_register(ViPipe, 0x37b4, 0x08);
	sc285sl_write_register(ViPipe, 0x37b5, 0x04);
	sc285sl_write_register(ViPipe, 0x37b6, 0x78);
	sc285sl_write_register(ViPipe, 0x37b7, 0x82);
	sc285sl_write_register(ViPipe, 0x37b8, 0x82);
	sc285sl_write_register(ViPipe, 0x37b9, 0x82);
	sc285sl_write_register(ViPipe, 0x37ba, 0x80);
	sc285sl_write_register(ViPipe, 0x37bb, 0x80);
	sc285sl_write_register(ViPipe, 0x37bc, 0x80);
	sc285sl_write_register(ViPipe, 0x37bd, 0x04);
	sc285sl_write_register(ViPipe, 0x37be, 0x08);
	sc285sl_write_register(ViPipe, 0x37bf, 0x04);
	sc285sl_write_register(ViPipe, 0x37c0, 0x18);
	sc285sl_write_register(ViPipe, 0x37c1, 0x00);
	sc285sl_write_register(ViPipe, 0x37c2, 0x08);
	sc285sl_write_register(ViPipe, 0x37c3, 0x00);
	sc285sl_write_register(ViPipe, 0x37c4, 0x18);
	sc285sl_write_register(ViPipe, 0x37c5, 0x00);
	sc285sl_write_register(ViPipe, 0x37c6, 0x08);
	sc285sl_write_register(ViPipe, 0x37c7, 0x00);
	sc285sl_write_register(ViPipe, 0x37c8, 0x18);
	sc285sl_write_register(ViPipe, 0x37c9, 0x00);
	sc285sl_write_register(ViPipe, 0x37ca, 0x00);
	sc285sl_write_register(ViPipe, 0x37cb, 0x00);
	sc285sl_write_register(ViPipe, 0x37cc, 0x80);
	sc285sl_write_register(ViPipe, 0x37cd, 0x80);
	sc285sl_write_register(ViPipe, 0x37ce, 0x80);
	sc285sl_write_register(ViPipe, 0x37cf, 0x80);
	sc285sl_write_register(ViPipe, 0x37d6, 0x80);
	sc285sl_write_register(ViPipe, 0x37d7, 0x04);
	sc285sl_write_register(ViPipe, 0x37d8, 0x00);
	sc285sl_write_register(ViPipe, 0x37d9, 0x04);
	sc285sl_write_register(ViPipe, 0x37da, 0x08);
	sc285sl_write_register(ViPipe, 0x37db, 0x04);
	sc285sl_write_register(ViPipe, 0x37dc, 0x18);
	sc285sl_write_register(ViPipe, 0x37dd, 0x04);
	sc285sl_write_register(ViPipe, 0x37de, 0x38);
	sc285sl_write_register(ViPipe, 0x37df, 0x04);
	sc285sl_write_register(ViPipe, 0x37e0, 0x78);
	sc285sl_write_register(ViPipe, 0x37e1, 0x00);
	sc285sl_write_register(ViPipe, 0x37e2, 0x00);
	sc285sl_write_register(ViPipe, 0x37e3, 0x00);
	sc285sl_write_register(ViPipe, 0x37e4, 0x00);
	sc285sl_write_register(ViPipe, 0x37e5, 0x00);
	sc285sl_write_register(ViPipe, 0x37e6, 0x04);
	sc285sl_write_register(ViPipe, 0x37e7, 0x00);
	sc285sl_write_register(ViPipe, 0x37e8, 0x04);
	sc285sl_write_register(ViPipe, 0x37e9, 0x08);
	sc285sl_write_register(ViPipe, 0x37ea, 0x04);
	sc285sl_write_register(ViPipe, 0x37eb, 0x18);
	sc285sl_write_register(ViPipe, 0x37ec, 0x04);
	sc285sl_write_register(ViPipe, 0x37ed, 0x38);
	sc285sl_write_register(ViPipe, 0x37ee, 0x04);
	sc285sl_write_register(ViPipe, 0x37ef, 0x78);
	sc285sl_write_register(ViPipe, 0x37fa, 0x21);
	sc285sl_write_register(ViPipe, 0x37fb, 0x65);
	sc285sl_write_register(ViPipe, 0x37fc, 0x18);
	sc285sl_write_register(ViPipe, 0x37fd, 0x35);
	sc285sl_write_register(ViPipe, 0x3900, 0x05);
	sc285sl_write_register(ViPipe, 0x3903, 0x40);
	sc285sl_write_register(ViPipe, 0x3907, 0x01);
	sc285sl_write_register(ViPipe, 0x3908, 0x00);
	sc285sl_write_register(ViPipe, 0x391a, 0x80);
	sc285sl_write_register(ViPipe, 0x391b, 0x60);
	sc285sl_write_register(ViPipe, 0x391c, 0x33);
	sc285sl_write_register(ViPipe, 0x391d, 0x00);
	sc285sl_write_register(ViPipe, 0x391f, 0x61);
	sc285sl_write_register(ViPipe, 0x3926, 0xe2);
	sc285sl_write_register(ViPipe, 0x3933, 0x80);
	sc285sl_write_register(ViPipe, 0x3934, 0x02);
	sc285sl_write_register(ViPipe, 0x3935, 0x01);
	sc285sl_write_register(ViPipe, 0x3936, 0x10);
	sc285sl_write_register(ViPipe, 0x3937, 0x75);
	sc285sl_write_register(ViPipe, 0x3938, 0x73);
	sc285sl_write_register(ViPipe, 0x3939, 0x00);
	sc285sl_write_register(ViPipe, 0x393a, 0x00);
	sc285sl_write_register(ViPipe, 0x393b, 0x00);
	sc285sl_write_register(ViPipe, 0x393c, 0x20);
	sc285sl_write_register(ViPipe, 0x39c9, 0x30);
	sc285sl_write_register(ViPipe, 0x39dd, 0x00);
	sc285sl_write_register(ViPipe, 0x39de, 0x0c);
	sc285sl_write_register(ViPipe, 0x39e7, 0x08);
	sc285sl_write_register(ViPipe, 0x39e8, 0x08);
	sc285sl_write_register(ViPipe, 0x39e9, 0x80);
	sc285sl_write_register(ViPipe, 0x3c01, 0xb4);
	sc285sl_write_register(ViPipe, 0x3e00, 0x00);
	sc285sl_write_register(ViPipe, 0x3e01, 0x9f);
	sc285sl_write_register(ViPipe, 0x3e02, 0x00);
	sc285sl_write_register(ViPipe, 0x3e03, 0x0b);
	sc285sl_write_register(ViPipe, 0x3e04, 0x09);
	sc285sl_write_register(ViPipe, 0x3e05, 0xf0);
	sc285sl_write_register(ViPipe, 0x3e06, 0x00);
	sc285sl_write_register(ViPipe, 0x3e07, 0x80);
	sc285sl_write_register(ViPipe, 0x3e08, 0x00);
	sc285sl_write_register(ViPipe, 0x3e09, 0x20);
	sc285sl_write_register(ViPipe, 0x3e16, 0x02);
	sc285sl_write_register(ViPipe, 0x3e17, 0xb3);
	sc285sl_write_register(ViPipe, 0x3e18, 0x01);
	sc285sl_write_register(ViPipe, 0x3e19, 0x59);
	sc285sl_write_register(ViPipe, 0x3e23, 0x00);
	sc285sl_write_register(ViPipe, 0x3e24, 0xaa);
	sc285sl_write_register(ViPipe, 0x4402, 0x03);
	sc285sl_write_register(ViPipe, 0x4403, 0x0c);
	sc285sl_write_register(ViPipe, 0x4404, 0x22);
	sc285sl_write_register(ViPipe, 0x4405, 0x2c);
	sc285sl_write_register(ViPipe, 0x440c, 0x38);
	sc285sl_write_register(ViPipe, 0x440d, 0x38);
	sc285sl_write_register(ViPipe, 0x440e, 0x2a);
	sc285sl_write_register(ViPipe, 0x440f, 0x47);
	sc285sl_write_register(ViPipe, 0x4412, 0x01);
	sc285sl_write_register(ViPipe, 0x4424, 0x01);
	sc285sl_write_register(ViPipe, 0x4509, 0x1e);
	sc285sl_write_register(ViPipe, 0x450d, 0x0c);
	sc285sl_write_register(ViPipe, 0x4800, 0x24);
	sc285sl_write_register(ViPipe, 0x480f, 0x03);
	sc285sl_write_register(ViPipe, 0x4837, 0x11);
	sc285sl_write_register(ViPipe, 0x5000, 0x2e);
	sc285sl_write_register(ViPipe, 0x5011, 0x01);
	sc285sl_write_register(ViPipe, 0x530d, 0x08);
	sc285sl_write_register(ViPipe, 0x5310, 0x01);
	sc285sl_write_register(ViPipe, 0x5311, 0x10);
	sc285sl_write_register(ViPipe, 0x5312, 0x04);
	sc285sl_write_register(ViPipe, 0x5314, 0x07);
	sc285sl_write_register(ViPipe, 0x5315, 0x10);
	sc285sl_write_register(ViPipe, 0x5316, 0x0b);
	sc285sl_write_register(ViPipe, 0x5318, 0x0e);
	sc285sl_write_register(ViPipe, 0x531a, 0x11);
	sc285sl_write_register(ViPipe, 0x531c, 0x14);
	sc285sl_write_register(ViPipe, 0x531d, 0x10);
	sc285sl_write_register(ViPipe, 0x531e, 0x1a);
	sc285sl_write_register(ViPipe, 0x531f, 0x10);
	sc285sl_write_register(ViPipe, 0x5400, 0x01);
	sc285sl_write_register(ViPipe, 0x5404, 0x0c);
	sc285sl_write_register(ViPipe, 0x5405, 0x10);
	sc285sl_write_register(ViPipe, 0x5784, 0x10);
	sc285sl_write_register(ViPipe, 0x5785, 0x08);
	sc285sl_write_register(ViPipe, 0x5787, 0x0a);
	sc285sl_write_register(ViPipe, 0x5788, 0x0a);
	sc285sl_write_register(ViPipe, 0x5789, 0x08);
	sc285sl_write_register(ViPipe, 0x578a, 0x0a);
	sc285sl_write_register(ViPipe, 0x578b, 0x0a);
	sc285sl_write_register(ViPipe, 0x578c, 0x08);
	sc285sl_write_register(ViPipe, 0x5790, 0x08);
	sc285sl_write_register(ViPipe, 0x5791, 0x04);
	sc285sl_write_register(ViPipe, 0x5792, 0x04);
	sc285sl_write_register(ViPipe, 0x5793, 0x08);
	sc285sl_write_register(ViPipe, 0x5794, 0x04);
	sc285sl_write_register(ViPipe, 0x5795, 0x04);
	sc285sl_write_register(ViPipe, 0x57ac, 0x00);
	sc285sl_write_register(ViPipe, 0x57ad, 0x00);
	sc285sl_write_register(ViPipe, 0x36e9, 0x40);
	sc285sl_write_register(ViPipe, 0x37f9, 0x20);
	sc285sl_write_register(ViPipe, 0x4402, 0x03);
	sc285sl_write_register(ViPipe, 0x4403, 0x0c);
	sc285sl_write_register(ViPipe, 0x4404, 0x22);
	sc285sl_write_register(ViPipe, 0x4405, 0x2c);
	sc285sl_write_register(ViPipe, 0x440c, 0x38);
	sc285sl_write_register(ViPipe, 0x440d, 0x38);
	sc285sl_write_register(ViPipe, 0x440e, 0x2a);
	sc285sl_write_register(ViPipe, 0x440f, 0x47);
	sc285sl_write_register(ViPipe, 0x4412, 0x01);
	sc285sl_write_register(ViPipe, 0x4406, 0x00);
	sc285sl_write_register(ViPipe, 0x4407, 0x7e);
	sc285sl_write_register(ViPipe, 0x4424, 0x01);
	sc285sl_write_register(ViPipe, 0x0100, 0x01);

	delay_ms(10);

	sc285sl_default_reg_init(ViPipe);
	sc285sl_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC285SL 1080P 30fps 10bit WDR(2to1) Init OK!===\n", ViPipe);
}