#include <unistd.h>

#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "cvi_sensor.h"

#include "xs9950_cmos_ex.h"

#define AHD_PWDN_GRP 1
#define AHD_PWDN_NUM 5

void xs9950_fw_init(VI_PIPE ViPipe);
void xs9950_set_cvbs_ntsc_60(VI_PIPE ViPipe);
void xs9950_set_cvbs_pal_50(VI_PIPE ViPipe);
void xs9950_set_720p_25(VI_PIPE ViPipe);
void xs9950_set_720p_30(VI_PIPE ViPipe);
void xs9950_set_1080p_25(VI_PIPE ViPipe);
void xs9950_set_1080p_30(VI_PIPE ViPipe);
void xs9950_set_tvi_1080p_25(VI_PIPE ViPipe);
void xs9950_set_tvi_1080p_25(VI_PIPE ViPipe);

const CVI_U8 xs9950_i2c_addr = 0x30;        /* I2C slave address of XS9950*/
const CVI_U32 xs9950_addr_byte = 2;
const CVI_U32 xs9950_data_byte = 1;
static csi_gpio_t g_irgpio = {0};

int xs9950_gpio_init(VI_PIPE ViPipe)
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

int xs9950_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunXs9950_BusInfo[ViPipe].s8I2cDev;
	return sensor_i2c_init(i2c_id);
}

int xs9950_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunXs9950_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int xs9950_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunXs9950_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, xs9950_i2c_addr, (CVI_U32)addr, xs9950_addr_byte, xs9950_data_byte);
}

