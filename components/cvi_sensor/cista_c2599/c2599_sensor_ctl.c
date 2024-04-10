#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "c2599_cmos_ex.h"

static void c2599_linear_1200p30_init(VI_PIPE ViPipe);

CVI_U8 c2599_i2c_addr = 0x36;        /* I2C Address of C2599 */
const CVI_U32 c2599_addr_byte = 2;
const CVI_U32 c2599_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int c2599_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunC2599_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int c2599_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunC2599_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int c2599_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunC2599_BusInfo[ViPipe].s8I2cDev;

    return sensor_i2c_read(i2c_id, c2599_i2c_addr, (CVI_U32)addr, c2599_addr_byte, c2599_data_byte);
}

int c2599_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunC2599_BusInfo[ViPipe].s8I2cDev;

    return sensor_i2c_write(i2c_id, c2599_i2c_addr, (CVI_U32)addr, c2599_addr_byte,
            (CVI_U32)data, c2599_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void c2599_standby(VI_PIPE ViPipe)
{
	c2599_write_register(ViPipe, 0x0100, 0x00);
}

void c2599_restart(VI_PIPE ViPipe)
{
	c2599_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	c2599_write_register(ViPipe, 0x0100, 0x01);
}

void c2599_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastC2599[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		c2599_write_register(ViPipe,
				g_pastC2599[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastC2599[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void c2599_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = 0;
	CVI_U8 x_shift, y_shift;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		x_shift = 4;
		y_shift = 4;
		break;
	case ISP_SNS_MIRROR:
		val |= 0x1;
		x_shift = 5;
		y_shift = 4;
		break;
	case ISP_SNS_FLIP:
		val |= 0x02;
		x_shift = 4;
		y_shift = 5;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x03;
		x_shift = 5;
		y_shift = 5;
		break;
	default:
		return;
	}

	c2599_write_register(ViPipe, 0x0101, val);
	c2599_write_register(ViPipe, 0x3009, x_shift);
	c2599_write_register(ViPipe, 0x300b, y_shift);
}

#define C2599_CHIP_ID_HI_ADDR		0x0000
#define C2599_CHIP_ID_LO_ADDR		0x0001
#define C2599_CHIP_ID			0x0401

int c2599_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	if (c2599_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	delay_ms(5);

	nVal = c2599_read_register(ViPipe, C2599_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = c2599_read_register(ViPipe, C2599_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID = %d, chip_id\n");
	if (chip_id != C2599_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_SUCCESS;
	}

	return CVI_SUCCESS;
}


void c2599_init(VI_PIPE ViPipe)
{
	c2599_i2c_init(ViPipe);

	c2599_linear_1200p30_init(ViPipe);

	g_pastC2599[ViPipe]->bInit = CVI_TRUE;
}

void c2599_exit(VI_PIPE ViPipe)
{
	c2599_i2c_exit(ViPipe);
}

/* 1200P30 and 1200P25 */
static void c2599_linear_1200p30_init(VI_PIPE ViPipe)
{
	c2599_write_register(ViPipe, 0x0400, 0x41);
	c2599_write_register(ViPipe, 0x0401, 0xa5);
	c2599_write_register(ViPipe, 0x0403, 0x36);
	c2599_write_register(ViPipe, 0x3885, 0x22);
	c2599_write_register(ViPipe, 0x32af, 0x88);

	c2599_write_register(ViPipe, 0x3280, 0x2b);
	c2599_write_register(ViPipe, 0x3284, 0xA3);

	c2599_write_register(ViPipe, 0x3288, 0x10);
	c2599_write_register(ViPipe, 0x328b, 0x61);
	c2599_write_register(ViPipe, 0x328C, 0x48);
	c2599_write_register(ViPipe, 0x32aa, 0x03);
	c2599_write_register(ViPipe, 0x32ab, 0x08);
	c2599_write_register(ViPipe, 0x3C00, 0x43);
	c2599_write_register(ViPipe, 0x3C01, 0x03);
	c2599_write_register(ViPipe, 0x3182, 0x30);

	c2599_write_register(ViPipe, 0x3218, 0x28);
	c2599_write_register(ViPipe, 0x3805, 0x08);
	c2599_write_register(ViPipe, 0x3808, 0x16);
	c2599_write_register(ViPipe, 0x3809, 0x96);
	c2599_write_register(ViPipe, 0x380a, 0x7d);
	c2599_write_register(ViPipe, 0x380e, 0x0d);
	c2599_write_register(ViPipe, 0x380c, 0x01);
	c2599_write_register(ViPipe, 0x0202, 0x04);
	c2599_write_register(ViPipe, 0x0203, 0xd0);
	c2599_write_register(ViPipe, 0x3108, 0xcf);
	c2599_write_register(ViPipe, 0x3115, 0x30);

	c2599_write_register(ViPipe, 0x3212, 0x2f);
	c2599_write_register(ViPipe, 0x3298, 0x48);
	c2599_write_register(ViPipe, 0x32af, 0x80);
	c2599_write_register(ViPipe, 0x3212, 0x4A);
	c2599_write_register(ViPipe, 0x3287, 0x48);
	c2599_write_register(ViPipe, 0x3881, 0x00);

	c2599_write_register(ViPipe, 0x3224, 0x70);

	c2599_write_register(ViPipe, 0x0400, 0x47);
	c2599_write_register(ViPipe, 0x0404, 0x08);
	c2599_write_register(ViPipe, 0x0405, 0x80);
	c2599_write_register(ViPipe, 0x0406, 0x02);
	c2599_write_register(ViPipe, 0x0407, 0x80);
	c2599_write_register(ViPipe, 0x3403, 0x38);
	c2599_write_register(ViPipe, 0x3411, 0x00);
	c2599_write_register(ViPipe, 0x3412, 0x01);
	c2599_write_register(ViPipe, 0x3415, 0x01);
	c2599_write_register(ViPipe, 0x3416, 0x01);
	c2599_write_register(ViPipe, 0x3500, 0x10);
	c2599_write_register(ViPipe, 0x3584, 0x02);
	c2599_write_register(ViPipe, 0xe000, 0x31);
	c2599_write_register(ViPipe, 0xe001, 0x08);
	c2599_write_register(ViPipe, 0xe002, 0x4f);
	c2599_write_register(ViPipe, 0xe00c, 0x31);
	c2599_write_register(ViPipe, 0xe00d, 0x08);
	c2599_write_register(ViPipe, 0xe00e, 0xef);
	c2599_write_register(ViPipe, 0xe018, 0x32);
	c2599_write_register(ViPipe, 0xe019, 0xa9);
	c2599_write_register(ViPipe, 0xe01a, 0x50);
	c2599_write_register(ViPipe, 0xe01b, 0x32);
	c2599_write_register(ViPipe, 0xe01c, 0xac);
	c2599_write_register(ViPipe, 0xe01d, 0x44);
	c2599_write_register(ViPipe, 0xe01e, 0x32);
	c2599_write_register(ViPipe, 0xe01f, 0xad);
	c2599_write_register(ViPipe, 0xe020, 0x9f);
	c2599_write_register(ViPipe, 0x3500, 0x00);
	c2599_write_register(ViPipe, 0x3584, 0x22);

	c2599_write_register(ViPipe, 0x3293, 0x00);
	c2599_write_register(ViPipe, 0x32a9, 0x3F);
	c2599_write_register(ViPipe, 0x3290, 0xB4);

	c2599_write_register(ViPipe, 0x32ac, 0x5f);
	c2599_write_register(ViPipe, 0x0309, 0x10);
	c2599_write_register(ViPipe, 0x32ab, 0x19);
	c2599_write_register(ViPipe, 0x328c, 0xc1);
	c2599_write_register(ViPipe, 0x0216, 0x01);
	c2599_write_register(ViPipe, 0x3286, 0x04);

	c2599_write_register(ViPipe, 0x32a9, 0x50);
	c2599_write_register(ViPipe, 0x32ac, 0x44);
	c2599_write_register(ViPipe, 0x32ad, 0xb0);
	c2599_write_register(ViPipe, 0x3211, 0x15);
	c2599_write_register(ViPipe, 0x3216, 0x1a);
	c2599_write_register(ViPipe, 0x3217, 0x12);
	c2599_write_register(ViPipe, 0x0216, 0x01);
	c2599_write_register(ViPipe, 0x0202, 0x04);
	c2599_write_register(ViPipe, 0x0203, 0xd0);
	c2599_write_register(ViPipe, 0x3295, 0x05);
	c2599_write_register(ViPipe, 0x3882, 0x05);
	c2599_write_register(ViPipe, 0x328b, 0xa1);
	c2599_write_register(ViPipe, 0x328d, 0x08);
	c2599_write_register(ViPipe, 0x3904, 0x03);
	c2599_write_register(ViPipe, 0x0343, 0x88);

	c2599_write_register(ViPipe, 0xe018, 0x32);
	c2599_write_register(ViPipe, 0xe019, 0xa9);
	c2599_write_register(ViPipe, 0xe01a, 0x50);

	c2599_write_register(ViPipe, 0xe01b, 0x32);
	c2599_write_register(ViPipe, 0xe01c, 0xac);
	c2599_write_register(ViPipe, 0xe01d, 0x44);

	c2599_write_register(ViPipe, 0xe01e, 0x32);
	c2599_write_register(ViPipe, 0xe01f, 0xad);
	c2599_write_register(ViPipe, 0xe020, 0x9f);

	delay_ms(200);
	c2599_default_reg_init(ViPipe);

	c2599_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===C2599 1200P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

