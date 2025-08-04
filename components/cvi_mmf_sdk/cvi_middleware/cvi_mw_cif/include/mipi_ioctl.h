#ifndef MODULES_VPU_INCLUDE_CIF_IOCTL_H_
#define MODULES_VPU_INCLUDE_CIF_IOCTL_H_

#include "cvi_type.h"
#include "cvi_comm_cif.h"

int mipi_ioctl_reset_snsr_gpio(sns_rst_config *rst_config, CVI_U32 rstEnable);
int mipi_ioctl_reset_mipi(CVI_U32 devNo);
int mipi_ioctl_set_dev_attr(struct combo_dev_attr_s *stDevAttr);
int mipi_ioctl_enable_snsr_clk(CVI_U32 devNo, CVI_U32 clkEnable);
int mipi_ioctl_set_snsr_mclk(struct mclk_pll_s *mclk);
int mipi_ioctl_set_output_clk_edge(struct clk_edge_s *clk);

#endif
