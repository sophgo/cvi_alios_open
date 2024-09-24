/* SPDX-License-Identifier: GPL-2.0-or-later
 * CV182x I2s driver on CVITEK CV182x
 *
 * Copyright 2022 CVITEK
 *
 * Author: Ruilong.Chen
 *
 */


#include "dw_i2s.h"
#include <unistd.h>

bool concurrent_rx_enable = false;

void i2s_usleep(unsigned long useconds)
{
	usleep(useconds);
}


void devmem(volatile u64 addr, volatile u32 val)
{
    *((u32 *)addr) = val;
}

void set_i2s_debug_gpio(bool is_i2s3)
{
#ifdef __DEBUG__
    if (is_i2s3) {// 182x
		debug("%s \n", __func__);
        devmem(0x1810034, 0x0301);
        devmem(0x1810000, 0x1e1f0504);
        devmem(0x1810004, 0x0b0a1b1d);
        devmem(0x1810020, 0x4);

        devmem(0x1810034, 0x0304);
        devmem(0x1810000, 0x1e1f0100);
        devmem(0x1810004, 0x07061c1d);
        devmem(0x1810020, 0x4);

        devmem(0x3001024, 0x7);
        devmem(0x3001028, 0x7);
        devmem(0x300102c, 0x7);
        devmem(0x3001030, 0x7);

        devmem(0x4130060, 0x180);
        devmem(0x300a000, 0x3);
        devmem(0x300a100, 0x3);
    }else {
    //TODO.
    }
#endif
}

static void i2s_dump_reg(struct i2s_tdm_regs *i2s_reg)
{
#ifdef __DEBUG__
    printf("%s start addr:%p \n", __func__, i2s_reg);
    printf("BLK_MODE_SETTING = 0x%x \n",i2s_reg->blk_mode_setting);
    printf("FRAME_SETTING = 0x%x \n",i2s_reg->frame_setting);
    printf("SLOT_SETTING1 = 0x%x \n",i2s_reg->slot_setting1);
    printf("SLOT_SETTING2 = 0x%x \n",i2s_reg->slot_setting2);
    printf("DATA_FORMAT = 0x%x \n",i2s_reg->data_format);
    printf("BLK_CFG = 0x%x \n",i2s_reg->blk_cfg);
    printf("I2S_ENABLE = 0x%x \n",i2s_reg->i2s_enable);
    printf("I2S_INT_EN = 0x%x \n",i2s_reg->i2s_int_en);
    printf("FIFO_THRESHOLD = 0x%x \n",i2s_reg->fifo_threshold);
    printf("I2S_LRCK_MASTER = 0x%x \n",i2s_reg->i2s_lrck_master);
    printf("I2S_CLK_CTRL0 = 0x%x \n",i2s_reg->i2s_clk_ctrl0);
    printf("I2S_CLK_CTRL1 = 0x%x \n",i2s_reg->i2s_clk_ctrl1);
    printf("I2S_PCM_SYNTH = 0x%x \n",i2s_reg->i2s_pcm_synth);
#endif
}

void i2s_prepare_clk(void)
{
#define Clock_Gen_base_address      0x03002000
#define CV1835_CLK_SDMA_AUD0_DIV	0x098
#define CV1835_CLK_SDMA_AUD1_DIV	0x09C
#define CV1835_CLK_SDMA_AUD2_DIV	0x0A0
#define CV1835_CLK_SDMA_AUD3_DIV	0x0A4
#define CV1835_CLK_AUDSRC_DIV		0x118

#define PLL_G2_base_address             0x03002800
#define pll_g2_ctrl                     0x00
#define CV1835_CLK_A0PLL_CSR		    0x0C

#define CV1835_CLK_A0PLL_SSC_SYN_CTRL	0x50
#define CV1835_CLK_A0PLL_SSC_SYN_SET	0x54
#define CV1835_CLK_A0PLL_SSC_SYN_SPAN	0x58
#define CV1835_CLK_A0PLL_SSC_SYN_STEP	0x5C
    static bool initonce = false;
#ifdef ARCH_CV181X
    u32 aud_div;
    u32 apll_div = mmio_read_32(PLL_G2_base_address+CV1835_CLK_A0PLL_CSR) >> 17;
#endif
    if (initonce)
        return;
    initonce = true;
#if defined(CV183X)

    debug("%s #ifdef CV183X \n", __func__);
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD0_DIV) = 0x00110009;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD1_DIV) = 0x00110009;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD2_DIV) = 0x00110009;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD3_DIV) = 0x00110009;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_AUDSRC_DIV)    = 0x00000001;

    *(volatile u32*)(PLL_G2_base_address+pll_g2_ctrl) = 0x00000000;
    *(volatile u32*)(PLL_G2_base_address+CV1835_CLK_A0PLL_CSR) = 0x01108201;
    *(volatile u32*)(PLL_G2_base_address+CV1835_CLK_A0PLL_SSC_SYN_CTRL) = 0x00000001;

#elif defined(ARCH_CV181X)

    debug("%s \n", __func__);
    apll_div = (apll_div & 0x7f);
    aud_div = ((((apll_div << 1) + apll_div) << 16) | 0x9);
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD0_DIV) = aud_div;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD1_DIV) = aud_div;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD2_DIV) = aud_div;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD3_DIV) = aud_div;
    aud_div = (((apll_div << 1) << 16) | 0x9);
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_AUDSRC_DIV)    = aud_div;

