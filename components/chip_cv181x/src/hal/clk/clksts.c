#include <aos/cli.h>
#include <aos/list.h>
#include <aos/kernel.h>
#include <mmio.h>
#include <stddef.h>
#include <stdio.h>
#include "clk_def.h"

/* top_pll_g2 */
#define REG_PLL_G2_CTRL			0x03002800
#define REG_PLL_G2_STATUS		0x03002804
#define REG_MIPIMPLL_CSR		0x03002808
#define REG_APLL0_CSR			0x0300280C
#define REG_DISPPLL_CSR			0x03002810
#define REG_CAM0PLL_CSR			0x03002814
#define REG_CAM1PLL_CSR			0x03002818
#define REG_PLL_G2_SSC_SYN_CTRL		0x03002840
#define REG_APLL_SSC_SYN_CTRL		0x03002850
#define REG_APLL_SSC_SYN_SET		0x03002854
#define REG_APLL_SSC_SYN_SPAN		0x03002858
#define REG_APLL_SSC_SYN_STEP		0x0300285C
#define REG_DISPPLL_SSC_SYN_CTRL	0x03002860
#define REG_DISPPLL_SSC_SYN_SET		0x03002864
#define REG_DISPPLL_SSC_SYN_SPAN	0x03002868
#define REG_DISPPLL_SSC_SYN_STEP	0x0300286C
#define REG_CAM0PLL_SSC_SYN_CTRL	0x03002870
#define REG_CAM0PLL_SSC_SYN_SET		0x03002874
#define REG_CAM0PLL_SSC_SYN_SPAN	0x03002878
#define REG_CAM0PLL_SSC_SYN_STEP	0x0300287C
#define REG_CAM1PLL_SSC_SYN_CTRL	0x03002880
#define REG_CAM1PLL_SSC_SYN_SET		0x03002884
#define REG_CAM1PLL_SSC_SYN_SPAN	0x03002888
#define REG_CAM1PLL_SSC_SYN_STEP	0x0300288C
#define REG_APLL_FRAC_DIV_CTRL		0x03002890

#define REG_MPLL_CSR 0x03002908
#define REG_TPLL_CSR 0x0300290c
#define REG_FPLL_CSR 0x03002910

#define REG_CLK_CAM0_SRC_DIV		0x030028C0
#define REG_CLK_CAM1_SRC_DIV		0x030028C4

/* clkgen */
#define REG_CLK_EN_0				0x03002000
#define REG_CLK_EN_1				0x03002004
#define REG_CLK_EN_2				0x03002008
#define REG_CLK_EN_3				0x0300200C
#define REG_CLK_EN_4				0x03002010
#define REG_CLK_SEL_0				0x03002020
#define REG_CLK_BYP_0				0x03002030
#define REG_CLK_BYP_1				0x03002034

#define REG_DIV_CLK_A53_0			0x03002040
#define REG_DIV_CLK_A53_1			0x03002044
#define REG_DIV_CLK_CPU_AXI0		0x03002048
#define REG_DIV_CLK_CPU_GIC			0x03002050
#define REG_DIV_CLK_TPU				0x03002054
#define REG_DIV_CLK_EMMC			0x03002064
#define REG_DIV_CLK_100K_EMMC		0x0300206C
#define REG_DIV_CLK_SD0				0x03002070
#define REG_DIV_CLK_100K_SD0		0x03002078
#define REG_DIV_CLK_SD1				0x0300207C
#define REG_DIV_CLK_100K_SD1		0x03002084
#define REG_DIV_CLK_SPI_NAND		0x03002088
#define REG_DIV_CLK_500M_ETH0		0x0300208C
#define REG_DIV_CLK_500M_ETH1		0x03002090
#define REG_DIV_CLK_GPIO_DB			0x03002094
#define REG_DIV_CLK_SDMA_AUD0		0x03002098
#define REG_DIV_CLK_SDMA_AUD1		0x0300209C
#define REG_DIV_CLK_SDMA_AUD2		0x030020A0
#define REG_DIV_CLK_SDMA_AUD3		0x030020A4
#define REG_DIV_CLK_CAM0_200		0x030020A8
#define REG_DIV_CLK_AXI4			0x030020B8
#define REG_DIV_CLK_AXI6			0x030020BC
#define REG_DIV_CLK_DSI_ESC			0x030020C4
#define REG_DIV_CLK_AXI_VIP			0x030020C8
#define REG_DIV_CLK_SRC_VIP_SYS_0	0x030020D0
#define REG_DIV_CLK_SRC_VIP_SYS_1	0x030020D8
#define REG_DIV_CLK_DISP_SRC_VIP	0x030020E0
#define REG_DIV_CLK_AXI_VIDEO_CODEC	0x030020E4
#define REG_DIV_CLK_VC_SRC0			0x030020EC
#define REG_DIV_CLK_1M				0x030020FC
#define REG_DIV_CLK_SPI				0x03002100
#define REG_DIV_CLK_I2C				0x03002104
#define REG_DIV_CLK_SRC_VIP_SYS_2	0x03002110
#define REG_DIV_CLK_AUDSRC			0x03002118
#define REG_DIV_CLK_PWM_SRC_0		0x03002120
#define REG_DIV_CLK_AP_DEBUG		0x03002128
#define REG_DIV_CLK_RTCSYS_SRC_0	0x0300212C
#define REG_DIV_CLK_C906_0_0		0x03002130
#define REG_DIV_CLK_C906_0_1		0x03002134
#define REG_DIV_CLK_C906_1_0		0x03002138
#define REG_DIV_CLK_C906_1_1		0x0300213C
#define REG_DIV_CLK_SRC_VIP_SYS_3	0x03002140
#define REG_DIV_CLK_SRC_VIP_SYS_4	0x03002144

struct clk_gate {
    uint32_t	reg;
    int8_t		shift;
};

struct clk_div {
    uint32_t	reg;
    int8_t		shift;
    int8_t		width;
    int16_t     initval;
};

struct clk_mux {
    uint32_t	reg;
    int8_t		shift;
    int8_t		width;
};

struct clk_hw {
    uint8_t id;
    const char *name;
    struct clk_gate gate;
    struct clk_div div[2]; /* 0: DIV_IN0, 1: DIV_IN1 */
    struct clk_mux mux[3]; /* 0: bypass, 1: CLK_SEL, 2: CLK_SRC(DIV_IN0_SRC_MUX) */
    const uint8_t *const parents_tbl;
};

struct clk_pll {
    uint8_t	id;
    const char	*name;
    uint32_t	reg_csr;
    uint32_t	reg_ssc;
    int16_t		post_div_sel; /* -1: postdiv*/
    const uint8_t *const parents_tbl;
};

struct clk_fixed {
    uint8_t	id;
    const char	*name;
    uint32_t freq_rate;
};

struct clk_resource {
    void *clk;
    uint8_t type;
    uint8_t enabled;
    int16_t parent_id;
    uint32_t freq_rate;
    slist_t list;
    slist_t sublist;
};

#define CLK_G6_PLL(_id, _name, _parent, _reg_csr)  {		\
        .id = _id,						\
        .name = _name,					\
        .reg_csr = _reg_csr,				\
        .reg_ssc = 0,					\
        .post_div_sel = -1,					\
        .parents_tbl = _parent		\
    }

#define CLK_G2_PLL(_id, _name, _parent, _reg_csr, _reg_ssr)  {		\
        .id = _id,						\
        .name = _name,					\
        .reg_csr = _reg_csr,				\
        .reg_ssc = _reg_ssr,					\
        .post_div_sel = -1,					\
        .parents_tbl = _parent		\
    }

