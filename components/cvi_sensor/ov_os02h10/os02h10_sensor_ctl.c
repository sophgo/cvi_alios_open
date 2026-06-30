#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "os02h10_cmos_ex.h"

#ifndef OS02H10_CHIP_VERSION_1C
static void os02h10_linear_1080p30_init(VI_PIPE ViPipe);
#else
static void os02h10_1c_linear_1080p30_init(VI_PIPE ViPipe);
#endif

#define OS02H10_I2C_RETRY_CNT		3
#define OS02H10_I2C_RETRY_DELAY_US	2000
#define OS02H10_I2C_WRITE_GAP_US	800
#define OS02H10_PAGE_SWITCH_DELAY_US	2000
#define OS02H10_MIPI_ENABLE_DELAY_US	5000

CVI_U8 os02h10_i2c_addr = 0x3c;
const CVI_U32 os02h10_addr_byte = 1;
const CVI_U32 os02h10_data_byte = 1;

int os02h10_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOs02h10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int os02h10_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOs02h10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int os02h10_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOs02h10_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, os02h10_i2c_addr, (CVI_U32)addr, os02h10_addr_byte, os02h10_data_byte);

}

int os02h10_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunOs02h10_BusInfo[ViPipe].s8I2cDev;
	int retry;
	int ret = CVI_FAILURE;

	for (retry = 0; retry < OS02H10_I2C_RETRY_CNT; retry++) {
		ret = sensor_i2c_write(i2c_id, os02h10_i2c_addr, (CVI_U32)addr, os02h10_addr_byte,
				(CVI_U32)data, os02h10_data_byte);
		if (ret == CVI_SUCCESS)
			break;
		usleep(OS02H10_I2C_RETRY_DELAY_US);
	}

	if (ret != CVI_SUCCESS) {
		printf("os02h10 i2c write failed: pipe=%d i2c=%d reg=0x%02x val=0x%02x\n",
			ViPipe, i2c_id, addr, data);
		return ret;
	}

	if (addr == 0xfd) {
		usleep(OS02H10_PAGE_SWITCH_DELAY_US);
	} else if ((addr == 0xb1) && (data == 0x02)) {
		usleep(OS02H10_MIPI_ENABLE_DELAY_US);
	} else {
		usleep(OS02H10_I2C_WRITE_GAP_US);
	}

	return CVI_SUCCESS;
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void os02h10_standby(VI_PIPE ViPipe)
{
	printf("os02h10 standby.\n");
}

void os02h10_restart(VI_PIPE ViPipe)
{
	printf("os02h10 restart.\n");
}

