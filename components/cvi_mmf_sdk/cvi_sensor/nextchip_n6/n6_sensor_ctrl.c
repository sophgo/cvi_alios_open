#include <unistd.h>
#include <cvi_comm_video.h>
#include <linux/spi/spidev.h>
#include "cvi_sns_ctrl.h"
#include "n6_cmos_ex.h"
#include <pthread.h>
#include <signal.h>
#include "sensor_i2c.h"

const CVI_U8 n6_i2c_addr = 0x31;        /* I2C slave address of N6, SA0=0:0x32, SA0=1:0x33*/
const CVI_U32 n6_addr_byte = 1;
const CVI_U32 n6_data_byte = 1;
static CVI_U32 adet;
//static CVI_U32 fmt = 1; //0:960h 1:720P 2:1080P
//static unsigned int ntpal = 0; //0:ntsc/30p 1:pal/25p
static CVI_U32 mclk = 1; //0:1458 1:756

#define N6_TEST_PATTERN 1

int n6_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunN6_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunN6_AddrInfo[ViPipe].s8I2cAddr);
}

int n6_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunN6_BusInfo[ViPipe].s8I2cDev);
}

int n6_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunN6_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunN6_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							n6_addr_byte, n6_data_byte);
}

int n6_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunN6_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunN6_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							n6_addr_byte, (CVI_U32)data, n6_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void n6_common_setting(VI_PIPE ViPipe)
{
	CVI_U8 ch;

	n6_write_register(ViPipe, 0xff, 0x00);
	n6_write_register(ViPipe, 0x80, 0x0f);
	n6_write_register(ViPipe, 0x00, 0x10);
	n6_write_register(ViPipe, 0x01, 0x10);
	n6_write_register(ViPipe, 0x02, 0x10);
	n6_write_register(ViPipe, 0x03, 0x10);
	n6_write_register(ViPipe, 0x22, 0x0b);
	n6_write_register(ViPipe, 0x23, 0x41);
	n6_write_register(ViPipe, 0x26, 0x0b);
	n6_write_register(ViPipe, 0x27, 0x41);
	n6_write_register(ViPipe, 0x2a, 0x0b);
	n6_write_register(ViPipe, 0x2b, 0x41);
	n6_write_register(ViPipe, 0x2e, 0x0b);
	n6_write_register(ViPipe, 0x2f, 0x41);

	n6_write_register(ViPipe, 0xff, 0x01);
	n6_write_register(ViPipe, 0x98, 0x30);
	n6_write_register(ViPipe, 0xed, 0x00);

	for (ch = 0; ch < 4; ch++) {
		n6_write_register(ViPipe, 0xff, 0x05 + ch);
		n6_write_register(ViPipe, 0x00, 0xd0);
		n6_write_register(ViPipe, 0x01, 0x22);
		n6_write_register(ViPipe, 0x47, 0xee);
		n6_write_register(ViPipe, 0x50, 0xc6);
		n6_write_register(ViPipe, 0x57, 0x00);
		n6_write_register(ViPipe, 0x58, 0x77);
		n6_write_register(ViPipe, 0x5b, 0x41);
		n6_write_register(ViPipe, 0x5c, 0x78);
		n6_write_register(ViPipe, 0xB8, 0xB8);
	}

	n6_write_register(ViPipe, 0xff, 0x09);
	n6_write_register(ViPipe, 0x50, 0x30);
	n6_write_register(ViPipe, 0x51, 0x6f);
	n6_write_register(ViPipe, 0x52, 0x67);
	n6_write_register(ViPipe, 0x53, 0x48);
	n6_write_register(ViPipe, 0x54, 0x30);
	n6_write_register(ViPipe, 0x55, 0x6f);
	n6_write_register(ViPipe, 0x56, 0x67);
	n6_write_register(ViPipe, 0x57, 0x48);
	n6_write_register(ViPipe, 0x58, 0x30);
	n6_write_register(ViPipe, 0x59, 0x6f);
	n6_write_register(ViPipe, 0x5a, 0x67);
	n6_write_register(ViPipe, 0x5b, 0x48);
	n6_write_register(ViPipe, 0x5c, 0x30);
	n6_write_register(ViPipe, 0x5d, 0x6f);
	n6_write_register(ViPipe, 0x5e, 0x67);
	n6_write_register(ViPipe, 0x5f, 0x48);

	n6_write_register(ViPipe, 0xff, 0x0a);
	n6_write_register(ViPipe, 0x25, 0x10);
	n6_write_register(ViPipe, 0x27, 0x1e);
	n6_write_register(ViPipe, 0x30, 0xac);
	n6_write_register(ViPipe, 0x31, 0x78);
	n6_write_register(ViPipe, 0x32, 0x17);
	n6_write_register(ViPipe, 0x33, 0xc1);
	n6_write_register(ViPipe, 0x34, 0x40);
	n6_write_register(ViPipe, 0x35, 0x00);
	n6_write_register(ViPipe, 0x36, 0xc3);
	n6_write_register(ViPipe, 0x37, 0x0a);
	n6_write_register(ViPipe, 0x38, 0x00);
	n6_write_register(ViPipe, 0x39, 0x02);
	n6_write_register(ViPipe, 0x3a, 0x00);
	n6_write_register(ViPipe, 0x3b, 0xb2);
	n6_write_register(ViPipe, 0xa5, 0x10);
	n6_write_register(ViPipe, 0xa7, 0x1e);
	n6_write_register(ViPipe, 0xb0, 0xac);
	n6_write_register(ViPipe, 0xb1, 0x78);
	n6_write_register(ViPipe, 0xb2, 0x17);
	n6_write_register(ViPipe, 0xb3, 0xc1);
	n6_write_register(ViPipe, 0xb4, 0x40);
	n6_write_register(ViPipe, 0xb5, 0x00);
	n6_write_register(ViPipe, 0xb6, 0xc3);
	n6_write_register(ViPipe, 0xb7, 0x0a);
	n6_write_register(ViPipe, 0xb8, 0x00);
	n6_write_register(ViPipe, 0xb9, 0x02);
	n6_write_register(ViPipe, 0xba, 0x00);
	n6_write_register(ViPipe, 0xbb, 0xb2);
	n6_write_register(ViPipe, 0xff, 0x0b);
	n6_write_register(ViPipe, 0x25, 0x10);
	n6_write_register(ViPipe, 0x27, 0x1e);
	n6_write_register(ViPipe, 0x30, 0xac);
	n6_write_register(ViPipe, 0x31, 0x78);
	n6_write_register(ViPipe, 0x32, 0x17);
	n6_write_register(ViPipe, 0x33, 0xc1);
	n6_write_register(ViPipe, 0x34, 0x40);
	n6_write_register(ViPipe, 0x35, 0x00);
	n6_write_register(ViPipe, 0x36, 0xc3);
	n6_write_register(ViPipe, 0x37, 0x0a);
	n6_write_register(ViPipe, 0x38, 0x00);
	n6_write_register(ViPipe, 0x39, 0x02);
	n6_write_register(ViPipe, 0x3a, 0x00);
	n6_write_register(ViPipe, 0x3b, 0xb2);
	n6_write_register(ViPipe, 0xa5, 0x10);
	n6_write_register(ViPipe, 0xa7, 0x1e);
	n6_write_register(ViPipe, 0xb0, 0xac);
	n6_write_register(ViPipe, 0xb1, 0x78);
	n6_write_register(ViPipe, 0xb2, 0x17);
	n6_write_register(ViPipe, 0xb3, 0xc1);
	n6_write_register(ViPipe, 0xb4, 0x40);
	n6_write_register(ViPipe, 0xb5, 0x00);
	n6_write_register(ViPipe, 0xb6, 0xc3);
	n6_write_register(ViPipe, 0xb7, 0x0a);
	n6_write_register(ViPipe, 0xb8, 0x00);
	n6_write_register(ViPipe, 0xb9, 0x02);
	n6_write_register(ViPipe, 0xba, 0x00);
	n6_write_register(ViPipe, 0xbb, 0xb2);

	n6_write_register(ViPipe, 0xff, 0x13);
	n6_write_register(ViPipe, 0x05, 0xa0);
	n6_write_register(ViPipe, 0x31, 0xff);
	n6_write_register(ViPipe, 0x07, 0x47);
	n6_write_register(ViPipe, 0x12, 0x04);
	n6_write_register(ViPipe, 0x1e, 0x1f);
	n6_write_register(ViPipe, 0x1f, 0x27);
	n6_write_register(ViPipe, 0x2e, 0x10);
	n6_write_register(ViPipe, 0x2f, 0xc8);
	n6_write_register(ViPipe, 0x31, 0xff);
	n6_write_register(ViPipe, 0x32, 0x00);
	n6_write_register(ViPipe, 0x33, 0x00);
	n6_write_register(ViPipe, 0x72, 0x05);
	n6_write_register(ViPipe, 0x7a, 0xf0);

	n6_write_register(ViPipe, 0xff, _MAR_BANK_);
	n6_write_register(ViPipe, 0x10, 0xff);
	n6_write_register(ViPipe, 0x11, 0xff);
	if (mclk == 1) {
		n6_write_register(ViPipe, 0x30, 0x0f);
		n6_write_register(ViPipe, 0x32, 0x92);
		n6_write_register(ViPipe, 0x34, 0xcd);
		n6_write_register(ViPipe, 0x36, 0x04);
		n6_write_register(ViPipe, 0x38, 0x58);
	} else {
		n6_write_register(ViPipe, 0x30, 0x0f);
		n6_write_register(ViPipe, 0x32, 0xff);
		n6_write_register(ViPipe, 0x34, 0xcd);
		n6_write_register(ViPipe, 0x36, 0x04);
		n6_write_register(ViPipe, 0x38, 0xff);
	}
	n6_write_register(ViPipe, 0x3c, 0x01);
	n6_write_register(ViPipe, 0x3d, 0x11);
	n6_write_register(ViPipe, 0x3e, 0x11);
	n6_write_register(ViPipe, 0x45, 0x60);
	n6_write_register(ViPipe, 0x46, 0x49);

	n6_write_register(ViPipe, 0xff, _MTX_BANK_);
	n6_write_register(ViPipe, 0xe9, 0x03);
	n6_write_register(ViPipe, 0x03, 0x02);
	n6_write_register(ViPipe, 0x01, 0xe4);
	n6_write_register(ViPipe, 0x00, 0x7d);
	n6_write_register(ViPipe, 0x01, 0xe0);
	n6_write_register(ViPipe, 0x02, 0xa0);
	n6_write_register(ViPipe, 0x20, 0x1e);
	n6_write_register(ViPipe, 0x20, 0x1f);

	if (mclk == 1) {
		n6_write_register(ViPipe, 0x04, 0x38);
		n6_write_register(ViPipe, 0x45, 0xc4);
		n6_write_register(ViPipe, 0x46, 0x01);
		n6_write_register(ViPipe, 0x47, 0x1b);
		n6_write_register(ViPipe, 0x48, 0x08);
		n6_write_register(ViPipe, 0x65, 0xc4);
		n6_write_register(ViPipe, 0x66, 0x01);
		n6_write_register(ViPipe, 0x67, 0x1b);
		n6_write_register(ViPipe, 0x68, 0x08);
		n6_write_register(ViPipe, 0x85, 0xc4);
		n6_write_register(ViPipe, 0x86, 0x01);
		n6_write_register(ViPipe, 0x87, 0x1b);
		n6_write_register(ViPipe, 0x88, 0x08);
		n6_write_register(ViPipe, 0xa5, 0xc4);
		n6_write_register(ViPipe, 0xa6, 0x01);
		n6_write_register(ViPipe, 0xa7, 0x1b);
		n6_write_register(ViPipe, 0xa8, 0x08);
		n6_write_register(ViPipe, 0xc5, 0xc4);
		n6_write_register(ViPipe, 0xc6, 0x01);
		n6_write_register(ViPipe, 0xc7, 0x1b);
		n6_write_register(ViPipe, 0xc8, 0x08);
	} else {
		n6_write_register(ViPipe, 0x04, 0x6c);
		n6_write_register(ViPipe, 0x45, 0xcd);
		n6_write_register(ViPipe, 0x46, 0x42);
		n6_write_register(ViPipe, 0x47, 0x36);
		n6_write_register(ViPipe, 0x48, 0x0f);
		n6_write_register(ViPipe, 0x65, 0xcd);
		n6_write_register(ViPipe, 0x66, 0x42);
		n6_write_register(ViPipe, 0x67, 0x0e);
		n6_write_register(ViPipe, 0x68, 0x0f);
		n6_write_register(ViPipe, 0x85, 0xcd);
		n6_write_register(ViPipe, 0x86, 0x42);
		n6_write_register(ViPipe, 0x87, 0x0e);
		n6_write_register(ViPipe, 0x88, 0x0f);
		n6_write_register(ViPipe, 0xa5, 0xcd);
		n6_write_register(ViPipe, 0xa6, 0x42);
		n6_write_register(ViPipe, 0xa7, 0x0e);
		n6_write_register(ViPipe, 0xa8, 0x0f);
		n6_write_register(ViPipe, 0xc5, 0xcd);
		n6_write_register(ViPipe, 0xc6, 0x42);
		n6_write_register(ViPipe, 0xc7, 0x0e);
		n6_write_register(ViPipe, 0xc8, 0x0f);
	}
	n6_write_register(ViPipe, 0xeb, 0x8d);

	n6_write_register(ViPipe, 0xff, _MAR_BANK_);
	n6_write_register(ViPipe, 0x00, 0xff);
	n6_write_register(ViPipe, 0x40, 0x01);
	n6_write_register(ViPipe, 0x40, 0x00);
	n6_write_register(ViPipe, 0xff, 0x01);
	n6_write_register(ViPipe, 0x97, 0x00);
	n6_write_register(ViPipe, 0x97, 0x0f);

	n6_write_register(ViPipe, 0xff, 0x00);  //test pattern
	n6_write_register(ViPipe, 0x78, 0xba);
	n6_write_register(ViPipe, 0x79, 0xac);
	n6_write_register(ViPipe, 0xff, 0x05);
	n6_write_register(ViPipe, 0x2c, 0x08);
	n6_write_register(ViPipe, 0x6a, 0x80);
	n6_write_register(ViPipe, 0xff, 0x06);
	n6_write_register(ViPipe, 0x2c, 0x08);
	n6_write_register(ViPipe, 0x6a, 0x80);
	n6_write_register(ViPipe, 0xff, 0x07);
	n6_write_register(ViPipe, 0x2c, 0x08);
	n6_write_register(ViPipe, 0x6a, 0x80);
	n6_write_register(ViPipe, 0xff, 0x08);
	n6_write_register(ViPipe, 0x2c, 0x08);
	n6_write_register(ViPipe, 0x6a, 0x80);
}

