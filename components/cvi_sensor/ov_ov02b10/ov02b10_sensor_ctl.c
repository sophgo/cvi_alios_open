#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "ov02b10_cmos_ex.h"

static void ov02b10_linear_1200p30_init(VI_PIPE ViPipe);
static void ov02b10_linear_600p60_init(VI_PIPE ViPipe);

CVI_U8 ov02b10_i2c_addr = 0x3C;        /* I2C Address of OV02B10 */
const CVI_U32 ov02b10_addr_byte = 1;
const CVI_U32 ov02b10_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int ov02b10_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOv02b10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int ov02b10_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOv02b10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int ov02b10_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOv02b10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, ov02b10_i2c_addr, (CVI_U32)addr, ov02b10_addr_byte, ov02b10_data_byte);

}

int ov02b10_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOv02b10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, ov02b10_i2c_addr, (CVI_U32)addr, ov02b10_addr_byte,
				(CVI_U32)data, ov02b10_data_byte);

}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void ov02b10_standby(VI_PIPE ViPipe)
{
	ov02b10_write_register(ViPipe, 0xFB, 0x00); /* STANDBY */
}

void ov02b10_restart(VI_PIPE ViPipe)
{
	ov02b10_write_register(ViPipe, 0xFB, 0x01); /* standby */
}

