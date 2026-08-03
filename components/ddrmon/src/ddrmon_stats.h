/* SPDX-License-Identifier: Apache-2.0 */

#ifndef CVI_DDRMON_STATS_H_
#define CVI_DDRMON_STATS_H_
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize statistics ring buffer
 * @note Called once during ddrmon_init()
 */
void ddrmon_stats_init(void);

/**
 * @brief Cleanup statistics module resources
 * @note Called during ddrmon_deinit()
 */
void ddrmon_stats_deinit(void);

/**
 * @brief Reset statistics ring buffer (clear all samples)
 * @note Automatically called during ddrmon_start() to avoid stale data pollution.
 *       Can also be called manually via 'ddrmon reset' command to restart statistics
 *       without stopping the monitor.
 * @note Thread-safe, can be called while monitoring is running
 */
void ddrmon_stats_reset(void);

/**
 * @brief Push new bandwidth sample to ring buffer
 * @param ts_ms Timestamp in milliseconds
 * @param rd Read bandwidth in MB/s
 * @param wr Write bandwidth in MB/s
 * @param tot Total bandwidth in MB/s
 * @note Thread-safe, called by sampling thread
 */
void ddrmon_stats_push(uint64_t ts_ms, double rd, double wr, double tot);

/**
 * @brief Query bandwidth statistics over time window
 * @param last_ms Time window in milliseconds (0 = all available data)
 * @param min_v Output: minimum bandwidth (can be NULL)
 * @param max_v Output: maximum bandwidth (can be NULL)
 * @param avg_v Output: average bandwidth (can be NULL)
 * @param sample_count Output: number of samples used in calculation (can be NULL)
 * @return 0 on success, -1 if no data, -2 if no samples in window
 * @note Thread-safe, scans ring buffer backwards from newest sample
 * @note IMPORTANT: Subject to ring buffer capacity limitation (default 16384 samples).
 *       Max retention window = buffer_capacity * sampling_period.
 */
int ddrmon_stats_query_ex(uint64_t last_ms, double *min_v, double *max_v, double *avg_v, uint32_t *sample_count);

/**
 * @brief Get ring buffer status information
 * @param total_samples Output: number of valid samples in ring (can be NULL)
 * @param ring_capacity Output: maximum ring buffer capacity (can be NULL)
 */
void ddrmon_stats_get_info(uint32_t *total_samples, uint32_t *ring_capacity);

/**
 * @brief Get time span of samples in ring buffer
 * @param span_ms Output: time span between newest and oldest sample in milliseconds (can be NULL)
 * @note Returns 0 if buffer has less than 2 samples
 */
void ddrmon_stats_get_span(uint64_t *span_ms);

/**
 * @brief Query bandwidth statistics with read/write breakdown
 * @param last_ms Time window in milliseconds (0 = all available data)
 * @param min_rd Output: minimum read bandwidth (can be NULL)
 * @param max_rd Output: maximum read bandwidth (can be NULL)
 * @param avg_rd Output: average read bandwidth (can be NULL)
 * @param min_wr Output: minimum write bandwidth (can be NULL)
 * @param max_wr Output: maximum write bandwidth (can be NULL)
 * @param avg_wr Output: average write bandwidth (can be NULL)
 * @param min_tot Output: minimum total bandwidth (can be NULL)
 * @param max_tot Output: maximum total bandwidth (can be NULL)
 * @param avg_tot Output: average total bandwidth (can be NULL)
 * @param sample_count Output: number of samples used in calculation (can be NULL)
 * @return 0 on success, -1 if no data, -2 if no samples in window
 * @note Thread-safe, scans ring buffer backwards from newest sample
 */
struct ddrmon_stats_rw_query {
	uint64_t last_ms;
	double min_rd;
	double max_rd;
	double avg_rd;
	double min_wr;
	double max_wr;
	double avg_wr;
	double min_tot;
	double max_tot;
	double avg_tot;
	uint32_t sample_count;
};

int ddrmon_stats_query_rw(struct ddrmon_stats_rw_query *q);

typedef struct {
	uint64_t ts_ms;
	double rd;
	double wr;
	double tot;
} ddrmon_sample_t;

int ddrmon_stats_peek_range(uint64_t start_ms, uint64_t end_ms,
	ddrmon_sample_t *out, uint32_t max_n, uint32_t *out_n);

#ifdef __cplusplus
}
#endif
#endif
