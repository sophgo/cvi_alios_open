#ifndef _DSI_PANELS_
#define _DSI_PANELS_

#include "cvi_type.h"
#include "cvi_mipi_tx.h"
#include "cvi_comm_mipi_tx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

struct panel_desc_s {
	char *panel_name;
	struct combo_dev_cfg_s *dev_cfg;
	const struct hs_settle_s *hs_timing_cfg;
	const struct dsc_instr *dsi_init_cmds;
	int dsi_init_cmds_size;
};

#if CONFIG_PANEL_ILI9488
#include "dsi_ili9488.h"
static struct panel_desc_s panel_desc = {
	.panel_name = "ILI9488-320x480",
	.dev_cfg = &dev_cfg_ili9488_320x480,
	.hs_timing_cfg = &hs_timing_cfg_ili9488_320x480,
	.dsi_init_cmds = dsi_init_cmds_ili9488_320x480,
	.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_ili9488_320x480)
};
#elif CONFIG_PANEL_ST7701S
#include "dsi_st7701.h"
static struct panel_desc_s panel_desc = {
	.panel_name = "ST7701-480x800",
	.dev_cfg = &dev_cfg_st7701_480x800,
	.hs_timing_cfg = &hs_timing_cfg_st7701_480x800,
	.dsi_init_cmds = dsi_init_cmds_st7701_480x800,
	.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_st7701_480x800)
};
#elif CONFIG_PANEL_ST7701_480x640
#include "dsi_st7701_480x640.h"
static struct panel_desc_s panel_desc = {
	.panel_name = "ST7701-480x640",
	.dev_cfg = &dev_cfg_st7701_480x640,
	.hs_timing_cfg = &hs_timing_cfg_st7701_480x640,
	.dsi_init_cmds = dsi_init_cmds_st7701_480x640,
	.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_st7701_480x640)
};
#elif CONFIG_PANEL_ST7703
#include "dsi_st7703.h"
static struct panel_desc_s panel_desc = {
	.panel_name = "ST7703-640x480",
	.dev_cfg = &dev_cfg_st7703_640x480,
	.hs_timing_cfg = &hs_timing_cfg_st7703_640x480,
	.dsi_init_cmds = dsi_init_cmds_st7703_640x480,
	.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_st7703_640x480)
};
#elif CONFIG_PANEL_HX8394
#include "dsi_hx8394_evb.h"
static struct panel_desc_s panel_desc = {
	.panel_name = "HX8394-720x1280",
	.dev_cfg = &dev_cfg_hx8394_720x1280,
	.hs_timing_cfg = &hs_timing_cfg_hx8394_720x1280,
	.dsi_init_cmds = dsi_init_cmds_hx8394_720x1280,
	.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_hx8394_720x1280)
};
#elif CONFIG_PANEL_OTA7290B
#include "dsi_ota7290b_1920.h"
static struct panel_desc_s panel_desc = {
	.panel_name = "OTA7290B-440x1920",
	.dev_cfg = &dev_cfg_ota7290b_440x1920,
	.hs_timing_cfg = &hs_timing_cfg_ota7290b_440x1920,
	.dsi_init_cmds = dsi_init_cmds_ota7290b_440x1920,
	.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_ota7290b_440x1920)
};
#else
#include "dsi_hx8394_evb.h"
static struct panel_desc_s panel_desc = {
	.panel_name = "HX8394-720x1280",
	.dev_cfg = &dev_cfg_hx8394_720x1280,
	.hs_timing_cfg = &hs_timing_cfg_hx8394_720x1280,
	.dsi_init_cmds = dsi_init_cmds_hx8394_720x1280,
	.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_hx8394_720x1280)
};
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // _DSI_PANELS_
