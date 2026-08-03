// SPDX-License-Identifier: Apache-2.0

#include <aos/kernel.h>
#include <aos/cli.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ddrmon_api.h"
#include "ddrmon_stats.h"

/* Structure to hold DDR monitor statistics */
struct ddrmon_stats_ctx {
	/* Hardware info */
	uint32_t rate_mhz;
	uint32_t bus_bits;
	/* Bandwidth statistics */
	double min_rd, max_rd, avg_rd;
	double min_wr, max_wr, avg_wr;
	double min_tot, max_tot, avg_tot;
	/* Instantaneous bandwidth */
	double inst_rd, inst_wr, inst_tot;
	/* Ring buffer info */
	uint32_t sample_cnt;
	uint32_t ring_capacity;
	uint64_t span_ms;
	/* Runtime */
	uint64_t runtime_ms;
};

/* Helper: gather DDR statistics */
static int gather_stats(uint64_t runtime_ms, struct ddrmon_stats_ctx *ctx)
{
	ddrmon_get_info(&ctx->rate_mhz, &ctx->bus_bits);
	ddrmon_get_ring_status(NULL, &ctx->ring_capacity);
	ddrmon_stats_get_span(&ctx->span_ms);
	ctx->runtime_ms = runtime_ms;
	struct ddrmon_stats_rw_query q;

	memset(&q, 0, sizeof(q));
	q.last_ms = runtime_ms;
	int ret = ddrmon_stats_query_rw(&q);

	if (ret == 0) {
		ctx->min_rd = q.min_rd; ctx->max_rd = q.max_rd; ctx->avg_rd = q.avg_rd;
		ctx->min_wr = q.min_wr; ctx->max_wr = q.max_wr; ctx->avg_wr = q.avg_wr;
		ctx->min_tot = q.min_tot; ctx->max_tot = q.max_tot; ctx->avg_tot = q.avg_tot;
		ctx->sample_cnt = q.sample_count;
	}
	return ret;
}

/* Helper: display statistics */
static void display_stats(const char *title, const struct ddrmon_stats_ctx *ctx,
	int stats_ret, int show_instant)
{
	/* Display formatted DDR monitor statistics */
	if (stats_ret == 0) {
		uint64_t max_window_ms = ((uint64_t)ctx->ring_capacity) * ddrmon_get_period_ms();
		uint64_t effective_window_ms = (ctx->span_ms > 0) ? ctx->span_ms : ctx->runtime_ms;

		printf("\n===== %s =====\n", title);
		printf("DDR Rate   : %u MHz\n", ctx->rate_mhz);
		printf("Bus Width  : %u bits\n", ctx->bus_bits);
		printf("Sample Period: %u ms\n", ddrmon_get_period_ms());
		printf("Ring Buffer: %u/%u samples\n", ctx->sample_cnt, ctx->ring_capacity);
		printf("----------------------------\n");

		// Display instantaneous bandwidth (only for info command)
		if (show_instant) {
			printf("Instant BW : %u MB/s (R:%u W:%u)\n",
				   (uint32_t)(ctx->inst_tot + 0.5),
				   (uint32_t)(ctx->inst_rd + 0.5),
				   (uint32_t)(ctx->inst_wr + 0.5));
			printf("----------------------------\n");
		}

		// Display statistics
		printf("Minimum BW : %u MB/s (R:%u W:%u)\n",
			   (uint32_t)(ctx->min_tot + 0.5),
			   (uint32_t)(ctx->min_rd + 0.5),
			   (uint32_t)(ctx->min_wr + 0.5));
		printf("Maximum BW : %u MB/s (R:%u W:%u)\n",
			   (uint32_t)(ctx->max_tot + 0.5),
			   (uint32_t)(ctx->max_rd + 0.5),
			   (uint32_t)(ctx->max_wr + 0.5));
		printf("Average BW : %u MB/s (R:%u W:%u)\n",
			   (uint32_t)(ctx->avg_tot + 0.5),
			   (uint32_t)(ctx->avg_rd + 0.5),
			   (uint32_t)(ctx->avg_wr + 0.5));
		printf("Stats Window: %u.%03u s (max %u.%03u s)\n",
			   (uint32_t)(effective_window_ms / 1000),
			   (uint32_t)(effective_window_ms % 1000),
			   (uint32_t)(max_window_ms / 1000),
			   (uint32_t)(max_window_ms % 1000));

		// Warning if ring buffer is full
		if (ctx->sample_cnt >= ctx->ring_capacity) {
			printf("[WARNING] Ring buffer overflow detected! Oldest samples are being discarded.\n");
			printf("          - Use 'ddrmon reset' to clear buffer and restart statistics\n");
			printf("          - Or use 'ddrmon period <ms>' to extend retention window\n");
		}

		printf("=================================\n");
	} else {
		if (show_instant) {
			printf("No statistics available yet\n");
			printf("(monitoring for %u.%03u seconds)\n",
				   (uint32_t)(ctx->runtime_ms / 1000),
				   (uint32_t)(ctx->runtime_ms % 1000));
		} else {
			printf("[ddrmon] no statistics available (not enough samples)\n");
		}
	}
}

