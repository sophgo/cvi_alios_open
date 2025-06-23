#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx415_cmos_ex.h"
#include "sensor_i2c.h"

/* Chip ID */
#define IMX415_CHIP_ID_ADDR_H	0x3f13
#define IMX415_CHIP_ID_ADDR_L	0x3f12
#define IMX415_CHIP_ID			0x0514

static void imx415_linear_2m60_init(VI_PIPE ViPipe);
static void imx415_linear_5m25_init(VI_PIPE ViPipe);
static void imx415_linear_8m25_init(VI_PIPE ViPipe);
static void imx415_linear_8m30_init(VI_PIPE ViPipe);
static void imx415_linear_4m25_init(VI_PIPE ViPipe);
static void imx415_wdr_4m25_2to1_init(VI_PIPE ViPipe);

const CVI_U32 imx415_addr_byte = 2;
const CVI_U32 imx415_data_byte = 1;
int imx415_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx415_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx415_AddrInfo[ViPipe].s8I2cAddr);
}

int imx415_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx415_BusInfo[ViPipe].s8I2cDev);
}

int imx415_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx415_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx415_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx415_addr_byte, imx415_data_byte);
}

int imx415_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx415_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx415_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx415_addr_byte, (CVI_U32)data, imx415_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx415_standby(VI_PIPE ViPipe)
{
	(void)(ViPipe);
}

void imx415_restart(VI_PIPE ViPipe)
{
	(void)(ViPipe);
	delay_ms(20);
}

void imx415_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx415[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx415_write_register(ViPipe,
				g_pastImx415[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx415[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx415_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 u8Filp = 0;
	CVI_U8 u8Mirror = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		u8Mirror = 1;
		break;
	case ISP_SNS_FLIP:
		u8Filp = 1;
		break;
	case ISP_SNS_MIRROR_FLIP:
		u8Filp = 1;
		u8Mirror = 1;
		break;
	default:
		return;
	}

	imx415_write_register(ViPipe, 0x3030, u8Mirror | (u8Filp << 1));
}

int imx415_probe(VI_PIPE ViPipe)
{
	int val1, val2;
	int read_data = 0;
	int i;

	if (imx415_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	imx415_write_register(ViPipe, 0x3000, 0x00);/* STANDBY */
	delay_ms(20);
	for (i = 0; i < 3; i++) {
		val1 = imx415_read_register(ViPipe, IMX415_CHIP_ID_ADDR_H);
		val1 &= 0x0f;
		val2 = imx415_read_register(ViPipe, IMX415_CHIP_ID_ADDR_L);
		read_data = ((val1 & 0xFF) << 8) | (val2 & 0xFF);

		if (read_data != IMX415_CHIP_ID) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "loop(%d), chip id(0x%x) mismatch, read(0x%x)\n",
				i, IMX415_CHIP_ID, read_data);
			delay_ms(20);
		} else {
			return CVI_SUCCESS;
		}
	}

	return CVI_FAILURE;
}

void imx415_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx415[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx415[ViPipe]->u8ImgMode;

	imx415_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == IMX415_MODE_4M25_WDR)
			imx415_wdr_4m25_2to1_init(ViPipe);
		else{
		}
	} else {
		if (u8ImgMode == IMX415_MODE_4M25)
			imx415_linear_4m25_init(ViPipe);
		else if (u8ImgMode == IMX415_MODE_8M30)
			imx415_linear_8m30_init(ViPipe);
		else if (u8ImgMode == IMX415_MODE_8M25)
			imx415_linear_8m25_init(ViPipe);
		else if (u8ImgMode == IMX415_MODE_5M25)
			imx415_linear_5m25_init(ViPipe);
		else if (u8ImgMode == IMX415_MODE_2M60)
			imx415_linear_2m60_init(ViPipe);
		else {
		}
	}
	g_pastImx415[ViPipe]->bInit = CVI_TRUE;
}

// void imx415_exit(VI_PIPE ViPipe)
// {
// 	imx415_i2c_exit(ViPipe);
// }

static void imx415_linear_4m25_init(VI_PIPE ViPipe)
{
	// 37.125M 2568x1440@25
	delay_ms(4);
	imx415_write_register(ViPipe, 0x3008, 0x7F);
	imx415_write_register(ViPipe, 0x300A, 0x5B);
	imx415_write_register(ViPipe, 0x3024, 0x8C);
	imx415_write_register(ViPipe, 0x3025, 0x0A);
	imx415_write_register(ViPipe, 0x3028, 0x4C);
	imx415_write_register(ViPipe, 0x3029, 0x04);
	imx415_write_register(ViPipe, 0x3030, 0x01);
	imx415_write_register(ViPipe, 0x3033, 0x06);
	imx415_write_register(ViPipe, 0x3050, 0x08);
	imx415_write_register(ViPipe, 0x30C1, 0x00);
	imx415_write_register(ViPipe, 0x3116, 0x24);
	imx415_write_register(ViPipe, 0x3118, 0x80);
	imx415_write_register(ViPipe, 0x311E, 0x24);
	imx415_write_register(ViPipe, 0x32D4, 0x21);
	imx415_write_register(ViPipe, 0x32EC, 0xA1);
	imx415_write_register(ViPipe, 0x344C, 0x2B);
	imx415_write_register(ViPipe, 0x344D, 0x01);
	imx415_write_register(ViPipe, 0x344E, 0xED);
	imx415_write_register(ViPipe, 0x344F, 0x01);
	imx415_write_register(ViPipe, 0x3450, 0xF6);
	imx415_write_register(ViPipe, 0x3451, 0x02);
	imx415_write_register(ViPipe, 0x3452, 0x7F);
	imx415_write_register(ViPipe, 0x3453, 0x03);
	imx415_write_register(ViPipe, 0x358A, 0x04);
	imx415_write_register(ViPipe, 0x35A1, 0x02);
	imx415_write_register(ViPipe, 0x35EC, 0x27);
	imx415_write_register(ViPipe, 0x35EE, 0x8D);
	imx415_write_register(ViPipe, 0x35F0, 0x8D);
	imx415_write_register(ViPipe, 0x35F2, 0x29);
	imx415_write_register(ViPipe, 0x36BC, 0x0C);
	imx415_write_register(ViPipe, 0x36CC, 0x53);
	imx415_write_register(ViPipe, 0x36CD, 0x00);
	imx415_write_register(ViPipe, 0x36CE, 0x3C);
	imx415_write_register(ViPipe, 0x36D0, 0x8C);
	imx415_write_register(ViPipe, 0x36D1, 0x00);
	imx415_write_register(ViPipe, 0x36D2, 0x71);
	imx415_write_register(ViPipe, 0x36D4, 0x3C);
	imx415_write_register(ViPipe, 0x36D6, 0x53);
	imx415_write_register(ViPipe, 0x36D7, 0x00);
	imx415_write_register(ViPipe, 0x36D8, 0x71);
	imx415_write_register(ViPipe, 0x36DA, 0x8C);
	imx415_write_register(ViPipe, 0x36DB, 0x00);
	imx415_write_register(ViPipe, 0x3720, 0x00);
	imx415_write_register(ViPipe, 0x3724, 0x02);
	imx415_write_register(ViPipe, 0x3726, 0x02);
	imx415_write_register(ViPipe, 0x3732, 0x02);
	imx415_write_register(ViPipe, 0x3734, 0x03);
	imx415_write_register(ViPipe, 0x3736, 0x03);
	imx415_write_register(ViPipe, 0x3742, 0x03);
	imx415_write_register(ViPipe, 0x3862, 0xE0);
	imx415_write_register(ViPipe, 0x38CC, 0x30);
	imx415_write_register(ViPipe, 0x38CD, 0x2F);
	imx415_write_register(ViPipe, 0x395C, 0x0C);
	imx415_write_register(ViPipe, 0x39A4, 0x07);
	imx415_write_register(ViPipe, 0x39A8, 0x32);
	imx415_write_register(ViPipe, 0x39AA, 0x32);
	imx415_write_register(ViPipe, 0x39AC, 0x32);
	imx415_write_register(ViPipe, 0x39AE, 0x32);
	imx415_write_register(ViPipe, 0x39B0, 0x32);
	imx415_write_register(ViPipe, 0x39B2, 0x2F);
	imx415_write_register(ViPipe, 0x39B4, 0x2D);
	imx415_write_register(ViPipe, 0x39B6, 0x28);
	imx415_write_register(ViPipe, 0x39B8, 0x30);
	imx415_write_register(ViPipe, 0x39BA, 0x30);
	imx415_write_register(ViPipe, 0x39BC, 0x30);
	imx415_write_register(ViPipe, 0x39BE, 0x30);
	imx415_write_register(ViPipe, 0x39C0, 0x30);
	imx415_write_register(ViPipe, 0x39C2, 0x2E);
	imx415_write_register(ViPipe, 0x39C4, 0x2B);
	imx415_write_register(ViPipe, 0x39C6, 0x25);
	imx415_write_register(ViPipe, 0x3A42, 0xD1);
	imx415_write_register(ViPipe, 0x3A4C, 0x77);
	imx415_write_register(ViPipe, 0x3AE0, 0x02);
	imx415_write_register(ViPipe, 0x3AEC, 0x0C);
	imx415_write_register(ViPipe, 0x3B00, 0x2E);
	imx415_write_register(ViPipe, 0x3B06, 0x29);
	imx415_write_register(ViPipe, 0x3B98, 0x25);
	imx415_write_register(ViPipe, 0x3B99, 0x21);
	imx415_write_register(ViPipe, 0x3B9B, 0x13);
	imx415_write_register(ViPipe, 0x3B9C, 0x13);
	imx415_write_register(ViPipe, 0x3B9D, 0x13);
	imx415_write_register(ViPipe, 0x3B9E, 0x13);
	imx415_write_register(ViPipe, 0x3BA1, 0x00);
	imx415_write_register(ViPipe, 0x3BA2, 0x06);
	imx415_write_register(ViPipe, 0x3BA3, 0x0B);
	imx415_write_register(ViPipe, 0x3BA4, 0x10);
	imx415_write_register(ViPipe, 0x3BA5, 0x14);
	imx415_write_register(ViPipe, 0x3BA6, 0x18);
	imx415_write_register(ViPipe, 0x3BA7, 0x1A);
	imx415_write_register(ViPipe, 0x3BA8, 0x1A);
	imx415_write_register(ViPipe, 0x3BA9, 0x1A);
	imx415_write_register(ViPipe, 0x3BAC, 0xED);
	imx415_write_register(ViPipe, 0x3BAD, 0x01);
	imx415_write_register(ViPipe, 0x3BAE, 0xF6);
	imx415_write_register(ViPipe, 0x3BAF, 0x02);
	imx415_write_register(ViPipe, 0x3BB0, 0xA2);
	imx415_write_register(ViPipe, 0x3BB1, 0x03);
	imx415_write_register(ViPipe, 0x3BB2, 0xE0);
	imx415_write_register(ViPipe, 0x3BB3, 0x03);
	imx415_write_register(ViPipe, 0x3BB4, 0xE0);
	imx415_write_register(ViPipe, 0x3BB5, 0x03);
	imx415_write_register(ViPipe, 0x3BB6, 0xE0);
	imx415_write_register(ViPipe, 0x3BB7, 0x03);
	imx415_write_register(ViPipe, 0x3BB8, 0xE0);
	imx415_write_register(ViPipe, 0x3BBA, 0xE0);
	imx415_write_register(ViPipe, 0x3BBC, 0xDA);
	imx415_write_register(ViPipe, 0x3BBE, 0x88);
	imx415_write_register(ViPipe, 0x3BC0, 0x44);
	imx415_write_register(ViPipe, 0x3BC2, 0x7B);
	imx415_write_register(ViPipe, 0x3BC4, 0xA2);
	imx415_write_register(ViPipe, 0x3BC8, 0xBD);
	imx415_write_register(ViPipe, 0x3BCA, 0xBD);
	imx415_write_register(ViPipe, 0x4004, 0x48);
	imx415_write_register(ViPipe, 0x4005, 0x09);
	imx415_write_register(ViPipe, 0x400C, 0x00);
	imx415_write_register(ViPipe, 0x4018, 0x8F);
	imx415_write_register(ViPipe, 0x401A, 0x4F);
	imx415_write_register(ViPipe, 0x401C, 0x47);
	imx415_write_register(ViPipe, 0x401E, 0x37);
	imx415_write_register(ViPipe, 0x4020, 0x4F);
	imx415_write_register(ViPipe, 0x4022, 0x87);
	imx415_write_register(ViPipe, 0x4024, 0x4F);
	imx415_write_register(ViPipe, 0x4026, 0x7F);
	imx415_write_register(ViPipe, 0x4028, 0x3F);
	imx415_write_register(ViPipe, 0x3000, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x00);
	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===IMX415 4M25 Init success!===\n", ViPipe);
}

