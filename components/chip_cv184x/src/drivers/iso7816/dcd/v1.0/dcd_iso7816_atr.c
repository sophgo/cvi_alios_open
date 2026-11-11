/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     dcd_iso7816_atr.c
 * @brief    CSI Source File for ISO7816 Driver
 * @version  V1.0
 * @date     02. Nov 2019
 ******************************************************************************/

#include "dcd_iso7816.h"
#include "stdio.h"

#define DBG_INFO(fmt, args...) //do {printf(fmt, ##args);}while(0)

static chars_t t_s(atr_analysis_t *atr, uint8_t x, uint32_t mask, chars_t sta)
{
    while(mask <= 0x80) {
        if (mask & atr->sections) {
            return sta;
        }
        mask *= 2;
        sta++;
    }

    if (atr->histbytes) {
        return TK_S;
    }

    if (atr->t1) {
        return TCK_S;
    }

    return  TEND_S;
}

static chars_t t0_s(atr_analysis_t *atr, uint8_t x)
{
    atr->sections = x & 0xF0;
    atr->histbytes = x & 0xF;
    atr->t1=(atr->sections  & 0x80)?1:0; //When TDi is not transmitted, T=0 is used.

    return t_s(atr, x, 0x10, TA1_S);
}

static chars_t ta1_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, x, 0x20, TB1_S);
}

static chars_t tb1_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, x, 0x40, TC1_S);
}

static chars_t tc1_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, x, 0x80, TD1_S);
}

static chars_t td1_s(atr_analysis_t *atr, uint8_t x)
{
    atr->sections = x & 0xf0;
    return t_s(atr, x, 0x10, TA2_S);
}

static chars_t ta2_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, x, 0x20, TB2_S);
}

static chars_t tb2_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, x, 0x40, TC2_S);
}

static chars_t tc2_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, x, 0x80, TD2_S);
}

static chars_t td2_s(atr_analysis_t *atr, uint8_t x)
{
    atr->sections = x & 0xf0;
    return t_s(atr, x, 0x10, TA3_S);
}

static chars_t ta3_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, 0x33, 0x20, TB3_S);
}

static chars_t tb3_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, x, 0x40, TC3_S);
}

static chars_t tc3_s(atr_analysis_t *atr, uint8_t x)
{
    return t_s(atr, x, 0x80, TD3_S);
}

static chars_t td3_s(atr_analysis_t *atr, uint8_t x)
{
    if (atr->histbytes) {
        return TK_S;
    }

    if (atr->t1) {
        return TCK_S;
    }

    return TEND_S;
}

static chars_t tk_s(atr_analysis_t *atr, uint8_t x)
{
    if (atr->histbytes) {
        atr->histbytes--;
    } else {
        if (atr->t1) {
            return TCK_S;
        }
        return TEND_S;
    }

    if (atr->histbytes > 0) {
        return TK_S;
    } else {
        if (atr->t1) {
            return TCK_S;
        }
    }

    return TEND_S;
}

static chars_t tck_s(atr_analysis_t *atr, uint8_t x)
{
    return TEND_S;
}

typedef chars_t (*atr_sta_change_func)(atr_analysis_t *atr, uint8_t x);

static atr_sta_change_func sta_change[] ={t0_s, ta1_s, tb1_s, tc1_s, td1_s, ta2_s, tb2_s, tc2_s,
                                      td2_s, ta3_s, tb3_s, tc3_s, td3_s, tk_s, tck_s};

chars_t atr_next_state(dcd_iso7816_priv_t *priv, uint8_t x)
{
    atr_analysis_t *atr = &priv->atr;
    if (atr->atr_sta >= T0_S && atr->atr_sta <= TCK_S) {
        return sta_change[atr->atr_sta](atr, x);
    }

    return atr->atr_sta;
}
#define CONFIG_ISO7816_NUM 1
static atr_result_t atr_result[CONFIG_ISO7816_NUM];

typedef struct {
    int32_t fi;
    int32_t f_max;
} atr_fi_t;

static void ta1_s_parse(atr_result_t *result, uint8_t data)
{
    atr_fi_t atr_fi[] = {{0, 4000000}, {1, 5000000}, {2, 6000000}, {3, 8000000},
                         {4, 12000000}, {5, 16000000}, {6, 20000000}, {9, 5000000},
                         {10, 7500000}, {11, 10000000}, {12, 15000000}, {13, 20000000},};

    uint8_t f =(data & 0xf0) >> 4;
    uint8_t d = data & 0x0f;

    if (f <= 6) {
        ;
    } else if (f >= 9 && f <= 13) {
        f -= 2;
    } else {
        f = 0xff;
    }

    if (f <= 11) {
        result->fi = atr_fi[f].fi;
        result->f_max = atr_fi[f].f_max;
        DBG_INFO("F=%un\t\tFmax=%uMHz\n", result->fi, result->f_max / 1000000);
    } else {
        result->fi = 0;
        result->f_max = 5000000;
        DBG_INFO("Fi reserved for future use\n");
    }

    if (d > 0 && d <= 9) {
        result->di = d;
        DBG_INFO("\t\tD=%u\n", result->di);
    } else {
        result->di = 1;
        DBG_INFO("\t\tDi reserved for future use\n");
    }

    DBG_INFO("---------------------------------------------------\n");
}

