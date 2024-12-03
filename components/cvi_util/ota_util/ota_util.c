#include "ota_util.h"
#include "cvi_util.h"

static pthread_t ota_pthread;
static struct sched_param sch_param;
static pthread_attr_t ota_pthread_attr;
static bool b_ota_flag = 0;

/**
 * @brief  setting the ota flag
 * @note
 * @param  value: flag value
 * @retval None
 */
void cvi_ota_set_flag(int value)
{
    if ((value != 0) && (value != 1)) {
        LOG_UTIL(UTIL_ERRO, "cvi_ota_set_flag FAIL, ret value = %d\n", value);
        return ;
    }
    b_ota_flag = value;
}

/**
 * @brief  setting the ota flag
 * @note
 * @param  None
 * @retval None
 */
void cvi_ota_set_flag_cli(int argc, char** argv)
{
    b_ota_flag = 1;
}

/**
 * @brief  decide whether setting the ota flag
 * @note
 * @param  None
 * @retval None
 */
void* cvi_ota_flag_set_pthread(void* args)
{
    while (!b_ota_flag){
        aos_msleep(500);
    }
    aos_msleep(1000);
    LOG_UTIL(UTIL_DBUG, "boot_ota_pthread ---action\n");
    int ret = 0;
    int writesize = 0;
    csi_spiflash_t spiflash_handle;
    csi_spiflash_info_t spiflash_info;
    uint8_t* env_buffer = NULL;

    ret = csi_spiflash_spi_init(&spiflash_handle, 0, NULL);
    if (ret != 0) {
        LOG_UTIL(UTIL_ERRO, "csi_spiflash_spi_init FAIL, ret value = %d\n", ret);
        return NULL;
    }
    ret = csi_spiflash_get_flash_info(&spiflash_handle, &spiflash_info);
    if (ret != 0) {
        LOG_UTIL(UTIL_ERRO, "csi_spiflash_get_flash_info FAIL, ret value = %d\n", ret);
        return NULL;
    }
    LOG_UTIL(UTIL_INFO,
             " flash_id :%d\n flash_size :%d\n xip_addr :%d\n sector_size :%d\n page_size :%d\n",
             spiflash_info.flash_id, spiflash_info.flash_size, spiflash_info.xip_addr,
             spiflash_info.sector_size, spiflash_info.page_size);
    ret = csi_spiflash_erase(&spiflash_handle, spiflash_info.flash_size - spiflash_info.sector_size,
                             spiflash_info.sector_size);
    if (ret < 0) {
        LOG_UTIL(UTIL_ERRO, "csi_spiflash_erase FAIL, ret value = %d\n", ret);
        return NULL;
    }
     // read env data
    env_buffer = (uint8_t*)malloc(spiflash_info.page_size);
    if (!env_buffer) {
        LOG_UTIL(UTIL_ERRO, "malloc env buffer FAIL \n");
        return NULL;
    }
    memset(env_buffer, 0, spiflash_info.page_size);
    env_buffer[1] = 1;
    writesize = csi_spiflash_program(&spiflash_handle,
                                     spiflash_info.flash_size - spiflash_info.sector_size,
                                     env_buffer,
                                     spiflash_info.page_size);
    if (writesize != spiflash_info.page_size) {
        LOG_UTIL(UTIL_ERRO, "csi_spiflash_program FAIL \n");
        return NULL;
    }

    free(env_buffer);
    csi_spiflash_spi_uninit(&spiflash_handle);

    aos_msleep(500);

    aos_reboot();
    return NULL;
}

/**
 * @brief  Flag setting thread
 * @note
 * @param  None
 * @retval None
 */
void cvi_ota_init(void)
{
    int ret = 0;
    sch_param.sched_priority = 40;
    pthread_attr_init(&ota_pthread_attr);
    pthread_attr_setschedpolicy(&ota_pthread_attr, SCHED_RR);
    pthread_attr_setschedparam(&ota_pthread_attr, &sch_param);
    pthread_attr_setinheritsched(&ota_pthread_attr, PTHREAD_EXPLICIT_SCHED);
    ret = pthread_create(&ota_pthread, &ota_pthread_attr, cvi_ota_flag_set_pthread, NULL);

    if (ret) {
        LOG_UTIL(UTIL_ERRO, "pthread_create FAIL, ret value = %d\n", ret);
    }
    return;
}

ALIOS_CLI_CMD_REGISTER(cvi_ota_set_flag_cli, ota_set_flag, setting cdc ota flag);