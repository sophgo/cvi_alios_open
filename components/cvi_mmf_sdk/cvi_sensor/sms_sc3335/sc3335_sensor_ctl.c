#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc3335_cmos_ex.h"
#include "sensor_i2c.h"

#define SC3335_CHIP_ID_HI_ADDR		0x3107
#define SC3335_CHIP_ID_LO_ADDR		0x3108
#define SC3335_CHIP_ID			0xcc1a

static void sc3335_linear_1296P30_init(VI_PIPE ViPipe);

CVI_U8 sc3335_i2c_addr = 0x30;
const CVI_U32 sc3335_addr_byte = 2;
const CVI_U32 sc3335_data_byte = 1;

int sc3335_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC3335_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC3335_AddrInfo[ViPipe].s8I2cAddr);
}

int sc3335_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC3335_BusInfo[ViPipe].s8I2cDev);
}

int sc3335_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC3335_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC3335_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc3335_addr_byte, sc3335_data_byte);
}

int sc3335_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC3335_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC3335_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc3335_addr_byte, (CVI_U32)data, sc3335_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc3335_standby(VI_PIPE ViPipe)
{
	sc3335_write_register(ViPipe, 0x0100, 0x00);
}

void sc3335_restart(VI_PIPE ViPipe)
{
	sc3335_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc3335_write_register(ViPipe, 0x0100, 0x01);
}

