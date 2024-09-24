/* SPDX-License-Identifier: GPL-2.0-or-later
 * CV181x ADC driver on CVITEK CV181x
 *
 * Copyright 2022 CVITEK
 *
 * Author: Ruilong.Chen
 *
 */


#include "cv181x_adc_dac.h"

static struct cv182xadc g_adc;
static struct cv182xadc *adc;

static int adc_vol_list[25] = {
    ADC_VOL_GAIN_0,
    ADC_VOL_GAIN_1,
    ADC_VOL_GAIN_2,
    ADC_VOL_GAIN_3,
    ADC_VOL_GAIN_4,
    ADC_VOL_GAIN_5,
    ADC_VOL_GAIN_6,
    ADC_VOL_GAIN_7,
    ADC_VOL_GAIN_8,
    ADC_VOL_GAIN_9,
    ADC_VOL_GAIN_10,
    ADC_VOL_GAIN_11,
    ADC_VOL_GAIN_12,
    ADC_VOL_GAIN_13,
    ADC_VOL_GAIN_14,
    ADC_VOL_GAIN_15,
    ADC_VOL_GAIN_16,
    ADC_VOL_GAIN_17,
    ADC_VOL_GAIN_18,
    ADC_VOL_GAIN_19,
    ADC_VOL_GAIN_20,
    ADC_VOL_GAIN_21,
    ADC_VOL_GAIN_22,
    ADC_VOL_GAIN_23,
    ADC_VOL_GAIN_24
};

u32 old_adc_voll;
u32 old_adc_volr;

static void writel(u32 val, volatile u32 *addr)
{
    *addr = val;
}

static u32 readl(volatile u32 *addr)
{
   return *addr;
}

static int cv182xadc_hw_params(struct cv182xadc *adc, u32 rate)
{

	debug("%s start %d \n",__func__, __LINE__);
	u32 ctrl1 = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1) & ~AUDIO_PHY_REG_RXADC_CIC_OPT_MASK;
	u32 ana3 = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA3) & ~AUDIO_PHY_REG_CTUNE_RXADC_MASK;
	u32 ana0;
	u32 clk = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CLK) &
			~(AUDIO_RXADC_SCK_DIV_MASK | AUDIO_RXADC_DLYEN_MASK);
	//u32 spare0 = adc_read_reg(adc->adc_base, AUDIO_PHY_SPARE_0) & ~AUDIO_ADC_SCK_DIV_MASK;
	volatile u32 *dac;

	/* ECO function, register naming is not corrected, use ioremap to access register of DAC */
	dac = (volatile u32 *)(0x0300A000);
	ana0 = readl(dac + AUDIO_PHY_TXDAC_ANA0) & ~AUDIO_PHY_REG_ADDI_TXDAC_MASK;

	if (rate >= 8000 && rate <= 48000) {
		debug("%s, set rate to %d\n", __func__, rate);

		switch (rate) {
		case 8000:
			ctrl1 |= RXADC_CIC_DS_512;
			ana3 |= RXADC_CTUNE_MCLK_16384;
			ana0 |= ADDI_TXDAC_GAIN_RATIO_1;
			//spare0 |= SPARE_SCK_DIV(8); /* 4096 / 8 / 32 / 2 (if CIC is 128 need divide 2 in addition)*/
			clk |= RXADC_SCK_DIV(32) | RXADC_DLYEN(0x21); /* 16384 / 8 / 32 / 2 */
			break;
		case 11025:
			ctrl1 |= RXADC_CIC_DS_256;
			ana3 |= RXADC_CTUNE_MCLK_11298;
			ana0 |= ADDI_TXDAC_GAIN_RATIO_1;
			clk |= RXADC_SCK_DIV(16) | RXADC_DLYEN(0x17); /* 112896 / 11.025 / 32 / 2 */
			//spare0 |= SPARE_SCK_DIV(8); /* 5644 / 11.025 / 32 / 2 */
			break;
		case 16000:
			ctrl1 |= RXADC_CIC_DS_256;
			ana3 |= RXADC_CTUNE_MCLK_16384;
			ana0 |= ADDI_TXDAC_GAIN_RATIO_1;
			//spare0 |= SPARE_SCK_DIV(8); /* 8192 / 16 / 32 / 2 */
			clk |= RXADC_SCK_DIV(16) | RXADC_DLYEN(0x21); /* 16384 / 16 / 32 / 2 */
			break;
		case 22050:
			ctrl1 |= RXADC_CIC_DS_128;
			ana3 |= RXADC_CTUNE_MCLK_11298;
			ana0 &= ADDI_TXDAC_GAIN_RATIO_1;
			//spare0 |= SPARE_SCK_DIV(8); /* 11298 / 22.05 / 32 / 2 */
			clk |= RXADC_SCK_DIV(8) | RXADC_DLYEN(0x17); /* 112896 / 22.05 / 32 / 2 */
			break;
		case 32000:
			ctrl1 &= RXADC_CIC_DS_128;
			ana3 |= RXADC_CTUNE_MCLK_16384;
			ana0 &= ADDI_TXDAC_GAIN_RATIO_1;
			//spare0 |= SPARE_SCK_DIV(4); /* 8192 / 32 / 32 / 2 */
			clk |= RXADC_SCK_DIV(8) | RXADC_DLYEN(0x21); /* 16384 / 32 / 32 / 2 */
			break;
		case 44100:
			ctrl1 &= RXADC_CIC_DS_64;
			ana3 |= RXADC_CTUNE_MCLK_11298;
			ana0 &= ADDI_TXDAC_GAIN_RATIO_1;
			//spare0 |= SPARE_SCK_DIV(4); /* 11298 / 44.1 / 32 / 2 */
			clk |= RXADC_SCK_DIV(4) | RXADC_DLYEN(0x17); /* 112896 / 44.1 / 32 / 2 */
			break;
		case 48000:
			ctrl1 &= RXADC_CIC_DS_64;
			ana3 |= RXADC_CTUNE_MCLK_12288;
			ana0 &= ADDI_TXDAC_GAIN_RATIO_1;
			//spare0 |= SPARE_SCK_DIV(4); /* 12288 / 48 / 32 / 2 */
			clk |= RXADC_SCK_DIV(4) | RXADC_DLYEN(0x19); /* 16384 / 16 / 32 / 2 */
			break;
		default:
			ctrl1 |= RXADC_CIC_DS_256;
			ana3 |= RXADC_CTUNE_MCLK_16384;
			ana0 |= ADDI_TXDAC_GAIN_RATIO_1;
			clk |= RXADC_SCK_DIV(16) | RXADC_DLYEN(0x21); /* 16384 / 16 / 32 / 2 */
			//spare0 |= SPARE_SCK_DIV(8); /* 8192 / 16 / 32 / 2 */
			printf("%s, unsupported sample rate. Set with default 16KHz\n", __func__);
			break;
		}

		adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1, ctrl1);
		adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA3, ana3);
		adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CLK, clk);

		printf("adc_clk:%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CLK));

		//adc_write_reg(adc->adc_base, AUDIO_PHY_SPARE_0, spare0);
		writel(ana0, dac + AUDIO_PHY_TXDAC_ANA0);
	} else {
		printf("%s, unsupported sample rate\n", __func__);
		return 0;
	}

	return 0;
}

