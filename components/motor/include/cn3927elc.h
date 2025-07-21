#ifndef __CN3927ELC_H__
#define __CN3927ELC_H__
#include <stdint.h>

/* unit:step */
#define CN3927ELC_MIN_POS 0
/* unit:step */
#define CN3927ELC_MAX_POS 1023
#define CN3927ELC_IIC_ADDR 0x0C

/**
 * @brief motor init
 *
 * @param i2c_num 0~4
 * @return int32_t
 */
int32_t cn3927elc_init(uint8_t i2c_num);
/**
 * @brief motor deinit
 *
 * @param i2c_num 0~4
 * @return int32_t
 */
int32_t cn3927elc_deinit(uint8_t i2c_num);
/**
 * @brief motor clockwise
 *
 * @param step 0~1023
 * @return int32_t
 */
int32_t cn3927elc_cw(uint32_t step);
/**
 * @brief motor Counter-clockwise
 *
 * @param step 0~1023
 * @return int32_t
 */
int32_t cn3927elc_ccw(uint32_t step);
/**
 * @brief
 *
 * @param pos uint: step
 * @return int32_t
 */
int32_t cn3927elc_get_pos(uint32_t* pos);
#endif