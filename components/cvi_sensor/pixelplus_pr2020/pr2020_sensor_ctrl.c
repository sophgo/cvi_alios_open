#include <unistd.h>

#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "cvi_sensor.h"

#include "pr2020_cmos_ex.h"

#define AHD_PWDN_GRP 1
#define AHD_PWDN_NUM 5

const CVI_U8 pr2020_i2c_addr = 0x5C;        /* I2C slave address of PR2020*/
const CVI_U32 pr2020_addr_byte = 1;
const CVI_U32 pr2020_data_byte = 1;
static CVI_U32 detect_cnt = 0;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};
csi_gpio_t g_irgpio = {0};

int pr2020_gpio_init(VI_PIPE ViPipe)
{
	(void) ViPipe;

	// AHD_PWR_EN
    if (csi_gpio_init(&g_irgpio, AHD_PWDN_GRP) != CSI_OK) {
        printf("%s gpio init err \n",__func__);
        return CVI_FAILURE;
    }
	csi_gpio_dir(&g_irgpio, (1 << AHD_PWDN_NUM), GPIO_DIRECTION_OUTPUT);
	csi_gpio_write(&g_irgpio, (1 << AHD_PWDN_NUM), 1);
//	if (csi_gpio_read(&g_irgpio , (1 << IR_PWDN_NUM)) != 1){
//		return CVI_FAILURE;
//	}

	return CVI_SUCCESS;
}

int pr2020_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunPr2020_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_init(i2c_id);
}

int pr2020_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunPr2020_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int pr2020_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunPr2020_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, pr2020_i2c_addr, (CVI_U32)addr, pr2020_addr_byte, pr2020_data_byte);
}

int pr2020_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunPr2020_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, pr2020_i2c_addr, (CVI_U32)addr, pr2020_addr_byte,
		(CVI_U32)data, pr2020_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void pr2020_fw_init(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	pr2020_write_register(ViPipe, 0xFF, 0x00);
	pr2020_write_register(ViPipe, 0xD0, 0x30);
	pr2020_write_register(ViPipe, 0xD1, 0x08);
	pr2020_write_register(ViPipe, 0xD2, 0x21);
	pr2020_write_register(ViPipe, 0xD3, 0x00);
	pr2020_write_register(ViPipe, 0xD8, 0x37);
	pr2020_write_register(ViPipe, 0xD9, 0x08);

	pr2020_write_register(ViPipe, 0xFF, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xE4);//no-video data, 0xe4: black, 0xe5: blue
	pr2020_write_register(ViPipe, 0x01, 0x61);
	pr2020_write_register(ViPipe, 0x02, 0x00);
	pr2020_write_register(ViPipe, 0x03, 0x57);
	pr2020_write_register(ViPipe, 0x04, 0x0C);
	pr2020_write_register(ViPipe, 0x05, 0x88);
	pr2020_write_register(ViPipe, 0x06, 0x04);
	pr2020_write_register(ViPipe, 0x07, 0xB2);
	pr2020_write_register(ViPipe, 0x08, 0x44);
	pr2020_write_register(ViPipe, 0x09, 0x34);
	pr2020_write_register(ViPipe, 0x0A, 0x02);
	pr2020_write_register(ViPipe, 0x0B, 0x14);
	pr2020_write_register(ViPipe, 0x0C, 0x04);
	pr2020_write_register(ViPipe, 0x0D, 0x08);
	pr2020_write_register(ViPipe, 0x0E, 0x5E);
	pr2020_write_register(ViPipe, 0x0F, 0x5E);
	pr2020_write_register(ViPipe, 0x10, 0x26);
	printf("ViPipe:%d,===PR2020 FW Init OK!===\n", ViPipe);
}

void pr2020_set_cvbs_ntsc_60(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys
	pr2020_write_register(ViPipe, 0xff, 0x00);
	pr2020_write_register(ViPipe, 0x10, 0x30);
	pr2020_write_register(ViPipe, 0x11, 0x07);
	pr2020_write_register(ViPipe, 0x12, 0x00);
	pr2020_write_register(ViPipe, 0x13, 0x00);
	pr2020_write_register(ViPipe, 0x14, 0x21);//b[1:0); => Select Camera Input. VinP(1), VinN(3), Differ(0).
	pr2020_write_register(ViPipe, 0x15, 0x44);
	pr2020_write_register(ViPipe, 0x16, 0x0d);
	pr2020_write_register(ViPipe, 0x40, 0x00);
	pr2020_write_register(ViPipe, 0x47, 0x36);
	pr2020_write_register(ViPipe, 0x4e, 0x3f);
	pr2020_write_register(ViPipe, 0x80, 0x56);
	pr2020_write_register(ViPipe, 0x81, 0x0e);
	pr2020_write_register(ViPipe, 0x82, 0x0d);
	pr2020_write_register(ViPipe, 0x84, 0x30);
	pr2020_write_register(ViPipe, 0x86, 0x20);
	pr2020_write_register(ViPipe, 0x87, 0x00);
	pr2020_write_register(ViPipe, 0x8a, 0x00);
	pr2020_write_register(ViPipe, 0x90, 0x00);
	pr2020_write_register(ViPipe, 0x91, 0x00);
	pr2020_write_register(ViPipe, 0x92, 0x00);
	pr2020_write_register(ViPipe, 0x94, 0xff);
	pr2020_write_register(ViPipe, 0x95, 0xff);
	pr2020_write_register(ViPipe, 0x96, 0xff);
	pr2020_write_register(ViPipe, 0xa0, 0x00);
	pr2020_write_register(ViPipe, 0xa1, 0x20);
	pr2020_write_register(ViPipe, 0xa4, 0x01);
	pr2020_write_register(ViPipe, 0xa5, 0xe3);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0x12);
	pr2020_write_register(ViPipe, 0xa8, 0x00);
	pr2020_write_register(ViPipe, 0xd0, 0x30);
	pr2020_write_register(ViPipe, 0xd1, 0x08);
	pr2020_write_register(ViPipe, 0xd2, 0x21);
	pr2020_write_register(ViPipe, 0xd3, 0x00);
	pr2020_write_register(ViPipe, 0xd8, 0x31);
	pr2020_write_register(ViPipe, 0xd9, 0x08);
	pr2020_write_register(ViPipe, 0xda, 0x21);
	pr2020_write_register(ViPipe, 0xe0, 0x3e);
	pr2020_write_register(ViPipe, 0xe1, 0x80);
	pr2020_write_register(ViPipe, 0xe2, 0x38);
	pr2020_write_register(ViPipe, 0xe3, 0x19);
	pr2020_write_register(ViPipe, 0xe4, 0x19);
	pr2020_write_register(ViPipe, 0xea, 0x01);
	pr2020_write_register(ViPipe, 0xeb, 0xff);
	pr2020_write_register(ViPipe, 0xf1, 0x44);
	pr2020_write_register(ViPipe, 0xf2, 0x01);

	//Page1 vdec
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xe4);
	pr2020_write_register(ViPipe, 0x01, 0x61);
	pr2020_write_register(ViPipe, 0x02, 0x00);
	pr2020_write_register(ViPipe, 0x03, 0x57);
	pr2020_write_register(ViPipe, 0x04, 0x0c);
	pr2020_write_register(ViPipe, 0x05, 0x88);
	pr2020_write_register(ViPipe, 0x06, 0x04);
	pr2020_write_register(ViPipe, 0x07, 0xb2);
	pr2020_write_register(ViPipe, 0x08, 0x44);
	pr2020_write_register(ViPipe, 0x09, 0x34);
	pr2020_write_register(ViPipe, 0x0a, 0x02);
	pr2020_write_register(ViPipe, 0x0b, 0x14);
	pr2020_write_register(ViPipe, 0x0c, 0x04);
	pr2020_write_register(ViPipe, 0x0d, 0x08);
	pr2020_write_register(ViPipe, 0x0e, 0x5e);
	pr2020_write_register(ViPipe, 0x0f, 0x5e);
	pr2020_write_register(ViPipe, 0x10, 0x26);
	pr2020_write_register(ViPipe, 0x11, 0x00);
	pr2020_write_register(ViPipe, 0x12, 0x02);
	pr2020_write_register(ViPipe, 0x13, 0x90);
	pr2020_write_register(ViPipe, 0x14, 0xd0);
	pr2020_write_register(ViPipe, 0x15, 0x10);
	pr2020_write_register(ViPipe, 0x16, 0xf0);
	pr2020_write_register(ViPipe, 0x17, 0x00);
	pr2020_write_register(ViPipe, 0x18, 0x21);
	pr2020_write_register(ViPipe, 0x19, 0x4a);
	pr2020_write_register(ViPipe, 0x1a, 0x20);
	pr2020_write_register(ViPipe, 0x1b, 0x07);
	pr2020_write_register(ViPipe, 0x1c, 0x00);
	pr2020_write_register(ViPipe, 0x1d, 0x42);
	pr2020_write_register(ViPipe, 0x1e, 0x40);
	pr2020_write_register(ViPipe, 0x1f, 0xd0);
	pr2020_write_register(ViPipe, 0x20, 0x80);
	pr2020_write_register(ViPipe, 0x21, 0x70);
	pr2020_write_register(ViPipe, 0x22, 0x65);
	pr2020_write_register(ViPipe, 0x23, 0x83);
	pr2020_write_register(ViPipe, 0x24, 0x80);
	pr2020_write_register(ViPipe, 0x25, 0x80);
	pr2020_write_register(ViPipe, 0x26, 0x84);
	pr2020_write_register(ViPipe, 0x27, 0x80);
	pr2020_write_register(ViPipe, 0x28, 0x00);
	pr2020_write_register(ViPipe, 0x29, 0x5f);
	pr2020_write_register(ViPipe, 0x2a, 0x20);
	pr2020_write_register(ViPipe, 0x2b, 0x00);
	pr2020_write_register(ViPipe, 0x2c, 0x00);
	pr2020_write_register(ViPipe, 0x2d, 0x00);
	pr2020_write_register(ViPipe, 0x2e, 0x00);
	pr2020_write_register(ViPipe, 0x2f, 0x00);
	pr2020_write_register(ViPipe, 0x30, 0x00);
	pr2020_write_register(ViPipe, 0x31, 0x00);
	pr2020_write_register(ViPipe, 0x32, 0xc0);
	pr2020_write_register(ViPipe, 0x33, 0x14);
	pr2020_write_register(ViPipe, 0x34, 0x14);
	pr2020_write_register(ViPipe, 0x35, 0x80);
	pr2020_write_register(ViPipe, 0x36, 0x80);
	pr2020_write_register(ViPipe, 0x37, 0xe2);
	pr2020_write_register(ViPipe, 0x38, 0x41);
	pr2020_write_register(ViPipe, 0x39, 0x00);
	pr2020_write_register(ViPipe, 0x3a, 0xac);
	pr2020_write_register(ViPipe, 0x3b, 0x04);
	pr2020_write_register(ViPipe, 0x3c, 0x01);
	pr2020_write_register(ViPipe, 0x3d, 0x21);
	pr2020_write_register(ViPipe, 0x3e, 0x06);
	pr2020_write_register(ViPipe, 0x3f, 0xd5);
	pr2020_write_register(ViPipe, 0x40, 0x05);
	pr2020_write_register(ViPipe, 0x41, 0x55);
	pr2020_write_register(ViPipe, 0x42, 0x01);
	pr2020_write_register(ViPipe, 0x43, 0x30);
	pr2020_write_register(ViPipe, 0x44, 0x6a);
	pr2020_write_register(ViPipe, 0x45, 0x00);
	pr2020_write_register(ViPipe, 0x46, 0x06);
	pr2020_write_register(ViPipe, 0x47, 0x2b);
	pr2020_write_register(ViPipe, 0x48, 0xb9);
	pr2020_write_register(ViPipe, 0x49, 0x00);
	pr2020_write_register(ViPipe, 0x4a, 0x7b);
	pr2020_write_register(ViPipe, 0x4b, 0x60);
	pr2020_write_register(ViPipe, 0x4c, 0x00);
	pr2020_write_register(ViPipe, 0x4d, 0x47);
	pr2020_write_register(ViPipe, 0x4e, 0x02);
	pr2020_write_register(ViPipe, 0x4f, 0x20);
	pr2020_write_register(ViPipe, 0x50, 0x01);
	pr2020_write_register(ViPipe, 0x51, 0x28);
	pr2020_write_register(ViPipe, 0x52, 0x40);
	pr2020_write_register(ViPipe, 0x53, 0x0c);
	pr2020_write_register(ViPipe, 0x54, 0x0f);
	pr2020_write_register(ViPipe, 0x55, 0x8d);
	pr2020_write_register(ViPipe, 0x70, 0x06);
	pr2020_write_register(ViPipe, 0x71, 0x08);
	pr2020_write_register(ViPipe, 0x72, 0x0a);
	pr2020_write_register(ViPipe, 0x73, 0x0c);
	pr2020_write_register(ViPipe, 0x74, 0x0e);
	pr2020_write_register(ViPipe, 0x75, 0x10);
	pr2020_write_register(ViPipe, 0x76, 0x12);
	pr2020_write_register(ViPipe, 0x77, 0x14);
	pr2020_write_register(ViPipe, 0x78, 0x06);
	pr2020_write_register(ViPipe, 0x79, 0x08);
	pr2020_write_register(ViPipe, 0x7a, 0x0a);
	pr2020_write_register(ViPipe, 0x7b, 0x0c);
	pr2020_write_register(ViPipe, 0x7c, 0x0e);
	pr2020_write_register(ViPipe, 0x7d, 0x10);
	pr2020_write_register(ViPipe, 0x7e, 0x12);
	pr2020_write_register(ViPipe, 0x7f, 0x14);
	pr2020_write_register(ViPipe, 0x80, 0x00);
	pr2020_write_register(ViPipe, 0x81, 0x09);
	pr2020_write_register(ViPipe, 0x82, 0x00);
	pr2020_write_register(ViPipe, 0x83, 0x07);
	pr2020_write_register(ViPipe, 0x84, 0x00);
	pr2020_write_register(ViPipe, 0x85, 0x17);
	pr2020_write_register(ViPipe, 0x86, 0x03);
	pr2020_write_register(ViPipe, 0x87, 0xe5);
	pr2020_write_register(ViPipe, 0x88, 0x04);
	pr2020_write_register(ViPipe, 0x89, 0x48);
	pr2020_write_register(ViPipe, 0x8a, 0x04);
	pr2020_write_register(ViPipe, 0x8b, 0x48);
	pr2020_write_register(ViPipe, 0x8c, 0x05);
	pr2020_write_register(ViPipe, 0x8d, 0xf0);
	pr2020_write_register(ViPipe, 0x8e, 0x05);
	pr2020_write_register(ViPipe, 0x8f, 0x47);
	pr2020_write_register(ViPipe, 0x90, 0x03);
	pr2020_write_register(ViPipe, 0x91, 0x13);
	pr2020_write_register(ViPipe, 0x92, 0x73);
	pr2020_write_register(ViPipe, 0x93, 0xe8);
	pr2020_write_register(ViPipe, 0x94, 0x0f);
	pr2020_write_register(ViPipe, 0x95, 0x5e);
	pr2020_write_register(ViPipe, 0x96, 0x03);
	pr2020_write_register(ViPipe, 0x97, 0xd0);
	pr2020_write_register(ViPipe, 0x98, 0x17);
	pr2020_write_register(ViPipe, 0x99, 0x34);
	pr2020_write_register(ViPipe, 0x9a, 0x13);
	pr2020_write_register(ViPipe, 0x9b, 0x56);
	pr2020_write_register(ViPipe, 0x9c, 0x0b);
	pr2020_write_register(ViPipe, 0x9d, 0x9a);
	pr2020_write_register(ViPipe, 0x9e, 0x09);
	pr2020_write_register(ViPipe, 0x9f, 0xab);
	pr2020_write_register(ViPipe, 0xa0, 0x01);
	pr2020_write_register(ViPipe, 0xa1, 0x74);
	pr2020_write_register(ViPipe, 0xa2, 0x01);
	pr2020_write_register(ViPipe, 0xa3, 0x6b);
	pr2020_write_register(ViPipe, 0xa4, 0x00);
	pr2020_write_register(ViPipe, 0xa5, 0xba);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0xa3);
	pr2020_write_register(ViPipe, 0xa8, 0x01);
	pr2020_write_register(ViPipe, 0xa9, 0x39);
	pr2020_write_register(ViPipe, 0xaa, 0x01);
	pr2020_write_register(ViPipe, 0xab, 0x39);
	pr2020_write_register(ViPipe, 0xac, 0x00);
	pr2020_write_register(ViPipe, 0xad, 0xc1);
	pr2020_write_register(ViPipe, 0xae, 0x00);
	pr2020_write_register(ViPipe, 0xaf, 0xc1);
	pr2020_write_register(ViPipe, 0xb0, 0x04);
	pr2020_write_register(ViPipe, 0xb1, 0xd4);
	pr2020_write_register(ViPipe, 0xb2, 0x07);
	pr2020_write_register(ViPipe, 0xb3, 0xda);
	pr2020_write_register(ViPipe, 0xb4, 0x00);
	pr2020_write_register(ViPipe, 0xb5, 0x17);
	pr2020_write_register(ViPipe, 0xb6, 0x08);
	pr2020_write_register(ViPipe, 0xb7, 0xe8);
	pr2020_write_register(ViPipe, 0xb8, 0xb0);
	pr2020_write_register(ViPipe, 0xb9, 0xce);
	pr2020_write_register(ViPipe, 0xba, 0x90);
	pr2020_write_register(ViPipe, 0xbb, 0x10);
	pr2020_write_register(ViPipe, 0xbc, 0x00);
	pr2020_write_register(ViPipe, 0xbd, 0x04);
	pr2020_write_register(ViPipe, 0xbe, 0x02);
	pr2020_write_register(ViPipe, 0xbf, 0xd0);
	pr2020_write_register(ViPipe, 0xc0, 0x00);
	pr2020_write_register(ViPipe, 0xc1, 0x10);
	pr2020_write_register(ViPipe, 0xc2, 0x00);
	pr2020_write_register(ViPipe, 0xc3, 0xf0);

	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0e);
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0f);
	printf("ViPipe:%d,==SET PR2020 CVBS_NTSC OK!===\n", ViPipe);
}

