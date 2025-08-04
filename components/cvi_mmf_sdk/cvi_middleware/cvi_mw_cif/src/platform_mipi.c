#include "platform_mipi.h"
#include "mipi_ioctl.h"


#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

CVI_S32 platform_mipi_SetSensorReset(CVI_S32 devno, CVI_U32 reset_port, CVI_U32 reset_pin, CVI_U32 reset_pol, CVI_U32 reset_enable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	sns_rst_config rst_config;

	rst_config.devno = devno;
	rst_config.gpio_port = reset_port;
	rst_config.gpio_pin = reset_pin;
	rst_config.gpio_active = reset_pol;

	s32Ret = mipi_ioctl_reset_snsr_gpio(&rst_config, reset_enable);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;

}

CVI_S32 platform_mipi_SetMipiReset(CVI_U32 devNo, CVI_U32 reset)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = mipi_ioctl_reset_mipi(devNo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;

}

CVI_S32 platform_mipi_SetMipiAttr(CVI_S32 ViPipe, const CVI_VOID *devAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct combo_dev_attr_s *comboAttr;

	MOD_CHECK_NULL_PTR(CVI_ID_SENSOR, devAttr);

	comboAttr = (struct combo_dev_attr_s *)devAttr;

	s32Ret = mipi_ioctl_set_dev_attr(comboAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;

}

CVI_S32 platform_mipi_SetSensorClock(CVI_U32 devNo, CVI_U32 clkEnable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = mipi_ioctl_enable_snsr_clk(devNo, clkEnable);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;

}

CVI_S32 platform_mipi_SetSnsMclk(struct mclk_pll_s *mclk)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	MOD_CHECK_NULL_PTR(CVI_ID_SENSOR, mclk);

	s32Ret = mipi_ioctl_set_snsr_mclk(mclk);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;

}

CVI_S32 platform_mipi_SetClkEdge(CVI_S32 devno, CVI_U32 is_up)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct clk_edge_s clk;

	clk.devno = devno;
	clk.edge = is_up ? CLK_UP_EDGE : CLK_DOWN_EDGE;

	s32Ret = mipi_ioctl_set_output_clk_edge(&clk);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;

}