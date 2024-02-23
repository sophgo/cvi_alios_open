#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aos/aos.h>
#include <aos/ringbuffer.h>
#include "devices/impl/uart_impl.h"

#include <aos/cli.h>
#include <ulog/ulog.h>

#include "usbd_core.h"
#include "usbd_cdc.h"

#include "usbd_descriptor.h"
#include "usbd_cdc_uart.h"
#include "usbd_cdc_urat_descriptor.h"
#include "usbd_comp.h"


#define CDC_MAX_RW_LEN (CDC_UART_MPS)

/*!< uart cfg */
#define EVENT_WRITE  0x0F0F0000
#define EVENT_READ   0x00000F0F
#define UART_RB_SIZE 4096

#define uart(dev) ((cdc_uart_dev_t *)dev)
#define usb_serial_uart_uninit rvm_hal_device_free

typedef struct {
    rvm_dev_t device;
    aos_event_t event_write_read;
    void (*write_event)(rvm_dev_t *dev, int event_id, void *priv);
    void *priv;
    int type;
    char *recv_buf;
    uint8_t *cdc_tx_buffer;
    uint8_t *cdc_rx_buffer;
    dev_ringbuf_t read_buffer;
} cdc_uart_dev_t;

static cdc_uart_dev_t *s_uart_ctx = NULL;
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t s_cdc_rx_buffer[CDC_MAX_RW_LEN];

static struct cdc_uart_device_info cdc_uart_info;
static uint8_t *cdc_uart_descriptor = NULL;

static cdc_uart_dev_t *dev_ctx()
{
    if (!s_uart_ctx) {
        USB_LOG_INFO("cdc ctx is null\n");
    }
    return s_uart_ctx;
}

static rvm_dev_t *usb_serial_uart_init(driver_t *drv, void *config, int id)
{
    s_uart_ctx = (cdc_uart_dev_t *)rvm_hal_device_new(drv, sizeof(cdc_uart_dev_t), id);

    return (rvm_dev_t *)s_uart_ctx;
}

static int usb_serial_uart_open(rvm_dev_t *dev)
{
    if (aos_event_new(&uart(dev)->event_write_read, 0) != 0) {
        return -1;
    }

    uart(dev)->recv_buf = (char *)aos_malloc(UART_RB_SIZE);

    if (uart(dev)->recv_buf == NULL) {
        goto error1;
    }

    uart(dev)->cdc_tx_buffer = usb_iomalloc(CDC_MAX_RW_LEN);
    if (uart(dev)->cdc_tx_buffer == NULL) {
        goto error2;
    }

    uart(dev)->cdc_rx_buffer = s_cdc_rx_buffer;
    if (uart(dev)->cdc_rx_buffer == NULL) {
        goto error3;
    }

    ringbuffer_create(&uart(dev)->read_buffer, uart(dev)->recv_buf, UART_RB_SIZE);

    return 0;

error3:
    aos_free(uart(dev)->cdc_tx_buffer);
error2:
    aos_free(uart(dev)->recv_buf);
error1:
    aos_event_free(&uart(dev)->event_write_read);

    return -1;
}

static int usb_serial_uart_close(rvm_dev_t *dev)
{
    aos_event_free(&uart(dev)->event_write_read);
    usb_iofree(uart(dev)->cdc_tx_buffer);
    aos_free(uart(dev)->recv_buf);
    return 0;
}

static int usb_serial_uart_config(rvm_dev_t *dev, rvm_hal_uart_config_t *config)
{
    return 0;
}

static int usb_serial_uart_set_type(rvm_dev_t *dev, int type)
{
    uart(dev)->type = type;

    return 0;
}

static int usb_serial_uart_set_buffer_size(rvm_dev_t *dev, uint32_t size)
{
    return 0;
}

static int usb_serial_uart_send(rvm_dev_t *dev, const void *data, uint32_t size, uint32_t timeout_ms)
{
    unsigned int actl_flags;
    int ret;
    long long time_enter, used_time;

    if (size > CDC_MAX_RW_LEN) {
        size = CDC_MAX_RW_LEN;
        USB_LOG_INFO("The size of a single send cannot exceed CDC_MAX_RW_LEN \n");
    }

    time_enter = aos_now_ms();

    memcpy(uart(dev)->cdc_tx_buffer, data, size);

    ret = usbd_ep_start_write(cdc_uart_info.cdc_uart_in_ep.ep_addr, uart(dev)->cdc_tx_buffer, size);

    while (1) {
        if (timeout_ms == 0 || timeout_ms <= (used_time = aos_now_ms() - time_enter)) {
            break;
        }

        if (aos_event_get(&uart(dev)->event_write_read, EVENT_WRITE, AOS_EVENT_OR_CLEAR, &actl_flags,
                          timeout_ms - used_time) == -1) {
            break;
        }
    }

    return ret;
}

static int usb_serial_uart_recv(rvm_dev_t *dev, void *data, uint32_t size, unsigned int timeout_ms)
{
    unsigned int actl_flags;
    int ret = 0;
    long long time_enter, used_time;
    void *temp_buf   = data;
    uint32_t temp_count = size;

    time_enter = aos_now_ms();

    while (1) {
        ret = ringbuffer_read(&uart(dev)->read_buffer, (uint8_t *)temp_buf, temp_count);

        temp_count = temp_count - ret;
        temp_buf   = (uint8_t *)temp_buf + ret;

        if (temp_count == 0 || timeout_ms == 0 || timeout_ms <= (used_time = aos_now_ms() - time_enter)) {
            break;
        }

        if (aos_event_get(&uart(dev)->event_write_read, EVENT_READ, AOS_EVENT_OR_CLEAR, &actl_flags,
                          timeout_ms - used_time) == -1) {
            break;
        }
    }

    return size - temp_count;
}