#define CV181X_CLK(_id, _name, _parents, _gate_reg, _gate_shift,		\
            _div_0_reg, _div_0_shift,			\
            _div_0_width, _div_0_initval,			\
            _div_1_reg, _div_1_shift,			\
            _div_1_width, _div_1_initval,			\
            _mux_0_reg, _mux_0_shift,			\
            _mux_1_reg, _mux_1_shift,			\
            _mux_2_reg, _mux_2_shift) {		\
        .id = _id,						\
        .name = _name,						\
        .gate.reg = _gate_reg,					\
        .gate.shift = _gate_shift,				\
        .div[0].reg = _div_0_reg,				\
        .div[0].shift = _div_0_shift,				\
        .div[0].width = _div_0_width,				\
        .div[0].initval = _div_0_initval,			\
        .div[1].reg = _div_1_reg,				\
        .div[1].shift = _div_1_shift,				\
        .div[1].width = _div_1_width,				\
        .div[1].initval = _div_1_initval,			\
        .mux[0].reg = _mux_0_reg,				\
        .mux[0].shift = _mux_0_shift,				\
        .mux[0].width = 1,					\
        .mux[1].reg = _mux_1_reg,				\
        .mux[1].shift = _mux_1_shift,				\
        .mux[1].width = 1,					\
        .mux[2].reg = _mux_2_reg,				\
        .mux[2].shift = _mux_2_shift,				\
        .mux[2].width = 2,					\
        .parents_tbl = _parents		\
    }

struct clk_fixed clk_osc = {
    .id = CLK_XTAL,
    .name = "osc",
    .freq_rate = 25000000
};

const uint8_t pll_parent[] = {CLK_XTAL};
const uint8_t frac_pll_parent[] = {CLK_MIPIMPLL};
const uint8_t clk_a53_parent[] = {CLK_XTAL, CLK_FPLL, CLK_TPLL, CLK_A0PLL, CLK_MIPIMPLL, CLK_MPLL};
const uint8_t clk_tpu_parent[] = {CLK_XTAL, CLK_TPLL, CLK_A0PLL, CLK_MIPIMPLL, CLK_FPLL};
const uint8_t clk_tpu_fab_parent[] = {CLK_XTAL, CLK_MIPIMPLL};
const uint8_t clk_pwm_parent[] = {CLK_PWM_SRC};
const uint8_t clk_cam0_200_parent[] = {CLK_XTAL, CLK_XTAL, CLK_DISPPLL};
const uint8_t clk_dsi_esc_parent[] = {CLK_XTAL, CLK_AXI6};
const uint8_t clk_disp_src_vip_parent[] = {CLK_XTAL, CLK_DISPPLL};
const uint8_t clk_cam0_parent[] = {CLK_CAM0PLL};
const uint8_t clk_cam1_parent[] = {CLK_CAM1PLL};
const uint8_t clk_i2c_parent[] = {CLK_XTAL, CLK_AXI6};
const uint8_t clk_vc_src1_parent[] = {CLK_XTAL, CLK_CAM1PLL};
const uint8_t clk_vip_parent[] = {CLK_XTAL, CLK_MIPIMPLL, CLK_CAM0PLL, CLK_DISPPLL, CLK_FPLL};
const uint8_t clk_c906_0_parent[] = {CLK_XTAL, CLK_FPLL, CLK_TPLL, CLK_A0PLL, CLK_MIPIMPLL, CLK_MPLL};
const uint8_t clk_c906_1_parent[] = {CLK_XTAL, CLK_FPLL, CLK_TPLL, CLK_A0PLL, CLK_DISPPLL, CLK_MPLL};
const uint8_t clk_axi_vc_parent[] = {CLK_XTAL, CLK_A0PLL, CLK_MIPIMPLL, CLK_CAM1PLL, CLK_FPLL};
const uint8_t clk_vc_src0_parent[] = {CLK_XTAL, CLK_DISPPLL, CLK_MIPIMPLL, CLK_CAM1PLL, CLK_FPLL};
const uint8_t clk_hw_parent_1m[] = {CLK_1M};
const uint8_t clk_hw_parent_osc[] = {CLK_XTAL};
const uint8_t clk_hw_parent_axi4[] = {CLK_AXI4};
const uint8_t clk_hw_parent_axi6[] = {CLK_AXI6};
const uint8_t clk_hw_parent_axi_vip[] = {CLK_AXI_VIP};
const uint8_t clk_hw_parent_cam0_200[] = {CLK_CAM0_200};
const uint8_t clk_hw_parent_xtal_misc[] = {CLK_XTAL_MISC};
const uint8_t clk_hw_parent_axi_vc[] = {CLK_AXI_VIDEO_CODEC};
const uint8_t clk_hw_parent_xtal_fpll[] = {CLK_XTAL, CLK_FPLL};
const uint8_t clk_hw_parent_ctal_a0pll[] = {CLK_XTAL, CLK_A0PLL}; //a24k_clk
const uint8_t clk_hw_parent_xtal_fpll_disppll[] = {CLK_XTAL, CLK_FPLL, CLK_DISPPLL};


struct clk_pll clk_mpll = CLK_G6_PLL(CLK_MPLL, "clk_mpll", pll_parent, REG_MPLL_CSR);
struct clk_pll clk_tpll = CLK_G6_PLL(CLK_TPLL, "clk_tpll", pll_parent, REG_TPLL_CSR);
struct clk_pll clk_fpll = CLK_G6_PLL(CLK_FPLL, "clk_fpll", pll_parent, REG_FPLL_CSR);
struct clk_pll clk_mipimpll = CLK_G2_PLL(CLK_MIPIMPLL, "clk_mipimpll", pll_parent, REG_MIPIMPLL_CSR, 0);
struct clk_pll clk_a0pll = CLK_G2_PLL(CLK_A0PLL, "clk_a0pll", frac_pll_parent, REG_APLL0_CSR, REG_APLL_SSC_SYN_CTRL);
struct clk_pll clk_disppll = CLK_G2_PLL(CLK_DISPPLL, "clk_disppll", frac_pll_parent, REG_DISPPLL_CSR, REG_DISPPLL_SSC_SYN_CTRL);
struct clk_pll clk_cam0pll = CLK_G2_PLL(CLK_CAM0PLL, "clk_cam0pll", frac_pll_parent, REG_CAM0PLL_CSR, REG_CAM0PLL_SSC_SYN_CTRL);
struct clk_pll clk_cam1pll = CLK_G2_PLL(CLK_CAM1PLL, "clk_cam1pll", frac_pll_parent, REG_CAM1PLL_CSR, REG_CAM1PLL_SSC_SYN_CTRL);
struct clk_hw clk_a53 = CV181X_CLK(CLK_A53, "clk_a53", clk_a53_parent,
        REG_CLK_EN_0, 0,
        REG_DIV_CLK_A53_0, 16, 4, 1,
        REG_DIV_CLK_A53_1, 16, 4, 2,
        REG_CLK_BYP_0, 0,
        REG_CLK_SEL_0, 0,
        REG_DIV_CLK_A53_0, 8);

struct clk_hw clk_cpu_axi0 = CV181X_CLK(CLK_CPU_AXI0, "clk_cpu_axi0",
        clk_hw_parent_xtal_fpll_disppll,
        REG_CLK_EN_0, 1,
        REG_DIV_CLK_CPU_AXI0, 16, 4, 3,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 1,
        0, -1,
        REG_DIV_CLK_CPU_AXI0, 8);
struct clk_hw clk_cpu_gic = CV181X_CLK(CLK_CPU_GIC, "clk_cpu_gic",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_0, 2,
        REG_DIV_CLK_CPU_GIC, 16, 4, 5,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 2,
        0, -1,
        0, -1);
