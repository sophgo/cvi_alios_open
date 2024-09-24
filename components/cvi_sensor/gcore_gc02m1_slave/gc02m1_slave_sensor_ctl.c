#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#include "gc02m1_slave_cmos_ex.h"
#define GC02M1_SLAVE_CHIP_ID_ADDR_H	0xf0
#define GC02M1_SLAVE_CHIP_ID_ADDR_L	0xf1
#define GC02M1_SLAVE_CHIP_ID		0x02e0

#define GC02M1_SLAVE_MIRROR_NORMAL    1
#define GC02M1_SLAVE_MIRROR_H         0
#define GC02M1_SLAVE_MIRROR_V         0
#define GC02M1_SLAVE_MIRROR_HV        0

#if GC02M1_SLAVE_MIRROR_NORMAL
#define GC02M1_SLAVE_MIRROR	        0x80
#elif GC02M1_SLAVE_MIRROR_H
#define GC02M1_SLAVE_MIRROR	        0x81
#elif GC02M1_SLAVE_MIRROR_V
#define GC02M1_SLAVE_MIRROR	        0x82
#elif GC02M1_SLAVE_MIRROR_HV
#define GC02M1_SLAVE_MIRROR	        0x83
#else
#define GC02M1_SLAVE_MIRROR	        0x80
#endif

extern CVI_U16 g_au16Gc02m1_Slave_UseHwSync[VI_MAX_PIPE_NUM];
static void gc02m1_slave_linear_1200p30_init(VI_PIPE ViPipe);

CVI_U8 gc02m1_slave_i2c_addr = 0x37;
const CVI_U32 gc02m1_slave_addr_byte = 1;
const CVI_U32 gc02m1_slave_data_byte = 1;

int gc02m1_slave_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc02m1_Slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc02m1_slave_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc02m1_Slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int gc02m1_slave_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc02m1_Slave_BusInfo[ViPipe].s8I2cDev;

    return sensor_i2c_read(i2c_id, gc02m1_slave_i2c_addr, (CVI_U32)addr, gc02m1_slave_addr_byte, gc02m1_slave_data_byte);
}

int gc02m1_slave_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc02m1_Slave_BusInfo[ViPipe].s8I2cDev;

    return sensor_i2c_write(i2c_id, gc02m1_slave_i2c_addr, (CVI_U32)addr, gc02m1_slave_addr_byte,
            (CVI_U32)data, gc02m1_slave_data_byte);
}

void gc02m1_slave_standby(VI_PIPE ViPipe)
{
	int nVal;

	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);

	nVal = gc02m1_slave_read_register(ViPipe, 0x3e);
	nVal &= ~(0x1 << 7);
	nVal &= ~(0x1 << 4);
	gc02m1_slave_write_register(ViPipe, 0x3e, nVal);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x01);

	nVal = gc02m1_slave_read_register(ViPipe, 0xf9);
	nVal |= (0x1 << 0);
	gc02m1_slave_write_register(ViPipe, 0xf9, nVal);

	printf("gc02m1_slave_standby\n");
}

void gc02m1_slave_restart(VI_PIPE ViPipe)
{
	int nVal;

	nVal = gc02m1_slave_read_register(ViPipe, 0xf9);
	nVal &= ~(0x1 << 0);
	gc02m1_slave_write_register(ViPipe, 0xf9, nVal);

	usleep(1);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x8e);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	nVal = gc02m1_slave_read_register(ViPipe, 0x3e);
	nVal |= (0x1 << 7);
	nVal |= (0x1 << 4);
	gc02m1_slave_write_register(ViPipe, 0x3e, nVal);

	printf("gc02m1_slave_restart\n");
}