#else

    debug("%s #ifdef !CV183X \n", __func__);
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD0_DIV) = 0x00110009;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD1_DIV) = 0x00110009;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD2_DIV) = 0x00110009;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD3_DIV) = 0x00110009;
    *(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_AUDSRC_DIV)    = 0x00150009;

    *(volatile u32*)(PLL_G2_base_address+pll_g2_ctrl) = 0x00000000;
    *(volatile u32*)(PLL_G2_base_address+CV1835_CLK_A0PLL_CSR) = 0x01108201;

    *(volatile u32*)(PLL_G2_base_address+CV1835_CLK_A0PLL_SSC_SYN_CTRL) = 0x00000000;
    *(volatile u32*)(PLL_G2_base_address+CV1835_CLK_A0PLL_SSC_SYN_SET) = 0x16FA5A5A;
    *(volatile u32*)(PLL_G2_base_address+CV1835_CLK_A0PLL_SSC_SYN_SPAN) = 0x00000000;
    *(volatile u32*)(PLL_G2_base_address+CV1835_CLK_A0PLL_SSC_SYN_STEP) = 0x00000000;
#endif
}

struct i2s_tdm_regs *i2s_get_base(unsigned int i2s_no)
{
    switch(i2s_no){
        case I2S0:
            return (struct i2s_tdm_regs *)CONFIG_SYS_I2S0_BASE;
        case I2S1:
            return (struct i2s_tdm_regs *)CONFIG_SYS_I2S1_BASE;
        case I2S2:
            return (struct i2s_tdm_regs *)CONFIG_SYS_I2S2_BASE;
        case I2S3:
            return (struct i2s_tdm_regs *)CONFIG_SYS_I2S3_BASE;
        default:
            printf("no such I2S device\n");
            break;
    }
	return NULL;
}

int i2s_get_no(unsigned int base_reg)
{
    switch(base_reg){
        case CONFIG_SYS_I2S0_BASE:
            return I2S0;
        case CONFIG_SYS_I2S1_BASE:
            return I2S1;
        case CONFIG_SYS_I2S2_BASE:
            return I2S2;
        case CONFIG_SYS_I2S3_BASE:
            return I2S3;
        default:
            printf("no such I2S device\n");
            break;
    }
	return -1;
}

struct i2s_sys_regs *i2s_get_sys_base(void)
{
	return (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
}


static void i2s_config_dma(struct i2s_tdm_regs *i2s_reg, bool on ,unsigned char fifo_threshold, unsigned char fifo_high_threshold)
{
	u32 blk_mode_setting = 0;
	u32 blk_cfg = 0;
	blk_mode_setting = readl(&i2s_reg->blk_mode_setting) & ~(DMA_MODE_MASK);
	if (on == true) {
		printf("%s dma mode \n", __func__);
		mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, blk_mode_setting | HW_DMA_MODE); /*not to use FIFO */
		mmio_write_32((uintptr_t)&i2s_reg->fifo_threshold, RX_FIFO_THRESHOLD(fifo_threshold)|TX_FIFO_THRESHOLD(fifo_threshold)|TX_FIFO_HIGH_THRESHOLD(fifo_high_threshold));
        blk_cfg = readl(&i2s_reg->blk_cfg) | AUTO_DISABLE_W_CH_EN | (0x1 << 17);
	} else {
		mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, blk_mode_setting | SW_MODE); /*not to use FIFO */
		mmio_write_32((uintptr_t)&i2s_reg->fifo_threshold, RX_FIFO_THRESHOLD(fifo_threshold)|TX_FIFO_THRESHOLD(fifo_threshold)|TX_FIFO_HIGH_THRESHOLD(fifo_high_threshold));
	}

	blk_cfg = readl(&i2s_reg->blk_cfg) | AUTO_DISABLE_W_CH_EN;//| (0x1 << 16);
	mmio_write_32((uintptr_t)&i2s_reg->blk_cfg, blk_cfg);
	//volatile u32 fifo_depth = (readl(&i2s_reg->fifo_threshold) & TX_FIFO_THRESHOLD_MASK) >> 16;
}

void i2s_set_clk_source(struct i2s_tdm_regs *i2s_reg, unsigned int src)
{

	u32 tmp = 0;
	tmp = readl(&i2s_reg->i2s_clk_ctrl0) & ~(AUD_CLK_SOURCE_MASK);
	tmp &= ~(BCLK_OUT_FORCE_EN);
	switch(src) {

	case AUD_CLK_FROM_MCLK_IN:
		tmp |= AUD_CLK_FROM_MCLK_IN | AUD_ENABLE;
		break;
	case AUD_CLK_FROM_PLL:
		tmp |= AUD_CLK_FROM_PLL | AUD_ENABLE;
		break;
	}
	debug("%s 0x%x \n", __func__, tmp);
	mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, tmp);
}

void i2s0_set_clk_sample_rate(struct i2s_tdm_regs *i2s_reg, unsigned int sample_rate)
{
#if defined(ARCH_CV181X)
	printf("%s sample_rate:%d\n", __func__, sample_rate);
	u32 clk_ctrl1 = 0;
	u32 aud_div = 0;
	u32 apll_div = mmio_read_32(PLL_G2_base_address+CV1835_CLK_A0PLL_CSR) >> 17;

	clk_ctrl1 = (readl(&i2s_reg->i2s_clk_ctrl1) & 0xffff0000);
	/* cv182x internal adc codec need dynamic MCLK frequency input */
	switch (sample_rate) {
	case 8000:
		clk_ctrl1 |= MCLK_DIV(1);
		break;
	case 11025:
		clk_ctrl1 |= MCLK_DIV(4);
		break;
	case 16000:
	case 32000:
		clk_ctrl1 |= MCLK_DIV(1);
		break;
	case 22050:
	case 44100:
	case 48000:
		clk_ctrl1 |= MCLK_DIV(2);
		break;
	default:
		printf("%s doesn't support this sample rate\n", __func__);
		break;
	}
	mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl1, clk_ctrl1);
	apll_div = (apll_div & 0x7f);
	if(sample_rate == 48000){
		aud_div = (((apll_div << 1) << 16) | 0x9);
	}
	else{
		aud_div = ((((apll_div << 1) + apll_div) << 16) | 0x9);
	}
	*(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD0_DIV) = aud_div;
	*(volatile u32*)(Clock_Gen_base_address+CV1835_CLK_SDMA_AUD3_DIV) = aud_div;
