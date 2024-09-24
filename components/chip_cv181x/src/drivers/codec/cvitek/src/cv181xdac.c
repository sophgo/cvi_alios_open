/* SPDX-License-Identifier: GPL-2.0-or-later
 * CV181x DAC driver on CVITEK CV181x
 *
 * Copyright 2022 CVITEK
 *
 * Author: Ruilong.Chen
 *
 */

#include "cv181x_adc_dac.h"

static struct cv182xdac g_dac;
static struct cv182xdac *dac;

static int cv182xdac_hw_params(struct cv182xdac *dac, u32 chan_nr, u32 rate)
{

	u32 ctrl1 = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL1) & ~AUDIO_PHY_REG_TXDAC_CIC_OPT_MASK;
	u32 tick = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE0) & ~AUDIO_PHY_REG_TXDAC_INIT_DLY_CNT_MASK;
	u32 ana2 = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);

	//avoid pop when just set/clear mute/unmute bit
	ana2 |= 0x0F0F;
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, ana2);

	switch (chan_nr) {
	case 1:
		ana2 &= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_OFF; /* turn R-channel on */
		dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, ana2);
		break;
	default:
		ana2 &= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_OFF; /* turn R-channel on */
		dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, ana2);
		break;
	}


	if (rate >= 8000 && rate <= 48000) {
		debug("%s, set rate to %d\n", __func__, rate);

		switch (rate) {
		case 8000:
			ctrl1 |= TXDAC_CIC_DS_512;
			tick |= 0x21;
			break;
		case 11025:
			ctrl1 |= TXDAC_CIC_DS_256;
			tick |= 0x17;
			break;
		case 16000:
			ctrl1 |= TXDAC_CIC_DS_256;
			tick |= 0x21;
			break;
		case 22050:
			ctrl1 |= TXDAC_CIC_DS_128;
			tick |= 0x17;
			break;
		case 32000:
			ctrl1 |= TXDAC_CIC_DS_128;
			tick |= 0x21;
			break;
		case 44100:
			ctrl1 &= TXDAC_CIC_DS_64;
			tick |= 0x17;
			break;
		case 48000:
			ctrl1 &= TXDAC_CIC_DS_64;
			tick |= 0x19;
			break;
		default:
			ctrl1 |= TXDAC_CIC_DS_256;
			tick |= 0x21;
			debug(dac->dev, "%s, set sample rate with default 16KHz\n", __func__);
			break;
		}
	} else {
		printf("%s, unsupported sample rate\n", __func__);
		return 0;
	}
	debug("%s, ctrl1=0x%x\n", __func__, ctrl1);
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL1, ctrl1);
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE0, tick);

	return 0;
}


static void cv182xdac_on(struct cv182xdac *dac)
{

	u32 val = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);

	debug("%s, before ctrl0_reg val=0x%08x\n",	__func__, val);

	if ((val & AUDIO_PHY_REG_TXDAC_EN_ON) | (val & AUDIO_PHY_REG_I2S_RX_EN_ON))
		printf("DAC already switched ON!!, val=0x%08x\n", val);

	val |= AUDIO_PHY_REG_TXDAC_EN_ON | AUDIO_PHY_REG_I2S_RX_EN_ON;
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, val);

	debug("%s, after ctrl0_reg val=0x%08x\n",
	    __func__, dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0));

}

static void cv182xdac_off(struct cv182xdac *dac)
{
	u32 val = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);

	debug("%s, before ctrl_reg val=0x%08x\n",
	    __func__, dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0));

	val &= AUDIO_PHY_REG_TXDAC_EN_OFF & AUDIO_PHY_REG_I2S_RX_EN_OFF;
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, val);

	debug("%s, after ctrl_reg val=0x%08x\n",
	    __func__, dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0));
}

static void cv182xdac_shutdown()
{
	cv182x_reset_dac();
	cv182xdac_off(dac);
}