static void imx415_wdr_4m25_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(4);

	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "===Imx415 sensor 4M25 2to1 WDR init success!===\n");
}

static void imx415_linear_8m25_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	// 37.125M 3864x2192@25
	imx415_write_register(ViPipe, 0x3000, 0x01);
	imx415_write_register(ViPipe, 0x3002, 0x01);
	imx415_write_register(ViPipe, 0x3008, 0x7F);
	imx415_write_register(ViPipe, 0x300A, 0x5B);
	imx415_write_register(ViPipe, 0x3024, 0x8C);
	imx415_write_register(ViPipe, 0x3025, 0x0A);
	imx415_write_register(ViPipe, 0x3028, 0x4C);
	imx415_write_register(ViPipe, 0x3029, 0x04);
	imx415_write_register(ViPipe, 0x3030, 0x01);
	imx415_write_register(ViPipe, 0x3033, 0x06);
	imx415_write_register(ViPipe, 0x3050, 0x08);
	imx415_write_register(ViPipe, 0x30C1, 0x00);
	imx415_write_register(ViPipe, 0x3116, 0x24);
	imx415_write_register(ViPipe, 0x3118, 0x80);
	imx415_write_register(ViPipe, 0x311E, 0x24);
	imx415_write_register(ViPipe, 0x32D4, 0x21);
	imx415_write_register(ViPipe, 0x32EC, 0xA1);
	imx415_write_register(ViPipe, 0x344C, 0x2B);
	imx415_write_register(ViPipe, 0x344D, 0x01);
	imx415_write_register(ViPipe, 0x344E, 0xED);
	imx415_write_register(ViPipe, 0x344F, 0x01);
	imx415_write_register(ViPipe, 0x3450, 0xF6);
	imx415_write_register(ViPipe, 0x3451, 0x02);
	imx415_write_register(ViPipe, 0x3452, 0x7F);
	imx415_write_register(ViPipe, 0x3453, 0x03);
	imx415_write_register(ViPipe, 0x358A, 0x04);
	imx415_write_register(ViPipe, 0x35A1, 0x02);
	imx415_write_register(ViPipe, 0x35EC, 0x27);
	imx415_write_register(ViPipe, 0x35EE, 0x8D);
	imx415_write_register(ViPipe, 0x35F0, 0x8D);
	imx415_write_register(ViPipe, 0x35F2, 0x29);
	imx415_write_register(ViPipe, 0x36BC, 0x0C);
	imx415_write_register(ViPipe, 0x36CC, 0x53);
	imx415_write_register(ViPipe, 0x36CD, 0x00);
	imx415_write_register(ViPipe, 0x36CE, 0x3C);
	imx415_write_register(ViPipe, 0x36D0, 0x8C);
	imx415_write_register(ViPipe, 0x36D1, 0x00);
	imx415_write_register(ViPipe, 0x36D2, 0x71);
	imx415_write_register(ViPipe, 0x36D4, 0x3C);
	imx415_write_register(ViPipe, 0x36D6, 0x53);
	imx415_write_register(ViPipe, 0x36D7, 0x00);
	imx415_write_register(ViPipe, 0x36D8, 0x71);
	imx415_write_register(ViPipe, 0x36DA, 0x8C);
	imx415_write_register(ViPipe, 0x36DB, 0x00);
	imx415_write_register(ViPipe, 0x3720, 0x00);
	imx415_write_register(ViPipe, 0x3724, 0x02);
	imx415_write_register(ViPipe, 0x3726, 0x02);
	imx415_write_register(ViPipe, 0x3732, 0x02);
	imx415_write_register(ViPipe, 0x3734, 0x03);
	imx415_write_register(ViPipe, 0x3736, 0x03);
	imx415_write_register(ViPipe, 0x3742, 0x03);
	imx415_write_register(ViPipe, 0x3862, 0xE0);
	imx415_write_register(ViPipe, 0x38CC, 0x30);
	imx415_write_register(ViPipe, 0x38CD, 0x2F);
	imx415_write_register(ViPipe, 0x395C, 0x0C);
	imx415_write_register(ViPipe, 0x39A4, 0x07);
	imx415_write_register(ViPipe, 0x39A8, 0x32);
	imx415_write_register(ViPipe, 0x39AA, 0x32);
	imx415_write_register(ViPipe, 0x39AC, 0x32);
	imx415_write_register(ViPipe, 0x39AE, 0x32);
	imx415_write_register(ViPipe, 0x39B0, 0x32);
	imx415_write_register(ViPipe, 0x39B2, 0x2F);
	imx415_write_register(ViPipe, 0x39B4, 0x2D);
	imx415_write_register(ViPipe, 0x39B6, 0x28);
	imx415_write_register(ViPipe, 0x39B8, 0x30);
	imx415_write_register(ViPipe, 0x39BA, 0x30);
	imx415_write_register(ViPipe, 0x39BC, 0x30);
	imx415_write_register(ViPipe, 0x39BE, 0x30);
	imx415_write_register(ViPipe, 0x39C0, 0x30);
	imx415_write_register(ViPipe, 0x39C2, 0x2E);
	imx415_write_register(ViPipe, 0x39C4, 0x2B);
	imx415_write_register(ViPipe, 0x39C6, 0x25);
	imx415_write_register(ViPipe, 0x3A42, 0xD1);
	imx415_write_register(ViPipe, 0x3A4C, 0x77);
	imx415_write_register(ViPipe, 0x3AE0, 0x02);
	imx415_write_register(ViPipe, 0x3AEC, 0x0C);
	imx415_write_register(ViPipe, 0x3B00, 0x2E);
	imx415_write_register(ViPipe, 0x3B06, 0x29);
	imx415_write_register(ViPipe, 0x3B98, 0x25);
	imx415_write_register(ViPipe, 0x3B99, 0x21);
	imx415_write_register(ViPipe, 0x3B9B, 0x13);
	imx415_write_register(ViPipe, 0x3B9C, 0x13);
	imx415_write_register(ViPipe, 0x3B9D, 0x13);
	imx415_write_register(ViPipe, 0x3B9E, 0x13);
	imx415_write_register(ViPipe, 0x3BA1, 0x00);
	imx415_write_register(ViPipe, 0x3BA2, 0x06);
	imx415_write_register(ViPipe, 0x3BA3, 0x0B);
	imx415_write_register(ViPipe, 0x3BA4, 0x10);
	imx415_write_register(ViPipe, 0x3BA5, 0x14);
	imx415_write_register(ViPipe, 0x3BA6, 0x18);
	imx415_write_register(ViPipe, 0x3BA7, 0x1A);
	imx415_write_register(ViPipe, 0x3BA8, 0x1A);
	imx415_write_register(ViPipe, 0x3BA9, 0x1A);
	imx415_write_register(ViPipe, 0x3BAC, 0xED);
	imx415_write_register(ViPipe, 0x3BAD, 0x01);
	imx415_write_register(ViPipe, 0x3BAE, 0xF6);
	imx415_write_register(ViPipe, 0x3BAF, 0x02);
	imx415_write_register(ViPipe, 0x3BB0, 0xA2);
	imx415_write_register(ViPipe, 0x3BB1, 0x03);
	imx415_write_register(ViPipe, 0x3BB2, 0xE0);
	imx415_write_register(ViPipe, 0x3BB3, 0x03);
	imx415_write_register(ViPipe, 0x3BB4, 0xE0);
	imx415_write_register(ViPipe, 0x3BB5, 0x03);
	imx415_write_register(ViPipe, 0x3BB6, 0xE0);
	imx415_write_register(ViPipe, 0x3BB7, 0x03);
	imx415_write_register(ViPipe, 0x3BB8, 0xE0);
	imx415_write_register(ViPipe, 0x3BBA, 0xE0);
	imx415_write_register(ViPipe, 0x3BBC, 0xDA);
	imx415_write_register(ViPipe, 0x3BBE, 0x88);
	imx415_write_register(ViPipe, 0x3BC0, 0x44);
	imx415_write_register(ViPipe, 0x3BC2, 0x7B);
	imx415_write_register(ViPipe, 0x3BC4, 0xA2);
	imx415_write_register(ViPipe, 0x3BC8, 0xBD);
	imx415_write_register(ViPipe, 0x3BCA, 0xBD);
	imx415_write_register(ViPipe, 0x4004, 0x48);
	imx415_write_register(ViPipe, 0x4005, 0x09);
	imx415_write_register(ViPipe, 0x400C, 0x00);
	imx415_write_register(ViPipe, 0x4018, 0x8F);
	imx415_write_register(ViPipe, 0x401A, 0x4F);
	imx415_write_register(ViPipe, 0x401C, 0x47);
	imx415_write_register(ViPipe, 0x401E, 0x37);
	imx415_write_register(ViPipe, 0x4020, 0x4F);
	imx415_write_register(ViPipe, 0x4022, 0x87);
	imx415_write_register(ViPipe, 0x4024, 0x4F);
	imx415_write_register(ViPipe, 0x4026, 0x7F);
	imx415_write_register(ViPipe, 0x4028, 0x3F);
	imx415_write_register(ViPipe, 0x3000, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x00);


	// 24M
	// imx415_write_register(ViPipe, 0x3000, 0x01);
	// imx415_write_register(ViPipe, 0x3001, 0x00);
	// imx415_write_register(ViPipe, 0x3002, 0x01);
	// imx415_write_register(ViPipe, 0x3003, 0x00);
	// imx415_write_register(ViPipe, 0x3008, 0x54);
	// imx415_write_register(ViPipe, 0x3009, 0x00);
	// imx415_write_register(ViPipe, 0x300A, 0x3B);
	// imx415_write_register(ViPipe, 0x300B, 0xA0);
	// imx415_write_register(ViPipe, 0x300C, 0x03);
	// imx415_write_register(ViPipe, 0x301C, 0x00);
	// imx415_write_register(ViPipe, 0x301D, 0x08);
	// imx415_write_register(ViPipe, 0x3020, 0x00);
	// imx415_write_register(ViPipe, 0x3021, 0x00);
	// imx415_write_register(ViPipe, 0x3022, 0x00);
	// imx415_write_register(ViPipe, 0x3023, 0x01);
	// imx415_write_register(ViPipe, 0x3024, 0x8E); // 25fps
	// imx415_write_register(ViPipe, 0x3025, 0x0A);
	// imx415_write_register(ViPipe, 0x3026, 0x00);
	// imx415_write_register(ViPipe, 0x3028, 0x2A);
	// imx415_write_register(ViPipe, 0x3029, 0x04);
	// imx415_write_register(ViPipe, 0x302C, 0x00);
	// imx415_write_register(ViPipe, 0x302D, 0x00);
	// imx415_write_register(ViPipe, 0x3030, 0x00);
	// imx415_write_register(ViPipe, 0x3031, 0x01);
	// imx415_write_register(ViPipe, 0x3032, 0x01);
	// imx415_write_register(ViPipe, 0x3033, 0x08);
	// imx415_write_register(ViPipe, 0x3040, 0x00);
	// imx415_write_register(ViPipe, 0x3041, 0x00);
	// imx415_write_register(ViPipe, 0x3042, 0x18);
	// imx415_write_register(ViPipe, 0x3043, 0x0F);
	// imx415_write_register(ViPipe, 0x3044, 0x00);
	// imx415_write_register(ViPipe, 0x3045, 0x00);
	// imx415_write_register(ViPipe, 0x3046, 0x20);
	// imx415_write_register(ViPipe, 0x3047, 0x11);
	// imx415_write_register(ViPipe, 0x3050, 0x08);
	// imx415_write_register(ViPipe, 0x3051, 0x00);
	// imx415_write_register(ViPipe, 0x3052, 0x00);
	// imx415_write_register(ViPipe, 0x3054, 0x19);
	// imx415_write_register(ViPipe, 0x3055, 0x00);
	// imx415_write_register(ViPipe, 0x3056, 0x00);
	// imx415_write_register(ViPipe, 0x3058, 0x3E);
	// imx415_write_register(ViPipe, 0x3059, 0x00);
	// imx415_write_register(ViPipe, 0x305A, 0x00);
	// imx415_write_register(ViPipe, 0x305C, 0x66);
	// imx415_write_register(ViPipe, 0x305D, 0x00);
	// imx415_write_register(ViPipe, 0x305E, 0x00);
	// imx415_write_register(ViPipe, 0x3060, 0x25);
	// imx415_write_register(ViPipe, 0x3061, 0x00);
	// imx415_write_register(ViPipe, 0x3062, 0x00);
	// imx415_write_register(ViPipe, 0x3064, 0x4A);
	// imx415_write_register(ViPipe, 0x3065, 0x00);
	// imx415_write_register(ViPipe, 0x3066, 0x00);
	// imx415_write_register(ViPipe, 0x3090, 0x00);
	// imx415_write_register(ViPipe, 0x3091, 0x00);
	// imx415_write_register(ViPipe, 0x3092, 0x00);
	// imx415_write_register(ViPipe, 0x3093, 0x00);
	// imx415_write_register(ViPipe, 0x3094, 0x00);
	// imx415_write_register(ViPipe, 0x3095, 0x00);
	// imx415_write_register(ViPipe, 0x3096, 0x00);
	// imx415_write_register(ViPipe, 0x3097, 0x00);
	// imx415_write_register(ViPipe, 0x30C0, 0x2A);
	// imx415_write_register(ViPipe, 0x30C1, 0x00);
	// imx415_write_register(ViPipe, 0x30CC, 0x00);
	// imx415_write_register(ViPipe, 0x30CD, 0x00);
	// imx415_write_register(ViPipe, 0x30CF, 0x00);
	// imx415_write_register(ViPipe, 0x30D9, 0x06);
	// imx415_write_register(ViPipe, 0x30DA, 0x02);
	// imx415_write_register(ViPipe, 0x30E2, 0x32);
	// imx415_write_register(ViPipe, 0x30E3, 0x00);
	// imx415_write_register(ViPipe, 0x3115, 0x00);
	// imx415_write_register(ViPipe, 0x3116, 0x23);
	// imx415_write_register(ViPipe, 0x3118, 0xB4);
	// imx415_write_register(ViPipe, 0x3119, 0x00);
	// imx415_write_register(ViPipe, 0x311A, 0xFC);
	// imx415_write_register(ViPipe, 0x311B, 0x00);
	// imx415_write_register(ViPipe, 0x311E, 0x23);
	// imx415_write_register(ViPipe, 0x3260, 0x01);
	// imx415_write_register(ViPipe, 0x32C8, 0x01);
	// imx415_write_register(ViPipe, 0x32D4, 0x21);
	// imx415_write_register(ViPipe, 0x32EC, 0xA1);
	// imx415_write_register(ViPipe, 0x344C, 0x2B);
	// imx415_write_register(ViPipe, 0x344D, 0x01);
	// imx415_write_register(ViPipe, 0x344E, 0xED);
	// imx415_write_register(ViPipe, 0x344F, 0x01);
	// imx415_write_register(ViPipe, 0x3450, 0xF6);
	// imx415_write_register(ViPipe, 0x3451, 0x02);
	// imx415_write_register(ViPipe, 0x3452, 0x7F);
	// imx415_write_register(ViPipe, 0x3453, 0x03);
	// imx415_write_register(ViPipe, 0x358A, 0x04);
	// imx415_write_register(ViPipe, 0x35A1, 0x02);
	// imx415_write_register(ViPipe, 0x35EC, 0x27);
	// imx415_write_register(ViPipe, 0x35EE, 0x8D);
	// imx415_write_register(ViPipe, 0x35F0, 0x8D);
	// imx415_write_register(ViPipe, 0x35F2, 0x29);
	// imx415_write_register(ViPipe, 0x36BC, 0x0C);
	// imx415_write_register(ViPipe, 0x36CC, 0x53);
	// imx415_write_register(ViPipe, 0x36CD, 0x00);
	// imx415_write_register(ViPipe, 0x36CE, 0x3C);
	// imx415_write_register(ViPipe, 0x36D0, 0x8C);
	// imx415_write_register(ViPipe, 0x36D1, 0x00);
	// imx415_write_register(ViPipe, 0x36D2, 0x71);
	// imx415_write_register(ViPipe, 0x36D4, 0x3C);
	// imx415_write_register(ViPipe, 0x36D6, 0x53);
	// imx415_write_register(ViPipe, 0x36D7, 0x00);
	// imx415_write_register(ViPipe, 0x36D8, 0x71);
	// imx415_write_register(ViPipe, 0x36DA, 0x8C);
	// imx415_write_register(ViPipe, 0x36DB, 0x00);
	// imx415_write_register(ViPipe, 0x3701, 0x03);
	// imx415_write_register(ViPipe, 0x3720, 0x00);
	// imx415_write_register(ViPipe, 0x3724, 0x02);
	// imx415_write_register(ViPipe, 0x3726, 0x02);
	// imx415_write_register(ViPipe, 0x3732, 0x02);
	// imx415_write_register(ViPipe, 0x3734, 0x03);
	// imx415_write_register(ViPipe, 0x3736, 0x03);
	// imx415_write_register(ViPipe, 0x3742, 0x03);
	// imx415_write_register(ViPipe, 0x3862, 0xE0);
	// imx415_write_register(ViPipe, 0x38CC, 0x30);
	// imx415_write_register(ViPipe, 0x38CD, 0x2F);
	// imx415_write_register(ViPipe, 0x395C, 0x0C);
	// imx415_write_register(ViPipe, 0x39A4, 0x07);
	// imx415_write_register(ViPipe, 0x39A8, 0x32);
	// imx415_write_register(ViPipe, 0x39AA, 0x32);
	// imx415_write_register(ViPipe, 0x39AC, 0x32);
	// imx415_write_register(ViPipe, 0x39AE, 0x32);
	// imx415_write_register(ViPipe, 0x39B0, 0x32);
	// imx415_write_register(ViPipe, 0x39B2, 0x2F);
	// imx415_write_register(ViPipe, 0x39B4, 0x2D);
	// imx415_write_register(ViPipe, 0x39B6, 0x28);
	// imx415_write_register(ViPipe, 0x39B8, 0x30);
	// imx415_write_register(ViPipe, 0x39BA, 0x30);
	// imx415_write_register(ViPipe, 0x39BC, 0x30);
	// imx415_write_register(ViPipe, 0x39BE, 0x30);
	// imx415_write_register(ViPipe, 0x39C0, 0x30);
	// imx415_write_register(ViPipe, 0x39C2, 0x2E);
	// imx415_write_register(ViPipe, 0x39C4, 0x2B);
	// imx415_write_register(ViPipe, 0x39C6, 0x25);
	// imx415_write_register(ViPipe, 0x3A42, 0xD1);
	// imx415_write_register(ViPipe, 0x3A4C, 0x77);
	// imx415_write_register(ViPipe, 0x3AE0, 0x02);
	// imx415_write_register(ViPipe, 0x3AEC, 0x0C);
	// imx415_write_register(ViPipe, 0x3B00, 0x2E);
	// imx415_write_register(ViPipe, 0x3B06, 0x29);
	// imx415_write_register(ViPipe, 0x3B98, 0x25);
	// imx415_write_register(ViPipe, 0x3B99, 0x21);
	// imx415_write_register(ViPipe, 0x3B9B, 0x13);
	// imx415_write_register(ViPipe, 0x3B9C, 0x13);
	// imx415_write_register(ViPipe, 0x3B9D, 0x13);
	// imx415_write_register(ViPipe, 0x3B9E, 0x13);
	// imx415_write_register(ViPipe, 0x3BA1, 0x00);
	// imx415_write_register(ViPipe, 0x3BA2, 0x06);
	// imx415_write_register(ViPipe, 0x3BA3, 0x0B);
	// imx415_write_register(ViPipe, 0x3BA4, 0x10);
	// imx415_write_register(ViPipe, 0x3BA5, 0x14);
	// imx415_write_register(ViPipe, 0x3BA6, 0x18);
	// imx415_write_register(ViPipe, 0x3BA7, 0x1A);
	// imx415_write_register(ViPipe, 0x3BA8, 0x1A);
	// imx415_write_register(ViPipe, 0x3BA9, 0x1A);
	// imx415_write_register(ViPipe, 0x3BAC, 0xED);
	// imx415_write_register(ViPipe, 0x3BAD, 0x01);
	// imx415_write_register(ViPipe, 0x3BAE, 0xF6);
	// imx415_write_register(ViPipe, 0x3BAF, 0x02);
	// imx415_write_register(ViPipe, 0x3BB0, 0xA2);
	// imx415_write_register(ViPipe, 0x3BB1, 0x03);
	// imx415_write_register(ViPipe, 0x3BB2, 0xE0);
	// imx415_write_register(ViPipe, 0x3BB3, 0x03);
	// imx415_write_register(ViPipe, 0x3BB4, 0xE0);
	// imx415_write_register(ViPipe, 0x3BB5, 0x03);
	// imx415_write_register(ViPipe, 0x3BB6, 0xE0);
	// imx415_write_register(ViPipe, 0x3BB7, 0x03);
	// imx415_write_register(ViPipe, 0x3BB8, 0xE0);
	// imx415_write_register(ViPipe, 0x3BBA, 0xE0);
	// imx415_write_register(ViPipe, 0x3BBC, 0xDA);
	// imx415_write_register(ViPipe, 0x3BBE, 0x88);
	// imx415_write_register(ViPipe, 0x3BC0, 0x44);
	// imx415_write_register(ViPipe, 0x3BC2, 0x7B);
	// imx415_write_register(ViPipe, 0x3BC4, 0xA2);
	// imx415_write_register(ViPipe, 0x3BC8, 0xBD);
	// imx415_write_register(ViPipe, 0x3BCA, 0xBD);
	// imx415_write_register(ViPipe, 0x4000, 0x10);
	// imx415_write_register(ViPipe, 0x4001, 0x03);
	// imx415_write_register(ViPipe, 0x4004, 0x00);
	// imx415_write_register(ViPipe, 0x4005, 0x06);
	// imx415_write_register(ViPipe, 0x400C, 0x01);
	// imx415_write_register(ViPipe, 0x4018, 0x9F);
	// imx415_write_register(ViPipe, 0x4019, 0x00);
	// imx415_write_register(ViPipe, 0x401A, 0x57);
	// imx415_write_register(ViPipe, 0x401B, 0x00);
	// imx415_write_register(ViPipe, 0x401C, 0x57);
	// imx415_write_register(ViPipe, 0x401D, 0x00);
	// imx415_write_register(ViPipe, 0x401E, 0x87);
	// imx415_write_register(ViPipe, 0x401F, 0x01);
	// imx415_write_register(ViPipe, 0x4020, 0x5F);
	// imx415_write_register(ViPipe, 0x4021, 0x00);
	// imx415_write_register(ViPipe, 0x4022, 0xA7);
	// imx415_write_register(ViPipe, 0x4023, 0x00);
	// imx415_write_register(ViPipe, 0x4024, 0x5F);
	// imx415_write_register(ViPipe, 0x4025, 0x00);
	// imx415_write_register(ViPipe, 0x4026, 0x97);
	// imx415_write_register(ViPipe, 0x4027, 0x00);
	// imx415_write_register(ViPipe, 0x4028, 0x4F);
	// imx415_write_register(ViPipe, 0x4029, 0x00);
	// imx415_write_register(ViPipe, 0x4074, 0x00);
	// imx415_write_register(ViPipe, 0x3000, 0x00);
	// imx415_write_register(ViPipe, 0x3002, 0x00);
	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===IMX415 3864x2192@25 Init success!===\n", ViPipe);
}

