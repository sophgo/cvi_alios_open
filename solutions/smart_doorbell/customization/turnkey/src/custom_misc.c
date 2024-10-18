#if CONFIG_QUICK_STARTUP_SUPPORT
#include <drv/tick.h>
#include <aos/cli.h>
#include "pthread.h"
#include "yoc/partition.h"
#include "yoc/partition_device.h"
#include "drv/spiflash.h"
#include "drv/spi.h"
#include <posix/timer.h>
#include <sys/time.h>

extern void PLATFORM_CLK_AXI4_Restore();

int MISC_WaitVideoStep1()
{

	return 0;
}
#endif