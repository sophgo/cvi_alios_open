#ifndef __MOTOR_H_
#define __MOTOR_H_
#include <stdint.h>

typedef enum _MOTOR_RESULT_E{
    MOTOR_RESULT_SCUCESS,
    MOTOR_RESULT_ERROR,
}MOTOR_RESULT_S;

/**
 * @brief motor type
 *
 */
typedef enum _MOTOR_TYPE_E{
    MOTOR_TYPE_NONE,
    MOTOR_TYPE_CN3927ELC,
    MOTOR_TYPE_DW9714V,
    MOTOR_TYPE_BUIT,
}MOTOR_TYPE_S;

/**
 * @brief
 *
 * @param motor_type
 * @param bus_id
 * @return int32_t
 */
int32_t motor_init(MOTOR_TYPE_S motor_type, uint8_t bus_id);
/**
 * @brief
 *
 * @param motor_type
 * @param bus_id
 * @return int32_t
 */
int32_t motor_deinit(MOTOR_TYPE_S motor_type, uint8_t bus_id);
/**
 * @brief
 *
 * @param motor_type
 * @param step
 * @return int32_t
 */
int32_t motor_cw(MOTOR_TYPE_S motor_type, uint32_t step);
/**
 * @brief
 *
 * @param motor_type
 * @param step
 * @return int32_t
 */
int32_t motor_ccw(MOTOR_TYPE_S motor_type, uint32_t step);
/**
 * @brief
 *
 * @param motor_type
 * @param pos
 * @return int32_t
 */
int32_t motor_get_pos(MOTOR_TYPE_S motor_type, uint32_t* pos);
#endif