static void imx415_linear_8m30_init(VI_PIPE ViPipe)
{
	imx415_write_register(ViPipe, 0x3000, 0x01);  // STANDBY
	imx415_write_register(ViPipe, 0x3001, 0x00);  // REGHOLD
	imx415_write_register(ViPipe, 0x3002, 0x01);  // XMSTA
	imx415_write_register(ViPipe, 0x3003, 0x00);  // XMASTER
	imx415_write_register(ViPipe, 0x3008, 0x54);  // BCWAIT_TIME[9:0]
	imx415_write_register(ViPipe, 0x3009, 0x00);  // BCWAIT_TIME[9:0]
	imx415_write_register(ViPipe, 0x300A, 0x3B);  // CPWAIT_TIME[9:0]
	imx415_write_register(ViPipe, 0x300B, 0xA0);  // CPWAIT_TIME[9:0]
	imx415_write_register(ViPipe, 0x301C, 0x00);  // WINMODE[3:0]
	imx415_write_register(ViPipe, 0x301D, 0x08);  // -
	imx415_write_register(ViPipe, 0x3020, 0x00);  // HADD
	imx415_write_register(ViPipe, 0x3021, 0x00);  // VADD
	imx415_write_register(ViPipe, 0x3022, 0x00);  // ADDMODE[1:0]
	imx415_write_register(ViPipe, 0x3023, 0x01);  // VCMODE
	imx415_write_register(ViPipe, 0x3024, 0x97);  // VMAX[19:0]
	imx415_write_register(ViPipe, 0x3025, 0x11);  // VMAX[19:0]
	imx415_write_register(ViPipe, 0x3026, 0x00);  // VMAX[19:0]
	imx415_write_register(ViPipe, 0x3028, 0x15);  // HMAX[15:0]
	imx415_write_register(ViPipe, 0x3029, 0x02);  // HMAX[15:0]
	imx415_write_register(ViPipe, 0x302C, 0x00);  // WDMODE[1:0]
	imx415_write_register(ViPipe, 0x302D, 0x00);  // WDSEL[1:0]
	imx415_write_register(ViPipe, 0x3030, 0x00);  // HREVERSE
	imx415_write_register(ViPipe, 0x3031, 0x00);  // ADBIT[1:0]
	imx415_write_register(ViPipe, 0x3032, 0x00);  // MDBIT
	imx415_write_register(ViPipe, 0x3033, 0x08);  // SYS_MODE[3:0]
	imx415_write_register(ViPipe, 0x3040, 0x00);  // PIX_HST[12:0]
	imx415_write_register(ViPipe, 0x3041, 0x00);  // PIX_HST[12:0]
	imx415_write_register(ViPipe, 0x3042, 0x18);  // PIX_HWIDTH[12:0]
	imx415_write_register(ViPipe, 0x3043, 0x0F);  // PIX_HWIDTH[12:0]
	imx415_write_register(ViPipe, 0x3044, 0x00);  // PIX_VST[12:0]
	imx415_write_register(ViPipe, 0x3045, 0x00);  // PIX_VST[12:0]
	imx415_write_register(ViPipe, 0x3046, 0x20);  // PIX_VWIDTH[12:0]
	imx415_write_register(ViPipe, 0x3047, 0x11);  // PIX_VWIDTH[12:0]
	imx415_write_register(ViPipe, 0x3050, 0x08);  // SHR0[19:0]
	imx415_write_register(ViPipe, 0x3051, 0x00);  // SHR0[19:0]
	imx415_write_register(ViPipe, 0x3052, 0x00);  // SHR0[19:0]
	imx415_write_register(ViPipe, 0x3054, 0x19);  // SHR1[19:0]
	imx415_write_register(ViPipe, 0x3055, 0x00);  // SHR1[19:0]
	imx415_write_register(ViPipe, 0x3056, 0x00);  // SHR1[19:0]
	imx415_write_register(ViPipe, 0x3058, 0x3E);  // SHR2[19:0]
	imx415_write_register(ViPipe, 0x3059, 0x00);  // SHR2[19:0]
	imx415_write_register(ViPipe, 0x305A, 0x00);  // SHR2[19:0]
	imx415_write_register(ViPipe, 0x305C, 0x66);  // SHR3[19:0]
	imx415_write_register(ViPipe, 0x305D, 0x00);  // SHR3[19:0]
	imx415_write_register(ViPipe, 0x305E, 0x00);  // SHR3[19:0]
	imx415_write_register(ViPipe, 0x3060, 0x25);  // RHS1[19:0]
	imx415_write_register(ViPipe, 0x3061, 0x00);  // RHS1[19:0]
	imx415_write_register(ViPipe, 0x3062, 0x00);  // RHS1[19:0]
	imx415_write_register(ViPipe, 0x3064, 0x4A);  // RHS2[19:0]
	imx415_write_register(ViPipe, 0x3065, 0x00);  // RHS2[19:0]
	imx415_write_register(ViPipe, 0x3066, 0x00);  // RHS2[19:0]
	imx415_write_register(ViPipe, 0x3090, 0x00);  // GAIN_PGC_0[8:0]
	imx415_write_register(ViPipe, 0x3091, 0x00);  // GAIN_PGC_0[8:0]
	imx415_write_register(ViPipe, 0x3092, 0x00);  // GAIN_PGC_1[8:0]
	imx415_write_register(ViPipe, 0x3093, 0x00);  // GAIN_PGC_1[8:0]
	imx415_write_register(ViPipe, 0x3094, 0x00);  // GAIN_PGC_2[8:0]
	imx415_write_register(ViPipe, 0x3095, 0x00);  // GAIN_PGC_2[8:0]
	imx415_write_register(ViPipe, 0x3096, 0x00);  // GAIN_PGC_3[8:0]
	imx415_write_register(ViPipe, 0x3097, 0x00);  // GAIN_PGC_3[8:0]
	imx415_write_register(ViPipe, 0x30C0, 0x2A);  // XVSOUTSEL[1:0]
	imx415_write_register(ViPipe, 0x30C1, 0x00);  // XVS_DRV[1:0]
	imx415_write_register(ViPipe, 0x30CC, 0x00);  // -
	imx415_write_register(ViPipe, 0x30CD, 0x00);  // -
	imx415_write_register(ViPipe, 0x30CF, 0x00);  // XVSMSKCNT_INT[1:0]
	imx415_write_register(ViPipe, 0x30D9, 0x06);  // DIG_CLP_VSTART[4:0]
	imx415_write_register(ViPipe, 0x30DA, 0x02);  // DIG_CLP_VNUM[1:0]
	imx415_write_register(ViPipe, 0x30E2, 0x32);  // BLKELVEL[9:0]
	imx415_write_register(ViPipe, 0x30E3, 0x00);  // BLKELVEL[9:0]
	imx415_write_register(ViPipe, 0x3115, 0x00);  // INCKSEL1[7:0]
	imx415_write_register(ViPipe, 0x3116, 0x23);  // INCKSEL2[7:0]
	imx415_write_register(ViPipe, 0x3118, 0xB4);  // INCKSEL3[10:0]
	imx415_write_register(ViPipe, 0x3119, 0x00);  // INCKSEL3[10:0]
	imx415_write_register(ViPipe, 0x311A, 0xFC);  // INCKSEL4[10:0]
	imx415_write_register(ViPipe, 0x311B, 0x00);  // INCKSEL4[10:0]
	imx415_write_register(ViPipe, 0x311E, 0x23);  // INCKSEL5[7:0]
	imx415_write_register(ViPipe, 0x3260, 0x01);  // GAIN_PGC_FIDMD
	imx415_write_register(ViPipe, 0x32C8, 0x01);  // -
	imx415_write_register(ViPipe, 0x32D4, 0x21);  // -
	imx415_write_register(ViPipe, 0x32EC, 0xA1);  // -
	imx415_write_register(ViPipe, 0x344C, 0x2B);  // -
	imx415_write_register(ViPipe, 0x344D, 0x01);  // -
	imx415_write_register(ViPipe, 0x344E, 0xED);  // -
	imx415_write_register(ViPipe, 0x344F, 0x01);  // -
	imx415_write_register(ViPipe, 0x3450, 0xF6);  // -
	imx415_write_register(ViPipe, 0x3451, 0x02);  // -
	imx415_write_register(ViPipe, 0x3452, 0x7F);  // -
	imx415_write_register(ViPipe, 0x3453, 0x03);  // -
	imx415_write_register(ViPipe, 0x358A, 0x04);  // -
	imx415_write_register(ViPipe, 0x35A1, 0x02);  // -
	imx415_write_register(ViPipe, 0x35EC, 0x27);  // -
	imx415_write_register(ViPipe, 0x35EE, 0x8D);  // -
	imx415_write_register(ViPipe, 0x35F0, 0x8D);  // -
	imx415_write_register(ViPipe, 0x35F2, 0x29);  // -
	imx415_write_register(ViPipe, 0x36BC, 0x0C);  // -
	imx415_write_register(ViPipe, 0x36CC, 0x53);  // -
	imx415_write_register(ViPipe, 0x36CD, 0x00);  // -
	imx415_write_register(ViPipe, 0x36CE, 0x3C);  // -
	imx415_write_register(ViPipe, 0x36D0, 0x8C);  // -
	imx415_write_register(ViPipe, 0x36D1, 0x00);  // -
	imx415_write_register(ViPipe, 0x36D2, 0x71);  // -
	imx415_write_register(ViPipe, 0x36D4, 0x3C);  // -
	imx415_write_register(ViPipe, 0x36D6, 0x53);  // -
	imx415_write_register(ViPipe, 0x36D7, 0x00);  // -
	imx415_write_register(ViPipe, 0x36D8, 0x71);  // -
	imx415_write_register(ViPipe, 0x36DA, 0x8C);  // -
	imx415_write_register(ViPipe, 0x36DB, 0x00);  // -
	imx415_write_register(ViPipe, 0x3701, 0x00);  // ADBIT1[7:0]
	imx415_write_register(ViPipe, 0x3720, 0x00);  // -
	imx415_write_register(ViPipe, 0x3724, 0x02);  // -
	imx415_write_register(ViPipe, 0x3726, 0x02);  // -
	imx415_write_register(ViPipe, 0x3732, 0x02);  // -
	imx415_write_register(ViPipe, 0x3734, 0x03);  // -
	imx415_write_register(ViPipe, 0x3736, 0x03);  // -
	imx415_write_register(ViPipe, 0x3742, 0x03);  // -
	imx415_write_register(ViPipe, 0x3862, 0xE0);  // -
	imx415_write_register(ViPipe, 0x38CC, 0x30);  // -
	imx415_write_register(ViPipe, 0x38CD, 0x2F);  // -
	imx415_write_register(ViPipe, 0x395C, 0x0C);  // -
	imx415_write_register(ViPipe, 0x39A4, 0x07);  // -
	imx415_write_register(ViPipe, 0x39A8, 0x32);  // -
	imx415_write_register(ViPipe, 0x39AA, 0x32);  // -
	imx415_write_register(ViPipe, 0x39AC, 0x32);  // -
	imx415_write_register(ViPipe, 0x39AE, 0x32);  // -
	imx415_write_register(ViPipe, 0x39B0, 0x32);  // -
	imx415_write_register(ViPipe, 0x39B2, 0x2F);  // -
	imx415_write_register(ViPipe, 0x39B4, 0x2D);  // -
	imx415_write_register(ViPipe, 0x39B6, 0x28);  // -
	imx415_write_register(ViPipe, 0x39B8, 0x30);  // -
	imx415_write_register(ViPipe, 0x39BA, 0x30);  // -
	imx415_write_register(ViPipe, 0x39BC, 0x30);  // -
	imx415_write_register(ViPipe, 0x39BE, 0x30);  // -
	imx415_write_register(ViPipe, 0x39C0, 0x30);  // -
	imx415_write_register(ViPipe, 0x39C2, 0x2E);  // -
	imx415_write_register(ViPipe, 0x39C4, 0x2B);  // -
	imx415_write_register(ViPipe, 0x39C6, 0x25);  // -
	imx415_write_register(ViPipe, 0x3A42, 0xD1);  // -
	imx415_write_register(ViPipe, 0x3A4C, 0x77);  // -
	imx415_write_register(ViPipe, 0x3AE0, 0x02);  // -
	imx415_write_register(ViPipe, 0x3AEC, 0x0C);  // -
	imx415_write_register(ViPipe, 0x3B00, 0x2E);  // -
	imx415_write_register(ViPipe, 0x3B06, 0x29);  // -
	imx415_write_register(ViPipe, 0x3B98, 0x25);  // -
	imx415_write_register(ViPipe, 0x3B99, 0x21);  // -
	imx415_write_register(ViPipe, 0x3B9B, 0x13);  // -
	imx415_write_register(ViPipe, 0x3B9C, 0x13);  // -
	imx415_write_register(ViPipe, 0x3B9D, 0x13);  // -
	imx415_write_register(ViPipe, 0x3B9E, 0x13);  // -
	imx415_write_register(ViPipe, 0x3BA1, 0x00);  // -
	imx415_write_register(ViPipe, 0x3BA2, 0x06);  // -
	imx415_write_register(ViPipe, 0x3BA3, 0x0B);  // -
	imx415_write_register(ViPipe, 0x3BA4, 0x10);  // -
	imx415_write_register(ViPipe, 0x3BA5, 0x14);  // -
	imx415_write_register(ViPipe, 0x3BA6, 0x18);  // -
	imx415_write_register(ViPipe, 0x3BA7, 0x1A);  // -
	imx415_write_register(ViPipe, 0x3BA8, 0x1A);  // -
	imx415_write_register(ViPipe, 0x3BA9, 0x1A);  // -
	imx415_write_register(ViPipe, 0x3BAC, 0xED);  // -
	imx415_write_register(ViPipe, 0x3BAD, 0x01);  // -
	imx415_write_register(ViPipe, 0x3BAE, 0xF6);  // -
	imx415_write_register(ViPipe, 0x3BAF, 0x02);  // -
	imx415_write_register(ViPipe, 0x3BB0, 0xA2);  // -
	imx415_write_register(ViPipe, 0x3BB1, 0x03);  // -
	imx415_write_register(ViPipe, 0x3BB2, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BB3, 0x03);  // -
	imx415_write_register(ViPipe, 0x3BB4, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BB5, 0x03);  // -
	imx415_write_register(ViPipe, 0x3BB6, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BB7, 0x03);  // -
	imx415_write_register(ViPipe, 0x3BB8, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BBA, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BBC, 0xDA);  // -
	imx415_write_register(ViPipe, 0x3BBE, 0x88);  // -
	imx415_write_register(ViPipe, 0x3BC0, 0x44);  // -
	imx415_write_register(ViPipe, 0x3BC2, 0x7B);  // -
	imx415_write_register(ViPipe, 0x3BC4, 0xA2);  // -
	imx415_write_register(ViPipe, 0x3BC8, 0xBD);  // -
	imx415_write_register(ViPipe, 0x3BCA, 0xBD);  // -
	imx415_write_register(ViPipe, 0x4000, 0x10);  // -
	imx415_write_register(ViPipe, 0x4001, 0x03);  // LANEMODE[2:0]
	imx415_write_register(ViPipe, 0x4004, 0x00);  // TXCLKESC_FREQ[15:0]
	imx415_write_register(ViPipe, 0x4005, 0x06);  // TXCLKESC_FREQ[15:0]
	imx415_write_register(ViPipe, 0x400C, 0x01);  // INCKSEL6
	imx415_write_register(ViPipe, 0x4018, 0x9F);  // TCLKPOST[15:0]
	imx415_write_register(ViPipe, 0x4019, 0x00);  // TCLKPOST[15:0]
	imx415_write_register(ViPipe, 0x401A, 0x57);  // TCLKPREPARE[15:0]
	imx415_write_register(ViPipe, 0x401B, 0x00);  // TCLKPREPARE[15:0]
	imx415_write_register(ViPipe, 0x401C, 0x57);  // TCLKTRAIL[15:0]
	imx415_write_register(ViPipe, 0x401D, 0x00);  // TCLKTRAIL[15:0]
	imx415_write_register(ViPipe, 0x401E, 0x87);  // TCLKZERO[15:0]
	imx415_write_register(ViPipe, 0x401F, 0x01);  // TCLKZERO[15:0]
	imx415_write_register(ViPipe, 0x4020, 0x5F);  // THSPREPARE[15:0]
	imx415_write_register(ViPipe, 0x4021, 0x00);  // THSPREPARE[15:0]
	imx415_write_register(ViPipe, 0x4022, 0xA7);  // THSZERO[15:0]
	imx415_write_register(ViPipe, 0x4023, 0x00);  // THSZERO[15:0]
	imx415_write_register(ViPipe, 0x4024, 0x5F);  // THSTRAIL[15:0]
	imx415_write_register(ViPipe, 0x4025, 0x00);  // THSTRAIL[15:0]
	imx415_write_register(ViPipe, 0x4026, 0x97);  // THSEXIT[15:0]
	imx415_write_register(ViPipe, 0x4027, 0x00);  // THSEXIT[15:0]
	imx415_write_register(ViPipe, 0x4028, 0x4F);  // TLPX[15:0]
	imx415_write_register(ViPipe, 0x4029, 0x00);  // TLPX[15:0]
	imx415_write_register(ViPipe, 0x4074, 0x00);  // INCKSEL7[2:0]
	imx415_write_register(ViPipe, 0x3000, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x00);
	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===IMX415 3840x2160@30 Init success!===\n", ViPipe);
}

