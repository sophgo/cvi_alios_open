#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "bf2253l_cmos_ex.h"

static void bf2253l_linear_1200p10_init(VI_PIPE ViPipe);

CVI_U8 bf2253l_i2c_addr = 0x6e;        /* I2C Address of BF2253L */
const CVI_U32 bf2253l_addr_byte = 1;
const CVI_U32 bf2253l_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int bf2253l_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBF2253L_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int bf2253l_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBF2253L_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int bf2253l_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBF2253L_BusInfo[ViPipe].s8I2cDev;

    return sensor_i2c_read(i2c_id, bf2253l_i2c_addr, (CVI_U32)addr, bf2253l_addr_byte, bf2253l_data_byte);
}

int bf2253l_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBF2253L_BusInfo[ViPipe].s8I2cDev;

    return sensor_i2c_write(i2c_id, bf2253l_i2c_addr, (CVI_U32)addr, bf2253l_addr_byte,
            (CVI_U32)data, bf2253l_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void bf2253l_prog(VI_PIPE ViPipe, int *rom)
{
	int i = 0;

	while (1) {
		int lookup = rom[i++];
		int addr = (lookup >> 16) & 0xFFFF;
		int data = lookup & 0xFFFF;

		if (addr == 0xFFFE)
			delay_ms(data);
		else if (addr != 0xFFFF)
			bf2253l_write_register(ViPipe, addr, data);
	}
}

void bf2253l_standby(VI_PIPE ViPipe)
{
	bf2253l_write_register(ViPipe, 0xe0, 0x01);
}

void bf2253l_restart(VI_PIPE ViPipe)
{
	bf2253l_write_register(ViPipe, 0xe0, 0x01);
	delay_ms(20);
	bf2253l_write_register(ViPipe, 0xe0, 0x00);
}

void bf2253l_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastBF2253L[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		bf2253l_write_register(ViPipe,
				g_pastBF2253L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastBF2253L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define BF2253L_CHIP_ID_HI_ADDR		0xfc
#define BF2253L_CHIP_ID_LO_ADDR		0xfd
#define BF2253L_CHIP_ID			0x2253

int bf2253l_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	usleep(4*1000);
	if (bf2253l_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = bf2253l_read_register(ViPipe, BF2253L_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = bf2253l_read_register(ViPipe, BF2253L_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != BF2253L_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}


void bf2253l_init(VI_PIPE ViPipe)
{
	bf2253l_linear_1200p10_init(ViPipe);

	g_pastBF2253L[ViPipe]->bInit = CVI_TRUE;
}

void bf2253l_exit(VI_PIPE ViPipe)
{
	bf2253l_i2c_exit(ViPipe);
}

/* 1200P10 and 1200P25 */
static void bf2253l_linear_1200p10_init(VI_PIPE ViPipe)
{
	bf2253l_write_register(ViPipe, 0xe1, 0x06);
	bf2253l_write_register(ViPipe, 0xe2, 0x06);
	bf2253l_write_register(ViPipe, 0xe3, 0x0e);
	bf2253l_write_register(ViPipe, 0xe4, 0x40);
	bf2253l_write_register(ViPipe, 0xe5, 0x67);
	bf2253l_write_register(ViPipe, 0xe6, 0x02);
	bf2253l_write_register(ViPipe, 0xe8, 0x84);
	bf2253l_write_register(ViPipe, 0x01, 0x14);
	bf2253l_write_register(ViPipe, 0x03, 0x98);
	bf2253l_write_register(ViPipe, 0x27, 0x21);
	bf2253l_write_register(ViPipe, 0x29, 0x20);
	bf2253l_write_register(ViPipe, 0x59, 0x10);
	bf2253l_write_register(ViPipe, 0x5a, 0x10);
	bf2253l_write_register(ViPipe, 0x5c, 0x11);
	bf2253l_write_register(ViPipe, 0x5d, 0x73);
	bf2253l_write_register(ViPipe, 0x6a, 0x2f);
	bf2253l_write_register(ViPipe, 0x6b, 0x0e);
	bf2253l_write_register(ViPipe, 0x6c, 0x7e);
	bf2253l_write_register(ViPipe, 0x6f, 0x10);
	bf2253l_write_register(ViPipe, 0x70, 0x08);
	bf2253l_write_register(ViPipe, 0x71, 0x05);
	bf2253l_write_register(ViPipe, 0x72, 0x10);
	bf2253l_write_register(ViPipe, 0x73, 0x08);
	bf2253l_write_register(ViPipe, 0x74, 0x05);
	bf2253l_write_register(ViPipe, 0x75, 0x06);
	bf2253l_write_register(ViPipe, 0x76, 0x20);
	bf2253l_write_register(ViPipe, 0x77, 0x03);
	bf2253l_write_register(ViPipe, 0x78, 0x0e);
	bf2253l_write_register(ViPipe, 0x79, 0x08);
	bf2253l_write_register(ViPipe, 0xe0, 0x00);
	bf2253l_default_reg_init(ViPipe);

	printf("ViPipe:%d,===BF2253L 1200P 10fps 10bit LINE Init OK!===\n", ViPipe);
}
