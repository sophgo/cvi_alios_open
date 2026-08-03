// SPDX-License-Identifier: Apache-2.0

#include <aos/kernel.h>
#include <string.h>
#include <stdio.h>
#include "ddrmon_api.h"

/**
 * @brief Initialize DDR monitor component
 * @note Must be called explicitly from solution app_main() — AliOS has no
 *       INIT_APP_EXPORT equivalent.
 */
int ddrmon_component_init(void)
{
#ifdef CVI_DDRMON
	printf("[ddrmon] component_init\n");
	ddrmon_params_t p;

	memset(&p, 0, sizeof(p));
#ifdef CVI_AXIMON_BASE
	p.aximon_base = (uintptr_t)(CVI_AXIMON_BASE);
#endif
#ifdef CVI_DDR_CTRL_BASE
	p.ddr_ctrl_base = (uintptr_t)(CVI_DDR_CTRL_BASE);
#endif
#ifdef CVI_DDR_TOP_BASE
	p.ddr_top_base = (uintptr_t)(CVI_DDR_TOP_BASE);
#endif
#ifdef __CV181X__
	p.soc_type = "cv181x";
#elif defined(__CV180X__)
	p.soc_type = "cv180x";
#else
	p.soc_type = "cv181x";
#endif
#ifdef CVI_DDRMON_DEFAULT_PERIOD_MS
	p.default_period_ms = CVI_DDRMON_DEFAULT_PERIOD_MS;
#else
	p.default_period_ms = 20;
#endif
	// Simplify port_mask - always enable all ports like Linux version
	p.port_mask = 0x3F;
#ifdef CVI_DDR_DATA_RATE_MHZ
	p.data_rate_mhz = (uint32_t)CVI_DDR_DATA_RATE_MHZ;
#else
	p.data_rate_mhz = 1333;
#endif
	int ret = ddrmon_init(&p);

	if (ret != DDRMON_OK) {
		printf("[ddrmon] init failed: %d\n", ret);
		return ret;
	}

	// Print startup info for quick validation (data rate and bus width)
	uint32_t rate_mhz = 0, bus_bits = 0;

	ret = ddrmon_get_info(&rate_mhz, &bus_bits);
	if (ret == DDRMON_OK) {
		printf("[ddrmon] DRAM rate=%u MT/s, bus_width=%u bits\n", rate_mhz, bus_bits);
	}

	// Register /dev/ddrmon VFS device
	ret = ddrmon_device_init();
	if (ret != 0) {
		printf("[ddrmon] device init failed: %d\n", ret);
		ddrmon_deinit();
		return ret;
	}
#endif
	return DDRMON_OK;
}