void ov02b10_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;
	CVI_U32 start = 0;
	CVI_U32 end = g_pastOv02b10[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum - 1;
	for (i = start; i < end; i++) {
		ov02b10_write_register(ViPipe,
				g_pastOv02b10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOv02b10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define OV02B10_FLIP	0x12
#define OV02B10_MIRROR	0x12
void ov02b10_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 flip, mirror;

	flip = ov02b10_read_register(ViPipe, OV02B10_FLIP);
	mirror = ov02b10_read_register(ViPipe, OV02B10_MIRROR);
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

	ov02b10_write_register(ViPipe, OV02B10_FLIP, flip);
	ov02b10_write_register(ViPipe, OV02B10_MIRROR, mirror);
}

#define OV02B10_CHIP_ID_ADDR_H		0x02
#define OV02B10_CHIP_ID_ADDR_L		0x03
#define OV02B10_CHIP_ID			0x2B

int ov02b10_probe(VI_PIPE ViPipe)
{
	int nVal, nVal2;

	// usleep(1000);
	if (ov02b10_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = ov02b10_read_register(ViPipe, OV02B10_CHIP_ID_ADDR_H);
	nVal2 = ov02b10_read_register(ViPipe, OV02B10_CHIP_ID_ADDR_L);

	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != OV02B10_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

void ov02b10_init(VI_PIPE ViPipe)
{
	CVI_U8		u8ImgMode = g_pastOv02b10[ViPipe]->u8ImgMode;
	WDR_MODE_E	enWDRMode =	g_pastOv02b10[ViPipe]->enWDRMode;
	// ov02b10_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
		if (u8ImgMode == OV02B10_MODE_1600X1200P30)
			ov02b10_linear_1200p30_init(ViPipe);
		else if(u8ImgMode == OV02B10_MODE_800X600P60)
			ov02b10_linear_600p60_init(ViPipe);
		else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this ImgMode[%d]!\n",u8ImgMode);
			return ;
		}
	}

	g_pastOv02b10[ViPipe]->bInit = CVI_TRUE;
}

void ov02b10_exit(VI_PIPE ViPipe)
{
	ov02b10_i2c_exit(ViPipe);
}

/* 1200P30 */
static void ov02b10_linear_1200p30_init(VI_PIPE ViPipe)
{
	ov02b10_write_register(ViPipe, 0xfc, 0x01);
	ov02b10_write_register(ViPipe, 0xfd, 0x00);
	ov02b10_write_register(ViPipe, 0xfd, 0x00);
	ov02b10_write_register(ViPipe, 0x24, 0x02);
	ov02b10_write_register(ViPipe, 0x25, 0x06);
	ov02b10_write_register(ViPipe, 0x29, 0x03);
	ov02b10_write_register(ViPipe, 0x2a, 0x34);
	ov02b10_write_register(ViPipe, 0x1e, 0x17);
	ov02b10_write_register(ViPipe, 0x33, 0x07);
	ov02b10_write_register(ViPipe, 0x35, 0x07);
	ov02b10_write_register(ViPipe, 0x4a, 0x0c);
	ov02b10_write_register(ViPipe, 0x3a, 0x05);
	ov02b10_write_register(ViPipe, 0x3b, 0x02);
	ov02b10_write_register(ViPipe, 0x3e, 0x00);
	ov02b10_write_register(ViPipe, 0x46, 0x01);
	ov02b10_write_register(ViPipe, 0x6d, 0x03);
	ov02b10_write_register(ViPipe, 0xfd, 0x01);
	ov02b10_write_register(ViPipe, 0x0e, 0x02);
	ov02b10_write_register(ViPipe, 0x0f, 0x1a);
	ov02b10_write_register(ViPipe, 0x18, 0x00);
	ov02b10_write_register(ViPipe, 0x22, 0xff);
	ov02b10_write_register(ViPipe, 0x23, 0x02);
	ov02b10_write_register(ViPipe, 0x17, 0x2c);
	ov02b10_write_register(ViPipe, 0x19, 0x20);
	ov02b10_write_register(ViPipe, 0x1b, 0x06);
	ov02b10_write_register(ViPipe, 0x1c, 0x04);
	ov02b10_write_register(ViPipe, 0x20, 0x03);
	ov02b10_write_register(ViPipe, 0x30, 0x01);
	ov02b10_write_register(ViPipe, 0x33, 0x01);
	ov02b10_write_register(ViPipe, 0x31, 0x0a);
	ov02b10_write_register(ViPipe, 0x32, 0x09);
	ov02b10_write_register(ViPipe, 0x38, 0x01);
	ov02b10_write_register(ViPipe, 0x39, 0x01);
	ov02b10_write_register(ViPipe, 0x3a, 0x01);
	ov02b10_write_register(ViPipe, 0x3b, 0x01);
	ov02b10_write_register(ViPipe, 0x4f, 0x04);
	ov02b10_write_register(ViPipe, 0x4e, 0x05);
	ov02b10_write_register(ViPipe, 0x50, 0x01);
	ov02b10_write_register(ViPipe, 0x35, 0x0c);
	ov02b10_write_register(ViPipe, 0x45, 0x2a);
	ov02b10_write_register(ViPipe, 0x46, 0x2a);
	ov02b10_write_register(ViPipe, 0x47, 0x2a);
	ov02b10_write_register(ViPipe, 0x48, 0x2a);
	ov02b10_write_register(ViPipe, 0x4a, 0x2c);
	ov02b10_write_register(ViPipe, 0x4b, 0x2c);
	ov02b10_write_register(ViPipe, 0x4c, 0x2c);
	ov02b10_write_register(ViPipe, 0x4d, 0x2c);
	ov02b10_write_register(ViPipe, 0x56, 0x3a);
	ov02b10_write_register(ViPipe, 0x57, 0x0a);
	ov02b10_write_register(ViPipe, 0x58, 0x24);
	ov02b10_write_register(ViPipe, 0x59, 0x20);
	ov02b10_write_register(ViPipe, 0x5a, 0x0a);
	ov02b10_write_register(ViPipe, 0x5b, 0xff);
	ov02b10_write_register(ViPipe, 0x37, 0x0a);
	ov02b10_write_register(ViPipe, 0x42, 0x0e);
	ov02b10_write_register(ViPipe, 0x68, 0x90);
	ov02b10_write_register(ViPipe, 0x69, 0xcd);
	ov02b10_write_register(ViPipe, 0x6a, 0x8f);
	ov02b10_write_register(ViPipe, 0x7c, 0x0a);
	ov02b10_write_register(ViPipe, 0x7d, 0x0a);
	ov02b10_write_register(ViPipe, 0x7e, 0x0a);
	ov02b10_write_register(ViPipe, 0x7f, 0x08);
	ov02b10_write_register(ViPipe, 0x83, 0x14);
	ov02b10_write_register(ViPipe, 0x84, 0x14);
	ov02b10_write_register(ViPipe, 0x86, 0x14);
	ov02b10_write_register(ViPipe, 0x87, 0x07);
	ov02b10_write_register(ViPipe, 0x88, 0x0f);
	ov02b10_write_register(ViPipe, 0x94, 0x02);
	ov02b10_write_register(ViPipe, 0x98, 0xd1);
	ov02b10_write_register(ViPipe, 0xfe, 0x02);
	ov02b10_write_register(ViPipe, 0xfd, 0x03);
	ov02b10_write_register(ViPipe, 0x97, 0x6c);
	ov02b10_write_register(ViPipe, 0x98, 0x60);
	ov02b10_write_register(ViPipe, 0x99, 0x60);
	ov02b10_write_register(ViPipe, 0x9a, 0x6c);
	ov02b10_write_register(ViPipe, 0xa1, 0x40);
	ov02b10_write_register(ViPipe, 0xaf, 0x04);
	ov02b10_write_register(ViPipe, 0xb1, 0x40);
	ov02b10_write_register(ViPipe, 0xae, 0x0d);
	ov02b10_write_register(ViPipe, 0x88, 0x5b);
	ov02b10_write_register(ViPipe, 0x89, 0x7c);
	ov02b10_write_register(ViPipe, 0xb4, 0x05);
	ov02b10_write_register(ViPipe, 0x8c, 0x40);
	ov02b10_write_register(ViPipe, 0x8e, 0x40);
	ov02b10_write_register(ViPipe, 0x90, 0x40);
	ov02b10_write_register(ViPipe, 0x92, 0x40);
	ov02b10_write_register(ViPipe, 0x9b, 0x46);
	ov02b10_write_register(ViPipe, 0xac, 0x40);
	ov02b10_write_register(ViPipe, 0xfd, 0x00);
	ov02b10_write_register(ViPipe, 0x5a, 0x15);
	ov02b10_write_register(ViPipe, 0x74, 0x01);
	ov02b10_write_register(ViPipe, 0xfd, 0x00);
	ov02b10_write_register(ViPipe, 0x50, 0x40);
	ov02b10_write_register(ViPipe, 0x52, 0xb0);
	ov02b10_write_register(ViPipe, 0xfd, 0x01);
	ov02b10_write_register(ViPipe, 0x03, 0x70);
	ov02b10_write_register(ViPipe, 0x05, 0x10);
	ov02b10_write_register(ViPipe, 0x07, 0x20);
	ov02b10_write_register(ViPipe, 0x09, 0xb0);
	ov02b10_write_register(ViPipe, 0xfd, 0x03);
	ov02b10_write_register(ViPipe, 0xc2, 0x01);
	ov02b10_write_register(ViPipe, 0xfb, 0x01);
	ov02b10_write_register(ViPipe, 0xfd ,0x01);

	ov02b10_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===OV02B10 1200P 30fps 10bit LINE Init OK!\n", ViPipe);
}

/* 600P30 */
static void ov02b10_linear_600p60_init(VI_PIPE ViPipe)
{
	ov02b10_write_register(ViPipe, 0xfc ,0x01);
	ov02b10_write_register(ViPipe, 0xfd ,0x00);
	ov02b10_write_register(ViPipe, 0xfd ,0x00);
	ov02b10_write_register(ViPipe, 0x24 ,0x02);
	ov02b10_write_register(ViPipe, 0x25 ,0x06);
	ov02b10_write_register(ViPipe, 0x29 ,0x03);
	ov02b10_write_register(ViPipe, 0x2a ,0xb4);
	ov02b10_write_register(ViPipe, 0x1e ,0x17);
	ov02b10_write_register(ViPipe, 0x33 ,0x07);
	ov02b10_write_register(ViPipe, 0x35 ,0x02);
	ov02b10_write_register(ViPipe, 0x4a ,0x0c);
	ov02b10_write_register(ViPipe, 0x3a ,0x05);
	ov02b10_write_register(ViPipe, 0x3b ,0x02);
	ov02b10_write_register(ViPipe, 0x3e ,0x00);
	ov02b10_write_register(ViPipe, 0x46 ,0x01);
	ov02b10_write_register(ViPipe, 0xfd ,0x01);
	ov02b10_write_register(ViPipe, 0x0e ,0x02);
	ov02b10_write_register(ViPipe, 0x0f ,0x1a);
	ov02b10_write_register(ViPipe, 0x18 ,0x00);
	ov02b10_write_register(ViPipe, 0x22 ,0xff);
	ov02b10_write_register(ViPipe, 0x23 ,0x02);
	ov02b10_write_register(ViPipe, 0x17 ,0x2c);
	ov02b10_write_register(ViPipe, 0x19 ,0x20);
	ov02b10_write_register(ViPipe, 0x1b ,0x06);
	ov02b10_write_register(ViPipe, 0x1c ,0x04);
	ov02b10_write_register(ViPipe, 0x20 ,0x03);
	ov02b10_write_register(ViPipe, 0x30 ,0x01);
	ov02b10_write_register(ViPipe, 0x33 ,0x01);
	ov02b10_write_register(ViPipe, 0x31 ,0x0a);
	ov02b10_write_register(ViPipe, 0x32 ,0x09);
	ov02b10_write_register(ViPipe, 0x38 ,0x01);
	ov02b10_write_register(ViPipe, 0x39 ,0x01);
	ov02b10_write_register(ViPipe, 0x3a ,0x01);
	ov02b10_write_register(ViPipe, 0x3b ,0x01);
	ov02b10_write_register(ViPipe, 0x4f ,0x04);
	ov02b10_write_register(ViPipe, 0x4e ,0x05);
	ov02b10_write_register(ViPipe, 0x50 ,0x01);
	ov02b10_write_register(ViPipe, 0x35 ,0x0c);
	ov02b10_write_register(ViPipe, 0x45 ,0x2a);
	ov02b10_write_register(ViPipe, 0x46 ,0x2a);
	ov02b10_write_register(ViPipe, 0x47 ,0x2a);
	ov02b10_write_register(ViPipe, 0x48 ,0x2a);
	ov02b10_write_register(ViPipe, 0x4a ,0x2c);
	ov02b10_write_register(ViPipe, 0x4b ,0x2c);
	ov02b10_write_register(ViPipe, 0x4c ,0x2c);
	ov02b10_write_register(ViPipe, 0x4d ,0x2c);
	ov02b10_write_register(ViPipe, 0x56 ,0x3a);
	ov02b10_write_register(ViPipe, 0x57 ,0x0a);
	ov02b10_write_register(ViPipe, 0x58 ,0x24);
	ov02b10_write_register(ViPipe, 0x59 ,0x20);
	ov02b10_write_register(ViPipe, 0x5a ,0x0a);
	ov02b10_write_register(ViPipe, 0x5b ,0xff);
	ov02b10_write_register(ViPipe, 0x37 ,0x0a);
	ov02b10_write_register(ViPipe, 0x42 ,0x0e);
	ov02b10_write_register(ViPipe, 0x68 ,0x90);
	ov02b10_write_register(ViPipe, 0x69 ,0xcd);
	ov02b10_write_register(ViPipe, 0x7c ,0x0b);
	ov02b10_write_register(ViPipe, 0x7d ,0x0b);
	ov02b10_write_register(ViPipe, 0x7e ,0x0b);
	ov02b10_write_register(ViPipe, 0x7f ,0x0b);
	ov02b10_write_register(ViPipe, 0x83 ,0x14);
	ov02b10_write_register(ViPipe, 0x84 ,0x14);
	ov02b10_write_register(ViPipe, 0x86 ,0x14);
	ov02b10_write_register(ViPipe, 0x87 ,0x07);
	ov02b10_write_register(ViPipe, 0x88 ,0x0f);
	ov02b10_write_register(ViPipe, 0x94 ,0x02);
	ov02b10_write_register(ViPipe, 0x98 ,0xd1);
	ov02b10_write_register(ViPipe, 0xfe ,0x02);
	ov02b10_write_register(ViPipe, 0xfd ,0x03);
	ov02b10_write_register(ViPipe, 0xae ,0x0d);
	ov02b10_write_register(ViPipe, 0x88 ,0x59);
	ov02b10_write_register(ViPipe, 0x89 ,0x7c);
	ov02b10_write_register(ViPipe, 0xb4 ,0x05);
	ov02b10_write_register(ViPipe, 0x8c ,0x40);
	ov02b10_write_register(ViPipe, 0x8e ,0x40);
	ov02b10_write_register(ViPipe, 0x90 ,0x40);
	ov02b10_write_register(ViPipe, 0x92 ,0x40);
	ov02b10_write_register(ViPipe, 0x9b ,0x40);
	ov02b10_write_register(ViPipe, 0xac ,0x40);
	ov02b10_write_register(ViPipe, 0xfd ,0x00);
	ov02b10_write_register(ViPipe, 0x5a ,0x15);
	ov02b10_write_register(ViPipe, 0x74 ,0x01);
	ov02b10_write_register(ViPipe, 0xfd ,0x00);
	ov02b10_write_register(ViPipe, 0x28 ,0x03);
	ov02b10_write_register(ViPipe, 0x4f ,0x03);
	ov02b10_write_register(ViPipe, 0x50 ,0x20);
	ov02b10_write_register(ViPipe, 0x51 ,0x02);
	ov02b10_write_register(ViPipe, 0x52 ,0x58);
	ov02b10_write_register(ViPipe, 0xfd ,0x01);
	ov02b10_write_register(ViPipe, 0x03 ,0x70);
	ov02b10_write_register(ViPipe, 0x05 ,0x10);
	ov02b10_write_register(ViPipe, 0x07 ,0x20);
	ov02b10_write_register(ViPipe, 0x09 ,0xb0);
	ov02b10_write_register(ViPipe, 0x6c ,0x09);
	ov02b10_write_register(ViPipe, 0xfe ,0x02);
	ov02b10_write_register(ViPipe, 0xfb ,0x01);
	ov02b10_write_register(ViPipe, 0xfd ,0x03);
	ov02b10_write_register(ViPipe, 0xc2 ,0x01);
	ov02b10_write_register(ViPipe, 0xfd ,0x01);

	ov02b10_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===OV02B10 600P 60fps 10bit LINE Init OK!\n", ViPipe);
}

