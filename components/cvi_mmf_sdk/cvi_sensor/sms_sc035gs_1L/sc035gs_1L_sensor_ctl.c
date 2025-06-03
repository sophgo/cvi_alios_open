#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc035gs_1L_cmos_ex.h"
#include "sensor_i2c.h"

static void sc035gs_1L_linear_1296P30_init(VI_PIPE ViPipe);

const CVI_U8 sc035gs_1L_i2c_addr = 0x30;        /* I2C Address of SC035GS_1L */
const CVI_U32 sc035gs_1L_addr_byte = 2;
const CVI_U32 sc035gs_1L_data_byte = 1;

int sc035gs_1L_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC035GS_1L_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC035GS_1L_AddrInfo[ViPipe].s8I2cAddr);
}

int sc035gs_1L_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC035GS_1L_BusInfo[ViPipe].s8I2cDev);
}

int sc035gs_1L_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC035GS_1L_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC035GS_1L_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc035gs_1L_addr_byte, sc035gs_1L_data_byte);
}

int sc035gs_1L_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC035GS_1L_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC035GS_1L_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc035gs_1L_addr_byte, (CVI_U32)data, sc035gs_1L_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc035gs_1L_standby(VI_PIPE ViPipe)
{
	sc035gs_1L_write_register(ViPipe, 0x0100, 0x00);
}

void sc035gs_1L_restart(VI_PIPE ViPipe)
{
	sc035gs_1L_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc035gs_1L_write_register(ViPipe, 0x0100, 0x01);
}

