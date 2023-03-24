/**************************************************************************************************
  Filename:       CRC.h
  Revised:        $Date: 2019-09-01 14:03:16 -0700 (Sun, 01 Aug 2017) $
  Revision:       $Revision: 23256 $
  Description:    This file contains the Simple BLE Peripheral sample application
                  definitions and prototypes.
  History :       mao
  Copyright 2017 - 2019 Shenzhou Fingercrystal .col. All rights reserved.
  Should you have any questions regarding your right to use this Software,
  contact Shenzhou Fingercrystal at www.fingercrystal.com.
**************************************************************************************************/
#ifndef    _CRC_H_
#define    _CRC_H_


typedef struct {
	unsigned int crc;
} CRC32_CTX;

void InitCRC32(CRC32_CTX *ctx);

uint32_t CRC32_Update(CRC32_CTX *ctx, uint8_t *pBuf, uint32_t Len);

uint32_t CRC32Software(uint8_t *pData, uint32_t Length);

uint32_t CRC16(uint8_t *puchMsg, uint32_t usDataLen);

uint8_t CRC8(const uint8_t *ptr, uint8_t len);

//uint32_t CRC32Software( uint8_t *pData, uint32_t Length );

//uint32_t CRC16(uint8_t *puchMsg, uint32_t usDataLen);

unsigned short CRC16_CCITT(unsigned char *data, unsigned long len);



#endif


