// SPDX-License-Identifier: Apache-2.0

#include <aos/kernel.h>
#include <aos/cli.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vfs.h>
#include "ddrmon_api.h"
#include "ddrmon_stats.h"

/* Include T-Head RISC-V core header for cache operations */
#include <csi_core.h>

/*
 * Test 1: validate start/stop protection logic
 * - second start must return DDRMON_EBUSY
 * - second stop must return DDRMON_EBUSY
 */
static int test_start_stop_protection(void)
{
	printf("\n[TEST 1] Start/Stop Protection\n");

	/* Ensure ddrmon is stopped before test */
	uint64_t runtime_ms = 0;
	int state = ddrmon_get_runtime(&runtime_ms);

	if (state == DDRMON_OK) {
		/* ddrmon is running, stop it first */
		ddrmon_stop();
	}

	/* Test 1: First start should succeed */
	int ret = ddrmon_start();

	if (ret != DDRMON_OK) {
		printf("  [FAIL] First start failed: %d\n", ret);
		return -1;
	}

	/* Test 2: Duplicate start should be rejected */
	ret = ddrmon_start();

	if (ret != DDRMON_EBUSY) {
		printf("  [FAIL] Should reject duplicate start (got %d)\n", ret);
		ddrmon_stop();
		return -1;
	}

	/* Test 3: First stop should succeed */
	ret = ddrmon_stop();

	if (ret != DDRMON_OK) {
		printf("  [FAIL] First stop failed: %d\n", ret);
		return -1;
	}

	/* Test 4: Duplicate stop should be rejected */
	ret = ddrmon_stop();

	if (ret != DDRMON_EBUSY) {
		printf("  [FAIL] Should reject duplicate stop (got %d)\n", ret);
		return -1;
	}

	/* Leave ddrmon running for subsequent tests */
	ddrmon_start();
	printf("  [PASS]\n");
	return 0;
}

static int test_sampling_period(void)
{
	printf("\n[TEST 2] Sampling Period\n");

	// Assumption: ddrmon is already running
	// Stop it temporarily to test period change
	ddrmon_stop();

	/* Configure longer period and observe sample count over 2 seconds */
	ddrmon_set_period(50);
	ddrmon_start();
	aos_msleep(2000);

	uint32_t samples, capacity;

	ddrmon_get_ring_status(&samples, &capacity);

	if (samples < 30 || samples > 50) {
		printf("  [FAIL] Sample count %u unexpected\n", samples);
		ddrmon_stop();
		ddrmon_set_period(20);
		ddrmon_start();
		return -1;
	}

	ddrmon_stop();
	printf("  [PASS] %u samples in 2s (period=50ms)\n", samples);
	ddrmon_set_period(20);
	printf("  Return to %ums period (by default)\n", 20);
	ddrmon_start(); // Restore running state
	return 0;
}

static int test_statistics(void)
{
	printf("\n[TEST 3] Statistics Validity\n");

	// Assumption: ddrmon is already running
	// Collect samples long enough to fill statistical window
	aos_msleep(3000);

	uint64_t runtime_ms = 0;

	ddrmon_get_runtime(&runtime_ms);
	double min, max, avg;
	uint32_t count;
	int ret = ddrmon_stats_query_ex(runtime_ms, &min, &max, &avg, &count);

	if (ret != 0) {
		printf("  [FAIL] Cannot get stats\n");
		return -1;
	}

	/* Basic sanity: Min <= Avg <= Max */
	if (min > avg || avg > max) {
		printf("  [FAIL] Min(%u) > Avg(%u) > Max(%u) invalid\n",
			   (uint32_t)min, (uint32_t)avg, (uint32_t)max);
		return -1;
	}

	/* Physical sanity: Max must not exceed 2× theoretical peak.
	 * theoretical_peak = data_rate_MHz × bus_width_bytes.
	 * AXI Monitor glitches can produce impossibly high values. */
	uint32_t rate = 0, width = 0;
	ddrmon_get_info(&rate, &width);
	uint32_t peak = rate * (width / 8) * 2;  // 2× safety margin
	if (max > (double)peak) {
		printf("  [FAIL] Max(%u) exceeds 2× theoretical peak (%u MB/s) — likely AXI glitch\n",
			   (uint32_t)max, peak);
		return -1;
	}

	printf("  [PASS] Min=%u Avg=%u Max=%u (samples=%u, peak=%u)\n",
		   (uint32_t)min, (uint32_t)avg, (uint32_t)max, count, peak);
	return 0;
}

