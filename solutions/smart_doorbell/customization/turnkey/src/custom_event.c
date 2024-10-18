#include "custom_event.h"
#include <aos/kernel.h>
#include <drv/gpio.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "app_ai.h"
#include "cvi_tpu_interface.h"
#include "gui_display.h"
#include "rtsp_func.h"
#include "usbd_uvc.h"

#if (CONFIG_ALGOKIT_ENABLE == 1)
#include "cvi_algokit.h"
#endif

#if CONFIG_TCP_SERVER_SUPPORT
#include <pthread.h>
#include "cvi_tcp_server.h"
#endif

#if CONFIG_UVC_SWITCH_BY_PIN
#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)
#endif

#if CONFIG_TCP_SERVER_SUPPORT
static int32_t gTerminate = 0;

static void* cvi_handle_tcp_server(void* pc_args)
{
    printf("[enter] %s\n", __FUNCTION__);
    cvi_tcp_server_init("192.168.11.10", 9527);

    while (!gTerminate) {
        aos_msleep(3000);
    }
    return NULL;
}
#endif


static void* cvi_handle_uvc_server(void* p_args)
{
    int32_t s32_ret;
    static uint32_t u32_gpio_val   = 1;
    static uint32_t u32_last_gpio_val = 1;
    while (1) {
        static csi_gpio_t st_gpio = {0};
        uint32_t u32_gpio_bank = 0;
        uint32_t u32_gpio_pin  = 30;

        s32_ret = csi_gpio_init(&st_gpio, u32_gpio_bank);
        if (s32_ret != CSI_OK) {
            printf("csi_gpio_init FAIL %d\n", s32_ret);
        }
        s32_ret = csi_gpio_dir(&st_gpio, GPIO_PIN_MASK(u32_gpio_pin), GPIO_DIRECTION_INPUT);
        if (s32_ret != CSI_OK) {
            printf("csi_gpio_dir FAIL %d\n", s32_ret);
        }

        u32_gpio_val = csi_gpio_read(&st_gpio, GPIO_PIN_MASK(u32_gpio_pin));

        csi_gpio_uninit(&st_gpio);

        if (u32_gpio_val != u32_last_gpio_val) {
            printf("read u32_gpio_val : %d\n", u32_gpio_val);
            if (u32_gpio_val != 0) {
                char* pc_args[] = {"uvc_switch", "0", "0", "0", "0"};
                uvc_switch(5, pc_args);
            } else {
                char* pc_args[] = {"uvc_switch", "0", "0", "1", "0"};
                uvc_switch(5, pc_args);
            }
            u32_last_gpio_val = u32_gpio_val;
        }
        aos_msleep(500);
    }
    return NULL;
}

#if (CONFIG_ALGOKIT_ENABLE)

static void* cvi_algokit_handler(void* args)
{
    struct timeval start, end;

    gettimeofday(&start, NULL);

    cvi_tpu_init();
    cvi_algokit_init();

    gettimeofday(&end, NULL);
    printf("algokit init spend time %f ms\n",
           (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0);

    pthread_exit(NULL);
    return NULL;
}
#endif


int32_t APP_CustomEventStart(void)
{
#if (CONFIG_APP_RTSP_SUPPORT)
    cvi_rtsp_init();
#endif

#if (CONFIG_ALGOKIT_ENABLE)
    pthread_attr_t st_thread_attr;
    pthread_t st_algokit_handler_tid;

    pthread_attr_init(&st_thread_attr);
    pthread_attr_setstacksize(&st_thread_attr, 81920);
    pthread_create(&st_algokit_handler_tid, &st_thread_attr, cvi_algokit_handler, NULL);
    pthread_setname_np(st_algokit_handler_tid, "ALGOKIT");
#endif

#if CONFIG_TCP_SERVER_SUPPORT
    static pthread_t st_tcp_server_tid;
    if (pthread_create(&st_tcp_server_tid, NULL, cvi_handle_tcp_server, NULL)) {
        printf("cvi_handle_tcp_server create fail\n");
        return -1;
    }
#endif

#if CONFIG_UVC_SWITCH_BY_PIN
    static pthread_t st_switch_uvc_tid;
    if (pthread_create(&st_switch_uvc_tid, NULL, cvi_handle_uvc_server, NULL)) {
        printf("cvi_handle_tcp_server create fail\n");
        return -1;
    }
#endif

    return 0;
}
