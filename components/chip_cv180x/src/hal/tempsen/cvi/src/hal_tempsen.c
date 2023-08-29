#include <mmio.h>
#include <drv/tick.h>
#include "hal_tempsen.h"

#define TEMPSEN_MASK(REG_NAME)	 tempsen_top_##REG_NAME##_MASK
#define TEMPSEN_OFFSET(REG_NAME) tempsen_top_##REG_NAME##_OFFSET
#define TEMPSEN_SET(REG_NAME, VAL)                                             \
	mmio_clrsetbits_32(hal_tempsen_base + tempsen_top_##REG_NAME,              \
			   TEMPSEN_MASK(REG_NAME),                             \
			   (VAL) << TEMPSEN_OFFSET(REG_NAME))
#define TEMPSEN_GET(REG_NAME)                                                  \
	((mmio_read_32(hal_tempsen_base + tempsen_top_##REG_NAME) &                \
	  TEMPSEN_MASK(REG_NAME)) >>                                           \
	 TEMPSEN_OFFSET(REG_NAME))

#define BIT(nr) (UINT64_C(1) << (nr))
#define CHAN0	BIT(0)

static unsigned long hal_tempsen_base = 0UL;

void set_hal_tempsen_base(unsigned long base)
{
	hal_tempsen_base = base;
}

unsigned long get_hal_tempsen_base(void)
{
	return hal_tempsen_base;
}

void tempsen_init(void)
{
	unsigned int regval;

	/* clear all interrupt status */
	regval = TEMPSEN_GET(sta_tempsen_intr_raw);
	TEMPSEN_SET(sta_tempsen_intr_clr, regval);

	/* clear max result */
	TEMPSEN_SET(clr_tempsen_ch0_max_result, 1);
	TEMPSEN_SET(clr_tempsen_ch1_max_result, 1);

	/* set chop period to 3:1024T */
	TEMPSEN_SET(reg_tempsen_chopsel, 0x3);

	/* set acc period to 2:2048T*/
	TEMPSEN_SET(reg_tempsen_accsel, 0x2);

	/* set tempsen clock divider to 25M/(0x31+1)= 0.5M ,T=2us */
	TEMPSEN_SET(reg_tempsen_cyc_clkdiv, 0x31);

	/* set reg_tempsen_auto_cycle */
	TEMPSEN_SET(reg_tempsen_auto_cycle, 0x100000);

	/* set ddr hi/lo threshold */
	TEMPSEN_SET(reg_tempsen_ddr_hi_th, 0x400); //85 C
	TEMPSEN_SET(reg_tempsen_ddr_lo_th, 0x3E4); //75 C

	/* enable ddr auto refresh rate ctrl signal output */
	TEMPSEN_SET(reg_tempsen_ddr_out_en, 1);

	/* enable tempsen channel */
	TEMPSEN_SET(reg_tempsen_sel, 0x1);
	TEMPSEN_SET(reg_tempsen_en, 1);
}

void tempsen_uninit(void)
{
	unsigned int regval;
	/* disable tempsen channel */
	TEMPSEN_SET(reg_tempsen_sel, 0x0);
	TEMPSEN_SET(reg_tempsen_en, 0);

	/* clear all interrupt status */
	regval = TEMPSEN_GET(sta_tempsen_intr_raw);
	TEMPSEN_SET(sta_tempsen_intr_clr, regval);
}

int wait_for_finish(unsigned int timeout_ms)
{
	unsigned int time = csi_tick_get_ms();
	while (!(TEMPSEN_GET(sta_tempsen_intr_raw) & CHAN0))
		if (csi_tick_get_ms() - time >= timeout_ms)
			return -1;
	return 0;
}

unsigned int read_temp(void)
{
	return TEMPSEN_GET(sta_tempsen_ch0_result);
}

unsigned int read_max_temp(void)
{
	return TEMPSEN_GET(sta_tempsen_ch0_max_result);
}

unsigned int tempsen_set_force_val(unsigned int force_val)
{
	unsigned int temp;
	/* set/enable force value*/
	TEMPSEN_SET(reg_tempsen_force_result, force_val);
	TEMPSEN_SET(reg_tempsen_force_en, 1);
	wait_for_finish(1000);
	temp = read_temp();
	TEMPSEN_SET(reg_tempsen_force_en, 0);
	return temp;
}