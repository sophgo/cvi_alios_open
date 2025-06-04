#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc4336p_cmos_ex.h"
#include "sensor_i2c.h"

static void sc4336p_linear_1440p30_init(VI_PIPE ViPipe);

CVI_U8 sc4336p_i2c_addr = 0x30;
const CVI_U32 sc4336p_addr_byte = 2;
const CVI_U32 sc4336p_data_byte = 1;

int sc4336p_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC4336P_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSc4336p_AddrInfo[ViPipe].s8I2cAddr);
}

int sc4336p_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC4336P_BusInfo[ViPipe].s8I2cDev);
}

int sc4336p_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC4336P_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSc4336p_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc4336p_addr_byte, sc4336p_data_byte);
}

int sc4336p_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC4336P_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSc4336p_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc4336p_addr_byte, (CVI_U32)data, sc4336p_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc4336p_standby(VI_PIPE ViPipe)
{
	sc4336p_write_register(ViPipe, 0x0100, 0x00);
}

void sc4336p_restart(VI_PIPE ViPipe)
{
	sc4336p_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc4336p_write_register(ViPipe, 0x0100, 0x01);
}

void sc4336p_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC4336P[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc4336p_write_register(ViPipe,
				g_pastSC4336P[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC4336P[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void sc4336p_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc4336p_write_register(ViPipe, 0x3221, val);
}

#define SC4336P_CHIP_ID_HI_ADDR		0x3107
#define SC4336P_CHIP_ID_LO_ADDR		0x3108
#define SC4336P_CHIP_ID			    0x9c42

int sc4336p_probe(VI_PIPE ViPipe)
{
	//int nVal;
	//CVI_U16 chip_id;

	//if (sc4336p_i2c_init(ViPipe) != CVI_SUCCESS)
	//	return CVI_FAILURE;

	//delay_ms(5);

	//nVal = sc4336p_read_register(ViPipe, SC4336P_CHIP_ID_HI_ADDR);
	//if (nVal < 0) {
	//	CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
	//	return nVal;
	//}
	//chip_id = (nVal & 0xFF) << 8;
	//nVal = sc4336p_read_register(ViPipe, SC4336P_CHIP_ID_LO_ADDR);
	//if (nVal < 0) {
	//	CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
	//	return nVal;
	//}
	//chip_id |= (nVal & 0xFF);

	//if (chip_id != SC4336P_CHIP_ID) {
	//	CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
	//	return CVI_FAILURE;
	//}
	printf("%d\n", ViPipe);
	return CVI_SUCCESS;
}


void sc4336p_init(VI_PIPE ViPipe)
{
	sc4336p_i2c_init(ViPipe);

	sc4336p_linear_1440p30_init(ViPipe);

	g_pastSC4336P[ViPipe]->bInit = CVI_TRUE;
}

/* 1440P30 and 1440P25 */
static void sc4336p_linear_1440p30_init(VI_PIPE ViPipe)
{
	sc4336p_write_register(ViPipe, 0x0103, 0x01);
	sc4336p_write_register(ViPipe, 0x36e9, 0x80);
	sc4336p_write_register(ViPipe, 0x37f9, 0x80);
	sc4336p_write_register(ViPipe, 0x300a, 0x24);
	sc4336p_write_register(ViPipe, 0x301f, 0x0b);
	sc4336p_write_register(ViPipe, 0x3032, 0xa0);
	sc4336p_write_register(ViPipe, 0x30b8, 0x44);
	sc4336p_write_register(ViPipe, 0x3224, 0x92);
	sc4336p_write_register(ViPipe, 0x3253, 0x10);
	sc4336p_write_register(ViPipe, 0x3301, 0x0a);
	sc4336p_write_register(ViPipe, 0x3302, 0xff);
	sc4336p_write_register(ViPipe, 0x3305, 0x00);
	sc4336p_write_register(ViPipe, 0x3306, 0x90);
	sc4336p_write_register(ViPipe, 0x3308, 0x08);
	sc4336p_write_register(ViPipe, 0x330a, 0x01);
	sc4336p_write_register(ViPipe, 0x330b, 0xb0);
	sc4336p_write_register(ViPipe, 0x330d, 0xf0);
	sc4336p_write_register(ViPipe, 0x3314, 0x14);
	sc4336p_write_register(ViPipe, 0x3333, 0x10);
	sc4336p_write_register(ViPipe, 0x3334, 0x40);
	sc4336p_write_register(ViPipe, 0x335e, 0x06);
	sc4336p_write_register(ViPipe, 0x335f, 0x0a);
	sc4336p_write_register(ViPipe, 0x3364, 0x5e);
	sc4336p_write_register(ViPipe, 0x337d, 0x0e);
	sc4336p_write_register(ViPipe, 0x338f, 0x20);
	sc4336p_write_register(ViPipe, 0x3390, 0x08);
	sc4336p_write_register(ViPipe, 0x3391, 0x09);
	sc4336p_write_register(ViPipe, 0x3392, 0x0f);
	sc4336p_write_register(ViPipe, 0x3393, 0x18);
	sc4336p_write_register(ViPipe, 0x3394, 0x60);
	sc4336p_write_register(ViPipe, 0x3395, 0xff);
	sc4336p_write_register(ViPipe, 0x3396, 0x08);
	sc4336p_write_register(ViPipe, 0x3397, 0x09);
	sc4336p_write_register(ViPipe, 0x3398, 0x0f);
	sc4336p_write_register(ViPipe, 0x3399, 0x0a);
	sc4336p_write_register(ViPipe, 0x339a, 0x18);
	sc4336p_write_register(ViPipe, 0x339b, 0x60);
	sc4336p_write_register(ViPipe, 0x339c, 0xff);
	sc4336p_write_register(ViPipe, 0x33a2, 0x04);
	sc4336p_write_register(ViPipe, 0x33ad, 0x0c);
	sc4336p_write_register(ViPipe, 0x33b2, 0x40);
	sc4336p_write_register(ViPipe, 0x33b3, 0x30);
	sc4336p_write_register(ViPipe, 0x33f8, 0x00);
	sc4336p_write_register(ViPipe, 0x33f9, 0xb0);
	sc4336p_write_register(ViPipe, 0x33fa, 0x00);
	sc4336p_write_register(ViPipe, 0x33fb, 0xf8);
	sc4336p_write_register(ViPipe, 0x33fc, 0x09);
	sc4336p_write_register(ViPipe, 0x33fd, 0x1f);
	sc4336p_write_register(ViPipe, 0x349f, 0x03);
	sc4336p_write_register(ViPipe, 0x34a6, 0x09);
	sc4336p_write_register(ViPipe, 0x34a7, 0x1f);
	sc4336p_write_register(ViPipe, 0x34a8, 0x28);
	sc4336p_write_register(ViPipe, 0x34a9, 0x28);
	sc4336p_write_register(ViPipe, 0x34aa, 0x01);
	sc4336p_write_register(ViPipe, 0x34ab, 0xe0);
	sc4336p_write_register(ViPipe, 0x34ac, 0x02);
	sc4336p_write_register(ViPipe, 0x34ad, 0x28);
	sc4336p_write_register(ViPipe, 0x34f8, 0x1f);
	sc4336p_write_register(ViPipe, 0x34f9, 0x20);
	sc4336p_write_register(ViPipe, 0x3630, 0xc0);
	sc4336p_write_register(ViPipe, 0x3631, 0x84);
	sc4336p_write_register(ViPipe, 0x3632, 0x54);
	sc4336p_write_register(ViPipe, 0x3633, 0x44);
	sc4336p_write_register(ViPipe, 0x3637, 0x49);
	sc4336p_write_register(ViPipe, 0x363f, 0xc0);
	sc4336p_write_register(ViPipe, 0x3641, 0x28);
	sc4336p_write_register(ViPipe, 0x3670, 0x56);
	sc4336p_write_register(ViPipe, 0x3674, 0xb0);
	sc4336p_write_register(ViPipe, 0x3675, 0xa0);
	sc4336p_write_register(ViPipe, 0x3676, 0xa0);
	sc4336p_write_register(ViPipe, 0x3677, 0x84);
	sc4336p_write_register(ViPipe, 0x3678, 0x88);
	sc4336p_write_register(ViPipe, 0x3679, 0x8d);
	sc4336p_write_register(ViPipe, 0x367c, 0x09);
	sc4336p_write_register(ViPipe, 0x367d, 0x0b);
	sc4336p_write_register(ViPipe, 0x367e, 0x08);
	sc4336p_write_register(ViPipe, 0x367f, 0x0f);
	sc4336p_write_register(ViPipe, 0x3696, 0x24);
	sc4336p_write_register(ViPipe, 0x3697, 0x34);
	sc4336p_write_register(ViPipe, 0x3698, 0x34);
	sc4336p_write_register(ViPipe, 0x36a0, 0x0f);
	sc4336p_write_register(ViPipe, 0x36a1, 0x1f);
	sc4336p_write_register(ViPipe, 0x36b0, 0x81);
	sc4336p_write_register(ViPipe, 0x36b1, 0x83);
	sc4336p_write_register(ViPipe, 0x36b2, 0x85);
	sc4336p_write_register(ViPipe, 0x36b3, 0x8b);
	sc4336p_write_register(ViPipe, 0x36b4, 0x09);
	sc4336p_write_register(ViPipe, 0x36b5, 0x0b);
	sc4336p_write_register(ViPipe, 0x36b6, 0x0f);
	sc4336p_write_register(ViPipe, 0x370f, 0x01);
	sc4336p_write_register(ViPipe, 0x3722, 0x09);
	sc4336p_write_register(ViPipe, 0x3724, 0x21);
	sc4336p_write_register(ViPipe, 0x3771, 0x09);
	sc4336p_write_register(ViPipe, 0x3772, 0x05);
	sc4336p_write_register(ViPipe, 0x3773, 0x05);
	sc4336p_write_register(ViPipe, 0x377a, 0x0f);
	sc4336p_write_register(ViPipe, 0x377b, 0x1f);
	sc4336p_write_register(ViPipe, 0x3905, 0x8c);
	sc4336p_write_register(ViPipe, 0x391d, 0x02);
	sc4336p_write_register(ViPipe, 0x391f, 0x49);
	sc4336p_write_register(ViPipe, 0x3926, 0x21);
	sc4336p_write_register(ViPipe, 0x3933, 0x80);
	sc4336p_write_register(ViPipe, 0x3934, 0x03);
	sc4336p_write_register(ViPipe, 0x3937, 0x7b);
	sc4336p_write_register(ViPipe, 0x3939, 0x00);
	sc4336p_write_register(ViPipe, 0x393a, 0x00);
	sc4336p_write_register(ViPipe, 0x39dc, 0x02);
	sc4336p_write_register(ViPipe, 0x3e00, 0x00);
	sc4336p_write_register(ViPipe, 0x3e01, 0x5d);
	sc4336p_write_register(ViPipe, 0x3e02, 0x40);
	sc4336p_write_register(ViPipe, 0x440d, 0x10);
	sc4336p_write_register(ViPipe, 0x440e, 0x01);
	sc4336p_write_register(ViPipe, 0x4509, 0x28);
	sc4336p_write_register(ViPipe, 0x450d, 0x32);
	sc4336p_write_register(ViPipe, 0x5000, 0x06);
	sc4336p_write_register(ViPipe, 0x5780, 0x76);
	sc4336p_write_register(ViPipe, 0x5784, 0x10);
	sc4336p_write_register(ViPipe, 0x5785, 0x04);
	sc4336p_write_register(ViPipe, 0x5787, 0x0a);
	sc4336p_write_register(ViPipe, 0x5788, 0x0a);
	sc4336p_write_register(ViPipe, 0x5789, 0x08);
	sc4336p_write_register(ViPipe, 0x578a, 0x0a);
	sc4336p_write_register(ViPipe, 0x578b, 0x0a);
	sc4336p_write_register(ViPipe, 0x578c, 0x08);
	sc4336p_write_register(ViPipe, 0x578d, 0x40);
	sc4336p_write_register(ViPipe, 0x5790, 0x08);
	sc4336p_write_register(ViPipe, 0x5791, 0x04);
	sc4336p_write_register(ViPipe, 0x5792, 0x04);
	sc4336p_write_register(ViPipe, 0x5793, 0x08);
	sc4336p_write_register(ViPipe, 0x5794, 0x04);
	sc4336p_write_register(ViPipe, 0x5795, 0x04);
	sc4336p_write_register(ViPipe, 0x5799, 0x46);
	sc4336p_write_register(ViPipe, 0x579a, 0x77);
	sc4336p_write_register(ViPipe, 0x57a1, 0x04);
	sc4336p_write_register(ViPipe, 0x57a8, 0xd2);
	sc4336p_write_register(ViPipe, 0x57aa, 0x2a);
	sc4336p_write_register(ViPipe, 0x57ab, 0x7f);
	sc4336p_write_register(ViPipe, 0x57ac, 0x00);
	sc4336p_write_register(ViPipe, 0x57ad, 0x00);
	sc4336p_write_register(ViPipe, 0x57d9, 0x46);
	sc4336p_write_register(ViPipe, 0x57da, 0x77);
	sc4336p_write_register(ViPipe, 0x59e2, 0x08);
	sc4336p_write_register(ViPipe, 0x59e3, 0x03);
	sc4336p_write_register(ViPipe, 0x59e4, 0x00);
	sc4336p_write_register(ViPipe, 0x59e5, 0x10);
	sc4336p_write_register(ViPipe, 0x59e6, 0x06);
	sc4336p_write_register(ViPipe, 0x59e7, 0x00);
	sc4336p_write_register(ViPipe, 0x59e8, 0x08);
	sc4336p_write_register(ViPipe, 0x59e9, 0x02);
	sc4336p_write_register(ViPipe, 0x59ea, 0x00);
	sc4336p_write_register(ViPipe, 0x59eb, 0x10);
	sc4336p_write_register(ViPipe, 0x59ec, 0x04);
	sc4336p_write_register(ViPipe, 0x59ed, 0x00);
	sc4336p_write_register(ViPipe, 0x5ae0, 0xfe);
	sc4336p_write_register(ViPipe, 0x5ae1, 0x40);
	sc4336p_write_register(ViPipe, 0x5ae2, 0x38);
	sc4336p_write_register(ViPipe, 0x5ae3, 0x30);
	sc4336p_write_register(ViPipe, 0x5ae4, 0x28);
	sc4336p_write_register(ViPipe, 0x5ae5, 0x38);
	sc4336p_write_register(ViPipe, 0x5ae6, 0x30);
	sc4336p_write_register(ViPipe, 0x5ae7, 0x28);
	sc4336p_write_register(ViPipe, 0x5ae8, 0x3f);
	sc4336p_write_register(ViPipe, 0x5ae9, 0x34);
	sc4336p_write_register(ViPipe, 0x5aea, 0x2c);
	sc4336p_write_register(ViPipe, 0x5aeb, 0x3f);
	sc4336p_write_register(ViPipe, 0x5aec, 0x34);
	sc4336p_write_register(ViPipe, 0x5aed, 0x2c);
	sc4336p_write_register(ViPipe, 0x36e9, 0x44);
	sc4336p_write_register(ViPipe, 0x37f9, 0x44);
	sc4336p_write_register(ViPipe, 0x0100, 0x01);

	sc4336p_default_reg_init(ViPipe);

	sc4336p_write_register(ViPipe, 0x0100, 0x01);

	delay_ms(100);

	printf("ViPipe:%d,===SC4336P 1440P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