void gc02m1_slave_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc02m1_Slave[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc02m1_slave_write_register(ViPipe,
				g_pastGc02m1_Slave[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc02m1_Slave[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int gc02m1_slave_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (gc02m1_slave_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc02m1_slave_read_register(ViPipe, GC02M1_SLAVE_CHIP_ID_ADDR_H);
	nVal2 = gc02m1_slave_read_register(ViPipe, GC02M1_SLAVE_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC02M1_SLAVE_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc02m1_slave_init(VI_PIPE ViPipe)
{
	gc02m1_slave_i2c_init(ViPipe);

	gc02m1_slave_linear_1200p30_init(ViPipe);

	g_pastGc02m1_Slave[ViPipe]->bInit = CVI_TRUE;
}

void gc02m1_slave_exit(VI_PIPE ViPipe)
{
	gc02m1_slave_i2c_exit(ViPipe);
}

static void gc02m1_slave_linear_1200p30_init(VI_PIPE ViPipe)
{

	/*system*/
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xf4, 0x41);
	gc02m1_slave_write_register(ViPipe, 0xf5, 0xc0);
	gc02m1_slave_write_register(ViPipe, 0xf6, 0x44);
	gc02m1_slave_write_register(ViPipe, 0xf8, 0x32);
	gc02m1_slave_write_register(ViPipe, 0xf9, 0x82);
	gc02m1_slave_write_register(ViPipe, 0xfa, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xfd, 0x80);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x81);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x03);
	gc02m1_slave_write_register(ViPipe, 0x01, 0x0b);
	gc02m1_slave_write_register(ViPipe, 0xf7, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x80);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x80);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x80);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x8e);
	/*CISCTL*/
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x87, 0x09);
	gc02m1_slave_write_register(ViPipe, 0xee, 0x72);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_slave_write_register(ViPipe, 0x8c, 0x90);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x90, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x03, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x04, 0x7d);
	gc02m1_slave_write_register(ViPipe, 0x41, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x42, 0xf4);
	gc02m1_slave_write_register(ViPipe, 0x05, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x06, 0x48);
	gc02m1_slave_write_register(ViPipe, 0x07, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x08, 0x18);
	gc02m1_slave_write_register(ViPipe, 0x9d, 0x18);
	gc02m1_slave_write_register(ViPipe, 0x09, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x0a, 0x02);
	gc02m1_slave_write_register(ViPipe, 0x0d, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x0e, 0xbc);
	gc02m1_slave_write_register(ViPipe, 0x17, GC02M1_SLAVE_MIRROR);
	gc02m1_slave_write_register(ViPipe, 0x19, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x24, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x56, 0x20);
	gc02m1_slave_write_register(ViPipe, 0x5b, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x5e, 0x01);
	/*analog Register width*/
	gc02m1_slave_write_register(ViPipe, 0x21, 0x3c);
	gc02m1_slave_write_register(ViPipe, 0x44, 0x20);
	gc02m1_slave_write_register(ViPipe, 0xcc, 0x01);
	/*analog mode*/
	gc02m1_slave_write_register(ViPipe, 0x1a, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x1f, 0x11);
	gc02m1_slave_write_register(ViPipe, 0x27, 0x30);
	gc02m1_slave_write_register(ViPipe, 0x2b, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x33, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x53, 0x90);
	gc02m1_slave_write_register(ViPipe, 0xe6, 0x50);
	/*analog voltage*/
	gc02m1_slave_write_register(ViPipe, 0x39, 0x07);
	gc02m1_slave_write_register(ViPipe, 0x43, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x46, 0x2a);
	gc02m1_slave_write_register(ViPipe, 0x7c, 0xa0);
	gc02m1_slave_write_register(ViPipe, 0xd0, 0xbe);
	gc02m1_slave_write_register(ViPipe, 0xd1, 0x60);
	gc02m1_slave_write_register(ViPipe, 0xd2, 0x40);
	gc02m1_slave_write_register(ViPipe, 0xd3, 0xf3);
	gc02m1_slave_write_register(ViPipe, 0xde, 0x1d);
	/*analog current*/
	gc02m1_slave_write_register(ViPipe, 0xcd, 0x05);
	gc02m1_slave_write_register(ViPipe, 0xce, 0x6f);
	/*CISCTL RESET*/
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x88);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x10);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x8e);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x88);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x10);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xfc, 0x8e);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x04);
	gc02m1_slave_write_register(ViPipe, 0xe0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	/*ISP*/
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_slave_write_register(ViPipe, 0x53, 0x44);
	gc02m1_slave_write_register(ViPipe, 0x87, 0x50);
	gc02m1_slave_write_register(ViPipe, 0x89, 0x03);
	/*Gain*/
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xb0, 0x74);
	gc02m1_slave_write_register(ViPipe, 0xb1, 0x04);
	gc02m1_slave_write_register(ViPipe, 0xb2, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xb6, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x04);
	gc02m1_slave_write_register(ViPipe, 0xd8, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x40);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x60);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xc0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x2a);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x80);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x40);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xa0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x90);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x19);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xc0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xD0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x2F);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x90);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x39);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x20);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x04);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x20);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x0f);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x40);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x1a);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x60);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x20);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x25);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x80);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xa0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x2c);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xa0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x32);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xc0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x20);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x38);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x60);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x3c);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x02);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0xa0);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x40);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x80);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x02);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x18);
	gc02m1_slave_write_register(ViPipe, 0xc0, 0x5c);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x9f, 0x10);
	/*BLK*/
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x26, 0x20);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_slave_write_register(ViPipe, 0x40, 0x22);
	gc02m1_slave_write_register(ViPipe, 0x46, 0x7f);
	gc02m1_slave_write_register(ViPipe, 0x49, 0x0f);
	gc02m1_slave_write_register(ViPipe, 0x4a, 0xf0);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x14, 0x80);
	gc02m1_slave_write_register(ViPipe, 0x15, 0x80);
	gc02m1_slave_write_register(ViPipe, 0x16, 0x80);
	gc02m1_slave_write_register(ViPipe, 0x17, 0x80);
	/*ant _blooming*/
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_slave_write_register(ViPipe, 0x41, 0x20);
	gc02m1_slave_write_register(ViPipe, 0x4c, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x4d, 0x0c);
	gc02m1_slave_write_register(ViPipe, 0x44, 0x08);
	gc02m1_slave_write_register(ViPipe, 0x48, 0x03);
	/*Window 1600X1200*/
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_slave_write_register(ViPipe, 0x90, 0x01);
	gc02m1_slave_write_register(ViPipe, 0x91, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x92, 0x06);
	gc02m1_slave_write_register(ViPipe, 0x93, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x94, 0x06);
	gc02m1_slave_write_register(ViPipe, 0x95, 0x04);
	gc02m1_slave_write_register(ViPipe, 0x96, 0xb0);
	gc02m1_slave_write_register(ViPipe, 0x97, 0x06);
	gc02m1_slave_write_register(ViPipe, 0x98, 0x40);
	/*mipi*/
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x03);
	gc02m1_slave_write_register(ViPipe, 0x01, 0x23);
	gc02m1_slave_write_register(ViPipe, 0x03, 0xce);
	gc02m1_slave_write_register(ViPipe, 0x04, 0x48);
	gc02m1_slave_write_register(ViPipe, 0x15, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x21, 0x10);
	gc02m1_slave_write_register(ViPipe, 0x22, 0x05);
	gc02m1_slave_write_register(ViPipe, 0x23, 0x20);
	gc02m1_slave_write_register(ViPipe, 0x25, 0x20);
	gc02m1_slave_write_register(ViPipe, 0x26, 0x08);
	gc02m1_slave_write_register(ViPipe, 0x29, 0x06);
	gc02m1_slave_write_register(ViPipe, 0x2a, 0x0a);
	gc02m1_slave_write_register(ViPipe, 0x2b, 0x08);
	/*out*/
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_slave_write_register(ViPipe, 0x8c, 0x10);
	gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_slave_write_register(ViPipe, 0x3e, 0x90);

	if (g_au16Gc02m1_Slave_UseHwSync[ViPipe]) {
		/*frame sync slave*/
		gc02m1_slave_write_register(ViPipe, 0xfe, 0x00);
		gc02m1_slave_write_register(ViPipe, 0x7f, 0x29);
		gc02m1_slave_write_register(ViPipe, 0x82, 0x08);
		gc02m1_slave_write_register(ViPipe, 0x83, 0x0f);
		gc02m1_slave_write_register(ViPipe, 0x88, 0x00);
		gc02m1_slave_write_register(ViPipe, 0x89, 0x04);
		gc02m1_slave_write_register(ViPipe, 0x8a, 0x00);
		gc02m1_slave_write_register(ViPipe, 0x8b, 0x12);
		gc02m1_slave_write_register(ViPipe, 0x85, 0x51);
	}

	gc02m1_slave_default_reg_init(ViPipe);

	printf("ViPipe:%d,===GC02M1_SLAVE 1200P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}
