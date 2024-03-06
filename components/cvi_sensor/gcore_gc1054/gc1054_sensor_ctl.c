#include <unistd.h>
#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "gc1054_cmos_ex.h"

static void gc1054_linear_720p30_init(VI_PIPE ViPipe);

const CVI_U8 gc1054_i2c_addr = 0x21;//0x42   
const CVI_U32 gc1054_addr_byte = 1;
const CVI_U32 gc1054_data_byte = 1;

int gc1054_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc1054_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc1054_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc1054_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int gc1054_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc1054_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc1054_i2c_addr, (CVI_U32)addr, gc1054_addr_byte, gc1054_data_byte);
}


int gc1054_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc1054_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc1054_i2c_addr, (CVI_U32)addr, gc1054_addr_byte,
						(CVI_U32)data, gc1054_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void gc1054_standby(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);
	printf("gc1054_standby\n");
}

void gc1054_restart(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);
	printf("gc1054_restart\n");
}

void gc1054_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc1054[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc1054_write_register(ViPipe,
				g_pastGc1054[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc1054[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void gc1054_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastGc1054[ViPipe]->enWDRMode;

	gc1054_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
		gc1054_linear_720p30_init(ViPipe);
	}
	g_pastGc1054[ViPipe]->bInit = CVI_TRUE;
}

void gc1054_exit(VI_PIPE ViPipe)
{
	gc1054_i2c_exit(ViPipe);
}

static void gc1054_linear_720p30_init(VI_PIPE ViPipe)
{
	/****system****/
	gc1054_write_register(ViPipe, 0xf2, 0x00);
	gc1054_write_register(ViPipe, 0xf6, 0x00);
	gc1054_write_register(ViPipe, 0xfc, 0x04);
	gc1054_write_register(ViPipe, 0xf7, 0x01);
	gc1054_write_register(ViPipe, 0xf8, 0x0b);
	gc1054_write_register(ViPipe, 0xf9, 0x06);
	gc1054_write_register(ViPipe, 0xfa, 0x80);
	gc1054_write_register(ViPipe, 0xfc, 0x0e);
	/****CISCTL & ANALOG****/
	gc1054_write_register(ViPipe, 0xfe, 0x00);
	gc1054_write_register(ViPipe, 0x03, 0x02);
	gc1054_write_register(ViPipe, 0x04, 0xa6);
	gc1054_write_register(ViPipe, 0x05, 0x02); //HB
	gc1054_write_register(ViPipe, 0x06, 0x07);
	gc1054_write_register(ViPipe, 0x07, 0x00); //VB
	gc1054_write_register(ViPipe, 0x08, 0x2a);
	gc1054_write_register(ViPipe, 0x09, 0x00);
	gc1054_write_register(ViPipe, 0x0a, 0x04); //row start
	gc1054_write_register(ViPipe, 0x0b, 0x00);
	gc1054_write_register(ViPipe, 0x0c, 0x00); //col start
	gc1054_write_register(ViPipe, 0x0d, 0x02);
	gc1054_write_register(ViPipe, 0x0e, 0xd8); //height 724
	gc1054_write_register(ViPipe, 0x0f, 0x05);
	gc1054_write_register(ViPipe, 0x10, 0x10); //width 1288
	gc1054_write_register(ViPipe, 0x17, 0xc0);
	gc1054_write_register(ViPipe, 0x18, 0x02);
	gc1054_write_register(ViPipe, 0x19, 0x08);
	gc1054_write_register(ViPipe, 0x1a, 0x18);
	gc1054_write_register(ViPipe, 0x1d, 0x12);
	gc1054_write_register(ViPipe, 0x1e, 0x50);
	gc1054_write_register(ViPipe, 0x1f, 0x80);
	gc1054_write_register(ViPipe, 0x21, 0x3a);
	gc1054_write_register(ViPipe, 0x23, 0xe8);
	gc1054_write_register(ViPipe, 0x25, 0x10);
	gc1054_write_register(ViPipe, 0x28, 0x20);
	gc1054_write_register(ViPipe, 0x34, 0x0a); //data low
	gc1054_write_register(ViPipe, 0x3c, 0x10);
	gc1054_write_register(ViPipe, 0x3d, 0x0e);
	gc1054_write_register(ViPipe, 0xcc, 0x8f);
	gc1054_write_register(ViPipe, 0xcd, 0x9a);
	gc1054_write_register(ViPipe, 0xcf, 0x70);
	gc1054_write_register(ViPipe, 0xd0, 0x9a);
	gc1054_write_register(ViPipe, 0xd1, 0xc5);
	gc1054_write_register(ViPipe, 0xd2, 0xed); //data high
	gc1054_write_register(ViPipe, 0xd8, 0x3c); //dacin offset
	gc1054_write_register(ViPipe, 0xd9, 0x7a);
	gc1054_write_register(ViPipe, 0xda, 0x12);
	gc1054_write_register(ViPipe, 0xdb, 0x50);
	gc1054_write_register(ViPipe, 0xde, 0x0c);
	gc1054_write_register(ViPipe, 0xe3, 0x60);
	gc1054_write_register(ViPipe, 0xe4, 0x78);
	gc1054_write_register(ViPipe, 0xfe, 0x01);
	gc1054_write_register(ViPipe, 0xe3, 0x01);
	gc1054_write_register(ViPipe, 0xe6, 0x16); //ramps offset
	gc1054_write_register(ViPipe, 0xfe, 0x01);
	gc1054_write_register(ViPipe, 0x80, 0x50);
	gc1054_write_register(ViPipe, 0x88, 0x73);
	gc1054_write_register(ViPipe, 0x89, 0x03);
	gc1054_write_register(ViPipe, 0x90, 0x01);
	gc1054_write_register(ViPipe, 0x92, 0x00); //crop win 2<=y<=4
	gc1054_write_register(ViPipe, 0x94, 0x00); //crop win 2<=x<=5
	gc1054_write_register(ViPipe, 0x95, 0x02); //crop win height
	gc1054_write_register(ViPipe, 0x96, 0xd4);
	gc1054_write_register(ViPipe, 0x97, 0x05); //crop win width
	gc1054_write_register(ViPipe, 0x98, 0x08);
	/*blk*/
	gc1054_write_register(ViPipe, 0xfe, 0x01);
	gc1054_write_register(ViPipe, 0x40, 0x22);
	gc1054_write_register(ViPipe, 0x43, 0x03);
	gc1054_write_register(ViPipe, 0x4e, 0x3c);
	gc1054_write_register(ViPipe, 0x4f, 0x00);
	gc1054_write_register(ViPipe, 0x60, 0x00);
	gc1054_write_register(ViPipe, 0x61, 0x80);
	/*gain*/
	gc1054_write_register(ViPipe, 0xfe, 0x01);
	gc1054_write_register(ViPipe, 0xb0, 0x48);
	gc1054_write_register(ViPipe, 0xb1, 0x01);
	gc1054_write_register(ViPipe, 0xb2, 0x00);
	gc1054_write_register(ViPipe, 0xb6, 0x00);
	gc1054_write_register(ViPipe, 0xfe, 0x02);
	gc1054_write_register(ViPipe, 0x01, 0x00);
	gc1054_write_register(ViPipe, 0x02, 0x01);
	gc1054_write_register(ViPipe, 0x03, 0x02);
	gc1054_write_register(ViPipe, 0x04, 0x03);
	gc1054_write_register(ViPipe, 0x05, 0x04);
	gc1054_write_register(ViPipe, 0x06, 0x05);
	gc1054_write_register(ViPipe, 0x07, 0x06);
	gc1054_write_register(ViPipe, 0x08, 0x0e);
	gc1054_write_register(ViPipe, 0x09, 0x16);
	gc1054_write_register(ViPipe, 0x0a, 0x1e);
	gc1054_write_register(ViPipe, 0x0b, 0x36);
	gc1054_write_register(ViPipe, 0x0c, 0x3e);
	gc1054_write_register(ViPipe, 0x0d, 0x56);
	gc1054_write_register(ViPipe, 0xfe, 0x02);
	gc1054_write_register(ViPipe, 0xb0, 0x00); //col_gain[11:8]
	gc1054_write_register(ViPipe, 0xb1, 0x00);
	gc1054_write_register(ViPipe, 0xb2, 0x00);
	gc1054_write_register(ViPipe, 0xb3, 0x11);
	gc1054_write_register(ViPipe, 0xb4, 0x22);
	gc1054_write_register(ViPipe, 0xb5, 0x54);
	gc1054_write_register(ViPipe, 0xb6, 0xb8);
	gc1054_write_register(ViPipe, 0xb7, 0x60);
	gc1054_write_register(ViPipe, 0xb9, 0x00); //col_gain[12]
	gc1054_write_register(ViPipe, 0xba, 0xc0);
	gc1054_write_register(ViPipe, 0xc0, 0x20); //col_gain[7:0]
	gc1054_write_register(ViPipe, 0xc1, 0x2d);
	gc1054_write_register(ViPipe, 0xc2, 0x40);
	gc1054_write_register(ViPipe, 0xc3, 0x5b);
	gc1054_write_register(ViPipe, 0xc4, 0x80);
	gc1054_write_register(ViPipe, 0xc5, 0xb5);
	gc1054_write_register(ViPipe, 0xc6, 0x00);
	gc1054_write_register(ViPipe, 0xc7, 0x6a);
	gc1054_write_register(ViPipe, 0xc8, 0x00);
	gc1054_write_register(ViPipe, 0xc9, 0xd4);
	gc1054_write_register(ViPipe, 0xca, 0x00);
	gc1054_write_register(ViPipe, 0xcb, 0xa8);
	gc1054_write_register(ViPipe, 0xcc, 0x00);
	gc1054_write_register(ViPipe, 0xcd, 0x50);
	gc1054_write_register(ViPipe, 0xce, 0x00);
	gc1054_write_register(ViPipe, 0xcf, 0xa1);
	/****DARKSUN****/
	gc1054_write_register(ViPipe, 0xfe, 0x02);
	gc1054_write_register(ViPipe, 0x54, 0xf7);
	gc1054_write_register(ViPipe, 0x55, 0xf0);
	gc1054_write_register(ViPipe, 0x56, 0x00);
	gc1054_write_register(ViPipe, 0x57, 0x00);
	gc1054_write_register(ViPipe, 0x58, 0x00);
	gc1054_write_register(ViPipe, 0x5a, 0x04);
	/****DD****/
	gc1054_write_register(ViPipe, 0xfe, 0x04);
	gc1054_write_register(ViPipe, 0x40, 0x40); //blc 0x40-10bit
	gc1054_write_register(ViPipe, 0x81, 0x8a);
	/****DVP & MIPI****/
	gc1054_write_register(ViPipe, 0xfe, 0x03);
	gc1054_write_register(ViPipe, 0x01, 0x03);
	gc1054_write_register(ViPipe, 0x02, 0x11);
	gc1054_write_register(ViPipe, 0x03, 0x90);
	gc1054_write_register(ViPipe, 0x10, 0x90);
	gc1054_write_register(ViPipe, 0x11, 0x2b);
	gc1054_write_register(ViPipe, 0x12, 0x4a); //lwc 1280*5/4
	gc1054_write_register(ViPipe, 0x13, 0x06);
	gc1054_write_register(ViPipe, 0x15, 0x06);
	gc1054_write_register(ViPipe, 0x21, 0x02);
	gc1054_write_register(ViPipe, 0x22, 0x02);
	gc1054_write_register(ViPipe, 0x23, 0x08);
	gc1054_write_register(ViPipe, 0x24, 0x02);
	gc1054_write_register(ViPipe, 0x25, 0x10);
	gc1054_write_register(ViPipe, 0x26, 0x04);
	gc1054_write_register(ViPipe, 0x29, 0x02);
	gc1054_write_register(ViPipe, 0x2a, 0x02);
	gc1054_write_register(ViPipe, 0x2b, 0x08);
	gc1054_write_register(ViPipe, 0x42, 0x08);
	gc1054_write_register(ViPipe, 0x43, 0x05);
	gc1054_write_register(ViPipe, 0xfe, 0x00);

	gc1054_default_reg_init(ViPipe);
	delay_ms(40);
	printf("ViPipe:%d,===GC1054 720P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