int xs9950_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunXs9950_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, xs9950_i2c_addr, (CVI_U32)addr, xs9950_addr_byte,
		(CVI_U32)data, xs9950_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void xs9950_fw_init(VI_PIPE ViPipe)
{
    // enVoClkEdge = NI_VO_CLK_EDGE_RISING;  // 上升沿采样
    // vo_clk = 148.5MHz
    // {0x4135, 0x02, 0x00},  // 此为默认值，BT656 D0-D7以及VO_HS/VS IO电平，bit1=0:1.8V，bit1=1:3.3V
    // {0x101F, 0x00, 0x00},  // 此为默认值，bit4=0: VCCA18_33 is 1.8V.  bit4=1:VCCA18_33 is 3.3V
    // {0x4107, 0x40, 0x00},  // 此为默认值，bit0=0:iic_rst_ls3v is 1.8V   bit0=1: iic_rst_ls3v is 3.3V
	xs9950_write_register(ViPipe, 0x4300, 0x05);
	xs9950_write_register(ViPipe, 0x4300, 0x15);
	xs9950_write_register(ViPipe, 0x4080, 0x07);
	xs9950_write_register(ViPipe, 0x4119, 0x01);
	xs9950_write_register(ViPipe, 0x0803, 0x00);
	xs9950_write_register(ViPipe, 0x4020, 0x00);
	xs9950_write_register(ViPipe, 0x080e, 0x00);
	xs9950_write_register(ViPipe, 0x080e, 0x20);
	xs9950_write_register(ViPipe, 0x080e, 0x28);
	xs9950_write_register(ViPipe, 0x4020, 0x03);
	xs9950_write_register(ViPipe, 0x0803, 0x0f);
	xs9950_write_register(ViPipe, 0x0100, 0x35);
	xs9950_write_register(ViPipe, 0x0104, 0x48);
	xs9950_write_register(ViPipe, 0x0300, 0x3f);
	xs9950_write_register(ViPipe, 0x0105, 0xe1);
	xs9950_write_register(ViPipe, 0x0101, 0x42);
	xs9950_write_register(ViPipe, 0x0102, 0x40);
	xs9950_write_register(ViPipe, 0x0116, 0x3c);
	xs9950_write_register(ViPipe, 0x0117, 0x23);
	xs9950_write_register(ViPipe, 0x0333, 0x09);
	xs9950_write_register(ViPipe, 0x0337, 0xd9);
	xs9950_write_register(ViPipe, 0x0338, 0x0a);
	xs9950_write_register(ViPipe, 0x01bf, 0x4e);
	xs9950_write_register(ViPipe, 0x010e, 0x78);
	xs9950_write_register(ViPipe, 0x010f, 0x92);
	xs9950_write_register(ViPipe, 0x0110, 0x70);
	xs9950_write_register(ViPipe, 0x0111, 0x40);
	xs9950_write_register(ViPipe, 0x01e1, 0xff);
	xs9950_write_register(ViPipe, 0x0314, 0x66);
	xs9950_write_register(ViPipe, 0x0130, 0x10);
	xs9950_write_register(ViPipe, 0x0315, 0x23);
	xs9950_write_register(ViPipe, 0x0b64, 0x02);
	xs9950_write_register(ViPipe, 0x01e2, 0x03);
	xs9950_write_register(ViPipe, 0x0b55, 0x80);
	xs9950_write_register(ViPipe, 0x0b56, 0x00);
	xs9950_write_register(ViPipe, 0x0b59, 0x04);
	xs9950_write_register(ViPipe, 0x0b5a, 0x01);
	xs9950_write_register(ViPipe, 0x0b5c, 0x07);
	xs9950_write_register(ViPipe, 0x0b5e, 0x05);
	xs9950_write_register(ViPipe, 0x0b4b, 0x10);
	xs9950_write_register(ViPipe, 0x0b4e, 0x05);
	xs9950_write_register(ViPipe, 0x0b51, 0x21);
	xs9950_write_register(ViPipe, 0x0b30, 0xbc);
	xs9950_write_register(ViPipe, 0x0b31, 0x19);
	xs9950_write_register(ViPipe, 0x0b15, 0x03);
	xs9950_write_register(ViPipe, 0x0b16, 0x03);
	xs9950_write_register(ViPipe, 0x0b17, 0x03);
	xs9950_write_register(ViPipe, 0x0b07, 0x03);
	xs9950_write_register(ViPipe, 0x0b08, 0x05);
	xs9950_write_register(ViPipe, 0x0b1a, 0x10);
	xs9950_write_register(ViPipe, 0x0158, 0x03);
	xs9950_write_register(ViPipe, 0x0a88, 0x20);
	xs9950_write_register(ViPipe, 0x0a61, 0x09);
	xs9950_write_register(ViPipe, 0x0a62, 0x00);
	xs9950_write_register(ViPipe, 0x0a63, 0x0e);
	xs9950_write_register(ViPipe, 0x0a64, 0x00);
	xs9950_write_register(ViPipe, 0x0a65, 0xfc);
	xs9950_write_register(ViPipe, 0x0a67, 0xe5);
	xs9950_write_register(ViPipe, 0x0a69, 0xef);
	xs9950_write_register(ViPipe, 0x0a6b, 0x1b);
	xs9950_write_register(ViPipe, 0x0a6d, 0x2f);
	xs9950_write_register(ViPipe, 0x0a6f, 0x00);
	xs9950_write_register(ViPipe, 0x0a71, 0xc2);
	xs9950_write_register(ViPipe, 0x0a72, 0xff);
	xs9950_write_register(ViPipe, 0x0a73, 0xd0);
	xs9950_write_register(ViPipe, 0x0a74, 0xff);
	xs9950_write_register(ViPipe, 0x0a75, 0x29);
	xs9950_write_register(ViPipe, 0x0a77, 0x57);
	xs9950_write_register(ViPipe, 0x0a78, 0x00);
	xs9950_write_register(ViPipe, 0x0a79, 0x10);
	xs9950_write_register(ViPipe, 0x0a7a, 0x00);
	xs9950_write_register(ViPipe, 0x0a7b, 0xaa);
	xs9950_write_register(ViPipe, 0x0a7d, 0xb2);
	xs9950_write_register(ViPipe, 0x0a7f, 0x24);
	xs9950_write_register(ViPipe, 0x0a80, 0x00);
	xs9950_write_register(ViPipe, 0x0a81, 0x69);
	xs9950_write_register(ViPipe, 0x0a82, 0x00);
	xs9950_write_register(ViPipe, 0x0802, 0x02);
	xs9950_write_register(ViPipe, 0x0501, 0x81);
	xs9950_write_register(ViPipe, 0x0b74, 0xfc);
	xs9950_write_register(ViPipe, 0x01dc, 0x01);
	xs9950_write_register(ViPipe, 0x0804, 0x04);
	xs9950_write_register(ViPipe, 0x4018, 0x01);
	xs9950_write_register(ViPipe, 0x0b56, 0x01);
	xs9950_write_register(ViPipe, 0x0b73, 0x02);
	xs9950_write_register(ViPipe, 0x4210, 0x0c);
	xs9950_write_register(ViPipe, 0x420b, 0x2f);
	xs9950_write_register(ViPipe, 0x0504, 0x89);  // bit[4:7] free_run颜色, 取值范围:[0, 8]
	xs9950_write_register(ViPipe, 0x0507, 0x13);  // bit[5] 0:上升沿采集模式; 1:上下边沿采集模式;
	xs9950_write_register(ViPipe, 0x0503, 0x00);
	xs9950_write_register(ViPipe, 0x0502, 0x0c);  // bit4 bt data0-7 取反，改为data7-0
	xs9950_write_register(ViPipe, 0x015a, 0x00);
	xs9950_write_register(ViPipe, 0x015b, 0x24);
	xs9950_write_register(ViPipe, 0x015c, 0x80);
	xs9950_write_register(ViPipe, 0x015d, 0x16);
	xs9950_write_register(ViPipe, 0x015e, 0xd0);
	xs9950_write_register(ViPipe, 0x015f, 0x02);
	xs9950_write_register(ViPipe, 0x0160, 0xee);
	xs9950_write_register(ViPipe, 0x0161, 0x02);
	xs9950_write_register(ViPipe, 0x0165, 0x00);
	xs9950_write_register(ViPipe, 0x0166, 0x0f);
	xs9950_write_register(ViPipe, 0x4030, 0x15);
	xs9950_write_register(ViPipe, 0x4134, 0x0a);  // 增加BT输出驱动能力0x6 -> 0xa
	xs9950_write_register(ViPipe, 0x0803, 0x0f);
	xs9950_write_register(ViPipe, 0x4412, 0x01);
	xs9950_write_register(ViPipe, 0x0803, 0x1f);
	xs9950_write_register(ViPipe, 0x10e3, 0x04);
	xs9950_write_register(ViPipe, 0x10eb, 0xfd);
	xs9950_write_register(ViPipe, 0x0800, 0x07);
	xs9950_write_register(ViPipe, 0x0805, 0x07);
	xs9950_write_register(ViPipe, 0x4200, 0x02);  // 0x4200=2：选择VINA，0x4200=0：选择VINB，0x4200=4：选择VINC，0x4200=6：选择VIND
	xs9950_write_register(ViPipe, 0x060b, 0x00);
    // {0x0111, 0x68, 0x00},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x4202, 0x0e, 0x00},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x4203, 0x04, 0x00},  // 适当调整这三个寄存器，也可增加清晰度和饱和度
    // {0x0100, 0x38, 0x00},  //
    // {0x01ce, 0x00, 0x00},  // 调整自动曝光
	printf("ViPipe:%d,===XS9950 FW BT656 Init OK!===\n", ViPipe);
}