static void cv182xadc_on(struct cv182xadc *adc)
{

	u32 val = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);

	debug("%s, before rxadc reg val=0x%08x\n",
	__func__, adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));

	if ((val & AUDIO_PHY_REG_RXADC_EN_ON) | (val & AUDIO_PHY_REG_I2S_TX_EN_ON))
		printf("ADC or I2S TX already switched ON!!, val=0x%08x\n", val);

	val |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
	adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, val);

	debug("%s, after rxadc reg val=0x%08x\n",
	__func__, adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));

}

static void cv182xadc_off(struct cv182xadc *adc)
{

	u32 val = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);

	val &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
	adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, val);

	debug("%s, after rxadc reg val=0x%08x\n",
	__func__, adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));

}

static void cv182xadc_shutdown(struct cv182xadc *adc)
{
	cv182x_reset_adc();
	cv182xadc_off(adc);
}

int cv182xadc_ioctl(u32 cmd, struct cvi_vol_ctrl vol, u32 val)
{
	u32 val2;
	u32 temp;

	pr_debug("%s, received cmd=%u, val=%d\n", __func__, cmd, val);
	adc = &g_adc;
	adc->adc_base = (volatile u32 *)(0x0300A100);
	switch (cmd) {
	case ACODEC_SOFT_RESET_CTRL:
		cv182x_reset_adc();
		break;

	case ACODEC_SET_INPUT_VOL:
		pr_debug("adc: ACODEC_SET_INPUT_VOL\n");
		if ((val < 0) | (val > 24))
			pr_err("Only support range 0 [0dB] ~ 24 [48dB]\n");
		else if (val == 0) {
			/* set mute */
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2)
				| AUDIO_PHY_REG_MUTEL_ON
				| AUDIO_PHY_REG_MUTER_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
			temp = (adc_vol_list[val] | (adc_vol_list[val] << 16));
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);
		} else {
			val2 = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & AUDIO_PHY_REG_ADC_VOLL_MASK);
			for (temp = 0; temp < 25; temp++) {
				if (val2 == adc_vol_list[temp])
					break;
			}
			if (temp == 0) {
				/* unmute */
				temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2)
					& AUDIO_PHY_REG_MUTEL_OFF
					& AUDIO_PHY_REG_MUTEL_OFF;
				adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
			}
			temp = (adc_vol_list[val] | (adc_vol_list[val] << 16));
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);
		}
		break;

	case ACODEC_GET_INPUT_VOL:
		pr_debug("adc: ACODEC_GET_INPUT_VOL\n");
		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & AUDIO_PHY_REG_ADC_VOLL_MASK);
		for (temp = 0; temp < 25; temp++) {
			if (val == adc_vol_list[temp])
				break;
		}
		if (temp == 25)
			pr_info("adc: cannot find, out of range\n");

		//if (copy_to_user(argp, &temp, sizeof(temp)))
		//	pr_err("adc, failed to return input vol\n");
		break;

	case ACODEC_SET_I2S1_FS:
		pr_info("adc: ACODEC_SET_I2S1_FS is not support\n");
		break;

	case ACODEC_SET_MIXER_MIC:
		pr_info("ACODEC_SET_MIXER_MIC is not support\n");
		break;
	case ACODEC_SET_GAIN_MICL:
		pr_debug("adc: ACODEC_SET_GAIN_MICL\n");
		if ((val < 0) | (val > 24))
			pr_err("Only support range 0 [0dB] ~ 24 [48dB]\n");
		else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & ~AUDIO_PHY_REG_ADC_VOLL_MASK;
			temp |= adc_vol_list[val];
			old_adc_voll = val;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);
		}
		break;
	case ACODEC_SET_GAIN_MICR:
		pr_debug("adc: ACODEC_SET_GAIN_MICR\n");
		if ((val < 0) | (val > 24))
			pr_err("Only support range 0 [0dB] ~ 24 [48dB]\n");
		else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & ~AUDIO_PHY_REG_ADC_VOLR_MASK;
			temp |= (adc_vol_list[val] << 16);
			old_adc_volr = val;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);
		}
		break;

	case ACODEC_SET_ADCL_VOL:

		pr_info("adc: ACODEC_SET_ADCL_VOL to %d, mute=%d\n", vol.vol_ctrl, vol.vol_ctrl_mute);

		if (vol.vol_ctrl_mute == 1) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) | AUDIO_PHY_REG_MUTEL_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		} else if ((vol.vol_ctrl < 0) | (vol.vol_ctrl > 24))
			pr_err("adc-L: Only support range 0 [0dB] ~ 24 [48dB]\n");
		else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & ~AUDIO_PHY_REG_ADC_VOLL_MASK;
			temp |= adc_vol_list[val];
			old_adc_voll = val;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);

			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) & AUDIO_PHY_REG_MUTEL_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		}

		break;

	case ACODEC_SET_ADCR_VOL:

		pr_debug("adc: ACODEC_SET_ADCR_VOL to %d, mute=%d\n", vol.vol_ctrl, vol.vol_ctrl_mute);

		if (vol.vol_ctrl_mute == 1) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) | AUDIO_PHY_REG_MUTER_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		} else if ((vol.vol_ctrl < 0) | (vol.vol_ctrl > 24))
			pr_err("adc-R: Only support range 0 [0dB] ~ 24 [48dB]\n");
		else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & ~AUDIO_PHY_REG_ADC_VOLR_MASK;
			temp |= (adc_vol_list[val] << 16);
			old_adc_volr = val;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);

			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) & AUDIO_PHY_REG_MUTER_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		}
		break;
	case ACODEC_SET_MICL_MUTE:
		pr_debug("adc: ACODEC_SET_MICL_MUTE\n");
		if (val == 0)
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) & AUDIO_PHY_REG_MUTEL_OFF;
		else
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) | AUDIO_PHY_REG_MUTEL_ON;

		adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		break;
	case ACODEC_SET_MICR_MUTE:
		pr_debug("adc: ACODEC_SET_MICR_MUTE\n");
		if (val == 0)
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) & AUDIO_PHY_REG_MUTER_OFF;
		else
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) | AUDIO_PHY_REG_MUTER_ON;

		adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		break;

	case ACODEC_GET_GAIN_MICL:
		pr_debug("adc: ACODEC_GET_GAIN_MICL\n");
		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & AUDIO_PHY_REG_ADC_VOLL_MASK);

		for (temp = 0; temp < 25; temp++) {
			if (val == adc_vol_list[temp])
				break;
		}

		//if (copy_to_user(argp, &temp, sizeof(temp)))
		//	pr_err("failed to return MICL gain\n");
		break;
	case ACODEC_GET_GAIN_MICR:
		pr_debug("adc: ACODEC_GET_GAIN_MICR\n");
		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & AUDIO_PHY_REG_ADC_VOLR_MASK) >> 16;

		for (temp = 0; temp < 25; temp++) {
			if (val == adc_vol_list[temp])
				break;
		}
	//	if (copy_to_user(argp, &temp, sizeof(temp)))
		//	pr_err("failed to return MICR gain\n");
		break;

	case ACODEC_GET_ADCL_VOL:
		pr_debug("adc: ACODEC_GET_ADCL_VOL\n");

		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & AUDIO_PHY_REG_ADC_VOLL_MASK);
		for (temp = 0; temp < 25; temp++) {
			if (val == adc_vol_list[temp])
				break;
		}
		vol.vol_ctrl = temp;
		vol.vol_ctrl_mute = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) & AUDIO_PHY_REG_MUTEL_RXPGA_MASK;

		//if (copy_to_user(argp, &vol, sizeof(vol)))
		//	pr_err("failed to return ADCL vol\n");

		break;
	case ACODEC_GET_ADCR_VOL:
		pr_debug("adc: ACODEC_GET_ADCR_VOL\n");

		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) & AUDIO_PHY_REG_ADC_VOLR_MASK) >> 16;
		for (temp = 0; temp < 25; temp++) {
			if (val == adc_vol_list[temp])
				break;
		}
		vol.vol_ctrl = temp;
		vol.vol_ctrl_mute = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) & AUDIO_PHY_REG_MUTER_RXPGA_MASK;

		//if (copy_to_user(argp, &vol, sizeof(vol)))
		//	pr_err("failed to return ADCR vol\n");

		break;

	case ACODEC_SET_PD_ADCL:
		pr_debug("adc: ACODEC_SET_PD_ADCL, val=%d\n", val);
		if (val == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		} else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		}
		break;
	case ACODEC_SET_PD_ADCR:
		pr_debug("adc: ACODEC_SET_PD_ADCR, val=%d\n", val);
		if (val == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		} else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		}
		break;

	case ACODEC_SET_PD_LINEINL:
		pr_debug("adc: ACODEC_SET_PD_LINEINL, val=%d\n", val);
		if (val == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		} else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		}
		break;
	case ACODEC_SET_PD_LINEINR:
		pr_debug("adc: ACODEC_SET_PD_LINEINR, val=%d\n", val);
		if (val == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		} else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		}
		break;
	case ACODEC_SET_ADC_HP_FILTER:
		pr_info("adc: ACODEC_SET_ADC_HP_FILTER is not support\n");
		break;
	default:
		pr_info("%s, received unsupport cmd=%u\n", __func__, cmd);
		break;
	}

	return 0;
}

