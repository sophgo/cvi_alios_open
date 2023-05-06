#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#include "bf314a_cmos_ex.h"

#define BF314A_CHIP_ID_ADDR_H	0xfc
#define BF314A_CHIP_ID_ADDR_L	0xfd
#define BF314A_CHIP_ID		0x314a


static void bf314a_linear_720p30_init(VI_PIPE ViPipe);

CVI_U8 bf314a_i2c_addr = 0x6e;
const CVI_U32 bf314a_addr_byte = 1;
const CVI_U32 bf314a_data_byte = 1;

int bf314a_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBf314a_BusInfo[ViPipe].s8I2cDev;
	printf("bf314a i2c init\r\n");
	return sensor_i2c_init(i2c_id);
}

int bf314a_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBf314a_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int bf314a_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBf314a_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, bf314a_i2c_addr,
		(CVI_U32)addr, bf314a_addr_byte, bf314a_data_byte);
}

int bf314a_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunBf314a_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, bf314a_i2c_addr, (CVI_U32)addr, bf314a_addr_byte,
		(CVI_U32)data, bf314a_data_byte);
}

int bf314a_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(4 * 1000);
	if (bf314a_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = bf314a_read_register(ViPipe, BF314A_CHIP_ID_ADDR_H);
	nVal2 = bf314a_read_register(ViPipe, BF314A_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != BF314A_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void bf314a_init(VI_PIPE ViPipe)
{
	bf314a_linear_720p30_init(ViPipe);

	g_pastBf314a[ViPipe]->bInit = CVI_TRUE;
}

void bf314a_exit(VI_PIPE ViPipe)
{
	bf314a_i2c_exit(ViPipe);
}

static void delay_ms(int ms)
{
	udelay(ms * 1000);
}

void bf314a_standby(VI_PIPE ViPipe)
{
	bf314a_write_register(ViPipe, 0xf3, 0x01);
	printf("%s\r\n", __func__);
}

void bf314a_restart(VI_PIPE ViPipe)
{
	bf314a_write_register(ViPipe, 0xf3, 0x01);
	delay_ms(20);
	bf314a_write_register(ViPipe, 0xf3, 0x00);
}

void bf314a_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastBf314a[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (i == LINEAR_FLIP_MIRROR_ADDR) {
			g_pastBf314a[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data = 0x21; //flip mirror default
		}
		bf314a_write_register(ViPipe,
				g_pastBf314a[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastBf314a[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

static void bf314a_linear_720p30_init(VI_PIPE ViPipe)
{
	//**********************************************************************/
	bf314a_write_register(ViPipe, 0xf2, 0x01);
	bf314a_write_register(ViPipe, 0xf3, 0x00);
	bf314a_write_register(ViPipe, 0xf3, 0x00);    //power up
	bf314a_write_register(ViPipe, 0x00, 0x21);    //mirro and vertical flip
	bf314a_write_register(ViPipe, 0x7d, 0x0f);    //MIPI:0f,DVP:07
	bf314a_write_register(ViPipe, 0xeb, 0x05);    //MIPI:05,DVP:04
	bf314a_write_register(ViPipe, 0x0b, 0x20);    //行长1600
	bf314a_write_register(ViPipe, 0x0c, 0x03);
	bf314a_write_register(ViPipe, 0x03, 0xcf);
	bf314a_write_register(ViPipe, 0x15, 0xca);
	bf314a_write_register(ViPipe, 0x16, 0xd4);
	bf314a_write_register(ViPipe, 0x19, 0xca);
	bf314a_write_register(ViPipe, 0x1b, 0x3f);
	bf314a_write_register(ViPipe, 0x1d, 0x3f);
	bf314a_write_register(ViPipe, 0x24, 0x6c);
	bf314a_write_register(ViPipe, 0x25, 0xc8);
	bf314a_write_register(ViPipe, 0x26, 0xff);
	bf314a_write_register(ViPipe, 0x27, 0x58);
	bf314a_write_register(ViPipe, 0x29, 0x41);
	bf314a_write_register(ViPipe, 0x2b, 0xaa);
	bf314a_write_register(ViPipe, 0xe0, 0x16);      //MIPICLK:360M
	bf314a_write_register(ViPipe, 0xe1, 0xb4);
	bf314a_write_register(ViPipe, 0xe2, 0x3a);
	bf314a_write_register(ViPipe, 0xe3, 0x4a);
	bf314a_write_register(ViPipe, 0xe4, 0x32);
	bf314a_write_register(ViPipe, 0xe5, 0x86);
	bf314a_write_register(ViPipe, 0xe8, 0x60);
	bf314a_write_register(ViPipe, 0x5e, 0x32);   //black Lock
	bf314a_write_register(ViPipe, 0x59, 0x10);   //black target B
	bf314a_write_register(ViPipe, 0x5a, 0x10);   //black target Gb
	bf314a_write_register(ViPipe, 0x5b, 0x10);   //black target R
	bf314a_write_register(ViPipe, 0x5c, 0x10);   //black target Gr
	bf314a_write_register(ViPipe, 0x6a, 0x1f);   //模拟增益
	bf314a_write_register(ViPipe, 0x6b, 0x01);   //积分时间 :2step
	bf314a_write_register(ViPipe, 0x6c, 0xc2);   //积分时间
	bf314a_write_register(ViPipe, 0x6d, 0x14);

	//out 1288*728
	bf314a_write_register(ViPipe, 0xcd, 0x08);
	bf314a_write_register(ViPipe, 0xcf, 0xd8);

	bf314a_default_reg_init(ViPipe);
	printf("ViPipe:%d,===BF314A 720P 30fps 10bit LINEAR Init OK!===\r\n", ViPipe);
}