void n6_set_chn_960h(VI_PIPE ViPipe, CVI_U8 ch, CVI_U8 ntpal)
{
	CVI_U8 val_0x54, val_20x01;

	CVI_TRACE_SNS(CVI_DBG_INFO, "%s ch=%d\n", __func__, ch);

	n6_write_register(ViPipe, 0xff, 0x00);
	n6_write_register(ViPipe, 0x08 + ch, ntpal ? 0xdd : 0xa0);
	n6_write_register(ViPipe, 0x18 + ch, 0x08);
	n6_write_register(ViPipe, 0x22 + ch * 4, 0x0b);
	n6_write_register(ViPipe, 0x23 + ch * 4, 0x41);
	n6_write_register(ViPipe, 0x30 + ch, 0x12);
	n6_write_register(ViPipe, 0x34 + ch, 0x01);
	val_0x54 = n6_read_register(ViPipe, 0x54);
	if (ntpal)
		val_0x54 &= ~(0x10 << ch);
	else
		val_0x54 |= (0x10 << ch);
	n6_write_register(ViPipe, 0x54, val_0x54);
	n6_write_register(ViPipe, 0x58 + ch, ntpal ? 0x80 : 0x90);
	n6_write_register(ViPipe, 0x5c + ch, ntpal ? 0xbe : 0xbc);
	n6_write_register(ViPipe, 0x64 + ch, ntpal ? 0xa0 : 0x81);
	n6_write_register(ViPipe, 0x81 + ch, ntpal ? 0xf0 : 0xe0);
	n6_write_register(ViPipe, 0x85 + ch, 0x00);
	n6_write_register(ViPipe, 0x89 + ch, 0x00);
	n6_write_register(ViPipe, 0x8e + ch, 0x00);
	n6_write_register(ViPipe, 0xa0 + ch, 0x05);

	n6_write_register(ViPipe, 0xff, 0x01);
	n6_write_register(ViPipe, 0x84 + ch, 0x02);
	n6_write_register(ViPipe, 0x88 + ch, 0x00);
	n6_write_register(ViPipe, 0x8c + ch, 0x40);
	n6_write_register(ViPipe, 0xa0 + ch, 0x20);
	n6_write_register(ViPipe, 0xed, 0x00);

	n6_write_register(ViPipe, 0xff, 0x05 + ch);
	n6_write_register(ViPipe, 0x01, 0x22);
	n6_write_register(ViPipe, 0x05, 0x00);
	n6_write_register(ViPipe, 0x08, 0x55);
	n6_write_register(ViPipe, 0x25, 0xdc);
	n6_write_register(ViPipe, 0x28, 0x80);
	n6_write_register(ViPipe, 0x2f, 0x00);
	n6_write_register(ViPipe, 0x30, 0xe0);
	n6_write_register(ViPipe, 0x31, 0x43);
	n6_write_register(ViPipe, 0x32, 0xa2);
	n6_write_register(ViPipe, 0x47, 0x04);
	n6_write_register(ViPipe, 0x50, 0x84);
	n6_write_register(ViPipe, 0x57, 0x00);
	n6_write_register(ViPipe, 0x58, 0x77);
	n6_write_register(ViPipe, 0x5b, 0x43);
	n6_write_register(ViPipe, 0x5c, 0x78);
	n6_write_register(ViPipe, 0x5f, 0x00);
	n6_write_register(ViPipe, 0x62, 0x20);
	n6_write_register(ViPipe, 0x7b, 0x00);
	n6_write_register(ViPipe, 0x7c, 0x01);
	n6_write_register(ViPipe, 0x7d, 0x80);
	n6_write_register(ViPipe, 0x80, 0x00);
	n6_write_register(ViPipe, 0x90, 0x01);
	n6_write_register(ViPipe, 0xa9, 0x00);
	n6_write_register(ViPipe, 0xb5, 0x00);
	n6_write_register(ViPipe, 0xb8, 0xb9);
	n6_write_register(ViPipe, 0xb9, 0x72);
	n6_write_register(ViPipe, 0xd1, 0x00);
	n6_write_register(ViPipe, 0xd5, 0x80);

	n6_write_register(ViPipe, 0xff, 0x09);
	n6_write_register(ViPipe, 0x96 + ch * 0x20, 0x10);
	n6_write_register(ViPipe, 0x98 + ch * 0x20, ntpal ? 0xc0 : 0xe0);
	n6_write_register(ViPipe, 0x9e + ch * 0x20, 0x00);

	n6_write_register(ViPipe, 0xff, _MAR_BANK_);
	val_20x01 = n6_read_register(ViPipe, 0x01);
	val_20x01 &= (~(0x03 << (ch * 2)));
	val_20x01 |= (0x02 << (ch * 2));
	n6_write_register(ViPipe, 0x01, val_20x01);
	n6_write_register(ViPipe, 0x12 + ch * 2, 0xe0);
	n6_write_register(ViPipe, 0x13 + ch * 2, 0x01);

}

