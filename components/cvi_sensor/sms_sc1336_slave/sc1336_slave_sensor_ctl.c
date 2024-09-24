#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "sc1336_slave_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>


static void sc1336_slave_linear_720p60_init(VI_PIPE ViPipe);
static void sc1336_slave_linear_720p30_init(VI_PIPE ViPipe);

#define SC1336_SLAVE_CHIP_ID_HI_ADDR		0x3107
#define SC1336_SLAVE_CHIP_ID_LO_ADDR		0x3108
#define SC1336_SLAVE_CHIP_ID				0xca3f

CVI_U8 sc1336_slave_i2c_addr = 0x32;        /* I2C Address of SC1336_SLAVE */

/*msg type  address:16-bit  data:8-bit  dev addr:7-bit*/
const CVI_U32 sc1336_slave_addr_byte = 2;
const CVI_U32 sc1336_slave_data_byte = 1;
// static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int sc1336_slave_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunsc1336_Slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc1336_slave_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunsc1336_Slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc1336_slave_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunsc1336_Slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc1336_slave_i2c_addr, (CVI_U32)addr, sc1336_slave_addr_byte, sc1336_slave_data_byte);
}

int sc1336_slave_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunsc1336_Slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sc1336_slave_i2c_addr, (CVI_U32)addr, sc1336_slave_addr_byte,
				(CVI_U32)data, sc1336_slave_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc1336_slave_standby(VI_PIPE ViPipe)
{
	sc1336_slave_write_register(ViPipe, 0x0100, 0x00);
}

void sc1336_slave_restart(VI_PIPE ViPipe)
{
	sc1336_slave_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc1336_slave_write_register(ViPipe, 0x0100, 0x01);
}

