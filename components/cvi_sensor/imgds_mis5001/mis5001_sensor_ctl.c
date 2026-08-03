#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"

#include "mis5001_cmos_ex.h"

#define MIS5001_CHIP_ID_ADDR_H		0x3000
#define MIS5001_CHIP_ID_ADDR_L		0x3001
#define MIS5001_CHIP_ID			0x1311
static void mis5001_linear_1080p15_init(VI_PIPE ViPipe);

CVI_U8 mis5001_i2c_addr = 0x30;        /* I2C Address of MIS5001 */
const CVI_U32 mis5001_addr_byte = 2;
const CVI_U32 mis5001_data_byte = 1;
// static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int mis5001_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunMIS5001_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int mis5001_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunMIS5001_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

int mis5001_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunMIS5001_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, mis5001_i2c_addr, (CVI_U32)addr, mis5001_addr_byte, mis5001_data_byte);
}

int mis5001_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunMIS5001_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, mis5001_i2c_addr, (CVI_U32)addr, mis5001_addr_byte,
		(CVI_U32)data, mis5001_data_byte);
}

void mis5001_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastMIS5001[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		mis5001_write_register(ViPipe,
				g_pastMIS5001[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastMIS5001[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int mis5001_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;
	int nVal3;

	usleep(50);
	if (mis5001_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = mis5001_read_register(ViPipe, MIS5001_CHIP_ID_ADDR_H);
	nVal2 = mis5001_read_register(ViPipe, MIS5001_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	nVal3 = (((nVal & 0xFF) << 8) | (nVal2 & 0xFF));
	if (nVal3 != MIS5001_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! true chip id [%x]  read chip id [%x]\n", MIS5001_CHIP_ID, nVal3);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void mis5001_init(VI_PIPE ViPipe)
{
	mis5001_i2c_init(ViPipe);

	mis5001_linear_1080p15_init(ViPipe);

	g_pastMIS5001[ViPipe]->bInit = CVI_TRUE;
}

void mis5001_exit(VI_PIPE ViPipe)
{
	mis5001_i2c_exit(ViPipe);
}

static void mis5001_linear_1080p15_init(VI_PIPE ViPipe)
{
	mis5001_write_register(ViPipe, 0x300a, 0x01);
	mis5001_write_register(ViPipe, 0x3006, 0x02);
	mis5001_write_register(ViPipe, 0xffff, 0x64);
	delay_ms(100);
	mis5001_write_register(ViPipe, 0x3307, 0x72);
	mis5001_write_register(ViPipe, 0x310f, 0xB8);
	mis5001_write_register(ViPipe, 0x310e, 0x0B);
	mis5001_write_register(ViPipe, 0x4220, 0x2b);
	mis5001_write_register(ViPipe, 0x4221, 0x6b);
	mis5001_write_register(ViPipe, 0x4222, 0xab);
	mis5001_write_register(ViPipe, 0x4223, 0xeb);
	mis5001_write_register(ViPipe, 0x3011, 0x2b);
	mis5001_write_register(ViPipe, 0x3302, 0x03);
	mis5001_write_register(ViPipe, 0x310d, 0xea);
	mis5001_write_register(ViPipe, 0x310c, 0x07);
	mis5001_write_register(ViPipe, 0x3115, 0x50);
	mis5001_write_register(ViPipe, 0x3114, 0x01);
	mis5001_write_register(ViPipe, 0x3117, 0xcf);
	mis5001_write_register(ViPipe, 0x3116, 0x08);
	mis5001_write_register(ViPipe, 0x3111, 0x0c);
	mis5001_write_register(ViPipe, 0x3110, 0x00);
	mis5001_write_register(ViPipe, 0x3113, 0x8D);
	mis5001_write_register(ViPipe, 0x3112, 0x07);
	mis5001_write_register(ViPipe, 0x3128, 0x0f);
	mis5001_write_register(ViPipe, 0x3129, 0xff);
	mis5001_write_register(ViPipe, 0x3012, 0x03);
	mis5001_write_register(ViPipe, 0x3306, 0x01);
	mis5001_write_register(ViPipe, 0x3309, 0x02);
	mis5001_write_register(ViPipe, 0x330a, 0x04);
	mis5001_write_register(ViPipe, 0x330b, 0x09);
	mis5001_write_register(ViPipe, 0x3f00, 0x01);
	mis5001_write_register(ViPipe, 0x3f02, 0x07);
	mis5001_write_register(ViPipe, 0x3f01, 0x00);
	mis5001_write_register(ViPipe, 0x3f04, 0x2a);
	mis5001_write_register(ViPipe, 0x3f03, 0x00);
	mis5001_write_register(ViPipe, 0x3f06, 0xa5);
	mis5001_write_register(ViPipe, 0x3f05, 0x04);
	mis5001_write_register(ViPipe, 0x3f08, 0xff);
	mis5001_write_register(ViPipe, 0x3f07, 0x1f);
	mis5001_write_register(ViPipe, 0x3f0a, 0xa4);
	mis5001_write_register(ViPipe, 0x3f09, 0x01);
	mis5001_write_register(ViPipe, 0x3f0c, 0x38);
	mis5001_write_register(ViPipe, 0x3f0b, 0x00);
	mis5001_write_register(ViPipe, 0x3f0e, 0xff);
	mis5001_write_register(ViPipe, 0x3f0d, 0x1f);
	mis5001_write_register(ViPipe, 0x3f10, 0xff);
	mis5001_write_register(ViPipe, 0x3f0f, 0x1f);
	mis5001_write_register(ViPipe, 0x3f13, 0x07);
	mis5001_write_register(ViPipe, 0x3f12, 0x00);
	mis5001_write_register(ViPipe, 0x3f15, 0x9d);
	mis5001_write_register(ViPipe, 0x3f14, 0x01);
	mis5001_write_register(ViPipe, 0x3f17, 0x31);
	mis5001_write_register(ViPipe, 0x3f16, 0x00);
	mis5001_write_register(ViPipe, 0x3f19, 0x73);
	mis5001_write_register(ViPipe, 0x3f18, 0x01);
	mis5001_write_register(ViPipe, 0x3f1b, 0x00);
	mis5001_write_register(ViPipe, 0x3f1a, 0x00);
	mis5001_write_register(ViPipe, 0x3f1d, 0xa9);
	mis5001_write_register(ViPipe, 0x3f1c, 0x04);
	mis5001_write_register(ViPipe, 0x3f1f, 0xff);
	mis5001_write_register(ViPipe, 0x3f1e, 0x1f);
	mis5001_write_register(ViPipe, 0x3f21, 0xff);
	mis5001_write_register(ViPipe, 0x3f20, 0x1f);
	mis5001_write_register(ViPipe, 0x3f23, 0x85);
	mis5001_write_register(ViPipe, 0x3f22, 0x00);
	mis5001_write_register(ViPipe, 0x3f25, 0x27);
	mis5001_write_register(ViPipe, 0x3f24, 0x01);
	mis5001_write_register(ViPipe, 0x3f28, 0x46);
	mis5001_write_register(ViPipe, 0x3f27, 0x00);
	mis5001_write_register(ViPipe, 0x3f2a, 0x07);
	mis5001_write_register(ViPipe, 0x3f29, 0x00);
	mis5001_write_register(ViPipe, 0x3f2c, 0x3f);
	mis5001_write_register(ViPipe, 0x3f2b, 0x00);
	mis5001_write_register(ViPipe, 0x3f2e, 0x70);
	mis5001_write_register(ViPipe, 0x3f2d, 0x01);
	mis5001_write_register(ViPipe, 0x3f30, 0x38);
	mis5001_write_register(ViPipe, 0x3f2f, 0x00);
	mis5001_write_register(ViPipe, 0x3f32, 0x3f);
	mis5001_write_register(ViPipe, 0x3f31, 0x00);
	mis5001_write_register(ViPipe, 0x3f34, 0xd1);
	mis5001_write_register(ViPipe, 0x3f33, 0x00);
	mis5001_write_register(ViPipe, 0x3f36, 0xc0);
	mis5001_write_register(ViPipe, 0x3f35, 0x00);
	mis5001_write_register(ViPipe, 0x3f38, 0x2f);
	mis5001_write_register(ViPipe, 0x3f37, 0x02);
	mis5001_write_register(ViPipe, 0x3f3a, 0x5d);
	mis5001_write_register(ViPipe, 0x3f39, 0x02);
	mis5001_write_register(ViPipe, 0x3f4f, 0x5d);
	mis5001_write_register(ViPipe, 0x3f4e, 0x02);
	mis5001_write_register(ViPipe, 0x3f51, 0x5d);
	mis5001_write_register(ViPipe, 0x3f50, 0x02);
	mis5001_write_register(ViPipe, 0x3f53, 0x5d);
	mis5001_write_register(ViPipe, 0x3f52, 0x02);
	mis5001_write_register(ViPipe, 0x3f55, 0x50);
	mis5001_write_register(ViPipe, 0x3f54, 0x02);
	mis5001_write_register(ViPipe, 0x3f3c, 0x9a);
	mis5001_write_register(ViPipe, 0x3f3b, 0x00);
	mis5001_write_register(ViPipe, 0x3f3e, 0x09);
	mis5001_write_register(ViPipe, 0x3f3d, 0x04);
	mis5001_write_register(ViPipe, 0x3f40, 0x93);
	mis5001_write_register(ViPipe, 0x3f3f, 0x01);
	mis5001_write_register(ViPipe, 0x3f42, 0x8f);
	mis5001_write_register(ViPipe, 0x3f41, 0x00);
	mis5001_write_register(ViPipe, 0x3f44, 0xb0);
	mis5001_write_register(ViPipe, 0x3f43, 0x04);
	mis5001_write_register(ViPipe, 0x312b, 0x4a);
	mis5001_write_register(ViPipe, 0x312a, 0x00);
	mis5001_write_register(ViPipe, 0x312f, 0xb2);
	mis5001_write_register(ViPipe, 0x312e, 0x00);
	mis5001_write_register(ViPipe, 0x3124, 0x09);
	mis5001_write_register(ViPipe, 0x4200, 0x0c);
	mis5001_write_register(ViPipe, 0x4201, 0x00);
	mis5001_write_register(ViPipe, 0x4202, 0x40);
	mis5001_write_register(ViPipe, 0x420C, 0x26);
	mis5001_write_register(ViPipe, 0x420e, 0x35);
	mis5001_write_register(ViPipe, 0x4216, 0x6c);
	mis5001_write_register(ViPipe, 0x4217, 0xdc);
	mis5001_write_register(ViPipe, 0x4218, 0x02);
	mis5001_write_register(ViPipe, 0x4240, 0x8d);
	mis5001_write_register(ViPipe, 0x4242, 0x0f);
	mis5001_write_register(ViPipe, 0x4224, 0x80);
	mis5001_write_register(ViPipe, 0x4225, 0x07);
	mis5001_write_register(ViPipe, 0x4226, 0x80);
	mis5001_write_register(ViPipe, 0x4227, 0x07);
	mis5001_write_register(ViPipe, 0x4228, 0x80);
	mis5001_write_register(ViPipe, 0x4229, 0x07);
	mis5001_write_register(ViPipe, 0x422a, 0x80);
	mis5001_write_register(ViPipe, 0x422b, 0x07);
	mis5001_write_register(ViPipe, 0x422c, 0x80);
	mis5001_write_register(ViPipe, 0x422d, 0x07);
	mis5001_write_register(ViPipe, 0x422e, 0x80);
	mis5001_write_register(ViPipe, 0x422f, 0x07);
	mis5001_write_register(ViPipe, 0x4230, 0x80);
	mis5001_write_register(ViPipe, 0x4231, 0x07);
	mis5001_write_register(ViPipe, 0x4232, 0x80);
	mis5001_write_register(ViPipe, 0x4233, 0x07);
	mis5001_write_register(ViPipe, 0x4509, 0x0f);
	mis5001_write_register(ViPipe, 0x4505, 0x00);
	mis5001_write_register(ViPipe, 0x4501, 0xff);
	mis5001_write_register(ViPipe, 0x4502, 0x33);
	mis5001_write_register(ViPipe, 0x4503, 0x11);
	mis5001_write_register(ViPipe, 0x4501, 0xf0);
	mis5001_write_register(ViPipe, 0x4502, 0x30);
	mis5001_write_register(ViPipe, 0x4503, 0x10);
	mis5001_write_register(ViPipe, 0x3006, 0x00);
	mis5001_write_register(ViPipe, 0x3308, 0x04);
	mis5001_write_register(ViPipe, 0x3A01, 0xA0);
	mis5001_write_register(ViPipe, 0x401E, 0x3C);
	mis5001_write_register(ViPipe, 0x401d, 0xa0);
	mis5001_write_register(ViPipe, 0x3012, 0x03);
	mis5001_write_register(ViPipe, 0x3500, 0x1B);
	mis5001_write_register(ViPipe, 0x3501, 0x03);
	mis5001_write_register(ViPipe, 0x3E00, 0x00);
	mis5001_write_register(ViPipe, 0x3E01, 0x10);
	mis5001_write_register(ViPipe, 0x400D, 0x30);
	mis5001_write_register(ViPipe, 0x3508, 0x0a);
	mis5001_write_register(ViPipe, 0x3508, 0x04);
	mis5001_write_register(ViPipe, 0x3513, 0x01);
	mis5001_write_register(ViPipe, 0x3514, 0x09);
	mis5001_write_register(ViPipe, 0x3515, 0x0b);
	mis5001_write_register(ViPipe, 0x3702, 0x80);
	mis5001_write_register(ViPipe, 0x3704, 0x80);
	mis5001_write_register(ViPipe, 0x3706, 0x80);
	mis5001_write_register(ViPipe, 0x3708, 0x80);
	mis5001_write_register(ViPipe, 0x400D, 0x30);
	mis5001_write_register(ViPipe, 0x4004, 0x00);
	mis5001_write_register(ViPipe, 0x4005, 0x30);
	mis5001_write_register(ViPipe, 0x4009, 0x09);
	mis5001_write_register(ViPipe, 0x400a, 0x48);
	mis5001_write_register(ViPipe, 0x4006, 0x86);
	mis5001_write_register(ViPipe, 0x4019, 0x08);
	mis5001_write_register(ViPipe, 0x401b, 0x00);
	mis5001_write_register(ViPipe, 0x3f42, 0x58);
	mis5001_write_register(ViPipe, 0x3f49, 0x60);
	mis5001_write_register(ViPipe, 0x3f38, 0x38);
	mis5001_write_register(ViPipe, 0x4501, 0xff);
	mis5001_write_register(ViPipe, 0x3006, 0x00);
	mis5001_write_register(ViPipe, 0xffff, 0x03);
	mis5001_write_register(ViPipe, 0x4501, 0xf0);

	mis5001_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===MIS5001 1920x1920 15fps 10bit LINEAR Init OK!===\n", ViPipe);
}

