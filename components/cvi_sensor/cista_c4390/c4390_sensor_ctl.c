#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "c4390_cmos_ex.h"

static void c4390_linear_1440p30_init(VI_PIPE ViPipe);

CVI_U8 c4390_i2c_addr = 0x36;        /* I2C Address of C4390 */
const CVI_U32 c4390_addr_byte = 2;
const CVI_U32 c4390_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int c4390_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunC4390_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int c4390_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunC4390_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int c4390_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunC4390_BusInfo[ViPipe].s8I2cDev;

    return sensor_i2c_read(i2c_id, c4390_i2c_addr, (CVI_U32)addr, c4390_addr_byte, c4390_data_byte);
}

int c4390_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunC4390_BusInfo[ViPipe].s8I2cDev;

    return sensor_i2c_write(i2c_id, c4390_i2c_addr, (CVI_U32)addr, c4390_addr_byte,
            (CVI_U32)data, c4390_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void c4390_standby(VI_PIPE ViPipe)
{
	c4390_write_register(ViPipe, 0x0100, 0x00);
}

void c4390_restart(VI_PIPE ViPipe)
{
	c4390_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	c4390_write_register(ViPipe, 0x0100, 0x01);
}

void c4390_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastC4390[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		c4390_write_register(ViPipe,
				g_pastC4390[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastC4390[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void c4390_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	c4390_write_register(ViPipe, 0x0101, val);
	c4390_write_register(ViPipe, 0x3009, x_shift);
	c4390_write_register(ViPipe, 0x300b, y_shift);
}

#define C4390_CHIP_ID_HI_ADDR		0x0000
#define C4390_CHIP_ID_LO_ADDR		0x0001
#define C4390_CHIP_ID			0x0401

int c4390_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	if (c4390_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	delay_ms(5);

	nVal = c4390_read_register(ViPipe, C4390_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = c4390_read_register(ViPipe, C4390_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID = %d, chip_id\n");
	if (chip_id != C4390_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_SUCCESS;
	}

	return CVI_SUCCESS;
}


void c4390_init(VI_PIPE ViPipe)
{
	c4390_i2c_init(ViPipe);

	c4390_linear_1440p30_init(ViPipe);

	g_pastC4390[ViPipe]->bInit = CVI_TRUE;
}

void c4390_exit(VI_PIPE ViPipe)
{
	c4390_i2c_exit(ViPipe);
}

/* 1440P30 and 1440P25 */
static void c4390_linear_1440p30_init(VI_PIPE ViPipe)
{
	c4390_write_register(ViPipe, 0x32aa, 0x05);
	c4390_write_register(ViPipe, 0x32ab, 0x08);
	c4390_write_register(ViPipe, 0x32ac, 0xdf);
	c4390_write_register(ViPipe, 0x3211, 0x10);
	c4390_write_register(ViPipe, 0x3290, 0xa7);
	c4390_write_register(ViPipe, 0x3182, 0x40);
	c4390_write_register(ViPipe, 0x3292, 0x00);
	c4390_write_register(ViPipe, 0x3296, 0x10);
	c4390_write_register(ViPipe, 0x3291, 0x04);
	c4390_write_register(ViPipe, 0x3286, 0x77);
	c4390_write_register(ViPipe, 0x3287, 0x50);
	c4390_write_register(ViPipe, 0x3215, 0x1f);
	c4390_write_register(ViPipe, 0x3216, 0x2f);
	c4390_write_register(ViPipe, 0x3217, 0x11);
	c4390_write_register(ViPipe, 0x3280, 0x8c);
	c4390_write_register(ViPipe, 0x32c8, 0x22);
	c4390_write_register(ViPipe, 0x32ca, 0x22);
	c4390_write_register(ViPipe, 0x3d1f, 0x22);
	c4390_write_register(ViPipe, 0x3d21, 0x22);
	c4390_write_register(ViPipe, 0x3885, 0x22);
	c4390_write_register(ViPipe, 0x3607, 0x22);
	c4390_write_register(ViPipe, 0x3605, 0x22);
	c4390_write_register(ViPipe, 0x3288, 0x50);
	c4390_write_register(ViPipe, 0x0401, 0x3b);
	c4390_write_register(ViPipe, 0x0403, 0x00);
	c4390_write_register(ViPipe, 0x3584, 0x02);
	c4390_write_register(ViPipe, 0x3087, 0x07);
	c4390_write_register(ViPipe, 0x0340, 0x05);
	c4390_write_register(ViPipe, 0x0341, 0xc0);
	c4390_write_register(ViPipe, 0x0342, 0x0b);
	c4390_write_register(ViPipe, 0x0343, 0x28);
	c4390_write_register(ViPipe, 0x3180, 0x20);
	c4390_write_register(ViPipe, 0x3187, 0x14);
	c4390_write_register(ViPipe, 0x3114, 0x4a);
	c4390_write_register(ViPipe, 0x3115, 0x00);
	c4390_write_register(ViPipe, 0x3126, 0x04);
	c4390_write_register(ViPipe, 0x3c01, 0x13);
	c4390_write_register(ViPipe, 0x3584, 0x22);
	c4390_write_register(ViPipe, 0x308c, 0x70);
	c4390_write_register(ViPipe, 0x308d, 0x71);
	c4390_write_register(ViPipe, 0x3403, 0x00);
	c4390_write_register(ViPipe, 0x3407, 0x06);
	c4390_write_register(ViPipe, 0x3410, 0x04);
	c4390_write_register(ViPipe, 0x3414, 0x06);
	c4390_write_register(ViPipe, 0x3600, 0x08);
	c4390_write_register(ViPipe, 0x3500, 0x10);
	c4390_write_register(ViPipe, 0x3584, 0x02);
	c4390_write_register(ViPipe, 0x3411, 0x08);
	c4390_write_register(ViPipe, 0x3412, 0x09);
	c4390_write_register(ViPipe, 0x3415, 0x01);
	c4390_write_register(ViPipe, 0x3416, 0x01);
	c4390_write_register(ViPipe, 0xe060, 0x31);
	c4390_write_register(ViPipe, 0xe061, 0x12);
	c4390_write_register(ViPipe, 0xe062, 0xe4);
	c4390_write_register(ViPipe, 0xe06c, 0x31);
	c4390_write_register(ViPipe, 0xe06d, 0x12);
	c4390_write_register(ViPipe, 0xe06e, 0xe8);
	c4390_write_register(ViPipe, 0x0400, 0x47);
	c4390_write_register(ViPipe, 0x0404, 0x05);
	c4390_write_register(ViPipe, 0x0405, 0x00);
	c4390_write_register(ViPipe, 0x0406, 0x05);
	c4390_write_register(ViPipe, 0x0407, 0x00);
	c4390_write_register(ViPipe, 0xe000, 0x31);
	c4390_write_register(ViPipe, 0xe001, 0x0a);
	c4390_write_register(ViPipe, 0xe002, 0x88);
	c4390_write_register(ViPipe, 0xe003, 0x31);
	c4390_write_register(ViPipe, 0xe004, 0x0b);
	c4390_write_register(ViPipe, 0xe005, 0x88);
	c4390_write_register(ViPipe, 0xe006, 0x31);
	c4390_write_register(ViPipe, 0xe007, 0x0c);
	c4390_write_register(ViPipe, 0xe008, 0x88);
	c4390_write_register(ViPipe, 0xe009, 0x31);
	c4390_write_register(ViPipe, 0xe00a, 0x0d);
	c4390_write_register(ViPipe, 0xe00b, 0x88);
	c4390_write_register(ViPipe, 0xe00c, 0x32);
	c4390_write_register(ViPipe, 0xe00d, 0xac);
	c4390_write_register(ViPipe, 0xe00e, 0xdf);
	c4390_write_register(ViPipe, 0xe00f, 0x31);
	c4390_write_register(ViPipe, 0xe010, 0x87);
	c4390_write_register(ViPipe, 0xe011, 0x04);
	c4390_write_register(ViPipe, 0xe030, 0x31);
	c4390_write_register(ViPipe, 0xe031, 0x0a);
	c4390_write_register(ViPipe, 0xe032, 0x90);
	c4390_write_register(ViPipe, 0xe033, 0x31);
	c4390_write_register(ViPipe, 0xe034, 0x0b);
	c4390_write_register(ViPipe, 0xe035, 0x90);
	c4390_write_register(ViPipe, 0xe036, 0x31);
	c4390_write_register(ViPipe, 0xe037, 0x0c);
	c4390_write_register(ViPipe, 0xe038, 0xa0);
	c4390_write_register(ViPipe, 0xe039, 0x31);
	c4390_write_register(ViPipe, 0xe03a, 0x0d);
	c4390_write_register(ViPipe, 0xe03b, 0x90);
	c4390_write_register(ViPipe, 0xe03c, 0x32);
	c4390_write_register(ViPipe, 0xe03d, 0xac);
	c4390_write_register(ViPipe, 0xe03e, 0xdb);
	c4390_write_register(ViPipe, 0xe03f, 0x31);
	c4390_write_register(ViPipe, 0xe040, 0x87);
	c4390_write_register(ViPipe, 0xe041, 0x14);
	c4390_write_register(ViPipe, 0x3500, 0x00);
	c4390_write_register(ViPipe, 0x3584, 0x22);
	c4390_write_register(ViPipe, 0x3108, 0xcf);
	c4390_write_register(ViPipe, 0x3112, 0xe4);
	c4390_write_register(ViPipe, 0x3113, 0xf3);
	c4390_write_register(ViPipe, 0x3181, 0x50);
	c4390_write_register(ViPipe, 0x300b, 0x10);
	c4390_write_register(ViPipe, 0x310b, 0x85);
	c4390_write_register(ViPipe, 0x310c, 0x90);
	c4390_write_register(ViPipe, 0x310d, 0x85);
	c4390_write_register(ViPipe, 0x0309, 0x10);
	c4390_write_register(ViPipe, 0x0307, 0x4f);
	c4390_write_register(ViPipe, 0x3517, 0x4f);
	c4390_write_register(ViPipe, 0x0304, 0x00);
	c4390_write_register(ViPipe, 0x3880, 0x00);
	c4390_write_register(ViPipe, 0x3881, 0x08);
	c4390_write_register(ViPipe, 0x3805, 0x07);
	c4390_write_register(ViPipe, 0x3806, 0x04);
	c4390_write_register(ViPipe, 0x3807, 0x04);
	c4390_write_register(ViPipe, 0x3808, 0x14);
	c4390_write_register(ViPipe, 0x3809, 0x85);
	c4390_write_register(ViPipe, 0x380a, 0x6d);
	c4390_write_register(ViPipe, 0x380b, 0xaa);
	c4390_write_register(ViPipe, 0x0340, 0x05);
	c4390_write_register(ViPipe, 0x0341, 0xc0);
	c4390_write_register(ViPipe, 0x0347, 0x00);
	c4390_write_register(ViPipe, 0x034b, 0xaf);
	c4390_write_register(ViPipe, 0x034c, 0x0a);
	c4390_write_register(ViPipe, 0x034d, 0x00);
	c4390_write_register(ViPipe, 0x034e, 0x05);
	c4390_write_register(ViPipe, 0x034f, 0xa0);
	c4390_write_register(ViPipe, 0x3009, 0x05);
	c4390_write_register(ViPipe, 0x300b, 0x04);
	c4390_write_register(ViPipe, 0x0101, 0x01);

	c4390_default_reg_init(ViPipe);

	c4390_write_register(ViPipe, 0x0100, 0x01);
	c4390_write_register(ViPipe, 0x0202, 0x05);
	c4390_write_register(ViPipe, 0x0203, 0xc0);
	c4390_write_register(ViPipe, 0x0205, 0x08);


	delay_ms(100);

	printf("ViPipe:%d,===C4390 1440P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

