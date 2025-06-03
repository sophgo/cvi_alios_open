#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx327_sublvds_cmos_ex.h"
#include "sensor_i2c.h"

static void imx327_sublvds_wdr_1080p30_2to1_init(VI_PIPE ViPipe);
static void imx327_sublvds_linear_1080p30_init(VI_PIPE ViPipe);

CVI_U8 imx327_sublvds_i2c_addr = 0x36;
const CVI_U32 imx327_sublvds_addr_byte = 2;
const CVI_U32 imx327_sublvds_data_byte = 1;

int imx327_sublvds_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx327_sublvds_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx327_sublvds_AddrInfo[ViPipe].s8I2cAddr);
}

int imx327_sublvds_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx327_sublvds_BusInfo[ViPipe].s8I2cDev);
}

int imx327_sublvds_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx327_sublvds_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx327_sublvds_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx327_sublvds_addr_byte, imx327_sublvds_data_byte);
}

int imx327_sublvds_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx327_sublvds_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx327_sublvds_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx327_sublvds_addr_byte, (CVI_U32)data, imx327_sublvds_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx327_sublvds_standby(VI_PIPE ViPipe)
{
	imx327_sublvds_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx327_sublvds_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */
}

void imx327_sublvds_restart(VI_PIPE ViPipe)
{
	imx327_sublvds_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx327_sublvds_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	imx327_sublvds_write_register(ViPipe, 0x304b, 0x0a);
}