struct clk_hw clk_xtal_a53 = CV181X_CLK(CLK_XTAL_A53, "clk_xtal_a53",
        clk_hw_parent_osc,
        REG_CLK_EN_0, 3,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_tpu = CV181X_CLK(CLK_TPU, "clk_tpu",
        clk_tpu_parent,
        REG_CLK_EN_0, 4,
        REG_DIV_CLK_TPU, 16, 4, 3,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 3,
        0, -1,
        REG_DIV_CLK_TPU, 8);
struct clk_hw clk_tpu_fab = CV181X_CLK(CLK_TPU_FAB, "clk_tpu_fab",
        clk_tpu_fab_parent,
        REG_CLK_EN_0, 5,
        0, -1, 0, 0,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 4,
        0, -1,
        0, -1);
struct clk_hw clk_ahb_rom = CV181X_CLK(CLK_AHB_ROM, "clk_ahb_rom",
        clk_hw_parent_axi4,
        REG_CLK_EN_0, 6,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_ddr_axi_reg = CV181X_CLK(CLK_DDR_AXI_REG, "clk_ddr_axi_reg",
        clk_hw_parent_axi6,
        REG_CLK_EN_0, 7,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_rtc_25m = CV181X_CLK(CLK_RTC_25M, "clk_rtc_25m",
        clk_hw_parent_osc,
        REG_CLK_EN_0, 8,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_tempsen = CV181X_CLK(CLK_TEMPSEN, "clk_tempsen",
        clk_hw_parent_osc,
        REG_CLK_EN_0, 9,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_saradc = CV181X_CLK(CLK_SARADC, "clk_saradc",
        clk_hw_parent_osc,
        REG_CLK_EN_0, 10,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_efuse = CV181X_CLK(CLK_EFUSE, "clk_efuse",
        clk_hw_parent_osc,
        REG_CLK_EN_0, 11,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_efuse = CV181X_CLK(CLK_APB_EFUSE, "clk_apb_efuse",
        clk_hw_parent_osc,
        REG_CLK_EN_0, 12,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_debug = CV181X_CLK(CLK_DEBUG, "clk_debug",
        clk_hw_parent_osc,
        REG_CLK_EN_0, 13,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_xtal_misc = CV181X_CLK(CLK_XTAL_MISC, "clk_xtal_misc",
        clk_hw_parent_osc,
        REG_CLK_EN_0, 14,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_axi4_emmc = CV181X_CLK(CLK_AXI4_EMMC, "clk_axi4_emmc",
        clk_hw_parent_axi4,
        REG_CLK_EN_0, 15,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_emmc = CV181X_CLK(CLK_EMMC, "clk_emmc",
        clk_hw_parent_xtal_fpll_disppll,
        REG_CLK_EN_0, 16,
        REG_DIV_CLK_EMMC, 16, 5, 15,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 5,
        0, -1,
        REG_DIV_CLK_EMMC, 8);
struct clk_hw clk_100k_emmc = CV181X_CLK(CLK_100K_EMMC, "clk_100k_emmc",
        clk_hw_parent_1m,
        REG_CLK_EN_0, 17,
        REG_DIV_CLK_100K_EMMC, 16, 8, 10,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_axi4_sd0 = CV181X_CLK(CLK_AXI4_SD0, "clk_axi4_sd0",
        clk_hw_parent_axi4,
        REG_CLK_EN_0, 18,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sd0 = CV181X_CLK(CLK_SD0, "clk_sd0",
        clk_hw_parent_xtal_fpll_disppll,
        REG_CLK_EN_0, 19,
        REG_DIV_CLK_SD0, 16, 5, 15,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 6,
        0, -1,
        REG_DIV_CLK_SD0, 8);
struct clk_hw clk_100k_sd0 = CV181X_CLK(CLK_100K_SD0, "clk_100k_sd0",
        clk_hw_parent_1m,
        REG_CLK_EN_0, 20,
        REG_DIV_CLK_100K_SD0, 16, 8, 10,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_axi4_sd1 = CV181X_CLK(CLK_AXI4_SD1, "clk_axi4_sd1",
        clk_hw_parent_axi4,
        REG_CLK_EN_0, 21,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sd1 = CV181X_CLK(CLK_SD1, "clk_sd1",
        clk_hw_parent_xtal_fpll_disppll,
        REG_CLK_EN_0, 22,
        REG_DIV_CLK_SD1, 16, 5, 15,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 7,
        0, -1,
        REG_DIV_CLK_SD1, 8);
struct clk_hw clk_100k_sd1 = CV181X_CLK(CLK_100K_SD1, "clk_100k_sd1",
        clk_hw_parent_1m,
        REG_CLK_EN_0, 23,
        REG_DIV_CLK_100K_SD1, 16, 8, 10,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_spi_nand = CV181X_CLK(CLK_SPI_NAND, "clk_spi_nand",
        clk_hw_parent_xtal_fpll_disppll,
        REG_CLK_EN_0, 24,
        REG_DIV_CLK_SPI_NAND, 16, 5, 8,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 8,
        0, -1,
        REG_DIV_CLK_SPI_NAND, 8);
struct clk_hw clk_500m_eth0 = CV181X_CLK(CLK_500M_ETH0, "clk_500m_eth0",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_0, 25,
        REG_DIV_CLK_500M_ETH0, 16, 4, 3,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 9,
        0, -1,
        0, -1);
struct clk_hw clk_axi4_eth0 = CV181X_CLK(CLK_AXI4_ETH0, "clk_axi4_eth0",
        clk_hw_parent_axi4,
        REG_CLK_EN_0, 26,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_500m_eth1 = CV181X_CLK(CLK_500M_ETH1, "clk_500m_eth1",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_0, 27,
        REG_DIV_CLK_500M_ETH1, 16, 4, 3,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 10,
        0, -1,
        0, -1);
struct clk_hw clk_axi4_eth1 = CV181X_CLK(CLK_AXI4_ETH1, "clk_axi4_eth1",
        clk_hw_parent_axi4,
        REG_CLK_EN_0, 28,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_gpio = CV181X_CLK(CLK_APB_GPIO, "clk_apb_gpio",
        clk_hw_parent_axi6,
        REG_CLK_EN_0, 29,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_gpio_intr = CV181X_CLK(CLK_APB_GPIO_INTR, "clk_apb_gpio_intr",
        clk_hw_parent_axi6,
        REG_CLK_EN_0, 30,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_gpio_db = CV181X_CLK(CLK_GPIO_DB, "clk_gpio_db",
        clk_hw_parent_1m,
        REG_CLK_EN_0, 31,
        REG_DIV_CLK_GPIO_DB, 16, 16, 10,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_ahb_sf = CV181X_CLK(CLK_AHB_SF, "clk_ahb_sf",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 0,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sdma_axi = CV181X_CLK(CLK_SDMA_AXI, "clk_sdma_axi",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 1,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sdma_aud0 = CV181X_CLK(CLK_SDMA_AUD0, "clk_sdma_aud0",
        clk_hw_parent_ctal_a0pll,
        REG_CLK_EN_1, 2,
        REG_DIV_CLK_SDMA_AUD0, 16, 8, 18,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 11,
        0, -1,
        REG_DIV_CLK_SDMA_AUD0, 8);
struct clk_hw clk_sdma_aud1 = CV181X_CLK(CLK_SDMA_AUD1, "clk_sdma_aud1",
        clk_hw_parent_ctal_a0pll,
        REG_CLK_EN_1, 3,
        REG_DIV_CLK_SDMA_AUD1, 16, 8, 18,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 12,
        0, -1,
        REG_DIV_CLK_SDMA_AUD1, 8);
struct clk_hw clk_sdma_aud2 = CV181X_CLK(CLK_SDMA_AUD2, "clk_sdma_aud2",
        clk_hw_parent_ctal_a0pll,
        REG_CLK_EN_1, 4,
        REG_DIV_CLK_SDMA_AUD2, 16, 8, 18,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 13,
        0, -1,
        REG_DIV_CLK_SDMA_AUD2, 8);
struct clk_hw clk_sdma_aud3 = CV181X_CLK(CLK_SDMA_AUD3, "clk_sdma_aud3",
        clk_hw_parent_ctal_a0pll,
        REG_CLK_EN_1, 5,
        REG_DIV_CLK_SDMA_AUD3, 16, 8, 18,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 14,
        0, -1,
        REG_DIV_CLK_SDMA_AUD3, 8);
struct clk_hw clk_apb_i2c = CV181X_CLK(CLK_APB_I2C, "clk_apb_i2c",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 6,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_wdt = CV181X_CLK(CLK_APB_WDT, "clk_apb_wdt",
        clk_hw_parent_osc,
        REG_CLK_EN_1, 7,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_pwm = CV181X_CLK(CLK_PWM, "clk_pwm",
        clk_pwm_parent,
        REG_CLK_EN_1, 8,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_spi0 = CV181X_CLK(CLK_APB_SPI0, "clk_apb_spi0",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 9,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_spi1 = CV181X_CLK(CLK_APB_SPI1, "clk_apb_spi1",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 10,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_spi2 = CV181X_CLK(CLK_APB_SPI2, "clk_apb_spi2",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 11,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_spi3 = CV181X_CLK(CLK_APB_SPI3, "clk_apb_spi3",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 12,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_cam0_200 = CV181X_CLK(CLK_CAM0_200, "clk_cam0_200",
        clk_cam0_200_parent,
        REG_CLK_EN_1, 13,
        REG_DIV_CLK_CAM0_200, 16, 4, 1,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 16,
        0, -1,
        REG_DIV_CLK_CAM0_200, 8);
struct clk_hw clk_uart0 = CV181X_CLK(CLK_UART0, "clk_uart0",
        clk_hw_parent_cam0_200,
        REG_CLK_EN_1, 14,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_uart0 = CV181X_CLK(CLK_APB_UART0, "clk_apb_uart0",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 15,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_uart1 = CV181X_CLK(CLK_UART1, "clk_uart1",
        clk_hw_parent_cam0_200,
        REG_CLK_EN_1, 16,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_uart1 = CV181X_CLK(CLK_APB_UART1, "clk_apb_uart1",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 17,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_uart2 = CV181X_CLK(CLK_UART2, "clk_uart2",
        clk_hw_parent_cam0_200,
        REG_CLK_EN_1, 18,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_uart2 = CV181X_CLK(CLK_APB_UART2, "clk_apb_uart2",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 19,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_uart3 = CV181X_CLK(CLK_UART3, "clk_uart3",
        clk_hw_parent_cam0_200,
        REG_CLK_EN_1, 20,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_uart3 = CV181X_CLK(CLK_APB_UART3, "clk_apb_uart3",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 21,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_uart4 = CV181X_CLK(CLK_UART4, "clk_uart4",
        clk_hw_parent_cam0_200,
        REG_CLK_EN_1, 22,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_uart4 = CV181X_CLK(CLK_APB_UART4, "clk_apb_uart4",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 23,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2s0 = CV181X_CLK(CLK_APB_I2S0, "clk_apb_i2s0",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 24,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2s1 = CV181X_CLK(CLK_APB_I2S1, "clk_apb_i2s1",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 25,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2s2 = CV181X_CLK(CLK_APB_I2S2, "clk_apb_i2s2",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 26,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2s3 = CV181X_CLK(CLK_APB_I2S3, "clk_apb_i2s3",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 27,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_axi4_usb = CV181X_CLK(CLK_AXI4_USB, "clk_axi4_usb",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 28,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_usb = CV181X_CLK(CLK_APB_USB, "clk_apb_usb",
        clk_hw_parent_axi4,
        REG_CLK_EN_1, 29,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_125m_usb = CV181X_CLK(CLK_125M_USB, "clk_125m_usb",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_1, 30,
        0, -1, 0, 0,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 17,
        0, -1,
        0, -1);
struct clk_hw clk_33k_usb = CV181X_CLK(CLK_33K_USB, "clk_33k_usb",
        clk_hw_parent_1m,
        REG_CLK_EN_1, 31,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_12m_usb = CV181X_CLK(CLK_12M_USB, "clk_12m_usb",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_2, 0,
        0, -1, 0, 0,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 18,
        0, -1,
        0, -1);
struct clk_hw clk_axi4 = CV181X_CLK(CLK_AXI4, "clk_axi4",
        clk_hw_parent_xtal_fpll_disppll,
        REG_CLK_EN_2, 1,
        REG_DIV_CLK_AXI4, 16, 4, 5,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 19,
        0, -1,
        REG_DIV_CLK_AXI4, 8);
struct clk_hw clk_axi6 = CV181X_CLK(CLK_AXI6, "clk_axi6",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_2, 2,
        REG_DIV_CLK_AXI6, 16, 4, 15,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 20,
        0, -1,
        0, -1);
struct clk_hw clk_dsi_esc = CV181X_CLK(CLK_DSI_ESC, "clk_dsi_esc",
        clk_dsi_esc_parent,
        REG_CLK_EN_2, 3,
        REG_DIV_CLK_DSI_ESC, 16, 4, 5,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 21,
        0, -1,
        0, -1);
struct clk_hw clk_axi_vip = CV181X_CLK(CLK_AXI_VIP, "clk_axi_vip",
        clk_vip_parent,
        REG_CLK_EN_2, 4,
        REG_DIV_CLK_AXI_VIP, 16, 4, 3,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 22,
        0, -1,
        REG_DIV_CLK_AXI_VIP, 8);
struct clk_hw clk_src_vip_sys_0 = CV181X_CLK(CLK_SRC_VIP_SYS_0, "clk_src_vip_sys_0",
        clk_vip_parent,
        REG_CLK_EN_2, 5,
        REG_DIV_CLK_SRC_VIP_SYS_0, 16, 4, 6,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 23,
        0, -1,
        REG_DIV_CLK_SRC_VIP_SYS_0, 8);
struct clk_hw clk_src_vip_sys_1 = CV181X_CLK(CLK_SRC_VIP_SYS_1, "clk_src_vip_sys_1",
        clk_vip_parent,
        REG_CLK_EN_2, 6,
        REG_DIV_CLK_SRC_VIP_SYS_1, 16, 4, 5,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 24,
        0, -1,
        REG_DIV_CLK_SRC_VIP_SYS_1, 8);
struct clk_hw clk_disp_src_vip = CV181X_CLK(CLK_DISP_SRC_VIP, "clk_disp_src_vip",
        clk_disp_src_vip_parent,
        REG_CLK_EN_2, 7,
        REG_DIV_CLK_DISP_SRC_VIP, 16, 4, 8,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 25,
        0, -1,
        0, -1);
struct clk_hw clk_axi_video_codec = CV181X_CLK(CLK_AXI_VIDEO_CODEC, "clk_axi_video_codec",
        clk_axi_vc_parent,
        REG_CLK_EN_2, 8,
        REG_DIV_CLK_AXI_VIDEO_CODEC, 16, 4, 2,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 26,
        0, -1,
        REG_DIV_CLK_AXI_VIDEO_CODEC, 8);
struct clk_hw clk_vc_src0 = CV181X_CLK(CLK_VC_SRC0, "clk_vc_src0",
        clk_vc_src0_parent,
        REG_CLK_EN_2, 9,
        REG_DIV_CLK_VC_SRC0, 16, 4, 2,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 27,
        0, -1,
        REG_DIV_CLK_VC_SRC0, 8);
struct clk_hw clk_h264c = CV181X_CLK(CLK_H264C, "clk_h264c",
        clk_hw_parent_axi_vc,
        REG_CLK_EN_2, 10,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_h265c = CV181X_CLK(CLK_H265C, "clk_h265c",
        clk_hw_parent_axi_vc,
        REG_CLK_EN_2, 11,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_jpeg = CV181X_CLK(CLK_JPEG, "clk_jpeg",
        clk_hw_parent_axi_vc,
        REG_CLK_EN_2, 12,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_jpeg = CV181X_CLK(CLK_APB_JPEG, "clk_apb_jpeg",
        clk_hw_parent_axi6,
        REG_CLK_EN_2, 13,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_h264c = CV181X_CLK(CLK_APB_H264C, "clk_apb_h264c",
        clk_hw_parent_axi6,
        REG_CLK_EN_2, 14,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_h265c = CV181X_CLK(CLK_APB_H265C, "clk_apb_h265c",
        clk_hw_parent_axi6,
        REG_CLK_EN_2, 15,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_cam0 = CV181X_CLK(CLK_CAM0, "clk_cam0",
        clk_cam0_parent,
        REG_CLK_EN_2, 16,
        REG_CLK_CAM0_SRC_DIV, 16, 6, -1,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        REG_CLK_CAM0_SRC_DIV, 8);
struct clk_hw clk_cam1 = CV181X_CLK(CLK_CAM1, "clk_cam1",
        clk_cam1_parent,
        REG_CLK_EN_2, 17,
        REG_CLK_CAM1_SRC_DIV, 16, 6, -1,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        REG_CLK_CAM1_SRC_DIV, 8);
struct clk_hw clk_csi_mac0_vip = CV181X_CLK(CLK_CSI_MAC0_VIP, "clk_csi_mac0_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 18,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_csi_mac1_vip = CV181X_CLK(CLK_CSI_MAC1_VIP, "clk_csi_mac1_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 19,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_isp_top_vip = CV181X_CLK(CLK_ISP_TOP_VIP, "clk_isp_top_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 20,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_img_d_vip = CV181X_CLK(CLK_IMG_D_VIP, "clk_img_d_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 21,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_img_v_vip = CV181X_CLK(CLK_IMG_V_VIP, "clk_img_v_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 22,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sc_top_vip = CV181X_CLK(CLK_SC_TOP_VIP, "clk_sc_top_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 23,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sc_d_vip = CV181X_CLK(CLK_SC_D_VIP, "clk_sc_d_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 24,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sc_v1_vip = CV181X_CLK(CLK_SC_V1_VIP, "clk_sc_v1_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 25,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sc_v2_vip = CV181X_CLK(CLK_SC_V2_VIP, "clk_sc_v2_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 26,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_sc_v3_vip = CV181X_CLK(CLK_SC_V3_VIP, "clk_sc_v3_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 27,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_dwa_vip = CV181X_CLK(CLK_DWA_VIP, "clk_dwa_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 28,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_bt_vip = CV181X_CLK(CLK_BT_VIP, "clk_bt_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 29,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_disp_vip = CV181X_CLK(CLK_DISP_VIP, "clk_disp_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 30,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_dsi_mac_vip = CV181X_CLK(CLK_DSI_MAC_VIP, "clk_dsi_mac_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_2, 31,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_lvds0_vip = CV181X_CLK(CLK_LVDS0_VIP, "clk_lvds0_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_3, 0,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_lvds1_vip = CV181X_CLK(CLK_LVDS1_VIP, "clk_lvds1_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_3, 1,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_csi0_rx_vip = CV181X_CLK(CLK_CSI0_RX_VIP, "clk_csi0_rx_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_3, 2,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_csi1_rx_vip = CV181X_CLK(CLK_CSI1_RX_VIP, "clk_csi1_rx_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_3, 3,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_pad_vi_vip = CV181X_CLK(CLK_PAD_VI_VIP, "clk_pad_vi_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_3, 4,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_1m = CV181X_CLK(CLK_1M, "clk_1m",
        clk_hw_parent_osc,
        REG_CLK_EN_3, 5,
        REG_DIV_CLK_1M, 16, 6, 25,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_spi = CV181X_CLK(CLK_SPI, "clk_spi",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_3, 6,
        REG_DIV_CLK_SPI, 16, 6, 8,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 30,
        0, -1,
        0, -1);
struct clk_hw clk_i2c = CV181X_CLK(CLK_I2C, "clk_i2c",
        clk_i2c_parent,
        REG_CLK_EN_3, 7,
        REG_DIV_CLK_I2C, 16, 4, 1,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 31,
        0, -1,
        0, -1);
struct clk_hw clk_pm = CV181X_CLK(CLK_PM, "clk_pm",
        clk_hw_parent_axi6,
        REG_CLK_EN_3, 8,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_timer0 = CV181X_CLK(CLK_TIMER0, "clk_timer0",
        clk_hw_parent_xtal_misc,
        REG_CLK_EN_3, 9,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_timer1 = CV181X_CLK(CLK_TIMER1, "clk_timer1",
        clk_hw_parent_xtal_misc,
        REG_CLK_EN_3, 10,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_timer2 = CV181X_CLK(CLK_TIMER2, "clk_timer2",
        clk_hw_parent_xtal_misc,
        REG_CLK_EN_3, 11,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_timer3 = CV181X_CLK(CLK_TIMER3, "clk_timer3",
        clk_hw_parent_xtal_misc,
        REG_CLK_EN_3, 12,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_timer4 = CV181X_CLK(CLK_TIMER4, "clk_timer4",
        clk_hw_parent_xtal_misc,
        REG_CLK_EN_3, 13,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_timer5 = CV181X_CLK(CLK_TIMER5, "clk_timer5",
        clk_hw_parent_xtal_misc,
        REG_CLK_EN_3, 14,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_timer6 = CV181X_CLK(CLK_TIMER6, "clk_timer6",
        clk_hw_parent_xtal_misc,
        REG_CLK_EN_3, 15,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_timer7 = CV181X_CLK(CLK_TIMER7, "clk_timer7",
        clk_hw_parent_xtal_misc,
        REG_CLK_EN_3, 16,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2c0 = CV181X_CLK(CLK_APB_I2C0, "clk_apb_i2c0",
        clk_hw_parent_axi4,
        REG_CLK_EN_3, 17,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2c1 = CV181X_CLK(CLK_APB_I2C1, "clk_apb_i2c1",
        clk_hw_parent_axi4,
        REG_CLK_EN_3, 18,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2c2 = CV181X_CLK(CLK_APB_I2C2, "clk_apb_i2c2",
        clk_hw_parent_axi4,
        REG_CLK_EN_3, 19,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2c3 = CV181X_CLK(CLK_APB_I2C3, "clk_apb_i2c3",
        clk_hw_parent_axi4,
        REG_CLK_EN_3, 20,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_apb_i2c4 = CV181X_CLK(CLK_APB_I2C4, "clk_apb_i2c4",
        clk_hw_parent_axi4,
        REG_CLK_EN_3, 21,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_wgn = CV181X_CLK(CLK_WGN, "clk_wgn",
        clk_hw_parent_osc,
        REG_CLK_EN_3, 22,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_wgn0 = CV181X_CLK(CLK_WGN0, "clk_wgn0",
        clk_hw_parent_osc,
        REG_CLK_EN_3, 23,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_wgn1 = CV181X_CLK(CLK_WGN1, "clk_wgn1",
        clk_hw_parent_osc,
        REG_CLK_EN_3, 24,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_wgn2 = CV181X_CLK(CLK_WGN2, "clk_wgn2",
        clk_hw_parent_osc,
        REG_CLK_EN_3, 25,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_keyscan = CV181X_CLK(CLK_KEYSCAN, "clk_keyscan",
        clk_hw_parent_osc,
        REG_CLK_EN_3, 26,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_ahb_sf1 = CV181X_CLK(CLK_AHB_SF1, "clk_ahb_sf1",
        clk_hw_parent_axi4,
        REG_CLK_EN_3, 27,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_vc_src1 = CV181X_CLK(CLK_VC_SRC1, "clk_vc_src1",
        clk_vc_src1_parent,
        REG_CLK_EN_3, 28,
        0, -1, 0, 0,
        0, -1, 0, 0,
        REG_CLK_BYP_1, 0,
        0, -1,
        0, -1);
struct clk_hw clk_src_vip_sys_2 = CV181X_CLK(CLK_SRC_VIP_SYS_2, "clk_src_vip_sys_2",
        clk_vip_parent,
        REG_CLK_EN_3, 29,
        REG_DIV_CLK_SRC_VIP_SYS_2, 16, 4, 2,
        0, -1, 0, 0,
        REG_CLK_BYP_1, 1,
        0, -1,
        REG_DIV_CLK_SRC_VIP_SYS_2, 8);
struct clk_hw clk_pad_vi1_vip = CV181X_CLK(CLK_PAD_VI1_VIP, "clk_pad_vi1_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_3, 30,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_cfg_reg_vip = CV181X_CLK(CLK_CFG_REG_VIP, "clk_cfg_reg_vip",
        clk_hw_parent_axi6,
        REG_CLK_EN_3, 31,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_cfg_reg_vc = CV181X_CLK(CLK_CFG_REG_VC, "clk_cfg_reg_vc",
        clk_hw_parent_axi6,
        REG_CLK_EN_4, 0,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_audsrc = CV181X_CLK(CLK_AUDSRC, "clk_audsrc",
        clk_hw_parent_ctal_a0pll,
        REG_CLK_EN_4, 1,
        REG_DIV_CLK_AUDSRC, 16, 8, 18,
        0, -1, 0, 0,
        REG_CLK_BYP_1, 2,
        0, -1,
        REG_DIV_CLK_AUDSRC, 8);
struct clk_hw clk_apb_audsrc = CV181X_CLK(CLK_APB_AUDSRC, "clk_apb_audsrc",
        clk_hw_parent_axi4,
        REG_CLK_EN_4, 2,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_vc_src2 = CV181X_CLK(CLK_VC_SRC2, "clk_vc_src2",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_4, 3,
        0, -1, 0, 0,
        0, -1, 0, 0,
        REG_CLK_BYP_1, 3,
        0, -1,
        0, -1);
struct clk_hw clk_pwm_src = CV181X_CLK(CLK_PWM_SRC, "clk_pwm_src",
        clk_hw_parent_xtal_fpll_disppll,
        REG_CLK_EN_4, 4,
        REG_DIV_CLK_PWM_SRC_0, 16, 6, 10,
        0, -1, 0, 0,
        REG_CLK_BYP_0, 15,
        0, -1,
        REG_DIV_CLK_PWM_SRC_0, 8);
struct clk_hw clk_ap_debug = CV181X_CLK(CLK_AP_DEBUG, "clk_ap_debug",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_4, 5,
        REG_DIV_CLK_AP_DEBUG, 16, 4, 5,
        0, -1, 0, 0,
        REG_CLK_BYP_1, 4,
        0, -1,
        0, -1);
struct clk_hw clk_src_rtc_sys_0 = CV181X_CLK(CLK_SRC_RTC_SYS_0, "clk_src_rtc_sys_0",
        clk_hw_parent_xtal_fpll,
        REG_CLK_EN_4, 6,
        REG_DIV_CLK_RTCSYS_SRC_0, 16, 4, 5,
        0, -1, 0, 0,
        REG_CLK_BYP_1, 5,
        0, -1,
        0, -1);
struct clk_hw clk_pad_vi2_vip = CV181X_CLK(CLK_PAD_VI2_VIP, "clk_pad_vi2_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 7,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_csi_be_vip = CV181X_CLK(CLK_CSI_BE_VIP, "clk_csi_be_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 8,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_vip_ip0 = CV181X_CLK(CLK_VIP_IP0, "clk_vip_ip0",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 9,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_vip_ip1 = CV181X_CLK(CLK_VIP_IP1, "clk_vip_ip1",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 10,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_vip_ip2 = CV181X_CLK(CLK_VIP_IP2, "clk_vip_ip2",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 11,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_vip_ip3 = CV181X_CLK(CLK_VIP_IP3, "clk_vip_ip3",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 12,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_c906_0 = CV181X_CLK(CLK_C906_0, "clk_c906_0",
        clk_c906_0_parent,
        REG_CLK_EN_4, 13,
        REG_DIV_CLK_C906_0_0, 16, 4, 1,
        REG_DIV_CLK_C906_0_1, 16, 4, 2,
        REG_CLK_BYP_1, 6,
        REG_CLK_SEL_0, 23,
        REG_DIV_CLK_C906_0_0, 8);
struct clk_hw clk_c906_1 = CV181X_CLK(CLK_C906_1, "clk_c906_1",
        clk_c906_1_parent,
        REG_CLK_EN_4, 14,
        REG_DIV_CLK_C906_1_0, 16, 4, 2,
        REG_DIV_CLK_C906_1_1, 16, 4, 2,
        REG_CLK_BYP_1, 7,
        REG_CLK_SEL_0, 24,
        REG_DIV_CLK_C906_1_0, 8);
struct clk_hw clk_src_vip_sys_3 = CV181X_CLK(CLK_SRC_VIP_SYS_3, "clk_src_vip_sys_3",
        clk_vip_parent,
        REG_CLK_EN_4, 15,
        REG_DIV_CLK_SRC_VIP_SYS_3, 16, 4, 2,
        0, -1, 0, 0,
        REG_CLK_BYP_1, 8,
        0, -1,
        REG_DIV_CLK_SRC_VIP_SYS_3, 8);
struct clk_hw clk_src_vip_sys_4 = CV181X_CLK(CLK_SRC_VIP_SYS_4, "clk_src_vip_sys_4",
        clk_vip_parent,
        REG_CLK_EN_4, 16,
        REG_DIV_CLK_SRC_VIP_SYS_4, 16, 4, 3,
        0, -1, 0, 0,
        REG_CLK_BYP_1, 9,
        0, -1,
        REG_DIV_CLK_SRC_VIP_SYS_4, 8);
struct clk_hw clk_ive_vip = CV181X_CLK(CLK_IVE_VIP, "clk_ive_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 17,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_raw_vip = CV181X_CLK(CLK_RAW_VIP, "clk_raw_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 18,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_osdc_vip = CV181X_CLK(CLK_OSDC_VIP, "clk_osdc_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 19,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_csi_mac2_vip = CV181X_CLK(CLK_CSI_MAC2_VIP, "clk_csi_mac2_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 20,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);
struct clk_hw clk_cam0_vip = CV181X_CLK(CLK_CAM0_VIP, "clk_cam0_vip",
        clk_hw_parent_axi_vip,
        REG_CLK_EN_4, 21,
        0, -1, 0, 0,
        0, -1, 0, 0,
        0, -1,
        0, -1,
        0, -1);

AOS_SLIST_HEAD(clk_list);

#define CLK_TYPE_XTAL 0
#define CLK_TYPE_PLL_G2 1
#define CLK_TYPE_PLL_G6 2
#define CLK_TYPE_NORMAL 3

/* G2 Synthesizer register offset */
#define G2_SSC_CTRL_MASK		0xFF
#define G2_SSC_CTRL_OFFSET		0x40
#define SSC_SYN_SET_MASK		0x0F
#define SSC_SYN_SET_OFFSET		0x04

#define div_mask(width) ((1 << (width)) - 1)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
# define do_div(n, base) ({					\
    uint32_t __base = (base);				\
    uint32_t __rem;						\
    __rem = ((uint64_t)(n)) % __base;			\
    (n) = ((uint64_t)(n)) / __base;				\
    __rem;							\
})
#ifndef BIT
#define BIT(s) (1 << (s))
#endif

static struct clk_resource cv181x_clks[CLK_MAX] = {
    {(void *)&clk_osc, CLK_TYPE_XTAL, 1, -1, 0, },
    {(void *)&clk_mpll, CLK_TYPE_PLL_G6, 1, -1, 0, },
    {(void *)&clk_tpll, CLK_TYPE_PLL_G6, 1, -1, 0, },
    {(void *)&clk_fpll, CLK_TYPE_PLL_G6, 1, -1, 0, },
    {(void *)&clk_mipimpll, CLK_TYPE_PLL_G2, 1, -1, 0, },
    {(void *)&clk_a0pll, CLK_TYPE_PLL_G2, 1, -1, 0, },
    {(void *)&clk_disppll, CLK_TYPE_PLL_G2, 1, -1, 0, },
    {(void *)&clk_cam0pll, CLK_TYPE_PLL_G2, 1, -1, 0, },
    {(void *)&clk_cam1pll, CLK_TYPE_PLL_G2, 1, -1, 0, },
    {(void *)&clk_a53, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_cpu_axi0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_cpu_gic, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_xtal_a53, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_tpu, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_tpu_fab, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_ahb_rom, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_ddr_axi_reg, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_rtc_25m, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_tempsen, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_saradc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_efuse, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_efuse, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_debug, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_xtal_misc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi4_emmc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_emmc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_100k_emmc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi4_sd0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sd0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_100k_sd0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi4_sd1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sd1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_100k_sd1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_spi_nand, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_500m_eth0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi4_eth0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_500m_eth1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi4_eth1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_gpio, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_gpio_intr, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_gpio_db, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_ahb_sf, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sdma_axi, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sdma_aud0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sdma_aud1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sdma_aud2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sdma_aud3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2c, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_wdt, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_pwm, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_spi0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_spi1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_spi2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_spi3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_cam0_200, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_uart0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_uart0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_uart1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_uart1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_uart2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_uart2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_uart3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_uart3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_uart4, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_uart4, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2s0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2s1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2s2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2s3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi4_usb, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_usb, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_125m_usb, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_33k_usb, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_12m_usb, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi4, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi6, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_dsi_esc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_src_vip_sys_0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_src_vip_sys_1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_disp_src_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_axi_video_codec, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_vc_src0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_h264c, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_h265c, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_jpeg, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_jpeg, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_h264c, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_h265c, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_cam0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_cam1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_csi_mac0_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_csi_mac1_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_isp_top_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_img_d_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_img_v_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sc_top_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sc_d_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sc_v1_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sc_v2_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_sc_v3_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_dwa_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_bt_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_disp_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_dsi_mac_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_lvds0_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_lvds1_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_csi0_rx_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_csi1_rx_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_pad_vi_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_1m, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_spi, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_i2c, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_pm, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_timer0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_timer1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_timer2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_timer3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_timer4, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_timer5, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_timer6, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_timer7, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2c0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2c1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2c2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2c3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_i2c4, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_wgn, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_wgn0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_wgn1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_wgn2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_keyscan, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_ahb_sf1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_vc_src1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_src_vip_sys_2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_pad_vi1_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_cfg_reg_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_cfg_reg_vc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_audsrc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_apb_audsrc, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_vc_src2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_pwm_src, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_ap_debug, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_src_rtc_sys_0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_pad_vi2_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_csi_be_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_vip_ip0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_vip_ip1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_vip_ip2, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_vip_ip3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_c906_0, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_c906_1, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_src_vip_sys_3, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_src_vip_sys_4, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_ive_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_raw_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_osdc_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_csi_mac2_vip, CLK_TYPE_NORMAL, 0, -1, 0, },
    {(void *)&clk_cam0_vip, CLK_TYPE_NORMAL, 0, -1, 0, }
};

static uint32_t clk_pll_rate_calc(uint32_t regval, s16 post_div_sel, uint32_t parent_rate)
{
    uint64_t numerator;
    uint32_t predivsel, postdivsel, divsel;
    uint32_t denominator;

    predivsel = regval & 0x7f;
    postdivsel = post_div_sel < 0 ? (regval >> 8) & 0x7f : (uint32_t)post_div_sel;
    divsel = (regval >> 17) & 0x7f;

    numerator = parent_rate * divsel;
    denominator = predivsel * postdivsel;
    do_div(numerator, denominator);

    return (uint32_t)numerator;
}

static uint32_t clk_pll_g6_calc_freq_rate(uint8_t id,
                           uint32_t parent_rate)
{
    struct clk_pll *pll_hw = (struct clk_pll *)cv181x_clks[id].clk;
    uint32_t rate;
    uint32_t regval;

    regval = mmio_read_32(pll_hw->reg_csr);
    rate = clk_pll_rate_calc(regval, pll_hw->post_div_sel, parent_rate);

    return rate;
}

static uint32_t clk_pll_g2_calc_freq_rate(uint8_t id,
                           uint32_t parent_rate)
{
    struct clk_pll *pll_hw = (struct clk_pll *)cv181x_clks[id].clk;
    uint32_t reg_ssc_set;
    uint32_t reg_g2_ssc_ctrl;
    uint32_t regval_csr;
    uint32_t regval_ssc_set;
    uint32_t regval_g2_ssc_ctrl;
    uint64_t numerator;
    uint32_t denominator;
    uint32_t clk_ref;
    uint32_t rate;

    regval_csr = mmio_read_32(pll_hw->reg_csr);

    /* pll without synthesizer */
    if (pll_hw->reg_ssc == 0) {
        clk_ref = parent_rate;
        goto rate_calc;
    }

    /* calculate synthesizer freq */
    reg_ssc_set = (pll_hw->reg_ssc & ~SSC_SYN_SET_MASK) + SSC_SYN_SET_OFFSET;
    reg_g2_ssc_ctrl = (pll_hw->reg_ssc & ~G2_SSC_CTRL_MASK) + G2_SSC_CTRL_OFFSET;

    regval_ssc_set = mmio_read_32(reg_ssc_set);
    regval_g2_ssc_ctrl = mmio_read_32(reg_g2_ssc_ctrl);

    /* bit0 sel_syn_clk */
    numerator = (regval_g2_ssc_ctrl & 0x1) ? parent_rate : (parent_rate >> 1);

    numerator <<= 26;
    denominator = regval_ssc_set;
    if (denominator)
        do_div(numerator, denominator);
    else
        printf("pll ssc_set is zero\n");

    clk_ref = numerator;

rate_calc:
    rate = clk_pll_rate_calc(regval_csr, pll_hw->post_div_sel, clk_ref);

    return rate;
}

static int clk_hw_is_bypassed(uint8_t id)
{
    struct clk_hw *clk_hw = (struct clk_hw *)cv181x_clks[id].clk;
    uint32_t reg_addr = clk_hw->mux[0].reg;
    int val;

    if (clk_hw->mux[0].shift >= 0) {
        val = mmio_read_32(reg_addr) >> clk_hw->mux[0].shift;
        val &= 0x1; //width
    } else {
        val = 0;
    }

    return val;
}

static int clk_hw_get_clk_sel(uint8_t id)
{
    uint32_t val;
    struct clk_hw *clk_hw = (struct clk_hw *)cv181x_clks[id].clk;
    uint32_t reg_addr = clk_hw->mux[1].reg;

    if (clk_hw->mux[1].shift >= 0) {
        val = mmio_read_32(reg_addr) >> clk_hw->mux[1].shift;
        val &= 0x1; //width
        val ^= 0x1; //invert value
    } else {
        val = 0;
    }

    return val;
}

static int clk_hw_get_src_sel(uint8_t id)
{
    uint32_t val;
    struct clk_hw *clk_hw = (struct clk_hw *)cv181x_clks[id].clk;
    uint32_t reg_addr = clk_hw->mux[2].reg;

    if (clk_hw->mux[2].shift >= 0) {
        val = mmio_read_32(reg_addr) >> clk_hw->mux[2].shift;
        val &= 0x3; //width
    } else {
        val = 0;
    }

    return val;
}

static uint8_t clk_hw_get_parent(uint8_t id) {
    struct clk_hw *clk = (struct clk_hw *)cv181x_clks[id].clk;
    uint8_t clk_sel = clk_hw_get_clk_sel(id);
    uint8_t src_sel = clk_hw_get_src_sel(id);
    uint8_t parent_idx = 0;

    /*
     * | 0     | 1     | 2     | 3     | 4     | 5     |
     * +-------+-------+-------+-------+-------+-------+
     * | XTAL  | DIV_1 | src_0 | src_1 | src_2 | src_3 |
     * | XTAL  | src_0 | src_1 | src_2 | src_3 |       |
     * | DIV_1 | src_0 | src_1 | src_2 | src_3 |       |
     * | src_0 | src_1 | src_2 | src_3 |       |       |
     * +-------+-------+-------+-------+-------+-------+
     */

    if (clk->mux[0].shift >= 0) {
        // clk with bypass reg
        if (clk_hw_is_bypassed(id)) {
            parent_idx = 0;
        } else {
            if (clk->mux[1].shift >= 0) {
                // clk with clk_sel reg
                if (clk_sel) {
                    parent_idx = 1;
                } else {
                    parent_idx = src_sel + 2;
                }
            } else {
                // clk without clk_sel reg
                parent_idx = src_sel + 1;
            }
        }
    } else {
        // clk without bypass reg
        if (clk->mux[1].shift >= 0) {
            // clk with clk_sel reg
            if (clk_sel) {
                parent_idx = 0;
            } else {
                parent_idx = src_sel + 1;
            }
        } else {
            //clk without clk_sel reg
            parent_idx = src_sel;
        }
    }
    return clk->parents_tbl[parent_idx];
}

static int clk_is_enabled(uint8_t id) {
    uint32_t val;
    struct clk_hw *clk = (struct clk_hw *)cv181x_clks[id].clk;

    if (cv181x_clks[id].type != CLK_TYPE_NORMAL) {
        return 1;
    }

    if (clk->gate.shift < 0) {
        printf("ERROR gate shift");
    }
    val = mmio_read_32(clk->gate.reg);
    val &= BIT(clk->gate.shift);

    return val ? 1 : 0;
}

static int16_t clk_get_parent(uint8_t id) {
    int16_t parent_id = -1;

    switch(cv181x_clks[id].type) {
    case CLK_TYPE_XTAL:
        parent_id = -1;
        break;
    case CLK_TYPE_PLL_G2:
        if (id == CLK_MIPIMPLL)
            parent_id = CLK_XTAL;
        else
            parent_id = CLK_MIPIMPLL;
        break;
    case CLK_TYPE_PLL_G6:
        parent_id = CLK_XTAL;
        break;
    case CLK_TYPE_NORMAL:
        parent_id = clk_hw_get_parent(id);
        break;
    }
    return parent_id;
}

static uint32_t clk_hw_calc_freq_rate(uint8_t id,
                          uint32_t parent_rate)
{
    struct clk_hw *clk_hw = (struct clk_hw *)cv181x_clks[id].clk;
    uint8_t clk_sel = clk_hw_get_clk_sel(id);
    uint32_t reg_addr = clk_hw->div[clk_sel].reg;
    uint32_t val;
    uint32_t rate = 0;

    if ((clk_hw->mux[0].shift >= 0) && clk_hw_is_bypassed(id))
        return parent_rate;
    if ((clk_hw->div[0].reg == 0) && (clk_hw->div[1].reg == 0))
        return parent_rate;

    if ((clk_hw->div[clk_sel].initval > 0) && !(mmio_read_32(reg_addr) & BIT(3))) {
        val = clk_hw->div[clk_sel].initval;
    } else {
        val = mmio_read_32(reg_addr) >> clk_hw->div[clk_sel].shift;
        val &= div_mask(clk_hw->div[clk_sel].width);
    }
    rate = DIV_ROUND_UP(parent_rate, val);

    return rate;
}

static uint32_t clk_get_freq_rate(uint8_t id) {
    uint32_t parent_rate, rate = 0;
    int16_t parent_id;
    struct clk_fixed *fixed_clk = (struct clk_fixed *)cv181x_clks[id].clk;

    if (cv181x_clks[id].freq_rate)
        return  cv181x_clks[id].freq_rate;

    parent_id = clk_get_parent(id);
    switch(cv181x_clks[id].type) {
    case CLK_TYPE_XTAL:
        rate = fixed_clk->freq_rate;
        break;
    case CLK_TYPE_PLL_G2:
        parent_rate = clk_get_freq_rate(parent_id);
        rate = clk_pll_g2_calc_freq_rate(id, parent_rate);
        break;
    case CLK_TYPE_PLL_G6:
        parent_rate = clk_get_freq_rate(parent_id);
        rate = clk_pll_g6_calc_freq_rate(id, parent_rate);
        break;
    case CLK_TYPE_NORMAL:
        parent_rate = clk_get_freq_rate(parent_id);
        rate = clk_hw_calc_freq_rate(id, parent_rate);
        break;
    }
    return rate;
}

static uint8_t indent = 0;
void show_clk_info(slist_t *slist) {
    struct clk_resource *tmp;
    if (slist!=NULL) {
        tmp = slist_entry(slist, struct clk_resource, list);
        for (uint8_t i=0; i<indent;i++) printf(" ");
        printf("%*s %7d %10d\n", indent-24, ((struct clk_fixed *)tmp->clk)->name, tmp->enabled, tmp->freq_rate);
        aos_msleep(2);

        if (tmp->sublist.next != NULL) {
            tmp = slist_entry(tmp->sublist.next, struct clk_resource, list);
            indent++;
            show_clk_info(&tmp->list);
            indent--;
        }

        if (slist->next != NULL) {
            show_clk_info(slist->next);
        }
    }
    return;
}

void clk_get_status() {
    slist_init(&clk_list);
    for(int i=0; i<CLK_MAX; i++) {
        cv181x_clks[i].freq_rate = 0;
        slist_init(&cv181x_clks[i].list);
        slist_init(&cv181x_clks[i].sublist);
    }
    for(int i=0; i<CLK_MAX; i++) {
        cv181x_clks[i].enabled = clk_is_enabled(i);
        cv181x_clks[i].freq_rate = clk_get_freq_rate(i);
        cv181x_clks[i].parent_id = clk_get_parent(i);

        if (cv181x_clks[i].parent_id < 0)
            slist_add_tail(&cv181x_clks[i].list, &clk_list);
        else
            slist_add_tail(&cv181x_clks[i].list, &cv181x_clks[cv181x_clks[i].parent_id].sublist);

    }

    printf("%-24s enabled frequency\n", "clk_name");
    show_clk_info(clk_list.next);
}
ALIOS_CLI_CMD_REGISTER(clk_get_status, clkstat, dump clock status);
