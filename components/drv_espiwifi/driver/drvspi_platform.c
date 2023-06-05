#include "drvspi_platform.h"
#include "platform_os.h"
#include "drv/pin.h"
#include "drv/gpio.h"
#include "cvi_gpio.h"
#include "drv/spi.h"
#include "pinctrl-mars.h"
#include "drv/irq.h"
#include <drv/tick.h>
#include <unistd.h> //for usleep

//#define SPI_CS_GPIO

typedef struct {
    csi_gpio_t spicsio;
    csi_spi_t spitrshandle;
    csi_gpio_t gpio;
    drvspi_platform_irq_call irqcall;
    aos_mutex_t spi_trans_mutex;
} drv_spi_platform_priv_t;

drv_spi_platform_priv_t g_drvspi_priv;

static drv_spi_platform_priv_t *drvspi_platform_get_priv(void)
{
    return &g_drvspi_priv;
}

int drv_spi_platform_gpio_value(int gpionum)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    int gpioval = csi_gpio_read(&priv->gpio, 1 << gpionum);

    return (gpioval >> gpionum);
}


static int drv_spi_platform_callback(csi_spi_t *spi, csi_spi_event_t event, void *arg)
{
#ifdef SPI_CS_GPIO
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
#endif

    switch (event) {
        case SPI_EVENT_SEND_COMPLETE:
        //    *((unsigned char *)arg) = 0;
            break;

        case SPI_EVENT_RECEIVE_COMPLETE:
			*((unsigned char *)arg) = 0;
            break;

        case SPI_EVENT_SEND_RECEIVE_COMPLETE:
            *((unsigned char *)arg) = 0;
            break;

        default:
            // platform_log_e("%s[%d] unkown event %d\n", __FUNCTION__, __LINE__, event);
            return -1;
    }

#ifdef SPI_CS_GPIO
    csi_gpio_write(&priv->spicsio, (1 << DRVSPI_PLATFORM_CS_NUM), 1);
#endif
    return 0;
}


int drv_spi_platfrom_read(unsigned char *rbuff, unsigned int len)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    unsigned char *syncbuff;
    volatile unsigned char spiflag;
    csi_error_t ret = 0;
    uint32_t timestart = 0;

    /* 判断mutex是否有效 */
    ret = aos_mutex_is_valid(&priv->spi_trans_mutex);
    if (ret != 1) {
        platform_log_e("%s[%d] spi_trans_mutex no valid!\n", __FUNCTION__, __LINE__);
        return -1;
    }
    /* 请求互斥信号量，timeout为1000ms */
    ret = aos_mutex_lock(&priv->spi_trans_mutex, 1000);
    if (ret != 0) {
        platform_log_e("%s[%d] spi_trans_mutex lock failed!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    spiflag = 1;

    syncbuff = platform_memory_alloc(len);
    if (syncbuff == NULL) {
        platform_log_e("%s[%d] alloc sync buff failed\n", __FUNCTION__, __LINE__);
		aos_mutex_unlock(&priv->spi_trans_mutex);
		//platform_memory_free(syncbuff);
        return -1;
    }

    ret = csi_spi_attach_callback(&priv->spitrshandle, drv_spi_platform_callback, (void *)&spiflag);

#ifdef SPI_CS_GPIO
    csi_gpio_write(&priv->spicsio, (1 << DRVSPI_PLATFORM_CS_NUM), 0);
#endif

    // ret = csi_spi_send_receive_dma(&priv->spitrshandle, rbuff, syncbuff, len);
   ret = csi_spi_send_receive_async(&priv->spitrshandle, rbuff, syncbuff, len);
    if (ret != 0) {
        platform_log_e("%s[%d] spi sync failed ret %d\n", __FUNCTION__, __LINE__, ret);
		platform_memory_free(syncbuff);
		aos_mutex_unlock(&priv->spi_trans_mutex);
        return -1;
    }
    // platform_msleep(1);
    timestart = csi_tick_get();
	while (spiflag) {
        if ((csi_tick_get() - timestart) > 1000) {
            platform_log_e("%s[%d] csi_tick_get out time! spiflag = %d\n", __FUNCTION__, __LINE__,spiflag);
            ret = -1;
            platform_msleep(1);
            break;
        }
    }

    platform_memory_set(rbuff, 0, len);
    memcpy(rbuff, syncbuff, len);
	platform_memory_free(syncbuff);

    /* 释放互斥信号量 */
    aos_mutex_unlock(&priv->spi_trans_mutex);

    return ret;
}

int drv_spi_platfrom_write(unsigned char *wbuff, unsigned int len)
{
    return drv_spi_platfrom_read(wbuff, len);
}

static int drv_spi_platform_pinmux_init(void)
{
#ifdef SPI_CS_GPIO
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    int ret;
#endif

    PINMUX_CONFIG(SD1_CLK, SPI2_SCK);
    PINMUX_CONFIG(SD1_CMD, SPI2_SDO);
    PINMUX_CONFIG(SD1_D0, SPI2_SDI);
#ifdef SPI_CS_GPIO
    PINMUX_CONFIG(SD1_D3, PWR_GPIO_18);

    ret = csi_gpio_init(&priv->spicsio, 4);
    if (ret != CSI_OK) {
        platform_log_e("%s[%d] csi_gpio_init failed\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ret = csi_gpio_dir(&priv->spicsio, (1 << DRVSPI_PLATFORM_CS_NUM), GPIO_DIRECTION_OUTPUT);
    if (ret != CSI_OK) {
        platform_log_e("%s[%d] csi_gpio_dir failed\n", __FUNCTION__, __LINE__);
        return -1;
    }
    csi_gpio_write(&priv->spicsio, (1 << DRVSPI_PLATFORM_CS_NUM), 1);
#else
    PINMUX_CONFIG(SD1_D3, SPI2_CS_X);
#endif

    return 0;
}

void drv_spi_platform_irq_enable(int irqnum)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    int ret = csi_gpio_irq_enable(&priv->gpio, 1 << irqnum, true);

    if (ret != CSI_OK) {
        platform_log_e("%s[%d] csi_gpio_irq_enable failed\n", __FUNCTION__, __LINE__);
    }
}

void drv_spi_platform_irq_disable(int irqnum)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    int ret = csi_gpio_irq_enable(&priv->gpio, 1 << irqnum, false);

    if (ret != CSI_OK) {
        platform_log_e("%s[%d] csi_gpio_irq_disable failed\n", __FUNCTION__, __LINE__);
    }
}

static void drv_spi_platform_irq_handle(cvi_gpio_t *gpio, unsigned int pin, void *args)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();

    priv->irqcall(gpio->args);
}

