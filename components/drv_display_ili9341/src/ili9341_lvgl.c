/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include "ili9341_lvgl.h"
#include "aos/cli.h"

#include <stdio.h>
#include <sys/ioctl.h>

#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#include <aos/kernel.h>
#include <aos/cli.h>
#include "drv/spi.h"
#include <unistd.h>
#include <sys/prctl.h>
#include <time.h>
#include <sys/time.h>

#include "lvgl.h"
#include "lv_demos.h"
#include <devices/display.h>


#define ILI9341_TFTWIDTH 240
#define ILI9341_TFTHEIGHT 320

// #define ILI9341_DISPLAY_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT

#define ILI9341_FBSIZE (ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT * 3)

ili9341_dev_t ili9341_dev = {0};
csi_spi_t spi_handler;

typedef enum {
    CMD = 0,
    DAT = 1,
} ili9341_dc_t;

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)
//设置如：GPIOA[18]输出1，则gpio_grp=0, gpio_num=18, level=1
void _gpio_output(u8 gpio_grp, u8 gpio_num, u8 level)
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

int _config_spi(csi_spi_t *spi_handler)
{
	int spi_ret = 0;

	spi_ret = csi_spi_init(spi_handler, 1); //用spi 1
	if (spi_ret == CSI_ERROR) {
		printf("spi init failed returned, actual %d\n", spi_ret);
		return -1;
	}

	// test csi_spi_mode
	spi_ret = csi_spi_mode(spi_handler, SPI_MASTER);
	if (spi_ret == CSI_ERROR) {
		printf("set spi mode failed, actual ret %d\n", spi_ret);
		return -1;
	}

	// test csi_spi_cp_format
	// spi_ret = csi_spi_cp_format(spi_handler, SPI_FORMAT_CPOL0_CPHA0); //spi mode 0
    spi_ret = csi_spi_cp_format(spi_handler, SPI_FORMAT_CPOL1_CPHA1); //spi mode 3
	if (spi_ret == CSI_ERROR) {
		printf("set spi format failed returned, actual ret %d\n", spi_ret);
		return -1;
	}

	// test csi_spi_frame_len
	spi_ret = csi_spi_frame_len(spi_handler, SPI_FRAME_LEN_8);
	if (spi_ret == CSI_ERROR) {
		printf("set spi frame len failed, actual ret %d\n", spi_ret);
		return -1;
	}

	// test csi_spi_baud
	csi_spi_baud(spi_handler, 50000000);
	return spi_ret;
}

void ili9341_dc_write_bytes(ili9341_dev_t ili9341_dev,
                                          ili9341_dc_t  ili9341_dc,
                                          uint8_t *bytes,
                                          uint32_t      size)
{
    _gpio_output(ili9341_dev.gpio_fd, ili9341_dev.gpio_dc_id, ili9341_dc);
    csi_spi_send_async(&spi_handler, bytes, size);
    udelay(10);
}

void ili9341_dc_write_byte(ili9341_dev_t ili9341_dev,
                                         ili9341_dc_t  ili9341_dc,
                                         uint8_t       byte)
{
    _gpio_output(ili9341_dev.gpio_fd, ili9341_dev.gpio_dc_id, ili9341_dc);
    csi_spi_send_async(&spi_handler, &byte, 1);
    udelay(10);
}

void ili9341_set_address_window(ili9341_dev_t ili9341_dev,
                            uint16_t      x_0,
                            uint16_t      y_0,
                            uint16_t      x_1,
                            uint16_t      y_1)
{
    uint8_t data[4] = {0};
    if (x_0 < x_1) {
        data[0] = x_0 >> 8;
        data[1] = x_0;
        data[2] = x_1 >> 8;
        data[3] = x_1;
    } else {
        data[0] = x_1 >> 8;
        data[1] = x_1;
        data[2] = x_0 >> 8;
        data[3] = x_0;
    }
    ili9341_dc_write_byte(ili9341_dev, CMD, ILI9341_CASET);
    ili9341_dc_write_bytes(ili9341_dev, DAT, data, 4);
    if (y_0 < y_1) {
        data[0] = y_0 >> 8;
        data[1] = y_0;
        data[2] = y_1 >> 8;
        data[3] = y_1;
    } else {
        data[0] = y_1 >> 8;
        data[1] = y_1;
        data[2] = y_0 >> 8;
        data[3] = y_0;
    }
    ili9341_dc_write_byte(ili9341_dev, CMD, ILI9341_PASET);
    ili9341_dc_write_bytes(ili9341_dev, DAT, data, 4);

    ili9341_dc_write_byte(ili9341_dev, CMD, ILI9341_RAMWR); //发送命令开始数据写入

}

