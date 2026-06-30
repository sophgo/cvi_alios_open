#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "cvi_sensor.h"
#include "tp9951_cmos_ex.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define TP9951_CHIP_ID_HI_ADDR		0xfe
#define TP9951_CHIP_ID_LO_ADDR		0xff
#define TP9951_CHIP_ID			0x2860

#define IS_VIDEO_PRESENT(status)       (((status) & 0x88) == 0x08)  // VDLOSS=0 & VDET=1
#define IS_SIGNAL_STABLE(status)       (((status) & 0x20) == 0x20)   // HLOCK=1
#define EXTRACT_CVSTD(detection)       ((detection) & 0x07)          // CVSTD


CVI_U8 tp9951_i2c_addr = 0x44; /* I2C slave address of TP9951, SA0=0:0x44, SA0=1:0x45 */
const CVI_U32 tp9951_addr_byte = 1;
const CVI_U32 tp9951_data_byte = 1;

static void tp9951_set_1080p_30(VI_PIPE ViPipe);
static void tp9951_set_1080p_25(VI_PIPE ViPipe);
static void tp9951_set_720p_30(VI_PIPE ViPipe);
static void tp9951_set_720p_25(VI_PIPE ViPipe);


int tp9951_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = g_aunTP9951_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_init(i2c_id);
}


int tp9951_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunTP9951_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_exit(i2c_id);
}

int tp9951_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunTP9951_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_read(i2c_id, tp9951_i2c_addr, (CVI_U32)addr, tp9951_addr_byte, tp9951_data_byte);
}

int tp9951_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunTP9951_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_write(i2c_id, tp9951_i2c_addr, (CVI_U32)addr, tp9951_addr_byte,
		(CVI_U32)data, tp9951_data_byte);
}

