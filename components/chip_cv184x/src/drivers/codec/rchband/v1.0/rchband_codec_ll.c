#include "rchband_codec_ll.h"

#define CODEC_BASE_ADDR		0x60044000U

extern void mdelay(uint32_t ms);

volatile uint8_t back_mic_baise_reg;

volatile uint8_t CodecRegValue[100];
volatile uint8_t CodecIp;

// Codec延时初始化时间，单位10mS
volatile uint32_t  codec_delay_init_timer;

volatile uint8_t real_hp_volume = 25U, real_mic_volume;
#define    HP_OUT_MAX_VOLUME_DB         0x2fU

// 0x28*0.75 + 8 = 40*0.75+8 = 38DB
#define    MIC_IN_MAX_VOLUME_DB         0x28U

void codec_write_delay(uint32_t delay_ms)
{
    mdelay(delay_ms);
}

unsigned char read_codec_reg(unsigned char addr)
{
    return (unsigned char)(*(volatile uint32_t *) ((uint32_t )addr + CODEC_BASE_ADDR));
}

void write_codec_reg(unsigned char addr,unsigned char b)
{
    (*(volatile unsigned char *) ((uint32_t)addr + CODEC_BASE_ADDR)) = b;
}

/*
 *    CODEC I2S Master, 22Bit
 *    WN8032 I2S Slave
 *    48K: MCLK 12.288
 */
void rchband_codec_init(codec_config_t  config)
{
    uint32_t i;

    back_mic_baise_reg = 0x00U;

    i = 0U;

    while (CodecRegPartOne[i][0] != 0xffU) {
        if (CodecRegPartOne[i][0] < 0x70U) {
            write_codec_reg(CodecRegPartOne[i][0], CodecRegPartOne[i][1]);
        } else if (CodecRegPartOne[i][0] == 0xFEU) {
            codec_write_delay((uint32_t)CodecRegPartOne[i][1]);
        }

        i++;
    }

    if (config.e_output_path == HP_OUT_SINGLE_ENDED) {
        codec_delay_init_timer = 320U;
    } else {
        codec_delay_init_timer = 50U;
    }

    for (i = 0U; i < 90U; i++) {
        CodecRegValue[i] = 0x00U;  ///< read_codec_reg(i+1);
    }


    mdelay(codec_delay_init_timer * 10U);
    write_codec_reg(0x0FU, 0x7DU);		// // PowerUp DAC Current Reference,DAC Regulator
    mdelay(80U);

    if (config.e_output_path == HP_OUT_DIFFERENTIAL) {
        write_codec_reg(0x13U, 0x7CU | DAC_DIFFER_END);	// differ
    } else {
        write_codec_reg(0x13U, 0x7CU | DAC_SINGLE_END);	// Single-ended or Virtual Ground
    }

    mdelay(30U);

    if (config.e_output_path == HP_OUT_DIFFERENTIAL) {
        write_codec_reg(0x13U, 0x00U | DAC_DIFFER_END);	// differ
    } else {
        write_codec_reg(0x13U, 0x00U | DAC_SINGLE_END);	// Single-ended or Virtual Ground
    }

    mdelay(20U);
    i = 0U ;

    while (CodecRegPartTwo[i][0] != 0xffU) {
        if (CodecRegPartTwo[i][0] < 0x70U) {
            write_codec_reg(CodecRegPartTwo[i][0], CodecRegPartTwo[i][1]);
        }

        i++;
    }

    mdelay(10U);
    write_codec_reg(0x1EU, 0x00U);		// DAC Internal Master Clock Divider of MCLK from SoC,set 000 only
    write_codec_reg(0x0BU, 0x00U);		// Enable PCM digital loopback from ADC to DAC
    mdelay(40U);
    write_codec_reg(0x40U, 0x04U);		// Soft Reset to ADC Analog Core, Active High
    mdelay(10U);
    write_codec_reg(0x40U, 0x00U);		// DAI Interface,ACLK_O : disable
    write_codec_reg(0x41U, 0x02U);		// AD I2S,16bit
    write_codec_reg(0x42U, 0x2CU);		// AD High Pass Filter,3.73Hz
    write_codec_reg(0x43U, 0x04U);		// ACLK_O = MCLK1/AD_MCLKDIV = MCLK1/4
    write_codec_reg(0x44U, 0x90U);		// Reset Sinc Filter,
    mdelay(10U);
    i = 0U;

    while (CodecRegPartThree[i][0] != 0xffU) {
        if (CodecRegPartThree[i][0] < 0x70U) {
            write_codec_reg(CodecRegPartThree[i][0], CodecRegPartThree[i][1]);
        }

        i++;
    }

    mdelay(10U);
    rchband_codec_set_input_path(config.e_input_path);
    mdelay(10U);
    rchband_codec_power_on_dac();
    rchband_codec_set_dac_volume_per((uint8_t)real_hp_volume, HP_OUT_MAX_VOLUME_DB);
    mdelay(10U);
    write_codec_reg(0x09U, 0x86U); //关闭静音
    rchband_codec_set_mic_volume_per((uint8_t)real_mic_volume, MIC_IN_MAX_VOLUME_DB); // 初始化完成
    rchband_codec_set_mic_bias_power(true);     // 打开mic bias
}

