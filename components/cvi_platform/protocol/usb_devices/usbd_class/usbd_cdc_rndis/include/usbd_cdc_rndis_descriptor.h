#ifndef __USBD_CDC_RNDIS_DESCRIPTOR_H__
#define __USBD_CDC_RNDIS_DESCRIPTOR_H__

#include <stdint.h>

#include "usbd_cdc_rndis.h"

#if CONFIG_USB_HS
#define CDC_RNDIS_MPS 512
#else
#define CDC_RNDIS_MPS 64
#endif


uint8_t *cdc_rndis_build_descriptor(struct cdc_rndis_device_info *cdc_rndis_info, uint32_t *desc_len);
void cdc_rndis_destroy_descriptor(uint8_t *desc);

#endif