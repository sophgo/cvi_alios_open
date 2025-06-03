#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc020hgs_cmos_ex.h"
#include "sensor_i2c.h"

static void sc020hgs_linear_400P120_init(VI_PIPE ViPipe);

const CVI_U8 sc020hgs_i2c_addr = 0x30;        /* I2C Address of SC020HGS */
const CVI_U32 sc020hgs_addr_byte = 2;
const CVI_U32 sc020hgs_data_byte = 1;

int sc020hgs_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC020HGS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC020HGS_AddrInfo[ViPipe].s8I2cAddr);
}

int sc020hgs_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC020HGS_BusInfo[ViPipe].s8I2cDev);
}

int sc020hgs_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC020HGS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC020HGS_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc020hgs_addr_byte, sc020hgs_data_byte);
}

int sc020hgs_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC020HGS_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC020HGS_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc020hgs_addr_byte, (CVI_U32)data, sc020hgs_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc020hgs_standby(VI_PIPE ViPipe)
{
	sc020hgs_write_register(ViPipe, 0x2100, 0x00);
}

void sc020hgs_restart(VI_PIPE ViPipe)
{
	sc020hgs_write_register(ViPipe, 0x2100, 0x00);
	delay_ms(20);
	sc020hgs_write_register(ViPipe, 0x2100, 0x01);
}

void sc020hgs_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = sc020hgs_read_register(ViPipe, 0x3221) & ~0x66;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x6;
		break;
	case ISP_SNS_FLIP:
		val |= 0x60;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x66;
		break;
	default:
		return;
	}

	sc020hgs_write_register(ViPipe, 0x3221, val);
}


