/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 */

/******************************************************************************
 * @file     spinand_vendor.c
 * @brief
 * @version
 * @date     2020-03-26
 ******************************************************************************/

#include <string.h>
#include <drv/spinand.h>
#include "spinand_vendor.h"
#include "stdlib.h"

#define SPINAND_MFR_GIGADEVICE			0xC8
#define SPINAND_MFR_WINBOND		        0xEF
#define SPINAND_MFR_MACRONIX		    0xC2
#define SPINAND_MFR_FM                  0xA1

#define WINBOND_CFG_BUF_READ		    (1<<3)

#define ECC_CHECK_4_TO_5_MSK			(3<<4)
#define ECC_CHECK_4_TO_6_MSK       		(7<<4)

#define SPINAND_PAGE_BUF_MAX_SZ         (4096+128)
extern csi_error_t csi_spinand_update_config(csi_spinand_t *spinand, uint8_t mask, uint8_t val);
extern csi_error_t csi_spinand_select_target(csi_spinand_t *spinand, uint32_t target);
/* g_nand_page_buf need 64byte-aligned and size need multiple times of 64. if use dma mode ,buf need reserve 8byte space for nand cmd and adrr */
static uint8_t g_nand_page_buf[SPINAND_PAGE_BUF_MAX_SZ] __attribute__((aligned(64)));
/*********************************************************************************
 * 
 *                  ESMT DEV-LIST 
 * 
 *********************************************************************************/
static const csi_spinand_manufacturer_ops_t esmt_spinand_manuf_ops = {
};

static csi_error_t esmt_check_ecc_status_1(void *spinand,uint8_t status)
{
	if( (status & ECC_CHECK_4_TO_5_MSK) == (2<<4) ){
		/* uncorected error */
		return CSI_ERROR;
	}

	return CSI_OK;
}

