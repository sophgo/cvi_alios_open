
#include <unistd.h>
#include <stdint.h>
#include <drv/tick.h>

#include "sensor.h"

#define GC02M1_MULTI_CHIP_ID_ADDR_H	0xf0
#define GC02M1_MULTI_CHIP_ID_ADDR_L	0xf1
#define GC02M1_MULTI_CHIP_ID		0x02e0

static void gc02m1_multi_linear_1200p30_init(uint8_t ViPipe);

uint8_t gc02m1_multi_i2c_addr = 0x10;
uint8_t gc02m1_multi_i2c_dev = 1;
const uint32_t gc02m1_multi_addr_byte = 1;
const uint32_t gc02m1_multi_data_byte = 1;

int gc02m1_multi_i2c_init(uint8_t ViPipe)
{
	uint8_t i2c_id = gc02m1_multi_i2c_dev;

	return sensor_i2c_init(i2c_id);
}

int gc02m1_multi_i2c_exit(uint8_t ViPipe)
{
	uint8_t i2c_id = gc02m1_multi_i2c_dev;

	return sensor_i2c_exit(i2c_id);
}

int gc02m1_multi_read_register(uint8_t ViPipe, int addr)
{
	uint8_t i2c_id = gc02m1_multi_i2c_dev;

    return sensor_i2c_read(i2c_id, gc02m1_multi_i2c_addr, (uint32_t)addr, gc02m1_multi_addr_byte, gc02m1_multi_data_byte);
}

int gc02m1_multi_write_register(uint8_t ViPipe, int addr, int data)
{
	uint8_t i2c_id = gc02m1_multi_i2c_dev;

    return sensor_i2c_write(i2c_id, gc02m1_multi_i2c_addr, (uint32_t)addr, gc02m1_multi_addr_byte,
            (uint32_t)data, gc02m1_multi_data_byte);
}