#else
	u32 clk_ctrl1 = 0;

	clk_ctrl1 = (readl(&i2s_reg->i2s_clk_ctrl1) & 0xffff0000);
	/* cv182x internal adc codec need dynamic MCLK frequency input */
	switch (sample_rate) {
	case 8000:
		clk_ctrl1 |= MCLK_DIV(6);
		break;
	case 11025:
		clk_ctrl1 |= MCLK_DIV(4);
		break;
	case 16000:
	case 32000:
		clk_ctrl1 |= MCLK_DIV(3);
		break;
	case 22050:
	case 44100:
	case 48000:
		clk_ctrl1 |= MCLK_DIV(2);
		break;
	default:
		printf("%s doesn't support this sample rate\n", __func__);
		break;
	}

	mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl1, clk_ctrl1);
	//i2s_dump_reg(i2s_reg);

#endif

	return;
}


/*
 * Sets the frame size for I2S sample rate
 *
 * @param i2s_reg	i2s regiter address
 */
void i2s_set_sample_rate(struct i2s_tdm_regs *i2s_reg, unsigned int sample_rate, u32 chan_nr, char *shortname)
{
    u32 frame_setting = 0;
    u32 slot_setting = 0;
    u32 data_format = 0;
    u32 clk_ctrl = 0;
    u32 div_multiplier = 2; //_/  ; /* if use audio PLL (25 or 24.576Mhz), div_multiplier should be 2 */
    u32 clk_ctrl1 = 0;
    u32 audio_clk = 24576000;
    u32 mclk_div = 0;
    u32 bclk_div = 0;


    debug("Set sample rate to %d, div_multiplier = %d\n", sample_rate, div_multiplier);
    frame_setting = readl(&i2s_reg->frame_setting);
    slot_setting = readl(&i2s_reg->slot_setting1);
    data_format = readl(&i2s_reg->data_format);
    clk_ctrl = readl(&i2s_reg->i2s_clk_ctrl1);

    frame_setting &= ~(FRAME_LENGTH_MASK | FS_ACT_LENGTH_MASK); // 4
    slot_setting &= ~(SLOT_SIZE_MASK | DATA_SIZE_MASK);
    data_format &= ~(WORD_LENGTH_MASK | SKIP_TX_INACT_SLOT_MASK);// 4

#if defined(CONFIG_USE_AUDIO_PLL)
    clk_ctrl = MCLK_DIV(2); /* audio PLL is 25 or 24.576 Mhz, need to div with 2*/
    div_multiplier = 2; //_/ 1;
#else
    clk_ctrl = MCLK_DIV(1); /* mclk_in is 12.288 Mhz, no need to div*/
#endif


#if defined(ARCH_CV181X)
	if ((sample_rate == 8000 || sample_rate == 16000 || sample_rate == 32000)) {
		audio_clk = 16384000;
	}
	switch (sample_rate) {
	case 8000:
	case 16000:
	case 32000:
		/* apll is 16.384Mhz, no need to divide */
		clk_ctrl1 |= MCLK_DIV(1);
		mclk_div = 1;
		break;
	case 48000:
		clk_ctrl1 |= MCLK_DIV(2);
		mclk_div = 2;
		break;
	default:
		printf("%s doesn't support this sample rate\n", __func__);
		break;
	}
#endif

	switch(sample_rate) {
		case 8000:
		    frame_setting |= FRAME_LENGTH(64);
		    slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		    data_format = WORD_LEN_16;

#if defined(ARCH_CV182X)
			printf("%s 8k\n", __func__);
#elif defined(ARCH_CV182X)
			clk_ctrl1 |= MCLK_DIV(6);
            mclk_div = 6;
#else
			clk_ctrl1 |= MCLK_DIV(6);
            mclk_div = 1;
#endif

		    bclk_div = (audio_clk / 1000) / (WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		    clk_ctrl1 |= BCLK_DIV(bclk_div);

		    break;

		case 12000:
		    frame_setting |= FRAME_LENGTH(64);
		    slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		    data_format = WORD_LEN_32;
		    clk_ctrl |= BCLK_DIV(16 * div_multiplier);
		    break;

		case 16000:
		    frame_setting |= FRAME_LENGTH(64);
		    slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		    data_format = WORD_LEN_16;
#if defined(ARCH_CV181X)
			printf("%s 16k\n", __func__);
#elif defined(ARCH_CV182X)
			clk_ctrl1 |= MCLK_DIV(3);
            mclk_div = 3;
#else
			clk_ctrl1 |= MCLK_DIV(3);
            mclk_div = 1;
#endif

		    bclk_div = (audio_clk / 1000) / (WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		    clk_ctrl1 |= BCLK_DIV(bclk_div);
		    break;

		case 24000:
		    frame_setting |= FRAME_LENGTH(64);
		    slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		    data_format = WORD_LEN_32;
		    clk_ctrl |= BCLK_DIV(8 * div_multiplier);
		    break;
		case 32000:
		    frame_setting |= FRAME_LENGTH(64);
		    slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		    data_format = WORD_LEN_16;

#if defined(ARCH_CV181X)
			printf("%s 32k\n", __func__);
#elif defined(ARCH_CV182X)
			clk_ctrl1 |= MCLK_DIV(3);
			mclk_div = 3;
#else
			clk_ctrl1 |= MCLK_DIV(3);
			mclk_div = 1;
#endif
		    bclk_div = (audio_clk / 1000) / (WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		    clk_ctrl1 |= BCLK_DIV(bclk_div);
		    break;

		case 48000:
		    frame_setting |= FRAME_LENGTH(64);
		    slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		    data_format = WORD_LEN_16;
#if defined(ARCH_CV181X)
			printf("%s 48k\n", __func__);
#elif defined(ARCH_CV182X)
			clk_ctrl1 |= MCLK_DIV(2);
			mclk_div = 2;
#else
			clk_ctrl1 |= MCLK_DIV(2);
            mclk_div = 1;
#endif

		    bclk_div = (audio_clk / 1000) / (WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		    clk_ctrl1 |= BCLK_DIV(bclk_div);
		    break;

		case 96000:
		    frame_setting |= FRAME_LENGTH(64);
		    slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		    data_format = WORD_LEN_32;
#if defined(ARCH_CV181X)
			printf("%s 96k\n", __func__);
#elif defined(ARCH_CV182X)
			clk_ctrl1 |= MCLK_DIV(3);
            mclk_div = 3;
#else
			clk_ctrl1 |= MCLK_DIV(3);
            mclk_div = 1;
#endif
		    bclk_div = (audio_clk / 1000) / (WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		    clk_ctrl1 |= BCLK_DIV(bclk_div);
		    break;

		case 192000:
		    frame_setting |= FRAME_LENGTH(64);
		    slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		    data_format = WORD_LEN_32;
#if defined(ARCH_CV181X)
			printf("%s 192k\n", __func__);
#elif defined(ARCH_CV182X)
			clk_ctrl1 |= MCLK_DIV(3);
            mclk_div = 3;
#else
			clk_ctrl1 |= MCLK_DIV(3);
            mclk_div = 1;
#endif

		    bclk_div = (audio_clk / 1000) / (WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		    clk_ctrl1 |= BCLK_DIV(bclk_div);
		    break;
		}

	 mmio_write_32((uintptr_t)&i2s_reg->frame_setting, frame_setting);
	 mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, slot_setting);
	 mmio_write_32((uintptr_t)&i2s_reg->data_format, data_format);
	 mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl1, clk_ctrl1);
}

void i2s_set_ws_clock_cycle(struct i2s_tdm_regs *i2s_reg, unsigned int ws_clk, u8 aud_mode)
{

	u32 frame_setting = 0;
	u32 slot_setting1 = 0;

	debug("start Set ws clkcycle to 0x%d .................... \n", ws_clk);
	frame_setting = readl(&i2s_reg->frame_setting) & ~(FRAME_LENGTH_MASK | FS_ACT_LENGTH_MASK);
	slot_setting1 = readl(&i2s_reg->slot_setting1) & ~(SLOT_SIZE_MASK | DATA_SIZE_MASK);

	if ((aud_mode == I2S_MODE) || (aud_mode == LJ_MODE) || (aud_mode == RJ_MODE)) {
		switch(ws_clk) {
			case WSS_16_CLKCYCLE:
				debug("%s,case WSS_16_CLKCYCLE: \n", __func__);
#if defined(ARCH_CV181X)
				frame_setting |= FRAME_LENGTH(32) | FS_ACT_LENGTH(16);
				frame_setting |= 0x4000;
				slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);

#elif defined(ARCH_CV182X)
				frame_setting |= FRAME_LENGTH(32) | FS_ACT_LENGTH(16);
				slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);
#else
				frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
				slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(16);
#endif
				break;
			case WSS_24_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(48) | FS_ACT_LENGTH(24);
				slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
				break;
			case WSS_32_CLKCYCLE:
				debug("%s,case WSS_32_CLKCYCLE: \n", __func__);
				frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
				slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(16);
				break;
			case WSS_256_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(256);
				slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
				break;
		}
	} else if ((aud_mode == PCM_A_MODE) || (aud_mode == PCM_B_MODE)){
		switch(ws_clk) {
			case WSS_16_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(32) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);
				break;
			case WSS_24_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(48) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
				break;
			case WSS_32_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(32);
				break;
			case WSS_256_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
				break;
		}
	} else if (aud_mode == TDM_MODE){
		unsigned int slot_no = (readl(&i2s_reg->slot_setting1) & SLOT_NUM_MASK)+1;

		debug("set TDM mode with slot number=%d\n", slot_no);
		switch(ws_clk) {
			case WSS_16_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(slot_no * 16) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);
				break;
			case WSS_24_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(slot_no * 24) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
				break;
			case WSS_32_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(slot_no * 32) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(32);
				break;
			case WSS_256_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
				break;
		}
	}

	debug("Set ws clk cycle to %d\n", ws_clk);
	mmio_write_32((uintptr_t)&i2s_reg->frame_setting, frame_setting);
	mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, slot_setting1);

}


