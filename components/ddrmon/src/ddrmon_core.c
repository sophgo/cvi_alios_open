// SPDX-License-Identifier: Apache-2.0

#include <aos/kernel.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ddrmon_api.h"
#include "ddrmon_regs.h"
#include "ddrmon_stats.h"

#define DDRMON_PORTS 6
#define DDRMON_PERIOD_MIN_MS  20
#define DDRMON_PERIOD_MAX_MS  10000   // 10s 上限，避免线程永久阻塞

static struct {
	uintptr_t base;
	uintptr_t ddr_ctrl;
	uintptr_t ddr_top;
	const char *soc_type;
	uint32_t period_ms;
	uint32_t port_mask;
	uint32_t data_rate_mhz; // temp until HW register known
	uint32_t bus_width_bits; // DDR bus width, probed or default
	aos_task_t th;
	aos_mutex_t lock;
	int running;
	int thread_exited; // flag to indicate thread has fully exited
	int initialized; // flag to prevent double-init
	uint64_t start_time_ms; // timestamp when start() is called
	uint32_t log_interval_ms;  // auto-log interval (0=disabled)
	uint64_t last_log_time_ms; // timestamp of last log output
	// last sampled values per port
	double last_rd[DDRMON_PORTS];
	double last_wr[DDRMON_PORTS];
	double last_tot[DDRMON_PORTS];
} g_ctx;

/* MMIO accessors: volatile is required for hardware register access */
static inline uint32_t mmio_read32(uintptr_t addr)
{
	return *(volatile uint32_t *)addr; /* intentional: MMIO */
}
static inline void mmio_write32(uintptr_t addr, uint32_t val)
{
	*(volatile uint32_t *)addr = val; /* intentional: MMIO */
}

// DDR data rate auto-probe support (aligned with Linux HAL implementation)
// Reference: linux_5.10/drivers/soc/cvitek/mon/hal/cv181x/mon_platform.c
#define CLK_DRAMRATE_REG_MAGIC   (0x03002900u + 0x54u)
#define CLK_DIVIDEND_MAGIC       0x1770000000ULL

// Probe DRAM data rate from hardware register (SoC-specific: cv181x requires divide-by-2)
static uint32_t ddrmon_probe_dram_rate(void)
{
	// Read raw clock divisor register
	uint32_t reg = mmio_read32((uintptr_t)CLK_DRAMRATE_REG_MAGIC);

	if (reg == 0 || reg == 0xFFFFFFFFu) {
		// Treat invalid/empty value as probe failure
		return 0;
	}

	// Compute rate according to Linux logic: (CLK_DIVIDEND_MAGIC / reg) << 4
	uint64_t dividend = CLK_DIVIDEND_MAGIC;
	uint64_t q = dividend / (uint64_t)reg;
	uint32_t rate = (uint32_t)q;

	rate <<= 4;

	// SoC-specific adjustment: BOTH cv180x and cv181x require divide-by-2
	// Note: This matches the actual Linux driver behavior (both hal/ directories have >> 1)
	// The comment "cv181x specific" in Linux driver is a copy-paste error
	if (g_ctx.soc_type && (!strncmp(g_ctx.soc_type, "cv180", 5) || !strncmp(g_ctx.soc_type, "cv181", 5))) {
		rate >>= 1;
	}

	if (rate == 0) {
		return 0;
	}
	return rate;
}

// Probe DDR bus width: prefer config macro, fall back to 32
static uint32_t ddrmon_probe_bus_width(void)
{
	(void)(g_ctx.ddr_ctrl);
#ifdef CVI_DDR_BUS_WIDTH_BITS
	return (uint32_t)CVI_DDR_BUS_WIDTH_BITS;
#else
	return 32;
#endif
}
static inline void port_op(uintptr_t base, uint32_t v1, uint32_t v2)
{
	mmio_write32(base + AXIMON_OFFSET_CONTROL, v1);
	mmio_write32(base + AXIMON_OFFSET_CONTROL, v2);
}

static void snapshot_all(void)
{
	for (int i = 0; i < DDRMON_PORTS; ++i) {
		port_op(g_ctx.base + port_write_base(i), AXIMON_SNAPSHOT_REGVALUE_1, AXIMON_SNAPSHOT_REGVALUE_2);
		port_op(g_ctx.base + port_read_base(i), AXIMON_SNAPSHOT_REGVALUE_1, AXIMON_SNAPSHOT_REGVALUE_2);
	}
}