/*
 *  复位初始化状态机到重新初始化Line In
 */
uint32_t rchband_codec_get_retry_init_mic_tick(void)
{
    return (codec_delay_init_timer + 15U);
}

/*
 *  读取CODEC所有寄存器到CodecRegValue数组
 */
void rchband_codec_read_all_reg(void)
{
    uint8_t i;

    CodecIp = read_codec_reg(0U);

    for (i = 0U; i < 90U; i++) {
        CodecRegValue[i] = read_codec_reg(i + 1U);
    }
}

void rchband_codec_write_codec_regVal(uint8_t addr, uint8_t val)
{
    write_codec_reg(addr, val);
}

uint8_t rchband_codec_read_codec_regVal(uint8_t addr)
{
    uint8_t val;
    val = read_codec_reg(addr);
    return val;
}

/*
 *  功能描述： 控制Codec进入低功耗模式
 *  输入参数：
 */
uint8_t rchband_codec_power_down_codec()
{
    uint32_t k, val;
    write_codec_reg(0x01U, 0x04U);	// soft reset DAC
    write_codec_reg(0x40U, 0x04U);	// soft reset ADC

    for (k = 0U; k < 10U; k++) {
        val = (uint32_t)read_codec_reg(0x04U);
    }

    write_codec_reg(0x01U, 0x13U);
    write_codec_reg(0x40U, 0x43U);
    return (uint8_t)val;
}



/*
 *  功能描述： 设置耳机输出是否静音
 *  输入参数： f_mute
 *            true: 静音
 *            false: 非静音
 */
void  rchband_codec_set_dac_mute(bool f_mute)
{
    if (f_mute) {
        write_codec_reg(0x09U, 0x87U);
    } else {
        write_codec_reg(0x09U, 0x86U);
    }
}

/*
 *	功能描述： 控制麦克风baise电压是否输出
 *  输入参数： f_power_up
 *            true: 开mic baise
 *            false: 关mic baise *
 */
void rchband_codec_set_mic_bias_power(bool f_power_up)
{
    if (back_mic_baise_reg != 0U) {
        if (f_power_up == true) {
            back_mic_baise_reg |= 0x40U;
            write_codec_reg(0x4BU, back_mic_baise_reg);
        } else {
            back_mic_baise_reg &= 0xBFU;
            write_codec_reg(0x4BU, back_mic_baise_reg);
        }
    }
}

/*
 *  功能描述： 控制耳机输出驱动电源
 *  输入参数： f_power_on
 *            true: 打开耳机输出电源
 *            false: 关闭耳机输出电源
 */
void  rchband_codec_set_dac_power_on(bool f_power_on)
{
    if (f_power_on) {
        write_codec_reg(0x0FU, 0x3DU);	// 0x7D
    } else {
        write_codec_reg(0x0FU, 0x3CU);	// 0x7C
    }
}

