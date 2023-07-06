#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "ov5647_cmos_ex.h"

static void ov5647_linear_1080p30_init(VI_PIPE ViPipe);

CVI_U8 ov5647_i2c_addr = 0x36;        /* I2C Address of OV5647 */
const CVI_U32 ov5647_addr_byte = 2;
const CVI_U32 ov5647_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int ov5647_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOv5647_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int ov5647_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOv5647_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int ov5647_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOv5647_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, ov5647_i2c_addr, (CVI_U32)addr, ov5647_addr_byte, ov5647_data_byte);

}

int ov5647_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOv5647_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, ov5647_i2c_addr, (CVI_U32)addr, ov5647_addr_byte,
				(CVI_U32)data, ov5647_data_byte);

}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void ov5647_standby(VI_PIPE ViPipe)
{
	ov5647_write_register(ViPipe, 0x0100, 0x00); /* STANDBY */
}

void ov5647_restart(VI_PIPE ViPipe)
{
	ov5647_write_register(ViPipe, 0x0100, 0x01); /* standby */
}

void ov5647_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;
	CVI_U32 start = 1;
	CVI_U32 end = g_pastOv5647[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum - 3;

	for (i = start; i < end; i++) {
		ov5647_write_register(ViPipe,
				g_pastOv5647[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOv5647[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define OV5647_FLIP	0x3820
#define OV5647_MIRROR	0x3821
void ov5647_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 flip, mirror;

	flip = ov5647_read_register(ViPipe, OV5647_FLIP);
	mirror = ov5647_read_register(ViPipe, OV5647_MIRROR);
	flip &= ~(0x3 << 1);
	mirror &= ~(0x3 << 1);

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		mirror |= 0x3 << 1;
		break;
	case ISP_SNS_FLIP:
		flip |= 0x3 << 1;
		break;
	case ISP_SNS_MIRROR_FLIP:
		flip |= 0x3 << 1;
		mirror |= 0x3 << 1;
		break;
	default:
		return;
	}

	ov5647_write_register(ViPipe, OV5647_FLIP, flip);
	ov5647_write_register(ViPipe, OV5647_MIRROR, mirror);
}

#define OV5647_CHIP_ID_ADDR_H		0x300A
#define OV5647_CHIP_ID_ADDR_L		0x300B
#define OV5647_CHIP_ID			0x5647

int ov5647_probe(VI_PIPE ViPipe)
{
	int nVal, nVal2;

	usleep(1000);
	if (ov5647_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = ov5647_read_register(ViPipe, OV5647_CHIP_ID_ADDR_H);
	nVal2 = ov5647_read_register(ViPipe, OV5647_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != OV5647_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

void ov5647_init(VI_PIPE ViPipe)
{
	ov5647_i2c_init(ViPipe);

	delay_ms(10);

	ov5647_linear_1080p30_init(ViPipe);

	g_pastOv5647[ViPipe]->bInit = CVI_TRUE;
}

void ov5647_exit(VI_PIPE ViPipe)
{
	ov5647_i2c_exit(ViPipe);
}

/* 1080P30 */
static void ov5647_linear_1080p30_init(VI_PIPE ViPipe)
{
	ov5647_write_register(ViPipe, 0x0100, 0x00);
	ov5647_write_register(ViPipe, 0x0103, 0x01);
	ov5647_write_register(ViPipe, 0x3035, 0x11);
	ov5647_write_register(ViPipe, 0x3036, 0x64);
	ov5647_write_register(ViPipe, 0x303c, 0x11);
	ov5647_write_register(ViPipe, 0x3821, 0x00);
	ov5647_write_register(ViPipe, 0x3820, 0x00);
	ov5647_write_register(ViPipe, 0x370c, 0x0f);
	ov5647_write_register(ViPipe, 0x3612, 0x5b);
	ov5647_write_register(ViPipe, 0x3618, 0x04);
	ov5647_write_register(ViPipe, 0x5000, 0x06);
	ov5647_write_register(ViPipe, 0x5002, 0x40);
	ov5647_write_register(ViPipe, 0x5003, 0x08);
	ov5647_write_register(ViPipe, 0x5a00, 0x08);
	ov5647_write_register(ViPipe, 0x3000, 0xff);
	ov5647_write_register(ViPipe, 0x3001, 0xff);
	ov5647_write_register(ViPipe, 0x3002, 0xff);
	ov5647_write_register(ViPipe, 0x301d, 0xf0);
	ov5647_write_register(ViPipe, 0x3503, 0x07);
	ov5647_write_register(ViPipe, 0x3a18, 0x00);
	ov5647_write_register(ViPipe, 0x3a19, 0xf8);
	ov5647_write_register(ViPipe, 0x3c01, 0x80);
	ov5647_write_register(ViPipe, 0x3b07, 0x0c);
	ov5647_write_register(ViPipe, 0x380c, 0x09);
	ov5647_write_register(ViPipe, 0x380d, 0x70);
	ov5647_write_register(ViPipe, 0x380e, 0x04);
	ov5647_write_register(ViPipe, 0x380f, 0x50);
	ov5647_write_register(ViPipe, 0x3814, 0x11);
	ov5647_write_register(ViPipe, 0x3815, 0x11);
	ov5647_write_register(ViPipe, 0x3708, 0x64);
	ov5647_write_register(ViPipe, 0x3709, 0x12);
	ov5647_write_register(ViPipe, 0x3808, 0x07);
	ov5647_write_register(ViPipe, 0x3809, 0x80);
	ov5647_write_register(ViPipe, 0x380a, 0x04);
	ov5647_write_register(ViPipe, 0x380b, 0x38);
	ov5647_write_register(ViPipe, 0x3800, 0x01);
	ov5647_write_register(ViPipe, 0x3801, 0x5c);
	ov5647_write_register(ViPipe, 0x3802, 0x01);
	ov5647_write_register(ViPipe, 0x3803, 0xb2);
	ov5647_write_register(ViPipe, 0x3804, 0x08);
	ov5647_write_register(ViPipe, 0x3805, 0xe3);
	ov5647_write_register(ViPipe, 0x3806, 0x05);
	ov5647_write_register(ViPipe, 0x3807, 0xf1);
	ov5647_write_register(ViPipe, 0x3630, 0x2e);
	ov5647_write_register(ViPipe, 0x3632, 0xe2);
	ov5647_write_register(ViPipe, 0x3633, 0x23);
	ov5647_write_register(ViPipe, 0x3634, 0x44);
	ov5647_write_register(ViPipe, 0x3620, 0x64);
	ov5647_write_register(ViPipe, 0x3621, 0xe0);
	ov5647_write_register(ViPipe, 0x3600, 0x37);
	ov5647_write_register(ViPipe, 0x3704, 0xa0);
	ov5647_write_register(ViPipe, 0x3703, 0x5a);
	ov5647_write_register(ViPipe, 0x3715, 0x78);
	ov5647_write_register(ViPipe, 0x3717, 0x01);
	ov5647_write_register(ViPipe, 0x3731, 0x02);
	ov5647_write_register(ViPipe, 0x370b, 0x60);
	ov5647_write_register(ViPipe, 0x3705, 0x1a);
	ov5647_write_register(ViPipe, 0x3f05, 0x02);
	ov5647_write_register(ViPipe, 0x3f06, 0x10);
	ov5647_write_register(ViPipe, 0x3f01, 0x0a);
	ov5647_write_register(ViPipe, 0x3a08, 0x01);
	ov5647_write_register(ViPipe, 0x3a09, 0x4b);
	ov5647_write_register(ViPipe, 0x3a0a, 0x01);
	ov5647_write_register(ViPipe, 0x3a0b, 0x13);
	ov5647_write_register(ViPipe, 0x3a0d, 0x04);
	ov5647_write_register(ViPipe, 0x3a0e, 0x03);
	ov5647_write_register(ViPipe, 0x3a0f, 0x58);
	ov5647_write_register(ViPipe, 0x3a10, 0x50);
	ov5647_write_register(ViPipe, 0x3a1b, 0x58);
	ov5647_write_register(ViPipe, 0x3a1e, 0x50);
	ov5647_write_register(ViPipe, 0x3a11, 0x60);
	ov5647_write_register(ViPipe, 0x3a1f, 0x28);
	ov5647_write_register(ViPipe, 0x4001, 0x02);
	ov5647_write_register(ViPipe, 0x4004, 0x04);
	ov5647_write_register(ViPipe, 0x4000, 0x09);
	ov5647_write_register(ViPipe, 0x4050, 0x6e);
	ov5647_write_register(ViPipe, 0x4051, 0x8f);
	ov5647_write_register(ViPipe, 0x0100, 0x01);
	ov5647_write_register(ViPipe, 0x3000, 0x00);
	ov5647_write_register(ViPipe, 0x3001, 0x00);
	ov5647_write_register(ViPipe, 0x3002, 0x00);
	ov5647_write_register(ViPipe, 0x3017, 0xe0);
	ov5647_write_register(ViPipe, 0x301c, 0xfc);
	ov5647_write_register(ViPipe, 0x3636, 0x06);
	ov5647_write_register(ViPipe, 0x3016, 0x08);
	ov5647_write_register(ViPipe, 0x3827, 0xec);
	ov5647_write_register(ViPipe, 0x3018, 0x44);
	ov5647_write_register(ViPipe, 0x3035, 0x21);
	ov5647_write_register(ViPipe, 0x3106, 0xf5);
	ov5647_write_register(ViPipe, 0x3034, 0x1a);
	ov5647_write_register(ViPipe, 0x301c, 0xf8);

	//ov5647_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===OV5647 1080P 30fps 10bit LINE Init OK!\n", ViPipe);
}