void os02h10_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;
	for (i = 0; i < g_pastOs02h10[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastOs02h10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			os02h10_write_register(ViPipe,
				g_pastOs02h10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastOs02h10[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

#define OS02H10_CHIP_ID_ADDR_0		0x02
#define OS02H10_CHIP_ID_ADDR_1		0x03
#define OS02H10_CHIP_ID_ADDR_2		0x04
#define OS02H10_CHIP_ID_ADDR_3		0x05
#define OS02H10_CHIP_ID			0x53024810

int os02h10_probe(VI_PIPE ViPipe)
{
	int nVal0, nVal1, nVal2, nVal3;
	CVI_U32 chip_id;

	usleep(1000);
	if (os02h10_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal0  = os02h10_read_register(ViPipe, OS02H10_CHIP_ID_ADDR_0);
	nVal1 = os02h10_read_register(ViPipe, OS02H10_CHIP_ID_ADDR_1);
	nVal2  = os02h10_read_register(ViPipe, OS02H10_CHIP_ID_ADDR_2);
	nVal3 = os02h10_read_register(ViPipe, OS02H10_CHIP_ID_ADDR_3);
	if (nVal0 < 0 || nVal1 < 0 || nVal2 < 0 || nVal3 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return CVI_FAILURE;
	}
	chip_id = (nVal0 << 24) | (nVal1 << 16) | (nVal2 << 8) | nVal3;
	if (chip_id != OS02H10_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

void os02h10_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastOs02h10[ViPipe]->enWDRMode;

	os02h10_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "not surpport this WDR_MODE_E!\n");
	} else {
#if OS02H10_CHIP_VERSION_1C
		os02h10_1c_linear_1080p30_init(ViPipe);
#else
		os02h10_linear_1080p30_init(ViPipe);
#endif
	}
	g_pastOs02h10[ViPipe]->bInit = CVI_TRUE;
}

void os02h10_exit(VI_PIPE ViPipe)
{
	os02h10_i2c_exit(ViPipe);
}

/* 1080P30 */
#ifndef OS02H10_CHIP_VERSION_1C
static void os02h10_linear_1080p30_init(VI_PIPE ViPipe)
{
	delay_ms(5);
	os02h10_write_register(ViPipe, 0xfd, 0x00);
	os02h10_write_register(ViPipe, 0x20, 0x00);
	os02h10_write_register(ViPipe, 0x53, 0xfe);
	os02h10_write_register(ViPipe, 0x54, 0x7f);
	os02h10_write_register(ViPipe, 0x61, 0x4e);
	os02h10_write_register(ViPipe, 0x63, 0x03);
	os02h10_write_register(ViPipe, 0x64, 0x00);
	os02h10_write_register(ViPipe, 0x65, 0x00);
	os02h10_write_register(ViPipe, 0x66, 0x02);
	os02h10_write_register(ViPipe, 0x67, 0x00);
	os02h10_write_register(ViPipe, 0x8d, 0x00);
	os02h10_write_register(ViPipe, 0x8e, 0x07);
	os02h10_write_register(ViPipe, 0x8f, 0x80);
	os02h10_write_register(ViPipe, 0x90, 0x04);
	os02h10_write_register(ViPipe, 0x91, 0x38);
	os02h10_write_register(ViPipe, 0x95, 0x88);
	os02h10_write_register(ViPipe, 0x98, 0x88);
	os02h10_write_register(ViPipe, 0xb6, 0x40);
	os02h10_write_register(ViPipe, 0x23, 0x00);
	os02h10_write_register(ViPipe, 0x24, 0x05);
	os02h10_write_register(ViPipe, 0x26, 0x49);
	os02h10_write_register(ViPipe, 0x29, 0x00);
	os02h10_write_register(ViPipe, 0x2a, 0x03);
	os02h10_write_register(ViPipe, 0x31, 0x90);
	os02h10_write_register(ViPipe, 0x32, 0x04);
	os02h10_write_register(ViPipe, 0x33, 0x00);
	os02h10_write_register(ViPipe, 0x38, 0x00);
	os02h10_write_register(ViPipe, 0x55, 0x00);
	os02h10_write_register(ViPipe, 0xf5, 0x01);

	os02h10_write_register(ViPipe, 0xfd, 0x01);
	os02h10_write_register(ViPipe, 0x03, 0x00);
	os02h10_write_register(ViPipe, 0x04, 0x04);
	os02h10_write_register(ViPipe, 0x05, 0x04);
	os02h10_write_register(ViPipe, 0x06, 0x65);
	os02h10_write_register(ViPipe, 0x09, 0x00);
	os02h10_write_register(ViPipe, 0x0a, 0x40);
	os02h10_write_register(ViPipe, 0x24, 0xff);
	os02h10_write_register(ViPipe, 0x3e, 0x3c);
	os02h10_write_register(ViPipe, 0x3f, 0x01);
	os02h10_write_register(ViPipe, 0x01, 0x01);
	os02h10_write_register(ViPipe, 0x11, 0x40);
	os02h10_write_register(ViPipe, 0x14, 0x65);
	os02h10_write_register(ViPipe, 0x15, 0x00);
	os02h10_write_register(ViPipe, 0x16, 0x95);
	os02h10_write_register(ViPipe, 0x18, 0xf3);
	os02h10_write_register(ViPipe, 0x19, 0x47);
	os02h10_write_register(ViPipe, 0x1a, 0x03);
	os02h10_write_register(ViPipe, 0x1b, 0x50);
	os02h10_write_register(ViPipe, 0x1c, 0xf0);
	os02h10_write_register(ViPipe, 0x1d, 0x3d);
	os02h10_write_register(ViPipe, 0x1e, 0x00);
	os02h10_write_register(ViPipe, 0x1f, 0x27);
	os02h10_write_register(ViPipe, 0x21, 0x74);
	os02h10_write_register(ViPipe, 0x22, 0x90);
	os02h10_write_register(ViPipe, 0x25, 0x00);
	os02h10_write_register(ViPipe, 0x26, 0x77);
	os02h10_write_register(ViPipe, 0x27, 0xf9);
	os02h10_write_register(ViPipe, 0x2a, 0x4e);
	os02h10_write_register(ViPipe, 0x2e, 0x28);
	os02h10_write_register(ViPipe, 0x34, 0xf6);
	os02h10_write_register(ViPipe, 0x35, 0x22);
	os02h10_write_register(ViPipe, 0x36, 0xa2);
	os02h10_write_register(ViPipe, 0x50, 0x00);
	os02h10_write_register(ViPipe, 0x51, 0x08);
	os02h10_write_register(ViPipe, 0x52, 0x07);
	os02h10_write_register(ViPipe, 0x53, 0x00);
	os02h10_write_register(ViPipe, 0x55, 0x35);
	os02h10_write_register(ViPipe, 0x56, 0x01);
	os02h10_write_register(ViPipe, 0x57, 0x0a);
	os02h10_write_register(ViPipe, 0x59, 0x01);
	os02h10_write_register(ViPipe, 0x5b, 0x19);
	os02h10_write_register(ViPipe, 0x5d, 0x08);
	os02h10_write_register(ViPipe, 0x5e, 0x00);
	os02h10_write_register(ViPipe, 0x5f, 0x1d);
	os02h10_write_register(ViPipe, 0x60, 0x1e);
	os02h10_write_register(ViPipe, 0x68, 0x05);
	os02h10_write_register(ViPipe, 0x69, 0x04);
	os02h10_write_register(ViPipe, 0x6a, 0x00);
	os02h10_write_register(ViPipe, 0x70, 0x64);
	os02h10_write_register(ViPipe, 0x71, 0x15);
	os02h10_write_register(ViPipe, 0x72, 0x42);
	os02h10_write_register(ViPipe, 0x7b, 0x40);
	os02h10_write_register(ViPipe, 0x80, 0x08);
	os02h10_write_register(ViPipe, 0x83, 0x08);
	os02h10_write_register(ViPipe, 0x85, 0x14);
	os02h10_write_register(ViPipe, 0x86, 0x2d);
	os02h10_write_register(ViPipe, 0x8a, 0xf9);
	os02h10_write_register(ViPipe, 0x95, 0x25);
	os02h10_write_register(ViPipe, 0x97, 0x10);
	os02h10_write_register(ViPipe, 0x99, 0x30);
	os02h10_write_register(ViPipe, 0x9f, 0x10);
	os02h10_write_register(ViPipe, 0xa8, 0x10);
	os02h10_write_register(ViPipe, 0xad, 0x00);
	os02h10_write_register(ViPipe, 0xb0, 0x63);
	os02h10_write_register(ViPipe, 0xb1, 0x63);
	os02h10_write_register(ViPipe, 0xb2, 0x63);
	os02h10_write_register(ViPipe, 0xb3, 0x63);
	os02h10_write_register(ViPipe, 0xb4, 0x61);
	os02h10_write_register(ViPipe, 0xb5, 0x61);
	os02h10_write_register(ViPipe, 0xb6, 0x61);
	os02h10_write_register(ViPipe, 0xb7, 0x61);
	os02h10_write_register(ViPipe, 0xb8, 0x77);
	os02h10_write_register(ViPipe, 0xb9, 0x66);
	os02h10_write_register(ViPipe, 0xba, 0x07);
	os02h10_write_register(ViPipe, 0xbb, 0x07);
	os02h10_write_register(ViPipe, 0xbc, 0x07);
	os02h10_write_register(ViPipe, 0xbd, 0x07);
	os02h10_write_register(ViPipe, 0x29, 0x0a);
	os02h10_write_register(ViPipe, 0x2b, 0x03);
	os02h10_write_register(ViPipe, 0x30, 0x04);
	os02h10_write_register(ViPipe, 0x92, 0x05);
	os02h10_write_register(ViPipe, 0xc4, 0x00);
	os02h10_write_register(ViPipe, 0xc5, 0x00);
	os02h10_write_register(ViPipe, 0xc6, 0x13);
	os02h10_write_register(ViPipe, 0xc7, 0x6d);
	os02h10_write_register(ViPipe, 0xc9, 0x00);
	os02h10_write_register(ViPipe, 0xd0, 0x77);
	os02h10_write_register(ViPipe, 0xd1, 0x66);
	os02h10_write_register(ViPipe, 0xd5, 0x10);
	os02h10_write_register(ViPipe, 0xd7, 0x3d);
	os02h10_write_register(ViPipe, 0xdc, 0x01);
	os02h10_write_register(ViPipe, 0xdd, 0x01);
	os02h10_write_register(ViPipe, 0xde, 0x04);

	os02h10_write_register(ViPipe, 0xfd, 0x02);
	os02h10_write_register(ViPipe, 0x68, 0x02);
	os02h10_write_register(ViPipe, 0x6d, 0x5d);
	os02h10_write_register(ViPipe, 0x6e, 0xdc);
	os02h10_write_register(ViPipe, 0x78, 0x70);
	os02h10_write_register(ViPipe, 0x79, 0x70);
	os02h10_write_register(ViPipe, 0x7a, 0x70);
	os02h10_write_register(ViPipe, 0x7b, 0x70);
	os02h10_write_register(ViPipe, 0x80, 0x60);
	os02h10_write_register(ViPipe, 0x81, 0x60);
	os02h10_write_register(ViPipe, 0x82, 0x12);
	os02h10_write_register(ViPipe, 0x83, 0x60);
	os02h10_write_register(ViPipe, 0x84, 0x60);
	os02h10_write_register(ViPipe, 0x9f, 0x12);
	os02h10_write_register(ViPipe, 0xa1, 0x04);
	os02h10_write_register(ViPipe, 0xa2, 0x04);
	os02h10_write_register(ViPipe, 0xa3, 0x38);
	os02h10_write_register(ViPipe, 0xa5, 0x04);
	os02h10_write_register(ViPipe, 0xa6, 0x07);
	os02h10_write_register(ViPipe, 0xa7, 0x80);
	os02h10_write_register(ViPipe, 0x72, 0x40);
	os02h10_write_register(ViPipe, 0x73, 0x40);
	os02h10_write_register(ViPipe, 0x74, 0x40);
	os02h10_write_register(ViPipe, 0x75, 0x40);

	/* Temperature sensor settings. */
	os02h10_write_register(ViPipe, 0xfd, 0x06);
	os02h10_write_register(ViPipe, 0x00, 0x04); /* r_tmp_slope_h[7:0] */
	os02h10_write_register(ViPipe, 0x01, 0x7c); /* r_tmp_slope_l[7:0] */
	os02h10_write_register(ViPipe, 0x02, 0xbf); /* r_tmp_offset_3[7:0] */
	os02h10_write_register(ViPipe, 0x03, 0x86); /* r_tmp_offset_2[7:0] */
	os02h10_write_register(ViPipe, 0x04, 0xbd); /* r_tmp_offset_1[7:0] */
	os02h10_write_register(ViPipe, 0x05, 0x02); /* r_tmp_offset_0[7:0] */

	os02h10_write_register(ViPipe, 0xfd, 0x02);
	os02h10_write_register(ViPipe, 0x52, 0xff);
	os02h10_write_register(ViPipe, 0xfd, 0x01);
	os02h10_write_register(ViPipe, 0xfd, 0x00);
	os02h10_write_register(ViPipe, 0xb1, 0x02); /* [1] mipi_en */
	os02h10_write_register(ViPipe, 0xfd, 0x01);

	os02h10_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===OS02H10 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}
#endif

/* 1080P30 - 1C version */
#if OS02H10_CHIP_VERSION_1C
static void os02h10_1c_linear_1080p30_init(VI_PIPE ViPipe)
{
	os02h10_write_register(ViPipe, 0xfd, 0x00);
	os02h10_write_register(ViPipe, 0x20, 0x00);
	delay_ms(5);
	os02h10_write_register(ViPipe, 0x53, 0xfe);
	os02h10_write_register(ViPipe, 0x54, 0x7f);
	os02h10_write_register(ViPipe, 0x61, 0x8c);
	os02h10_write_register(ViPipe, 0x63, 0x05);
	os02h10_write_register(ViPipe, 0x64, 0x00);
	os02h10_write_register(ViPipe, 0x65, 0x00);
	os02h10_write_register(ViPipe, 0x66, 0x02);
	os02h10_write_register(ViPipe, 0x67, 0x00);
	os02h10_write_register(ViPipe, 0x8d, 0x00);
	os02h10_write_register(ViPipe, 0x8e, 0x07);
	os02h10_write_register(ViPipe, 0x8f, 0x80);
	os02h10_write_register(ViPipe, 0x90, 0x04);
	os02h10_write_register(ViPipe, 0x91, 0x38);
	os02h10_write_register(ViPipe, 0x95, 0x88);
	os02h10_write_register(ViPipe, 0x98, 0x88);
	os02h10_write_register(ViPipe, 0xb6, 0x40);
	os02h10_write_register(ViPipe, 0x23, 0x00);
	os02h10_write_register(ViPipe, 0x24, 0x05);
	os02h10_write_register(ViPipe, 0x26, 0x4d);
	os02h10_write_register(ViPipe, 0x29, 0x00);
	os02h10_write_register(ViPipe, 0x2a, 0x03);
	os02h10_write_register(ViPipe, 0x31, 0x90);
	os02h10_write_register(ViPipe, 0x32, 0x04);
	os02h10_write_register(ViPipe, 0x33, 0x00);
	os02h10_write_register(ViPipe, 0x38, 0x00);
	os02h10_write_register(ViPipe, 0x55, 0x00);
	os02h10_write_register(ViPipe, 0xf5, 0x01);

	os02h10_write_register(ViPipe, 0xfd, 0x01);
	os02h10_write_register(ViPipe, 0x03, 0x00);
	os02h10_write_register(ViPipe, 0x04, 0x04);
	os02h10_write_register(ViPipe, 0x05, 0x04);
	os02h10_write_register(ViPipe, 0x06, 0x65);
	os02h10_write_register(ViPipe, 0x09, 0x00);
	os02h10_write_register(ViPipe, 0x0a, 0x00);
	os02h10_write_register(ViPipe, 0x24, 0xff);
	os02h10_write_register(ViPipe, 0x3e, 0x3c);
	os02h10_write_register(ViPipe, 0x3f, 0x01);
	os02h10_write_register(ViPipe, 0x01, 0x01);
	os02h10_write_register(ViPipe, 0x11, 0x40);
	os02h10_write_register(ViPipe, 0x14, 0x65);
	os02h10_write_register(ViPipe, 0x15, 0x00);
	os02h10_write_register(ViPipe, 0x16, 0x95);
	os02h10_write_register(ViPipe, 0x18, 0xf3);
	os02h10_write_register(ViPipe, 0x19, 0x47);
	os02h10_write_register(ViPipe, 0x1a, 0x03);
	os02h10_write_register(ViPipe, 0x1b, 0x50);
	os02h10_write_register(ViPipe, 0x1c, 0xf0);
	os02h10_write_register(ViPipe, 0x1d, 0x3d);
	os02h10_write_register(ViPipe, 0x1e, 0x00);
	os02h10_write_register(ViPipe, 0x1f, 0x27);
	os02h10_write_register(ViPipe, 0x21, 0x74);
	os02h10_write_register(ViPipe, 0x22, 0x90);
	os02h10_write_register(ViPipe, 0x25, 0x00);
	os02h10_write_register(ViPipe, 0x26, 0x77);
	os02h10_write_register(ViPipe, 0x27, 0xf9);
	os02h10_write_register(ViPipe, 0x2a, 0x57);
	os02h10_write_register(ViPipe, 0x2e, 0x28);
	os02h10_write_register(ViPipe, 0x34, 0xf6);
	os02h10_write_register(ViPipe, 0x35, 0x22);
	os02h10_write_register(ViPipe, 0x36, 0xa2);
	os02h10_write_register(ViPipe, 0x50, 0x00);
	os02h10_write_register(ViPipe, 0x51, 0x08);
	os02h10_write_register(ViPipe, 0x52, 0x07);
	os02h10_write_register(ViPipe, 0x53, 0x00);
	os02h10_write_register(ViPipe, 0x55, 0x35);
	os02h10_write_register(ViPipe, 0x56, 0x01);
	os02h10_write_register(ViPipe, 0x57, 0x0a);
	os02h10_write_register(ViPipe, 0x59, 0x01);
	os02h10_write_register(ViPipe, 0x5b, 0x14);
	os02h10_write_register(ViPipe, 0x5d, 0x08);
	os02h10_write_register(ViPipe, 0x5e, 0x00);
	os02h10_write_register(ViPipe, 0x5f, 0x1d);
	os02h10_write_register(ViPipe, 0x60, 0x1e);
	os02h10_write_register(ViPipe, 0x68, 0x05);
	os02h10_write_register(ViPipe, 0x69, 0x04);
	os02h10_write_register(ViPipe, 0x6a, 0x00);
	os02h10_write_register(ViPipe, 0x70, 0x64);
	os02h10_write_register(ViPipe, 0x71, 0x15);
	os02h10_write_register(ViPipe, 0x72, 0x42);
	os02h10_write_register(ViPipe, 0x7b, 0x40);
	os02h10_write_register(ViPipe, 0x80, 0x08);
	os02h10_write_register(ViPipe, 0x83, 0x08);
	os02h10_write_register(ViPipe, 0x85, 0x14);
	os02h10_write_register(ViPipe, 0x86, 0x2d);
	os02h10_write_register(ViPipe, 0x8a, 0xf9);
	os02h10_write_register(ViPipe, 0x95, 0x25);
	os02h10_write_register(ViPipe, 0x97, 0x10);
	os02h10_write_register(ViPipe, 0x99, 0x30);
	os02h10_write_register(ViPipe, 0x9f, 0x10);
	os02h10_write_register(ViPipe, 0xa8, 0x10);
	os02h10_write_register(ViPipe, 0xad, 0x00);
	os02h10_write_register(ViPipe, 0xb0, 0x63);
	os02h10_write_register(ViPipe, 0xb1, 0x63);
	os02h10_write_register(ViPipe, 0xb2, 0x63);
	os02h10_write_register(ViPipe, 0xb3, 0x63);
	os02h10_write_register(ViPipe, 0xb4, 0x61);
	os02h10_write_register(ViPipe, 0xb5, 0x61);
	os02h10_write_register(ViPipe, 0xb6, 0x61);
	os02h10_write_register(ViPipe, 0xb7, 0x61);
	os02h10_write_register(ViPipe, 0xb8, 0x77);
	os02h10_write_register(ViPipe, 0xb9, 0x66);
	os02h10_write_register(ViPipe, 0xba, 0x03);
	os02h10_write_register(ViPipe, 0xbb, 0x03);
	os02h10_write_register(ViPipe, 0xbc, 0x03);
	os02h10_write_register(ViPipe, 0xbd, 0x03);
	os02h10_write_register(ViPipe, 0x29, 0x0a);
	os02h10_write_register(ViPipe, 0x2b, 0x03);
	os02h10_write_register(ViPipe, 0x30, 0x04);
	os02h10_write_register(ViPipe, 0x92, 0x05);
	os02h10_write_register(ViPipe, 0xc4, 0x00);
	os02h10_write_register(ViPipe, 0xc5, 0x00);
	os02h10_write_register(ViPipe, 0xc6, 0x13);
	os02h10_write_register(ViPipe, 0xc7, 0x6d);
	os02h10_write_register(ViPipe, 0xc9, 0x00);
	os02h10_write_register(ViPipe, 0xd0, 0x77);
	os02h10_write_register(ViPipe, 0xd1, 0x66);
	os02h10_write_register(ViPipe, 0xd5, 0x11);
	os02h10_write_register(ViPipe, 0xd7, 0x40);
	os02h10_write_register(ViPipe, 0xdc, 0x01);
	os02h10_write_register(ViPipe, 0xdd, 0x01);
	os02h10_write_register(ViPipe, 0xde, 0x04);

	os02h10_write_register(ViPipe, 0xfd, 0x02);
	os02h10_write_register(ViPipe, 0x68, 0x02);
	os02h10_write_register(ViPipe, 0x6d, 0x5d);
	os02h10_write_register(ViPipe, 0x6e, 0xdc);
	os02h10_write_register(ViPipe, 0x78, 0x70);
	os02h10_write_register(ViPipe, 0x79, 0x70);
	os02h10_write_register(ViPipe, 0x7a, 0x70);
	os02h10_write_register(ViPipe, 0x7b, 0x70);
	os02h10_write_register(ViPipe, 0x80, 0x60);
	os02h10_write_register(ViPipe, 0x81, 0x60);
	os02h10_write_register(ViPipe, 0x82, 0x12);
	os02h10_write_register(ViPipe, 0x83, 0x60);
	os02h10_write_register(ViPipe, 0x84, 0x60);
	os02h10_write_register(ViPipe, 0x9f, 0x12);
	os02h10_write_register(ViPipe, 0xa1, 0x04);
	os02h10_write_register(ViPipe, 0xa2, 0x04);
	os02h10_write_register(ViPipe, 0xa3, 0x38);
	os02h10_write_register(ViPipe, 0xa5, 0x04);
	os02h10_write_register(ViPipe, 0xa6, 0x07);
	os02h10_write_register(ViPipe, 0xa7, 0x80);
	os02h10_write_register(ViPipe, 0x72, 0x40);
	os02h10_write_register(ViPipe, 0x73, 0x40);
	os02h10_write_register(ViPipe, 0x74, 0x40);
	os02h10_write_register(ViPipe, 0x75, 0x40);

	os02h10_write_register(ViPipe, 0xfd, 0x00);
	os02h10_write_register(ViPipe, 0xb1, 0x02);
	os02h10_write_register(ViPipe, 0xfd, 0x01);

	os02h10_default_reg_init(ViPipe);

	delay_ms(100);

	printf("ViPipe:%d,===OS02H10-1C 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}
#endif