static void clear_all_counters(void)
{
	for (int i = 0; i < DDRMON_PORTS; ++i) {
		port_op(g_ctx.base + port_write_base(i), 0x00000002, 0x00000000);
		port_op(g_ctx.base + port_read_base(i), 0x00000002, 0x00000000);
	}
}

static void axi_mon_reset_all(void)
{
	for (int i = 0; i < DDRMON_PORTS; ++i) {
		uintptr_t wr_base = g_ctx.base + port_write_base(i);
		uintptr_t rd_base = g_ctx.base + port_read_base(i);

		mmio_write32(wr_base, AXIMON_SELECT_CLK);
		mmio_write32(rd_base, AXIMON_SELECT_CLK);
		mmio_write32(wr_base + AXIMON_OFFSET_ID, 0x0);
		mmio_write32(rd_base + AXIMON_OFFSET_ID, 0x0);
	}
}

static void select_clk_all(void)
{
	/* Enable AXIMON clock gating */
	uintptr_t ddr_top_cg = g_ctx.ddr_top + 0x14;
	uint32_t val = mmio_read32(ddr_top_cg);

	val |= (1u << 8);
	mmio_write32(ddr_top_cg, val);
	axi_mon_reset_all();
}


/* Sample bandwidth: snapshot -> read -> calculate */
static void sample_once(void)
{
	// Snapshot all counters (freezes for read, then auto-resets to 0)
	snapshot_all();

	// Read cycle counter from first enabled port's write block
	uint32_t tick = 0;
	for (int i = 0; i < DDRMON_PORTS; ++i) {
		tick = mmio_read32(g_ctx.base + port_write_base(i) + AXIMON_OFFSET_CYCLE);
		break; // Only need one cycle counter
	}

	// Read current byte counters
	uint32_t wr_bytes[DDRMON_PORTS] = {0};
	uint32_t rd_bytes[DDRMON_PORTS] = {0};

	for (int i = 0; i < DDRMON_PORTS; ++i) {
		wr_bytes[i] = mmio_read32(g_ctx.base + port_write_base(i) + AXIMON_OFFSET_BYTECNTS);
		rd_bytes[i] = mmio_read32(g_ctx.base + port_read_base(i) + AXIMON_OFFSET_BYTECNTS);
	}

	// Use absolute tick value (snapshot auto-resets counters after each read)
	uint32_t data_rate = g_ctx.data_rate_mhz;
	if (!data_rate)
		data_rate = 1000; // fallback 1GHz data rate

	// Compute duration_us with robust fallbacks
	// Linux formula: duration_us = snapshot_tick / (data_rate >> 2)
	double duration_us = 0.0;
	uint32_t denom = (data_rate >> 2);
	double expected_dur_us = (double)g_ctx.period_ms * 1000.0;

	if (tick == 0 || denom == 0) {
		duration_us = expected_dur_us;
	} else {
		duration_us = (double)tick / (double)denom;
		// Guard A (duration sanity): reject samples where tick-derived duration
		// deviates more than ±50% from the expected sample period.
		// A too-small tick means the snapshot didn't latch properly;
		// a too-large tick means counters may have wrapped or accumulated.
		// In either case, skip this sample entirely to avoid polluting min/max/avg.
		if (duration_us < expected_dur_us * 0.5 ||
		    duration_us > expected_dur_us * 1.5) {
			return;
		}
	}
	uint64_t ts_ms = (uint64_t)aos_now_ms();

	// Guard B (BW sanity): compute theoretical maximum bandwidth limit.
	// DDR max BW (MB/s) = data_rate_mhz * bus_width_bits / 8.
	// Use 3x margin to account for burst traffic while rejecting obvious glitches.
	double bw_limit = (double)(g_ctx.data_rate_mhz * g_ctx.bus_width_bits) / 8.0 * 3.0;

	// Compute new snapshot values locally to avoid exposing partial updates
	double wr_new[DDRMON_PORTS] = {0};
	double rd_new[DDRMON_PORTS] = {0};
	double tot_new[DDRMON_PORTS] = {0};

	for (int i = 0; i < DDRMON_PORTS; ++i) {
		// Use absolute values: snapshot auto-resets counters to 0 after latching
		// So wr_bytes[i] and rd_bytes[i] are the bytes transferred since last snapshot
		// Calculate bandwidth: bytes/us = MB/s
		double wr = (double)wr_bytes[i] / duration_us;
		double rd = (double)rd_bytes[i] / duration_us;

		// Guard B: if any port's BW exceeds the theoretical limit,
		// the entire sample is unreliable — discard it.
		if (wr > bw_limit || rd > bw_limit)
			return;

		wr_new[i] = wr;
		rd_new[i] = rd;
		tot_new[i] = wr + rd;
	}

	// Aggregate total across enabled ports from the fresh snapshot
	double sum_rd = 0.0, sum_wr = 0.0, sum_tot = 0.0;

	for (int j = 0; j < DDRMON_PORTS; ++j) {
		sum_rd += rd_new[j];
		sum_wr += wr_new[j];
		sum_tot += tot_new[j];
	}

	// Commit atomically under mutex
	aos_mutex_lock(&g_ctx.lock, AOS_WAIT_FOREVER);
	for (int i = 0; i < DDRMON_PORTS; ++i) {
		g_ctx.last_wr[i] = wr_new[i];
		g_ctx.last_rd[i] = rd_new[i];
		g_ctx.last_tot[i] = tot_new[i];
	}
	aos_mutex_unlock(&g_ctx.lock);

	// Push bandwidth sample with read/write breakdown to statistics ring
	ddrmon_stats_push(ts_ms, sum_rd, sum_wr, sum_tot);
}

