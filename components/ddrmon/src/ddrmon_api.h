/* SPDX-License-Identifier: Apache-2.0 */

#ifndef CVI_DDRMON_API_H_
#define CVI_DDRMON_API_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DDRMON custom error codes (OS-independent) ---- */
#define DDRMON_OK      0
#define DDRMON_ERR     (-1)
#define DDRMON_EBUSY   (-2)
#define DDRMON_EEMPTY  (-3)
#define DDRMON_EINVAL  (-4)

#ifdef CVI_DDRMON_DEBUG
#define DDRMON_DBG(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
#define DDRMON_DBG(fmt, ...)  ((void)0)
#endif

#define DDRMON_NULL     NULL
#define DDRMON_UNUSED(x) (void)(x)

/* ---- Aligned attribute (GCC compatible) ---- */
#ifndef __aligned
#define __aligned(N)   __attribute__((aligned(N)))
#endif

typedef struct {
	uintptr_t aximon_base;      // AXI MON base address (MMIO)
	uintptr_t ddr_ctrl_base;    // DDR controller base (optional)
	uintptr_t ddr_top_base;     // DDR top base (optional clock gate)
	const char *soc_type;       // "cv180x" or "cv181x"
	uint32_t default_period_ms; // sampling period
	uint32_t port_mask;         // bit0~bit5 for M1~M6 (always set to 0x3F to enable all ports)
	uint32_t data_rate_mhz;     // temporary data rate until HW reg is available
} ddrmon_params_t;

// ===== ioctl interface =====
// Provide minimal _IO/_IOR/_IOW/_IOWR macros (OS-independent).
// Guarded by #ifndef to avoid conflict if platform already defines them
// (e.g. Linux headers or AliOS VFS sys/ioctl.h). AliOS VFS passes cmd and
// arg through to vfs_file_ops_t::ioctl untouched, so these encoding macros
// are compatible as long as IOC_MAGIC ('d') doesn't collide with other drivers.
#ifndef _IOC_NRBITS
#define _IOC_NRBITS     8
#define _IOC_TYPEBITS   8
#define _IOC_SIZEBITS   14
#define _IOC_DIRBITS    2

#define _IOC_NRMASK     ((1 << _IOC_NRBITS)-1)
#define _IOC_TYPEMASK   ((1 << _IOC_TYPEBITS)-1)
#define _IOC_SIZEMASK   ((1 << _IOC_SIZEBITS)-1)
#define _IOC_DIRMASK    ((1 << _IOC_DIRBITS)-1)

#define _IOC_NRSHIFT    0
#define _IOC_TYPESHIFT  (_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT  (_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT   (_IOC_SIZESHIFT+_IOC_SIZEBITS)

#define _IOC_NONE       0U
#define _IOC_WRITE      1U
#define _IOC_READ       2U

#define _IOC(dir, type, nr, size) \
	(((dir)  << _IOC_DIRSHIFT)  | \
	 ((type) << _IOC_TYPESHIFT) | \
	 ((nr)   << _IOC_NRSHIFT)   | \
	 ((size) << _IOC_SIZESHIFT))

#define _IO(type, nr)            _IOC(_IOC_NONE, (type), (nr), 0)
#define _IOR(type, nr, dt)        _IOC(_IOC_READ, (type), (nr), sizeof(dt))
#define _IOW(type, nr, dt)        _IOC(_IOC_WRITE, (type), (nr), sizeof(dt))
#define _IOWR(type, nr, dt)       _IOC(_IOC_READ|_IOC_WRITE, (type), (nr), sizeof(dt))
#endif

#ifndef _IOC_NR
#define _IOC_NR(cmd)            ((cmd) & _IOC_NRMASK)
#endif

#ifndef _IOC_TYPE
#define _IOC_TYPE(cmd)          (((cmd) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
#endif

#define DDRMON_IOC_MAGIC    ('d')

// Payload types for ioctl
// Note: Structures use natural alignment for embedded compatibility
typedef struct {
	uint32_t port;         // input: 0..5 for port, or 0xFF for aggregate
	double   rd_MBps;      // output
	double   wr_MBps;      // output
	double   tot_MBps;     // output
} __aligned(8) ddrmon_instant_t;

typedef struct {
	uint32_t port;         // input (currently ignored in stats backend; kept for extensibility)
	uint64_t last_ms;      // input: time window in ms
	double   min_MBps;     // output
	double   max_MBps;     // output
	double   avg_MBps;     // output
} __aligned(8) ddrmon_stats_req_t;

typedef struct {
	uint32_t data_rate_mhz;    // output
	uint32_t bus_width_bits;   // output
} ddrmon_info_t;

// Special port value for aggregated bandwidth
#define DDRMON_PORT_AGGR  (0xFFu)

// ioctl command IDs
#define DDRMON_IOC_START          _IOW(DDRMON_IOC_MAGIC, 0x01, uint32_t)
#define DDRMON_IOC_STOP           _IOW(DDRMON_IOC_MAGIC, 0x02, uint32_t)
#define DDRMON_IOC_SET_PERIOD_MS  _IOW(DDRMON_IOC_MAGIC, 0x03, uint32_t)
#define DDRMON_IOC_GET_INSTANT    _IOWR(DDRMON_IOC_MAGIC, 0x04, ddrmon_instant_t)
#define DDRMON_IOC_GET_STATS      _IOWR(DDRMON_IOC_MAGIC, 0x05, ddrmon_stats_req_t)
#define DDRMON_IOC_GET_DDR_INFO   _IOR(DDRMON_IOC_MAGIC, 0x06, ddrmon_info_t)

