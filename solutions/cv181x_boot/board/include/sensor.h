#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <stdint.h>

int sensor_init(void);

int sensor_i2c_init(uint8_t i2c_id);

int sensor_i2c_exit(uint8_t i2c_id);

int sensor_i2c_read(uint8_t i2c_id, uint8_t snsr_i2c_addr,
	uint32_t addr,uint32_t snsr_addr_byte,
	uint32_t snsr_data_byte);

int sensor_i2c_write(uint8_t i2c_id, uint8_t snsr_i2c_addr,
	uint32_t addr, uint32_t snsr_addr_byte,
	uint32_t data, uint32_t snsr_data_byte);

void sensor_pinmux_init(void);

#endif