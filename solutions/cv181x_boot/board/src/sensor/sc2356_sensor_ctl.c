
#include "drv/common.h"
#include <unistd.h>
#include <stdint.h>
#include <drv/tick.h>

#include "sensor.h"


#define SC2356_CHIP_ID_HI_ADDR		0x3107
#define SC2356_CHIP_ID_LO_ADDR		0x3108
#define SC2356_CHIP_ID			    0xeb52


static void sc2356_linear_1600x1200p30_init(uint8_t ViPipe);

uint8_t sc2356_i2c_addr = 0x36;        /* I2C Address of SC2356 */
uint8_t sc2356_i2c_dev = 1;
const int sc2356_addr_byte = 2;
const int sc2356_data_byte = 1;

int sc2356_i2c_init(uint8_t ViPipe)
{
	uint8_t i2c_id = (uint8_t)sc2356_i2c_dev;

	return sensor_i2c_init(i2c_id);
}

int sc2356_i2c_exit(uint8_t ViPipe)
{
	uint8_t i2c_id = (uint8_t)sc2356_i2c_dev;

	return sensor_i2c_exit(i2c_id);
}

int sc2356_read_register(uint8_t ViPipe, int addr)
{
    uint8_t i2c_id = (uint8_t)sc2356_i2c_dev;

	return sensor_i2c_read(i2c_id, sc2356_i2c_addr, (uint32_t)addr, sc2356_addr_byte, sc2356_data_byte);
}

int sc2356_write_register(uint8_t ViPipe, int addr, int data)
{
	uint8_t i2c_id = (uint8_t)sc2356_i2c_dev;

	return sensor_i2c_write(i2c_id, sc2356_i2c_addr, (uint32_t)addr, sc2356_addr_byte,
				(uint32_t)data, sc2356_data_byte);
}

int sc2356_probe(uint8_t i2c_addr, uint8_t i2c_dev)
{
	int nVal;
	uint16_t chip_id;
    sc2356_i2c_addr = i2c_addr;
	sc2356_i2c_dev = i2c_dev;
    uint8_t ViPipe = 0;


	//usleep(4*1000);
	if (sc2356_i2c_init(ViPipe) != 0)
		return -1;

	nVal = sc2356_read_register(ViPipe, SC2356_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		//CVI_TRACE_SNS(3, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc2356_read_register(ViPipe, SC2356_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		//CVI_TRACE_SNS(3, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC2356_CHIP_ID) {
		//CVI_TRACE_SNS(3, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return -1;
	}

	return 0;
}

void sc2356_init(uint8_t ViPipe)
{

	sc2356_i2c_init(ViPipe);
    /* Developer should which sensor size to enable */
    sc2356_linear_1600x1200p30_init(ViPipe);

}

void sc2356_exit(uint8_t ViPipe)
{
	sc2356_i2c_exit(ViPipe);
}