static const csi_spinand_info_t esmt_spinand_table[] = {
	SPINAND_INFO("F50L1G41LB",
		     SPINAND_ID(0xc8, 0x01),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(esmt_check_ecc_status_1),
		     ),
	SPINAND_INFO("F50L512M41A",
		     SPINAND_ID(0xc8, 0x20),
		     NAND_MEMORG(1, 2048, 64, 64, 512, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(esmt_check_ecc_status_1),
		     ),
	SPINAND_INFO("F50L1G41A",
		     SPINAND_ID(0xc8, 0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(esmt_check_ecc_status_1),
		     ),
	SPINAND_INFO("F50L2G41KA",
		     SPINAND_ID(0xc8, 0x41),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(esmt_check_ecc_status_1),
		     ),

	SPINAND_INFO("F50L2G41XA",
		     SPINAND_ID(0x2c, 0x24),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 20, 2, 1, 1),
		     NAND_ECCREQ(8, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(esmt_check_ecc_status_1),
		     ),
	SPINAND_INFO("F50L4G41XB",
		     SPINAND_ID(0x2c, 0x34),
		     NAND_MEMORG(1, 4096, 256, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(esmt_check_ecc_status_1),
		     ),
};

const csi_spinand_manufacturer_t esmt_spinand_manufacturer = {
	.name = "ESMT",
	.chips = esmt_spinand_table,
	.nchips = ARRAY_SIZE(esmt_spinand_table),
	.ops = &esmt_spinand_manuf_ops,
};



/*********************************************************************************
 * 
 *                  GIGADEVICE DEV-LIST 
 * 
 *********************************************************************************/
static const csi_spinand_manufacturer_ops_t gigadevice_spinand_manuf_ops = {
};

#if 0
static csi_error_t gigadevice_check_ecc_status_1(void *spinand,uint8_t status)
{
	if( (status & ECC_CHECK_4_TO_6_MSK) == ECC_CHECK_4_TO_6_MSK ){
		/* uncorected error */
		return CSI_ERROR;
	}

	return CSI_OK;
}
#endif

static csi_error_t gigadevice_check_ecc_status_2(void *spinand,uint8_t status)
{
	if( (status & ECC_CHECK_4_TO_5_MSK) == (2<<4) ){
		/* uncorected error */
		return CSI_ERROR;
	}

	return CSI_OK;
}

static const csi_spinand_info_t gigadevice_spinand_table[] = {
	SPINAND_INFO("GD5F1GQ4UAYIG",
		     SPINAND_ID(0xc8, 0xf1),
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F1GQ4UBYIG",
		     SPINAND_ID(0xc8, 0xd1),
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F1GM7UExxG",
		     SPINAND_ID(0xc8, 0x91),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F1GM7RExxG",
		     SPINAND_ID(0xc8, 0x81),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F1GQ5UExxG",
		     SPINAND_ID(0xc8, 0x51),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F1GQ5RExxG",
		     SPINAND_ID(0xc8, 0x41),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F1GQ5UExxH",
		     SPINAND_ID(0xc8, 0x31),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F2GQ4UAYIG",
		     SPINAND_ID(0xc8, 0xf2),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F2GQ4UBYIG",
		     SPINAND_ID(0xc8, 0xd2),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F2GM7UExxG",
		     SPINAND_ID(0xc8, 0x92),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F2GM7RExxG",
		     SPINAND_ID(0xc8, 0x82),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F2GQxxx",
		     SPINAND_ID(0xc8, 0x52),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
			),
	SPINAND_INFO("GD5F2GQxxx",
		     SPINAND_ID(0xc8, 0x42),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
			),
	SPINAND_INFO("GD5F4GQ4UAYIG",
		     SPINAND_ID(0xc8, 0xf4),
		     NAND_MEMORG(1, 2048, 64, 64, 4096, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F4GQ4UBYIG",
		     SPINAND_ID(0xc8, 0xd4),
		     NAND_MEMORG(1, 4096, 256, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),		
			 SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
		     ),
	SPINAND_INFO("GD5F4GQ6UExxG",
		     SPINAND_ID(0xc8, 0x55),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 80, 1, 2, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
			),
	SPINAND_INFO("GD5F4GQ6RExxG",
		     SPINAND_ID(0xc8, 0x45),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 80, 1, 2, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(gigadevice_check_ecc_status_2),
			),
};

const csi_spinand_manufacturer_t gigadevice_spinand_manufacturer = {
	.id = SPINAND_MFR_GIGADEVICE,
	.name = "GigaDevice",
	.chips = gigadevice_spinand_table,
	.nchips = ARRAY_SIZE(gigadevice_spinand_table),
	.ops = &gigadevice_spinand_manuf_ops,
};

/*********************************************************************************
 * 
 *                   WINBOND DEV-LIST 
 * 
 *********************************************************************************/
static csi_error_t w25m02gv_select_target(void *spinand,uint32_t target)
{
	csi_spinand_t *handle = (csi_spinand_t*) spinand;
	 spi_mem_op_t op = SPI_MEM_OP(CVI_SPI_MEM_OP_CMD(0xc2,1),
					  			  CVI_SPI_MEM_OP_NO_ADDR,
					              CVI_SPI_MEM_OP_NO_DUMMY,
					              CVI_SPI_MEM_OP_DATA_OUT(1,handle->scractbuf,1));

	handle->scractbuf[0] = target;
	return handle->spi_mem(handle,&op);
}

static const csi_spinand_info_t winbond_spinand_table[] = {
	SPINAND_INFO("W25N01GV",/* 3.3v */
		     SPINAND_ID(0xef, 0xaa,0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(NULL),
		     ),
	SPINAND_INFO("W25M02GV",
		     SPINAND_ID(0xef, 0xab,0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 20, 1, 1, 2),
		     NAND_ECCREQ(4, 512),
		     0,
		     SPINAND_SELECT_TARGET(w25m02gv_select_target),
			 SPINAND_CHECK_ECC_STATUS(NULL),
			),	
	SPINAND_INFO("W25N01KV",
		     SPINAND_ID(0xef, 0xae,0x21),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(NULL),
		     ),	
	SPINAND_INFO("W25N02GV",
		     SPINAND_ID(0xef, 0xaa,0x22),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(NULL),
		     ),
	SPINAND_INFO("W25N04GV",
		     SPINAND_ID(0xef, 0xaa,0x23),
		     NAND_MEMORG(1, 2048, 64, 64, 4096, 80, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
			 0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(NULL),
		     ),
};

static csi_error_t winbond_spinand_init(void *spinand)
{
	uint32_t i;
	csi_spinand_t* pspinand = (csi_spinand_t*)spinand;
	csi_spinand_info_t *pInfo = (csi_spinand_info_t*)pspinand->chip_info;
	for(i = 0; i < pInfo->memorg.ntargets;i++){
		csi_spinand_select_target(pspinand,i);
		csi_spinand_update_config(pspinand,WINBOND_CFG_BUF_READ,WINBOND_CFG_BUF_READ);
	}

	return 0;
}

static const csi_spinand_manufacturer_ops_t winbond_spinand_manuf_ops = {
	.init = winbond_spinand_init,
	.uninit = NULL
};

const csi_spinand_manufacturer_t winbond_spinand_manufacturer = {
	.id = SPINAND_MFR_WINBOND,
	.name = "Winbond",
	.chips = winbond_spinand_table,
	.nchips = ARRAY_SIZE(winbond_spinand_table),
	.ops = &winbond_spinand_manuf_ops,
};

/*********************************************************************************
 * 
 *                   MACRONIX DEV-LIST 
 * 
 *********************************************************************************/
static csi_error_t macronix_check_ecc_status_1(void *spinand, uint8_t status)
{
	if( (status & ECC_CHECK_4_TO_5_MSK) == (2<<4) ){
		/* uncorected error */
		return CSI_ERROR;
	}

	return CSI_OK;
}

static const  csi_spinand_info_t macronix_spinand_table[] = {
	SPINAND_INFO("MX35LF1GE4AB",
		     SPINAND_ID(0xc2, 0x12),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(macronix_check_ecc_status_1),
	),
	SPINAND_INFO("MX35LF2GE4AB",
		     SPINAND_ID(0xc2, 0x22),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 2, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(macronix_check_ecc_status_1),
	),
	SPINAND_INFO("MX35LF2GE4AD",
		     SPINAND_ID(0xc2, 0x26, 0x03),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(macronix_check_ecc_status_1),
	),
	SPINAND_INFO("MX35LF4GE4AD",
		     SPINAND_ID(0xc2, 0x37, 0x03),
		     NAND_MEMORG(1, 4096, 128, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(macronix_check_ecc_status_1),
	),
};

const  csi_spinand_manufacturer_ops_t macronix_spinand_manuf_ops = {
};

const  csi_spinand_manufacturer_t macronix_spinand_manufacturer = {
	.id = SPINAND_MFR_MACRONIX,
	.name = "Macronix",
	.chips = macronix_spinand_table,
	.nchips = ARRAY_SIZE(macronix_spinand_table),
	.ops = &macronix_spinand_manuf_ops,
};

/*********************************************************************************
 * 
 *                   FM DEV-LIST 
 * 
 *********************************************************************************/

static csi_error_t fm_check_ecc_status_1(void *spinand,uint8_t status)
{
	if( (status & ECC_CHECK_4_TO_5_MSK) == (2<<4) ){
		/* uncorected error */
		return CSI_ERROR;
	}

	return CSI_OK;
}

static const  csi_spinand_info_t fm_spinand_table[] = {
	SPINAND_INFO("FM25S01A",
		     SPINAND_ID(0xa1, 0xe4),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(fm_check_ecc_status_1),
			 ),
	SPINAND_INFO("FM25S02A",
		     SPINAND_ID(0xa1, 0xe5),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 40, 2, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(fm_check_ecc_status_1),
			 ),
	
};

const  csi_spinand_manufacturer_ops_t fm_spinand_manuf_ops = {
};

const  csi_spinand_manufacturer_t fm_spinand_manufacturer = {
	.id = SPINAND_MFR_FM,
	.name = "fudan_micro",
	.chips = fm_spinand_table,
	.nchips = ARRAY_SIZE(fm_spinand_table),
	.ops = &fm_spinand_manuf_ops,
};

/*********************************************************************************
 * 
 *                   Other Vendors flash
 * 
 *********************************************************************************/

static csi_error_t other_check_ecc_status_1(void *spinand,uint8_t status)
{
	if( (status & ECC_CHECK_4_TO_5_MSK) == (2<<4) ){
		/* uncorected error */
		return CSI_ERROR;
	}

	return CSI_OK;
}
const  csi_spinand_manufacturer_ops_t other_spinand_manuf_ops = {
};

static const  csi_spinand_info_t other_spinand_table[] = {
	SPINAND_INFO("XT26G11C",
		     SPINAND_ID(0x0b, 0x15),
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),

	SPINAND_INFO("MT29F1G01",
		     SPINAND_ID(0x2c, 0x12),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),
	SPINAND_INFO("MT29F2G01",
		     SPINAND_ID(0x2c, 0x22),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("MT29F4G01",
		     SPINAND_ID(0x2c, 0x32),
		     NAND_MEMORG(1, 2048, 64, 64, 4096, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("MT29F1G01ABAFDWB",
		     SPINAND_ID(0x2c, 0x14),
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	

	SPINAND_INFO("TC58CVG2S0HRAIJ",
		     SPINAND_ID(0x98, 0xed, 0x51),
		     NAND_MEMORG(1, 4096, 128, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("TC58CVG1S0HRAIJ",
		     SPINAND_ID(0x98, 0xeb, 0x40),
		     NAND_MEMORG(1, 2048, 128, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("TC58CVG1S3HxAI",
		     SPINAND_ID(0x98, 0xcb),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("TC58CVG0S3HRAIJ",
		     SPINAND_ID(0x98, 0xe2),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     0,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	

	SPINAND_INFO("DS35Q1GA-IB",
		     SPINAND_ID(0xe5, 0x71),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("DS35Q1GB-IB",
		     SPINAND_ID(0xe5, 0xf1),
		     NAND_MEMORG(1, 2048, 128, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	

	SPINAND_INFO("F35SQA512M",
		     SPINAND_ID(0xcd, 0x70, 0x70),
		     NAND_MEMORG(1, 2048, 64, 64, 512, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("F35SQA001G",
		     SPINAND_ID(0xcd, 0x71, 0x71),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("F35SQA002G",
		     SPINAND_ID(0xcd, 0x72, 0x72),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	

	SPINAND_INFO("HYF2GQ4UAACAE",
		     SPINAND_ID(0xc9, 0x52),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(14, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	

	SPINAND_INFO("FM25S01A",
		     SPINAND_ID(0xa1, 0xe4),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
	SPINAND_INFO("FM25S02A",
		     SPINAND_ID(0xa1, 0xe5),
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 20, 1, 1, 1),
		     NAND_ECCREQ(1, 512),
		     SPINAND_HAS_QE_EN,
			 SPINAND_SELECT_TARGET(NULL),
			 SPINAND_CHECK_ECC_STATUS(other_check_ecc_status_1),
			 ),	
};


const  csi_spinand_manufacturer_t other_spinand_manufacturer = {

	.name = "other vendors flash",
	.chips = other_spinand_table,
	.nchips = ARRAY_SIZE(other_spinand_table),
	.ops = &other_spinand_manuf_ops,
};


const csi_spinand_manufacturer_t *spinand_manufacturers[] = {
	&esmt_spinand_manufacturer,
	&gigadevice_spinand_manufacturer,
	&winbond_spinand_manufacturer,
	&fm_spinand_manufacturer,
	&macronix_spinand_manufacturer,
	&other_spinand_manufacturer,
};

csi_error_t csi_spinand_match_vendor(csi_spinand_t *spinand, uint8_t *id_array)
{
    uint32_t    i,j;
	csi_spinand_info_t         *pnandinfo = NULL;
	csi_spinand_manufacturer_t *pvendor = NULL;

    for(i = 0; i < ARRAY_SIZE(spinand_manufacturers); i++){
		pvendor = (csi_spinand_manufacturer_t*)spinand_manufacturers[i];

		// /* match vendor id */
		// if(id_array[0] != pvendor->id ){
		// 	continue;
		// }

		/* match device id */
		for(j = 0; j < pvendor->nchips; j++)
		{
			pnandinfo = (csi_spinand_info_t*)(&pvendor->chips[j]);

			if(memcmp(&id_array[0],pnandinfo->devid.id,pnandinfo->devid.len)){
				continue;
			}
			
			spinand->chip_info = pnandinfo;
			spinand->maf       = pvendor;
			uint32_t buf_len;
			buf_len = pnandinfo->memorg.pagesize + pnandinfo->memorg.oobsize;
			spinand->xfer.xfer_buf = (void*)(g_nand_page_buf);
			spinand->xfer.xfer_buf_len = buf_len;
			return CSI_OK;
		}

	}

	return CSI_ERROR;
}