#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "sc2356_cmos_ex.h"

#define SC2356_CHIP_ID_HI_ADDR		0x3107
#define SC2356_CHIP_ID_LO_ADDR		0x3108
#define SC2356_CHIP_ID			    0xeb52

static void sc2356_linear_360P93_init(VI_PIPE ViPipe);
static void sc2356_linear_1600x1200p30_init(VI_PIPE ViPipe);

const CVI_U8 sc2356_i2c_addr = 0x36;        /* I2C Address of SC2356 */
const CVI_U32 sc2356_addr_byte = 2;
const CVI_U32 sc2356_data_byte = 1;

int sc2356_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC2356_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc2356_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC2356_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc2356_read_register(VI_PIPE ViPipe, int addr)
{
CVI_U8 i2c_id = (CVI_U8)g_aunSC2356_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc2356_i2c_addr, (CVI_U32)addr, sc2356_addr_byte, sc2356_data_byte);
}

int sc2356_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC2356_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sc2356_i2c_addr, (CVI_U32)addr, sc2356_addr_byte,
				(CVI_U32)data, sc2356_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc2356_prog(VI_PIPE ViPipe, int *rom)
{
	int i = 0;

	while (1) {
		int lookup = rom[i++];
		int addr = (lookup >> 16) & 0xFFFF;
		int data = lookup & 0xFFFF;

		if (addr == 0xFFFE)
			delay_ms(data);
		else if (addr != 0xFFFF)
			sc2356_write_register(ViPipe, addr, data);
	}
}

void sc2356_standby(VI_PIPE ViPipe)
{
	sc2356_write_register(ViPipe, 0x0100, 0x00);
}

void sc2356_restart(VI_PIPE ViPipe)
{
	sc2356_write_register(ViPipe, 0x0103, 0x01);
}

