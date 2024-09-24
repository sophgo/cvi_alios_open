/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#include <stdio.h>

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#define REG_SDMA_DMA_CH_REMAP0	0x03000154U
#define REG_SDMA_DMA_CH_REMAP1	0x03000158U

typedef enum {
	CVI_I2S0_RX = 0,
	CVI_I2S0_TX,
	CVI_I2S1_RX,
	CVI_I2S1_TX,
	CVI_I2S2_RX,
	CVI_I2S2_TX,
	CVI_I2S3_RX,
	CVI_I2S3_TX,
	CVI_UART0_RX,
	CVI_UART0_TX,
	CVI_UART1_RX,
	CVI_UART1_TX,
	CVI_UART2_RX,
	CVI_UART2_TX,
	CVI_UART3_RX,
	CVI_UART3_TX,
	CVI_SPI0_RX,
	CVI_SPI0_TX,
	CVI_SPI1_RX,
	CVI_SPI1_TX,
	CVI_SPI2_RX,
	CVI_SPI2_TX,
	CVI_SPI3_RX,
	CVI_SPI3_TX,
	CVI_I2C0_RX,
	CVI_I2C0_TX,
	CVI_I2C1_RX,
	CVI_I2C1_TX,
	CVI_I2C2_RX,
	CVI_I2C2_TX,
	CVI_I2C3_RX,
	CVI_I2C3_TX,
	CVI_I2C4_RX,
	CVI_I2C4_TX,
	CVI_TDM0_RX,
	CVI_TDM0_TX,
	CVI_TDM1_RX,
	CVI_AUDSRC,
	CVI_SPI_NAND,
	CVI_SPI_NOR,
	CVI_UART4_RX,
	CVI_UART4_TX,
	CVI_SPI_NOR1,
}request_index_t;

struct dma_remap_item{
	request_index_t hs_id;
	uint8_t channel_id;
};

static struct dma_remap_item remap_table[] = {
	{CVI_UART2_TX, 1},	//13
	{CVI_UART2_RX, 0},	//13
	{CVI_I2S3_TX, 2},	//13
	{CVI_I2S0_RX, 3},	//13
	{CVI_SPI3_RX, 4},
	{CVI_SPI3_TX, 5},
	{CVI_SPI_NAND, 7}

};

static uint32_t remap0_val;
static uint32_t remap1_val;

static void dma_hs_remap(request_index_t hs_id, uint8_t channel_id)
{
	if(channel_id < 4){
		remap0_val |= hs_id << (channel_id << 3);
	} else {
		channel_id -= 4;
		remap1_val |= hs_id << (channel_id << 3);
	}
}

void dma_hs_remap_init(void)
{
	int table_size = sizeof(remap_table) / sizeof(struct dma_remap_item);

	for(int i = 0; i < table_size; i++){
		dma_hs_remap(remap_table[i].hs_id, remap_table[i].channel_id);
	}

	remap0_val |= 1 << 31;
	remap1_val |= 1 << 31;

	*((volatile uint32_t *)REG_SDMA_DMA_CH_REMAP0) = remap0_val;
	*((volatile uint32_t *)REG_SDMA_DMA_CH_REMAP1) = remap1_val;

	//printf("remap0=%x\r\n", *((volatile uint32_t *)REG_SDMA_DMA_CH_REMAP0));
	//printf("remap1=%x\r\n", *((volatile uint32_t *)REG_SDMA_DMA_CH_REMAP1));
}