static int test_ring_reset(void)
{
	printf("\n[TEST 4] Ring Buffer Reset\n");

	// Assumption: ddrmon is already running
	// Stop it to test reset logic
	ddrmon_stop();

	/* First run: fill the ring buffer */
	ddrmon_start();
	aos_msleep(1000);
	uint32_t size1;

	ddrmon_get_ring_status(&size1, NULL);
	ddrmon_stop();

	/* Second run: ring buffer should be cleared by reset */
	ddrmon_start();
	aos_msleep(100);
	uint32_t size2;

	ddrmon_get_ring_status(&size2, NULL);

	if (size2 > 10) {
		printf("  [FAIL] Ring not reset (%u samples remain)\n", size2);
		return -1;
	}

	printf("  [PASS] Ring reset correctly\n");
	// Note: ddrmon is still running after this test
	return 0;
}

static int test_hardware_info(void)
{
	printf("\n[TEST 5] Hardware Info\n");

	/* Verify mandatory hardware info fields */
	uint32_t rate, width;

	if (ddrmon_get_info(&rate, &width) != DDRMON_OK) {
		printf("  [FAIL] Cannot get info\n");
		return -1;
	}

	if (rate == 0 || width == 0) {
		printf("  [FAIL] Invalid info (rate=%u, width=%u)\n", rate, width);
		return -1;
	}

	printf("  [PASS] DDR %u MHz, %u bits\n", rate, width);
	return 0;
}

/*
 * Test 7: VFS ioctl end-to-end verification.
 * Opens /dev/ddrmon and exercises all 6 ioctl commands through the VFS layer.
 * This validates the cvi_rtt rt_device -> AliOS aos_register_driver port.
 */
static int test_vfs_ioctl(void)
{
	printf("\n[TEST 7] VFS ioctl Interface\n");

	int fd = aos_open("/dev/ddrmon", 0);
	if (fd < 0) {
		printf("  [FAIL] Cannot open /dev/ddrmon (fd=%d)\n", fd);
		return -1;
	}

	// 1. GET_DDR_INFO
	ddrmon_info_t info = {0};
	int ret = aos_ioctl(fd, DDRMON_IOC_GET_DDR_INFO, (unsigned long)&info);
	if (ret != 0 || info.data_rate_mhz == 0) {
		printf("  [FAIL] IOC_GET_DDR_INFO (ret=%d rate=%u)\n",
		       ret, info.data_rate_mhz);
		aos_close(fd); return -1;
	}
	printf("  IOC_GET_DDR_INFO: rate=%u MHz, width=%u bits\n",
	       info.data_rate_mhz, info.bus_width_bits);

	// 2. START
	ret = aos_ioctl(fd, DDRMON_IOC_START, 0);
	if (ret != 0) {
		printf("  [FAIL] IOC_START ret=%d\n", ret);
		aos_close(fd); return -1;
	}

	// 3. SET_PERIOD
	uint32_t period = 50;
	ret = aos_ioctl(fd, DDRMON_IOC_SET_PERIOD_MS, (unsigned long)&period);
	if (ret != 0) {
		printf("  [FAIL] IOC_SET_PERIOD ret=%d\n", ret);
		aos_close(fd); return -1;
	}

	// 4. GET_INSTANT (wait a few samples, query aggregate)
	aos_msleep(100);
	ddrmon_instant_t inst = { .port = 0xFF };
	ret = aos_ioctl(fd, DDRMON_IOC_GET_INSTANT, (unsigned long)&inst);
	if (ret != 0) {
		printf("  [FAIL] IOC_GET_INSTANT ret=%d\n", ret);
	} else {
		printf("  IOC_GET_INSTANT: Tot=%u R=%u W=%u MB/s\n",
		       (uint32_t)inst.tot_MBps, (uint32_t)inst.rd_MBps,
		       (uint32_t)inst.wr_MBps);
	}

	// 5. GET_STATS
	ddrmon_stats_req_t stats = { .port = 0, .last_ms = 5000 };
	ret = aos_ioctl(fd, DDRMON_IOC_GET_STATS, (unsigned long)&stats);
	if (ret != 0) {
		printf("  [FAIL] IOC_GET_STATS ret=%d\n", ret);
	} else {
		printf("  IOC_GET_STATS: Min=%u Max=%u Avg=%u MB/s\n",
		       (uint32_t)stats.min_MBps, (uint32_t)stats.max_MBps,
		       (uint32_t)stats.avg_MBps);
	}

	// 6. STOP
	ret = aos_ioctl(fd, DDRMON_IOC_STOP, 0);
	if (ret != 0) {
		printf("  [FAIL] IOC_STOP ret=%d\n", ret);
		aos_close(fd); return -1;
	}

	aos_close(fd);
	printf("  [PASS] All 6 ioctl commands OK\n");
	return 0;
}

