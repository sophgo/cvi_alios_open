#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc438ai_cmos_ex.h"
#include "sensor_i2c.h"

#define SC438AI_PATTERN_ENABLE	0

static void sc438ai_linear_1440p30_init(VI_PIPE ViPipe);
static void sc438ai_linear_1440p30_master_init(VI_PIPE ViPipe);
static void sc438ai_linear_1440p30_slave_init(VI_PIPE ViPipe);

CVI_U8 sc438ai_i2c_addr = 0x30;        /* I2C Address of SC438AI */
const CVI_U32 sc438ai_addr_byte = 2;
const CVI_U32 sc438ai_data_byte = 1;

int sc438ai_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC438AI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC438AI_AddrInfo[ViPipe].s8I2cAddr);
}

int sc438ai_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC438AI_BusInfo[ViPipe].s8I2cDev);
}

int sc438ai_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC438AI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC438AI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc438ai_addr_byte, sc438ai_data_byte);
}

int sc438ai_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC438AI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC438AI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc438ai_addr_byte, (CVI_U32)data, sc438ai_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc438ai_standby(VI_PIPE ViPipe)
{
	sc438ai_write_register(ViPipe, 0x0100, 0x00);
}

void sc438ai_restart(VI_PIPE ViPipe)
{
	sc438ai_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc438ai_write_register(ViPipe, 0x0100, 0x01);
}

