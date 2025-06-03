extern void driver_base_init(void);
extern void driver_sys_init();
extern void driver_cif_init();
extern void driver_snsr_i2c_init();
extern void driver_vi_init();
extern void driver_vpss_init();
extern void driver_vc_init();
extern void driver_ldc_init();
extern void driver_rgn_init();
extern void driver_vo_init();
extern void driver_mipi_tx_init();
extern void driver_mipi_tx_exit();
extern void driver_vo_exit();
extern void driver_rgn_exit();
extern void driver_ldc_exit();
extern void driver_vc_deinit();
extern void driver_vpss_exit();
extern void driver_vi_exit();
extern void driver_snsr_i2c_exit();
extern void driver_cif_exit();
extern void driver_sys_exit();
extern void driver_base_exit();

//Similar to Linux ko insmod
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

//Similar to Linux ko rmmod
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

