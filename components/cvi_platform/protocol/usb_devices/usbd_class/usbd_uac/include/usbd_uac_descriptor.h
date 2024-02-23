#ifndef __USBD_UAC_DESCRIPTOR_H__
#define __USBD_UAC_DESCRIPTOR_H__

#include <stdint.h>

#include "usbd_uac.h"

/* AUDIO Class Config */
#define AUDIO_FREQ 8000U

#define AUDIO_SAMPLE_FREQ(frq) (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

/* Audio Channel Number */
#define AUDIO_CHANNEL_NUM 1

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define AUDIO_OUT_PACKET ((uint32_t)((AUDIO_FREQ * 2 * AUDIO_CHANNEL_NUM) / 1000))
/* 16bit(2 Bytes) 双声道(Mono:2) */
#define AUDIO_IN_PACKET ((uint32_t)((AUDIO_FREQ * 2 * AUDIO_CHANNEL_NUM) / 1000))


uint8_t *uac_build_descriptor(struct uac_device_info *uac_info, uint32_t *desc_len);
void uac_destroy_descriptor(uint8_t *desc);

#endif
