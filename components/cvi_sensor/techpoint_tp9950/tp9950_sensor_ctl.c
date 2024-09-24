#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "cvi_sensor.h"
#include "tp9950_cmos_ex.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define TP9950_CHIP_ID_HI_ADDR		0xfe
#define TP9950_CHIP_ID_LO_ADDR		0xff
#define TP9950_CHIP_ID			0x2850

#define AHD_PWDN_GRP 1
#define AHD_PWDN_NUM 12

CVI_U8 tp9950_i2c_addr = 0x44;        /* I2C slave address of TP9950, SA0=0:0x44, SA0=1:0x45*/
const CVI_U32 tp9950_addr_byte = 1;
const CVI_U32 tp9950_data_byte = 1;
static csi_gpio_t g_ahdGpio = {0};

int tp9950_gpio_init(VI_PIPE ViPipe)
{
	(void) ViPipe;

	// AHD_PWR_EN
	if (csi_gpio_init(&g_ahdGpio, AHD_PWDN_GRP) != CSI_OK) {
		printf("%s gpio init err \n",__func__);
		return CVI_FAILURE;
	}
	csi_gpio_dir(&g_ahdGpio, (1 << AHD_PWDN_NUM), GPIO_DIRECTION_OUTPUT);
	csi_gpio_write(&g_ahdGpio, (1 << AHD_PWDN_NUM), 1);

	return CVI_SUCCESS;
}

int tp9950_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = g_aunTP9950_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_init(i2c_id);
}

int tp9950_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunTP9950_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int tp9950_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunTP9950_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, tp9950_i2c_addr, (CVI_U32)addr, tp9950_addr_byte, tp9950_data_byte);
}

int tp9950_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunTP9950_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, tp9950_i2c_addr, (CVI_U32)addr, tp9950_addr_byte,
		(CVI_U32)data, tp9950_data_byte);
}


