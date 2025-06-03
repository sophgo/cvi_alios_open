#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc233hgs_cmos_ex.h"
#include "sensor_i2c.h"

static void sc233hgs_wdr_1080p60_2to1_init(VI_PIPE ViPipe);
static void sc233hgs_linear_1080p60_init(VI_PIPE ViPipe);

const CVI_U8 sc233hgs_i2c_addr   = 0x32;        /* I2C Address of SC233HGS */
const CVI_U32 sc233hgs_addr_byte = 2;
const CVI_U32 sc233hgs_data_byte = 1;

int sc233hgs_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC233HGS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC233HGS_AddrInfo[ViPipe].s8I2cAddr);
}

int sc233hgs_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC233HGS_BusInfo[ViPipe].s8I2cDev);
}

int sc233hgs_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC233HGS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC233HGS_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc233hgs_addr_byte, sc233hgs_data_byte);
}

int sc233hgs_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC233HGS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC233HGS_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc233hgs_addr_byte, (CVI_U32)data, sc233hgs_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc233hgs_standby(VI_PIPE ViPipe)
{
	sc233hgs_write_register(ViPipe, 0x0100, 0x00);
}

void sc233hgs_restart(VI_PIPE ViPipe)
{
	sc233hgs_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc233hgs_write_register(ViPipe, 0x0100, 0x01);
}

