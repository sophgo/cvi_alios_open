#include <stdbool.h>
#include <aos/kv.h>
#include <debug/dbg.h>
#include <uservice/uservice.h>
#include <yoc/partition.h>
#include <yoc/init.h>
#include <vfs.h>
#include <board.h>
#ifdef CONFIG_FS_EXT4
#include <ext4_vfs.h>
#include <ext4_debug.h>
#endif
#ifdef CONFIG_FS_FAT
#include <fatfs_vfs.h>
#endif
#ifdef CONFIG_FS_LFS
#include <littlefs_vfs.h>
#endif

#include <drv/pin.h>
#include <pinctrl-mars.h>
#include <drv/pin.h>
#include <pinctrl-mars.h>

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

static void _GPIOSetValue(u8 gpio_grp, u8 gpio_num, u8 level)
{
	csi_error_t ret;
	csi_gpio_t gpio = {0};

	ret = csi_gpio_init(&gpio, gpio_grp);
	if(ret != CSI_OK) {
		printf("csi_gpio_init failed\r\n");
		return;
	}
	// gpio write
	ret = csi_gpio_dir(&gpio , GPIO_PIN_MASK(gpio_num), GPIO_DIRECTION_OUTPUT);

	if(ret != CSI_OK) {
		printf("csi_gpio_dir failed\r\n");
		return;
	}
	csi_gpio_write(&gpio , GPIO_PIN_MASK(gpio_num), level);
	//printf("test pin end and success.\r\n");
	csi_gpio_uninit(&gpio);
}

static void _UsbPinmux(void)
{
	// SOC_PORT_SEL
	PINMUX_CONFIG(PWR_GPIO0, PWR_GPIO_0);
	PINMUX_CONFIG(PWR_GPIO1, PWR_GPIO_1);
}

static void _UsbIoInit(void)
{
#if CONFIG_SUPPORT_USB_HC
	_GPIOSetValue(4, 0, 0);
	_GPIOSetValue(4, 1, 0);
#elif CONFIG_SUPPORT_USB_DC
	_GPIOSetValue(4, 0, 1);
	_GPIOSetValue(4, 1, 1);
#endif
}

#define TAG "init"

static void stduart_init(void)
{
    extern void console_init(int idx, uint32_t baud, uint16_t buf_size);
    console_init(CONSOLE_UART_IDX, CONFIG_CLI_USART_BAUD, CONFIG_CONSOLE_UART_BUFSIZE);
}

static void fs_init(void)
{
#ifdef CONFIG_FS_EXT4
    ext4_dmask_set(DEBUG_ALL);
    if (vfs_ext4_register()) {
        LOGE(TAG, "ext4 register failed.");
    } else {
        LOGD(TAG, "ext4 register ok.");
    }
#endif
#ifdef CONFIG_FS_FAT
    if (vfs_fatfs_register()) {
        LOGE(TAG, "fatfs register failed.");
    } else {
        LOGD(TAG, "fatfs register ok.");
    }
#endif
#ifdef CONFIG_FS_LFS
    if (vfs_lfs_register("lfs")) {
        LOGE(TAG, "littlefs register failed.");
    } else {
        LOGD(TAG, "littlefs register ok.");
    }
#endif
}

extern void board_cli_init();
void board_yoc_init(void)
{
    board_init();
    stduart_init();
    printf("###YoC###[%s,%s]\n", __DATE__, __TIME__);
    printf("cpu clock is %dHz\n", soc_get_cpu_freq(0));
    board_cli_init();
    event_service_init(NULL);
    ulog_init();
    aos_set_log_level(AOS_LL_DEBUG);
    int ret = partition_init();
    if (ret <= 0) {
        LOGE(TAG, "partition init failed");
        aos_assert(false);
    }
    ret = aos_vfs_init();
    if (ret < 0) {
        LOGE(TAG, "vfs init failed");
        aos_assert(false);
    }
    fs_init();

	_UsbPinmux();
	_UsbIoInit();
}
