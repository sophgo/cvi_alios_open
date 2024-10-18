#include "gpio_util.h"
#include "cvi_util.h"

#define GPIO_PIN_MASK(_u8_gpio_num) (1 << _u8_gpio_num)

/**
 * @brief  Set specific GPIO pin level
 * @note
 * @param  u8_gpio_grp: GPIO group num
 * @param  u8_gpio_num: GPIO offset in group
 * @param  u8_level: value to be set
 * @retval None
 */
void cvi_gpio_set_val(uint8_t u8_gpio_grp, uint8_t u8_gpio_num, cvi_gpio_level_e e_level)
{
    csi_error_t st_ret;
    csi_gpio_t st_gpio = {0};

    st_ret = csi_gpio_init(&st_gpio, u8_gpio_grp);
    if (st_ret != CSI_OK) {
        LOG_UTIL(UTIL_ERRO, "csi_gpio_init FAIL, ret vale = %d\n", st_ret);
        return;
    }

    st_ret = csi_gpio_dir(&st_gpio, GPIO_PIN_MASK(u8_gpio_num), GPIO_DIRECTION_OUTPUT);
    if (st_ret != CSI_OK) {
        LOG_UTIL(UTIL_ERRO, "csi_gpio_dir FAIL, ret vale = %d\n", st_ret);
        return;
    }
    csi_gpio_write(&st_gpio, GPIO_PIN_MASK(u8_gpio_num), e_level);
    csi_gpio_uninit(&st_gpio);
}

/**
 * @brief Get specific GPIO pin level
 * @note
 * @param  u8_gpio_grp: GPIO group num
 * @param  u8_gpio_num: GPIO offset in group
 * @retval GPIO pin level
 */
void cvi_gpio_get_val(uint8_t u8_gpio_grp, uint8_t u8_gpio_num, cvi_gpio_level_e* e_level)
{
    csi_error_t st_ret;
    csi_gpio_t st_gpio = {0};

    st_ret = csi_gpio_init(&st_gpio, u8_gpio_grp);
    if (st_ret != CSI_OK) {
        LOG_UTIL(UTIL_ERRO, "csi_gpio_init FAIL, ret vale = %d\n", st_ret);
        return;
    }

    st_ret = csi_gpio_dir(&st_gpio, GPIO_PIN_MASK(u8_gpio_num), GPIO_DIRECTION_INPUT);
    if (st_ret != CSI_OK) {
        LOG_UTIL(UTIL_ERRO, "csi_gpio_dir FAIL, ret vale = %d\n", st_ret);
        return;
    }

    *e_level = csi_gpio_read(&st_gpio, GPIO_PIN_MASK(u8_gpio_num));
    csi_gpio_uninit(&st_gpio);

    return;
}