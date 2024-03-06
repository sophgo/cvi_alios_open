#include <unistd.h>
#include <stdint.h>
#include <drv/tick.h>

#include "sensor.h"

#define SC201CS_SLAVE_CHIP_ID_ADDR_H	0x3107
#define SC201CS_SLAVE_CHIP_ID_ADDR_L	0x3108
#define SC201CS_SLAVE_CHIP_ID		0xeb2c

static void sc201cs_slave_linear_1200p30_init(uint8_t ViPipe);

uint8_t sc201cs_slave_i2c_addr = 0x32;
uint8_t sc201cs_slave_i2c_dev = 1;
const uint32_t sc201cs_slave_addr_byte = 2;
const uint32_t sc201cs_slave_data_byte = 1;

int sc201cs_slave_i2c_init(uint8_t ViPipe)
{
	uint8_t i2c_id = (uint8_t)sc201cs_slave_i2c_dev;

	return sensor_i2c_init(i2c_id);
}

int sc201cs_slave_i2c_exit(uint8_t ViPipe)
{
	uint8_t i2c_id = (uint8_t)sc201cs_slave_i2c_dev;

	return sensor_i2c_exit(i2c_id);
}

int sc201cs_slave_read_register(uint8_t ViPipe, int addr)
{
	uint8_t i2c_id = (uint8_t)sc201cs_slave_i2c_dev;

	return sensor_i2c_read(i2c_id, sc201cs_slave_i2c_addr, (uint32_t)addr, sc201cs_slave_addr_byte,
		sc201cs_slave_data_byte);
}

int sc201cs_slave_write_register(uint8_t ViPipe, int addr, int data)
{
	uint8_t i2c_id = (uint8_t)sc201cs_slave_i2c_dev;

	return sensor_i2c_write(i2c_id, sc201cs_slave_i2c_addr, (uint32_t)addr, sc201cs_slave_addr_byte,
		(uint32_t)data, sc201cs_slave_data_byte);
}

int sc201cs_slave_probe(uint8_t i2c_addr, uint8_t i2c_dev)
{
	// int nVal;
	// int nVal2;
	uint8_t ViPipe = 0;

	sc201cs_slave_i2c_addr = i2c_addr;
	sc201cs_slave_i2c_dev = i2c_dev;

	if (sc201cs_slave_i2c_init(ViPipe) != 0)
		return -1;

	// nVal  = sc201cs_slave_read_register(ViPipe, SC201CS_SLAVE_CHIP_ID_ADDR_H);
	// nVal2 = sc201cs_slave_read_register(ViPipe, SC201CS_SLAVE_CHIP_ID_ADDR_L);
	// if (nVal < 0 || nVal2 < 0) {
	// 	printf("read sensor id error.\n");
	// 	return nVal;
	// }

	// if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != SC201CS_SLAVE_CHIP_ID) {
	// 	printf("Sensor ID Mismatch! Use the wrong sensor??\n");
	// 	return -1;
	// }

	return 0;
}

void sc201cs_slave_init(uint8_t ViPipe)
{
	sc201cs_slave_linear_1200p30_init(ViPipe);
}

void sc201cs_slave_exit(uint8_t ViPipe)
{
	sc201cs_slave_i2c_exit(ViPipe);
}