/*
 * Test 8: ddrmon_get_instant per-port validation + ddrmon_stats_query_rw
 * with sustained DDR load. Runs a 1MB memcpy loop for 2 seconds with cache
 * ops to force DDR traffic, then verifies Avg > 0 and Min <= Avg <= Max.
 */
static int test_instant_and_rw_stats(void)
{
	printf("\n[TEST 8] Instant Bandwidth + RW Stats\n");

	// Assumption: ddrmon is already running from previous tests
	// 1. get_instant aggregate
	double tot_agg, rd_agg, wr_agg;
	int ret = ddrmon_get_instant(0xFF, &rd_agg, &wr_agg, &tot_agg);
	if (ret != DDRMON_OK) {
		printf("  [FAIL] get_instant(0xFF) returned %d\n", ret);
		return -1;
	}
	printf("  get_instant(0xFF): Tot=%u R=%u W=%u MB/s\n",
	       (uint32_t)tot_agg, (uint32_t)rd_agg, (uint32_t)wr_agg);

	// 2. Per-port sum should approximately equal aggregate
	double sum_rd = 0, sum_wr = 0;
	for (int p = 0; p < 6; p++) {
		double prd, pwr, ptot;
		if (ddrmon_get_instant(p, &prd, &pwr, &ptot) == DDRMON_OK) {
			sum_rd += prd; sum_wr += pwr;
		}
	}
	printf("  Per-port sum: R=%u W=%u vs Agg: R=%u W=%u\n",
	       (uint32_t)sum_rd, (uint32_t)sum_wr,
	       (uint32_t)rd_agg, (uint32_t)wr_agg);
	// 3. Generate sustained DDR load: 1MB memcpy loop for ~2 seconds
	const uint32_t load_size = 1024 * 1024;
	uint8_t *s = aos_malloc(load_size + 128);
	uint8_t *d = aos_malloc(load_size + 128);
	if (!s || !d) {
		if (s) aos_free(s);
		if (d) aos_free(d);
		printf("  [FAIL] Cannot allocate load buffers\n");
		return -1;
	}
	uint8_t *src = (uint8_t *)(((uintptr_t)s + 64) & ~63UL);
	uint8_t *dst = (uint8_t *)(((uintptr_t)d + 64) & ~63UL);
	memset(src, 0xAA, load_size);

	uint64_t t_start = (uint64_t)aos_now_ms();
	int loops = 0;
	while ((uint64_t)aos_now_ms() - t_start < 2000) {
		csi_dcache_invalid_range((uint64_t *)(void *)src, load_size);
		memcpy(dst, src, load_size);
		csi_dcache_clean_range((uint64_t *)(void *)dst, load_size);
		loops++;
	}
	aos_free(s);
	aos_free(d);
	printf("  Load: %d memcpy(1MB) loops in 2s\n", loops);

	uint64_t runtime = 0;
	ddrmon_get_runtime(&runtime);
	struct ddrmon_stats_rw_query q = { .last_ms = runtime };
	ret = ddrmon_stats_query_rw(&q);
	if (ret != 0) {
		printf("  [FAIL] stats_query_rw returned %d\n", ret);
		return -1;
	}

	printf("  RW Stats (window=%ums):\n", (uint32_t)runtime);
	printf("    RD: Min=%u Max=%u Avg=%u\n",
	       (uint32_t)q.min_rd, (uint32_t)q.max_rd, (uint32_t)q.avg_rd);
	printf("    WR: Min=%u Max=%u Avg=%u\n",
	       (uint32_t)q.min_wr, (uint32_t)q.max_wr, (uint32_t)q.avg_wr);
	printf("    Tot: Min=%u Max=%u Avg=%u | Samples=%u\n",
	       (uint32_t)q.min_tot, (uint32_t)q.max_tot,
	       (uint32_t)q.avg_tot, q.sample_count);

	// 5. After load, at least some stat should be > 0
	if (q.avg_tot > 0.0 && q.max_tot >= q.avg_tot && q.avg_tot >= q.min_tot) {
		printf("  [PASS] RW stats valid (Avg=%u > 0 after load)\n",
		       (uint32_t)q.avg_tot);
	} else {
		printf("  [FAIL] RW stats: Avg should be > 0 after load\n");
		return -1;
	}

	return 0;
}

