#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx585_cmos_ex.h"
#include "sensor_i2c.h"

static void imx585_linear_8M30_init(VI_PIPE ViPipe);
static void imx585_wdr_8M30_2to1_init(VI_PIPE ViPipe);

CVI_U8 imx585_i2c_addr = 0x36;
const CVI_U32 imx585_addr_byte = 2;
const CVI_U32 imx585_data_byte = 1;

int imx585_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx585_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx585_AddrInfo[ViPipe].s8I2cAddr);
}

int imx585_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx585_BusInfo[ViPipe].s8I2cDev);
}

int imx585_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx585_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx585_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx585_addr_byte, imx585_data_byte);
}

int imx585_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx585_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx585_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx585_addr_byte, (CVI_U32)data, imx585_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx585_standby(VI_PIPE ViPipe)
{
	imx585_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx585_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */
}

void imx585_restart(VI_PIPE ViPipe)
{
	imx585_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx585_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
}

void imx585_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx585[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx585_write_register(ViPipe,
				g_pastImx585[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx585[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx585_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	imx585_write_register(ViPipe, 0x3020, u8Mirror);
	imx585_write_register(ViPipe, 0x3021, u8Filp);
}

void imx585_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx585[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx585[ViPipe]->u8ImgMode;

	imx585_i2c_init(ViPipe);
	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == IMX585_MODE_8M30_WDR) {
			imx585_wdr_8M30_2to1_init(ViPipe);
		}
	} else {
		if (u8ImgMode == IMX585_MODE_8M30)
			imx585_linear_8M30_init(ViPipe);
		else {
		}
	}
	g_pastImx585[ViPipe]->bInit = CVI_TRUE;
}

static void imx585_linear_8M30_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	imx585_write_register(ViPipe, 0x3000, 0x01); //STANDBY
	imx585_write_register(ViPipe, 0x3001, 0x00); //XTMSTA
	imx585_write_register(ViPipe, 0x3002, 0x01);//BCWAIT_TIME[7:0]
	imx585_write_register(ViPipe, 0x3014, 0x01);//CPWAIT_TIME[7:0]
	imx585_write_register(ViPipe, 0x3015, 0x05);//HMAX[15:0]
	imx585_write_register(ViPipe, 0x3018, 0x10);//
	imx585_write_register(ViPipe, 0x301A, 0x00);//INCKSEL 1[8:0]
	imx585_write_register(ViPipe, 0x301B, 0x00);//INCKSEL2[1:0]
	imx585_write_register(ViPipe, 0x301C, 0x00);//INCKSEL4[1:0]
	imx585_write_register(ViPipe, 0x301E, 0x01);//SYS_MODE
	imx585_write_register(ViPipe, 0x3020, 0x01);//XVS_DRV[1:0]
	imx585_write_register(ViPipe, 0x3021, 0x01);//-
	imx585_write_register(ViPipe, 0x3022, 0x02);//-
	imx585_write_register(ViPipe, 0x3023, 0x01);//-
	imx585_write_register(ViPipe, 0x3024, 0x00);//-
	imx585_write_register(ViPipe, 0x3028, 0xCA);//-
	imx585_write_register(ViPipe, 0x3029, 0x08);//-
	imx585_write_register(ViPipe, 0x302A, 0x00);//-
	imx585_write_register(ViPipe, 0x302C, 0x4C);//-
	imx585_write_register(ViPipe, 0x302D, 0x04);//-
	imx585_write_register(ViPipe, 0x3030, 0x00);//-
	imx585_write_register(ViPipe, 0x3031, 0x00);//-
	imx585_write_register(ViPipe, 0x3032, 0x00);//-
	imx585_write_register(ViPipe, 0x303C, 0x00);//-
	imx585_write_register(ViPipe, 0x303D, 0x00);//-
	imx585_write_register(ViPipe, 0x303E, 0x10);//-
	imx585_write_register(ViPipe, 0x303F, 0x0F);//-
	imx585_write_register(ViPipe, 0x3040, 0x03);//-
	imx585_write_register(ViPipe, 0x3042, 0x00);//-
	imx585_write_register(ViPipe, 0x3043, 0x00);//-
	imx585_write_register(ViPipe, 0x3044, 0x00);//-
	imx585_write_register(ViPipe, 0x3045, 0x00);//-
	imx585_write_register(ViPipe, 0x3046, 0x84);//-
	imx585_write_register(ViPipe, 0x3047, 0x08);//-
	imx585_write_register(ViPipe, 0x3050, 0x08);//-
	imx585_write_register(ViPipe, 0x3051, 0x00);//-
	imx585_write_register(ViPipe, 0x3052, 0x00);//-
	imx585_write_register(ViPipe, 0x3054, 0x0E);//-
	imx585_write_register(ViPipe, 0x3055, 0x00);//-
	imx585_write_register(ViPipe, 0x3056, 0x00);//-
	imx585_write_register(ViPipe, 0x3058, 0x8A);//-
	imx585_write_register(ViPipe, 0x3059, 0x01);//-
	imx585_write_register(ViPipe, 0x305A, 0x00);//-
	imx585_write_register(ViPipe, 0x3060, 0x16);//-
	imx585_write_register(ViPipe, 0x3061, 0x01);//-
	imx585_write_register(ViPipe, 0x3062, 0x00);//-
	imx585_write_register(ViPipe, 0x3064, 0xC4);//-
	imx585_write_register(ViPipe, 0x3065, 0x0C);//-
	imx585_write_register(ViPipe, 0x3066, 0x00);//-
	imx585_write_register(ViPipe, 0x3069, 0x00);//-
	imx585_write_register(ViPipe, 0x306A, 0x00);//-
	imx585_write_register(ViPipe, 0x306C, 0x00);//-
	imx585_write_register(ViPipe, 0x306D, 0x00);//-
	imx585_write_register(ViPipe, 0x306E, 0x00);//-
	imx585_write_register(ViPipe, 0x306F, 0x00);//-
	imx585_write_register(ViPipe, 0x3070, 0x00);//-
	imx585_write_register(ViPipe, 0x3071, 0x00);//-
	imx585_write_register(ViPipe, 0x3074, 0x64);//-
	imx585_write_register(ViPipe, 0x3081, 0x00);//-
	imx585_write_register(ViPipe, 0x308C, 0x00);//-
	imx585_write_register(ViPipe, 0x308D, 0x01);//-
	imx585_write_register(ViPipe, 0x3094, 0x00);//-
	imx585_write_register(ViPipe, 0x3095, 0x00);//-
	imx585_write_register(ViPipe, 0x3096, 0x00);//-
	imx585_write_register(ViPipe, 0x3097, 0x00);//-
	imx585_write_register(ViPipe, 0x309C, 0x00);//-
	imx585_write_register(ViPipe, 0x309D, 0x00);//-
	imx585_write_register(ViPipe, 0x30A4, 0xAA);//-
	imx585_write_register(ViPipe, 0x30A6, 0x00);//-
	imx585_write_register(ViPipe, 0x30CC, 0x00);//-
	imx585_write_register(ViPipe, 0x30CD, 0x00);//-
	imx585_write_register(ViPipe, 0x30D5, 0x04);//-
	imx585_write_register(ViPipe, 0x30DC, 0x32);//-
	imx585_write_register(ViPipe, 0x30DD, 0x00);//-
	imx585_write_register(ViPipe, 0x3400, 0x01);//-
	imx585_write_register(ViPipe, 0x3460, 0x21);//-
	imx585_write_register(ViPipe, 0x3478, 0xA1);//TCLKPOST[15:0]
	imx585_write_register(ViPipe, 0x347C, 0x01);//TCLKPREPARE[15:0]
	imx585_write_register(ViPipe, 0x3480, 0x01);//TCLKTRAIL[15:0]
	imx585_write_register(ViPipe, 0x36D0, 0x00);//TCLKZERO[15:0]
	imx585_write_register(ViPipe, 0x36D1, 0x10);//THSPREPARE[15:0]
	imx585_write_register(ViPipe, 0x36D4, 0x00);//THSZERO[15:0]
	imx585_write_register(ViPipe, 0x36D5, 0x10);//THSTRAIL[15:0]
	imx585_write_register(ViPipe, 0x36E2, 0x00);//THSEXIT[15:0]
	imx585_write_register(ViPipe, 0x36E4, 0x00);//TLPX[15:0]
	imx585_write_register(ViPipe, 0x36E5, 0x00);//-
	imx585_write_register(ViPipe, 0x36E6, 0x00); //STANDBY
	imx585_write_register(ViPipe, 0x36E8, 0x00); //XTMSTA
	imx585_write_register(ViPipe, 0x36E9, 0x00);//BCWAIT_TIME[7:0]
	imx585_write_register(ViPipe, 0x36EA, 0x00);//CPWAIT_TIME[7:0]
	imx585_write_register(ViPipe, 0x36EC, 0x00);//HMAX[15:0]
	imx585_write_register(ViPipe, 0x36EE, 0x00);//
	imx585_write_register(ViPipe, 0x36EF, 0x00);//INCKSEL 1[8:0]
	imx585_write_register(ViPipe, 0x3930, 0x0C);//INCKSEL2[1:0]
	imx585_write_register(ViPipe, 0x3931, 0x01);//INCKSEL4[1:0]
	imx585_write_register(ViPipe, 0x3A4C, 0x39);//SYS_MODE
	imx585_write_register(ViPipe, 0x3A4D, 0x01);//XVS_DRV[1:0]
	imx585_write_register(ViPipe, 0x3A4E, 0x14);//-
	imx585_write_register(ViPipe, 0x3A50, 0x48);//-
	imx585_write_register(ViPipe, 0x3A51, 0x01);//-
	imx585_write_register(ViPipe, 0x3A52, 0x14);//-
	imx585_write_register(ViPipe, 0x3A56, 0x00);//-
	imx585_write_register(ViPipe, 0x3A5A, 0x00);//-
	imx585_write_register(ViPipe, 0x3A5E, 0x00);//-
	imx585_write_register(ViPipe, 0x3A62, 0x00);//-
	imx585_write_register(ViPipe, 0x3A6A, 0x20);//-
	imx585_write_register(ViPipe, 0x3A6C, 0x42);//-
	imx585_write_register(ViPipe, 0x3A6E, 0xA0);//-
	imx585_write_register(ViPipe, 0x3B2C, 0x0C);//-
	imx585_write_register(ViPipe, 0x3B30, 0x1C);//-
	imx585_write_register(ViPipe, 0x3B34, 0x0C);//-
	imx585_write_register(ViPipe, 0x3B38, 0x1C);//-
	imx585_write_register(ViPipe, 0x3BA0, 0x0C);//-
	imx585_write_register(ViPipe, 0x3BA4, 0x1C);//-
	imx585_write_register(ViPipe, 0x3BA8, 0x0C);//-
	imx585_write_register(ViPipe, 0x3BAC, 0x1C);//-
	imx585_write_register(ViPipe, 0x3D3C, 0x11);//-
	imx585_write_register(ViPipe, 0x3D46, 0x0B);//-
	imx585_write_register(ViPipe, 0x3DE0, 0x3F);//-
	imx585_write_register(ViPipe, 0x3DE1, 0x08);//-
	imx585_write_register(ViPipe, 0x3E10, 0x10);//-
	imx585_write_register(ViPipe, 0x3E14, 0x87);//-
	imx585_write_register(ViPipe, 0x3E16, 0x91);//-
	imx585_write_register(ViPipe, 0x3E18, 0x91);//-
	imx585_write_register(ViPipe, 0x3E1A, 0x87);//-
	imx585_write_register(ViPipe, 0x3E1C, 0x78);//-
	imx585_write_register(ViPipe, 0x3E1E, 0x50);//-
	imx585_write_register(ViPipe, 0x3E20, 0x50);//-
	imx585_write_register(ViPipe, 0x3E22, 0x50);//-
	imx585_write_register(ViPipe, 0x3E24, 0x87);//-
	imx585_write_register(ViPipe, 0x3E26, 0x91);//-
	imx585_write_register(ViPipe, 0x3E28, 0x91);//-
	imx585_write_register(ViPipe, 0x3E2A, 0x87);//-
	imx585_write_register(ViPipe, 0x3E2C, 0x78);//-
	imx585_write_register(ViPipe, 0x3E2E, 0x50);//-
	imx585_write_register(ViPipe, 0x3E30, 0x50);//-
	imx585_write_register(ViPipe, 0x3E32, 0x50);//-
	imx585_write_register(ViPipe, 0x3E34, 0x87);//-
	imx585_write_register(ViPipe, 0x3E36, 0x91);//-
	imx585_write_register(ViPipe, 0x3E38, 0x91);//-
	imx585_write_register(ViPipe, 0x3E3A, 0x87);//-
	imx585_write_register(ViPipe, 0x3E3C, 0x78);//-
	imx585_write_register(ViPipe, 0x3E3E, 0x50);//-
	imx585_write_register(ViPipe, 0x3E40, 0x50);//-
	imx585_write_register(ViPipe, 0x3E42, 0x50);//-
	imx585_write_register(ViPipe, 0x4054, 0x64);//-
	imx585_write_register(ViPipe, 0x4148, 0xFE);//-
	imx585_write_register(ViPipe, 0x4149, 0x05);//-
	imx585_write_register(ViPipe, 0x414A, 0xFF);//-
	imx585_write_register(ViPipe, 0x414B, 0x05);//-
	imx585_write_register(ViPipe, 0x420A, 0x03);//-
	imx585_write_register(ViPipe, 0x4231, 0x08);//-
	imx585_write_register(ViPipe, 0x423D, 0x9C);//-
	imx585_write_register(ViPipe, 0x4242, 0xB4);//-
	imx585_write_register(ViPipe, 0x4246, 0xB4);//-
	imx585_write_register(ViPipe, 0x424E, 0xB4);//-
	imx585_write_register(ViPipe, 0x425C, 0xB4);//-
	imx585_write_register(ViPipe, 0x425E, 0xB6);//-
	imx585_write_register(ViPipe, 0x426C, 0xB4);//-
	imx585_write_register(ViPipe, 0x426E, 0xB6);//-
	imx585_write_register(ViPipe, 0x428C, 0xB4);//-
	imx585_write_register(ViPipe, 0x428E, 0xB6);//-
	imx585_write_register(ViPipe, 0x4708, 0x00);//TCLKPOST[15:0]
	imx585_write_register(ViPipe, 0x4709, 0x00);//TCLKPREPARE[15:0]
	imx585_write_register(ViPipe, 0x470A, 0xFF);//TCLKTRAIL[15:0]
	imx585_write_register(ViPipe, 0x470B, 0x03);//TCLKZERO[15:0]
	imx585_write_register(ViPipe, 0x470C, 0x00);//THSPREPARE[15:0]
	imx585_write_register(ViPipe, 0x470D, 0x00);//THSZERO[15:0]
	imx585_write_register(ViPipe, 0x470E, 0xFF);//THSTRAIL[15:0]
	imx585_write_register(ViPipe, 0x470F, 0x03);//THSEXIT[15:0]
	imx585_write_register(ViPipe, 0x47EB, 0x1C);//TLPX[15:0]
	imx585_write_register(ViPipe, 0x47F0, 0xA6);//-
	imx585_write_register(ViPipe, 0x47F2, 0xA6); //STANDBY
	imx585_write_register(ViPipe, 0x47F4, 0xA0); //XTMSTA
	imx585_write_register(ViPipe, 0x47F6, 0x96);//BCWAIT_TIME[7:0]
	imx585_write_register(ViPipe, 0x4808, 0xA6);//CPWAIT_TIME[7:0]
	imx585_write_register(ViPipe, 0x480A, 0xA6);//HMAX[15:0]
	imx585_write_register(ViPipe, 0x480C, 0xA0);//
	imx585_write_register(ViPipe, 0x480E, 0x96);//INCKSEL 1[8:0]
	imx585_write_register(ViPipe, 0x492C, 0xB2);//INCKSEL2[1:0]
	imx585_write_register(ViPipe, 0x4930, 0x03);//INCKSEL4[1:0]
	imx585_write_register(ViPipe, 0x4932, 0x03);//SYS_MODE
	imx585_write_register(ViPipe, 0x4936, 0x5B);//XVS_DRV[1:0]
	imx585_write_register(ViPipe, 0x4938, 0x82);//-
	imx585_write_register(ViPipe, 0x493C, 0x23);//-
	imx585_write_register(ViPipe, 0x493E, 0x23);//-
	imx585_write_register(ViPipe, 0x4940, 0x23);//-
	imx585_write_register(ViPipe, 0x4BA8, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BA9, 0x03);//-
	imx585_write_register(ViPipe, 0x4BAC, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BAD, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BAE, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BAF, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BB0, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BB1, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BB2, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BB3, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BB4, 0x1C);//-
	imx585_write_register(ViPipe, 0x4BB8, 0x03);//-
	imx585_write_register(ViPipe, 0x4BB9, 0x03);//-
	imx585_write_register(ViPipe, 0x4BBA, 0x03);//-
	imx585_write_register(ViPipe, 0x4BBB, 0x03);//-
	imx585_write_register(ViPipe, 0x4BBC, 0x03);//-
	imx585_write_register(ViPipe, 0x4BBD, 0x03);//-
	imx585_write_register(ViPipe, 0x4BBE, 0x03);//-
	imx585_write_register(ViPipe, 0x4BBF, 0x03);//-
	imx585_write_register(ViPipe, 0x4BC0, 0x03);//-
	imx585_write_register(ViPipe, 0x4C14, 0x87);//-
	imx585_write_register(ViPipe, 0x4C16, 0x91);//-
	imx585_write_register(ViPipe, 0x4C18, 0x91);//-
	imx585_write_register(ViPipe, 0x4C1A, 0x87);//-
	imx585_write_register(ViPipe, 0x4C1C, 0x78);//-
	imx585_write_register(ViPipe, 0x4C1E, 0x50);//-
	imx585_write_register(ViPipe, 0x4C20, 0x50);//-
	imx585_write_register(ViPipe, 0x4C22, 0x50);//-
	imx585_write_register(ViPipe, 0x4C24, 0x87);//-
	imx585_write_register(ViPipe, 0x4C26, 0x91);//-
	imx585_write_register(ViPipe, 0x4C28, 0x91);//-
	imx585_write_register(ViPipe, 0x4C2A, 0x87);//-
	imx585_write_register(ViPipe, 0x4C2C, 0x78);//-
	imx585_write_register(ViPipe, 0x4C2E, 0x50);//-
	imx585_write_register(ViPipe, 0x4C30, 0x50);//-
	imx585_write_register(ViPipe, 0x4C32, 0x50);//-
	imx585_write_register(ViPipe, 0x4C34, 0x87);//-
	imx585_write_register(ViPipe, 0x4C36, 0x91);//-
	imx585_write_register(ViPipe, 0x4C38, 0x91);//-
	imx585_write_register(ViPipe, 0x4C3A, 0x87);//-
	imx585_write_register(ViPipe, 0x4C3C, 0x78);//-
	imx585_write_register(ViPipe, 0x4C3E, 0x50);//-
	imx585_write_register(ViPipe, 0x4C40, 0x50);//-
	imx585_write_register(ViPipe, 0x4C42, 0x50);//-
	imx585_write_register(ViPipe, 0x4D12, 0x1F);//-
	imx585_write_register(ViPipe, 0x4D13, 0x1E);//-
	imx585_write_register(ViPipe, 0x4D26, 0x33);//-
	imx585_write_register(ViPipe, 0x4E0E, 0x59);//-
	imx585_write_register(ViPipe, 0x4E14, 0x55);//-
	imx585_write_register(ViPipe, 0x4E16, 0x59);//-
	imx585_write_register(ViPipe, 0x4E1E, 0x3B);//-
	imx585_write_register(ViPipe, 0x4E20, 0x47);//-
	imx585_write_register(ViPipe, 0x4E22, 0x54);//-
	imx585_write_register(ViPipe, 0x4E26, 0x81);//-
	imx585_write_register(ViPipe, 0x4E2C, 0x7D);//-
	imx585_write_register(ViPipe, 0x4E2E, 0x81);//-
	imx585_write_register(ViPipe, 0x4E36, 0x63);//-
	imx585_write_register(ViPipe, 0x4E38, 0x6F);//-
	imx585_write_register(ViPipe, 0x4E3A, 0x7C);//-
	imx585_write_register(ViPipe, 0x4F3A, 0x3C);//-
	imx585_write_register(ViPipe, 0x4F3C, 0x46);//-
	imx585_write_register(ViPipe, 0x4F3E, 0x59);//TCLKPOST[15:0]
	imx585_write_register(ViPipe, 0x4F42, 0x64);//TCLKPREPARE[15:0]
	imx585_write_register(ViPipe, 0x4F44, 0x6E);//TCLKTRAIL[15:0]
	imx585_write_register(ViPipe, 0x4F46, 0x81);//TCLKZERO[15:0]
	imx585_write_register(ViPipe, 0x4F4A, 0x82);//THSPREPARE[15:0]
	imx585_write_register(ViPipe, 0x4F5A, 0x81);//THSZERO[15:0]
	imx585_write_register(ViPipe, 0x4F62, 0xAA);//THSTRAIL[15:0]
	imx585_write_register(ViPipe, 0x4F72, 0xA9);//THSEXIT[15:0]
	imx585_write_register(ViPipe, 0x4F78, 0x36);//TLPX[15:0]
	imx585_write_register(ViPipe, 0x4F7A, 0x41);//-
	imx585_write_register(ViPipe, 0x4F7C, 0x61); //STANDBY
	imx585_write_register(ViPipe, 0x4F7D, 0x01); //XTMSTA
	imx585_write_register(ViPipe, 0x4F7E, 0x7C);//BCWAIT_TIME[7:0]
	imx585_write_register(ViPipe, 0x4F7F, 0x01);//CPWAIT_TIME[7:0]
	imx585_write_register(ViPipe, 0x4F80, 0x77);//HMAX[15:0]
	imx585_write_register(ViPipe, 0x4F82, 0x7B);//
	imx585_write_register(ViPipe, 0x4F88, 0x37);//INCKSEL 1[8:0]
	imx585_write_register(ViPipe, 0x4F8A, 0x40);//INCKSEL2[1:0]
	imx585_write_register(ViPipe, 0x4F8C, 0x62);//INCKSEL4[1:0]
	imx585_write_register(ViPipe, 0x4F8D, 0x01);//SYS_MODE
	imx585_write_register(ViPipe, 0x4F8E, 0x76);//XVS_DRV[1:0]
	imx585_write_register(ViPipe, 0x4F8F, 0x01);//-
	imx585_write_register(ViPipe, 0x4F90, 0x5E);//-
	imx585_write_register(ViPipe, 0x4F91, 0x02);//-
	imx585_write_register(ViPipe, 0x4F92, 0x69);//-
	imx585_write_register(ViPipe, 0x4F93, 0x02);//-
	imx585_write_register(ViPipe, 0x4F94, 0x89);//-
	imx585_write_register(ViPipe, 0x4F95, 0x02);//-
	imx585_write_register(ViPipe, 0x4F96, 0xA4);//-
	imx585_write_register(ViPipe, 0x4F97, 0x02);//-
	imx585_write_register(ViPipe, 0x4F98, 0x9F);//-
	imx585_write_register(ViPipe, 0x4F99, 0x02);//-
	imx585_write_register(ViPipe, 0x4F9A, 0xA3);//-
	imx585_write_register(ViPipe, 0x4F9B, 0x02);//-
	imx585_write_register(ViPipe, 0x4FA0, 0x5F);//-
	imx585_write_register(ViPipe, 0x4FA1, 0x02);//-
	imx585_write_register(ViPipe, 0x4FA2, 0x68);//-
	imx585_write_register(ViPipe, 0x4FA3, 0x02);//-
	imx585_write_register(ViPipe, 0x4FA4, 0x8A);//-
	imx585_write_register(ViPipe, 0x4FA5, 0x02);//-
	imx585_write_register(ViPipe, 0x4FA6, 0x9E);//-
	imx585_write_register(ViPipe, 0x4FA7, 0x02);//-
	imx585_write_register(ViPipe, 0x519E, 0x79);//-
	imx585_write_register(ViPipe, 0x51A6, 0xA1);//-
	imx585_write_register(ViPipe, 0x51F0, 0xAC);//-
	imx585_write_register(ViPipe, 0x51F2, 0xAA);//-
	imx585_write_register(ViPipe, 0x51F4, 0xA5);//-
	imx585_write_register(ViPipe, 0x51F6, 0xA0);//-
	imx585_write_register(ViPipe, 0x5200, 0x9B);//-
	imx585_write_register(ViPipe, 0x5202, 0x91);//-
	imx585_write_register(ViPipe, 0x5204, 0x87);//-
	imx585_write_register(ViPipe, 0x5206, 0x82);//-
	imx585_write_register(ViPipe, 0x5208, 0xAC);//-
	imx585_write_register(ViPipe, 0x520A, 0xAA);//-
	imx585_write_register(ViPipe, 0x520C, 0xA5);//-
	imx585_write_register(ViPipe, 0x520E, 0xA0);//-
	imx585_write_register(ViPipe, 0x5210, 0x9B);//-
	imx585_write_register(ViPipe, 0x5212, 0x91);//-
	imx585_write_register(ViPipe, 0x5214, 0x87);//-
	imx585_write_register(ViPipe, 0x5216, 0x82);//-
	imx585_write_register(ViPipe, 0x5218, 0xAC);//-
	imx585_write_register(ViPipe, 0x521A, 0xAA);//-
	imx585_write_register(ViPipe, 0x521C, 0xA5);//-
	imx585_write_register(ViPipe, 0x521E, 0xA0);//-
	imx585_write_register(ViPipe, 0x5220, 0x9B);//-
	imx585_write_register(ViPipe, 0x5222, 0x91);//-
	imx585_write_register(ViPipe, 0x5224, 0x87);//-
	imx585_write_register(ViPipe, 0x5226, 0x82);//-
	imx585_default_reg_init(ViPipe);

	imx585_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(200);
	imx585_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	printf("ViPipe:%d,===IMX585 8M 30fps 12bit LINEAR Init OK!===\n", ViPipe);
}