void n6_set_chn_720p(VI_PIPE ViPipe, CVI_U8 ch, CVI_U8 ntpal)
{
	CVI_U8 val_0x54, val_20x01;

	CVI_TRACE_SNS(CVI_DBG_INFO, "%s ch=%d\n", __func__, ch);

	n6_write_register(ViPipe, 0xff, 0x00);
	n6_write_register(ViPipe, 0x08 + ch, 0x00);
	n6_write_register(ViPipe, 0x18 + ch, 0x3f);
	n6_write_register(ViPipe, 0x30 + ch, 0x12);
	n6_write_register(ViPipe, 0x34 + ch, 0x00);
	val_0x54 = n6_read_register(ViPipe, 0x54);
	val_0x54 &= ~(0x10 << ch);
	n6_write_register(ViPipe, 0x54, val_0x54);
	n6_write_register(ViPipe, 0x58 + ch, ntpal ? 0x80 : 0x80);
	n6_write_register(ViPipe, 0x5c + ch, ntpal ? 0x00 : 0x00);
	n6_write_register(ViPipe, 0x64 + ch, ntpal ? 0x01 : 0x01);
	n6_write_register(ViPipe, 0x81 + ch, ntpal ? 0x0d : 0x0c);
	n6_write_register(ViPipe, 0x85 + ch, 0x00);
	n6_write_register(ViPipe, 0x89 + ch, 0x00);
	n6_write_register(ViPipe, 0x8e + ch, 0x00);
	n6_write_register(ViPipe, 0xa0 + ch, 0x05);

	n6_write_register(ViPipe, 0xff, 0x01);
	n6_write_register(ViPipe, 0x84 + ch, 0x02);
	n6_write_register(ViPipe, 0x88 + ch, 0x00);
	n6_write_register(ViPipe, 0x8c + ch, 0x40);
	n6_write_register(ViPipe, 0xa0 + ch, 0x20);

	n6_write_register(ViPipe, 0xff, 0x05 + ch);
	n6_write_register(ViPipe, 0x01, 0x22);
	n6_write_register(ViPipe, 0x05, 0x04);
	n6_write_register(ViPipe, 0x08, 0x55);
	n6_write_register(ViPipe, 0x25, 0xdc);
	n6_write_register(ViPipe, 0x28, 0x80);
	n6_write_register(ViPipe, 0x2f, 0x00);
	n6_write_register(ViPipe, 0x30, 0xe0);
	n6_write_register(ViPipe, 0x31, 0x43);
	n6_write_register(ViPipe, 0x32, 0xa2);
	n6_write_register(ViPipe, 0x47, 0xee);
	n6_write_register(ViPipe, 0x50, 0xc6);
	n6_write_register(ViPipe, 0x57, 0x00);
	n6_write_register(ViPipe, 0x58, 0x77);
	n6_write_register(ViPipe, 0x5b, 0x41);
	n6_write_register(ViPipe, 0x5c, 0x7C);
	n6_write_register(ViPipe, 0x5f, 0x00);
	n6_write_register(ViPipe, 0x62, 0x20);
	n6_write_register(ViPipe, 0x7b, 0x11);
	n6_write_register(ViPipe, 0x7c, 0x01);
	n6_write_register(ViPipe, 0x7d, 0x80);
	n6_write_register(ViPipe, 0x80, 0x00);
	n6_write_register(ViPipe, 0x90, 0x01);
	n6_write_register(ViPipe, 0xa9, 0x00);
	n6_write_register(ViPipe, 0xb5, 0x40);
	n6_write_register(ViPipe, 0xb8, 0x39);
	n6_write_register(ViPipe, 0xb9, 0x72);
	n6_write_register(ViPipe, 0xd1, 0x00);
	n6_write_register(ViPipe, 0xd5, 0x80);

	n6_write_register(ViPipe, 0xff, 0x09);
	n6_write_register(ViPipe, 0x96 + ch * 0x20, 0x00);
	n6_write_register(ViPipe, 0x98 + ch * 0x20, 0x00);
	n6_write_register(ViPipe, 0x9e + ch * 0x20, 0x00);

	n6_write_register(ViPipe, 0xff, _MAR_BANK_);
	val_20x01 = n6_read_register(ViPipe, 0x01);
	val_20x01 &= (~(0x03 << (ch * 2)));
	val_20x01 |= (0x01 << (ch * 2));
	n6_write_register(ViPipe, 0x01, val_20x01);
	n6_write_register(ViPipe, 0x12 + ch * 2, 0x80);
	n6_write_register(ViPipe, 0x13 + ch * 2, 0x02);

}

