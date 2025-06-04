/**
 * @file flash.h
 * @copyright Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup hal_flash FLASH
 *  flash hal API.
 *
 *  @{
 */

#include <stdint.h>

/* Define for partition_options set */
#define PAR_OPT_READ_POS  ( 0 ) /**< Read attribute bit in partition_options */
#define PAR_OPT_WRITE_POS ( 1 ) /**< write attribute bit in partition_options */

#define PAR_OPT_READ_MASK  ( 0x1u << PAR_OPT_READ_POS )     /**< Read attribute mask bit */
#define PAR_OPT_WRITE_MASK ( 0x1u << PAR_OPT_WRITE_POS )    /**< Write attribute mask bit */

#define PAR_OPT_READ_DIS  ( 0x0u << PAR_OPT_READ_POS )      /**< Read disable bit set */
#define PAR_OPT_READ_EN   ( 0x1u << PAR_OPT_READ_POS )      /**< Read enable bit set */
#define PAR_OPT_WRITE_DIS ( 0x0u << PAR_OPT_WRITE_POS )     /**< write disable bit set */
#define PAR_OPT_WRITE_EN  ( 0x1u << PAR_OPT_WRITE_POS )     /**< write enable bit set */

/* hal flash partition define */
typedef enum {
    HAL_PARTITION_ERROR = -1,
    HAL_PARTITION_BOOTLOADER,   /**< Bootloader partition index */
    HAL_PARTITION_APPLICATION,  /**< App partition index; Or OTA A partition */
    HAL_PARTITION_ATE,          /**< For ATE */
    HAL_PARTITION_OTA_TEMP,     /**< For OTA upgrade */
    HAL_PARTITION_RF_FIRMWARE,  /**< For RF firmware */
    HAL_PARTITION_PARAMETER_1,  /**< For OTA args */
    HAL_PARTITION_PARAMETER_2,  /**< For kv storage */
    HAL_PARTITION_PARAMETER_3,  /**< For User defined */
    HAL_PARTITION_PARAMETER_4,  /**< Used by security */
    HAL_PARTITION_BT_FIRMWARE,  /**< For bt firmware */
    HAL_PARTITION_SPIFFS,       /**< For spiffs file system */
    HAL_PARTITION_LITTLEFS,
    HAL_PARTITION_LITTLEFS2,
    HAL_PARTITION_LITTLEFS3,
    HAL_PARTITION_CUSTOM_1,     /**< For User defined */
    HAL_PARTITION_CUSTOM_2,     /**< For User defined */
    HAL_PARTITION_2ND_BOOT,     /**< For 2nd boot */
    HAL_PARTITION_MBINS_APP,    /**< For app bin when muti bins */
    HAL_PARTITION_MBINS_KERNEL, /**< For kernel bin when muti bins */
    HAL_PARTITION_OTA_SUB,      /**< For Sub device OTA */
    HAL_PARTITION_GPT,
    HAL_PARTITION_ENV,
    HAL_PARTITION_ENV_REDUND,
    HAL_PARTITION_RTOSA,
    HAL_PARTITION_RTOSB,
    HAL_PARTITION_BOOT1,
    HAL_PARTITION_BOOT1_REDUND,
    HAL_PARTITION_MAX,
    HAL_ALL_FLASH_0,           /**< For all flash0 operation */
    HAL_ALL_FLASH_1,           /**< For all flash1 operation */
    HAL_ALL_FLASH_2,           /**< For all flash2 operation */ 
    HAL_PARTITION_NONE,
} hal_partition_t;

typedef enum {
    HAL_FLASH_ERR_OK,           /* operation success */
    HAL_FLASH_ERR_NAND_BAD,     /* Bad block */
    HAL_FLASH_ERR_NAND_READ,    /* Read fail, can't correct */
    HAL_FLASH_ERR_NAND_WRITE,   /* Write fail */
    HAL_FLASH_ERR_NAND_ERASE,   /* Erase fail */
    HAL_FLASH_ERR_NAND_FLIPS,   /* Too many bitflips, uncorrected */
    /* add more hereafter */
} hal_flash_err_t;

/* Define for partition owner */
typedef enum {
    HAL_FLASH_EMBEDDED,
    HAL_FLASH_SPI,
    HAL_FLASH_QSPI,
    HAL_FLASH_MAX,
    HAL_FLASH_NONE,
} hal_flash_t;

/* Hal flash partition manage struct */
typedef struct {
    hal_flash_t  partition_owner;
    const char  *partition_description;
    uint32_t     partition_start_addr;
    uint32_t     partition_length;
    uint32_t     partition_options; /**< Read\write enable or disable */ //sector_count
} hal_logic_partition_t;

/**
 * Get the information of the specified flash area
 *
 * @param[in]  in_partition     The target flash logical partition
 * @param[out]  partition       The buffer to store partition info
 *
 * @return  0: On success,  otherwise is error
 */
int32_t hal_flash_info_get(hal_partition_t in_partition, hal_logic_partition_t *partition);

/**
 * Erase an area on a Flash logical partition
 *
 * @note  Erase on an address will erase all data on a sector that the
 *        address is belonged to, this function does not save data that
 *        beyond the address area but in the affected sector, the data
 *        will be lost.
 *
 * @param[in]  in_partition  The target flash logical partition which should be erased
 * @param[in]  off_set       Start address of the erased flash area
 * @param[in]  size          Size of the erased flash area
 *
 * @return  0 : On success,  otherwise is error
 */
int32_t hal_flash_erase(hal_partition_t in_partition, uint32_t off_set, uint32_t size);

/**
 * Write data to an area on a flash logical partition without erase
 *
 * @param[in]  in_partition    The target flash logical partition which should be read which should be written
 * @param[in/out]  off_set     Point to the start address that the data is written to, and
 *                             point to the last unwritten address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  inBuffer        point to the data buffer that will be written to flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success,  otherwise is error
 */
int32_t hal_flash_write(hal_partition_t in_partition, uint32_t *off_set,
                        const void *in_buf, uint32_t in_buf_len);

/**
 * Write data to an area on a flash logical partition with erase first
 *
 * @param[in]  in_partition    The target flash logical partition which should be read which should be written
 * @param[in/out]  off_set     Point to the start address that the data is written to, and
 *                             point to the last unwritten address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  inBuffer        point to the data buffer that will be written to flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success,  otherwise is error
 */
int32_t hal_flash_erase_write(hal_partition_t in_partition, uint32_t *off_set,
                              const void *in_buf, uint32_t in_buf_len);

/**
 * Read data from an area on a Flash to data buffer in RAM
 *
 * @param[in]  in_partition    The target flash logical partition which should be read
 * @param[in/out]  off_set     Point to the start address that the data is read, and
 *                             point to the last unread address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  outBuffer       Point to the data buffer that stores the data read from flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success,  otherwise is error
 */
int32_t hal_flash_read(hal_partition_t in_partition, uint32_t *off_set,
                       void *out_buf, uint32_t in_buf_len);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* HAL_FLASH_H */