static void imx585_wdr_8M30_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	imx585_write_register(ViPipe, 0x3000, 0x01);  //STANDBY
	imx585_write_register(ViPipe, 0x3001, 0x00);  //REGHOLD
	imx585_write_register(ViPipe, 0x3002, 0x01);  //XMSTA
	imx585_write_register(ViPipe, 0x3014, 0x01);  //INCK_SEL[3:0]
	imx585_write_register(ViPipe, 0x3015, 0x03);  //DATARATE_SEL[3:0]
	imx585_write_register(ViPipe, 0x3018, 0x10);  //WINMODE[4:0]
	imx585_write_register(ViPipe, 0x301A, 0x01);  //WDMODE
	imx585_write_register(ViPipe, 0x301B, 0x00);  //ADDMODE[1:0]
	imx585_write_register(ViPipe, 0x301C, 0x01);  //THIN_V_EN
	imx585_write_register(ViPipe, 0x301E, 0x01);  //VCMODE
	imx585_write_register(ViPipe, 0x3020, 0x01);  //HREVERSE
	imx585_write_register(ViPipe, 0x3021, 0x01);  //VREVERSE
	imx585_write_register(ViPipe, 0x3022, 0x00);  //ADBIT[1:0]
	imx585_write_register(ViPipe, 0x3023, 0x00);  //MDBIT[1:0]
	imx585_write_register(ViPipe, 0x3024, 0x00);  //COMBI_EN
	imx585_write_register(ViPipe, 0x3028, 0xCA);  //VMAX[19:0]
	imx585_write_register(ViPipe, 0x3029, 0x08);  //VMAX[19:0]
	imx585_write_register(ViPipe, 0x302A, 0x00);  //VMAX[19:0]
	imx585_write_register(ViPipe, 0x302C, 0x26);  //HMAX[15:0]
	imx585_write_register(ViPipe, 0x302D, 0x02);  //HMAX[15:0]
	imx585_write_register(ViPipe, 0x3030, 0x00);  //FDG_SEL0[1:0]
	imx585_write_register(ViPipe, 0x3031, 0x00);  //FDG_SEL1[1:0]
	imx585_write_register(ViPipe, 0x3032, 0x00);  //FDG_SEL2[1:0]
	imx585_write_register(ViPipe, 0x303C, 0x00);  //PIX_HST[12:0]
	imx585_write_register(ViPipe, 0x303D, 0x00);  //PIX_HST[12:0]
	imx585_write_register(ViPipe, 0x303E, 0x10);  //PIX_HWIDTH[12:0]
	imx585_write_register(ViPipe, 0x303F, 0x0F);  //PIX_HWIDTH[12:0]
	imx585_write_register(ViPipe, 0x3040, 0x03);  //LANEMODE[2:0]
	imx585_write_register(ViPipe, 0x3042, 0x00);  //XSIZE_OVERLAP[10:0]
	imx585_write_register(ViPipe, 0x3043, 0x00);  //XSIZE_OVERLAP[10:0]
	imx585_write_register(ViPipe, 0x3044, 0x00);  //PIX_VST[11:0]
	imx585_write_register(ViPipe, 0x3045, 0x00);  //PIX_VST[11:0]
	imx585_write_register(ViPipe, 0x3046, 0x84);  //PIX_VWIDTH[11:0]
	imx585_write_register(ViPipe, 0x3047, 0x08);  //PIX_VWIDTH[11:0]
	imx585_write_register(ViPipe, 0x3050, 0x24);  //SHR0[19:0]
	imx585_write_register(ViPipe, 0x3051, 0x09);  //SHR0[19:0]
	imx585_write_register(ViPipe, 0x3052, 0x00);  //SHR0[19:0]
	imx585_write_register(ViPipe, 0x3054, 0x0A);  //SHR1[19:0]
	imx585_write_register(ViPipe, 0x3055, 0x00);  //SHR1[19:0]
	imx585_write_register(ViPipe, 0x3056, 0x00);  //SHR1[19:0]
	imx585_write_register(ViPipe, 0x3058, 0x8A);  //SHR2[19:0]
	imx585_write_register(ViPipe, 0x3059, 0x01);  //SHR2[19:0]
	imx585_write_register(ViPipe, 0x305A, 0x00);  //SHR2[19:0]
	imx585_write_register(ViPipe, 0x3060, 0x92);  //RHS1[19:0]
	imx585_write_register(ViPipe, 0x3061, 0x00);  //RHS1[19:0]
	imx585_write_register(ViPipe, 0x3062, 0x00);  //RHS1[19:0]
	imx585_write_register(ViPipe, 0x3064, 0xC4);  //RHS2[19:0]
	imx585_write_register(ViPipe, 0x3065, 0x0C);  //RHS2[19:0]
	imx585_write_register(ViPipe, 0x3066, 0x00);  //RHS2[19:0]
	imx585_write_register(ViPipe, 0x3069, 0x00);  //-
	imx585_write_register(ViPipe, 0x306A, 0x00);  //CHDR_GAIN_EN
	imx585_write_register(ViPipe, 0x306C, 0x00);  //GAIN[10:0]
	imx585_write_register(ViPipe, 0x306D, 0x00);  //GAIN[10:0]
	imx585_write_register(ViPipe, 0x306E, 0x00);  //GAIN_1[10:0]
	imx585_write_register(ViPipe, 0x306F, 0x00);  //GAIN_1[10:0]
	imx585_write_register(ViPipe, 0x3070, 0x00);  //GAIN_2[10:0]
	imx585_write_register(ViPipe, 0x3071, 0x00);  //GAIN_2[10:0]
	imx585_write_register(ViPipe, 0x3074, 0x64);  //-
	imx585_write_register(ViPipe, 0x3081, 0x00);  //EXP_GAIN
	imx585_write_register(ViPipe, 0x308C, 0x00);  //CHDR_DGAIN0_HG[15:0]
	imx585_write_register(ViPipe, 0x308D, 0x01);  //CHDR_DGAIN0_HG[15:0]
	imx585_write_register(ViPipe, 0x3094, 0x00);  //CHDR_AGAIN0_LG[10:0]
	imx585_write_register(ViPipe, 0x3095, 0x00);  //CHDR_AGAIN0_LG[10:0]
	imx585_write_register(ViPipe, 0x3096, 0x00);  //CHDR_AGAIN1[10:0]
	imx585_write_register(ViPipe, 0x3097, 0x00);  //CHDR_AGAIN1[10:0]
	imx585_write_register(ViPipe, 0x309C, 0x00);  //CHDR_AGAIN0_HG[10:0]
	imx585_write_register(ViPipe, 0x309D, 0x00);  //CHDR_AGAIN0_HG[10:0]
	imx585_write_register(ViPipe, 0x30A4, 0xAA);  //XVSOUTSEL[1:0]
	imx585_write_register(ViPipe, 0x30A6, 0x00);  //XVS_DRV[1:0]
	imx585_write_register(ViPipe, 0x30CC, 0x00);  //-
	imx585_write_register(ViPipe, 0x30CD, 0x00);  //-
	imx585_write_register(ViPipe, 0x30D5, 0x04);  //DIG_CLP_VSTART[4:0]
	imx585_write_register(ViPipe, 0x30DC, 0x32);  //BLKLEVEL[15:0]
	imx585_write_register(ViPipe, 0x30DD, 0x00);  //BLKLEVEL[15:0]
	imx585_write_register(ViPipe, 0x3400, 0x01);  //GAIN_PGC_FIDMD
	imx585_write_register(ViPipe, 0x3460, 0x21);  //-
	imx585_write_register(ViPipe, 0x3478, 0xA1);  //-
	imx585_write_register(ViPipe, 0x347C, 0x01);  //-
	imx585_write_register(ViPipe, 0x3480, 0x01);  //-
	imx585_write_register(ViPipe, 0x36D0, 0x00);  //EXP_TH_H
	imx585_write_register(ViPipe, 0x36D1, 0x10);  //EXP_TH_H
	imx585_write_register(ViPipe, 0x36D4, 0x00);  //EXP_TH_L
	imx585_write_register(ViPipe, 0x36D5, 0x10);  //EXP_TH_L
	imx585_write_register(ViPipe, 0x36E2, 0x00);  //EXP_BK
	imx585_write_register(ViPipe, 0x36E4, 0x00);  //CCMP2_EXP
	imx585_write_register(ViPipe, 0x36E5, 0x00);  //CCMP2_EXP
	imx585_write_register(ViPipe, 0x36E6, 0x00);  //CCMP2_EXP
	imx585_write_register(ViPipe, 0x36E8, 0x00);  //CCMP1_EXP
	imx585_write_register(ViPipe, 0x36E9, 0x00);  //CCMP1_EXP
	imx585_write_register(ViPipe, 0x36EA, 0x00);  //CCMP1_EXP
	imx585_write_register(ViPipe, 0x36EC, 0x00);  //ACMP2_EXP
	imx585_write_register(ViPipe, 0x36EE, 0x00);  //ACMP1_EXP
	imx585_write_register(ViPipe, 0x36EF, 0x00);  //CCMP_EN
	imx585_write_register(ViPipe, 0x3930, 0x66);  //DUR
	imx585_write_register(ViPipe, 0x3931, 0x00);  //DUR
	imx585_write_register(ViPipe, 0x3A4C, 0x39);  //WAIT_ST0
	imx585_write_register(ViPipe, 0x3A4D, 0x01);  //WAIT_ST0
	imx585_write_register(ViPipe, 0x3A4E, 0x14);  //-
	imx585_write_register(ViPipe, 0x3A50, 0x48);  //WAIT_ST1
	imx585_write_register(ViPipe, 0x3A51, 0x01);  //WAIT_ST1
	imx585_write_register(ViPipe, 0x3A52, 0x14);  //-
	imx585_write_register(ViPipe, 0x3A56, 0x00);  //-
	imx585_write_register(ViPipe, 0x3A5A, 0x00);  //-
	imx585_write_register(ViPipe, 0x3A5E, 0x00);  //-
	imx585_write_register(ViPipe, 0x3A62, 0x00);  //-
	imx585_write_register(ViPipe, 0x3A6A, 0x20);  //-
	imx585_write_register(ViPipe, 0x3A6C, 0x42);  //-
	imx585_write_register(ViPipe, 0x3A6E, 0xA0);  //-
	imx585_write_register(ViPipe, 0x3B2C, 0x0C);  //-
	imx585_write_register(ViPipe, 0x3B30, 0x1C);  //-
	imx585_write_register(ViPipe, 0x3B34, 0x0C);  //-
	imx585_write_register(ViPipe, 0x3B38, 0x1C);  //-
	imx585_write_register(ViPipe, 0x3BA0, 0x0C);  //-
	imx585_write_register(ViPipe, 0x3BA4, 0x1C);  //-
	imx585_write_register(ViPipe, 0x3BA8, 0x0C);  //-
	imx585_write_register(ViPipe, 0x3BAC, 0x1C);  //-
	imx585_write_register(ViPipe, 0x3D3C, 0x11);  //-
	imx585_write_register(ViPipe, 0x3D46, 0x0B);  //-
	imx585_write_register(ViPipe, 0x3DE0, 0x3F);  //-
	imx585_write_register(ViPipe, 0x3DE1, 0x08);  //-
	imx585_write_register(ViPipe, 0x3E10, 0x10);  //ADTHEN[2:0]
	imx585_write_register(ViPipe, 0x3E14, 0x87);  //-
	imx585_write_register(ViPipe, 0x3E16, 0x91);  //-
	imx585_write_register(ViPipe, 0x3E18, 0x91);  //-
	imx585_write_register(ViPipe, 0x3E1A, 0x87);  //-
	imx585_write_register(ViPipe, 0x3E1C, 0x78);  //-
	imx585_write_register(ViPipe, 0x3E1E, 0x50);  //-
	imx585_write_register(ViPipe, 0x3E20, 0x50);  //-
	imx585_write_register(ViPipe, 0x3E22, 0x50);  //-
	imx585_write_register(ViPipe, 0x3E24, 0x87);  //-
	imx585_write_register(ViPipe, 0x3E26, 0x91);  //-
	imx585_write_register(ViPipe, 0x3E28, 0x91);  //-
	imx585_write_register(ViPipe, 0x3E2A, 0x87);  //-
	imx585_write_register(ViPipe, 0x3E2C, 0x78);  //-
	imx585_write_register(ViPipe, 0x3E2E, 0x50);  //-
	imx585_write_register(ViPipe, 0x3E30, 0x50);  //-
	imx585_write_register(ViPipe, 0x3E32, 0x50);  //-
	imx585_write_register(ViPipe, 0x3E34, 0x87);  //-
	imx585_write_register(ViPipe, 0x3E36, 0x91);  //-
	imx585_write_register(ViPipe, 0x3E38, 0x91);  //-
	imx585_write_register(ViPipe, 0x3E3A, 0x87);  //-
	imx585_write_register(ViPipe, 0x3E3C, 0x78);  //-
	imx585_write_register(ViPipe, 0x3E3E, 0x50);  //-
	imx585_write_register(ViPipe, 0x3E40, 0x50);  //-
	imx585_write_register(ViPipe, 0x3E42, 0x50);  //-
	imx585_write_register(ViPipe, 0x4054, 0x64);  //-
	imx585_write_register(ViPipe, 0x4148, 0xFE);  //-
	imx585_write_register(ViPipe, 0x4149, 0x05);  //-
	imx585_write_register(ViPipe, 0x414A, 0xFF);  //-
	imx585_write_register(ViPipe, 0x414B, 0x05);  //-
	imx585_write_register(ViPipe, 0x420A, 0x03);  //-
	imx585_write_register(ViPipe, 0x4231, 0x18);  //-
	imx585_write_register(ViPipe, 0x423D, 0x9C);  //-
	imx585_write_register(ViPipe, 0x4242, 0xB4);  //-
	imx585_write_register(ViPipe, 0x4246, 0xB4);  //-
	imx585_write_register(ViPipe, 0x424E, 0xB4);  //-
	imx585_write_register(ViPipe, 0x425C, 0xB4);  //-
	imx585_write_register(ViPipe, 0x425E, 0xB6);  //-
	imx585_write_register(ViPipe, 0x426C, 0xB4);  //-
	imx585_write_register(ViPipe, 0x426E, 0xB6);  //-
	imx585_write_register(ViPipe, 0x428C, 0xB4);  //-
	imx585_write_register(ViPipe, 0x428E, 0xB6);  //-
	imx585_write_register(ViPipe, 0x4708, 0x00);  //-
	imx585_write_register(ViPipe, 0x4709, 0x00);  //-
	imx585_write_register(ViPipe, 0x470A, 0xFF);  //-
	imx585_write_register(ViPipe, 0x470B, 0x03);  //-
	imx585_write_register(ViPipe, 0x470C, 0x00);  //-
	imx585_write_register(ViPipe, 0x470D, 0x00);  //-
	imx585_write_register(ViPipe, 0x470E, 0xFF);  //-
	imx585_write_register(ViPipe, 0x470F, 0x03);  //-
	imx585_write_register(ViPipe, 0x47EB, 0x1C);  //-
	imx585_write_register(ViPipe, 0x47F0, 0xA6);  //-
	imx585_write_register(ViPipe, 0x47F2, 0xA6);  //-
	imx585_write_register(ViPipe, 0x47F4, 0xA0);  //-
	imx585_write_register(ViPipe, 0x47F6, 0x96);  //-
	imx585_write_register(ViPipe, 0x4808, 0xA6);  //-
	imx585_write_register(ViPipe, 0x480A, 0xA6);  //-
	imx585_write_register(ViPipe, 0x480C, 0xA0);  //-
	imx585_write_register(ViPipe, 0x480E, 0x96);  //-
	imx585_write_register(ViPipe, 0x492C, 0xB2);  //-
	imx585_write_register(ViPipe, 0x4930, 0x03);  //-
	imx585_write_register(ViPipe, 0x4932, 0x03);  //-
	imx585_write_register(ViPipe, 0x4936, 0x5B);  //-
	imx585_write_register(ViPipe, 0x4938, 0x82);  //-
	imx585_write_register(ViPipe, 0x493C, 0x23);  //WAIT_10_SHF
	imx585_write_register(ViPipe, 0x493E, 0x23);  //-
	imx585_write_register(ViPipe, 0x4940, 0x23);  //WAIT_12_SHF
	imx585_write_register(ViPipe, 0x4BA8, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BA9, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BAC, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BAD, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BAE, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BAF, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BB0, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BB1, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BB2, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BB3, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BB4, 0x1C);  //-
	imx585_write_register(ViPipe, 0x4BB8, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BB9, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BBA, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BBB, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BBC, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BBD, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BBE, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BBF, 0x03);  //-
	imx585_write_register(ViPipe, 0x4BC0, 0x03);  //-
	imx585_write_register(ViPipe, 0x4C14, 0x87);  //-
	imx585_write_register(ViPipe, 0x4C16, 0x91);  //-
	imx585_write_register(ViPipe, 0x4C18, 0x91);  //-
	imx585_write_register(ViPipe, 0x4C1A, 0x87);  //-
	imx585_write_register(ViPipe, 0x4C1C, 0x78);  //-
	imx585_write_register(ViPipe, 0x4C1E, 0x50);  //-
	imx585_write_register(ViPipe, 0x4C20, 0x50);  //-
	imx585_write_register(ViPipe, 0x4C22, 0x50);  //-
	imx585_write_register(ViPipe, 0x4C24, 0x87);  //-
	imx585_write_register(ViPipe, 0x4C26, 0x91);  //-
	imx585_write_register(ViPipe, 0x4C28, 0x91);  //-
	imx585_write_register(ViPipe, 0x4C2A, 0x87);  //-
	imx585_write_register(ViPipe, 0x4C2C, 0x78);  //-
	imx585_write_register(ViPipe, 0x4C2E, 0x50);  //-
	imx585_write_register(ViPipe, 0x4C30, 0x50);  //-
	imx585_write_register(ViPipe, 0x4C32, 0x50);  //-
	imx585_write_register(ViPipe, 0x4C34, 0x87);  //-
	imx585_write_register(ViPipe, 0x4C36, 0x91);  //-
	imx585_write_register(ViPipe, 0x4C38, 0x91);  //-
	imx585_write_register(ViPipe, 0x4C3A, 0x87);  //-
	imx585_write_register(ViPipe, 0x4C3C, 0x78);  //-
	imx585_write_register(ViPipe, 0x4C3E, 0x50);  //-
	imx585_write_register(ViPipe, 0x4C40, 0x50);  //-
	imx585_write_register(ViPipe, 0x4C42, 0x50);  //-
	imx585_write_register(ViPipe, 0x4D12, 0x1F);  //-
	imx585_write_register(ViPipe, 0x4D13, 0x1E);  //-
	imx585_write_register(ViPipe, 0x4D26, 0x33);  //-
	imx585_write_register(ViPipe, 0x4E0E, 0x59);  //-
	imx585_write_register(ViPipe, 0x4E14, 0x55);  //-
	imx585_write_register(ViPipe, 0x4E16, 0x59);  //-
	imx585_write_register(ViPipe, 0x4E1E, 0x3B);  //-
	imx585_write_register(ViPipe, 0x4E20, 0x47);  //-
	imx585_write_register(ViPipe, 0x4E22, 0x54);  //-
	imx585_write_register(ViPipe, 0x4E26, 0x81);  //-
	imx585_write_register(ViPipe, 0x4E2C, 0x7D);  //-
	imx585_write_register(ViPipe, 0x4E2E, 0x81);  //-
	imx585_write_register(ViPipe, 0x4E36, 0x63);  //-
	imx585_write_register(ViPipe, 0x4E38, 0x6F);  //-
	imx585_write_register(ViPipe, 0x4E3A, 0x7C);  //-
	imx585_write_register(ViPipe, 0x4F3A, 0x3C);  //-
	imx585_write_register(ViPipe, 0x4F3C, 0x46);  //-
	imx585_write_register(ViPipe, 0x4F3E, 0x59);  //-
	imx585_write_register(ViPipe, 0x4F42, 0x64);  //-
	imx585_write_register(ViPipe, 0x4F44, 0x6E);  //-
	imx585_write_register(ViPipe, 0x4F46, 0x81);  //-
	imx585_write_register(ViPipe, 0x4F4A, 0x82);  //-
	imx585_write_register(ViPipe, 0x4F5A, 0x81);  //-
	imx585_write_register(ViPipe, 0x4F62, 0xAA);  //-
	imx585_write_register(ViPipe, 0x4F72, 0xA9);  //-
	imx585_write_register(ViPipe, 0x4F78, 0x36);  //-
	imx585_write_register(ViPipe, 0x4F7A, 0x41);  //-
	imx585_write_register(ViPipe, 0x4F7C, 0x61);  //-
	imx585_write_register(ViPipe, 0x4F7D, 0x01);  //-
	imx585_write_register(ViPipe, 0x4F7E, 0x7C);  //-
	imx585_write_register(ViPipe, 0x4F7F, 0x01);  //-
	imx585_write_register(ViPipe, 0x4F80, 0x77);  //-
	imx585_write_register(ViPipe, 0x4F82, 0x7B);  //-
	imx585_write_register(ViPipe, 0x4F88, 0x37);  //-
	imx585_write_register(ViPipe, 0x4F8A, 0x40);  //-
	imx585_write_register(ViPipe, 0x4F8C, 0x62);  //-
	imx585_write_register(ViPipe, 0x4F8D, 0x01);  //-
	imx585_write_register(ViPipe, 0x4F8E, 0x76);  //-
	imx585_write_register(ViPipe, 0x4F8F, 0x01);  //-
	imx585_write_register(ViPipe, 0x4F90, 0x5E);  //-
	imx585_write_register(ViPipe, 0x4F91, 0x02);  //-
	imx585_write_register(ViPipe, 0x4F92, 0x69);  //-
	imx585_write_register(ViPipe, 0x4F93, 0x02);  //-
	imx585_write_register(ViPipe, 0x4F94, 0x89);  //-
	imx585_write_register(ViPipe, 0x4F95, 0x02);  //-
	imx585_write_register(ViPipe, 0x4F96, 0xA4);  //-
	imx585_write_register(ViPipe, 0x4F97, 0x02);  //-
	imx585_write_register(ViPipe, 0x4F98, 0x9F);  //-
	imx585_write_register(ViPipe, 0x4F99, 0x02);  //-
	imx585_write_register(ViPipe, 0x4F9A, 0xA3);  //-
	imx585_write_register(ViPipe, 0x4F9B, 0x02);  //-
	imx585_write_register(ViPipe, 0x4FA0, 0x5F);  //-
	imx585_write_register(ViPipe, 0x4FA1, 0x02);  //-
	imx585_write_register(ViPipe, 0x4FA2, 0x68);  //-
	imx585_write_register(ViPipe, 0x4FA3, 0x02);  //-
	imx585_write_register(ViPipe, 0x4FA4, 0x8A);  //-
	imx585_write_register(ViPipe, 0x4FA5, 0x02);  //-
	imx585_write_register(ViPipe, 0x4FA6, 0x9E);  //-
	imx585_write_register(ViPipe, 0x4FA7, 0x02);  //-
	imx585_write_register(ViPipe, 0x519E, 0x79);  //-
	imx585_write_register(ViPipe, 0x51A6, 0xA1);  //-
	imx585_write_register(ViPipe, 0x51F0, 0xAC);  //-
	imx585_write_register(ViPipe, 0x51F2, 0xAA);  //-
	imx585_write_register(ViPipe, 0x51F4, 0xA5);  //-
	imx585_write_register(ViPipe, 0x51F6, 0xA0);  //-
	imx585_write_register(ViPipe, 0x5200, 0x9B);  //-
	imx585_write_register(ViPipe, 0x5202, 0x91);  //-
	imx585_write_register(ViPipe, 0x5204, 0x87);  //-
	imx585_write_register(ViPipe, 0x5206, 0x82);  //-
	imx585_write_register(ViPipe, 0x5208, 0xAC);  //-
	imx585_write_register(ViPipe, 0x520A, 0xAA);  //-
	imx585_write_register(ViPipe, 0x520C, 0xA5);  //-
	imx585_write_register(ViPipe, 0x520E, 0xA0);  //-
	imx585_write_register(ViPipe, 0x5210, 0x9B);  //-
	imx585_write_register(ViPipe, 0x5212, 0x91);  //-
	imx585_write_register(ViPipe, 0x5214, 0x87);  //-
	imx585_write_register(ViPipe, 0x5216, 0x82);  //-
	imx585_write_register(ViPipe, 0x5218, 0xAC);  //-
	imx585_write_register(ViPipe, 0x521A, 0xAA);  //-
	imx585_write_register(ViPipe, 0x521C, 0xA5);  //-
	imx585_write_register(ViPipe, 0x521E, 0xA0);  //-
	imx585_write_register(ViPipe, 0x5220, 0x9B);  //-
	imx585_write_register(ViPipe, 0x5222, 0x91);  //-
	imx585_write_register(ViPipe, 0x5224, 0x87);  //-
	imx585_write_register(ViPipe, 0x5226, 0x82);  //-

	imx585_default_reg_init(ViPipe);

	imx585_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(200);
	imx585_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	printf("ViPipe:%d,===Imx585 sensor 8M30fps 12bit 2to1 WDR(60fps->30fps) init success!=====\n", ViPipe);
}
