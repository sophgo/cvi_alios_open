#ifndef __PLATFORM_MIPI_H__
#define __PLATFORM_MIPI_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <inttypes.h>
#include "cvi_sns_ctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

CVI_S32 platform_mipi_SetSensorReset(CVI_S32 devno, CVI_U32 reset_port, CVI_U32 reset_pin, CVI_U32 reset_pol, CVI_U32 reset_enable);
CVI_S32 platform_mipi_SetMipiReset(CVI_U32 devNo, CVI_U32 reset);
CVI_S32 platform_mipi_SetMipiAttr(CVI_S32 ViPipe, const CVI_VOID *devAttr);
CVI_S32 platform_mipi_SetSensorClock(CVI_U32 devNo, CVI_U32 clkEnable);
CVI_S32 platform_mipi_SetSnsMclk(struct mclk_pll_s *mclk);
CVI_S32 platform_mipi_SetClkEdge(CVI_S32 devno, CVI_U32 is_up);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
