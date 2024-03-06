#include <unistd.h>

#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sc230ai_cmos_ex.h"
#include "sensor_i2c.h"

#define SC230AI_CHIP_ID_ADDR_H 0x3107
#define SC230AI_CHIP_ID_ADDR_L 0x3108
#define SC230AI_CHIP_ID 0xcb34

CVI_U8 sc230ai_i2c_addr = 0x30;
const CVI_U32 sc230ai_addr_byte = 2;
const CVI_U32 sc230ai_data_byte = 1;

static void sc230ai_linear_1080p30_init(VI_PIPE ViPipe);

CVI_U8 sc230ai_i2c_addr_list[] = {SC230AI_I2C_ADDR_1, SC230AI_I2C_ADDR_2};
CVI_S8 sc230ai_i2c_dev_list[] = {1, 1};
CVI_U8 sc230ai_cur_idx = 0;

int sc230ai_i2c_init(VI_PIPE ViPipe) {
	CVI_U8 i2c_id = (CVI_U8)g_aunSc230ai_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc230ai_i2c_exit(VI_PIPE ViPipe) {
	CVI_U8 i2c_id = (CVI_U8)g_aunSc230ai_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc230ai_read_register(VI_PIPE ViPipe, int addr) {
	CVI_U8 i2c_id = (CVI_U8)g_aunSc230ai_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc230ai_i2c_addr, (CVI_U32)addr, sc230ai_addr_byte,
						   sc230ai_data_byte);
}

int sc230ai_write_register(VI_PIPE ViPipe, int addr, int data) {
	CVI_U8 i2c_id = (CVI_U8)g_aunSc230ai_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sc230ai_i2c_addr, (CVI_U32)addr, sc230ai_addr_byte,
							(CVI_U32)data, sc230ai_data_byte);
}

void sc230ai_standby(VI_PIPE ViPipe) {
	sc230ai_write_register(ViPipe, 0x3019, 0xff);
	sc230ai_write_register(ViPipe, 0x0100, 0x00);

	printf("%s i2c_addr:%x, i2c_dev:%d\n", __func__, sc230ai_i2c_addr,
		   g_aunSc230ai_BusInfo[ViPipe].s8I2cDev);
}

void sc230ai_restart(VI_PIPE ViPipe) {
	sc230ai_write_register(ViPipe, 0x3019, 0xfe);
	sc230ai_write_register(ViPipe, 0x0100, 0x01);

	printf("%s i2c_addr:%x, i2c_dev:%d\n", __func__, sc230ai_i2c_addr,
		   g_aunSc230ai_BusInfo[ViPipe].s8I2cDev);
}

