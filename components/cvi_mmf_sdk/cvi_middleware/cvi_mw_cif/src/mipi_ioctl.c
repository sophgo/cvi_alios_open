#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "osal.h"
#include "osal_ioctl.h"
#include "mipi_ioctl.h"
#include "driver_cif.h"
#include "cvi_type.h"

int mipi_ioctl_reset_snsr_gpio(sns_rst_config *rst_config, CVI_U32 rstEnable)
{
	CVI_S32 ret;

	sns_rst_config cfg;

	osal_memset(&cfg, 0, sizeof(sns_rst_config));

	osal_memcpy(&cfg, rst_config, sizeof(cfg));

	if (rstEnable) {
		ret = driver_cif_ioctl(CVI_MIPI_RESET_SENSOR, (unsigned long)&cfg);
	} else {
		ret = driver_cif_ioctl(CVI_MIPI_UNRESET_SENSOR, (unsigned long)&cfg);
	}

	return ret;
}

int mipi_ioctl_reset_mipi(CVI_U32 devNo)
{
	CVI_S32 ret;

	ret = driver_cif_ioctl(CVI_MIPI_RESET_MIPI, (unsigned long)&devNo);

	return ret;
}

int mipi_ioctl_set_dev_attr(struct combo_dev_attr_s *stDevAttr)
{
	CVI_S32 ret;

	struct combo_dev_attr_s cfg;

	osal_memset(&cfg, 0, sizeof(struct combo_dev_attr_s));

	osal_memcpy(&cfg, stDevAttr, sizeof(cfg));

	ret = driver_cif_ioctl(CVI_MIPI_SET_DEV_ATTR, (unsigned long)&cfg);

	return ret;
}

int mipi_ioctl_enable_snsr_clk(CVI_U32 devNo, CVI_U32 clkEnable)
{
	CVI_S32 ret;

	if (clkEnable) {
		ret = driver_cif_ioctl(CVI_MIPI_ENABLE_SENSOR_CLOCK, (unsigned long)&devNo);
	} else {
		ret = driver_cif_ioctl(CVI_MIPI_DISABLE_SENSOR_CLOCK, (unsigned long)&devNo);
	}

	return ret;
}

int mipi_ioctl_set_snsr_mclk(struct mclk_pll_s *mclk)
{
	CVI_S32 ret;

	struct mclk_pll_s cfg;

	osal_memset(&cfg, 0, sizeof(struct mclk_pll_s));

	osal_memcpy(&cfg, mclk, sizeof(cfg));

	ret = driver_cif_ioctl(CVI_MIPI_SET_SENSOR_CLOCK, (unsigned long)&cfg);

	return ret;
}

int mipi_ioctl_set_output_clk_edge(struct clk_edge_s *clk)
{
	CVI_S32 ret;

	struct clk_edge_s cfg;

	osal_memset(&cfg, 0, sizeof(struct clk_edge_s));

	osal_memcpy(&cfg, clk, sizeof(cfg));

	ret = driver_cif_ioctl(CVI_MIPI_SET_OUTPUT_CLK_EDGE, (unsigned long)&cfg);

	return ret;
}