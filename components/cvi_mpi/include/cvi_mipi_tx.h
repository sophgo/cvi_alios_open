#ifndef __CVI_MIPI_TX_H__
#define __CVI_MIPI_TX_H__

#include "cvi_comm_mipi_tx.h"
//#include "board_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MIPI_TX_RT_PRIO 30

struct panel_desc_s {
	char *panel_name;
	const struct combo_dev_cfg_s *dev_cfg;
	const struct hs_settle_s *hs_timing_cfg;
	const struct dsc_instr *dsi_init_cmds;
	int dsi_init_cmds_size;
};

/**
 * @brief MIPI TX Device attribute settings.
 *
 * @param fd(In), device handle.
 * @param dev_cfg(In), dev attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_Cfg(int fd, const struct combo_dev_cfg_s *dev_cfg);
/**
 * @brief Display Command Set.
 *
 * @param fd(In), device handle.
 * @param cmd_info(In), cmd info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_SendCmd(int fd, struct cmd_info_s *cmd_info);
/**
 * @brief Display Command Get.
 *
 * @param fd(In), device handle.
 * @param cmd_info(out), cmd info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_RecvCmd(int fd, struct get_cmd_info_s *cmd_info);
/**
 * @brief MIPI TX Device enable.
 *
 * @param fd(In), device handle.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_Enable(int fd);
/**
 * @brief MIPI TX Device disable.
 *
 * @param fd(In), device handle.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_Disable(int fd);
/**
 * @brief MIPI TX hs prepare/zero/trail time set.
 *
 * @param fd(In), device handle.
 * @param hs_cfg(In), hs_settle attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_SetHsSettle(int fd, const struct hs_settle_s *hs_cfg);
/**
 * @brief MIPI TX hs prepare/zero/trail time get.
 *
 * @param fd(In), device handle.
 * @param hs_cfg(out), hs_settle attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_GetHsSettle(int fd, struct hs_settle_s *hs_cfg);
/**
 * @brief MIPI TX power off.
 *
 * @param fd(In), device handle.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_POWEROFF(int fd);
/**
 * @brief MIPI TX init.
 *
 * @param dev_cfg(In), dev attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_INIT(const struct combo_dev_cfg_s *dev_cfg);
/**
 * @brief MIPI TX suspend.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_SUSPEND(void);
/**
 * @brief MIPI TX resume.
 *
 * @param dev_cfg(In), dev attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
int CVI_MIPI_TX_RESUME(const struct combo_dev_cfg_s *dev_cfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // __CVI_MIPI_TX_H__
