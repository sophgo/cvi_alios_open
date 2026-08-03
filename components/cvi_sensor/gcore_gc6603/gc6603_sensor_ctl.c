#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"

#include "gc6603_cmos_ex.h"

#define GC6603_CHIP_ID_ADDR_H	0x03f2
#define GC6603_CHIP_ID_ADDR_L	0x03f3
#define GC6603_CHIP_ID		0x5623

static void gc6603_linear_2l_4mp15_init(VI_PIPE ViPipe);
static void gc6603_linear_2l_5mp15_init(VI_PIPE ViPipe);

CVI_U8 gc6603_i2c_addr = 0x31;
const CVI_U32 gc6603_addr_byte = 2;
const CVI_U32 gc6603_data_byte = 1;
// static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int gc6603_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc6603_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc6603_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc6603_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int gc6603_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc6603_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc6603_i2c_addr, (CVI_U32)addr, gc6603_addr_byte, gc6603_data_byte);
}

int gc6603_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc6603_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc6603_i2c_addr, (CVI_U32)addr, gc6603_addr_byte,
		(CVI_U32)data, gc6603_data_byte);
}

static void delay_ms(int ms)
{
	udelay(ms * 1000);
}

void gc6603_standby(VI_PIPE ViPipe)
{

	printf("gc6603_standby\n");
}

void gc6603_restart(VI_PIPE ViPipe)
{

	printf("gc6603_restart\n");
}

