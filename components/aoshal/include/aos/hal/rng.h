/**
 * @file rng.h
 * @copyright Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef HAL_RNG_H
#define HAL_RNG_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup hal_rng RNG
 *  rng hal API.
 *
 *  @{
 */

#include <stdint.h>

typedef struct {
    uint8_t  port; /**< random device port */
    void    *priv; /**< priv data */
} random_dev_t;

/**
 * Fill in a memory buffer with random data
 *
 * @param[in]   random       the random device
 * @param[out]  inBuffer     Point to a valid memory buffer, this function will fill
 *                           in this memory with random numbers after executed
 * @param[in]   inByteCount  Length of the memory buffer (bytes)
 *
 * @return  0 : on success,  otherwise is error
 */
int32_t hal_random_num_read(random_dev_t random, void *buf, int32_t bytes);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* HAL_RNG_H */

