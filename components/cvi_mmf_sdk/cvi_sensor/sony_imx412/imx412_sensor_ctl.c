#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx412_cmos_ex.h"
#include "sensor_i2c.h"

static void imx412_linear_12M30_init(VI_PIPE ViPipe);
static void imx412_wdr_12M30_2to1_init(VI_PIPE ViPipe);
static void imx412_wdr_8M30_2to1_init(VI_PIPE ViPipe);

const CVI_U32 imx412_addr_byte = 2;
const CVI_U32 imx412_data_byte = 1;

int imx412_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx412_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx412_AddrInfo[ViPipe].s8I2cAddr);
}

int imx412_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx412_BusInfo[ViPipe].s8I2cDev);
}

int imx412_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx412_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx412_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx412_addr_byte, imx412_data_byte);
}

int imx412_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx412_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx412_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx412_addr_byte, (CVI_U32)data, imx412_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx412_standby(VI_PIPE ViPipe)
{
	imx412_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx412_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */
}

void imx412_restart(VI_PIPE ViPipe)
{
	imx412_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx412_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
}

void imx412_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx412[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx412_write_register(ViPipe,
				g_pastImx412[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx412[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx412_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	imx412_write_register(ViPipe, 0x0101, u8Mirror | (u8Filp << 1));
}

void imx412_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx412[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx412[ViPipe]->u8ImgMode;

	imx412_i2c_init(ViPipe);
	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == IMX412_MODE_12M30_WDR) {
			imx412_wdr_12M30_2to1_init(ViPipe);
		} else if (u8ImgMode == IMX412_MODE_8M30_WDR) {
			imx412_wdr_8M30_2to1_init(ViPipe);
		} else {
		}
	} else {
		if (u8ImgMode == IMX412_MODE_12M30)
			imx412_linear_12M30_init(ViPipe);
		else {
		}
	}
	g_pastImx412[ViPipe]->bInit = CVI_TRUE;
}

// void imx412_exit(VI_PIPE ViPipe)
// {
// 	imx412_i2c_exit(ViPipe);
// }
static void imx412_linear_12M30_init(VI_PIPE ViPipe)
{
	delay_ms(30);
	imx412_write_register(ViPipe, 0x0136, 0x18);// HMAX[15:0]
	imx412_write_register(ViPipe, 0x0137, 0x00);//
	imx412_write_register(ViPipe, 0x3C7E, 0x01);// INCKSEL 1[8:0]
	imx412_write_register(ViPipe, 0x3C7F, 0x02);// INCKSEL2[1:0]
	imx412_write_register(ViPipe, 0x38A8, 0x1F);// INCKSEL4[1:0]
	imx412_write_register(ViPipe, 0x38A9, 0xFF);// SYS_MODE
	imx412_write_register(ViPipe, 0x38AA, 0x1F);// XVS_DRV[1:0]
	imx412_write_register(ViPipe, 0x38AB, 0xFF);// -
	imx412_write_register(ViPipe, 0x55D4, 0x00);// -
	imx412_write_register(ViPipe, 0x55D5, 0x00);// -
	imx412_write_register(ViPipe, 0x55D6, 0x07);// -
	imx412_write_register(ViPipe, 0x55D7, 0xFF);// -
	imx412_write_register(ViPipe, 0x55E8, 0x07);// -
	imx412_write_register(ViPipe, 0x55E9, 0xFF);// -
	imx412_write_register(ViPipe, 0x55EA, 0x00);// -
	imx412_write_register(ViPipe, 0x55EB, 0x00);// -
	imx412_write_register(ViPipe, 0x575C, 0x07);// -
	imx412_write_register(ViPipe, 0x575D, 0xFF);// -
	imx412_write_register(ViPipe, 0x575E, 0x00);// -
	imx412_write_register(ViPipe, 0x575F, 0x00);// -
	imx412_write_register(ViPipe, 0x5764, 0x00);// -
	imx412_write_register(ViPipe, 0x5765, 0x00);// -
	imx412_write_register(ViPipe, 0x5766, 0x07);// -
	imx412_write_register(ViPipe, 0x5767, 0xFF);// -
	imx412_write_register(ViPipe, 0x5974, 0x04);// -
	imx412_write_register(ViPipe, 0x5975, 0x01);// -
	imx412_write_register(ViPipe, 0x5F10, 0x09);// -
	imx412_write_register(ViPipe, 0x5F11, 0x92);// -
	imx412_write_register(ViPipe, 0x5F12, 0x32);// -
	imx412_write_register(ViPipe, 0x5F13, 0x72);// -
	imx412_write_register(ViPipe, 0x5F14, 0x16);// -
	imx412_write_register(ViPipe, 0x5F15, 0xBA);// -
	imx412_write_register(ViPipe, 0x5F17, 0x13);// -
	imx412_write_register(ViPipe, 0x5F18, 0x24);// -
	imx412_write_register(ViPipe, 0x5F19, 0x60);// -
	imx412_write_register(ViPipe, 0x5F1A, 0xE3);// -
	imx412_write_register(ViPipe, 0x5F1B, 0xAD);// -
	imx412_write_register(ViPipe, 0x5F1C, 0x74);// -
	imx412_write_register(ViPipe, 0x5F2D, 0x25);// -
	imx412_write_register(ViPipe, 0x5F5C, 0xD0);// -
	imx412_write_register(ViPipe, 0x6A22, 0x00);// -
	imx412_write_register(ViPipe, 0x6A23, 0x1D);// -
	imx412_write_register(ViPipe, 0x7BA8, 0x00);// -
	imx412_write_register(ViPipe, 0x7BA9, 0x00);// -
	imx412_write_register(ViPipe, 0x886B, 0x00);// -
	imx412_write_register(ViPipe, 0x9002, 0x0A);// -
	imx412_write_register(ViPipe, 0x9004, 0x1A);// -
	imx412_write_register(ViPipe, 0x9214, 0x93);// -
	imx412_write_register(ViPipe, 0x9215, 0x69);// -
	imx412_write_register(ViPipe, 0x9216, 0x93);// -
	imx412_write_register(ViPipe, 0x9217, 0x6B);// -
	imx412_write_register(ViPipe, 0x9218, 0x93);// -
	imx412_write_register(ViPipe, 0x9219, 0x6D);// -
	imx412_write_register(ViPipe, 0x921A, 0x57);// -
	imx412_write_register(ViPipe, 0x921B, 0x58);// -
	imx412_write_register(ViPipe, 0x921C, 0x57);// -
	imx412_write_register(ViPipe, 0x921D, 0x59);// -
	imx412_write_register(ViPipe, 0x921E, 0x57);// -
	imx412_write_register(ViPipe, 0x921F, 0x5A);// -
	imx412_write_register(ViPipe, 0x9220, 0x57);// -
	imx412_write_register(ViPipe, 0x9221, 0x5B);// -
	imx412_write_register(ViPipe, 0x9222, 0x93);// -
	imx412_write_register(ViPipe, 0x9223, 0x02);// -
	imx412_write_register(ViPipe, 0x9224, 0x93);// -
	imx412_write_register(ViPipe, 0x9225, 0x03);// -
	imx412_write_register(ViPipe, 0x9226, 0x93);// -
	imx412_write_register(ViPipe, 0x9227, 0x04);// -
	imx412_write_register(ViPipe, 0x9228, 0x93);// -
	imx412_write_register(ViPipe, 0x9229, 0x05);// -
	imx412_write_register(ViPipe, 0x922A, 0x98);// -
	imx412_write_register(ViPipe, 0x922B, 0x21);// -
	imx412_write_register(ViPipe, 0x922C, 0xB2);// -
	imx412_write_register(ViPipe, 0x922D, 0xDB);// TCLKPOST[15:0]
	imx412_write_register(ViPipe, 0x922E, 0xB2);// TCLKPREPARE[15:0]
	imx412_write_register(ViPipe, 0x922F, 0xDC);// TCLKTRAIL[15:0]
	imx412_write_register(ViPipe, 0x9230, 0xB2);// TCLKZERO[15:0]
	imx412_write_register(ViPipe, 0x9231, 0xDD);// THSPREPARE[15:0]
	imx412_write_register(ViPipe, 0x9232, 0xB2);// THSZERO[15:0]
	imx412_write_register(ViPipe, 0x9233, 0xE1);// THSTRAIL[15:0]
	imx412_write_register(ViPipe, 0x9234, 0xB2);// THSEXIT[15:0]
	imx412_write_register(ViPipe, 0x9235, 0xE2);// TLPX[15:0]
	imx412_write_register(ViPipe, 0x9236, 0xB2);// -
	imx412_write_register(ViPipe, 0x9237, 0xE3); // STANDBY
	imx412_write_register(ViPipe, 0x9238, 0xB7); // XTMSTA
	imx412_write_register(ViPipe, 0x9239, 0xB9);// BCWAIT_TIME[7:0]
	imx412_write_register(ViPipe, 0x923A, 0xB7);// CPWAIT_TIME[7:0]
	imx412_write_register(ViPipe, 0x923B, 0xBB);// HMAX[15:0]
	imx412_write_register(ViPipe, 0x923C, 0xB7);//
	imx412_write_register(ViPipe, 0x923D, 0xBC);// INCKSEL 1[8:0]
	imx412_write_register(ViPipe, 0x923E, 0xB7);// INCKSEL2[1:0]
	imx412_write_register(ViPipe, 0x923F, 0xC5);// INCKSEL4[1:0]
	imx412_write_register(ViPipe, 0x9240, 0xB7);// SYS_MODE
	imx412_write_register(ViPipe, 0x9241, 0xC7);// XVS_DRV[1:0]
	imx412_write_register(ViPipe, 0x9242, 0xB7);// -
	imx412_write_register(ViPipe, 0x9243, 0xC9);// -
	imx412_write_register(ViPipe, 0x9244, 0x98);// -
	imx412_write_register(ViPipe, 0x9245, 0x56);// -
	imx412_write_register(ViPipe, 0x9246, 0x98);// -
	imx412_write_register(ViPipe, 0x9247, 0x55);// -
	imx412_write_register(ViPipe, 0x9380, 0x00);// -
	imx412_write_register(ViPipe, 0x9381, 0x62);// -
	imx412_write_register(ViPipe, 0x9382, 0x00);// -
	imx412_write_register(ViPipe, 0x9383, 0x56);// -
	imx412_write_register(ViPipe, 0x9384, 0x00);// -
	imx412_write_register(ViPipe, 0x9385, 0x52);// -
	imx412_write_register(ViPipe, 0x9388, 0x00);// -
	imx412_write_register(ViPipe, 0x9389, 0x55);// -
	imx412_write_register(ViPipe, 0x938A, 0x00);// -
	imx412_write_register(ViPipe, 0x938B, 0x55);// -
	imx412_write_register(ViPipe, 0x938C, 0x00);// -
	imx412_write_register(ViPipe, 0x938D, 0x41);// -
	imx412_write_register(ViPipe, 0x5078, 0x01);// -
	imx412_write_register(ViPipe, 0x9827, 0x20);// -
	imx412_write_register(ViPipe, 0x9830, 0x0A);// -
	imx412_write_register(ViPipe, 0x9833, 0x0A);// -
	imx412_write_register(ViPipe, 0x9834, 0x32);// -
	imx412_write_register(ViPipe, 0x9837, 0x22);// -
	imx412_write_register(ViPipe, 0x983C, 0x04);// -
	imx412_write_register(ViPipe, 0x983F, 0x0A);// -
	imx412_write_register(ViPipe, 0x994F, 0x00);// -
	imx412_write_register(ViPipe, 0x9A48, 0x06);// -
	imx412_write_register(ViPipe, 0x9A49, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4A, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4B, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4E, 0x03);// -
	imx412_write_register(ViPipe, 0x9A4F, 0x03);// -
	imx412_write_register(ViPipe, 0x9A54, 0x03);// -
	imx412_write_register(ViPipe, 0x9A66, 0x03);// -
	imx412_write_register(ViPipe, 0x9A67, 0x03);// -
	imx412_write_register(ViPipe, 0xA2C9, 0x02);// -
	imx412_write_register(ViPipe, 0xA2CB, 0x02);// -
	imx412_write_register(ViPipe, 0xA2CD, 0x02);// -
	imx412_write_register(ViPipe, 0xB249, 0x3F);// -
	imx412_write_register(ViPipe, 0xB24F, 0x3F);// -
	imx412_write_register(ViPipe, 0xB290, 0x3F);// -
	imx412_write_register(ViPipe, 0xB293, 0x3F);// -
	imx412_write_register(ViPipe, 0xB296, 0x3F);// -
	imx412_write_register(ViPipe, 0xB299, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A2, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A8, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A9, 0x0D);// -
	imx412_write_register(ViPipe, 0xB2AA, 0x0D);// -
	imx412_write_register(ViPipe, 0xB2AB, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2BA, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BB, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BC, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BD, 0x10);// -
	imx412_write_register(ViPipe, 0xB2C0, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2C3, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2D2, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2DE, 0x20);// -
	imx412_write_register(ViPipe, 0xB2DF, 0x20);// -
	imx412_write_register(ViPipe, 0xB2E0, 0x20);// -
	imx412_write_register(ViPipe, 0xB2EA, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2ED, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2EE, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2EF, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2F0, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2F1, 0x2F);// TCLKPOST[15:0]
	imx412_write_register(ViPipe, 0xB2F2, 0x2F);// TCLKPREPARE[15:0]
	imx412_write_register(ViPipe, 0xB2F9, 0x0E);// TCLKTRAIL[15:0]
	imx412_write_register(ViPipe, 0xB2FA, 0x0E);// TCLKZERO[15:0]
	imx412_write_register(ViPipe, 0xB2FB, 0x0E);// THSPREPARE[15:0]
	imx412_write_register(ViPipe, 0xB759, 0x01);// THSZERO[15:0]
	imx412_write_register(ViPipe, 0xB765, 0x3F);// THSTRAIL[15:0]
	imx412_write_register(ViPipe, 0xB76B, 0x3F);// THSEXIT[15:0]
	imx412_write_register(ViPipe, 0xB7B3, 0x03);// TLPX[15:0]
	imx412_write_register(ViPipe, 0xB7B5, 0x03);// -
	imx412_write_register(ViPipe, 0xB7B7, 0x03); // STANDBY
	imx412_write_register(ViPipe, 0xB7BF, 0x03); // XTMSTA
	imx412_write_register(ViPipe, 0xB7C1, 0x03);// BCWAIT_TIME[7:0]
	imx412_write_register(ViPipe, 0xB7C3, 0x03);// CPWAIT_TIME[7:0]
	imx412_write_register(ViPipe, 0xB7EF, 0x02);// HMAX[15:0]
	imx412_write_register(ViPipe, 0xB7F5, 0x1F);//
	imx412_write_register(ViPipe, 0xB7F7, 0x1F);// INCKSEL 1[8:0]
	imx412_write_register(ViPipe, 0xB7F9, 0x1F);// INCKSEL2[1:0]

	imx412_write_register(ViPipe, 0x0112, 0x0A);// INCKSEL4[1:0]
	imx412_write_register(ViPipe, 0x0113, 0x0A);// SYS_MODE
	imx412_write_register(ViPipe, 0x0114, 0x03);// XVS_DRV[1:0]
	imx412_write_register(ViPipe, 0x0342, 0x11);// -
	imx412_write_register(ViPipe, 0x0343, 0xA0);// -
	imx412_write_register(ViPipe, 0x0340, 0x0D);// -
	imx412_write_register(ViPipe, 0x0341, 0xDA);// -
	imx412_write_register(ViPipe, 0x3210, 0x00);// -
	imx412_write_register(ViPipe, 0x0344, 0x00);// -
	imx412_write_register(ViPipe, 0x0345, 0x00);// -
	imx412_write_register(ViPipe, 0x0346, 0x00);// -
	imx412_write_register(ViPipe, 0x0347, 0x00);// -
	imx412_write_register(ViPipe, 0x0348, 0x0F);// -
	imx412_write_register(ViPipe, 0x0349, 0xD7);// -
	imx412_write_register(ViPipe, 0x034A, 0x0B);// -
	imx412_write_register(ViPipe, 0x034B, 0xDF);// -
	imx412_write_register(ViPipe, 0x00E3, 0x00);// -
	imx412_write_register(ViPipe, 0x00E4, 0x00);// -
	imx412_write_register(ViPipe, 0x00E5, 0x01);// -
	imx412_write_register(ViPipe, 0x00FC, 0x0A);// -
	imx412_write_register(ViPipe, 0x00FD, 0x0A);// -
	imx412_write_register(ViPipe, 0x00FE, 0x0A);// -
	imx412_write_register(ViPipe, 0x00FF, 0x0A);// -
	imx412_write_register(ViPipe, 0xE013, 0x00);// -
	imx412_write_register(ViPipe, 0x0220, 0x00);// -
	imx412_write_register(ViPipe, 0x0221, 0x11);// -
	imx412_write_register(ViPipe, 0x0381, 0x01);// -
	imx412_write_register(ViPipe, 0x0383, 0x01);// -
	imx412_write_register(ViPipe, 0x0385, 0x01);// -
	imx412_write_register(ViPipe, 0x0387, 0x01);// -
	imx412_write_register(ViPipe, 0x0900, 0x00);// -
	imx412_write_register(ViPipe, 0x0901, 0x11);// -
	imx412_write_register(ViPipe, 0x0902, 0x00);// -
	imx412_write_register(ViPipe, 0x3140, 0x02);// -
	imx412_write_register(ViPipe, 0x3241, 0x11);// -
	imx412_write_register(ViPipe, 0x3250, 0x03);// -
	imx412_write_register(ViPipe, 0x3E10, 0x00);// -
	imx412_write_register(ViPipe, 0x3E11, 0x00);// -
	imx412_write_register(ViPipe, 0x3F0D, 0x00);// -
	imx412_write_register(ViPipe, 0x3F42, 0x00);// -
	imx412_write_register(ViPipe, 0x3F43, 0x00);// -
	imx412_write_register(ViPipe, 0x0401, 0x00);// -
	imx412_write_register(ViPipe, 0x0404, 0x00);// -
	imx412_write_register(ViPipe, 0x0405, 0x10);// -
	imx412_write_register(ViPipe, 0x0408, 0x00);// -
	imx412_write_register(ViPipe, 0x0409, 0x00);// -
	imx412_write_register(ViPipe, 0x040A, 0x00);// -
	imx412_write_register(ViPipe, 0x040B, 0x00);// -
	imx412_write_register(ViPipe, 0x040C, 0x0F);// -
	imx412_write_register(ViPipe, 0x040D, 0xD8);// -
	imx412_write_register(ViPipe, 0x040E, 0x0B);// -
	imx412_write_register(ViPipe, 0x040F, 0xE0);// -
	imx412_write_register(ViPipe, 0x034C, 0x0F);// -
	imx412_write_register(ViPipe, 0x034D, 0xD8);// -
	imx412_write_register(ViPipe, 0x034E, 0x0B);// -
	imx412_write_register(ViPipe, 0x034F, 0xE0);// -
	imx412_write_register(ViPipe, 0x0301, 0x05);// -
	imx412_write_register(ViPipe, 0x0303, 0x02);// -
	imx412_write_register(ViPipe, 0x0305, 0x04);// -
	imx412_write_register(ViPipe, 0x0306, 0x00);// -
	imx412_write_register(ViPipe, 0x0307, 0xC8);// -
	imx412_write_register(ViPipe, 0x0309, 0x0A);// -
	imx412_write_register(ViPipe, 0x030B, 0x01);// -
	imx412_write_register(ViPipe, 0x030D, 0x02);// -
	imx412_write_register(ViPipe, 0x030E, 0x01);// -
	imx412_write_register(ViPipe, 0x030F, 0x5E);// -
	imx412_write_register(ViPipe, 0x0310, 0x00);// -
	imx412_write_register(ViPipe, 0x0820, 0x12);// -
	imx412_write_register(ViPipe, 0x0821, 0xC0);// TCLKPOST[15:0]
	imx412_write_register(ViPipe, 0x0822, 0x00);// TCLKPREPARE[15:0]
	imx412_write_register(ViPipe, 0x0823, 0x00);// TCLKTRAIL[15:0]
	imx412_write_register(ViPipe, 0x3E20, 0x01);// TCLKZERO[15:0]
	imx412_write_register(ViPipe, 0x3E37, 0x00);// THSPREPARE[15:0]
	imx412_write_register(ViPipe, 0x3F50, 0x00);// THSZERO[15:0]
	imx412_write_register(ViPipe, 0x3F56, 0x00);// THSTRAIL[15:0]
	imx412_write_register(ViPipe, 0x3F57, 0xE2);// THSEXIT[15:0]
	imx412_write_register(ViPipe, 0x3C0A, 0x5A);// TLPX[15:0]
	imx412_write_register(ViPipe, 0x3C0B, 0x55);// -
	imx412_write_register(ViPipe, 0x3C0C, 0x28); // STANDBY
	imx412_write_register(ViPipe, 0x3C0D, 0x07); // XTMSTA
	imx412_write_register(ViPipe, 0x3C0E, 0xFF);// BCWAIT_TIME[7:0]
	imx412_write_register(ViPipe, 0x3C0F, 0x00);// CPWAIT_TIME[7:0]
	imx412_write_register(ViPipe, 0x3C10, 0x00);// HMAX[15:0]
	imx412_write_register(ViPipe, 0x3C11, 0x02);//
	imx412_write_register(ViPipe, 0x3C12, 0x00);// INCKSEL 1[8:0]
	imx412_write_register(ViPipe, 0x3C13, 0x03);// INCKSEL2[1:0]
	imx412_write_register(ViPipe, 0x3C14, 0x00);// INCKSEL4[1:0]
	imx412_write_register(ViPipe, 0x3C15, 0x00);// SYS_MODE
	imx412_write_register(ViPipe, 0x3C16, 0x0C);// XVS_DRV[1:0]
	imx412_write_register(ViPipe, 0x3C17, 0x0C);// -
	imx412_write_register(ViPipe, 0x3C18, 0x0C);// -
	imx412_write_register(ViPipe, 0x3C19, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1A, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1B, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1C, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1D, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1E, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1F, 0x00);// -
	imx412_write_register(ViPipe, 0x3C20, 0x00);// -
	imx412_write_register(ViPipe, 0x3C21, 0x00);// -
	imx412_write_register(ViPipe, 0x3C22, 0x3F);// -
	imx412_write_register(ViPipe, 0x3C23, 0x0A);// -
	imx412_write_register(ViPipe, 0x3E35, 0x01);// -
	imx412_write_register(ViPipe, 0x3F4A, 0x03);// -
	imx412_write_register(ViPipe, 0x3F4B, 0xBF);// -
	imx412_write_register(ViPipe, 0x3F26, 0x00);// -


	imx412_write_register(ViPipe, 0x0202, 0x0D);// -
	imx412_write_register(ViPipe, 0x0203, 0xC4);// -
	imx412_write_register(ViPipe, 0x0204, 0x00);// -
	imx412_write_register(ViPipe, 0x0205, 0x00);// -
	imx412_write_register(ViPipe, 0x020E, 0x01);// -
	imx412_write_register(ViPipe, 0x020F, 0x00);// -
	imx412_write_register(ViPipe, 0x0210, 0x01);// -
	imx412_write_register(ViPipe, 0x0211, 0x00);// -
	imx412_write_register(ViPipe, 0x0212, 0x01);// -
	imx412_write_register(ViPipe, 0x0213, 0x00);// -
	imx412_write_register(ViPipe, 0x0214, 0x01);// -
	imx412_write_register(ViPipe, 0x0215, 0x00);// -

	delay_ms(200);
	imx412_write_register(ViPipe, 0x0100, 0x01);// -

	printf("ViPipe:%d,===IMX412 12M 30fps 12bit LINEAR Init OK!===\n", ViPipe);
}

static void imx412_wdr_12M30_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(30);
	imx412_write_register(ViPipe, 0x0136, 0x1B);// - EXCK_FREQ[15:8] INCK-27MHz
	imx412_write_register(ViPipe, 0x0137, 0x00);// - EXCK_FREQ[7:0]
	imx412_write_register(ViPipe, 0x3C7E, 0x01);// - Register version
	imx412_write_register(ViPipe, 0x3C7F, 0x02);// -
	imx412_write_register(ViPipe, 0x38A8, 0x1F);// - Global Setting
	imx412_write_register(ViPipe, 0x38A9, 0xFF);// -
	imx412_write_register(ViPipe, 0x38AA, 0x1F);// -
	imx412_write_register(ViPipe, 0x38AB, 0xFF);// -
	imx412_write_register(ViPipe, 0x55D4, 0x00);// -
	imx412_write_register(ViPipe, 0x55D5, 0x00);// -
	imx412_write_register(ViPipe, 0x55D6, 0x07);// -
	imx412_write_register(ViPipe, 0x55D7, 0xFF);// -
	imx412_write_register(ViPipe, 0x55E8, 0x07);// -
	imx412_write_register(ViPipe, 0x55E9, 0xFF);// -
	imx412_write_register(ViPipe, 0x55EA, 0x00);// -
	imx412_write_register(ViPipe, 0x55EB, 0x00);// -
	imx412_write_register(ViPipe, 0x575C, 0x07);// -
	imx412_write_register(ViPipe, 0x575D, 0xFF);// -
	imx412_write_register(ViPipe, 0x575E, 0x00);// -
	imx412_write_register(ViPipe, 0x575F, 0x00);// -
	imx412_write_register(ViPipe, 0x5764, 0x00);// -
	imx412_write_register(ViPipe, 0x5765, 0x00);// -
	imx412_write_register(ViPipe, 0x5766, 0x07);// -
	imx412_write_register(ViPipe, 0x5767, 0xFF);// -
	imx412_write_register(ViPipe, 0x5974, 0x04);// -
	imx412_write_register(ViPipe, 0x5975, 0x01);// -
	imx412_write_register(ViPipe, 0x5F10, 0x09);// -
	imx412_write_register(ViPipe, 0x5F11, 0x92);// -
	imx412_write_register(ViPipe, 0x5F12, 0x32);// -
	imx412_write_register(ViPipe, 0x5F13, 0x72);// -
	imx412_write_register(ViPipe, 0x5F14, 0x16);// -
	imx412_write_register(ViPipe, 0x5F15, 0xBA);// -
	imx412_write_register(ViPipe, 0x5F17, 0x13);// -
	imx412_write_register(ViPipe, 0x5F18, 0x24);// -
	imx412_write_register(ViPipe, 0x5F19, 0x60);// -
	imx412_write_register(ViPipe, 0x5F1A, 0xE3);// -
	imx412_write_register(ViPipe, 0x5F1B, 0xAD);// -
	imx412_write_register(ViPipe, 0x5F1C, 0x74);// -
	imx412_write_register(ViPipe, 0x5F2D, 0x25);// -
	imx412_write_register(ViPipe, 0x5F5C, 0xD0);// -
	imx412_write_register(ViPipe, 0x6A22, 0x00);// -
	imx412_write_register(ViPipe, 0x6A23, 0x1D);// -
	imx412_write_register(ViPipe, 0x7BA8, 0x00);// -
	imx412_write_register(ViPipe, 0x7BA9, 0x00);// -
	imx412_write_register(ViPipe, 0x886B, 0x00);// -
	imx412_write_register(ViPipe, 0x9002, 0x0A);// -
	imx412_write_register(ViPipe, 0x9004, 0x1A);// -
	imx412_write_register(ViPipe, 0x9214, 0x93);// -
	imx412_write_register(ViPipe, 0x9215, 0x69);// -
	imx412_write_register(ViPipe, 0x9216, 0x93);// -
	imx412_write_register(ViPipe, 0x9217, 0x6B);// -
	imx412_write_register(ViPipe, 0x9218, 0x93);// -
	imx412_write_register(ViPipe, 0x9219, 0x6D);// -
	imx412_write_register(ViPipe, 0x921A, 0x57);// -
	imx412_write_register(ViPipe, 0x921B, 0x58);// -
	imx412_write_register(ViPipe, 0x921C, 0x57);// -
	imx412_write_register(ViPipe, 0x921D, 0x59);// -
	imx412_write_register(ViPipe, 0x921E, 0x57);// -
	imx412_write_register(ViPipe, 0x921F, 0x5A);// -
	imx412_write_register(ViPipe, 0x9220, 0x57);// -
	imx412_write_register(ViPipe, 0x9221, 0x5B);// -
	imx412_write_register(ViPipe, 0x9222, 0x93);// -
	imx412_write_register(ViPipe, 0x9223, 0x02);// -
	imx412_write_register(ViPipe, 0x9224, 0x93);// -
	imx412_write_register(ViPipe, 0x9225, 0x03);// -
	imx412_write_register(ViPipe, 0x9226, 0x93);// -
	imx412_write_register(ViPipe, 0x9227, 0x04);// -
	imx412_write_register(ViPipe, 0x9228, 0x93);// -
	imx412_write_register(ViPipe, 0x9229, 0x05);// -
	imx412_write_register(ViPipe, 0x922A, 0x98);// -
	imx412_write_register(ViPipe, 0x922B, 0x21);// -
	imx412_write_register(ViPipe, 0x922C, 0xB2);// -
	imx412_write_register(ViPipe, 0x922D, 0xDB);// -
	imx412_write_register(ViPipe, 0x922E, 0xB2);// -
	imx412_write_register(ViPipe, 0x922F, 0xDC);// -
	imx412_write_register(ViPipe, 0x9230, 0xB2);// -
	imx412_write_register(ViPipe, 0x9231, 0xDD);// -
	imx412_write_register(ViPipe, 0x9232, 0xB2);// -
	imx412_write_register(ViPipe, 0x9233, 0xE1);// -
	imx412_write_register(ViPipe, 0x9234, 0xB2);// -
	imx412_write_register(ViPipe, 0x9235, 0xE2);// -
	imx412_write_register(ViPipe, 0x9236, 0xB2);// -
	imx412_write_register(ViPipe, 0x9237, 0xE3);// -
	imx412_write_register(ViPipe, 0x9238, 0xB7);// -
	imx412_write_register(ViPipe, 0x9239, 0xB9);// -
	imx412_write_register(ViPipe, 0x923A, 0xB7);// -
	imx412_write_register(ViPipe, 0x923B, 0xBB);// -
	imx412_write_register(ViPipe, 0x923C, 0xB7);// -
	imx412_write_register(ViPipe, 0x923D, 0xBC);// -
	imx412_write_register(ViPipe, 0x923E, 0xB7);// -
	imx412_write_register(ViPipe, 0x923F, 0xC5);// -
	imx412_write_register(ViPipe, 0x9240, 0xB7);// -
	imx412_write_register(ViPipe, 0x9241, 0xC7);// -
	imx412_write_register(ViPipe, 0x9242, 0xB7);// -
	imx412_write_register(ViPipe, 0x9243, 0xC9);// -
	imx412_write_register(ViPipe, 0x9244, 0x98);// -
	imx412_write_register(ViPipe, 0x9245, 0x56);// -
	imx412_write_register(ViPipe, 0x9246, 0x98);// -
	imx412_write_register(ViPipe, 0x9247, 0x55);// -
	imx412_write_register(ViPipe, 0x9380, 0x00);// -
	imx412_write_register(ViPipe, 0x9381, 0x62);// -
	imx412_write_register(ViPipe, 0x9382, 0x00);// -
	imx412_write_register(ViPipe, 0x9383, 0x56);// -
	imx412_write_register(ViPipe, 0x9384, 0x00);// -
	imx412_write_register(ViPipe, 0x9385, 0x52);// -
	imx412_write_register(ViPipe, 0x9388, 0x00);// -
	imx412_write_register(ViPipe, 0x9389, 0x55);// -
	imx412_write_register(ViPipe, 0x938A, 0x00);// -
	imx412_write_register(ViPipe, 0x938B, 0x55);// -
	imx412_write_register(ViPipe, 0x938C, 0x00);// -
	imx412_write_register(ViPipe, 0x938D, 0x41);// -
	imx412_write_register(ViPipe, 0x5078, 0x01);// -
	imx412_write_register(ViPipe, 0x9827, 0x20);// - Image Quality adjustment setting
	imx412_write_register(ViPipe, 0x9830, 0x0A);// -
	imx412_write_register(ViPipe, 0x9833, 0x0A);// -
	imx412_write_register(ViPipe, 0x9834, 0x32);// -
	imx412_write_register(ViPipe, 0x9837, 0x22);// -
	imx412_write_register(ViPipe, 0x983C, 0x04);// -
	imx412_write_register(ViPipe, 0x983F, 0x0A);// -
	imx412_write_register(ViPipe, 0x994F, 0x00);// -
	imx412_write_register(ViPipe, 0x9A48, 0x06);// -
	imx412_write_register(ViPipe, 0x9A49, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4A, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4B, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4E, 0x03);// -
	imx412_write_register(ViPipe, 0x9A4F, 0x03);// -
	imx412_write_register(ViPipe, 0x9A54, 0x03);// -
	imx412_write_register(ViPipe, 0x9A66, 0x03);// -
	imx412_write_register(ViPipe, 0x9A67, 0x03);// -
	imx412_write_register(ViPipe, 0xA2C9, 0x02);// -
	imx412_write_register(ViPipe, 0xA2CB, 0x02);// -
	imx412_write_register(ViPipe, 0xA2CD, 0x02);// -
	imx412_write_register(ViPipe, 0xB249, 0x3F);// -
	imx412_write_register(ViPipe, 0xB24F, 0x3F);// -
	imx412_write_register(ViPipe, 0xB290, 0x3F);// -
	imx412_write_register(ViPipe, 0xB293, 0x3F);// -
	imx412_write_register(ViPipe, 0xB296, 0x3F);// -
	imx412_write_register(ViPipe, 0xB299, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A2, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A8, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A9, 0x0D);// -
	imx412_write_register(ViPipe, 0xB2AA, 0x0D);// -
	imx412_write_register(ViPipe, 0xB2AB, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2BA, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BB, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BC, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BD, 0x10);// -
	imx412_write_register(ViPipe, 0xB2C0, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2C3, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2D2, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2DE, 0x20);// -
	imx412_write_register(ViPipe, 0xB2DF, 0x20);// -
	imx412_write_register(ViPipe, 0xB2E0, 0x20);// -
	imx412_write_register(ViPipe, 0xB2EA, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2ED, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2EE, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2EF, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2F0, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2F1, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2F2, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2F9, 0x0E);// -
	imx412_write_register(ViPipe, 0xB2FA, 0x0E);// -
	imx412_write_register(ViPipe, 0xB2FB, 0x0E);// -
	imx412_write_register(ViPipe, 0xB759, 0x01);// -
	imx412_write_register(ViPipe, 0xB765, 0x3F);// -
	imx412_write_register(ViPipe, 0xB76B, 0x3F);// -
	imx412_write_register(ViPipe, 0xB7B3, 0x03);// -
	imx412_write_register(ViPipe, 0xB7B5, 0x03);// -
	imx412_write_register(ViPipe, 0xB7B7, 0x03);// -
	imx412_write_register(ViPipe, 0xB7BF, 0x03);// -
	imx412_write_register(ViPipe, 0xB7C1, 0x03);// -
	imx412_write_register(ViPipe, 0xB7C3, 0x03);// -
	imx412_write_register(ViPipe, 0xB7EF, 0x02);// -
	imx412_write_register(ViPipe, 0xB7F5, 0x1F);// -
	imx412_write_register(ViPipe, 0xB7F7, 0x1F);// -
	imx412_write_register(ViPipe, 0xB7F9, 0x1F);// -
/*MIPi Setting */
	imx412_write_register(ViPipe, 0x0112, 0x0A);// - RAW10
	imx412_write_register(ViPipe, 0x0113, 0x0A);// - RAW10
	imx412_write_register(ViPipe, 0x0114, 0x03);// - 4Lane
	imx412_write_register(ViPipe, 0x0342, 0x11);// - Frame Horizontal Clock Count
	imx412_write_register(ViPipe, 0x0343, 0xA0);// -
	imx412_write_register(ViPipe, 0x0340, 0x0C);// - Frame Vertical Clock Count
	imx412_write_register(ViPipe, 0x0341, 0x1F);// -
	imx412_write_register(ViPipe, 0x3210, 0x00);// -
	imx412_write_register(ViPipe, 0x0344, 0x00);// - Visible Size
	imx412_write_register(ViPipe, 0x0345, 0x00);// -
	imx412_write_register(ViPipe, 0x0346, 0x00);// -
	imx412_write_register(ViPipe, 0x0347, 0x00);// -
	imx412_write_register(ViPipe, 0x0348, 0x0F);// -
	imx412_write_register(ViPipe, 0x0349, 0xD7);// -
	imx412_write_register(ViPipe, 0x034A, 0x0B);// -
	imx412_write_register(ViPipe, 0x034B, 0xDF);// -
/* Mode Setting  */
	imx412_write_register(ViPipe, 0x00E3, 0x01);// - DOL-HDR Enable
	imx412_write_register(ViPipe, 0x00E4, 0x01);// - 2 frames to 1
	imx412_write_register(ViPipe, 0x00E5, 0x00);// - virtual channel mode
	imx412_write_register(ViPipe, 0x00FC, 0x0A);// - RAW10
	imx412_write_register(ViPipe, 0x00FD, 0x0A);// -
	imx412_write_register(ViPipe, 0x00FE, 0x0A);// -
	imx412_write_register(ViPipe, 0x00FF, 0x0A);// -
	imx412_write_register(ViPipe, 0xE013, 0x01);// -
	imx412_write_register(ViPipe, 0x0220, 0x00);// -
	imx412_write_register(ViPipe, 0x0221, 0x11);// -
	imx412_write_register(ViPipe, 0x0381, 0x01);// -
	imx412_write_register(ViPipe, 0x0383, 0x01);// -
	imx412_write_register(ViPipe, 0x0385, 0x01);// -
	imx412_write_register(ViPipe, 0x0387, 0x01);// -
	imx412_write_register(ViPipe, 0x0900, 0x00);// -
	imx412_write_register(ViPipe, 0x0901, 0x11);// -
	imx412_write_register(ViPipe, 0x0902, 0x00);// -
	imx412_write_register(ViPipe, 0x3140, 0x02);// -
	imx412_write_register(ViPipe, 0x3241, 0x11);// -
	imx412_write_register(ViPipe, 0x3250, 0x03);// -
	imx412_write_register(ViPipe, 0x3E10, 0x01);// -
	imx412_write_register(ViPipe, 0x3E11, 0x02);// -
	imx412_write_register(ViPipe, 0x3F0D, 0x00);// -
	imx412_write_register(ViPipe, 0x3F42, 0x00);// -
	imx412_write_register(ViPipe, 0x3F43, 0x00);// -
	imx412_write_register(ViPipe, 0x0401, 0x00);// - Digital Crop & Scaling
	imx412_write_register(ViPipe, 0x0404, 0x00);// -
	imx412_write_register(ViPipe, 0x0405, 0x10);// -
	imx412_write_register(ViPipe, 0x0408, 0x00);// -
	imx412_write_register(ViPipe, 0x0409, 0x00);// -
	imx412_write_register(ViPipe, 0x040A, 0x00);// -
	imx412_write_register(ViPipe, 0x040B, 0x00);// -
	imx412_write_register(ViPipe, 0x040C, 0x0F);// -
	imx412_write_register(ViPipe, 0x040D, 0xD8);// -
	imx412_write_register(ViPipe, 0x040E, 0x0B);// -
	imx412_write_register(ViPipe, 0x040F, 0xE0);// -
	imx412_write_register(ViPipe, 0x034C, 0x0F);// - Output Crop
	imx412_write_register(ViPipe, 0x034D, 0xD8);// -
	imx412_write_register(ViPipe, 0x034E, 0x0B);// -
	imx412_write_register(ViPipe, 0x034F, 0xE0);// -
	/*Clock Setting */
	imx412_write_register(ViPipe, 0x0301, 0x05);// - IVT_PXCK_DIV
	imx412_write_register(ViPipe, 0x0303, 0x02);// - IVT_SYCK_DIV
	imx412_write_register(ViPipe, 0x0305, 0x04);// -
	imx412_write_register(ViPipe, 0x0306, 0x01);// -
	imx412_write_register(ViPipe, 0x0307, 0x37);// -
	imx412_write_register(ViPipe, 0x0309, 0x0A);// -
	imx412_write_register(ViPipe, 0x030B, 0x01);// -
	imx412_write_register(ViPipe, 0x030D, 0x02);// -
	imx412_write_register(ViPipe, 0x030E, 0x01);// -
	imx412_write_register(ViPipe, 0x030F, 0x5E);// -
	imx412_write_register(ViPipe, 0x0310, 0x00);// -
	imx412_write_register(ViPipe, 0x0820, 0x20);// -
	imx412_write_register(ViPipe, 0x0821, 0xCD);// -
	imx412_write_register(ViPipe, 0x0822, 0x00);// -
	imx412_write_register(ViPipe, 0x0823, 0x00);// -
	imx412_write_register(ViPipe, 0x3E20, 0x01);// - Output Data Select Setting
	imx412_write_register(ViPipe, 0x3E37, 0x00);// -
	imx412_write_register(ViPipe, 0x3F50, 0x00);// - PowerSave Setting
	imx412_write_register(ViPipe, 0x3F56, 0x00);// -
	imx412_write_register(ViPipe, 0x3F57, 0x92);// -
	imx412_write_register(ViPipe, 0x3C0A, 0x5A);// - Other Setting
	imx412_write_register(ViPipe, 0x3C0B, 0x55);// -
	imx412_write_register(ViPipe, 0x3C0C, 0x28);// -
	imx412_write_register(ViPipe, 0x3C0D, 0x07);// -
	imx412_write_register(ViPipe, 0x3C0E, 0xFF);// -
	imx412_write_register(ViPipe, 0x3C0F, 0x00);// -
	imx412_write_register(ViPipe, 0x3C10, 0x00);// -
	imx412_write_register(ViPipe, 0x3C11, 0x02);// -
	imx412_write_register(ViPipe, 0x3C12, 0x00);// -
	imx412_write_register(ViPipe, 0x3C13, 0x03);// -
	imx412_write_register(ViPipe, 0x3C14, 0x00);// -
	imx412_write_register(ViPipe, 0x3C15, 0x00);// -
	imx412_write_register(ViPipe, 0x3C16, 0x0C);// -
	imx412_write_register(ViPipe, 0x3C17, 0x0C);// -
	imx412_write_register(ViPipe, 0x3C18, 0x0C);// -
	imx412_write_register(ViPipe, 0x3C19, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1A, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1B, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1C, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1D, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1E, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1F, 0x00);// -
	imx412_write_register(ViPipe, 0x3C20, 0x00);// -
	imx412_write_register(ViPipe, 0x3C21, 0x00);// -
	imx412_write_register(ViPipe, 0x3C22, 0x3F);// -
	imx412_write_register(ViPipe, 0x3C23, 0x0A);// -
	imx412_write_register(ViPipe, 0x3E35, 0x01);// -
	imx412_write_register(ViPipe, 0x3F4A, 0x03);// -
	imx412_write_register(ViPipe, 0x3F4B, 0xBF);// -
	imx412_write_register(ViPipe, 0x3F26, 0x00);// -
	imx412_write_register(ViPipe, 0x0202, 0x0C);// - Integration Time Setting
	imx412_write_register(ViPipe, 0x0203, 0x09);// -
	imx412_write_register(ViPipe, 0x0204, 0x00);// - Gain Setting
	imx412_write_register(ViPipe, 0x0205, 0x00);// -
	imx412_write_register(ViPipe, 0x020E, 0x01);// -
	imx412_write_register(ViPipe, 0x020F, 0x00);// -
	imx412_write_register(ViPipe, 0x0210, 0x01);// -
	imx412_write_register(ViPipe, 0x0211, 0x00);// -
	imx412_write_register(ViPipe, 0x0212, 0x01);// -
	imx412_write_register(ViPipe, 0x0213, 0x00);// -
	imx412_write_register(ViPipe, 0x0214, 0x01);// -
	imx412_write_register(ViPipe, 0x0215, 0x00);// -

	imx412_write_register(ViPipe, 0x00ea, 0x00);// -
	imx412_write_register(ViPipe, 0x00eb, 0x00);// -
	imx412_write_register(ViPipe, 0x00ec, 0x00);// -
	imx412_write_register(ViPipe, 0x00ed, 0x00);// -

	imx412_default_reg_init(ViPipe);

	delay_ms(200);
	imx412_write_register(ViPipe, 0x0100, 0x01);// - Streaming

	printf("\nViPipe:%d,===IMX412 12M 30fps 10bit WDR Init OK!===\n", ViPipe);
}