/*
 * 功能描述： 使能耳机输出驱动，关闭静音
 *           主要用在Codec初始后，打开耳机功能
 */
void rchband_codec_power_on_dac(void)
{
    rchband_codec_set_dac_power_on((bool)true);
    rchband_codec_set_dac_mute((bool)false);
}

/*
 * 功能描述： 设置Codec输入路径
 * 输入参数： path
 *     MIC_IN_SINGLE_ENDED   :  麦克风单端输入
 *     MIC_IN_DIFFERENTIAL   :  麦克风差分输入
 *     LINE_IN_SINGLE_ENDED  :  线性单端输入
 *     LINE_IN_DIFFERENTIAL  :  线性差分输入
 */
void rchband_codec_set_input_path(CODEC_INPUT_PATH_E path)
{
    if (path == MIC_IN_SINGLE_ENDED) {
        back_mic_baise_reg = 0x9FU;
        write_codec_reg(0x4BU, back_mic_baise_reg);
        write_codec_reg(0x4CU, 0x3CU);
    } else if (path == MIC_IN_DIFFERENTIAL) {
        back_mic_baise_reg = 0x9FU;
        write_codec_reg(0x4BU, back_mic_baise_reg);
        write_codec_reg(0x4CU, 0x1EU);
    } else if (path == LINE_IN_SINGLE_ENDED) {
        back_mic_baise_reg = 0x9DU;
        write_codec_reg(0x4BU, back_mic_baise_reg);
        write_codec_reg(0x4CU, 0x20U);
    } else if (path == LINE_IN_DIFFERENTIAL) {
        back_mic_baise_reg = 0x9DU;
        write_codec_reg(0x4BU, back_mic_baise_reg);
        write_codec_reg(0x4CU, 0x12U);
    }
}

/*
 *  功能描述： 设置麦克风输入放大增益
 *  输入参数： vol_db
 *            放大增益,范围 0 ~ 0x3f，步进0.75DB
 *            20DB + vol_db*0x75
 *            0（8DB）, 1(8.75DB) ... 0x10(20DB) ... 0x3f(55.25DB)
 */
bool rchband_codec_set_mic_volume(uint8_t vol_db)
{
    bool ret = (bool)true;

    if (vol_db <= 0x3fU) {
        write_codec_reg(0x51U, vol_db | 0xC0U);
        write_codec_reg(0x52U, vol_db | 0xC0U);
        ret = (bool)true;
    } else {
        ret = (bool)false;
    }
    return ret;
}

/*
 *  功能描述： 设置麦克风音量
 *  输入参数：
 *       volume
 *            麦克风音量百分比，范围：0 ~ 100
 *       max_db
 *            最在输出音量，0x00 ~ 0x3F,步进0.75DB
 *            20DB + vol_db*0x75
 *            0（8DB）, 1(8.75DB) ... 0x10(20DB) ... 0x3f(55.25DB)
 *
 */
uint8_t rchband_codec_set_mic_volume_per(uint8_t per, uint8_t max_db)
{
    uint16_t  temp, max_temp;

    temp = (uint16_t)per;

    if (temp > 100U) {
        temp = 100U;
    }

    max_temp = (uint16_t)max_db;

    if (max_temp > 0x3fU) {
        max_temp = 0x3fU;
    }

    temp *= max_temp;
    temp /= 100U;

    if (temp > 0x3fU) {
        temp = 0x3fU;
    }

    rchband_codec_set_mic_volume((uint8_t)((uint8_t)temp & 0xffU));
    return (uint8_t)((uint8_t)temp & 0xffU);
}

/*
 *  功能描述： 设置线性输入放大增益
 *  输入参数： vol_db
 *            放大增益,范围 0 ~ 0x3f，步进0.75DB
 *            20DB + vol_db*0x75
 *            0（-12DB）, 1(-11.25DB) ... 0x10(0DB) ... 0x3f(35.25DB)
 */
bool rchband_codec_set_line_in_volume(uint8_t vol_db)
{
    bool ret = (bool)true;
    if (vol_db <= (uint8_t)0x3f) {
        write_codec_reg(0x51U, vol_db);
        write_codec_reg(0x52U, vol_db);
         ret = (bool)true;
    } else {
        ret = (bool)false;
    }
    return ret;
}

