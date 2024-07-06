#include <aos/cli.h>
#include <aos/kernel.h>
#include <stdio.h>
#include "board.h"
//#include "cvi_sys.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "debug/debug_dumpsys.h"
#include "drv/spi.h"
#include "drv/spiflash.h"
#include "yoc/partition.h"
#include "yoc/partition_device.h"


static int usbd_cdc_update_flag_write(int argc, char** argv)
{
    int ret = 0;
    int writesize = 0;
    csi_spiflash_t spiflash_handle;
    csi_spiflash_info_t spiflash_info;
    uint8_t* env_buffer = NULL;

    ret = csi_spiflash_spi_init(&spiflash_handle, 0, NULL);
    if (ret != 0) {
        printf("csi_spiflash_spi_init failed\n");
        return ret;
    }
    ret = csi_spiflash_get_flash_info(&spiflash_handle, &spiflash_info);
    if (ret != 0) {
        printf("csi_spiflash_get_flash_info failed\n");
        return ret;
    }
    // read env data
    env_buffer = (uint8_t*)malloc(spiflash_info.page_size);
    if (!env_buffer) {
        printf("Failed to allocate memory for size %u\n", spiflash_info.page_size);
        return ret;
    }
    memset(env_buffer, 0, spiflash_info.page_size);
    env_buffer[1] = 1;
    writesize = csi_spiflash_program(&spiflash_handle,
                                     spiflash_info.flash_size - spiflash_info.sector_size,
                                     env_buffer,
                                     spiflash_info.page_size);
    if (writesize != spiflash_info.page_size) {
        printf("nor flash program failed!\n");
        ret = -1;
    }

    free(env_buffer);
    csi_spiflash_spi_uninit(&spiflash_handle);
    return ret;
}

static int usbd_cdc_update_flag_read(int argc, char** argv)
{
    int ret = 0;
    csi_spiflash_t spiflash_handle;
    csi_spiflash_info_t spiflash_info;
    uint8_t* env_buffer = NULL;

    ret = csi_spiflash_spi_init(&spiflash_handle, 0, NULL);
    if (ret != 0) {
        printf("csi_spiflash_spi_init failed\n");
        return ret;
    }
    ret = csi_spiflash_get_flash_info(&spiflash_handle, &spiflash_info);
    if (ret != 0) {
        printf("csi_spiflash_get_flash_info failed\n");
        return ret;
    }
    printf(" flash_id :%d\n flash_size :%d\n xip_addr :%d\n sector_size :%d\n page_size :%d\n",
           spiflash_info.flash_id,
           spiflash_info.flash_size,
           spiflash_info.xip_addr,
           spiflash_info.sector_size,
           spiflash_info.page_size);
    // read env data
    env_buffer = (uint8_t*)malloc(spiflash_info.page_size);
    if (!env_buffer) {
        printf("Failed to allocate memory for size %u\n", spiflash_info.page_size);
        return ret;
    }
    memset(env_buffer, 0, spiflash_info.page_size);

    ret = csi_spiflash_read(&spiflash_handle,
                            spiflash_info.flash_size - spiflash_info.sector_size,
                            env_buffer,
                            spiflash_info.page_size);
    if (ret < 0) {
        printf("csi_spiflash_read failed with %d\n", ret);
        return ret;
    }
    printf(" cdc update flag :%d\n\n", env_buffer[1]);

    free(env_buffer);
    csi_spiflash_spi_uninit(&spiflash_handle);
    return ret;
}

static int usbd_cdc_update_flag_erase(int argc, char** argv)
{
    int ret = 0;
    csi_spiflash_t spiflash_handle;
    csi_spiflash_info_t spiflash_info;

    ret = csi_spiflash_spi_init(&spiflash_handle, 0, NULL);
    if (ret != 0) {
        printf("csi_spiflash_spi_init failed\n");
        return ret;
    }
    ret = csi_spiflash_get_flash_info(&spiflash_handle, &spiflash_info);
    if (ret != 0) {
        printf("csi_spiflash_get_flash_info failed\n");
        return ret;
    }
    printf(" flash_id :%d\n flash_size :%d\n xip_addr :%d\n sector_size :%d\n page_size :%d\n",
           spiflash_info.flash_id,
           spiflash_info.flash_size,
           spiflash_info.xip_addr,
           spiflash_info.sector_size,
           spiflash_info.page_size);
    ret = csi_spiflash_erase(&spiflash_handle,
                             spiflash_info.flash_size - spiflash_info.sector_size,
                             spiflash_info.sector_size);
    if (ret < 0) {
        printf("csi_spiflash_erase failed\n");
        return ret;
    }
    csi_spiflash_spi_uninit(&spiflash_handle);
    return ret;
}

ALIOS_CLI_CMD_REGISTER(usbd_cdc_update_flag_write, update_flag_w, write update flag);
ALIOS_CLI_CMD_REGISTER(usbd_cdc_update_flag_read, update_flag_r, read update flag);
ALIOS_CLI_CMD_REGISTER(usbd_cdc_update_flag_erase, update_flag_e, erase update flag);