void i2s_set_resolution(struct i2s_tdm_regs *i2s_reg, unsigned int data_size, unsigned int slot_size)
{

	u32 slot_setting1 = 0;

	debug("Set resolution to data_size=%d, slot_size = %d\n", data_size, slot_size);
	slot_setting1 = readl(&i2s_reg->slot_setting1) & ~(DATA_SIZE_MASK | SLOT_SIZE_MASK);
	slot_setting1 |= DATA_SIZE(data_size) | SLOT_SIZE(slot_size);

	mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, slot_setting1);
}


/*
 * Set the i2s enable
 * @param i2s_req 	i2s register address
 * @param enable 	1 to enable i2s, 0 to disable i2s
*/

void i2s_switch(int on, struct i2s_tdm_regs *i2s_reg)
{

	u32 i2s_enable = readl(&i2s_reg->i2s_enable);
	u32 aud_enable = readl(&i2s_reg->i2s_clk_ctrl0);
	u32 role = (readl(&i2s_reg->blk_mode_setting) & ROLE_MASK);
	if (on) {
		if (i2s_enable == I2S_OFF) {
			mmio_write_32((uintptr_t)&i2s_reg->i2s_enable, I2S_ON);
		}
	} else {
		if (i2s_enable == I2S_ON) {
			mmio_write_32((uintptr_t)&i2s_reg->i2s_enable, I2S_OFF);
		}

		//do not disable AUD_ENABLE due to external codec still need MCLK to do configuration
		if (((aud_enable & AUD_ENABLE) == AUD_ENABLE) && role == MASTER_MODE) {
			//mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, aud_enable & ~(AUD_ENABLE));
		}
	}
}