void sc233hgs_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC233HGS[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc233hgs_write_register(ViPipe,
				g_pastSC233HGS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC233HGS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define sc233hgs_CHIP_ID_HI_ADDR		0x3107
#define sc233hgs_CHIP_ID_LO_ADDR		0x3108
#define sc233hgs_CHIP_ID				0xcb61

void sc233hgs_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc233hgs_write_register(ViPipe, 0x3221, val);
}

int sc233hgs_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc233hgs_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc233hgs_read_register(ViPipe, sc233hgs_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc233hgs_read_register(ViPipe, sc233hgs_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != sc233hgs_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc233hgs_init(VI_PIPE ViPipe)
{
	WDR_MODE_E       enWDRMode;
	CVI_BOOL          bInit;
	CVI_U8            u8ImgMode;

	bInit       = g_pastSC233HGS[ViPipe]->bInit;
	enWDRMode   = g_pastSC233HGS[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastSC233HGS[ViPipe]->u8ImgMode;

	sc233hgs_i2c_init(ViPipe);

	/* When sensor first init, config all registers */
	if (bInit == CVI_FALSE) {
		if (enWDRMode == WDR_MODE_2To1_LINE) {
			if (u8ImgMode == SC233HGS_MODE_1080P60_WDR) {
				/* sc233hgs_MODE_1080P30_WDR */
				sc233hgs_wdr_1080p60_2to1_init(ViPipe);
			} else {
			}
		} else {
			sc233hgs_linear_1080p60_init(ViPipe);
		}
	}
	/* When sensor switch mode(linear<->WDR or resolution), config different registers(if possible) */
	else {
		if (enWDRMode == WDR_MODE_2To1_LINE) {
			if (u8ImgMode == SC233HGS_MODE_1080P60) {
				/* sc233hgs_MODE_1080P30_WDR */
				sc233hgs_wdr_1080p60_2to1_init(ViPipe);
			} else {
			}
		} else {
			sc233hgs_linear_1080p60_init(ViPipe);
		}
	}
	g_pastSC233HGS[ViPipe]->bInit = CVI_TRUE;
}

/* 1080P60*/
static void sc233hgs_linear_1080p60_init(VI_PIPE ViPipe)
{
	sc233hgs_write_register(ViPipe, 0x2100, 0x00);
	sc233hgs_write_register(ViPipe, 0x36e9, 0x80);
	sc233hgs_write_register(ViPipe, 0x37f9, 0x80);
	sc233hgs_write_register(ViPipe, 0x300c, 0x24);
	sc233hgs_write_register(ViPipe, 0x3018, 0x3a);
	sc233hgs_write_register(ViPipe, 0x3019, 0x0c);
	sc233hgs_write_register(ViPipe, 0x301f, 0x9d);
	sc233hgs_write_register(ViPipe, 0x3044, 0x10);
	sc233hgs_write_register(ViPipe, 0x3062, 0x00);
	sc233hgs_write_register(ViPipe, 0x3202, 0x00);
	sc233hgs_write_register(ViPipe, 0x3203, 0x3c);
	sc233hgs_write_register(ViPipe, 0x3206, 0x04);
	sc233hgs_write_register(ViPipe, 0x3207, 0x83);
	sc233hgs_write_register(ViPipe, 0x320a, 0x04);
	sc233hgs_write_register(ViPipe, 0x320b, 0x38);
	sc233hgs_write_register(ViPipe, 0x320c, 0x04);
	sc233hgs_write_register(ViPipe, 0x320d, 0x65);
	sc233hgs_write_register(ViPipe, 0x320e, 0x05);
	sc233hgs_write_register(ViPipe, 0x320f, 0xc9);
	sc233hgs_write_register(ViPipe, 0x321f, 0x0b);
	sc233hgs_write_register(ViPipe, 0x322f, 0x00);
	sc233hgs_write_register(ViPipe, 0x3231, 0x00);
	sc233hgs_write_register(ViPipe, 0x3250, 0x03);
	sc233hgs_write_register(ViPipe, 0x32e2, 0x00);
	sc233hgs_write_register(ViPipe, 0x3301, 0x18);
	sc233hgs_write_register(ViPipe, 0x3304, 0x68);
	sc233hgs_write_register(ViPipe, 0x3306, 0x48);
	sc233hgs_write_register(ViPipe, 0x3309, 0x88);
	sc233hgs_write_register(ViPipe, 0x330b, 0xf0);
	sc233hgs_write_register(ViPipe, 0x330d, 0x30);
	sc233hgs_write_register(ViPipe, 0x3314, 0xc0);
	sc233hgs_write_register(ViPipe, 0x3315, 0x30);
	sc233hgs_write_register(ViPipe, 0x3317, 0x30);
	sc233hgs_write_register(ViPipe, 0x3318, 0x1f);
	sc233hgs_write_register(ViPipe, 0x331d, 0x32);
	sc233hgs_write_register(ViPipe, 0x331f, 0x02);
	sc233hgs_write_register(ViPipe, 0x3320, 0xc1);
	sc233hgs_write_register(ViPipe, 0x333b, 0x30);
	sc233hgs_write_register(ViPipe, 0x3352, 0x1a);
	sc233hgs_write_register(ViPipe, 0x3356, 0x1c);
	sc233hgs_write_register(ViPipe, 0x3363, 0x0f);
	sc233hgs_write_register(ViPipe, 0x3385, 0x59);
	sc233hgs_write_register(ViPipe, 0x3387, 0x79);
	sc233hgs_write_register(ViPipe, 0x33b0, 0x00);
	sc233hgs_write_register(ViPipe, 0x33ef, 0x04);
	sc233hgs_write_register(ViPipe, 0x33f8, 0x02);
	sc233hgs_write_register(ViPipe, 0x33fa, 0x02);
	sc233hgs_write_register(ViPipe, 0x341c, 0x06);
	sc233hgs_write_register(ViPipe, 0x341e, 0x24);
	sc233hgs_write_register(ViPipe, 0x341f, 0x27);
	sc233hgs_write_register(ViPipe, 0x34ad, 0x05);
	sc233hgs_write_register(ViPipe, 0x34af, 0x02);
	sc233hgs_write_register(ViPipe, 0x34de, 0x02);
	sc233hgs_write_register(ViPipe, 0x34f2, 0x00);
	sc233hgs_write_register(ViPipe, 0x3619, 0x20);
	sc233hgs_write_register(ViPipe, 0x361a, 0x90);
	sc233hgs_write_register(ViPipe, 0x3630, 0x80);
	sc233hgs_write_register(ViPipe, 0x3633, 0x44);
	sc233hgs_write_register(ViPipe, 0x3637, 0x3e);
	sc233hgs_write_register(ViPipe, 0x363a, 0x00);
	sc233hgs_write_register(ViPipe, 0x363b, 0x17);
	sc233hgs_write_register(ViPipe, 0x363c, 0x0f);
	sc233hgs_write_register(ViPipe, 0x363d, 0x1f);
	sc233hgs_write_register(ViPipe, 0x363e, 0xa8);
	sc233hgs_write_register(ViPipe, 0x363f, 0x86);
	sc233hgs_write_register(ViPipe, 0x3648, 0x4a);
	sc233hgs_write_register(ViPipe, 0x364a, 0x22);
	sc233hgs_write_register(ViPipe, 0x3660, 0x02);
	sc233hgs_write_register(ViPipe, 0x3661, 0x05);
	sc233hgs_write_register(ViPipe, 0x3662, 0x0b);
	sc233hgs_write_register(ViPipe, 0x3663, 0x17);
	sc233hgs_write_register(ViPipe, 0x3664, 0x17);
	sc233hgs_write_register(ViPipe, 0x3665, 0x08);
	sc233hgs_write_register(ViPipe, 0x3666, 0x18);
	sc233hgs_write_register(ViPipe, 0x3667, 0x38);
	sc233hgs_write_register(ViPipe, 0x3668, 0x78);
	sc233hgs_write_register(ViPipe, 0x3670, 0xf0);
	sc233hgs_write_register(ViPipe, 0x3671, 0xc8);
	sc233hgs_write_register(ViPipe, 0x3672, 0xa8);
	sc233hgs_write_register(ViPipe, 0x367c, 0x43);
	sc233hgs_write_register(ViPipe, 0x367d, 0x43);
	sc233hgs_write_register(ViPipe, 0x367e, 0x43);
	sc233hgs_write_register(ViPipe, 0x36c0, 0x08);
	sc233hgs_write_register(ViPipe, 0x36c1, 0x38);
	sc233hgs_write_register(ViPipe, 0x36c6, 0x08);
	sc233hgs_write_register(ViPipe, 0x36c7, 0x38);
	sc233hgs_write_register(ViPipe, 0x36ea, 0x0a);
	sc233hgs_write_register(ViPipe, 0x36eb, 0x0c);
	sc233hgs_write_register(ViPipe, 0x36ec, 0x43);
	sc233hgs_write_register(ViPipe, 0x36ed, 0x14);
	sc233hgs_write_register(ViPipe, 0x3718, 0x04);
	sc233hgs_write_register(ViPipe, 0x3719, 0x06);
	sc233hgs_write_register(ViPipe, 0x3722, 0x08);
	sc233hgs_write_register(ViPipe, 0x3728, 0xa0);
	sc233hgs_write_register(ViPipe, 0x372a, 0x10);
	sc233hgs_write_register(ViPipe, 0x3778, 0x08);
	sc233hgs_write_register(ViPipe, 0x3779, 0x18);
	sc233hgs_write_register(ViPipe, 0x377a, 0xd8);
	sc233hgs_write_register(ViPipe, 0x3794, 0x00);
	sc233hgs_write_register(ViPipe, 0x3795, 0x00);
	sc233hgs_write_register(ViPipe, 0x3796, 0x00);
	sc233hgs_write_register(ViPipe, 0x3798, 0x10);
	sc233hgs_write_register(ViPipe, 0x3799, 0x10);
	sc233hgs_write_register(ViPipe, 0x379a, 0x10);
	sc233hgs_write_register(ViPipe, 0x37c4, 0x08);
	sc233hgs_write_register(ViPipe, 0x37c5, 0x38);
	sc233hgs_write_register(ViPipe, 0x37d2, 0x08);
	sc233hgs_write_register(ViPipe, 0x37d3, 0x38);
	sc233hgs_write_register(ViPipe, 0x37d4, 0x08);
	sc233hgs_write_register(ViPipe, 0x37d5, 0x38);
	sc233hgs_write_register(ViPipe, 0x37fa, 0x04);
	sc233hgs_write_register(ViPipe, 0x37fb, 0xe5);
	sc233hgs_write_register(ViPipe, 0x37fc, 0x01);
	sc233hgs_write_register(ViPipe, 0x37fd, 0x34);
	sc233hgs_write_register(ViPipe, 0x3900, 0x1d);
	sc233hgs_write_register(ViPipe, 0x3901, 0x06);
	sc233hgs_write_register(ViPipe, 0x3903, 0x40);
	sc233hgs_write_register(ViPipe, 0x3904, 0x0a);
	sc233hgs_write_register(ViPipe, 0x3905, 0x4d);
	sc233hgs_write_register(ViPipe, 0x391f, 0x44);
	sc233hgs_write_register(ViPipe, 0x3933, 0x00);
	sc233hgs_write_register(ViPipe, 0x3934, 0x20);
	sc233hgs_write_register(ViPipe, 0x3935, 0x00);
	sc233hgs_write_register(ViPipe, 0x3936, 0x3c);
	sc233hgs_write_register(ViPipe, 0x3937, 0x7e);
	sc233hgs_write_register(ViPipe, 0x3938, 0x64);
	sc233hgs_write_register(ViPipe, 0x3939, 0x1f);
	sc233hgs_write_register(ViPipe, 0x393a, 0xaa);
	sc233hgs_write_register(ViPipe, 0x393b, 0x00);
	sc233hgs_write_register(ViPipe, 0x393c, 0x20);
	sc233hgs_write_register(ViPipe, 0x39c1, 0x6c);
	sc233hgs_write_register(ViPipe, 0x3c0a, 0x2a);
	sc233hgs_write_register(ViPipe, 0x3e00, 0x00);
	sc233hgs_write_register(ViPipe, 0x3e01, 0x5c);
	sc233hgs_write_register(ViPipe, 0x3e02, 0x30);
	sc233hgs_write_register(ViPipe, 0x3e03, 0x0b);
	sc233hgs_write_register(ViPipe, 0x3e08, 0x00);
	sc233hgs_write_register(ViPipe, 0x3e09, 0x40);
	sc233hgs_write_register(ViPipe, 0x3e15, 0x00);
	sc233hgs_write_register(ViPipe, 0x3e1c, 0x00);
	sc233hgs_write_register(ViPipe, 0x3e8e, 0x00);
	sc233hgs_write_register(ViPipe, 0x4330, 0x50);
	sc233hgs_write_register(ViPipe, 0x4331, 0x20);
	sc233hgs_write_register(ViPipe, 0x4333, 0x02);
	sc233hgs_write_register(ViPipe, 0x4360, 0x07);
	sc233hgs_write_register(ViPipe, 0x4362, 0xf0);
	sc233hgs_write_register(ViPipe, 0x4364, 0xf0);
	sc233hgs_write_register(ViPipe, 0x4365, 0x18);
	sc233hgs_write_register(ViPipe, 0x4366, 0x38);
	sc233hgs_write_register(ViPipe, 0x4368, 0x58);
	sc233hgs_write_register(ViPipe, 0x436a, 0x78);
	sc233hgs_write_register(ViPipe, 0x436b, 0x18);
	sc233hgs_write_register(ViPipe, 0x436c, 0x38);
	sc233hgs_write_register(ViPipe, 0x4370, 0x20);
	sc233hgs_write_register(ViPipe, 0x4371, 0x10);
	sc233hgs_write_register(ViPipe, 0x4372, 0x18);
	sc233hgs_write_register(ViPipe, 0x4373, 0x38);
	sc233hgs_write_register(ViPipe, 0x450d, 0x10);
	sc233hgs_write_register(ViPipe, 0x4837, 0x10);
	sc233hgs_write_register(ViPipe, 0x4b0a, 0x92);
	sc233hgs_write_register(ViPipe, 0x5000, 0x38);
	sc233hgs_write_register(ViPipe, 0x5002, 0x00);
	sc233hgs_write_register(ViPipe, 0x502e, 0x21);
	sc233hgs_write_register(ViPipe, 0x5034, 0x01);
	sc233hgs_write_register(ViPipe, 0x5104, 0x14);
	sc233hgs_write_register(ViPipe, 0x5105, 0x10);
	sc233hgs_write_register(ViPipe, 0x5106, 0x04);
	sc233hgs_write_register(ViPipe, 0x5107, 0x59);
	sc233hgs_write_register(ViPipe, 0x510d, 0x44);
	sc233hgs_write_register(ViPipe, 0x5302, 0x00);
	sc233hgs_write_register(ViPipe, 0x5787, 0x0a);
	sc233hgs_write_register(ViPipe, 0x5788, 0x0a);
	sc233hgs_write_register(ViPipe, 0x5789, 0x08);
	sc233hgs_write_register(ViPipe, 0x578a, 0x0a);
	sc233hgs_write_register(ViPipe, 0x578b, 0x0a);
	sc233hgs_write_register(ViPipe, 0x578c, 0x08);
	sc233hgs_write_register(ViPipe, 0x578d, 0x40);
	sc233hgs_write_register(ViPipe, 0x5790, 0x08);
	sc233hgs_write_register(ViPipe, 0x5791, 0x04);
	sc233hgs_write_register(ViPipe, 0x5792, 0x04);
	sc233hgs_write_register(ViPipe, 0x5793, 0x08);
	sc233hgs_write_register(ViPipe, 0x5794, 0x04);
	sc233hgs_write_register(ViPipe, 0x5795, 0x04);
	sc233hgs_write_register(ViPipe, 0x57aa, 0x2a);
	sc233hgs_write_register(ViPipe, 0x57ab, 0x7f);
	sc233hgs_write_register(ViPipe, 0x57ac, 0x00);
	sc233hgs_write_register(ViPipe, 0x57ad, 0x00);
	sc233hgs_write_register(ViPipe, 0x36e9, 0x00);
	sc233hgs_write_register(ViPipe, 0x37f9, 0x04);
	sc233hgs_write_register(ViPipe, 0x4412, 0x01);
	sc233hgs_write_register(ViPipe, 0x4402, 0x02);
	sc233hgs_write_register(ViPipe, 0x4403, 0x0a);
	sc233hgs_write_register(ViPipe, 0x4404, 0x1e);
	sc233hgs_write_register(ViPipe, 0x440c, 0x32);
	sc233hgs_write_register(ViPipe, 0x440d, 0x32);
	sc233hgs_write_register(ViPipe, 0x440e, 0x26);
	sc233hgs_write_register(ViPipe, 0x440f, 0x3f);
	sc233hgs_write_register(ViPipe, 0x4405, 0x28);
	sc233hgs_write_register(ViPipe, 0x4424, 0x01);
	sc233hgs_write_register(ViPipe, 0x4407, 0xa0);
	sc233hgs_write_register(ViPipe, 0x2100, 0x01);

	sc233hgs_default_reg_init(ViPipe);

	sc233hgs_write_register(ViPipe, 0x2100, 0x01);

	printf("ViPipe:%d,===sc233hgs 1080P 60fps 12bit LINE Init OK!===\n", ViPipe);
}

static void sc233hgs_wdr_1080p60_2to1_init(VI_PIPE ViPipe)
{
	sc233hgs_write_register(ViPipe, 0x2100, 0x00);
	sc233hgs_write_register(ViPipe, 0x36e9, 0x80);
	sc233hgs_write_register(ViPipe, 0x37f9, 0x80);
	sc233hgs_write_register(ViPipe, 0x300c, 0x24);
	sc233hgs_write_register(ViPipe, 0x3018, 0x3a);
	sc233hgs_write_register(ViPipe, 0x3019, 0x0c);
	sc233hgs_write_register(ViPipe, 0x301f, 0x9c);
	sc233hgs_write_register(ViPipe, 0x3044, 0x10);
	sc233hgs_write_register(ViPipe, 0x3062, 0x00);
	sc233hgs_write_register(ViPipe, 0x3202, 0x00);
	sc233hgs_write_register(ViPipe, 0x3203, 0x3c);
	sc233hgs_write_register(ViPipe, 0x3206, 0x04);
	sc233hgs_write_register(ViPipe, 0x3207, 0x83);
	sc233hgs_write_register(ViPipe, 0x320a, 0x04);
	sc233hgs_write_register(ViPipe, 0x320b, 0x38);
	sc233hgs_write_register(ViPipe, 0x320c, 0x04);
	sc233hgs_write_register(ViPipe, 0x320d, 0x65);
	sc233hgs_write_register(ViPipe, 0x320e, 0x05);
	sc233hgs_write_register(ViPipe, 0x320f, 0xc9);
	sc233hgs_write_register(ViPipe, 0x321f, 0x0b);
	sc233hgs_write_register(ViPipe, 0x322f, 0x00);
	sc233hgs_write_register(ViPipe, 0x3231, 0x00);
	sc233hgs_write_register(ViPipe, 0x3250, 0x03);
	sc233hgs_write_register(ViPipe, 0x3282, 0x03);
	sc233hgs_write_register(ViPipe, 0x32e2, 0x00);
	sc233hgs_write_register(ViPipe, 0x3301, 0x18);
	sc233hgs_write_register(ViPipe, 0x3304, 0x68);
	sc233hgs_write_register(ViPipe, 0x3306, 0x48);
	sc233hgs_write_register(ViPipe, 0x3309, 0x88);
	sc233hgs_write_register(ViPipe, 0x330b, 0xf0);
	sc233hgs_write_register(ViPipe, 0x330d, 0x30);
	sc233hgs_write_register(ViPipe, 0x3314, 0xc0);
	sc233hgs_write_register(ViPipe, 0x3315, 0x30);
	sc233hgs_write_register(ViPipe, 0x3317, 0x30);
	sc233hgs_write_register(ViPipe, 0x3318, 0x1f);
	sc233hgs_write_register(ViPipe, 0x331d, 0x32);
	sc233hgs_write_register(ViPipe, 0x331f, 0x02);
	sc233hgs_write_register(ViPipe, 0x3320, 0xc1);
	sc233hgs_write_register(ViPipe, 0x333b, 0x30);
	sc233hgs_write_register(ViPipe, 0x3352, 0x1a);
	sc233hgs_write_register(ViPipe, 0x3356, 0x1c);
	sc233hgs_write_register(ViPipe, 0x3363, 0x0f);
	sc233hgs_write_register(ViPipe, 0x3385, 0x59);
	sc233hgs_write_register(ViPipe, 0x3387, 0x79);
	sc233hgs_write_register(ViPipe, 0x33b0, 0x00);
	sc233hgs_write_register(ViPipe, 0x33ef, 0x04);
	sc233hgs_write_register(ViPipe, 0x33f8, 0x02);
	sc233hgs_write_register(ViPipe, 0x33fa, 0x02);
	sc233hgs_write_register(ViPipe, 0x341c, 0x06);
	sc233hgs_write_register(ViPipe, 0x341e, 0x24);
	sc233hgs_write_register(ViPipe, 0x341f, 0x27);
	sc233hgs_write_register(ViPipe, 0x34ad, 0x05);
	sc233hgs_write_register(ViPipe, 0x34af, 0x02);
	sc233hgs_write_register(ViPipe, 0x34de, 0x02);
	sc233hgs_write_register(ViPipe, 0x34f2, 0x00);
	sc233hgs_write_register(ViPipe, 0x3619, 0x20);
	sc233hgs_write_register(ViPipe, 0x361a, 0x90);
	sc233hgs_write_register(ViPipe, 0x3630, 0x80);
	sc233hgs_write_register(ViPipe, 0x3633, 0x44);
	sc233hgs_write_register(ViPipe, 0x3637, 0x3e);
	sc233hgs_write_register(ViPipe, 0x363a, 0x00);
	sc233hgs_write_register(ViPipe, 0x363b, 0x17);
	sc233hgs_write_register(ViPipe, 0x363c, 0x0f);
	sc233hgs_write_register(ViPipe, 0x363d, 0x1f);
	sc233hgs_write_register(ViPipe, 0x363e, 0xa8);
	sc233hgs_write_register(ViPipe, 0x363f, 0x86);
	sc233hgs_write_register(ViPipe, 0x3648, 0x4a);
	sc233hgs_write_register(ViPipe, 0x364a, 0x22);
	sc233hgs_write_register(ViPipe, 0x364e, 0x00);
	sc233hgs_write_register(ViPipe, 0x3660, 0x02);
	sc233hgs_write_register(ViPipe, 0x3661, 0x05);
	sc233hgs_write_register(ViPipe, 0x3662, 0x0b);
	sc233hgs_write_register(ViPipe, 0x3663, 0x17);
	sc233hgs_write_register(ViPipe, 0x3664, 0x17);
	sc233hgs_write_register(ViPipe, 0x3665, 0x08);
	sc233hgs_write_register(ViPipe, 0x3666, 0x18);
	sc233hgs_write_register(ViPipe, 0x3667, 0x38);
	sc233hgs_write_register(ViPipe, 0x3668, 0x78);
	sc233hgs_write_register(ViPipe, 0x3670, 0xf0);
	sc233hgs_write_register(ViPipe, 0x3671, 0xc8);
	sc233hgs_write_register(ViPipe, 0x3672, 0xa8);
	sc233hgs_write_register(ViPipe, 0x367c, 0x43);
	sc233hgs_write_register(ViPipe, 0x367d, 0x43);
	sc233hgs_write_register(ViPipe, 0x367e, 0x43);
	sc233hgs_write_register(ViPipe, 0x36c0, 0x08);
	sc233hgs_write_register(ViPipe, 0x36c1, 0x38);
	sc233hgs_write_register(ViPipe, 0x36c6, 0x08);
	sc233hgs_write_register(ViPipe, 0x36c7, 0x38);
	sc233hgs_write_register(ViPipe, 0x36ea, 0x0a);
	sc233hgs_write_register(ViPipe, 0x36eb, 0x0c);
	sc233hgs_write_register(ViPipe, 0x36ec, 0x43);
	sc233hgs_write_register(ViPipe, 0x36ed, 0x14);
	sc233hgs_write_register(ViPipe, 0x3718, 0x04);
	sc233hgs_write_register(ViPipe, 0x3719, 0x06);
	sc233hgs_write_register(ViPipe, 0x3722, 0x08);
	sc233hgs_write_register(ViPipe, 0x3727, 0x10);
	sc233hgs_write_register(ViPipe, 0x3728, 0xa0);
	sc233hgs_write_register(ViPipe, 0x372a, 0x10);
	sc233hgs_write_register(ViPipe, 0x3778, 0x08);
	sc233hgs_write_register(ViPipe, 0x3779, 0x18);
	sc233hgs_write_register(ViPipe, 0x377a, 0xd8);
	sc233hgs_write_register(ViPipe, 0x3794, 0x00);
	sc233hgs_write_register(ViPipe, 0x3795, 0x00);
	sc233hgs_write_register(ViPipe, 0x3796, 0x00);
	sc233hgs_write_register(ViPipe, 0x3798, 0x10);
	sc233hgs_write_register(ViPipe, 0x3799, 0x10);
	sc233hgs_write_register(ViPipe, 0x379a, 0x10);
	sc233hgs_write_register(ViPipe, 0x37c4, 0x08);
	sc233hgs_write_register(ViPipe, 0x37c5, 0x38);
	sc233hgs_write_register(ViPipe, 0x37d2, 0x08);
	sc233hgs_write_register(ViPipe, 0x37d3, 0x38);
	sc233hgs_write_register(ViPipe, 0x37d4, 0x08);
	sc233hgs_write_register(ViPipe, 0x37d5, 0x38);
	sc233hgs_write_register(ViPipe, 0x37fa, 0x04);
	sc233hgs_write_register(ViPipe, 0x37fb, 0xe5);
	sc233hgs_write_register(ViPipe, 0x37fc, 0x01);
	sc233hgs_write_register(ViPipe, 0x37fd, 0x34);
	sc233hgs_write_register(ViPipe, 0x3900, 0x1d);
	sc233hgs_write_register(ViPipe, 0x3901, 0x06);
	sc233hgs_write_register(ViPipe, 0x3903, 0x40);
	sc233hgs_write_register(ViPipe, 0x3904, 0x0a);
	sc233hgs_write_register(ViPipe, 0x3905, 0x4d);
	sc233hgs_write_register(ViPipe, 0x391f, 0x44);
	sc233hgs_write_register(ViPipe, 0x3933, 0x00);
	sc233hgs_write_register(ViPipe, 0x3934, 0x20);
	sc233hgs_write_register(ViPipe, 0x3935, 0x00);
	sc233hgs_write_register(ViPipe, 0x3936, 0x3c);
	sc233hgs_write_register(ViPipe, 0x3937, 0x7e);
	sc233hgs_write_register(ViPipe, 0x3938, 0x64);
	sc233hgs_write_register(ViPipe, 0x3939, 0x1f);
	sc233hgs_write_register(ViPipe, 0x393a, 0xaa);
	sc233hgs_write_register(ViPipe, 0x393b, 0x00);
	sc233hgs_write_register(ViPipe, 0x393c, 0x20);
	sc233hgs_write_register(ViPipe, 0x39c1, 0x6c);
	sc233hgs_write_register(ViPipe, 0x3c0a, 0x2a);
	sc233hgs_write_register(ViPipe, 0x3e00, 0x00);
	sc233hgs_write_register(ViPipe, 0x3e01, 0x59);
	sc233hgs_write_register(ViPipe, 0x3e02, 0x60);
	sc233hgs_write_register(ViPipe, 0x3e03, 0x0b);
	sc233hgs_write_register(ViPipe, 0x3e08, 0x00);
	sc233hgs_write_register(ViPipe, 0x3e09, 0x40);
	sc233hgs_write_register(ViPipe, 0x3e15, 0x00);
	sc233hgs_write_register(ViPipe, 0x3e1c, 0x00);
	sc233hgs_write_register(ViPipe, 0x3e31, 0x01);
	sc233hgs_write_register(ViPipe, 0x3e32, 0x60);
	sc233hgs_write_register(ViPipe, 0x3e8e, 0x00);
	sc233hgs_write_register(ViPipe, 0x4330, 0x50);
	sc233hgs_write_register(ViPipe, 0x4331, 0x20);
	sc233hgs_write_register(ViPipe, 0x4333, 0x02);
	sc233hgs_write_register(ViPipe, 0x4360, 0x07);
	sc233hgs_write_register(ViPipe, 0x4362, 0xf0);
	sc233hgs_write_register(ViPipe, 0x4364, 0xf0);
	sc233hgs_write_register(ViPipe, 0x4365, 0x18);
	sc233hgs_write_register(ViPipe, 0x4366, 0x38);
	sc233hgs_write_register(ViPipe, 0x4368, 0x58);
	sc233hgs_write_register(ViPipe, 0x436a, 0x78);
	sc233hgs_write_register(ViPipe, 0x436b, 0x18);
	sc233hgs_write_register(ViPipe, 0x436c, 0x38);
	sc233hgs_write_register(ViPipe, 0x4370, 0x20);
	sc233hgs_write_register(ViPipe, 0x4371, 0x10);
	sc233hgs_write_register(ViPipe, 0x4372, 0x18);
	sc233hgs_write_register(ViPipe, 0x4373, 0x38);
	sc233hgs_write_register(ViPipe, 0x450d, 0x10);
	sc233hgs_write_register(ViPipe, 0x4837, 0x10);
	sc233hgs_write_register(ViPipe, 0x4b0a, 0x92);
	sc233hgs_write_register(ViPipe, 0x5000, 0x38);
	sc233hgs_write_register(ViPipe, 0x5002, 0x00);
	sc233hgs_write_register(ViPipe, 0x502e, 0x21);
	sc233hgs_write_register(ViPipe, 0x5034, 0x01);
	sc233hgs_write_register(ViPipe, 0x5104, 0x14);
	sc233hgs_write_register(ViPipe, 0x5105, 0x10);
	sc233hgs_write_register(ViPipe, 0x5106, 0x04);
	sc233hgs_write_register(ViPipe, 0x5107, 0x59);
	sc233hgs_write_register(ViPipe, 0x510d, 0x44);
	sc233hgs_write_register(ViPipe, 0x5302, 0x00);
	sc233hgs_write_register(ViPipe, 0x5787, 0x0a);
	sc233hgs_write_register(ViPipe, 0x5788, 0x0a);
	sc233hgs_write_register(ViPipe, 0x5789, 0x08);
	sc233hgs_write_register(ViPipe, 0x578a, 0x0a);
	sc233hgs_write_register(ViPipe, 0x578b, 0x0a);
	sc233hgs_write_register(ViPipe, 0x578c, 0x08);
	sc233hgs_write_register(ViPipe, 0x578d, 0x40);
	sc233hgs_write_register(ViPipe, 0x5790, 0x08);
	sc233hgs_write_register(ViPipe, 0x5791, 0x04);
	sc233hgs_write_register(ViPipe, 0x5792, 0x04);
	sc233hgs_write_register(ViPipe, 0x5793, 0x08);
	sc233hgs_write_register(ViPipe, 0x5794, 0x04);
	sc233hgs_write_register(ViPipe, 0x5795, 0x04);
	sc233hgs_write_register(ViPipe, 0x57aa, 0x2a);
	sc233hgs_write_register(ViPipe, 0x57ab, 0x7f);
	sc233hgs_write_register(ViPipe, 0x57ac, 0x00);
	sc233hgs_write_register(ViPipe, 0x57ad, 0x00);
	sc233hgs_write_register(ViPipe, 0x36e9, 0x00);
	sc233hgs_write_register(ViPipe, 0x37f9, 0x04);
	sc233hgs_write_register(ViPipe, 0x4412, 0x01);
	sc233hgs_write_register(ViPipe, 0x4402, 0x02);
	sc233hgs_write_register(ViPipe, 0x4403, 0x0a);
	sc233hgs_write_register(ViPipe, 0x4404, 0x1e);
	sc233hgs_write_register(ViPipe, 0x440c, 0x32);
	sc233hgs_write_register(ViPipe, 0x440d, 0x32);
	sc233hgs_write_register(ViPipe, 0x440e, 0x26);
	sc233hgs_write_register(ViPipe, 0x440f, 0x3f);
	sc233hgs_write_register(ViPipe, 0x4405, 0x28);
	sc233hgs_write_register(ViPipe, 0x4424, 0x01);
	sc233hgs_write_register(ViPipe, 0x4407, 0xa0);
	sc233hgs_write_register(ViPipe, 0x2100, 0x01);

	// sc233hgs_default_reg_init(ViPipe);

	sc233hgs_write_register(ViPipe, 0x2100, 0x01);

	delay_ms(50);

	printf("===sc233hgs sensor 1080P60fps 12bit 2to1 WDR(60fps->30fps) init success!=====\n");
}
