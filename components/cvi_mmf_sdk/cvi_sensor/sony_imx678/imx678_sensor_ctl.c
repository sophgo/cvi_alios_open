#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx678_cmos_ex.h"
#include "sensor_i2c.h"

static void imx678_linear_8M30_init(VI_PIPE ViPipe);
static void imx678_wdr_5M25_2to1_init(VI_PIPE ViPipe);

const CVI_U32 imx678_addr_byte = 2;
const CVI_U32 imx678_data_byte = 1;

int imx678_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx678_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx678_AddrInfo[ViPipe].s8I2cAddr);
}

int imx678_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx678_BusInfo[ViPipe].s8I2cDev);
}

int imx678_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx678_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx678_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx678_addr_byte, imx678_data_byte);
}

int imx678_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx678_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx678_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx678_addr_byte, (CVI_U32)data, imx678_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx678_standby(VI_PIPE ViPipe)
{
	imx678_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx678_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */
}

void imx678_restart(VI_PIPE ViPipe)
{
	imx678_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(24);
	imx678_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
}

void imx678_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx678[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx678_write_register(ViPipe,
				g_pastImx678[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx678[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx678_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 flip, mirror;

	flip = imx678_read_register(ViPipe, 0x3021);
	mirror = imx678_read_register(ViPipe, 0x3020);

	flip &= ~0x1;
	mirror &= ~0x1;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		mirror |= 0x1;
		break;
	case ISP_SNS_FLIP:
		flip |= 0x1;
		break;
	case ISP_SNS_MIRROR_FLIP:
		mirror |= 0x1;
		flip |= 0x1;
		break;
	default:
		return;
	}

	imx678_write_register(ViPipe, 0x3021, flip);
	imx678_write_register(ViPipe, 0x3020, mirror);
}

#define IMX678_CHIP_ID_ADDR_1	0x4d13
#define IMX678_CHIP_ID_ADDR_2	0x4d12
#define IMX678_CHIP_ID	        0x876

int imx678_probe(VI_PIPE ViPipe)
{

	delay_ms(1); //waitting i2c stable
	if (imx678_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	// imx678_write_register(ViPipe, 0x3000, 0x00);
	delay_ms(24); //registers become accessible after waiting 80 ms after standby cancel (STANDBY= 0)
	return CVI_SUCCESS;
}

void imx678_init(VI_PIPE ViPipe)
{
	WDR_MODE_E       enWDRMode;
	CVI_U8            u8ImgMode;

	int nVal_1;
	int nVal_2;

	enWDRMode   = g_pastImx678[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx678[ViPipe]->u8ImgMode;

	delay_ms(1); //waitting i2c stable
	if (imx678_i2c_init(ViPipe) != CVI_SUCCESS) {
		return ;
	}

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == IMX678_MODE_5M25_WDR) {
			imx678_wdr_5M25_2to1_init(ViPipe);
		}
	} else {
		imx678_linear_8M30_init(ViPipe);
	}

	nVal_1 = imx678_read_register(ViPipe, IMX678_CHIP_ID_ADDR_1);
	nVal_2 = imx678_read_register(ViPipe, IMX678_CHIP_ID_ADDR_2);

	if (nVal_1 < 0 || nVal_2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error\n");
		return ;
	}

	if ((((nVal_1 & 0x0F) << 8) | (nVal_2 & 0xFF)) != IMX678_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return ;
	}
	g_pastImx678[ViPipe]->bInit = CVI_TRUE;
}

static void imx678_linear_8M30_init(VI_PIPE ViPipe)
{
	delay_ms(40);
	imx678_write_register(ViPipe, 0x3000, 0x01);
	delay_ms(40);
	imx678_write_register(ViPipe, 0x3002, 0x01);
	imx678_write_register(ViPipe, 0x3014, 0x04);
	imx678_write_register(ViPipe, 0x3015, 0x05);
	imx678_write_register(ViPipe, 0x3022, 0x00);
	imx678_write_register(ViPipe, 0x3023, 0x00);
	imx678_write_register(ViPipe, 0x3050, 0x03);
	imx678_write_register(ViPipe, 0x30A6, 0x00);
	imx678_write_register(ViPipe, 0x3460, 0x22);
	imx678_write_register(ViPipe, 0x355A, 0x64);
	imx678_write_register(ViPipe, 0x3A02, 0x7A);
	imx678_write_register(ViPipe, 0x3A10, 0xEC);
	imx678_write_register(ViPipe, 0x3A12, 0x71);
	imx678_write_register(ViPipe, 0x3A14, 0xDE);
	imx678_write_register(ViPipe, 0x3A20, 0x2B);
	imx678_write_register(ViPipe, 0x3A24, 0x22);
	imx678_write_register(ViPipe, 0x3A25, 0x25);
	imx678_write_register(ViPipe, 0x3A26, 0x2A);
	imx678_write_register(ViPipe, 0x3A27, 0x2C);
	imx678_write_register(ViPipe, 0x3A28, 0x39);
	imx678_write_register(ViPipe, 0x3A29, 0x38);
	imx678_write_register(ViPipe, 0x3A30, 0x04);
	imx678_write_register(ViPipe, 0x3A31, 0x04);
	imx678_write_register(ViPipe, 0x3A32, 0x03);
	imx678_write_register(ViPipe, 0x3A33, 0x03);
	imx678_write_register(ViPipe, 0x3A34, 0x09);
	imx678_write_register(ViPipe, 0x3A35, 0x06);
	imx678_write_register(ViPipe, 0x3A38, 0xCD);
	imx678_write_register(ViPipe, 0x3A3A, 0x4C);
	imx678_write_register(ViPipe, 0x3A3C, 0xB9);
	imx678_write_register(ViPipe, 0x3A3E, 0x30);
	imx678_write_register(ViPipe, 0x3A40, 0x2C);
	imx678_write_register(ViPipe, 0x3A42, 0x39);
	imx678_write_register(ViPipe, 0x3A4E, 0x00);
	imx678_write_register(ViPipe, 0x3A52, 0x00);
	imx678_write_register(ViPipe, 0x3A56, 0x00);
	imx678_write_register(ViPipe, 0x3A5A, 0x00);
	imx678_write_register(ViPipe, 0x3A5E, 0x00);
	imx678_write_register(ViPipe, 0x3A62, 0x00);
	imx678_write_register(ViPipe, 0x3A6E, 0xA0);
	imx678_write_register(ViPipe, 0x3A70, 0x50);
	imx678_write_register(ViPipe, 0x3A8C, 0x04);
	imx678_write_register(ViPipe, 0x3A8D, 0x03);
	imx678_write_register(ViPipe, 0x3A8E, 0x09);
	imx678_write_register(ViPipe, 0x3A90, 0x38);
	imx678_write_register(ViPipe, 0x3A91, 0x42);
	imx678_write_register(ViPipe, 0x3A92, 0x3C);
	imx678_write_register(ViPipe, 0x3B0E, 0xF3);
	imx678_write_register(ViPipe, 0x3B12, 0xE5);
	imx678_write_register(ViPipe, 0x3B27, 0xC0);
	imx678_write_register(ViPipe, 0x3B2E, 0xEF);
	imx678_write_register(ViPipe, 0x3B30, 0x6A);
	imx678_write_register(ViPipe, 0x3B32, 0xF6);
	imx678_write_register(ViPipe, 0x3B36, 0xE1);
	imx678_write_register(ViPipe, 0x3B3A, 0xE8);
	imx678_write_register(ViPipe, 0x3B5A, 0x17);
	imx678_write_register(ViPipe, 0x3B5E, 0xEF);
	imx678_write_register(ViPipe, 0x3B60, 0x6A);
	imx678_write_register(ViPipe, 0x3B62, 0xF6);
	imx678_write_register(ViPipe, 0x3B66, 0xE1);
	imx678_write_register(ViPipe, 0x3B6A, 0xE8);
	imx678_write_register(ViPipe, 0x3B88, 0xEC);
	imx678_write_register(ViPipe, 0x3B8A, 0xED);
	imx678_write_register(ViPipe, 0x3B94, 0x71);
	imx678_write_register(ViPipe, 0x3B96, 0x72);
	imx678_write_register(ViPipe, 0x3B98, 0xDE);
	imx678_write_register(ViPipe, 0x3B9A, 0xDF);
	imx678_write_register(ViPipe, 0x3C0F, 0x06);
	imx678_write_register(ViPipe, 0x3C10, 0x06);
	imx678_write_register(ViPipe, 0x3C11, 0x06);
	imx678_write_register(ViPipe, 0x3C12, 0x06);
	imx678_write_register(ViPipe, 0x3C13, 0x06);
	imx678_write_register(ViPipe, 0x3C18, 0x20);
	imx678_write_register(ViPipe, 0x3C3A, 0x7A);
	imx678_write_register(ViPipe, 0x3C40, 0xF4);
	imx678_write_register(ViPipe, 0x3C48, 0xE6);
	imx678_write_register(ViPipe, 0x3C54, 0xCE);
	imx678_write_register(ViPipe, 0x3C56, 0xD0);
	imx678_write_register(ViPipe, 0x3C6C, 0x53);
	imx678_write_register(ViPipe, 0x3C6E, 0x55);
	imx678_write_register(ViPipe, 0x3C70, 0xC0);
	imx678_write_register(ViPipe, 0x3C72, 0xC2);
	imx678_write_register(ViPipe, 0x3C7E, 0xCE);
	imx678_write_register(ViPipe, 0x3C8C, 0xCF);
	imx678_write_register(ViPipe, 0x3C8E, 0xEB);
	imx678_write_register(ViPipe, 0x3C98, 0x54);
	imx678_write_register(ViPipe, 0x3C9A, 0x70);
	imx678_write_register(ViPipe, 0x3C9C, 0xC1);
	imx678_write_register(ViPipe, 0x3C9E, 0xDD);
	imx678_write_register(ViPipe, 0x3CB0, 0x7A);
	imx678_write_register(ViPipe, 0x3CB2, 0xBA);
	imx678_write_register(ViPipe, 0x3CC8, 0xBC);
	imx678_write_register(ViPipe, 0x3CCA, 0x7C);
	imx678_write_register(ViPipe, 0x3CD4, 0xEA);
	imx678_write_register(ViPipe, 0x3CD5, 0x01);
	imx678_write_register(ViPipe, 0x3CD6, 0x4A);
	imx678_write_register(ViPipe, 0x3CD8, 0x00);
	imx678_write_register(ViPipe, 0x3CD9, 0x00);
	imx678_write_register(ViPipe, 0x3CDA, 0xFF);
	imx678_write_register(ViPipe, 0x3CDB, 0x03);
	imx678_write_register(ViPipe, 0x3CDC, 0x00);
	imx678_write_register(ViPipe, 0x3CDD, 0x00);
	imx678_write_register(ViPipe, 0x3CDE, 0xFF);
	imx678_write_register(ViPipe, 0x3CDF, 0x03);
	imx678_write_register(ViPipe, 0x3CE4, 0x4C);
	imx678_write_register(ViPipe, 0x3CE6, 0xEC);
	imx678_write_register(ViPipe, 0x3CE7, 0x01);
	imx678_write_register(ViPipe, 0x3CE8, 0xFF);
	imx678_write_register(ViPipe, 0x3CE9, 0x03);
	imx678_write_register(ViPipe, 0x3CEA, 0x00);
	imx678_write_register(ViPipe, 0x3CEB, 0x00);
	imx678_write_register(ViPipe, 0x3CEC, 0xFF);
	imx678_write_register(ViPipe, 0x3CED, 0x03);
	imx678_write_register(ViPipe, 0x3CEE, 0x00);
	imx678_write_register(ViPipe, 0x3CEF, 0x00);
	imx678_write_register(ViPipe, 0x3E28, 0x82);
	imx678_write_register(ViPipe, 0x3E2A, 0x80);
	imx678_write_register(ViPipe, 0x3E30, 0x85);
	imx678_write_register(ViPipe, 0x3E32, 0x7D);
	imx678_write_register(ViPipe, 0x3E5C, 0xCE);
	imx678_write_register(ViPipe, 0x3E5E, 0xD3);
	imx678_write_register(ViPipe, 0x3E70, 0x53);
	imx678_write_register(ViPipe, 0x3E72, 0x58);
	imx678_write_register(ViPipe, 0x3E74, 0xC0);
	imx678_write_register(ViPipe, 0x3E76, 0xC5);
	imx678_write_register(ViPipe, 0x3E78, 0xC0);
	imx678_write_register(ViPipe, 0x3E79, 0x01);
	imx678_write_register(ViPipe, 0x3E7A, 0xD4);
	imx678_write_register(ViPipe, 0x3E7B, 0x01);
	imx678_write_register(ViPipe, 0x3EB4, 0x0B);
	imx678_write_register(ViPipe, 0x3EB5, 0x02);
	imx678_write_register(ViPipe, 0x3EB6, 0x4D);
	imx678_write_register(ViPipe, 0x3EEC, 0xF3);
	imx678_write_register(ViPipe, 0x3EEE, 0xE7);
	imx678_write_register(ViPipe, 0x3F01, 0x01);
	imx678_write_register(ViPipe, 0x3F24, 0x10);
	imx678_write_register(ViPipe, 0x3F28, 0x2D);
	imx678_write_register(ViPipe, 0x3F2A, 0x2D);
	imx678_write_register(ViPipe, 0x3F2C, 0x2D);
	imx678_write_register(ViPipe, 0x3F2E, 0x2D);
	imx678_write_register(ViPipe, 0x3F30, 0x23);
	imx678_write_register(ViPipe, 0x3F38, 0x2D);
	imx678_write_register(ViPipe, 0x3F3A, 0x2D);
	imx678_write_register(ViPipe, 0x3F3C, 0x2D);
	imx678_write_register(ViPipe, 0x3F3E, 0x28);
	imx678_write_register(ViPipe, 0x3F40, 0x1E);
	imx678_write_register(ViPipe, 0x3F48, 0x2D);
	imx678_write_register(ViPipe, 0x3F4A, 0x2D);
	imx678_write_register(ViPipe, 0x4004, 0xE4);
	imx678_write_register(ViPipe, 0x4006, 0xFF);
	imx678_write_register(ViPipe, 0x4018, 0x69);
	imx678_write_register(ViPipe, 0x401A, 0x84);
	imx678_write_register(ViPipe, 0x401C, 0xD6);
	imx678_write_register(ViPipe, 0x401E, 0xF1);
	imx678_write_register(ViPipe, 0x4038, 0xDE);
	imx678_write_register(ViPipe, 0x403A, 0x00);
	imx678_write_register(ViPipe, 0x403B, 0x01);
	imx678_write_register(ViPipe, 0x404C, 0x63);
	imx678_write_register(ViPipe, 0x404E, 0x85);
	imx678_write_register(ViPipe, 0x4050, 0xD0);
	imx678_write_register(ViPipe, 0x4052, 0xF2);
	imx678_write_register(ViPipe, 0x4108, 0xDD);
	imx678_write_register(ViPipe, 0x410A, 0xF7);
	imx678_write_register(ViPipe, 0x411C, 0x62);
	imx678_write_register(ViPipe, 0x411E, 0x7C);
	imx678_write_register(ViPipe, 0x4120, 0xCF);
	imx678_write_register(ViPipe, 0x4122, 0xE9);
	imx678_write_register(ViPipe, 0x4138, 0xE6);
	imx678_write_register(ViPipe, 0x413A, 0xF1);
	imx678_write_register(ViPipe, 0x414C, 0x6B);
	imx678_write_register(ViPipe, 0x414E, 0x76);
	imx678_write_register(ViPipe, 0x4150, 0xD8);
	imx678_write_register(ViPipe, 0x4152, 0xE3);
	imx678_write_register(ViPipe, 0x417E, 0x03);
	imx678_write_register(ViPipe, 0x417F, 0x01);
	imx678_write_register(ViPipe, 0x4186, 0xE0);
	imx678_write_register(ViPipe, 0x4190, 0xF3);
	imx678_write_register(ViPipe, 0x4192, 0xF7);
	imx678_write_register(ViPipe, 0x419C, 0x78);
	imx678_write_register(ViPipe, 0x419E, 0x7C);
	imx678_write_register(ViPipe, 0x41A0, 0xE5);
	imx678_write_register(ViPipe, 0x41A2, 0xE9);
	imx678_write_register(ViPipe, 0x41C8, 0xE2);
	imx678_write_register(ViPipe, 0x41CA, 0xFD);
	imx678_write_register(ViPipe, 0x41DC, 0x67);
	imx678_write_register(ViPipe, 0x41DE, 0x82);
	imx678_write_register(ViPipe, 0x41E0, 0xD4);
	imx678_write_register(ViPipe, 0x41E2, 0xEF);
	imx678_write_register(ViPipe, 0x4200, 0xDE);
	imx678_write_register(ViPipe, 0x4202, 0xDA);
	imx678_write_register(ViPipe, 0x4218, 0x63);
	imx678_write_register(ViPipe, 0x421A, 0x5F);
	imx678_write_register(ViPipe, 0x421C, 0xD0);
	imx678_write_register(ViPipe, 0x421E, 0xCC);
	imx678_write_register(ViPipe, 0x425A, 0x82);
	imx678_write_register(ViPipe, 0x425C, 0xEF);
	imx678_write_register(ViPipe, 0x4348, 0xFE);
	imx678_write_register(ViPipe, 0x4349, 0x06);
	imx678_write_register(ViPipe, 0x4352, 0xCE);
	imx678_write_register(ViPipe, 0x4420, 0x0B);
	imx678_write_register(ViPipe, 0x4421, 0x02);
	imx678_write_register(ViPipe, 0x4422, 0x4D);
	imx678_write_register(ViPipe, 0x4426, 0xF5);
	imx678_write_register(ViPipe, 0x442A, 0xE7);
	imx678_write_register(ViPipe, 0x4432, 0xF5);
	imx678_write_register(ViPipe, 0x4436, 0xE7);
	imx678_write_register(ViPipe, 0x4466, 0xB4);
	imx678_write_register(ViPipe, 0x446E, 0x32);
	imx678_write_register(ViPipe, 0x449F, 0x1C);
	imx678_write_register(ViPipe, 0x44A4, 0x2C);
	imx678_write_register(ViPipe, 0x44A6, 0x2C);
	imx678_write_register(ViPipe, 0x44A8, 0x2C);
	imx678_write_register(ViPipe, 0x44AA, 0x2C);
	imx678_write_register(ViPipe, 0x44B4, 0x2C);
	imx678_write_register(ViPipe, 0x44B6, 0x2C);
	imx678_write_register(ViPipe, 0x44B8, 0x2C);
	imx678_write_register(ViPipe, 0x44BA, 0x2C);
	imx678_write_register(ViPipe, 0x44C4, 0x2C);
	imx678_write_register(ViPipe, 0x44C6, 0x2C);
	imx678_write_register(ViPipe, 0x44C8, 0x2C);
	imx678_write_register(ViPipe, 0x4506, 0xF3);
	imx678_write_register(ViPipe, 0x450E, 0xE5);
	imx678_write_register(ViPipe, 0x4516, 0xF3);
	imx678_write_register(ViPipe, 0x4522, 0xE5);
	imx678_write_register(ViPipe, 0x4524, 0xF3);
	imx678_write_register(ViPipe, 0x452C, 0xE5);
	imx678_write_register(ViPipe, 0x453C, 0x22);
	imx678_write_register(ViPipe, 0x453D, 0x1B);
	imx678_write_register(ViPipe, 0x453E, 0x1B);
	imx678_write_register(ViPipe, 0x453F, 0x15);
	imx678_write_register(ViPipe, 0x4540, 0x15);
	imx678_write_register(ViPipe, 0x4541, 0x15);
	imx678_write_register(ViPipe, 0x4542, 0x15);
	imx678_write_register(ViPipe, 0x4543, 0x15);
	imx678_write_register(ViPipe, 0x4544, 0x15);
	imx678_write_register(ViPipe, 0x4548, 0x00);
	imx678_write_register(ViPipe, 0x4549, 0x01);
	imx678_write_register(ViPipe, 0x454A, 0x01);
	imx678_write_register(ViPipe, 0x454B, 0x06);
	imx678_write_register(ViPipe, 0x454C, 0x06);
	imx678_write_register(ViPipe, 0x454D, 0x06);
	imx678_write_register(ViPipe, 0x454E, 0x06);
	imx678_write_register(ViPipe, 0x454F, 0x06);
	imx678_write_register(ViPipe, 0x4550, 0x06);
	imx678_write_register(ViPipe, 0x4554, 0x55);
	imx678_write_register(ViPipe, 0x4555, 0x02);
	imx678_write_register(ViPipe, 0x4556, 0x42);
	imx678_write_register(ViPipe, 0x4557, 0x05);
	imx678_write_register(ViPipe, 0x4558, 0xFD);
	imx678_write_register(ViPipe, 0x4559, 0x05);
	imx678_write_register(ViPipe, 0x455A, 0x94);
	imx678_write_register(ViPipe, 0x455B, 0x06);
	imx678_write_register(ViPipe, 0x455D, 0x06);
	imx678_write_register(ViPipe, 0x455E, 0x49);
	imx678_write_register(ViPipe, 0x455F, 0x07);
	imx678_write_register(ViPipe, 0x4560, 0x7F);
	imx678_write_register(ViPipe, 0x4561, 0x07);
	imx678_write_register(ViPipe, 0x4562, 0xA5);
	imx678_write_register(ViPipe, 0x4564, 0x55);
	imx678_write_register(ViPipe, 0x4565, 0x02);
	imx678_write_register(ViPipe, 0x4566, 0x42);
	imx678_write_register(ViPipe, 0x4567, 0x05);
	imx678_write_register(ViPipe, 0x4568, 0xFD);
	imx678_write_register(ViPipe, 0x4569, 0x05);
	imx678_write_register(ViPipe, 0x456A, 0x94);
	imx678_write_register(ViPipe, 0x456B, 0x06);
	imx678_write_register(ViPipe, 0x456D, 0x06);
	imx678_write_register(ViPipe, 0x456E, 0x49);
	imx678_write_register(ViPipe, 0x456F, 0x07);
	imx678_write_register(ViPipe, 0x4572, 0xA5);
	imx678_write_register(ViPipe, 0x460C, 0x7D);
	imx678_write_register(ViPipe, 0x460E, 0xB1);
	imx678_write_register(ViPipe, 0x4614, 0xA8);
	imx678_write_register(ViPipe, 0x4616, 0xB2);
	imx678_write_register(ViPipe, 0x461C, 0x7E);
	imx678_write_register(ViPipe, 0x461E, 0xA7);
	imx678_write_register(ViPipe, 0x4624, 0xA8);
	imx678_write_register(ViPipe, 0x4626, 0xB2);
	imx678_write_register(ViPipe, 0x462C, 0x7E);
	imx678_write_register(ViPipe, 0x462E, 0x8A);
	imx678_write_register(ViPipe, 0x4630, 0x94);
	imx678_write_register(ViPipe, 0x4632, 0xA7);
	imx678_write_register(ViPipe, 0x4634, 0xFB);
	imx678_write_register(ViPipe, 0x4636, 0x2F);
	imx678_write_register(ViPipe, 0x4638, 0x81);
	imx678_write_register(ViPipe, 0x4639, 0x01);
	imx678_write_register(ViPipe, 0x463A, 0xB5);
	imx678_write_register(ViPipe, 0x463B, 0x01);
	imx678_write_register(ViPipe, 0x463C, 0x26);
	imx678_write_register(ViPipe, 0x463E, 0x30);
	imx678_write_register(ViPipe, 0x4640, 0xAC);
	imx678_write_register(ViPipe, 0x4641, 0x01);
	imx678_write_register(ViPipe, 0x4642, 0xB6);
	imx678_write_register(ViPipe, 0x4643, 0x01);
	imx678_write_register(ViPipe, 0x4644, 0xFC);
	imx678_write_register(ViPipe, 0x4646, 0x25);
	imx678_write_register(ViPipe, 0x4648, 0x82);
	imx678_write_register(ViPipe, 0x4649, 0x01);
	imx678_write_register(ViPipe, 0x464A, 0xAB);
	imx678_write_register(ViPipe, 0x464B, 0x01);
	imx678_write_register(ViPipe, 0x464C, 0x26);
	imx678_write_register(ViPipe, 0x464E, 0x30);
	imx678_write_register(ViPipe, 0x4654, 0xFC);
	imx678_write_register(ViPipe, 0x4656, 0x08);
	imx678_write_register(ViPipe, 0x4658, 0x12);
	imx678_write_register(ViPipe, 0x465A, 0x25);
	imx678_write_register(ViPipe, 0x4662, 0xFC);
	imx678_write_register(ViPipe, 0x46A2, 0xFB);
	imx678_write_register(ViPipe, 0x46D6, 0xF3);
	imx678_write_register(ViPipe, 0x46E6, 0x00);
	imx678_write_register(ViPipe, 0x46E8, 0xFF);
	imx678_write_register(ViPipe, 0x46E9, 0x03);
	imx678_write_register(ViPipe, 0x46EC, 0x7A);
	imx678_write_register(ViPipe, 0x46EE, 0xE5);
	imx678_write_register(ViPipe, 0x46F4, 0xEE);
	imx678_write_register(ViPipe, 0x46F6, 0xF2);
	imx678_write_register(ViPipe, 0x470C, 0xFF);
	imx678_write_register(ViPipe, 0x470D, 0x03);
	imx678_write_register(ViPipe, 0x470E, 0x00);
	imx678_write_register(ViPipe, 0x4714, 0xE0);
	imx678_write_register(ViPipe, 0x4716, 0xE4);
	imx678_write_register(ViPipe, 0x471E, 0xED);
	imx678_write_register(ViPipe, 0x472E, 0x00);
	imx678_write_register(ViPipe, 0x4730, 0xFF);
	imx678_write_register(ViPipe, 0x4731, 0x03);
	imx678_write_register(ViPipe, 0x4734, 0x7B);
	imx678_write_register(ViPipe, 0x4736, 0xDF);
	imx678_write_register(ViPipe, 0x4754, 0x7D);
	imx678_write_register(ViPipe, 0x4756, 0x8B);
	imx678_write_register(ViPipe, 0x4758, 0x93);
	imx678_write_register(ViPipe, 0x475A, 0xB1);
	imx678_write_register(ViPipe, 0x475C, 0xFB);
	imx678_write_register(ViPipe, 0x475E, 0x09);
	imx678_write_register(ViPipe, 0x4760, 0x11);
	imx678_write_register(ViPipe, 0x4762, 0x2F);
	imx678_write_register(ViPipe, 0x4766, 0xCC);
	imx678_write_register(ViPipe, 0x4776, 0xCB);
	imx678_write_register(ViPipe, 0x477E, 0x4A);
	imx678_write_register(ViPipe, 0x478E, 0x49);
	imx678_write_register(ViPipe, 0x4794, 0x7C);
	imx678_write_register(ViPipe, 0x4796, 0x8F);
	imx678_write_register(ViPipe, 0x4798, 0xB3);
	imx678_write_register(ViPipe, 0x4799, 0x00);
	imx678_write_register(ViPipe, 0x479A, 0xCC);
	imx678_write_register(ViPipe, 0x479C, 0xC1);
	imx678_write_register(ViPipe, 0x479E, 0xCB);
	imx678_write_register(ViPipe, 0x47A4, 0x7D);
	imx678_write_register(ViPipe, 0x47A6, 0x8E);
	imx678_write_register(ViPipe, 0x47A8, 0xB4);
	imx678_write_register(ViPipe, 0x47A9, 0x00);
	imx678_write_register(ViPipe, 0x47AA, 0xC0);
	imx678_write_register(ViPipe, 0x47AC, 0xFA);
	imx678_write_register(ViPipe, 0x47AE, 0x0D);
	imx678_write_register(ViPipe, 0x47B0, 0x31);
	imx678_write_register(ViPipe, 0x47B1, 0x01);
	imx678_write_register(ViPipe, 0x47B2, 0x4A);
	imx678_write_register(ViPipe, 0x47B3, 0x01);
	imx678_write_register(ViPipe, 0x47B4, 0x3F);
	imx678_write_register(ViPipe, 0x47B6, 0x49);
	imx678_write_register(ViPipe, 0x47BC, 0xFB);
	imx678_write_register(ViPipe, 0x47BE, 0x0C);
	imx678_write_register(ViPipe, 0x47C0, 0x32);
	imx678_write_register(ViPipe, 0x47C1, 0x01);
	imx678_write_register(ViPipe, 0x47C2, 0x3E);
	imx678_write_register(ViPipe, 0x47C3, 0x01);
	imx678_write_register(ViPipe, 0x4E3C, 0x07);

	imx678_default_reg_init(ViPipe);

	imx678_write_register(ViPipe, 0x3000, 0x00); /* Set register STANDBY after the other registers. */
	delay_ms(40);
	imx678_write_register(ViPipe, 0x3002, 0x00); /* “Master mode start” by setting register XMSTA to “0” */
	delay_ms(40);
	printf("ViPipe:%d,===IMX678 8M30fps 10bit LINE Init OK!===\n", ViPipe);
}

//25fps 1440Mbps vmax 2200
static void imx678_wdr_5M25_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(4);

	imx678_write_register(ViPipe, 0x3000, 0x01); //STANDBY
	imx678_write_register(ViPipe, 0x3001, 0x00); //REGHOLD
	imx678_write_register(ViPipe, 0x3002, 0x01); //XMSTA
	imx678_write_register(ViPipe, 0x3014, 0x03); //INCK_SEL[3:0]
	imx678_write_register(ViPipe, 0x3015, 0x03); //DATARATE_SEL[3:0]
	imx678_write_register(ViPipe, 0x3018, 0x00); //WINMODE[3:0]
	imx678_write_register(ViPipe, 0x3019, 0x00); //CFMODE
	imx678_write_register(ViPipe, 0x301A, 0x01); //WDMODE[7:0]
	imx678_write_register(ViPipe, 0x301B, 0x00); //ADDMODE[1:0]
	imx678_write_register(ViPipe, 0x301C, 0x01); //THIN_V_EN[7:0]
	imx678_write_register(ViPipe, 0x301E, 0x01); //VCMODE[7:0]
	imx678_write_register(ViPipe, 0x3020, 0x00); //HREVERSE
	imx678_write_register(ViPipe, 0x3021, 0x00); //VREVERSE
	imx678_write_register(ViPipe, 0x3022, 0x01); //ADBIT[1:0]
	imx678_write_register(ViPipe, 0x3023, 0x01); //MDBIT
	imx678_write_register(ViPipe, 0x3028, 0x98); //VMAX[19:0]
	imx678_write_register(ViPipe, 0x3029, 0x08); //VMAX[19:0]
	imx678_write_register(ViPipe, 0x302A, 0x00); //VMAX[19:0]
	imx678_write_register(ViPipe, 0x302C, 0xA3); //HMAX[15:0]
	imx678_write_register(ViPipe, 0x302D, 0x02); //HMAX[15:0]
	imx678_write_register(ViPipe, 0x3030, 0x00); //FDG_SEL0[1:0]
	imx678_write_register(ViPipe, 0x3031, 0x00); //FDG_SEL1[1:0]
	imx678_write_register(ViPipe, 0x3032, 0x00); //FDG_SEL2[1:0]
	imx678_write_register(ViPipe, 0x303C, 0x00); //PIX_HST[12:0]
	imx678_write_register(ViPipe, 0x303D, 0x00); //PIX_HST[12:0]
	imx678_write_register(ViPipe, 0x303E, 0x30); //PIX_HWIDTH[12:0]
	imx678_write_register(ViPipe, 0x303F, 0x0A); //PIX_HWIDTH[12:0]
	imx678_write_register(ViPipe, 0x3040, 0x03); //LANEMODE[2:0]
	imx678_write_register(ViPipe, 0x3044, 0x00); //PIX_VST[11:0]
	imx678_write_register(ViPipe, 0x3045, 0x00); //PIX_VST[11:0]
	imx678_write_register(ViPipe, 0x3046, 0xAC); //PIX_VWIDTH[11:0]
	imx678_write_register(ViPipe, 0x3047, 0x07); //PIX_VWIDTH[11:0]
	imx678_write_register(ViPipe, 0x304C, 0x00); //GAIN_HG0[10:0]
	imx678_write_register(ViPipe, 0x304D, 0x00); //GAIN_HG0[10:0]
	imx678_write_register(ViPipe, 0x3050, 0x50); //SHR0[19:0]
	imx678_write_register(ViPipe, 0x3051, 0x0A); //SHR0[19:0]
	imx678_write_register(ViPipe, 0x3052, 0x00); //SHR0[19:0]
	imx678_write_register(ViPipe, 0x3054, 0x05); //SHR1[19:0]
	imx678_write_register(ViPipe, 0x3055, 0x00); //SHR1[19:0]
	imx678_write_register(ViPipe, 0x3056, 0x00); //SHR1[19:0]
	imx678_write_register(ViPipe, 0x3058, 0x53); //SHR2[19:0]
	imx678_write_register(ViPipe, 0x3059, 0x00); //SHR2[19:0]
	imx678_write_register(ViPipe, 0x305A, 0x00); //SHR2[19:0]
	imx678_write_register(ViPipe, 0x3060, 0x75); //RHS1[19:0]
	imx678_write_register(ViPipe, 0x3061, 0x00); //RHS1[19:0]
	imx678_write_register(ViPipe, 0x3062, 0x00); //RHS1[19:0]
	imx678_write_register(ViPipe, 0x3064, 0x56); //RHS2[19:0]
	imx678_write_register(ViPipe, 0x3065, 0x00); //RHS2[19:0]
	imx678_write_register(ViPipe, 0x3066, 0x00); //RHS2[19:0]
	imx678_write_register(ViPipe, 0x3070, 0x00); //GAIN_0[10:0]
	imx678_write_register(ViPipe, 0x3071, 0x00); //GAIN_0[10:0]
	imx678_write_register(ViPipe, 0x3072, 0x00); //GAIN_1[10:0]
	imx678_write_register(ViPipe, 0x3073, 0x00); //GAIN_1[10:0]
	imx678_write_register(ViPipe, 0x3074, 0x00); //GAIN_2[10:0]
	imx678_write_register(ViPipe, 0x3075, 0x00); //GAIN_2[10:0]
	imx678_write_register(ViPipe, 0x30A4, 0xAA); //XVSOUTSEL[1:0]
	imx678_write_register(ViPipe, 0x30A6, 0x00); //XVS_DRV[1:0]
	imx678_write_register(ViPipe, 0x30CC, 0x00);
	imx678_write_register(ViPipe, 0x30CD, 0x00);
	imx678_write_register(ViPipe, 0x30CE, 0x02);
	imx678_write_register(ViPipe, 0x30DC, 0x32); //BLKLEVEL[9:0]
	imx678_write_register(ViPipe, 0x30DD, 0x40); //BLKLEVEL[9:0]
	imx678_write_register(ViPipe, 0x310C, 0x01);
	imx678_write_register(ViPipe, 0x3130, 0x01);
	imx678_write_register(ViPipe, 0x3148, 0x00);
	imx678_write_register(ViPipe, 0x315E, 0x10);
	imx678_write_register(ViPipe, 0x3400, 0x01); //GAIN_PGC_FIDMD
	imx678_write_register(ViPipe, 0x3460, 0x22);
	imx678_write_register(ViPipe, 0x347B, 0x02);
	imx678_write_register(ViPipe, 0x3492, 0x08);
	imx678_write_register(ViPipe, 0x3890, 0x08); //HFR_EN[3:0]
	imx678_write_register(ViPipe, 0x3891, 0x00); //HFR_EN[3:0]
	imx678_write_register(ViPipe, 0x3893, 0x00);
	imx678_write_register(ViPipe, 0x3B1D, 0x17);
	imx678_write_register(ViPipe, 0x3B44, 0x3F);
	imx678_write_register(ViPipe, 0x3B60, 0x03);
	imx678_write_register(ViPipe, 0x3C03, 0x04);
	imx678_write_register(ViPipe, 0x3C04, 0x04);
	imx678_write_register(ViPipe, 0x3C0A, 0x1F);
	imx678_write_register(ViPipe, 0x3C0B, 0x1F);
	imx678_write_register(ViPipe, 0x3C0C, 0x1F);
	imx678_write_register(ViPipe, 0x3C0D, 0x1F);
	imx678_write_register(ViPipe, 0x3C0E, 0x1F);
	imx678_write_register(ViPipe, 0x3C0F, 0x1F);
	imx678_write_register(ViPipe, 0x3C30, 0x73);
	imx678_write_register(ViPipe, 0x3C3C, 0x20);
	imx678_write_register(ViPipe, 0x3C44, 0x06);
	imx678_write_register(ViPipe, 0x3C7C, 0xB9);
	imx678_write_register(ViPipe, 0x3C7D, 0x01);
	imx678_write_register(ViPipe, 0x3C7E, 0xB7);
	imx678_write_register(ViPipe, 0x3C7F, 0x01);
	imx678_write_register(ViPipe, 0x3CB0, 0x00);
	imx678_write_register(ViPipe, 0x3CB2, 0xFF);
	imx678_write_register(ViPipe, 0x3CB3, 0x03);
	imx678_write_register(ViPipe, 0x3CB4, 0xFF);
	imx678_write_register(ViPipe, 0x3CB5, 0x03);
	imx678_write_register(ViPipe, 0x3CBA, 0xFF);
	imx678_write_register(ViPipe, 0x3CBB, 0x03);
	imx678_write_register(ViPipe, 0x3CC0, 0xFF);
	imx678_write_register(ViPipe, 0x3CC1, 0x03);
	imx678_write_register(ViPipe, 0x3CC2, 0x00);
	imx678_write_register(ViPipe, 0x3CC6, 0xFF);
	imx678_write_register(ViPipe, 0x3CC7, 0x03);
	imx678_write_register(ViPipe, 0x3CC8, 0xFF);
	imx678_write_register(ViPipe, 0x3CC9, 0x03);
	imx678_write_register(ViPipe, 0x3E00, 0x1E);
	imx678_write_register(ViPipe, 0x3E02, 0x04);
	imx678_write_register(ViPipe, 0x3E03, 0x00);
	imx678_write_register(ViPipe, 0x3E20, 0x04);
	imx678_write_register(ViPipe, 0x3E21, 0x00);
	imx678_write_register(ViPipe, 0x3E22, 0x1E);
	imx678_write_register(ViPipe, 0x3E24, 0xBA);
	imx678_write_register(ViPipe, 0x3E72, 0x85);
	imx678_write_register(ViPipe, 0x3E76, 0x0C);
	imx678_write_register(ViPipe, 0x3E77, 0x01);
	imx678_write_register(ViPipe, 0x3E7A, 0x85);
	imx678_write_register(ViPipe, 0x3E7E, 0x1F);
	imx678_write_register(ViPipe, 0x3E82, 0xA6);
	imx678_write_register(ViPipe, 0x3E86, 0x2D);
	imx678_write_register(ViPipe, 0x3EE2, 0x33);
	imx678_write_register(ViPipe, 0x3EE3, 0x03);
	imx678_write_register(ViPipe, 0x4490, 0x07);
	imx678_write_register(ViPipe, 0x4494, 0x19);
	imx678_write_register(ViPipe, 0x4495, 0x00);
	imx678_write_register(ViPipe, 0x4496, 0xBB);
	imx678_write_register(ViPipe, 0x4497, 0x00);
	imx678_write_register(ViPipe, 0x4498, 0x55);
	imx678_write_register(ViPipe, 0x449A, 0x50);
	imx678_write_register(ViPipe, 0x449C, 0x50);
	imx678_write_register(ViPipe, 0x449E, 0x50);
	imx678_write_register(ViPipe, 0x44A0, 0x3C);
	imx678_write_register(ViPipe, 0x44A2, 0x19);
	imx678_write_register(ViPipe, 0x44A4, 0x19);
	imx678_write_register(ViPipe, 0x44A6, 0x19);
	imx678_write_register(ViPipe, 0x44A8, 0x4B);
	imx678_write_register(ViPipe, 0x44AA, 0x4B);
	imx678_write_register(ViPipe, 0x44AC, 0x4B);
	imx678_write_register(ViPipe, 0x44AE, 0x4B);
	imx678_write_register(ViPipe, 0x44B0, 0x3C);
	imx678_write_register(ViPipe, 0x44B2, 0x19);
	imx678_write_register(ViPipe, 0x44B4, 0x19);
	imx678_write_register(ViPipe, 0x44B6, 0x19);
	imx678_write_register(ViPipe, 0x44B8, 0x4B);
	imx678_write_register(ViPipe, 0x44BA, 0x4B);
	imx678_write_register(ViPipe, 0x44BC, 0x4B);
	imx678_write_register(ViPipe, 0x44BE, 0x4B);
	imx678_write_register(ViPipe, 0x44C0, 0x3C);
	imx678_write_register(ViPipe, 0x44C2, 0x19);
	imx678_write_register(ViPipe, 0x44C4, 0x19);
	imx678_write_register(ViPipe, 0x44C6, 0x19);
	imx678_write_register(ViPipe, 0x44C8, 0xF0);
	imx678_write_register(ViPipe, 0x44CA, 0xEB);
	imx678_write_register(ViPipe, 0x44CC, 0xEB);
	imx678_write_register(ViPipe, 0x44CE, 0xE6);
	imx678_write_register(ViPipe, 0x44D0, 0xE6);
	imx678_write_register(ViPipe, 0x44D2, 0xBB);
	imx678_write_register(ViPipe, 0x44D4, 0xBB);
	imx678_write_register(ViPipe, 0x44D6, 0xBB);
	imx678_write_register(ViPipe, 0x44D8, 0xE6);
	imx678_write_register(ViPipe, 0x44DA, 0xE6);
	imx678_write_register(ViPipe, 0x44DC, 0xE6);
	imx678_write_register(ViPipe, 0x44DE, 0xE6);
	imx678_write_register(ViPipe, 0x44E0, 0xE6);
	imx678_write_register(ViPipe, 0x44E2, 0xBB);
	imx678_write_register(ViPipe, 0x44E4, 0xBB);
	imx678_write_register(ViPipe, 0x44E6, 0xBB);
	imx678_write_register(ViPipe, 0x44E8, 0xE6);
	imx678_write_register(ViPipe, 0x44EA, 0xE6);
	imx678_write_register(ViPipe, 0x44EC, 0xE6);
	imx678_write_register(ViPipe, 0x44EE, 0xE6);
	imx678_write_register(ViPipe, 0x44F0, 0xE6);
	imx678_write_register(ViPipe, 0x44F2, 0xBB);
	imx678_write_register(ViPipe, 0x44F4, 0xBB);
	imx678_write_register(ViPipe, 0x44F6, 0xBB);
	imx678_write_register(ViPipe, 0x4538, 0x15);
	imx678_write_register(ViPipe, 0x4539, 0x15);
	imx678_write_register(ViPipe, 0x453A, 0x15);
	imx678_write_register(ViPipe, 0x4544, 0x15);
	imx678_write_register(ViPipe, 0x4545, 0x15);
	imx678_write_register(ViPipe, 0x4546, 0x15);
	imx678_write_register(ViPipe, 0x4550, 0x10);
	imx678_write_register(ViPipe, 0x4551, 0x10);
	imx678_write_register(ViPipe, 0x4552, 0x10);
	imx678_write_register(ViPipe, 0x4553, 0x10);
	imx678_write_register(ViPipe, 0x4554, 0x10);
	imx678_write_register(ViPipe, 0x4555, 0x10);
	imx678_write_register(ViPipe, 0x4556, 0x10);
	imx678_write_register(ViPipe, 0x4557, 0x10);
	imx678_write_register(ViPipe, 0x4558, 0x10);
	imx678_write_register(ViPipe, 0x455C, 0x10);
	imx678_write_register(ViPipe, 0x455D, 0x10);
	imx678_write_register(ViPipe, 0x455E, 0x10);
	imx678_write_register(ViPipe, 0x455F, 0x10);
	imx678_write_register(ViPipe, 0x4560, 0x10);
	imx678_write_register(ViPipe, 0x4561, 0x10);
	imx678_write_register(ViPipe, 0x4562, 0x10);
	imx678_write_register(ViPipe, 0x4563, 0x10);
	imx678_write_register(ViPipe, 0x4564, 0x10);
	imx678_write_register(ViPipe, 0x4569, 0x01);
	imx678_write_register(ViPipe, 0x456A, 0x01);
	imx678_write_register(ViPipe, 0x456B, 0x06);
	imx678_write_register(ViPipe, 0x456C, 0x06);
	imx678_write_register(ViPipe, 0x456D, 0x06);
	imx678_write_register(ViPipe, 0x456E, 0x06);
	imx678_write_register(ViPipe, 0x456F, 0x06);
	imx678_write_register(ViPipe, 0x4570, 0x06);

	imx678_default_reg_init(ViPipe);

	imx678_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(80);
	imx678_write_register(ViPipe, 0x3002, 0x00); /* master mode start */

	printf("ViPipe:%d,===Imx678 5M25fps 12bit 2to1 WDR Init Ok!====\n", ViPipe);
}