void pr2020_set_cvbs_pal_50(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys
	pr2020_write_register(ViPipe, 0xff, 0x00);
	pr2020_write_register(ViPipe, 0x10, 0x21);
	pr2020_write_register(ViPipe, 0x11, 0x07);
	pr2020_write_register(ViPipe, 0x12, 0x00);
	pr2020_write_register(ViPipe, 0x13, 0x00);
	pr2020_write_register(ViPipe, 0x14, 0x21);//b[1:0); => Select Camera Input. VinP(1), VinN(3), Differ(0).
	pr2020_write_register(ViPipe, 0x15, 0x44);
	pr2020_write_register(ViPipe, 0x16, 0x0d);
	pr2020_write_register(ViPipe, 0x40, 0x00);
	pr2020_write_register(ViPipe, 0x47, 0x36);
	pr2020_write_register(ViPipe, 0x4e, 0x3f);
	pr2020_write_register(ViPipe, 0x80, 0x56);
	pr2020_write_register(ViPipe, 0x81, 0x0e);
	pr2020_write_register(ViPipe, 0x82, 0x0d);
	pr2020_write_register(ViPipe, 0x84, 0x30);
	pr2020_write_register(ViPipe, 0x86, 0x20);
	pr2020_write_register(ViPipe, 0x87, 0x00);
	pr2020_write_register(ViPipe, 0x8a, 0x00);
	pr2020_write_register(ViPipe, 0x90, 0x00);
	pr2020_write_register(ViPipe, 0x91, 0x00);
	pr2020_write_register(ViPipe, 0x92, 0x00);
	pr2020_write_register(ViPipe, 0x94, 0xff);
	pr2020_write_register(ViPipe, 0x95, 0xff);
	pr2020_write_register(ViPipe, 0x96, 0xff);
	pr2020_write_register(ViPipe, 0xa0, 0x00);
	pr2020_write_register(ViPipe, 0xa1, 0x20);
	pr2020_write_register(ViPipe, 0xa4, 0x01);
	pr2020_write_register(ViPipe, 0xa5, 0xe3);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0x12);
	pr2020_write_register(ViPipe, 0xa8, 0x00);
	pr2020_write_register(ViPipe, 0xd0, 0x30);
	pr2020_write_register(ViPipe, 0xd1, 0x08);
	pr2020_write_register(ViPipe, 0xd2, 0x21);
	pr2020_write_register(ViPipe, 0xd3, 0x00);
	pr2020_write_register(ViPipe, 0xd8, 0x31);
	pr2020_write_register(ViPipe, 0xd9, 0x08);
	pr2020_write_register(ViPipe, 0xda, 0x21);
	pr2020_write_register(ViPipe, 0xe0, 0x3e);
	pr2020_write_register(ViPipe, 0xe1, 0x80);
	pr2020_write_register(ViPipe, 0xe2, 0x38);
	pr2020_write_register(ViPipe, 0xe3, 0x19);
	pr2020_write_register(ViPipe, 0xe4, 0x19);
	pr2020_write_register(ViPipe, 0xea, 0x01);
	pr2020_write_register(ViPipe, 0xeb, 0xff);
	pr2020_write_register(ViPipe, 0xf1, 0x44);
	pr2020_write_register(ViPipe, 0xf2, 0x01);

	//Page1 vdec
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xe4);
	pr2020_write_register(ViPipe, 0x01, 0x61);
	pr2020_write_register(ViPipe, 0x02, 0x00);
	pr2020_write_register(ViPipe, 0x03, 0x57);
	pr2020_write_register(ViPipe, 0x04, 0x0c);
	pr2020_write_register(ViPipe, 0x05, 0x88);
	pr2020_write_register(ViPipe, 0x06, 0x04);
	pr2020_write_register(ViPipe, 0x07, 0xb2);
	pr2020_write_register(ViPipe, 0x08, 0x44);
	pr2020_write_register(ViPipe, 0x09, 0x34);
	pr2020_write_register(ViPipe, 0x0a, 0x02);
	pr2020_write_register(ViPipe, 0x0b, 0x14);
	pr2020_write_register(ViPipe, 0x0c, 0x04);
	pr2020_write_register(ViPipe, 0x0d, 0x08);
	pr2020_write_register(ViPipe, 0x0e, 0x5e);
	pr2020_write_register(ViPipe, 0x0f, 0x5e);
	pr2020_write_register(ViPipe, 0x10, 0x26);
	pr2020_write_register(ViPipe, 0x11, 0x00);
	pr2020_write_register(ViPipe, 0x12, 0x22);
	pr2020_write_register(ViPipe, 0x13, 0xe0);
	pr2020_write_register(ViPipe, 0x14, 0xd0);
	pr2020_write_register(ViPipe, 0x15, 0x16);
	pr2020_write_register(ViPipe, 0x16, 0x20);
	pr2020_write_register(ViPipe, 0x17, 0x00);
	pr2020_write_register(ViPipe, 0x18, 0x21);
	pr2020_write_register(ViPipe, 0x19, 0x4a);
	pr2020_write_register(ViPipe, 0x1a, 0x20);
	pr2020_write_register(ViPipe, 0x1b, 0x06);
	pr2020_write_register(ViPipe, 0x1c, 0x31);
	pr2020_write_register(ViPipe, 0x1d, 0x42);
	pr2020_write_register(ViPipe, 0x1e, 0x50);
	pr2020_write_register(ViPipe, 0x1f, 0xd0);
	pr2020_write_register(ViPipe, 0x20, 0x80);
	pr2020_write_register(ViPipe, 0x21, 0x75);
	pr2020_write_register(ViPipe, 0x22, 0x65);
	pr2020_write_register(ViPipe, 0x23, 0x83);
	pr2020_write_register(ViPipe, 0x24, 0x80);
	pr2020_write_register(ViPipe, 0x25, 0x80);
	pr2020_write_register(ViPipe, 0x26, 0x84);
	pr2020_write_register(ViPipe, 0x27, 0x80);
	pr2020_write_register(ViPipe, 0x28, 0x00);
	pr2020_write_register(ViPipe, 0x29, 0x5f);
	pr2020_write_register(ViPipe, 0x2a, 0x20);
	pr2020_write_register(ViPipe, 0x2b, 0x00);
	pr2020_write_register(ViPipe, 0x2c, 0x00);
	pr2020_write_register(ViPipe, 0x2d, 0x00);
	pr2020_write_register(ViPipe, 0x2e, 0x00);
	pr2020_write_register(ViPipe, 0x2f, 0x00);
	pr2020_write_register(ViPipe, 0x30, 0x00);
	pr2020_write_register(ViPipe, 0x31, 0x00);
	pr2020_write_register(ViPipe, 0x32, 0xc0);
	pr2020_write_register(ViPipe, 0x33, 0x20);
	pr2020_write_register(ViPipe, 0x34, 0x20);
	pr2020_write_register(ViPipe, 0x35, 0x10);
	pr2020_write_register(ViPipe, 0x36, 0x10);
	pr2020_write_register(ViPipe, 0x37, 0xc4);
	pr2020_write_register(ViPipe, 0x38, 0x42);
	pr2020_write_register(ViPipe, 0x39, 0x00);
	pr2020_write_register(ViPipe, 0x3a, 0xac);
	pr2020_write_register(ViPipe, 0x3b, 0x04);
	pr2020_write_register(ViPipe, 0x3c, 0x01);
	pr2020_write_register(ViPipe, 0x3d, 0x21);
	pr2020_write_register(ViPipe, 0x3e, 0x06);
	pr2020_write_register(ViPipe, 0x3f, 0xd5);
	pr2020_write_register(ViPipe, 0x40, 0x85);
	pr2020_write_register(ViPipe, 0x41, 0x55);
	pr2020_write_register(ViPipe, 0x42, 0x01);
	pr2020_write_register(ViPipe, 0x43, 0x31);
	pr2020_write_register(ViPipe, 0x44, 0x6a);
	pr2020_write_register(ViPipe, 0x45, 0x00);
	pr2020_write_register(ViPipe, 0x46, 0x07);
	pr2020_write_register(ViPipe, 0x47, 0xa4);
	pr2020_write_register(ViPipe, 0x48, 0xa5);
	pr2020_write_register(ViPipe, 0x49, 0x00);
	pr2020_write_register(ViPipe, 0x4a, 0x7b);
	pr2020_write_register(ViPipe, 0x4b, 0x60);
	pr2020_write_register(ViPipe, 0x4c, 0x00);
	pr2020_write_register(ViPipe, 0x4d, 0x47);
	pr2020_write_register(ViPipe, 0x4e, 0x02);
	pr2020_write_register(ViPipe, 0x4f, 0x20);
	pr2020_write_register(ViPipe, 0x50, 0x01);
	pr2020_write_register(ViPipe, 0x51, 0x28);
	pr2020_write_register(ViPipe, 0x52, 0x40);
	pr2020_write_register(ViPipe, 0x53, 0x0c);
	pr2020_write_register(ViPipe, 0x54, 0x0f);
	pr2020_write_register(ViPipe, 0x55, 0x8d);
	pr2020_write_register(ViPipe, 0x70, 0x06);
	pr2020_write_register(ViPipe, 0x71, 0x08);
	pr2020_write_register(ViPipe, 0x72, 0x0a);
	pr2020_write_register(ViPipe, 0x73, 0x0c);
	pr2020_write_register(ViPipe, 0x74, 0x0e);
	pr2020_write_register(ViPipe, 0x75, 0x10);
	pr2020_write_register(ViPipe, 0x76, 0x12);
	pr2020_write_register(ViPipe, 0x77, 0x14);
	pr2020_write_register(ViPipe, 0x78, 0x06);
	pr2020_write_register(ViPipe, 0x79, 0x08);
	pr2020_write_register(ViPipe, 0x7a, 0x0a);
	pr2020_write_register(ViPipe, 0x7b, 0x0c);
	pr2020_write_register(ViPipe, 0x7c, 0x0e);
	pr2020_write_register(ViPipe, 0x7d, 0x10);
	pr2020_write_register(ViPipe, 0x7e, 0x12);
	pr2020_write_register(ViPipe, 0x7f, 0x14);
	pr2020_write_register(ViPipe, 0x80, 0x00);
	pr2020_write_register(ViPipe, 0x81, 0x09);
	pr2020_write_register(ViPipe, 0x82, 0x00);
	pr2020_write_register(ViPipe, 0x83, 0x07);
	pr2020_write_register(ViPipe, 0x84, 0x00);
	pr2020_write_register(ViPipe, 0x85, 0x17);
	pr2020_write_register(ViPipe, 0x86, 0x03);
	pr2020_write_register(ViPipe, 0x87, 0xe5);
	pr2020_write_register(ViPipe, 0x88, 0x05);
	pr2020_write_register(ViPipe, 0x89, 0x24);
	pr2020_write_register(ViPipe, 0x8a, 0x05);
	pr2020_write_register(ViPipe, 0x8b, 0x24);
	pr2020_write_register(ViPipe, 0x8c, 0x05);
	pr2020_write_register(ViPipe, 0x8d, 0xf0);
	pr2020_write_register(ViPipe, 0x8e, 0x05);
	pr2020_write_register(ViPipe, 0x8f, 0x47);
	pr2020_write_register(ViPipe, 0x90, 0x02);
	pr2020_write_register(ViPipe, 0x91, 0xb4);
	pr2020_write_register(ViPipe, 0x92, 0x73);
	pr2020_write_register(ViPipe, 0x93, 0xe8);
	pr2020_write_register(ViPipe, 0x94, 0x0f);
	pr2020_write_register(ViPipe, 0x95, 0x5e);
	pr2020_write_register(ViPipe, 0x96, 0x03);
	pr2020_write_register(ViPipe, 0x97, 0xd0);
	pr2020_write_register(ViPipe, 0x98, 0x17);
	pr2020_write_register(ViPipe, 0x99, 0x34);
	pr2020_write_register(ViPipe, 0x9a, 0x13);
	pr2020_write_register(ViPipe, 0x9b, 0x56);
	pr2020_write_register(ViPipe, 0x9c, 0x0b);
	pr2020_write_register(ViPipe, 0x9d, 0x9a);
	pr2020_write_register(ViPipe, 0x9e, 0x09);
	pr2020_write_register(ViPipe, 0x9f, 0xab);
	pr2020_write_register(ViPipe, 0xa0, 0x01);
	pr2020_write_register(ViPipe, 0xa1, 0x74);
	pr2020_write_register(ViPipe, 0xa2, 0x01);
	pr2020_write_register(ViPipe, 0xa3, 0x6b);
	pr2020_write_register(ViPipe, 0xa4, 0x00);
	pr2020_write_register(ViPipe, 0xa5, 0xba);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0xa3);
	pr2020_write_register(ViPipe, 0xa8, 0x01);
	pr2020_write_register(ViPipe, 0xa9, 0x39);
	pr2020_write_register(ViPipe, 0xaa, 0x01);
	pr2020_write_register(ViPipe, 0xab, 0x39);
	pr2020_write_register(ViPipe, 0xac, 0x00);
	pr2020_write_register(ViPipe, 0xad, 0xc1);
	pr2020_write_register(ViPipe, 0xae, 0x00);
	pr2020_write_register(ViPipe, 0xaf, 0xc1);
	pr2020_write_register(ViPipe, 0xb0, 0x05);
	pr2020_write_register(ViPipe, 0xb1, 0xcc);
	pr2020_write_register(ViPipe, 0xb2, 0x09);
	pr2020_write_register(ViPipe, 0xb3, 0x6d);
	pr2020_write_register(ViPipe, 0xb4, 0x00);
	pr2020_write_register(ViPipe, 0xb5, 0x17);
	pr2020_write_register(ViPipe, 0xb6, 0x08);
	pr2020_write_register(ViPipe, 0xb7, 0xe8);
	pr2020_write_register(ViPipe, 0xb8, 0xb0);
	pr2020_write_register(ViPipe, 0xb9, 0xce);
	pr2020_write_register(ViPipe, 0xba, 0x90);
	pr2020_write_register(ViPipe, 0xbb, 0x10);
	pr2020_write_register(ViPipe, 0xbc, 0x00);
	pr2020_write_register(ViPipe, 0xbd, 0x04);
	pr2020_write_register(ViPipe, 0xbe, 0x02);
	pr2020_write_register(ViPipe, 0xbf, 0xd0);
	pr2020_write_register(ViPipe, 0xc0, 0x00);
	pr2020_write_register(ViPipe, 0xc1, 0x16);
	pr2020_write_register(ViPipe, 0xc2, 0x01);
	pr2020_write_register(ViPipe, 0xc3, 0x20);

	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0e);
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0f);
	printf("ViPipe:%d,==SET PR2020 CVBS_PAL OK!===\n", ViPipe);
}

