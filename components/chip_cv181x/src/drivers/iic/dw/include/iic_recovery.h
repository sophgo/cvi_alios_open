/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * IIC Bus Recovery Configuration Interface
 */
#ifndef __IIC_RECOVERY_H__
#define __IIC_RECOVERY_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IIC recovery pin configuration callback function types */
typedef void (*iic_recovery_pinmux_config_func_t)(void);

/* IIC recovery configuration structure */
typedef struct {
    uint8_t enable;                               /* Enable/disable recovery for this IIC */
    uint8_t scl_gpio_grp;                         /* SCL GPIO group */
    uint8_t scl_gpio_num;                         /* SCL GPIO number */
    uint8_t sda_gpio_grp;                         /* SDA GPIO group */
    uint8_t sda_gpio_num;                         /* SDA GPIO number */
    iic_recovery_pinmux_config_func_t config_gpio_mode;   /* Function to configure pins as GPIO */
    iic_recovery_pinmux_config_func_t config_iic_mode;    /* Function to configure pins as IIC */
} iic_recovery_config_t;

/**
 * @brief Set IIC recovery configuration for specified IIC bus
 * @param iic_idx IIC index
 * @param config Pointer to IIC recovery configuration
 * @return 0 if success, -1 if failed
 */
int csi_iic_set_recovery_config(uint8_t iic_idx, const iic_recovery_config_t *config);

/**
 * @brief Get IIC recovery configuration for specified IIC bus
 * @param iic_idx IIC index
 * @param config Pointer to store configuration
 * @return 0 if success, -1 if failed
 */
int csi_iic_get_recovery_config(uint8_t iic_idx, iic_recovery_config_t *config);

/**
 * @brief Enable/disable IIC recovery for specified IIC bus
 * @param iic_idx IIC index
 * @param enable 1 to enable, 0 to disable
 * @return 0 if success, -1 if failed
 */
int csi_iic_recovery_enable(uint8_t iic_idx, uint8_t enable);

/**
 * @brief Check if IIC recovery is enabled for specified IIC bus
 * @param iic_idx IIC index
 * @return 1 if enabled, 0 if disabled, -1 if error
 */
int csi_iic_recovery_is_enabled(uint8_t iic_idx);

#ifdef __cplusplus
}
#endif

#endif /* __IIC_RECOVERY_H__ */