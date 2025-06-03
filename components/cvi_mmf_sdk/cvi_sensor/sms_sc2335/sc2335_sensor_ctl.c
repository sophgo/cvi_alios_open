#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "cvi_sns_ctrl.h"
#include "sc2335_cmos_ex.h"
#include "sensor_i2c.h"

#define SC2335_CHIP_ID_HI_ADDR		0x3107
#define SC2335_CHIP_ID_LO_ADDR		0x3108
#define SC2335_CHIP_ID			0xcb14

static void sc2335_linear_1080p30_init(VI_PIPE ViPipe);

const CVI_U8 sc2335_i2c_addr = 0x30;        /* I2C Address of SC2335 */
const CVI_U32 sc2335_addr_byte = 2;
const CVI_U32 sc2335_data_byte = 1;

int sc2335_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC2335_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC2335_AddrInfo[ViPipe].s8I2cAddr);
}

int sc2335_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC2335_BusInfo[ViPipe].s8I2cDev);
}

int sc2335_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC2335_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC2335_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc2335_addr_byte, sc2335_data_byte);
}

int sc2335_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC2335_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC2335_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc2335_addr_byte, (CVI_U32)data, sc2335_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc2335_prog(VI_PIPE ViPipe, int *rom)
{
	int i = 0;

	while (1) {
		int lookup = rom[i++];
		int addr = (lookup >> 16) & 0xFFFF;
		int data = lookup & 0xFFFF;

		if (addr == 0xFFFE)
			delay_ms(data);
		else if (addr != 0xFFFF)
			sc2335_write_register(ViPipe, addr, data);
	}
}

void sc2335_standby(VI_PIPE ViPipe)
{
	sc2335_write_register(ViPipe, 0x0100, 0x00);
}

void sc2335_restart(VI_PIPE ViPipe)
{
	sc2335_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc2335_write_register(ViPipe, 0x0100, 0x01);
}

