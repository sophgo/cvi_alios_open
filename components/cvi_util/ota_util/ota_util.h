#ifndef __USBD_CDC_OTA_UTIL_H__
#define __USBD_CDC_OTA_UTIL_H__

#include <aos/kernel.h>
#include <pthread.h>
#include <stdio.h>
#include "cvi_util.h"
#include "drv/spiflash.h"
#include <aos/cli.h>

void cvi_ota_set_flag(int value);
void cvi_ota_init(void);

#endif