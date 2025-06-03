/* SPDX-License-Identifier: GPL-2.0-or-later
 * CV181x ADC driver on CVITEK CV181x
 *
 * Copyright 2022 CVITEK
 *
 * Author: Ruilong.Chen
 *
 */


#include "codec_adc_dac.h"

static struct codec_adc g_adc;
static struct codec_adc *adc;

static int adc_vol_list[49] = {
//	0x2000000, /* -3dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=1, Reg_gstep=0x0 */
	0x1,       /* 0dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x1 */
	0x2,       /* 1dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x2 */
	0x4,       /* 2dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x4 */
	0x8,       /* 3dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x8 */
	0x10,      /* 4dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x10 */
	0x20,      /* 5dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x20 */
	0x40,      /* 6dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x40 */
	0x80,      /* 7dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x80 */
	0x100,     /* 8dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x100 */
	0x200,     /* 9dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x200 */
	0x400,     /* 10dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x400 */
	0x800,     /* 11dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x800 */
	0x1000,    /* 12dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x1000 */
	0x2000,    /* 13dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x2000 */
	0x4000,    /* 14dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x4000 */
	0x8000,    /* 15dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x8000 */
	0x10000,    /* 16dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x10000 */
	0x20000,    /* 17dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x20000 */
	0x40000,    /* 18dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x40000 */
	0x80000,    /* 19dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x80000 */
	0x100000,    /* 20dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x100000 */
	0x200000,    /* 21dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x200000 */
	0x400000,    /* 22dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x400000 */
	0x800000,    /* 23dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x800000 */
	0x1000000,    /* 24dB: Reg_gain_rxadc=0, Reg_g6db=0, reg_g3db_rxpga=0, Reg_gstep=0x1000000 */
	0x4080000, /* 25dB: Reg_gain_rxadc=0, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x80000 */
	0x4100000, /* 26dB: Reg_gain_rxadc=0, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x100000 */
	0x4200000, /* 27dB: Reg_gain_rxadc=0, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x200000 */
	0x4400000, /* 28dB: Reg_gain_rxadc=0, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x400000 */
	0x4800000, /* 29dB: Reg_gain_rxadc=0, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x800000 */
	0x5000000, /* 30dB: Reg_gain_rxadc=0, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x1000000 */
	0xC080000, /* 31dB: Reg_gain_rxadc=1, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x80000 */
	0xC100000, /* 32dB: Reg_gain_rxadc=1, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x100000 */
	0xC200000, /* 33dB: Reg_gain_rxadc=1, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x200000 */
	0xC400000, /* 34dB: Reg_gain_rxadc=1, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x400000 */
	0xC800000, /* 35dB: Reg_gain_rxadc=1, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x800000 */
	0xD000000, /* 36dB: Reg_gain_rxadc=1, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x1000000 */
	0x14080000, /* 37dB: Reg_gain_rxadc=2, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x80000 */
	0x14100000, /* 38dB: Reg_gain_rxadc=2, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x100000 */
	0x14200000, /* 39dB: Reg_gain_rxadc=2, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x200000 */
	0x14400000, /* 40dB: Reg_gain_rxadc=2, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x400000 */
	0x14800000, /* 41dB: Reg_gain_rxadc=2, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x800000 */
	0x15000000, /* 42dB: Reg_gain_rxadc=2, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x1000000 */
	0x1C080000, /* 43dB: Reg_gain_rxadc=3, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x80000 */
	0x1C100000, /* 44dB: Reg_gain_rxadc=3, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x100000 */
	0x1C200000, /* 45dB: Reg_gain_rxadc=3, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x200000 */
	0x1C400000, /* 46dB: Reg_gain_rxadc=3, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x400000 */
	0x1C800000, /* 47dB: Reg_gain_rxadc=3, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x800000 */
	0x1D000000, /* 48dB: Reg_gain_rxadc=3, Reg_g6db=1, reg_g3db_rxpga=0, Reg_gstep=0x1000000 */
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

static int codec_adc_hw_params(struct codec_adc *adc, u32 rate)
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

static void codec_adc_on(struct codec_adc *adc)
{

	u32 val = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);

	debug("%s, before rxadc reg val=0x%08x\n",
	__func__, adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));

	if ((val & AUDIO_PHY_REG_RXADC_EN_ON) | (val & AUDIO_PHY_REG_I2S_TX_EN_ON))
		printf("ADC or I2S TX already switched ON!!, val=0x%08x\n", val);
	val |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON | AUDIO_PHY_REG_RXADC_DCB_FIX_ON;
	adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, val);

	debug("%s, after rxadc reg val=0x%08x\n",
	__func__, adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));

}

static void codec_adc_off(struct codec_adc *adc)
{

	u32 val = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);

	val &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF & AUDIO_PHY_REG_RXADC_DCB_FIX_OFF;
	adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, val);

	debug("%s, after rxadc reg val=0x%08x\n",
	__func__, adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));

}

static void codec_adc_shutdown(struct codec_adc *adc)
{
	adc_reset();
	codec_adc_off(adc);
}

