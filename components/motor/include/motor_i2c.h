#ifndef __MOTOR_I2C_H__
#define __MOTOR_I2C_H__
#include <drv/iic.h>
#include <stdint.h>

int32_t motor_i2c_init(uint8_t i2c_num);
int32_t motor_i2c_exit(uint8_t i2c_num);
int32_t motor_i2c_write(uint8_t i2c_num, uint8_t device_addr,
            uint32_t reg_addr, csi_iic_mem_addr_size_t reg_addr_len,
            uint8_t *pdata, uint32_t data_len);

#endif