int cv182xdac_ioctl(u32 cmd, struct cvi_vol_ctrl vol, u32 val)
{

	u32 temp;
	dac = &g_dac;
	dac->dac_base = (volatile u32 *)(0x0300A000);
	switch (cmd) {
	case ACODEC_SOFT_RESET_CTRL:
		cv182x_reset_dac();
		break;

	case ACODEC_SET_OUTPUT_VOL:
		debug("dac: ACODEC_SET_OUTPUT_VOL with val=%d\n", val);
		if (vol.vol_ctrl_mute == 1) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			temp |= AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_ON | AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
		}else if ((val < 0) | (val > 32))
			printf("Only support range 0 [mute] ~ 32 [maximum]\n");
		else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1)
					& ~(AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK | AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK);
			temp |= DAC_VOL_L(val) | DAC_VOL_R(val);
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1, temp);

			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			temp &= AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_OFF;
			temp &= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_OFF;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
		}
		break;

	case ACODEC_GET_OUTPUT_VOL:
		debug("dac: ACODEC_GET_OUTPUT_VOL\n");
		temp = ((dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1)
				& AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK) + 1) / CV182x_DAC_VOL_STEP;
		debug("dac: return val=%d\n", temp);
		//_/ if (copy_to_user(argp, &temp, sizeof(temp)))
			//_/ printf("dac, failed to return output vol\n");
		break;

	case ACODEC_SET_I2S1_FS:
		debug("dac: ACODEC_SET_I2S1_FS is not support\n");
		break;

	case ACODEC_SET_DACL_VOL:
		debug("dac: ACODEC_SET_DACL_VOL\n");

		if (vol.vol_ctrl_mute == 1) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			temp |= AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
		} else if ((vol.vol_ctrl < 0) | (vol.vol_ctrl > 32))
			printf("dac-L: Only support range 0 [mute] ~ 32 [maximum]\n");
		else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1) & ~AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK;
			temp |= DAC_VOL_L(vol.vol_ctrl);
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1, temp);

			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			temp &= AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_OFF;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
		}
		break;

	case ACODEC_SET_DACR_VOL:
		debug("dac: ACODEC_SET_DACR_VOL\n");

		if (vol.vol_ctrl_mute == 1) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			temp |= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
		} else if ((vol.vol_ctrl < 0) | (vol.vol_ctrl > 32))
			printf("dac-R: Only support range 0 [mute] ~ 32 [maximum]\n");
		else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1) & ~AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK;
			temp |= DAC_VOL_R(vol.vol_ctrl);
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1, temp);

			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			temp &= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_OFF;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
		}
		break;

	case ACODEC_SET_DACL_MUTE:
		debug("dac: ACODEC_SET_DACL_MUTE, val=%d\n", val);
		temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
		if (val == 0)
			temp &= AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_OFF;
		else
			temp |= AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_ON;

		dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
		break;
	case ACODEC_SET_DACR_MUTE:
		debug("dac: ACODEC_SET_DACR_MUTE, val=%d\n", val);
		temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
		if (val == 0)
			temp &= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_OFF;
		else
			temp |= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_ON;
		dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
		break;

	case ACODEC_GET_DACL_VOL:
		debug("dac: ACODEC_GET_DACL_VOL\n");
		temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
		if (temp & AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_MASK) {
			vol.vol_ctrl = 0;
			vol.vol_ctrl_mute = 1;
		} else {
			temp = ((dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1)
			& AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK) + 1) / CV182x_DAC_VOL_STEP;
			vol.vol_ctrl = temp;
			vol.vol_ctrl_mute = 0;
		}
		//_/ if (copy_to_user(argp, &vol, sizeof(vol)))
			//_/ printf("failed to return DACL vol\n");
		break;
	case ACODEC_GET_DACR_VOL:
		temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
		debug("dac: ACODEC_GET_DACR_VOL, txdac_ana2=0x%x\n", temp);
		if (temp & AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_MASK) {
			vol.vol_ctrl = 0;
			vol.vol_ctrl_mute = 1;
		} else {
			temp = (((dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1)
			& AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK) >> 16) + 1) / CV182x_DAC_VOL_STEP;
			vol.vol_ctrl = temp;
			vol.vol_ctrl_mute = 0;
		}
		//_/ if (copy_to_user(argp, &vol, sizeof(vol)))
			//_/ printf("failed to return DACR vol\n");
		break;

	case ACODEC_SET_PD_DACL:
		debug("dac: ACODEC_SET_PD_DACL, val=%d\n", val);
		if (val == 0) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);
			temp &= AUDIO_PHY_REG_TXDAC_EN_ON | AUDIO_PHY_REG_I2S_RX_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, temp);
		} else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);
			temp &= AUDIO_PHY_REG_TXDAC_EN_OFF & AUDIO_PHY_REG_I2S_RX_EN_OFF;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, temp);
		}
		break;
	case ACODEC_SET_PD_DACR:
		debug("dac: ACODEC_SET_PD_DACR, val=%d\n", val);
		if (val == 0) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);
			temp &= AUDIO_PHY_REG_TXDAC_EN_ON | AUDIO_PHY_REG_I2S_RX_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, temp);
		} else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);
			temp &= AUDIO_PHY_REG_TXDAC_EN_OFF & AUDIO_PHY_REG_I2S_RX_EN_OFF;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, temp);
		}
		break;
	case ACODEC_SET_DAC_DE_EMPHASIS:
		debug("dac: ACODEC_SET_DAC_DE_EMPHASIS is not support\n");
		break;
	default:
		printf("%s, received unsupported cmd=%u\n", __func__, cmd);
		break;
	}

	return 0;
}

static void dump_182xdac_reg(struct cv182xdac *dac)
{
#ifdef __DEBUG__
    printf("AUDIO_PHY_TXDAC_CTRL0 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0));
    printf("AUDIO_PHY_TXDAC_CTRL1 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL1));
    printf("AUDIO_PHY_TXDAC_AFE0 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE0));
    printf("AUDIO_PHY_TXDAC_AFE1 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1));
    printf("AUDIO_PHY_TXDAC_ANA0 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA0));
    printf("AUDIO_PHY_TXDAC_ANA1 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA1));
    printf("AUDIO_PHY_TXDAC_ANA2 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2));

    printf("status : \n");
    printf("AUDIO_PHY_TXDAC_ANA3 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA3));
    printf("AUDIO_PHY_RXADC_STATUS = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_RXADC_STATUS));
    printf("AUDIO_PHY_RXADC_ANA1 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_RXADC_ANA1));
    printf("AUDIO_PHY_RXADC_ANA4 = 0x%x \n", dac_read_reg(dac->dac_base, AUDIO_PHY_RXADC_ANA4));
#endif

}

#if 0
static void set_reg()
{
    printf("%s dac_write_reg\n", __func__);
    dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0,0x00000003);
    dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL1,0x00000011);
    dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE0,0x00000009);
    dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1,0x00F100F3);
    dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA0,0x00000300);
    dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA1,0x00000000);
    dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2,0x00000000);

}
#endif

int cv182xdac_init(u32 rate, u32 chan_nr)
{

    debug("%s start rate = %d, chan_nr = %d\n", __func__, rate, chan_nr);
    dac = &g_dac;
    dac->dac_base = (volatile u32 *)(0x0300A000);
    cv182xdac_shutdown();
    cv182xdac_hw_params(dac, chan_nr, rate);
    cv182xdac_on(dac);
	dump_182xdac_reg(dac);

    return 0;
}