/*
 * 1280x960p
 * dev:0x60 / 0x62 / 0x64 / 0x66
 * ch : 0 ~ 3
 * ntpal: 1:25p, 0:30p
 */
void n6_set_chn_960p(VI_PIPE ViPipe, CVI_U8 ch, CVI_U8 ntpal)
{
	CVI_U8 val_0x54, val_20x01;

	CVI_TRACE_SNS(CVI_DBG_INFO, "%s ch=%d\n", __func__, ch);

	n6_write_register(ViPipe, 0xff, 0x00);
	n6_write_register(ViPipe, 0x08 + ch, 0x00);
	n6_write_register(ViPipe, 0x18 + ch, 0x0f);
	n6_write_register(ViPipe, 0x30 + ch, 0x12);
	n6_write_register(ViPipe, 0x34 + ch, 0x00);
	val_0x54 = n6_read_register(ViPipe, 0x54);
	val_0x54 &= ~(0x10 << ch);
	n6_write_register(ViPipe, 0x54, val_0x54);
	n6_write_register(ViPipe, 0x58 + ch, ntpal ? 0x40 : 0x48);
	n6_write_register(ViPipe, 0x5c + ch, ntpal ? 0x80 : 0x80);
	n6_write_register(ViPipe, 0x64 + ch, ntpal ? 0x28 : 0x28);
	n6_write_register(ViPipe, 0x81 + ch, ntpal ? 0x07 : 0x06);
	n6_write_register(ViPipe, 0x85 + ch, 0x0b);
	n6_write_register(ViPipe, 0x89 + ch, 0x00);
	n6_write_register(ViPipe, 0x8e + ch, 0x00);
	n6_write_register(ViPipe, 0xa0 + ch, 0x05);

	n6_write_register(ViPipe, 0xff, 0x01);
	n6_write_register(ViPipe, 0x84 + ch, 0x02);
	n6_write_register(ViPipe, 0x88 + ch, 0x00);
	n6_write_register(ViPipe, 0x8c + ch, 0x40);
	n6_write_register(ViPipe, 0xa0 + ch, 0x20);

	n6_write_register(ViPipe, 0xff, 0x05 + ch);
	n6_write_register(ViPipe, 0x01, 0x22);
	n6_write_register(ViPipe, 0x05, 0x04);
	n6_write_register(ViPipe, 0x08, 0x55);
	n6_write_register(ViPipe, 0x25, 0xdc);
	n6_write_register(ViPipe, 0x28, 0x80);
	n6_write_register(ViPipe, 0x2f, 0x00);
	n6_write_register(ViPipe, 0x30, 0xe0);
	n6_write_register(ViPipe, 0x31, 0x43);
	n6_write_register(ViPipe, 0x32, 0xa2);
	n6_write_register(ViPipe, 0x47, 0xee);
	n6_write_register(ViPipe, 0x50, 0xc6);
	n6_write_register(ViPipe, 0x57, 0x00);
	n6_write_register(ViPipe, 0x58, 0x77);
	n6_write_register(ViPipe, 0x5b, 0x41);
	n6_write_register(ViPipe, 0x5c, 0x7C);
	n6_write_register(ViPipe, 0x5f, 0x00);
	n6_write_register(ViPipe, 0x62, 0x20);
	n6_write_register(ViPipe, 0x7b, 0x11);
	n6_write_register(ViPipe, 0x7c, 0x01);
	n6_write_register(ViPipe, 0x7d, 0x80);
	n6_write_register(ViPipe, 0x80, 0x00);
	n6_write_register(ViPipe, 0x90, 0x01);
	n6_write_register(ViPipe, 0xa9, 0x00);
	n6_write_register(ViPipe, 0xb5, 0x40);
	n6_write_register(ViPipe, 0xb8, 0x39);
	n6_write_register(ViPipe, 0xb9, 0x72);
	n6_write_register(ViPipe, 0xd1, 0x00);
	n6_write_register(ViPipe, 0xd5, 0x80);

	n6_write_register(ViPipe, 0xff, 0x09);
	n6_write_register(ViPipe, 0x96 + ch * 0x20, 0x00);
	n6_write_register(ViPipe, 0x98 + ch * 0x20, 0x00);
	n6_write_register(ViPipe, 0x9e + ch * 0x20, 0x00);

	n6_write_register(ViPipe, 0xff, _MAR_BANK_);
	val_20x01 = n6_read_register(ViPipe, 0x01);
	val_20x01 &= (~(0x03 << (ch * 2)));
	//val_20x01 |=(0x01<<(ch*2));
	n6_write_register(ViPipe, 0x01, val_20x01);
	n6_write_register(ViPipe, 0x12 + ch * 2, 0x80);
	n6_write_register(ViPipe, 0x13 + ch * 2, 0x02);

}

