#include <unistd.h>

#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sc530ai_4L_cmos_ex.h"
#include "sensor_i2c.h"

static void sc530ai_4l_linear_1620p30_init(VI_PIPE ViPipe);
static void sc530ai_4l_wdr_1620p30_init(VI_PIPE ViPipe);

CVI_U32 sc530ai_4l_i2c_addr = 0x30; /* I2C Address of SC530AI_4L */
const CVI_U32 sc530ai_4l_addr_byte = 2;
const CVI_U32 sc530ai_4l_data_byte = 1;

int sc530ai_4l_i2c_init(VI_PIPE ViPipe) {
	CVI_U8 i2c_id = g_aunSC530AI_4L_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_init(i2c_id);
}

int sc530ai_4l_i2c_exit(VI_PIPE ViPipe) {
	CVI_U8 i2c_id = g_aunSC530AI_4L_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_exit(i2c_id);
}

int sc530ai_4l_read_register(VI_PIPE ViPipe, int addr) {
	CVI_U8 i2c_id = g_aunSC530AI_4L_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_read(i2c_id, sc530ai_4l_i2c_addr, (CVI_U32)addr, sc530ai_4l_addr_byte,
						   sc530ai_4l_data_byte);
}

int sc530ai_4l_write_register(VI_PIPE ViPipe, int addr, int data) {
	CVI_U8 i2c_id = g_aunSC530AI_4L_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_write(i2c_id, sc530ai_4l_i2c_addr, (CVI_U32)addr, sc530ai_4l_addr_byte,
							(CVI_U32)data, sc530ai_4l_data_byte);
}

static void delay_ms(int ms) { udelay(ms * 1000); }

void sc530ai_4l_standby(VI_PIPE ViPipe) { sc530ai_4l_write_register(ViPipe, 0x0100, 0x00); }

void sc530ai_4l_restart(VI_PIPE ViPipe) {
	sc530ai_4l_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc530ai_4l_write_register(ViPipe, 0x0100, 0x01);
}

