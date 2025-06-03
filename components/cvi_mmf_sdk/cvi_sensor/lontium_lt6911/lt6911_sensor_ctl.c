#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "lt6911_cmos_ex.h"
#include "sensor_i2c.h"

#define LT6911_I2C_DEV 1
#define LT6911_I2C_BANK_ADDR 0xff

CVI_U8 lt6911_i2c_addr = 0x2b;
const CVI_U32 lt6911_addr_byte = 1;
const CVI_U32 lt6911_data_byte = 1;

int lt6911_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunLt6911_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunLt6911_AddrInfo[ViPipe].s8I2cAddr);
}

int lt6911_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunLt6911_BusInfo[ViPipe].s8I2cDev);
}

int lt6911_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunLt6911_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunLt6911_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							lt6911_addr_byte, lt6911_data_byte);
}

int lt6911_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunLt6911_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunLt6911_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							lt6911_addr_byte, (CVI_U32)data, lt6911_data_byte);
}

static int lt6911_i2c_read(VI_PIPE ViPipe, int RegAddr)
{
	uint8_t bank = RegAddr >> 8;
	uint8_t addr = RegAddr & 0xff;

	lt6911_write_register(ViPipe, LT6911_I2C_BANK_ADDR, bank);
	return lt6911_read_register(ViPipe, addr);
}

static int lt6911_i2c_write(VI_PIPE ViPipe, int RegAddr, int data)
{
	uint8_t bank = RegAddr >> 8;
	uint8_t addr = RegAddr & 0xff;

	lt6911_write_register(ViPipe, LT6911_I2C_BANK_ADDR, bank);
	return lt6911_write_register(ViPipe, addr, data);
}

int lt6911_read(VI_PIPE ViPipe, int addr)
{
	int data = 0;

	lt6911_i2c_write(ViPipe, 0x80ee, 0x01);
	data = lt6911_i2c_read(ViPipe, addr);
	lt6911_i2c_write(ViPipe, 0x80ee, 0x00);
	return data;
}

int lt6911_write(VI_PIPE ViPipe, int addr, int data)
{
	lt6911_i2c_write(ViPipe, 0x80ee, 0x01);
	lt6911_i2c_write(ViPipe, addr, data);
	lt6911_i2c_write(ViPipe, 0x80ee, 0x00);
	return CVI_SUCCESS;
}

#define LT6911_CHIP_ID_ADDR_H	0xa000
#define LT6911_CHIP_ID_ADDR_L	0xa001
#define LT6911_CHIP_ID          0x1605

int  lt6911_probe(VI_PIPE ViPipe)
{
	//int nVal;
	//int nVal2;

	usleep(50);
	if (lt6911_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	//nVal  = lt6911_read(ViPipe, LT6911_CHIP_ID_ADDR_H);
	//nVal2 = lt6911_read(ViPipe, LT6911_CHIP_ID_ADDR_L);
	//if (nVal < 0 || nVal2 < 0) {
	//	CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
	//	return nVal;
	//}
	//printf("data:%02x %02x\n", nVal, nVal2);
	//if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != LT6911_CHIP_ID) {
	//	CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
	//	return CVI_FAILURE;
	//}

	return CVI_SUCCESS;
}

void lt6911_init(VI_PIPE ViPipe)
{
	lt6911_i2c_init(ViPipe);
	usleep(1500 * 1000);
}