void n6_set_chn_1080p(VI_PIPE ViPipe, CVI_U8 ch, CVI_U8 ntpal)
{
	CVI_U8 val_0x54, val_20x01;

	CVI_TRACE_SNS(CVI_DBG_INFO, "%s ch=%d\n", __func__, ch);

	n6_write_register(ViPipe, 0xff, 0x00);
	n6_write_register(ViPipe, 0x08 + ch, 0x00);
	n6_write_register(ViPipe, 0x18 + ch, 0x3f);
	n6_write_register(ViPipe, 0x30 + ch, 0x12);
	n6_write_register(ViPipe, 0x34 + ch, 0x00);
	val_0x54 = n6_read_register(ViPipe, 0x54);
	val_0x54 &= ~(0x10 << ch);
	n6_write_register(ViPipe, 0x54, val_0x54);
	n6_write_register(ViPipe, 0x58 + ch, ntpal ? 0x80 : 0x80);
	n6_write_register(ViPipe, 0x5c + ch, ntpal ? 0x00 : 0x00);
	n6_write_register(ViPipe, 0x64 + ch, ntpal ? 0x01 : 0x01);
	n6_write_register(ViPipe, 0x81 + ch, ntpal ? 0x03 : 0x02);
	n6_write_register(ViPipe, 0x85 + ch, 0x00);
	n6_write_register(ViPipe, 0x89 + ch, 0x10);
	n6_write_register(ViPipe, 0x8e + ch, 0x00);
	n6_write_register(ViPipe, 0xa0 + ch, 0x05);

	n6_write_register(ViPipe, 0xff, 0x01);
	n6_write_register(ViPipe, 0x84 + ch, 0x02);
	n6_write_register(ViPipe, 0x88 + ch, 0x00);
	n6_write_register(ViPipe, 0x8c + ch, 0x40);
	n6_write_register(ViPipe, 0xa0 + ch, 0x20);

	n6_write_register(ViPipe, 0xff, 0x05 + ch);
	n6_write_register(ViPipe, 0x01, 0x22);
	n6_write_register(ViPipe, 0x05, 0x04);
	n6_write_register(ViPipe, 0x08, 0x55);
	n6_write_register(ViPipe, 0x25, 0xdc);
	n6_write_register(ViPipe, 0x28, 0x80);
	n6_write_register(ViPipe, 0x2f, 0x00);
	n6_write_register(ViPipe, 0x30, 0xe0);
	n6_write_register(ViPipe, 0x31, 0x41);
	n6_write_register(ViPipe, 0x32, 0xa2);
	n6_write_register(ViPipe, 0x47, 0xee);
	n6_write_register(ViPipe, 0x50, 0xc6);
	n6_write_register(ViPipe, 0x57, 0x00);
	n6_write_register(ViPipe, 0x58, 0x77);
	n6_write_register(ViPipe, 0x5b, 0x41);
	n6_write_register(ViPipe, 0x5c, 0x7C);
	n6_write_register(ViPipe, 0x5f, 0x00);
	n6_write_register(ViPipe, 0x62, 0x20);
	n6_write_register(ViPipe, 0x7b, 0x11);
	n6_write_register(ViPipe, 0x7c, 0x01);
	n6_write_register(ViPipe, 0x7d, 0x80);
	n6_write_register(ViPipe, 0x80, 0x00);
	n6_write_register(ViPipe, 0x90, 0x01);
	n6_write_register(ViPipe, 0xa9, 0x00);
	n6_write_register(ViPipe, 0xb5, 0x40);
	n6_write_register(ViPipe, 0xb8, 0x39);
	n6_write_register(ViPipe, 0xb9, 0x72);
	n6_write_register(ViPipe, 0xd1, 0x00);
	n6_write_register(ViPipe, 0xd5, 0x80);

	n6_write_register(ViPipe, 0xff, 0x09);
	n6_write_register(ViPipe, 0x96 + ch * 0x20, 0x00);
	n6_write_register(ViPipe, 0x98 + ch * 0x20, 0x00);
	n6_write_register(ViPipe, 0x9e + ch * 0x20, 0x00);

	n6_write_register(ViPipe, 0xff, _MAR_BANK_);
	val_20x01 = n6_read_register(ViPipe, 0x01);
	val_20x01 &= (~(0x03 << (ch * 2)));
	n6_write_register(ViPipe, 0x01, val_20x01);
	n6_write_register(ViPipe, 0x12 + ch*2, 0xc0);
	n6_write_register(ViPipe, 0x13 + ch*2, 0x03);

}