void pr2020_set_720p_25(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys
	pr2020_write_register(ViPipe, 0xff, 0x00);
	pr2020_write_register(ViPipe, 0x10, 0x82);
	pr2020_write_register(ViPipe, 0x11, 0x07);
	pr2020_write_register(ViPipe, 0x12, 0x00);
	pr2020_write_register(ViPipe, 0x13, 0x00);
	pr2020_write_register(ViPipe, 0x14, 0x21);//b[1:0); => Select Camera Input. VinP(1), VinN(3), Differ(0).
	pr2020_write_register(ViPipe, 0x15, 0x44);
	pr2020_write_register(ViPipe, 0x16, 0x0d);
	pr2020_write_register(ViPipe, 0x40, 0x00);
	pr2020_write_register(ViPipe, 0x47, 0x3a);
	pr2020_write_register(ViPipe, 0x4e, 0x3f);
	pr2020_write_register(ViPipe, 0x80, 0x56);
	pr2020_write_register(ViPipe, 0x81, 0x0e);
	pr2020_write_register(ViPipe, 0x82, 0x0d);
	pr2020_write_register(ViPipe, 0x84, 0x30);
	pr2020_write_register(ViPipe, 0x86, 0x20);
	pr2020_write_register(ViPipe, 0x87, 0x00);
	pr2020_write_register(ViPipe, 0x8a, 0x00);
	pr2020_write_register(ViPipe, 0x90, 0x00);
	pr2020_write_register(ViPipe, 0x91, 0x00);
	pr2020_write_register(ViPipe, 0x92, 0x00);
	pr2020_write_register(ViPipe, 0x94, 0xff);
	pr2020_write_register(ViPipe, 0x95, 0xff);
	pr2020_write_register(ViPipe, 0x96, 0xff);
	pr2020_write_register(ViPipe, 0xa0, 0x00);
	pr2020_write_register(ViPipe, 0xa1, 0x20);
	pr2020_write_register(ViPipe, 0xa4, 0x01);
	pr2020_write_register(ViPipe, 0xa5, 0xe3);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0x12);
	pr2020_write_register(ViPipe, 0xa8, 0x00);
	pr2020_write_register(ViPipe, 0xd0, 0x30);
	pr2020_write_register(ViPipe, 0xd1, 0x08);
	pr2020_write_register(ViPipe, 0xd2, 0x21);
	pr2020_write_register(ViPipe, 0xd3, 0x00);
	pr2020_write_register(ViPipe, 0xd8, 0x31);
	pr2020_write_register(ViPipe, 0xd9, 0x08);
	pr2020_write_register(ViPipe, 0xda, 0x21);
	pr2020_write_register(ViPipe, 0xe0, 0x39);
	pr2020_write_register(ViPipe, 0xe1, 0x90);
	pr2020_write_register(ViPipe, 0xe2, 0x38);
	pr2020_write_register(ViPipe, 0xe3, 0x19);
	pr2020_write_register(ViPipe, 0xe4, 0x19);
	pr2020_write_register(ViPipe, 0xea, 0x01);
	pr2020_write_register(ViPipe, 0xeb, 0xff);
	pr2020_write_register(ViPipe, 0xf1, 0x44);
	pr2020_write_register(ViPipe, 0xf2, 0x01);

	//Page1 vdec
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xe4);
	pr2020_write_register(ViPipe, 0x01, 0x61);
	pr2020_write_register(ViPipe, 0x02, 0x00);
	pr2020_write_register(ViPipe, 0x03, 0x57);
	pr2020_write_register(ViPipe, 0x04, 0x0c);
	pr2020_write_register(ViPipe, 0x05, 0x88);
	pr2020_write_register(ViPipe, 0x06, 0x04);
	pr2020_write_register(ViPipe, 0x07, 0xb2);
	pr2020_write_register(ViPipe, 0x08, 0x44);
	pr2020_write_register(ViPipe, 0x09, 0x34);
	pr2020_write_register(ViPipe, 0x0a, 0x02);
	pr2020_write_register(ViPipe, 0x0b, 0x14);
	pr2020_write_register(ViPipe, 0x0c, 0x04);
	pr2020_write_register(ViPipe, 0x0d, 0x08);
	pr2020_write_register(ViPipe, 0x0e, 0x5e);
	pr2020_write_register(ViPipe, 0x0f, 0x5e);
	pr2020_write_register(ViPipe, 0x10, 0x26);
	pr2020_write_register(ViPipe, 0x11, 0x01);
	pr2020_write_register(ViPipe, 0x12, 0x45);
	pr2020_write_register(ViPipe, 0x13, 0x0c);
	pr2020_write_register(ViPipe, 0x14, 0x00);
	pr2020_write_register(ViPipe, 0x15, 0x1b);
	pr2020_write_register(ViPipe, 0x16, 0xd0);
	pr2020_write_register(ViPipe, 0x17, 0x00);
	pr2020_write_register(ViPipe, 0x18, 0x41);
	pr2020_write_register(ViPipe, 0x19, 0x46);
	pr2020_write_register(ViPipe, 0x1a, 0x22);
	pr2020_write_register(ViPipe, 0x1b, 0x05);
	pr2020_write_register(ViPipe, 0x1c, 0xea);
	pr2020_write_register(ViPipe, 0x1d, 0x45);
	pr2020_write_register(ViPipe, 0x1e, 0x4c);
	pr2020_write_register(ViPipe, 0x1f, 0x00);
	pr2020_write_register(ViPipe, 0x20, 0x80);
	pr2020_write_register(ViPipe, 0x21, 0x80);
	pr2020_write_register(ViPipe, 0x22, 0x90);
	pr2020_write_register(ViPipe, 0x23, 0x80);
	pr2020_write_register(ViPipe, 0x24, 0x80);
	pr2020_write_register(ViPipe, 0x25, 0x80);
	pr2020_write_register(ViPipe, 0x26, 0x84);
	pr2020_write_register(ViPipe, 0x27, 0x82);
	pr2020_write_register(ViPipe, 0x28, 0x00);
	pr2020_write_register(ViPipe, 0x29, 0x7d);
	pr2020_write_register(ViPipe, 0x2a, 0x00);
	pr2020_write_register(ViPipe, 0x2b, 0x00);
	pr2020_write_register(ViPipe, 0x2c, 0x00);
	pr2020_write_register(ViPipe, 0x2d, 0x00);
	pr2020_write_register(ViPipe, 0x2e, 0x00);
	pr2020_write_register(ViPipe, 0x2f, 0x00);
	pr2020_write_register(ViPipe, 0x30, 0x00);
	pr2020_write_register(ViPipe, 0x31, 0x00);
	pr2020_write_register(ViPipe, 0x32, 0xc0);
	pr2020_write_register(ViPipe, 0x33, 0x14);
	pr2020_write_register(ViPipe, 0x34, 0x14);
	pr2020_write_register(ViPipe, 0x35, 0x80);
	pr2020_write_register(ViPipe, 0x36, 0x80);
	pr2020_write_register(ViPipe, 0x37, 0xaa);
	pr2020_write_register(ViPipe, 0x38, 0x48);
	pr2020_write_register(ViPipe, 0x39, 0x08);
	pr2020_write_register(ViPipe, 0x3a, 0x27);
	pr2020_write_register(ViPipe, 0x3b, 0x02);
	pr2020_write_register(ViPipe, 0x3c, 0x01);
	pr2020_write_register(ViPipe, 0x3d, 0x23);
	pr2020_write_register(ViPipe, 0x3e, 0x02);
	pr2020_write_register(ViPipe, 0x3f, 0xc4);
	pr2020_write_register(ViPipe, 0x40, 0x05);
	pr2020_write_register(ViPipe, 0x41, 0x55);
	pr2020_write_register(ViPipe, 0x42, 0x01);
	pr2020_write_register(ViPipe, 0x43, 0x33);
	pr2020_write_register(ViPipe, 0x44, 0x6a);
	pr2020_write_register(ViPipe, 0x45, 0x00);
	pr2020_write_register(ViPipe, 0x46, 0x09);
	pr2020_write_register(ViPipe, 0x47, 0xe2);
	pr2020_write_register(ViPipe, 0x48, 0x01);
	pr2020_write_register(ViPipe, 0x49, 0x00);
	pr2020_write_register(ViPipe, 0x4a, 0x7b);
	pr2020_write_register(ViPipe, 0x4b, 0x60);
	pr2020_write_register(ViPipe, 0x4c, 0x00);
	pr2020_write_register(ViPipe, 0x4d, 0x4a);
	pr2020_write_register(ViPipe, 0x4e, 0x00);
	pr2020_write_register(ViPipe, 0x4f, 0x20);
	pr2020_write_register(ViPipe, 0x50, 0x01);
	pr2020_write_register(ViPipe, 0x51, 0x28);
	pr2020_write_register(ViPipe, 0x52, 0x40);
	pr2020_write_register(ViPipe, 0x53, 0x0c);
	pr2020_write_register(ViPipe, 0x54, 0x0f);
	pr2020_write_register(ViPipe, 0x55, 0x8d);
	pr2020_write_register(ViPipe, 0x70, 0x06);
	pr2020_write_register(ViPipe, 0x71, 0x08);
	pr2020_write_register(ViPipe, 0x72, 0x0a);
	pr2020_write_register(ViPipe, 0x73, 0x0c);
	pr2020_write_register(ViPipe, 0x74, 0x0e);
	pr2020_write_register(ViPipe, 0x75, 0x10);
	pr2020_write_register(ViPipe, 0x76, 0x12);
	pr2020_write_register(ViPipe, 0x77, 0x14);
	pr2020_write_register(ViPipe, 0x78, 0x06);
	pr2020_write_register(ViPipe, 0x79, 0x08);
	pr2020_write_register(ViPipe, 0x7a, 0x0a);
	pr2020_write_register(ViPipe, 0x7b, 0x0c);
	pr2020_write_register(ViPipe, 0x7c, 0x0e);
	pr2020_write_register(ViPipe, 0x7d, 0x10);
	pr2020_write_register(ViPipe, 0x7e, 0x12);
	pr2020_write_register(ViPipe, 0x7f, 0x14);
	pr2020_write_register(ViPipe, 0x80, 0x00);
	pr2020_write_register(ViPipe, 0x81, 0x09);
	pr2020_write_register(ViPipe, 0x82, 0x00);
	pr2020_write_register(ViPipe, 0x83, 0x07);
	pr2020_write_register(ViPipe, 0x84, 0x00);
	pr2020_write_register(ViPipe, 0x85, 0x17);
	pr2020_write_register(ViPipe, 0x86, 0x03);
	pr2020_write_register(ViPipe, 0x87, 0xe5);
	pr2020_write_register(ViPipe, 0x88, 0x0a);
	pr2020_write_register(ViPipe, 0x89, 0x48);
	pr2020_write_register(ViPipe, 0x8a, 0x0a);
	pr2020_write_register(ViPipe, 0x8b, 0x48);
	pr2020_write_register(ViPipe, 0x8c, 0x0b);
	pr2020_write_register(ViPipe, 0x8d, 0xe0);
	pr2020_write_register(ViPipe, 0x8e, 0x05);
	pr2020_write_register(ViPipe, 0x8f, 0x47);
	pr2020_write_register(ViPipe, 0x90, 0x05);
	pr2020_write_register(ViPipe, 0x91, 0x69);
	pr2020_write_register(ViPipe, 0x92, 0x73);
	pr2020_write_register(ViPipe, 0x93, 0xe8);
	pr2020_write_register(ViPipe, 0x94, 0x0f);
	pr2020_write_register(ViPipe, 0x95, 0x5e);
	pr2020_write_register(ViPipe, 0x96, 0x07);
	pr2020_write_register(ViPipe, 0x97, 0x90);
	pr2020_write_register(ViPipe, 0x98, 0x17);
	pr2020_write_register(ViPipe, 0x99, 0x34);
	pr2020_write_register(ViPipe, 0x9a, 0x13);
	pr2020_write_register(ViPipe, 0x9b, 0x56);
	pr2020_write_register(ViPipe, 0x9c, 0x0b);
	pr2020_write_register(ViPipe, 0x9d, 0x9a);
	pr2020_write_register(ViPipe, 0x9e, 0x09);
	pr2020_write_register(ViPipe, 0x9f, 0xab);
	pr2020_write_register(ViPipe, 0xa0, 0x01);
	pr2020_write_register(ViPipe, 0xa1, 0x74);
	pr2020_write_register(ViPipe, 0xa2, 0x01);
	pr2020_write_register(ViPipe, 0xa3, 0x6b);
	pr2020_write_register(ViPipe, 0xa4, 0x00);
	pr2020_write_register(ViPipe, 0xa5, 0xba);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0xa3);
	pr2020_write_register(ViPipe, 0xa8, 0x01);
	pr2020_write_register(ViPipe, 0xa9, 0x39);
	pr2020_write_register(ViPipe, 0xaa, 0x01);
	pr2020_write_register(ViPipe, 0xab, 0x39);
	pr2020_write_register(ViPipe, 0xac, 0x00);
	pr2020_write_register(ViPipe, 0xad, 0xc1);
	pr2020_write_register(ViPipe, 0xae, 0x00);
	pr2020_write_register(ViPipe, 0xaf, 0xc1);
	pr2020_write_register(ViPipe, 0xb0, 0x0b);
	pr2020_write_register(ViPipe, 0xb1, 0x99);
	pr2020_write_register(ViPipe, 0xb2, 0x12);
	pr2020_write_register(ViPipe, 0xb3, 0xca);
	pr2020_write_register(ViPipe, 0xb4, 0x00);
	pr2020_write_register(ViPipe, 0xb5, 0x17);
	pr2020_write_register(ViPipe, 0xb6, 0x08);
	pr2020_write_register(ViPipe, 0xb7, 0xe8);
	pr2020_write_register(ViPipe, 0xb8, 0xb0);
	pr2020_write_register(ViPipe, 0xb9, 0xce);
	pr2020_write_register(ViPipe, 0xba, 0x90);
	pr2020_write_register(ViPipe, 0xbb, 0x00);
	pr2020_write_register(ViPipe, 0xbc, 0x00);
	pr2020_write_register(ViPipe, 0xbd, 0x04);
	pr2020_write_register(ViPipe, 0xbe, 0x05);
	pr2020_write_register(ViPipe, 0xbf, 0x00);
	pr2020_write_register(ViPipe, 0xc0, 0x00);
	pr2020_write_register(ViPipe, 0xc1, 0x12);
	pr2020_write_register(ViPipe, 0xc2, 0x02);
	pr2020_write_register(ViPipe, 0xc3, 0xd0);

	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0e);
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0f);

	//add for AHD setting
	pr2020_write_register(ViPipe, 0xFF, 0x00);
	pr2020_write_register(ViPipe, 0xD0, 0x30);
	pr2020_write_register(ViPipe, 0xD1, 0x08);
	pr2020_write_register(ViPipe, 0xD2, 0x21);
	pr2020_write_register(ViPipe, 0xD3, 0x00);
	pr2020_write_register(ViPipe, 0xD8, 0x37);
	pr2020_write_register(ViPipe, 0xD9, 0x08);

	pr2020_write_register(ViPipe, 0xFF, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xE4);//no-video data, 0xe4: black, 0xe5: blue

	printf("ViPipe:%d,===PR2020 720P 25fps 8bit Init OK!===\n", ViPipe);
}

