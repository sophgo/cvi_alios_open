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

#define RW_SPEED_TEST
typedef struct {
	uint32_t idx;
	uint32_t offset;
	uint32_t size;
} test_spiflash_args_t;

static struct sched_param param;
static pthread_attr_t pthread_attr;
static pthread_t pthreadId = 0;

static void count_clock_time(struct timespec *start, struct timespec *end, struct timespec *time_gap)
{
        if (end->tv_nsec < start->tv_nsec) {
                time_gap->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
                time_gap->tv_sec = end->tv_sec - start->tv_sec - 1;
        } else {
                time_gap->tv_sec = end->tv_sec - start->tv_sec;
                time_gap->tv_nsec = end->tv_nsec - start->tv_nsec;
        }
}

static void false_division(uint32_t *val, uint64_t dividend, uint64_t divisor, int bit)
{
    int cycle = 1;
    val[0] = dividend / divisor;
    do {
        if (dividend < divisor)
                dividend *= 10;
        else
                dividend = (dividend % divisor) * 10;
        val[cycle] = dividend / divisor;
        cycle++;
    }while(cycle < bit);
}

static void *nor_flash_read_test(void *args)
{
	static int ret = 0;
	uint8_t *rx_data = NULL;
	csi_spiflash_info_t info;
	csi_spiflash_t spiflash_handle;
	struct timespec start, end, time_gap;
	unsigned long long spend_time;
	uint32_t val[4] = {0};
	test_spiflash_args_t spiflash_args;

	// The param for test
	spiflash_args.idx = 0;
	spiflash_args.offset = 0x02E000;
	spiflash_args.size = 0x4B0000;

	printf("nor_flash_read_test start ##cur_ms:%d\n", csi_tick_get_ms());

	csi_spiflash_spi_init(&spiflash_handle, spiflash_args.idx, NULL);
	csi_spiflash_get_flash_info(&spiflash_handle, &info);
	if (spiflash_args.size == 0)
		return &ret;
#if 0
	if (spiflash_args.offset & (info.sector_size - 1)) {
		printf("offset not align to %u \n", info.sector_size);
		ret = -1;
		return &ret;
	}
#endif
	if (spiflash_args.size & (info.sector_size - 1))
		spiflash_args.size = (spiflash_args.size + (info.sector_size - 1)) &~((info.sector_size - 1));
	if ((spiflash_args.offset + spiflash_args.size) > info.flash_size) {
		printf("the range is greater flash size:%#x \n", info.flash_size);
		return &ret;
	}
	rx_data = (uint8_t *)malloc(sizeof(uint8_t) * spiflash_args.size);
	if (rx_data == NULL) {
		printf("malloc rx data buffer failed!\n");
		ret = -1;
		return &ret;
	}
	memset(rx_data, 0xa, sizeof(uint8_t) * spiflash_args.size);
#ifdef  RW_SPEED_TEST
	clock_gettime(CLOCK_REALTIME, &start);
#endif
	printf("[%s %d]===> read data......\n", __func__, __LINE__);
	ret = csi_spiflash_read(&spiflash_handle, spiflash_args.offset, rx_data, spiflash_args.size);
	if (ret < 0) {
		printf("nor flash read failed, ret:%d \n", ret);
		goto free;
	}
#ifdef  RW_SPEED_TEST
    printf("nor_flash_read_test end ##cur_ms:%d\n", csi_tick_get_ms());
	clock_gettime(CLOCK_REALTIME, &end);
	count_clock_time(&start, &end, &time_gap);
	spend_time = time_gap.tv_sec * 1000000000 + time_gap.tv_nsec;
	printf("read spend time %llu ns ,data size:%#x byte\n", spend_time, spiflash_args.size);
	false_division(val, 0x4B0000, spend_time / 1000, 4);
	printf("read speed is %u.%u%u%u MB/s\n", val[0], val[1], val[2], val[3]);
#endif
free:
	free(rx_data);
	csi_spiflash_spi_uninit(&spiflash_handle);
	return &ret;
}

#if 0
void test_flash_read(int argc,char **argv)
{
    struct sched_param param;
    param.sched_priority = 30;
    pthread_attr_t pthread_attr;
	pthread_t pthreadId = 0;
    pthread_attr_init(&pthread_attr);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
	pthread_attr_setschedparam(&pthread_attr, &param);
	pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&pthread_attr, 6*1024);
	pthread_create(&pthreadId,&pthread_attr,nor_flash_read_test,NULL);
}
ALIOS_CLI_CMD_REGISTER(test_flash_read,test_flash_read,test_flash_read);
#endif

extern void PLATFORM_CLK_AXI4_Restore();

void MISC_SimulateReadAiModel()
{
    param.sched_priority = 30;
    pthread_attr_init(&pthread_attr);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
	pthread_attr_setschedparam(&pthread_attr, &param);
	pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&pthread_attr, 6 * 1024);
	pthread_create(&pthreadId,&pthread_attr,nor_flash_read_test,NULL);
}

int MISC_WaitVideoStep1()
{
	static int already_startup = 0;

	if (already_startup != 0) {
		return 0;
	}
	already_startup = 1;

    pthread_join(pthreadId, NULL);
    PLATFORM_CLK_AXI4_Restore();
    printf("%s done:%u ms\n", __FUNCTION__, csi_tick_get_ms());
	return 0;
}
#endif