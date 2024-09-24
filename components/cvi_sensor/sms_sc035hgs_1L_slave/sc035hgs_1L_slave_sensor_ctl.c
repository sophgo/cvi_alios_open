#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "sc035hgs_1L_slave_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

static void sc035hgs_1L_slave_linear_1296P30_init(VI_PIPE ViPipe);

CVI_U8 sc035hgs_1L_slave_i2c_addr = 0x32;        /* I2C Address of SC035HGS_1L */
CVI_U32 sc035hgs_1L_slave_addr_byte = 2;
CVI_U32 sc035hgs_1L_slave_data_byte = 1;
// static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int sc035hgs_1L_slave_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC035HGS_1L_SLAVE_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc035hgs_1L_slave_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC035HGS_1L_SLAVE_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc035hgs_1L_slave_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC035HGS_1L_SLAVE_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc035hgs_1L_slave_i2c_addr, (CVI_U32)addr, sc035hgs_1L_slave_addr_byte, sc035hgs_1L_slave_data_byte);

}


int sc035hgs_1L_slave_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSC035HGS_1L_SLAVE_BusInfo[ViPipe].s8I2cDev;
	// printf("%s %d i2cid:%#x, i2c_addr:%#x, addr:%#x, addrbyte:%d, databyte:%d, data:%#x. \n"
	// 	, __func__, __LINE__, i2c_id, sc035hgs_1L_slave_i2c_addr, addr, sc035hgs_1L_slave_addr_byte, sc035hgs_1L_slave_data_byte, data);
	return sensor_i2c_write(i2c_id, sc035hgs_1L_slave_i2c_addr, (CVI_U32)addr, sc035hgs_1L_slave_addr_byte,
				(CVI_U32)data, sc035hgs_1L_slave_data_byte);

}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc035hgs_1L_slave_standby(VI_PIPE ViPipe)
{
	sc035hgs_1L_slave_write_register(ViPipe, 0x0100, 0x00);
}

void sc035hgs_1L_slave_restart(VI_PIPE ViPipe)
{
	sc035hgs_1L_slave_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc035hgs_1L_slave_write_register(ViPipe, 0x0100, 0x01);
}

