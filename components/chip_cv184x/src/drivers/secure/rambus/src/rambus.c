
/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

#include "rambus.h"
#include "drv/rng.h"
#include "drv/tick.h"
#include "csi_core.h"

uint32_t rb_get_random_byte(uint8_t *buf, uint32_t count)
{
        uint32_t ret = CSI_OK;
#ifndef CONFIG_RB_RAND_DEBUG
        uint32_t wc = count / sizeof(uint32_t);
        uint32_t bc = count % sizeof(uint32_t);
        uint8_t *p  = buf;
        uint32_t t;
        uint8_t  tc = 0;
        int      i  = 0;

        CHECK_RET_WITH_RET(buf != NULL, CSI_ERROR);

        if (wc > 0) {
                ret = csi_rng_get_multi_word((uint32_t *)p, wc);
                CHECK_RET_WITH_RET(ret == CSI_OK, CSI_ERROR);
                p += (wc * sizeof(uint32_t));
                for (i = 0; i < wc * sizeof(uint32_t); i++) {
                        if (buf[i] != 0) {
                                tc = buf[i];
                                break;
                        }
                }
        }

        if (bc > 0) {
                ret = csi_rng_get_single_word(&t);
                CHECK_RET_WITH_RET(ret == CSI_OK, CSI_ERROR);
                memcpy(p, (uint8_t *)&t, bc);
                p = (uint8_t *)&t;
                if (tc == 0) {
                        for (i = 0; i < sizeof(uint32_t); i++) {
                                if (p[i] != 0) {
                                        tc = p[i];
                                        break;
                                }
                        }
                }
        }

        //check zero and fix
        if (tc == 0) {
                TRACE_ERR(CSI_ERROR);
                return CSI_ERROR;
        }
        p = buf;
        for (int i = 0; i < count; i++) {
                if (*p == 0) {
                        TRACE_N(tc);
                        *p = tc;
                }
                p++;
        }
#else
        memset(buf, 0xff, count);
#endif

        return ret;
}

#ifdef CONFIG_ALG_PERF_TEST
uint64_t g_freq_timer       = 3 * 1000 * 1000;
uint64_t g_freq_ip          = 50 * 1000 * 1000;
uint64_t g_start_ctr        = 0;
uint64_t g_end_ctr          = 0;
uint64_t g_data_len_in_bits = 0;
/* 1 bpc, 2 tps, 3 bps*/
uint32_t g_type = 0;
#endif

void rb_perf_init(uint32_t data_len_in_bits, uint32_t type)
{
#ifdef CONFIG_ALG_PERF_TEST
        if (data_len_in_bits != 0) {
                g_data_len_in_bits = data_len_in_bits;
        }
        if (type != 0) {
                g_type = type;
        }
        //gpio init
        Device_Write32((void *)0xffe7008004, 0, 0x180);
        // //low
        // Device_Write32((void *)0xffe7008000, 0, 0x80);

#endif
}

void rb_perf_start()
{
#ifdef CONFIG_ALG_PERF_TEST
        g_start_ctr = ((((uint64_t)csi_coret_get_valueh() << 32U) |
                        csi_coret_get_value()));
        //high
        // TRACE_ERR(11);
        Device_Write32((void *)0xffe7008000, 0, 0x180);

#endif
}

void rb_perf_end()
{
#ifdef CONFIG_ALG_PERF_TEST
        g_end_ctr = ((((uint64_t)csi_coret_get_valueh() << 32U) |
                      csi_coret_get_value()));
        //low
        Device_Write32((void *)0xffe7008000, 0, 0x80);
        // TRACE_ERR(12);
#endif
}

#ifdef CONFIG_ALG_PERF_TEST
static uint64_t s_get_ip_cycle()
{
        return (g_end_ctr - g_start_ctr) * g_freq_ip / g_freq_timer;
}

static uint64_t s_get_us()
{
        return (g_end_ctr - g_start_ctr) * 1000 * 1000 / g_freq_timer;
}
#endif

/* 0 if NONE */
void rb_perf_get(char *ncase)
{
#ifdef CONFIG_ALG_PERF_TEST
        uint64_t res = 0;

        if (g_type == 1) {
                res = g_data_len_in_bits * 1000 / s_get_ip_cycle();
                LOG_CRIT("%s, %d: start:%lu, end: %lu, bits: %lu\n",
                         __FUNCTION__, __LINE__, g_start_ctr, g_end_ctr,
                         g_data_len_in_bits);

                LOG_CRIT("%s, %d: bits: %lu, us: %lu\n", __FUNCTION__, __LINE__,
                         g_data_len_in_bits, s_get_us());
                LOG_CRIT("===PERF %s bits per kcycle:%lu\n", ncase, res);
                return;
        } else if (g_type == 2) {
                res = g_freq_timer * 1000 / ((g_end_ctr - g_start_ctr));
                LOG_CRIT("%s, %d: start:%lu, end: %lu, bits: %lu\n",
                         __FUNCTION__, __LINE__, g_start_ctr, g_end_ctr,
                         g_data_len_in_bits);

                LOG_CRIT("%s, %d: bits: %lu, us: %lu\n", __FUNCTION__, __LINE__,
                         g_data_len_in_bits, s_get_us());
                LOG_CRIT("===PERF:%s  times per ksecond:%lu\n", ncase, res);
                return;
        } else if (g_type == 3) { //bps
                res = g_data_len_in_bits * g_freq_timer /
                      ((g_end_ctr - g_start_ctr));
                LOG_CRIT("%s, %d: start:%lu, end: %lu, bits: %lu\n",
                         __FUNCTION__, __LINE__, g_start_ctr, g_end_ctr,
                         g_data_len_in_bits);

                LOG_CRIT("%s, %d: bits: %lu, us: %lu\n", __FUNCTION__, __LINE__,
                         g_data_len_in_bits, s_get_us());
                LOG_CRIT("===PERF:%s  bits per second:%lu\n", ncase, res);
                return;
        }

        TRACE_ERR(-1);
#endif
}