static void imx415_linear_5m25_init(VI_PIPE ViPipe)
{
	delay_ms(4);

	// 24M 2568x2160@25
	imx415_write_register(ViPipe, 0x3000, 0x01);
	imx415_write_register(ViPipe, 0x3001, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x01);
	imx415_write_register(ViPipe, 0x3003, 0x00);
	imx415_write_register(ViPipe, 0x3008, 0x54);
	imx415_write_register(ViPipe, 0x3009, 0x00);
	imx415_write_register(ViPipe, 0x300A, 0x3B);
	imx415_write_register(ViPipe, 0x300B, 0xA0);
	imx415_write_register(ViPipe, 0x301C, 0x04);
	imx415_write_register(ViPipe, 0x301D, 0x08);
	imx415_write_register(ViPipe, 0x3020, 0x00);
	imx415_write_register(ViPipe, 0x3021, 0x00);
	imx415_write_register(ViPipe, 0x3022, 0x00);
	imx415_write_register(ViPipe, 0x3023, 0x01);
	imx415_write_register(ViPipe, 0x3024, 0x8C);
	imx415_write_register(ViPipe, 0x3025, 0x0A);
	imx415_write_register(ViPipe, 0x3026, 0x00);
	imx415_write_register(ViPipe, 0x3028, 0x4C);
	imx415_write_register(ViPipe, 0x3029, 0x04);
	imx415_write_register(ViPipe, 0x302C, 0x00);
	imx415_write_register(ViPipe, 0x302D, 0x00);
	imx415_write_register(ViPipe, 0x3030, 0x01);
	imx415_write_register(ViPipe, 0x3031, 0x01);
	imx415_write_register(ViPipe, 0x3032, 0x01);
	imx415_write_register(ViPipe, 0x3033, 0x06);
	imx415_write_register(ViPipe, 0x3040, 0x88);
	imx415_write_register(ViPipe, 0x3041, 0x02);
	imx415_write_register(ViPipe, 0x3042, 0x08);
	imx415_write_register(ViPipe, 0x3043, 0x0A);
	imx415_write_register(ViPipe, 0x3044, 0x20);
	imx415_write_register(ViPipe, 0x3045, 0x00);
	imx415_write_register(ViPipe, 0x3046, 0xE0);
	imx415_write_register(ViPipe, 0x3047, 0x10);
	imx415_write_register(ViPipe, 0x3050, 0x08);
	imx415_write_register(ViPipe, 0x3051, 0x00);
	imx415_write_register(ViPipe, 0x3052, 0x00);
	imx415_write_register(ViPipe, 0x3054, 0x19);
	imx415_write_register(ViPipe, 0x3055, 0x00);
	imx415_write_register(ViPipe, 0x3056, 0x00);
	imx415_write_register(ViPipe, 0x3058, 0x3E);
	imx415_write_register(ViPipe, 0x3059, 0x00);
	imx415_write_register(ViPipe, 0x305A, 0x00);
	imx415_write_register(ViPipe, 0x305C, 0x66);
	imx415_write_register(ViPipe, 0x305D, 0x00);
	imx415_write_register(ViPipe, 0x305E, 0x00);
	imx415_write_register(ViPipe, 0x3060, 0x25);
	imx415_write_register(ViPipe, 0x3061, 0x00);
	imx415_write_register(ViPipe, 0x3062, 0x00);
	imx415_write_register(ViPipe, 0x3064, 0x4A);
	imx415_write_register(ViPipe, 0x3065, 0x00);
	imx415_write_register(ViPipe, 0x3066, 0x00);
	imx415_write_register(ViPipe, 0x3090, 0x00);
	imx415_write_register(ViPipe, 0x3091, 0x00);
	imx415_write_register(ViPipe, 0x3092, 0x00);
	imx415_write_register(ViPipe, 0x3093, 0x00);
	imx415_write_register(ViPipe, 0x3094, 0x00);
	imx415_write_register(ViPipe, 0x3095, 0x00);
	imx415_write_register(ViPipe, 0x3096, 0x00);
	imx415_write_register(ViPipe, 0x3097, 0x00);
	imx415_write_register(ViPipe, 0x30C0, 0x2A);
	imx415_write_register(ViPipe, 0x30C1, 0x00);
	imx415_write_register(ViPipe, 0x30CC, 0x00);
	imx415_write_register(ViPipe, 0x30CD, 0x00);
	imx415_write_register(ViPipe, 0x30CF, 0x00);
	imx415_write_register(ViPipe, 0x30D9, 0x06);
	imx415_write_register(ViPipe, 0x30DA, 0x02);
	imx415_write_register(ViPipe, 0x30E2, 0x32);
	imx415_write_register(ViPipe, 0x30E3, 0x00);
	imx415_write_register(ViPipe, 0x3115, 0x00);
	imx415_write_register(ViPipe, 0x3116, 0x24);
	imx415_write_register(ViPipe, 0x3118, 0xC6);
	imx415_write_register(ViPipe, 0x3119, 0x00);
	imx415_write_register(ViPipe, 0x311A, 0x5A);
	imx415_write_register(ViPipe, 0x311B, 0x01);
	imx415_write_register(ViPipe, 0x311E, 0x24);
	imx415_write_register(ViPipe, 0x3260, 0x01);
	imx415_write_register(ViPipe, 0x32C8, 0x01);
	imx415_write_register(ViPipe, 0x32D4, 0x21);
	imx415_write_register(ViPipe, 0x32EC, 0xA1);
	imx415_write_register(ViPipe, 0x344C, 0x2B);
	imx415_write_register(ViPipe, 0x344D, 0x01);
	imx415_write_register(ViPipe, 0x344E, 0xED);
	imx415_write_register(ViPipe, 0x344F, 0x01);
	imx415_write_register(ViPipe, 0x3450, 0xF6);
	imx415_write_register(ViPipe, 0x3451, 0x02);
	imx415_write_register(ViPipe, 0x3452, 0x7F);
	imx415_write_register(ViPipe, 0x3453, 0x03);
	imx415_write_register(ViPipe, 0x358A, 0x04);
	imx415_write_register(ViPipe, 0x35A1, 0x02);
	imx415_write_register(ViPipe, 0x35EC, 0x27);
	imx415_write_register(ViPipe, 0x35EE, 0x8D);
	imx415_write_register(ViPipe, 0x35F0, 0x8D);
	imx415_write_register(ViPipe, 0x35F2, 0x29);
	imx415_write_register(ViPipe, 0x36BC, 0x0C);
	imx415_write_register(ViPipe, 0x36CC, 0x53);
	imx415_write_register(ViPipe, 0x36CD, 0x00);
	imx415_write_register(ViPipe, 0x36CE, 0x3C);
	imx415_write_register(ViPipe, 0x36D0, 0x8C);
	imx415_write_register(ViPipe, 0x36D1, 0x00);
	imx415_write_register(ViPipe, 0x36D2, 0x71);
	imx415_write_register(ViPipe, 0x36D4, 0x3C);
	imx415_write_register(ViPipe, 0x36D6, 0x53);
	imx415_write_register(ViPipe, 0x36D7, 0x00);
	imx415_write_register(ViPipe, 0x36D8, 0x71);
	imx415_write_register(ViPipe, 0x36DA, 0x8C);
	imx415_write_register(ViPipe, 0x36DB, 0x00);
	imx415_write_register(ViPipe, 0x3701, 0x03);
	imx415_write_register(ViPipe, 0x3720, 0x00);
	imx415_write_register(ViPipe, 0x3724, 0x02);
	imx415_write_register(ViPipe, 0x3726, 0x02);
	imx415_write_register(ViPipe, 0x3732, 0x02);
	imx415_write_register(ViPipe, 0x3734, 0x03);
	imx415_write_register(ViPipe, 0x3736, 0x03);
	imx415_write_register(ViPipe, 0x3742, 0x03);
	imx415_write_register(ViPipe, 0x3862, 0xE0);
	imx415_write_register(ViPipe, 0x38CC, 0x30);
	imx415_write_register(ViPipe, 0x38CD, 0x2F);
	imx415_write_register(ViPipe, 0x395C, 0x0C);
	imx415_write_register(ViPipe, 0x39A4, 0x07);
	imx415_write_register(ViPipe, 0x39A8, 0x32);
	imx415_write_register(ViPipe, 0x39AA, 0x32);
	imx415_write_register(ViPipe, 0x39AC, 0x32);
	imx415_write_register(ViPipe, 0x39AE, 0x32);
	imx415_write_register(ViPipe, 0x39B0, 0x32);
	imx415_write_register(ViPipe, 0x39B2, 0x2F);
	imx415_write_register(ViPipe, 0x39B4, 0x2D);
	imx415_write_register(ViPipe, 0x39B6, 0x28);
	imx415_write_register(ViPipe, 0x39B8, 0x30);
	imx415_write_register(ViPipe, 0x39BA, 0x30);
	imx415_write_register(ViPipe, 0x39BC, 0x30);
	imx415_write_register(ViPipe, 0x39BE, 0x30);
	imx415_write_register(ViPipe, 0x39C0, 0x30);
	imx415_write_register(ViPipe, 0x39C2, 0x2E);
	imx415_write_register(ViPipe, 0x39C4, 0x2B);
	imx415_write_register(ViPipe, 0x39C6, 0x25);
	imx415_write_register(ViPipe, 0x3A42, 0xD1);
	imx415_write_register(ViPipe, 0x3A4C, 0x77);
	imx415_write_register(ViPipe, 0x3AE0, 0x02);
	imx415_write_register(ViPipe, 0x3AEC, 0x0C);
	imx415_write_register(ViPipe, 0x3B00, 0x2E);
	imx415_write_register(ViPipe, 0x3B06, 0x29);
	imx415_write_register(ViPipe, 0x3B98, 0x25);
	imx415_write_register(ViPipe, 0x3B99, 0x21);
	imx415_write_register(ViPipe, 0x3B9B, 0x13);
	imx415_write_register(ViPipe, 0x3B9C, 0x13);
	imx415_write_register(ViPipe, 0x3B9D, 0x13);
	imx415_write_register(ViPipe, 0x3B9E, 0x13);
	imx415_write_register(ViPipe, 0x3BA1, 0x00);
	imx415_write_register(ViPipe, 0x3BA2, 0x06);
	imx415_write_register(ViPipe, 0x3BA3, 0x0B);
	imx415_write_register(ViPipe, 0x3BA4, 0x10);
	imx415_write_register(ViPipe, 0x3BA5, 0x14);
	imx415_write_register(ViPipe, 0x3BA6, 0x18);
	imx415_write_register(ViPipe, 0x3BA7, 0x1A);
	imx415_write_register(ViPipe, 0x3BA8, 0x1A);
	imx415_write_register(ViPipe, 0x3BA9, 0x1A);
	imx415_write_register(ViPipe, 0x3BAC, 0xED);
	imx415_write_register(ViPipe, 0x3BAD, 0x01);
	imx415_write_register(ViPipe, 0x3BAE, 0xF6);
	imx415_write_register(ViPipe, 0x3BAF, 0x02);
	imx415_write_register(ViPipe, 0x3BB0, 0xA2);
	imx415_write_register(ViPipe, 0x3BB1, 0x03);
	imx415_write_register(ViPipe, 0x3BB2, 0xE0);
	imx415_write_register(ViPipe, 0x3BB3, 0x03);
	imx415_write_register(ViPipe, 0x3BB4, 0xE0);
	imx415_write_register(ViPipe, 0x3BB5, 0x03);
	imx415_write_register(ViPipe, 0x3BB6, 0xE0);
	imx415_write_register(ViPipe, 0x3BB7, 0x03);
	imx415_write_register(ViPipe, 0x3BB8, 0xE0);
	imx415_write_register(ViPipe, 0x3BBA, 0xE0);
	imx415_write_register(ViPipe, 0x3BBC, 0xDA);
	imx415_write_register(ViPipe, 0x3BBE, 0x88);
	imx415_write_register(ViPipe, 0x3BC0, 0x44);
	imx415_write_register(ViPipe, 0x3BC2, 0x7B);
	imx415_write_register(ViPipe, 0x3BC4, 0xA2);
	imx415_write_register(ViPipe, 0x3BC8, 0xBD);
	imx415_write_register(ViPipe, 0x3BCA, 0xBD);
	imx415_write_register(ViPipe, 0x4000, 0x10);
	imx415_write_register(ViPipe, 0x4001, 0x03);
	imx415_write_register(ViPipe, 0x4004, 0x00);
	imx415_write_register(ViPipe, 0x4005, 0x06);
	imx415_write_register(ViPipe, 0x400C, 0x00);
	imx415_write_register(ViPipe, 0x4018, 0x8F);
	imx415_write_register(ViPipe, 0x4019, 0x00);
	imx415_write_register(ViPipe, 0x401A, 0x4F);
	imx415_write_register(ViPipe, 0x401B, 0x00);
	imx415_write_register(ViPipe, 0x401C, 0x47);
	imx415_write_register(ViPipe, 0x401D, 0x00);
	imx415_write_register(ViPipe, 0x401E, 0x37);
	imx415_write_register(ViPipe, 0x401F, 0x01);
	imx415_write_register(ViPipe, 0x4020, 0x4F);
	imx415_write_register(ViPipe, 0x4021, 0x00);
	imx415_write_register(ViPipe, 0x4022, 0x87);
	imx415_write_register(ViPipe, 0x4023, 0x00);
	imx415_write_register(ViPipe, 0x4024, 0x4F);
	imx415_write_register(ViPipe, 0x4025, 0x00);
	imx415_write_register(ViPipe, 0x4026, 0x7F);
	imx415_write_register(ViPipe, 0x4027, 0x00);
	imx415_write_register(ViPipe, 0x4028, 0x3F);
	imx415_write_register(ViPipe, 0x4029, 0x00);
	imx415_write_register(ViPipe, 0x4074, 0x00);
	imx415_write_register(ViPipe, 0x3000, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x00);
	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===IMX415 2568x2160@25 Init success!===\n", ViPipe);
}

