#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "gc3003_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>


#define GC3003_CHIP_ID_ADDR_H   0x03f0
#define GC3003_CHIP_ID_ADDR_L   0x03f1
#define GC3003_CHIP_ID          0x3003

static void gc3003_linear_1296p30_init(VI_PIPE ViPipe);

CVI_U8 gc3003_i2c_addr = 0x37;//0x6e
const CVI_U32 gc3003_addr_byte = 2;
const CVI_U32 gc3003_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int gc3003_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc3003_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc3003_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc3003_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int gc3003_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc3003_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc3003_i2c_addr, (CVI_U32)addr, gc3003_addr_byte, gc3003_data_byte);
}


int gc3003_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc3003_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc3003_i2c_addr, (CVI_U32)addr, gc3003_addr_byte,
				(CVI_U32)data, gc3003_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void gc3003_standby(VI_PIPE ViPipe)
{
	gc3003_write_register(ViPipe, 0x031d, 0x2d);
	gc3003_write_register(ViPipe, 0x023e, 0x00);
	gc3003_write_register(ViPipe, 0x0203, 0x00);
	gc3003_write_register(ViPipe, 0x0216, 0x00);
	gc3003_write_register(ViPipe, 0x031d, 0x28);
	gc3003_write_register(ViPipe, 0x031d, 0x2e);
	gc3003_write_register(ViPipe, 0x03f7, 0x00);
	gc3003_write_register(ViPipe, 0x03fc, 0x01);
	gc3003_write_register(ViPipe, 0x03f9, 0x42);
	gc3003_write_register(ViPipe, 0x031d, 0x28);

	printf("gc3003_standby\n");
}

void gc3003_restart(VI_PIPE ViPipe)
{
	gc3003_write_register(ViPipe, 0x03f9, 0x13);
	gc3003_write_register(ViPipe, 0x03fc, 0xce);
	gc3003_write_register(ViPipe, 0x03f7, 0x01);
	gc3003_write_register(ViPipe, 0x0216, 0x29);
	gc3003_write_register(ViPipe, 0x031d, 0x2d);
	gc3003_write_register(ViPipe, 0x0203, 0x4e);
	gc3003_write_register(ViPipe, 0x023e, 0x99);
	gc3003_write_register(ViPipe, 0x031d, 0x28);

	printf("gc3003_restart\n");
}

