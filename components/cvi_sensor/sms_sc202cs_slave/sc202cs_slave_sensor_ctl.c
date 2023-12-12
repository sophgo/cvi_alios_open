#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#include "sc202cs_slave_cmos_ex.h"

#define SC202CS_SLAVE_CHIP_ID_ADDR_H	0x3107
#define SC202CS_SLAVE_CHIP_ID_ADDR_L	0x3108
#define SC202CS_SLAVE_CHIP_ID		0xeb52

static void sc202cs_slave_linear_1200p30_init(VI_PIPE ViPipe);

CVI_U8 sc202cs_slave_i2c_addr = 0x10;
const CVI_U32 sc202cs_slave_addr_byte = 2;
const CVI_U32 sc202cs_slave_data_byte = 1;

int sc202cs_slave_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc202cs_slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc202cs_slave_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc202cs_slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc202cs_slave_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc202cs_slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc202cs_slave_i2c_addr, (CVI_U32)addr, sc202cs_slave_addr_byte,
		sc202cs_slave_data_byte);
}

int sc202cs_slave_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc202cs_slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sc202cs_slave_i2c_addr, (CVI_U32)addr, sc202cs_slave_addr_byte,
		(CVI_U32)data, sc202cs_slave_data_byte);
}

static void delay_ms(int ms)
{
	udelay(ms * 1000);
}

void sc202cs_slave_standby(VI_PIPE ViPipe)
{
	sc202cs_slave_write_register(ViPipe, 0x0100, 0x00);

	printf("%s\n", __func__);
}

void sc202cs_slave_restart(VI_PIPE ViPipe)
{
	sc202cs_slave_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc202cs_slave_write_register(ViPipe, 0x0100, 0x01);

	printf("%s\n", __func__);
}