static void imx412_wdr_8M30_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(30);
	imx412_write_register(ViPipe, 0x0136, 0x1B);// - EXCK_FREQ[15:8] INCK-27MHz
	imx412_write_register(ViPipe, 0x0137, 0x00);// - EXCK_FREQ[7:0]
	imx412_write_register(ViPipe, 0x3C7E, 0x02);// - Register version
	imx412_write_register(ViPipe, 0x3C7F, 0x02);// -
	imx412_write_register(ViPipe, 0x38A8, 0x1F);// - Global Setting
	imx412_write_register(ViPipe, 0x38A9, 0xFF);// -
	imx412_write_register(ViPipe, 0x38AA, 0x1F);// -
	imx412_write_register(ViPipe, 0x38AB, 0xFF);// -
	imx412_write_register(ViPipe, 0x55D4, 0x00);// -
	imx412_write_register(ViPipe, 0x55D5, 0x00);// -
	imx412_write_register(ViPipe, 0x55D6, 0x07);// -
	imx412_write_register(ViPipe, 0x55D7, 0xFF);// -
	imx412_write_register(ViPipe, 0x55E8, 0x07);// -
	imx412_write_register(ViPipe, 0x55E9, 0xFF);// -
	imx412_write_register(ViPipe, 0x55EA, 0x00);// -
	imx412_write_register(ViPipe, 0x55EB, 0x00);// -
	imx412_write_register(ViPipe, 0x575C, 0x07);// -
	imx412_write_register(ViPipe, 0x575D, 0xFF);// -
	imx412_write_register(ViPipe, 0x575E, 0x00);// -
	imx412_write_register(ViPipe, 0x575F, 0x00);// -
	imx412_write_register(ViPipe, 0x5764, 0x00);// -
	imx412_write_register(ViPipe, 0x5765, 0x00);// -
	imx412_write_register(ViPipe, 0x5766, 0x07);// -
	imx412_write_register(ViPipe, 0x5767, 0xFF);// -
	imx412_write_register(ViPipe, 0x5974, 0x04);// -
	imx412_write_register(ViPipe, 0x5975, 0x01);// -
	imx412_write_register(ViPipe, 0x5F10, 0x09);// -
	imx412_write_register(ViPipe, 0x5F11, 0x92);// -
	imx412_write_register(ViPipe, 0x5F12, 0x32);// -
	imx412_write_register(ViPipe, 0x5F13, 0x72);// -
	imx412_write_register(ViPipe, 0x5F14, 0x16);// -
	imx412_write_register(ViPipe, 0x5F15, 0xBA);// -
	imx412_write_register(ViPipe, 0x5F17, 0x13);// -
	imx412_write_register(ViPipe, 0x5F18, 0x24);// -
	imx412_write_register(ViPipe, 0x5F19, 0x60);// -
	imx412_write_register(ViPipe, 0x5F1A, 0xE3);// -
	imx412_write_register(ViPipe, 0x5F1B, 0xAD);// -
	imx412_write_register(ViPipe, 0x5F1C, 0x74);// -
	imx412_write_register(ViPipe, 0x5F2D, 0x25);// -
	imx412_write_register(ViPipe, 0x5F5C, 0xD0);// -
	imx412_write_register(ViPipe, 0x6A22, 0x00);// -
	imx412_write_register(ViPipe, 0x6A23, 0x1D);// -
	imx412_write_register(ViPipe, 0x7BA8, 0x00);// -
	imx412_write_register(ViPipe, 0x7BA9, 0x00);// -
	imx412_write_register(ViPipe, 0x886B, 0x00);// -
	imx412_write_register(ViPipe, 0x9002, 0x0A);// -
	imx412_write_register(ViPipe, 0x9004, 0x1A);// -
	imx412_write_register(ViPipe, 0x9214, 0x93);// -
	imx412_write_register(ViPipe, 0x9215, 0x69);// -
	imx412_write_register(ViPipe, 0x9216, 0x93);// -
	imx412_write_register(ViPipe, 0x9217, 0x6B);// -
	imx412_write_register(ViPipe, 0x9218, 0x93);// -
	imx412_write_register(ViPipe, 0x9219, 0x6D);// -
	imx412_write_register(ViPipe, 0x921A, 0x57);// -
	imx412_write_register(ViPipe, 0x921B, 0x58);// -
	imx412_write_register(ViPipe, 0x921C, 0x57);// -
	imx412_write_register(ViPipe, 0x921D, 0x59);// -
	imx412_write_register(ViPipe, 0x921E, 0x57);// -
	imx412_write_register(ViPipe, 0x921F, 0x5A);// -
	imx412_write_register(ViPipe, 0x9220, 0x57);// -
	imx412_write_register(ViPipe, 0x9221, 0x5B);// -
	imx412_write_register(ViPipe, 0x9222, 0x93);// -
	imx412_write_register(ViPipe, 0x9223, 0x02);// -
	imx412_write_register(ViPipe, 0x9224, 0x93);// -
	imx412_write_register(ViPipe, 0x9225, 0x03);// -
	imx412_write_register(ViPipe, 0x9226, 0x93);// -
	imx412_write_register(ViPipe, 0x9227, 0x04);// -
	imx412_write_register(ViPipe, 0x9228, 0x93);// -
	imx412_write_register(ViPipe, 0x9229, 0x05);// -
	imx412_write_register(ViPipe, 0x922A, 0x98);// -
	imx412_write_register(ViPipe, 0x922B, 0x21);// -
	imx412_write_register(ViPipe, 0x922C, 0xB2);// -
	imx412_write_register(ViPipe, 0x922D, 0xDB);// -
	imx412_write_register(ViPipe, 0x922E, 0xB2);// -
	imx412_write_register(ViPipe, 0x922F, 0xDC);// -
	imx412_write_register(ViPipe, 0x9230, 0xB2);// -
	imx412_write_register(ViPipe, 0x9231, 0xDD);// -
	imx412_write_register(ViPipe, 0x9232, 0xB2);// -
	imx412_write_register(ViPipe, 0x9233, 0xE1);// -
	imx412_write_register(ViPipe, 0x9234, 0xB2);// -
	imx412_write_register(ViPipe, 0x9235, 0xE2);// -
	imx412_write_register(ViPipe, 0x9236, 0xB2);// -
	imx412_write_register(ViPipe, 0x9237, 0xE3);// -
	imx412_write_register(ViPipe, 0x9238, 0xB7);// -
	imx412_write_register(ViPipe, 0x9239, 0xB9);// -
	imx412_write_register(ViPipe, 0x923A, 0xB7);// -
	imx412_write_register(ViPipe, 0x923B, 0xBB);// -
	imx412_write_register(ViPipe, 0x923C, 0xB7);// -
	imx412_write_register(ViPipe, 0x923D, 0xBC);// -
	imx412_write_register(ViPipe, 0x923E, 0xB7);// -
	imx412_write_register(ViPipe, 0x923F, 0xC5);// -
	imx412_write_register(ViPipe, 0x9240, 0xB7);// -
	imx412_write_register(ViPipe, 0x9241, 0xC7);// -
	imx412_write_register(ViPipe, 0x9242, 0xB7);// -
	imx412_write_register(ViPipe, 0x9243, 0xC9);// -
	imx412_write_register(ViPipe, 0x9244, 0x98);// -
	imx412_write_register(ViPipe, 0x9245, 0x56);// -
	imx412_write_register(ViPipe, 0x9246, 0x98);// -
	imx412_write_register(ViPipe, 0x9247, 0x55);// -
	imx412_write_register(ViPipe, 0x9380, 0x00);// -
	imx412_write_register(ViPipe, 0x9381, 0x62);// -
	imx412_write_register(ViPipe, 0x9382, 0x00);// -
	imx412_write_register(ViPipe, 0x9383, 0x56);// -
	imx412_write_register(ViPipe, 0x9384, 0x00);// -
	imx412_write_register(ViPipe, 0x9385, 0x52);// -
	imx412_write_register(ViPipe, 0x9388, 0x00);// -
	imx412_write_register(ViPipe, 0x9389, 0x55);// -
	imx412_write_register(ViPipe, 0x938A, 0x00);// -
	imx412_write_register(ViPipe, 0x938B, 0x55);// -
	imx412_write_register(ViPipe, 0x938C, 0x00);// -
	imx412_write_register(ViPipe, 0x938D, 0x41);// -
	imx412_write_register(ViPipe, 0x5078, 0x01);// -

	imx412_write_register(ViPipe, 0x9827, 0x20);// - Image Quality adjustment setting
	imx412_write_register(ViPipe, 0x9830, 0x0A);// -
	imx412_write_register(ViPipe, 0x9833, 0x0A);// -
	imx412_write_register(ViPipe, 0x9834, 0x32);// -
	imx412_write_register(ViPipe, 0x9837, 0x22);// -
	imx412_write_register(ViPipe, 0x983C, 0x04);// -
	imx412_write_register(ViPipe, 0x983F, 0x0A);// -
	imx412_write_register(ViPipe, 0x994F, 0x00);// -
	imx412_write_register(ViPipe, 0x9A48, 0x06);// -
	imx412_write_register(ViPipe, 0x9A49, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4A, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4B, 0x06);// -
	imx412_write_register(ViPipe, 0x9A4E, 0x03);// -
	imx412_write_register(ViPipe, 0x9A4F, 0x03);// -
	imx412_write_register(ViPipe, 0x9A54, 0x03);// -
	imx412_write_register(ViPipe, 0x9A66, 0x03);// -
	imx412_write_register(ViPipe, 0x9A67, 0x03);// -
	imx412_write_register(ViPipe, 0xA2C9, 0x02);// -
	imx412_write_register(ViPipe, 0xA2CB, 0x02);// -
	imx412_write_register(ViPipe, 0xA2CD, 0x02);// -
	imx412_write_register(ViPipe, 0xB249, 0x3F);// -
	imx412_write_register(ViPipe, 0xB24F, 0x3F);// -
	imx412_write_register(ViPipe, 0xB290, 0x3F);// -
	imx412_write_register(ViPipe, 0xB293, 0x3F);// -
	imx412_write_register(ViPipe, 0xB296, 0x3F);// -
	imx412_write_register(ViPipe, 0xB299, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A2, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A8, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2A9, 0x0D);// -
	imx412_write_register(ViPipe, 0xB2AA, 0x0D);// -
	imx412_write_register(ViPipe, 0xB2AB, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2BA, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BB, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BC, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2BD, 0x10);// -
	imx412_write_register(ViPipe, 0xB2C0, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2C3, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2D2, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2DE, 0x20);// -
	imx412_write_register(ViPipe, 0xB2DF, 0x20);// -
	imx412_write_register(ViPipe, 0xB2E0, 0x20);// -
	imx412_write_register(ViPipe, 0xB2EA, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2ED, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2EE, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2EF, 0x3F);// -
	imx412_write_register(ViPipe, 0xB2F0, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2F1, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2F2, 0x2F);// -
	imx412_write_register(ViPipe, 0xB2F9, 0x0E);// -
	imx412_write_register(ViPipe, 0xB2FA, 0x0E);// -
	imx412_write_register(ViPipe, 0xB2FB, 0x0E);// -
	imx412_write_register(ViPipe, 0xB759, 0x01);// -
	imx412_write_register(ViPipe, 0xB765, 0x3F);// -
	imx412_write_register(ViPipe, 0xB76B, 0x3F);// -
	imx412_write_register(ViPipe, 0xB7B3, 0x03);// -
	imx412_write_register(ViPipe, 0xB7B5, 0x03);// -
	imx412_write_register(ViPipe, 0xB7B7, 0x03);// -
	imx412_write_register(ViPipe, 0xB7BF, 0x03);// -
	imx412_write_register(ViPipe, 0xB7C1, 0x03);// -
	imx412_write_register(ViPipe, 0xB7C3, 0x03);// -
	imx412_write_register(ViPipe, 0xB7EF, 0x02);// -
	imx412_write_register(ViPipe, 0xB7F5, 0x1F);// -
	imx412_write_register(ViPipe, 0xB7F7, 0x1F);// -
	imx412_write_register(ViPipe, 0xB7F9, 0x1F);// -