int gc02m1_multi_probe(uint8_t i2c_addr, uint8_t i2c_dev)
{
	int nVal;
	int nVal2;
	uint8_t ViPipe = 1;

	gc02m1_multi_i2c_addr = i2c_addr;
	gc02m1_multi_i2c_dev = i2c_dev;

	if (gc02m1_multi_i2c_init(ViPipe) != 0)
		return -1;
	nVal = gc02m1_multi_read_register(ViPipe, GC02M1_MULTI_CHIP_ID_ADDR_H);
	nVal2 = gc02m1_multi_read_register(ViPipe, GC02M1_MULTI_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0)
	{
		printf("read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC02M1_MULTI_CHIP_ID)
	{
		printf("Sensor ID Mismatch! Use the wrong sensor??\n");
		return -1;
	}
	return 0;
}

void gc02m1_multi_init(uint8_t ViPipe)
{
	gc02m1_multi_linear_1200p30_init(ViPipe);
}

void gc02m1_multi_exit(uint8_t ViPipe)
{
	gc02m1_multi_i2c_exit(ViPipe);
}

static void gc02m1_multi_linear_1200p30_init(uint8_t ViPipe)
{
	/*system*/
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xf4, 0x41);
	gc02m1_multi_write_register(ViPipe, 0xf5, 0xc0);
	gc02m1_multi_write_register(ViPipe, 0xf6, 0x44);
	gc02m1_multi_write_register(ViPipe, 0xf8, 0x32);
	gc02m1_multi_write_register(ViPipe, 0xf9, 0x82);
	gc02m1_multi_write_register(ViPipe, 0xfa, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xfd, 0x80);
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x81);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x03);
	gc02m1_multi_write_register(ViPipe, 0x01, 0x0b);
	gc02m1_multi_write_register(ViPipe, 0xf7, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x80);
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x80);
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x80);
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x8e);
	/*CISCTL*/
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x87, 0x09);
	gc02m1_multi_write_register(ViPipe, 0xee, 0x72);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_multi_write_register(ViPipe, 0x8c, 0x90);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x90, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x03, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x04, 0x7d);
	gc02m1_multi_write_register(ViPipe, 0x41, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x42, 0xf4);
	gc02m1_multi_write_register(ViPipe, 0x05, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x06, 0x48);
	gc02m1_multi_write_register(ViPipe, 0x07, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x08, 0x18);
	gc02m1_multi_write_register(ViPipe, 0x9d, 0x18);
	gc02m1_multi_write_register(ViPipe, 0x09, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x0a, 0x02);
	gc02m1_multi_write_register(ViPipe, 0x0d, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x0e, 0xbc);
	gc02m1_multi_write_register(ViPipe, 0x17, 0x80);
	gc02m1_multi_write_register(ViPipe, 0x19, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x24, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x56, 0x20);
	gc02m1_multi_write_register(ViPipe, 0x5b, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x5e, 0x01);
	/*analog Register width*/
	gc02m1_multi_write_register(ViPipe, 0x21, 0x3c);
	gc02m1_multi_write_register(ViPipe, 0x44, 0x20);
	gc02m1_multi_write_register(ViPipe, 0xcc, 0x01);
	/*analog mode*/
	gc02m1_multi_write_register(ViPipe, 0x1a, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x1f, 0x11);
	gc02m1_multi_write_register(ViPipe, 0x27, 0x30);
	gc02m1_multi_write_register(ViPipe, 0x2b, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x33, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x53, 0x90);
	gc02m1_multi_write_register(ViPipe, 0xe6, 0x50);
	/*analog voltage*/
	gc02m1_multi_write_register(ViPipe, 0x39, 0x07);
	gc02m1_multi_write_register(ViPipe, 0x43, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x46, 0x2a);
	gc02m1_multi_write_register(ViPipe, 0x7c, 0xa0);
	gc02m1_multi_write_register(ViPipe, 0xd0, 0xbe);
	gc02m1_multi_write_register(ViPipe, 0xd1, 0x60);
	gc02m1_multi_write_register(ViPipe, 0xd2, 0x40);
	gc02m1_multi_write_register(ViPipe, 0xd3, 0xf3);
	gc02m1_multi_write_register(ViPipe, 0xde, 0x1d);
	/*analog current*/
	gc02m1_multi_write_register(ViPipe, 0xcd, 0x05);
	gc02m1_multi_write_register(ViPipe, 0xce, 0x6f);
	/*CISCTL RESET*/
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x88);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x10);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x8e);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x88);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x10);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xfc, 0x8e);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x04);
	gc02m1_multi_write_register(ViPipe, 0xe0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	/*ISP*/
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_multi_write_register(ViPipe, 0x53, 0x44);
	gc02m1_multi_write_register(ViPipe, 0x87, 0x50);
	gc02m1_multi_write_register(ViPipe, 0x89, 0x03);
	/*Gain*/
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xb0, 0x74);
	gc02m1_multi_write_register(ViPipe, 0xb1, 0x04);
	gc02m1_multi_write_register(ViPipe, 0xb2, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xb6, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x04);
	gc02m1_multi_write_register(ViPipe, 0xd8, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x40);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x60);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xc0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x2a);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x80);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x40);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xa0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x90);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x19);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xc0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xD0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x2F);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x90);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x39);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x20);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x04);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x20);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x0f);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x40);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x1a);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x60);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x20);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x25);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x80);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xa0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x2c);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xa0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x32);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xc0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x20);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x38);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xe0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x01);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x60);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x3c);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x00);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x02);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0xa0);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x40);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x80);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x02);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x18);
	gc02m1_multi_write_register(ViPipe, 0xc0, 0x5c);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x9f, 0x10);
	/*BLK*/
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x26, 0x20);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_multi_write_register(ViPipe, 0x40, 0x22);
	gc02m1_multi_write_register(ViPipe, 0x46, 0x7f);
	gc02m1_multi_write_register(ViPipe, 0x49, 0x0f);
	gc02m1_multi_write_register(ViPipe, 0x4a, 0xf0);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x14, 0x80);
	gc02m1_multi_write_register(ViPipe, 0x15, 0x80);
	gc02m1_multi_write_register(ViPipe, 0x16, 0x80);
	gc02m1_multi_write_register(ViPipe, 0x17, 0x80);
	/*ant _blooming*/
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_multi_write_register(ViPipe, 0x41, 0x20);
	gc02m1_multi_write_register(ViPipe, 0x4c, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x4d, 0x0c);
	gc02m1_multi_write_register(ViPipe, 0x44, 0x08);
	gc02m1_multi_write_register(ViPipe, 0x48, 0x03);
	/*Window 1600X1200*/
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_multi_write_register(ViPipe, 0x90, 0x01);
	gc02m1_multi_write_register(ViPipe, 0x91, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x92, 0x06);
	gc02m1_multi_write_register(ViPipe, 0x93, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x94, 0x06);
	gc02m1_multi_write_register(ViPipe, 0x95, 0x04);
	gc02m1_multi_write_register(ViPipe, 0x96, 0xb0);
	gc02m1_multi_write_register(ViPipe, 0x97, 0x06);
	gc02m1_multi_write_register(ViPipe, 0x98, 0x40);
	/*mipi*/
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x03);
	gc02m1_multi_write_register(ViPipe, 0x01, 0x23);
	gc02m1_multi_write_register(ViPipe, 0x03, 0xce);
	gc02m1_multi_write_register(ViPipe, 0x04, 0x48);
	gc02m1_multi_write_register(ViPipe, 0x15, 0x00);
	gc02m1_multi_write_register(ViPipe, 0x21, 0x10);
	gc02m1_multi_write_register(ViPipe, 0x22, 0x05);
	gc02m1_multi_write_register(ViPipe, 0x23, 0x20);
	gc02m1_multi_write_register(ViPipe, 0x25, 0x20);
	gc02m1_multi_write_register(ViPipe, 0x26, 0x08);
	gc02m1_multi_write_register(ViPipe, 0x29, 0x06);
	gc02m1_multi_write_register(ViPipe, 0x2a, 0x0a);
	gc02m1_multi_write_register(ViPipe, 0x2b, 0x08);
	/*out*/
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x01);
	gc02m1_multi_write_register(ViPipe, 0x8c, 0x10);
	gc02m1_multi_write_register(ViPipe, 0xfe, 0x00);

	printf("ViPipe:%d,===GC02M1_MULTI 1200P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}