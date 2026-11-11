/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <aos/kernel.h>
#include <aos/debug.h>
#include "aos/hal/uart.h"

#define DUMP_PRINT_DEFAULT_SIZE 0x1000

/* this structure should be modified both linux & alios side */
struct dump_uart_s {
	char* dump_uart_ptr;
	unsigned int  dump_uart_max_size;
	unsigned int  dump_uart_pos;
	unsigned char dump_uart_enable;
	unsigned char dump_uart_overflow;
} __attribute__((packed));

struct dump_uart_s *dump_uart = NULL;

static uart_dev_t _uart, *g_console_handle;
static int g_console_uart;
static uint16_t g_console_buf_size = 128;
static aos_mutex_t g_console_mutex_handle;
const char *console_get_devname(void)
{
    static char console_devname[32] = {0};

    if (g_console_handle) {
        snprintf(console_devname, sizeof(console_devname), "uart%d", g_console_handle->port);
    }

    return console_devname;
}

uint16_t console_get_buffer_size(void)
{
    return g_console_buf_size;
}

void console_init(int idx, uint32_t baud, uint16_t buf_size)
{
    int rc;
    char *ptr;
    int dump_size;
    aos_mutex_new(&g_console_mutex_handle);

    _uart.port                = idx;
    _uart.config.baud_rate    = baud;
    _uart.config.mode         = MODE_TX_RX;
    _uart.config.flow_control = FLOW_CONTROL_DISABLED;
    _uart.config.stop_bits    = STOP_BITS_1;
    _uart.config.parity       = NO_PARITY;
    _uart.config.data_width   = DATA_WIDTH_8BIT;

    rc = hal_uart_init(&_uart);
    if (rc == 0) {
        g_console_handle = &_uart;
        g_console_buf_size = buf_size;
    }
    g_console_uart = idx;

    dump_size = DUMP_PRINT_DEFAULT_SIZE;
    ptr = malloc(dump_size + 0x40);
    if (ptr == 0)
        return;
    dump_uart = (struct dump_uart_s *)((((unsigned long)ptr) + 0x3F) & ~0x3F);
    dump_uart->dump_uart_pos = 0;
    dump_uart->dump_uart_overflow = 0;
    dump_uart->dump_uart_max_size = dump_size - sizeof(struct dump_uart_s);
    dump_uart->dump_uart_ptr = (char *)dump_uart + sizeof(struct dump_uart_s);
}

void console_deinit(void)
{
    //TODO:
    hal_uart_finalize(g_console_handle);
    g_console_handle = NULL;
    aos_mutex_free(&g_console_mutex_handle);
    return;
}

int console_get_uart(void)
{
    return g_console_uart;
}

int get_dump_uart_handle(void **handle, unsigned int *len)
{
    if (dump_uart) {
        *handle = dump_uart;
        *len = dump_uart->dump_uart_max_size + sizeof(struct dump_uart_s);
        return 0;
    }

    return -1;
}

int uart_putc(int ch)
{
    char *ptr = NULL;
    CHECK_PARAM(g_console_handle, -1);
    if (ch == '\n') {
        int data = '\r';
        if (!aos_irq_context()) {
            hal_uart_send(g_console_handle, &data, 1, AOS_WAIT_FOREVER);
        } else {
            hal_uart_send_irq(g_console_handle, &data, 1);
        }
    }

    if (!aos_irq_context()) {
        hal_uart_send(g_console_handle, &ch, 1, AOS_WAIT_FOREVER);
    } else {
        hal_uart_send_irq(g_console_handle, &ch, 1);
    }

    if (dump_uart)
        ptr = dump_uart->dump_uart_ptr;
    if (ptr) {
        ptr[dump_uart->dump_uart_pos++] = ch;
        if (dump_uart->dump_uart_pos >= dump_uart->dump_uart_max_size) {
            dump_uart->dump_uart_overflow = 1;
            dump_uart->dump_uart_pos = 0;
        }
    }

    return 0;
}

int uart_getc(void)
{
    if (g_console_handle != NULL) {
        char ch = 0;
        uint32_t rev_length = 0;

        hal_uart_recv_II(g_console_handle, &ch, 1, &rev_length, AOS_WAIT_FOREVER);

        return (int)ch;
    }

    return 0;
}

int uart_write(const void *buf, size_t size)
{
    int count = 0;
    CHECK_PARAM(g_console_handle, -1);
    aos_mutex_lock(&g_console_mutex_handle, AOS_WAIT_FOREVER);

#if 0
    if (!aos_irq_context()) {
        hal_uart_send(g_console_handle, buf, size, AOS_WAIT_FOREVER);
    } else {
        hal_uart_send_poll(g_console_handle, buf, size);
    }
#else
    for (count = 0; count < size; count++) {
        uart_putc(*(char *)buf++);
    }
#endif

    aos_mutex_unlock(&g_console_mutex_handle);

    return size;
}

int console_flush_uart_ringbuffer(void)
{
    return hal_uart_flush_ringbuffer(g_console_handle);
}

__attribute__((weak)) int os_critical_enter(unsigned int *lock)
{
    int ret;
    CHECK_PARAM(g_console_handle, -1);
    ret = aos_mutex_lock(&g_console_mutex_handle,10000);
    return ret;
}

__attribute__((weak)) int os_critical_exit(unsigned int *lock)
{
    int ret;
    CHECK_PARAM(g_console_handle, -1);
    ret = aos_mutex_unlock(&g_console_mutex_handle);
    return ret;
}

int console_push_data_to_ringbuffer(const void *buf, size_t size)
{
    if (g_console_handle != NULL) {

        hal_uart_push_data_to_ringbuffer(g_console_handle, buf, size);

        return (int)size;
    }

    return -1;
}