/*
 * Sets the i2s transfer control
 *
 * @param i2s_reg	i2s regiter address
 * @param on		1 to enable tx , 0 to disable tx transfer
 */
static void i2s_txctrl(struct i2s_tdm_regs *i2s_reg, int on)
{
    u32 blk_mode_setting = 0;
    u32 clk_ctrl = 0;

    blk_mode_setting = (readl(&i2s_reg->blk_mode_setting) & ~(TXRX_MODE_MASK));
    clk_ctrl = (readl(&i2s_reg->i2s_clk_ctrl0) & ~(AUD_SWITCH));

    blk_mode_setting |= TX_MODE;
    mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, blk_mode_setting);

    if ((blk_mode_setting & ROLE_MASK) == MASTER_MODE)
    {
	    if (on)
	    {
		    debug("Enable aud_en\n");
		    mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl| AUD_ENABLE);
	    } else {
		    debug("Disalbe aud_en\n");
		    //mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl & ~(AUD_ENABLE));
	    }
    } else {
        mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl & ~(AUD_ENABLE));
    }

}

/*
 * Sets the i2s receiver control
 *
 * @param i2s_reg	i2s regiter address
 * @param on		1 to enable rx , 0 to disable rx transfer
 */
static void i2s_rxctrl(struct i2s_tdm_regs *i2s_reg, int on)
{
    u32 blk_mode_setting = 0;
    u32 clk_ctrl = 0;

    blk_mode_setting = (readl(&i2s_reg->blk_mode_setting) & ~(TXRX_MODE_MASK));
    clk_ctrl = (readl(&i2s_reg->i2s_clk_ctrl0) & ~(BCLK_OUT_FORCE_EN));

    blk_mode_setting |= RX_MODE;
    mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, blk_mode_setting);

    if ((blk_mode_setting & ROLE_MASK) == MASTER_MODE)
    {
	    if (on)
	    {
		    debug("Enable aud_en\n");
		    mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_ENABLE);
	    } else {
		    debug("Disalbe aud_en\n");
		    mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_DISABLE);
	    }
    } else {
        mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_DISABLE);
    }


}

/*
 * flushes the i2stx fifo
 *
 * @param i2s_reg	i2s regiter address
 */
void i2s_sw_reset(struct i2s_tdm_regs *i2s_reg)
{
    int timeout_count = 0;
    if ((readl(&i2s_reg->blk_mode_setting) & TXRX_MODE_MASK) == TX_MODE)
    {
        mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, TX_FIFO_RESET_PULL_UP);
        i2s_usleep(10);
        mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, TX_FIFO_RESET_PULL_DOWN);

        mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_TX_PULL_UP);

        i2s_usleep(10);
        while (1) {
            if ((readl(&i2s_reg->tx_status) & RESET_TX_SCLK) >> 23) {
                debug("TX Reset complete\n");
                break;
            } else if (timeout_count > I2S_TIMEOUT){
                printf("TX Reset Timeout\n");
                break;
            }
            i2s_usleep(1000);
            timeout_count++;
        }
         mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_TX_PULL_DOWN);

    } else { /* reset RX*/
        u32 val = readl(&i2s_reg->i2s_clk_ctrl0);
        debug("Reset i2s RX, 0x%x \n", val);
        mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, val | AUD_ENABLE);

        mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, RX_FIFO_RESET_PULL_UP);
        i2s_usleep(10);
        mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, RX_FIFO_RESET_PULL_DOWN);

        mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_RX_PULL_UP);
        i2s_usleep(10);
        timeout_count = 0;
        while (1) {
            u32 tmp=readl(&i2s_reg->rx_status);
            if ((tmp & RESET_RX_SCLK) >> 23) {
                debug("RX Reset complete\n");
                break;
            } else if (timeout_count > I2S_TIMEOUT){
                printf("RX Reset Timeout\n");
                break;
            }
            i2s_usleep(1000);
            timeout_count++;
        }
         mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_RX_PULL_DOWN);

         mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, val);
    }
}