void ili9341_draw_rect(ili9341_dev_t ili9341_dev,
                       uint16_t      x,
                       uint16_t      y,
                       uint16_t      width,
                       uint16_t      height,
                       uint32_t      color)
{
    ili9341_set_address_window(ili9341_dev, x, y, x + width - 1, y + height - 1);
    uint32_t       bufferSize   = width * height * sizeof(uint16_t);
    unsigned char *burst_buffer = (unsigned char *)malloc(bufferSize);
    for (uint32_t i = 0; i < bufferSize; i++) {
        burst_buffer[i] = (bufferSize % 2) ? color : color >> 8;
    }
    ili9341_dc_write_bytes(ili9341_dev, DAT, burst_buffer, bufferSize);
    free(burst_buffer);
}

void ili9341_init_command(ili9341_dev_t ili9341_dev)
{
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xCF);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0xD9); // 或0xC1
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x30);

    ili9341_dc_write_byte(ili9341_dev, CMD, 0xED);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x64);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x03);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x12);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x81);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xE8);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x85);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x10);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x7A);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xCB);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x39);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x2C);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x34);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x02);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xF7);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x20);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xEA);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xC0); // Power control
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x1B); // VRH[5:0]
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xC1); // Power control
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x12); // SAP[2:0];BT[3:0]
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xC5); // VCM control
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x08); // 0x30
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x26); // 0x30
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xC7); // VCM control2
    ili9341_dc_write_byte(ili9341_dev, DAT, 0xB7);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0x36); // Memory Access Control
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x08);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0x3A); //像素格式寄存器
    // ili9341_dc_write_byte(ili9341_dev, DAT, 0x55); //设置为RGB565格式
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x66); //设置为RGB666格式
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xB1);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x1A);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xB6); // Display Function Control
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x0A);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0xA2);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xF2); // 3Gamma Function Disable
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, CMD, 0x26); // Gamma curve selected
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x01);

    // Set Gamma
    // Positive Gamma Correction
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xE0);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x0F);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x1D);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x1A);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x0A);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x0D);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x07);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x49);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x66);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x3B);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x07);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x11);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x01);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x09);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x05);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x04);
    // Negative Gamma Correction
    ili9341_dc_write_byte(ili9341_dev, CMD, 0xE1);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x18);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x1D);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x02);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x0F);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x04);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x36);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x13);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x4C);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x07);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x13);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x0F);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x2E);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x2F);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x05);

    // Set the display's internal frame rate
    ili9341_dc_write_byte(ili9341_dev, CMD, 0x2B);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x01);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x3f);

    // Set the display's column address range
    ili9341_dc_write_byte(ili9341_dev, CMD, 0x2A);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0x00);
    ili9341_dc_write_byte(ili9341_dev, DAT, 0xef);

    // Exit Sleep mode
    ili9341_dc_write_byte(ili9341_dev, CMD, 0x11);
    usleep(1000 * 120); // Wait for the screen to wake up

    // Turn on the display
    ili9341_dc_write_byte(ili9341_dev, CMD, 0x29);

}

void ili9341_init()
{

    ili9341_dev.gpio_fd = 4;
    ili9341_dev.gpio_reset_id = 26;
    ili9341_dev.gpio_dc_id = 25;
    ili9341_dev.gpio_cs_id = 20;

    _gpio_output(ili9341_dev.gpio_fd, ili9341_dev.gpio_cs_id, 1);
    usleep(50 * 1000);

    //初始化spi

    _config_spi(&spi_handler);

    //初始化屏幕之前复位
    _gpio_output(ili9341_dev.gpio_fd, ili9341_dev.gpio_reset_id, 1);
    usleep(50 * 1000);
    _gpio_output(ili9341_dev.gpio_fd, ili9341_dev.gpio_reset_id, 0);
    usleep(50 * 1000);
    _gpio_output(ili9341_dev.gpio_fd, ili9341_dev.gpio_reset_id, 1);
    usleep(50 * 1000);
    printf("reset ok\n");
    _gpio_output(ili9341_dev.gpio_fd, ili9341_dev.gpio_cs_id, 0);
    printf("cs ok\n");
    usleep(1000 * 1000);
    //初始化屏幕
    ili9341_init_command(ili9341_dev);
    printf("ili9341 hardware_init done\n");
    usleep(1000 * 1000);

#if 0
    ili9341_draw_rect(ili9341_dev, 0, 0, 200, 200, 0xFB00, &spi_handler);
    printf("ili9341_draw_rect done\n");
#endif
}