void sc202cs_slave_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSc202cs_slave[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc202cs_slave_write_register(ViPipe,
				g_pastSc202cs_slave[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSc202cs_slave[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void sc202cs_slave_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc202cs_slave_write_register(ViPipe, 0x3221, val);
}

int sc202cs_slave_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	if (sc202cs_slave_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = sc202cs_slave_read_register(ViPipe, SC202CS_SLAVE_CHIP_ID_ADDR_H);
	nVal2 = sc202cs_slave_read_register(ViPipe, SC202CS_SLAVE_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != SC202CS_SLAVE_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		CVI_TRACE_SNS(CVI_DBG_ERR, "nVal:%#x, nVal2:%#x\n", nVal, nVal2);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sc202cs_slave_init(VI_PIPE ViPipe)
{
	// sc202cs_slave_i2c_init(ViPipe);

	sc202cs_slave_linear_1200p30_init(ViPipe);

	g_pastSc202cs_slave[ViPipe]->bInit = CVI_TRUE;
}

void sc202cs_slave_exit(VI_PIPE ViPipe)
{
	sc202cs_slave_i2c_exit(ViPipe);
}

static void sc202cs_slave_linear_1200p30_init(VI_PIPE ViPipe)
{
	sc202cs_slave_write_register(ViPipe, 0x0103,0x01);
	sc202cs_slave_write_register(ViPipe, 0x0100,0x00);
	sc202cs_slave_write_register(ViPipe, 0x36e9,0x80);
	sc202cs_slave_write_register(ViPipe, 0x36e9,0x24);
	sc202cs_slave_write_register(ViPipe, 0x301f,0x01);
	sc202cs_slave_write_register(ViPipe, 0x3301,0xff);
	sc202cs_slave_write_register(ViPipe, 0x3304,0x68);
	sc202cs_slave_write_register(ViPipe, 0x3306,0x40);
	sc202cs_slave_write_register(ViPipe, 0x3308,0x08);
	sc202cs_slave_write_register(ViPipe, 0x3309,0xa8);
	sc202cs_slave_write_register(ViPipe, 0x330b,0xb0);
	sc202cs_slave_write_register(ViPipe, 0x330c,0x18);
	sc202cs_slave_write_register(ViPipe, 0x330d,0xff);
	sc202cs_slave_write_register(ViPipe, 0x330e,0x20);
	sc202cs_slave_write_register(ViPipe, 0x331e,0x59);
	sc202cs_slave_write_register(ViPipe, 0x331f,0x99);
	sc202cs_slave_write_register(ViPipe, 0x3333,0x10);
	sc202cs_slave_write_register(ViPipe, 0x335e,0x06);
	sc202cs_slave_write_register(ViPipe, 0x335f,0x08);
	sc202cs_slave_write_register(ViPipe, 0x3364,0x1f);
	sc202cs_slave_write_register(ViPipe, 0x337c,0x02);
	sc202cs_slave_write_register(ViPipe, 0x337d,0x0a);
	sc202cs_slave_write_register(ViPipe, 0x338f,0xa0);
	sc202cs_slave_write_register(ViPipe, 0x3390,0x01);
	sc202cs_slave_write_register(ViPipe, 0x3391,0x03);
	sc202cs_slave_write_register(ViPipe, 0x3392,0x1f);
	sc202cs_slave_write_register(ViPipe, 0x3393,0xff);
	sc202cs_slave_write_register(ViPipe, 0x3394,0xff);
	sc202cs_slave_write_register(ViPipe, 0x3395,0xff);
	sc202cs_slave_write_register(ViPipe, 0x33a2,0x04);
	sc202cs_slave_write_register(ViPipe, 0x33ad,0x0c);
	sc202cs_slave_write_register(ViPipe, 0x33b1,0x20);
	sc202cs_slave_write_register(ViPipe, 0x33b3,0x38);
	sc202cs_slave_write_register(ViPipe, 0x33f9,0x40);
	sc202cs_slave_write_register(ViPipe, 0x33fb,0x48);
	sc202cs_slave_write_register(ViPipe, 0x33fc,0x0f);
	sc202cs_slave_write_register(ViPipe, 0x33fd,0x1f);
	sc202cs_slave_write_register(ViPipe, 0x349f,0x03);
	sc202cs_slave_write_register(ViPipe, 0x34a6,0x03);
	sc202cs_slave_write_register(ViPipe, 0x34a7,0x1f);
	sc202cs_slave_write_register(ViPipe, 0x34a8,0x38);
	sc202cs_slave_write_register(ViPipe, 0x34a9,0x30);
	sc202cs_slave_write_register(ViPipe, 0x34ab,0xb0);
	sc202cs_slave_write_register(ViPipe, 0x34ad,0xb0);
	sc202cs_slave_write_register(ViPipe, 0x34f8,0x1f);
	sc202cs_slave_write_register(ViPipe, 0x34f9,0x20);
	sc202cs_slave_write_register(ViPipe, 0x3630,0xa0);
	sc202cs_slave_write_register(ViPipe, 0x3631,0x92);
	sc202cs_slave_write_register(ViPipe, 0x3632,0x64);
	sc202cs_slave_write_register(ViPipe, 0x3633,0x43);
	sc202cs_slave_write_register(ViPipe, 0x3637,0x49);
	sc202cs_slave_write_register(ViPipe, 0x363a,0x85);
	sc202cs_slave_write_register(ViPipe, 0x363c,0x0f);
	sc202cs_slave_write_register(ViPipe, 0x3650,0x31);
	sc202cs_slave_write_register(ViPipe, 0x3670,0x0d);
	sc202cs_slave_write_register(ViPipe, 0x3674,0xc0);
	sc202cs_slave_write_register(ViPipe, 0x3675,0xa0);
	sc202cs_slave_write_register(ViPipe, 0x3676,0xa0);
	sc202cs_slave_write_register(ViPipe, 0x3677,0x92);
	sc202cs_slave_write_register(ViPipe, 0x3678,0x96);
	sc202cs_slave_write_register(ViPipe, 0x3679,0x9a);
	sc202cs_slave_write_register(ViPipe, 0x367c,0x03);
	sc202cs_slave_write_register(ViPipe, 0x367d,0x0f);
	sc202cs_slave_write_register(ViPipe, 0x367e,0x01);
	sc202cs_slave_write_register(ViPipe, 0x367f,0x0f);
	sc202cs_slave_write_register(ViPipe, 0x3698,0x83);
	sc202cs_slave_write_register(ViPipe, 0x3699,0x86);
	sc202cs_slave_write_register(ViPipe, 0x369a,0x8c);
	sc202cs_slave_write_register(ViPipe, 0x369b,0x94);
	sc202cs_slave_write_register(ViPipe, 0x36a2,0x01);
	sc202cs_slave_write_register(ViPipe, 0x36a3,0x03);
	sc202cs_slave_write_register(ViPipe, 0x36a4,0x07);
	sc202cs_slave_write_register(ViPipe, 0x36ae,0x0f);
	sc202cs_slave_write_register(ViPipe, 0x36af,0x1f);
	sc202cs_slave_write_register(ViPipe, 0x36bd,0x22);
	sc202cs_slave_write_register(ViPipe, 0x36be,0x22);
	sc202cs_slave_write_register(ViPipe, 0x36bf,0x22);
	sc202cs_slave_write_register(ViPipe, 0x36d0,0x01);
	sc202cs_slave_write_register(ViPipe, 0x370f,0x02);
	sc202cs_slave_write_register(ViPipe, 0x3721,0x6c);
	sc202cs_slave_write_register(ViPipe, 0x3722,0x8d);
	sc202cs_slave_write_register(ViPipe, 0x3725,0xc5);
	sc202cs_slave_write_register(ViPipe, 0x3727,0x14);
	sc202cs_slave_write_register(ViPipe, 0x3728,0x04);
	sc202cs_slave_write_register(ViPipe, 0x37b7,0x04);
	sc202cs_slave_write_register(ViPipe, 0x37b8,0x04);
	sc202cs_slave_write_register(ViPipe, 0x37b9,0x06);
	sc202cs_slave_write_register(ViPipe, 0x37bd,0x07);
	sc202cs_slave_write_register(ViPipe, 0x37be,0x0f);
	sc202cs_slave_write_register(ViPipe, 0x3901,0x02);
	sc202cs_slave_write_register(ViPipe, 0x3903,0x40);
	sc202cs_slave_write_register(ViPipe, 0x3905,0x8d);
	sc202cs_slave_write_register(ViPipe, 0x3907,0x00);
	sc202cs_slave_write_register(ViPipe, 0x3908,0x41);
	sc202cs_slave_write_register(ViPipe, 0x391f,0x41);
	sc202cs_slave_write_register(ViPipe, 0x3933,0x80);
	sc202cs_slave_write_register(ViPipe, 0x3934,0x02);
	sc202cs_slave_write_register(ViPipe, 0x3937,0x6f);
	sc202cs_slave_write_register(ViPipe, 0x393a,0x01);
	sc202cs_slave_write_register(ViPipe, 0x393d,0x01);
	sc202cs_slave_write_register(ViPipe, 0x393e,0xc0);
	sc202cs_slave_write_register(ViPipe, 0x39dd,0x41);
	sc202cs_slave_write_register(ViPipe, 0x3e00,0x00);
	sc202cs_slave_write_register(ViPipe, 0x3e01,0x4d);
	sc202cs_slave_write_register(ViPipe, 0x3e02,0xc0);
	sc202cs_slave_write_register(ViPipe, 0x3e09,0x00);
	sc202cs_slave_write_register(ViPipe, 0x4509,0x28);
	sc202cs_slave_write_register(ViPipe, 0x450d,0x61);
	sc202cs_slave_write_register(ViPipe, 0x0100,0x01);

	sc202cs_slave_default_reg_init(ViPipe);

	printf("ViPipe:%d,===SC202CS_SLAVE 1200P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}