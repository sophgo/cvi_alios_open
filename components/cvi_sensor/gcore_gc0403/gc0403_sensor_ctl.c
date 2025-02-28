#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "gc0403_cmos_ex.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>


#define GC0403_CHIP_ID_ADDR_H   0xf0
#define GC0403_CHIP_ID_ADDR_L   0xf1
#define GC0403_CHIP_ID          0x0403

static void gc0403_linear_576p60_init(VI_PIPE ViPipe);

CVI_U8 gc0403_i2c_addr_write = 0x3c;//0x6e
CVI_U8 gc0403_i2c_addr_read = 0x3c;
const CVI_U32 gc0403_addr_byte = 1;
const CVI_U32 gc0403_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int gc0403_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc0403_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc0403_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc0403_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int gc0403_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc0403_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc0403_i2c_addr_read, (CVI_U32)addr, gc0403_addr_byte, gc0403_data_byte);
}


int gc0403_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc0403_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc0403_i2c_addr_write, (CVI_U32)addr, gc0403_addr_byte,
				(CVI_U32)data, gc0403_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void gc0403_standby(VI_PIPE ViPipe)
{

	printf("gc0403_standby\n");
}

void gc0403_restart(VI_PIPE ViPipe)
{

	printf("gc0403_restart\n");
}

