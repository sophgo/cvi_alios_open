/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_mipi.h
 * Description:
 *   Common cif definitions.
 */
#ifndef __CVI_MIPI_H__
#define __CVI_MIPI_H__

#include "cvi_comm_cif.h"
#include "cvi_type.h"

CVI_S32 CVI_MIPI_SetMipiReset(CVI_S32 devno, CVI_U32 reset);
CVI_S32 CVI_MIPI_SetSensorClock(CVI_S32 devno, CVI_U32 enable);
CVI_S32 CVI_MIPI_SetSensorReset(CVI_S32 devno, CVI_U32 reset_port, CVI_U32 reset_pin,
								CVI_U32 reset_pol, CVI_U32 reset_enable);
CVI_S32 CVI_MIPI_SetMipiAttr(CVI_S32 ViPipe, const CVI_VOID *devAttr);
CVI_S32 CVI_MIPI_SetClkEdge(CVI_S32 devno, CVI_U32 is_up);
CVI_S32 CVI_MIPI_SetSnsMclk(struct mclk_pll_s *mclk);


#endif