int tp9950_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	if (tp9950_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = tp9950_read_register(ViPipe, TP9950_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = tp9950_read_register(ViPipe, TP9950_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != TP9950_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void tp9950_init(VI_PIPE ViPipe)
{
	if (tp9950_i2c_init(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "tp9950 i2c init fail\n");
		return;
	}

	if (tp9950_probe(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "tp9950 probe fail\n");
		return;
	}
}

void tp9950_exit(VI_PIPE ViPipe)
{
	tp9950_i2c_exit(ViPipe);
}

void tp9950_fw_init(VI_PIPE ViPipe)
{
	tp9950_write_register(ViPipe, 0x02, 0x40);
	tp9950_write_register(ViPipe, 0x05, 0x00);
	tp9950_write_register(ViPipe, 0x06, 0x32);
	tp9950_write_register(ViPipe, 0x07, 0xC0);
	tp9950_write_register(ViPipe, 0x08, 0x00);
	tp9950_write_register(ViPipe, 0x09, 0x24);
	tp9950_write_register(ViPipe, 0x0A, 0x48);
	tp9950_write_register(ViPipe, 0x0B, 0xC0);
	tp9950_write_register(ViPipe, 0x0C, 0x03);
	tp9950_write_register(ViPipe, 0x0D, 0x50);
	tp9950_write_register(ViPipe, 0x0E, 0x00);
	tp9950_write_register(ViPipe, 0x0F, 0x00);
	tp9950_write_register(ViPipe, 0x10, 0x00);
	tp9950_write_register(ViPipe, 0x11, 0x40);
	tp9950_write_register(ViPipe, 0x12, 0x60);
	tp9950_write_register(ViPipe, 0x13, 0x00);
	tp9950_write_register(ViPipe, 0x14, 0x00);
	tp9950_write_register(ViPipe, 0x15, 0x03);
	tp9950_write_register(ViPipe, 0x16, 0xD2);
	tp9950_write_register(ViPipe, 0x17, 0x80);
	tp9950_write_register(ViPipe, 0x18, 0x29);
	tp9950_write_register(ViPipe, 0x19, 0x38);
	tp9950_write_register(ViPipe, 0x1A, 0x47);
	tp9950_write_register(ViPipe, 0x1B, 0x01);
	tp9950_write_register(ViPipe, 0x1E, 0x80);
	tp9950_write_register(ViPipe, 0x1F, 0x80);
	tp9950_write_register(ViPipe, 0x20, 0x30);
	tp9950_write_register(ViPipe, 0x21, 0x84);
	tp9950_write_register(ViPipe, 0x22, 0x36);
	tp9950_write_register(ViPipe, 0x23, 0x3C);
	tp9950_write_register(ViPipe, 0x24, 0x04);
	tp9950_write_register(ViPipe, 0x25, 0xFF);
	tp9950_write_register(ViPipe, 0x26, 0x05);
	tp9950_write_register(ViPipe, 0x27, 0x2D);
	tp9950_write_register(ViPipe, 0x28, 0x00);
	tp9950_write_register(ViPipe, 0x29, 0x48);
	tp9950_write_register(ViPipe, 0x2A, 0x30);
	tp9950_write_register(ViPipe, 0x2B, 0x60);
	tp9950_write_register(ViPipe, 0x2D, 0x30);
	tp9950_write_register(ViPipe, 0x2E, 0x70);
	tp9950_write_register(ViPipe, 0x2F, 0x00);
	tp9950_write_register(ViPipe, 0x30, 0x48);
	tp9950_write_register(ViPipe, 0x31, 0xBB);
	tp9950_write_register(ViPipe, 0x32, 0x2E);
	tp9950_write_register(ViPipe, 0x33, 0x90);
	tp9950_write_register(ViPipe, 0x34, 0x00);
	tp9950_write_register(ViPipe, 0x35, 0x05);
	tp9950_write_register(ViPipe, 0x36, 0xDC);
	tp9950_write_register(ViPipe, 0x37, 0x00);
	tp9950_write_register(ViPipe, 0x38, 0x00);
	tp9950_write_register(ViPipe, 0x39, 0x1C);
	tp9950_write_register(ViPipe, 0x3A, 0x32);
	tp9950_write_register(ViPipe, 0x3B, 0x26);
	tp9950_write_register(ViPipe, 0x3C, 0x00);
	tp9950_write_register(ViPipe, 0x3D, 0x60);
	tp9950_write_register(ViPipe, 0x3E, 0x00);
	tp9950_write_register(ViPipe, 0x3F, 0x00);
	tp9950_write_register(ViPipe, 0x40, 0x00);
	tp9950_write_register(ViPipe, 0x42, 0x00);
	tp9950_write_register(ViPipe, 0x43, 0x00);
	tp9950_write_register(ViPipe, 0x44, 0x00);
	tp9950_write_register(ViPipe, 0x45, 0x00);
	tp9950_write_register(ViPipe, 0x46, 0x00);
	tp9950_write_register(ViPipe, 0x47, 0x00);
	tp9950_write_register(ViPipe, 0x48, 0x00);
	tp9950_write_register(ViPipe, 0x49, 0x00);
	tp9950_write_register(ViPipe, 0x4A, 0x00);
	tp9950_write_register(ViPipe, 0x4B, 0x00);
	tp9950_write_register(ViPipe, 0x4C, 0x43);
	tp9950_write_register(ViPipe, 0x4D, 0x00);
	tp9950_write_register(ViPipe, 0x4E, 0x17);
	tp9950_write_register(ViPipe, 0x4F, 0x00);

	tp9950_write_register(ViPipe, 0xF0, 0x00);
	tp9950_write_register(ViPipe, 0xF1, 0x00);
	tp9950_write_register(ViPipe, 0xF2, 0x00);
	tp9950_write_register(ViPipe, 0xF4, 0x20);
	tp9950_write_register(ViPipe, 0xF5, 0x10);
	tp9950_write_register(ViPipe, 0xF6, 0x00);
	tp9950_write_register(ViPipe, 0xF7, 0x00);
	tp9950_write_register(ViPipe, 0xF8, 0x00);
	tp9950_write_register(ViPipe, 0xF9, 0x00);
	tp9950_write_register(ViPipe, 0xFB, 0x00);
	tp9950_write_register(ViPipe, 0xFC, 0x80);

	tp9950_write_register(ViPipe, 0x40, 0x08);
	tp9950_write_register(ViPipe, 0x00, 0x00);
	tp9950_write_register(ViPipe, 0x01, 0xf8);
	tp9950_write_register(ViPipe, 0x02, 0x44);
	tp9950_write_register(ViPipe, 0x08, 0xF0);
	tp9950_write_register(ViPipe, 0x13, 0x04);
	tp9950_write_register(ViPipe, 0x14, 0x73);
	tp9950_write_register(ViPipe, 0x15, 0x08);
	tp9950_write_register(ViPipe, 0x20, 0x12);
	tp9950_write_register(ViPipe, 0x34, 0x1b);
	tp9950_write_register(ViPipe, 0x23, 0x02);
	tp9950_write_register(ViPipe, 0x23, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===tp9950 FW Init OK!===\n", ViPipe);
}

static void tp9950_set_1080p_30(VI_PIPE ViPipe)
{
	tp9950_write_register(ViPipe, 0x02, 0xCC);
	tp9950_write_register(ViPipe, 0x05, 0x00);
	tp9950_write_register(ViPipe, 0x06, 0x32);
	tp9950_write_register(ViPipe, 0x07, 0xC0);
	tp9950_write_register(ViPipe, 0x08, 0x00);
	tp9950_write_register(ViPipe, 0x09, 0x24);
	tp9950_write_register(ViPipe, 0x0A, 0x48);
	tp9950_write_register(ViPipe, 0x0B, 0xC0);
	tp9950_write_register(ViPipe, 0x0C, 0x03);
	tp9950_write_register(ViPipe, 0x0D, 0x72);
	tp9950_write_register(ViPipe, 0x0E, 0x00);
	tp9950_write_register(ViPipe, 0x0F, 0x00);
	tp9950_write_register(ViPipe, 0x10, 0x00);
	tp9950_write_register(ViPipe, 0x11, 0x40);
	tp9950_write_register(ViPipe, 0x12, 0x60);
	tp9950_write_register(ViPipe, 0x13, 0x00);
	tp9950_write_register(ViPipe, 0x14, 0x00);
	tp9950_write_register(ViPipe, 0x15, 0x01);
	tp9950_write_register(ViPipe, 0x16, 0xF0);
	tp9950_write_register(ViPipe, 0x17, 0x80);
	tp9950_write_register(ViPipe, 0x18, 0x2A);
	tp9950_write_register(ViPipe, 0x19, 0x38);
	tp9950_write_register(ViPipe, 0x1A, 0x47);
	tp9950_write_register(ViPipe, 0x1B, 0x01);
	tp9950_write_register(ViPipe, 0x1C, 0x08);
	tp9950_write_register(ViPipe, 0x1D, 0x98);
	tp9950_write_register(ViPipe, 0x1E, 0x80);
	tp9950_write_register(ViPipe, 0x1F, 0x80);
	tp9950_write_register(ViPipe, 0x20, 0x38);
	tp9950_write_register(ViPipe, 0x21, 0x46);
	tp9950_write_register(ViPipe, 0x22, 0x36);
	tp9950_write_register(ViPipe, 0x23, 0x3C);
	tp9950_write_register(ViPipe, 0x24, 0x04);
	tp9950_write_register(ViPipe, 0x25, 0xFE);
	tp9950_write_register(ViPipe, 0x26, 0x0D);
	tp9950_write_register(ViPipe, 0x27, 0x2D);
	tp9950_write_register(ViPipe, 0x28, 0x00);
	tp9950_write_register(ViPipe, 0x29, 0x48);
	tp9950_write_register(ViPipe, 0x2A, 0x30);
	tp9950_write_register(ViPipe, 0x2B, 0x60);
	tp9950_write_register(ViPipe, 0x2C, 0x3A);
	tp9950_write_register(ViPipe, 0x2D, 0x54);
	tp9950_write_register(ViPipe, 0x2E, 0x40);
	tp9950_write_register(ViPipe, 0x2F, 0x00);
	tp9950_write_register(ViPipe, 0x30, 0xA5);
	tp9950_write_register(ViPipe, 0x31, 0x95);
	tp9950_write_register(ViPipe, 0x32, 0xE0);
	tp9950_write_register(ViPipe, 0x33, 0x60);
	tp9950_write_register(ViPipe, 0x34, 0x00);
	tp9950_write_register(ViPipe, 0x35, 0x05);
	tp9950_write_register(ViPipe, 0x36, 0xDC);
	tp9950_write_register(ViPipe, 0x37, 0x00);
	tp9950_write_register(ViPipe, 0x38, 0x00);
	tp9950_write_register(ViPipe, 0x39, 0x1C);
	tp9950_write_register(ViPipe, 0x3A, 0x32);
	tp9950_write_register(ViPipe, 0x3B, 0x26);
	tp9950_write_register(ViPipe, 0x3C, 0x00);
	tp9950_write_register(ViPipe, 0x3D, 0x60);
	tp9950_write_register(ViPipe, 0x3E, 0x00);
	tp9950_write_register(ViPipe, 0x3F, 0x00);
	tp9950_write_register(ViPipe, 0x40, 0x00);
	tp9950_write_register(ViPipe, 0x41, 0x00);
	tp9950_write_register(ViPipe, 0x42, 0x00);
	tp9950_write_register(ViPipe, 0x43, 0x00);
	tp9950_write_register(ViPipe, 0x44, 0x00);
	tp9950_write_register(ViPipe, 0x45, 0x00);
	tp9950_write_register(ViPipe, 0x46, 0x00);
	tp9950_write_register(ViPipe, 0x47, 0x00);
	tp9950_write_register(ViPipe, 0x48, 0x00);
	tp9950_write_register(ViPipe, 0x49, 0x00);
	tp9950_write_register(ViPipe, 0x4A, 0x00);
	tp9950_write_register(ViPipe, 0x4B, 0x00);
	tp9950_write_register(ViPipe, 0x4C, 0x43);
	tp9950_write_register(ViPipe, 0x4D, 0x00);
	tp9950_write_register(ViPipe, 0x4E, 0x17);
	tp9950_write_register(ViPipe, 0x4F, 0x00);

	tp9950_write_register(ViPipe, 0xF0, 0x00);
	tp9950_write_register(ViPipe, 0xF1, 0x00);
	tp9950_write_register(ViPipe, 0xF2, 0x00);
	tp9950_write_register(ViPipe, 0xF3, 0x07);
	tp9950_write_register(ViPipe, 0xF4, 0x20);
	tp9950_write_register(ViPipe, 0xF5, 0x10);
	tp9950_write_register(ViPipe, 0xF6, 0x00);
	tp9950_write_register(ViPipe, 0xF7, 0x00);
	tp9950_write_register(ViPipe, 0xF8, 0x00);
	tp9950_write_register(ViPipe, 0xF9, 0x00);
	tp9950_write_register(ViPipe, 0xFA, 0x03);
	tp9950_write_register(ViPipe, 0xFB, 0x00);
	tp9950_write_register(ViPipe, 0xFC, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x08);
	tp9950_write_register(ViPipe, 0x00, 0x00);
	tp9950_write_register(ViPipe, 0x01, 0xf8);
	tp9950_write_register(ViPipe, 0x02, 0x00);
	tp9950_write_register(ViPipe, 0x08, 0xF0);
	tp9950_write_register(ViPipe, 0x13, 0x04);
	tp9950_write_register(ViPipe, 0x14, 0x73);
	tp9950_write_register(ViPipe, 0x15, 0x08);
	tp9950_write_register(ViPipe, 0x20, 0x12);
	tp9950_write_register(ViPipe, 0x34, 0x1b);
	tp9950_write_register(ViPipe, 0x23, 0x02);
	tp9950_write_register(ViPipe, 0x23, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===TP9950 1080P 30fps Init OK!===\n", ViPipe);
}

void tp9950_set_1080p_25(VI_PIPE ViPipe)
{
	tp9950_write_register(ViPipe, 0x02, 0xCC);
	tp9950_write_register(ViPipe, 0x05, 0x00);
	tp9950_write_register(ViPipe, 0x06, 0x32);
	tp9950_write_register(ViPipe, 0x07, 0xC0);
	tp9950_write_register(ViPipe, 0x08, 0x00);
	tp9950_write_register(ViPipe, 0x09, 0x24);
	tp9950_write_register(ViPipe, 0x0A, 0x48);
	tp9950_write_register(ViPipe, 0x0B, 0xC0);
	tp9950_write_register(ViPipe, 0x0C, 0x03);
	tp9950_write_register(ViPipe, 0x0D, 0x73);
	tp9950_write_register(ViPipe, 0x0E, 0x00);
	tp9950_write_register(ViPipe, 0x0F, 0x00);
	tp9950_write_register(ViPipe, 0x10, 0x00);
	tp9950_write_register(ViPipe, 0x11, 0x40);
	tp9950_write_register(ViPipe, 0x12, 0x60);
	tp9950_write_register(ViPipe, 0x13, 0x00);
	tp9950_write_register(ViPipe, 0x14, 0x00);
	tp9950_write_register(ViPipe, 0x15, 0x01);
	tp9950_write_register(ViPipe, 0x16, 0xF0);
	tp9950_write_register(ViPipe, 0x17, 0x80);
	tp9950_write_register(ViPipe, 0x18, 0x2A);
	tp9950_write_register(ViPipe, 0x19, 0x38);
	tp9950_write_register(ViPipe, 0x1A, 0x47);
	tp9950_write_register(ViPipe, 0x1B, 0x01);
	tp9950_write_register(ViPipe, 0x1C, 0x0A);
	tp9950_write_register(ViPipe, 0x1D, 0x50);
	tp9950_write_register(ViPipe, 0x1E, 0x80);
	tp9950_write_register(ViPipe, 0x1F, 0x80);
	tp9950_write_register(ViPipe, 0x20, 0x3C);
	tp9950_write_register(ViPipe, 0x21, 0x46);
	tp9950_write_register(ViPipe, 0x22, 0x36);
	tp9950_write_register(ViPipe, 0x23, 0x3C);
	tp9950_write_register(ViPipe, 0x24, 0x04);
	tp9950_write_register(ViPipe, 0x25, 0xFE);
	tp9950_write_register(ViPipe, 0x26, 0x0D);
	tp9950_write_register(ViPipe, 0x27, 0x2D);
	tp9950_write_register(ViPipe, 0x28, 0x00);
	tp9950_write_register(ViPipe, 0x29, 0x48);
	tp9950_write_register(ViPipe, 0x2A, 0x30);
	tp9950_write_register(ViPipe, 0x2B, 0x60);
	tp9950_write_register(ViPipe, 0x2C, 0x3A);
	tp9950_write_register(ViPipe, 0x2D, 0x54);
	tp9950_write_register(ViPipe, 0x2E, 0x40);
	tp9950_write_register(ViPipe, 0x2F, 0x00);
	tp9950_write_register(ViPipe, 0x30, 0xA5);
	tp9950_write_register(ViPipe, 0x31, 0x86);
	tp9950_write_register(ViPipe, 0x32, 0xFB);
	tp9950_write_register(ViPipe, 0x33, 0x60);
	tp9950_write_register(ViPipe, 0x34, 0x00);
	tp9950_write_register(ViPipe, 0x35, 0x05);
	tp9950_write_register(ViPipe, 0x36, 0xDC);
	tp9950_write_register(ViPipe, 0x37, 0x00);
	tp9950_write_register(ViPipe, 0x38, 0x00);
	tp9950_write_register(ViPipe, 0x39, 0x1C);
	tp9950_write_register(ViPipe, 0x3A, 0x32);
	tp9950_write_register(ViPipe, 0x3B, 0x26);
	tp9950_write_register(ViPipe, 0x3C, 0x00);
	tp9950_write_register(ViPipe, 0x3D, 0x60);
	tp9950_write_register(ViPipe, 0x3E, 0x00);
	tp9950_write_register(ViPipe, 0x3F, 0x00);
	tp9950_write_register(ViPipe, 0x40, 0x00);
	tp9950_write_register(ViPipe, 0x41, 0x00);
	tp9950_write_register(ViPipe, 0x42, 0x00);
	tp9950_write_register(ViPipe, 0x43, 0x00);
	tp9950_write_register(ViPipe, 0x44, 0x00);
	tp9950_write_register(ViPipe, 0x45, 0x00);
	tp9950_write_register(ViPipe, 0x46, 0x00);
	tp9950_write_register(ViPipe, 0x47, 0x00);
	tp9950_write_register(ViPipe, 0x48, 0x00);
	tp9950_write_register(ViPipe, 0x49, 0x00);
	tp9950_write_register(ViPipe, 0x4A, 0x00);
	tp9950_write_register(ViPipe, 0x4B, 0x00);
	tp9950_write_register(ViPipe, 0x4C, 0x43);
	tp9950_write_register(ViPipe, 0x4D, 0x00);
	tp9950_write_register(ViPipe, 0x4E, 0x17);
	tp9950_write_register(ViPipe, 0x4F, 0x00);

	tp9950_write_register(ViPipe, 0xF0, 0x00);
	tp9950_write_register(ViPipe, 0xF1, 0x00);
	tp9950_write_register(ViPipe, 0xF2, 0x00);
	tp9950_write_register(ViPipe, 0xF3, 0x07);
	tp9950_write_register(ViPipe, 0xF4, 0x20);
	tp9950_write_register(ViPipe, 0xF5, 0x10);
	tp9950_write_register(ViPipe, 0xF6, 0x00);
	tp9950_write_register(ViPipe, 0xF7, 0x00);
	tp9950_write_register(ViPipe, 0xF8, 0x00);
	tp9950_write_register(ViPipe, 0xF9, 0x00);
	tp9950_write_register(ViPipe, 0xFA, 0x03);
	tp9950_write_register(ViPipe, 0xFB, 0x00);
	tp9950_write_register(ViPipe, 0xFC, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x08);
	tp9950_write_register(ViPipe, 0x00, 0x00);
	tp9950_write_register(ViPipe, 0x01, 0xf8);
	tp9950_write_register(ViPipe, 0x02, 0x00);
	tp9950_write_register(ViPipe, 0x08, 0xF0);
	tp9950_write_register(ViPipe, 0x13, 0x04);
	tp9950_write_register(ViPipe, 0x14, 0x73);
	tp9950_write_register(ViPipe, 0x15, 0x08);
	tp9950_write_register(ViPipe, 0x20, 0x12);
	tp9950_write_register(ViPipe, 0x34, 0x1b);
	tp9950_write_register(ViPipe, 0x23, 0x02);
	tp9950_write_register(ViPipe, 0x23, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===TP9950 1080P 25fps Init OK!===\n", ViPipe);
}

void tp9950_set_720p_30(VI_PIPE ViPipe)
{
	tp9950_write_register(ViPipe, 0x02, 0xCE);
	tp9950_write_register(ViPipe, 0x05, 0x00);
	tp9950_write_register(ViPipe, 0x06, 0x32);
	tp9950_write_register(ViPipe, 0x07, 0xC0);
	tp9950_write_register(ViPipe, 0x08, 0x00);
	tp9950_write_register(ViPipe, 0x09, 0x24);
	tp9950_write_register(ViPipe, 0x0A, 0x48);
	tp9950_write_register(ViPipe, 0x0B, 0xC0);
	tp9950_write_register(ViPipe, 0x0C, 0x03);
	tp9950_write_register(ViPipe, 0x0D, 0x70);
	tp9950_write_register(ViPipe, 0x0E, 0x00);
	tp9950_write_register(ViPipe, 0x0F, 0x00);
	tp9950_write_register(ViPipe, 0x10, 0x00);
	tp9950_write_register(ViPipe, 0x11, 0x40);
	tp9950_write_register(ViPipe, 0x12, 0x40);
	tp9950_write_register(ViPipe, 0x13, 0x00);
	tp9950_write_register(ViPipe, 0x14, 0x00);
	tp9950_write_register(ViPipe, 0x15, 0x13);
	tp9950_write_register(ViPipe, 0x16, 0x16);
	tp9950_write_register(ViPipe, 0x17, 0x00);
	tp9950_write_register(ViPipe, 0x18, 0x1B);
	tp9950_write_register(ViPipe, 0x19, 0xD0);
	tp9950_write_register(ViPipe, 0x1A, 0x25);
	tp9950_write_register(ViPipe, 0x1B, 0x01);
	tp9950_write_register(ViPipe, 0x1C, 0x06);
	tp9950_write_register(ViPipe, 0x1D, 0x72);
	tp9950_write_register(ViPipe, 0x1E, 0x80);
	tp9950_write_register(ViPipe, 0x1F, 0x80);
	tp9950_write_register(ViPipe, 0x20, 0x40);
	tp9950_write_register(ViPipe, 0x21, 0x46);
	tp9950_write_register(ViPipe, 0x22, 0x36);
	tp9950_write_register(ViPipe, 0x23, 0x3C);
	tp9950_write_register(ViPipe, 0x24, 0x04);
	tp9950_write_register(ViPipe, 0x25, 0xFE);
	tp9950_write_register(ViPipe, 0x26, 0x01);
	tp9950_write_register(ViPipe, 0x27, 0x2D);
	tp9950_write_register(ViPipe, 0x28, 0x00);
	tp9950_write_register(ViPipe, 0x29, 0x48);
	tp9950_write_register(ViPipe, 0x2A, 0x30);
	tp9950_write_register(ViPipe, 0x2B, 0x60);
	tp9950_write_register(ViPipe, 0x2C, 0x3A);
	tp9950_write_register(ViPipe, 0x2D, 0x5A);
	tp9950_write_register(ViPipe, 0x2E, 0x40);
	tp9950_write_register(ViPipe, 0x2F, 0x00);
	tp9950_write_register(ViPipe, 0x30, 0x9D);
	tp9950_write_register(ViPipe, 0x31, 0xCA);
	tp9950_write_register(ViPipe, 0x32, 0x01);
	tp9950_write_register(ViPipe, 0x33, 0xD0);
	tp9950_write_register(ViPipe, 0x34, 0x00);
	tp9950_write_register(ViPipe, 0x35, 0x25);
	tp9950_write_register(ViPipe, 0x36, 0xDC);
	tp9950_write_register(ViPipe, 0x37, 0x00);
	tp9950_write_register(ViPipe, 0x38, 0x00);
	tp9950_write_register(ViPipe, 0x39, 0x18);
	tp9950_write_register(ViPipe, 0x3A, 0x32);
	tp9950_write_register(ViPipe, 0x3B, 0x26);
	tp9950_write_register(ViPipe, 0x3C, 0x00);
	tp9950_write_register(ViPipe, 0x3D, 0x60);
	tp9950_write_register(ViPipe, 0x3E, 0x00);
	tp9950_write_register(ViPipe, 0x3F, 0x00);
	tp9950_write_register(ViPipe, 0x40, 0x00);
	tp9950_write_register(ViPipe, 0x41, 0x00);
	tp9950_write_register(ViPipe, 0x42, 0x00);
	tp9950_write_register(ViPipe, 0x43, 0x00);
	tp9950_write_register(ViPipe, 0x44, 0x00);
	tp9950_write_register(ViPipe, 0x45, 0x00);
	tp9950_write_register(ViPipe, 0x46, 0x00);
	tp9950_write_register(ViPipe, 0x47, 0x00);
	tp9950_write_register(ViPipe, 0x48, 0x00);
	tp9950_write_register(ViPipe, 0x49, 0x00);
	tp9950_write_register(ViPipe, 0x4A, 0x00);
	tp9950_write_register(ViPipe, 0x4B, 0x00);
	tp9950_write_register(ViPipe, 0x4C, 0x43);
	tp9950_write_register(ViPipe, 0x4D, 0x00);
	tp9950_write_register(ViPipe, 0x4E, 0x17);
	tp9950_write_register(ViPipe, 0x4F, 0x00);

	tp9950_write_register(ViPipe, 0xF0, 0x00);
	tp9950_write_register(ViPipe, 0xF1, 0x00);
	tp9950_write_register(ViPipe, 0xF2, 0x00);
	tp9950_write_register(ViPipe, 0xF3, 0x00);
	tp9950_write_register(ViPipe, 0xF4, 0x20);
	tp9950_write_register(ViPipe, 0xF5, 0x10);
	tp9950_write_register(ViPipe, 0xF6, 0x00);
	tp9950_write_register(ViPipe, 0xF7, 0x00);
	tp9950_write_register(ViPipe, 0xF8, 0x00);
	tp9950_write_register(ViPipe, 0xF9, 0x00);
	tp9950_write_register(ViPipe, 0xFA, 0x88);
	tp9950_write_register(ViPipe, 0xFB, 0x00);
	tp9950_write_register(ViPipe, 0xFC, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x08);
	tp9950_write_register(ViPipe, 0x01, 0xf8);
	tp9950_write_register(ViPipe, 0x02, 0x00);
	tp9950_write_register(ViPipe, 0x08, 0x0f);
	tp9950_write_register(ViPipe, 0x13, 0x24);
	tp9950_write_register(ViPipe, 0x14, 0x46);
	tp9950_write_register(ViPipe, 0x15, 0x08);
	tp9950_write_register(ViPipe, 0x20, 0x12);
	tp9950_write_register(ViPipe, 0x34, 0x1b);
	tp9950_write_register(ViPipe, 0x23, 0x02);
	tp9950_write_register(ViPipe, 0x23, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===TP9950 720P 30fps Init OK!===\n", ViPipe);
}


void tp9950_set_720p_25(VI_PIPE ViPipe)
{
	tp9950_write_register(ViPipe, 0x02, 0xCE);
	tp9950_write_register(ViPipe, 0x05, 0x00);
	tp9950_write_register(ViPipe, 0x06, 0x32);
	tp9950_write_register(ViPipe, 0x07, 0xC0);
	tp9950_write_register(ViPipe, 0x08, 0x00);
	tp9950_write_register(ViPipe, 0x09, 0x24);
	tp9950_write_register(ViPipe, 0x0A, 0x48);
	tp9950_write_register(ViPipe, 0x0B, 0xC0);
	tp9950_write_register(ViPipe, 0x0C, 0x03);
	tp9950_write_register(ViPipe, 0x0D, 0x71);
	tp9950_write_register(ViPipe, 0x0E, 0x00);
	tp9950_write_register(ViPipe, 0x0F, 0x00);
	tp9950_write_register(ViPipe, 0x10, 0x00);
	tp9950_write_register(ViPipe, 0x11, 0x40);
	tp9950_write_register(ViPipe, 0x12, 0x40);
	tp9950_write_register(ViPipe, 0x13, 0x00);
	tp9950_write_register(ViPipe, 0x14, 0x00);
	tp9950_write_register(ViPipe, 0x15, 0x13);
	tp9950_write_register(ViPipe, 0x16, 0x16);
	tp9950_write_register(ViPipe, 0x17, 0x00);
	tp9950_write_register(ViPipe, 0x18, 0x1B);
	tp9950_write_register(ViPipe, 0x19, 0xD0);
	tp9950_write_register(ViPipe, 0x1A, 0x25);
	tp9950_write_register(ViPipe, 0x1B, 0x01);
	tp9950_write_register(ViPipe, 0x1C, 0x07);
	tp9950_write_register(ViPipe, 0x1D, 0xBC);
	tp9950_write_register(ViPipe, 0x1E, 0x80);
	tp9950_write_register(ViPipe, 0x1F, 0x80);
	tp9950_write_register(ViPipe, 0x20, 0x40);
	tp9950_write_register(ViPipe, 0x21, 0x46);
	tp9950_write_register(ViPipe, 0x22, 0x36);
	tp9950_write_register(ViPipe, 0x23, 0x3C);
	tp9950_write_register(ViPipe, 0x24, 0x04);
	tp9950_write_register(ViPipe, 0x25, 0xFE);
	tp9950_write_register(ViPipe, 0x26, 0x01);
	tp9950_write_register(ViPipe, 0x27, 0x2D);
	tp9950_write_register(ViPipe, 0x28, 0x00);
	tp9950_write_register(ViPipe, 0x29, 0x48);
	tp9950_write_register(ViPipe, 0x2A, 0x30);
	tp9950_write_register(ViPipe, 0x2B, 0x60);
	tp9950_write_register(ViPipe, 0x2C, 0x3A);
	tp9950_write_register(ViPipe, 0x2D, 0x5A);
	tp9950_write_register(ViPipe, 0x2E, 0x40);
	tp9950_write_register(ViPipe, 0x2F, 0x00);
	tp9950_write_register(ViPipe, 0x30, 0x9E);
	tp9950_write_register(ViPipe, 0x31, 0x20);
	tp9950_write_register(ViPipe, 0x32, 0x10);
	tp9950_write_register(ViPipe, 0x33, 0x90);
	tp9950_write_register(ViPipe, 0x34, 0x00);
	tp9950_write_register(ViPipe, 0x35, 0x25);
	tp9950_write_register(ViPipe, 0x36, 0xDC);
	tp9950_write_register(ViPipe, 0x37, 0x00);
	tp9950_write_register(ViPipe, 0x38, 0x00);
	tp9950_write_register(ViPipe, 0x39, 0x18);
	tp9950_write_register(ViPipe, 0x3A, 0x32);
	tp9950_write_register(ViPipe, 0x3B, 0x26);
	tp9950_write_register(ViPipe, 0x3C, 0x00);
	tp9950_write_register(ViPipe, 0x3D, 0x60);
	tp9950_write_register(ViPipe, 0x3E, 0x00);
	tp9950_write_register(ViPipe, 0x3F, 0x00);
	tp9950_write_register(ViPipe, 0x40, 0x00);
	tp9950_write_register(ViPipe, 0x41, 0x00);
	tp9950_write_register(ViPipe, 0x42, 0x00);
	tp9950_write_register(ViPipe, 0x43, 0x00);
	tp9950_write_register(ViPipe, 0x44, 0x00);
	tp9950_write_register(ViPipe, 0x45, 0x00);
	tp9950_write_register(ViPipe, 0x46, 0x00);
	tp9950_write_register(ViPipe, 0x47, 0x00);
	tp9950_write_register(ViPipe, 0x48, 0x00);
	tp9950_write_register(ViPipe, 0x49, 0x00);
	tp9950_write_register(ViPipe, 0x4A, 0x00);
	tp9950_write_register(ViPipe, 0x4B, 0x00);
	tp9950_write_register(ViPipe, 0x4C, 0x43);
	tp9950_write_register(ViPipe, 0x4D, 0x00);
	tp9950_write_register(ViPipe, 0x4E, 0x17);
	tp9950_write_register(ViPipe, 0x4F, 0x00);

	tp9950_write_register(ViPipe, 0xF0, 0x00);
	tp9950_write_register(ViPipe, 0xF1, 0x00);
	tp9950_write_register(ViPipe, 0xF2, 0x00);
	tp9950_write_register(ViPipe, 0xF3, 0x00);
	tp9950_write_register(ViPipe, 0xF4, 0x20);
	tp9950_write_register(ViPipe, 0xF5, 0x10);
	tp9950_write_register(ViPipe, 0xF6, 0x00);
	tp9950_write_register(ViPipe, 0xF7, 0x00);
	tp9950_write_register(ViPipe, 0xF8, 0x00);
	tp9950_write_register(ViPipe, 0xF9, 0x00);
	tp9950_write_register(ViPipe, 0xFA, 0x88);
	tp9950_write_register(ViPipe, 0xFB, 0x00);
	tp9950_write_register(ViPipe, 0xFC, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x08);
	tp9950_write_register(ViPipe, 0x01, 0xf8);
	tp9950_write_register(ViPipe, 0x02, 0x00);
	tp9950_write_register(ViPipe, 0x08, 0x0f);
	tp9950_write_register(ViPipe, 0x13, 0x24);
	tp9950_write_register(ViPipe, 0x14, 0x46);
	tp9950_write_register(ViPipe, 0x15, 0x08);
	tp9950_write_register(ViPipe, 0x20, 0x12);
	tp9950_write_register(ViPipe, 0x34, 0x1b);
	tp9950_write_register(ViPipe, 0x23, 0x02);
	tp9950_write_register(ViPipe, 0x23, 0x00);

	tp9950_write_register(ViPipe, 0x40, 0x00);

	printf("ViPipe:%d,===TP9950 720P 25fps Init OK!===\n", ViPipe);
}

int AHD_tp9950_Init(VI_PIPE ViPipe, bool isFirstInit)
{
	if (isFirstInit) {
		if (tp9950_gpio_init(ViPipe) != CVI_SUCCESS) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "tp9950 gpio init fail\n");
			return CVI_FAILURE;
		}
	}

	tp9950_init(ViPipe);

	if (isFirstInit) {
		printf("\nLoading TP9950 sensor\n");
		tp9950_fw_init(ViPipe);
	}

	return CVI_SUCCESS;
}

int AHD_tp9950_Deinit(VI_PIPE ViPipe)
{
	return CVI_SUCCESS;
}

CVI_S32 AHD_tp9950_set_mode(VI_PIPE ViPipe, CVI_S32 mode)
{
	switch (mode) {
	case AHD_MODE_1280X720P25:
		tp9950_set_720p_25(ViPipe);
		break;
	case AHD_MODE_1280X720P30:
		tp9950_set_720p_30(ViPipe);
		break;
	case AHD_MODE_1920X1080P25:
		tp9950_set_1080p_25(ViPipe);
		break;
	case AHD_MODE_1920X1080P30:
		tp9950_set_1080p_30(ViPipe);
		break;
	default:
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 AHD_tp9950_get_mode(VI_PIPE ViPipe)
{
	CVI_U8 lockstatus = 0;
	CVI_U8 detvideo = 0;
	SNS_AHD_MODE_S signal_type = AHD_MODE_NONE;

	lockstatus = tp9950_read_register(ViPipe, 0x01);
	detvideo = tp9950_read_register(ViPipe, 0x03);
	CVI_TRACE_SNS(CVI_DBG_INFO, "detvideo = 0x%2x, lockstatus = 0x%2x!!!\n",
			detvideo, lockstatus);

	if (((lockstatus & 0x80) == 0x00) &&
		(((lockstatus & 0x69) == 0x68) || ((detvideo & 0x07) < 0x6))) { // camera plug in
		// for  mode catch start
		if ((detvideo & 0x07) == 0x05) { //720p25fps
				signal_type = AHD_MODE_1280X720P25;
		} else if ((detvideo & 0x07) == 0x04) { //720p30fps
				signal_type = AHD_MODE_1280X720P30;
		} else if ((detvideo & 0x07) == 0x03) { //1080p25fps
				signal_type = AHD_MODE_1920X1080P25;
		} else if ((detvideo & 0x07) == 0x02) { //1080p30fps
				signal_type = AHD_MODE_1920X1080P30;
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "detect nothing! detvideo = 0x%2x, lockstatus = 0x%2x\n",
				detvideo, lockstatus);
			signal_type = AHD_MODE_NONE;
			return signal_type;
		}
	} else {
		signal_type = AHD_MODE_NONE;
		CVI_TRACE_SNS(CVI_DBG_INFO, "tp9950 has no signal!\n");
	}

	return signal_type;
}

CVI_S32 AHD_tp9950_set_bus_info(VI_PIPE ViPipe, CVI_S32 astI2cDev)
{
	if (ViPipe > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "invalid vipipe !!\n");
		return CVI_FAILURE;
	}
	g_aunTP9950_BusInfo[ViPipe].s8I2cDev = astI2cDev;
	return CVI_SUCCESS;
}

SNS_AHD_OBJ_S stAhdTp9950Obj =
{
    .pfnAhdInit = AHD_tp9950_Init,
    .pfnAhdDeinit = AHD_tp9950_Deinit,
    .pfnGetAhdMode = AHD_tp9950_get_mode,
    .pfnSetAhdMode = AHD_tp9950_set_mode,
    .pfnSetAhdBusInfo = AHD_tp9950_set_bus_info,
};