unsigned char n6_read_vfc(VI_PIPE ViPipe, CVI_U8 ch)
{
	CVI_U8 ch_vfc = 0xff;

	n6_write_register(ViPipe, 0xff, 0x05 + ch);
	ch_vfc = n6_read_register(ViPipe, 0xf0);

	return ch_vfc;
}

void n6_device_auto_detect(VI_PIPE ViPipe)
{
	CVI_U8 ch_vfc[16] = {0xff, 0xff, 0xff, 0xff, 0xff};
	CVI_U8 val_13x70, val_13x71;
	int check_cnt = 0;
	CVI_U8 ch;

	CVI_TRACE_SNS(CVI_DBG_INFO, "%s auto detection routine\n", __func__);

	n6_write_register(ViPipe, 0xFF, 0x13);
	n6_write_register(ViPipe, 0x30, 0x7f);
	n6_write_register(ViPipe, 0x70, 0xf0);
	n6_write_register(ViPipe, 0xFF, 0x00);
	n6_write_register(ViPipe, 0x00, 0x18);
	n6_write_register(ViPipe, 0x01, 0x18);
	n6_write_register(ViPipe, 0x02, 0x18);
	n6_write_register(ViPipe, 0x03, 0x18);
	n6_write_register(ViPipe, 0x00, 0x10);
	n6_write_register(ViPipe, 0x01, 0x10);
	n6_write_register(ViPipe, 0x02, 0x10);
	n6_write_register(ViPipe, 0x03, 0x10);
	while ((check_cnt++) < 50) {
		for (ch = 0; ch < 4; ch++) {
			ch_vfc[ch] = n6_read_vfc(ViPipe, ch);
			if (ch_vfc[ch] != 0xff)
				CVI_TRACE_SNS(CVI_DBG_DEBUG, "ch[%d] video vfc read value0 : %2x\n", ch, ch_vfc[ch]);
		}
		delay_ms(40);
	}
	for (ch = 0; ch < 4; ch++) {
		n6_write_register(ViPipe, 0xFF, 0x13);
		val_13x70 = n6_read_register(ViPipe, 0x70);
		val_13x70 |= (0x01<<ch);
		n6_write_register(ViPipe, 0x70, val_13x70);
		val_13x71 = n6_read_register(ViPipe, 0x71);
		val_13x71 |= (0x01<<ch);
		n6_write_register(ViPipe, 0x71, val_13x71);

		switch (ch_vfc[ch]) { //only check ch 0
		case 0x00:
			n6_set_chn_960h(ViPipe, ch, VFMT_NTSC);
			break;
		case 0x10:
			n6_set_chn_960h(ViPipe, ch, VFMT_PAL);
			break;
		case 0x20:
			n6_set_chn_720p(ViPipe, ch, VFMT_NTSC);
			break;
		case 0x21:
			n6_set_chn_720p(ViPipe, ch, VFMT_PAL);
			break;
		case 0x30:
			n6_set_chn_1080p(ViPipe, ch, VFMT_NTSC);
			break;
		case 0x31:
			n6_set_chn_1080p(ViPipe, ch, VFMT_PAL);
			break;
		case 0xa0:
			n6_set_chn_960p(ViPipe, ch, VFMT_NTSC);
			break;
		case 0xa1:
			n6_set_chn_960p(ViPipe, ch, VFMT_PAL);
			break;
		default:
			CVI_TRACE_SNS(CVI_DBG_DEBUG, "ch_vfc not valid,set to default format\n");
			n6_write_register(ViPipe, 0xFF, 0x13);
			val_13x70 = n6_read_register(ViPipe, 0x70);
			val_13x70 &= (~(0x01<<ch));
			n6_write_register(ViPipe, 0x70, val_13x70);
			n6_set_chn_1080p(ViPipe, ch, VFMT_NTSC);
			break;
		}
	}
}