void sc3335_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC3335[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastSC3335[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			sc3335_write_register(ViPipe,
				g_pastSC3335[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC3335[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void sc3335_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc3335_write_register(ViPipe, 0x3221, val);
}


int sc3335_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc3335_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc3335_read_register(ViPipe, SC3335_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc3335_read_register(ViPipe, SC3335_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC3335_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sc3335_init(VI_PIPE ViPipe)
{
	sc3335_i2c_init(ViPipe);

	//linear mode only
	sc3335_linear_1296P30_init(ViPipe);

	g_pastSC3335[ViPipe]->bInit = CVI_TRUE;
}

/* 1296P30 and 1296P25 */
static void sc3335_linear_1296P30_init(VI_PIPE ViPipe)
{
	sc3335_write_register(ViPipe, 0x0103, 0x01);
	delay_ms(33);
	sc3335_write_register(ViPipe, 0x0100, 0x00);
	sc3335_write_register(ViPipe, 0x36e9, 0x80);
	sc3335_write_register(ViPipe, 0x36f9, 0x80);
	sc3335_write_register(ViPipe, 0x301f, 0x01);
	sc3335_write_register(ViPipe, 0x3253, 0x04);
	sc3335_write_register(ViPipe, 0x3301, 0x04);
	sc3335_write_register(ViPipe, 0x3302, 0x10);
	sc3335_write_register(ViPipe, 0x3304, 0x40);
	sc3335_write_register(ViPipe, 0x3306, 0x40);
	sc3335_write_register(ViPipe, 0x3309, 0x50);
	sc3335_write_register(ViPipe, 0x330b, 0xb6);
	sc3335_write_register(ViPipe, 0x330e, 0x29);
	sc3335_write_register(ViPipe, 0x3310, 0x06);
	sc3335_write_register(ViPipe, 0x3314, 0x96);
	sc3335_write_register(ViPipe, 0x331e, 0x39);
	sc3335_write_register(ViPipe, 0x331f, 0x49);
	sc3335_write_register(ViPipe, 0x3320, 0x09);
	sc3335_write_register(ViPipe, 0x3333, 0x10);
	sc3335_write_register(ViPipe, 0x334c, 0x01);
	sc3335_write_register(ViPipe, 0x3364, 0x17);
	sc3335_write_register(ViPipe, 0x3367, 0x01);
	sc3335_write_register(ViPipe, 0x3390, 0x04);
	sc3335_write_register(ViPipe, 0x3391, 0x08);
	sc3335_write_register(ViPipe, 0x3392, 0x38);
	sc3335_write_register(ViPipe, 0x3393, 0x05);
	sc3335_write_register(ViPipe, 0x3394, 0x09);
	sc3335_write_register(ViPipe, 0x3395, 0x16);
	sc3335_write_register(ViPipe, 0x33ac, 0x0c);
	sc3335_write_register(ViPipe, 0x33ae, 0x1c);
	sc3335_write_register(ViPipe, 0x3622, 0x16);
	sc3335_write_register(ViPipe, 0x3637, 0x22);
	sc3335_write_register(ViPipe, 0x363a, 0x1f);
	sc3335_write_register(ViPipe, 0x363c, 0x05);
	sc3335_write_register(ViPipe, 0x3670, 0x0e);
	sc3335_write_register(ViPipe, 0x3674, 0xb0);
	sc3335_write_register(ViPipe, 0x3675, 0x88);
	sc3335_write_register(ViPipe, 0x3676, 0x68);
	sc3335_write_register(ViPipe, 0x3677, 0x84);
	sc3335_write_register(ViPipe, 0x3678, 0x85);
	sc3335_write_register(ViPipe, 0x3679, 0x86);
	sc3335_write_register(ViPipe, 0x367c, 0x18);
	sc3335_write_register(ViPipe, 0x367d, 0x38);
	sc3335_write_register(ViPipe, 0x367e, 0x08);
	sc3335_write_register(ViPipe, 0x367f, 0x18);
	sc3335_write_register(ViPipe, 0x3690, 0x43);
	sc3335_write_register(ViPipe, 0x3691, 0x43);
	sc3335_write_register(ViPipe, 0x3692, 0x44);
	sc3335_write_register(ViPipe, 0x369c, 0x18);
	sc3335_write_register(ViPipe, 0x369d, 0x38);
	sc3335_write_register(ViPipe, 0x36ea, 0x3b);
	sc3335_write_register(ViPipe, 0x36eb, 0x0d);
	sc3335_write_register(ViPipe, 0x36ec, 0x1c);
	sc3335_write_register(ViPipe, 0x36ed, 0x24);
	sc3335_write_register(ViPipe, 0x36fa, 0x3b);
	sc3335_write_register(ViPipe, 0x36fb, 0x00);
	sc3335_write_register(ViPipe, 0x36fc, 0x10);
	sc3335_write_register(ViPipe, 0x36fd, 0x24);
	sc3335_write_register(ViPipe, 0x3908, 0x82);
	sc3335_write_register(ViPipe, 0x391f, 0x18);
	sc3335_write_register(ViPipe, 0x3e01, 0xa8);
	sc3335_write_register(ViPipe, 0x3e02, 0x20);
	sc3335_write_register(ViPipe, 0x3f09, 0x48);
	sc3335_write_register(ViPipe, 0x4505, 0x08);
	sc3335_write_register(ViPipe, 0x4509, 0x20);
	sc3335_write_register(ViPipe, 0x5799, 0x00);
	sc3335_write_register(ViPipe, 0x59e0, 0x60);
	sc3335_write_register(ViPipe, 0x59e1, 0x08);
	sc3335_write_register(ViPipe, 0x59e2, 0x3f);
	sc3335_write_register(ViPipe, 0x59e3, 0x18);
	sc3335_write_register(ViPipe, 0x59e4, 0x18);
	sc3335_write_register(ViPipe, 0x59e5, 0x3f);
	sc3335_write_register(ViPipe, 0x59e6, 0x06);
	sc3335_write_register(ViPipe, 0x59e7, 0x02);
	sc3335_write_register(ViPipe, 0x59e8, 0x38);
	sc3335_write_register(ViPipe, 0x59e9, 0x10);
	sc3335_write_register(ViPipe, 0x59ea, 0x0c);
	sc3335_write_register(ViPipe, 0x59eb, 0x10);
	sc3335_write_register(ViPipe, 0x59ec, 0x04);
	sc3335_write_register(ViPipe, 0x59ed, 0x02);
	sc3335_write_register(ViPipe, 0x36e9, 0x23);
	sc3335_write_register(ViPipe, 0x36f9, 0x23);

	sc3335_default_reg_init(ViPipe);

	sc3335_write_register(ViPipe, 0x0100, 0x01);

	delay_ms(100);

	printf("ViPipe:%d,===SC3335 1296P 30fps 10bit LINE Init OK!===\n", ViPipe);
}
