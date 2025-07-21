#ifndef __DW9714V_H__
#define __DW9714V_H__

#include <stdint.h>

/* unit:step */
#define DW9714V_MIN_POS 0
/* unit:step */
#define DW9714V_MAX_POS 1023
#define DW9714V_IIC_ADDR 0x0C

#define DW9714V_REG_IC_INFO_ADDR    0x00
#define DW9714V_REG_IC_VER_ADDR     0x01
#define DW9714V_REG_CONTROL_ADDR    0x02
#define DW9714V_REG_VCM_MSB_ADDR    0x03
#define DW9714V_REG_VCM_LSB_ADDR    0x04
#define DW9714V_REG_STATUS_ADDR     0x05
#define DW9714V_REG_SAC_CFG_ADDR    0x06
#define DW9714V_REG_PRESC_ADDR      0x07
#define DW9714V_REG_SACT_ADDR       0x08
#define DW9714V_REG_PRESET_ADDR     0x09
#define DW9714V_REG_NRC_ADDR        0x0A

/**
 * @brief motor init
 *
 * @param i2c_num 0~4
 * @return int32_t
 */
int32_t dw9714v_init(uint8_t i2c_num);
/**
 * @brief motor deinit
 *
 * @param i2c_num 0~4
 * @return int32_t
 */
int32_t dw9714v_deinit(uint8_t i2c_num);
/**
 * @brief motor clockwise
 *
 * @param step 0~1023
 * @return int32_t
 */
int32_t dw9714v_cw(uint32_t step);
/**
 * @brief motor Counter-clockwise
 *
 * @param step 0~1023
 * @return int32_t
 */
int32_t dw9714v_ccw(uint32_t step);
/**
 * @brief
 *
 * @param pos uint: step
 * @return int32_t
 */
int32_t dw9714v_get_pos(uint32_t* pos);
#endif