void gc6603_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc6603[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc6603_write_register(ViPipe,
				g_pastGc6603[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc6603[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		// printf("reg init addr = %x, data = %x\n", g_pastGc6603[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr, g_pastGc6603[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int gc6603_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (gc6603_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc6603_read_register(ViPipe, GC6603_CHIP_ID_ADDR_H);
	nVal2 = gc6603_read_register(ViPipe, GC6603_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC6603_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc6603_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode;
	u8ImgMode = g_pastGc6603[ViPipe]->u8ImgMode;

	gc6603_i2c_init(ViPipe);

	if (u8ImgMode == GC6603_MODE_2L_2048X2048P15){
		gc6603_linear_2l_4mp15_init(ViPipe);
	}else if (u8ImgMode == GC6603_MODE_2L_2592x1944P15) {
		gc6603_linear_2l_5mp15_init(ViPipe);
	} else {
		gc6603_linear_2l_4mp15_init(ViPipe);
	}

	g_pastGc6603[ViPipe]->bInit = CVI_TRUE;
}

void gc6603_exit(VI_PIPE ViPipe)
{
	gc6603_i2c_exit(ViPipe);
}

static void gc6603_linear_2l_4mp15_init(VI_PIPE ViPipe)
{
	gc6603_write_register(ViPipe, 0x03fe, 0xf0);
	gc6603_write_register(ViPipe, 0x03fe, 0x00);
	gc6603_write_register(ViPipe, 0x03fe, 0x10);
	gc6603_write_register(ViPipe, 0x0938, 0x01);
	gc6603_write_register(ViPipe, 0x0360, 0xfd);
	gc6603_write_register(ViPipe, 0x091b, 0x1a);
	gc6603_write_register(ViPipe, 0x091c, 0x28); //18
	gc6603_write_register(ViPipe, 0x091e, 0x00);
	gc6603_write_register(ViPipe, 0x091d, 0x06);
	gc6603_write_register(ViPipe, 0x091f, 0x81);
	gc6603_write_register(ViPipe, 0x0920, 0xa1);
	gc6603_write_register(ViPipe, 0x0922, 0x3a);
	gc6603_write_register(ViPipe, 0x0923, 0x10);
	gc6603_write_register(ViPipe, 0x0928, 0x00);
	gc6603_write_register(ViPipe, 0x0934, 0xb7);
	gc6603_write_register(ViPipe, 0x0935, 0x06);
	gc6603_write_register(ViPipe, 0x0936, 0x00);
	gc6603_write_register(ViPipe, 0x0937, 0x81);
	gc6603_write_register(ViPipe, 0x031b, 0x00);
	gc6603_write_register(ViPipe, 0x031c, 0x4f);
	gc6603_write_register(ViPipe, 0x031e, 0x00);
	gc6603_write_register(ViPipe, 0x03e0, 0x00);
	gc6603_write_register(ViPipe, 0x0314, 0x10);
	gc6603_write_register(ViPipe, 0x0219, 0x47);
	gc6603_write_register(ViPipe, 0x022b, 0x10);
	gc6603_write_register(ViPipe, 0x0259, 0x08);
	gc6603_write_register(ViPipe, 0x025a, 0x44);
	gc6603_write_register(ViPipe, 0x025b, 0x10);
	gc6603_write_register(ViPipe, 0x0340, 0x10);
	gc6603_write_register(ViPipe, 0x0341, 0xcc);
	gc6603_write_register(ViPipe, 0x0342, 0x02); //03
	gc6603_write_register(ViPipe, 0x0343, 0x58); //e8
	gc6603_write_register(ViPipe, 0x0346, 0x00);
	gc6603_write_register(ViPipe, 0x0347, 0x40);
	gc6603_write_register(ViPipe, 0x0348, 0x08);
	gc6603_write_register(ViPipe, 0x0349, 0x20);
	gc6603_write_register(ViPipe, 0x034a, 0x08);
	gc6603_write_register(ViPipe, 0x034b, 0x20);
	gc6603_write_register(ViPipe, 0x070c, 0x9f);
	gc6603_write_register(ViPipe, 0x070d, 0x00);
	gc6603_write_register(ViPipe, 0x070e, 0x5c);
	gc6603_write_register(ViPipe, 0x070f, 0x09);
	gc6603_write_register(ViPipe, 0x0053, 0x05);
	gc6603_write_register(ViPipe, 0x0099, 0x00);
	gc6603_write_register(ViPipe, 0x009b, 0x20);
	gc6603_write_register(ViPipe, 0x0094, 0x08);
	gc6603_write_register(ViPipe, 0x0095, 0x00);
	gc6603_write_register(ViPipe, 0x0096, 0x08);
	gc6603_write_register(ViPipe, 0x0097, 0x00);
	gc6603_write_register(ViPipe, 0x0e4c, 0x3e); //3c
	gc6603_write_register(ViPipe, 0x0902, 0x0b);
	gc6603_write_register(ViPipe, 0x0903, 0x15);
	gc6603_write_register(ViPipe, 0x0904, 0x14);
	gc6603_write_register(ViPipe, 0x0907, 0x14);
	gc6603_write_register(ViPipe, 0x0908, 0x15);
	gc6603_write_register(ViPipe, 0x090e, 0x26);
	gc6603_write_register(ViPipe, 0x090f, 0x15);
	gc6603_write_register(ViPipe, 0x0244, 0x76);
	gc6603_write_register(ViPipe, 0x0724, 0x0c);
	gc6603_write_register(ViPipe, 0x0727, 0x0c);
	gc6603_write_register(ViPipe, 0x072a, 0x18);
	gc6603_write_register(ViPipe, 0x072b, 0x19);
	gc6603_write_register(ViPipe, 0x0709, 0x40);
	gc6603_write_register(ViPipe, 0x0719, 0x40);
	gc6603_write_register(ViPipe, 0x0912, 0x03); //01
	gc6603_write_register(ViPipe, 0x0913, 0x00);
	gc6603_write_register(ViPipe, 0x0e66, 0x10);
	gc6603_write_register(ViPipe, 0x0e69, 0x80);
	gc6603_write_register(ViPipe, 0x0e6a, 0xc0);
	gc6603_write_register(ViPipe, 0x0e6b, 0x02);
	gc6603_write_register(ViPipe, 0x0223, 0x00);
	gc6603_write_register(ViPipe, 0x0e81, 0x02);
	gc6603_write_register(ViPipe, 0x0e30, 0x00);
	gc6603_write_register(ViPipe, 0x0e33, 0x80);
	gc6603_write_register(ViPipe, 0x0242, 0x35); //65
	gc6603_write_register(ViPipe, 0x0243, 0x08);
	gc6603_write_register(ViPipe, 0x0361, 0xbc);
	gc6603_write_register(ViPipe, 0x0362, 0x0f);
	gc6603_write_register(ViPipe, 0x0e34, 0x04);
	gc6603_write_register(ViPipe, 0x0e47, 0x55);
	gc6603_write_register(ViPipe, 0x0e61, 0x0d);
	gc6603_write_register(ViPipe, 0x0e62, 0x0d);
	gc6603_write_register(ViPipe, 0x023a, 0x05);
	gc6603_write_register(ViPipe, 0x0e64, 0x0c);
	gc6603_write_register(ViPipe, 0x0e20, 0x0c);
	gc6603_write_register(ViPipe, 0x0e6e, 0x20); //50
	gc6603_write_register(ViPipe, 0x0e6f, 0x28); //58
	gc6603_write_register(ViPipe, 0x0e70, 0x18); //24
	gc6603_write_register(ViPipe, 0x0e71, 0x28);
	gc6603_write_register(ViPipe, 0x0e28, 0x78); //38
	gc6603_write_register(ViPipe, 0x0e4d, 0x80);
	gc6603_write_register(ViPipe, 0x0245, 0x08);
	gc6603_write_register(ViPipe, 0x0240, 0x06);
	gc6603_write_register(ViPipe, 0x0e63, 0x06);
	gc6603_write_register(ViPipe, 0x0236, 0x02);
	gc6603_write_register(ViPipe, 0x0261, 0x60);
	gc6603_write_register(ViPipe, 0x0262, 0x28);
	gc6603_write_register(ViPipe, 0x0072, 0x00);
	gc6603_write_register(ViPipe, 0x0074, 0x01);
	gc6603_write_register(ViPipe, 0x0087, 0x53);
	gc6603_write_register(ViPipe, 0x0704, 0x07);
	gc6603_write_register(ViPipe, 0x0705, 0x28);
	gc6603_write_register(ViPipe, 0x0706, 0x02);
	gc6603_write_register(ViPipe, 0x0715, 0x28);
	gc6603_write_register(ViPipe, 0x0716, 0x02);
	gc6603_write_register(ViPipe, 0x0708, 0xc0);
	gc6603_write_register(ViPipe, 0x0718, 0xc0);
	gc6603_write_register(ViPipe, 0x0076, 0x01);
	gc6603_write_register(ViPipe, 0x021a, 0x10);
	gc6603_write_register(ViPipe, 0x0052, 0x02);
	gc6603_write_register(ViPipe, 0x0448, 0x06);
	gc6603_write_register(ViPipe, 0x0449, 0x04);
	gc6603_write_register(ViPipe, 0x044a, 0x04);
	gc6603_write_register(ViPipe, 0x044b, 0x06);
	gc6603_write_register(ViPipe, 0x044c, 0x78);
	gc6603_write_register(ViPipe, 0x044d, 0x7a);
	gc6603_write_register(ViPipe, 0x044e, 0x7a);
	gc6603_write_register(ViPipe, 0x044f, 0x78);
	gc6603_write_register(ViPipe, 0x0046, 0x30);
	gc6603_write_register(ViPipe, 0x0002, 0xa9);
	gc6603_write_register(ViPipe, 0x0005, 0x83);
	gc6603_write_register(ViPipe, 0x0006, 0x83);
	gc6603_write_register(ViPipe, 0x001a, 0x83);
	gc6603_write_register(ViPipe, 0x0075, 0x65);
	gc6603_write_register(ViPipe, 0x0202, 0x01);
	gc6603_write_register(ViPipe, 0x0203, 0xe0);
	gc6603_write_register(ViPipe, 0x0914, 0x03); //01
	gc6603_write_register(ViPipe, 0x0915, 0x00);
	gc6603_write_register(ViPipe, 0x0225, 0x00);
	gc6603_write_register(ViPipe, 0x0e67, 0x0d); //0f
	gc6603_write_register(ViPipe, 0x0e68, 0x0d); //0f
	gc6603_write_register(ViPipe, 0x0089, 0x03);
	gc6603_write_register(ViPipe, 0x0144, 0x00);
	gc6603_write_register(ViPipe, 0x0122, 0x08);
	gc6603_write_register(ViPipe, 0x0123, 0x27);
	gc6603_write_register(ViPipe, 0x0126, 0x0a);
	gc6603_write_register(ViPipe, 0x0129, 0x08);
	gc6603_write_register(ViPipe, 0x012a, 0x0d);
	gc6603_write_register(ViPipe, 0x012b, 0x0a);
	gc6603_write_register(ViPipe, 0x0180, 0x46);
	gc6603_write_register(ViPipe, 0x0181, 0x30);
	gc6603_write_register(ViPipe, 0x0185, 0x01);
	gc6603_write_register(ViPipe, 0x0106, 0x38);
	gc6603_write_register(ViPipe, 0x010d, 0x0a);
	gc6603_write_register(ViPipe, 0x010e, 0x00);
	gc6603_write_register(ViPipe, 0x0111, 0x2b);
	gc6603_write_register(ViPipe, 0x0112, 0x0a);
	gc6603_write_register(ViPipe, 0x0113, 0x0a);
	gc6603_write_register(ViPipe, 0x0114, 0x01);
	gc6603_write_register(ViPipe, 0x0100, 0x09);
	gc6603_write_register(ViPipe, 0x0221, 0x05);
	gc6603_write_register(ViPipe, 0x023b, 0x13);
	gc6603_write_register(ViPipe, 0x0352, 0x70);
	gc6603_write_register(ViPipe, 0x0357, 0x00);
	gc6603_write_register(ViPipe, 0x0b00, 0x40);
	gc6603_write_register(ViPipe, 0x08ef, 0x01);
	gc6603_write_register(ViPipe, 0x03fe, 0x00);
	gc6603_write_register(ViPipe, 0x031f, 0x01);
	gc6603_write_register(ViPipe, 0x031f, 0x00);
	gc6603_write_register(ViPipe, 0x0318, 0x0e);
	gc6603_write_register(ViPipe, 0x0a67, 0x80);
	gc6603_write_register(ViPipe, 0x0a50, 0x41);
	gc6603_write_register(ViPipe, 0x0a51, 0x41);
	gc6603_write_register(ViPipe, 0x0a52, 0x41);
	gc6603_write_register(ViPipe, 0x0a54, 0x26);
	gc6603_write_register(ViPipe, 0x0a55, 0x26);
	gc6603_write_register(ViPipe, 0x0a4e, 0x0c);
	gc6603_write_register(ViPipe, 0x0a4f, 0x0c);
	gc6603_write_register(ViPipe, 0x0a65, 0x17);
	gc6603_write_register(ViPipe, 0x0a53, 0x00);
	gc6603_write_register(ViPipe, 0x0a98, 0x04);
	gc6603_write_register(ViPipe, 0x05be, 0x00);
	gc6603_write_register(ViPipe, 0x05a9, 0x01);
	gc6603_write_register(ViPipe, 0x0a67, 0x80);
	gc6603_write_register(ViPipe, 0x0a88, 0x20);
	gc6603_write_register(ViPipe, 0x0a89, 0x08);
	gc6603_write_register(ViPipe, 0x0a8a, 0x90);
	gc6603_write_register(ViPipe, 0x0a8b, 0x0a);
	gc6603_write_register(ViPipe, 0x0028, 0x0a);
	gc6603_write_register(ViPipe, 0x0029, 0x90);
	gc6603_write_register(ViPipe, 0x002a, 0x08);
	gc6603_write_register(ViPipe, 0x002b, 0x20);
	gc6603_write_register(ViPipe, 0x0023, 0x00);
	gc6603_write_register(ViPipe, 0x0024, 0x01);
	gc6603_write_register(ViPipe, 0x0025, 0x38);
	gc6603_write_register(ViPipe, 0x0a70, 0x07);
	gc6603_write_register(ViPipe, 0x0a73, 0xe0);
	gc6603_write_register(ViPipe, 0x0a80, 0x7b);
	gc6603_write_register(ViPipe, 0x0a82, 0x00);
	gc6603_write_register(ViPipe, 0x0a83, 0x80);
	gc6603_write_register(ViPipe, 0x0a5a, 0x80);
	delay_ms(20);
	gc6603_write_register(ViPipe, 0x05be, 0x01);
	gc6603_write_register(ViPipe, 0x0a70, 0x00);
	gc6603_write_register(ViPipe, 0x0080, 0x02);
	gc6603_write_register(ViPipe, 0x0021, 0x40);
	gc6603_write_register(ViPipe, 0x0a67, 0x00);

	gc6603_default_reg_init(ViPipe);

	printf("ViPipe:%d,===GC6603 2L 4M 15fps 10bit LINEAR Init OK!===\n", ViPipe);
}


static void gc6603_linear_2l_5mp15_init(VI_PIPE ViPipe)
{

	gc6603_write_register(ViPipe, 0x03fe, 0xf0);
	gc6603_write_register(ViPipe, 0x03fe, 0x00);
	gc6603_write_register(ViPipe, 0x03fe, 0x10);
	gc6603_write_register(ViPipe, 0x0938, 0x01);
	gc6603_write_register(ViPipe, 0x0360, 0xfd);
	gc6603_write_register(ViPipe, 0x091b, 0x1a);
	gc6603_write_register(ViPipe, 0x091c, 0x28);
	gc6603_write_register(ViPipe, 0x091e, 0x00);
	gc6603_write_register(ViPipe, 0x091d, 0x06);
	gc6603_write_register(ViPipe, 0x091f, 0x81);
	gc6603_write_register(ViPipe, 0x0920, 0xa1);
	gc6603_write_register(ViPipe, 0x0922, 0x3a);
	gc6603_write_register(ViPipe, 0x0923, 0x10);
	gc6603_write_register(ViPipe, 0x0928, 0x00);
	gc6603_write_register(ViPipe, 0x0934, 0xb7);
	gc6603_write_register(ViPipe, 0x0935, 0x06);
	gc6603_write_register(ViPipe, 0x0936, 0x00);
	gc6603_write_register(ViPipe, 0x0937, 0x81);
	gc6603_write_register(ViPipe, 0x031b, 0x00);
	gc6603_write_register(ViPipe, 0x031c, 0x4f);
	gc6603_write_register(ViPipe, 0x031e, 0x00);
	gc6603_write_register(ViPipe, 0x03e0, 0x00);
	gc6603_write_register(ViPipe, 0x0314, 0x10);
	gc6603_write_register(ViPipe, 0x0219, 0x47);
	gc6603_write_register(ViPipe, 0x022b, 0x10);
	gc6603_write_register(ViPipe, 0x0259, 0x08);
	gc6603_write_register(ViPipe, 0x025a, 0x44);
	gc6603_write_register(ViPipe, 0x025b, 0x10);
	gc6603_write_register(ViPipe, 0x0340, 0x10);
	gc6603_write_register(ViPipe, 0x0341, 0xcc);
	gc6603_write_register(ViPipe, 0x0342, 0x02);
	gc6603_write_register(ViPipe, 0x0343, 0x58);
	gc6603_write_register(ViPipe, 0x0346, 0x00);
	gc6603_write_register(ViPipe, 0x0347, 0x40);
	gc6603_write_register(ViPipe, 0x0348, 0x0a);
	gc6603_write_register(ViPipe, 0x0349, 0x90);
	gc6603_write_register(ViPipe, 0x034a, 0x08);
	gc6603_write_register(ViPipe, 0x034b, 0x20);
	gc6603_write_register(ViPipe, 0x034e, 0x0a);
	gc6603_write_register(ViPipe, 0x034f, 0xc0);
	gc6603_write_register(ViPipe, 0x070c, 0x03);
	gc6603_write_register(ViPipe, 0x070d, 0x00);
	gc6603_write_register(ViPipe, 0x070e, 0x94);
	gc6603_write_register(ViPipe, 0x070f, 0x0a);
	gc6603_write_register(ViPipe, 0x0053, 0x05);
	gc6603_write_register(ViPipe, 0x0099, 0x34);
	gc6603_write_register(ViPipe, 0x009b, 0x30);
	gc6603_write_register(ViPipe, 0x0094, 0x0a);
	gc6603_write_register(ViPipe, 0x0095, 0x20); //2592
	gc6603_write_register(ViPipe, 0x0096, 0x07);
	gc6603_write_register(ViPipe, 0x0097, 0x98); //1944
	gc6603_write_register(ViPipe, 0x0e4c, 0x3e);
	gc6603_write_register(ViPipe, 0x0902, 0x0b);
	gc6603_write_register(ViPipe, 0x0903, 0x15);
	gc6603_write_register(ViPipe, 0x0904, 0x14);
	gc6603_write_register(ViPipe, 0x0907, 0x14);
	gc6603_write_register(ViPipe, 0x0908, 0x15);
	gc6603_write_register(ViPipe, 0x090e, 0x26);
	gc6603_write_register(ViPipe, 0x090f, 0x15);
	gc6603_write_register(ViPipe, 0x0244, 0x75);
	gc6603_write_register(ViPipe, 0x0724, 0x0c);
	gc6603_write_register(ViPipe, 0x0727, 0x0c);
	gc6603_write_register(ViPipe, 0x072a, 0x18);
	gc6603_write_register(ViPipe, 0x072b, 0x19);
	gc6603_write_register(ViPipe, 0x0709, 0x40);
	gc6603_write_register(ViPipe, 0x0719, 0x40);
	gc6603_write_register(ViPipe, 0x0912, 0x03);
	gc6603_write_register(ViPipe, 0x0913, 0x00);
	gc6603_write_register(ViPipe, 0x0e66, 0x10);
	gc6603_write_register(ViPipe, 0x0e69, 0x80);
	gc6603_write_register(ViPipe, 0x0e6a, 0xc0);
	gc6603_write_register(ViPipe, 0x0e6b, 0x02);
	gc6603_write_register(ViPipe, 0x0223, 0x00);
	gc6603_write_register(ViPipe, 0x0e81, 0x02);
	gc6603_write_register(ViPipe, 0x0e30, 0x00);
	gc6603_write_register(ViPipe, 0x0e33, 0x80);
	gc6603_write_register(ViPipe, 0x0242, 0x35);
	gc6603_write_register(ViPipe, 0x0243, 0x08);
	gc6603_write_register(ViPipe, 0x0361, 0xbc);
	gc6603_write_register(ViPipe, 0x0362, 0x0f);
	gc6603_write_register(ViPipe, 0x0e34, 0x04);
	gc6603_write_register(ViPipe, 0x0e47, 0x55);
	gc6603_write_register(ViPipe, 0x0e61, 0x0d);
	gc6603_write_register(ViPipe, 0x0e62, 0x0d);
	gc6603_write_register(ViPipe, 0x023a, 0xc0);
	gc6603_write_register(ViPipe, 0x0e64, 0x0c);
	gc6603_write_register(ViPipe, 0x0e20, 0x0c);
	gc6603_write_register(ViPipe, 0x0e6e, 0x20);
	gc6603_write_register(ViPipe, 0x0e6f, 0x22);
	gc6603_write_register(ViPipe, 0x0e70, 0x14);
	gc6603_write_register(ViPipe, 0x0e71, 0x28);
	gc6603_write_register(ViPipe, 0x0e28, 0x78);
	gc6603_write_register(ViPipe, 0x0e4d, 0x80);
	gc6603_write_register(ViPipe, 0x0245, 0x08);
	gc6603_write_register(ViPipe, 0x0240, 0x06);
	gc6603_write_register(ViPipe, 0x0e63, 0x06);
	gc6603_write_register(ViPipe, 0x0236, 0x02);
	gc6603_write_register(ViPipe, 0x0261, 0x60);
	gc6603_write_register(ViPipe, 0x0262, 0x28);
	gc6603_write_register(ViPipe, 0x0072, 0x00);
	gc6603_write_register(ViPipe, 0x0074, 0x01);
	gc6603_write_register(ViPipe, 0x0087, 0x53);
	gc6603_write_register(ViPipe, 0x0704, 0x07);
	gc6603_write_register(ViPipe, 0x0705, 0x28);
	gc6603_write_register(ViPipe, 0x0706, 0x02);
	gc6603_write_register(ViPipe, 0x0715, 0x28);
	gc6603_write_register(ViPipe, 0x0716, 0x02);
	gc6603_write_register(ViPipe, 0x0708, 0xc0);
	gc6603_write_register(ViPipe, 0x0718, 0xc0);
	gc6603_write_register(ViPipe, 0x0076, 0x01);
	gc6603_write_register(ViPipe, 0x021a, 0x10);
	gc6603_write_register(ViPipe, 0x0052, 0x02);
	gc6603_write_register(ViPipe, 0x0448, 0x06);
	gc6603_write_register(ViPipe, 0x0449, 0x04);
	gc6603_write_register(ViPipe, 0x044a, 0x04);
	gc6603_write_register(ViPipe, 0x044b, 0x06);
	gc6603_write_register(ViPipe, 0x044c, 0x78);
	gc6603_write_register(ViPipe, 0x044d, 0x7a);
	gc6603_write_register(ViPipe, 0x044e, 0x7a);
	gc6603_write_register(ViPipe, 0x044f, 0x78);
	gc6603_write_register(ViPipe, 0x0046, 0x30);
	gc6603_write_register(ViPipe, 0x0002, 0xa9);
	gc6603_write_register(ViPipe, 0x0005, 0x83);
	gc6603_write_register(ViPipe, 0x0006, 0x83);
	gc6603_write_register(ViPipe, 0x001a, 0x83);
	gc6603_write_register(ViPipe, 0x0075, 0x65);
	gc6603_write_register(ViPipe, 0x0202, 0x08);
	gc6603_write_register(ViPipe, 0x0203, 0x46);
	gc6603_write_register(ViPipe, 0x0914, 0x03);
	gc6603_write_register(ViPipe, 0x0915, 0x00);
	gc6603_write_register(ViPipe, 0x0225, 0x00);
	gc6603_write_register(ViPipe, 0x0e67, 0x0d);
	gc6603_write_register(ViPipe, 0x0e68, 0x0d);
	gc6603_write_register(ViPipe, 0x0089, 0x03);
	gc6603_write_register(ViPipe, 0x0144, 0x00);
	gc6603_write_register(ViPipe, 0x0122, 0x08);
	gc6603_write_register(ViPipe, 0x0123, 0x27);
	gc6603_write_register(ViPipe, 0x0126, 0x0a);
	gc6603_write_register(ViPipe, 0x0129, 0x08);
	gc6603_write_register(ViPipe, 0x012a, 0x0d);
	gc6603_write_register(ViPipe, 0x012b, 0x0a);
	gc6603_write_register(ViPipe, 0x0180, 0x46);
	gc6603_write_register(ViPipe, 0x0181, 0x30);
	gc6603_write_register(ViPipe, 0x0185, 0x01);
	gc6603_write_register(ViPipe, 0x0106, 0x38);
	gc6603_write_register(ViPipe, 0x010d, 0x0c);
	gc6603_write_register(ViPipe, 0x010e, 0xa8);
	gc6603_write_register(ViPipe, 0x0111, 0x2b);
	gc6603_write_register(ViPipe, 0x0112, 0x0a);
	gc6603_write_register(ViPipe, 0x0113, 0x0a);
	gc6603_write_register(ViPipe, 0x0114, 0x01);
	gc6603_write_register(ViPipe, 0x0100, 0x09);
	gc6603_write_register(ViPipe, 0x0221, 0x05);
	gc6603_write_register(ViPipe, 0x023b, 0x13);
	gc6603_write_register(ViPipe, 0x0352, 0x70);
	gc6603_write_register(ViPipe, 0x0357, 0x00);
	gc6603_write_register(ViPipe, 0x0b00, 0x40);
	gc6603_write_register(ViPipe, 0x08ef, 0x01);
	gc6603_write_register(ViPipe, 0x03fe, 0x00);
	gc6603_write_register(ViPipe, 0x031f, 0x01);
	gc6603_write_register(ViPipe, 0x031f, 0x00);
	gc6603_write_register(ViPipe, 0x0318, 0x0e);
	gc6603_write_register(ViPipe, 0x0a67, 0x80);
	gc6603_write_register(ViPipe, 0x0a50, 0x41);
	gc6603_write_register(ViPipe, 0x0a51, 0x41);
	gc6603_write_register(ViPipe, 0x0a52, 0x41);
	gc6603_write_register(ViPipe, 0x0a54, 0x26);
	gc6603_write_register(ViPipe, 0x0a55, 0x26);
	gc6603_write_register(ViPipe, 0x0a4e, 0x0c);
	gc6603_write_register(ViPipe, 0x0a4f, 0x0c);
	gc6603_write_register(ViPipe, 0x0a65, 0x17);
	gc6603_write_register(ViPipe, 0x0a53, 0x00);
	gc6603_write_register(ViPipe, 0x0a98, 0x04);
	gc6603_write_register(ViPipe, 0x05be, 0x00);
	gc6603_write_register(ViPipe, 0x05a9, 0x01);
	gc6603_write_register(ViPipe, 0x0a67, 0x80);
	gc6603_write_register(ViPipe, 0x0023, 0x00);
	gc6603_write_register(ViPipe, 0x0025, 0x00);
	gc6603_write_register(ViPipe, 0x0028, 0x0a);
	gc6603_write_register(ViPipe, 0x0029, 0x90);
	gc6603_write_register(ViPipe, 0x002a, 0x08);
	gc6603_write_register(ViPipe, 0x002b, 0x20);
	gc6603_write_register(ViPipe, 0x0a8b, 0x0a);
	gc6603_write_register(ViPipe, 0x0a8a, 0x90);
	gc6603_write_register(ViPipe, 0x0a89, 0x08);
	gc6603_write_register(ViPipe, 0x0a88, 0x20);
	gc6603_write_register(ViPipe, 0x0a70, 0x07);
	gc6603_write_register(ViPipe, 0x0a73, 0xe0);
	gc6603_write_register(ViPipe, 0x0a80, 0x7b);
	gc6603_write_register(ViPipe, 0x0a82, 0x00);
	gc6603_write_register(ViPipe, 0x0a83, 0x80);
	gc6603_write_register(ViPipe, 0x0a5a, 0x80);
	delay_ms(20);
	gc6603_write_register(ViPipe, 0x05be, 0x01);
	gc6603_write_register(ViPipe, 0x0a70, 0x00);
	gc6603_write_register(ViPipe, 0x0080, 0x02);
	gc6603_write_register(ViPipe, 0x0021, 0x40);
	gc6603_write_register(ViPipe, 0x0a67, 0x00);

	gc6603_default_reg_init(ViPipe);

	printf("ViPipe:%d,===GC6603 2L 5M 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}
