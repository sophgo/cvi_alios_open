#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "k06_cmos_ex.h"
#include "sensor_i2c.h"

#define K06_CHIP_ID_HI_ADDR		0x0A
#define K06_CHIP_ID_LO_ADDR		0x0B
#define K06_CHIP_ID			0x0852

static void k06_linear_1440p25_init(VI_PIPE ViPipe);

CVI_U8 k06_i2c_addr = 0x40;        /* I2C Address of K06 */
const CVI_U32 k06_addr_byte = 1;
const CVI_U32 k06_data_byte = 1;

int k06_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunK06_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunK06_AddrInfo[ViPipe].s8I2cAddr);
}

int k06_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunK06_BusInfo[ViPipe].s8I2cDev);
}

int k06_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunK06_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunK06_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							k06_addr_byte, k06_addr_byte);
}

int k06_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunK06_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunK06_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							k06_addr_byte, (CVI_U32)data, k06_addr_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void k06_standby(VI_PIPE ViPipe)
{
	k06_write_register(ViPipe, 0x12, 0x40);
}

void k06_restart(VI_PIPE ViPipe)
{
	k06_write_register(ViPipe, 0x12, 0x40);
	delay_ms(20);
	k06_write_register(ViPipe, 0x12, 0x00);
}

void k06_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastK06[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		k06_write_register(ViPipe,
				g_pastK06[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastK06[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void k06_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = k06_read_register(ViPipe, 0x12) & (~0x30);

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		val |= 0x30;
		break;
	case ISP_SNS_MIRROR:
		val |= (0x1 << 4);
		break;
	case ISP_SNS_FLIP:
		val |= (0x2 << 4);
		break;
	case ISP_SNS_MIRROR_FLIP:
		val &= ~(0x1 << 4);
		break;
	default:
		return;
	}

	k06_write_register(ViPipe, 0x12, val);
}

int k06_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	usleep(4*1000);
	if (k06_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = k06_read_register(ViPipe, K06_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = k06_read_register(ViPipe, K06_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != K06_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void k06_init(VI_PIPE ViPipe)
{
	k06_i2c_init(ViPipe);

	k06_linear_1440p25_init(ViPipe);
	g_pastK06[ViPipe]->bInit = CVI_TRUE;
}

static void k06_linear_1440p25_init(VI_PIPE ViPipe)
{
	k06_write_register(ViPipe, 0x12, 0x70);
	k06_write_register(ViPipe, 0x48, 0x86);
	k06_write_register(ViPipe, 0x48, 0x06);
	k06_write_register(ViPipe, 0x0E, 0x11);
	k06_write_register(ViPipe, 0x0F, 0x04);
	k06_write_register(ViPipe, 0x10, 0x48);
	k06_write_register(ViPipe, 0x11, 0x80);
	k06_write_register(ViPipe, 0x46, 0x08);
	k06_write_register(ViPipe, 0x0D, 0xA0);
	k06_write_register(ViPipe, 0x57, 0x67);
	k06_write_register(ViPipe, 0x58, 0x1F);
	k06_write_register(ViPipe, 0x5F, 0x41);
	k06_write_register(ViPipe, 0x60, 0x20);
	k06_write_register(ViPipe, 0x20, 0x80);
	k06_write_register(ViPipe, 0x21, 0x04);
	k06_write_register(ViPipe, 0x22, 0xDC);
	k06_write_register(ViPipe, 0x23, 0x05);
	k06_write_register(ViPipe, 0x24, 0x80);
	k06_write_register(ViPipe, 0x25, 0xA0);
	k06_write_register(ViPipe, 0x26, 0x52);
	k06_write_register(ViPipe, 0x27, 0x27);
	k06_write_register(ViPipe, 0x28, 0x15);
	k06_write_register(ViPipe, 0x29, 0x04);
	k06_write_register(ViPipe, 0x2A, 0x20);
	k06_write_register(ViPipe, 0x2B, 0x14);
	k06_write_register(ViPipe, 0x2C, 0x00);
	k06_write_register(ViPipe, 0x2D, 0x00);
	k06_write_register(ViPipe, 0x2E, 0x6E);
	k06_write_register(ViPipe, 0x2F, 0x04);
	k06_write_register(ViPipe, 0x41, 0x06);
	k06_write_register(ViPipe, 0x42, 0x05);
	k06_write_register(ViPipe, 0x47, 0x46);
	k06_write_register(ViPipe, 0x76, 0x80);
	k06_write_register(ViPipe, 0x77, 0x0C);
	k06_write_register(ViPipe, 0x80, 0x01);
	k06_write_register(ViPipe, 0xAF, 0x12);
	k06_write_register(ViPipe, 0xAA, 0x04);
	k06_write_register(ViPipe, 0x1D, 0x00);
	k06_write_register(ViPipe, 0x1E, 0x04);
	k06_write_register(ViPipe, 0x6C, 0x40);
	k06_write_register(ViPipe, 0x9E, 0xF8);
	k06_write_register(ViPipe, 0x0C, 0x30);
	k06_write_register(ViPipe, 0x6F, 0x80);
	k06_write_register(ViPipe, 0x6E, 0x2C);
	k06_write_register(ViPipe, 0x70, 0xF9);
	k06_write_register(ViPipe, 0x71, 0xDD);
	k06_write_register(ViPipe, 0x72, 0xD5);
	k06_write_register(ViPipe, 0x73, 0x5A);
	k06_write_register(ViPipe, 0x74, 0x02);
	k06_write_register(ViPipe, 0x78, 0x1D);
	k06_write_register(ViPipe, 0x89, 0x01);
	k06_write_register(ViPipe, 0x6B, 0x20);
	k06_write_register(ViPipe, 0x86, 0x40);
	k06_write_register(ViPipe, 0x30, 0x8D);
	k06_write_register(ViPipe, 0x31, 0x08);
	k06_write_register(ViPipe, 0x32, 0x20);
	k06_write_register(ViPipe, 0x33, 0x5C);
	k06_write_register(ViPipe, 0x34, 0x30);
	k06_write_register(ViPipe, 0x35, 0x30);
	k06_write_register(ViPipe, 0x3A, 0xB6);
	k06_write_register(ViPipe, 0x56, 0x92);
	k06_write_register(ViPipe, 0x59, 0x48);
	k06_write_register(ViPipe, 0x5A, 0x01);
	k06_write_register(ViPipe, 0x61, 0x00);
	k06_write_register(ViPipe, 0x64, 0xC0);
	k06_write_register(ViPipe, 0x7F, 0x46);
	k06_write_register(ViPipe, 0x85, 0x44);
	k06_write_register(ViPipe, 0x8A, 0x00);
	k06_write_register(ViPipe, 0x91, 0x58);
	k06_write_register(ViPipe, 0x94, 0xE0);
	k06_write_register(ViPipe, 0x9B, 0x8F);
	k06_write_register(ViPipe, 0xA6, 0x02);
	k06_write_register(ViPipe, 0xA7, 0xA0);
	k06_write_register(ViPipe, 0xA9, 0x48);
	k06_write_register(ViPipe, 0x45, 0x09);
	k06_write_register(ViPipe, 0x5B, 0xA5);
	k06_write_register(ViPipe, 0x5C, 0x8C);
	k06_write_register(ViPipe, 0x5D, 0x97);
	k06_write_register(ViPipe, 0x5E, 0x48);
	k06_write_register(ViPipe, 0x65, 0x32);
	k06_write_register(ViPipe, 0x66, 0x80);
	k06_write_register(ViPipe, 0x67, 0x44);
	k06_write_register(ViPipe, 0x68, 0x00);
	k06_write_register(ViPipe, 0x69, 0x74);
	k06_write_register(ViPipe, 0x6A, 0x2B);
	k06_write_register(ViPipe, 0x7A, 0x82);
	k06_write_register(ViPipe, 0x8D, 0x6F);
	k06_write_register(ViPipe, 0x8F, 0x90);
	k06_write_register(ViPipe, 0xA4, 0xC7);
	k06_write_register(ViPipe, 0xA5, 0xAF);
	k06_write_register(ViPipe, 0xB7, 0x21);
	k06_write_register(ViPipe, 0x97, 0x20);
	k06_write_register(ViPipe, 0x13, 0x81);
	k06_write_register(ViPipe, 0x96, 0x84);
	k06_write_register(ViPipe, 0x4A, 0x01);
	k06_write_register(ViPipe, 0x7E, 0x4C);
	k06_write_register(ViPipe, 0x50, 0x02);
	k06_write_register(ViPipe, 0x93, 0xC0);
	k06_write_register(ViPipe, 0xB5, 0x4C);
	k06_write_register(ViPipe, 0xB1, 0x00);
	k06_write_register(ViPipe, 0xA1, 0x0F);
	k06_write_register(ViPipe, 0xA3, 0x40);
	k06_write_register(ViPipe, 0x49, 0x10);
	k06_write_register(ViPipe, 0x8C, 0xFF);
	k06_write_register(ViPipe, 0x8E, 0x00);
	k06_write_register(ViPipe, 0x8B, 0x01);
	k06_write_register(ViPipe, 0xBC, 0x11);
	k06_write_register(ViPipe, 0x82, 0x00);
	k06_write_register(ViPipe, 0x9F, 0x50);
	k06_write_register(ViPipe, 0x19, 0x20);
	k06_write_register(ViPipe, 0x1B, 0x4F);

	k06_default_reg_init(ViPipe);
//	k06_write_register(ViPipe, 0x12, 0x00);
	k06_write_register(ViPipe, 0x12, 0x30);
	k06_write_register(ViPipe, 0x48, 0x86);
	k06_write_register(ViPipe, 0x48, 0x06);
	printf("ViPipe:%d,===K06 1440P 25fps 10bit LINE Init OK!===\n", ViPipe);
}