/*MIPi Setting */
	imx412_write_register(ViPipe, 0x0112, 0x0A);// - RAW10
	imx412_write_register(ViPipe, 0x0113, 0x0A);// - RAW10
	imx412_write_register(ViPipe, 0x0114, 0x03);// - 4Lane
	imx412_write_register(ViPipe, 0x0342, 0x11);// - Frame Horizontal Clock Count
	imx412_write_register(ViPipe, 0x0343, 0xA0);// -
	imx412_write_register(ViPipe, 0x0340, 0x0C);// - Frame Vertical Clock Count
	imx412_write_register(ViPipe, 0x0341, 0x1D);// -
	imx412_write_register(ViPipe, 0x3210, 0x00);// -
	imx412_write_register(ViPipe, 0x0344, 0x00);// - Visible Size
	imx412_write_register(ViPipe, 0x0345, 0x00);// -
	imx412_write_register(ViPipe, 0x0346, 0x01);// -
	imx412_write_register(ViPipe, 0x0347, 0x78);// -
	imx412_write_register(ViPipe, 0x0348, 0x0F);// -
	imx412_write_register(ViPipe, 0x0349, 0xD7);// -
	imx412_write_register(ViPipe, 0x034A, 0x0A);// -
	imx412_write_register(ViPipe, 0x034B, 0x67);// -