void pr2020_set_720p_30(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys
	pr2020_write_register(ViPipe, 0xff, 0x00);
	pr2020_write_register(ViPipe, 0x10, 0x92);
	pr2020_write_register(ViPipe, 0x11, 0x07);
	pr2020_write_register(ViPipe, 0x12, 0x00);
	pr2020_write_register(ViPipe, 0x13, 0x00);
	pr2020_write_register(ViPipe, 0x14, 0x21);//b[1:0); => Select Camera Input. VinP(1), VinN(3), Differ(0).
	pr2020_write_register(ViPipe, 0x15, 0x44);
	pr2020_write_register(ViPipe, 0x16, 0x0d);
	pr2020_write_register(ViPipe, 0x40, 0x00);
	pr2020_write_register(ViPipe, 0x47, 0x3a);
	pr2020_write_register(ViPipe, 0x4e, 0x3f);
	pr2020_write_register(ViPipe, 0x80, 0x56);
	pr2020_write_register(ViPipe, 0x81, 0x0e);
	pr2020_write_register(ViPipe, 0x82, 0x0d);
	pr2020_write_register(ViPipe, 0x84, 0x30);
	pr2020_write_register(ViPipe, 0x86, 0x20);
	pr2020_write_register(ViPipe, 0x87, 0x00);
	pr2020_write_register(ViPipe, 0x8a, 0x00);
	pr2020_write_register(ViPipe, 0x90, 0x00);
	pr2020_write_register(ViPipe, 0x91, 0x00);
	pr2020_write_register(ViPipe, 0x92, 0x00);
	pr2020_write_register(ViPipe, 0x94, 0xff);
	pr2020_write_register(ViPipe, 0x95, 0xff);
	pr2020_write_register(ViPipe, 0x96, 0xff);
	pr2020_write_register(ViPipe, 0xa0, 0x00);
	pr2020_write_register(ViPipe, 0xa1, 0x20);
	pr2020_write_register(ViPipe, 0xa4, 0x01);
	pr2020_write_register(ViPipe, 0xa5, 0xe3);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0x12);
	pr2020_write_register(ViPipe, 0xa8, 0x00);
	pr2020_write_register(ViPipe, 0xd0, 0x30);
	pr2020_write_register(ViPipe, 0xd1, 0x08);
	pr2020_write_register(ViPipe, 0xd2, 0x21);
	pr2020_write_register(ViPipe, 0xd3, 0x00);
	pr2020_write_register(ViPipe, 0xd8, 0x31);
	pr2020_write_register(ViPipe, 0xd9, 0x08);
	pr2020_write_register(ViPipe, 0xda, 0x21);
	pr2020_write_register(ViPipe, 0xe0, 0x39);
	pr2020_write_register(ViPipe, 0xe1, 0x90);
	pr2020_write_register(ViPipe, 0xe2, 0x38);
	pr2020_write_register(ViPipe, 0xe3, 0x19);
	pr2020_write_register(ViPipe, 0xe4, 0x19);
	pr2020_write_register(ViPipe, 0xea, 0x01);
	pr2020_write_register(ViPipe, 0xeb, 0xff);
	pr2020_write_register(ViPipe, 0xf1, 0x44);
	pr2020_write_register(ViPipe, 0xf2, 0x01);

	//Page1 vdec
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xe4);
	pr2020_write_register(ViPipe, 0x01, 0x61);
	pr2020_write_register(ViPipe, 0x02, 0x00);
	pr2020_write_register(ViPipe, 0x03, 0x57);
	pr2020_write_register(ViPipe, 0x04, 0x0c);
	pr2020_write_register(ViPipe, 0x05, 0x88);
	pr2020_write_register(ViPipe, 0x06, 0x04);
	pr2020_write_register(ViPipe, 0x07, 0xb2);
	pr2020_write_register(ViPipe, 0x08, 0x44);
	pr2020_write_register(ViPipe, 0x09, 0x34);
	pr2020_write_register(ViPipe, 0x0a, 0x02);
	pr2020_write_register(ViPipe, 0x0b, 0x14);
	pr2020_write_register(ViPipe, 0x0c, 0x04);
	pr2020_write_register(ViPipe, 0x0d, 0x08);
	pr2020_write_register(ViPipe, 0x0e, 0x5e);
	pr2020_write_register(ViPipe, 0x0f, 0x5e);
	pr2020_write_register(ViPipe, 0x10, 0x26);
	pr2020_write_register(ViPipe, 0x11, 0x00);
	pr2020_write_register(ViPipe, 0x12, 0x45);
	pr2020_write_register(ViPipe, 0x13, 0xfc);
	pr2020_write_register(ViPipe, 0x14, 0x00);
	pr2020_write_register(ViPipe, 0x15, 0x18);
	pr2020_write_register(ViPipe, 0x16, 0xd0);
	pr2020_write_register(ViPipe, 0x17, 0x00);
	pr2020_write_register(ViPipe, 0x18, 0x41);
	pr2020_write_register(ViPipe, 0x19, 0x46);
	pr2020_write_register(ViPipe, 0x1a, 0x22);
	pr2020_write_register(ViPipe, 0x1b, 0x05);
	pr2020_write_register(ViPipe, 0x1c, 0xea);
	pr2020_write_register(ViPipe, 0x1d, 0x45);
	pr2020_write_register(ViPipe, 0x1e, 0x40);
	pr2020_write_register(ViPipe, 0x1f, 0x00);
	pr2020_write_register(ViPipe, 0x20, 0x80);
	pr2020_write_register(ViPipe, 0x21, 0x80);
	pr2020_write_register(ViPipe, 0x22, 0x90);
	pr2020_write_register(ViPipe, 0x23, 0x80);
	pr2020_write_register(ViPipe, 0x24, 0x80);
	pr2020_write_register(ViPipe, 0x25, 0x80);
	pr2020_write_register(ViPipe, 0x26, 0x84);
	pr2020_write_register(ViPipe, 0x27, 0x82);
	pr2020_write_register(ViPipe, 0x28, 0x00);
	pr2020_write_register(ViPipe, 0x29, 0x7b);
	pr2020_write_register(ViPipe, 0x2a, 0xa6);
	pr2020_write_register(ViPipe, 0x2b, 0x00);
	pr2020_write_register(ViPipe, 0x2c, 0x00);
	pr2020_write_register(ViPipe, 0x2d, 0x00);
	pr2020_write_register(ViPipe, 0x2e, 0x00);
	pr2020_write_register(ViPipe, 0x2f, 0x00);
	pr2020_write_register(ViPipe, 0x30, 0x00);
	pr2020_write_register(ViPipe, 0x31, 0x00);
	pr2020_write_register(ViPipe, 0x32, 0xc0);
	pr2020_write_register(ViPipe, 0x33, 0x14);
	pr2020_write_register(ViPipe, 0x34, 0x14);
	pr2020_write_register(ViPipe, 0x35, 0x80);
	pr2020_write_register(ViPipe, 0x36, 0x80);
	pr2020_write_register(ViPipe, 0x37, 0xaa);
	pr2020_write_register(ViPipe, 0x38, 0x48);
	pr2020_write_register(ViPipe, 0x39, 0x08);
	pr2020_write_register(ViPipe, 0x3a, 0x27);
	pr2020_write_register(ViPipe, 0x3b, 0x02);
	pr2020_write_register(ViPipe, 0x3c, 0x01);
	pr2020_write_register(ViPipe, 0x3d, 0x23);
	pr2020_write_register(ViPipe, 0x3e, 0x02);
	pr2020_write_register(ViPipe, 0x3f, 0xc4);
	pr2020_write_register(ViPipe, 0x40, 0x05);
	pr2020_write_register(ViPipe, 0x41, 0x55);
	pr2020_write_register(ViPipe, 0x42, 0x01);
	pr2020_write_register(ViPipe, 0x43, 0x33);
	pr2020_write_register(ViPipe, 0x44, 0x6a);
	pr2020_write_register(ViPipe, 0x45, 0x00);
	pr2020_write_register(ViPipe, 0x46, 0x09);
	pr2020_write_register(ViPipe, 0x47, 0xdc);
	pr2020_write_register(ViPipe, 0x48, 0xa0);
	pr2020_write_register(ViPipe, 0x49, 0x00);
	pr2020_write_register(ViPipe, 0x4a, 0x7b);
	pr2020_write_register(ViPipe, 0x4b, 0x60);
	pr2020_write_register(ViPipe, 0x4c, 0x00);
	pr2020_write_register(ViPipe, 0x4d, 0x4a);
	pr2020_write_register(ViPipe, 0x4e, 0x00);
	pr2020_write_register(ViPipe, 0x4f, 0x20);
	pr2020_write_register(ViPipe, 0x50, 0x01);
	pr2020_write_register(ViPipe, 0x51, 0x28);
	pr2020_write_register(ViPipe, 0x52, 0x40);
	pr2020_write_register(ViPipe, 0x53, 0x0c);
	pr2020_write_register(ViPipe, 0x54, 0x0f);
	pr2020_write_register(ViPipe, 0x55, 0x8d);
	pr2020_write_register(ViPipe, 0x70, 0x06);
	pr2020_write_register(ViPipe, 0x71, 0x08);
	pr2020_write_register(ViPipe, 0x72, 0x0a);
	pr2020_write_register(ViPipe, 0x73, 0x0c);
	pr2020_write_register(ViPipe, 0x74, 0x0e);
	pr2020_write_register(ViPipe, 0x75, 0x10);
	pr2020_write_register(ViPipe, 0x76, 0x12);
	pr2020_write_register(ViPipe, 0x77, 0x14);
	pr2020_write_register(ViPipe, 0x78, 0x06);
	pr2020_write_register(ViPipe, 0x79, 0x08);
	pr2020_write_register(ViPipe, 0x7a, 0x0a);
	pr2020_write_register(ViPipe, 0x7b, 0x0c);
	pr2020_write_register(ViPipe, 0x7c, 0x0e);
	pr2020_write_register(ViPipe, 0x7d, 0x10);
	pr2020_write_register(ViPipe, 0x7e, 0x12);
	pr2020_write_register(ViPipe, 0x7f, 0x14);
	pr2020_write_register(ViPipe, 0x80, 0x00);
	pr2020_write_register(ViPipe, 0x81, 0x09);
	pr2020_write_register(ViPipe, 0x82, 0x00);
	pr2020_write_register(ViPipe, 0x83, 0x07);
	pr2020_write_register(ViPipe, 0x84, 0x00);
	pr2020_write_register(ViPipe, 0x85, 0x17);
	pr2020_write_register(ViPipe, 0x86, 0x03);
	pr2020_write_register(ViPipe, 0x87, 0xe5);
	pr2020_write_register(ViPipe, 0x88, 0x08);
	pr2020_write_register(ViPipe, 0x89, 0x91);
	pr2020_write_register(ViPipe, 0x8a, 0x08);
	pr2020_write_register(ViPipe, 0x8b, 0x91);
	pr2020_write_register(ViPipe, 0x8c, 0x0b);
	pr2020_write_register(ViPipe, 0x8d, 0xe0);
	pr2020_write_register(ViPipe, 0x8e, 0x05);
	pr2020_write_register(ViPipe, 0x8f, 0x47);
	pr2020_write_register(ViPipe, 0x90, 0x05);
	pr2020_write_register(ViPipe, 0x91, 0xa0);
	pr2020_write_register(ViPipe, 0x92, 0x73);
	pr2020_write_register(ViPipe, 0x93, 0xe8);
	pr2020_write_register(ViPipe, 0x94, 0x0f);
	pr2020_write_register(ViPipe, 0x95, 0x5e);
	pr2020_write_register(ViPipe, 0x96, 0x07);
	pr2020_write_register(ViPipe, 0x97, 0x90);
	pr2020_write_register(ViPipe, 0x98, 0x17);
	pr2020_write_register(ViPipe, 0x99, 0x34);
	pr2020_write_register(ViPipe, 0x9a, 0x13);
	pr2020_write_register(ViPipe, 0x9b, 0x56);
	pr2020_write_register(ViPipe, 0x9c, 0x0b);
	pr2020_write_register(ViPipe, 0x9d, 0x9a);
	pr2020_write_register(ViPipe, 0x9e, 0x09);
	pr2020_write_register(ViPipe, 0x9f, 0xab);
	pr2020_write_register(ViPipe, 0xa0, 0x01);
	pr2020_write_register(ViPipe, 0xa1, 0x74);
	pr2020_write_register(ViPipe, 0xa2, 0x01);
	pr2020_write_register(ViPipe, 0xa3, 0x6b);
	pr2020_write_register(ViPipe, 0xa4, 0x00);
	pr2020_write_register(ViPipe, 0xa5, 0xba);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0xa3);
	pr2020_write_register(ViPipe, 0xa8, 0x01);
	pr2020_write_register(ViPipe, 0xa9, 0x39);
	pr2020_write_register(ViPipe, 0xaa, 0x01);
	pr2020_write_register(ViPipe, 0xab, 0x39);
	pr2020_write_register(ViPipe, 0xac, 0x00);
	pr2020_write_register(ViPipe, 0xad, 0xc1);
	pr2020_write_register(ViPipe, 0xae, 0x00);
	pr2020_write_register(ViPipe, 0xaf, 0xc1);
	pr2020_write_register(ViPipe, 0xb0, 0x09);
	pr2020_write_register(ViPipe, 0xb1, 0xaa);
	pr2020_write_register(ViPipe, 0xb2, 0x0f);
	pr2020_write_register(ViPipe, 0xb3, 0xae);
	pr2020_write_register(ViPipe, 0xb4, 0x00);
	pr2020_write_register(ViPipe, 0xb5, 0x17);
	pr2020_write_register(ViPipe, 0xb6, 0x08);
	pr2020_write_register(ViPipe, 0xb7, 0xe8);
	pr2020_write_register(ViPipe, 0xb8, 0xb0);
	pr2020_write_register(ViPipe, 0xb9, 0xce);
	pr2020_write_register(ViPipe, 0xba, 0x90);
	pr2020_write_register(ViPipe, 0xbb, 0x00);
	pr2020_write_register(ViPipe, 0xbc, 0x00);
	pr2020_write_register(ViPipe, 0xbd, 0x04);
	pr2020_write_register(ViPipe, 0xbe, 0x05);
	pr2020_write_register(ViPipe, 0xbf, 0x00);
	pr2020_write_register(ViPipe, 0xc0, 0x00);
	pr2020_write_register(ViPipe, 0xc1, 0x18);
	pr2020_write_register(ViPipe, 0xc2, 0x02);
	pr2020_write_register(ViPipe, 0xc3, 0xd0);

	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0e);
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0f);

	//add for AHD setting
	pr2020_write_register(ViPipe, 0xFF, 0x00);
	pr2020_write_register(ViPipe, 0xD0, 0x30);
	pr2020_write_register(ViPipe, 0xD1, 0x08);
	pr2020_write_register(ViPipe, 0xD2, 0x21);
	pr2020_write_register(ViPipe, 0xD3, 0x00);
	pr2020_write_register(ViPipe, 0xD8, 0x37);
	pr2020_write_register(ViPipe, 0xD9, 0x08);

	pr2020_write_register(ViPipe, 0xFF, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xE4);//no-video data, 0xe4: black, 0xe5: blue

	printf("ViPipe:%d,===PR2020 720P 30fps 8bit Init OK!===\n", ViPipe);
}

