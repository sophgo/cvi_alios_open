#ifndef __UAC_DESCRIPTOR_H__
#define __UAC_DESCRIPTOR_H__

#include <stdint.h>

/* AUDIO Class Config */
#define AUDIO_FREQ 16000U

#define AUDIO_FIRST_INTERFACE 2
#define AUDIO_INTERFACE_COUNT 3
#define AUDIO_CHANNEL_NUM 2

uint8_t *uac_build_descriptor(uint32_t *len);
void uac_destroy_descriptor(uint8_t *desc);

#endif