#include "cvi_tempsen.h"
#include "hal_tempsen.h"

cvi_error_t cvi_tempsen_init(cvi_tempsen_t *tps)
{
	if (!tps->reg_base)
		return CVI_ERROR;
	set_hal_tempsen_base(tps->reg_base);
	tempsen_init();

	return CVI_OK;
}

cvi_error_t cvi_tempsen_uninit(cvi_tempsen_t *tps)
{
	if (!tps->reg_base)
		return CVI_ERROR;
	tempsen_uninit();
	set_hal_tempsen_base(0UL);
	return CVI_OK;
}

int cvi_tempsen_read_temp(cvi_tempsen_t *tps, unsigned int timeout_ms)
{
	if (!get_hal_tempsen_base())
		return CVI_ERROR;

	unsigned int temp, result;
	if (wait_for_finish(timeout_ms)) {
		return CVI_TIMEOUT;
	}
	temp = read_temp();
	// calculate temperature (m℃)
	result = (temp * 1000) * 716 / 2048 - 273000;
	pr_debug("temp regval:0x%x, temp:%d mC\n", temp, result);
	return result;
}

int cvi_tempsen_read_max_temp(cvi_tempsen_t *tps)
{
	if (!get_hal_tempsen_base())
		return CVI_ERROR;
	unsigned int temp, result;
	temp = read_max_temp();
	// calculate temperature (m℃)
	result = (temp * 1000) * 716 / 2048 - 273000;
	pr_debug("temp regval:0x%x, temp:%d mC\n", temp, result);
	return result;
}

unsigned int cvi_tempsen_test_force_val(cvi_tempsen_t *tps,
					unsigned int   force_val)
{
	return tempsen_set_force_val(force_val);
}