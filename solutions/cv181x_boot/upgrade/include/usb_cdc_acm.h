#ifndef __USB_CDC_ACM__
#define __USB_CDC_ACM__

#include <stdint.h>

void cdc_acm_init(void);
void cdc_acm_deinit(void);
int cdc_acm_write(uint8_t *buffer, uint32_t nbytes);
int cdc_acm_read(uint8_t *buffer, uint32_t nbytes, uint32_t timeout_ms);


#endif