/*
 *	Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file    baud_calc.c
 * @brief   source file for the PWM capture uart bandrate driver
 * @version V1.0
 * @date    30. July 2020
 * ******************************************************/
#include <drv/baud_calc.h>

#define PWM_CONFIG_SAMPLING_TIMES                   (24U)                 ///< Minimum unit of sampling times
#define PWM_CONFIG_COMPARE_TIMES                    ( 1U)                 ///< Sampling result comparison times
#define PWM_CONFIG_PERCENTAGE                       ( 5U)                 ///< Upper and lower return difference (percentage)

#define PWM_REGS_BASEADDRESS                        (WJ_PWM_BASE)
#define PWM_REGS_ADDR_PWMCFG                       (*(volatile uint32_t *)(PWM_REGS_BASEADDRESS))
#define PWM_REGS_ADDR_CAPCTL                       (*(volatile uint32_t *)(PWM_REGS_BASEADDRESS + 0x74U))
#define PWM_REGS_ADDR_CAPINTEN                     (*(volatile uint32_t *)(PWM_REGS_BASEADDRESS + 0x78U))
#define PWM_REGS_ADDR_CAPIS                        (*(volatile uint32_t *)(PWM_REGS_BASEADDRESS + 0x84U))
#define PWM_REGS_ADDR_CAPIC                        (*(volatile uint32_t *)(PWM_REGS_BASEADDRESS + 0x80U))
#define PWM_REGS_ADDR_CAPCOUNT_L(_CH_)             (*(volatile uint32_t *)(PWM_REGS_BASEADDRESS + 0x88U + ((_CH_ / 2U) << 2)))
#ifdef CONFIG_BAUD_JUPITER_BC


#define PWM_REGS_ADDR_CAPCOUNT_H(_CH_)             (*(volatile uint32_t *)(PWM_REGS_BASEADDRESS + 0x128U + ((_CH_ / 2U) << 2)))
#define PWM_REGS_ADDR_CAPCOUNT(_CH_)                ((PWM_REGS_ADDR_CAPCOUNT_H(_CH_) & 0xFFFF0000U) + (PWM_REGS_ADDR_CAPCOUNT_L(_CH_) >> (_CH_ % 2U << 4)))
#define PMW_CAL_LOOP_VALUE(_STOP_, _START_)         ((_STOP_ >= _START_) ? (_STOP_ - _START_) : (((0xFFFFFFFFU - _START_) + 1U) + _STOP_))

#endif

#ifdef CONFIG_BAUD_DANICA


#define PWM_REGS_ADDR_CAPCOUNT(_CH_)                (PWM_REGS_ADDR_CAPCOUNT_L(_CH_) >> (_CH_ % 2U << 4))
#define PMW_CAL_LOOP_VALUE(_STOP_, _START_)         ((_STOP_ >= _START_) ? (_STOP_ - _START_) : (((0xFFFFU - _START_) + 1U) + _STOP_))

#endif

#define PWM_BAUDRATE_0000110                       (    110U)
#define PWM_BAUDRATE_0000300                       (    300U)
#define PWM_BAUDRATE_0000600                       (    600U)
#define PWM_BAUDRATE_0001200                       (   1200U)
#define PWM_BAUDRATE_0002400                       (   2400U)
#define PWM_BAUDRATE_0004800                       (   4800U)
#define PWM_BAUDRATE_0009600                       (   9600U)
#define PWM_BAUDRATE_0014400                       (  14400U)
#define PWM_BAUDRATE_0019200                       (  19200U)
#define PWM_BAUDRATE_0038400                       (  38400U)
#define PWM_BAUDRATE_0056000                       (  56000U)
#define PWM_BAUDRATE_0057600                       (  57600U)
#define PWM_BAUDRATE_0115200                       ( 115200U)
#define PWM_BAUDRATE_0128000                       ( 128000U)
#define PWM_BAUDRATE_0230400                       ( 230400U)
#define PWM_BAUDRATE_0256000                       ( 256000U)
#define PWM_BAUDRATE_0460800                       ( 460800U)
#define PWM_BAUDRATE_0500000                       ( 500000U)
#define PWM_BAUDRATE_0512000                       ( 512000U)
#define PWM_BAUDRATE_0600000                       ( 600000U)
#define PWM_BAUDRATE_0750000                       ( 750000U)
#define PWM_BAUDRATE_0921600                       ( 921600U)
#define PWM_BAUDRATE_1000000                       (1000000U)
#define PWM_BAUDRATE_1280000                       (1280000U)
#define PWM_BAUDRATE_1500000                       (1500000U)
#define PWM_BAUDRATE_2000000                       (2000000U)
#define PWM_BAUDRATE_2500000                       (2500000U)
#define PWM_BAUDRATE_3000000                       (3000000U)
#define PWM_BAUDRATE_3500000                       (3500000U)

