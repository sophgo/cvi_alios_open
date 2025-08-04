#include "platform_mipi.h"
#include "cvi_comm_cif.h"
#include "cvi_sns_ctrl.h"

CVI_S32 CVI_MIPI_SetMipiReset(CVI_U32 devNo, CVI_U32 reset)
{
	return platform_mipi_SetMipiReset(devNo, reset);
}

CVI_S32 CVI_MIPI_SetSensorClock(CVI_U32 devNo, CVI_U32 clkEnable)
{
	return platform_mipi_SetSensorClock(devNo, clkEnable);
}

CVI_S32 CVI_MIPI_SetSensorReset(CVI_S32 devno, CVI_U32 reset_port,
				CVI_U32 reset_pin, CVI_U32 reset_pol, CVI_U32 reset_enable)
{
	return platform_mipi_SetSensorReset(devno, reset_port, reset_pin, reset_pol, reset_enable);
}

CVI_S32 CVI_MIPI_SetMipiAttr(CVI_S32 ViPipe, const CVI_VOID *devAttr)
{
	return platform_mipi_SetMipiAttr(ViPipe, devAttr);
}

CVI_S32 CVI_MIPI_SetClkEdge(CVI_S32 devno, CVI_U32 is_up)
{
	return platform_mipi_SetClkEdge(devno, is_up);
}

CVI_S32 CVI_MIPI_SetSnsMclk(struct mclk_pll_s *mclk)
{
	return platform_mipi_SetSnsMclk(mclk);
}