/*
 *  功能描述： 设置耳机输出音量
 *  输入参数： volume
 *            耳机输出音量，范围：0 ~ 0x3f,步进0.75DB
 *            0（-35.25DB）... 0x2f(0DB) ... 0x3f(12DB)
 */
uint8_t rchband_codec_set_dac_volume(uint8_t volume)
{
    uint8_t reg;

    reg = volume ;

    if (reg == (uint8_t)0U) {
        rchband_codec_set_dac_mute((bool)true);
    } else {
        rchband_codec_set_dac_mute((bool)false);
    }

    write_codec_reg(0x17U, reg);
    write_codec_reg(0x18U, reg);
    return 1U;
}

/*
 *  功能描述： 设置耳机输出音量
 *  输入参数：
 *       volume
 *            耳机输出音量百分比，范围：0 ~ 100
 *       max_db
 *            最在输出音量，0x00 ~ 0x3F,步进0.75DB
 *             0（-35.25DB）... 0x2f(0DB) ... 0x3f(12DB)
 *            CODEC Gain最大可设到12DB,但是很多耳机的最大音量不需要输出到12DB
 *
 */
uint8_t rchband_codec_set_dac_volume_per(uint8_t per, uint8_t max_db)
{
    uint16_t  temp, max_temp;

    temp = (uint16_t)per;

    if (temp > 100U) {
        temp = 100U;
    }

    max_temp = (uint16_t)max_db;

    if (max_temp > 0x3fU) {
        max_temp = 0x3fU;
    }

    temp *= max_temp;
    temp /= 100U;

    if (temp > 0x3fU) {
        temp = 0x3fU;
    }

    if (per == 0U) {
        // 0% 时静音
        rchband_codec_set_dac_mute((bool)true);
    } else {
        rchband_codec_set_dac_mute((bool)false);
    }

    rchband_codec_set_dac_volume((uint8_t)temp);
    return (uint8_t)temp;
}

/*
 *  功能描述： 设置MIC BIAS输出电压
 *  输入参数： bias
 *            BIAS_1V57 = 1.57V
 *            BIAS_1V81 = 1.81V
 *            BIAS_1V96 = 1.96V
 *            BIAS_2V35 = 2.35V
 */
bool rchband_codec_set_mic_bias(CODEC_MIC_BIAS_E bias)
{
    bool ret = (bool)true;
    if (bias <= BIAS_2V35) {
        write_codec_reg(0x4EU, (((uint8_t)bias & (uint8_t)0x03U) << 4U));
        ret =  (bool)true;
    } else {
        ret = (bool)false;
    }
    return ret;
}

/*
 * 向CODEC中连续写入寄存器
 * 输入参数
 *     addr : 启始地址
 *     len  : 写入寄存器长度
 *     pbuf : 写入数据指针
*/
bool  rchband_codec_write_block_reg(uint8_t addr, uint8_t len, uint8_t *pbuf)
{
    uint8_t i, reg_addr;
    reg_addr = addr;

    for (i = 0U; i < len; i++) {
        write_codec_reg(reg_addr, pbuf[i]);
        reg_addr ++;
    }

    return (bool)true;
}

/*
 *  配置寄存器，解决Codec掉电时，单端耳机产生的噪音
 */
void rchband_codec_power_down_setting(void)
{
    write_codec_reg(0x0fU, 0x3DU);
    write_codec_reg(0x09U, 0x87U);
    write_codec_reg(0x0fU, 0x7DU);
    write_codec_reg(0x0fU, 0x48U);
    // delay 1 Sec
    codec_write_delay(1000U);
    write_codec_reg(0x0fU, 0x40U);
    write_codec_reg(0x13U, 0x01U);
    write_codec_reg(0x12U, 0x00U);
}