#if 0
static void i2s_subsys_int(unsigned int i2s_no)
{
	struct i2s_sys_regs *i2s_sys_reg = (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
	u32 bclk_oen_sel = readl(&i2s_sys_reg->i2s_bclk_oen_sel);

	bclk_oen_sel |= 1 << i2s_no;

	mmio_write_32((uintptr_t)&i2s_sys_reg->i2s_bclk_oen_sel, bclk_oen_sel);
}

static void i2s_subsys_bclk_oen_switch(int on, unsigned int i2s_no)
{
	struct i2s_sys_regs *i2s_sys_reg = i2s_get_sys_base();

	u32 bclk_oen_sel = readl(&i2s_sys_reg->i2s_bclk_oen_sel);

	if (on)
		bclk_oen_sel |= 1 << (i2s_no + 8);
	else
		bclk_oen_sel &= ~(1 << (i2s_no + 8));

	mmio_write_32((uintptr_t)&i2s_sys_reg->i2s_bclk_oen_sel, bclk_oen_sel);
}



static void i2s_fifo_reset(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
    if (stream == CVI_PCM_STREAM_PLAYBACK) {
        mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, TX_FIFO_RESET_PULL_UP);
        mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, TX_FIFO_RESET_PULL_DOWN);
    } else { /* reset RX*/
        mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, RX_FIFO_RESET_PULL_UP);
        mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, RX_FIFO_RESET_PULL_DOWN);
    }
}



static void i2s_reset(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
    int timeout_count = 0;
    if (stream == CVI_PCM_STREAM_PLAYBACK) {
        mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_TX_PULL_UP);
        i2s_usleep(10);
        while (1) {
            if ((readl(&i2s_reg->tx_status) & RESET_TX_SCLK) >> 23) {
                debug("TX Reset complete\n");
                break;
            } else if (timeout_count > I2S_TIMEOUT){
                printf("TX Reset Timeout %d \n", I2S_TIMEOUT);
                break;
            }
            i2s_usleep(1000);
            timeout_count++;
        }
        mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_TX_PULL_DOWN);
    } else {
        mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_RX_PULL_UP);

         i2s_usleep(10);
         timeout_count = 0;
         while (1) {
             u32 tmp=readl(&i2s_reg->rx_status);

             if ((tmp & RESET_RX_SCLK) >> 23) {
                 debug("RX Reset complete\n");
                 break;
             } else if (timeout_count > I2S_TIMEOUT){
                 printf("RX Reset Timeout\n");
                 break;
             }
             i2s_usleep(1000);
             timeout_count++;
         }
          mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_RX_PULL_DOWN);

    }
}

#endif
void i2s_set_interrupt(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
    if (stream == CVI_PCM_STREAM_PLAYBACK)
        mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en, I2S_INT_TXDA | I2S_INT_TXFO | I2S_INT_TXFU);
    else
        mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en, I2S_INT_RXDA | I2S_INT_RXFO | I2S_INT_RXFU);
}

void i2s_disable_all_interrupt(struct i2s_tdm_regs *i2s_reg)
{
    mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en, 0x00);
}


void i2s_clear_irqs(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
	u32 irq = readl(&i2s_reg->i2s_int);

	/* I2S_INT is write 1 clear */

	if (stream == CVI_PCM_STREAM_PLAYBACK)
        mmio_write_32((uintptr_t)&i2s_reg->i2s_int,
						irq & (I2S_INT_TXDA | I2S_INT_TXFO | I2S_INT_TXFU
						|I2S_INT_TXDA_RAW | I2S_INT_TXFO_RAW | I2S_INT_TXFU_RAW));
	else
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int,
						irq & (I2S_INT_RXDA | I2S_INT_RXFO | I2S_INT_RXFU
						|I2S_INT_RXDA_RAW | I2S_INT_RXFO_RAW | I2S_INT_RXFU_RAW));
}

void i2s_disable_irqs(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
    u32 irq = readl(&i2s_reg->i2s_int_en);

	if (stream == CVI_PCM_STREAM_PLAYBACK)
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en, irq & ~(I2S_INT_TXDA | I2S_INT_TXFO | I2S_INT_TXFU));
	else
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en, irq & ~(I2S_INT_RXDA | I2S_INT_RXFO | I2S_INT_RXFU));
}

void i2s_enable_irqs(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
	u32 irq = readl(&i2s_reg->i2s_int_en);

	if (stream == CVI_PCM_STREAM_PLAYBACK)
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en, irq | I2S_INT_TXDA | I2S_INT_TXFO | I2S_INT_TXFU);
	else
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en, irq | I2S_INT_RXDA | I2S_INT_RXFO | I2S_INT_RXFU);

}

void i2s_set_audio_gpio(int role)
{
	switch(role) {
    	case MASTER_MODE:
    		mmio_clrsetbits_32(REG_AUDIO_GPIO_BASE, 0xf << 0, 0x1);
    		break;
    	case SLAVE_MODE:
    		mmio_clrsetbits_32(REG_AUDIO_GPIO_BASE, 0xf << 0, 0xB);
    		break;
    	default:
    		break;
	}
}

/*
 * Sets I2S Clcok format
 *
 * @param fmt		i2s clock properties
 * @param i2s_reg	i2s regiter address
 *
 * @return		int value 0 for success, -1 in case of error
 */

