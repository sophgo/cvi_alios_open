#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "bf2257cs_cmos_ex.h"

#define BF2257CS_CHIP_ID_HI_ADDR		0xfc
#define BF2257CS_CHIP_ID_LO_ADDR		0xfd
#define BF2257CS_CHIP_ID			0x2257

static void bf2257cs_linear_1200p30_init(VI_PIPE ViPipe);

CVI_U8 bf2257cs_i2c_addr = 0x3e;        /* I2C Address of BF2257CS */
const CVI_U32 bf2257cs_addr_byte = 1;
const CVI_U32 bf2257cs_data_byte = 1;


int bf2257cs_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBF2257CS_BusInfo[ViPipe].s8I2cDev;
	printf("bf2257cs i2c init\r\n");
	return sensor_i2c_init(i2c_id);
}

int bf2257cs_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBF2257CS_BusInfo[ViPipe].s8I2cDev;
	printf("bf2257cs i2c exit\r\n");
	return sensor_i2c_exit(i2c_id);
}

int bf2257cs_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBF2257CS_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, bf2257cs_i2c_addr,
		(CVI_U32)addr, bf2257cs_addr_byte, bf2257cs_data_byte);
}

int bf2257cs_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBF2257CS_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, bf2257cs_i2c_addr, (CVI_U32)addr, bf2257cs_addr_byte,
		(CVI_U32)data, bf2257cs_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void bf2257cs_prog(VI_PIPE ViPipe, int *rom)
{
	int i = 0;

	while (1) {
		int lookup = rom[i++];
		int addr = (lookup >> 16) & 0xFFFF;
		int data = lookup & 0xFFFF;

		if (addr == 0xFFFE)
			delay_ms(data);
		else if (addr != 0xFFFF)
			bf2257cs_write_register(ViPipe, addr, data);
	}
}

void bf2257cs_standby(VI_PIPE ViPipe)
{
	bf2257cs_write_register(ViPipe, 0xf3, 0x01);
	printf("%s\r\n", __func__);
}

void bf2257cs_restart(VI_PIPE ViPipe)
{
	bf2257cs_write_register(ViPipe, 0xf3, 0x01);
	delay_ms(20);
	bf2257cs_write_register(ViPipe, 0xf3, 0x00);
}