void sc2356_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC2356[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastSC2356[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			sc2356_write_register(ViPipe,
				g_pastSC2356[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC2356[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void sc2356_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc2356_write_register(ViPipe, 0x3221, val);
}

int sc2356_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	usleep(4*1000);
	if (sc2356_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc2356_read_register(ViPipe, SC2356_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc2356_read_register(ViPipe, SC2356_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC2356_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sc2356_init(VI_PIPE ViPipe)
{
    CVI_U8 u8ImgMode = g_pastSC2356[ViPipe]->u8ImgMode;
	sc2356_i2c_init(ViPipe);

	//linear mode only
    if (u8ImgMode == SC2356_MODE_496X360P93)
        sc2356_linear_360P93_init(ViPipe);
    else if (u8ImgMode == SC2356_MODE_1600X1200P30)
        sc2356_linear_1600x1200p30_init(ViPipe);
    else {
    }

	g_pastSC2356[ViPipe]->bInit = CVI_TRUE;
}

void sc2356_exit(VI_PIPE ViPipe)
{
	sc2356_i2c_exit(ViPipe);
}

/* 496x360@93fps binning*/
static void sc2356_linear_360P93_init(VI_PIPE ViPipe)
{
    sc2356_write_register(ViPipe, 0x0103, 0x01);
    sc2356_write_register(ViPipe, 0x0100, 0x00);
    sc2356_write_register(ViPipe, 0x36e9, 0x80);
    sc2356_write_register(ViPipe, 0x3031, 0x08);
    sc2356_write_register(ViPipe, 0x3037, 0x00);
    sc2356_write_register(ViPipe, 0x303f, 0x82);
    // sc2356_write_register(ViPipe, 0x3208, 0x03);
    // sc2356_write_register(ViPipe, 0x3209, 0x20);
    // sc2356_write_register(ViPipe, 0x320a, 0x02);
    // sc2356_write_register(ViPipe, 0x320b, 0x58);

    // sc2356_write_register(ViPipe, 0x3211, 0x02);
    sc2356_write_register(ViPipe, 0x3213, 0x02);
    sc2356_write_register(ViPipe, 0x3215, 0x31);
    sc2356_write_register(ViPipe, 0x3220, 0x01);
    sc2356_write_register(ViPipe, 0x3253, 0x18);
    sc2356_write_register(ViPipe, 0x3301, 0x09);
    sc2356_write_register(ViPipe, 0x3304, 0x58);
    sc2356_write_register(ViPipe, 0x3306, 0x48);
    sc2356_write_register(ViPipe, 0x3308, 0x10);
    sc2356_write_register(ViPipe, 0x3309, 0xa0);
    sc2356_write_register(ViPipe, 0x330a, 0x00);
    sc2356_write_register(ViPipe, 0x330b, 0xd4);
    sc2356_write_register(ViPipe, 0x330d, 0x38);
    sc2356_write_register(ViPipe, 0x330e, 0x40);
    sc2356_write_register(ViPipe, 0x3310, 0x06);
    sc2356_write_register(ViPipe, 0x331c, 0x02);
    sc2356_write_register(ViPipe, 0x331e, 0x49);
    sc2356_write_register(ViPipe, 0x331f, 0x91);
    sc2356_write_register(ViPipe, 0x3320, 0x0c);
    sc2356_write_register(ViPipe, 0x3323, 0x0e);
    sc2356_write_register(ViPipe, 0x3328, 0x0c);
    sc2356_write_register(ViPipe, 0x3329, 0x10);
    sc2356_write_register(ViPipe, 0x332b, 0x0a);
    sc2356_write_register(ViPipe, 0x3364, 0x57);
    sc2356_write_register(ViPipe, 0x338c, 0x03);
    sc2356_write_register(ViPipe, 0x338d, 0xff);
    sc2356_write_register(ViPipe, 0x3390, 0x01);
    sc2356_write_register(ViPipe, 0x3391, 0x03);
    sc2356_write_register(ViPipe, 0x3392, 0x07);
    sc2356_write_register(ViPipe, 0x3393, 0x0a);
    sc2356_write_register(ViPipe, 0x3394, 0x0b);
    sc2356_write_register(ViPipe, 0x3395, 0xff);
    sc2356_write_register(ViPipe, 0x33ac, 0x08);
    sc2356_write_register(ViPipe, 0x33ae, 0x14);
    sc2356_write_register(ViPipe, 0x33af, 0x38);
    sc2356_write_register(ViPipe, 0x33b2, 0x01);
    sc2356_write_register(ViPipe, 0x33b3, 0x1f);
    sc2356_write_register(ViPipe, 0x3622, 0xe0);
    sc2356_write_register(ViPipe, 0x3624, 0x41);
    sc2356_write_register(ViPipe, 0x3625, 0x08);
    sc2356_write_register(ViPipe, 0x3626, 0x01);
    sc2356_write_register(ViPipe, 0x3630, 0x08);
    sc2356_write_register(ViPipe, 0x3633, 0x44);
    sc2356_write_register(ViPipe, 0x3636, 0x44);
    sc2356_write_register(ViPipe, 0x3637, 0xe9);
    sc2356_write_register(ViPipe, 0x3638, 0x0d);
    sc2356_write_register(ViPipe, 0x3670, 0x0d);
    sc2356_write_register(ViPipe, 0x3677, 0x82);
    sc2356_write_register(ViPipe, 0x3678, 0x84);
    sc2356_write_register(ViPipe, 0x3679, 0x84);
    sc2356_write_register(ViPipe, 0x367e, 0x01);
    sc2356_write_register(ViPipe, 0x367f, 0x07);
    sc2356_write_register(ViPipe, 0x3690, 0x44);
    sc2356_write_register(ViPipe, 0x3691, 0x44);
    sc2356_write_register(ViPipe, 0x3692, 0x33);
    sc2356_write_register(ViPipe, 0x369c, 0x03);
    sc2356_write_register(ViPipe, 0x369d, 0x07);
    sc2356_write_register(ViPipe, 0x369e, 0xe0);
    sc2356_write_register(ViPipe, 0x369f, 0xee);
    sc2356_write_register(ViPipe, 0x36a0, 0xf0);
    sc2356_write_register(ViPipe, 0x36a1, 0x03);
    sc2356_write_register(ViPipe, 0x36a2, 0x07);
    sc2356_write_register(ViPipe, 0x36ea, 0xc9);
    sc2356_write_register(ViPipe, 0x36eb, 0x42);
    sc2356_write_register(ViPipe, 0x36ed, 0x24);
    sc2356_write_register(ViPipe, 0x3904, 0x04);
    sc2356_write_register(ViPipe, 0x3948, 0x02);
    sc2356_write_register(ViPipe, 0x3949, 0x58);
    // sc2356_write_register(ViPipe, 0x3e01, 0x4d);
    // sc2356_write_register(ViPipe, 0x3e02, 0xa0);
    sc2356_write_register(ViPipe, 0x3e09, 0x00);
    sc2356_write_register(ViPipe, 0x3e26, 0x00);
    sc2356_write_register(ViPipe, 0x3f03, 0x0c);
    sc2356_write_register(ViPipe, 0x4502, 0x14);
    sc2356_write_register(ViPipe, 0x4509, 0x38);
    sc2356_write_register(ViPipe, 0x4815, 0x00);
    sc2356_write_register(ViPipe, 0x4819, 0x05);
    sc2356_write_register(ViPipe, 0x481b, 0x03);
    sc2356_write_register(ViPipe, 0x481d, 0x09);
    sc2356_write_register(ViPipe, 0x481f, 0x02);
    sc2356_write_register(ViPipe, 0x4821, 0x09);
    sc2356_write_register(ViPipe, 0x4823, 0x03);
    sc2356_write_register(ViPipe, 0x4825, 0x02);
    sc2356_write_register(ViPipe, 0x4827, 0x03);
    sc2356_write_register(ViPipe, 0x4829, 0x04);
    sc2356_write_register(ViPipe, 0x5000, 0x46);
    sc2356_write_register(ViPipe, 0x5900, 0xf1);
    sc2356_write_register(ViPipe, 0x5901, 0x04);
    sc2356_write_register(ViPipe, 0x36e9, 0x00);

	//496x360@93fps
	sc2356_write_register(ViPipe, 0x3208, 0x01);
	sc2356_write_register(ViPipe, 0x3209, 0xf0);
	sc2356_write_register(ViPipe, 0x320a, 0x01);
	sc2356_write_register(ViPipe, 0x320b, 0x68);
	sc2356_write_register(ViPipe, 0x3210, 0x01);
	sc2356_write_register(ViPipe, 0x3211, 0x16);
	sc2356_write_register(ViPipe, 0x3203, 0xf0);
	sc2356_write_register(ViPipe, 0x3206, 0x03);
	sc2356_write_register(ViPipe, 0x3207, 0xc7);
	sc2356_write_register(ViPipe, 0x320e, 0x01);
	sc2356_write_register(ViPipe, 0x320f, 0x92);
	sc2356_write_register(ViPipe, 0x3e01, 0x18);
	sc2356_write_register(ViPipe, 0x3e02, 0xe0);
	sc2356_write_register(ViPipe, 0x301f, 0x0c);
	sc2356_write_register(ViPipe, 0x0100, 0x01);

	sc2356_default_reg_init(ViPipe);
	delay_ms(10);

	printf("ViPipe:%d,===SC2356 469x360@93fps 8bit LINE Init OK!===\n", ViPipe);
}

static void sc2356_linear_1600x1200p30_init(VI_PIPE ViPipe)
{
    sc2356_write_register(ViPipe, 0x0103, 0x01);
    sc2356_write_register(ViPipe, 0x0100, 0x00);
    sc2356_write_register(ViPipe, 0x36e9, 0x80);
    sc2356_write_register(ViPipe, 0x36ea, 0x0a);
    sc2356_write_register(ViPipe, 0x36eb, 0x0c);
    sc2356_write_register(ViPipe, 0x36ec, 0x01);
    sc2356_write_register(ViPipe, 0x36ed, 0x18);
    sc2356_write_register(ViPipe, 0x36e9, 0x10);
    sc2356_write_register(ViPipe, 0x301f, 0x1b);
    sc2356_write_register(ViPipe, 0x3301, 0xff);
    sc2356_write_register(ViPipe, 0x3304, 0x68);
    sc2356_write_register(ViPipe, 0x3306, 0x40);
    sc2356_write_register(ViPipe, 0x3308, 0x08);
    sc2356_write_register(ViPipe, 0x3309, 0xa8);
    sc2356_write_register(ViPipe, 0x330b, 0xb0);
    sc2356_write_register(ViPipe, 0x330c, 0x18);
    sc2356_write_register(ViPipe, 0x330d, 0xff);
    sc2356_write_register(ViPipe, 0x330e, 0x20);
    sc2356_write_register(ViPipe, 0x331e, 0x59);
    sc2356_write_register(ViPipe, 0x331f, 0x99);
    sc2356_write_register(ViPipe, 0x3333, 0x10);
    sc2356_write_register(ViPipe, 0x335e, 0x06);
    sc2356_write_register(ViPipe, 0x335f, 0x08);
    sc2356_write_register(ViPipe, 0x3364, 0x1f);
    sc2356_write_register(ViPipe, 0x337c, 0x02);
    sc2356_write_register(ViPipe, 0x337d, 0x0a);
    sc2356_write_register(ViPipe, 0x338f, 0xa0);
    sc2356_write_register(ViPipe, 0x3390, 0x01);
    sc2356_write_register(ViPipe, 0x3391, 0x03);
    sc2356_write_register(ViPipe, 0x3392, 0x1f);
    sc2356_write_register(ViPipe, 0x3393, 0xff);
    sc2356_write_register(ViPipe, 0x3394, 0xff);
    sc2356_write_register(ViPipe, 0x3395, 0xff);
    sc2356_write_register(ViPipe, 0x33a2, 0x04);
    sc2356_write_register(ViPipe, 0x33ad, 0x0c);
    sc2356_write_register(ViPipe, 0x33b1, 0x20);
    sc2356_write_register(ViPipe, 0x33b3, 0x38);
    sc2356_write_register(ViPipe, 0x33f9, 0x40);
    sc2356_write_register(ViPipe, 0x33fb, 0x48);
    sc2356_write_register(ViPipe, 0x33fc, 0x0f);
    sc2356_write_register(ViPipe, 0x33fd, 0x1f);
    sc2356_write_register(ViPipe, 0x349f, 0x03);
    sc2356_write_register(ViPipe, 0x34a6, 0x03);
    sc2356_write_register(ViPipe, 0x34a7, 0x1f);
    sc2356_write_register(ViPipe, 0x34a8, 0x38);
    sc2356_write_register(ViPipe, 0x34a9, 0x30);
    sc2356_write_register(ViPipe, 0x34ab, 0xb0);
    sc2356_write_register(ViPipe, 0x34ad, 0xb0);
    sc2356_write_register(ViPipe, 0x34f8, 0x1f);
    sc2356_write_register(ViPipe, 0x34f9, 0x20);
    sc2356_write_register(ViPipe, 0x3630, 0xa0);
    sc2356_write_register(ViPipe, 0x3631, 0x92);
    sc2356_write_register(ViPipe, 0x3632, 0x64);
    sc2356_write_register(ViPipe, 0x3633, 0x43);
    sc2356_write_register(ViPipe, 0x3637, 0x49);
    sc2356_write_register(ViPipe, 0x363a, 0x85);
    sc2356_write_register(ViPipe, 0x363c, 0x0f);
    sc2356_write_register(ViPipe, 0x3650, 0x31);
    sc2356_write_register(ViPipe, 0x3670, 0x0d);
    sc2356_write_register(ViPipe, 0x3674, 0xc0);
    sc2356_write_register(ViPipe, 0x3675, 0xa0);
    sc2356_write_register(ViPipe, 0x3676, 0xa0);
    sc2356_write_register(ViPipe, 0x3677, 0x92);
    sc2356_write_register(ViPipe, 0x3678, 0x96);
    sc2356_write_register(ViPipe, 0x3679, 0x9a);
    sc2356_write_register(ViPipe, 0x367c, 0x03);
    sc2356_write_register(ViPipe, 0x367d, 0x0f);
    sc2356_write_register(ViPipe, 0x367e, 0x01);
    sc2356_write_register(ViPipe, 0x367f, 0x0f);
    sc2356_write_register(ViPipe, 0x3698, 0x83);
    sc2356_write_register(ViPipe, 0x3699, 0x86);
    sc2356_write_register(ViPipe, 0x369a, 0x8c);
    sc2356_write_register(ViPipe, 0x369b, 0x94);
    sc2356_write_register(ViPipe, 0x36a2, 0x01);
    sc2356_write_register(ViPipe, 0x36a3, 0x03);
    sc2356_write_register(ViPipe, 0x36a4, 0x07);
    sc2356_write_register(ViPipe, 0x36ae, 0x0f);
    sc2356_write_register(ViPipe, 0x36af, 0x1f);
    sc2356_write_register(ViPipe, 0x36bd, 0x22);
    sc2356_write_register(ViPipe, 0x36be, 0x22);
    sc2356_write_register(ViPipe, 0x36bf, 0x22);
    sc2356_write_register(ViPipe, 0x36d0, 0x01);
    sc2356_write_register(ViPipe, 0x370f, 0x02);
    sc2356_write_register(ViPipe, 0x3721, 0x6c);
    sc2356_write_register(ViPipe, 0x3722, 0x8d);
    sc2356_write_register(ViPipe, 0x3725, 0xc5);
    sc2356_write_register(ViPipe, 0x3727, 0x14);
    sc2356_write_register(ViPipe, 0x3728, 0x04);
    sc2356_write_register(ViPipe, 0x37b7, 0x04);
    sc2356_write_register(ViPipe, 0x37b8, 0x04);
    sc2356_write_register(ViPipe, 0x37b9, 0x06);
    sc2356_write_register(ViPipe, 0x37bd, 0x07);
    sc2356_write_register(ViPipe, 0x37be, 0x0f);
    sc2356_write_register(ViPipe, 0x3901, 0x02);
    sc2356_write_register(ViPipe, 0x3903, 0x40);
    sc2356_write_register(ViPipe, 0x3905, 0x8d);
    sc2356_write_register(ViPipe, 0x3907, 0x00);
    sc2356_write_register(ViPipe, 0x3908, 0x41);
    sc2356_write_register(ViPipe, 0x391f, 0x41);
    sc2356_write_register(ViPipe, 0x3933, 0x80);
    sc2356_write_register(ViPipe, 0x3934, 0x02);
    sc2356_write_register(ViPipe, 0x3937, 0x6f);
    sc2356_write_register(ViPipe, 0x393a, 0x01);
    sc2356_write_register(ViPipe, 0x393d, 0x01);
    sc2356_write_register(ViPipe, 0x393e, 0xc0);
    sc2356_write_register(ViPipe, 0x39dd, 0x41);
    sc2356_write_register(ViPipe, 0x3e00, 0x00);
    sc2356_write_register(ViPipe, 0x3e01, 0x4d);
    sc2356_write_register(ViPipe, 0x3e02, 0xc0);
    sc2356_write_register(ViPipe, 0x3e09, 0x00);
    sc2356_write_register(ViPipe, 0x4509, 0x28);
    sc2356_write_register(ViPipe, 0x450d, 0x61);
    sc2356_write_register(ViPipe, 0x0100, 0x01);

    sc2356_default_reg_init(ViPipe);
	delay_ms(10);

	printf("ViPipe:%d,===SC2356 1600x120@30fps 10bit LINE Init OK!===\n", ViPipe);
}