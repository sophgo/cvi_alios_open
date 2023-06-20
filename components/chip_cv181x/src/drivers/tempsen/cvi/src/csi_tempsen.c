#include "cvi_tempsen.h"

csi_error_t csi_tempsen_init(csi_tempsen_t *tps)
{
	CSI_PARAM_CHK(tps, CSI_ERROR);

	if (target_get(DEV_DW_TEMPSEN_TAG, 0, tps) != CSI_OK) {
		pr_debug("tempsen init failed!\n");
		return CSI_ERROR;
	}
	return cvi_tempsen_init(tps);
}

csi_error_t csi_tempsen_uninit(csi_tempsen_t *tps)
{
	CSI_PARAM_CHK(tps, CSI_ERROR);

	return cvi_tempsen_uninit(tps);
}

int csi_tempsen_read_temp(csi_tempsen_t *tps)
{
	CSI_PARAM_CHK(tps, CSI_ERROR);

	// 1s timeout
	return cvi_tempsen_read_temp(tps, 1000);
}

int csi_tempsen_read_max_temp(csi_tempsen_t *tps)
{
	CSI_PARAM_CHK(tps, CSI_ERROR);

	return cvi_tempsen_read_max_temp(tps);
}

#ifdef CONFIG_PM
csi_error_t dw_tempsen_pm_action(csi_dev_t *dev, csi_pm_dev_action_t action)
{
	CSI_PARAM_CHK(dev, CSI_ERROR);

	csi_error_t   ret = CSI_OK;
	csi_pm_dev_t *pm_dev = &dev->pm_dev;

	switch (action) {
	case PM_DEV_SUSPEND:
		cvi_tempsen_init(dev);
		break;

	case PM_DEV_RESUME:
		cvi_tempsen_uninit(dev);
		break;

	default:
		ret = CSI_ERROR;
		break;
	}

	return ret;
}

csi_error_t csi_tempsen_enable_pm(csi_tempsen_t *tps)
{
	return csi_pm_dev_register(tps, dw_tempsen_pm_action, 0U, 0U);
}

void csi_tempsen_disable_pm(csi_tempsen_t *tps)
{
	csi_pm_dev_unregister(tps);
}
#endif