// Output auto-log to console (called periodically)
static void ddrmon_output_log(void)
{
	uint64_t now = (uint64_t)aos_now_ms();
	uint64_t total_runtime_ms = now - g_ctx.start_time_ms;
	uint64_t window_ms = now - g_ctx.last_log_time_ms;  // Time since last log

	// Check retention window
	uint32_t ring_capacity = 0;

	ddrmon_get_ring_status(NULL, &ring_capacity);
	uint64_t max_window_ms = ((uint64_t)ring_capacity) * g_ctx.period_ms;

	// Query stats for this log interval only
	struct ddrmon_stats_rw_query q;

	memset(&q, 0, sizeof(q));
	q.last_ms = window_ms;
	int ret = ddrmon_stats_query_rw(&q);
	if (ret == 0) {
		uint32_t min_part = (uint32_t)(total_runtime_ms / 60000);
		uint32_t sec_part = (uint32_t)((total_runtime_ms % 60000) / 1000);

		printf("[ddrmon autolog] %02u:%02u | Tot:%u | R:%u | W:%u MB/s | Samples:%u",
			   min_part, sec_part,
			   (uint32_t)(q.avg_tot + 0.5),
			   (uint32_t)(q.avg_rd + 0.5),
			   (uint32_t)(q.avg_wr + 0.5),
			   q.sample_count);

		// Warn if log interval exceeds ring buffer capacity
		if (window_ms > max_window_ms) {
			printf(" [WARN: log interval %u.%03us > max retention %u.%03us]",
			   (uint32_t)(window_ms / 1000),
			   (uint32_t)((window_ms % 1000)),
			   (uint32_t)(max_window_ms / 1000),
			   (uint32_t)((max_window_ms % 1000)));
		}
		printf("\n");
	} else {
		printf("[ddrmon autolog] No data available (ret=%d)\n", ret);
	}

	// Update last log time for next interval calculation
	g_ctx.last_log_time_ms = now;
}

static void ddrmon_thread(void *param)
{
	(void)(param);
	g_ctx.thread_exited = 0;

	while (g_ctx.running) {
		sample_once();

		if (g_ctx.log_interval_ms > 0) {
			uint64_t now = (uint64_t)aos_now_ms();
			if (now - g_ctx.last_log_time_ms >= g_ctx.log_interval_ms)
				ddrmon_output_log();
		}

		aos_msleep(g_ctx.period_ms);
	}

	g_ctx.thread_exited = 1;
	aos_task_exit(0);
}