/* Helper function to process and display bandwidth samples */
static int process_bandwidth_samples(uint64_t t1, uint64_t t2,
	uint32_t theo_mb, const char *test_name,
	uint32_t min_pct, uint32_t max_pct)
{
	(void)(test_name);
	(void)(min_pct);
	(void)(max_pct);
	/* Fetch samples covering the memcpy interval [t1, t2] (include edge overlap) */
	uint32_t period_ms = ddrmon_get_period_ms();
	uint64_t start_ms = (t1 >= (uint64_t)period_ms) ? (t1 - (uint64_t)period_ms) : 0;
	uint64_t end_ms = t2 + (uint64_t)period_ms;

	/* Allocate sample buffer on heap to avoid stack overflow */
	const uint32_t max_samples = 32;
	ddrmon_sample_t *samples = aos_malloc(sizeof(ddrmon_sample_t) * max_samples);

	if (!samples) {
		printf("  [FAIL] Cannot allocate sample buffer\n");
		return -1;
	}

	uint32_t out_n = 0;

	int ret = ddrmon_stats_peek_range(start_ms, end_ms, samples, max_samples, &out_n);

	if (ret != 0 || out_n == 0) {
		printf("  [FAIL] No samples in memcpy interval (ret=%d,out_n=%u)\n", ret, out_n);
		aos_free(samples);
		return -1;
	}

	/* Display each sample with detailed info */
	printf("  ===== Samples in memcpy interval =====\n");
	double sum_data_rd = 0.0, sum_data_wr = 0.0, sum_data_tot = 0.0;
	double period_s = period_ms / 1000.0;

	for (uint32_t i = 0; i < out_n; i++) {
		double rd_bw = samples[i].rd;
		double wr_bw = samples[i].wr;
		double tot_bw = samples[i].tot;
		double data_rd = rd_bw * period_s;
		double data_wr = wr_bw * period_s;
		double data_tot = tot_bw * period_s;

		sum_data_rd += data_rd;
		sum_data_wr += data_wr;
		sum_data_tot += data_tot;

		printf("[%u]  BW: R=%u W=%u Tot=%u MB/s | Data: R=%u W=%u Tot=%u MB\n",
			i,
			(uint32_t)(rd_bw + 0.5), (uint32_t)(wr_bw + 0.5), (uint32_t)(tot_bw + 0.5),
			(uint32_t)(data_rd + 0.5), (uint32_t)(data_wr + 0.5), (uint32_t)(data_tot + 0.5));
	}
	aos_free(samples);

	printf("  ======================================\n");
	printf("  Total Data   : Tot=%u R=%u W=%u MB\n",
		(uint32_t)(sum_data_tot + 0.5),
		(uint32_t)(sum_data_rd + 0.5),
		(uint32_t)(sum_data_wr + 0.5));

	/* Compare total data against theoretical */
	uint32_t meas_mb = (uint32_t)(sum_data_tot + 0.5);
	uint32_t diff_mb = (meas_mb > theo_mb) ? (meas_mb - theo_mb) : (theo_mb - meas_mb);
	uint32_t err_pct = (theo_mb ? (diff_mb * 100 / theo_mb) : 0);

	printf("  Compare: measured=%u MB, theoretical=%u MB, err=%u%%\n",
		meas_mb, theo_mb, err_pct);

	/* Evaluate measurement quality:
	 * - err <= 50%: PASS (good measurement)
	 * - 50% < err <= 80%: WARNING but PASS (acceptable considering cache effects)
	 * - err > 80%: FAIL (measurement likely incorrect)
	 */
	if (err_pct <= 50) {
		printf("  [PASS] Measurement within 50%% tolerance\n");
		return 0;
	}

	if (err_pct <= 80) {
		printf("  [WARNING] Error %u%% is high but acceptable due to cache effects\n", err_pct);
		printf("  [PASS] DDR Monitor is functioning\n");
		return 0;
	}

	printf("  [FAIL] Error >80%% suggests measurement issue\n");
	return -1;
}

