/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <drv/tick.h>

#include "ring_buffer.h"
#include "common.h"
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

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

#define CDC_MAX_LEN (CDC_MAX_MPS)

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
#ifdef CONFIG_USB_HS
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
#define UART_RB_SIZE (1 * 1024 * 1024)

static uint8_t *cdc_rx_buffer = NULL;
static uint8_t *cdc_tx_buffer = NULL;
static uint8_t *rx_ring_buffer = NULL;
static dev_ringbuf_t read_buffer;

static int ctx_init(void)
{
    cdc_rx_buffer = (uint8_t *)align_iomalloc(CDC_MAX_LEN, CONFIG_USB_ALIGN_SIZE);
    if (!cdc_rx_buffer) {
        printf("Failed alloc size %d\n", CDC_MAX_LEN);
        return -1;
    }

    cdc_tx_buffer = (uint8_t *)align_iomalloc(CDC_MAX_LEN, CONFIG_USB_ALIGN_SIZE);
    if (!cdc_tx_buffer) {
        printf("Failed alloc size %d\n", CDC_MAX_LEN);
        return -1;
    }

    rx_ring_buffer = (uint8_t *)malloc(UART_RB_SIZE);
    if (!rx_ring_buffer) {
        printf("Failed alloc size %d\n", UART_RB_SIZE);
        return -1;
    }

    ringbuffer_create(&read_buffer, (char *)rx_ring_buffer, UART_RB_SIZE);

    return 0;
}

void usbd_configure_done_callback(void)
{
    /* setup first out ep read transfer */
    usbd_ep_start_read(CDC_OUT_EP, cdc_rx_buffer, CDC_MAX_LEN);
}

static void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
    // read
    // printf("actual read len:%d\r\n", nbytes);
    // for (int i = 0; i < nbytes; i++) {
    //     printf("%02x ", cdc_rx_buffer[i]);
    // }
    // printf("\n");
    ringbuffer_write(&read_buffer, cdc_rx_buffer, nbytes);

    // test echo
    // usbd_ep_start_write(CDC_IN_EP, cdc_rx_buffer, nbytes);

    /* setup next out ep read transfer */
    usbd_ep_start_read(CDC_OUT_EP, cdc_rx_buffer, CDC_MAX_LEN);
}

static void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes)
{
    // write
    // USB_LOG_RAW("actual in len:%d\r\n", nbytes);

    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(CDC_IN_EP, NULL, 0);
    } else {
        // aos_event_set(&dev_ctx()->event_write_read, EVENT_WRITE, AOS_EVENT_OR);
        // notify
    }
}

/*!< endpoint call back */
static struct usbd_endpoint cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

static struct usbd_endpoint cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};

static struct usbd_interface intf0;
static struct usbd_interface intf1;

int cdc_acm_write(uint8_t *buffer, uint32_t nbytes)
{
    if (!cdc_tx_buffer || nbytes > CDC_MAX_LEN) {
        printf("cdc write failed, buffer:%p, nbytes: %d\n", buffer, nbytes);
        return -1;
    }

    memcpy(cdc_tx_buffer, buffer, nbytes);
    return usbd_ep_start_write(CDC_IN_EP, cdc_tx_buffer, nbytes);
}

int cdc_acm_read(uint8_t *buffer, uint32_t nbytes, uint32_t timeout_ms)
{
    int ret = 0;

    if (!cdc_rx_buffer) {
        return ret;
    }

    uint16_t read_len = 0;
    uint16_t already_len = 0;
    uint8_t retry_cnt = timeout_ms;
    retry_cnt = (retry_cnt == 0) ? 1 : retry_cnt;

    while(retry_cnt--) {
        read_len = ringbuffer_read(&read_buffer, buffer + already_len, nbytes - already_len);
        already_len += read_len;

        if (already_len == nbytes) {
            ret = 0;
            break;
        }

        mdelay(1);
    }

    return already_len;
}


void cdc_acm_init(void)
{
    ctx_init();
    usbd_desc_register(cdc_descriptor);
    usbd_add_interface(usbd_cdc_acm_init_intf(&intf0));
    usbd_add_interface(usbd_cdc_acm_init_intf(&intf1));
    usbd_add_endpoint(&cdc_out_ep);
    usbd_add_endpoint(&cdc_in_ep);
    usbd_initialize();
}

void cdc_acm_deinit(void)
{
    usbd_deinitialize();
}
