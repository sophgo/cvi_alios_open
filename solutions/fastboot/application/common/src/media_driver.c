#include "driver_base.h"
#include "driver_sys.h"
#include "driver_vpss.h"
#include "driver_vi.h"
#include "driver_vc.h"
#include "driver_ldc.h"
#include "driver_rgn.h"
#include "driver_cif.h"
#include "driver_snsr_i2c.h"
#include "driver_vo.h"
#include "driver_mipi_tx.h"


void media_driver_init(void)
{
	driver_base_init();
	driver_sys_init();
	driver_cif_init();
	driver_snsr_i2c_init();
	driver_vi_init();
	driver_vpss_init();
	driver_vc_init();
	driver_ldc_init();
	driver_rgn_init();
	driver_vo_init();
	driver_mipi_tx_init();
}

void media_driver_exit(void)
{
	driver_mipi_tx_exit();
	driver_vo_exit();
	driver_rgn_exit();
	driver_ldc_exit();
	driver_vc_deinit();
	driver_vpss_exit();
	driver_vi_exit();
	driver_snsr_i2c_exit();
	driver_cif_exit();
	driver_sys_exit();
	driver_base_exit();
}