static void ta2_s_parse(atr_result_t *result, uint8_t data)
{
     DBG_INFO("TA2\t%02X\t\n", data);

    if (data & (1 << 8)) {
        result->protocol_negotiation_en = 0;
        DBG_INFO("TA2\t8bit=1 can't negotiation\t\n");
    } else {
        DBG_INFO("TA2\t8bit=0 can negotiation\t\n");
        result->protocol_negotiation_en = 1;
    }

    if (data & (1 << 5)) {
        result->di = 1;
        result->fi = 372;
        result->f_max = 5000000;
        DBG_INFO("TA2\t5bit=1 used Dd and Fd\t\n");
    } else {
        DBG_INFO("TA2\t5bit=0 used Di and Fi\t\n");
    }

    DBG_INFO("---------------------------------------------------\n");
}

void atr_parse(dcd_iso7816_priv_t *priv)
{
    uint8_t hist = 0;
    uint8_t index = 1;
    priv->atr.atr_sta = T0_S;
    uint8_t data;
    uint8_t *buf = priv->atr.buf;
    atr_result_t *result = &atr_result[priv->idx];
    DBG_INFO("---------------------------------------------------\n");
    while(priv->atr.atr_sta!=TEND_S) {
        data = *buf;
        buf++;
        switch(priv->atr.atr_sta) {
            case T0_S:
                hist = (data & 0xf);
                result->history_byte_num = hist;
                DBG_INFO("T0\t%02X\tTA1%spresent\n", data, ((data & 0x10) ? " ":" not "));
                DBG_INFO("\t\tTB1%spresent\n", (data & 0x20)?" " : " not ");
                DBG_INFO("\t\tTC1%spresent\n", (data & 0x40)?" " : " not ");
                DBG_INFO("\t\tTD1%spresent\n", (data & 0x80)?" " : " not ");
                DBG_INFO("\t\t%d historical characters\n", data & 0xF);
                DBG_INFO("---------------------------------------------------\n");
                break;
            case TA1_S:
                DBG_INFO("TA1\t%02X\t", data);
                ta1_s_parse(result, data);
                break;
            case TB1_S:
                break;
            case TC1_S:
                result->N = data;
                if (data == 0xFF) {
                    DBG_INFO("TC1\t%02X\tMinimum delay between start edges\n", data);
                } else if (!data) {
                    DBG_INFO("TC1\t%02X\tNo extra guard time\n", data);
                } else {
                  DBG_INFO("TC1\t%02X\t\%02X number of extra guard time\n", data, data);
                }
                DBG_INFO("---------------------------------------------------\n");
                break;
            case TD1_S:
                result->T = data & 0xf;
                DBG_INFO("TD1\t%02X\tTA2%spresent\n", data, ((data & 0x10) ? " " : " not "));
                DBG_INFO("\t\tTB2%spresent\n", (data & 0x20) ? " " : " not ");
                DBG_INFO("\t\tTC2%spresent\n", (data & 0x40) ? " " : " not ");
                DBG_INFO("\t\tTD2%spresent\n", (data & 0x80) ? " " : " not ");
                DBG_INFO("\t\tT=%d is used\n", (data & 0xf));
                DBG_INFO("---------------------------------------------------\n");
                break;
            case TA2_S:
                ta2_s_parse(result, data);
                break;
            case TB2_S:
                ;
                break;
            case TC2_S:
                result->wi = data;
                DBG_INFO("TC2\t%02X\t\n", data);
                DBG_INFO("---------------------------------------------------\n");;
                break;
            case TD2_S:
                result->TD2_T = data & 0xf;
                DBG_INFO("TD2\t%02X\tTA3%spresent\n", data, ((data & 0x10) ? " " : " not "));
                DBG_INFO("\t\tTB3%spresent\n", (data & 0x20) ? " " : " not ");
                DBG_INFO("\t\tTC3%spresent\n", (data & 0x40) ? " " : " not ");
                DBG_INFO("\t\tTD3%spresent\n", (data & 0x80) ? " " : " not ");
                DBG_INFO("\t\tT=%d is used\n", (data & 0xf));
                DBG_INFO("---------------------------------------------------\n");
                break;
            case TA3_S:
                if (result->TD2_T == 0xf) {
                    result->clk_stop_type = (data & 0xc0) >> 7;
                    result->support_voltage_class = data & 0x0f;
                }
                DBG_INFO("\t\tis %d bytes\n", data);
                DBG_INFO("---------------------------------------------------\n");
                break;
            case TB3_S:
                result->cwi = data & 0xf;
                result->bwi = (data & 0xf0 >> 4);
                DBG_INFO("TB3\t%02X\t%d characters waiting time\n", data, data & 0xF);
                DBG_INFO("\t\t%d block waiting time\n", data >> 4);
                DBG_INFO("---------------------------------------------------\n");
                break;
            case TC3_S:
                result->t1_check = data & 1;
                DBG_INFO("TC3\t%02X\t\n", data);
                DBG_INFO("---------------------------------------------------\n");
                break;
            case TD3_S:
                DBG_INFO("\t\twarring\n");
                DBG_INFO("---------------------------------------------------\n");
                break;
            case TK_S:
                result->history_data[index - 1] = data;
                if (index != hist) {
                    DBG_INFO("T%d\t%02X\n", index, data);
                } else {
                    DBG_INFO("T%d\t%02X\n", index, data);
                    DBG_INFO("---------------------------------------------------\n");
                }
                index++;
                break;
            case TCK_S:
                DBG_INFO("TCK\t%02X\tCheck character\n", data);
                DBG_INFO("---------------------------------------------------\n");
                break;
            default:
                DBG_INFO("data 0x%x", data);
                DBG_INFO("error\n");
                break;
        }
        priv->atr.atr_sta = atr_next_state(priv, data);
    };
}

atr_result_t * atr_get_result(int idx)
{
    return &atr_result[idx];
}
