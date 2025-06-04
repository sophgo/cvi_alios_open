#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "f37p_cmos_ex.h"
#include "sensor_i2c.h"

static void f37p_linear_1080p30_init(VI_PIPE ViPipe);

CVI_U8 f37p_i2c_addr = 0x40;        /* I2C Address of F37P */
const CVI_U32 f37p_addr_byte = 1;
const CVI_U32 f37p_data_byte = 1;
int f37p_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunF37P_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunF37P_AddrInfo[ViPipe].s8I2cAddr);
}

int f37p_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunF37P_BusInfo[ViPipe].s8I2cDev);
}

int f37p_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunF37P_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunF37P_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							f37p_addr_byte, f37p_data_byte);
}

int f37p_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunF37P_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunF37P_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							f37p_addr_byte, (CVI_U32)data, f37p_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void f37p_standby(VI_PIPE ViPipe)
{
	f37p_write_register(ViPipe, 0x12, 0x40);
}

void f37p_restart(VI_PIPE ViPipe)
{
	f37p_write_register(ViPipe, 0x12, 0x40);
	delay_ms(20);
	f37p_write_register(ViPipe, 0x12, 0x00);
}

void f37p_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastF37P[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		f37p_write_register(ViPipe,
				g_pastF37P[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastF37P[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void f37p_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = f37p_read_register(ViPipe, 0x12) & ~0x30;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x20;
		break;
	case ISP_SNS_FLIP:
		val |= 0x10;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x30;
		break;
	default:
		return;
	}

	f37p_write_register(ViPipe, 0x12, val);
}

#define F37P_CHIP_ID_HI_ADDR		0x0A
#define F37P_CHIP_ID_LO_ADDR		0x0B
#define F37P_CHIP_ID			0x0841

int f37p_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	if (f37p_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	delay_ms(4);

	nVal = f37p_read_register(ViPipe, F37P_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = f37p_read_register(ViPipe, F37P_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != F37P_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void f37p_init(VI_PIPE ViPipe)
{

	f37p_i2c_init(ViPipe);

	f37p_linear_1080p30_init(ViPipe);

	g_pastF37P[ViPipe]->bInit = CVI_TRUE;
}

/* 1080P30 and 1080P25 */
static void f37p_linear_1080p30_init(VI_PIPE ViPipe)
{
	f37p_write_register(ViPipe, 0x12, 0x40);
	f37p_write_register(ViPipe, 0x48, 0x8A);
	f37p_write_register(ViPipe, 0x48, 0x0A);
	f37p_write_register(ViPipe, 0x0E, 0x19);
	f37p_write_register(ViPipe, 0x0F, 0x04);
	f37p_write_register(ViPipe, 0x10, 0x20);
	f37p_write_register(ViPipe, 0x11, 0x80);
	f37p_write_register(ViPipe, 0x46, 0x09);
	f37p_write_register(ViPipe, 0x47, 0x66);
	f37p_write_register(ViPipe, 0x0D, 0xF2);
	f37p_write_register(ViPipe, 0x57, 0x6A);
	f37p_write_register(ViPipe, 0x58, 0x22);
	f37p_write_register(ViPipe, 0x5F, 0x41);
	f37p_write_register(ViPipe, 0x60, 0x24);
	f37p_write_register(ViPipe, 0xA5, 0xC0);
	f37p_write_register(ViPipe, 0x20, 0x00);
	f37p_write_register(ViPipe, 0x21, 0x05);
	f37p_write_register(ViPipe, 0x22, 0x65);
	f37p_write_register(ViPipe, 0x23, 0x04);
	f37p_write_register(ViPipe, 0x24, 0xC0);
	f37p_write_register(ViPipe, 0x25, 0x38);
	f37p_write_register(ViPipe, 0x26, 0x43);
	f37p_write_register(ViPipe, 0x27, 0xC6);
	f37p_write_register(ViPipe, 0x28, 0x15);
	f37p_write_register(ViPipe, 0x29, 0x04);
	f37p_write_register(ViPipe, 0x2A, 0xBB);
	f37p_write_register(ViPipe, 0x2B, 0x14);
	f37p_write_register(ViPipe, 0x2C, 0x02);
	f37p_write_register(ViPipe, 0x2D, 0x00);
	f37p_write_register(ViPipe, 0x2E, 0x14);
	f37p_write_register(ViPipe, 0x2F, 0x04);
	f37p_write_register(ViPipe, 0x41, 0xC5);
	f37p_write_register(ViPipe, 0x42, 0x33);
	f37p_write_register(ViPipe, 0x47, 0x46);
	f37p_write_register(ViPipe, 0x76, 0x60);
	f37p_write_register(ViPipe, 0x77, 0x09);
	f37p_write_register(ViPipe, 0x80, 0x01);
	f37p_write_register(ViPipe, 0xAF, 0x22);
	f37p_write_register(ViPipe, 0xAB, 0x00);
	f37p_write_register(ViPipe, 0x1D, 0x00);
	f37p_write_register(ViPipe, 0x1E, 0x04);
	f37p_write_register(ViPipe, 0x6C, 0x40);
	f37p_write_register(ViPipe, 0x9E, 0xF8);
	f37p_write_register(ViPipe, 0x6E, 0x2C);
	f37p_write_register(ViPipe, 0x70, 0x6C);
	f37p_write_register(ViPipe, 0x71, 0x6D);
	f37p_write_register(ViPipe, 0x72, 0x6A);
	f37p_write_register(ViPipe, 0x73, 0x56);
	f37p_write_register(ViPipe, 0x74, 0x02);
	f37p_write_register(ViPipe, 0x78, 0x9D);
	f37p_write_register(ViPipe, 0x89, 0x01);
	f37p_write_register(ViPipe, 0x6B, 0x20);
	f37p_write_register(ViPipe, 0x86, 0x40);
	f37p_write_register(ViPipe, 0x31, 0x10);
	f37p_write_register(ViPipe, 0x32, 0x18);
	f37p_write_register(ViPipe, 0x33, 0xE8);
	f37p_write_register(ViPipe, 0x34, 0x5E);
	f37p_write_register(ViPipe, 0x35, 0x5E);
	f37p_write_register(ViPipe, 0x3A, 0xAF);
	f37p_write_register(ViPipe, 0x3B, 0x00);
	f37p_write_register(ViPipe, 0x3C, 0xFF);
	f37p_write_register(ViPipe, 0x3D, 0xFF);
	f37p_write_register(ViPipe, 0x3E, 0xFF);
	f37p_write_register(ViPipe, 0x3F, 0xBB);
	f37p_write_register(ViPipe, 0x40, 0xFF);
	f37p_write_register(ViPipe, 0x56, 0x92);
	f37p_write_register(ViPipe, 0x59, 0xAF);
	f37p_write_register(ViPipe, 0x5A, 0x47);
	f37p_write_register(ViPipe, 0x61, 0x18);
	f37p_write_register(ViPipe, 0x6F, 0x04);
	f37p_write_register(ViPipe, 0x85, 0x5F);
	f37p_write_register(ViPipe, 0x8A, 0x44);
	f37p_write_register(ViPipe, 0x91, 0x13);
	f37p_write_register(ViPipe, 0x94, 0xA0);
	f37p_write_register(ViPipe, 0x9B, 0x83);
	f37p_write_register(ViPipe, 0x9C, 0xE1);
	f37p_write_register(ViPipe, 0xA4, 0x80);
	f37p_write_register(ViPipe, 0xA6, 0x22);
	f37p_write_register(ViPipe, 0xA9, 0x1C);
	f37p_write_register(ViPipe, 0x5B, 0xE7);
	f37p_write_register(ViPipe, 0x5C, 0x28);
	f37p_write_register(ViPipe, 0x5D, 0x67);
	f37p_write_register(ViPipe, 0x5E, 0x11);
	f37p_write_register(ViPipe, 0x62, 0x21);
	f37p_write_register(ViPipe, 0x63, 0x0F);
	f37p_write_register(ViPipe, 0x64, 0xD0);
	f37p_write_register(ViPipe, 0x65, 0x02);
	f37p_write_register(ViPipe, 0x67, 0x49);
	f37p_write_register(ViPipe, 0x66, 0x00);
	f37p_write_register(ViPipe, 0x68, 0x00);
	f37p_write_register(ViPipe, 0x69, 0x72);
	f37p_write_register(ViPipe, 0x6A, 0x12);
	f37p_write_register(ViPipe, 0x7A, 0x00);
	f37p_write_register(ViPipe, 0x82, 0x20);
	f37p_write_register(ViPipe, 0x8D, 0x47);
	f37p_write_register(ViPipe, 0x8F, 0x90);
	f37p_write_register(ViPipe, 0x45, 0x01);
	f37p_write_register(ViPipe, 0x97, 0x20);
	f37p_write_register(ViPipe, 0x13, 0x81);
	f37p_write_register(ViPipe, 0x96, 0x84);
	f37p_write_register(ViPipe, 0x4A, 0x01);
	f37p_write_register(ViPipe, 0xB1, 0x00);
	f37p_write_register(ViPipe, 0xA1, 0x0F);
	f37p_write_register(ViPipe, 0xBE, 0x00);
	f37p_write_register(ViPipe, 0x7E, 0x48);
	f37p_write_register(ViPipe, 0xB5, 0xC0);
	f37p_write_register(ViPipe, 0x50, 0x02);
	f37p_write_register(ViPipe, 0x49, 0x10);
	f37p_write_register(ViPipe, 0x7F, 0x57);
	f37p_write_register(ViPipe, 0x90, 0x00);
	f37p_write_register(ViPipe, 0x7B, 0x4A);
	f37p_write_register(ViPipe, 0x7C, 0x0C);
	f37p_write_register(ViPipe, 0x8C, 0xFF);
	f37p_write_register(ViPipe, 0x8E, 0x00);
	f37p_write_register(ViPipe, 0x8B, 0x01);
	f37p_write_register(ViPipe, 0x0C, 0x00);
	f37p_write_register(ViPipe, 0xBC, 0x11);
	f37p_write_register(ViPipe, 0x19, 0x20);
	f37p_write_register(ViPipe, 0x1B, 0x4F);

	f37p_default_reg_init(ViPipe);

	f37p_write_register(ViPipe, 0x12, 0x00);

	delay_ms(80);

	printf("ViPipe:%d,===F37P 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