static void imx415_linear_2m60_init(VI_PIPE ViPipe)
{
	imx415_write_register(ViPipe, 0x3000, 0x01);  // STANDBY
	imx415_write_register(ViPipe, 0x3001, 0x00);  // REGHOLD
	imx415_write_register(ViPipe, 0x3002, 0x01);  // XMSTA
	imx415_write_register(ViPipe, 0x3003, 0x00);  // XMASTER
	imx415_write_register(ViPipe, 0x3008, 0x5D);  // BCWAIT_TIME[9:0]
	imx415_write_register(ViPipe, 0x3009, 0x00);  // BCWAIT_TIME[9:0]
	imx415_write_register(ViPipe, 0x300A, 0x42);  // CPWAIT_TIME[9:0]
	imx415_write_register(ViPipe, 0x300B, 0xA0);  // CPWAIT_TIME[9:0]
	imx415_write_register(ViPipe, 0x301C, 0x00);  // WINMODE[3:0]
	imx415_write_register(ViPipe, 0x301D, 0x08);  // -
	imx415_write_register(ViPipe, 0x3020, 0x01);  // HADD
	imx415_write_register(ViPipe, 0x3021, 0x01);  // VADD
	imx415_write_register(ViPipe, 0x3022, 0x01);  // ADDMODE[1:0]
	imx415_write_register(ViPipe, 0x3023, 0x01);  // VCMODE
	imx415_write_register(ViPipe, 0x3024, 0xEB);  // VMAX[19:0]
	imx415_write_register(ViPipe, 0x3025, 0x08);  // VMAX[19:0]
	imx415_write_register(ViPipe, 0x3026, 0x00);  // VMAX[19:0]
	imx415_write_register(ViPipe, 0x3028, 0x1E);  // HMAX[15:0]
	imx415_write_register(ViPipe, 0x3029, 0x02);  // HMAX[15:0]
	imx415_write_register(ViPipe, 0x302C, 0x00);  // WDMODE[1:0]
	imx415_write_register(ViPipe, 0x302D, 0x00);  // WDSEL[1:0]
	imx415_write_register(ViPipe, 0x3030, 0x00);  // HREVERSE
	imx415_write_register(ViPipe, 0x3031, 0x00);  // ADBIT[1:0]
	imx415_write_register(ViPipe, 0x3032, 0x01);  // MDBIT
	imx415_write_register(ViPipe, 0x3033, 0x05);  // SYS_MODE[3:0]
	imx415_write_register(ViPipe, 0x3040, 0x00);  // PIX_HST[12:0]
	imx415_write_register(ViPipe, 0x3041, 0x00);  // PIX_HST[12:0]
	imx415_write_register(ViPipe, 0x3042, 0x18);  // PIX_HWIDTH[12:0]
	imx415_write_register(ViPipe, 0x3043, 0x0F);  // PIX_HWIDTH[12:0]
	imx415_write_register(ViPipe, 0x3044, 0x00);  // PIX_VST[12:0]
	imx415_write_register(ViPipe, 0x3045, 0x00);  // PIX_VST[12:0]
	imx415_write_register(ViPipe, 0x3046, 0x20);  // PIX_VWIDTH[12:0]
	imx415_write_register(ViPipe, 0x3047, 0x11);  // PIX_VWIDTH[12:0]
	imx415_write_register(ViPipe, 0x3050, 0x08);  // SHR0[19:0]
	imx415_write_register(ViPipe, 0x3051, 0x00);  // SHR0[19:0]
	imx415_write_register(ViPipe, 0x3052, 0x00);  // SHR0[19:0]
	imx415_write_register(ViPipe, 0x3054, 0x19);  // SHR1[19:0]
	imx415_write_register(ViPipe, 0x3055, 0x00);  // SHR1[19:0]
	imx415_write_register(ViPipe, 0x3056, 0x00);  // SHR1[19:0]
	imx415_write_register(ViPipe, 0x3058, 0x3E);  // SHR2[19:0]
	imx415_write_register(ViPipe, 0x3059, 0x00);  // SHR2[19:0]
	imx415_write_register(ViPipe, 0x305A, 0x00);  // SHR2[19:0]
	imx415_write_register(ViPipe, 0x305C, 0x66);  // SHR3[19:0]
	imx415_write_register(ViPipe, 0x305D, 0x00);  // SHR3[19:0]
	imx415_write_register(ViPipe, 0x305E, 0x00);  // SHR3[19:0]
	imx415_write_register(ViPipe, 0x3060, 0x25);  // RHS1[19:0]
	imx415_write_register(ViPipe, 0x3061, 0x00);  // RHS1[19:0]
	imx415_write_register(ViPipe, 0x3062, 0x00);  // RHS1[19:0]
	imx415_write_register(ViPipe, 0x3064, 0x4A);  // RHS2[19:0]
	imx415_write_register(ViPipe, 0x3065, 0x00);  // RHS2[19:0]
	imx415_write_register(ViPipe, 0x3066, 0x00);  // RHS2[19:0]
	imx415_write_register(ViPipe, 0x3090, 0x00);  // GAIN_PGC_0[8:0]
	imx415_write_register(ViPipe, 0x3091, 0x00);  // GAIN_PGC_0[8:0]
	imx415_write_register(ViPipe, 0x3092, 0x00);  // GAIN_PGC_1[8:0]
	imx415_write_register(ViPipe, 0x3093, 0x00);  // GAIN_PGC_1[8:0]
	imx415_write_register(ViPipe, 0x3094, 0x00);  // GAIN_PGC_2[8:0]
	imx415_write_register(ViPipe, 0x3095, 0x00);  // GAIN_PGC_2[8:0]
	imx415_write_register(ViPipe, 0x3096, 0x00);  // GAIN_PGC_3[8:0]
	imx415_write_register(ViPipe, 0x3097, 0x00);  // GAIN_PGC_3[8:0]
	imx415_write_register(ViPipe, 0x30C0, 0x2A);  // XVSOUTSEL[1:0]
	imx415_write_register(ViPipe, 0x30C1, 0x00);  // XVS_DRV[1:0]
	imx415_write_register(ViPipe, 0x30CC, 0x00);  // -
	imx415_write_register(ViPipe, 0x30CD, 0x00);  // -
	imx415_write_register(ViPipe, 0x30CF, 0x00);  // XVSMSKCNT_INT[1:0]
	imx415_write_register(ViPipe, 0x30D9, 0x02);  // DIG_CLP_VSTART[4:0]
	imx415_write_register(ViPipe, 0x30DA, 0x01);  // DIG_CLP_VNUM[1:0]
	imx415_write_register(ViPipe, 0x30E2, 0x32);  // BLKELVEL[9:0]
	imx415_write_register(ViPipe, 0x30E3, 0x00);  // BLKELVEL[9:0]
	imx415_write_register(ViPipe, 0x3115, 0x00);  // INCKSEL1[7:0]
	imx415_write_register(ViPipe, 0x3116, 0x23);  // INCKSEL2[7:0]
	imx415_write_register(ViPipe, 0x3118, 0xC6);  // INCKSEL3[10:0]
	imx415_write_register(ViPipe, 0x3119, 0x00);  // INCKSEL3[10:0]
	imx415_write_register(ViPipe, 0x311A, 0xE7);  // INCKSEL4[10:0]
	imx415_write_register(ViPipe, 0x311B, 0x00);  // INCKSEL4[10:0]
	imx415_write_register(ViPipe, 0x311E, 0x23);  // INCKSEL5[7:0]
	imx415_write_register(ViPipe, 0x3260, 0x01);  // GAIN_PGC_FIDMD
	imx415_write_register(ViPipe, 0x32C8, 0x01);  // -
	imx415_write_register(ViPipe, 0x32D4, 0x21);  // -
	imx415_write_register(ViPipe, 0x32EC, 0xA1);  // -
	imx415_write_register(ViPipe, 0x344C, 0x2B);  // -
	imx415_write_register(ViPipe, 0x344D, 0x01);  // -
	imx415_write_register(ViPipe, 0x344E, 0xED);  // -
	imx415_write_register(ViPipe, 0x344F, 0x01);  // -
	imx415_write_register(ViPipe, 0x3450, 0xF6);  // -
	imx415_write_register(ViPipe, 0x3451, 0x02);  // -
	imx415_write_register(ViPipe, 0x3452, 0x7F);  // -
	imx415_write_register(ViPipe, 0x3453, 0x03);  // -
	imx415_write_register(ViPipe, 0x358A, 0x04);  // -
	imx415_write_register(ViPipe, 0x35A1, 0x02);  // -
	imx415_write_register(ViPipe, 0x35EC, 0x27);  // -
	imx415_write_register(ViPipe, 0x35EE, 0x8D);  // -
	imx415_write_register(ViPipe, 0x35F0, 0x8D);  // -
	imx415_write_register(ViPipe, 0x35F2, 0x29);  // -
	imx415_write_register(ViPipe, 0x36BC, 0x0C);  // -
	imx415_write_register(ViPipe, 0x36CC, 0x53);  // -
	imx415_write_register(ViPipe, 0x36CD, 0x00);  // -
	imx415_write_register(ViPipe, 0x36CE, 0x3C);  // -
	imx415_write_register(ViPipe, 0x36D0, 0x8C);  // -
	imx415_write_register(ViPipe, 0x36D1, 0x00);  // -
	imx415_write_register(ViPipe, 0x36D2, 0x71);  // -
	imx415_write_register(ViPipe, 0x36D4, 0x3C);  // -
	imx415_write_register(ViPipe, 0x36D6, 0x53);  // -
	imx415_write_register(ViPipe, 0x36D7, 0x00);  // -
	imx415_write_register(ViPipe, 0x36D8, 0x71);  // -
	imx415_write_register(ViPipe, 0x36DA, 0x8C);  // -
	imx415_write_register(ViPipe, 0x36DB, 0x00);  // -
	imx415_write_register(ViPipe, 0x3701, 0x00);  // ADBIT1[7:0]
	imx415_write_register(ViPipe, 0x3720, 0x00);  // -
	imx415_write_register(ViPipe, 0x3724, 0x02);  // -
	imx415_write_register(ViPipe, 0x3726, 0x02);  // -
	imx415_write_register(ViPipe, 0x3732, 0x02);  // -
	imx415_write_register(ViPipe, 0x3734, 0x03);  // -
	imx415_write_register(ViPipe, 0x3736, 0x03);  // -
	imx415_write_register(ViPipe, 0x3742, 0x03);  // -
	imx415_write_register(ViPipe, 0x3862, 0xE0);  // -
	imx415_write_register(ViPipe, 0x38CC, 0x30);  // -
	imx415_write_register(ViPipe, 0x38CD, 0x2F);  // -
	imx415_write_register(ViPipe, 0x395C, 0x0C);  // -
	imx415_write_register(ViPipe, 0x39A4, 0x07);  // -
	imx415_write_register(ViPipe, 0x39A8, 0x32);  // -
	imx415_write_register(ViPipe, 0x39AA, 0x32);  // -
	imx415_write_register(ViPipe, 0x39AC, 0x32);  // -
	imx415_write_register(ViPipe, 0x39AE, 0x32);  // -
	imx415_write_register(ViPipe, 0x39B0, 0x32);  // -
	imx415_write_register(ViPipe, 0x39B2, 0x2F);  // -
	imx415_write_register(ViPipe, 0x39B4, 0x2D);  // -
	imx415_write_register(ViPipe, 0x39B6, 0x28);  // -
	imx415_write_register(ViPipe, 0x39B8, 0x30);  // -
	imx415_write_register(ViPipe, 0x39BA, 0x30);  // -
	imx415_write_register(ViPipe, 0x39BC, 0x30);  // -
	imx415_write_register(ViPipe, 0x39BE, 0x30);  // -
	imx415_write_register(ViPipe, 0x39C0, 0x30);  // -
	imx415_write_register(ViPipe, 0x39C2, 0x2E);  // -
	imx415_write_register(ViPipe, 0x39C4, 0x2B);  // -
	imx415_write_register(ViPipe, 0x39C6, 0x25);  // -
	imx415_write_register(ViPipe, 0x3A42, 0xD1);  // -
	imx415_write_register(ViPipe, 0x3A4C, 0x77);  // -
	imx415_write_register(ViPipe, 0x3AE0, 0x02);  // -
	imx415_write_register(ViPipe, 0x3AEC, 0x0C);  // -
	imx415_write_register(ViPipe, 0x3B00, 0x2E);  // -
	imx415_write_register(ViPipe, 0x3B06, 0x29);  // -
	imx415_write_register(ViPipe, 0x3B98, 0x25);  // -
	imx415_write_register(ViPipe, 0x3B99, 0x21);  // -
	imx415_write_register(ViPipe, 0x3B9B, 0x13);  // -
	imx415_write_register(ViPipe, 0x3B9C, 0x13);  // -
	imx415_write_register(ViPipe, 0x3B9D, 0x13);  // -
	imx415_write_register(ViPipe, 0x3B9E, 0x13);  // -
	imx415_write_register(ViPipe, 0x3BA1, 0x00);  // -
	imx415_write_register(ViPipe, 0x3BA2, 0x06);  // -
	imx415_write_register(ViPipe, 0x3BA3, 0x0B);  // -
	imx415_write_register(ViPipe, 0x3BA4, 0x10);  // -
	imx415_write_register(ViPipe, 0x3BA5, 0x14);  // -
	imx415_write_register(ViPipe, 0x3BA6, 0x18);  // -
	imx415_write_register(ViPipe, 0x3BA7, 0x1A);  // -
	imx415_write_register(ViPipe, 0x3BA8, 0x1A);  // -
	imx415_write_register(ViPipe, 0x3BA9, 0x1A);  // -
	imx415_write_register(ViPipe, 0x3BAC, 0xED);  // -
	imx415_write_register(ViPipe, 0x3BAD, 0x01);  // -
	imx415_write_register(ViPipe, 0x3BAE, 0xF6);  // -
	imx415_write_register(ViPipe, 0x3BAF, 0x02);  // -
	imx415_write_register(ViPipe, 0x3BB0, 0xA2);  // -
	imx415_write_register(ViPipe, 0x3BB1, 0x03);  // -
	imx415_write_register(ViPipe, 0x3BB2, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BB3, 0x03);  // -
	imx415_write_register(ViPipe, 0x3BB4, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BB5, 0x03);  // -
	imx415_write_register(ViPipe, 0x3BB6, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BB7, 0x03);  // -
	imx415_write_register(ViPipe, 0x3BB8, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BBA, 0xE0);  // -
	imx415_write_register(ViPipe, 0x3BBC, 0xDA);  // -
	imx415_write_register(ViPipe, 0x3BBE, 0x88);  // -
	imx415_write_register(ViPipe, 0x3BC0, 0x44);  // -
	imx415_write_register(ViPipe, 0x3BC2, 0x7B);  // -
	imx415_write_register(ViPipe, 0x3BC4, 0xA2);  // -
	imx415_write_register(ViPipe, 0x3BC8, 0xBD);  // -
	imx415_write_register(ViPipe, 0x3BCA, 0xBD);  // -
	imx415_write_register(ViPipe, 0x4000, 0x10);  // -
	imx415_write_register(ViPipe, 0x4001, 0x03);  // LANEMODE[2:0]
	imx415_write_register(ViPipe, 0x4004, 0xC0);  // TXCLKESC_FREQ[15:0]
	imx415_write_register(ViPipe, 0x4005, 0x06);  // TXCLKESC_FREQ[15:0]
	imx415_write_register(ViPipe, 0x400C, 0x00);  // INCKSEL6
	imx415_write_register(ViPipe, 0x4018, 0x7F);  // TCLKPOST[15:0]
	imx415_write_register(ViPipe, 0x4019, 0x00);  // TCLKPOST[15:0]
	imx415_write_register(ViPipe, 0x401A, 0x37);  // TCLKPREPARE[15:0]
	imx415_write_register(ViPipe, 0x401B, 0x00);  // TCLKPREPARE[15:0]
	imx415_write_register(ViPipe, 0x401C, 0x37);  // TCLKTRAIL[15:0]
	imx415_write_register(ViPipe, 0x401D, 0x00);  // TCLKTRAIL[15:0]
	imx415_write_register(ViPipe, 0x401E, 0xF7);  // TCLKZERO[15:0]
	imx415_write_register(ViPipe, 0x401F, 0x00);  // TCLKZERO[15:0]
	imx415_write_register(ViPipe, 0x4020, 0x3F);  // THSPREPARE[15:0]
	imx415_write_register(ViPipe, 0x4021, 0x00);  // THSPREPARE[15:0]
	imx415_write_register(ViPipe, 0x4022, 0x6F);  // THSZERO[15:0]
	imx415_write_register(ViPipe, 0x4023, 0x00);  // THSZERO[15:0]
	imx415_write_register(ViPipe, 0x4024, 0x3F);  // THSTRAIL[15:0]
	imx415_write_register(ViPipe, 0x4025, 0x00);  // THSTRAIL[15:0]
	imx415_write_register(ViPipe, 0x4026, 0x5F);  // THSEXIT[15:0]
	imx415_write_register(ViPipe, 0x4027, 0x00);  // THSEXIT[15:0]
	imx415_write_register(ViPipe, 0x4028, 0x2F);  // TLPX[15:0]
	imx415_write_register(ViPipe, 0x4029, 0x00);  // TLPX[15:0]
	imx415_write_register(ViPipe, 0x4074, 0x01);  // INCKSEL7[2:0]
	imx415_write_register(ViPipe, 0x3000, 0x00);  // STANDBY
	imx415_write_register(ViPipe, 0x3002 ,0x00);  // REGHOLD
	imx415_default_reg_init(ViPipe);
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===IMX415 1920X1080@60 Init success!===\n", ViPipe);
}