void sc438ai_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC438AI[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc438ai_write_register(ViPipe,
				g_pastSC438AI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC438AI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define SC438AI_CHIP_ID_HI_ADDR		0x3107
#define SC438AI_CHIP_ID_LO_ADDR		0x3108
#define SC438AI_CHIP_ID			0xce78

void sc438ai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc438ai_write_register(ViPipe, 0x3221, val);
}

int sc438ai_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc438ai_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc438ai_read_register(ViPipe, SC438AI_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc438ai_read_register(ViPipe, SC438AI_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC438AI_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc438ai_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode = g_pastSC438AI[ViPipe]->u8ImgMode;

	sc438ai_i2c_init(ViPipe);

	if (u8ImgMode == SC438AI_MODE_1440P30_2L)
		sc438ai_linear_1440p30_init(ViPipe);
	else if (u8ImgMode == SC438AI_MODE_1440P30_2L_MASTER)
		sc438ai_linear_1440p30_master_init(ViPipe);
	else if (u8ImgMode == SC438AI_MODE_1440P30_2L_SLAVE)
		sc438ai_linear_1440p30_slave_init(ViPipe);

	g_pastSC438AI[ViPipe]->bInit = CVI_TRUE;
}

/* 1440P30 */
static void sc438ai_linear_1440p30_init(VI_PIPE ViPipe)
{
	sc438ai_write_register(ViPipe, 0x0103, 0x01);
	sc438ai_write_register(ViPipe, 0x0100, 0x00);
	sc438ai_write_register(ViPipe, 0x36e9, 0x80);
	sc438ai_write_register(ViPipe, 0x37f9, 0x80);
	sc438ai_write_register(ViPipe, 0x23b0, 0x00);
	sc438ai_write_register(ViPipe, 0x23b1, 0x08);
	sc438ai_write_register(ViPipe, 0x23b2, 0x00);
	sc438ai_write_register(ViPipe, 0x23b3, 0x18);
	sc438ai_write_register(ViPipe, 0x23b4, 0x00);
	sc438ai_write_register(ViPipe, 0x23b5, 0x38);
	sc438ai_write_register(ViPipe, 0x23b6, 0x04);
	sc438ai_write_register(ViPipe, 0x23b7, 0x08);
	sc438ai_write_register(ViPipe, 0x23b8, 0x04);
	sc438ai_write_register(ViPipe, 0x23b9, 0x18);
	sc438ai_write_register(ViPipe, 0x23ba, 0x04);
	sc438ai_write_register(ViPipe, 0x23bb, 0x38);
	sc438ai_write_register(ViPipe, 0x23c0, 0x04);
	sc438ai_write_register(ViPipe, 0x23c1, 0x00);
	sc438ai_write_register(ViPipe, 0x23c2, 0x04);
	sc438ai_write_register(ViPipe, 0x23c3, 0x18);
	sc438ai_write_register(ViPipe, 0x23c4, 0x04);
	sc438ai_write_register(ViPipe, 0x23c5, 0x78);
	sc438ai_write_register(ViPipe, 0x23c6, 0x04);
	sc438ai_write_register(ViPipe, 0x23c7, 0x08);
	sc438ai_write_register(ViPipe, 0x23c8, 0x04);
	sc438ai_write_register(ViPipe, 0x23c9, 0x78);
	sc438ai_write_register(ViPipe, 0x3018, 0x3b);
	sc438ai_write_register(ViPipe, 0x3019, 0x0c);
	sc438ai_write_register(ViPipe, 0x301e, 0xf0);
	sc438ai_write_register(ViPipe, 0x301f, 0x08);
	sc438ai_write_register(ViPipe, 0x302c, 0x00);
	sc438ai_write_register(ViPipe, 0x3032, 0x20);
	sc438ai_write_register(ViPipe, 0x30b8, 0x44);

	// 2688X1520
	// sc438ai_write_register(ViPipe, 0x3200, 0x00);
	// sc438ai_write_register(ViPipe, 0x3201, 0x00);
	// sc438ai_write_register(ViPipe, 0x3202, 0x00);
	// sc438ai_write_register(ViPipe, 0x3203, 0xd4);
	// sc438ai_write_register(ViPipe, 0x3204, 0x0a);
	// sc438ai_write_register(ViPipe, 0x3205, 0x87);
	// sc438ai_write_register(ViPipe, 0x3206, 0x06);
	// sc438ai_write_register(ViPipe, 0x3207, 0xcb);
	// sc438ai_write_register(ViPipe, 0x3208, 0x0a);
	// sc438ai_write_register(ViPipe, 0x3209, 0x80);
	// sc438ai_write_register(ViPipe, 0x320a, 0x05);
	// sc438ai_write_register(ViPipe, 0x320b, 0xf0);
	// sc438ai_write_register(ViPipe, 0x3210, 0x00);
	// sc438ai_write_register(ViPipe, 0x3211, 0x04);
	// sc438ai_write_register(ViPipe, 0x3212, 0x00);
	// sc438ai_write_register(ViPipe, 0x3213, 0x04);

	// 2560X1440
	sc438ai_write_register(ViPipe, 0x3200, 0x00);
	sc438ai_write_register(ViPipe, 0x3201, 0x40);
	sc438ai_write_register(ViPipe, 0x3202, 0x00);
	sc438ai_write_register(ViPipe, 0x3203, 0xfc);
	sc438ai_write_register(ViPipe, 0x3204, 0x0a);
	sc438ai_write_register(ViPipe, 0x3205, 0x47);
	sc438ai_write_register(ViPipe, 0x3206, 0x06);
	sc438ai_write_register(ViPipe, 0x3207, 0xa3);
	sc438ai_write_register(ViPipe, 0x3208, 0x0a);
	sc438ai_write_register(ViPipe, 0x3209, 0x00);
	sc438ai_write_register(ViPipe, 0x320a, 0x05);
	sc438ai_write_register(ViPipe, 0x320b, 0xa0);
	sc438ai_write_register(ViPipe, 0x3210, 0x00);
	sc438ai_write_register(ViPipe, 0x3211, 0x04);
	sc438ai_write_register(ViPipe, 0x3212, 0x00);
	sc438ai_write_register(ViPipe, 0x3213, 0x04);

	// HTS
	sc438ai_write_register(ViPipe, 0x320c, 0x05);
	sc438ai_write_register(ViPipe, 0x320d, 0xdc);
	// VTS normal mode
	sc438ai_write_register(ViPipe, 0x320e, 0x07);
	sc438ai_write_register(ViPipe, 0x320f, 0xd0);

	// slave mode
	// sc438ai_write_register(ViPipe, 0x3222, 0x01); // slave efsync
	// sc438ai_write_register(ViPipe, 0x3230, 0x00);
	// sc438ai_write_register(ViPipe, 0x3231, 0x04);
	// sc438ai_write_register(ViPipe, 0x300a, 0x40);
	// sc438ai_write_register(ViPipe, 0x320e, 0x07);
	// sc438ai_write_register(ViPipe, 0x320f, 0xce); // vts-2

	// master mode
	// sc438ai_write_register(ViPipe, 0x3222, 0x00); // output fsync
	// sc438ai_write_register(ViPipe, 0x3224, 0xc2);
	// sc438ai_write_register(ViPipe, 0x3230, 0x00);
	// sc438ai_write_register(ViPipe, 0x3231, 0x04);
	// sc438ai_write_register(ViPipe, 0x3232, 0xa0);
	// sc438ai_write_register(ViPipe, 0x300a, 0x24);

	// non-continue mode
	// slave mode
	// sc438ai_write_register(ViPipe, 0x3250, 0x40);
	// master mode & normal mode
	sc438ai_write_register(ViPipe, 0x3250, 0x00);
	sc438ai_write_register(ViPipe, 0x4800, 0x24);
	sc438ai_write_register(ViPipe, 0x4816, 0x21);
	sc438ai_write_register(ViPipe, 0x4837, 0x11);

	// continue mode
	// sc438ai_write_register(ViPipe, 0x3250, 0x40);
	// sc438ai_write_register(ViPipe, 0x4800, 0x04);

	sc438ai_write_register(ViPipe, 0x3214, 0x11);
	sc438ai_write_register(ViPipe, 0x3215, 0x11);
	sc438ai_write_register(ViPipe, 0x3223, 0xc0);
	sc438ai_write_register(ViPipe, 0x327f, 0x3f);
	sc438ai_write_register(ViPipe, 0x32e0, 0x00);
	sc438ai_write_register(ViPipe, 0x3301, 0x12);
	sc438ai_write_register(ViPipe, 0x3302, 0x20);
	sc438ai_write_register(ViPipe, 0x3304, 0xc0);
	sc438ai_write_register(ViPipe, 0x3306, 0xb0);
	sc438ai_write_register(ViPipe, 0x3309, 0xf0);
	sc438ai_write_register(ViPipe, 0x330a, 0x01);
	sc438ai_write_register(ViPipe, 0x330b, 0x70);
	sc438ai_write_register(ViPipe, 0x330d, 0x10);
	sc438ai_write_register(ViPipe, 0x3310, 0x18);
	sc438ai_write_register(ViPipe, 0x331e, 0xb1);
	sc438ai_write_register(ViPipe, 0x331f, 0xe1);
	sc438ai_write_register(ViPipe, 0x3333, 0x10);
	sc438ai_write_register(ViPipe, 0x3334, 0x40);
	sc438ai_write_register(ViPipe, 0x3364, 0x56);
	sc438ai_write_register(ViPipe, 0x338f, 0x80);
	sc438ai_write_register(ViPipe, 0x3393, 0x1c);
	sc438ai_write_register(ViPipe, 0x3394, 0x2c);
	sc438ai_write_register(ViPipe, 0x3395, 0x3c);
	sc438ai_write_register(ViPipe, 0x3399, 0x0c);
	sc438ai_write_register(ViPipe, 0x339a, 0x10);
	sc438ai_write_register(ViPipe, 0x339b, 0x18);
	sc438ai_write_register(ViPipe, 0x339c, 0x80);
	sc438ai_write_register(ViPipe, 0x33ac, 0x10);
	sc438ai_write_register(ViPipe, 0x33ad, 0x2c);
	sc438ai_write_register(ViPipe, 0x33ae, 0xb0);
	sc438ai_write_register(ViPipe, 0x33af, 0xe0);
	sc438ai_write_register(ViPipe, 0x33b0, 0x0f);
	sc438ai_write_register(ViPipe, 0x33b2, 0x2c);
	sc438ai_write_register(ViPipe, 0x33b3, 0x02);
	sc438ai_write_register(ViPipe, 0x349f, 0x03);
	sc438ai_write_register(ViPipe, 0x34a8, 0x02);
	sc438ai_write_register(ViPipe, 0x34a9, 0x08);
	sc438ai_write_register(ViPipe, 0x34aa, 0x01);
	sc438ai_write_register(ViPipe, 0x34ab, 0x70);
	sc438ai_write_register(ViPipe, 0x34ac, 0x01);
	sc438ai_write_register(ViPipe, 0x34ad, 0x70);
	sc438ai_write_register(ViPipe, 0x34f9, 0x12);
	sc438ai_write_register(ViPipe, 0x3631, 0x0f);
	sc438ai_write_register(ViPipe, 0x3632, 0x8d);
	sc438ai_write_register(ViPipe, 0x3633, 0x4d);
	sc438ai_write_register(ViPipe, 0x363b, 0x58);
	sc438ai_write_register(ViPipe, 0x363c, 0xd8);
	sc438ai_write_register(ViPipe, 0x363d, 0x20);
	sc438ai_write_register(ViPipe, 0x3641, 0x08);
	sc438ai_write_register(ViPipe, 0x3670, 0x32);
	sc438ai_write_register(ViPipe, 0x3671, 0x34);
	sc438ai_write_register(ViPipe, 0x3672, 0x26);
	sc438ai_write_register(ViPipe, 0x3673, 0x04);
	sc438ai_write_register(ViPipe, 0x3674, 0x08);
	sc438ai_write_register(ViPipe, 0x3675, 0x04);
	sc438ai_write_register(ViPipe, 0x3676, 0x18);
	sc438ai_write_register(ViPipe, 0x367e, 0x49);
	sc438ai_write_register(ViPipe, 0x367f, 0x49);
	sc438ai_write_register(ViPipe, 0x3680, 0x49);
	sc438ai_write_register(ViPipe, 0x3681, 0x04);
	sc438ai_write_register(ViPipe, 0x3682, 0x08);
	sc438ai_write_register(ViPipe, 0x3683, 0x04);
	sc438ai_write_register(ViPipe, 0x3684, 0x38);
	sc438ai_write_register(ViPipe, 0x3685, 0xc1);
	sc438ai_write_register(ViPipe, 0x3686, 0xc2);
	sc438ai_write_register(ViPipe, 0x3687, 0xc1);
	sc438ai_write_register(ViPipe, 0x3688, 0xc1);
	sc438ai_write_register(ViPipe, 0x3689, 0xc1);
	sc438ai_write_register(ViPipe, 0x368a, 0xc1);
	sc438ai_write_register(ViPipe, 0x368b, 0xc4);
	sc438ai_write_register(ViPipe, 0x368c, 0xc1);
	sc438ai_write_register(ViPipe, 0x368d, 0x00);
	sc438ai_write_register(ViPipe, 0x368e, 0x08);
	sc438ai_write_register(ViPipe, 0x368f, 0x00);
	sc438ai_write_register(ViPipe, 0x3690, 0x18);
	sc438ai_write_register(ViPipe, 0x3691, 0x04);
	sc438ai_write_register(ViPipe, 0x3692, 0x00);
	sc438ai_write_register(ViPipe, 0x3693, 0x04);
	sc438ai_write_register(ViPipe, 0x3694, 0x08);
	sc438ai_write_register(ViPipe, 0x3695, 0x04);
	sc438ai_write_register(ViPipe, 0x3696, 0x18);
	sc438ai_write_register(ViPipe, 0x3697, 0x04);
	sc438ai_write_register(ViPipe, 0x3698, 0x38);
	sc438ai_write_register(ViPipe, 0x3699, 0x04);
	sc438ai_write_register(ViPipe, 0x369a, 0x78);
	sc438ai_write_register(ViPipe, 0x36d0, 0x0d);
	sc438ai_write_register(ViPipe, 0x36ea, 0x0a);
	sc438ai_write_register(ViPipe, 0x36eb, 0x0c);
	sc438ai_write_register(ViPipe, 0x36ec, 0x43);
	sc438ai_write_register(ViPipe, 0x36ed, 0xaa);
	sc438ai_write_register(ViPipe, 0x370f, 0x13);
	sc438ai_write_register(ViPipe, 0x3721, 0x6c);
	sc438ai_write_register(ViPipe, 0x3722, 0x8b);
	sc438ai_write_register(ViPipe, 0x3724, 0xd1);
	sc438ai_write_register(ViPipe, 0x3729, 0x34);
	sc438ai_write_register(ViPipe, 0x37b0, 0x17);
	sc438ai_write_register(ViPipe, 0x37b1, 0x17);
	sc438ai_write_register(ViPipe, 0x37b2, 0x13);
	sc438ai_write_register(ViPipe, 0x37b3, 0x04);
	sc438ai_write_register(ViPipe, 0x37b4, 0x08);
	sc438ai_write_register(ViPipe, 0x37b5, 0x04);
	sc438ai_write_register(ViPipe, 0x37b6, 0x38);
	sc438ai_write_register(ViPipe, 0x37b7, 0x1d);
	sc438ai_write_register(ViPipe, 0x37b8, 0x1f);
	sc438ai_write_register(ViPipe, 0x37b9, 0x1f);
	sc438ai_write_register(ViPipe, 0x37ba, 0x04);
	sc438ai_write_register(ViPipe, 0x37bb, 0x04);
	sc438ai_write_register(ViPipe, 0x37bc, 0x04);
	sc438ai_write_register(ViPipe, 0x37bd, 0x04);
	sc438ai_write_register(ViPipe, 0x37be, 0x08);
	sc438ai_write_register(ViPipe, 0x37bf, 0x04);
	sc438ai_write_register(ViPipe, 0x37c0, 0x38);
	sc438ai_write_register(ViPipe, 0x37c1, 0x04);
	sc438ai_write_register(ViPipe, 0x37c2, 0x08);
	sc438ai_write_register(ViPipe, 0x37c3, 0x04);
	sc438ai_write_register(ViPipe, 0x37c4, 0x38);
	sc438ai_write_register(ViPipe, 0x37fa, 0x09);
	sc438ai_write_register(ViPipe, 0x37fb, 0x22);
	sc438ai_write_register(ViPipe, 0x37fc, 0x30);
	sc438ai_write_register(ViPipe, 0x37fd, 0x26);
	sc438ai_write_register(ViPipe, 0x3900, 0x05);
	sc438ai_write_register(ViPipe, 0x3901, 0x00);
	sc438ai_write_register(ViPipe, 0x3902, 0xc0);
	sc438ai_write_register(ViPipe, 0x3903, 0x40);
	sc438ai_write_register(ViPipe, 0x3905, 0x2d);
	sc438ai_write_register(ViPipe, 0x391a, 0x72);
	sc438ai_write_register(ViPipe, 0x391b, 0x39);
	sc438ai_write_register(ViPipe, 0x391c, 0x22);
	sc438ai_write_register(ViPipe, 0x391d, 0x00);
	sc438ai_write_register(ViPipe, 0x391f, 0x41);
	sc438ai_write_register(ViPipe, 0x3926, 0xe0);
	sc438ai_write_register(ViPipe, 0x3933, 0x80);
	sc438ai_write_register(ViPipe, 0x3934, 0x03);
	sc438ai_write_register(ViPipe, 0x3935, 0x01);
	sc438ai_write_register(ViPipe, 0x3936, 0xc0);
	sc438ai_write_register(ViPipe, 0x3937, 0x6a);
	sc438ai_write_register(ViPipe, 0x3938, 0x6b);
	sc438ai_write_register(ViPipe, 0x3939, 0x0f);
	sc438ai_write_register(ViPipe, 0x393a, 0xf6);
	sc438ai_write_register(ViPipe, 0x393d, 0x05);
	sc438ai_write_register(ViPipe, 0x393e, 0x30);
	sc438ai_write_register(ViPipe, 0x39dd, 0x00);
	sc438ai_write_register(ViPipe, 0x39de, 0x06);
	sc438ai_write_register(ViPipe, 0x39e7, 0x04);
	sc438ai_write_register(ViPipe, 0x39e8, 0x04);
	sc438ai_write_register(ViPipe, 0x39e9, 0x80);
	sc438ai_write_register(ViPipe, 0x3e00, 0x00);
	sc438ai_write_register(ViPipe, 0x3e01, 0x7c);
	sc438ai_write_register(ViPipe, 0x3e02, 0x80);
	sc438ai_write_register(ViPipe, 0x3e03, 0x0b);
	sc438ai_write_register(ViPipe, 0x3e16, 0x01);
	sc438ai_write_register(ViPipe, 0x3e17, 0x44);
	sc438ai_write_register(ViPipe, 0x3e18, 0x01);
	sc438ai_write_register(ViPipe, 0x3e19, 0x44);
	sc438ai_write_register(ViPipe, 0x440e, 0x02);
	sc438ai_write_register(ViPipe, 0x4509, 0x18);
	sc438ai_write_register(ViPipe, 0x450d, 0x07);
	sc438ai_write_register(ViPipe, 0x480f, 0x03);
	sc438ai_write_register(ViPipe, 0x5000, 0x06);
	sc438ai_write_register(ViPipe, 0x5780, 0x76);
	sc438ai_write_register(ViPipe, 0x5784, 0x10);
	sc438ai_write_register(ViPipe, 0x5785, 0x08);
	sc438ai_write_register(ViPipe, 0x5787, 0x16);
	sc438ai_write_register(ViPipe, 0x5788, 0x16);
	sc438ai_write_register(ViPipe, 0x5789, 0x15);
	sc438ai_write_register(ViPipe, 0x578a, 0x16);
	sc438ai_write_register(ViPipe, 0x578b, 0x16);
	sc438ai_write_register(ViPipe, 0x578c, 0x15);
	sc438ai_write_register(ViPipe, 0x578d, 0x41);
	sc438ai_write_register(ViPipe, 0x5790, 0x11);
	sc438ai_write_register(ViPipe, 0x5791, 0x0f);
	sc438ai_write_register(ViPipe, 0x5792, 0x0f);
	sc438ai_write_register(ViPipe, 0x5793, 0x11);
	sc438ai_write_register(ViPipe, 0x5794, 0x0f);
	sc438ai_write_register(ViPipe, 0x5795, 0x0f);
	sc438ai_write_register(ViPipe, 0x5799, 0x46);
	sc438ai_write_register(ViPipe, 0x579a, 0x77);
	sc438ai_write_register(ViPipe, 0x57a1, 0x04);
	sc438ai_write_register(ViPipe, 0x57a8, 0xd2);
	sc438ai_write_register(ViPipe, 0x57aa, 0x2a);
	sc438ai_write_register(ViPipe, 0x57ab, 0x7f);
	sc438ai_write_register(ViPipe, 0x57ac, 0x00);
	sc438ai_write_register(ViPipe, 0x57ad, 0x00);
	sc438ai_write_register(ViPipe, 0x36e9, 0x53);
	sc438ai_write_register(ViPipe, 0x37f9, 0x00);
#if SC438AI_PATTERN_ENABLE
	sc438ai_write_register(ViPipe, 0x4501, 0xcc);
#endif
	sc438ai_default_reg_init(ViPipe);
	sc438ai_write_register(ViPipe, 0x0100, 0x01);


	printf("ViPipe:%d,===SC438AI 1440P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

static void sc438ai_linear_1440p30_master_init(VI_PIPE ViPipe)
{
	sc438ai_write_register(ViPipe, 0x0103, 0x01);
	sc438ai_write_register(ViPipe, 0x0100, 0x00);
	sc438ai_write_register(ViPipe, 0x36e9, 0x80);
	sc438ai_write_register(ViPipe, 0x37f9, 0x80);
	sc438ai_write_register(ViPipe, 0x23b0, 0x00);
	sc438ai_write_register(ViPipe, 0x23b1, 0x08);
	sc438ai_write_register(ViPipe, 0x23b2, 0x00);
	sc438ai_write_register(ViPipe, 0x23b3, 0x18);
	sc438ai_write_register(ViPipe, 0x23b4, 0x00);
	sc438ai_write_register(ViPipe, 0x23b5, 0x38);
	sc438ai_write_register(ViPipe, 0x23b6, 0x04);
	sc438ai_write_register(ViPipe, 0x23b7, 0x08);
	sc438ai_write_register(ViPipe, 0x23b8, 0x04);
	sc438ai_write_register(ViPipe, 0x23b9, 0x18);
	sc438ai_write_register(ViPipe, 0x23ba, 0x04);
	sc438ai_write_register(ViPipe, 0x23bb, 0x38);
	sc438ai_write_register(ViPipe, 0x23c0, 0x04);
	sc438ai_write_register(ViPipe, 0x23c1, 0x00);
	sc438ai_write_register(ViPipe, 0x23c2, 0x04);
	sc438ai_write_register(ViPipe, 0x23c3, 0x18);
	sc438ai_write_register(ViPipe, 0x23c4, 0x04);
	sc438ai_write_register(ViPipe, 0x23c5, 0x78);
	sc438ai_write_register(ViPipe, 0x23c6, 0x04);
	sc438ai_write_register(ViPipe, 0x23c7, 0x08);
	sc438ai_write_register(ViPipe, 0x23c8, 0x04);
	sc438ai_write_register(ViPipe, 0x23c9, 0x78);
	sc438ai_write_register(ViPipe, 0x3018, 0x3b);
	sc438ai_write_register(ViPipe, 0x3019, 0x0c);
	sc438ai_write_register(ViPipe, 0x301e, 0xf0);
	sc438ai_write_register(ViPipe, 0x301f, 0x08);
	sc438ai_write_register(ViPipe, 0x302c, 0x00);
	sc438ai_write_register(ViPipe, 0x3032, 0x20);
	sc438ai_write_register(ViPipe, 0x30b8, 0x44);

	// 2688X1520
	// sc438ai_write_register(ViPipe, 0x3200, 0x00);
	// sc438ai_write_register(ViPipe, 0x3201, 0x00);
	// sc438ai_write_register(ViPipe, 0x3202, 0x00);
	// sc438ai_write_register(ViPipe, 0x3203, 0xd4);
	// sc438ai_write_register(ViPipe, 0x3204, 0x0a);
	// sc438ai_write_register(ViPipe, 0x3205, 0x87);
	// sc438ai_write_register(ViPipe, 0x3206, 0x06);
	// sc438ai_write_register(ViPipe, 0x3207, 0xcb);
	// sc438ai_write_register(ViPipe, 0x3208, 0x0a);
	// sc438ai_write_register(ViPipe, 0x3209, 0x80);
	// sc438ai_write_register(ViPipe, 0x320a, 0x05);
	// sc438ai_write_register(ViPipe, 0x320b, 0xf0);
	// sc438ai_write_register(ViPipe, 0x3210, 0x00);
	// sc438ai_write_register(ViPipe, 0x3211, 0x04);
	// sc438ai_write_register(ViPipe, 0x3212, 0x00);
	// sc438ai_write_register(ViPipe, 0x3213, 0x04);

	// 2560X1440
	sc438ai_write_register(ViPipe, 0x3200, 0x00);
	sc438ai_write_register(ViPipe, 0x3201, 0x40);
	sc438ai_write_register(ViPipe, 0x3202, 0x00);
	sc438ai_write_register(ViPipe, 0x3203, 0xfc);
	sc438ai_write_register(ViPipe, 0x3204, 0x0a);
	sc438ai_write_register(ViPipe, 0x3205, 0x47);
	sc438ai_write_register(ViPipe, 0x3206, 0x06);
	sc438ai_write_register(ViPipe, 0x3207, 0xa3);
	sc438ai_write_register(ViPipe, 0x3208, 0x0a);
	sc438ai_write_register(ViPipe, 0x3209, 0x00);
	sc438ai_write_register(ViPipe, 0x320a, 0x05);
	sc438ai_write_register(ViPipe, 0x320b, 0xa0);
	sc438ai_write_register(ViPipe, 0x3210, 0x00);
	sc438ai_write_register(ViPipe, 0x3211, 0x04);
	sc438ai_write_register(ViPipe, 0x3212, 0x00);
	sc438ai_write_register(ViPipe, 0x3213, 0x04);

	// HTS
	sc438ai_write_register(ViPipe, 0x320c, 0x05);
	sc438ai_write_register(ViPipe, 0x320d, 0xdc);
	// VTS normal mode
	sc438ai_write_register(ViPipe, 0x320e, 0x07);
	sc438ai_write_register(ViPipe, 0x320f, 0xd0);

	// slave mode
	// sc438ai_write_register(ViPipe, 0x3222, 0x01); // slave efsync
	// sc438ai_write_register(ViPipe, 0x3230, 0x00);
	// sc438ai_write_register(ViPipe, 0x3231, 0x04);
	// sc438ai_write_register(ViPipe, 0x300a, 0x40);
	// sc438ai_write_register(ViPipe, 0x320e, 0x07);
	// sc438ai_write_register(ViPipe, 0x320f, 0xce); // vts-2

	// master mode
	sc438ai_write_register(ViPipe, 0x3222, 0x00); // output fsync
	sc438ai_write_register(ViPipe, 0x3224, 0xc2);
	sc438ai_write_register(ViPipe, 0x3230, 0x00);
	sc438ai_write_register(ViPipe, 0x3231, 0x04);
	sc438ai_write_register(ViPipe, 0x3232, 0xa0);
	sc438ai_write_register(ViPipe, 0x300a, 0x24);

	// non-continue mode
	// slave mode
	// sc438ai_write_register(ViPipe, 0x3250, 0x40);
	// master mode & normal mode
	sc438ai_write_register(ViPipe, 0x3250, 0x00);
	sc438ai_write_register(ViPipe, 0x4800, 0x24);
	sc438ai_write_register(ViPipe, 0x4816, 0x21);
	sc438ai_write_register(ViPipe, 0x4837, 0x11);

	// continue mode
	// sc438ai_write_register(ViPipe, 0x3250, 0x40);
	// sc438ai_write_register(ViPipe, 0x4800, 0x04);

	sc438ai_write_register(ViPipe, 0x3214, 0x11);
	sc438ai_write_register(ViPipe, 0x3215, 0x11);
	sc438ai_write_register(ViPipe, 0x3223, 0xc0);
	sc438ai_write_register(ViPipe, 0x327f, 0x3f);
	sc438ai_write_register(ViPipe, 0x32e0, 0x00);
	sc438ai_write_register(ViPipe, 0x3301, 0x12);
	sc438ai_write_register(ViPipe, 0x3302, 0x20);
	sc438ai_write_register(ViPipe, 0x3304, 0xc0);
	sc438ai_write_register(ViPipe, 0x3306, 0xb0);
	sc438ai_write_register(ViPipe, 0x3309, 0xf0);
	sc438ai_write_register(ViPipe, 0x330a, 0x01);
	sc438ai_write_register(ViPipe, 0x330b, 0x70);
	sc438ai_write_register(ViPipe, 0x330d, 0x10);
	sc438ai_write_register(ViPipe, 0x3310, 0x18);
	sc438ai_write_register(ViPipe, 0x331e, 0xb1);
	sc438ai_write_register(ViPipe, 0x331f, 0xe1);
	sc438ai_write_register(ViPipe, 0x3333, 0x10);
	sc438ai_write_register(ViPipe, 0x3334, 0x40);
	sc438ai_write_register(ViPipe, 0x3364, 0x56);
	sc438ai_write_register(ViPipe, 0x338f, 0x80);
	sc438ai_write_register(ViPipe, 0x3393, 0x1c);
	sc438ai_write_register(ViPipe, 0x3394, 0x2c);
	sc438ai_write_register(ViPipe, 0x3395, 0x3c);
	sc438ai_write_register(ViPipe, 0x3399, 0x0c);
	sc438ai_write_register(ViPipe, 0x339a, 0x10);
	sc438ai_write_register(ViPipe, 0x339b, 0x18);
	sc438ai_write_register(ViPipe, 0x339c, 0x80);
	sc438ai_write_register(ViPipe, 0x33ac, 0x10);
	sc438ai_write_register(ViPipe, 0x33ad, 0x2c);
	sc438ai_write_register(ViPipe, 0x33ae, 0xb0);
	sc438ai_write_register(ViPipe, 0x33af, 0xe0);
	sc438ai_write_register(ViPipe, 0x33b0, 0x0f);
	sc438ai_write_register(ViPipe, 0x33b2, 0x2c);
	sc438ai_write_register(ViPipe, 0x33b3, 0x02);
	sc438ai_write_register(ViPipe, 0x349f, 0x03);
	sc438ai_write_register(ViPipe, 0x34a8, 0x02);
	sc438ai_write_register(ViPipe, 0x34a9, 0x08);
	sc438ai_write_register(ViPipe, 0x34aa, 0x01);
	sc438ai_write_register(ViPipe, 0x34ab, 0x70);
	sc438ai_write_register(ViPipe, 0x34ac, 0x01);
	sc438ai_write_register(ViPipe, 0x34ad, 0x70);
	sc438ai_write_register(ViPipe, 0x34f9, 0x12);
	sc438ai_write_register(ViPipe, 0x3631, 0x0f);
	sc438ai_write_register(ViPipe, 0x3632, 0x8d);
	sc438ai_write_register(ViPipe, 0x3633, 0x4d);
	sc438ai_write_register(ViPipe, 0x363b, 0x58);
	sc438ai_write_register(ViPipe, 0x363c, 0xd8);
	sc438ai_write_register(ViPipe, 0x363d, 0x20);
	sc438ai_write_register(ViPipe, 0x3641, 0x08);
	sc438ai_write_register(ViPipe, 0x3670, 0x32);
	sc438ai_write_register(ViPipe, 0x3671, 0x34);
	sc438ai_write_register(ViPipe, 0x3672, 0x26);
	sc438ai_write_register(ViPipe, 0x3673, 0x04);
	sc438ai_write_register(ViPipe, 0x3674, 0x08);
	sc438ai_write_register(ViPipe, 0x3675, 0x04);
	sc438ai_write_register(ViPipe, 0x3676, 0x18);
	sc438ai_write_register(ViPipe, 0x367e, 0x49);
	sc438ai_write_register(ViPipe, 0x367f, 0x49);
	sc438ai_write_register(ViPipe, 0x3680, 0x49);
	sc438ai_write_register(ViPipe, 0x3681, 0x04);
	sc438ai_write_register(ViPipe, 0x3682, 0x08);
	sc438ai_write_register(ViPipe, 0x3683, 0x04);
	sc438ai_write_register(ViPipe, 0x3684, 0x38);
	sc438ai_write_register(ViPipe, 0x3685, 0xc1);
	sc438ai_write_register(ViPipe, 0x3686, 0xc2);
	sc438ai_write_register(ViPipe, 0x3687, 0xc1);
	sc438ai_write_register(ViPipe, 0x3688, 0xc1);
	sc438ai_write_register(ViPipe, 0x3689, 0xc1);
	sc438ai_write_register(ViPipe, 0x368a, 0xc1);
	sc438ai_write_register(ViPipe, 0x368b, 0xc4);
	sc438ai_write_register(ViPipe, 0x368c, 0xc1);
	sc438ai_write_register(ViPipe, 0x368d, 0x00);
	sc438ai_write_register(ViPipe, 0x368e, 0x08);
	sc438ai_write_register(ViPipe, 0x368f, 0x00);
	sc438ai_write_register(ViPipe, 0x3690, 0x18);
	sc438ai_write_register(ViPipe, 0x3691, 0x04);
	sc438ai_write_register(ViPipe, 0x3692, 0x00);
	sc438ai_write_register(ViPipe, 0x3693, 0x04);
	sc438ai_write_register(ViPipe, 0x3694, 0x08);
	sc438ai_write_register(ViPipe, 0x3695, 0x04);
	sc438ai_write_register(ViPipe, 0x3696, 0x18);
	sc438ai_write_register(ViPipe, 0x3697, 0x04);
	sc438ai_write_register(ViPipe, 0x3698, 0x38);
	sc438ai_write_register(ViPipe, 0x3699, 0x04);
	sc438ai_write_register(ViPipe, 0x369a, 0x78);
	sc438ai_write_register(ViPipe, 0x36d0, 0x0d);
	sc438ai_write_register(ViPipe, 0x36ea, 0x0a);
	sc438ai_write_register(ViPipe, 0x36eb, 0x0c);
	sc438ai_write_register(ViPipe, 0x36ec, 0x43);
	sc438ai_write_register(ViPipe, 0x36ed, 0xaa);
	sc438ai_write_register(ViPipe, 0x370f, 0x13);
	sc438ai_write_register(ViPipe, 0x3721, 0x6c);
	sc438ai_write_register(ViPipe, 0x3722, 0x8b);
	sc438ai_write_register(ViPipe, 0x3724, 0xd1);
	sc438ai_write_register(ViPipe, 0x3729, 0x34);
	sc438ai_write_register(ViPipe, 0x37b0, 0x17);
	sc438ai_write_register(ViPipe, 0x37b1, 0x17);
	sc438ai_write_register(ViPipe, 0x37b2, 0x13);
	sc438ai_write_register(ViPipe, 0x37b3, 0x04);
	sc438ai_write_register(ViPipe, 0x37b4, 0x08);
	sc438ai_write_register(ViPipe, 0x37b5, 0x04);
	sc438ai_write_register(ViPipe, 0x37b6, 0x38);
	sc438ai_write_register(ViPipe, 0x37b7, 0x1d);
	sc438ai_write_register(ViPipe, 0x37b8, 0x1f);
	sc438ai_write_register(ViPipe, 0x37b9, 0x1f);
	sc438ai_write_register(ViPipe, 0x37ba, 0x04);
	sc438ai_write_register(ViPipe, 0x37bb, 0x04);
	sc438ai_write_register(ViPipe, 0x37bc, 0x04);
	sc438ai_write_register(ViPipe, 0x37bd, 0x04);
	sc438ai_write_register(ViPipe, 0x37be, 0x08);
	sc438ai_write_register(ViPipe, 0x37bf, 0x04);
	sc438ai_write_register(ViPipe, 0x37c0, 0x38);
	sc438ai_write_register(ViPipe, 0x37c1, 0x04);
	sc438ai_write_register(ViPipe, 0x37c2, 0x08);
	sc438ai_write_register(ViPipe, 0x37c3, 0x04);
	sc438ai_write_register(ViPipe, 0x37c4, 0x38);
	sc438ai_write_register(ViPipe, 0x37fa, 0x09);
	sc438ai_write_register(ViPipe, 0x37fb, 0x22);
	sc438ai_write_register(ViPipe, 0x37fc, 0x30);
	sc438ai_write_register(ViPipe, 0x37fd, 0x26);
	sc438ai_write_register(ViPipe, 0x3900, 0x05);
	sc438ai_write_register(ViPipe, 0x3901, 0x00);
	sc438ai_write_register(ViPipe, 0x3902, 0xc0);
	sc438ai_write_register(ViPipe, 0x3903, 0x40);
	sc438ai_write_register(ViPipe, 0x3905, 0x2d);
	sc438ai_write_register(ViPipe, 0x391a, 0x72);
	sc438ai_write_register(ViPipe, 0x391b, 0x39);
	sc438ai_write_register(ViPipe, 0x391c, 0x22);
	sc438ai_write_register(ViPipe, 0x391d, 0x00);
	sc438ai_write_register(ViPipe, 0x391f, 0x41);
	sc438ai_write_register(ViPipe, 0x3926, 0xe0);
	sc438ai_write_register(ViPipe, 0x3933, 0x80);
	sc438ai_write_register(ViPipe, 0x3934, 0x03);
	sc438ai_write_register(ViPipe, 0x3935, 0x01);
	sc438ai_write_register(ViPipe, 0x3936, 0xc0);
	sc438ai_write_register(ViPipe, 0x3937, 0x6a);
	sc438ai_write_register(ViPipe, 0x3938, 0x6b);
	sc438ai_write_register(ViPipe, 0x3939, 0x0f);
	sc438ai_write_register(ViPipe, 0x393a, 0xf6);
	sc438ai_write_register(ViPipe, 0x393d, 0x05);
	sc438ai_write_register(ViPipe, 0x393e, 0x30);
	sc438ai_write_register(ViPipe, 0x39dd, 0x00);
	sc438ai_write_register(ViPipe, 0x39de, 0x06);
	sc438ai_write_register(ViPipe, 0x39e7, 0x04);
	sc438ai_write_register(ViPipe, 0x39e8, 0x04);
	sc438ai_write_register(ViPipe, 0x39e9, 0x80);
	sc438ai_write_register(ViPipe, 0x3e00, 0x00);
	sc438ai_write_register(ViPipe, 0x3e01, 0x7c);
	sc438ai_write_register(ViPipe, 0x3e02, 0x80);
	sc438ai_write_register(ViPipe, 0x3e03, 0x0b);
	sc438ai_write_register(ViPipe, 0x3e16, 0x01);
	sc438ai_write_register(ViPipe, 0x3e17, 0x44);
	sc438ai_write_register(ViPipe, 0x3e18, 0x01);
	sc438ai_write_register(ViPipe, 0x3e19, 0x44);
	sc438ai_write_register(ViPipe, 0x440e, 0x02);
	sc438ai_write_register(ViPipe, 0x4509, 0x18);
	sc438ai_write_register(ViPipe, 0x450d, 0x07);
	sc438ai_write_register(ViPipe, 0x480f, 0x03);
	sc438ai_write_register(ViPipe, 0x5000, 0x06);
	sc438ai_write_register(ViPipe, 0x5780, 0x76);
	sc438ai_write_register(ViPipe, 0x5784, 0x10);
	sc438ai_write_register(ViPipe, 0x5785, 0x08);
	sc438ai_write_register(ViPipe, 0x5787, 0x16);
	sc438ai_write_register(ViPipe, 0x5788, 0x16);
	sc438ai_write_register(ViPipe, 0x5789, 0x15);
	sc438ai_write_register(ViPipe, 0x578a, 0x16);
	sc438ai_write_register(ViPipe, 0x578b, 0x16);
	sc438ai_write_register(ViPipe, 0x578c, 0x15);
	sc438ai_write_register(ViPipe, 0x578d, 0x41);
	sc438ai_write_register(ViPipe, 0x5790, 0x11);
	sc438ai_write_register(ViPipe, 0x5791, 0x0f);
	sc438ai_write_register(ViPipe, 0x5792, 0x0f);
	sc438ai_write_register(ViPipe, 0x5793, 0x11);
	sc438ai_write_register(ViPipe, 0x5794, 0x0f);
	sc438ai_write_register(ViPipe, 0x5795, 0x0f);
	sc438ai_write_register(ViPipe, 0x5799, 0x46);
	sc438ai_write_register(ViPipe, 0x579a, 0x77);
	sc438ai_write_register(ViPipe, 0x57a1, 0x04);
	sc438ai_write_register(ViPipe, 0x57a8, 0xd2);
	sc438ai_write_register(ViPipe, 0x57aa, 0x2a);
	sc438ai_write_register(ViPipe, 0x57ab, 0x7f);
	sc438ai_write_register(ViPipe, 0x57ac, 0x00);
	sc438ai_write_register(ViPipe, 0x57ad, 0x00);
	sc438ai_write_register(ViPipe, 0x36e9, 0x53);
	sc438ai_write_register(ViPipe, 0x37f9, 0x00);
#if SC438AI_PATTERN_ENABLE
	sc438ai_write_register(ViPipe, 0x4501, 0xcc);
#endif
	sc438ai_default_reg_init(ViPipe);
	sc438ai_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC438AI 1440P 30fps 10bit MASTER Init OK!===\n", ViPipe);
}

static void sc438ai_linear_1440p30_slave_init(VI_PIPE ViPipe)
{
	sc438ai_write_register(ViPipe, 0x0103, 0x01);
	sc438ai_write_register(ViPipe, 0x0100, 0x00);
	sc438ai_write_register(ViPipe, 0x36e9, 0x80);
	sc438ai_write_register(ViPipe, 0x37f9, 0x80);
	sc438ai_write_register(ViPipe, 0x23b0, 0x00);
	sc438ai_write_register(ViPipe, 0x23b1, 0x08);
	sc438ai_write_register(ViPipe, 0x23b2, 0x00);
	sc438ai_write_register(ViPipe, 0x23b3, 0x18);
	sc438ai_write_register(ViPipe, 0x23b4, 0x00);
	sc438ai_write_register(ViPipe, 0x23b5, 0x38);
	sc438ai_write_register(ViPipe, 0x23b6, 0x04);
	sc438ai_write_register(ViPipe, 0x23b7, 0x08);
	sc438ai_write_register(ViPipe, 0x23b8, 0x04);
	sc438ai_write_register(ViPipe, 0x23b9, 0x18);
	sc438ai_write_register(ViPipe, 0x23ba, 0x04);
	sc438ai_write_register(ViPipe, 0x23bb, 0x38);
	sc438ai_write_register(ViPipe, 0x23c0, 0x04);
	sc438ai_write_register(ViPipe, 0x23c1, 0x00);
	sc438ai_write_register(ViPipe, 0x23c2, 0x04);
	sc438ai_write_register(ViPipe, 0x23c3, 0x18);
	sc438ai_write_register(ViPipe, 0x23c4, 0x04);
	sc438ai_write_register(ViPipe, 0x23c5, 0x78);
	sc438ai_write_register(ViPipe, 0x23c6, 0x04);
	sc438ai_write_register(ViPipe, 0x23c7, 0x08);
	sc438ai_write_register(ViPipe, 0x23c8, 0x04);
	sc438ai_write_register(ViPipe, 0x23c9, 0x78);
	sc438ai_write_register(ViPipe, 0x3018, 0x3b);
	sc438ai_write_register(ViPipe, 0x3019, 0x0c);
	sc438ai_write_register(ViPipe, 0x301e, 0xf0);
	sc438ai_write_register(ViPipe, 0x301f, 0x08);
	sc438ai_write_register(ViPipe, 0x302c, 0x00);
	sc438ai_write_register(ViPipe, 0x3032, 0x20);
	sc438ai_write_register(ViPipe, 0x30b8, 0x44);

	// 2688X1520
	// sc438ai_write_register(ViPipe, 0x3200, 0x00);
	// sc438ai_write_register(ViPipe, 0x3201, 0x00);
	// sc438ai_write_register(ViPipe, 0x3202, 0x00);
	// sc438ai_write_register(ViPipe, 0x3203, 0xd4);
	// sc438ai_write_register(ViPipe, 0x3204, 0x0a);
	// sc438ai_write_register(ViPipe, 0x3205, 0x87);
	// sc438ai_write_register(ViPipe, 0x3206, 0x06);
	// sc438ai_write_register(ViPipe, 0x3207, 0xcb);
	// sc438ai_write_register(ViPipe, 0x3208, 0x0a);
	// sc438ai_write_register(ViPipe, 0x3209, 0x80);
	// sc438ai_write_register(ViPipe, 0x320a, 0x05);
	// sc438ai_write_register(ViPipe, 0x320b, 0xf0);
	// sc438ai_write_register(ViPipe, 0x3210, 0x00);
	// sc438ai_write_register(ViPipe, 0x3211, 0x04);
	// sc438ai_write_register(ViPipe, 0x3212, 0x00);
	// sc438ai_write_register(ViPipe, 0x3213, 0x04);

	// 2560X1440
	sc438ai_write_register(ViPipe, 0x3200, 0x00);
	sc438ai_write_register(ViPipe, 0x3201, 0x40);
	sc438ai_write_register(ViPipe, 0x3202, 0x00);
	sc438ai_write_register(ViPipe, 0x3203, 0xfc);
	sc438ai_write_register(ViPipe, 0x3204, 0x0a);
	sc438ai_write_register(ViPipe, 0x3205, 0x47);
	sc438ai_write_register(ViPipe, 0x3206, 0x06);
	sc438ai_write_register(ViPipe, 0x3207, 0xa3);
	sc438ai_write_register(ViPipe, 0x3208, 0x0a);
	sc438ai_write_register(ViPipe, 0x3209, 0x00);
	sc438ai_write_register(ViPipe, 0x320a, 0x05);
	sc438ai_write_register(ViPipe, 0x320b, 0xa0);
	sc438ai_write_register(ViPipe, 0x3210, 0x00);
	sc438ai_write_register(ViPipe, 0x3211, 0x04);
	sc438ai_write_register(ViPipe, 0x3212, 0x00);
	sc438ai_write_register(ViPipe, 0x3213, 0x04);

	// HTS
	sc438ai_write_register(ViPipe, 0x320c, 0x05);
	sc438ai_write_register(ViPipe, 0x320d, 0xdc);
	// VTS normal mode
	// sc438ai_write_register(ViPipe, 0x320e, 0x07);
	// sc438ai_write_register(ViPipe, 0x320f, 0xd0);

	// slave mode
	sc438ai_write_register(ViPipe, 0x3222, 0x01); // slave efsync
	sc438ai_write_register(ViPipe, 0x3230, 0x00);
	sc438ai_write_register(ViPipe, 0x3231, 0x04);
	sc438ai_write_register(ViPipe, 0x300a, 0x40);
	sc438ai_write_register(ViPipe, 0x320e, 0x07);
	sc438ai_write_register(ViPipe, 0x320f, 0xce); // vts-2

	// master mode
	// sc438ai_write_register(ViPipe, 0x3222, 0x00); // output fsync
	// sc438ai_write_register(ViPipe, 0x3224, 0xc2);
	// sc438ai_write_register(ViPipe, 0x3230, 0x00);
	// sc438ai_write_register(ViPipe, 0x3231, 0x04);
	// sc438ai_write_register(ViPipe, 0x3232, 0xa0);
	// sc438ai_write_register(ViPipe, 0x300a, 0x24);

	// non-continue mode
	// slave mode
	sc438ai_write_register(ViPipe, 0x3250, 0x40);
	// master mode & normal mode
	// sc438ai_write_register(ViPipe, 0x3250, 0x00);
	sc438ai_write_register(ViPipe, 0x4800, 0x24);
	sc438ai_write_register(ViPipe, 0x4816, 0x21);
	sc438ai_write_register(ViPipe, 0x4837, 0x11);

	// continue mode
	// sc438ai_write_register(ViPipe, 0x3250, 0x40);
	// sc438ai_write_register(ViPipe, 0x4800, 0x04);

	sc438ai_write_register(ViPipe, 0x3214, 0x11);
	sc438ai_write_register(ViPipe, 0x3215, 0x11);
	sc438ai_write_register(ViPipe, 0x3223, 0xc0);
	sc438ai_write_register(ViPipe, 0x327f, 0x3f);
	sc438ai_write_register(ViPipe, 0x32e0, 0x00);
	sc438ai_write_register(ViPipe, 0x3301, 0x12);
	sc438ai_write_register(ViPipe, 0x3302, 0x20);
	sc438ai_write_register(ViPipe, 0x3304, 0xc0);
	sc438ai_write_register(ViPipe, 0x3306, 0xb0);
	sc438ai_write_register(ViPipe, 0x3309, 0xf0);
	sc438ai_write_register(ViPipe, 0x330a, 0x01);
	sc438ai_write_register(ViPipe, 0x330b, 0x70);
	sc438ai_write_register(ViPipe, 0x330d, 0x10);
	sc438ai_write_register(ViPipe, 0x3310, 0x18);
	sc438ai_write_register(ViPipe, 0x331e, 0xb1);
	sc438ai_write_register(ViPipe, 0x331f, 0xe1);
	sc438ai_write_register(ViPipe, 0x3333, 0x10);
	sc438ai_write_register(ViPipe, 0x3334, 0x40);
	sc438ai_write_register(ViPipe, 0x3364, 0x56);
	sc438ai_write_register(ViPipe, 0x338f, 0x80);
	sc438ai_write_register(ViPipe, 0x3393, 0x1c);
	sc438ai_write_register(ViPipe, 0x3394, 0x2c);
	sc438ai_write_register(ViPipe, 0x3395, 0x3c);
	sc438ai_write_register(ViPipe, 0x3399, 0x0c);
	sc438ai_write_register(ViPipe, 0x339a, 0x10);
	sc438ai_write_register(ViPipe, 0x339b, 0x18);
	sc438ai_write_register(ViPipe, 0x339c, 0x80);
	sc438ai_write_register(ViPipe, 0x33ac, 0x10);
	sc438ai_write_register(ViPipe, 0x33ad, 0x2c);
	sc438ai_write_register(ViPipe, 0x33ae, 0xb0);
	sc438ai_write_register(ViPipe, 0x33af, 0xe0);
	sc438ai_write_register(ViPipe, 0x33b0, 0x0f);
	sc438ai_write_register(ViPipe, 0x33b2, 0x2c);
	sc438ai_write_register(ViPipe, 0x33b3, 0x02);
	sc438ai_write_register(ViPipe, 0x349f, 0x03);
	sc438ai_write_register(ViPipe, 0x34a8, 0x02);
	sc438ai_write_register(ViPipe, 0x34a9, 0x08);
	sc438ai_write_register(ViPipe, 0x34aa, 0x01);
	sc438ai_write_register(ViPipe, 0x34ab, 0x70);
	sc438ai_write_register(ViPipe, 0x34ac, 0x01);
	sc438ai_write_register(ViPipe, 0x34ad, 0x70);
	sc438ai_write_register(ViPipe, 0x34f9, 0x12);
	sc438ai_write_register(ViPipe, 0x3631, 0x0f);
	sc438ai_write_register(ViPipe, 0x3632, 0x8d);
	sc438ai_write_register(ViPipe, 0x3633, 0x4d);
	sc438ai_write_register(ViPipe, 0x363b, 0x58);
	sc438ai_write_register(ViPipe, 0x363c, 0xd8);
	sc438ai_write_register(ViPipe, 0x363d, 0x20);
	sc438ai_write_register(ViPipe, 0x3641, 0x08);
	sc438ai_write_register(ViPipe, 0x3670, 0x32);
	sc438ai_write_register(ViPipe, 0x3671, 0x34);
	sc438ai_write_register(ViPipe, 0x3672, 0x26);
	sc438ai_write_register(ViPipe, 0x3673, 0x04);
	sc438ai_write_register(ViPipe, 0x3674, 0x08);
	sc438ai_write_register(ViPipe, 0x3675, 0x04);
	sc438ai_write_register(ViPipe, 0x3676, 0x18);
	sc438ai_write_register(ViPipe, 0x367e, 0x49);
	sc438ai_write_register(ViPipe, 0x367f, 0x49);
	sc438ai_write_register(ViPipe, 0x3680, 0x49);
	sc438ai_write_register(ViPipe, 0x3681, 0x04);
	sc438ai_write_register(ViPipe, 0x3682, 0x08);
	sc438ai_write_register(ViPipe, 0x3683, 0x04);
	sc438ai_write_register(ViPipe, 0x3684, 0x38);
	sc438ai_write_register(ViPipe, 0x3685, 0xc1);
	sc438ai_write_register(ViPipe, 0x3686, 0xc2);
	sc438ai_write_register(ViPipe, 0x3687, 0xc1);
	sc438ai_write_register(ViPipe, 0x3688, 0xc1);
	sc438ai_write_register(ViPipe, 0x3689, 0xc1);
	sc438ai_write_register(ViPipe, 0x368a, 0xc1);
	sc438ai_write_register(ViPipe, 0x368b, 0xc4);
	sc438ai_write_register(ViPipe, 0x368c, 0xc1);
	sc438ai_write_register(ViPipe, 0x368d, 0x00);
	sc438ai_write_register(ViPipe, 0x368e, 0x08);
	sc438ai_write_register(ViPipe, 0x368f, 0x00);
	sc438ai_write_register(ViPipe, 0x3690, 0x18);
	sc438ai_write_register(ViPipe, 0x3691, 0x04);
	sc438ai_write_register(ViPipe, 0x3692, 0x00);
	sc438ai_write_register(ViPipe, 0x3693, 0x04);
	sc438ai_write_register(ViPipe, 0x3694, 0x08);
	sc438ai_write_register(ViPipe, 0x3695, 0x04);
	sc438ai_write_register(ViPipe, 0x3696, 0x18);
	sc438ai_write_register(ViPipe, 0x3697, 0x04);
	sc438ai_write_register(ViPipe, 0x3698, 0x38);
	sc438ai_write_register(ViPipe, 0x3699, 0x04);
	sc438ai_write_register(ViPipe, 0x369a, 0x78);
	sc438ai_write_register(ViPipe, 0x36d0, 0x0d);
	sc438ai_write_register(ViPipe, 0x36ea, 0x0a);
	sc438ai_write_register(ViPipe, 0x36eb, 0x0c);
	sc438ai_write_register(ViPipe, 0x36ec, 0x43);
	sc438ai_write_register(ViPipe, 0x36ed, 0xaa);
	sc438ai_write_register(ViPipe, 0x370f, 0x13);
	sc438ai_write_register(ViPipe, 0x3721, 0x6c);
	sc438ai_write_register(ViPipe, 0x3722, 0x8b);
	sc438ai_write_register(ViPipe, 0x3724, 0xd1);
	sc438ai_write_register(ViPipe, 0x3729, 0x34);
	sc438ai_write_register(ViPipe, 0x37b0, 0x17);
	sc438ai_write_register(ViPipe, 0x37b1, 0x17);
	sc438ai_write_register(ViPipe, 0x37b2, 0x13);
	sc438ai_write_register(ViPipe, 0x37b3, 0x04);
	sc438ai_write_register(ViPipe, 0x37b4, 0x08);
	sc438ai_write_register(ViPipe, 0x37b5, 0x04);
	sc438ai_write_register(ViPipe, 0x37b6, 0x38);
	sc438ai_write_register(ViPipe, 0x37b7, 0x1d);
	sc438ai_write_register(ViPipe, 0x37b8, 0x1f);
	sc438ai_write_register(ViPipe, 0x37b9, 0x1f);
	sc438ai_write_register(ViPipe, 0x37ba, 0x04);
	sc438ai_write_register(ViPipe, 0x37bb, 0x04);
	sc438ai_write_register(ViPipe, 0x37bc, 0x04);
	sc438ai_write_register(ViPipe, 0x37bd, 0x04);
	sc438ai_write_register(ViPipe, 0x37be, 0x08);
	sc438ai_write_register(ViPipe, 0x37bf, 0x04);
	sc438ai_write_register(ViPipe, 0x37c0, 0x38);
	sc438ai_write_register(ViPipe, 0x37c1, 0x04);
	sc438ai_write_register(ViPipe, 0x37c2, 0x08);
	sc438ai_write_register(ViPipe, 0x37c3, 0x04);
	sc438ai_write_register(ViPipe, 0x37c4, 0x38);
	sc438ai_write_register(ViPipe, 0x37fa, 0x09);
	sc438ai_write_register(ViPipe, 0x37fb, 0x22);
	sc438ai_write_register(ViPipe, 0x37fc, 0x30);
	sc438ai_write_register(ViPipe, 0x37fd, 0x26);
	sc438ai_write_register(ViPipe, 0x3900, 0x05);
	sc438ai_write_register(ViPipe, 0x3901, 0x00);
	sc438ai_write_register(ViPipe, 0x3902, 0xc0);
	sc438ai_write_register(ViPipe, 0x3903, 0x40);
	sc438ai_write_register(ViPipe, 0x3905, 0x2d);
	sc438ai_write_register(ViPipe, 0x391a, 0x72);
	sc438ai_write_register(ViPipe, 0x391b, 0x39);
	sc438ai_write_register(ViPipe, 0x391c, 0x22);
	sc438ai_write_register(ViPipe, 0x391d, 0x00);
	sc438ai_write_register(ViPipe, 0x391f, 0x41);
	sc438ai_write_register(ViPipe, 0x3926, 0xe0);
	sc438ai_write_register(ViPipe, 0x3933, 0x80);
	sc438ai_write_register(ViPipe, 0x3934, 0x03);
	sc438ai_write_register(ViPipe, 0x3935, 0x01);
	sc438ai_write_register(ViPipe, 0x3936, 0xc0);
	sc438ai_write_register(ViPipe, 0x3937, 0x6a);
	sc438ai_write_register(ViPipe, 0x3938, 0x6b);
	sc438ai_write_register(ViPipe, 0x3939, 0x0f);
	sc438ai_write_register(ViPipe, 0x393a, 0xf6);
	sc438ai_write_register(ViPipe, 0x393d, 0x05);
	sc438ai_write_register(ViPipe, 0x393e, 0x30);
	sc438ai_write_register(ViPipe, 0x39dd, 0x00);
	sc438ai_write_register(ViPipe, 0x39de, 0x06);
	sc438ai_write_register(ViPipe, 0x39e7, 0x04);
	sc438ai_write_register(ViPipe, 0x39e8, 0x04);
	sc438ai_write_register(ViPipe, 0x39e9, 0x80);
	sc438ai_write_register(ViPipe, 0x3e00, 0x00);
	sc438ai_write_register(ViPipe, 0x3e01, 0x7c);
	sc438ai_write_register(ViPipe, 0x3e02, 0x80);
	sc438ai_write_register(ViPipe, 0x3e03, 0x0b);
	sc438ai_write_register(ViPipe, 0x3e16, 0x01);
	sc438ai_write_register(ViPipe, 0x3e17, 0x44);
	sc438ai_write_register(ViPipe, 0x3e18, 0x01);
	sc438ai_write_register(ViPipe, 0x3e19, 0x44);
	sc438ai_write_register(ViPipe, 0x440e, 0x02);
	sc438ai_write_register(ViPipe, 0x4509, 0x18);
	sc438ai_write_register(ViPipe, 0x450d, 0x07);
	sc438ai_write_register(ViPipe, 0x480f, 0x03);
	sc438ai_write_register(ViPipe, 0x5000, 0x06);
	sc438ai_write_register(ViPipe, 0x5780, 0x76);
	sc438ai_write_register(ViPipe, 0x5784, 0x10);
	sc438ai_write_register(ViPipe, 0x5785, 0x08);
	sc438ai_write_register(ViPipe, 0x5787, 0x16);
	sc438ai_write_register(ViPipe, 0x5788, 0x16);
	sc438ai_write_register(ViPipe, 0x5789, 0x15);
	sc438ai_write_register(ViPipe, 0x578a, 0x16);
	sc438ai_write_register(ViPipe, 0x578b, 0x16);
	sc438ai_write_register(ViPipe, 0x578c, 0x15);
	sc438ai_write_register(ViPipe, 0x578d, 0x41);
	sc438ai_write_register(ViPipe, 0x5790, 0x11);
	sc438ai_write_register(ViPipe, 0x5791, 0x0f);
	sc438ai_write_register(ViPipe, 0x5792, 0x0f);
	sc438ai_write_register(ViPipe, 0x5793, 0x11);
	sc438ai_write_register(ViPipe, 0x5794, 0x0f);
	sc438ai_write_register(ViPipe, 0x5795, 0x0f);
	sc438ai_write_register(ViPipe, 0x5799, 0x46);
	sc438ai_write_register(ViPipe, 0x579a, 0x77);
	sc438ai_write_register(ViPipe, 0x57a1, 0x04);
	sc438ai_write_register(ViPipe, 0x57a8, 0xd2);
	sc438ai_write_register(ViPipe, 0x57aa, 0x2a);
	sc438ai_write_register(ViPipe, 0x57ab, 0x7f);
	sc438ai_write_register(ViPipe, 0x57ac, 0x00);
	sc438ai_write_register(ViPipe, 0x57ad, 0x00);
	sc438ai_write_register(ViPipe, 0x36e9, 0x53);
	sc438ai_write_register(ViPipe, 0x37f9, 0x00);
#if SC438AI_PATTERN_ENABLE
	sc438ai_write_register(ViPipe, 0x4501, 0xcc);
#endif
	sc438ai_default_reg_init(ViPipe);
	sc438ai_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC438AI 1440P 30fps 10bit SLAVE Init OK!===\n", ViPipe);
}