void xs9950_set_cvbs_ntsc_60(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys
	printf("ViPipe:%d,==SET XS9950 CVBS_NTSC OK!===\n", ViPipe);
}

void xs9950_set_cvbs_pal_50(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys
	printf("ViPipe:%d,==SET XS9950 CVBS_PAL OK!===\n", ViPipe);
}

void xs9950_set_720p_25(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	//Page0 sys

	printf("ViPipe:%d,===XS9950 720P 25fps 8bit Init OK!===\n", ViPipe);
}

void xs9950_set_720p_30(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);
	printf("ViPipe:%d,===XS9950 720P 30fps 8bit Init OK!===\n", ViPipe);
}

void xs9950_set_1080p_25(VI_PIPE ViPipe)
{
	xs9950_write_register(ViPipe, 0x0627, 0x14);
	xs9950_write_register(ViPipe, 0x010c, 0x00);
	xs9950_write_register(ViPipe, 0x0800, 0x05);
	xs9950_write_register(ViPipe, 0x0805, 0x05);
	xs9950_write_register(ViPipe, 0x0b50, 0x08);
	xs9950_write_register(ViPipe, 0x0e08, 0x00);
	xs9950_write_register(ViPipe, 0x010d, 0x44);
	xs9950_write_register(ViPipe, 0x010c, 0x01);
	xs9950_write_register(ViPipe, 0x0121, 0x7a);
	xs9950_write_register(ViPipe, 0x0122, 0x6b);
	xs9950_write_register(ViPipe, 0x0130, 0x10);
	xs9950_write_register(ViPipe, 0x01a9, 0x00);
	xs9950_write_register(ViPipe, 0x01aa, 0x04);
	xs9950_write_register(ViPipe, 0x0156, 0x00);
	xs9950_write_register(ViPipe, 0x0157, 0x08);
	xs9950_write_register(ViPipe, 0x0105, 0xc1);  // AHD制式如发现颜色饱和度异常，可选择开关0x105的bit5
	xs9950_write_register(ViPipe, 0x0101, 0x42);
	xs9950_write_register(ViPipe, 0x0102, 0x40);
	xs9950_write_register(ViPipe, 0x0116, 0x3c);
	xs9950_write_register(ViPipe, 0x0117, 0x23);  // AHD制式如发现颜色饱和度异常，可选择打开0x105的bit5，并微调0x117
	xs9950_write_register(ViPipe, 0x01e2, 0x03);
	xs9950_write_register(ViPipe, 0x420b, 0x21);
	xs9950_write_register(ViPipe, 0x0106, 0x80);
	xs9950_write_register(ViPipe, 0x0107, 0x00);
	xs9950_write_register(ViPipe, 0x0108, 0x80);
	xs9950_write_register(ViPipe, 0x0109, 0x00);
	xs9950_write_register(ViPipe, 0x010a, 0x1b);
	xs9950_write_register(ViPipe, 0x010b, 0x01);
	xs9950_write_register(ViPipe, 0x011d, 0x17);
	xs9950_write_register(ViPipe, 0x0e08, 0x01);
	xs9950_write_register(ViPipe, 0x0a60, 0x04);
	xs9950_write_register(ViPipe, 0x0a5c, 0x56);
	xs9950_write_register(ViPipe, 0x0a5d, 0x00);
	xs9950_write_register(ViPipe, 0x0a5e, 0xe7);
	xs9950_write_register(ViPipe, 0x0a5f, 0x38);
	xs9950_write_register(ViPipe, 0x0156, 0x50);
	xs9950_write_register(ViPipe, 0x0157, 0x07);
	xs9950_write_register(ViPipe, 0x0156, 0x00);
	xs9950_write_register(ViPipe, 0x0157, 0x08);
	xs9950_write_register(ViPipe, 0x0503, 0x04);
	xs9950_write_register(ViPipe, 0x015a, 0xd1);
	xs9950_write_register(ViPipe, 0x015b, 0x15);
	xs9950_write_register(ViPipe, 0x015c, 0x00);
	xs9950_write_register(ViPipe, 0x015d, 0x0f);
	xs9950_write_register(ViPipe, 0x015e, 0x38);
	xs9950_write_register(ViPipe, 0x015f, 0x04);
	xs9950_write_register(ViPipe, 0x0160, 0x65);
	xs9950_write_register(ViPipe, 0x0161, 0x04);
	xs9950_write_register(ViPipe, 0x0165, 0x44);
	xs9950_write_register(ViPipe, 0x0166, 0x0f);
	xs9950_write_register(ViPipe, 0x0A00, 0x00);
	xs9950_write_register(ViPipe, 0x0A01, 0x00);
	xs9950_write_register(ViPipe, 0x0A02, 0xFE);
	xs9950_write_register(ViPipe, 0x0A03, 0xFF);
	xs9950_write_register(ViPipe, 0x0A04, 0x04);
	xs9950_write_register(ViPipe, 0x0A05, 0x00);
	xs9950_write_register(ViPipe, 0x0A06, 0xFD);
	xs9950_write_register(ViPipe, 0x0A07, 0xFF);
	xs9950_write_register(ViPipe, 0x0A08, 0x00);
	xs9950_write_register(ViPipe, 0x0A09, 0x00);
	xs9950_write_register(ViPipe, 0x0A0A, 0x04);
	xs9950_write_register(ViPipe, 0x0A0B, 0x00);
	xs9950_write_register(ViPipe, 0x0A0C, 0xF9);
	xs9950_write_register(ViPipe, 0x0A0D, 0xFF);
	xs9950_write_register(ViPipe, 0x0A0E, 0x06);
	xs9950_write_register(ViPipe, 0x0A0F, 0x00);
	xs9950_write_register(ViPipe, 0x0A10, 0x00);
	xs9950_write_register(ViPipe, 0x0A11, 0x00);
	xs9950_write_register(ViPipe, 0x0A12, 0xF8);
	xs9950_write_register(ViPipe, 0x0A13, 0xFF);
	xs9950_write_register(ViPipe, 0x0A14, 0x0E);
	xs9950_write_register(ViPipe, 0x0A15, 0x00);
	xs9950_write_register(ViPipe, 0x0A16, 0xF5);
	xs9950_write_register(ViPipe, 0x0A17, 0xFF);
	xs9950_write_register(ViPipe, 0x0A18, 0x00);
	xs9950_write_register(ViPipe, 0x0A19, 0x00);
	xs9950_write_register(ViPipe, 0x0A1A, 0x0F);
	xs9950_write_register(ViPipe, 0x0A1B, 0x00);
	xs9950_write_register(ViPipe, 0x0A1C, 0xE7);
	xs9950_write_register(ViPipe, 0x0A1D, 0xFF);
	xs9950_write_register(ViPipe, 0x0A1E, 0x14);
	xs9950_write_register(ViPipe, 0x0A1F, 0x00);
	xs9950_write_register(ViPipe, 0x0A20, 0x00);
	xs9950_write_register(ViPipe, 0x0A21, 0x00);
	xs9950_write_register(ViPipe, 0x0A22, 0xE3);
	xs9950_write_register(ViPipe, 0x0A23, 0xFF);
	xs9950_write_register(ViPipe, 0x0A24, 0x31);
	xs9950_write_register(ViPipe, 0x0A25, 0x00);
	xs9950_write_register(ViPipe, 0x0A26, 0xD5);
	xs9950_write_register(ViPipe, 0x0A27, 0xFF);
	xs9950_write_register(ViPipe, 0x0A28, 0x00);
	xs9950_write_register(ViPipe, 0x0A29, 0x00);
	xs9950_write_register(ViPipe, 0x0A2A, 0x4B);
	xs9950_write_register(ViPipe, 0x0A2B, 0x00);
	xs9950_write_register(ViPipe, 0x0A2C, 0x5F);
	xs9950_write_register(ViPipe, 0x0A2D, 0xFF);
	xs9950_write_register(ViPipe, 0x0A2E, 0xE6);
	xs9950_write_register(ViPipe, 0x0A2F, 0x00);
	xs9950_write_register(ViPipe, 0x0A30, 0x00);
	xs9950_write_register(ViPipe, 0x0A31, 0x03);
	xs9950_write_register(ViPipe, 0x0A60, 0x01);

	printf("ViPipe:%d,===XS9950 1080P 25fps 8bit Init OK!===\n", ViPipe);
}