void gc0403_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc0403[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc0403_write_register(ViPipe,
				g_pastGc0403[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc0403[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void gc0403_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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
	gc0403_write_register(ViPipe, 0x0015, value);
	gc0403_write_register(ViPipe, 0x0d15, value);
}

int gc0403_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (gc0403_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc0403_read_register(ViPipe, GC0403_CHIP_ID_ADDR_H);
	nVal2 = gc0403_read_register(ViPipe, GC0403_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC0403_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc0403_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastGc0403[ViPipe]->enWDRMode;

	gc0403_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
		gc0403_linear_576p60_init(ViPipe);
	}
	g_pastGc0403[ViPipe]->bInit = CVI_TRUE;
}

void gc0403_exit(VI_PIPE ViPipe)
{
	gc0403_i2c_exit(ViPipe);
}

static void gc0403_linear_576p60_init(VI_PIPE ViPipe)
{
	gc0403_write_register(ViPipe, 0xfe,0x80);
	gc0403_write_register(ViPipe, 0xfe,0x80);
	gc0403_write_register(ViPipe, 0xfe,0x80);
	gc0403_write_register(ViPipe, 0xf2,0x00); //sync_pad_io_ebi
	gc0403_write_register(ViPipe, 0xf6,0x00); //up down
	gc0403_write_register(ViPipe, 0xfc,0xc6);
	gc0403_write_register(ViPipe, 0xf7,0x19); //pll enable
	gc0403_write_register(ViPipe, 0xf8,0x03); //Pll mode 2
	gc0403_write_register(ViPipe, 0xf9,0x3e); //[0] pll enable//解决IOVDD 电流大问题
	gc0403_write_register(ViPipe, 0xfe,0x03);
	gc0403_write_register(ViPipe, 0x06,0x80);
	gc0403_write_register(ViPipe, 0x06,0x00);
	gc0403_write_register(ViPipe, 0xfe,0x00);
	gc0403_write_register(ViPipe, 0xf9,0x2e);
	gc0403_write_register(ViPipe, 0xfe,0x00);
	gc0403_write_register(ViPipe, 0xfa,0x00); //div
	gc0403_write_register(ViPipe, 0xfe,0x00);
	///////////////////////////////////////////
	//////   ANALOG & CISCTL   ////////////////
	///////////////////////////////////////////
	gc0403_write_register(ViPipe, 0x03,0x02);
	gc0403_write_register(ViPipe, 0x04,0x55);

	gc0403_write_register(ViPipe, 0x05,0x00);
	gc0403_write_register(ViPipe, 0x06,0xbb);//48
	gc0403_write_register(ViPipe, 0x07,0x00);
	gc0403_write_register(ViPipe, 0x08,0x46);

	gc0403_write_register(ViPipe, 0x0c,0x04);
	gc0403_write_register(ViPipe, 0x0d,0x02);
	gc0403_write_register(ViPipe, 0x0e,0x48);//584
	gc0403_write_register(ViPipe, 0x0f,0x03);
	gc0403_write_register(ViPipe, 0x10,0x08);//776

	gc0403_write_register(ViPipe, 0x11,0x23);//44FPN 列异常
	gc0403_write_register(ViPipe, 0x12,0x10);
	gc0403_write_register(ViPipe, 0x13,0x11);
	gc0403_write_register(ViPipe, 0x14,0x01);
	gc0403_write_register(ViPipe, 0x15,0x00);
	gc0403_write_register(ViPipe, 0x16,0xc0);
	gc0403_write_register(ViPipe, 0x17,0x14);
	gc0403_write_register(ViPipe, 0x18,0x02);
	gc0403_write_register(ViPipe, 0x19,0x38);
	gc0403_write_register(ViPipe, 0x1a,0x11);
	gc0403_write_register(ViPipe, 0x1b,0x06);//06
	gc0403_write_register(ViPipe, 0x1c,0x04);
	gc0403_write_register(ViPipe, 0x1d,0x00);
	gc0403_write_register(ViPipe, 0x1e,0xfc);
	gc0403_write_register(ViPipe, 0x1f,0x09);
	gc0403_write_register(ViPipe, 0x20,0xb5);
	gc0403_write_register(ViPipe, 0x21,0x3f);
	gc0403_write_register(ViPipe, 0x22,0xe6);
	gc0403_write_register(ViPipe, 0x23,0x32);
	gc0403_write_register(ViPipe, 0x24,0x2f);
	gc0403_write_register(ViPipe, 0x27,0x00);
	gc0403_write_register(ViPipe, 0x28,0x00);
	gc0403_write_register(ViPipe, 0x2a,0x00);
	gc0403_write_register(ViPipe, 0x2b,0x00);
	gc0403_write_register(ViPipe, 0x2c,0x00);
	gc0403_write_register(ViPipe, 0x2d,0x01);
	gc0403_write_register(ViPipe, 0x2e,0xf0);
	gc0403_write_register(ViPipe, 0x2f,0x01);
	gc0403_write_register(ViPipe, 0x25,0xc0);
	gc0403_write_register(ViPipe, 0x3d,0xd0);
	gc0403_write_register(ViPipe, 0x3e,0x45);
	gc0403_write_register(ViPipe, 0x3f,0x1f);
	gc0403_write_register(ViPipe, 0xc2,0x17);//1a
	gc0403_write_register(ViPipe, 0x30,0x22);
	gc0403_write_register(ViPipe, 0x31,0x23);
	gc0403_write_register(ViPipe, 0x32,0x02);
	gc0403_write_register(ViPipe, 0x33,0x03);
	gc0403_write_register(ViPipe, 0x34,0x04);
	gc0403_write_register(ViPipe, 0x35,0x05);
	gc0403_write_register(ViPipe, 0x36,0x06);
	gc0403_write_register(ViPipe, 0x37,0x07);
	gc0403_write_register(ViPipe, 0x38,0x0f);
	gc0403_write_register(ViPipe, 0x39,0x17);
	gc0403_write_register(ViPipe, 0x3a,0x1f);
	///////////////////////////////////////////
	////////////   ISP   //////////////////////
	///////////////////////////////////////////
	gc0403_write_register(ViPipe, 0xfe,0x00);
	gc0403_write_register(ViPipe, 0x8a,0x00);
	gc0403_write_register(ViPipe, 0x8c,0x07);
	gc0403_write_register(ViPipe, 0x8e,0x02); //luma value not normal
	gc0403_write_register(ViPipe, 0x90,0x01);
	gc0403_write_register(ViPipe, 0x94,0x02);
	gc0403_write_register(ViPipe, 0x95,0x02);
	gc0403_write_register(ViPipe, 0x96,0x40); //576
	gc0403_write_register(ViPipe, 0x97,0x03);
	gc0403_write_register(ViPipe, 0x98,0x00); //768
	///////////////////////////////////////////
	////////////	 BLK	/////////////////////
	///////////////////////////////////////////
	gc0403_write_register(ViPipe, 0xfe,0x00);
	gc0403_write_register(ViPipe, 0x18,0x02);
	gc0403_write_register(ViPipe, 0x40,0x22);
	gc0403_write_register(ViPipe, 0x41,0x01);
	gc0403_write_register(ViPipe, 0x5e,0x00);
	gc0403_write_register(ViPipe, 0x66,0x20);
	///////////////////////////////////////////
	////////////	 MIPI	/////////////////////
	///////////////////////////////////////////
	gc0403_write_register(ViPipe, 0xfe,0x03);
	gc0403_write_register(ViPipe, 0x01,0x83);
	gc0403_write_register(ViPipe, 0x02,0x44); //0x11
	gc0403_write_register(ViPipe, 0x03,0x96);
	gc0403_write_register(ViPipe, 0x04,0x01);
	gc0403_write_register(ViPipe, 0x05,0x00);
	gc0403_write_register(ViPipe, 0x06,0xa4);
	gc0403_write_register(ViPipe, 0x10,0x90);
	gc0403_write_register(ViPipe, 0x11,0x2b);
	gc0403_write_register(ViPipe, 0x12,0xc0);//c0
	gc0403_write_register(ViPipe, 0x13,0x03);
	gc0403_write_register(ViPipe, 0x15,0x02);
	gc0403_write_register(ViPipe, 0x21,0x10);
	gc0403_write_register(ViPipe, 0x22,0x03);
	gc0403_write_register(ViPipe, 0x23,0x20);
	gc0403_write_register(ViPipe, 0x24,0x02);
	gc0403_write_register(ViPipe, 0x25,0x10);
	gc0403_write_register(ViPipe, 0x26,0x05);
	gc0403_write_register(ViPipe, 0x21,0x10);
	gc0403_write_register(ViPipe, 0x29,0x03);
	gc0403_write_register(ViPipe, 0x2a,0x0a);
	gc0403_write_register(ViPipe, 0x2b,0x04);
	gc0403_write_register(ViPipe, 0xfe,0x00);
	gc0403_write_register(ViPipe, 0xb0,0x50);
	gc0403_write_register(ViPipe, 0xb6,0x00);

	gc0403_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===GC0403 576P 60fps 10bit LINE Init OK!===\n", ViPipe);


}

