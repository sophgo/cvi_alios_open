#ifndef __SENSOR_I2C_H_
#define __SENSOR_I2C_H_

#if defined(CONFIG_DUAL_OS)
#include <drv/iic.h>
#else
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#endif
#include "cvi_type.h"
#include "cvi_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define IIC_MAX 20

#define UNUSED(x) ((void)(x))

int sensor_i2c_init(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr);

int sensor_i2c_exit(CVI_S32 ViPipe, CVI_U8 i2c_id);

int sensor_i2c_read(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr,
			CVI_U32 addr,CVI_U32 snsr_addr_byte,
			CVI_U32 snsr_data_byte);

int sensor_i2c_write(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr,
			CVI_U32 addr, CVI_U32 snsr_addr_byte,
			CVI_U32 data, CVI_U32 snsr_data_byte);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SENSOR_I2C_H_ */