void bf2257cs_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastBF2257CS[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if(i == LINEAR_FLIP_MIRROR_ADDR) {
			g_pastBF2257CS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data = 0x21; //flip mirror default
		}
		bf2257cs_write_register(ViPipe,
				g_pastBF2257CS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastBF2257CS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int bf2257cs_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(4 * 1000);
	if (bf2257cs_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = bf2257cs_read_register(ViPipe, BF2257CS_CHIP_ID_HI_ADDR);
	nVal2 = bf2257cs_read_register(ViPipe, BF2257CS_CHIP_ID_LO_ADDR);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != BF2257CS_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void bf2257cs_init(VI_PIPE ViPipe)
{
	bf2257cs_linear_1200p30_init(ViPipe);

	g_pastBF2257CS[ViPipe]->bInit = CVI_TRUE;
}

void bf2257cs_exit(VI_PIPE ViPipe)
{
	bf2257cs_i2c_exit(ViPipe);
}

/* 1200P10 and 1200P25 */
static void bf2257cs_linear_1200p30_init(VI_PIPE ViPipe)
{
	printf("sensor init settings\n");
	bf2257cs_write_register(ViPipe, 0xf2, 0x01);
	bf2257cs_write_register(ViPipe, 0x00, 0x01);
	bf2257cs_write_register(ViPipe, 0x02, 0xb7);
	bf2257cs_write_register(ViPipe, 0x1e, 0x04);
	bf2257cs_write_register(ViPipe, 0x22, 0x2a);
	bf2257cs_write_register(ViPipe, 0x23, 0x3b);
	bf2257cs_write_register(ViPipe, 0x24, 0x48);
	bf2257cs_write_register(ViPipe, 0x25, 0xa1);
	bf2257cs_write_register(ViPipe, 0x2b, 0x7d);
	bf2257cs_write_register(ViPipe, 0x2c, 0x7e);
	bf2257cs_write_register(ViPipe, 0x34, 0x1e);
	bf2257cs_write_register(ViPipe, 0x18, 0x3c);
	bf2257cs_write_register(ViPipe, 0x27, 0x3c);
	bf2257cs_write_register(ViPipe, 0x14, 0x79);
	bf2257cs_write_register(ViPipe, 0x15, 0x7f);
	bf2257cs_write_register(ViPipe, 0xe0, 0x00);
	bf2257cs_write_register(ViPipe, 0xe1, 0x01);
	bf2257cs_write_register(ViPipe, 0xe2, 0x08);
	bf2257cs_write_register(ViPipe, 0xe5, 0xe3);
	bf2257cs_write_register(ViPipe, 0xe6, 0xa0);
	bf2257cs_write_register(ViPipe, 0xe7, 0x33);
	bf2257cs_write_register(ViPipe, 0xe8, 0x12);
	bf2257cs_write_register(ViPipe, 0xe9, 0x89);
	bf2257cs_write_register(ViPipe, 0xea, 0x86);
	bf2257cs_write_register(ViPipe, 0xeb, 0x80);
	bf2257cs_write_register(ViPipe, 0xec, 0x91);
	bf2257cs_write_register(ViPipe, 0xed, 0x60);
	bf2257cs_write_register(ViPipe, 0xe3, 0x78);
	bf2257cs_write_register(ViPipe, 0xe4, 0xe0);
	bf2257cs_write_register(ViPipe, 0x06, 0x10);
	bf2257cs_write_register(ViPipe, 0x07, 0x00);
	bf2257cs_write_register(ViPipe, 0x0b, 0x80);
	bf2257cs_write_register(ViPipe, 0x0c, 0x03);
	bf2257cs_write_register(ViPipe, 0x59, 0x40);
	bf2257cs_write_register(ViPipe, 0x5a, 0x40);
	bf2257cs_write_register(ViPipe, 0x5b, 0x40);
	bf2257cs_write_register(ViPipe, 0x5c, 0x40);
	bf2257cs_write_register(ViPipe, 0x70, 0x08);
	bf2257cs_write_register(ViPipe, 0x71, 0x07);
	bf2257cs_write_register(ViPipe, 0x72, 0x12);
	bf2257cs_write_register(ViPipe, 0x73, 0x09);
	bf2257cs_write_register(ViPipe, 0x74, 0x08);
	bf2257cs_write_register(ViPipe, 0x75, 0x06);
	bf2257cs_write_register(ViPipe, 0x76, 0x20);
	bf2257cs_write_register(ViPipe, 0x77, 0x02);
	bf2257cs_write_register(ViPipe, 0x78, 0x10);
	bf2257cs_write_register(ViPipe, 0x79, 0x09);
	bf2257cs_write_register(ViPipe, 0x7a, 0x00);
	bf2257cs_write_register(ViPipe, 0x7b, 0x00);
	bf2257cs_write_register(ViPipe, 0x7c, 0x00);
	bf2257cs_write_register(ViPipe, 0x7d, 0x0f);
	bf2257cs_write_register(ViPipe, 0xca, 0x60);
	bf2257cs_write_register(ViPipe, 0xcb, 0x40);
	bf2257cs_write_register(ViPipe, 0xcc, 0x04);
	bf2257cs_write_register(ViPipe, 0xcd, 0x44);
	bf2257cs_write_register(ViPipe, 0xce, 0x04);
	bf2257cs_write_register(ViPipe, 0xcf, 0xb4);
	bf2257cs_write_register(ViPipe, 0x6a, 0xff);
	bf2257cs_write_register(ViPipe, 0x6b, 0x02);
	bf2257cs_write_register(ViPipe, 0x6c, 0xe1);
	bf2257cs_write_register(ViPipe, 0x6d, 0x0f);
	bf2257cs_write_register(ViPipe, 0x01, 0x4b);
	bf2257cs_write_register(ViPipe, 0xf3, 0x00);
	bf2257cs_write_register(ViPipe, 0xd0, 0x00);
	bf2257cs_write_register(ViPipe, 0x01, 0x43);
	bf2257cs_default_reg_init(ViPipe);

	printf("ViPipe:%d,===BF2257CS 1200P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

