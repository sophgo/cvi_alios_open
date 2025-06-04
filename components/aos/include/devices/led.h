/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef DEVICE_LED_PAT_H
#define DEVICE_LED_PAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <devices/device.h>
#include <devices/hal/led_impl.h>

#define COLOR_RED_OFF   16
#define COLOR_GREEN_OFF 8
#define COLOR_BLUE_OFF  0

#define COLOR_SET(r, g, b) (r << COLOR_RED_OFF | g << COLOR_GREEN_OFF | b << COLOR_BLUE_OFF)
#define COLOR_GET(c, off) ((c >> off) & 0xff)

#define COLOR_RED_GET(c)   COLOR_GET(c, COLOR_RED_OFF)
#define COLOR_GREEN_GET(c) COLOR_GET(c, COLOR_GREEN_OFF)
#define COLOR_BLUE_GET(c)  COLOR_GET(c, COLOR_BLUE_OFF)

#define COLOR_RED       COLOR_SET(0xff, 0, 0)   /* 红灯亮 */
#define COLOR_GREEN     COLOR_SET(0, 0xff, 0)   /* 绿灯亮 */
#define COLOR_BLUE      COLOR_SET(0, 0, 0xff)   /* 蓝灯亮 */

#define COLOR_WHITE     COLOR_SET(0xff, 0xff, 0xff) /* 全亮 */
#define COLOR_BLACK     COLOR_SET(0, 0, 0)          /* 全灭 */

#define BLINK_ON        1
#define BLINK_OFF       0

#define LED_PIN_NOT_SET 0xffffffff

/**
 * 打开设备
 * @param name 设备名称
 *             使用led_rgb_register接口注册YoC通用的LED设备名称前缀固定为"ledrgb"
 *             例如注册调用led_rgb_register(&led_config, 1)
 *             传入的name为 "ledrgb1"
 * @return 返回设备句柄dev_t
 */
#define led_open(name) device_open(name)

/**
 * 打开设备
 * @param name 设备名称
 *             使用led_rgb_register接口注册YoC通用的LED设备名称前缀固定为"ledrgb"
 * @param id   设备id
 *             例如注册调用led_rgb_register(&led_config, 1)
 *             传入的name为 "ledrgb1"
 * @return 返回设备句柄dev_t
 */
#define led_open_id(name, id) device_open_id(name, id)

/**
 * 关闭设备
 * 由于LED需要长期使用，一般情况下无需关闭
 * @param dev 设备句柄
 */
#define led_close(dev) device_close(dev)

/**
 * 控制灯的状态
 * YoC实现的通用驱动,支持GPIO控制RGB三色亮灭,不支持灰度
 * 接口支持灰度保留扩展
 *
 * @param dev 设备句柄
 * @param color 灯亮灭控制，可使用宏COLOR_RED COLOR_GREEN COLOR_BLUE COLOR_GENERIC COLOR_BLACK COLOR_SET
 * @param on_time 亮的持续时间(ms), <=0 闪烁功能无效
 * @param off_time 灭的持续时间(ms), <=0 闪烁功能无效
 */
int led_control(aos_dev_t *dev, int color, int on_time, int off_time);

#ifdef __cplusplus
}
#endif

#endif