void sc020hgs_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC020HGS[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastSC020HGS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			sc020hgs_write_register(ViPipe,
				g_pastSC020HGS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC020HGS[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void sc020hgs_init(VI_PIPE ViPipe)
{
	sc020hgs_i2c_init(ViPipe);

	//linear mode only
	sc020hgs_linear_400P120_init(ViPipe);

	g_pastSC020HGS[ViPipe]->bInit = CVI_TRUE;
}

/* 1296P30 and 1296P25 */
static void sc020hgs_linear_400P120_init(VI_PIPE ViPipe)
{
	sc020hgs_write_register(ViPipe, 0x2103, 0x01);
	delay_ms(33);
	sc020hgs_write_register(ViPipe, 0x2100, 0x00);
	sc020hgs_write_register(ViPipe, 0x36e9, 0x80);
	sc020hgs_write_register(ViPipe, 0x300a, 0x40);
	sc020hgs_write_register(ViPipe, 0x301f, 0x07);
	sc020hgs_write_register(ViPipe, 0x3033, 0xa8);
	sc020hgs_write_register(ViPipe, 0x320e, 0x03);
	sc020hgs_write_register(ViPipe, 0x320f, 0x64);
	sc020hgs_write_register(ViPipe, 0x3250, 0x03);
	sc020hgs_write_register(ViPipe, 0x3303, 0x28);
	sc020hgs_write_register(ViPipe, 0x3304, 0x40);
	sc020hgs_write_register(ViPipe, 0x3305, 0x00);
	sc020hgs_write_register(ViPipe, 0x3306, 0x30);
	sc020hgs_write_register(ViPipe, 0x3309, 0x40);
	sc020hgs_write_register(ViPipe, 0x330a, 0x00);
	sc020hgs_write_register(ViPipe, 0x330b, 0x60);
	sc020hgs_write_register(ViPipe, 0x3314, 0xc8);
	sc020hgs_write_register(ViPipe, 0x3317, 0xe8);
	sc020hgs_write_register(ViPipe, 0x331f, 0x02);
	sc020hgs_write_register(ViPipe, 0x3320, 0xf1);
	sc020hgs_write_register(ViPipe, 0x3363, 0x9f);
	sc020hgs_write_register(ViPipe, 0x3385, 0x31);
	sc020hgs_write_register(ViPipe, 0x3387, 0x31);
	sc020hgs_write_register(ViPipe, 0x33ad, 0x38);
	sc020hgs_write_register(ViPipe, 0x33b5, 0x08);
	sc020hgs_write_register(ViPipe, 0x33b6, 0x18);
	sc020hgs_write_register(ViPipe, 0x33b8, 0x28);
	sc020hgs_write_register(ViPipe, 0x33ba, 0x28);
	sc020hgs_write_register(ViPipe, 0x33ef, 0x04);
	sc020hgs_write_register(ViPipe, 0x33f8, 0x00);
	sc020hgs_write_register(ViPipe, 0x33fa, 0x00);
	sc020hgs_write_register(ViPipe, 0x341c, 0x10);
	sc020hgs_write_register(ViPipe, 0x341e, 0x20);
	sc020hgs_write_register(ViPipe, 0x341f, 0x20);
	sc020hgs_write_register(ViPipe, 0x3420, 0x08);
	sc020hgs_write_register(ViPipe, 0x3421, 0x18);
	sc020hgs_write_register(ViPipe, 0x3422, 0x38);
	sc020hgs_write_register(ViPipe, 0x3424, 0x28);
	sc020hgs_write_register(ViPipe, 0x3426, 0x28);
	sc020hgs_write_register(ViPipe, 0x3428, 0x28);
	sc020hgs_write_register(ViPipe, 0x3435, 0x07);
	sc020hgs_write_register(ViPipe, 0x3436, 0x01);
	sc020hgs_write_register(ViPipe, 0x3437, 0x78);
	sc020hgs_write_register(ViPipe, 0x3438, 0x01);
	sc020hgs_write_register(ViPipe, 0x3439, 0x03);
	sc020hgs_write_register(ViPipe, 0x343a, 0x00);
	sc020hgs_write_register(ViPipe, 0x343b, 0xd7);
	sc020hgs_write_register(ViPipe, 0x343c, 0x00);
	sc020hgs_write_register(ViPipe, 0x343d, 0xca);
	sc020hgs_write_register(ViPipe, 0x349c, 0x01);
	sc020hgs_write_register(ViPipe, 0x34f3, 0x08);
	sc020hgs_write_register(ViPipe, 0x34f4, 0x18);
	sc020hgs_write_register(ViPipe, 0x34f5, 0x38);
	sc020hgs_write_register(ViPipe, 0x3618, 0x92);
	sc020hgs_write_register(ViPipe, 0x3619, 0x01);
	sc020hgs_write_register(ViPipe, 0x3630, 0x46);
	sc020hgs_write_register(ViPipe, 0x3631, 0x76);
	sc020hgs_write_register(ViPipe, 0x3632, 0x1c);
	sc020hgs_write_register(ViPipe, 0x3633, 0xa6);
	sc020hgs_write_register(ViPipe, 0x3634, 0x90);
	sc020hgs_write_register(ViPipe, 0x3635, 0x36);
	sc020hgs_write_register(ViPipe, 0x3637, 0x0c);
	sc020hgs_write_register(ViPipe, 0x3638, 0x04);
	sc020hgs_write_register(ViPipe, 0x3670, 0x72);
	sc020hgs_write_register(ViPipe, 0x3671, 0x62);
	sc020hgs_write_register(ViPipe, 0x3672, 0x64);
	sc020hgs_write_register(ViPipe, 0x3673, 0x08);
	sc020hgs_write_register(ViPipe, 0x3674, 0x18);
	sc020hgs_write_register(ViPipe, 0x3675, 0x74);
	sc020hgs_write_register(ViPipe, 0x3676, 0x74);
	sc020hgs_write_register(ViPipe, 0x3677, 0x74);
	sc020hgs_write_register(ViPipe, 0x3678, 0x08);
	sc020hgs_write_register(ViPipe, 0x3679, 0x18);
	sc020hgs_write_register(ViPipe, 0x368f, 0x0c);
	sc020hgs_write_register(ViPipe, 0x3690, 0x0c);
	sc020hgs_write_register(ViPipe, 0x3691, 0x0c);
	sc020hgs_write_register(ViPipe, 0x3692, 0x08);
	sc020hgs_write_register(ViPipe, 0x3693, 0x18);
	sc020hgs_write_register(ViPipe, 0x3699, 0xc4);
	sc020hgs_write_register(ViPipe, 0x369a, 0xc5);
	sc020hgs_write_register(ViPipe, 0x369b, 0xc3);
	sc020hgs_write_register(ViPipe, 0x369c, 0xc4);
	sc020hgs_write_register(ViPipe, 0x369d, 0xce);
	sc020hgs_write_register(ViPipe, 0x369e, 0x08);
	sc020hgs_write_register(ViPipe, 0x369f, 0x18);
	sc020hgs_write_register(ViPipe, 0x36a0, 0x38);
	sc020hgs_write_register(ViPipe, 0x36a1, 0x78);
	sc020hgs_write_register(ViPipe, 0x36ea, 0x0a);
	sc020hgs_write_register(ViPipe, 0x36eb, 0x04);
	sc020hgs_write_register(ViPipe, 0x36ec, 0x45);
	sc020hgs_write_register(ViPipe, 0x36ed, 0xaa);
	sc020hgs_write_register(ViPipe, 0x3721, 0x8b);
	sc020hgs_write_register(ViPipe, 0x3723, 0xb1);
	sc020hgs_write_register(ViPipe, 0x3726, 0x80);
	sc020hgs_write_register(ViPipe, 0x3727, 0x18);
	sc020hgs_write_register(ViPipe, 0x391f, 0x41);
	sc020hgs_write_register(ViPipe, 0x3e00, 0x00);
	sc020hgs_write_register(ViPipe, 0x3e01, 0x35);
	sc020hgs_write_register(ViPipe, 0x3e02, 0xb0);
	sc020hgs_write_register(ViPipe, 0x3e03, 0x0b);
	sc020hgs_write_register(ViPipe, 0x4330, 0x18);
	sc020hgs_write_register(ViPipe, 0x4331, 0x02);
	sc020hgs_write_register(ViPipe, 0x4350, 0x02);
	sc020hgs_write_register(ViPipe, 0x435c, 0x0d);
	sc020hgs_write_register(ViPipe, 0x4360, 0x1f);
	sc020hgs_write_register(ViPipe, 0x4361, 0x00);
	sc020hgs_write_register(ViPipe, 0x4362, 0x60);
	sc020hgs_write_register(ViPipe, 0x4363, 0x00);
	sc020hgs_write_register(ViPipe, 0x4364, 0x60);
	sc020hgs_write_register(ViPipe, 0x4365, 0x18);
	sc020hgs_write_register(ViPipe, 0x4366, 0x38);
	sc020hgs_write_register(ViPipe, 0x4367, 0x00);
	sc020hgs_write_register(ViPipe, 0x4368, 0x30);
	sc020hgs_write_register(ViPipe, 0x4369, 0x00);
	sc020hgs_write_register(ViPipe, 0x436a, 0x30);
	sc020hgs_write_register(ViPipe, 0x436b, 0x18);
	sc020hgs_write_register(ViPipe, 0x436c, 0x38);
	sc020hgs_write_register(ViPipe, 0x436d, 0x78);
	sc020hgs_write_register(ViPipe, 0x4370, 0x02);
	sc020hgs_write_register(ViPipe, 0x4371, 0x02);
	sc020hgs_write_register(ViPipe, 0x4372, 0x08);
	sc020hgs_write_register(ViPipe, 0x4373, 0x18);
	sc020hgs_write_register(ViPipe, 0x437a, 0x00);
	sc020hgs_write_register(ViPipe, 0x437b, 0x38);
	sc020hgs_write_register(ViPipe, 0x4380, 0x02);
	sc020hgs_write_register(ViPipe, 0x4381, 0x02);
	sc020hgs_write_register(ViPipe, 0x4382, 0x00);
	sc020hgs_write_register(ViPipe, 0x4383, 0x08);
	sc020hgs_write_register(ViPipe, 0x4384, 0x18);
	sc020hgs_write_register(ViPipe, 0x4385, 0x38);
	sc020hgs_write_register(ViPipe, 0x440e, 0x02);
	sc020hgs_write_register(ViPipe, 0x4509, 0x1c);
	sc020hgs_write_register(ViPipe, 0x4819, 0x08);
	sc020hgs_write_register(ViPipe, 0x481b, 0x05);
	sc020hgs_write_register(ViPipe, 0x481d, 0x11);
	sc020hgs_write_register(ViPipe, 0x481f, 0x04);
	sc020hgs_write_register(ViPipe, 0x4821, 0x09);
	sc020hgs_write_register(ViPipe, 0x4823, 0x04);
	sc020hgs_write_register(ViPipe, 0x4825, 0x04);
	sc020hgs_write_register(ViPipe, 0x4827, 0x04);
	sc020hgs_write_register(ViPipe, 0x4829, 0x07);
	sc020hgs_write_register(ViPipe, 0x36e9, 0x20);
	sc020hgs_default_reg_init(ViPipe);

	delay_ms(100);

	sc020hgs_write_register(ViPipe, 0x2100, 0x01);



	printf("ViPipe:%d,===SC020HGS 400P 120fps 10bit LINE Init OK!===\n", ViPipe);
}