void pr2020_set_1080p_25(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys
	pr2020_write_register(ViPipe, 0xff, 0x00);
	pr2020_write_register(ViPipe, 0x10, 0x83);
	pr2020_write_register(ViPipe, 0x11, 0x07);
	pr2020_write_register(ViPipe, 0x12, 0x00);
	pr2020_write_register(ViPipe, 0x13, 0x00);
	pr2020_write_register(ViPipe, 0x14, 0x21);//b[1:0} => Select Camera Input. VinP(1), VinN(3), Differ(0).
	pr2020_write_register(ViPipe, 0x15, 0x44);
	pr2020_write_register(ViPipe, 0x16, 0x0d);
	pr2020_write_register(ViPipe, 0x40, 0x00);
	pr2020_write_register(ViPipe, 0x47, 0x3a);
	pr2020_write_register(ViPipe, 0x4e, 0x3f);
	pr2020_write_register(ViPipe, 0x80, 0x56);
	pr2020_write_register(ViPipe, 0x81, 0x0e);
	pr2020_write_register(ViPipe, 0x82, 0x0d);
	pr2020_write_register(ViPipe, 0x84, 0x30);
	pr2020_write_register(ViPipe, 0x86, 0x20);
	pr2020_write_register(ViPipe, 0x87, 0x00);
	pr2020_write_register(ViPipe, 0x8a, 0x00);
	pr2020_write_register(ViPipe, 0x90, 0x00);
	pr2020_write_register(ViPipe, 0x91, 0x00);
	pr2020_write_register(ViPipe, 0x92, 0x00);
	pr2020_write_register(ViPipe, 0x94, 0xff);
	pr2020_write_register(ViPipe, 0x95, 0xff);
	pr2020_write_register(ViPipe, 0x96, 0xff);
	pr2020_write_register(ViPipe, 0xa0, 0x00);
	pr2020_write_register(ViPipe, 0xa1, 0x20);
	pr2020_write_register(ViPipe, 0xa4, 0x01);
	pr2020_write_register(ViPipe, 0xa5, 0xe3);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0x12);
	pr2020_write_register(ViPipe, 0xa8, 0x00);
	pr2020_write_register(ViPipe, 0xd0, 0x30);
	pr2020_write_register(ViPipe, 0xd1, 0x08);
	pr2020_write_register(ViPipe, 0xd2, 0x21);
	pr2020_write_register(ViPipe, 0xd3, 0x00);
	pr2020_write_register(ViPipe, 0xd8, 0x30);
	pr2020_write_register(ViPipe, 0xd9, 0x08);
	pr2020_write_register(ViPipe, 0xda, 0x21);
	pr2020_write_register(ViPipe, 0xe0, 0x35);
	pr2020_write_register(ViPipe, 0xe1, 0x80);//[6] 0:cb-y-cr-y 1:y-cb-y-cr
	pr2020_write_register(ViPipe, 0xe2, 0x18);
	pr2020_write_register(ViPipe, 0xe3, 0x00);
	pr2020_write_register(ViPipe, 0xe4, 0x00);
	pr2020_write_register(ViPipe, 0xea, 0x01);
	pr2020_write_register(ViPipe, 0xeb, 0xff);
	pr2020_write_register(ViPipe, 0xf1, 0x44);
	pr2020_write_register(ViPipe, 0xf2, 0x01);

	//Page1 vdec
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xe4);
	pr2020_write_register(ViPipe, 0x01, 0x61);
	pr2020_write_register(ViPipe, 0x02, 0x00);
	pr2020_write_register(ViPipe, 0x03, 0x57);
	pr2020_write_register(ViPipe, 0x04, 0x0c);
	pr2020_write_register(ViPipe, 0x05, 0x88);
	pr2020_write_register(ViPipe, 0x06, 0x04);
	pr2020_write_register(ViPipe, 0x07, 0xb2);
	pr2020_write_register(ViPipe, 0x08, 0x44);
	pr2020_write_register(ViPipe, 0x09, 0x34);
	pr2020_write_register(ViPipe, 0x0a, 0x02);
	pr2020_write_register(ViPipe, 0x0b, 0x14);
	pr2020_write_register(ViPipe, 0x0c, 0x04);
	pr2020_write_register(ViPipe, 0x0d, 0x08);
	pr2020_write_register(ViPipe, 0x0e, 0x5e);
	pr2020_write_register(ViPipe, 0x0f, 0x5e);
	pr2020_write_register(ViPipe, 0x10, 0x26);
	pr2020_write_register(ViPipe, 0x11, 0x00);
	pr2020_write_register(ViPipe, 0x12, 0x87);
	pr2020_write_register(ViPipe, 0x13, 0x24);
	pr2020_write_register(ViPipe, 0x14, 0x80);
	pr2020_write_register(ViPipe, 0x15, 0x2a);
	pr2020_write_register(ViPipe, 0x16, 0x38);
	pr2020_write_register(ViPipe, 0x17, 0x00);
	pr2020_write_register(ViPipe, 0x18, 0x80);
	pr2020_write_register(ViPipe, 0x19, 0x48);
	pr2020_write_register(ViPipe, 0x1a, 0x6c);
	pr2020_write_register(ViPipe, 0x1b, 0x05);
	pr2020_write_register(ViPipe, 0x1c, 0x61);
	pr2020_write_register(ViPipe, 0x1d, 0x07);
	pr2020_write_register(ViPipe, 0x1e, 0x7e);
	pr2020_write_register(ViPipe, 0x1f, 0x80);
	pr2020_write_register(ViPipe, 0x20, 0x80);
	pr2020_write_register(ViPipe, 0x21, 0x80);
	pr2020_write_register(ViPipe, 0x22, 0x90);
	pr2020_write_register(ViPipe, 0x23, 0x80);
	pr2020_write_register(ViPipe, 0x24, 0x80);
	pr2020_write_register(ViPipe, 0x25, 0x80);
	pr2020_write_register(ViPipe, 0x26, 0x84);
	pr2020_write_register(ViPipe, 0x27, 0x82);
	pr2020_write_register(ViPipe, 0x28, 0x00);
	pr2020_write_register(ViPipe, 0x29, 0xff);
	pr2020_write_register(ViPipe, 0x2a, 0xff);
	pr2020_write_register(ViPipe, 0x2b, 0x00);
	pr2020_write_register(ViPipe, 0x2c, 0x00);
	pr2020_write_register(ViPipe, 0x2d, 0x00);
	pr2020_write_register(ViPipe, 0x2e, 0x00);
	pr2020_write_register(ViPipe, 0x2f, 0x00);
	pr2020_write_register(ViPipe, 0x30, 0x00);
	pr2020_write_register(ViPipe, 0x31, 0x00);
	pr2020_write_register(ViPipe, 0x32, 0xc0);
	pr2020_write_register(ViPipe, 0x33, 0x14);
	pr2020_write_register(ViPipe, 0x34, 0x14);
	pr2020_write_register(ViPipe, 0x35, 0x80);
	pr2020_write_register(ViPipe, 0x36, 0x80);
	pr2020_write_register(ViPipe, 0x37, 0xad);
	pr2020_write_register(ViPipe, 0x38, 0x4b);
	pr2020_write_register(ViPipe, 0x39, 0x08);
	pr2020_write_register(ViPipe, 0x3a, 0x21);
	pr2020_write_register(ViPipe, 0x3b, 0x02);
	pr2020_write_register(ViPipe, 0x3c, 0x01);
	pr2020_write_register(ViPipe, 0x3d, 0x23);
	pr2020_write_register(ViPipe, 0x3e, 0x05);
	pr2020_write_register(ViPipe, 0x3f, 0xc8);
	pr2020_write_register(ViPipe, 0x40, 0x05);
	pr2020_write_register(ViPipe, 0x41, 0x55);
	pr2020_write_register(ViPipe, 0x42, 0x01);
	pr2020_write_register(ViPipe, 0x43, 0x38);
	pr2020_write_register(ViPipe, 0x44, 0x6a);
	pr2020_write_register(ViPipe, 0x45, 0x00);
	pr2020_write_register(ViPipe, 0x46, 0x14);
	pr2020_write_register(ViPipe, 0x47, 0xb0);
	pr2020_write_register(ViPipe, 0x48, 0xdf);
	pr2020_write_register(ViPipe, 0x49, 0x00);
	pr2020_write_register(ViPipe, 0x4a, 0x7b);
	pr2020_write_register(ViPipe, 0x4b, 0x60);
	pr2020_write_register(ViPipe, 0x4c, 0x00);
	pr2020_write_register(ViPipe, 0x4d, 0x26);
	pr2020_write_register(ViPipe, 0x4e, 0x00);
	pr2020_write_register(ViPipe, 0x4f, 0x20);//RK:0x24
	pr2020_write_register(ViPipe, 0x50, 0x01);
	pr2020_write_register(ViPipe, 0x51, 0x28);
	pr2020_write_register(ViPipe, 0x52, 0x40);
	pr2020_write_register(ViPipe, 0x53, 0x0c);
	pr2020_write_register(ViPipe, 0x54, 0x0f);
	pr2020_write_register(ViPipe, 0x55, 0x8d);
	pr2020_write_register(ViPipe, 0x70, 0x06);
	pr2020_write_register(ViPipe, 0x71, 0x08);
	pr2020_write_register(ViPipe, 0x72, 0x0a);
	pr2020_write_register(ViPipe, 0x73, 0x0c);
	pr2020_write_register(ViPipe, 0x74, 0x0e);
	pr2020_write_register(ViPipe, 0x75, 0x10);
	pr2020_write_register(ViPipe, 0x76, 0x12);
	pr2020_write_register(ViPipe, 0x77, 0x14);
	pr2020_write_register(ViPipe, 0x78, 0x06);
	pr2020_write_register(ViPipe, 0x79, 0x08);
	pr2020_write_register(ViPipe, 0x7a, 0x0a);
	pr2020_write_register(ViPipe, 0x7b, 0x0c);
	pr2020_write_register(ViPipe, 0x7c, 0x0e);
	pr2020_write_register(ViPipe, 0x7d, 0x10);
	pr2020_write_register(ViPipe, 0x7e, 0x12);
	pr2020_write_register(ViPipe, 0x7f, 0x14);
	pr2020_write_register(ViPipe, 0x80, 0x00);
	pr2020_write_register(ViPipe, 0x81, 0x09);
	pr2020_write_register(ViPipe, 0x82, 0x00);
	pr2020_write_register(ViPipe, 0x83, 0x07);
	pr2020_write_register(ViPipe, 0x84, 0x00);
	pr2020_write_register(ViPipe, 0x85, 0x17);
	pr2020_write_register(ViPipe, 0x86, 0x03);
	pr2020_write_register(ViPipe, 0x87, 0xe5);
	pr2020_write_register(ViPipe, 0x88, 0x05);
	pr2020_write_register(ViPipe, 0x89, 0x24);
	pr2020_write_register(ViPipe, 0x8a, 0x05);
	pr2020_write_register(ViPipe, 0x8b, 0x24);
	pr2020_write_register(ViPipe, 0x8c, 0x08);
	pr2020_write_register(ViPipe, 0x8d, 0xe8);
	pr2020_write_register(ViPipe, 0x8e, 0x05);
	pr2020_write_register(ViPipe, 0x8f, 0x47);
	pr2020_write_register(ViPipe, 0x90, 0x02);
	pr2020_write_register(ViPipe, 0x91, 0xb4);
	pr2020_write_register(ViPipe, 0x92, 0x73);
	pr2020_write_register(ViPipe, 0x93, 0xe8);
	pr2020_write_register(ViPipe, 0x94, 0x0f);
	pr2020_write_register(ViPipe, 0x95, 0x5e);
	pr2020_write_register(ViPipe, 0x96, 0x03);
	pr2020_write_register(ViPipe, 0x97, 0xd0);
	pr2020_write_register(ViPipe, 0x98, 0x17);
	pr2020_write_register(ViPipe, 0x99, 0x34);
	pr2020_write_register(ViPipe, 0x9a, 0x13);
	pr2020_write_register(ViPipe, 0x9b, 0x56);
	pr2020_write_register(ViPipe, 0x9c, 0x0b);
	pr2020_write_register(ViPipe, 0x9d, 0x9a);
	pr2020_write_register(ViPipe, 0x9e, 0x09);
	pr2020_write_register(ViPipe, 0x9f, 0xab);
	pr2020_write_register(ViPipe, 0xa0, 0x01);
	pr2020_write_register(ViPipe, 0xa1, 0x74);
	pr2020_write_register(ViPipe, 0xa2, 0x01);
	pr2020_write_register(ViPipe, 0xa3, 0x6b);
	pr2020_write_register(ViPipe, 0xa4, 0x00);
	pr2020_write_register(ViPipe, 0xa5, 0xba);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0xa3);
	pr2020_write_register(ViPipe, 0xa8, 0x01);
	pr2020_write_register(ViPipe, 0xa9, 0x39);
	pr2020_write_register(ViPipe, 0xaa, 0x01);
	pr2020_write_register(ViPipe, 0xab, 0x39);
	pr2020_write_register(ViPipe, 0xac, 0x00);
	pr2020_write_register(ViPipe, 0xad, 0xc1);
	pr2020_write_register(ViPipe, 0xae, 0x00);
	pr2020_write_register(ViPipe, 0xaf, 0xc1);
	pr2020_write_register(ViPipe, 0xb0, 0x05);
	pr2020_write_register(ViPipe, 0xb1, 0xcc);
	pr2020_write_register(ViPipe, 0xb2, 0x09);
	pr2020_write_register(ViPipe, 0xb3, 0x6d);
	pr2020_write_register(ViPipe, 0xb4, 0x00);
	pr2020_write_register(ViPipe, 0xb5, 0x17);
	pr2020_write_register(ViPipe, 0xb6, 0x08);
	pr2020_write_register(ViPipe, 0xb7, 0xe8);
	pr2020_write_register(ViPipe, 0xb8, 0xb0);
	pr2020_write_register(ViPipe, 0xb9, 0xce);
	pr2020_write_register(ViPipe, 0xba, 0x90);
	pr2020_write_register(ViPipe, 0xbb, 0x00);
	pr2020_write_register(ViPipe, 0xbc, 0x00);
	pr2020_write_register(ViPipe, 0xbd, 0x04);
	pr2020_write_register(ViPipe, 0xbe, 0x07);
	pr2020_write_register(ViPipe, 0xbf, 0x80);
	pr2020_write_register(ViPipe, 0xc0, 0x00);
	pr2020_write_register(ViPipe, 0xc1, 0x20);
	pr2020_write_register(ViPipe, 0xc2, 0x04);
	pr2020_write_register(ViPipe, 0xc3, 0x38);

	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0e);
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0f);

	//add for AHD setting
	pr2020_write_register(ViPipe, 0xFF, 0x00);
	pr2020_write_register(ViPipe, 0xD0, 0x30);
	pr2020_write_register(ViPipe, 0xD1, 0x08);
	pr2020_write_register(ViPipe, 0xD2, 0x21);
	pr2020_write_register(ViPipe, 0xD3, 0x00);
	pr2020_write_register(ViPipe, 0xD8, 0x37);
	pr2020_write_register(ViPipe, 0xD9, 0x08);

	pr2020_write_register(ViPipe, 0xFF, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xE4);//no-video data, 0xe4: black, 0xe5: blue

	printf("ViPipe:%d,===PR2020 1080P 25fps 8bit Init OK!===\n", ViPipe);
}