int ddrmon_init(const ddrmon_params_t *p)
{
	// Idempotent: avoid double-init resource leak
	if (g_ctx.initialized) {
		return DDRMON_OK;
	}

	memset(&g_ctx, 0, sizeof(g_ctx));
	g_ctx.base = p->aximon_base;
	g_ctx.ddr_ctrl = p->ddr_ctrl_base;
	g_ctx.ddr_top = p->ddr_top_base;
	g_ctx.soc_type = p->soc_type;
	if (p->default_period_ms > DDRMON_PERIOD_MAX_MS) {
		printf("[ddrmon] invalid default_period_ms %u, max %u\n",
			   p->default_period_ms, DDRMON_PERIOD_MAX_MS);
		return DDRMON_ERR;
	}
	g_ctx.period_ms = p->default_period_ms ? p->default_period_ms : DDRMON_PERIOD_MIN_MS;
	g_ctx.port_mask = p->port_mask ? p->port_mask : 0x3F;
	g_ctx.data_rate_mhz = p->data_rate_mhz;

	// Auto-probe DRAM data rate; fall back to Kconfig-provided value on failure
	uint32_t probed = ddrmon_probe_dram_rate();

	if (probed) {
		g_ctx.data_rate_mhz = probed;
	}

	// Probe DDR bus width; default to 32 bits if not available
	g_ctx.bus_width_bits = ddrmon_probe_bus_width();
	if (g_ctx.bus_width_bits == 0)
		g_ctx.bus_width_bits = 32;

	// Create core mutex BEFORE stats_init (so failure path is clean)
	if (aos_mutex_new(&g_ctx.lock) != 0) {
		printf("[ddrmon] failed to create core mutex\n");
		return DDRMON_ERR;
	}

	// Enable AXIMON clock gating first
	select_clk_all();

	ddrmon_stats_init();
	g_ctx.initialized = 1;

	printf("[ddrmon] Initialization complete\n");
	return DDRMON_OK;
}

int ddrmon_start(void)
{
	if (!g_ctx.initialized)
		return DDRMON_ERR;
	if (g_ctx.running)
		return DDRMON_EBUSY; // Already running, must stop first

	// Reset statistics ring buffer to avoid stale data pollution from previous runs
	ddrmon_stats_reset();

	// Ensure counters start from zero for the first sampling window
	clear_all_counters();

	// Perform initial snapshot to clear snapshot registers
	// This prevents the first sample from reading stale values from previous run
	snapshot_all();

	g_ctx.running = 1;
	g_ctx.thread_exited = 0; // Will be set to 1 by thread before exit
	g_ctx.start_time_ms = (uint64_t)aos_now_ms(); // Record start time
	g_ctx.last_log_time_ms = g_ctx.start_time_ms;     // Initialize log timestamp

	int ret = aos_task_new_ext(&g_ctx.th, "ddrmon", ddrmon_thread, NULL,
				   4096, AOS_DEFAULT_APP_PRI);
	if (ret != 0) {
		g_ctx.running = 0;
		printf("[ddrmon] failed to create thread: %d\n", ret);
		return DDRMON_ERR;
	}

	return DDRMON_OK;
}

int ddrmon_stop(void)
{
	if (!g_ctx.initialized)
		return DDRMON_ERR;
	if (!g_ctx.running)
		return DDRMON_EBUSY; // Not running, must start first

	g_ctx.running = 0;

	// Wait for thread to exit with short polling to avoid long blocking
	// Maximum wait time: 2 periods, but poll every 5ms for responsiveness
	uint32_t max_wait_ms = (g_ctx.period_ms + 5) * 2;
	uint32_t poll_interval_ms = 5;
	uint32_t elapsed_ms = 0;

	while (!g_ctx.thread_exited && elapsed_ms < max_wait_ms) {
		aos_msleep(poll_interval_ms);
		elapsed_ms += poll_interval_ms;
	}

	// Clean up thread handle
	if (g_ctx.thread_exited) {
		aos_task_delete(&g_ctx.th);
	}

	return DDRMON_OK;
}

int ddrmon_set_period(uint32_t ms)
{
	DDRMON_DBG("[ddrmon] set_period %u\n", ms);
	if (!g_ctx.initialized)
		return DDRMON_ERR;
	if (ms > DDRMON_PERIOD_MAX_MS)
		return DDRMON_EINVAL;
	g_ctx.period_ms = ms ? ms : DDRMON_PERIOD_MIN_MS;
	return DDRMON_OK;
}

uint32_t ddrmon_get_period_ms(void)
{
	return g_ctx.period_ms;
}