int i2s_set_fmt(struct i2s_tdm_regs *i2s_reg,
       unsigned char role,
       unsigned char aud_mode,
       unsigned int fmt,
       unsigned char slot_no)
{
	unsigned int tmp = 0;
	unsigned int tmp2 = 0;
	unsigned int codec_fmt = 0;

	tmp = readl(&i2s_reg->frame_setting) & ~(FS_OFFSET_MASK | FS_IDEF_MASK | FS_ACT_LENGTH_MASK);
	tmp2 = readl(&i2s_reg->slot_setting1) & ~(SLOT_NUM_MASK);

  	switch(aud_mode) {
	case I2S_MODE:
		tmp |= FS_OFFSET_1_BIT | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK)+1)/2);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_I2S;
		break;
	case LJ_MODE:
		tmp |=NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK)+1)/2);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_LEFT_J;
		break;
	case RJ_MODE:
		tmp |=NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK)+1)/2);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 &= ~(FB_OFFSET_MASK);
		tmp2 |= SLOT_NUM(slot_no)|FB_OFFSET((((tmp & FS_ACT_LENGTH_MASK) >> 16)-((tmp2 & DATA_SIZE_MASK) >> 16)));
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_RIGHT_J;
		break;
    case PCM_A_MODE:
		tmp |=FS_OFFSET_1_BIT | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_DSP_A;
		break;
    case PCM_B_MODE:
		tmp |=NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_DSP_B;
		break;
    case TDM_MODE:
		tmp |=NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting2, 0x0f); /* enable slot 0-3 for TDM */
		codec_fmt |= SND_SOC_DAIFMT_PDM;
		break;
	default:

		debug("%s: Invalid format\n", __func__);
		return-1;
	}

    tmp=readl(&i2s_reg->blk_mode_setting) & ~(SAMPLE_EDGE_MASK | FS_SAMPLE_RX_DELAY_MASK); /* clear bit 2~4 to set frame format */
    tmp2=readl(&i2s_reg->frame_setting) & ~(FS_POLARITY_MASK); /* clear bit 12 to set fs polarity */
    if((aud_mode == I2S_MODE) || (aud_mode == LJ_MODE) || (aud_mode == RJ_MODE)) {
	switch (fmt) {
		case FMT_IB_NF:
#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P; /* for crx */
			else
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
#else

			tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
#endif
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_LOW;
			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_IB_NF;
			break;

		case FMT_IB_IF:
#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P; /* for crx */
			else

				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
#else
			tmp |=RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
#endif
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);

			tmp2 |= FS_ACT_HIGH;

			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_IB_IF;
			break;

		case FMT_NB_NF:
			debug("Set format to NBNF\n");
#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N; /* for crx  */
			else
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P;
#else

			tmp |=RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
#endif

			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_LOW;
			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_NB_NF;
			break;
		case FMT_NB_IF:
			debug("Set format to NBIF\n");
#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N; /* for crx */
			else
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P;
#else
			tmp |=RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
#endif
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_HIGH;
			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_NB_IF;
			break;
		default:
			debug("%s: Invalid clock ploarity input\n", __func__);
			return-1;
		}
	} else {

#ifdef CONFIG_SHIFT_HALF_T
			if (role == MASTER_MODE)
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
			else
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
#else
			tmp |=RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
#endif
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_HIGH;
			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_IB_IF;
	}

	tmp=readl(&i2s_reg->blk_mode_setting) & ~(ROLE_MASK); /* clear bit 2~4 to set frame format */
	switch (role) {
    	case MASTER_MODE:
    		tmp |=MASTER_MODE;
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
    		codec_fmt |= SND_SOC_DAIFMT_CBS_CFS; /* Set codec to slave */
    		break;
    	case SLAVE_MODE:
    		tmp |= SLAVE_MODE;
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
    		codec_fmt |= SND_SOC_DAIFMT_CBM_CFM; /* Set codec to master*/
    		break;
    	default:

    		debug("%s: Invalid master selection\n", __func__);
    		return-1;
	}

	return 0;
}

int i2s_receive_rx_data(struct i2stx_info *pi2s_tx)
{
	struct i2s_tdm_regs *i2s_reg = pi2s_tx->base_address;
	volatile u32 fifo_depth = (readl(&i2s_reg->fifo_threshold) & RX_FIFO_THRESHOLD_MASK);
	printf("%s, i2s_reg base=%p, fifo_depth = %d, \n", __func__, i2s_reg, fifo_depth);

	i2s_rxctrl(i2s_reg, I2S_RX_ON);
	i2s_sw_reset(i2s_reg);
	i2s_clear_irqs(i2s_reg, CVI_PCM_STREAM_CAPTURE);
	i2s_switch(I2S_ON, i2s_reg);
	return 0;
}


int i2s_transfer_tx_data(struct i2stx_info *pi2s_tx)
{

    struct i2s_tdm_regs *i2s_reg = pi2s_tx->base_address;
    volatile u32 fifo_depth = (readl(&i2s_reg->fifo_threshold) & TX_FIFO_THRESHOLD_MASK) >> 16;

    printf("%s, i2s_reg base=%p, fifo_depth = %d, \n", __func__, i2s_reg, fifo_depth);

    /* fill the tx buffer before stating the tx transmit */

    i2s_txctrl(i2s_reg, I2S_TX_ON);
    i2s_sw_reset(i2s_reg);

    i2s_clear_irqs(i2s_reg, CVI_PCM_STREAM_PLAYBACK);
    i2s_switch(I2S_ON, i2s_reg);
    return 0;
}

