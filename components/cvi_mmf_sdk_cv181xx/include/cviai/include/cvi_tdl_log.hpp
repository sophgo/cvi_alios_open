#pragma once
#include <inttypes.h>
#include <syslog.h>
// #include <ulog/ulog.h>

#ifndef syslog
#define syslog(level, fmt, ...)            \
do {                                                   \
	printf(fmt, ##__VA_ARGS__);                \
} while (0)
#endif

#define MODULE_NAME "TDLSDK"
#define CVI_TDL_TAG "CVI_TDL_alios_v1"
#define CVI_TDL_LOG_CHN LOG_LOCAL7
#define LOGD(fmt, ...) \
  syslog(CVI_TDL_LOG_CHN | LOG_DEBUG, "[%s] [D] " fmt, MODULE_NAME, ##__VA_ARGS__)
#define LOGI(fmt, ...) \
  syslog(CVI_TDL_LOG_CHN | LOG_INFO, "[%s] [I] " fmt, MODULE_NAME, ##__VA_ARGS__)
#define LOGN(fmt, ...) \
  syslog(CVI_TDL_LOG_CHN | LOG_NOTICE, "[%s] [N] " fmt, MODULE_NAME, ##__VA_ARGS__)
#define LOGW(fmt, ...) \
  syslog(CVI_TDL_LOG_CHN | LOG_WARNING, "[%s] [W] " fmt, MODULE_NAME, ##__VA_ARGS__)
#define LOGE(fmt, ...) \
  syslog(CVI_TDL_LOG_CHN | LOG_ERR, "[%s] [E] " fmt, MODULE_NAME, ##__VA_ARGS__)
#define LOGC(fmt, ...) \
  syslog(CVI_TDL_LOG_CHN | LOG_CRIT, "[%s] [C] " fmt, MODULE_NAME, ##__VA_ARGS__)