int tp9951_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	nVal = tp9951_read_register(ViPipe, TP9951_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = tp9951_read_register(ViPipe, TP9951_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != TP9951_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void tp9951_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode;

	if (tp9951_i2c_init(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "tp9951 i2c init fail\n");
		return;
	}

	if (tp9951_probe(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "tp9951 probe fail\n");
		return;
	}

	u8ImgMode =  g_pastTP9951[ViPipe]->u8ImgMode;

	if (TP9951_MODE_1080P_30P == u8ImgMode) {
		tp9951_set_1080p_30(ViPipe);
	} else if (TP9951_MODE_1080P_25P == u8ImgMode) {
		tp9951_set_1080p_25(ViPipe);
	} else if (TP9951_MODE_720P_30P == u8ImgMode) {
		tp9951_set_720p_30(ViPipe);
	} else if(TP9951_MODE_720P_25P == u8ImgMode) {
		tp9951_set_720p_25(ViPipe);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Image Mode Not support!\n");
		return;
	}

	g_pastTP9951[ViPipe]->bInit = CVI_TRUE;
}

int AHD_tp9951_Init(VI_PIPE ViPipe, bool isFirstInit)
{
	UNUSED(isFirstInit);
	tp9951_init(ViPipe);

	return CVI_SUCCESS;
}

void tp9951_exit(VI_PIPE ViPipe)
{
	tp9951_i2c_exit(ViPipe);
}

static void tp9951_set_1080p_30(VI_PIPE ViPipe)
{
	int val;

	/* decoder page: common init */
	tp9951_write_register(ViPipe, 0x40, 0x00);
	tp9951_write_register(ViPipe, 0x06, 0x12);
	tp9951_write_register(ViPipe, 0x42, 0x00);
	tp9951_write_register(ViPipe, 0x4e, 0x00);
	tp9951_write_register(ViPipe, 0xfd, 0x80);
	tp9951_write_register(ViPipe, 0x54, 0x00);
	tp9951_write_register(ViPipe, 0x41, 0x00);  /* VIN1 (0x00=VIN1, 0x01=VIN2, etc.) */

	/* FHD30 base */
	tp9951_write_register(ViPipe, 0x02, 0x40);
	tp9951_write_register(ViPipe, 0x07, 0xc0);
	tp9951_write_register(ViPipe, 0x0b, 0xc0);
	tp9951_write_register(ViPipe, 0x0c, 0x03);
	tp9951_write_register(ViPipe, 0x0d, 0x50);
	tp9951_write_register(ViPipe, 0x15, 0x03);
	tp9951_write_register(ViPipe, 0x16, 0xd2);
	tp9951_write_register(ViPipe, 0x17, 0x80);
	tp9951_write_register(ViPipe, 0x18, 0x29);
	tp9951_write_register(ViPipe, 0x19, 0x38);
	tp9951_write_register(ViPipe, 0x1a, 0x47);
	tp9951_write_register(ViPipe, 0x1c, 0x08);
	tp9951_write_register(ViPipe, 0x1d, 0x98);
	tp9951_write_register(ViPipe, 0x20, 0x30);
	tp9951_write_register(ViPipe, 0x21, 0x84);
	tp9951_write_register(ViPipe, 0x22, 0x36);
	tp9951_write_register(ViPipe, 0x23, 0x3c);
	tp9951_write_register(ViPipe, 0x2b, 0x60);
	tp9951_write_register(ViPipe, 0x2c, 0x2a);
	tp9951_write_register(ViPipe, 0x2d, 0x30);
	tp9951_write_register(ViPipe, 0x2e, 0x70);
	tp9951_write_register(ViPipe, 0x30, 0x48);
	tp9951_write_register(ViPipe, 0x31, 0xbb);
	tp9951_write_register(ViPipe, 0x32, 0x2e);
	tp9951_write_register(ViPipe, 0x33, 0x90);
	tp9951_write_register(ViPipe, 0x35, 0x05);
	tp9951_write_register(ViPipe, 0x38, 0x00);
	tp9951_write_register(ViPipe, 0x39, 0x1c);

	/* AHD 1080p30 extra */
	tp9951_write_register(ViPipe, 0x02, 0x44);
	tp9951_write_register(ViPipe, 0x0d, 0x72);
	tp9951_write_register(ViPipe, 0x15, 0x01);
	tp9951_write_register(ViPipe, 0x16, 0xf0);
	tp9951_write_register(ViPipe, 0x18, 0x2a);
	tp9951_write_register(ViPipe, 0x20, 0x38);
	tp9951_write_register(ViPipe, 0x21, 0x46);
	tp9951_write_register(ViPipe, 0x25, 0xfe);
	tp9951_write_register(ViPipe, 0x26, 0x0d);
	tp9951_write_register(ViPipe, 0x2c, 0x3a);
	tp9951_write_register(ViPipe, 0x2d, 0x54);
	tp9951_write_register(ViPipe, 0x2e, 0x40);
	tp9951_write_register(ViPipe, 0x30, 0xa5);
	tp9951_write_register(ViPipe, 0x31, 0x95);
	tp9951_write_register(ViPipe, 0x32, 0xe0);
	tp9951_write_register(ViPipe, 0x33, 0x60);

	/* MIPI page */
	tp9951_write_register(ViPipe, 0x40, 0x08);
	tp9951_write_register(ViPipe, 0x02, 0x7d);
	tp9951_write_register(ViPipe, 0x03, 0x75);
	tp9951_write_register(ViPipe, 0x04, 0x75);
	tp9951_write_register(ViPipe, 0x13, 0xef);
	tp9951_write_register(ViPipe, 0x20, 0x00);
	tp9951_write_register(ViPipe, 0x23, 0x9e);

	/* 2-lane config */
	tp9951_write_register(ViPipe, 0x21, 0x12);
	tp9951_write_register(ViPipe, 0x12, 0x54);
	tp9951_write_register(ViPipe, 0x14, 0x41);
	tp9951_write_register(ViPipe, 0x15, 0x02);
	tp9951_write_register(ViPipe, 0x2a, 0x04);
	tp9951_write_register(ViPipe, 0x2b, 0x03);
	tp9951_write_register(ViPipe, 0x2c, 0x0a);
	tp9951_write_register(ViPipe, 0x2e, 0x02);

	/* reset + stream on */
	tp9951_write_register(ViPipe, 0x40, 0x00);
	val = tp9951_read_register(ViPipe, 0x06);
	if (val >= 0) {
		tp9951_write_register(ViPipe, 0x06, val | 0x80);  /* soft reset */
	}

	tp9951_write_register(ViPipe, 0x40, 0x08);
	val = tp9951_read_register(ViPipe, 0x10);
	if (val >= 0) {
		tp9951_write_register(ViPipe, 0x10, val | 0x80);  /* PLL reset assert */
		tp9951_write_register(ViPipe, 0x10, val);         /* PLL reset deassert */
	}

	tp9951_write_register(ViPipe, 0x28, 0x02);  /* stream off */
	tp9951_write_register(ViPipe, 0x28, 0x00);  /* stream on */
	tp9951_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===TP9951 1080P 30fps Init OK!===\n", ViPipe);

}

static void tp9951_set_1080p_25(VI_PIPE ViPipe)
{
	int val;

	/* decoder page: common init */
	tp9951_write_register(ViPipe, 0x40, 0x00);
	tp9951_write_register(ViPipe, 0x06, 0x12);
	tp9951_write_register(ViPipe, 0x42, 0x00);
	tp9951_write_register(ViPipe, 0x4e, 0x00);
	tp9951_write_register(ViPipe, 0xfd, 0x80);
	tp9951_write_register(ViPipe, 0x54, 0x00);
	tp9951_write_register(ViPipe, 0x41, 0x00);  /* VIN1 (0x00=VIN1, 0x01=VIN2, etc.) */

	/* FHD25 base */
	tp9951_write_register(ViPipe, 0x02, 0x40);
	tp9951_write_register(ViPipe, 0x07, 0xc0);
	tp9951_write_register(ViPipe, 0x0b, 0xc0);
	tp9951_write_register(ViPipe, 0x0c, 0x03);
	tp9951_write_register(ViPipe, 0x0d, 0x50);
	tp9951_write_register(ViPipe, 0x15, 0x03);
	tp9951_write_register(ViPipe, 0x16, 0xd2);
	tp9951_write_register(ViPipe, 0x17, 0x80);
	tp9951_write_register(ViPipe, 0x18, 0x29);
	tp9951_write_register(ViPipe, 0x19, 0x38);
	tp9951_write_register(ViPipe, 0x1a, 0x47);
	tp9951_write_register(ViPipe, 0x1c, 0x0a);
	tp9951_write_register(ViPipe, 0x1d, 0x50);
	tp9951_write_register(ViPipe, 0x20, 0x30);
	tp9951_write_register(ViPipe, 0x21, 0x84);
	tp9951_write_register(ViPipe, 0x22, 0x36);
	tp9951_write_register(ViPipe, 0x23, 0x3c);
	tp9951_write_register(ViPipe, 0x2b, 0x60);
	tp9951_write_register(ViPipe, 0x2c, 0x2a);
	tp9951_write_register(ViPipe, 0x2d, 0x30);
	tp9951_write_register(ViPipe, 0x2e, 0x70);
	tp9951_write_register(ViPipe, 0x30, 0x48);
	tp9951_write_register(ViPipe, 0x31, 0xbb);
	tp9951_write_register(ViPipe, 0x32, 0x2e);
	tp9951_write_register(ViPipe, 0x33, 0x90);
	tp9951_write_register(ViPipe, 0x35, 0x05);
	tp9951_write_register(ViPipe, 0x38, 0x00);
	tp9951_write_register(ViPipe, 0x39, 0x1c);

	/* AHD 1080p25 extra */
	tp9951_write_register(ViPipe, 0x02, 0x44);
	tp9951_write_register(ViPipe, 0x0d, 0x73);
	tp9951_write_register(ViPipe, 0x15, 0x01);
	tp9951_write_register(ViPipe, 0x16, 0xf0);
	tp9951_write_register(ViPipe, 0x18, 0x2a);
	tp9951_write_register(ViPipe, 0x20, 0x3c);
	tp9951_write_register(ViPipe, 0x21, 0x46);
	tp9951_write_register(ViPipe, 0x25, 0xfe);
	tp9951_write_register(ViPipe, 0x26, 0x0d);
	tp9951_write_register(ViPipe, 0x2c, 0x3a);
	tp9951_write_register(ViPipe, 0x2d, 0x54);
	tp9951_write_register(ViPipe, 0x2e, 0x40);
	tp9951_write_register(ViPipe, 0x30, 0xa5);
	tp9951_write_register(ViPipe, 0x31, 0x86);
	tp9951_write_register(ViPipe, 0x32, 0xfb);
	tp9951_write_register(ViPipe, 0x33, 0x60);

	/* MIPI page */
	tp9951_write_register(ViPipe, 0x40, 0x08);
	tp9951_write_register(ViPipe, 0x02, 0x7d);
	tp9951_write_register(ViPipe, 0x03, 0x75);
	tp9951_write_register(ViPipe, 0x04, 0x75);
	tp9951_write_register(ViPipe, 0x13, 0xef);
	tp9951_write_register(ViPipe, 0x20, 0x00);
	tp9951_write_register(ViPipe, 0x23, 0x9e);

	/* 2-lane config */
	tp9951_write_register(ViPipe, 0x21, 0x12);
	tp9951_write_register(ViPipe, 0x12, 0x54);
	tp9951_write_register(ViPipe, 0x14, 0x41);
	tp9951_write_register(ViPipe, 0x15, 0x02);
	tp9951_write_register(ViPipe, 0x2a, 0x04);
	tp9951_write_register(ViPipe, 0x2b, 0x03);
	tp9951_write_register(ViPipe, 0x2c, 0x0a);
	tp9951_write_register(ViPipe, 0x2e, 0x02);

	/* reset + stream on */
	tp9951_write_register(ViPipe, 0x40, 0x00);
	val = tp9951_read_register(ViPipe, 0x06);
	if (val >= 0) {
		tp9951_write_register(ViPipe, 0x06, val | 0x80);  /* soft reset */
	}

	tp9951_write_register(ViPipe, 0x40, 0x08);
	val = tp9951_read_register(ViPipe, 0x10);
	if (val >= 0) {
		tp9951_write_register(ViPipe, 0x10, val | 0x80);  /* PLL reset assert */
		tp9951_write_register(ViPipe, 0x10, val);         /* PLL reset deassert */
	}

	tp9951_write_register(ViPipe, 0x28, 0x02);  /* stream off */
	tp9951_write_register(ViPipe, 0x28, 0x00);  /* stream on */
	tp9951_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===TP9951 1080P 25fps Init OK!===\n", ViPipe);
}

static void tp9951_set_720p_30(VI_PIPE ViPipe)
{
	int val;

	/* decoder page: common init */
	tp9951_write_register(ViPipe, 0x40, 0x00);
	tp9951_write_register(ViPipe, 0x06, 0x12);
	tp9951_write_register(ViPipe, 0x42, 0x00);
	tp9951_write_register(ViPipe, 0x4e, 0x00);
	tp9951_write_register(ViPipe, 0xfd, 0x80);
	tp9951_write_register(ViPipe, 0x54, 0x00);
	tp9951_write_register(ViPipe, 0x41, 0x00);  /* VIN1 (0x00=VIN1, 0x01=VIN2, etc.) */

	/* HD30 base */
	tp9951_write_register(ViPipe, 0x02, 0x42);
	tp9951_write_register(ViPipe, 0x07, 0xc0);
	tp9951_write_register(ViPipe, 0x0b, 0xc0);
	tp9951_write_register(ViPipe, 0x0c, 0x13);
	tp9951_write_register(ViPipe, 0x0d, 0x50);
	tp9951_write_register(ViPipe, 0x15, 0x13);
	tp9951_write_register(ViPipe, 0x16, 0x15);
	tp9951_write_register(ViPipe, 0x17, 0x00);
	tp9951_write_register(ViPipe, 0x18, 0x19);
	tp9951_write_register(ViPipe, 0x19, 0xd0);
	tp9951_write_register(ViPipe, 0x1a, 0x25);
	tp9951_write_register(ViPipe, 0x1c, 0x06);
	tp9951_write_register(ViPipe, 0x1d, 0x72);
	tp9951_write_register(ViPipe, 0x20, 0x30);
	tp9951_write_register(ViPipe, 0x21, 0x84);
	tp9951_write_register(ViPipe, 0x22, 0x36);
	tp9951_write_register(ViPipe, 0x23, 0x3c);
	tp9951_write_register(ViPipe, 0x2b, 0x60);
	tp9951_write_register(ViPipe, 0x2c, 0x2a);
	tp9951_write_register(ViPipe, 0x2d, 0x30);
	tp9951_write_register(ViPipe, 0x2e, 0x70);
	tp9951_write_register(ViPipe, 0x30, 0x48);
	tp9951_write_register(ViPipe, 0x31, 0xbb);
	tp9951_write_register(ViPipe, 0x32, 0x2e);
	tp9951_write_register(ViPipe, 0x33, 0x90);
	tp9951_write_register(ViPipe, 0x35, 0x25);
	tp9951_write_register(ViPipe, 0x38, 0x00);
	tp9951_write_register(ViPipe, 0x39, 0x18);

	/* AHD 720p30 extra */
	tp9951_write_register(ViPipe, 0x02, 0x46);
	tp9951_write_register(ViPipe, 0x0d, 0x70);
	tp9951_write_register(ViPipe, 0x18, 0x1b);
	tp9951_write_register(ViPipe, 0x20, 0x40);
	tp9951_write_register(ViPipe, 0x21, 0x46);
	tp9951_write_register(ViPipe, 0x25, 0xfe);
	tp9951_write_register(ViPipe, 0x26, 0x01);
	tp9951_write_register(ViPipe, 0x2c, 0x3a);
	tp9951_write_register(ViPipe, 0x2d, 0x5a);
	tp9951_write_register(ViPipe, 0x2e, 0x40);
	tp9951_write_register(ViPipe, 0x30, 0x9d);
	tp9951_write_register(ViPipe, 0x31, 0xca);
	tp9951_write_register(ViPipe, 0x32, 0x01);
	tp9951_write_register(ViPipe, 0x33, 0xd0);

	/* MIPI page */
	tp9951_write_register(ViPipe, 0x40, 0x08);
	tp9951_write_register(ViPipe, 0x02, 0x7d);
	tp9951_write_register(ViPipe, 0x03, 0x75);
	tp9951_write_register(ViPipe, 0x04, 0x75);
	tp9951_write_register(ViPipe, 0x13, 0xef);
	tp9951_write_register(ViPipe, 0x20, 0x00);
	tp9951_write_register(ViPipe, 0x23, 0x9e);

	/* 2-lane config */
	tp9951_write_register(ViPipe, 0x21, 0x12);
	tp9951_write_register(ViPipe, 0x12, 0x54);
	tp9951_write_register(ViPipe, 0x14, 0x41);
	tp9951_write_register(ViPipe, 0x15, 0x12);
	tp9951_write_register(ViPipe, 0x2a, 0x02);
	tp9951_write_register(ViPipe, 0x2b, 0x01);
	tp9951_write_register(ViPipe, 0x2c, 0x06);
	tp9951_write_register(ViPipe, 0x2e, 0x02);

	/* reset + stream on */
	tp9951_write_register(ViPipe, 0x40, 0x00);
	val = tp9951_read_register(ViPipe, 0x06);
	if (val >= 0) {
		tp9951_write_register(ViPipe, 0x06, val | 0x80);  /* soft reset */
	}

	tp9951_write_register(ViPipe, 0x40, 0x08);
	val = tp9951_read_register(ViPipe, 0x10);
	if (val >= 0) {
		tp9951_write_register(ViPipe, 0x10, val | 0x80);  /* PLL reset assert */
		tp9951_write_register(ViPipe, 0x10, val);         /* PLL reset deassert */
	}

	tp9951_write_register(ViPipe, 0x28, 0x02);  /* stream off */
	tp9951_write_register(ViPipe, 0x28, 0x00);  /* stream on */
	tp9951_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===TP9951 720P 30fps Init OK!===\n", ViPipe);
}

static void tp9951_set_720p_25(VI_PIPE ViPipe)
{
	int val;

	/* decoder page: common init */
	tp9951_write_register(ViPipe, 0x40, 0x00);
	tp9951_write_register(ViPipe, 0x06, 0x12);
	tp9951_write_register(ViPipe, 0x42, 0x00);
	tp9951_write_register(ViPipe, 0x4e, 0x00);
	tp9951_write_register(ViPipe, 0xfd, 0x80);
	tp9951_write_register(ViPipe, 0x54, 0x00);
	tp9951_write_register(ViPipe, 0x41, 0x00);  /* VIN1 (0x00=VIN1, 0x01=VIN2, etc.) */

	/* HD25 base */
	tp9951_write_register(ViPipe, 0x02, 0x42);
	tp9951_write_register(ViPipe, 0x07, 0xc0);
	tp9951_write_register(ViPipe, 0x0b, 0xc0);
	tp9951_write_register(ViPipe, 0x0c, 0x13);
	tp9951_write_register(ViPipe, 0x0d, 0x50);
	tp9951_write_register(ViPipe, 0x15, 0x13);
	tp9951_write_register(ViPipe, 0x16, 0x15);
	tp9951_write_register(ViPipe, 0x17, 0x00);
	tp9951_write_register(ViPipe, 0x18, 0x19);
	tp9951_write_register(ViPipe, 0x19, 0xd0);
	tp9951_write_register(ViPipe, 0x1a, 0x25);
	tp9951_write_register(ViPipe, 0x1c, 0x07);
	tp9951_write_register(ViPipe, 0x1d, 0xbc);
	tp9951_write_register(ViPipe, 0x20, 0x30);
	tp9951_write_register(ViPipe, 0x21, 0x84);
	tp9951_write_register(ViPipe, 0x22, 0x36);
	tp9951_write_register(ViPipe, 0x23, 0x3c);
	tp9951_write_register(ViPipe, 0x2b, 0x60);
	tp9951_write_register(ViPipe, 0x2c, 0x2a);
	tp9951_write_register(ViPipe, 0x2d, 0x30);
	tp9951_write_register(ViPipe, 0x2e, 0x70);
	tp9951_write_register(ViPipe, 0x30, 0x48);
	tp9951_write_register(ViPipe, 0x31, 0xbb);
	tp9951_write_register(ViPipe, 0x32, 0x2e);
	tp9951_write_register(ViPipe, 0x33, 0x90);
	tp9951_write_register(ViPipe, 0x35, 0x25);
	tp9951_write_register(ViPipe, 0x38, 0x00);
	tp9951_write_register(ViPipe, 0x39, 0x18);

	/* AHD 720p25 extra */
	tp9951_write_register(ViPipe, 0x02, 0x46);
	tp9951_write_register(ViPipe, 0x0d, 0x71);
	tp9951_write_register(ViPipe, 0x18, 0x1b);
	tp9951_write_register(ViPipe, 0x20, 0x40);
	tp9951_write_register(ViPipe, 0x21, 0x46);
	tp9951_write_register(ViPipe, 0x25, 0xfe);
	tp9951_write_register(ViPipe, 0x26, 0x01);
	tp9951_write_register(ViPipe, 0x2c, 0x3a);
	tp9951_write_register(ViPipe, 0x2d, 0x5a);
	tp9951_write_register(ViPipe, 0x2e, 0x40);
	tp9951_write_register(ViPipe, 0x30, 0x9e);
	tp9951_write_register(ViPipe, 0x31, 0x20);
	tp9951_write_register(ViPipe, 0x32, 0x10);
	tp9951_write_register(ViPipe, 0x33, 0x90);

	/* MIPI page */
	tp9951_write_register(ViPipe, 0x40, 0x08);
	tp9951_write_register(ViPipe, 0x02, 0x7d);
	tp9951_write_register(ViPipe, 0x03, 0x75);
	tp9951_write_register(ViPipe, 0x04, 0x75);
	tp9951_write_register(ViPipe, 0x13, 0xef);
	tp9951_write_register(ViPipe, 0x20, 0x00);
	tp9951_write_register(ViPipe, 0x23, 0x9e);

	/* 2-lane config */
	tp9951_write_register(ViPipe, 0x21, 0x12);
	tp9951_write_register(ViPipe, 0x12, 0x54);
	tp9951_write_register(ViPipe, 0x14, 0x41);
	tp9951_write_register(ViPipe, 0x15, 0x12);
	tp9951_write_register(ViPipe, 0x2a, 0x02);
	tp9951_write_register(ViPipe, 0x2b, 0x01);
	tp9951_write_register(ViPipe, 0x2c, 0x06);
	tp9951_write_register(ViPipe, 0x2e, 0x02);

	/* reset + stream on */
	tp9951_write_register(ViPipe, 0x40, 0x00);
	val = tp9951_read_register(ViPipe, 0x06);
	if (val >= 0) {
		tp9951_write_register(ViPipe, 0x06, val | 0x80);  /* soft reset */
	}

	tp9951_write_register(ViPipe, 0x40, 0x08);
	val = tp9951_read_register(ViPipe, 0x10);
	if (val >= 0) {
		tp9951_write_register(ViPipe, 0x10, val | 0x80);  /* PLL reset assert */
		tp9951_write_register(ViPipe, 0x10, val);         /* PLL reset deassert */
	}

	tp9951_write_register(ViPipe, 0x28, 0x02);  /* stream off */
	tp9951_write_register(ViPipe, 0x28, 0x00);  /* stream on */
	tp9951_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===TP9951 720P 25fps Init OK!===\n", ViPipe);
}

int AHD_tp9951_Deinit(VI_PIPE ViPipe)
{
	return CVI_SUCCESS;
}

CVI_S32 AHD_tp9951_get_mode(VI_PIPE ViPipe)
{
	CVI_U8 lockstatus = 0;
	CVI_U8 detvideo = 0;
	SNS_AHD_MODE_S signal_type = AHD_MODE_NONE;

	lockstatus = tp9951_read_register(ViPipe, 0x01);
	detvideo = tp9951_read_register(ViPipe, 0x03);
	CVI_TRACE_SNS(CVI_DBG_INFO, "detvideo = 0x%2x, lockstatus = 0x%2x!!!\n",
			detvideo, lockstatus);

	if (IS_VIDEO_PRESENT(lockstatus) && IS_SIGNAL_STABLE(lockstatus)) {
		CVI_U8 cvstd = detvideo & 0x07;

		switch (cvstd) {
			case 0x02: //1080p30fps
				signal_type = AHD_MODE_1920X1080P30;
				break;

			case 0x03: //1080p25fps
				signal_type = AHD_MODE_1920X1080P25;
				break;

			case 0x04: //720p30fps
				signal_type = AHD_MODE_1280X720P30;
				break;

			case 0x05: // 720p25fps
				signal_type = AHD_MODE_1280X720P25;
				break;

			default:
				signal_type = AHD_MODE_NONE;
				CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupported CVSTD: 0x%02x\n", cvstd);
				break;
		}
	} else {
		signal_type = AHD_MODE_NONE;
		CVI_TRACE_SNS(CVI_DBG_ERR, "TP9951 - No valid AHD signal detected\n");
	}

	return signal_type;

}

CVI_S32 AHD_tp9951_set_mode(VI_PIPE ViPipe, CVI_S32 mode)
{
	switch (mode) {
	case AHD_MODE_1920X1080P30:
		tp9951_set_1080p_30(ViPipe);
		break;
	case AHD_MODE_1920X1080P25:
		tp9951_set_1080p_25(ViPipe);
		break;
	case AHD_MODE_1280X720P30:
		tp9951_set_720p_30(ViPipe);
		break;
	case AHD_MODE_1280X720P25:
		tp9951_set_720p_25(ViPipe);
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupported AHD mode: %d\n", mode);
		return CVI_FAILURE;
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 AHD_tp9951_set_bus_info(VI_PIPE ViPipe, CVI_S32 astI2cDev)
{
	if (ViPipe > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "invalid vipipe !!\n");
		return CVI_FAILURE;
	}
	g_aunTP9951_BusInfo[ViPipe].s8I2cDev = astI2cDev;
	return CVI_SUCCESS;
}

SNS_AHD_OBJ_S stAhdTp9951Obj =
{
    .pfnAhdInit = AHD_tp9951_Init,
    .pfnAhdDeinit = AHD_tp9951_Deinit,
    .pfnGetAhdMode = AHD_tp9951_get_mode,
    .pfnSetAhdMode = AHD_tp9951_set_mode,
    .pfnSetAhdBusInfo = AHD_tp9951_set_bus_info,
};