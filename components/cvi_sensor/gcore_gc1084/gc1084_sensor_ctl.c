#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "gc1084_cmos_ex.h"

#define GC1084_CHIP_ID_ADDR_H	0x03f0
#define GC1084_CHIP_ID_ADDR_L	0x03f1
#define GC1084_CHIP_ID		0x1084

static void gc1084_linear_720p30_init(VI_PIPE ViPipe);

const CVI_U8 gc1084_i2c_addr = 0x37;//0x42
const CVI_U32 gc1084_addr_byte = 2;
const CVI_U32 gc1084_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int gc1084_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc1084_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc1084_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc1084_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int gc1084_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc1084_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc1084_i2c_addr, (CVI_U32)addr, gc1084_addr_byte, gc1084_data_byte);
}

int gc1084_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc1084_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc1084_i2c_addr, (CVI_U32)addr, gc1084_addr_byte,
				(CVI_U32)data, gc1084_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void gc1084_standby(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);
	printf("%s\n", __func__);
}

void gc1084_restart(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);
	printf("%s\n", __func__);
}


int gc1084_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	if (gc1084_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc1084_read_register(ViPipe, GC1084_CHIP_ID_ADDR_H);
	nVal2 = gc1084_read_register(ViPipe, GC1084_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC1084_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc1084_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc1084[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc1084_write_register(ViPipe,
				g_pastGc1084[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc1084[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void gc1084_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastGc1084[ViPipe]->enWDRMode;

	gc1084_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
		gc1084_linear_720p30_init(ViPipe);
	}
	g_pastGc1084[ViPipe]->bInit = CVI_TRUE;
}

void gc1084_exit(VI_PIPE ViPipe)
{
	gc1084_i2c_exit(ViPipe);
}

static void gc1084_linear_720p30_init(VI_PIPE ViPipe)
{
	gc1084_write_register(ViPipe, 0x03fe, 0xf0);
	gc1084_write_register(ViPipe, 0x03fe, 0xf0);
	gc1084_write_register(ViPipe, 0x03fe, 0xf0);
	gc1084_write_register(ViPipe, 0x03fe, 0x00);
	gc1084_write_register(ViPipe, 0x03f2, 0x00);
	gc1084_write_register(ViPipe, 0x03f3, 0x00);
	gc1084_write_register(ViPipe, 0x03f4, 0x36);
	gc1084_write_register(ViPipe, 0x03f5, 0xc0);
	gc1084_write_register(ViPipe, 0x03f6, 0x13);
	gc1084_write_register(ViPipe, 0x03f7, 0x01);

	gc1084_write_register(ViPipe, 0x03f8, 0x2c);
	// gc1084_write_register(ViPipe, 0x03f8, 0x84);

	gc1084_write_register(ViPipe, 0x03f9, 0x21);
	gc1084_write_register(ViPipe, 0x03fc, 0xae);
	gc1084_write_register(ViPipe, 0x0d05, 0x08);
	gc1084_write_register(ViPipe, 0x0d06, 0x98);
	gc1084_write_register(ViPipe, 0x0d08, 0x10);
	gc1084_write_register(ViPipe, 0x0d0a, 0x02);
	gc1084_write_register(ViPipe, 0x000c, 0x03);
	gc1084_write_register(ViPipe, 0x0d0d, 0x02);
	gc1084_write_register(ViPipe, 0x0d0e, 0xd4);
	gc1084_write_register(ViPipe, 0x000f, 0x05);
	gc1084_write_register(ViPipe, 0x0010, 0x08);
	gc1084_write_register(ViPipe, 0x0017, 0x08);
	gc1084_write_register(ViPipe, 0x0d73, 0x92);
	gc1084_write_register(ViPipe, 0x0076, 0x00);
	gc1084_write_register(ViPipe, 0x0d76, 0x00);
	gc1084_write_register(ViPipe, 0x0d41, 0x02);
	gc1084_write_register(ViPipe, 0x0d42, 0xee);
	gc1084_write_register(ViPipe, 0x0d7a, 0x0a);
	gc1084_write_register(ViPipe, 0x006b, 0x18);
	gc1084_write_register(ViPipe, 0x0db0, 0x9d);
	gc1084_write_register(ViPipe, 0x0db1, 0x00);
	gc1084_write_register(ViPipe, 0x0db2, 0xac);
	gc1084_write_register(ViPipe, 0x0db3, 0xd5);
	gc1084_write_register(ViPipe, 0x0db4, 0x00);
	gc1084_write_register(ViPipe, 0x0db5, 0x97);
	gc1084_write_register(ViPipe, 0x0db6, 0x09);
	gc1084_write_register(ViPipe, 0x00d2, 0xfc);
	gc1084_write_register(ViPipe, 0x0d19, 0x31);
	gc1084_write_register(ViPipe, 0x0d20, 0x40);
	gc1084_write_register(ViPipe, 0x0d25, 0xcb);
	gc1084_write_register(ViPipe, 0x0d27, 0x03);
	gc1084_write_register(ViPipe, 0x0d29, 0x40);
	gc1084_write_register(ViPipe, 0x0d43, 0x20);
	gc1084_write_register(ViPipe, 0x0058, 0x60);
	gc1084_write_register(ViPipe, 0x00d6, 0x66);
	gc1084_write_register(ViPipe, 0x00d7, 0x19);
	gc1084_write_register(ViPipe, 0x0093, 0x02);
	gc1084_write_register(ViPipe, 0x00d9, 0x14);
	gc1084_write_register(ViPipe, 0x00da, 0xc1);
	gc1084_write_register(ViPipe, 0x0d2a, 0x00);
	gc1084_write_register(ViPipe, 0x0d28, 0x04);
	gc1084_write_register(ViPipe, 0x0dc2, 0x84);
	gc1084_write_register(ViPipe, 0x0050, 0x30);
	gc1084_write_register(ViPipe, 0x0080, 0x07);
	gc1084_write_register(ViPipe, 0x008c, 0x05);
	gc1084_write_register(ViPipe, 0x008d, 0xa8);
	gc1084_write_register(ViPipe, 0x0077, 0x01);
	gc1084_write_register(ViPipe, 0x0078, 0xee);
	gc1084_write_register(ViPipe, 0x0079, 0x02);
	gc1084_write_register(ViPipe, 0x0067, 0xc0);
	gc1084_write_register(ViPipe, 0x0054, 0xff);
	gc1084_write_register(ViPipe, 0x0055, 0x02);
	gc1084_write_register(ViPipe, 0x0056, 0x00);
	gc1084_write_register(ViPipe, 0x0057, 0x04);
	gc1084_write_register(ViPipe, 0x005a, 0xff);
	gc1084_write_register(ViPipe, 0x005b, 0x07);
	gc1084_write_register(ViPipe, 0x00d5, 0x03);
	gc1084_write_register(ViPipe, 0x0102, 0xa9);
	gc1084_write_register(ViPipe, 0x0d03, 0x02);
	gc1084_write_register(ViPipe, 0x0d04, 0xd0);
	gc1084_write_register(ViPipe, 0x007a, 0x60);
	gc1084_write_register(ViPipe, 0x04e0, 0xff);
	gc1084_write_register(ViPipe, 0x0414, 0x75);
	gc1084_write_register(ViPipe, 0x0415, 0x75);
	gc1084_write_register(ViPipe, 0x0416, 0x75);
	gc1084_write_register(ViPipe, 0x0417, 0x75);
	gc1084_write_register(ViPipe, 0x0122, 0x00);
	gc1084_write_register(ViPipe, 0x0121, 0x80);
	gc1084_write_register(ViPipe, 0x0428, 0x10);
	gc1084_write_register(ViPipe, 0x0429, 0x10);
	gc1084_write_register(ViPipe, 0x042a, 0x10);
	gc1084_write_register(ViPipe, 0x042b, 0x10);
	gc1084_write_register(ViPipe, 0x042c, 0x14);
	gc1084_write_register(ViPipe, 0x042d, 0x14);
	gc1084_write_register(ViPipe, 0x042e, 0x18);
	gc1084_write_register(ViPipe, 0x042f, 0x18);
	gc1084_write_register(ViPipe, 0x0430, 0x05);
	gc1084_write_register(ViPipe, 0x0431, 0x05);
	gc1084_write_register(ViPipe, 0x0432, 0x05);
	gc1084_write_register(ViPipe, 0x0433, 0x05);
	gc1084_write_register(ViPipe, 0x0434, 0x05);
	gc1084_write_register(ViPipe, 0x0435, 0x05);
	gc1084_write_register(ViPipe, 0x0436, 0x05);
	gc1084_write_register(ViPipe, 0x0437, 0x05);
	gc1084_write_register(ViPipe, 0x0153, 0x00);
	gc1084_write_register(ViPipe, 0x0190, 0x01);
	gc1084_write_register(ViPipe, 0x0192, 0x02);
	gc1084_write_register(ViPipe, 0x0194, 0x04);
	gc1084_write_register(ViPipe, 0x0195, 0x02);
	gc1084_write_register(ViPipe, 0x0196, 0xd0);
	gc1084_write_register(ViPipe, 0x0197, 0x05);
	gc1084_write_register(ViPipe, 0x0198, 0x00);
	gc1084_write_register(ViPipe, 0x0201, 0x23);
	gc1084_write_register(ViPipe, 0x0202, 0x53);
	gc1084_write_register(ViPipe, 0x0203, 0xce);
	gc1084_write_register(ViPipe, 0x0208, 0x39);
	gc1084_write_register(ViPipe, 0x0212, 0x06);
	gc1084_write_register(ViPipe, 0x0213, 0x40);
	gc1084_write_register(ViPipe, 0x0215, 0x12);
	gc1084_write_register(ViPipe, 0x0229, 0x05);
	gc1084_write_register(ViPipe, 0x023e, 0x98);
	gc1084_write_register(ViPipe, 0x031e, 0x3e);

	gc1084_default_reg_init(ViPipe);
	delay_ms(40);
	printf("ViPipe:%d,===GC1084 720P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