const uint32_t baud_table[] = {
    PWM_BAUDRATE_0000110, PWM_BAUDRATE_0000300, PWM_BAUDRATE_0000600, PWM_BAUDRATE_0001200,
    PWM_BAUDRATE_0002400, PWM_BAUDRATE_0004800, PWM_BAUDRATE_0009600, PWM_BAUDRATE_0014400,
    PWM_BAUDRATE_0019200, PWM_BAUDRATE_0038400, PWM_BAUDRATE_0056000, PWM_BAUDRATE_0057600,
    PWM_BAUDRATE_0115200, PWM_BAUDRATE_0128000, PWM_BAUDRATE_0230400, PWM_BAUDRATE_0256000,
    PWM_BAUDRATE_0460800, PWM_BAUDRATE_0500000, PWM_BAUDRATE_0512000, PWM_BAUDRATE_0600000,
    PWM_BAUDRATE_0750000, PWM_BAUDRATE_0921600, PWM_BAUDRATE_1000000, PWM_BAUDRATE_1280000,
    PWM_BAUDRATE_1500000, PWM_BAUDRATE_2000000, PWM_BAUDRATE_2500000, PWM_BAUDRATE_3000000,
    PWM_BAUDRATE_3500000
};

/**
 * 1 : Within the range of backlash
 * 0 : Out of tolerance range
*/
#define ALG_CALC_DIFF(_V_D_, _RANGE_)                ((_V_D_ > _RANGE_) ? 0U : 1U)
#define ALG_CALC_RANGE(_VAL_, _REF_, _RANGE_)        ((_VAL_ > _REF_) ? \
        ALG_CALC_DIFF((_VAL_ - _REF_), _RANGE_) : ALG_CALC_DIFF((_REF_ - _VAL_), _RANGE_))

/**
 * Calculate the absolute value
*/
#define ALG_CALC_ABSOLUTE(_VAL1_, _VAL2_)            ((_VAL1_ > _VAL2_) ? \
        (_VAL1_ - _VAL2_) : (_VAL2_ - _VAL1_))

/**
 * Calculate the standard value
*/
#define ALG_GET_STANDARD_VAL(_STA_, _ADDR_)          (((uint32_t)3 == _STA_) ? \
        ((*(_ADDR_ + 0U) < *(_ADDR_ + 2U)) ? *(_ADDR_ + 1U) : *(_ADDR_ + 3U)) :\
        ((0x02U == _STA_) ? (*(_ADDR_ + 3U)) : (*(_ADDR_ + 1U))))

/**
 * Calculate percentage value
*/
#define ALG_CALC_PERCENTAGE(_VAL_, _DOT_)            ((_VAL_ * _DOT_) / 100U)

#define ALG_BUBBLESORT_ARRAY(_ARR_, _SIZE_)             \
    do                                                  \
    {                                                   \
        uint32_t i = 0U;                                \
        uint32_t j = 0U;                                \
        for( i = 0U; i <(_SIZE_ - 1U); i++ ){           \
            for( j = 0U; j <(_SIZE_ - i - 1U); j++ ){   \
                if(_ARR_[j] > _ARR_[j + 1U]) {          \
                    uint32_t tmp = _ARR_[j];            \
                    _ARR_[j] = _ARR_[j + 1U];           \
                    _ARR_[j + 1U] = tmp;                \
                }                                       \
            }                                           \
        }                                               \
    } while (0)


static void drv_pwm_capture_init(uint32_t channel)
{
    PWM_REGS_ADDR_CAPCTL |= ((uint32_t)3U << (6U + (channel << 1)));
    PWM_REGS_ADDR_CAPINTEN |= ((uint32_t)1U << (6U + channel));

    PWM_REGS_ADDR_PWMCFG &= ~((channel << 1) | ((channel << 1) + 1U));
    PWM_REGS_ADDR_PWMCFG |= ((uint32_t)1U << (12U + channel));
}

