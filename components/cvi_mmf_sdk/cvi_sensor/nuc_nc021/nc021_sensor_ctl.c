#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "nc021_cmos_ex.h"
#include "sensor_i2c.h"

const CVI_U32 nc021_addr_byte = 1;
const CVI_U32 nc021_data_byte = 1;
#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

// #define MAX_BUF 64
int nc021_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunnc021_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunnc021_AddrInfo[ViPipe].s8I2cAddr);
}

int nc021_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunnc021_BusInfo[ViPipe].s8I2cDev);
}

int nc021_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunnc021_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunnc021_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							nc021_addr_byte, nc021_data_byte);
}

int nc021_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunnc021_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunnc021_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							nc021_addr_byte, (CVI_U32)data, nc021_data_byte);
}

void nc021_init(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);
}