int drv_spi_platform_irq_req(char *irqname, int irqnum, int irqmode, drvspi_platform_irq_call irqcall, void *args)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    int ret;

    priv->irqcall = irqcall;
    PINMUX_CONFIG(SD1_D2, PWR_GPIO_19);
    ret = csi_gpio_init(&(priv->gpio), 4);
    if (ret != CSI_OK) {
        platform_log_e("csi_gpio_init failed\n");
        return -1;
    }

    ret = csi_gpio_mode(&(priv->gpio), (1 << DRVSPI_PLATFORM_IRQ_NUM), GPIO_MODE_PULLDOWN);
    if (ret != CSI_OK) {
        platform_log_e("csi_gpio_mode failed\n");
        return -1;
    }

    ret = csi_gpio_dir(&(priv->gpio), (1 << DRVSPI_PLATFORM_IRQ_NUM), GPIO_DIRECTION_INPUT);
    if (ret != CSI_OK) {
        platform_log_e("csi_gpio_dir failed\n");
        return -2;
    }

    ret = csi_gpio_irq_mode(&(priv->gpio), (1 << irqnum), irqmode);
    if (ret != CSI_OK) {
        platform_log_e("csi_gpio_irq_mode failed\n");
        return -2;
    }

    ret = csi_gpio_attach_callback(&(priv->gpio), drv_spi_platform_irq_handle, args);
    if (ret != CSI_OK) {
        platform_log_e("csi_gpio_attach_callback failed\n");
        return -2;
    }

    ret = csi_gpio_irq_enable(&(priv->gpio), (1 << irqnum), true);
    if (ret != CSI_OK) {
        platform_log_e("csi_gpio_irq_enable failed\n");
        return -2;
    }

    return 0;
}

static int drvspi_platform_driver_init(void)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    int ret;

    ret = csi_spi_init(&priv->spitrshandle, 2);
    if (ret != CSI_OK) {
        platform_log_e("spi init failed, ret %d\n", ret);
        return -1;
    }

    ret = csi_spi_mode(&priv->spitrshandle, SPI_MASTER);
    if (ret != CSI_OK) {
        platform_log_e("set spi mode failed, ret %d\n", ret);
        return -1;
    }

    ret = csi_spi_cp_format(&priv->spitrshandle, SPI_FORMAT_CPOL0_CPHA0);
    if (ret != CSI_OK) {
        platform_log_e("set spi format failed, ret %d\n", ret);
        return -1;
    }

    ret = csi_spi_frame_len(&priv->spitrshandle, SPI_FRAME_LEN_8);
    if (ret != CSI_OK) {
        platform_log_e("set spi frame len failed, ret %d\n", ret);
        return -1;
    }

    csi_spi_baud(&priv->spitrshandle, 10000000);

	//  ret = csi_spi_link_dma(&priv->spitrshandle, NULL, NULL);
	//  if (ret != 0) {
	//     platform_log_e("%s[%d] spi dma failed ret %d\n", __FUNCTION__, __LINE__, ret);
	//     return -1;
	//   }

    return ret;
}

int drvspi_platform_init(void)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    int ret;

    platform_memory_set(priv, 0, sizeof(drv_spi_platform_priv_t));
    ret = drv_spi_platform_pinmux_init();
    if (ret != CSI_OK) {
        return -1;
    }

    ret = aos_mutex_new(&priv->spi_trans_mutex);
    if (ret != CSI_OK) {
        platform_log_e("%s[%d] spi_trans_mutex create failed!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ret = drvspi_platform_driver_init();
    if (ret != CSI_OK) {
        return -1;
    }

    return 0;
}

void drvspi_platform_exit(void)
{
    drv_spi_platform_priv_t *priv = drvspi_platform_get_priv();
    platform_memory_set(priv, 0, sizeof(drv_spi_platform_priv_t));
}

