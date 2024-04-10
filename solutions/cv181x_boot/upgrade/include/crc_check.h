#ifndef __CRC_CHECK__
#define __CRC_CHECK__

#include <stdint.h>

uint8_t calculate_crc8(const uint8_t *data, int length);

#endif