void sc035hgs_1L_slave_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC035HGS_1L_SLAVE[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastSC035HGS_1L_SLAVE[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			sc035hgs_1L_slave_write_register(ViPipe,
				g_pastSC035HGS_1L_SLAVE[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC035HGS_1L_SLAVE[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void sc035hgs_1L_slave_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc035hgs_1L_slave_write_register(ViPipe, 0x3221, val);
}

#define SC035HGS_1L_SLAVE_CHIP_ID_ADDR_H    0x3108
#define SC035HGS_1L_SLAVE_CHIP_ID_ADDR_L    0x3109
#define SC035HGS_1L_SLAVE_CHIP_ID           0x310B

int sc035hgs_1L_slave_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (sc035hgs_1L_slave_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = sc035hgs_1L_slave_read_register(ViPipe, SC035HGS_1L_SLAVE_CHIP_ID_ADDR_H);
	nVal2 = sc035hgs_1L_slave_read_register(ViPipe, SC035HGS_1L_SLAVE_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != SC035HGS_1L_SLAVE_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sc035hgs_1L_slave_init(VI_PIPE ViPipe)
{
	sc035hgs_1L_slave_i2c_init(ViPipe);

	//linear mode only
	sc035hgs_1L_slave_linear_1296P30_init(ViPipe);

	g_pastSC035HGS_1L_SLAVE[ViPipe]->bInit = CVI_TRUE;
}

void sc035hgs_1L_slave_exit(VI_PIPE ViPipe)
{
	sc035hgs_1L_slave_i2c_exit(ViPipe);
}

/* 1296P30 and 1296P25 */
static void sc035hgs_1L_slave_linear_1296P30_init(VI_PIPE ViPipe)
{
	sc035hgs_1L_slave_write_register(ViPipe, 0x0103, 0x01);
	delay_ms(33);
	sc035hgs_1L_slave_write_register(ViPipe, 0x0100, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x0100, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36e9, 0x80);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36f9, 0x80);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3000, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3001, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x300f, 0x0f);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3018, 0x13);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3019, 0xfe);
	sc035hgs_1L_slave_write_register(ViPipe, 0x301c, 0x78);
	sc035hgs_1L_slave_write_register(ViPipe, 0x301f, 0x62);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3031, 0x0a);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3037, 0x20);
	sc035hgs_1L_slave_write_register(ViPipe, 0x303f, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x320c, 0x04);
	sc035hgs_1L_slave_write_register(ViPipe, 0x320d, 0x70);
	sc035hgs_1L_slave_write_register(ViPipe, 0x320e, 0x02);
	sc035hgs_1L_slave_write_register(ViPipe, 0x320f, 0x10);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3217, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3218, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3220, 0x10);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3223, 0x48);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3226, 0x74);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3227, 0x07);
	sc035hgs_1L_slave_write_register(ViPipe, 0x323b, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3250, 0xf0);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3251, 0x02);	// 0x1
	sc035hgs_1L_slave_write_register(ViPipe, 0x3252, 0x02);	// 0xf8
	sc035hgs_1L_slave_write_register(ViPipe, 0x3253, 0x08);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3254, 0x02);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3255, 0x07);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3304, 0x48);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3305, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3306, 0x98);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3309, 0x50);
	sc035hgs_1L_slave_write_register(ViPipe, 0x330a, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x330b, 0x18);
	sc035hgs_1L_slave_write_register(ViPipe, 0x330c, 0x18);
	sc035hgs_1L_slave_write_register(ViPipe, 0x330f, 0x40);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3310, 0x10);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3314, 0x6b);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3315, 0x30);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3316, 0x68);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3317, 0x14);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3329, 0x5c);
	sc035hgs_1L_slave_write_register(ViPipe, 0x332d, 0x5c);
	sc035hgs_1L_slave_write_register(ViPipe, 0x332f, 0x60);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3335, 0x64);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3344, 0x64);
	sc035hgs_1L_slave_write_register(ViPipe, 0x335b, 0x80);
	sc035hgs_1L_slave_write_register(ViPipe, 0x335f, 0x80);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3366, 0x06);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3385, 0x31);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3387, 0x39);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3389, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x33b1, 0x03);
	sc035hgs_1L_slave_write_register(ViPipe, 0x33b2, 0x06);
	sc035hgs_1L_slave_write_register(ViPipe, 0x33bd, 0xe0);
	sc035hgs_1L_slave_write_register(ViPipe, 0x33bf, 0x10);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3621, 0xa4);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3622, 0x05);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3624, 0x47);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3630, 0x4a);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3631, 0x58);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3633, 0x52);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3635, 0x03);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3636, 0x25);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3637, 0x8a);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3638, 0x0f);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3639, 0x08);
	sc035hgs_1L_slave_write_register(ViPipe, 0x363a, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x363b, 0x48);
	sc035hgs_1L_slave_write_register(ViPipe, 0x363c, 0x86);
	sc035hgs_1L_slave_write_register(ViPipe, 0x363d, 0x10);
	sc035hgs_1L_slave_write_register(ViPipe, 0x363e, 0xf8);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3640, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3641, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36ea, 0x36);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36eb, 0x0e);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36ec, 0x0e);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36ed, 0x10);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36fa, 0x36);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36fb, 0x10);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36fc, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36fd, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3908, 0x91);
	sc035hgs_1L_slave_write_register(ViPipe, 0x391b, 0x81);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3d08, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3e01, 0x18);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3e02, 0xf0);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3e03, 0x2b);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3e06, 0x0c);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3f04, 0x03);
	sc035hgs_1L_slave_write_register(ViPipe, 0x3f05, 0x80);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4500, 0x59);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4501, 0xc4);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4603, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4800, 0x64);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4809, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4810, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4811, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4837, 0x1c);
	sc035hgs_1L_slave_write_register(ViPipe, 0x5011, 0x00);
	sc035hgs_1L_slave_write_register(ViPipe, 0x5988, 0x02);
	sc035hgs_1L_slave_write_register(ViPipe, 0x598e, 0x04);
	sc035hgs_1L_slave_write_register(ViPipe, 0x598f, 0x4e);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36e9, 0x44);
	sc035hgs_1L_slave_write_register(ViPipe, 0x36f9, 0x44);
	sc035hgs_1L_slave_write_register(ViPipe, 0x0100, 0x01);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4418, 0x0a);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4419, 0x80);
 
	sc035hgs_1L_slave_default_reg_init(ViPipe);

	sc035hgs_1L_slave_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(7);
	sc035hgs_1L_slave_write_register(ViPipe, 0x363d, 0x10);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4418, 0x0a);
	sc035hgs_1L_slave_write_register(ViPipe, 0x4419, 0x80);

	delay_ms(100);

	printf("ViPipe:%d,===SC035HGS_1L_SLAVE 480P 120fps 12bit LINE Init OK!===\n", ViPipe);
}