/* Mode Setting  */
	imx412_write_register(ViPipe, 0x00E3, 0x01);// - DOL-HDR Enable
	imx412_write_register(ViPipe, 0x00E4, 0x01);// - 2 frames to 1
	imx412_write_register(ViPipe, 0x00E5, 0x00);// - virtual channel mode
	imx412_write_register(ViPipe, 0x00FC, 0x0A);// - RAW10
	imx412_write_register(ViPipe, 0x00FD, 0x0A);// -
	imx412_write_register(ViPipe, 0x00FE, 0x0A);// -
	imx412_write_register(ViPipe, 0x00FF, 0x0A);// -
	imx412_write_register(ViPipe, 0xE013, 0x01);// -
	imx412_write_register(ViPipe, 0x0220, 0x00);// -
	imx412_write_register(ViPipe, 0x0221, 0x11);// -
	imx412_write_register(ViPipe, 0x0381, 0x01);// -
	imx412_write_register(ViPipe, 0x0383, 0x01);// -
	imx412_write_register(ViPipe, 0x0385, 0x01);// -
	imx412_write_register(ViPipe, 0x0387, 0x01);// -
	imx412_write_register(ViPipe, 0x0900, 0x00);// -
	imx412_write_register(ViPipe, 0x0901, 0x11);// -
	imx412_write_register(ViPipe, 0x0902, 0x00);// -
	imx412_write_register(ViPipe, 0x3140, 0x02);// -
	imx412_write_register(ViPipe, 0x3241, 0x11);// -
	imx412_write_register(ViPipe, 0x3250, 0x03);// -
	imx412_write_register(ViPipe, 0x3E10, 0x01);// -
	imx412_write_register(ViPipe, 0x3E11, 0x02);// -
	imx412_write_register(ViPipe, 0x3F0D, 0x00);// -
	imx412_write_register(ViPipe, 0x3F42, 0x00);// -
	imx412_write_register(ViPipe, 0x3F43, 0x00);// -
	imx412_write_register(ViPipe, 0x0401, 0x00);// - Digital Crop & Scaling
	imx412_write_register(ViPipe, 0x0404, 0x00);// -
	imx412_write_register(ViPipe, 0x0405, 0x10);// -
	imx412_write_register(ViPipe, 0x0408, 0x00);// -
	imx412_write_register(ViPipe, 0x0409, 0x00);// -
	imx412_write_register(ViPipe, 0x040A, 0x00);// -
	imx412_write_register(ViPipe, 0x040B, 0x00);// -
	imx412_write_register(ViPipe, 0x040C, 0x0F);// -
	imx412_write_register(ViPipe, 0x040D, 0xD8);// -
	imx412_write_register(ViPipe, 0x040E, 0x08);// -
	imx412_write_register(ViPipe, 0x040F, 0xF0);// -
	imx412_write_register(ViPipe, 0x034C, 0x0F);// - Output Crop
	imx412_write_register(ViPipe, 0x034D, 0xDC);// -
	imx412_write_register(ViPipe, 0x034E, 0x08);// -
	imx412_write_register(ViPipe, 0x034F, 0xF0);// -
	/*Clock Setting */
	imx412_write_register(ViPipe, 0x0301, 0x05);// - IVT_PXCK_DIV
	imx412_write_register(ViPipe, 0x0303, 0x02);// - IVT_SYCK_DIV
	imx412_write_register(ViPipe, 0x0305, 0x04);// -
	imx412_write_register(ViPipe, 0x0306, 0x01);// -
	imx412_write_register(ViPipe, 0x0307, 0x37);// -
	imx412_write_register(ViPipe, 0x0309, 0x0A);// -
	imx412_write_register(ViPipe, 0x030B, 0x01);// -
	imx412_write_register(ViPipe, 0x030D, 0x02);// -
	imx412_write_register(ViPipe, 0x030E, 0x01);// -
	imx412_write_register(ViPipe, 0x030F, 0x5E);// -
	imx412_write_register(ViPipe, 0x0310, 0x00);// -
	imx412_write_register(ViPipe, 0x0820, 0x20);// -
	imx412_write_register(ViPipe, 0x0821, 0xCD);// -
	imx412_write_register(ViPipe, 0x0822, 0x00);// -
	imx412_write_register(ViPipe, 0x0823, 0x00);// -
	imx412_write_register(ViPipe, 0x3E20, 0x01);// - Output Data Select Setting
	imx412_write_register(ViPipe, 0x3E37, 0x00);// -
	imx412_write_register(ViPipe, 0x3F50, 0x00);// - PowerSave Setting
	imx412_write_register(ViPipe, 0x3F56, 0x00);// -
	imx412_write_register(ViPipe, 0x3F57, 0x92);// -
	imx412_write_register(ViPipe, 0x3C0A, 0x5A);// - Other Setting
	imx412_write_register(ViPipe, 0x3C0B, 0x55);// -
	imx412_write_register(ViPipe, 0x3C0C, 0x28);// -
	imx412_write_register(ViPipe, 0x3C0D, 0x07);// -
	imx412_write_register(ViPipe, 0x3C0E, 0xFF);// -
	imx412_write_register(ViPipe, 0x3C0F, 0x00);// -
	imx412_write_register(ViPipe, 0x3C10, 0x00);// -
	imx412_write_register(ViPipe, 0x3C11, 0x02);// -
	imx412_write_register(ViPipe, 0x3C12, 0x00);// -
	imx412_write_register(ViPipe, 0x3C13, 0x03);// -
	imx412_write_register(ViPipe, 0x3C14, 0x00);// -
	imx412_write_register(ViPipe, 0x3C15, 0x00);// -
	imx412_write_register(ViPipe, 0x3C16, 0x0C);// -
	imx412_write_register(ViPipe, 0x3C17, 0x0C);// -
	imx412_write_register(ViPipe, 0x3C18, 0x0C);// -
	imx412_write_register(ViPipe, 0x3C19, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1A, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1B, 0x0A);// -
	imx412_write_register(ViPipe, 0x3C1C, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1D, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1E, 0x00);// -
	imx412_write_register(ViPipe, 0x3C1F, 0x00);// -
	imx412_write_register(ViPipe, 0x3C20, 0x00);// -
	imx412_write_register(ViPipe, 0x3C21, 0x00);// -
	imx412_write_register(ViPipe, 0x3C22, 0x3F);// -
	imx412_write_register(ViPipe, 0x3C23, 0x0A);// -
	imx412_write_register(ViPipe, 0x3E35, 0x01);// -
	imx412_write_register(ViPipe, 0x3F4A, 0x03);// -
	imx412_write_register(ViPipe, 0x3F4B, 0xBF);// -
	imx412_write_register(ViPipe, 0x3F26, 0x00);// -
	imx412_write_register(ViPipe, 0x0202, 0x18);// - Integration Time Setting
	imx412_write_register(ViPipe, 0x0202, 0x0C);// -
	imx412_write_register(ViPipe, 0x0203, 0x07);// - Gain Setting
	imx412_write_register(ViPipe, 0x0204, 0x00);
	imx412_write_register(ViPipe, 0x0205, 0x00);// -
	imx412_write_register(ViPipe, 0x020E, 0x01);// -
	imx412_write_register(ViPipe, 0x020F, 0x00);// -
	imx412_write_register(ViPipe, 0x0210, 0x01);// -
	imx412_write_register(ViPipe, 0x0211, 0x00);// -
	imx412_write_register(ViPipe, 0x0212, 0x01);// -
	imx412_write_register(ViPipe, 0x0213, 0x00);// -
	imx412_write_register(ViPipe, 0x0214, 0x01);// -
	imx412_write_register(ViPipe, 0x0215, 0x00);// -

	imx412_write_register(ViPipe, 0x00ea, 0x00);// -
	imx412_write_register(ViPipe, 0x00eb, 0x00);// -
	imx412_write_register(ViPipe, 0x00ec, 0x00);// -
	imx412_write_register(ViPipe, 0x00ed, 0x00);// -

	imx412_default_reg_init(ViPipe);

	delay_ms(200);
	imx412_write_register(ViPipe, 0x0100, 0x01);// - Streaming

	printf("\nViPipe:%d,===IMX412 8M(4K) 30fps 10bit WDR Init OK!===\n", ViPipe);
}