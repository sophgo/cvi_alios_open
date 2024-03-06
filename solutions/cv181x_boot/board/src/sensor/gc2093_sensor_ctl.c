#include <unistd.h>
#include <stdint.h>
#include <drv/tick.h>

#include "sensor.h"

static void gc2093_linear_1080p30_init(uint8_t ViPipe);

uint8_t gc2093_i2c_addr = 0x37;//0x6e
uint8_t gc2093_i2c_dev = 2;
const uint32_t gc2093_addr_byte = 2;
const uint32_t gc2093_data_byte = 1;

static int gc2093_i2c_init(uint8_t ViPipe)
{
	uint8_t i2c_id = (uint8_t)gc2093_i2c_dev;

	return sensor_i2c_init(i2c_id);
}

static int gc2093_i2c_exit(uint8_t ViPipe)
{
	uint8_t i2c_id = (uint8_t)gc2093_i2c_dev;

	return sensor_i2c_exit(i2c_id);
}

static int gc2093_read_register(uint8_t ViPipe, int addr)
{
	uint8_t i2c_id = (uint8_t)gc2093_i2c_dev;

	return sensor_i2c_read(i2c_id, gc2093_i2c_addr, (uint32_t)addr, gc2093_addr_byte, gc2093_data_byte);
}

static int gc2093_write_register(uint8_t ViPipe, int addr, int data)
{
	uint8_t i2c_id = (uint8_t)gc2093_i2c_dev;

	return sensor_i2c_write(i2c_id, gc2093_i2c_addr, (uint32_t)addr, gc2093_addr_byte,
				(uint32_t)data, gc2093_data_byte);
}


#define GC2093_CHIP_ID_ADDR_H	0x03f0
#define GC2093_CHIP_ID_ADDR_L	0x03f1
#define GC2093_CHIP_ID		0x2093