void sc1336_slave_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastsc1336_slave[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc1336_slave_write_register(ViPipe,
			g_pastsc1336_slave[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
			g_pastsc1336_slave[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int sc1336_slave_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	usleep(4*1000);
	if (sc1336_slave_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc1336_slave_read_register(ViPipe, SC1336_SLAVE_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc1336_slave_read_register(ViPipe, SC1336_SLAVE_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC1336_SLAVE_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc1336_slave_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode = g_pastsc1336_slave[ViPipe]->u8ImgMode;

	sc1336_slave_i2c_init(ViPipe);

	if ((u8ImgMode == SC1336_SLAVE_MODE_720P30) || (u8ImgMode == SC1336_SLAVE_MODE_720P30_WDR))
		sc1336_slave_linear_720p30_init(ViPipe);
	else if ((u8ImgMode == SC1336_SLAVE_MODE_720P60) || (u8ImgMode == SC1336_SLAVE_MODE_720P60_WDR))
		sc1336_slave_linear_720p60_init(ViPipe);

	g_pastsc1336_slave[ViPipe]->bInit = CVI_TRUE;
}

void sc1336_slave_exit(VI_PIPE ViPipe)
{
	sc1336_slave_i2c_exit(ViPipe);
}

/* 720P30 */
static void sc1336_slave_linear_720p30_init(VI_PIPE ViPipe)
{
/*****************************************************************/
	sc1336_slave_write_register(ViPipe, 0x0103, 0x01);
	sc1336_slave_write_register(ViPipe, 0x0100, 0x00);
	sc1336_slave_write_register(ViPipe, 0x36e9, 0x80);
	sc1336_slave_write_register(ViPipe, 0x37f9, 0x80);
	sc1336_slave_write_register(ViPipe, 0x301f, 0xff);
	sc1336_slave_write_register(ViPipe, 0x3248, 0x04);
	sc1336_slave_write_register(ViPipe, 0x3249, 0x0b);
	sc1336_slave_write_register(ViPipe, 0x3301, 0x03);
	sc1336_slave_write_register(ViPipe, 0x3302, 0x10);
	sc1336_slave_write_register(ViPipe, 0x3303, 0x10);
	sc1336_slave_write_register(ViPipe, 0x3304, 0x40);
	sc1336_slave_write_register(ViPipe, 0x3306, 0x38);
	sc1336_slave_write_register(ViPipe, 0x3307, 0x02);
	sc1336_slave_write_register(ViPipe, 0x3308, 0x08);
	sc1336_slave_write_register(ViPipe, 0x3309, 0x60);
	sc1336_slave_write_register(ViPipe, 0x330a, 0x00);
	sc1336_slave_write_register(ViPipe, 0x330b, 0x70);
	sc1336_slave_write_register(ViPipe, 0x330c, 0x16);
	sc1336_slave_write_register(ViPipe, 0x330d, 0x10);
	sc1336_slave_write_register(ViPipe, 0x330e, 0x10);
	sc1336_slave_write_register(ViPipe, 0x3318, 0x02);
	sc1336_slave_write_register(ViPipe, 0x331c, 0x01);
	sc1336_slave_write_register(ViPipe, 0x331e, 0x39);
	sc1336_slave_write_register(ViPipe, 0x331f, 0x59);
	sc1336_slave_write_register(ViPipe, 0x3327, 0x0a);
	sc1336_slave_write_register(ViPipe, 0x3333, 0x10);
	sc1336_slave_write_register(ViPipe, 0x3334, 0x40);
	sc1336_slave_write_register(ViPipe, 0x335e, 0x06);
	sc1336_slave_write_register(ViPipe, 0x335f, 0x0a);
	sc1336_slave_write_register(ViPipe, 0x3364, 0x1f);
	sc1336_slave_write_register(ViPipe, 0x337a, 0x02);
	sc1336_slave_write_register(ViPipe, 0x337b, 0x06);
	sc1336_slave_write_register(ViPipe, 0x337c, 0x02);
	sc1336_slave_write_register(ViPipe, 0x337d, 0x0e);
	sc1336_slave_write_register(ViPipe, 0x3390, 0x01);
	sc1336_slave_write_register(ViPipe, 0x3391, 0x07);
	sc1336_slave_write_register(ViPipe, 0x3392, 0x0f);
	sc1336_slave_write_register(ViPipe, 0x3393, 0x03);
	sc1336_slave_write_register(ViPipe, 0x3394, 0x03);
	sc1336_slave_write_register(ViPipe, 0x3395, 0x03);
	sc1336_slave_write_register(ViPipe, 0x3396, 0x48);
	sc1336_slave_write_register(ViPipe, 0x3397, 0x49);
	sc1336_slave_write_register(ViPipe, 0x3398, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x3399, 0x02);
	sc1336_slave_write_register(ViPipe, 0x339a, 0x04);
	sc1336_slave_write_register(ViPipe, 0x339b, 0x10);
	sc1336_slave_write_register(ViPipe, 0x339c, 0x90);
	sc1336_slave_write_register(ViPipe, 0x33a2, 0x04);
	sc1336_slave_write_register(ViPipe, 0x33a3, 0x04);
	sc1336_slave_write_register(ViPipe, 0x33ad, 0x0c);
	sc1336_slave_write_register(ViPipe, 0x33b1, 0x80);
	sc1336_slave_write_register(ViPipe, 0x33b2, 0x50);
	sc1336_slave_write_register(ViPipe, 0x33b3, 0x38);
	sc1336_slave_write_register(ViPipe, 0x33f9, 0x38);
	sc1336_slave_write_register(ViPipe, 0x33fb, 0x48);
	sc1336_slave_write_register(ViPipe, 0x33fc, 0x4b);
	sc1336_slave_write_register(ViPipe, 0x33fd, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x349f, 0x03);
	sc1336_slave_write_register(ViPipe, 0x34a6, 0x49);
	sc1336_slave_write_register(ViPipe, 0x34a7, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x34a8, 0x28);
	sc1336_slave_write_register(ViPipe, 0x34a9, 0x00);
	sc1336_slave_write_register(ViPipe, 0x34aa, 0x00);
	sc1336_slave_write_register(ViPipe, 0x34ab, 0x70);
	sc1336_slave_write_register(ViPipe, 0x34ac, 0x00);
	sc1336_slave_write_register(ViPipe, 0x34ad, 0x80);
	sc1336_slave_write_register(ViPipe, 0x3630, 0xc0);
	sc1336_slave_write_register(ViPipe, 0x3631, 0x84);
	sc1336_slave_write_register(ViPipe, 0x3632, 0x78);
	sc1336_slave_write_register(ViPipe, 0x3633, 0x42);
	sc1336_slave_write_register(ViPipe, 0x3637, 0x2a);
	sc1336_slave_write_register(ViPipe, 0x363a, 0x88);
	sc1336_slave_write_register(ViPipe, 0x363b, 0x03);
	sc1336_slave_write_register(ViPipe, 0x363c, 0x08);
	sc1336_slave_write_register(ViPipe, 0x3641, 0x3a);
	sc1336_slave_write_register(ViPipe, 0x3670, 0x0f);
	sc1336_slave_write_register(ViPipe, 0x3674, 0xb0);
	sc1336_slave_write_register(ViPipe, 0x3675, 0xc0);
	sc1336_slave_write_register(ViPipe, 0x3676, 0xc0);
	sc1336_slave_write_register(ViPipe, 0x367c, 0x40);
	sc1336_slave_write_register(ViPipe, 0x367d, 0x48);
	sc1336_slave_write_register(ViPipe, 0x3690, 0x33);
	sc1336_slave_write_register(ViPipe, 0x3691, 0x43);
	sc1336_slave_write_register(ViPipe, 0x3692, 0x53);
	sc1336_slave_write_register(ViPipe, 0x3693, 0x84);
	sc1336_slave_write_register(ViPipe, 0x3694, 0x88);
	sc1336_slave_write_register(ViPipe, 0x3695, 0x8c);
	sc1336_slave_write_register(ViPipe, 0x3698, 0x89);
	sc1336_slave_write_register(ViPipe, 0x3699, 0x92);
	sc1336_slave_write_register(ViPipe, 0x369a, 0xa5);
	sc1336_slave_write_register(ViPipe, 0x369b, 0xca);
	sc1336_slave_write_register(ViPipe, 0x369c, 0x48);
	sc1336_slave_write_register(ViPipe, 0x369d, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x369e, 0x48);
	sc1336_slave_write_register(ViPipe, 0x369f, 0x4b);
	sc1336_slave_write_register(ViPipe, 0x36a2, 0x49);
	sc1336_slave_write_register(ViPipe, 0x36a3, 0x4b);
	sc1336_slave_write_register(ViPipe, 0x36a4, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x36a6, 0x49);
	sc1336_slave_write_register(ViPipe, 0x36a7, 0x4b);
	sc1336_slave_write_register(ViPipe, 0x36ab, 0x74);
	sc1336_slave_write_register(ViPipe, 0x36ac, 0x74);
	sc1336_slave_write_register(ViPipe, 0x36ad, 0x78);
	sc1336_slave_write_register(ViPipe, 0x36d0, 0x01);
	sc1336_slave_write_register(ViPipe, 0x370f, 0x01);
	sc1336_slave_write_register(ViPipe, 0x3722, 0x01);
	sc1336_slave_write_register(ViPipe, 0x3724, 0x41);
	sc1336_slave_write_register(ViPipe, 0x3725, 0xc4);
	sc1336_slave_write_register(ViPipe, 0x37b0, 0x01);
	sc1336_slave_write_register(ViPipe, 0x37b1, 0x01);
	sc1336_slave_write_register(ViPipe, 0x37b2, 0x01);
	sc1336_slave_write_register(ViPipe, 0x37b3, 0x4b);
	sc1336_slave_write_register(ViPipe, 0x37b4, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x37fa, 0x0b);
	sc1336_slave_write_register(ViPipe, 0x37fb, 0x35);
	sc1336_slave_write_register(ViPipe, 0x37fc, 0x01);
	sc1336_slave_write_register(ViPipe, 0x37fd, 0x07);
	sc1336_slave_write_register(ViPipe, 0x3900, 0x0d);
	sc1336_slave_write_register(ViPipe, 0x3902, 0xdf);
	sc1336_slave_write_register(ViPipe, 0x3905, 0xb8);
	sc1336_slave_write_register(ViPipe, 0x3908, 0x41);
	sc1336_slave_write_register(ViPipe, 0x391b, 0x81);
	sc1336_slave_write_register(ViPipe, 0x391c, 0x10);
	sc1336_slave_write_register(ViPipe, 0x391f, 0x30);
	sc1336_slave_write_register(ViPipe, 0x3933, 0x81);
	sc1336_slave_write_register(ViPipe, 0x3934, 0xd9);
	sc1336_slave_write_register(ViPipe, 0x3940, 0x70);
	sc1336_slave_write_register(ViPipe, 0x3941, 0x00);
	sc1336_slave_write_register(ViPipe, 0x3942, 0x01);
	sc1336_slave_write_register(ViPipe, 0x3943, 0xdc);
	sc1336_slave_write_register(ViPipe, 0x3952, 0x02);
	sc1336_slave_write_register(ViPipe, 0x3953, 0x0f);
	sc1336_slave_write_register(ViPipe, 0x3e01, 0x5d);
	sc1336_slave_write_register(ViPipe, 0x3e02, 0x80);
	sc1336_slave_write_register(ViPipe, 0x3e08, 0x1f);
	sc1336_slave_write_register(ViPipe, 0x3e1b, 0x14);
	sc1336_slave_write_register(ViPipe, 0x4509, 0x1c);
	sc1336_slave_write_register(ViPipe, 0x481f, 0x01);
	sc1336_slave_write_register(ViPipe, 0x4827, 0x02);
	sc1336_slave_write_register(ViPipe, 0x4831, 0x02);
	sc1336_slave_write_register(ViPipe, 0x5799, 0x00);
	sc1336_slave_write_register(ViPipe, 0x5ae0, 0xfe);
	sc1336_slave_write_register(ViPipe, 0x5ae1, 0x40);
	sc1336_slave_write_register(ViPipe, 0x5ae2, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5ae3, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5ae4, 0x20);
	sc1336_slave_write_register(ViPipe, 0x5ae5, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5ae6, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5ae7, 0x20);
	sc1336_slave_write_register(ViPipe, 0x5ae8, 0x3c);
	sc1336_slave_write_register(ViPipe, 0x5ae9, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5aea, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5aeb, 0x3c);
	sc1336_slave_write_register(ViPipe, 0x5aec, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5aed, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5aee, 0xfe);
	sc1336_slave_write_register(ViPipe, 0x5aef, 0x40);
	sc1336_slave_write_register(ViPipe, 0x5af4, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5af5, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5af6, 0x20);
	sc1336_slave_write_register(ViPipe, 0x5af7, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5af8, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5af9, 0x20);
	sc1336_slave_write_register(ViPipe, 0x5afa, 0x3c);
	sc1336_slave_write_register(ViPipe, 0x5afb, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5afc, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5afd, 0x3c);
	sc1336_slave_write_register(ViPipe, 0x5afe, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5aff, 0x28);
	sc1336_slave_write_register(ViPipe, 0x36e9, 0x20);
	sc1336_slave_write_register(ViPipe, 0x37f9, 0x27);
/*****************************************************************/

	sc1336_slave_default_reg_init(ViPipe);

	sc1336_slave_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC1336_SLAVE 720P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

/* 720P60 */
static void sc1336_slave_linear_720p60_init(VI_PIPE ViPipe)
{

	sc1336_slave_write_register(ViPipe, 0x0103, 0x01);
	sc1336_slave_write_register(ViPipe, 0x0100, 0x00);
	sc1336_slave_write_register(ViPipe, 0x36e9, 0x80);
	sc1336_slave_write_register(ViPipe, 0x37f9, 0x80);
	sc1336_slave_write_register(ViPipe, 0x301f, 0x02);
	sc1336_slave_write_register(ViPipe, 0x3248, 0x04);
	sc1336_slave_write_register(ViPipe, 0x3249, 0x0b);
	sc1336_slave_write_register(ViPipe, 0x3301, 0x04);
	sc1336_slave_write_register(ViPipe, 0x3302, 0x10);
	sc1336_slave_write_register(ViPipe, 0x3303, 0x10);
	sc1336_slave_write_register(ViPipe, 0x3304, 0x40);
	sc1336_slave_write_register(ViPipe, 0x3306, 0x38);
	sc1336_slave_write_register(ViPipe, 0x3307, 0x02);
	sc1336_slave_write_register(ViPipe, 0x3308, 0x08);
	sc1336_slave_write_register(ViPipe, 0x3309, 0x60);
	sc1336_slave_write_register(ViPipe, 0x330a, 0x00);
	sc1336_slave_write_register(ViPipe, 0x330b, 0xa0);
	sc1336_slave_write_register(ViPipe, 0x330c, 0x16);
	sc1336_slave_write_register(ViPipe, 0x330d, 0x10);
	sc1336_slave_write_register(ViPipe, 0x330e, 0x10);
	sc1336_slave_write_register(ViPipe, 0x3318, 0x02);
	sc1336_slave_write_register(ViPipe, 0x331e, 0x39);
	sc1336_slave_write_register(ViPipe, 0x331f, 0x59);
	sc1336_slave_write_register(ViPipe, 0x3327, 0x0a);
	sc1336_slave_write_register(ViPipe, 0x3333, 0x10);
	sc1336_slave_write_register(ViPipe, 0x3334, 0x40);
	sc1336_slave_write_register(ViPipe, 0x335e, 0x06);
	sc1336_slave_write_register(ViPipe, 0x335f, 0x0a);
	sc1336_slave_write_register(ViPipe, 0x3364, 0x1f);
	sc1336_slave_write_register(ViPipe, 0x337a, 0x02);
	sc1336_slave_write_register(ViPipe, 0x337b, 0x06);
	sc1336_slave_write_register(ViPipe, 0x337c, 0x02);
	sc1336_slave_write_register(ViPipe, 0x337d, 0x0e);
	sc1336_slave_write_register(ViPipe, 0x3390, 0x01);
	sc1336_slave_write_register(ViPipe, 0x3391, 0x07);
	sc1336_slave_write_register(ViPipe, 0x3392, 0x0f);
	sc1336_slave_write_register(ViPipe, 0x3393, 0x04);
	sc1336_slave_write_register(ViPipe, 0x3394, 0x04);
	sc1336_slave_write_register(ViPipe, 0x3395, 0x04);
	sc1336_slave_write_register(ViPipe, 0x3396, 0x48);
	sc1336_slave_write_register(ViPipe, 0x3397, 0x49);
	sc1336_slave_write_register(ViPipe, 0x3398, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x3399, 0x04);
	sc1336_slave_write_register(ViPipe, 0x339a, 0x05);
	sc1336_slave_write_register(ViPipe, 0x339b, 0x20);
	sc1336_slave_write_register(ViPipe, 0x339c, 0x5c);
	sc1336_slave_write_register(ViPipe, 0x33a2, 0x04);
	sc1336_slave_write_register(ViPipe, 0x33a3, 0x04);
	sc1336_slave_write_register(ViPipe, 0x33ad, 0x0c);
	sc1336_slave_write_register(ViPipe, 0x33b1, 0x80);
	sc1336_slave_write_register(ViPipe, 0x33b2, 0x54);
	sc1336_slave_write_register(ViPipe, 0x33b3, 0x48);
	sc1336_slave_write_register(ViPipe, 0x33f9, 0x48);
	sc1336_slave_write_register(ViPipe, 0x33fb, 0x68);
	sc1336_slave_write_register(ViPipe, 0x33fc, 0x49);
	sc1336_slave_write_register(ViPipe, 0x33fd, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x349f, 0x03);
	sc1336_slave_write_register(ViPipe, 0x34a6, 0x49);
	sc1336_slave_write_register(ViPipe, 0x34a7, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x34a8, 0x30);
	sc1336_slave_write_register(ViPipe, 0x34a9, 0x18);
	sc1336_slave_write_register(ViPipe, 0x34aa, 0x00);
	sc1336_slave_write_register(ViPipe, 0x34ab, 0xa8);
	sc1336_slave_write_register(ViPipe, 0x34ac, 0x00);
	sc1336_slave_write_register(ViPipe, 0x34ad, 0xc8);
	sc1336_slave_write_register(ViPipe, 0x3630, 0xc0);
	sc1336_slave_write_register(ViPipe, 0x3631, 0x84);
	sc1336_slave_write_register(ViPipe, 0x3632, 0x74);
	sc1336_slave_write_register(ViPipe, 0x3633, 0x52);
	sc1336_slave_write_register(ViPipe, 0x3637, 0x2a);
	sc1336_slave_write_register(ViPipe, 0x363a, 0x89);
	sc1336_slave_write_register(ViPipe, 0x363b, 0x03);
	sc1336_slave_write_register(ViPipe, 0x363c, 0x08);
	sc1336_slave_write_register(ViPipe, 0x3641, 0x3a);
	sc1336_slave_write_register(ViPipe, 0x3670, 0x0f);
	sc1336_slave_write_register(ViPipe, 0x3674, 0xb0);
	sc1336_slave_write_register(ViPipe, 0x3675, 0xc0);
	sc1336_slave_write_register(ViPipe, 0x3676, 0xc0);
	sc1336_slave_write_register(ViPipe, 0x367c, 0x40);
	sc1336_slave_write_register(ViPipe, 0x367d, 0x48);
	sc1336_slave_write_register(ViPipe, 0x3690, 0x43);
	sc1336_slave_write_register(ViPipe, 0x3691, 0x43);
	sc1336_slave_write_register(ViPipe, 0x3692, 0x63);
	sc1336_slave_write_register(ViPipe, 0x3693, 0x84);
	sc1336_slave_write_register(ViPipe, 0x3694, 0x88);
	sc1336_slave_write_register(ViPipe, 0x3695, 0x8a);
	sc1336_slave_write_register(ViPipe, 0x3698, 0x89);
	sc1336_slave_write_register(ViPipe, 0x3699, 0x92);
	sc1336_slave_write_register(ViPipe, 0x369a, 0xa5);
	sc1336_slave_write_register(ViPipe, 0x369b, 0xca);
	sc1336_slave_write_register(ViPipe, 0x369c, 0x48);
	sc1336_slave_write_register(ViPipe, 0x369d, 0x5f);
	sc1336_slave_write_register(ViPipe, 0x369e, 0x48);
	sc1336_slave_write_register(ViPipe, 0x369f, 0x4b);
	sc1336_slave_write_register(ViPipe, 0x36a2, 0x49);
	sc1336_slave_write_register(ViPipe, 0x36a3, 0x4b);
	sc1336_slave_write_register(ViPipe, 0x36a4, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x36a6, 0x49);
	sc1336_slave_write_register(ViPipe, 0x36a7, 0x4b);
	sc1336_slave_write_register(ViPipe, 0x36ab, 0x74);
	sc1336_slave_write_register(ViPipe, 0x36ac, 0x74);
	sc1336_slave_write_register(ViPipe, 0x36ad, 0x78);
	sc1336_slave_write_register(ViPipe, 0x36d0, 0x01);
	sc1336_slave_write_register(ViPipe, 0x36eb, 0x0c);
	sc1336_slave_write_register(ViPipe, 0x370f, 0x01);
	sc1336_slave_write_register(ViPipe, 0x3722, 0x01);
	sc1336_slave_write_register(ViPipe, 0x3724, 0x41);
	sc1336_slave_write_register(ViPipe, 0x3725, 0xc4);
	sc1336_slave_write_register(ViPipe, 0x37b0, 0x01);
	sc1336_slave_write_register(ViPipe, 0x37b1, 0x01);
	sc1336_slave_write_register(ViPipe, 0x37b2, 0x01);
	sc1336_slave_write_register(ViPipe, 0x37b3, 0x4f);
	sc1336_slave_write_register(ViPipe, 0x37b4, 0x5f);
	sc1336_slave_write_register(ViPipe, 0x37fa, 0x0b);
	sc1336_slave_write_register(ViPipe, 0x37fb, 0x32);
	sc1336_slave_write_register(ViPipe, 0x37fc, 0x01);
	sc1336_slave_write_register(ViPipe, 0x37fd, 0x07);
	sc1336_slave_write_register(ViPipe, 0x3900, 0x0d);
	sc1336_slave_write_register(ViPipe, 0x3902, 0xdf);
	sc1336_slave_write_register(ViPipe, 0x3905, 0xb8);
	sc1336_slave_write_register(ViPipe, 0x3908, 0x41);
	sc1336_slave_write_register(ViPipe, 0x391b, 0x81);
	sc1336_slave_write_register(ViPipe, 0x391c, 0x10);
	sc1336_slave_write_register(ViPipe, 0x391f, 0x30);
	sc1336_slave_write_register(ViPipe, 0x3933, 0x81);
	sc1336_slave_write_register(ViPipe, 0x3934, 0xd4);
	sc1336_slave_write_register(ViPipe, 0x3940, 0x6b);
	sc1336_slave_write_register(ViPipe, 0x3941, 0x00);
	sc1336_slave_write_register(ViPipe, 0x3942, 0x01);
	sc1336_slave_write_register(ViPipe, 0x3943, 0xd7);
	sc1336_slave_write_register(ViPipe, 0x3952, 0x02);
	sc1336_slave_write_register(ViPipe, 0x3953, 0x0f);
	sc1336_slave_write_register(ViPipe, 0x3e01, 0x5d);
	sc1336_slave_write_register(ViPipe, 0x3e02, 0x80);
	sc1336_slave_write_register(ViPipe, 0x3e08, 0x1f);
	sc1336_slave_write_register(ViPipe, 0x3e1b, 0x14);
	sc1336_slave_write_register(ViPipe, 0x4509, 0x1c);
	sc1336_slave_write_register(ViPipe, 0x481f, 0x01);
	sc1336_slave_write_register(ViPipe, 0x4827, 0x02);
	sc1336_slave_write_register(ViPipe, 0x4831, 0x02);
	sc1336_slave_write_register(ViPipe, 0x5799, 0x06);
	sc1336_slave_write_register(ViPipe, 0x5ae0, 0xfe);
	sc1336_slave_write_register(ViPipe, 0x5ae1, 0x40);
	sc1336_slave_write_register(ViPipe, 0x5ae2, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5ae3, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5ae4, 0x20);
	sc1336_slave_write_register(ViPipe, 0x5ae5, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5ae6, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5ae7, 0x20);
	sc1336_slave_write_register(ViPipe, 0x5ae8, 0x3c);
	sc1336_slave_write_register(ViPipe, 0x5ae9, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5aea, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5aeb, 0x3c);
	sc1336_slave_write_register(ViPipe, 0x5aec, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5aed, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5aee, 0xfe);
	sc1336_slave_write_register(ViPipe, 0x5aef, 0x40);
	sc1336_slave_write_register(ViPipe, 0x5af4, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5af5, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5af6, 0x20);
	sc1336_slave_write_register(ViPipe, 0x5af7, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5af8, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5af9, 0x20);
	sc1336_slave_write_register(ViPipe, 0x5afa, 0x3c);
	sc1336_slave_write_register(ViPipe, 0x5afb, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5afc, 0x28);
	sc1336_slave_write_register(ViPipe, 0x5afd, 0x3c);
	sc1336_slave_write_register(ViPipe, 0x5afe, 0x30);
	sc1336_slave_write_register(ViPipe, 0x5aff, 0x28);
	sc1336_slave_write_register(ViPipe, 0x36e9, 0x20);
	sc1336_slave_write_register(ViPipe, 0x37f9, 0x27);


	sc1336_slave_default_reg_init(ViPipe);

	sc1336_slave_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC1336_SLAVE 720P 60fps 10bit LINE Init OK!===\n", ViPipe);
}
