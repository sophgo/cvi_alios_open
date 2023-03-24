#ifndef __UAC_H__
#define __UAC_H__

#include <stdint.h>
#include "usbd_audio.h"

#define AUDIO_IN_EP  0x83
#define AUDIO_OUT_EP 0x02

void usbd_audio_out_callback(uint8_t ep, uint32_t nbytes);
void usbd_audio_in_callback(uint8_t ep, uint32_t nbytes);

int MEDIA_UAC_Init(void);
int MEDIA_UAC_deInit(void);

#endif