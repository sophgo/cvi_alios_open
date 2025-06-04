/*
 * Copyright (C) 2020 FishSemi Inc. All rights reserved.
 */

#ifndef _DEVICE_NBIOT_MODULE_IMPL_H
#define _DEVICE_NBIOT_MODULE_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <devices/hal/net_impl.h>
#include <devices/nbiot.h>

typedef struct nbiot_driver {
    int (*start_nbiot)(aos_dev_t *dev);
    int (*stop_nbiot)(aos_dev_t *dev);
    int (*reset_nbiot)(aos_dev_t *dev);
    int (*get_nbiot_status)(aos_dev_t *dev, nbiot_status_t *status);
    int (*get_imsi)(aos_dev_t *dev, nbiot_imsi_t *imsi);
    int (*get_imei)(aos_dev_t *dev, nbiot_imei_t *imei);
    int (*get_csq)(aos_dev_t *dev, int *csq);
    int (*get_simcard_info)(aos_dev_t *dev, nbiot_iccid_t *iccid, int *insert);
    int (*get_cell_info)(aos_dev_t *dev, nbiot_cell_info_t *cellinfo);
    int (*set_nbiot_status_ind)(aos_dev_t *dev, int status);
    int (*set_nbiot_signal_strength_ind)(aos_dev_t *dev, int status);
} nbiot_driver_t;

#ifdef __cplusplus
}
#endif

#endif /*_DEVICE_NBIOT_MODULE_IMPL_H*/