static void usb_serial_uart_event(rvm_dev_t *dev, void (*event)(rvm_dev_t *dev, int event_id, void *priv), void *priv)
{
    uart(dev)->priv = priv;
    uart(dev)->write_event = event;
}

static uart_driver_t usb_serial_uart_driver = {
    .drv = {
        .name   = "usb_serial",
        .init   = usb_serial_uart_init,
        .uninit = usb_serial_uart_uninit,
        .open   = usb_serial_uart_open,
        .close  = usb_serial_uart_close,
    },
    .config          = usb_serial_uart_config,
    .set_type        = usb_serial_uart_set_type,
    .set_buffer_size = usb_serial_uart_set_buffer_size,
    .send            = usb_serial_uart_send,
    .recv            = usb_serial_uart_recv,
    .set_event       = usb_serial_uart_event,
};


static void drv_cdc_acm_uart_register(void)
{
    rvm_driver_register(&usb_serial_uart_driver.drv, NULL, 0);
}

static void drv_cdc_acm_uart_unregister(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%s%d", usb_serial_uart_driver.drv.name, 0);
    rvm_driver_unregister(buf);
}



static void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
    // USB_LOG_RAW("actual out len:%d\r\n", nbytes);
    if (ep != cdc_uart_info.cdc_uart_out_ep.ep_addr) {
        USB_LOG_ERR("ep invaild, %#x, %#x\n", ep, cdc_uart_info.cdc_uart_out_ep.ep_addr);
        return;
    }
    ringbuffer_write(&dev_ctx()->read_buffer, dev_ctx()->cdc_rx_buffer, nbytes);
    aos_event_set(&dev_ctx()->event_write_read, EVENT_READ, AOS_EVENT_OR);

    /* setup next out ep read transfer */
    usbd_ep_start_read(ep, dev_ctx()->cdc_rx_buffer, CDC_MAX_RW_LEN);
}

static void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes)
{
    // USB_LOG_RAW("actual in len:%d\r\n", nbytes);
    if (ep != cdc_uart_info.cdc_uart_in_ep.ep_addr) {
        USB_LOG_ERR("ep invaild, %#x, %#x\n", ep, cdc_uart_info.cdc_uart_in_ep.ep_addr);
        return;
    }
    if ((nbytes % CDC_UART_MPS) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(ep, NULL, 0);
    } else {
        aos_event_set(&dev_ctx()->event_write_read, EVENT_WRITE, AOS_EVENT_OR);
    }
}

static void cdc_uart_desc_register_cb()
{
	cdc_uart_destroy_descriptor(cdc_uart_descriptor);
}

static void cdc_uart_configure_done_callback(void)
{
    /* setup first out ep read transfer */
    usbd_ep_start_read(cdc_uart_info.cdc_uart_out_ep.ep_addr, s_cdc_rx_buffer, CDC_MAX_RW_LEN);
}

void cdc_uart_init(void)
{
    uint32_t desc_len;

    cdc_uart_info.cdc_uart_out_ep.ep_cb = usbd_cdc_acm_bulk_out;
    cdc_uart_info.cdc_uart_out_ep.ep_addr = comp_get_available_ep(0);
    cdc_uart_info.cdc_uart_in_ep.ep_cb = usbd_cdc_acm_bulk_in;
    cdc_uart_info.cdc_uart_in_ep.ep_addr = comp_get_available_ep(1);
    cdc_uart_info.cdc_uart_int_ep.ep_addr = comp_get_available_ep(1);
    cdc_uart_info.interface_nums = comp_get_interfaces_num();
    USB_LOG_INFO("cdc_uart out ep:%#x\n", cdc_uart_info.cdc_uart_out_ep.ep_addr);
    USB_LOG_INFO("cdc_uart in ep:%#x\n", cdc_uart_info.cdc_uart_in_ep.ep_addr);
    USB_LOG_INFO("cdc_uart int ep:%#x\n", cdc_uart_info.cdc_uart_int_ep.ep_addr);
    USB_LOG_INFO("interface_nums:%d\n", cdc_uart_info.interface_nums);

    cdc_uart_descriptor = cdc_uart_build_descriptor(&cdc_uart_info, &desc_len);
    comp_register_descriptors(USBD_TYPE_CDC_UART, cdc_uart_descriptor, desc_len, 2, cdc_uart_desc_register_cb);
    comp_register_cfg_done(USBD_TYPE_CDC_UART, cdc_uart_configure_done_callback);

    usbd_add_interface(usbd_cdc_acm_init_intf(&cdc_uart_info.cdc_uart_intf0));
    usbd_add_interface(usbd_cdc_acm_init_intf(&cdc_uart_info.cdc_uart_intf1));
    usbd_add_endpoint(&cdc_uart_info.cdc_uart_out_ep);
    usbd_add_endpoint(&cdc_uart_info.cdc_uart_in_ep);

    drv_cdc_acm_uart_register();
}

void cdc_uart_deinit(void)
{
    drv_cdc_acm_uart_unregister();
}