void n6_init(VI_PIPE ViPipe)
{
	n6_i2c_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_DEBUG, "Loading Nextchip N6 sensor\n");

	//check sensor chip id
	n6_write_register(ViPipe, 0xFF, 0x0);
	if (n6_read_register(ViPipe, 0xf4) != 0xd3) {
		CVI_TRACE_SNS(CVI_DBG_DEBUG, "read N6 chip id fail\n");
		return;
	}

	n6_common_setting(ViPipe);

	if (adet == 0) {   //manual mode
		CVI_TRACE_SNS(CVI_DBG_DEBUG, "manual mode 2\n");
		n6_set_chn_1080p(ViPipe, 0, VFMT_PAL);
		n6_set_chn_1080p(ViPipe, 1, VFMT_PAL);
		n6_set_chn_1080p(ViPipe, 2, VFMT_PAL);
		n6_set_chn_1080p(ViPipe, 3, VFMT_PAL);
	} else {
		CVI_TRACE_SNS(CVI_DBG_DEBUG, "auto detect mode\n");
		n6_device_auto_detect(ViPipe);
	}

#if N6_TEST_PATTERN
	//test_mode
	n6_write_register(ViPipe, 0xFF, 0x5);
	n6_write_register(ViPipe, 0x2c, 0x8);
	n6_write_register(ViPipe, 0xFF, 0x6);
	n6_write_register(ViPipe, 0x2c, 0x8);
	n6_write_register(ViPipe, 0xFF, 0x7);
	n6_write_register(ViPipe, 0x2c, 0x8);
	n6_write_register(ViPipe, 0xFF, 0x8);
	n6_write_register(ViPipe, 0x2c, 0x8);
	n6_write_register(ViPipe, 0xFF, 0x0);
	n6_write_register(ViPipe, 0x78, 0xaa);
	n6_write_register(ViPipe, 0x79, 0xaa);
#endif

	n6_write_register(ViPipe, 0xff, 0x23);  //continuous clock
	n6_write_register(ViPipe, 0xe1, 0x02);
	n6_write_register(ViPipe, 0xff, 0x33);  //continuous clock
	n6_write_register(ViPipe, 0xe1, 0x02);

	//wait for the sensor signal to stabilize
	delay_ms(300);
}

