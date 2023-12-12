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

/*!< endpoint address */
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

/*!< config descriptor size */
#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN)

#if CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

#define CDC_MAX_RW_LEN (CDC_MAX_MPS)

/*!< global descriptor */
static const uint8_t cdc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'V', 0x00,                  /* wcChar1 */
    'I', 0x00,                  /* wcChar2 */
    'T', 0x00,                  /* wcChar3 */
    'E', 0x00,                  /* wcChar4 */
    'K', 0x00,                  /* wcChar5 */
    'C', 0x00,                  /* wcChar6 */
    'D', 0x00,                  /* wcChar7 */
    'C', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'V', 0x00,                  /* wcChar1 */
    'I', 0x00,                  /* wcChar2 */
    'T', 0x00,                  /* wcChar3 */
    'E', 0x00,                  /* wcChar4 */
    'K', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'C', 0x00,                  /* wcChar10 */
    'D', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '3', 0x00,                  /* wcChar3 */
    '0', 0x00,                  /* wcChar4 */
    '9', 0x00,                  /* wcChar5 */
    '1', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
#if CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x02,
    0x02,
    0x01,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};


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

static struct usbd_interface intf0;
static struct usbd_interface intf1;
static uint8_t s_cdc_in_ep = CDC_IN_EP;
static uint8_t s_cdc_out_ep = CDC_OUT_EP;
static uint8_t s_cdc_init_ep = CDC_INT_EP;
static cdc_uart_dev_t *s_uart_ctx = NULL;
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t s_cdc_rx_buffer[CDC_MAX_RW_LEN];

static cdc_uart_dev_t *dev_ctx()
{
    if (!s_uart_ctx) {
        printf("cdc ctx is null\n");
    }
    return s_uart_ctx;
}

static void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
    // USB_LOG_RAW("actual out len:%d\r\n", nbytes);
    ringbuffer_write(&dev_ctx()->read_buffer, dev_ctx()->cdc_rx_buffer, nbytes);
    aos_event_set(&dev_ctx()->event_write_read, EVENT_READ, AOS_EVENT_OR);

    /* setup next out ep read transfer */
    usbd_ep_start_read(s_cdc_out_ep, dev_ctx()->cdc_rx_buffer, CDC_MAX_RW_LEN);
}

static void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes)
{
    // USB_LOG_RAW("actual in len:%d\r\n", nbytes);
    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(s_cdc_in_ep, NULL, 0);
    } else {
        aos_event_set(&dev_ctx()->event_write_read, EVENT_WRITE, AOS_EVENT_OR);
    }
}

/*!< endpoint call back */
static struct usbd_endpoint cdc_out_ep = {
    .ep_cb = usbd_cdc_acm_bulk_out
};

static struct usbd_endpoint cdc_in_ep = {
    .ep_cb = usbd_cdc_acm_bulk_in
};

uint32_t cdc_acm_comp_desc(uint8_t **desc, uint8_t bFirstInterface, uint8_t int_ep, uint8_t out_ep, uint8_t in_ep, uint8_t str_idx)
{
    if (!desc) {
        printf("desc is null\n");
        return 0;
    }

    s_cdc_in_ep = in_ep;
    s_cdc_out_ep = out_ep;
    s_cdc_init_ep = int_ep;

    cdc_out_ep.ep_addr = out_ep;
    cdc_in_ep.ep_addr = in_ep;

    uint8_t cdc_descriptor[] = {
        CDC_ACM_DESCRIPTOR_INIT(bFirstInterface, int_ep, out_ep, in_ep, str_idx),
    };

    uint32_t size = sizeof(cdc_descriptor);

    *desc = (uint8_t *)malloc(size);
    if (!*desc) {
        printf("alloc cdc desc faile, size %d\n", size);
        return 0;
    }

    memcpy(*desc, cdc_descriptor, size);
    return size;
}

void usbd_configure_done_callback(void)
{
    /* setup first out ep read transfer */
    usbd_ep_start_read(s_cdc_out_ep, s_cdc_rx_buffer, CDC_MAX_RW_LEN);
}

static void cdc_acm_init(void)
{
    usbd_desc_register(cdc_descriptor);
    usbd_add_interface(usbd_cdc_acm_init_intf(&intf0));
    usbd_add_interface(usbd_cdc_acm_init_intf(&intf1));
    cdc_out_ep.ep_addr = s_cdc_out_ep;
    cdc_in_ep.ep_addr = s_cdc_in_ep;
    usbd_add_endpoint(&cdc_out_ep);
    usbd_add_endpoint(&cdc_in_ep);
    usbd_initialize();
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
        printf("The size of a single send cannot exceed CDC_MAX_RW_LEN \n");
    }

    time_enter = aos_now_ms();

    memcpy(uart(dev)->cdc_tx_buffer, data, size);

    ret = usbd_ep_start_write(s_cdc_in_ep, uart(dev)->cdc_tx_buffer, size);

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

void cdc_acm_uart_drv_init(uint8_t idx)
{
	cdc_acm_init();
    rvm_driver_register(&usb_serial_uart_driver.drv, NULL, idx);
}

void cdc_acm_uart_drv_uninit()
{
	usbd_deinitialize();
}