// SPDX-License-Identifier: Apache-2.0

#include <aos/kernel.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ddrmon_stats.h"

// Determine ring depth from configuration, with safe fallback
#ifndef DDRMON_RING_DEPTH
# ifdef CVI_DDRMON_RING_DEPTH
#  define DDRMON_RING_DEPTH CVI_DDRMON_RING_DEPTH
# else
#  define DDRMON_RING_DEPTH 16384
# endif
#endif

static ddrmon_sample_t g_ring[DDRMON_RING_DEPTH];
static uint32_t g_head;
static uint32_t g_size;
static aos_mutex_t g_lock;

void ddrmon_stats_init(void)
{
	g_head = g_size = 0;
	memset(g_ring, 0, sizeof(g_ring));
	if (!aos_mutex_is_valid(&g_lock)) {
		if (aos_mutex_new(&g_lock) != 0) {
			printf("[ddrmon] WARNING: stats mutex create failed\n");
		}
	}
}

void ddrmon_stats_deinit(void)
{
	if (aos_mutex_is_valid(&g_lock)) {
		aos_mutex_free(&g_lock);
	}
	g_head = g_size = 0;
}

void ddrmon_stats_reset(void)
{
	if (!aos_mutex_is_valid(&g_lock))
		return;
	aos_mutex_lock(&g_lock, AOS_WAIT_FOREVER);
	g_head = g_size = 0;
	memset(g_ring, 0, sizeof(g_ring));
	aos_mutex_unlock(&g_lock);
}

void ddrmon_stats_push(uint64_t ts_ms, double rd, double wr, double tot)
{
	if (!aos_mutex_is_valid(&g_lock))
		return; // Avoid crash if not initialized
	aos_mutex_lock(&g_lock, AOS_WAIT_FOREVER);
	uint32_t idx = g_head % DDRMON_RING_DEPTH;

	g_ring[idx].ts_ms = ts_ms;
	g_ring[idx].rd = rd;
	g_ring[idx].wr = wr;
	g_ring[idx].tot = tot;
	g_head++;
	if (g_size < DDRMON_RING_DEPTH)
		g_size++;
	aos_mutex_unlock(&g_lock);
}

int ddrmon_stats_query_ex(uint64_t last_ms, double *min_v, double *max_v, double *avg_v, uint32_t *sample_count)
{
	if (!aos_mutex_is_valid(&g_lock))
		return -1; // Avoid crash if not initialized
	aos_mutex_lock(&g_lock, AOS_WAIT_FOREVER);
	if (g_size == 0) {
		aos_mutex_unlock(&g_lock);
		return -1;
	}
	uint64_t now = (uint64_t)aos_now_ms();
	double min_b = 0, max_b = 0, sum = 0.0; uint32_t n = 0;
	int first_sample = 1; // Flag to initialize min/max with first valid sample

	for (uint32_t i = 0; i < g_size; ++i) {
		uint32_t idx = (g_head - 1 - i + DDRMON_RING_DEPTH) % DDRMON_RING_DEPTH; // newest backward
		// Guard against timestamp overflow or time going backwards
		if (now < g_ring[idx].ts_ms || now - g_ring[idx].ts_ms > last_ms)
			break;
		double v = g_ring[idx].tot;

		if (first_sample) {
			min_b = v;
			max_b = v;
			first_sample = 0;
		} else {
			if (v < min_b)
				min_b = v;
			if (v > max_b)
				max_b = v;
		}
		sum += v; n++;
	}
	aos_mutex_unlock(&g_lock);
	if (n == 0)
		return -2;
	if (min_v)
		*min_v = min_b;
	if (max_v)
		*max_v = max_b;
	if (avg_v)
		*avg_v = sum / (double)n;
	if (sample_count)
		*sample_count = n;
	return 0;
}