static void sc2356_linear_1600x1200p30_init(uint8_t ViPipe)
{
    sc2356_write_register(ViPipe, 0x0103, 0x01);
    sc2356_write_register(ViPipe, 0x0100, 0x00);
    sc2356_write_register(ViPipe, 0x36e9, 0x80);
    sc2356_write_register(ViPipe, 0x36ea, 0x0a);
    sc2356_write_register(ViPipe, 0x36eb, 0x0c);
    sc2356_write_register(ViPipe, 0x36ec, 0x01);
    sc2356_write_register(ViPipe, 0x36ed, 0x18);
    sc2356_write_register(ViPipe, 0x36e9, 0x10);
    sc2356_write_register(ViPipe, 0x301f, 0x1b);
    sc2356_write_register(ViPipe, 0x3301, 0xff);
    sc2356_write_register(ViPipe, 0x3304, 0x68);
    sc2356_write_register(ViPipe, 0x3306, 0x40);
    sc2356_write_register(ViPipe, 0x3308, 0x08);
    sc2356_write_register(ViPipe, 0x3309, 0xa8);
    sc2356_write_register(ViPipe, 0x330b, 0xb0);
    sc2356_write_register(ViPipe, 0x330c, 0x18);
    sc2356_write_register(ViPipe, 0x330d, 0xff);
    sc2356_write_register(ViPipe, 0x330e, 0x20);
    sc2356_write_register(ViPipe, 0x331e, 0x59);
    sc2356_write_register(ViPipe, 0x331f, 0x99);
    sc2356_write_register(ViPipe, 0x3333, 0x10);
    sc2356_write_register(ViPipe, 0x335e, 0x06);
    sc2356_write_register(ViPipe, 0x335f, 0x08);
    sc2356_write_register(ViPipe, 0x3364, 0x1f);
    sc2356_write_register(ViPipe, 0x337c, 0x02);
    sc2356_write_register(ViPipe, 0x337d, 0x0a);
    sc2356_write_register(ViPipe, 0x338f, 0xa0);
    sc2356_write_register(ViPipe, 0x3390, 0x01);
    sc2356_write_register(ViPipe, 0x3391, 0x03);
    sc2356_write_register(ViPipe, 0x3392, 0x1f);
    sc2356_write_register(ViPipe, 0x3393, 0xff);
    sc2356_write_register(ViPipe, 0x3394, 0xff);
    sc2356_write_register(ViPipe, 0x3395, 0xff);
    sc2356_write_register(ViPipe, 0x33a2, 0x04);
    sc2356_write_register(ViPipe, 0x33ad, 0x0c);
    sc2356_write_register(ViPipe, 0x33b1, 0x20);
    sc2356_write_register(ViPipe, 0x33b3, 0x38);
    sc2356_write_register(ViPipe, 0x33f9, 0x40);
    sc2356_write_register(ViPipe, 0x33fb, 0x48);
    sc2356_write_register(ViPipe, 0x33fc, 0x0f);
    sc2356_write_register(ViPipe, 0x33fd, 0x1f);
    sc2356_write_register(ViPipe, 0x349f, 0x03);
    sc2356_write_register(ViPipe, 0x34a6, 0x03);
    sc2356_write_register(ViPipe, 0x34a7, 0x1f);
    sc2356_write_register(ViPipe, 0x34a8, 0x38);
    sc2356_write_register(ViPipe, 0x34a9, 0x30);
    sc2356_write_register(ViPipe, 0x34ab, 0xb0);
    sc2356_write_register(ViPipe, 0x34ad, 0xb0);
    sc2356_write_register(ViPipe, 0x34f8, 0x1f);
    sc2356_write_register(ViPipe, 0x34f9, 0x20);
    sc2356_write_register(ViPipe, 0x3630, 0xa0);
    sc2356_write_register(ViPipe, 0x3631, 0x92);
    sc2356_write_register(ViPipe, 0x3632, 0x64);
    sc2356_write_register(ViPipe, 0x3633, 0x43);
    sc2356_write_register(ViPipe, 0x3637, 0x49);
    sc2356_write_register(ViPipe, 0x363a, 0x85);
    sc2356_write_register(ViPipe, 0x363c, 0x0f);
    sc2356_write_register(ViPipe, 0x3650, 0x31);
    sc2356_write_register(ViPipe, 0x3670, 0x0d);
    sc2356_write_register(ViPipe, 0x3674, 0xc0);
    sc2356_write_register(ViPipe, 0x3675, 0xa0);
    sc2356_write_register(ViPipe, 0x3676, 0xa0);
    sc2356_write_register(ViPipe, 0x3677, 0x92);
    sc2356_write_register(ViPipe, 0x3678, 0x96);
    sc2356_write_register(ViPipe, 0x3679, 0x9a);
    sc2356_write_register(ViPipe, 0x367c, 0x03);
    sc2356_write_register(ViPipe, 0x367d, 0x0f);
    sc2356_write_register(ViPipe, 0x367e, 0x01);
    sc2356_write_register(ViPipe, 0x367f, 0x0f);
    sc2356_write_register(ViPipe, 0x3698, 0x83);
    sc2356_write_register(ViPipe, 0x3699, 0x86);
    sc2356_write_register(ViPipe, 0x369a, 0x8c);
    sc2356_write_register(ViPipe, 0x369b, 0x94);
    sc2356_write_register(ViPipe, 0x36a2, 0x01);
    sc2356_write_register(ViPipe, 0x36a3, 0x03);
    sc2356_write_register(ViPipe, 0x36a4, 0x07);
    sc2356_write_register(ViPipe, 0x36ae, 0x0f);
    sc2356_write_register(ViPipe, 0x36af, 0x1f);
    sc2356_write_register(ViPipe, 0x36bd, 0x22);
    sc2356_write_register(ViPipe, 0x36be, 0x22);
    sc2356_write_register(ViPipe, 0x36bf, 0x22);
    sc2356_write_register(ViPipe, 0x36d0, 0x01);
    sc2356_write_register(ViPipe, 0x370f, 0x02);
    sc2356_write_register(ViPipe, 0x3721, 0x6c);
    sc2356_write_register(ViPipe, 0x3722, 0x8d);
    sc2356_write_register(ViPipe, 0x3725, 0xc5);
    sc2356_write_register(ViPipe, 0x3727, 0x14);
    sc2356_write_register(ViPipe, 0x3728, 0x04);
    sc2356_write_register(ViPipe, 0x37b7, 0x04);
    sc2356_write_register(ViPipe, 0x37b8, 0x04);
    sc2356_write_register(ViPipe, 0x37b9, 0x06);
    sc2356_write_register(ViPipe, 0x37bd, 0x07);
    sc2356_write_register(ViPipe, 0x37be, 0x0f);
    sc2356_write_register(ViPipe, 0x3901, 0x02);
    sc2356_write_register(ViPipe, 0x3903, 0x40);
    sc2356_write_register(ViPipe, 0x3905, 0x8d);
    sc2356_write_register(ViPipe, 0x3907, 0x00);
    sc2356_write_register(ViPipe, 0x3908, 0x41);
    sc2356_write_register(ViPipe, 0x391f, 0x41);
    sc2356_write_register(ViPipe, 0x3933, 0x80);
    sc2356_write_register(ViPipe, 0x3934, 0x02);
    sc2356_write_register(ViPipe, 0x3937, 0x6f);
    sc2356_write_register(ViPipe, 0x393a, 0x01);
    sc2356_write_register(ViPipe, 0x393d, 0x01);
    sc2356_write_register(ViPipe, 0x393e, 0xc0);
    sc2356_write_register(ViPipe, 0x39dd, 0x41);
    sc2356_write_register(ViPipe, 0x3e00, 0x00);
    sc2356_write_register(ViPipe, 0x3e01, 0x4d);
    sc2356_write_register(ViPipe, 0x3e02, 0xc0);
    sc2356_write_register(ViPipe, 0x3e09, 0x00);
    sc2356_write_register(ViPipe, 0x4509, 0x28);
    sc2356_write_register(ViPipe, 0x450d, 0x61);
    sc2356_write_register(ViPipe, 0x0100, 0x01);

    //sc2356_default_reg_init(ViPipe);
	//delay_ms(10);

	//printf("ViPipe:%d,===SC2356 1600x1200@30fps 10bit LINE Init OK!===\n", ViPipe);
}

