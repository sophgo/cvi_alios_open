#ifndef __TEMPSEN_H
#define __TEMPSEN_H

#include <stdint.h>
#include <stdbool.h>
#include <drv/common.h>
#include "aos/cli.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define CSI_DRV_DEBUG

#ifdef CSI_DRV_DEBUG
#define pr_debug(x, args...)                                                   \
	aos_cli_printf("[%s|%d] - " x, __func__, __LINE__, ##args)
#else
#define pr_debug(x, args...)
#endif

typedef csi_dev_t csi_tempsen_t;

/**
 * @brief Initialize tempsen interface.
 *
 * @param tps tempsen handle to operate
 * @return csi_error_t
 */
csi_error_t csi_tempsen_init(csi_tempsen_t *tps);

/**
 * @brief Uninitialize tempsen interface.
 *
 * @param tps tempsen handle to operate
 * @return csi_error_t
 */
csi_error_t csi_tempsen_uninit(csi_tempsen_t *tps);

/**
 * @brief Read temperature(m℃)
 *
 * @param tps tempsen handle to operate
 * @return temperature(m℃) on success, or csi error code
 */
int csi_tempsen_read_temp(csi_tempsen_t *tps);

/**
 * @brief Read the max temperature(m℃) since init
 *
 * @param tps tempsen handle to operate
 * @return temperature on success(m℃), or csi error code
 */
int csi_tempsen_read_max_temp(csi_tempsen_t *tps);

#ifdef CONFIG_PM

/**
  \brief Enable tempsen low power mode
  \param tps tempsen handle to operate
  \return Error code \ref csi_error_t
*/
csi_error_t csi_tempsen_enable_pm(csi_tempsen_t *tps);

/**
  \brief Disable tempsen low power mode
  \param tps tempsen handle to operate
  \return None
*/
void csi_tempsen_disable_pm(csi_tempsen_t *tps);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __TEMPSEN_H */