int codec_adc_ioctl(u32 cmd, struct cvi_vol_ctrl vol, u32 val)
{
	u32 val2;
	u32 temp;
	u32 left_vol;
	u32 right_vol;

	pr_debug("%s, received cmd=%u, val=%d\n", __func__, cmd, val);
	adc = &g_adc;
	adc->adc_base = (volatile u32 *)(0x0300A100);
	switch (cmd) {
	case ACODEC_SOFT_RESET_CTRL:
		adc_reset();
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
			/* set left vol */
			left_vol = adc_vol_list[val];
			/* set right vol */
			right_vol = adc_vol_list[val];
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7, left_vol);
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA8, right_vol);
		} else {
			val2 = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7) & AUDIO_PHY_REG_ADC_VOLL_MASK);
			for (temp = 0; temp < 49; temp++) {
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
			/* set left vol */
			left_vol = adc_vol_list[val];
			/* set right vol */
			right_vol = adc_vol_list[val];
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7, left_vol);
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA8, right_vol);
		}
		break;

	case ACODEC_GET_INPUT_VOL:
		pr_debug("adc: ACODEC_GET_INPUT_VOL\n");
		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7) & AUDIO_PHY_REG_ADC_VOLL_MASK);
		for (temp = 0; temp < 49; temp++) {
			if (val == adc_vol_list[temp])
				break;
		}
		if (temp == 49)
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
		if ((val < 0) | (val > 48))
			pr_err("Only support range 0 [0dB] ~ 48 [48dB]\n");
		else {
			/* set left vol */
			left_vol = adc_vol_list[val];
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7, left_vol);
		}
		break;
	case ACODEC_SET_GAIN_MICR:
		pr_debug("adc: ACODEC_SET_GAIN_MICR\n");
		if ((val < 0) | (val > 48))
			pr_err("Only support range 0 [0dB] ~ 48 [48dB]\n");
		else {
			/* set right vol */
			right_vol = adc_vol_list[val];
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA8, right_vol);
		}
		break;

	case ACODEC_SET_ADCL_VOL:

		pr_debug("adc: ACODEC_SET_ADCL_VOL to %d, mute=%d\n", val, vol.vol_ctrl_mute);

		if (vol.vol_ctrl_mute == 1) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) | AUDIO_PHY_REG_MUTEL_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		} else if ((val < 0) | (val > 48))
			pr_err("adc-L: Only support range 0 [0dB] ~ 48 [48dB]\n");
		else {
			/* set left vol */
			left_vol = adc_vol_list[val];
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7, left_vol);
			/* unmute */
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) & AUDIO_PHY_REG_MUTEL_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		}

		break;

	case ACODEC_SET_ADCR_VOL:

		pr_debug("adc: ACODEC_SET_ADCR_VOL to %d, mute=%d\n", val, vol.vol_ctrl_mute);

		if (vol.vol_ctrl_mute == 1) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) | AUDIO_PHY_REG_MUTER_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
		} else if ((val < 0) | (val > 48))
			pr_err("adc-R: Only support range 0 [0dB] ~ 48 [48dB]\n");
		else {
			/* set right vol */
			right_vol = adc_vol_list[val];
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA8, right_vol);
			/* unmute */
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
		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7) & AUDIO_PHY_REG_ADC_VOLL_MASK);

		for (temp = 0; temp < 49; temp++) {
			if (val == adc_vol_list[temp])
				break;
		}

		//if (copy_to_user(argp, &temp, sizeof(temp)))
		//	pr_err("failed to return MICL gain\n");
		break;
	case ACODEC_GET_GAIN_MICR:
		pr_debug("adc: ACODEC_GET_GAIN_MICR\n");
		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA8) & AUDIO_PHY_REG_ADC_VOLR_MASK) >> 16;

		for (temp = 0; temp < 49; temp++) {
			if (val == adc_vol_list[temp])
				break;
		}
	//	if (copy_to_user(argp, &temp, sizeof(temp)))
		//	pr_err("failed to return MICR gain\n");
		break;

	case ACODEC_GET_ADCL_VOL:
		pr_debug("adc: ACODEC_GET_ADCL_VOL\n");

		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7) & AUDIO_PHY_REG_ADC_VOLL_MASK);
		for (temp = 0; temp < 49; temp++) {
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

		val = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA8) & AUDIO_PHY_REG_ADC_VOLR_MASK) >> 16;
		for (temp = 0; temp < 49; temp++) {
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

static void dump_adc_reg(struct codec_adc *adc)
{
#ifdef __DEBUG__
	printf("AUDIO_PHY_RXADC_CTRL0 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));
	printf("AUDIO_PHY_RXADC_CTRL1 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1));
	printf("AUDIO_PHY_RXADC_STATUS = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_STATUS));
	printf("AUDIO_PHY_RXADC_ANA7 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA7));
	printf("AUDIO_PHY_RXADC_ANA8 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA8));
	printf("AUDIO_PHY_RXADC_ANA1 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA1));
	printf("AUDIO_PHY_RXADC_ANA2 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2));
	printf("AUDIO_PHY_RXADC_ANA3 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA3));
	printf("AUDIO_PHY_RXADC_ANA4 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA4));
	printf("AUDIO_PHY_RXADC_ANA5 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA5));
	printf("AUDIO_PHY_RXADC_ANA6 = 0x%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA6));
#endif
}

int codec_adc_init(u32 rate)
{
    u32 ctrl1;
	struct cvi_vol_ctrl vol;

	vol.vol_ctrl = 0;
	vol.vol_ctrl_mute = 0;
	debug("%s,%d start\n", __func__, __LINE__);
    adc = &g_adc;
    adc->adc_base = (volatile u32 *)(0x0300A100);
	codec_adc_shutdown(adc);
    /* set default input vol gain to maxmum 48dB, vol range is 0~24 */
    ctrl1 = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1);
    adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1, ctrl1 | AUDIO_ADC_IGR_INIT_EN);
	codec_adc_on(adc);
	codec_adc_hw_params(adc, rate);
	codec_adc_ioctl(ACODEC_SET_ADCL_VOL, vol, 12);
	dump_adc_reg(adc);

    return 0;
}