void xs9950_set_1080p_30(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe=%d\n", ViPipe);

	printf("ViPipe:%d,===XS9950 1080P 30fps 8bit Init OK!===\n", ViPipe);
}


void xs9950_set_tvi_1080p_25(VI_PIPE ViPipe)
{
   printf("xs9950_set_tvi_1080p_25\n");
}

int xs9950_probe(VI_PIPE ViPipe)
{
	CVI_U32 chip_id = 0;
	int i=0;

	if (xs9950_i2c_init(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "XS9950 i2c init fail, please check i2c_id !!\n");
		return CVI_FAILURE;
	}

	// check sensor chip id
	for(i=0;i<3;i++)
	{
		chip_id = (xs9950_read_register(ViPipe, 0x40f0) << 8) | (xs9950_read_register(ViPipe, 0x40f1));
		printf("vipipe[%d] read XS9950 chip id ,read=0x%x\n", ViPipe, chip_id);
		delay_ms(1);
	}
	if (chip_id != 0x9950) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "vipipe[%d] read XS9950 chip id fail,read=0x%f\n", ViPipe, chip_id);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void xs9950_init(VI_PIPE ViPipe)
{
	xs9950_fw_init(ViPipe);
	xs9950_set_1080p_25(ViPipe);
}

void xs9950_exit(VI_PIPE ViPipe)
{
	CVI_TRACE_SNS(CVI_DBG_INFO, "Exit Pixelplus XS9950 Sensor\n");
	xs9950_i2c_exit(ViPipe);
}