void pr2020_set_1080p_30(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys
	pr2020_write_register(ViPipe, 0xff, 0x00);
	pr2020_write_register(ViPipe, 0x10, 0x93);
	pr2020_write_register(ViPipe, 0x11, 0x07);
	pr2020_write_register(ViPipe, 0x12, 0x00);
	pr2020_write_register(ViPipe, 0x13, 0x00);
	pr2020_write_register(ViPipe, 0x14, 0x21);//b[1:0} => Select Camera Input. VinP(1), VinN(3), Differ(0).
	pr2020_write_register(ViPipe, 0x15, 0x44);
	pr2020_write_register(ViPipe, 0x16, 0x0d);
	pr2020_write_register(ViPipe, 0x40, 0x00);
	pr2020_write_register(ViPipe, 0x47, 0x3a);
	pr2020_write_register(ViPipe, 0x4e, 0x3f);
	pr2020_write_register(ViPipe, 0x80, 0x56);
	pr2020_write_register(ViPipe, 0x81, 0x0e);
	pr2020_write_register(ViPipe, 0x82, 0x0d);
	pr2020_write_register(ViPipe, 0x84, 0x30);
	pr2020_write_register(ViPipe, 0x86, 0x20);
	pr2020_write_register(ViPipe, 0x87, 0x00);
	pr2020_write_register(ViPipe, 0x8a, 0x00);
	pr2020_write_register(ViPipe, 0x90, 0x00);
	pr2020_write_register(ViPipe, 0x91, 0x00);
	pr2020_write_register(ViPipe, 0x92, 0x00);
	pr2020_write_register(ViPipe, 0x94, 0xff);
	pr2020_write_register(ViPipe, 0x95, 0xff);
	pr2020_write_register(ViPipe, 0x96, 0xff);
	pr2020_write_register(ViPipe, 0xa0, 0x00);
	pr2020_write_register(ViPipe, 0xa1, 0x20);
	pr2020_write_register(ViPipe, 0xa4, 0x01);
	pr2020_write_register(ViPipe, 0xa5, 0xe3);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0x12);
	pr2020_write_register(ViPipe, 0xa8, 0x00);
	pr2020_write_register(ViPipe, 0xd0, 0x30);
	pr2020_write_register(ViPipe, 0xd1, 0x08);
	pr2020_write_register(ViPipe, 0xd2, 0x21);
	pr2020_write_register(ViPipe, 0xd3, 0x00);
	pr2020_write_register(ViPipe, 0xd8, 0x30);
	pr2020_write_register(ViPipe, 0xd9, 0x08);
	pr2020_write_register(ViPipe, 0xda, 0x21);
	pr2020_write_register(ViPipe, 0xe0, 0x35);
	pr2020_write_register(ViPipe, 0xe1, 0x80);
	pr2020_write_register(ViPipe, 0xe2, 0x18);
	pr2020_write_register(ViPipe, 0xe3, 0x00);
	pr2020_write_register(ViPipe, 0xe4, 0x00);
	pr2020_write_register(ViPipe, 0xea, 0x01);
	pr2020_write_register(ViPipe, 0xeb, 0xff);
	pr2020_write_register(ViPipe, 0xf1, 0x44);
	pr2020_write_register(ViPipe, 0xf2, 0x01);

	//Page1 vdec
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xe4);
	pr2020_write_register(ViPipe, 0x01, 0x61);
	pr2020_write_register(ViPipe, 0x02, 0x00);
	pr2020_write_register(ViPipe, 0x03, 0x57);
	pr2020_write_register(ViPipe, 0x04, 0x0c);
	pr2020_write_register(ViPipe, 0x05, 0x88);
	pr2020_write_register(ViPipe, 0x06, 0x04);
	pr2020_write_register(ViPipe, 0x07, 0xb2);
	pr2020_write_register(ViPipe, 0x08, 0x44);
	pr2020_write_register(ViPipe, 0x09, 0x34);
	pr2020_write_register(ViPipe, 0x0a, 0x02);
	pr2020_write_register(ViPipe, 0x0b, 0x14);
	pr2020_write_register(ViPipe, 0x0c, 0x04);
	pr2020_write_register(ViPipe, 0x0d, 0x08);
	pr2020_write_register(ViPipe, 0x0e, 0x5e);
	pr2020_write_register(ViPipe, 0x0f, 0x5e);
	pr2020_write_register(ViPipe, 0x10, 0x26);
	pr2020_write_register(ViPipe, 0x11, 0x00);
	pr2020_write_register(ViPipe, 0x12, 0x87);
	pr2020_write_register(ViPipe, 0x13, 0x2c);
	pr2020_write_register(ViPipe, 0x14, 0x80);
	pr2020_write_register(ViPipe, 0x15, 0x28);
	pr2020_write_register(ViPipe, 0x16, 0x38);
	pr2020_write_register(ViPipe, 0x17, 0x00);
	pr2020_write_register(ViPipe, 0x18, 0x80);
	pr2020_write_register(ViPipe, 0x19, 0x48);
	pr2020_write_register(ViPipe, 0x1a, 0x6c);
	pr2020_write_register(ViPipe, 0x1b, 0x05);
	pr2020_write_register(ViPipe, 0x1c, 0x61);
	pr2020_write_register(ViPipe, 0x1d, 0x07);
	pr2020_write_register(ViPipe, 0x1e, 0x7e);
	pr2020_write_register(ViPipe, 0x1f, 0x80);
	pr2020_write_register(ViPipe, 0x20, 0x80);
	pr2020_write_register(ViPipe, 0x21, 0x80);
	pr2020_write_register(ViPipe, 0x22, 0x90);
	pr2020_write_register(ViPipe, 0x23, 0x80);
	pr2020_write_register(ViPipe, 0x24, 0x80);
	pr2020_write_register(ViPipe, 0x25, 0x80);
	pr2020_write_register(ViPipe, 0x26, 0x84);
	pr2020_write_register(ViPipe, 0x27, 0x82);
	pr2020_write_register(ViPipe, 0x28, 0x00);
	pr2020_write_register(ViPipe, 0x29, 0xff);
	pr2020_write_register(ViPipe, 0x2a, 0xff);
	pr2020_write_register(ViPipe, 0x2b, 0x00);
	pr2020_write_register(ViPipe, 0x2c, 0x00);
	pr2020_write_register(ViPipe, 0x2d, 0x00);
	pr2020_write_register(ViPipe, 0x2e, 0x00);
	pr2020_write_register(ViPipe, 0x2f, 0x00);
	pr2020_write_register(ViPipe, 0x30, 0x00);
	pr2020_write_register(ViPipe, 0x31, 0x00);
	pr2020_write_register(ViPipe, 0x32, 0xc0);
	pr2020_write_register(ViPipe, 0x33, 0x14);
	pr2020_write_register(ViPipe, 0x34, 0x14);
	pr2020_write_register(ViPipe, 0x35, 0x80);
	pr2020_write_register(ViPipe, 0x36, 0x80);
	pr2020_write_register(ViPipe, 0x37, 0xad);
	pr2020_write_register(ViPipe, 0x38, 0x4b);
	pr2020_write_register(ViPipe, 0x39, 0x08);
	pr2020_write_register(ViPipe, 0x3a, 0x21);
	pr2020_write_register(ViPipe, 0x3b, 0x02);
	pr2020_write_register(ViPipe, 0x3c, 0x01);
	pr2020_write_register(ViPipe, 0x3d, 0x23);
	pr2020_write_register(ViPipe, 0x3e, 0x05);
	pr2020_write_register(ViPipe, 0x3f, 0xc8);
	pr2020_write_register(ViPipe, 0x40, 0x05);
	pr2020_write_register(ViPipe, 0x41, 0x55);
	pr2020_write_register(ViPipe, 0x42, 0x01);
	pr2020_write_register(ViPipe, 0x43, 0x38);
	pr2020_write_register(ViPipe, 0x44, 0x6a);
	pr2020_write_register(ViPipe, 0x45, 0x00);
	pr2020_write_register(ViPipe, 0x46, 0x14);
	pr2020_write_register(ViPipe, 0x47, 0xb2);
	pr2020_write_register(ViPipe, 0x48, 0xbc);
	pr2020_write_register(ViPipe, 0x49, 0x00);
	pr2020_write_register(ViPipe, 0x4a, 0x7b);
	pr2020_write_register(ViPipe, 0x4b, 0x60);
	pr2020_write_register(ViPipe, 0x4c, 0x00);
	pr2020_write_register(ViPipe, 0x4d, 0x26);
	pr2020_write_register(ViPipe, 0x4e, 0x00);
	pr2020_write_register(ViPipe, 0x4f, 0x20);//RK:0x24
	pr2020_write_register(ViPipe, 0x50, 0x01);
	pr2020_write_register(ViPipe, 0x51, 0x28);
	pr2020_write_register(ViPipe, 0x52, 0x40);
	pr2020_write_register(ViPipe, 0x53, 0x0c);
	pr2020_write_register(ViPipe, 0x54, 0x0f);
	pr2020_write_register(ViPipe, 0x55, 0x8d);
	pr2020_write_register(ViPipe, 0x70, 0x06);
	pr2020_write_register(ViPipe, 0x71, 0x08);
	pr2020_write_register(ViPipe, 0x72, 0x0a);
	pr2020_write_register(ViPipe, 0x73, 0x0c);
	pr2020_write_register(ViPipe, 0x74, 0x0e);
	pr2020_write_register(ViPipe, 0x75, 0x10);
	pr2020_write_register(ViPipe, 0x76, 0x12);
	pr2020_write_register(ViPipe, 0x77, 0x14);
	pr2020_write_register(ViPipe, 0x78, 0x06);
	pr2020_write_register(ViPipe, 0x79, 0x08);
	pr2020_write_register(ViPipe, 0x7a, 0x0a);
	pr2020_write_register(ViPipe, 0x7b, 0x0c);
	pr2020_write_register(ViPipe, 0x7c, 0x0e);
	pr2020_write_register(ViPipe, 0x7d, 0x10);
	pr2020_write_register(ViPipe, 0x7e, 0x12);
	pr2020_write_register(ViPipe, 0x7f, 0x14);
	pr2020_write_register(ViPipe, 0x80, 0x00);
	pr2020_write_register(ViPipe, 0x81, 0x09);
	pr2020_write_register(ViPipe, 0x82, 0x00);
	pr2020_write_register(ViPipe, 0x83, 0x07);
	pr2020_write_register(ViPipe, 0x84, 0x00);
	pr2020_write_register(ViPipe, 0x85, 0x17);
	pr2020_write_register(ViPipe, 0x86, 0x03);
	pr2020_write_register(ViPipe, 0x87, 0xe5);
	pr2020_write_register(ViPipe, 0x88, 0x04);
	pr2020_write_register(ViPipe, 0x89, 0x48);
	pr2020_write_register(ViPipe, 0x8a, 0x04);
	pr2020_write_register(ViPipe, 0x8b, 0x48);
	pr2020_write_register(ViPipe, 0x8c, 0x08);
	pr2020_write_register(ViPipe, 0x8d, 0xe8);
	pr2020_write_register(ViPipe, 0x8e, 0x05);
	pr2020_write_register(ViPipe, 0x8f, 0x47);
	pr2020_write_register(ViPipe, 0x90, 0x03);
	pr2020_write_register(ViPipe, 0x91, 0x13);
	pr2020_write_register(ViPipe, 0x92, 0x73);
	pr2020_write_register(ViPipe, 0x93, 0xe8);
	pr2020_write_register(ViPipe, 0x94, 0x0f);
	pr2020_write_register(ViPipe, 0x95, 0x5e);
	pr2020_write_register(ViPipe, 0x96, 0x03);
	pr2020_write_register(ViPipe, 0x97, 0xd0);
	pr2020_write_register(ViPipe, 0x98, 0x17);
	pr2020_write_register(ViPipe, 0x99, 0x34);
	pr2020_write_register(ViPipe, 0x9a, 0x13);
	pr2020_write_register(ViPipe, 0x9b, 0x56);
	pr2020_write_register(ViPipe, 0x9c, 0x0b);
	pr2020_write_register(ViPipe, 0x9d, 0x9a);
	pr2020_write_register(ViPipe, 0x9e, 0x09);
	pr2020_write_register(ViPipe, 0x9f, 0xab);
	pr2020_write_register(ViPipe, 0xa0, 0x01);
	pr2020_write_register(ViPipe, 0xa1, 0x74);
	pr2020_write_register(ViPipe, 0xa2, 0x01);
	pr2020_write_register(ViPipe, 0xa3, 0x6b);
	pr2020_write_register(ViPipe, 0xa4, 0x00);
	pr2020_write_register(ViPipe, 0xa5, 0xba);
	pr2020_write_register(ViPipe, 0xa6, 0x00);
	pr2020_write_register(ViPipe, 0xa7, 0xa3);
	pr2020_write_register(ViPipe, 0xa8, 0x01);
	pr2020_write_register(ViPipe, 0xa9, 0x39);
	pr2020_write_register(ViPipe, 0xaa, 0x01);
	pr2020_write_register(ViPipe, 0xab, 0x39);
	pr2020_write_register(ViPipe, 0xac, 0x00);
	pr2020_write_register(ViPipe, 0xad, 0xc1);
	pr2020_write_register(ViPipe, 0xae, 0x00);
	pr2020_write_register(ViPipe, 0xaf, 0xc1);
	pr2020_write_register(ViPipe, 0xb0, 0x04);
	pr2020_write_register(ViPipe, 0xb1, 0xd4);
	pr2020_write_register(ViPipe, 0xb2, 0x07);
	pr2020_write_register(ViPipe, 0xb3, 0xda);
	pr2020_write_register(ViPipe, 0xb4, 0x00);
	pr2020_write_register(ViPipe, 0xb5, 0x17);
	pr2020_write_register(ViPipe, 0xb6, 0x08);
	pr2020_write_register(ViPipe, 0xb7, 0xe8);
	pr2020_write_register(ViPipe, 0xb8, 0xb0);
	pr2020_write_register(ViPipe, 0xb9, 0xce);
	pr2020_write_register(ViPipe, 0xba, 0x90);
	pr2020_write_register(ViPipe, 0xbb, 0x00);
	pr2020_write_register(ViPipe, 0xbc, 0x00);
	pr2020_write_register(ViPipe, 0xbd, 0x04);
	pr2020_write_register(ViPipe, 0xbe, 0x07);
	pr2020_write_register(ViPipe, 0xbf, 0x80);
	pr2020_write_register(ViPipe, 0xc0, 0x00);
	pr2020_write_register(ViPipe, 0xc1, 0x20);
	pr2020_write_register(ViPipe, 0xc2, 0x04);
	pr2020_write_register(ViPipe, 0xc3, 0x38);

	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0e);
	pr2020_write_register(ViPipe, 0xff, 0x01);
	pr2020_write_register(ViPipe, 0x54, 0x0f);

	//add for AHD setting
	pr2020_write_register(ViPipe, 0xFF, 0x00);
	pr2020_write_register(ViPipe, 0xD0, 0x30);
	pr2020_write_register(ViPipe, 0xD1, 0x08);
	pr2020_write_register(ViPipe, 0xD2, 0x21);
	pr2020_write_register(ViPipe, 0xD3, 0x00);
	pr2020_write_register(ViPipe, 0xD8, 0x37);
	pr2020_write_register(ViPipe, 0xD9, 0x08);

	pr2020_write_register(ViPipe, 0xFF, 0x01);
	pr2020_write_register(ViPipe, 0x00, 0xE4);//no-video data, 0xe4: black, 0xe5: blue

	printf("ViPipe:%d,===PR2020 1080P 30fps 8bit Init OK!===\n", ViPipe);
}