void sc035gs_1L_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC035GS_1L[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastSC035GS_1L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			sc035gs_1L_write_register(ViPipe,
				g_pastSC035GS_1L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC035GS_1L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void sc035gs_1L_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = sc035gs_1L_read_register(ViPipe, 0x3221) & ~0x66;

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

	sc035gs_1L_write_register(ViPipe, 0x3221, val);
}

void sc035gs_1L_init(VI_PIPE ViPipe)
{
	sc035gs_1L_i2c_init(ViPipe);

	//linear mode only
	sc035gs_1L_linear_1296P30_init(ViPipe);

	g_pastSC035GS_1L[ViPipe]->bInit = CVI_TRUE;
}

/* 1296P30 and 1296P25 */
static void sc035gs_1L_linear_1296P30_init(VI_PIPE ViPipe)
{
	sc035gs_1L_write_register(ViPipe, 0x0103, 0x01);
	delay_ms(33);
	sc035gs_1L_write_register(ViPipe, 0x0100, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x36e9, 0x80);
	sc035gs_1L_write_register(ViPipe, 0x36f9, 0x80);
	sc035gs_1L_write_register(ViPipe, 0x3001, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x3000, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x300f, 0x0f);
	sc035gs_1L_write_register(ViPipe, 0x3018, 0x13);
	sc035gs_1L_write_register(ViPipe, 0x3019, 0xfe);
	sc035gs_1L_write_register(ViPipe, 0x301c, 0x78);
	sc035gs_1L_write_register(ViPipe, 0x301f, 0x07);
	sc035gs_1L_write_register(ViPipe, 0x3031, 0x0a);
	sc035gs_1L_write_register(ViPipe, 0x3037, 0x20);
	sc035gs_1L_write_register(ViPipe, 0x303f, 0x01);
	sc035gs_1L_write_register(ViPipe, 0x320c, 0x03);
	sc035gs_1L_write_register(ViPipe, 0x320d, 0x6e);
	sc035gs_1L_write_register(ViPipe, 0x320e, 0x02);
	sc035gs_1L_write_register(ViPipe, 0x320f, 0xab);
	sc035gs_1L_write_register(ViPipe, 0x3220, 0x10);
	sc035gs_1L_write_register(ViPipe, 0x3250, 0xc0);
	sc035gs_1L_write_register(ViPipe, 0x3251, 0x02);
	sc035gs_1L_write_register(ViPipe, 0x3252, 0x02);
	sc035gs_1L_write_register(ViPipe, 0x3253, 0x08);
//	sc035gs_1L_write_register(ViPipe, 0x3252, 0x02);
//	sc035gs_1L_write_register(ViPipe, 0x3253, 0xa6);
	sc035gs_1L_write_register(ViPipe, 0x3254, 0x02);
	sc035gs_1L_write_register(ViPipe, 0x3255, 0x07);
	sc035gs_1L_write_register(ViPipe, 0x3304, 0x48);
	sc035gs_1L_write_register(ViPipe, 0x3306, 0x38);
	sc035gs_1L_write_register(ViPipe, 0x3309, 0x68);
	sc035gs_1L_write_register(ViPipe, 0x330b, 0xe0);
	sc035gs_1L_write_register(ViPipe, 0x330c, 0x18);
	sc035gs_1L_write_register(ViPipe, 0x330f, 0x20);
	sc035gs_1L_write_register(ViPipe, 0x3310, 0x10);
	sc035gs_1L_write_register(ViPipe, 0x3314, 0x1e);
	sc035gs_1L_write_register(ViPipe, 0x3315, 0x38);
	sc035gs_1L_write_register(ViPipe, 0x3316, 0x40);
	sc035gs_1L_write_register(ViPipe, 0x3317, 0x10);
	sc035gs_1L_write_register(ViPipe, 0x3329, 0x34);
	sc035gs_1L_write_register(ViPipe, 0x332d, 0x34);
	sc035gs_1L_write_register(ViPipe, 0x332f, 0x38);
	sc035gs_1L_write_register(ViPipe, 0x3335, 0x3c);
	sc035gs_1L_write_register(ViPipe, 0x3344, 0x3c);
	sc035gs_1L_write_register(ViPipe, 0x335b, 0x80);
	sc035gs_1L_write_register(ViPipe, 0x335f, 0x80);
	sc035gs_1L_write_register(ViPipe, 0x3366, 0x06);
	sc035gs_1L_write_register(ViPipe, 0x3385, 0x31);
	sc035gs_1L_write_register(ViPipe, 0x3387, 0x51);
	sc035gs_1L_write_register(ViPipe, 0x3389, 0x01);
	sc035gs_1L_write_register(ViPipe, 0x33b1, 0x03);
	sc035gs_1L_write_register(ViPipe, 0x33b2, 0x06);
	sc035gs_1L_write_register(ViPipe, 0x3621, 0xa4);
	sc035gs_1L_write_register(ViPipe, 0x3622, 0x05);
	sc035gs_1L_write_register(ViPipe, 0x3624, 0x47);
	sc035gs_1L_write_register(ViPipe, 0x3630, 0x46);
	sc035gs_1L_write_register(ViPipe, 0x3631, 0x48);
	sc035gs_1L_write_register(ViPipe, 0x3633, 0x52);
	sc035gs_1L_write_register(ViPipe, 0x3635, 0x18);
	sc035gs_1L_write_register(ViPipe, 0x3636, 0x25);
	sc035gs_1L_write_register(ViPipe, 0x3637, 0x89);
	sc035gs_1L_write_register(ViPipe, 0x3638, 0x0f);
	sc035gs_1L_write_register(ViPipe, 0x3639, 0x08);
	sc035gs_1L_write_register(ViPipe, 0x363a, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x363b, 0x48);
	sc035gs_1L_write_register(ViPipe, 0x363c, 0x06);
	sc035gs_1L_write_register(ViPipe, 0x363d, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x363e, 0xf8);
	sc035gs_1L_write_register(ViPipe, 0x3640, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x3641, 0x01);
	sc035gs_1L_write_register(ViPipe, 0x36ea, 0x3b);
	sc035gs_1L_write_register(ViPipe, 0x36eb, 0x0e);
	sc035gs_1L_write_register(ViPipe, 0x36ec, 0x0e);
	sc035gs_1L_write_register(ViPipe, 0x36ed, 0x33);
	sc035gs_1L_write_register(ViPipe, 0x36fa, 0x3a);
	sc035gs_1L_write_register(ViPipe, 0x36fc, 0x01);
	sc035gs_1L_write_register(ViPipe, 0x3908, 0x91);
	sc035gs_1L_write_register(ViPipe, 0x3d08, 0x01);
	sc035gs_1L_write_register(ViPipe, 0x3e01, 0x14);
	sc035gs_1L_write_register(ViPipe, 0x3e02, 0x80);
	sc035gs_1L_write_register(ViPipe, 0x3e06, 0x0c);
	sc035gs_1L_write_register(ViPipe, 0x4500, 0x59);
	sc035gs_1L_write_register(ViPipe, 0x4501, 0xc4);
	sc035gs_1L_write_register(ViPipe, 0x4603, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x4809, 0x01);
	sc035gs_1L_write_register(ViPipe, 0x4837, 0x1b);
	sc035gs_1L_write_register(ViPipe, 0x5011, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x36e9, 0x00);
	sc035gs_1L_write_register(ViPipe, 0x36f9, 0x00);

	sc035gs_1L_default_reg_init(ViPipe);

	sc035gs_1L_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(18);
	sc035gs_1L_write_register(ViPipe, 0x4418, 0x08);
	sc035gs_1L_write_register(ViPipe, 0x4419, 0x8e);
	delay_ms(100);

	printf("ViPipe:%d,===SC035GS_1L 480P 120fps 10bit LINE Init OK!===\n", ViPipe);
}