CVI_S32 AHD_XS9950_set_mode(VI_PIPE ViPipe, CVI_S32 mode)
{
	switch (mode) {
	case AHD_MODE_1280X720P60:
		xs9950_set_cvbs_ntsc_60(ViPipe);
		break;
	case AHD_MODE_1280X720P50:
		xs9950_set_cvbs_pal_50(ViPipe);
		break;
	case AHD_MODE_1280X720P25:
		xs9950_set_720p_25(ViPipe);
		break;
	case AHD_MODE_1280X720P30:
		xs9950_set_720p_30(ViPipe);
		break;
	case AHD_MODE_1920X1080P25:
		xs9950_set_1080p_25(ViPipe);
		break;
	case AHD_MODE_1920X1080P30:
		xs9950_set_1080p_30(ViPipe);
		break;
	case AHD_MODE_BUIT:
	default:
		break;
	}
	return CVI_SUCCESS;
}


CVI_S32 AHD_XS9950_get_mode(VI_PIPE ViPipe)
{
    CVI_U8 lockstatus = 0;
	CVI_U8 detvideo = 0;
	SNS_AHD_MODE_S signal_type = AHD_MODE_NONE;

	lockstatus = xs9950_read_register(ViPipe, 0x00) & 0x0e;
	detvideo = xs9950_read_register(ViPipe, 0x01) & 0xFF;
	CVI_TRACE_SNS(CVI_DBG_INFO, "detvideo = 0x%2x, lockstatus = 0x%2x!!!\n", detvideo, lockstatus);
	if (lockstatus == 0x0e) { //camera plug in
        if(detvideo == 0x0 || detvideo == 0x40 || detvideo== 0x80){
            signal_type = AHD_MODE_1280X720P25;
        }else if(detvideo == 0x1 || detvideo == 0x41 || detvideo == 0x81 || detvideo == 0x1A){
            signal_type = AHD_MODE_1280X720P30;
        }else if(detvideo == 0x04 || detvideo == 0x44 || detvideo== 0x84){
            signal_type = AHD_MODE_1920X1080P25;
        }
        else if(detvideo == 0x05 || detvideo == 0x45 || detvideo== 0x85){
            signal_type = AHD_MODE_1920X1080P30;
        }
	} else {
		signal_type = AHD_MODE_NONE;
		CVI_TRACE_SNS(CVI_DBG_INFO, "xs9951 has no signal!\n");
	}
	return signal_type;
}

