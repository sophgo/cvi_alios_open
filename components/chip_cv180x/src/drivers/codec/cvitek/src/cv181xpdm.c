// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * CVITEK CV181x PDM driver
 *
 * Copyright 2020 CVITEK Inc.
 *
 */


#include "cv181x_pdm.h"
#include <stdio.h>
#include <mmio.h>

static struct cv181xpdm pdm;
static struct cv181xpdm *p_pdm;
/*static void writel(u32 val, volatile u32 *addr)
{
    *addr = val;
}

static u32 readl(volatile u32 *addr)
{
   return *addr;
}
*/
static inline void pdm_write_reg(unsigned long io_base, u32 reg, u32 val)
{
	mmio_write_32((uintptr_t)(io_base + reg), val);
}

static inline u32 pdm_read_reg(unsigned long io_base, int reg)
{
	return mmio_read_32((uintptr_t)(io_base + reg));
}

static u32 cv181xpdm_hw_params(struct cv181xpdm *pdm, u32 rate)
{

	u32 pdm_setting = pdm_read_reg(pdm->pdm_base, PDM_SETTING_REG);
	u32 i2s_setting = pdm_read_reg(pdm->pdm_base, I2S_SETTING_REG);

	if (rate >= 8000 && rate <= 192000) {
		debug("%s, set rate to %d\n", __func__, rate);

		switch (rate) {
		case 48000:
			pdm_write_reg(pdm->pdm_base, PDM_SETTING_REG, (pdm_setting & PDM_CLK_DIV_MASK) | PDM_48K);
			pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG, (i2s_setting & I2S_CLK_DIV_MASK) | I2S_48K);
			debug("%s, set sample rate with 48KHz\n", __func__);
			break;
		case 44100:
			pdm_write_reg(pdm->pdm_base, PDM_SETTING_REG, (pdm_setting & PDM_CLK_DIV_MASK) | PDM_44_1K);
			pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG, (i2s_setting & I2S_CLK_DIV_MASK) | I2S_44_1K);
			debug("%s, set sample rate with 44.1KHz\n", __func__);
			break;
		case 32000:
			pdm_write_reg(pdm->pdm_base, PDM_SETTING_REG, (pdm_setting & PDM_CLK_DIV_MASK) | PDM_32K);
			pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG, (i2s_setting & I2S_CLK_DIV_MASK) | I2S_32K);
			debug("%s, set sample rate with 32KHz\n", __func__);
			break;
		case 22050:
			pdm_write_reg(pdm->pdm_base, PDM_SETTING_REG, (pdm_setting & PDM_CLK_DIV_MASK) | PDM_22_05K);
			pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG, (i2s_setting & I2S_CLK_DIV_MASK) | I2S_22_05K);
			debug("%s, set sample rate with 22.05KHz\n", __func__);
			break;
		case 16000:
			pdm_write_reg(pdm->pdm_base, PDM_SETTING_REG, (pdm_setting & PDM_CLK_DIV_MASK) | PDM_16K);
			pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG, (i2s_setting & I2S_CLK_DIV_MASK) | I2S_16K);
			debug("%s, set sample rate with 16KHz\n", __func__);
			break;
		case 11025:
			pdm_write_reg(pdm->pdm_base, PDM_SETTING_REG, (pdm_setting & PDM_CLK_DIV_MASK) | PDM_11_025K);
			pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG, (i2s_setting & I2S_CLK_DIV_MASK) | I2S_11_025K);
			debug("%s, set sample rate with 11.025KHz\n", __func__);
			break;
		case 8000:
			pdm_write_reg(pdm->pdm_base, PDM_SETTING_REG, (pdm_setting & PDM_CLK_DIV_MASK) | PDM_8K);
			pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG, (i2s_setting & I2S_CLK_DIV_MASK) | I2S_8K);
			debug("%s, set sample rate with 8KHz\n", __func__);
			break;
		default:
			pdm_write_reg(pdm->pdm_base, PDM_SETTING_REG, (pdm_setting & PDM_CLK_DIV_MASK) | PDM_8K);
			pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG, (i2s_setting & I2S_CLK_DIV_MASK) | I2S_8K);
			debug("%s, set sample rate with default 48KHz\n", __func__);
			break;
		}
	} else {
		debug("Rate: %d is not supported\n", rate);
		return 0;
	}

	i2s_setting = pdm_read_reg(pdm->pdm_base, I2S_SETTING_REG);
	pdm_write_reg(pdm->pdm_base, I2S_SETTING_REG,
		      (i2s_setting & I2S_CHN_WIDTH_MASK) | I2S_CHN_WIDTH(I2S_CHN_WIDTH_16BIT));

	return 0;
}

static u32 cv181xpdm_startup(struct cv181xpdm *pdm)
{
	pdm_write_reg(pdm->pdm_base, PDM_EN_REG, PDM_EN);
	//mmio_write_32(0x041d0c30, 0x1);
	//printf("%s, val:%d\n", __func__, pdm_read_reg(pdm->pdm_base, PDM_EN_REG));
    //printf("1234===0x%x\n", mmio_read_32(0x041d0c30));
	return 0;
}

static void cv181xpdm_shutdown(struct cv181xpdm *pdm)
{
	pdm_write_reg(pdm->pdm_base, PDM_EN_REG, PDM_OFF);

	printf("%s\n", __func__);
}

int cv181xpdm_init(u32 rate)
{
    u64 mclk_source_addr = 0x0;
    //printf("%s,%d start \n", __func__, __LINE__);
	p_pdm = &pdm;
    p_pdm->pdm_base = (unsigned long)(0x041d0c00);
    mclk_source_addr = 0x04110000;
    if (mclk_source_addr)
        p_pdm->clk = (unsigned long)(mclk_source_addr);
    else
        debug("get MCLK source failed !!\n");
    cv181xpdm_hw_params(p_pdm, rate);
	cv181xpdm_startup(p_pdm);
    return 0;
}
int cv181xpdm_deinit(void)
{
	p_pdm = &pdm;
	cv181xpdm_shutdown(p_pdm);
	return 0;
}