/**
 * @brief Initialize DDR monitor module
 * @param p Configuration parameters (base addresses, period, port mask, etc.)
 * @return DDRMON_OK on success, DDRMON_ERR if already initialized or mutex creation failed
 * @note Idempotent: safe to call multiple times
 */
int ddrmon_init(const ddrmon_params_t *p);

/**
 * @brief Cleanup DDR monitor resources
 * @return DDRMON_OK on success
 * @note Stops sampling thread and releases all resources (mutex, device, etc.)
 */
int ddrmon_deinit(void);

/**
 * @brief Start bandwidth sampling thread
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized or thread creation failed,
 *         DDRMON_EBUSY if already running (must stop first)
 */
int ddrmon_start(void);

/**
 * @brief Stop bandwidth sampling thread
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized,
 *         DDRMON_EBUSY if not running (must start first)
 * @note Waits for thread to exit and cleans up thread handle
 */
int ddrmon_stop(void);

/**
 * @brief Set sampling period
 * @param ms Period in milliseconds (0 defaults to 20ms, max 10000ms)
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized, DDRMON_EINVAL if out of range
 */
int ddrmon_set_period(uint32_t ms);

/**
 * @brief Get current sampling period
 * @return Current sampling period in milliseconds
 */
uint32_t ddrmon_get_period_ms(void);

/**
 * @brief Get instantaneous bandwidth
 * @param port Port number (0..5) or 0xFF/0xFFFFFFFF for aggregated bandwidth
 * @param rd_MBps Output: read bandwidth in MB/s (can be NULL)
 * @param wr_MBps Output: write bandwidth in MB/s (can be NULL)
 * @param tot_MBps Output: total bandwidth in MB/s (can be NULL)
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized, -1 if port invalid, -2 if port disabled
 */
int ddrmon_get_instant(uint32_t port, double *rd_MBps, double *wr_MBps, double *tot_MBps);

/**
 * @brief Get bandwidth statistics over time window
 * @param port Port number (currently unused, stats track aggregate)
 * @param last_ms Time window in milliseconds (0 = all available data)
 * @param min_MBps Output: minimum total bandwidth in window (can be NULL)
 * @param max_MBps Output: maximum total bandwidth in window (can be NULL)
 * @param avg_MBps Output: average total bandwidth in window (can be NULL)
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized, -1 if no data, -2 if no samples in window
 * @note IMPORTANT: Statistics are limited by ring buffer capacity (default 16384 samples).
 *       Max retention window = buffer_capacity * sampling_period (e.g., 16384 * 20ms = 327.68s).
 *       When buffer is full, oldest samples are overwritten, so statistics reflect only
 *       the most recent samples within the retention window, NOT the entire duration since start.
 */
int ddrmon_get_stats(uint32_t port, uint64_t last_ms,
					 double *min_MBps, double *max_MBps, double *avg_MBps);

/**
 * @brief Get bandwidth statistics over time window (extended with sample count)
 * @param last_ms Time window in milliseconds (0 = all available data)
 * @param min_MBps Output: minimum total bandwidth in window (can be NULL)
 * @param max_MBps Output: maximum total bandwidth in window (can be NULL)
 * @param avg_MBps Output: average total bandwidth in window (can be NULL)
 * @param sample_count Output: number of samples used in calculation (can be NULL)
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized, -1 if no data, -2 if no samples in window
 * @note IMPORTANT: Statistics are limited by ring buffer capacity (default 16384 samples).
 *       Max retention window = buffer_capacity * sampling_period.
 */
int ddrmon_stats_query_ex(uint64_t last_ms, double *min_MBps, double *max_MBps,
						  double *avg_MBps, uint32_t *sample_count);

/**
 * @brief Get DDR hardware information
 * @param data_rate_mhz Output: DDR data rate in MHz (can be NULL)
 * @param bus_width_bits Output: DDR bus width in bits (can be NULL)
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized
 */
int ddrmon_get_info(uint32_t *data_rate_mhz, uint32_t *bus_width_bits);

/**
 * @brief Get monitoring runtime duration
 * @param runtime_ms Output: elapsed time in milliseconds since start (can be NULL)
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized, DDRMON_EEMPTY if never started
 */
int ddrmon_get_runtime(uint64_t *runtime_ms);

/**
 * @brief Set auto-log interval
 * @param interval_sec Log interval in seconds (0 to disable auto-logging)
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized
 * @note When enabled, bandwidth stats are automatically printed to console periodically
 */
int ddrmon_set_log_interval(uint32_t interval_sec);

/**
 * @brief Get current auto-log interval
 * @return Current log interval in seconds (0 if disabled)
 */
uint32_t ddrmon_get_log_interval(void);

/**
 * @brief Get statistics ring buffer status
 * @param total_samples Output: number of valid samples in ring (can be NULL)
 * @param ring_capacity Output: maximum ring buffer capacity (can be NULL)
 * @return DDRMON_OK on success, DDRMON_ERR if not initialized
 */
int ddrmon_get_ring_status(uint32_t *total_samples, uint32_t *ring_capacity);

/**
 * @brief Initialize DDR monitor component (call from solution app_main)
 * @return DDRMON_OK on success, error code on failure
 * @note Replaces RT-Thread INIT_APP_EXPORT; must be called explicitly at startup
 */
int ddrmon_component_init(void);

/**
 * @brief Register /dev/ddrmon VFS device (internal, called by component init)
 * @return 0 on success, error code on failure
 * @note Uses AliOS VFS vfs_register_driver instead of RT-Thread rt_device_register
 * @note Idempotent: safe to call multiple times
 */
int ddrmon_device_init(void);

/**
 * @brief Unregister /dev/ddrmon VFS device
 * @return 0 on success, error code on failure
 */
int ddrmon_device_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // CVI_DDRMON_API_H_