int gc2093_probe(uint8_t i2c_addr, uint8_t i2c_dev)
{
	int nVal;
	int nVal2;
	uint8_t ViPipe = 0;

	gc2093_i2c_addr = i2c_addr;
	gc2093_i2c_dev = i2c_dev;

	if (gc2093_i2c_init(ViPipe) != 0)
		return -1;

	nVal  = gc2093_read_register(ViPipe, GC2093_CHIP_ID_ADDR_H);
	nVal2 = gc2093_read_register(ViPipe, GC2093_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		printf("read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC2093_CHIP_ID) {
		printf("Sensor ID Mismatch! Use the wrong sensor??\n");
		return -1;
	}
	// printf("id: %#x, %#x\n", nVal, nVal2);

	return 0;
}

void gc2093_init()
{
	gc2093_linear_1080p30_init(0);
}

void gc2093_exit()
{
	gc2093_i2c_exit(0);
}

static void gc2093_linear_1080p30_init(uint8_t ViPipe)
{
	// printf("init setting start ##cur_ms:%d\n", csi_tick_get_ms());

	/****system****/
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0xf0);
	gc2093_write_register(ViPipe, 0x03fe, 0x00);
	gc2093_write_register(ViPipe, 0x03f2, 0x00);
	gc2093_write_register(ViPipe, 0x03f3, 0x00);
	gc2093_write_register(ViPipe, 0x03f4, 0x36);
	gc2093_write_register(ViPipe, 0x03f5, 0xc0);
	gc2093_write_register(ViPipe, 0x03f6, 0x0B);
	gc2093_write_register(ViPipe, 0x03f7, 0x11);
	gc2093_write_register(ViPipe, 0x03f8, 0x2A);
	gc2093_write_register(ViPipe, 0x03f9, 0x42);
	gc2093_write_register(ViPipe, 0x03fc, 0x8e);
	/****CISCTL & ANALOG*/
	gc2093_write_register(ViPipe, 0x0087, 0x18);
	gc2093_write_register(ViPipe, 0x00ee, 0x30);
	gc2093_write_register(ViPipe, 0x00d0, 0xbf);
	gc2093_write_register(ViPipe, 0x01a0, 0x00);
	gc2093_write_register(ViPipe, 0x01a4, 0x40);
	gc2093_write_register(ViPipe, 0x01a5, 0x40);
	gc2093_write_register(ViPipe, 0x01a6, 0x40);
	gc2093_write_register(ViPipe, 0x01af, 0x09);
	// gc2093_write_register(ViPipe, 0x0003, 0x04);
	// gc2093_write_register(ViPipe, 0x0004, 0x65);
	gc2093_write_register(ViPipe, 0x0003, 0x04);
	gc2093_write_register(ViPipe, 0x0004, 0x1b);

	gc2093_write_register(ViPipe, 0x0005, 0x05);
	gc2093_write_register(ViPipe, 0x0006, 0x78);
	gc2093_write_register(ViPipe, 0x0007, 0x00);
	gc2093_write_register(ViPipe, 0x0008, 0x11);
	gc2093_write_register(ViPipe, 0x0009, 0x00);
	gc2093_write_register(ViPipe, 0x000a, 0x02);
	gc2093_write_register(ViPipe, 0x000b, 0x00);
	gc2093_write_register(ViPipe, 0x000c, 0x04);
	gc2093_write_register(ViPipe, 0x000d, 0x04);
	gc2093_write_register(ViPipe, 0x000e, 0x40);
	gc2093_write_register(ViPipe, 0x000f, 0x07);
	gc2093_write_register(ViPipe, 0x0010, 0x8c);
	gc2093_write_register(ViPipe, 0x0013, 0x15);
	gc2093_write_register(ViPipe, 0x0019, 0x0c);
	gc2093_write_register(ViPipe, 0x0041, 0x04);
	gc2093_write_register(ViPipe, 0x0042, 0x65);
	gc2093_write_register(ViPipe, 0x0053, 0x60);
	gc2093_write_register(ViPipe, 0x008d, 0x92);
	gc2093_write_register(ViPipe, 0x0090, 0x00);
	gc2093_write_register(ViPipe, 0x00c7, 0xe1);
	gc2093_write_register(ViPipe, 0x001b, 0x73);
	gc2093_write_register(ViPipe, 0x0028, 0x0d);
	gc2093_write_register(ViPipe, 0x0029, 0x40);
	gc2093_write_register(ViPipe, 0x002b, 0x04);
	gc2093_write_register(ViPipe, 0x002e, 0x23);
	gc2093_write_register(ViPipe, 0x0037, 0x03);
	gc2093_write_register(ViPipe, 0x0043, 0x04);
	gc2093_write_register(ViPipe, 0x0044, 0x30);
	gc2093_write_register(ViPipe, 0x004a, 0x01);
	gc2093_write_register(ViPipe, 0x004b, 0x28);
	gc2093_write_register(ViPipe, 0x0055, 0x30);
	gc2093_write_register(ViPipe, 0x0066, 0x3f);
	gc2093_write_register(ViPipe, 0x0068, 0x3f);
	gc2093_write_register(ViPipe, 0x006b, 0x44);
	gc2093_write_register(ViPipe, 0x0077, 0x00);
	gc2093_write_register(ViPipe, 0x0078, 0x20);
	gc2093_write_register(ViPipe, 0x007c, 0xa1);
	gc2093_write_register(ViPipe, 0x00ce, 0x7c);
	gc2093_write_register(ViPipe, 0x00d3, 0xd4);
	gc2093_write_register(ViPipe, 0x00e6, 0x50);
	/*gain*/
	gc2093_write_register(ViPipe, 0x00b6, 0xc0);
	gc2093_write_register(ViPipe, 0x00b0, 0x68);
	gc2093_write_register(ViPipe, 0x00b3, 0x00);
	gc2093_write_register(ViPipe, 0x00b8, 0x01);
	gc2093_write_register(ViPipe, 0x00b9, 0x00);
	gc2093_write_register(ViPipe, 0x00b1, 0x01);
	gc2093_write_register(ViPipe, 0x00b2, 0x00);
	/*isp*/
	gc2093_write_register(ViPipe, 0x0101, 0x0c);
	gc2093_write_register(ViPipe, 0x0102, 0x89);
	gc2093_write_register(ViPipe, 0x0104, 0x01);
	gc2093_write_register(ViPipe, 0x0107, 0xa6);
	gc2093_write_register(ViPipe, 0x0108, 0xa9);
	gc2093_write_register(ViPipe, 0x0109, 0xa8);
	gc2093_write_register(ViPipe, 0x010a, 0xa7);
	gc2093_write_register(ViPipe, 0x010b, 0xff);
	gc2093_write_register(ViPipe, 0x010c, 0xff);
	gc2093_write_register(ViPipe, 0x010f, 0x00);
	gc2093_write_register(ViPipe, 0x0158, 0x00);
	gc2093_write_register(ViPipe, 0x0428, 0x86);
	gc2093_write_register(ViPipe, 0x0429, 0x86);
	gc2093_write_register(ViPipe, 0x042a, 0x86);
	gc2093_write_register(ViPipe, 0x042b, 0x68);
	gc2093_write_register(ViPipe, 0x042c, 0x68);
	gc2093_write_register(ViPipe, 0x042d, 0x68);
	gc2093_write_register(ViPipe, 0x042e, 0x68);
	gc2093_write_register(ViPipe, 0x042f, 0x68);
	gc2093_write_register(ViPipe, 0x0430, 0x4f);
	gc2093_write_register(ViPipe, 0x0431, 0x68);
	gc2093_write_register(ViPipe, 0x0432, 0x67);
	gc2093_write_register(ViPipe, 0x0433, 0x66);
	gc2093_write_register(ViPipe, 0x0434, 0x66);
	gc2093_write_register(ViPipe, 0x0435, 0x66);
	gc2093_write_register(ViPipe, 0x0436, 0x66);
	gc2093_write_register(ViPipe, 0x0437, 0x66);
	gc2093_write_register(ViPipe, 0x0438, 0x62);
	gc2093_write_register(ViPipe, 0x0439, 0x62);
	gc2093_write_register(ViPipe, 0x043a, 0x62);
	gc2093_write_register(ViPipe, 0x043b, 0x62);
	gc2093_write_register(ViPipe, 0x043c, 0x62);
	gc2093_write_register(ViPipe, 0x043d, 0x62);
	gc2093_write_register(ViPipe, 0x043e, 0x62);
	gc2093_write_register(ViPipe, 0x043f, 0x62);
	 /*dark sun*/
	gc2093_write_register(ViPipe, 0x0123, 0x08);
	gc2093_write_register(ViPipe, 0x0123, 0x00);
	gc2093_write_register(ViPipe, 0x0120, 0x01);
	gc2093_write_register(ViPipe, 0x0121, 0x04);
	gc2093_write_register(ViPipe, 0x0122, 0x65);
	gc2093_write_register(ViPipe, 0x0124, 0x03);
	gc2093_write_register(ViPipe, 0x0125, 0xff);
	gc2093_write_register(ViPipe, 0x001a, 0x8c);
	gc2093_write_register(ViPipe, 0x00c6, 0xe0);
	/*blk*/
	gc2093_write_register(ViPipe, 0x0026, 0x30);
	gc2093_write_register(ViPipe, 0x0142, 0x00);
	gc2093_write_register(ViPipe, 0x0149, 0x1e);
	gc2093_write_register(ViPipe, 0x014a, 0x0f);
	gc2093_write_register(ViPipe, 0x014b, 0x00);
	// gc2093_write_register(ViPipe, 0x0155, 0x07);
	gc2093_write_register(ViPipe, 0x0155, 0x08);

	gc2093_write_register(ViPipe, 0x0414, 0x78);
	gc2093_write_register(ViPipe, 0x0415, 0x78);
	gc2093_write_register(ViPipe, 0x0416, 0x78);
	gc2093_write_register(ViPipe, 0x0417, 0x78);
	gc2093_write_register(ViPipe, 0x04e0, 0x18);
	/*window*/
	gc2093_write_register(ViPipe, 0x0192, 0x02);
	gc2093_write_register(ViPipe, 0x0194, 0x03);
	gc2093_write_register(ViPipe, 0x0195, 0x04);
	gc2093_write_register(ViPipe, 0x0196, 0x38);
	gc2093_write_register(ViPipe, 0x0197, 0x07);
	gc2093_write_register(ViPipe, 0x0198, 0x80);
	/****DVP & MIPI****/
	gc2093_write_register(ViPipe, 0x019a, 0x06);
	gc2093_write_register(ViPipe, 0x007b, 0x2a);
	gc2093_write_register(ViPipe, 0x0023, 0x2d);
	gc2093_write_register(ViPipe, 0x0201, 0x27);
	gc2093_write_register(ViPipe, 0x0202, 0x56);
	gc2093_write_register(ViPipe, 0x0203, 0xb6);
	gc2093_write_register(ViPipe, 0x0212, 0x80);
	gc2093_write_register(ViPipe, 0x0213, 0x07);
	gc2093_write_register(ViPipe, 0x0215, 0x10);
	gc2093_write_register(ViPipe, 0x003e, 0x91);

	// printf("init setting done ##cur_ms:%d\n", csi_tick_get_ms());

	// printf("ViPipe:%d,===GC2093 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}