int drv_calc_baud_original(uint32_t idx, uint32_t channel)
{
    volatile int ret = 0;
    volatile uint32_t i = 0U, j = 0U, k = 0U, m = 0U;
    volatile uint32_t capcnt[PWM_CONFIG_SAMPLING_TIMES] = {0U};
    volatile uint32_t buff_c[PWM_CONFIG_SAMPLING_TIMES - 1] = {0U};
    volatile uint32_t buff_e[PWM_CONFIG_SAMPLING_TIMES / 2] = {0U};
    volatile uint32_t result[PWM_CONFIG_COMPARE_TIMES] = {0U};
    volatile uint32_t timeout = 0U;
    volatile uint32_t temp = 0U;

    drv_pwm_capture_init(channel);

    do {
        do {
            if (PWM_REGS_ADDR_CAPIS & (((uint32_t)1U << (6U + channel)))) {
                PWM_REGS_ADDR_CAPIC |= (uint32_t)1U << (6U + channel);
                timeout = 0U;
                capcnt[i] = PWM_REGS_ADDR_CAPCOUNT(channel);
                i++;
            } else {
                if (timeout++ > 10000000U) {
                    ret = -1;
                    break;
                }
            }

        } while (i < PWM_CONFIG_SAMPLING_TIMES);

        if (0 == ret) {
            ///< The baud rate is calculated according to the count value
            for (i = 0U ; i < (PWM_CONFIG_SAMPLING_TIMES - 1U); i++) {
                buff_c[i] = PMW_CAL_LOOP_VALUE(capcnt[i + 1U], capcnt[i]);
            }

            for (i = 2U; i < (PWM_CONFIG_SAMPLING_TIMES - 2U); i++) {
                temp = ALG_CALC_PERCENTAGE(buff_c[i + 1U], PWM_CONFIG_PERCENTAGE);

                ///< check the first value is legal
                if (ALG_CALC_RANGE(buff_c[i], (buff_c[i + 1U] << 2), temp)) {
                    k = i;
                    break;
                }
            }

            temp = ALG_CALC_PERCENTAGE(buff_c[k], PWM_CONFIG_PERCENTAGE);

            for (i = k, m = 0U; i < (PWM_CONFIG_SAMPLING_TIMES - 1U); i++) {

                if (k != 0U) {
                    if (ALG_CALC_RANGE(buff_c[i], buff_c[k], temp)) {
                        buff_e[m++] = buff_c[i];
                    }

                    if (0U == buff_c[i]) {
                        if (buff_c[i - 1U] > buff_c[k]) {
                            buff_e[m++] = (buff_c[i - 1U] / 10U) << 3;
                        }
                    }
                } else {
                    if (0U == buff_c[i]) {
                        buff_e[m++] = (buff_c[i - 1U] / 10U) << 3;
                    }
                }
            }

            temp = 0U;

            if (m > 2U) {
                ALG_BUBBLESORT_ARRAY(buff_e, (m / 2U));

                for (i = 1U; i < (m - 1U); i++) {
                    temp += buff_e[i];
                }

                temp /= (m - 2U);
            } else if (m > 0U) {
                for (i = 0U; i < m; i++) {
                    temp += buff_e[i];
                }

                temp /= m;
            } else {
                ret = -1;
            }

            if (temp) {
                ///< The active level is the low level with eight bits
                result[j] = (soc_get_pwm_freq(0U) << 3) / temp;
            }

            j++;
            i = 0U;
        } else {
            break;
        }
    } while (j < PWM_CONFIG_COMPARE_TIMES);

    if (0 == ret) {
        temp = 0U;

        if (PWM_CONFIG_COMPARE_TIMES > 2U) {
            ///< Delete max and min
            ALG_BUBBLESORT_ARRAY(result, (PWM_CONFIG_COMPARE_TIMES / 2U));

            for (i = 1U ; i < ((PWM_CONFIG_SAMPLING_TIMES / 2U) - 1U); i++) {
                temp += result[i];
            }

            temp /= ((PWM_CONFIG_SAMPLING_TIMES / 2U) - 2U);
        } else {
            ///< Take the average
            for (i = 0U ; i < PWM_CONFIG_COMPARE_TIMES ; i++) {
                temp += result[i];
            }

            temp /= PWM_CONFIG_COMPARE_TIMES;
        }

        ret = (int)temp;
    }

    return ret;
}

int drv_calc_baud_adjust(uint32_t idx, uint32_t channel)
{
    int ret = -1;
    int temp = drv_calc_baud_original(idx, channel);
    uint32_t i = 0U;
    uint32_t sta = 0U;
    uint32_t diff[2];
    uint32_t value[2][2] = {0U};

    if (temp > 0) {
        for (i = 0U; i < (sizeof(baud_table) / sizeof(uint32_t)); i++) {
            if (temp < (int)baud_table[i]) {
                break;
            }
        }

        if (i < 29U) {
            diff[0] = ALG_CALC_PERCENTAGE(baud_table[i], PWM_CONFIG_PERCENTAGE);

            if (ALG_CALC_RANGE((uint32_t)temp, baud_table[i], diff[0])) {
                value[0][0] = ALG_CALC_ABSOLUTE((uint32_t)temp, baud_table[i]);
                value[0][1] = baud_table[i];
                sta |= 0x01U;
            }
        }

        if (i > 0U) {
            diff[1] = ALG_CALC_PERCENTAGE(baud_table[i - 1U], PWM_CONFIG_PERCENTAGE);

            if (ALG_CALC_RANGE((uint32_t)temp, baud_table[i - 1U], diff[1])) {
                value[1][0] = ALG_CALC_ABSOLUTE((uint32_t)temp, baud_table[i - 1U]);
                value[1][1] = baud_table[i - 1U];
                sta |= 0x02U;
            }
        }

        if (sta & 0x03U) {
            ret = ALG_GET_STANDARD_VAL(sta, (uint32_t *)value);
        }
    }

    return ret;
}