void sc230ai_default_reg_init(VI_PIPE ViPipe) {
	CVI_U32 i;

	for (i = 0; i < g_pastSc230ai[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc230ai_write_register(
			ViPipe, g_pastSc230ai[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
			g_pastSc230ai[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void sc230ai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip) {
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

	sc230ai_write_register(ViPipe, 0x3221, val);
}

int sc230ai_probe(VI_PIPE ViPipe) {
	int nVal;
	int nVal2;

	if (sc230ai_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc230ai_read_register(ViPipe, SC230AI_CHIP_ID_ADDR_H);
	nVal2 = sc230ai_read_register(ViPipe, SC230AI_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != SC230AI_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		CVI_TRACE_SNS(CVI_DBG_ERR, "nVal:%#x, nVal2:%#x\n", nVal, nVal2);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sc230ai_init(VI_PIPE ViPipe) {
	sc230ai_linear_1080p30_init(ViPipe);
	g_pastSc230ai[ViPipe]->bInit = CVI_TRUE;
}

void sc230ai_exit(VI_PIPE ViPipe) { sc230ai_i2c_exit(ViPipe); }

// cleaned_0x5b_SC230AI_MIPI_27Minput_1lane_742.5Mbps_10bit_1920x1080_30fps
static void sc230ai_linear_1080p30_init(VI_PIPE ViPipe) {
	sc230ai_write_register(ViPipe, 0x0103, 0x01);
	sc230ai_write_register(ViPipe, 0x0100, 0x00);
	sc230ai_write_register(ViPipe, 0x36e9, 0x80);
	sc230ai_write_register(ViPipe, 0x37f9, 0x80);
	sc230ai_write_register(ViPipe, 0x3018, 0x12);
	sc230ai_write_register(ViPipe, 0x3019, 0x0e);
	sc230ai_write_register(ViPipe, 0x301f, 0x5b);
	sc230ai_write_register(ViPipe, 0x3301, 0x07);
	sc230ai_write_register(ViPipe, 0x3304, 0x50);
	sc230ai_write_register(ViPipe, 0x3306, 0x70);
	sc230ai_write_register(ViPipe, 0x3308, 0x18);
	sc230ai_write_register(ViPipe, 0x3309, 0x68);
	sc230ai_write_register(ViPipe, 0x330a, 0x01);
	sc230ai_write_register(ViPipe, 0x330b, 0x20);
	sc230ai_write_register(ViPipe, 0x3314, 0x15);
	sc230ai_write_register(ViPipe, 0x331e, 0x41);
	sc230ai_write_register(ViPipe, 0x331f, 0x59);
	sc230ai_write_register(ViPipe, 0x3333, 0x10);
	sc230ai_write_register(ViPipe, 0x3334, 0x40);
	sc230ai_write_register(ViPipe, 0x335d, 0x60);
	sc230ai_write_register(ViPipe, 0x335e, 0x06);
	sc230ai_write_register(ViPipe, 0x335f, 0x08);
	sc230ai_write_register(ViPipe, 0x3364, 0x5e);
	sc230ai_write_register(ViPipe, 0x337c, 0x02);
	sc230ai_write_register(ViPipe, 0x337d, 0x0a);
	sc230ai_write_register(ViPipe, 0x3390, 0x01);
	sc230ai_write_register(ViPipe, 0x3391, 0x0b);
	sc230ai_write_register(ViPipe, 0x3392, 0x0f);
	sc230ai_write_register(ViPipe, 0x3393, 0x09);
	sc230ai_write_register(ViPipe, 0x3394, 0x0d);
	sc230ai_write_register(ViPipe, 0x3395, 0x60);
	sc230ai_write_register(ViPipe, 0x3396, 0x48);
	sc230ai_write_register(ViPipe, 0x3397, 0x49);
	sc230ai_write_register(ViPipe, 0x3398, 0x4b);
	sc230ai_write_register(ViPipe, 0x3399, 0x06);
	sc230ai_write_register(ViPipe, 0x339a, 0x0a);
	sc230ai_write_register(ViPipe, 0x339b, 0x0d);
	sc230ai_write_register(ViPipe, 0x339c, 0x60);
	sc230ai_write_register(ViPipe, 0x33a2, 0x04);
	sc230ai_write_register(ViPipe, 0x33ad, 0x2c);
	sc230ai_write_register(ViPipe, 0x33af, 0x40);
	sc230ai_write_register(ViPipe, 0x33b1, 0x80);
	sc230ai_write_register(ViPipe, 0x33b3, 0x40);
	sc230ai_write_register(ViPipe, 0x33b9, 0x0a);
	sc230ai_write_register(ViPipe, 0x33f9, 0x78);
	sc230ai_write_register(ViPipe, 0x33fb, 0xa0);
	sc230ai_write_register(ViPipe, 0x33fc, 0x4f);
	sc230ai_write_register(ViPipe, 0x33fd, 0x5f);
	sc230ai_write_register(ViPipe, 0x349f, 0x03);
	sc230ai_write_register(ViPipe, 0x34a6, 0x4b);
	sc230ai_write_register(ViPipe, 0x34a7, 0x5f);
	sc230ai_write_register(ViPipe, 0x34a8, 0x30);
	sc230ai_write_register(ViPipe, 0x34a9, 0x20);
	sc230ai_write_register(ViPipe, 0x34aa, 0x01);
	sc230ai_write_register(ViPipe, 0x34ab, 0x28);
	sc230ai_write_register(ViPipe, 0x34ac, 0x01);
	sc230ai_write_register(ViPipe, 0x34ad, 0x58);
	sc230ai_write_register(ViPipe, 0x34f8, 0x7f);
	sc230ai_write_register(ViPipe, 0x34f9, 0x10);
	sc230ai_write_register(ViPipe, 0x3630, 0xc0);
	sc230ai_write_register(ViPipe, 0x3632, 0x54);
	sc230ai_write_register(ViPipe, 0x3633, 0x44);
	sc230ai_write_register(ViPipe, 0x363b, 0x20);
	sc230ai_write_register(ViPipe, 0x363c, 0x08);
	sc230ai_write_register(ViPipe, 0x3670, 0x09);
	sc230ai_write_register(ViPipe, 0x3674, 0xb0);
	sc230ai_write_register(ViPipe, 0x3675, 0x80);
	sc230ai_write_register(ViPipe, 0x3676, 0x88);
	sc230ai_write_register(ViPipe, 0x367c, 0x40);
	sc230ai_write_register(ViPipe, 0x367d, 0x49);
	sc230ai_write_register(ViPipe, 0x3690, 0x33);
	sc230ai_write_register(ViPipe, 0x3691, 0x33);
	sc230ai_write_register(ViPipe, 0x3692, 0x43);
	sc230ai_write_register(ViPipe, 0x369c, 0x49);
	sc230ai_write_register(ViPipe, 0x369d, 0x4f);
	sc230ai_write_register(ViPipe, 0x36ae, 0x4b);
	sc230ai_write_register(ViPipe, 0x36af, 0x4f);
	sc230ai_write_register(ViPipe, 0x36b0, 0x87);
	sc230ai_write_register(ViPipe, 0x36b1, 0x9b);
	sc230ai_write_register(ViPipe, 0x36b2, 0xb7);
	sc230ai_write_register(ViPipe, 0x36d0, 0x01);
	sc230ai_write_register(ViPipe, 0x36ec, 0x0c);
	sc230ai_write_register(ViPipe, 0x3722, 0x97);
	sc230ai_write_register(ViPipe, 0x3724, 0x22);
	sc230ai_write_register(ViPipe, 0x3728, 0x90);
	sc230ai_write_register(ViPipe, 0x3901, 0x02);
	sc230ai_write_register(ViPipe, 0x3902, 0xc5);
	sc230ai_write_register(ViPipe, 0x3904, 0x04);
	sc230ai_write_register(ViPipe, 0x3907, 0x00);
	sc230ai_write_register(ViPipe, 0x3908, 0x41);
	sc230ai_write_register(ViPipe, 0x3909, 0x00);
	sc230ai_write_register(ViPipe, 0x390a, 0x00);
	sc230ai_write_register(ViPipe, 0x3933, 0x84);
	sc230ai_write_register(ViPipe, 0x3934, 0x0a);
	sc230ai_write_register(ViPipe, 0x3940, 0x64);
	sc230ai_write_register(ViPipe, 0x3941, 0x00);
	sc230ai_write_register(ViPipe, 0x3942, 0x04);
	sc230ai_write_register(ViPipe, 0x3943, 0x0b);
	sc230ai_write_register(ViPipe, 0x3e00, 0x00);
	sc230ai_write_register(ViPipe, 0x3e01, 0x8c);
	sc230ai_write_register(ViPipe, 0x3e02, 0x10);
	sc230ai_write_register(ViPipe, 0x440e, 0x02);
	sc230ai_write_register(ViPipe, 0x450d, 0x11);
	sc230ai_write_register(ViPipe, 0x4819, 0x09);
	sc230ai_write_register(ViPipe, 0x481b, 0x05);
	sc230ai_write_register(ViPipe, 0x481d, 0x14);
	sc230ai_write_register(ViPipe, 0x481f, 0x04);
	sc230ai_write_register(ViPipe, 0x4821, 0x0a);
	sc230ai_write_register(ViPipe, 0x4823, 0x05);
	sc230ai_write_register(ViPipe, 0x4825, 0x04);
	sc230ai_write_register(ViPipe, 0x4827, 0x05);
	sc230ai_write_register(ViPipe, 0x4829, 0x08);
	sc230ai_write_register(ViPipe, 0x5010, 0x01);
	sc230ai_write_register(ViPipe, 0x5787, 0x08);
	sc230ai_write_register(ViPipe, 0x5788, 0x03);
	sc230ai_write_register(ViPipe, 0x5789, 0x00);
	sc230ai_write_register(ViPipe, 0x578a, 0x10);
	sc230ai_write_register(ViPipe, 0x578b, 0x08);
	sc230ai_write_register(ViPipe, 0x578c, 0x00);
	sc230ai_write_register(ViPipe, 0x5790, 0x08);
	sc230ai_write_register(ViPipe, 0x5791, 0x04);
	sc230ai_write_register(ViPipe, 0x5792, 0x00);
	sc230ai_write_register(ViPipe, 0x5793, 0x10);
	sc230ai_write_register(ViPipe, 0x5794, 0x08);
	sc230ai_write_register(ViPipe, 0x5795, 0x00);
	sc230ai_write_register(ViPipe, 0x5799, 0x06);
	sc230ai_write_register(ViPipe, 0x57ad, 0x00);
	sc230ai_write_register(ViPipe, 0x5ae0, 0xfe);
	sc230ai_write_register(ViPipe, 0x5ae1, 0x40);
	sc230ai_write_register(ViPipe, 0x5ae2, 0x3f);
	sc230ai_write_register(ViPipe, 0x5ae3, 0x38);
	sc230ai_write_register(ViPipe, 0x5ae4, 0x28);
	sc230ai_write_register(ViPipe, 0x5ae5, 0x3f);
	sc230ai_write_register(ViPipe, 0x5ae6, 0x38);
	sc230ai_write_register(ViPipe, 0x5ae7, 0x28);
	sc230ai_write_register(ViPipe, 0x5ae8, 0x3f);
	sc230ai_write_register(ViPipe, 0x5ae9, 0x3c);
	sc230ai_write_register(ViPipe, 0x5aea, 0x2c);
	sc230ai_write_register(ViPipe, 0x5aeb, 0x3f);
	sc230ai_write_register(ViPipe, 0x5aec, 0x3c);
	sc230ai_write_register(ViPipe, 0x5aed, 0x2c);
	sc230ai_write_register(ViPipe, 0x5af4, 0x3f);
	sc230ai_write_register(ViPipe, 0x5af5, 0x38);
	sc230ai_write_register(ViPipe, 0x5af6, 0x28);
	sc230ai_write_register(ViPipe, 0x5af7, 0x3f);
	sc230ai_write_register(ViPipe, 0x5af8, 0x38);
	sc230ai_write_register(ViPipe, 0x5af9, 0x28);
	sc230ai_write_register(ViPipe, 0x5afa, 0x3f);
	sc230ai_write_register(ViPipe, 0x5afb, 0x3c);
	sc230ai_write_register(ViPipe, 0x5afc, 0x2c);
	sc230ai_write_register(ViPipe, 0x5afd, 0x3f);
	sc230ai_write_register(ViPipe, 0x5afe, 0x3c);
	sc230ai_write_register(ViPipe, 0x5aff, 0x2c);
	sc230ai_write_register(ViPipe, 0x36e9, 0x20);
	sc230ai_write_register(ViPipe, 0x37f9, 0x27);
	sc230ai_write_register(ViPipe, 0x0100, 0x01);

	sc230ai_default_reg_init(ViPipe);

	printf("ViPipe:%d,===SC230AI 1080P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}