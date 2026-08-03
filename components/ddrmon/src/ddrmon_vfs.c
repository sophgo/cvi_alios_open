// SPDX-License-Identifier: Apache-2.0

#include <aos/kernel.h>
#include <vfs.h>
#include <vfs_types.h>
#include <errno.h>
#include <stdio.h>
#include "ddrmon_api.h"

// VFS ioctl callback — dispatcher for ddrmon ioctl commands
// Note: AliOS VFS passes 'unsigned long arg' instead of RT-Thread's 'void *args'.
//       The caller passes a pointer cast to unsigned long; we restore it here.
static int ddrmon_vfs_ioctl(vfs_file_t *fp, int cmd, unsigned long arg)
{
	(void)(fp);
	int ret;
	void *args = (void *)(uintptr_t)arg;

	DDRMON_DBG("[ddrmon] devctl cmd=0x%x nr=0x%x\n", cmd, _IOC_NR(cmd));

	if (_IOC_TYPE(cmd) != DDRMON_IOC_MAGIC)
		return -EINVAL;

	switch (_IOC_NR(cmd)) {
	case 0x01: /* DDRMON_IOC_START */
		DDRMON_DBG("[ddrmon] IOCTL START\n");
		ret = ddrmon_start();
		return (ret == DDRMON_OK) ? 0 : -1;

	case 0x02: /* DDRMON_IOC_STOP */
		DDRMON_DBG("[ddrmon] IOCTL STOP\n");
		ret = ddrmon_stop();
		return (ret == DDRMON_OK) ? 0 : -1;

	case 0x03: /* DDRMON_IOC_SET_PERIOD_MS */
		if (!args)
			return -EINVAL;
		DDRMON_DBG("[ddrmon] IOCTL SET_PERIOD_MS %u\n", *(uint32_t *)args);
		ret = ddrmon_set_period(*(uint32_t *)args);
		return (ret == DDRMON_OK) ? 0 : -1;

	case 0x04: /* DDRMON_IOC_GET_INSTANT */
		if (!args)
			return -EINVAL;
		{
			ddrmon_instant_t *req = (ddrmon_instant_t *)args;
			double rd = 0, wr = 0, tot = 0;

			DDRMON_DBG("[ddrmon] IOCTL GET_INSTANT port=%u\n", req->port);

			ret = ddrmon_get_instant(req->port, &rd, &wr, &tot);
			if (ret != DDRMON_OK)
				return (ret == DDRMON_ERR) ? -1 : -EIO;
			req->rd_MBps = rd;
			req->wr_MBps = wr;
			req->tot_MBps = tot;
			return 0;
		}

	case 0x05: /* DDRMON_IOC_GET_STATS */
		if (!args)
			return -EINVAL;
		{
			ddrmon_stats_req_t *req = (ddrmon_stats_req_t *)args;

			DDRMON_DBG("[ddrmon] IOCTL GET_STATS port=%u last_ms=%u\n",
			   req->port, (uint32_t)req->last_ms);
			double min_v = 0, max_v = 0, avg_v = 0;

			ret = ddrmon_get_stats(req->port, req->last_ms, &min_v, &max_v, &avg_v);
			if (ret != DDRMON_OK)
				return (ret == DDRMON_ERR) ? -1 : -EIO;
			req->min_MBps = min_v;
			req->max_MBps = max_v;
			req->avg_MBps = avg_v;
			return 0;
		}

	case 0x06: /* DDRMON_IOC_GET_DDR_INFO */
		if (!args)
			return -EINVAL;
		{
			DDRMON_DBG("[ddrmon] IOCTL GET_DDR_INFO\n");
			ddrmon_info_t *info = (ddrmon_info_t *)args;

			ret = ddrmon_get_info(&info->data_rate_mhz, &info->bus_width_bits);
			return (ret == DDRMON_OK) ? 0 : -1;
		}

	default:
		DDRMON_DBG("[ddrmon] IOCTL unknown nr=0x%x\n", _IOC_NR(cmd));
		return -ENOSYS;
	}
}

static int ddrmon_vfs_open(vfs_inode_t *node, vfs_file_t *fp)
{
	(void)(node);
	(void)(fp);
	return 0;
}

static int ddrmon_vfs_close(vfs_file_t *fp)
{
	(void)(fp);
	return 0;
}

static vfs_file_ops_t ddrmon_vfs_ops = {
	.open  = ddrmon_vfs_open,
	.close = ddrmon_vfs_close,
	.read  = NULL,
	.write = NULL,
	.ioctl = ddrmon_vfs_ioctl,
	.poll  = NULL,
	.lseek = NULL,
};

static int device_registered;

int ddrmon_device_init(void)
{
	printf("[ddrmon] device_init\n");

	// Idempotent: avoid double registration
	if (device_registered) {
		printf("[ddrmon] device already registered\n");
		return 0;
	}

	int ret = aos_register_driver("/dev/ddrmon", &ddrmon_vfs_ops, NULL);
	if (ret != 0) {
		printf("[ddrmon] register VFS driver failed: %d\n", ret);
		return ret;
	}

	device_registered = 1;
	printf("[ddrmon] device registered as /dev/ddrmon\n");
	return 0;
}

int ddrmon_device_deinit(void)
{
	printf("[ddrmon] device_deinit\n");
	if (!device_registered)
		return 0;

	int ret = aos_unregister_driver("/dev/ddrmon");
	if (ret != 0) {
		printf("[ddrmon] unregister VFS driver failed: %d\n", ret);
		return ret;
	}

	device_registered = 0;
	return 0;
}