void pr2020_init(VI_PIPE ViPipe)
{

}

void pr2020_exit(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "Exit Pixelplus PR2020 Sensor\n");
	pr2020_i2c_exit(ViPipe);
}

CVI_S32 AHD_PR2020_set_mode(VI_PIPE ViPipe, CVI_S32 mode)
{
	pr2020_write_register(ViPipe, 0xff, 0x00);//reset
	pr2020_write_register(ViPipe, 0x11, 0x00);	
	switch (mode) {
	case AHD_MODE_1280X720P60:
		pr2020_set_cvbs_ntsc_60(ViPipe);
		break;
	case AHD_MODE_1280X720P50:
		pr2020_set_cvbs_pal_50(ViPipe);
		break;
	case AHD_MODE_1280X720P25:
		pr2020_set_720p_25(ViPipe);
		break;
	case AHD_MODE_1280X720P30:
		pr2020_set_720p_30(ViPipe);
		break;
	case AHD_MODE_1920X1080P25:
		pr2020_set_1080p_25(ViPipe);
		break;
	case AHD_MODE_1920X1080P30:
		pr2020_set_1080p_30(ViPipe);
		break;
	default:
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 AHD_PR2020_get_mode(VI_PIPE ViPipe)
{
	CVI_U8 lockstatus = 0;
	CVI_U8 detvideo = 0;
	CVI_U8 temp = 0;
	SNS_AHD_MODE_S signal_type = AHD_MODE_NONE;

	lockstatus = pr2020_read_register(ViPipe, 0x01);
	detvideo = pr2020_read_register(ViPipe, 0x00);
	temp = pr2020_read_register(ViPipe, 0x10);
	CVI_TRACE_SNS(CVI_DBG_INFO, "detvideo = 0x%2x, lockstatus = 0x%2x, temp = 0x%2x!!!\n",
			detvideo, lockstatus, temp);
	if (((lockstatus & 0x18) == 0x18) && ((detvideo & 0x08) == 0x08)) { //camera plug in
		//for test start
		if ((detvideo & 0x03) == 0x00) { //NTSC
				signal_type = AHD_MODE_1280X720H_NTSC;
		} else if ((detvideo & 0x03) == 0x01) { //PAL
				signal_type = AHD_MODE_1280X720H_PAL;
		} else if ((detvideo & 0x03) == 0x02) { //720p
			if ((detvideo & 0x30) == 0x00) { //25fps
				signal_type = AHD_MODE_1280X720P25;
			} else if ((detvideo & 0x30) == 0x10) { //30fps
				signal_type = AHD_MODE_1280X720P30;
			} else if ((detvideo & 0x30) == 0x20) { //50fps
				signal_type = AHD_MODE_NONE;
			} else if ((detvideo & 0x30) == 0x30) { //60fps
				signal_type = AHD_MODE_NONE;
			}
		} else if ((detvideo & 0x03) == 0x03) { //1080p
			if ((detvideo & 0x30) == 0x00) { //25fps
				signal_type = AHD_MODE_1920X1080P25;
			} else if ((detvideo & 0x30) == 0x10) { //30fps
				signal_type = AHD_MODE_1920X1080P30;
			}
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "detect nothing!!!\n");
			signal_type = AHD_MODE_NONE;
			return signal_type;
		}
		pr2020_write_register(ViPipe, 0xff, 0x00);
		temp = pr2020_read_register(ViPipe, 0x11);
		if (temp != 0x00)
			pr2020_write_register(ViPipe, 0x11, 0x00);
#if 0
		//for test end
		//read camera plug and signal state
		//mdelay(100);
		CVI_TRACE_SNS(CVI_DBG_INFO, "pr2020 read reg 0x%2x, 0x%2x, 0x%2x, 0x%2x, 0x%2x\n",
				pr2020_read_register(ViPipe, 0xff),
				pr2020_read_register(ViPipe, 0x00),
				pr2020_read_register(ViPipe, 0x01),
				pr2020_read_register(ViPipe, 0x10),
				pr2020_read_register(ViPipe, 0x11));

#endif
	} else {
		pr2020_write_register(ViPipe, 0xff, 0x00);
		pr2020_write_register(ViPipe, 0x11, 0x00);
		signal_type = AHD_MODE_NONE;
		CVI_TRACE_SNS(CVI_DBG_INFO, "pr2020 has no signal!\n");
	}
	return signal_type;
}

CVI_S32 AHD_PR2020_set_bus_info(VI_PIPE ViPipe, CVI_S32 astI2cDev)
{
	if (ViPipe > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "invalid vipipe !!\n");
		return CVI_FAILURE;
	}
	g_aunPr2020_BusInfo[ViPipe].s8I2cDev = astI2cDev;
	return CVI_SUCCESS;
}