static void sc201cs_slave_linear_1200p30_init(uint8_t ViPipe)
{
	sc201cs_slave_write_register(ViPipe, 0x0103, 0x01);
	sc201cs_slave_write_register(ViPipe, 0x0100, 0x00);
	sc201cs_slave_write_register(ViPipe, 0x36e9, 0x80);
	sc201cs_slave_write_register(ViPipe, 0x36ea, 0x0f);
	sc201cs_slave_write_register(ViPipe, 0x36eb, 0x25);
	sc201cs_slave_write_register(ViPipe, 0x36ed, 0x04);
	sc201cs_slave_write_register(ViPipe, 0x36e9, 0x01);
	sc201cs_slave_write_register(ViPipe, 0x301f, 0x01);
	sc201cs_slave_write_register(ViPipe, 0x3248, 0x02);
	sc201cs_slave_write_register(ViPipe, 0x3253, 0x0a);
	sc201cs_slave_write_register(ViPipe, 0x3301, 0xff);
	sc201cs_slave_write_register(ViPipe, 0x3302, 0xff);
	sc201cs_slave_write_register(ViPipe, 0x3303, 0x10);
	sc201cs_slave_write_register(ViPipe, 0x3306, 0x28);
	sc201cs_slave_write_register(ViPipe, 0x3307, 0x02);
	sc201cs_slave_write_register(ViPipe, 0x330a, 0x00);
	sc201cs_slave_write_register(ViPipe, 0x330b, 0xb0);
	sc201cs_slave_write_register(ViPipe, 0x3318, 0x02);
	sc201cs_slave_write_register(ViPipe, 0x3320, 0x06);
	sc201cs_slave_write_register(ViPipe, 0x3321, 0x02);
	sc201cs_slave_write_register(ViPipe, 0x3326, 0x12);
	sc201cs_slave_write_register(ViPipe, 0x3327, 0x0e);
	sc201cs_slave_write_register(ViPipe, 0x3328, 0x03);
	sc201cs_slave_write_register(ViPipe, 0x3329, 0x0f);
	sc201cs_slave_write_register(ViPipe, 0x3364, 0x0f);
	sc201cs_slave_write_register(ViPipe, 0x33b3, 0x40);
	sc201cs_slave_write_register(ViPipe, 0x33f9, 0x2c);
	sc201cs_slave_write_register(ViPipe, 0x33fb, 0x38);
	sc201cs_slave_write_register(ViPipe, 0x33fc, 0x0f);
	sc201cs_slave_write_register(ViPipe, 0x33fd, 0x1f);
	sc201cs_slave_write_register(ViPipe, 0x349f, 0x03);
	sc201cs_slave_write_register(ViPipe, 0x34a6, 0x01);
	sc201cs_slave_write_register(ViPipe, 0x34a7, 0x1f);
	sc201cs_slave_write_register(ViPipe, 0x34a8, 0x40);
	sc201cs_slave_write_register(ViPipe, 0x34a9, 0x30);
	sc201cs_slave_write_register(ViPipe, 0x34ab, 0xa6);
	sc201cs_slave_write_register(ViPipe, 0x34ad, 0xa6);
	sc201cs_slave_write_register(ViPipe, 0x3622, 0x60);
	sc201cs_slave_write_register(ViPipe, 0x3625, 0x08);
	sc201cs_slave_write_register(ViPipe, 0x3630, 0xa8);
	sc201cs_slave_write_register(ViPipe, 0x3631, 0x84);
	sc201cs_slave_write_register(ViPipe, 0x3632, 0x90);
	sc201cs_slave_write_register(ViPipe, 0x3633, 0x43);
	sc201cs_slave_write_register(ViPipe, 0x3634, 0x09);
	sc201cs_slave_write_register(ViPipe, 0x3635, 0x82);
	sc201cs_slave_write_register(ViPipe, 0x3636, 0x48);
	sc201cs_slave_write_register(ViPipe, 0x3637, 0xe4);
	sc201cs_slave_write_register(ViPipe, 0x3641, 0x22);
	sc201cs_slave_write_register(ViPipe, 0x3670, 0x0e);
	sc201cs_slave_write_register(ViPipe, 0x3674, 0xc0);
	sc201cs_slave_write_register(ViPipe, 0x3675, 0xc0);
	sc201cs_slave_write_register(ViPipe, 0x3676, 0xc0);
	sc201cs_slave_write_register(ViPipe, 0x3677, 0x86);
	sc201cs_slave_write_register(ViPipe, 0x3678, 0x88);
	sc201cs_slave_write_register(ViPipe, 0x3679, 0x8c);
	sc201cs_slave_write_register(ViPipe, 0x367c, 0x01);
	sc201cs_slave_write_register(ViPipe, 0x367d, 0x0f);
	sc201cs_slave_write_register(ViPipe, 0x367e, 0x01);
	sc201cs_slave_write_register(ViPipe, 0x367f, 0x0f);
	sc201cs_slave_write_register(ViPipe, 0x3690, 0x43);
	sc201cs_slave_write_register(ViPipe, 0x3691, 0x43);
	sc201cs_slave_write_register(ViPipe, 0x3692, 0x53);
	sc201cs_slave_write_register(ViPipe, 0x369c, 0x01);
	sc201cs_slave_write_register(ViPipe, 0x369d, 0x1f);
	sc201cs_slave_write_register(ViPipe, 0x3900, 0x0d);
	sc201cs_slave_write_register(ViPipe, 0x3904, 0x06);
	sc201cs_slave_write_register(ViPipe, 0x3905, 0x98);
	sc201cs_slave_write_register(ViPipe, 0x391b, 0x81);
	sc201cs_slave_write_register(ViPipe, 0x391c, 0x10);
	sc201cs_slave_write_register(ViPipe, 0x391d, 0x19);
	sc201cs_slave_write_register(ViPipe, 0x3949, 0xc8);
	sc201cs_slave_write_register(ViPipe, 0x394b, 0x64);
	sc201cs_slave_write_register(ViPipe, 0x3952, 0x02);
	sc201cs_slave_write_register(ViPipe, 0x3e00, 0x00);
	sc201cs_slave_write_register(ViPipe, 0x3e01, 0x4d);
	sc201cs_slave_write_register(ViPipe, 0x3e02, 0xe0);
	sc201cs_slave_write_register(ViPipe, 0x4502, 0x34);
	sc201cs_slave_write_register(ViPipe, 0x4509, 0x30);

#if CONFIG_SENSOR_DUAL_SWITCH
	sc201cs_slave_write_register(ViPipe, 0x3019, 0xff);
	sc201cs_slave_write_register(ViPipe, 0x0100, 0x00);
#else
	sc201cs_slave_write_register(ViPipe, 0x0100, 0x01);
#endif

	// printf("ViPipe:%d,===SC201CS_SLAVE 1200P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}