void sc2335_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC2335[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastSC2335[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			sc2335_write_register(ViPipe,
				g_pastSC2335[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC2335[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void sc2335_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc2335_write_register(ViPipe, 0x3221, val);
}

int sc2335_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	usleep(4*1000);
	if (sc2335_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc2335_read_register(ViPipe, SC2335_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc2335_read_register(ViPipe, SC2335_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC2335_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

/* 1080P30 and 1080P25 */
static void sc2335_linear_1080p30_init(VI_PIPE ViPipe)
{
	sc2335_write_register(ViPipe, 0x0103, 0x01);
	sc2335_write_register(ViPipe, 0x0100, 0x00);
	sc2335_write_register(ViPipe, 0x36e9, 0x80);
	sc2335_write_register(ViPipe, 0x36f9, 0x80);
	sc2335_write_register(ViPipe, 0x301f, 0x02);
	sc2335_write_register(ViPipe, 0x3207, 0x3f);
	sc2335_write_register(ViPipe, 0x3249, 0x0f);
	sc2335_write_register(ViPipe, 0x3253, 0x08);
	sc2335_write_register(ViPipe, 0x3271, 0x00);
	sc2335_write_register(ViPipe, 0x3273, 0x03);
	sc2335_write_register(ViPipe, 0x3301, 0x06);
	sc2335_write_register(ViPipe, 0x3302, 0x09);
	sc2335_write_register(ViPipe, 0x3304, 0x28);
	sc2335_write_register(ViPipe, 0x3306, 0x30);
	sc2335_write_register(ViPipe, 0x330b, 0x94);
	sc2335_write_register(ViPipe, 0x330c, 0x08);
	sc2335_write_register(ViPipe, 0x330d, 0x18);
	sc2335_write_register(ViPipe, 0x330e, 0x14);
	sc2335_write_register(ViPipe, 0x330f, 0x05);
	sc2335_write_register(ViPipe, 0x3310, 0x06);
	sc2335_write_register(ViPipe, 0x3314, 0x96);
	sc2335_write_register(ViPipe, 0x3316, 0x00);
	sc2335_write_register(ViPipe, 0x331e, 0x21);
	sc2335_write_register(ViPipe, 0x332b, 0x08);
	sc2335_write_register(ViPipe, 0x3333, 0x10);
	sc2335_write_register(ViPipe, 0x3338, 0x80);
	sc2335_write_register(ViPipe, 0x333a, 0x04);
	sc2335_write_register(ViPipe, 0x334c, 0x04);
	sc2335_write_register(ViPipe, 0x335f, 0x04);
	sc2335_write_register(ViPipe, 0x3364, 0x17);
	sc2335_write_register(ViPipe, 0x3366, 0x62);
	sc2335_write_register(ViPipe, 0x337c, 0x05);
	sc2335_write_register(ViPipe, 0x337d, 0x09);
	sc2335_write_register(ViPipe, 0x337e, 0x00);
	sc2335_write_register(ViPipe, 0x3390, 0x08);
	sc2335_write_register(ViPipe, 0x3391, 0x18);
	sc2335_write_register(ViPipe, 0x3392, 0x38);
	sc2335_write_register(ViPipe, 0x3393, 0x09);
	sc2335_write_register(ViPipe, 0x3394, 0x20);
	sc2335_write_register(ViPipe, 0x3395, 0x20);
	sc2335_write_register(ViPipe, 0x33a2, 0x07);
	sc2335_write_register(ViPipe, 0x33ac, 0x04);
	sc2335_write_register(ViPipe, 0x33ae, 0x14);
	sc2335_write_register(ViPipe, 0x3614, 0x00);
	sc2335_write_register(ViPipe, 0x3622, 0x16);
	sc2335_write_register(ViPipe, 0x3630, 0x68);
	sc2335_write_register(ViPipe, 0x3631, 0x84);
	sc2335_write_register(ViPipe, 0x3637, 0x20);
	sc2335_write_register(ViPipe, 0x363a, 0x1f);
	sc2335_write_register(ViPipe, 0x363c, 0x0e);
	sc2335_write_register(ViPipe, 0x3670, 0x0e);
	sc2335_write_register(ViPipe, 0x3674, 0xa1);
	sc2335_write_register(ViPipe, 0x3675, 0x9c);
	sc2335_write_register(ViPipe, 0x3676, 0x9e);
	sc2335_write_register(ViPipe, 0x3677, 0x84);
	sc2335_write_register(ViPipe, 0x3678, 0x85);
	sc2335_write_register(ViPipe, 0x3679, 0x87);
	sc2335_write_register(ViPipe, 0x367c, 0x18);
	sc2335_write_register(ViPipe, 0x367d, 0x38);
	sc2335_write_register(ViPipe, 0x367e, 0x08);
	sc2335_write_register(ViPipe, 0x367f, 0x18);
	sc2335_write_register(ViPipe, 0x3690, 0x32);
	sc2335_write_register(ViPipe, 0x3691, 0x32);
	sc2335_write_register(ViPipe, 0x3692, 0x44);
	sc2335_write_register(ViPipe, 0x369c, 0x08);
	sc2335_write_register(ViPipe, 0x369d, 0x38);
	sc2335_write_register(ViPipe, 0x3908, 0x82);
	sc2335_write_register(ViPipe, 0x391f, 0x18);
	sc2335_write_register(ViPipe, 0x3e01, 0x8c);
	sc2335_write_register(ViPipe, 0x3e02, 0x00);
	sc2335_write_register(ViPipe, 0x3f00, 0x0d);
	sc2335_write_register(ViPipe, 0x3f04, 0x02);
	sc2335_write_register(ViPipe, 0x3f05, 0x0e);
	sc2335_write_register(ViPipe, 0x3f09, 0x48);
	sc2335_write_register(ViPipe, 0x4505, 0x0a);
	sc2335_write_register(ViPipe, 0x4509, 0x20);
	sc2335_write_register(ViPipe, 0x481d, 0x0a);
	sc2335_write_register(ViPipe, 0x4827, 0x03);
	sc2335_write_register(ViPipe, 0x5787, 0x10);
	sc2335_write_register(ViPipe, 0x5788, 0x06);
	sc2335_write_register(ViPipe, 0x578a, 0x10);
	sc2335_write_register(ViPipe, 0x578b, 0x06);
	sc2335_write_register(ViPipe, 0x5790, 0x10);
	sc2335_write_register(ViPipe, 0x5791, 0x10);
	sc2335_write_register(ViPipe, 0x5792, 0x00);
	sc2335_write_register(ViPipe, 0x5793, 0x10);
	sc2335_write_register(ViPipe, 0x5794, 0x10);
	sc2335_write_register(ViPipe, 0x5795, 0x00);
	sc2335_write_register(ViPipe, 0x5799, 0x00);
	sc2335_write_register(ViPipe, 0x57c7, 0x10);
	sc2335_write_register(ViPipe, 0x57c8, 0x06);
	sc2335_write_register(ViPipe, 0x57ca, 0x10);
	sc2335_write_register(ViPipe, 0x57cb, 0x06);
	sc2335_write_register(ViPipe, 0x57d1, 0x10);
	sc2335_write_register(ViPipe, 0x57d4, 0x10);
	sc2335_write_register(ViPipe, 0x57d9, 0x00);
	sc2335_write_register(ViPipe, 0x36e9, 0x20);
	sc2335_write_register(ViPipe, 0x36f9, 0x27);
	sc2335_write_register(ViPipe, 0x0100, 0x01);

	sc2335_default_reg_init(ViPipe);

	sc2335_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC2335 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

void sc2335_init(VI_PIPE ViPipe)
{
	sc2335_i2c_init(ViPipe);

	//linear mode only
	sc2335_linear_1080p30_init(ViPipe);

	g_pastSC2335[ViPipe]->bInit = CVI_TRUE;
}

