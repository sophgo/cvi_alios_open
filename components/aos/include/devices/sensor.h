/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef DEVICE_SENSOR_PAI_H
#define DEVICE_SENSOR_PAI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <devices/device.h>

// temperature unit is Celsius by default
struct _sensor_temperature {
    int degree;
};

// humidity unit is RH
struct _sensor_humidity {
    int rh;
};

// temperature unit is Celsius by default
// humidity unit is RH
struct _sensor_temphumi {
    int degree;
    int rh;
};

// binary sensor status is 0 or 1
struct _sensor_binary {
    int status;
};

// accelerate unit is g
// gyroscope unit is rad/s
struct _sensor_gravity {
    int acc_x;
    int acc_y;
    int acc_z;
    int gyro_x;
    int gyro_y;
    int gyro_z;
};

typedef struct _sensor_temphumi sensor_sht20_t;
typedef struct _sensor_temphumi sensor_simulate_t;
typedef struct _sensor_binary   sensor_light_t;
typedef struct _sensor_gravity  sensor_mpu6050_t;

#define sensor_open(name) device_open(name)
#define sensor_open_id(name, id) device_open_id(name, id)
#define sensor_close(dev) device_close(dev)

/**
  \brief       Notify a sensor to fetch data.
  \param[in]   dev  Pointer to device object.
  \return      0 on success, -1 on fail.
*/
int sensor_fetch(aos_dev_t *dev);

/**
  \brief       Get valude from a sensor device.
  \param[in]   dev  Pointer to device object.
  \param[in]   value  Returned sensor value.
  \param[in]   size   Returned sensor value size.
  \return      0 on success, -1 on fail.
*/
int sensor_getvalue(aos_dev_t *dev, void *value, size_t size);

#ifdef __cplusplus
}
#endif

#endif