static void dump_182xadc_reg(struct cv182xadc *adc)
{
#ifdef __DEBUG__
    printf("AUDIO_PHY_RXADC_CTRL0 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));
    printf("AUDIO_PHY_RXADC_CTRL1 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1));
    printf("AUDIO_PHY_RXADC_STATUS = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_STATUS));
    printf("AUDIO_PHY_RXADC_ANA0 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0));
    printf("AUDIO_PHY_RXADC_ANA1 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA1));
    printf("AUDIO_PHY_RXADC_ANA2 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2));
    printf("AUDIO_PHY_RXADC_ANA3 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA3));
    printf("AUDIO_PHY_RXADC_ANA4 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA4));
    printf("AUDIO_PHY_RXADC_ANA5 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA5));
    printf("AUDIO_PHY_RXADC_ANA6 = 0x%x \n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA6));
#endif
}

int cv182xadc_init(u32 rate)
{
    u32 ctrl1;
    u64 mclk_source_addr = 0x0;
    debug("%s,%d start \n", __func__, __LINE__);
    adc = &g_adc;
    adc->adc_base = (volatile u32 *)(0x0300A100);
    mclk_source_addr = 0x04130000;
    if (mclk_source_addr)
        adc->mclk_source = (u32 *)(mclk_source_addr);
    else
        printf("get MCLK source failed !!\n");

    cv182xadc_shutdown(adc);
    /* set default input vol gain to maxmum 48dB, vol range is 0~24 */
    ctrl1 = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1);
    adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1, ctrl1 | AUDIO_ADC_IGR_INIT_EN);
	cv182xadc_on(adc);
    cv182xadc_hw_params(adc, rate);
	dump_182xadc_reg(adc);

    return 0;
}
