#include <stdio.h>
#include <unistd.h>
#include <aos/kernel.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"
#include "rtsp_func.h"
#include "gui_display.h"
#include "app_ai.h"
#include "rtsp_func.h"
#include "ai_facekit.h"

#if CONFIG_USBD_UVC
#define UVCSWITCH_BY_PIN 1
#else
#define UVCSWITCH_BY_PIN 0
#endif

#if CONFIG_TCP_SERVER_SUPPORT
#include "cvi_tcp_server.h"
#include <unistd.h>
#include <pthread.h>
#endif

#if UVCSWITCH_BY_PIN
#include <drv/gpio.h>
#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)
#endif

#if CONFIG_TCP_SERVER_SUPPORT
static int gTerminate = 0;

static void* handle_tcp_server(void *args)
{
	printf("[enter] %s\n", __FUNCTION__);
	cvi_tcp_server_init("192.168.11.10", 9527);

	while (!gTerminate) {
		aos_msleep(3000);
		printf("x tcp server loop\n");
	}
    return (void *)NULL;
}
#endif

#if UVCSWITCH_BY_PIN

extern void uvc_switch(int argc, char** argv);
static void* handle_uvc_server(void *args)
{
	static int gpio_value = 1;
	static int record_value = 1;
	while (1) {
		static csi_gpio_t gpio = {0};
		unsigned int gpio_bank = 0;
		unsigned int gpio_pin = 30;
		int ret = 0;
		ret = csi_gpio_init(&gpio, gpio_bank);
		ret = csi_gpio_dir(&gpio, GPIO_PIN_MASK(gpio_pin), GPIO_DIRECTION_INPUT);
		if(ret != CSI_OK)
		{
			printf("csi_gpio faild %d\n", ret);
		}
		if (csi_gpio_read(&gpio, GPIO_PIN_MASK(gpio_pin) ) != 0U) {
			gpio_value = 1;
		} else {
			gpio_value = 0;
		}
		//printf("read gpio_value : %d \r\n",gpio_value);
		csi_gpio_uninit(&gpio);
		//printf("read gpio_value: %d,  record_value: %d\r\n",gpio_value, record_value);
		if(gpio_value != record_value)
		{
			printf("read gpio_value : %d \r\n",gpio_value);
			if(gpio_value == 1)
			{
				char *args[] = {"uvc_switch", "0", "0", "0", "0"};
				int nargs = sizeof(args) / sizeof(char*);
				uvc_switch(nargs, args);
			}
			else if(gpio_value == 0)
			{
				char *args[] = {"uvc_switch", "0", "1", "1", "0"};
				int nargs = sizeof(args) / sizeof(char*);
				uvc_switch(nargs, args);
			}
			record_value = gpio_value;
		}
		aos_msleep(500);
	}
    return (void *)NULL;
}
#endif

int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);
#if (CONFIG_APP_RTSP_SUPPORT == 1)
    cvi_rtsp_init();
#endif
    cvi_tpu_init();
	Ai_FACEKIT_INIT();
#if CONFIG_TCP_SERVER_SUPPORT
	static pthread_t tcpserver_task;
    if (pthread_create(&tcpserver_task, NULL, handle_tcp_server, NULL)) {
		printf("handle_tcp_server create fail\n");
		return -1;
	}
#endif

#if UVCSWITCH_BY_PIN
	static pthread_t switchuvc_task;
    if (pthread_create(&switchuvc_task, NULL, handle_uvc_server, NULL)) {
		printf("handle_tcp_server create fail\n");
		return -1;
	}
#endif

    return 0;
}