int ddrmon_stats_peek_range(uint64_t start_ms, uint64_t end_ms,
	ddrmon_sample_t *out, uint32_t max_n, uint32_t *out_n)
{
	uint32_t n = 0;

	if (!aos_mutex_is_valid(&g_lock))
		return -1;

	aos_mutex_lock(&g_lock, AOS_WAIT_FOREVER);
	if (g_size > 0) {
		uint32_t oldest = (g_head + DDRMON_RING_DEPTH - g_size) % DDRMON_RING_DEPTH;

		for (uint32_t i = 0; i < g_size; i++) {
			uint32_t idx = (oldest + i) % DDRMON_RING_DEPTH;
			uint64_t ts = g_ring[idx].ts_ms;

			if (ts < start_ms)
				continue;
			if (ts > end_ms)
				break;
			if (n < max_n && out) {
				out[n].ts_ms = g_ring[idx].ts_ms;
				out[n].rd = g_ring[idx].rd;
				out[n].wr = g_ring[idx].wr;
				out[n].tot = g_ring[idx].tot;
				n++;
			} else if (n >= max_n) {
				break;
			}
		}
	}
	aos_mutex_unlock(&g_lock);

	if (out_n)
		*out_n = n;

	return 0;
}

void ddrmon_stats_get_info(uint32_t *total_samples, uint32_t *ring_capacity)
{
	if (!aos_mutex_is_valid(&g_lock))
		return;
	aos_mutex_lock(&g_lock, AOS_WAIT_FOREVER);
	if (total_samples)
		*total_samples = g_size;
	if (ring_capacity)
		*ring_capacity = DDRMON_RING_DEPTH;
	aos_mutex_unlock(&g_lock);
}

void ddrmon_stats_get_span(uint64_t *span_ms)
{
	if (!span_ms)
		return;
	*span_ms = 0;
	if (!aos_mutex_is_valid(&g_lock))
		return;

	aos_mutex_lock(&g_lock, AOS_WAIT_FOREVER);
	if (g_size >= 2) {
		uint32_t newest_idx = (g_head + DDRMON_RING_DEPTH - 1) % DDRMON_RING_DEPTH;
		uint32_t oldest_idx = (g_head + DDRMON_RING_DEPTH - g_size) % DDRMON_RING_DEPTH;
		uint64_t newest_ts = g_ring[newest_idx].ts_ms;
		uint64_t oldest_ts = g_ring[oldest_idx].ts_ms;

		if (newest_ts >= oldest_ts) {
			*span_ms = newest_ts - oldest_ts;
		}
	}
	aos_mutex_unlock(&g_lock);
}

// Query bandwidth statistics with read/write breakdown
int ddrmon_stats_query_rw(struct ddrmon_stats_rw_query *q)
{
	if (!aos_mutex_is_valid(&g_lock)) {
		return -1;
	}
	if (!q) {
		return -1;
	}
	aos_mutex_lock(&g_lock, AOS_WAIT_FOREVER);
	if (g_size == 0) {
		aos_mutex_unlock(&g_lock);
		return -1;
	}
	uint64_t now = (uint64_t)aos_now_ms();
	double min_r = 0, max_r = 0, sum_r = 0.0;
	double min_w = 0, max_w = 0, sum_w = 0.0;
	double min_t = 0, max_t = 0, sum_t = 0.0;
	uint32_t n = 0;
	int first_sample = 1;

	for (uint32_t i = 0; i < g_size; ++i) {
		uint32_t idx = (g_head - 1 - i + DDRMON_RING_DEPTH) % DDRMON_RING_DEPTH;

		if (now < g_ring[idx].ts_ms || now - g_ring[idx].ts_ms > q->last_ms)
			break;

		double vr = g_ring[idx].rd;
		double vw = g_ring[idx].wr;
		double vt = g_ring[idx].tot;

		if (first_sample) {
			min_r = vr; max_r = vr;
			min_w = vw; max_w = vw;
			min_t = vt; max_t = vt;
			first_sample = 0;
		} else {
			if (vr < min_r)
				min_r = vr;
			if (vr > max_r)
				max_r = vr;
			if (vw < min_w)
				min_w = vw;
			if (vw > max_w)
				max_w = vw;
			if (vt < min_t)
				min_t = vt;
			if (vt > max_t)
				max_t = vt;
		}
		sum_r += vr; sum_w += vw; sum_t += vt;
		n++;
	}
	aos_mutex_unlock(&g_lock);

	if (n == 0)
		return -2;

	q->min_rd = min_r;
	q->max_rd = max_r;
	q->avg_rd = sum_r / (double)n;
	q->min_wr = min_w;
	q->max_wr = max_w;
	q->avg_wr = sum_w / (double)n;
	q->min_tot = min_t;
	q->max_tot = max_t;
	q->avg_tot = sum_t / (double)n;
	q->sample_count = n;
	return 0;
}