void gc3003_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc3003[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc3003_write_register(ViPipe,
				g_pastGc3003[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc3003[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void gc3003_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 value = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		value = 0x01;
		break;
	case ISP_SNS_FLIP:
		value = 0x02;
		break;
	case ISP_SNS_MIRROR_FLIP:
		value = 0x03;
		break;
	default:
		return;
	}
	gc3003_write_register(ViPipe, 0x0015, value);
	gc3003_write_register(ViPipe, 0x0d15, value);
}

int gc3003_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (gc3003_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc3003_read_register(ViPipe, GC3003_CHIP_ID_ADDR_H);
	nVal2 = gc3003_read_register(ViPipe, GC3003_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC3003_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc3003_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastGc3003[ViPipe]->enWDRMode;

	gc3003_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
		gc3003_linear_1296p30_init(ViPipe);
	}
	g_pastGc3003[ViPipe]->bInit = CVI_TRUE;
}

void gc3003_exit(VI_PIPE ViPipe)
{
	gc3003_i2c_exit(ViPipe);
}

static void gc3003_linear_1296p30_init(VI_PIPE ViPipe)
{
	gc3003_write_register(ViPipe, 0x03fe, 0xf0);
	gc3003_write_register(ViPipe, 0x03fe, 0xf0);
	gc3003_write_register(ViPipe, 0x03fe, 0xf0);
	gc3003_write_register(ViPipe, 0x03fe, 0x00);
	gc3003_write_register(ViPipe, 0x03f3, 0x00);
	gc3003_write_register(ViPipe, 0x03f5, 0xc0);
	gc3003_write_register(ViPipe, 0x03f6, 0x06);
	gc3003_write_register(ViPipe, 0x03f7, 0x01);
	gc3003_write_register(ViPipe, 0x03f8, 0x46);
	gc3003_write_register(ViPipe, 0x03f9, 0x13);
	gc3003_write_register(ViPipe, 0x03fa, 0x00);
	gc3003_write_register(ViPipe, 0x03e0, 0x16);
	gc3003_write_register(ViPipe, 0x03e1, 0x0d);
	gc3003_write_register(ViPipe, 0x03e2, 0x30);
	gc3003_write_register(ViPipe, 0x03e4, 0x08);
	gc3003_write_register(ViPipe, 0x03fc, 0xce);
	gc3003_write_register(ViPipe, 0x0d05, 0x05);
	gc3003_write_register(ViPipe, 0x0d06, 0x40);
	gc3003_write_register(ViPipe, 0x0d76, 0x00);
	gc3003_write_register(ViPipe, 0x0d41, 0x05);
	gc3003_write_register(ViPipe, 0x0d42, 0x3c);
	gc3003_write_register(ViPipe, 0x0d0a, 0x02);
	gc3003_write_register(ViPipe, 0x000c, 0x02);
	gc3003_write_register(ViPipe, 0x0d0d, 0x05);
	gc3003_write_register(ViPipe, 0x0d0e, 0x18);
	gc3003_write_register(ViPipe, 0x000f, 0x09);
	gc3003_write_register(ViPipe, 0x0010, 0x08);
	gc3003_write_register(ViPipe, 0x0017, 0x0c);
	gc3003_write_register(ViPipe, 0x0d53, 0x12);
	gc3003_write_register(ViPipe, 0x0051, 0x03);
	gc3003_write_register(ViPipe, 0x0082, 0x01);
	gc3003_write_register(ViPipe, 0x0086, 0x20);
	gc3003_write_register(ViPipe, 0x008a, 0x01);
	gc3003_write_register(ViPipe, 0x008b, 0x1d);
	gc3003_write_register(ViPipe, 0x008c, 0x05);
	gc3003_write_register(ViPipe, 0x008d, 0xd0);
	gc3003_write_register(ViPipe, 0x0db7, 0x01);
	gc3003_write_register(ViPipe, 0x0db0, 0x05);
	gc3003_write_register(ViPipe, 0x0db1, 0x00);
	gc3003_write_register(ViPipe, 0x0db2, 0x04);
	gc3003_write_register(ViPipe, 0x0db3, 0x54);
	gc3003_write_register(ViPipe, 0x0db4, 0x00);
	gc3003_write_register(ViPipe, 0x0db5, 0x17);
	gc3003_write_register(ViPipe, 0x0db6, 0x08);
	gc3003_write_register(ViPipe, 0x0d25, 0xcb);
	gc3003_write_register(ViPipe, 0x0d4a, 0x04);
	gc3003_write_register(ViPipe, 0x00d2, 0x70);
	gc3003_write_register(ViPipe, 0x00d7, 0x19);
	gc3003_write_register(ViPipe, 0x00d9, 0x10);
	gc3003_write_register(ViPipe, 0x00da, 0xc1);
	gc3003_write_register(ViPipe, 0x0d55, 0x1b);
	gc3003_write_register(ViPipe, 0x0d92, 0x17);
	gc3003_write_register(ViPipe, 0x0dc2, 0x30);
	gc3003_write_register(ViPipe, 0x0d2a, 0x30);
	gc3003_write_register(ViPipe, 0x0d19, 0x51);
	gc3003_write_register(ViPipe, 0x0d29, 0x30);
	gc3003_write_register(ViPipe, 0x0d20, 0x30);
	gc3003_write_register(ViPipe, 0x0d72, 0x12);
	gc3003_write_register(ViPipe, 0x0d4e, 0x12);
	gc3003_write_register(ViPipe, 0x0d43, 0x20);
	gc3003_write_register(ViPipe, 0x0050, 0x0c);
	gc3003_write_register(ViPipe, 0x006e, 0x03);
	gc3003_write_register(ViPipe, 0x0153, 0x50);
	gc3003_write_register(ViPipe, 0x0192, 0x04);
	gc3003_write_register(ViPipe, 0x0194, 0x04);
	gc3003_write_register(ViPipe, 0x0195, 0x05);
	gc3003_write_register(ViPipe, 0x0196, 0x10);
	gc3003_write_register(ViPipe, 0x0197, 0x09);
	gc3003_write_register(ViPipe, 0x0198, 0x00);
	gc3003_write_register(ViPipe, 0x0077, 0x01);
	gc3003_write_register(ViPipe, 0x0078, 0x65);
	gc3003_write_register(ViPipe, 0x0079, 0x04);
	gc3003_write_register(ViPipe, 0x0067, 0xc0);
	gc3003_write_register(ViPipe, 0x0054, 0xff);
	gc3003_write_register(ViPipe, 0x0055, 0x02);
	gc3003_write_register(ViPipe, 0x0056, 0x00);
	gc3003_write_register(ViPipe, 0x0057, 0x04);
	gc3003_write_register(ViPipe, 0x005a, 0xff);
	gc3003_write_register(ViPipe, 0x005b, 0x07);
	gc3003_write_register(ViPipe, 0x00d5, 0x03);
	gc3003_write_register(ViPipe, 0x0102, 0x10);
	gc3003_write_register(ViPipe, 0x0d4a, 0x04);
	gc3003_write_register(ViPipe, 0x04e0, 0xff);
	gc3003_write_register(ViPipe, 0x031e, 0x3e);
	gc3003_write_register(ViPipe, 0x0159, 0x01);
	gc3003_write_register(ViPipe, 0x014f, 0x28);
	gc3003_write_register(ViPipe, 0x0150, 0x40);
	gc3003_write_register(ViPipe, 0x0026, 0x00);
	gc3003_write_register(ViPipe, 0x0d26, 0xa0);
	gc3003_write_register(ViPipe, 0x0414, 0x74);
	gc3003_write_register(ViPipe, 0x0415, 0x74);
	gc3003_write_register(ViPipe, 0x0416, 0x74);
	gc3003_write_register(ViPipe, 0x0417, 0x74);
	gc3003_write_register(ViPipe, 0x0155, 0x00);
	gc3003_write_register(ViPipe, 0x0170, 0x3e);
	gc3003_write_register(ViPipe, 0x0171, 0x3e);
	gc3003_write_register(ViPipe, 0x0172, 0x3e);
	gc3003_write_register(ViPipe, 0x0173, 0x3e);
	gc3003_write_register(ViPipe, 0x0428, 0x0b);
	gc3003_write_register(ViPipe, 0x0429, 0x0b);
	gc3003_write_register(ViPipe, 0x042a, 0x0b);
	gc3003_write_register(ViPipe, 0x042b, 0x0b);
	gc3003_write_register(ViPipe, 0x042c, 0x0b);
	gc3003_write_register(ViPipe, 0x042d, 0x0b);
	gc3003_write_register(ViPipe, 0x042e, 0x0b);
	gc3003_write_register(ViPipe, 0x042f, 0x0b); //b_use
	gc3003_write_register(ViPipe, 0x0430, 0x05);
	gc3003_write_register(ViPipe, 0x0431, 0x05);
	gc3003_write_register(ViPipe, 0x0432, 0x05);
	gc3003_write_register(ViPipe, 0x0433, 0x05);
	gc3003_write_register(ViPipe, 0x0434, 0x04);
	gc3003_write_register(ViPipe, 0x0435, 0x04);
	gc3003_write_register(ViPipe, 0x0436, 0x04);
	gc3003_write_register(ViPipe, 0x0437, 0x04); //a_use
	gc3003_write_register(ViPipe, 0x0438, 0x18);
	gc3003_write_register(ViPipe, 0x0439, 0x18);
	gc3003_write_register(ViPipe, 0x043a, 0x18);
	gc3003_write_register(ViPipe, 0x043b, 0x18);
	gc3003_write_register(ViPipe, 0x043c, 0x1d);
	gc3003_write_register(ViPipe, 0x043d, 0x20);
	gc3003_write_register(ViPipe, 0x043e, 0x22);
	gc3003_write_register(ViPipe, 0x043f, 0x24); //d_use
	gc3003_write_register(ViPipe, 0x0468, 0x04);
	gc3003_write_register(ViPipe, 0x0469, 0x04);
	gc3003_write_register(ViPipe, 0x046a, 0x04);
	gc3003_write_register(ViPipe, 0x046b, 0x04);
	gc3003_write_register(ViPipe, 0x046c, 0x04);
	gc3003_write_register(ViPipe, 0x046d, 0x04);
	gc3003_write_register(ViPipe, 0x046e, 0x04);
	gc3003_write_register(ViPipe, 0x046f, 0x04); //c_use
	gc3003_write_register(ViPipe, 0x0108, 0xf0);
	gc3003_write_register(ViPipe, 0x0109, 0x80);
	gc3003_write_register(ViPipe, 0x0d03, 0x05);
	gc3003_write_register(ViPipe, 0x0d04, 0x00);
	gc3003_write_register(ViPipe, 0x007a, 0x60);
	gc3003_write_register(ViPipe, 0x00d0, 0x00);
	gc3003_write_register(ViPipe, 0x0080, 0x09);
	gc3003_write_register(ViPipe, 0x0291, 0x0f);
	gc3003_write_register(ViPipe, 0x0292, 0xff);
	gc3003_write_register(ViPipe, 0x0201, 0x27);
	gc3003_write_register(ViPipe, 0x0202, 0x53);
	gc3003_write_register(ViPipe, 0x0203, 0x4e);
	gc3003_write_register(ViPipe, 0x0206, 0x03);
	gc3003_write_register(ViPipe, 0x0212, 0x0b);
	gc3003_write_register(ViPipe, 0x0213, 0x40);
	gc3003_write_register(ViPipe, 0x0215, 0x12);
	gc3003_write_register(ViPipe, 0x023e, 0x99);
	gc3003_write_register(ViPipe, 0x03fe, 0x10);
	gc3003_write_register(ViPipe, 0x0183, 0x09);
	gc3003_write_register(ViPipe, 0x0187, 0x51);
	gc3003_write_register(ViPipe, 0x0d22, 0x04);
	gc3003_write_register(ViPipe, 0x0d21, 0x3C);
	gc3003_write_register(ViPipe, 0x0d03, 0x01);
	gc3003_write_register(ViPipe, 0x0d04, 0x28);
	gc3003_write_register(ViPipe, 0x0d23, 0x0e);
	gc3003_write_register(ViPipe, 0x03fe, 0x00);

	gc3003_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===GC3003 1296P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