// CLI command handler for 'ddrmon'
static int cmd_ddrmon(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: ddrmon <command> [args]\n");
		printf("Commands without parameters: start, stop, info, reset\n");
		printf("Commands with parameters:    period <ms>, autolog [interval_sec]\n");
		return 0;
	}
	if (!strcmp(argv[1], "start")) {
		int ret = ddrmon_start();

		if (ret == DDRMON_EBUSY) {
			printf("[ddrmon] Already started. Please 'ddrmon stop' first.\n");
			return -1;
		} else if (ret != DDRMON_OK) {
			printf("[ddrmon] start failed: %d\n", ret);
			return -1;
		}
		printf("[ddrmon] started\n");
	} else if (!strcmp(argv[1], "stop")) {
		uint64_t runtime_ms = 0;

		ddrmon_get_runtime(&runtime_ms);

		struct ddrmon_stats_ctx ctx = {0};
		int stats_ret = gather_stats(runtime_ms, &ctx);

		int ret = ddrmon_stop();

		if (ret == DDRMON_EBUSY) {
			printf("[ddrmon] Not started. Please 'ddrmon start' first.\n");
			return -1;
		} else if (ret != DDRMON_OK) {
			printf("[ddrmon] stop failed: %d\n", ret);
			return -1;
		}

		printf("[ddrmon] stopped\n");

		/* Display statistics (limited by ring buffer retention window) */
		display_stats("Monitoring Statistics", &ctx, stats_ret, 0);
	} else if (!strcmp(argv[1], "period") && argc >= 3) {
		int ms = atoi(argv[2]);
		int ret = ddrmon_set_period(ms);

		if (ret != DDRMON_OK) {
			printf("[ddrmon] set period failed: %d\n", ret);
			return -1;
		}
		printf("[ddrmon] period=%d ms\n", ms);
	} else if (!strcmp(argv[1], "info")) {
		uint64_t runtime_ms = 0;
		int ret = ddrmon_get_runtime(&runtime_ms);

		if (ret == DDRMON_EEMPTY) {
			printf("[ddrmon] Not started. Please run 'ddrmon start' first.\n");
			return -1;
		} else if (ret != DDRMON_OK) {
			printf("[ddrmon] get runtime failed: %d\n", ret);
			return -1;
		}

		struct ddrmon_stats_ctx ctx = {0};
		int inst_ret = ddrmon_get_instant(0xFF, &ctx.inst_rd, &ctx.inst_wr, &ctx.inst_tot);
		int stats_ret = gather_stats(runtime_ms, &ctx);

		/* Display statistics with instantaneous bandwidth */
		display_stats("DDR Monitor Status", &ctx, stats_ret, (inst_ret == 0));
	} else if (!strcmp(argv[1], "autolog")) {
		int interval_sec = 0;

		if (argc >= 3) {
			// Set log interval: <=0 to disable, >0 to enable
			interval_sec = atoi(argv[2]);
			if (interval_sec <= 0) {
				// Disable auto-logging
				ddrmon_set_log_interval(0);
				printf("[ddrmon] Auto-logging disabled\n");
			} else {
				// Enable auto-logging with specified interval
				int ret = ddrmon_set_log_interval((uint32_t)interval_sec);

				if (ret != DDRMON_OK) {
					printf("set log interval failed: %d\n", ret);
					return -1;
				}
				uint64_t runtime_ms = 0;
				int st = ddrmon_get_runtime(&runtime_ms);

				if (st == DDRMON_ERR) {
					printf("Not initialized. Please enable in menuconfig first.\n");
					return -1;
				}
				if (st == DDRMON_EEMPTY) {
					int sret = ddrmon_start();

					if (sret != DDRMON_OK && sret != DDRMON_EBUSY) {
						printf("[ddrmon] start failed: %d\n", sret);
						return -1;
					}
				}
				printf("[ddrmon] Enable auto-logging, interval=%ds\n", interval_sec);
			}
		} else {
			// Query current log status
			interval_sec = ddrmon_get_log_interval();
			if (interval_sec > 0)
				printf("[ddrmon] Auto-logging enabled, interval=%ds\n", interval_sec);
			else
				printf("[ddrmon] Auto-logging disabled\n");
		}
	} else if (!strcmp(argv[1], "reset")) {
		// Check if monitor is initialized
		uint32_t rate_mhz = 0;
		int ret = ddrmon_get_info(&rate_mhz, NULL);

		if (ret != DDRMON_OK) {
			printf("[ddrmon] Not initialized. Please enable in menuconfig first.\n");
			return -1;
		}

		// Clear ring buffer statistics
		ddrmon_stats_reset();
		printf("[ddrmon] Ring buffer cleared. Statistics reset.\n");
	} else {
		printf("Unknown subcmd\n");
	}
	return 0;
}
ALIOS_CLI_CMD_REGISTER(cmd_ddrmon, ddrmon, CVI DDR bandwidth monitor);
