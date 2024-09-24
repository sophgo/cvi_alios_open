#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "gc0308_cmos_ex.h"

static void gc0308_linear_480p30_init(VI_PIPE ViPipe);

CVI_U8 gc0308_i2c_addr = 0x21;
const CVI_U32 gc0308_addr_byte = 1;
const CVI_U32 gc0308_data_byte = 1;

int gc0308_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc0308_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
	}

int gc0308_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc0308_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int gc0308_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc0308_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc0308_i2c_addr, (CVI_U32)addr, gc0308_addr_byte, gc0308_data_byte);
}

int gc0308_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc0308_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc0308_i2c_addr, (CVI_U32)addr, gc0308_addr_byte,
		(CVI_U32)data, gc0308_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

#define GC0308_CHIP_ID_ADDR	0x00
#define GC0308_CHIP_ID		0x9b

int gc0308_probe(VI_PIPE ViPipe)
{
	int nVal;

	usleep(50);
	if (gc0308_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc0308_read_register(ViPipe, GC0308_CHIP_ID_ADDR);

	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((nVal & 0xFF) != GC0308_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor?? sensor id = %x\n", nVal);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc0308_init(VI_PIPE ViPipe)
{
	gc0308_i2c_init(ViPipe);

	gc0308_linear_480p30_init(ViPipe);

	g_pastGc0308[ViPipe]->bInit = CVI_TRUE;
}

void gc0308_exit(VI_PIPE ViPipe)
{
	gc0308_i2c_exit(ViPipe);
}

static void gc0308_linear_480p30_init(VI_PIPE ViPipe)
{
	gc0308_write_register(ViPipe, 0xfe , 0x80);
	gc0308_write_register(ViPipe, 0xfe , 0x00);   // set page0
	gc0308_write_register(ViPipe, 0xd2 , 0x10);   // close AEC
	gc0308_write_register(ViPipe, 0x22 , 0x55);   // close AWB
	gc0308_write_register(ViPipe, 0x03 , 0x01);
	gc0308_write_register(ViPipe, 0x04 , 0x2c);
	gc0308_write_register(ViPipe, 0x5a , 0x56);
	gc0308_write_register(ViPipe, 0x5b , 0x40);
	gc0308_write_register(ViPipe, 0x5c , 0x4a);
	gc0308_write_register(ViPipe, 0x22 , 0x57);   // Open AWB
	gc0308_write_register(ViPipe, 0x01 , 0x6a);
	gc0308_write_register(ViPipe, 0x02 , 0x0c);
	gc0308_write_register(ViPipe, 0x0f , 0x00);
	gc0308_write_register(ViPipe, 0xe2 , 0x00);
	gc0308_write_register(ViPipe, 0xe3 , 0x96);
	gc0308_write_register(ViPipe, 0xe4 , 0x01);
	gc0308_write_register(ViPipe, 0xe5 , 0xc2);
	gc0308_write_register(ViPipe, 0xe6 , 0x01);
	gc0308_write_register(ViPipe, 0xe7 , 0xc2);
	gc0308_write_register(ViPipe, 0xe8 , 0x01);
	gc0308_write_register(ViPipe, 0xe9 , 0xc2);
	gc0308_write_register(ViPipe, 0xea , 0x01);
	gc0308_write_register(ViPipe, 0xeb , 0xc2);
	gc0308_write_register(ViPipe, 0xec , 0x00);
	gc0308_write_register(ViPipe, 0x05 , 0x00);   //window
	gc0308_write_register(ViPipe, 0x06 , 0x00);
	gc0308_write_register(ViPipe, 0x07 , 0x00);
	gc0308_write_register(ViPipe, 0x08 , 0x00);
	gc0308_write_register(ViPipe, 0x09 , 0x01);
	gc0308_write_register(ViPipe, 0x0a , 0xe8);
	gc0308_write_register(ViPipe, 0x0b , 0x02);
	gc0308_write_register(ViPipe, 0x0c , 0x88);
	gc0308_write_register(ViPipe, 0x46 , 0x80); //crop
	gc0308_write_register(ViPipe, 0x47 , 0x00);
	gc0308_write_register(ViPipe, 0x48 , 0x00);
	gc0308_write_register(ViPipe, 0x49 , 0x01);
	gc0308_write_register(ViPipe, 0x4a , 0xe0);
	gc0308_write_register(ViPipe, 0x4b , 0x02);
	gc0308_write_register(ViPipe, 0x4c , 0x80);
	gc0308_write_register(ViPipe, 0x0d , 0x02);
	gc0308_write_register(ViPipe, 0x0e , 0x02);
	gc0308_write_register(ViPipe, 0x10 , 0x22);
	gc0308_write_register(ViPipe, 0x11 , 0xfd);
	gc0308_write_register(ViPipe, 0x12 , 0x2a);
	gc0308_write_register(ViPipe, 0x13 , 0x00);
	gc0308_write_register(ViPipe, 0x15 , 0x0a);
	gc0308_write_register(ViPipe, 0x16 , 0x05);
	gc0308_write_register(ViPipe, 0x17 , 0x01);
	gc0308_write_register(ViPipe, 0x18 , 0x44);
	gc0308_write_register(ViPipe, 0x19 , 0x44);
	gc0308_write_register(ViPipe, 0x1a , 0x1e);
	gc0308_write_register(ViPipe, 0x1b , 0x00);
	gc0308_write_register(ViPipe, 0x1c , 0xc1);
	gc0308_write_register(ViPipe, 0x1d , 0x08);
	gc0308_write_register(ViPipe, 0x1e , 0x60);
	gc0308_write_register(ViPipe, 0x1f , 0x16);
	gc0308_write_register(ViPipe, 0x20 , 0xff);
	gc0308_write_register(ViPipe, 0x21 , 0xf8);
	gc0308_write_register(ViPipe, 0x22 , 0x57);
	gc0308_write_register(ViPipe, 0x24 , 0xa0);
	gc0308_write_register(ViPipe, 0x25 , 0x0f);
	gc0308_write_register(ViPipe, 0x26 , 0x02);
	gc0308_write_register(ViPipe, 0x2f , 0x01);
	gc0308_write_register(ViPipe, 0x30 , 0xf7);
	gc0308_write_register(ViPipe, 0x31 , 0x50);
	gc0308_write_register(ViPipe, 0x32 , 0x00);
	gc0308_write_register(ViPipe, 0x39 , 0x04);
	gc0308_write_register(ViPipe, 0x3a , 0x18);
	gc0308_write_register(ViPipe, 0x3b , 0x20);
	gc0308_write_register(ViPipe, 0x3c , 0x00);
	gc0308_write_register(ViPipe, 0x3d , 0x00);
	gc0308_write_register(ViPipe, 0x3e , 0x00);
	gc0308_write_register(ViPipe, 0x3f , 0x00);
	gc0308_write_register(ViPipe, 0x50 , 0x10);
	gc0308_write_register(ViPipe, 0x53 , 0x82);
	gc0308_write_register(ViPipe, 0x54 , 0x80);
	gc0308_write_register(ViPipe, 0x55 , 0x80);
	gc0308_write_register(ViPipe, 0x56 , 0x82);
	gc0308_write_register(ViPipe, 0x8b , 0x40);
	gc0308_write_register(ViPipe, 0x8c , 0x40);
	gc0308_write_register(ViPipe, 0x8d , 0x40);
	gc0308_write_register(ViPipe, 0x8e , 0x2e);
	gc0308_write_register(ViPipe, 0x8f , 0x2e);
	gc0308_write_register(ViPipe, 0x90 , 0x2e);
	gc0308_write_register(ViPipe, 0x91 , 0x3c);
	gc0308_write_register(ViPipe, 0x92 , 0x50);
	gc0308_write_register(ViPipe, 0x5d , 0x12);
	gc0308_write_register(ViPipe, 0x5e , 0x1a);
	gc0308_write_register(ViPipe, 0x5f , 0x24);
	gc0308_write_register(ViPipe, 0x60 , 0x07);
	gc0308_write_register(ViPipe, 0x61 , 0x15);
	gc0308_write_register(ViPipe, 0x62 , 0x08);
	gc0308_write_register(ViPipe, 0x64 , 0x03);
	gc0308_write_register(ViPipe, 0x66 , 0xe8);
	gc0308_write_register(ViPipe, 0x67 , 0x86);
	gc0308_write_register(ViPipe, 0x68 , 0xa2);
	gc0308_write_register(ViPipe, 0x69 , 0x18);
	gc0308_write_register(ViPipe, 0x6a , 0x0f);
	gc0308_write_register(ViPipe, 0x6b , 0x00);
	gc0308_write_register(ViPipe, 0x6c , 0x5f);
	gc0308_write_register(ViPipe, 0x6d , 0x8f);
	gc0308_write_register(ViPipe, 0x6e , 0x55);
	gc0308_write_register(ViPipe, 0x6f , 0x38);
	gc0308_write_register(ViPipe, 0x70 , 0x15);
	gc0308_write_register(ViPipe, 0x71 , 0x33);
	gc0308_write_register(ViPipe, 0x72 , 0xdc);
	gc0308_write_register(ViPipe, 0x73 , 0x80);
	gc0308_write_register(ViPipe, 0x74 , 0x02);
	gc0308_write_register(ViPipe, 0x75 , 0x3f);
	gc0308_write_register(ViPipe, 0x76 , 0x02);
	gc0308_write_register(ViPipe, 0x76 , 0x02);
	gc0308_write_register(ViPipe, 0x77 , 0x36);
	gc0308_write_register(ViPipe, 0x78 , 0x88);
	gc0308_write_register(ViPipe, 0x79 , 0x81);
	gc0308_write_register(ViPipe, 0x7a , 0x81);
	gc0308_write_register(ViPipe, 0x7b , 0x22);
	gc0308_write_register(ViPipe, 0x7c , 0xff);
	gc0308_write_register(ViPipe, 0x93 , 0x48);
	gc0308_write_register(ViPipe, 0x94 , 0x00);
	gc0308_write_register(ViPipe, 0x95 , 0x05);
	gc0308_write_register(ViPipe, 0x96 , 0xe8);
	gc0308_write_register(ViPipe, 0x97 , 0x40);
	gc0308_write_register(ViPipe, 0x98 , 0xf0);
	gc0308_write_register(ViPipe, 0xb1 , 0x38);
	gc0308_write_register(ViPipe, 0xb2 , 0x38);
	gc0308_write_register(ViPipe, 0xbd , 0x38);
	gc0308_write_register(ViPipe, 0xbe , 0x36);
	gc0308_write_register(ViPipe, 0xd0 , 0xc9);
	gc0308_write_register(ViPipe, 0xd1 , 0x10);
	gc0308_write_register(ViPipe, 0xd3 , 0x80);
	gc0308_write_register(ViPipe, 0xd5 , 0xf2);
	gc0308_write_register(ViPipe, 0xd6 , 0x16);
	gc0308_write_register(ViPipe, 0xdb , 0x92);
	gc0308_write_register(ViPipe, 0xdc , 0xa5);
	gc0308_write_register(ViPipe, 0xdf , 0x23);
	gc0308_write_register(ViPipe, 0xd9 , 0x00);
	gc0308_write_register(ViPipe, 0xda , 0x00);
	gc0308_write_register(ViPipe, 0xe0 , 0x09);
	gc0308_write_register(ViPipe, 0xed , 0x04);
	gc0308_write_register(ViPipe, 0xee , 0xa0);
	gc0308_write_register(ViPipe, 0xef , 0x40);
	gc0308_write_register(ViPipe, 0x80 , 0x03);
	gc0308_write_register(ViPipe, 0x80 , 0x03);
	gc0308_write_register(ViPipe, 0x9F , 0x10);
	gc0308_write_register(ViPipe, 0xA0 , 0x20);
	gc0308_write_register(ViPipe, 0xA1 , 0x38);
	gc0308_write_register(ViPipe, 0xA2 , 0x4E);
	gc0308_write_register(ViPipe, 0xA3 , 0x63);
	gc0308_write_register(ViPipe, 0xA4 , 0x76);
	gc0308_write_register(ViPipe, 0xA5 , 0x87);
	gc0308_write_register(ViPipe, 0xA6 , 0xA2);
	gc0308_write_register(ViPipe, 0xA7 , 0xB8);
	gc0308_write_register(ViPipe, 0xA8 , 0xCA);
	gc0308_write_register(ViPipe, 0xA9 , 0xD8);
	gc0308_write_register(ViPipe, 0xAA , 0xE3);
	gc0308_write_register(ViPipe, 0xAB , 0xEB);
	gc0308_write_register(ViPipe, 0xAC , 0xF0);
	gc0308_write_register(ViPipe, 0xAD , 0xF8);
	gc0308_write_register(ViPipe, 0xAE , 0xFD);
	gc0308_write_register(ViPipe, 0xAF , 0xFF);
	gc0308_write_register(ViPipe, 0xc0 , 0x00);
	gc0308_write_register(ViPipe, 0xc1 , 0x10);
	gc0308_write_register(ViPipe, 0xc2 , 0x1C);
	gc0308_write_register(ViPipe, 0xc3 , 0x30);
	gc0308_write_register(ViPipe, 0xc4 , 0x43);
	gc0308_write_register(ViPipe, 0xc5 , 0x54);
	gc0308_write_register(ViPipe, 0xc6 , 0x65);
	gc0308_write_register(ViPipe, 0xc7 , 0x75);
	gc0308_write_register(ViPipe, 0xc8 , 0x93);
	gc0308_write_register(ViPipe, 0xc9 , 0xB0);
	gc0308_write_register(ViPipe, 0xca , 0xCB);
	gc0308_write_register(ViPipe, 0xcb , 0xE6);
	gc0308_write_register(ViPipe, 0xcc , 0xFF);
	gc0308_write_register(ViPipe, 0xf0 , 0x02);
	gc0308_write_register(ViPipe, 0xf1 , 0x01);
	gc0308_write_register(ViPipe, 0xf2 , 0x01);
	gc0308_write_register(ViPipe, 0xf3 , 0x30);
	gc0308_write_register(ViPipe, 0xf9 , 0x9f);
	gc0308_write_register(ViPipe, 0xfa , 0x78);
	//--------------------------------------------------------------
	gc0308_write_register(ViPipe, 0xfe , 0x01);// set page1
	gc0308_write_register(ViPipe, 0x00 , 0xf5);
	gc0308_write_register(ViPipe, 0x02 , 0x1a);
	gc0308_write_register(ViPipe, 0x0a , 0xa0);
	gc0308_write_register(ViPipe, 0x0b , 0x60);
	gc0308_write_register(ViPipe, 0x0c , 0x08);
	gc0308_write_register(ViPipe, 0x0e , 0x4c);
	gc0308_write_register(ViPipe, 0x0f , 0x39);
	gc0308_write_register(ViPipe, 0x11 , 0x3f);
	gc0308_write_register(ViPipe, 0x12 , 0x72);
	gc0308_write_register(ViPipe, 0x13 , 0x13);
	gc0308_write_register(ViPipe, 0x14 , 0x42);
	gc0308_write_register(ViPipe, 0x15 , 0x43);
	gc0308_write_register(ViPipe, 0x16 , 0xc2);
	gc0308_write_register(ViPipe, 0x17 , 0xa8);
	gc0308_write_register(ViPipe, 0x18 , 0x18);
	gc0308_write_register(ViPipe, 0x19 , 0x40);
	gc0308_write_register(ViPipe, 0x1a , 0xd0);
	gc0308_write_register(ViPipe, 0x1b , 0xf5);
	gc0308_write_register(ViPipe, 0x70 , 0x40);
	gc0308_write_register(ViPipe, 0x71 , 0x58);
	gc0308_write_register(ViPipe, 0x72 , 0x30);
	gc0308_write_register(ViPipe, 0x73 , 0x48);
	gc0308_write_register(ViPipe, 0x74 , 0x20);
	gc0308_write_register(ViPipe, 0x75 , 0x60);
	gc0308_write_register(ViPipe, 0x77 , 0x20);
	gc0308_write_register(ViPipe, 0x78 , 0x32);
	gc0308_write_register(ViPipe, 0x30 , 0x03);
	gc0308_write_register(ViPipe, 0x31 , 0x40);
	gc0308_write_register(ViPipe, 0x32 , 0xe0);
	gc0308_write_register(ViPipe, 0x33 , 0xe0);
	gc0308_write_register(ViPipe, 0x34 , 0xe0);
	gc0308_write_register(ViPipe, 0x35 , 0xb0);
	gc0308_write_register(ViPipe, 0x36 , 0xc0);
	gc0308_write_register(ViPipe, 0x37 , 0xc0);
	gc0308_write_register(ViPipe, 0x38 , 0x04);
	gc0308_write_register(ViPipe, 0x39 , 0x09);
	gc0308_write_register(ViPipe, 0x3a , 0x12);
	gc0308_write_register(ViPipe, 0x3b , 0x1C);
	gc0308_write_register(ViPipe, 0x3c , 0x28);
	gc0308_write_register(ViPipe, 0x3d , 0x31);
	gc0308_write_register(ViPipe, 0x3e , 0x44);
	gc0308_write_register(ViPipe, 0x3f , 0x57);
	gc0308_write_register(ViPipe, 0x40 , 0x6C);
	gc0308_write_register(ViPipe, 0x41 , 0x81);
	gc0308_write_register(ViPipe, 0x42 , 0x94);
	gc0308_write_register(ViPipe, 0x43 , 0xA7);
	gc0308_write_register(ViPipe, 0x44 , 0xB8);
	gc0308_write_register(ViPipe, 0x45 , 0xD6);
	gc0308_write_register(ViPipe, 0x46 , 0xEE);
	gc0308_write_register(ViPipe, 0x47 , 0x0d);
	gc0308_write_register(ViPipe, 0xfe , 0x00); // set page0
	gc0308_write_register(ViPipe, 0xd2 , 0x90);
	//-----------Update the registers 2010/07/06-------------//
	//Registers of Page0
	gc0308_write_register(ViPipe, 0xfe , 0x00); // set page0
	gc0308_write_register(ViPipe, 0x10 , 0x26);
	gc0308_write_register(ViPipe, 0x11 , 0x0d);
	gc0308_write_register(ViPipe, 0x1a , 0x2a);
	gc0308_write_register(ViPipe, 0x1c , 0x49);
	gc0308_write_register(ViPipe, 0x1d , 0x9a);
	gc0308_write_register(ViPipe, 0x1e , 0x61);
	gc0308_write_register(ViPipe, 0x3a , 0x20);
	gc0308_write_register(ViPipe, 0x50 , 0x14);
	gc0308_write_register(ViPipe, 0x53 , 0x80);
	gc0308_write_register(ViPipe, 0x56 , 0x80);
	gc0308_write_register(ViPipe, 0x8b , 0x20); //LSC
	gc0308_write_register(ViPipe, 0x8c , 0x20);
	gc0308_write_register(ViPipe, 0x8d , 0x20);
	gc0308_write_register(ViPipe, 0x8e , 0x14);
	gc0308_write_register(ViPipe, 0x8f , 0x10);
	gc0308_write_register(ViPipe, 0x90 , 0x14);
	gc0308_write_register(ViPipe, 0x94 , 0x02);
	gc0308_write_register(ViPipe, 0x95 , 0x07);
	gc0308_write_register(ViPipe, 0x96 , 0xe0);
	gc0308_write_register(ViPipe, 0xb1 , 0x40); // YCPT
	gc0308_write_register(ViPipe, 0xb2 , 0x40);
	gc0308_write_register(ViPipe, 0xb3 , 0x40);
	gc0308_write_register(ViPipe, 0xb6 , 0xe0);
	gc0308_write_register(ViPipe, 0xd0 , 0xcb);
	gc0308_write_register(ViPipe, 0xd3 , 0x48);
	gc0308_write_register(ViPipe, 0xf2 , 0x02);
	gc0308_write_register(ViPipe, 0xf7 , 0x12);
	gc0308_write_register(ViPipe, 0xf8 , 0x0a);
	//Registers of Page1
	gc0308_write_register(ViPipe, 0xfe , 0x01);// set page1
	gc0308_write_register(ViPipe, 0x02 , 0x20);
	gc0308_write_register(ViPipe, 0x04 , 0x10);
	gc0308_write_register(ViPipe, 0x05 , 0x08);
	gc0308_write_register(ViPipe, 0x06 , 0x20);
	gc0308_write_register(ViPipe, 0x08 , 0x0a);
	gc0308_write_register(ViPipe, 0x0e , 0x44);
	gc0308_write_register(ViPipe, 0x0f , 0x32);
	gc0308_write_register(ViPipe, 0x10 , 0x41);
	gc0308_write_register(ViPipe, 0x11 , 0x37);
	gc0308_write_register(ViPipe, 0x12 , 0x22);
	gc0308_write_register(ViPipe, 0x13 , 0x19);
	gc0308_write_register(ViPipe, 0x14 , 0x44);
	gc0308_write_register(ViPipe, 0x15 , 0x44);
	gc0308_write_register(ViPipe, 0x19 , 0x50);
	gc0308_write_register(ViPipe, 0x1a , 0xd8);
	gc0308_write_register(ViPipe, 0x32 , 0x10);
	gc0308_write_register(ViPipe, 0x35 , 0x00);
	gc0308_write_register(ViPipe, 0x36 , 0x80);
	gc0308_write_register(ViPipe, 0x37 , 0x00);
	//----------Update the registers end---------//
	gc0308_write_register(ViPipe, 0xfe , 0x00);// set page0
	gc0308_write_register(ViPipe, 0x9F , 0x0E);
	gc0308_write_register(ViPipe, 0xA0 , 0x1C);
	gc0308_write_register(ViPipe, 0xA1 , 0x34);
	gc0308_write_register(ViPipe, 0xA2 , 0x48);
	gc0308_write_register(ViPipe, 0xA3 , 0x5A);
	gc0308_write_register(ViPipe, 0xA4 , 0x6B);
	gc0308_write_register(ViPipe, 0xA5 , 0x7B);
	gc0308_write_register(ViPipe, 0xA6 , 0x95);
	gc0308_write_register(ViPipe, 0xA7 , 0xAB);
	gc0308_write_register(ViPipe, 0xA8 , 0xBF);
	gc0308_write_register(ViPipe, 0xA9 , 0xCE);
	gc0308_write_register(ViPipe, 0xAA , 0xD9);
	gc0308_write_register(ViPipe, 0xAB , 0xE4);
	gc0308_write_register(ViPipe, 0xAC , 0xEC);
	gc0308_write_register(ViPipe, 0xAD , 0xF7);
	gc0308_write_register(ViPipe, 0xAE , 0xFD);
	gc0308_write_register(ViPipe, 0xAF , 0xFF);
	gc0308_write_register(ViPipe, 0x14 , 0x10);

	delay_ms(100);

	// CVI_U32 value = 0;
	// gc0308_write_register(ViPipe, 0xfe , 0x00);
	// value = gc0308_read_register(ViPipe, 0x2e);
	// printf("sensor test image value: %d\n", value);
	// gc0308_write_register(ViPipe, 0x2e , 0x02); //test image

	printf("ViPipe:%d,===GC0308 480P 30fps YUV Init OK!===\n", ViPipe);
}