uint32_t rchband_codec_dac_sample_width(uint32_t sample_width)
{
    uint32_t ret = 0U;

    if (sample_width == 16U) {
        write_codec_reg(0x02U, 0x2U);
        ret = 0U;
    } else if (sample_width == 18U) {
        write_codec_reg(0x02U, 0x6U);
        ret =  0U;
    } else if (sample_width == 20U) {
        write_codec_reg(0x02U, 0xAU);
        ret =  0U;
    } else if (sample_width == 22U) {
        write_codec_reg(0x02U, 0xEU);
        ret =  0U;
    }else{
        ret =  1U;
    }

    return ret;
}

uint32_t rchband_codec_dac_sample_rate(uint32_t sample_rate)
{
    uint32_t ret = 0U;

    if (sample_rate == 32000U) {
        write_codec_reg(0x04U, 0x1U);
        ret = 0U;
    } else if (sample_rate == 44100U) {
        write_codec_reg(0x04U, 0x2U);
        ret = 0U;
    } else if (sample_rate == 48000U) {
        write_codec_reg(0x04U, 0x3U);
        ret = 0U;
    }else{
        ret =  1U;
    }

    return ret;
}

uint32_t rchband_codec_adc_sample_width(uint32_t sample_width)
{
    uint32_t ret = 0U;

    if (sample_width == 16U) {
        write_codec_reg(0x41U, 0x2U);
        ret = 0U;
    } else if (sample_width == 18U) {
        write_codec_reg(0x41U, 0x6U);
        ret = 0U;
    } else if (sample_width == 20U) {
        write_codec_reg(0x41U, 0xAU);
        ret = 0U;
    } else if (sample_width == 22U) {
        write_codec_reg(0x41U, 0xEU);
        ret = 0U;
    }else{
        ret = 1U;
    }
    return ret;
}

void rchband_codec_input_ch_config(uint32_t is_difference)
{
    if (is_difference == 1U) {
        back_mic_baise_reg = 0x9FU;
        write_codec_reg(0x4BU, back_mic_baise_reg);
        write_codec_reg(0x4CU, 0x1EU);
    } else {
        back_mic_baise_reg = 0x9FU;
        write_codec_reg(0x4BU, back_mic_baise_reg);
        write_codec_reg(0x4CU, 0x3CU);
    }

    rchband_codec_set_mic_bias_power((bool)true);     // 打开mic bias
}

uint32_t rchband_codec_output_ch_config(uint32_t is_difference)
{
    if (is_difference == 1U) {
        write_codec_reg(0x13U, 0x7CU | DAC_DIFFER_END);	// differ
        mdelay(30U);
        write_codec_reg(0x13U, 0x00U | DAC_DIFFER_END);	// differ
    } else {
        write_codec_reg(0x13U, 0x7CU | DAC_SINGLE_END);	// Single-ended or Virtual Ground
        mdelay(30U);
        write_codec_reg(0x13U, 0x00U | DAC_SINGLE_END);	// Single-ended or Virtual Ground
    }

    return 1U;
}

void rchband_codec_resume_form_low_power(void)
{
    write_codec_reg(0x09U, CodecRegValue[0x09]);   ///< dac mute config
    mdelay(10U);
    write_codec_reg(0x13U, CodecRegValue[0x13]);   ///< output path config
    mdelay(30U);
    write_codec_reg(0x17U, CodecRegValue[0x17]);   ///< dac left gain config
    write_codec_reg(0x18U, CodecRegValue[0x18]);   ///< dac right gain config
    mdelay(30U);
    write_codec_reg(0x41U, CodecRegValue[0x41]);   ///< adc sample width config
    write_codec_reg(0x4CU, CodecRegValue[0x4C]);   ///< input path config
    write_codec_reg(0x4BU, CodecRegValue[0x4B]);   ///< mic power config
    write_codec_reg(0x4EU, CodecRegValue[0x4E]);   ///< mic voltage config
    write_codec_reg(0x51U, CodecRegValue[0x51]);   ///< mic left gain config
    write_codec_reg(0x52U, CodecRegValue[0x52]);   ///< mic right gain config
    write_codec_reg(0x0FU, CodecRegValue[0x0F]);   ///< dac power config
    mdelay(80U);
}