void imx327_sublvds_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx327_sublvds[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx327_sublvds_write_register(ViPipe,
				g_pastImx327_sublvds[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx327_sublvds[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define IMX327_CHIP_ID_ADDR	0x31dc
#define IMX327_CHIP_ID		0x6
#define IMX327_CHIP_ID_MASK	0x6

void imx327_sublvds_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = imx327_sublvds_read_register(ViPipe, 0x3007) & ~0x3;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x2;
		break;
	case ISP_SNS_FLIP:
		val |= 0x1;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x3;
		break;
	default:
		return;
	}

	imx327_sublvds_write_register(ViPipe, 0x3007, val);
}

int imx327_sublvds_probe(VI_PIPE ViPipe)
{
	int nVal;

	usleep(100);
	if (imx327_sublvds_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = imx327_sublvds_read_register(ViPipe, IMX327_CHIP_ID_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((nVal & IMX327_CHIP_ID_MASK) != IMX327_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void imx327_sublvds_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx327_sublvds[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx327_sublvds[ViPipe]->u8ImgMode;

	imx327_sublvds_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == IMX327_SUBLVDS_MODE_1080P30_WDR) {
			imx327_sublvds_wdr_1080p30_2to1_init(ViPipe);
		}
	} else {
		imx327_sublvds_linear_1080p30_init(ViPipe);
	}
	g_pastImx327_sublvds[ViPipe]->bInit = CVI_TRUE;
}

/* 1080P30 and 1080P25 */
static void imx327_sublvds_linear_1080p30_init(VI_PIPE ViPipe)
{
	imx327_sublvds_write_register(ViPipe, 0x3003, 0x01); /* SW RESET */
	delay_ms(4);
	imx327_sublvds_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx327_sublvds_write_register(ViPipe, 0x3002, 0x00); /* XMSTA */
	imx327_sublvds_write_register(ViPipe, 0x3005, 0x01); /* ADBIT 10bit */
	imx327_sublvds_write_register(ViPipe, 0x3007, 0x00); /* VREVERS, ... */
	imx327_sublvds_write_register(ViPipe, 0x3009, 0x02);
	imx327_sublvds_write_register(ViPipe, 0x300A, 0xF0); /* BLKLEVEL */
	imx327_sublvds_write_register(ViPipe, 0x3011, 0x02);
	imx327_sublvds_write_register(ViPipe, 0x3014, 0x16); /* GAIN, 0x2A=>12.6dB TBD */
	imx327_sublvds_write_register(ViPipe, 0x3018, 0x65); /* VMAX[7:0] */
	imx327_sublvds_write_register(ViPipe, 0x3019, 0x04); /* VMAX[15:8] */
	imx327_sublvds_write_register(ViPipe, 0x301A, 0x00); /* VMAX[17:16]:=:0x301A[1:0] */
	imx327_sublvds_write_register(ViPipe, 0x301C, 0x30); /* HMAX[7:0], TBD */
	imx327_sublvds_write_register(ViPipe, 0x301D, 0x11); /* HMAX[15:8] */
	imx327_sublvds_write_register(ViPipe, 0x3020, 0x8C); /* SHS[7:0], TBD */
	imx327_sublvds_write_register(ViPipe, 0x3021, 0x01); /* SHS[15:8] */
	imx327_sublvds_write_register(ViPipe, 0x3022, 0x00); /* SHS[19:16] */
	imx327_sublvds_write_register(ViPipe, 0x3046, 0xE1);
	imx327_sublvds_write_register(ViPipe, 0x304B, 0x0A);
	imx327_sublvds_write_register(ViPipe, 0x305C, 0x18); /* INCKSEL1 */
	imx327_sublvds_write_register(ViPipe, 0x305D, 0x00); /* INCKSEL2 */
	imx327_sublvds_write_register(ViPipe, 0x305E, 0x20); /* INCKSEL3 */
	imx327_sublvds_write_register(ViPipe, 0x305F, 0x01); /* INCKSEL4 */
	imx327_sublvds_write_register(ViPipe, 0x309E, 0x4A);
	imx327_sublvds_write_register(ViPipe, 0x309F, 0x4A);
	imx327_sublvds_write_register(ViPipe, 0x30D2, 0x19);
	imx327_sublvds_write_register(ViPipe, 0x30D7, 0x03);
	imx327_sublvds_write_register(ViPipe, 0x3129, 0x00);
	imx327_sublvds_write_register(ViPipe, 0x313B, 0x61);
	imx327_sublvds_write_register(ViPipe, 0x315E, 0x1A);
	imx327_sublvds_write_register(ViPipe, 0x3164, 0x1A);
	imx327_sublvds_write_register(ViPipe, 0x317C, 0x00);
	imx327_sublvds_write_register(ViPipe, 0x31EC, 0x0E);
	imx327_sublvds_write_register(ViPipe, 0x3480, 0x49); /* incksel7 */

	imx327_sublvds_default_reg_init(ViPipe);

	imx327_sublvds_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx327_sublvds_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	imx327_sublvds_write_register(ViPipe, 0x304b, 0x0a);

	printf("ViPipe:%d,===IMX327 1080P 30fps 12bit LINE Init OK!===\n", ViPipe);
}

static void imx327_sublvds_wdr_1080p30_2to1_init(VI_PIPE ViPipe)
{
	imx327_sublvds_write_register(ViPipe, 0x3003, 0x01); /* SW RESET */
	delay_ms(4);
	imx327_sublvds_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx327_sublvds_write_register(ViPipe, 0x3002, 0x00); /* XMSTA */
	imx327_sublvds_write_register(ViPipe, 0x3005, 0x01); /* ADBIT */
	imx327_sublvds_write_register(ViPipe, 0x3007, 0x00); /* VREVERS, ... */
	imx327_sublvds_write_register(ViPipe, 0x3009, 0x01);
	imx327_sublvds_write_register(ViPipe, 0x300A, 0xF0); /* BLKLEVEL */
	imx327_sublvds_write_register(ViPipe, 0x300C, 0x11); /* WDMODE */
	imx327_sublvds_write_register(ViPipe, 0x3011, 0x02);
	imx327_sublvds_write_register(ViPipe, 0x3014, 0x16); /* GAIN, 0x2A=>12.6dB TBD */
	imx327_sublvds_write_register(ViPipe, 0x3018, 0x65); /* VMAX[7:0] */
	imx327_sublvds_write_register(ViPipe, 0x3019, 0x04); /* VMAX[15:8] */
	imx327_sublvds_write_register(ViPipe, 0x301A, 0x00); /* VMAX[17:16]:=:0x301A[1:0] */
	imx327_sublvds_write_register(ViPipe, 0x301C, 0x98); /* HMAX[7:0], TBD */
	imx327_sublvds_write_register(ViPipe, 0x301D, 0x08); /* HMAX[15:8] */
	imx327_sublvds_write_register(ViPipe, 0x3020, 0x02); /* SHS[7:0], TBD */
	imx327_sublvds_write_register(ViPipe, 0x3021, 0x00); /* SHS[15:8] */
	imx327_sublvds_write_register(ViPipe, 0x3022, 0x00); /* SHS[19:16] */
	imx327_sublvds_write_register(ViPipe, 0x3024, 0xC9); /* SHS2[7:0], TBD */
	imx327_sublvds_write_register(ViPipe, 0x3025, 0x07); /* SHS2[15:8] */
	imx327_sublvds_write_register(ViPipe, 0x3026, 0x00); /* SHS2[19:16] */
	imx327_sublvds_write_register(ViPipe, 0x3030, 0x0B); /* RHS1[7:0], TBD */
	imx327_sublvds_write_register(ViPipe, 0x3031, 0x00); /* RHS1[15:8] */
	imx327_sublvds_write_register(ViPipe, 0x3032, 0x00); /* RHS1[19:16] */
	imx327_sublvds_write_register(ViPipe, 0x3045, 0x03); /* DOLSCDEN [0] 1: pattern1 0: pattern2 */
	imx327_sublvds_write_register(ViPipe, 0x3046, 0xE1);
	imx327_sublvds_write_register(ViPipe, 0x304B, 0x0A);
	imx327_sublvds_write_register(ViPipe, 0x305C, 0x18); /* INCKSEL1 */
	imx327_sublvds_write_register(ViPipe, 0x305D, 0x00); /* INCKSEL2 */
	imx327_sublvds_write_register(ViPipe, 0x305E, 0x20); /* INCKSEL3 */
	imx327_sublvds_write_register(ViPipe, 0x305F, 0x01); /* INCKSEL4 */
	imx327_sublvds_write_register(ViPipe, 0x309E, 0x4A);
	imx327_sublvds_write_register(ViPipe, 0x309F, 0x4A);
	imx327_sublvds_write_register(ViPipe, 0x30D2, 0x19);
	imx327_sublvds_write_register(ViPipe, 0x30D7, 0x03);
	imx327_sublvds_write_register(ViPipe, 0x3106, 0x11); /*DOLHBFIXEN[7] 0: pattern1 1: pattern2 */
	imx327_sublvds_write_register(ViPipe, 0x3129, 0x00);
	imx327_sublvds_write_register(ViPipe, 0x313B, 0x61);
	imx327_sublvds_write_register(ViPipe, 0x315E, 0x1A);
	imx327_sublvds_write_register(ViPipe, 0x3164, 0x1A);
	imx327_sublvds_write_register(ViPipe, 0x317C, 0x00);
	imx327_sublvds_write_register(ViPipe, 0x31EC, 0x0E);
	imx327_sublvds_write_register(ViPipe, 0x3415, 0x00); /* NULL0_SIZE_V, set to 00h when DOL */
	imx327_sublvds_write_register(ViPipe, 0x3480, 0x49);

	imx327_sublvds_default_reg_init(ViPipe);

	if (g_au16Imx327_sublvds_GainMode[ViPipe] == SNS_GAIN_MODE_SHARE) {
		imx327_sublvds_write_register(ViPipe, 0x30F0, 0xF0);
		imx327_sublvds_write_register(ViPipe, 0x3010, 0x21);
	} else {
		imx327_sublvds_write_register(ViPipe, 0x30F0, 0x64);
		imx327_sublvds_write_register(ViPipe, 0x3010, 0x61);
	}

	imx327_sublvds_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx327_sublvds_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	imx327_sublvds_write_register(ViPipe, 0x304b, 0x0a);

	printf("===Imx327_sublvds sensor 1080P30fps 12bit 2to1 WDR(60fps->30fps) init success!=====\n");
}