CVI_S32 AHD_PR2020_detect_status(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType)
{
	if (ViPipe > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "invalid vipipe !!\n");
		return CVI_FAILURE;
	}
	CVI_S32 signal_type = -1;

	usleep(100 * 1000);

	signal_type = AHD_PR2020_get_mode(ViPipe);
	if (ahdOldType == signal_type) {
		detect_cnt = 0;
		return CVI_FAILURE;
	}
	if (ahdOldType != signal_type && detect_cnt < 3) {
		detect_cnt++;
		return CVI_FAILURE;
	}
	*ahdType = signal_type;

	return CVI_SUCCESS;
}

int AHD_PR2020_Init(VI_PIPE ViPipe, bool isFirstInit)
{
	CVI_U32 chip_id = 0;
	if (isFirstInit) {
		if (pr2020_gpio_init(ViPipe) != CVI_SUCCESS) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "PR2020 gpio init fail\n");
			return CVI_FAILURE;
		}
	}

	delay_ms(20);

	if (pr2020_i2c_init(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "PR2020 i2c init fail,please check i2c_id !!\n");
		return CVI_FAILURE;
	}

	// check sensor chip id
	pr2020_write_register(ViPipe, 0xff, 0x00);
	chip_id = (pr2020_read_register(ViPipe, 0xfc) << 8) | (pr2020_read_register(ViPipe, 0xfd));
	if (chip_id != 0x2000) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "vipipe[%d] read PR2020 chip id fail,read=%d\n", ViPipe, chip_id);
		return CVI_FAILURE;
	}

	if (isFirstInit) {
		printf("\nLoading Pixelplus PR2020 sensor\n");
		pr2020_write_register(ViPipe, 0xff, 0x00);//reset
		pr2020_write_register(ViPipe, 0x11, 0x00);
		pr2020_fw_init(ViPipe);
		// delay_ms(400);
	}
	return CVI_SUCCESS;
}

int AHD_PR2020_Deinit(VI_PIPE ViPipe)
{
	return CVI_SUCCESS;
}

SNS_AHD_OBJ_S stAhdPr2020Obj =
{
    .pfnAhdInit = AHD_PR2020_Init,
    .pfnAhdDeinit = AHD_PR2020_Deinit,
    .pfnGetAhdMode = AHD_PR2020_get_mode,
    .pfnSetAhdMode = AHD_PR2020_set_mode,
    .pfnSetAhdBusInfo = AHD_PR2020_set_bus_info,
	.pfnDetectAhdStatus = AHD_PR2020_detect_status,
};