void i2s_subsys_io_init(struct i2stx_info *pi2s_tx)
{
    struct i2s_sys_regs *i2s_sys_reg =
        (struct i2s_sys_regs *)pi2s_tx->sys_base_address;

	static bool initonce = false;
	if (initonce)
		return;
	initonce = true;

#ifdef CV183X
    if(pi2s_tx->id == I2S1 || pi2s_tx->id == I2S2){
        debug("%s #ifdef CV183X I2S1/I2S2 \n", __func__);
        i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x7554; /* BCLK from I2S1 as master, I2S2 BCLK form I2S1 */
        i2s_sys_reg->i2s_tdm_fs_in_sel = 0x7554; /* LRCK from I2S1 as master, I2S2 LRCK from I2S1 */
        i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x7654; /*select i2s_tdm1 sdi to IO SDI_i2s1 */
        i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x7654; /* select i2s_tdm1 sdo to IO SDO_i2s1 */
        i2s_sys_reg->i2s_tdm_multi_sync = 0x00; /*0x600 I2S2 sync with I2S1 */
    } else {
        debug("%s #ifdef CV183X I2S0/I2S3 \n", __func__);
        i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_fs_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_multi_sync = 0x00000000;

        i2s_sys_reg->i2s_bclk_oen_sel = 0x00000000;
    }
#else
    if(pi2s_tx->id == I2S1 || pi2s_tx->id == I2S2){

		debug("%s #ifdef !CV183X I2S1/I2S2 \n", __func__);
		i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x7554; /* BCLK from I2S1 as master, I2S2 BCLK form I2S1 */
		i2s_sys_reg->i2s_tdm_fs_in_sel = 0x7554; /* LRCK from I2S1 as master, I2S2 LRCK from I2S1 */
		i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x7654; /*select i2s_tdm1 sdi to IO SDI_i2s1 */
		i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x7654; /* select i2s_tdm1 sdo to IO SDO_i2s1 */
		i2s_sys_reg->i2s_tdm_multi_sync = 0x00; /*0x600 I2S2 sync with I2S1 */


    } else if(pi2s_tx->id == I2S3) {
        debug("%s #ifdef !CV183X I2S3 \n", __func__);
        i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_fs_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_multi_sync = 0x00000000;

        i2s_sys_reg->i2s_bclk_oen_sel = 0x00000000;
        i2s_sys_reg->i2s_bclk_out_ctrl = 0x00000000;
        i2s_sys_reg->audio_pdm_ctrl = 0x00000000;
        i2s_sys_reg->audio_phy_bypass1 = 0x00000000;
        i2s_sys_reg->audio_phy_bypass2 = 0x00000000;
        i2s_sys_reg->i2s_sys_clk_ctrl = 0x00000000;
        i2s_sys_reg->i2s0_master_clk_ctrl0 = 0x00000040;
        i2s_sys_reg->i2s0_master_clk_ctrl1 = 0x00020002;
        i2s_sys_reg->i2s1_master_clk_ctrl0 = 0x00000040;
        i2s_sys_reg->i2s1_master_clk_ctrl1 = 0x00020002;
        i2s_sys_reg->i2s2_master_clk_ctrl0 = 0x00000040;
        i2s_sys_reg->i2s2_master_clk_ctrl1 = 0x00020002;
        i2s_sys_reg->i2s3_master_clk_ctrl0 = 0x00000040;
        i2s_sys_reg->i2s3_master_clk_ctrl1 = 0x00020002;
        i2s_sys_reg->i2s_sys_lrck_ctrl  = 0x00000000;

    } else if (pi2s_tx->id == I2S0) {
        i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_fs_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x00007654;
        i2s_sys_reg->i2s_tdm_multi_sync = 0x00000000;
	}

#endif

}


int i2s_init(struct i2stx_info *pi2s_tx)
{
    int ret;
    struct i2s_tdm_regs *i2s_reg =
				(struct i2s_tdm_regs *)pi2s_tx->base_address;
 //   struct i2s_sys_regs *i2s_sys_reg =
 //       (struct i2s_sys_regs *)pi2s_tx->sys_base_address;

    debug("%s, tdm_base_reg_addr=%p, sys_base_reg_addr=%p\n", __func__, pi2s_tx->base_address, pi2s_tx->sys_base_address);
    i2s_prepare_clk();
    i2s_subsys_io_init(pi2s_tx);
    i2s_set_clk_source(i2s_reg, pi2s_tx->clk_src);
    i2s_disable_all_interrupt(i2s_reg);

    if(pi2s_tx->id == I2S3)
	    i2s_set_sample_rate(i2s_reg, pi2s_tx->samplingrate, pi2s_tx->slot_no, "cv181x_dac"); /* sample rate must first prior to fmt */
    if(pi2s_tx->id == I2S0)
	    i2s_set_sample_rate(i2s_reg, pi2s_tx->samplingrate, pi2s_tx->slot_no, "cv181x_adc"); /* sample rate must first prior to fmt */

    ret = i2s_set_fmt(i2s_reg, pi2s_tx->role, pi2s_tx->aud_mode, pi2s_tx->inv, pi2s_tx->slot_no);
    if (ret != 0) {
	    printf("%s:set format failed\n", __func__);
	    return -1;
    }

    i2s_set_resolution(i2s_reg, pi2s_tx->bitspersample, pi2s_tx->slot_no);

    i2s_set_ws_clock_cycle(i2s_reg, pi2s_tx->sync_div, pi2s_tx->aud_mode);
#ifdef USE_DMA_MODE
    i2s_config_dma(i2s_reg, true, pi2s_tx->fifo_threshold, pi2s_tx->fifo_high_threshold);
#else
	i2s_config_dma(i2s_reg, false, pi2s_tx->fifo_threshold, pi2s_tx->fifo_high_threshold);
#endif
    set_i2s_debug_gpio(pi2s_tx->id);
	i2s_dump_reg(pi2s_tx->base_address);

    return ret;
}