static void ili9341_disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *color_p) {
    // printf("in ili9341_disp_flush\n");
    uint32_t px_size = lv_color_format_get_size(lv_display_get_color_format(disp_drv));
    // printf("px_size:%d\n", px_size);
    // 计算要刷新的区域的大小
    uint32_t width = (area->x2 - area->x1 + 1);
    uint32_t height = (area->y2 - area->y1 + 1);
    uint32_t size = width * height * px_size;

    // 分配缓冲区用于存储8位数据
    uint8_t *buffer = (uint8_t *)malloc(size);
    if (!buffer) {
        // 内存分配失败
        lv_disp_flush_ready(disp_drv);
        return;
    }

    // 设置要刷新的显示区域
    ili9341_set_address_window(ili9341_dev, area->x1, area->y1, area->x2, area->y2);

    if (LV_COLOR_DEPTH == 16) {
        for (uint32_t i = 0; i < size; i++) {
            buffer[i] = color_p[i];
        }
        lv_draw_sw_rgb565_swap(buffer, width * height);
    }

    // if (LV_COLOR_DEPTH == 24){
    //     for (uint32_t i = 0; i < size; i++) {
    //         // 将RGB888转换为RGB666（只需丢弃每个分量的最低两位）
    //         buffer[i] = color_p[i] >> 2;
    //         buffer[i] = buffer[i] << 2;
    //     }

    // }

    if (LV_COLOR_DEPTH == 24) {
        for (uint32_t i = 0; i < size; i += 3) {
            // 交换红色和蓝色分量，保持绿色分量不变
            uint8_t red = color_p[i];     // 原始的红色分量
            uint8_t blue = color_p[i + 2]; // 原始的蓝色分量

            // 将BGR值写入buffer
            buffer[i] = blue;             // 蓝色分量
            buffer[i + 1] = color_p[i + 1]; // 绿色分量，保持不变
            buffer[i + 2] = red;          // 红色分量
        }
    }

    // 通过SPI发送数据
    ili9341_dc_write_bytes(ili9341_dev, DAT, buffer, size);
    // printf("ili9341_disp_flush done\n");

    // 释放缓冲区
    free(buffer);

    // 通知 LVGL 刷新完成
    lv_disp_flush_ready(disp_drv);

}

void ili9341_lvgl()
{
    lv_init();
    ili9341_init();

    static lv_color_t buf1[ILI9341_FBSIZE];
    lv_display_t * disp = lv_display_create(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT);
    lv_display_set_flush_cb(disp, ili9341_disp_flush);
    lv_display_set_buffers(disp, buf1, NULL, ILI9341_FBSIZE, LV_DISPLAY_RENDER_MODE_FULL);
    // printf("lv_display_set_buffers ok\n");

    // lv_demo_benchmark();
    // lv_demo_widgets();
    lv_demo_stress();

#if 0
    //画矩形
    /* 创建一个屏幕对象或者使用默认屏幕 */
    lv_obj_t * scr = lv_scr_act();

    /* 创建矩形对象（正方形） */
    lv_obj_t * rect = lv_obj_create(scr); /* 创建一个基本对象，父对象是屏幕 */
    lv_obj_set_size(rect, 100, 100); /* 设置矩形的大小为100x100像素 */
    lv_obj_align(rect, LV_ALIGN_CENTER, 0, 0); /* 将矩形居中对齐 */

    /* 设置矩形的样式（可选） */
    static lv_style_t style_rect;
    lv_style_init(&style_rect);
    lv_style_set_bg_color(&style_rect, lv_color_make(0x00, 0x00, 0xFF)); /* 设置背景颜色为蓝色*/
    lv_style_set_border_color(&style_rect, lv_color_make(0x00, 0xFF, 0x00)); /* 设置边框颜色为绿色 */
    lv_style_set_border_width(&style_rect, 10); /* 设置边框宽度为10像素 */
    lv_obj_add_style(rect, &style_rect, 0 ); /* 将样式应用到矩形对象 */
#endif

    while (1) {
        lv_task_handler();
        lv_tick_inc(1); //告诉lvgl，1ms过去了
        usleep(1000); //休眠1ms
    }

}

ALIOS_CLI_CMD_REGISTER(ili9341_lvgl, ili9341_lvgl_example, ili9341 init test example)