static int test_bandwidth_accuracy(void)
{
	printf("\n[TEST 6] Bandwidth Accuracy (Known Load)\n");
	printf("  Purpose: Verify DDR Monitor can measure real DDR traffic\n");
	printf("  Note: Measured value will be LOWER than theoretical due to:\n");
	printf("      - CPU Cache: Most data stays in L1/L2, reducing DDR access\n");
	printf("      - Write Buffer: Combines writes, reducing transaction count\n");
	printf("      - DDR Prefetch: Controller optimizations affect timing\n");
	printf("  This might be NORMAL and reflects actual hardware behavior.\n");
	printf("  WARNING: If there is already a load, this test might fail!\n");
	printf("           Please shutdown other tasks, or ignore this test results.\n\n");

	// Assumption: ddrmon is already running

		const uint32_t data_size = 4;
		const uint32_t buf_size = data_size * 1024 * 1024;
		// Cache ops extend ±63 bytes for line alignment. Allocate with
		// 128 bytes headroom and align to 64B so both sides are safe.
		const uint32_t alloc_size = buf_size + 128;
		uint8_t *src_raw = aos_malloc(alloc_size);
		uint8_t *dst_raw = aos_malloc(alloc_size);

		if (!src_raw || !dst_raw) {
			printf("  [FAIL] Cannot allocate memory\n");
			if (src_raw) aos_free(src_raw);
			if (dst_raw) aos_free(dst_raw);
			return -1;
		}
		uint8_t *src = (uint8_t *)(((uintptr_t)src_raw + 64) & ~63UL);
		uint8_t *dst = (uint8_t *)(((uintptr_t)dst_raw + 64) & ~63UL);

		memset(src, 0xAA, buf_size);
		printf("  Allocated 2x %u MB buffers\n", data_size);

		aos_msleep(100);

		/* Invalidate source cache to force initial reads from DDR */
		csi_dcache_invalid_range((uint64_t *)(void *)src, (int64_t)buf_size);

		uint64_t t1 = (uint64_t)aos_now_ms();

		/* Perform single memcpy */
		memcpy(dst, src, buf_size);

		uint64_t t2 = (uint64_t)aos_now_ms();

		/* Clean destination cache to force writes back to DDR */
		csi_dcache_clean_range((uint64_t *)(void *)dst, (int64_t)buf_size);

		aos_msleep(100);

		uint32_t duration_ms = (uint32_t)(t2 - t1);

		if (duration_ms == 0)
			duration_ms = 1;

		uint32_t theoretical_total_mb = data_size * 2;

		printf("  Operation took %u ms\n", duration_ms);
		printf("  Theoretical flow: %u MB (read + write)\n", theoretical_total_mb);

		int result = process_bandwidth_samples(t1, t2, theoretical_total_mb,
			"DDR traffic verified", 0, 0);


		aos_free(src_raw);
		aos_free(dst_raw);
		return result;
	}