int ddrmon_get_instant(uint32_t port, double *rd_MBps, double *wr_MBps, double *tot_MBps)
{
	// Check initialization state and running state
	if (!g_ctx.initialized || !aos_mutex_is_valid(&g_ctx.lock))
		return DDRMON_ERR;
	if (!g_ctx.running)
		sample_once();

	// Support per-port read, and special value 0xFF/0xFFFFFFFF for aggregated bandwidth
	int ret = DDRMON_OK;

	aos_mutex_lock(&g_ctx.lock, AOS_WAIT_FOREVER);

	int aggregate = (port == 0xFFFFFFFFu || port == 0xFFu);

	if (aggregate) {
		double sum_rd = 0.0, sum_wr = 0.0, sum_tot = 0.0;

		for (int i = 0; i < DDRMON_PORTS; ++i) {
			if (g_ctx.port_mask & (1u << i)) {
				sum_rd += g_ctx.last_rd[i];
				sum_wr += g_ctx.last_wr[i];
				sum_tot += g_ctx.last_tot[i];
			}
		}
		if (rd_MBps)
			*rd_MBps = sum_rd;
		if (wr_MBps)
			*wr_MBps = sum_wr;
		if (tot_MBps)
			*tot_MBps = sum_tot;
	} else {
		if (port >= DDRMON_PORTS) {
			ret = -1;
			goto out;
		}
		if (!(g_ctx.port_mask & (1u << port))) {
			ret = -2;
			goto out;
		}
		if (rd_MBps)
			*rd_MBps = g_ctx.last_rd[port];
		if (wr_MBps)
			*wr_MBps = g_ctx.last_wr[port];
		if (tot_MBps)
			*tot_MBps = g_ctx.last_tot[port];
	}

out:
	aos_mutex_unlock(&g_ctx.lock);
	return ret;
}

int ddrmon_get_stats(uint32_t port, uint64_t last_ms,
					 double *min_MBps, double *max_MBps, double *avg_MBps)
{
	DDRMON_DBG("[ddrmon] get_stats port=%u last_ms=%u\n", port, (uint32_t)last_ms);
	if (!g_ctx.initialized)
		return DDRMON_ERR;
	if (!g_ctx.running)
		sample_once();
	(void)(port); // current ring tracks total across enabled ports
	return ddrmon_stats_query_ex(last_ms, min_MBps, max_MBps, avg_MBps, NULL);
}

int ddrmon_get_info(uint32_t *data_rate_mhz, uint32_t *bus_width_bits)
{
	if (!g_ctx.initialized)
		return DDRMON_ERR;
	if (data_rate_mhz)
		*data_rate_mhz = g_ctx.data_rate_mhz;
	if (bus_width_bits)
		*bus_width_bits = g_ctx.bus_width_bits ? g_ctx.bus_width_bits : 32;
	return DDRMON_OK;
}

int ddrmon_get_runtime(uint64_t *runtime_ms)
{
	if (!g_ctx.initialized)
		return DDRMON_ERR;
	if (!g_ctx.running && g_ctx.start_time_ms == 0)
		return DDRMON_EEMPTY;

	if (runtime_ms) {
		uint64_t current = (uint64_t)aos_now_ms();
		*runtime_ms = (current >= g_ctx.start_time_ms) ?
					  (current - g_ctx.start_time_ms) : 0;
	}
	return DDRMON_OK;
}

int ddrmon_set_log_interval(uint32_t interval_sec)
{
	if (!g_ctx.initialized)
		return DDRMON_ERR;

	g_ctx.log_interval_ms = interval_sec * 1000;

	// Reset log timestamp to avoid immediate trigger
	if (g_ctx.running) {
		g_ctx.last_log_time_ms = (uint64_t)aos_now_ms();
	}

	return DDRMON_OK;
}

uint32_t ddrmon_get_log_interval(void)
{
	return g_ctx.log_interval_ms / 1000;  // Convert ms to seconds
}

int ddrmon_get_ring_status(uint32_t *total_samples, uint32_t *ring_capacity)
{
	if (!g_ctx.initialized)
		return DDRMON_ERR;
	ddrmon_stats_get_info(total_samples, ring_capacity);
	return DDRMON_OK;
}

int ddrmon_deinit(void)
{
	if (!g_ctx.initialized)
		return DDRMON_OK;

	// Stop sampling thread if running
	if (g_ctx.running) {
		ddrmon_stop();
	}

	// Clean up statistics module
	ddrmon_stats_deinit();

	// Clean up mutex
	if (aos_mutex_is_valid(&g_ctx.lock)) {
		aos_mutex_free(&g_ctx.lock);
	}

	// Clear context
	memset(&g_ctx, 0, sizeof(g_ctx));
	return DDRMON_OK;
}