CVI_S32 AHD_XS9950_set_bus_info(VI_PIPE ViPipe, CVI_S32 astI2cDev)
{
	if (ViPipe > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "invalid vipipe !!\n");
		return CVI_FAILURE;
	}
	g_aunXs9950_BusInfo[ViPipe].s8I2cDev = astI2cDev;
	return CVI_SUCCESS;
}

int AHD_XS9950_Init(VI_PIPE ViPipe, bool isFirstInit)
{
	CVI_U32 chip_id = 0;

	int i=0;
	if (isFirstInit) {
		printf("####### %s, %d\n", __func__, __LINE__);
		if (xs9950_gpio_init(ViPipe) != CVI_SUCCESS) {
			printf("####### %s, %d\n", __func__, __LINE__);
			CVI_TRACE_SNS(CVI_DBG_ERR, "XS9950 gpio init fail\n");
			return CVI_FAILURE;
		}
	}

	delay_ms(20);

	if (xs9950_i2c_init(ViPipe) != CVI_SUCCESS) {
		printf("####### %s, %d\n", __func__, __LINE__);
		CVI_TRACE_SNS(CVI_DBG_ERR, "XS9950 i2c init fail,please check i2c_id !!\n");
		return CVI_FAILURE;
	}

	// check sensor chip id
	for(i=0;i<3;i++)
	{
		chip_id = (xs9950_read_register(ViPipe, 0x40f0) << 8) | (xs9950_read_register(ViPipe, 0x40f1));
		printf("vipipe[%d] read XS9950 chip id ,read=0x%x\n", ViPipe, chip_id);
		delay_ms(1);
	}
	if (chip_id != 0x9950) {
		printf("####### %s, %d\n", __func__, __LINE__);
		CVI_TRACE_SNS(CVI_DBG_ERR, "vipipe[%d] read XS9950 chip id fail,read=0x%f\n", ViPipe, chip_id);
		return CVI_FAILURE;
	}
printf("####### %s, %d\n", __func__, __LINE__);
	if (isFirstInit) {
		printf("####### %s, %d\n", __func__, __LINE__);
		printf("\nLoading Pixelplus XS9950 sensor\n");
		xs9950_fw_init(ViPipe);
	}
	return CVI_SUCCESS;
}

int AHD_XS9950_Deinit(VI_PIPE ViPipe)
{
	return CVI_SUCCESS;
}

SNS_AHD_OBJ_S stAhdXs9950Obj =
{
    .pfnAhdInit = AHD_XS9950_Init,
    .pfnAhdDeinit = AHD_XS9950_Deinit,
    .pfnGetAhdMode = AHD_XS9950_get_mode,
    .pfnSetAhdMode = AHD_XS9950_set_mode,
    .pfnSetAhdBusInfo = AHD_XS9950_set_bus_info,
};