void sc530ai_4l_default_reg_init(VI_PIPE ViPipe) {
	CVI_U32 i;

	for (i = 0; i < g_pastSC530AI_4L[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc530ai_4l_write_register(
			ViPipe, g_pastSC530AI_4L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
			g_pastSC530AI_4L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define SC530AI_4L_CHIP_ID_HI_ADDR 0x3107
#define SC530AI_4L_CHIP_ID_LO_ADDR 0x3108
#define SC530AI_4L_CHIP_ID 0x8e39

int sc530ai_4l_probe(VI_PIPE ViPipe) {
	int nVal;
	CVI_U16 chip_id;

	usleep(4 * 1000);
	if (sc530ai_4l_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc530ai_4l_read_register(ViPipe, SC530AI_4L_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc530ai_4l_read_register(ViPipe, SC530AI_4L_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC530AI_4L_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sc530ai_4l_init(VI_PIPE ViPipe) {
	WDR_MODE_E enWDRMode;
	CVI_U8 u8ImgMode;

	enWDRMode = g_pastSC530AI_4L[ViPipe]->enWDRMode;
	u8ImgMode = g_pastSC530AI_4L[ViPipe]->u8ImgMode;

	sc530ai_4l_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == SC530AI_4L_MODE_1620P30_WDR) {
			sc530ai_4l_wdr_1620p30_init(ViPipe);
		}
	} else {
		sc530ai_4l_linear_1620p30_init(ViPipe);
	}

	g_pastSC530AI_4L[ViPipe]->bInit = CVI_TRUE;
}

void sc530ai_4l_exit(VI_PIPE ViPipe) { sc530ai_4l_i2c_exit(ViPipe); }

/*SC530AI LINEAR 1620P30 */
// cleaned_0x01_FT_SC530AI_27Minput_396Mbps_4lane_10bit_2880x1620_30fps(1)
static void sc530ai_4l_linear_1620p30_init(VI_PIPE ViPipe) {
	usleep(10 * 1000);
	sc530ai_4l_write_register(ViPipe, 0x0103, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x0100, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x36e9, 0x80);
	sc530ai_4l_write_register(ViPipe, 0x37f9, 0x80);
	sc530ai_4l_write_register(ViPipe, 0x301f, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x3250, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x3251, 0x98);
	sc530ai_4l_write_register(ViPipe, 0x3253, 0x0c);
	sc530ai_4l_write_register(ViPipe, 0x325f, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x3301, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3304, 0x50);
	sc530ai_4l_write_register(ViPipe, 0x3306, 0x88);
	sc530ai_4l_write_register(ViPipe, 0x3308, 0x14);
	sc530ai_4l_write_register(ViPipe, 0x3309, 0x70);
	sc530ai_4l_write_register(ViPipe, 0x330a, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x330b, 0xf8);
	sc530ai_4l_write_register(ViPipe, 0x330d, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x331e, 0x41);
	sc530ai_4l_write_register(ViPipe, 0x331f, 0x61);
	sc530ai_4l_write_register(ViPipe, 0x3333, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x335d, 0x60);
	sc530ai_4l_write_register(ViPipe, 0x335e, 0x06);
	sc530ai_4l_write_register(ViPipe, 0x335f, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3364, 0x56);
	sc530ai_4l_write_register(ViPipe, 0x3366, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x337c, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x337d, 0x0a);
	sc530ai_4l_write_register(ViPipe, 0x3390, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x3391, 0x03);
	sc530ai_4l_write_register(ViPipe, 0x3392, 0x07);
	sc530ai_4l_write_register(ViPipe, 0x3393, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3394, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3395, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3396, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x3397, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x3398, 0x4b);
	sc530ai_4l_write_register(ViPipe, 0x3399, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x339a, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x339b, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x339c, 0x1d);
	sc530ai_4l_write_register(ViPipe, 0x33a2, 0x04);
	sc530ai_4l_write_register(ViPipe, 0x33ae, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x33af, 0x50);
	sc530ai_4l_write_register(ViPipe, 0x33b1, 0x80);
	sc530ai_4l_write_register(ViPipe, 0x33b2, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x33b3, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x349f, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x34a6, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x34a7, 0x4b);
	sc530ai_4l_write_register(ViPipe, 0x34a8, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x34a9, 0x18);
	sc530ai_4l_write_register(ViPipe, 0x34f8, 0x5f);
	sc530ai_4l_write_register(ViPipe, 0x34f9, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3632, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x3633, 0x32);
	sc530ai_4l_write_register(ViPipe, 0x3637, 0x29);
	sc530ai_4l_write_register(ViPipe, 0x3638, 0xc1);
	sc530ai_4l_write_register(ViPipe, 0x363b, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x363d, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x3670, 0x09);
	sc530ai_4l_write_register(ViPipe, 0x3674, 0x8b);
	sc530ai_4l_write_register(ViPipe, 0x3675, 0xc6);
	sc530ai_4l_write_register(ViPipe, 0x3676, 0x8b);
	sc530ai_4l_write_register(ViPipe, 0x367c, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x367d, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x3690, 0x32);
	sc530ai_4l_write_register(ViPipe, 0x3691, 0x43);
	sc530ai_4l_write_register(ViPipe, 0x3692, 0x33);
	sc530ai_4l_write_register(ViPipe, 0x3693, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x3694, 0x4b);
	sc530ai_4l_write_register(ViPipe, 0x3698, 0x85);
	sc530ai_4l_write_register(ViPipe, 0x3699, 0x8f);
	sc530ai_4l_write_register(ViPipe, 0x369a, 0xa0);
	sc530ai_4l_write_register(ViPipe, 0x369b, 0xc3);
	sc530ai_4l_write_register(ViPipe, 0x36a2, 0x49);
	sc530ai_4l_write_register(ViPipe, 0x36a3, 0x4b);
	sc530ai_4l_write_register(ViPipe, 0x36a4, 0x4f);
	sc530ai_4l_write_register(ViPipe, 0x36d0, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x36ec, 0x13);
	sc530ai_4l_write_register(ViPipe, 0x370f, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x3722, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x3728, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x37b0, 0x03);
	sc530ai_4l_write_register(ViPipe, 0x37b1, 0x03);
	sc530ai_4l_write_register(ViPipe, 0x37b2, 0x83);
	sc530ai_4l_write_register(ViPipe, 0x37b3, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x37b4, 0x49);
	sc530ai_4l_write_register(ViPipe, 0x37fb, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x37fc, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x3901, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x3902, 0xc5);
	sc530ai_4l_write_register(ViPipe, 0x3904, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3905, 0x8c);
	sc530ai_4l_write_register(ViPipe, 0x3909, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x391d, 0x04);
	sc530ai_4l_write_register(ViPipe, 0x391f, 0x44);
	sc530ai_4l_write_register(ViPipe, 0x3926, 0x21);
	sc530ai_4l_write_register(ViPipe, 0x3929, 0x18);
	sc530ai_4l_write_register(ViPipe, 0x3933, 0x82);
	sc530ai_4l_write_register(ViPipe, 0x3934, 0x0a);
	sc530ai_4l_write_register(ViPipe, 0x3937, 0x5f);
	sc530ai_4l_write_register(ViPipe, 0x3939, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x393a, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x39dc, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x3e01, 0xcd);
	sc530ai_4l_write_register(ViPipe, 0x3e02, 0xa0);
	sc530ai_4l_write_register(ViPipe, 0x440e, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x4509, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x4837, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x5010, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x5780, 0x66);
	sc530ai_4l_write_register(ViPipe, 0x578d, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x5799, 0x06);
	sc530ai_4l_write_register(ViPipe, 0x57ad, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x5ae0, 0xfe);
	sc530ai_4l_write_register(ViPipe, 0x5ae1, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x5ae2, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5ae3, 0x2a);
	sc530ai_4l_write_register(ViPipe, 0x5ae4, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x5ae5, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5ae6, 0x2a);
	sc530ai_4l_write_register(ViPipe, 0x5ae7, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x5ae8, 0x3c);
	sc530ai_4l_write_register(ViPipe, 0x5ae9, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5aea, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x5aeb, 0x3c);
	sc530ai_4l_write_register(ViPipe, 0x5aec, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5aed, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x5aee, 0xfe);
	sc530ai_4l_write_register(ViPipe, 0x5aef, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x5af4, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5af5, 0x2a);
	sc530ai_4l_write_register(ViPipe, 0x5af6, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x5af7, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5af8, 0x2a);
	sc530ai_4l_write_register(ViPipe, 0x5af9, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x5afa, 0x3c);
	sc530ai_4l_write_register(ViPipe, 0x5afb, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5afc, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x5afd, 0x3c);
	sc530ai_4l_write_register(ViPipe, 0x5afe, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5aff, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x36e9, 0x44);
	sc530ai_4l_write_register(ViPipe, 0x37f9, 0x44);
	sc530ai_4l_write_register(ViPipe, 0x0100, 0x01);

	sc530ai_4l_default_reg_init(ViPipe);

	sc530ai_4l_write_register(ViPipe, 0x0100, 0x01);

	usleep(100 * 1000);

	printf("ViPipe:%d,===SC530AI_4L 1620P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

/*SC530AI WDR 1620P30 */
// cleaned_0x03_SC530AI_27Minput_792Mbps_4lane_10bit_2880x1620_SHDR_30fps(1)
static void sc530ai_4l_wdr_1620p30_init(VI_PIPE ViPipe) {
	usleep(10 * 1000);
	sc530ai_4l_write_register(ViPipe, 0x0103, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x0100, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x36e9, 0x80);
	sc530ai_4l_write_register(ViPipe, 0x37f9, 0x80);
	sc530ai_4l_write_register(ViPipe, 0x301f, 0x03);
	sc530ai_4l_write_register(ViPipe, 0x320e, 0x0c);
	sc530ai_4l_write_register(ViPipe, 0x320f, 0xe4);
	sc530ai_4l_write_register(ViPipe, 0x3250, 0xff);
	sc530ai_4l_write_register(ViPipe, 0x3251, 0x98);
	sc530ai_4l_write_register(ViPipe, 0x3253, 0x0c);
	sc530ai_4l_write_register(ViPipe, 0x325f, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x3281, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x3301, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3304, 0x58);
	sc530ai_4l_write_register(ViPipe, 0x3306, 0xa0);
	sc530ai_4l_write_register(ViPipe, 0x3308, 0x14);
	sc530ai_4l_write_register(ViPipe, 0x3309, 0x50);
	sc530ai_4l_write_register(ViPipe, 0x330a, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x330b, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x330d, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x331e, 0x49);
	sc530ai_4l_write_register(ViPipe, 0x331f, 0x41);
	sc530ai_4l_write_register(ViPipe, 0x3333, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x335d, 0x60);
	sc530ai_4l_write_register(ViPipe, 0x335e, 0x06);
	sc530ai_4l_write_register(ViPipe, 0x335f, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3364, 0x56);
	sc530ai_4l_write_register(ViPipe, 0x3366, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x337c, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x337d, 0x0a);
	sc530ai_4l_write_register(ViPipe, 0x3390, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x3391, 0x03);
	sc530ai_4l_write_register(ViPipe, 0x3392, 0x07);
	sc530ai_4l_write_register(ViPipe, 0x3393, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3394, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3395, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3396, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x3397, 0x4b);
	sc530ai_4l_write_register(ViPipe, 0x3398, 0x4f);
	sc530ai_4l_write_register(ViPipe, 0x3399, 0x0a);
	sc530ai_4l_write_register(ViPipe, 0x339a, 0x0a);
	sc530ai_4l_write_register(ViPipe, 0x339b, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x339c, 0x22);
	sc530ai_4l_write_register(ViPipe, 0x33a2, 0x04);
	sc530ai_4l_write_register(ViPipe, 0x33ad, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x33ae, 0x38);
	sc530ai_4l_write_register(ViPipe, 0x33af, 0x38);
	sc530ai_4l_write_register(ViPipe, 0x33b1, 0x80);
	sc530ai_4l_write_register(ViPipe, 0x33b2, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x33b3, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x3400, 0x04);
	sc530ai_4l_write_register(ViPipe, 0x349f, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x34a6, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x34a7, 0x4b);
	sc530ai_4l_write_register(ViPipe, 0x34a8, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x34a9, 0x18);
	sc530ai_4l_write_register(ViPipe, 0x34f8, 0x5f);
	sc530ai_4l_write_register(ViPipe, 0x34f9, 0x04);
	sc530ai_4l_write_register(ViPipe, 0x3632, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x3633, 0x32);
	sc530ai_4l_write_register(ViPipe, 0x3637, 0x29);
	sc530ai_4l_write_register(ViPipe, 0x3638, 0xCD);
	sc530ai_4l_write_register(ViPipe, 0x3639, 0xF8);
	sc530ai_4l_write_register(ViPipe, 0x363b, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x363d, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x3670, 0x09);
	sc530ai_4l_write_register(ViPipe, 0x3674, 0x88);
	sc530ai_4l_write_register(ViPipe, 0x3675, 0x88);
	sc530ai_4l_write_register(ViPipe, 0x3676, 0x88);
	sc530ai_4l_write_register(ViPipe, 0x367c, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x367d, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x3690, 0x33);
	sc530ai_4l_write_register(ViPipe, 0x3691, 0x34);
	sc530ai_4l_write_register(ViPipe, 0x3692, 0x55);
	sc530ai_4l_write_register(ViPipe, 0x3693, 0x4b);
	sc530ai_4l_write_register(ViPipe, 0x3694, 0x4f);
	sc530ai_4l_write_register(ViPipe, 0x3698, 0x85);
	sc530ai_4l_write_register(ViPipe, 0x3699, 0x8f);
	sc530ai_4l_write_register(ViPipe, 0x369a, 0xa0);
	sc530ai_4l_write_register(ViPipe, 0x369b, 0xD8);
	sc530ai_4l_write_register(ViPipe, 0x36a2, 0x49);
	sc530ai_4l_write_register(ViPipe, 0x36a3, 0x4b);
	sc530ai_4l_write_register(ViPipe, 0x36a4, 0x4f);
	sc530ai_4l_write_register(ViPipe, 0x36d0, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x370f, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x3722, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x3728, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x37b0, 0x03);
	sc530ai_4l_write_register(ViPipe, 0x37b1, 0x03);
	sc530ai_4l_write_register(ViPipe, 0x37b2, 0x83);
	sc530ai_4l_write_register(ViPipe, 0x37b3, 0x48);
	sc530ai_4l_write_register(ViPipe, 0x37b4, 0x4f);
	sc530ai_4l_write_register(ViPipe, 0x3901, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x3902, 0xc5);
	sc530ai_4l_write_register(ViPipe, 0x3904, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3905, 0x8d);
	sc530ai_4l_write_register(ViPipe, 0x3909, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x391d, 0x04);
	sc530ai_4l_write_register(ViPipe, 0x3926, 0x21);
	sc530ai_4l_write_register(ViPipe, 0x3929, 0x18);
	sc530ai_4l_write_register(ViPipe, 0x3933, 0x82);
	sc530ai_4l_write_register(ViPipe, 0x3934, 0x08);
	sc530ai_4l_write_register(ViPipe, 0x3937, 0x5b);
	sc530ai_4l_write_register(ViPipe, 0x3939, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x393a, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x39dc, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x3c0f, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x3e00, 0x01);
	sc530ai_4l_write_register(ViPipe, 0x3e01, 0x82);
	sc530ai_4l_write_register(ViPipe, 0x3e02, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x3e04, 0x18);
	sc530ai_4l_write_register(ViPipe, 0x3e05, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x3e23, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x3e24, 0xc8);
	sc530ai_4l_write_register(ViPipe, 0x440e, 0x02);
	sc530ai_4l_write_register(ViPipe, 0x4509, 0x20);
	sc530ai_4l_write_register(ViPipe, 0x4816, 0x11);
	sc530ai_4l_write_register(ViPipe, 0x5010, 0x10);
	sc530ai_4l_write_register(ViPipe, 0x5780, 0x66);
	sc530ai_4l_write_register(ViPipe, 0x578d, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x5799, 0x06);
	sc530ai_4l_write_register(ViPipe, 0x57ad, 0x00);
	sc530ai_4l_write_register(ViPipe, 0x5ae0, 0xfe);
	sc530ai_4l_write_register(ViPipe, 0x5ae1, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x5ae2, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5ae3, 0x2a);
	sc530ai_4l_write_register(ViPipe, 0x5ae4, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x5ae5, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5ae6, 0x2a);
	sc530ai_4l_write_register(ViPipe, 0x5ae7, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x5ae8, 0x3c);
	sc530ai_4l_write_register(ViPipe, 0x5ae9, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5aea, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x5aeb, 0x3c);
	sc530ai_4l_write_register(ViPipe, 0x5aec, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5aed, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x5aee, 0xfe);
	sc530ai_4l_write_register(ViPipe, 0x5aef, 0x40);
	sc530ai_4l_write_register(ViPipe, 0x5af4, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5af5, 0x2a);
	sc530ai_4l_write_register(ViPipe, 0x5af6, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x5af7, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5af8, 0x2a);
	sc530ai_4l_write_register(ViPipe, 0x5af9, 0x24);
	sc530ai_4l_write_register(ViPipe, 0x5afa, 0x3c);
	sc530ai_4l_write_register(ViPipe, 0x5afb, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5afc, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x5afd, 0x3c);
	sc530ai_4l_write_register(ViPipe, 0x5afe, 0x30);
	sc530ai_4l_write_register(ViPipe, 0x5aff, 0x28);
	sc530ai_4l_write_register(ViPipe, 0x36e9, 0x44);
	sc530ai_4l_write_register(ViPipe, 0x37f9, 0x44);
	sc530ai_4l_write_register(ViPipe, 0x0100, 0x01);

	sc530ai_4l_default_reg_init(ViPipe);

	sc530ai_4l_write_register(ViPipe, 0x0100, 0x01);

	usleep(100 * 1000);

	printf("ViPipe:%d,===SC530AI_4L 1620P 30fps 10bit WDR Init OK!===\n", ViPipe);
}