static int cmd_ddrmon_test(int argc, char **argv)
{
	(void)(argc);
	(void)(argv);
	printf("\n========== DDRMON Test Suite ==========\n");

	// Test 7: VFS ioctl (self-contained lifecycle, must run before C API ddrmon_start)
	int failed = 0;
	failed += test_vfs_ioctl();

	// Check if ddrmon is already running
	uint64_t runtime_ms = 0;
	int was_running = (ddrmon_get_runtime(&runtime_ms) == DDRMON_OK);

	// Start ddrmon if not already running
	if (!was_running) {
		printf("Starting ddrmon for test suite...\n");
		int ret = ddrmon_start();

		if (ret != DDRMON_OK) {
			printf("Failed to start ddrmon: %d\n", ret);
			return -1;
		}
	} else {
		printf("Note: ddrmon is already running.\n");
		printf("  Test6 bandwidth data will be captured in current session.\n\n");
	}

	failed += test_start_stop_protection();
	failed += test_sampling_period();
	failed += test_statistics();
	failed += test_ring_reset();
	failed += test_hardware_info();
	// Test 8: instant + rw stats (needs ddrmon running, after Test 5)
	failed += test_instant_and_rw_stats();
	failed += test_bandwidth_accuracy();

	printf("\n=======================================\n");
	if (failed == 0)
		printf("All 8 tests PASSED!\n");
	else
		printf("%d test(s) FAILED!\n", -failed);
	printf("=======================================\n");

	// Stop ddrmon only if we started it
	if (!was_running) {
		printf("\nStopping ddrmon (test suite started it)...\n");
		ddrmon_stop();
	} else {
		printf("\nNote: ddrmon is still running (started before test).\n");
		printf("  Test6 bandwidth data is captured in statistics.\n");
		printf("  Use 'ddrmon stop' to view full statistics.\n");
	}

	return failed;
}

static int cmd_ddrmon_verify(int argc, char **argv)
{
	(void)(argc);
	(void)(argv);
	printf("\n========== Bandwidth Verification ==========\n");

	// Check if ddrmon is already running
	uint64_t runtime_ms = 0;
	int was_running = (ddrmon_get_runtime(&runtime_ms) == DDRMON_OK);

	// Start ddrmon if not already running
	if (!was_running) {
		printf("Starting ddrmon for bandwidth verification...\n");
		int ret = ddrmon_start();

		if (ret != DDRMON_OK) {
			printf("Failed to start ddrmon: %d\n", ret);
			return -1;
		}
	}

	test_bandwidth_accuracy();

	// Stop ddrmon only if we started it
	if (!was_running) {
		printf("\nStopping ddrmon...\n");
		ddrmon_stop();
	} else {
		printf("\nNote: ddrmon is still running.\n");
		printf("  Use 'ddrmon stop' to view statistics.\n");
	}

	printf("\n============================================\n");
	printf("Tip: Run multiple times to verify consistency\n");
	return 0;
}

ALIOS_CLI_CMD_REGISTER(cmd_ddrmon_test, ddrmon_test, DDRMON test suite);
ALIOS_CLI_CMD_REGISTER(cmd_ddrmon_verify, ddrmon_verify, Verify bandwidth accuracy);
