/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_debug.h
 * Description:
 */

#ifndef __CVI_DEBUG_H__
#define __CVI_DEBUG_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <cvi_common.h>

#if defined(CONFIG_KERNEL_RHINO)
#include <debug/dbg.h>
#include <drv/tick.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*
 * Debug Config
 */
#define CONFIG_CVI_GDB_NO 1
#define CONFIG_CVI_GDB "n"
// #define CONFIG_CVI_LOG_TRACE_SUPPORT 1
#define CONFIG_CVI_LOG_TRACE_ALL 1
#define CONFIG_CVI_LOG_TRACE_LEVEL 4


#define CVI_DBG_EMERG      0   /* system is unusable                   */
#define CVI_DBG_ALERT      1   /* action must be taken immediately     */
#define CVI_DBG_CRIT       2   /* critical conditions                  */
#define CVI_DBG_ERR        3   /* error conditions                     */
#define CVI_DBG_WARN       4   /* warning conditions                   */
#define CVI_DBG_NOTICE     5   /* normal but significant condition     */
#define CVI_DBG_INFO       6   /* informational                        */
#define CVI_DBG_DEBUG      7   /* debug-level messages                 */

typedef struct _LOG_LEVEL_CONF_S {
	MOD_ID_E  enModId;
	CVI_S32   s32Level;
	char   cModName[16];
} LOG_LEVEL_CONF_S;

#define CVI_PRINT printf

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#if defined(CONFIG_KERNEL_RHINO)
extern CVI_S32 log_levels[CVI_ID_BUTT];
extern const CVI_CHAR *CVI_SYS_GetModName(MOD_ID_E id);
#else
extern CVI_S32 *log_levels;
#define _GENERATE_STRING(STRING) (#STRING),
static const char *const MOD_STRING[] = FOREACH_MOD(_GENERATE_STRING);
#define CVI_GET_MOD_NAME(id) (id < CVI_ID_BUTT)? MOD_STRING[id] : "UNDEF"
#endif

extern CVI_CHAR const *log_name[8];

#pragma GCC diagnostic pop


/* #ifdef CVI_DEBUG */
#if (CONFIG_CVI_LOG_TRACE_SUPPORT == 1)

	#define CVI_ASSERT(expr)                               \
	do {                                                   \
		if (!(expr)) {                                 \
			printf("\nASSERT at:\n"                \
			       "  >Function : %s\n"            \
			       "  >Line No. : %d\n"            \
			       "  >Condition: %s\n",           \
			       __func__, __LINE__, #expr);     \
			_exit(-1);                             \
		} \
	} while (0)

#ifndef FPGA_PORTING

#if defined(CONFIG_KERNEL_RHINO)
	#define CVI_TRACE(level, enModId, fmt, ...)            \
	do {                                                   \
		CVI_S32 LogLevel = (log_levels == NULL) ? CONFIG_CVI_LOG_TRACE_LEVEL : log_levels[enModId];      \
		if (level <= LogLevel)    \
			aos_debug_printf("[%d][%s-%s] " fmt, csi_tick_get_ms(), CVI_SYS_GetModName(enModId), log_name[level],    \
				##__VA_ARGS__);           \
	} while (0)
#else
	#define CVI_TRACE(level, enModId, fmt, ...)            \
	do {                                                   \
		CVI_S32 LogLevel = (log_levels == NULL) ? CONFIG_CVI_LOG_TRACE_LEVEL : log_levels[enModId];      \
		if (level <= LogLevel)    \
			syslog(LOG_LOCAL5|level, "[%s-%s] " fmt, CVI_GET_MOD_NAME(enModId), log_name[level], \
				##__VA_ARGS__);           \
	} while (0)
#endif

#else
	#define CVI_TRACE(level, enModId, fmt, ...) \
		printf(fmt, ##__VA_ARGS__)
#endif
#else
	#define CVI_ASSERT(expr)
	#define CVI_TRACE(level, enModId, fmt...)
#endif

#define CVI_TRACE_ID(level, id, fmt, ...)                                           \
		CVI_TRACE(level, id, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_LOG(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_LOG, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_SYS(level, fmt, ...)                                           \
		CVI_TRACE(level, CVI_ID_SYS, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_VB(level, fmt, ...)                                           \
		CVI_TRACE(level, CVI_ID_VB, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_SNS(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_VI, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_VI(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_VI, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_VPSS(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_VPSS, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_VO(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_VO, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_GDC(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_GDC, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_RGN(level, fmt, ...)                                           \
		CVI_TRACE(level, CVI_ID_RGN, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_MISC(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_SYS, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_IPCMSG(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_IPCMSG, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_DATAFIFO(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_DATAFIFO, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#define CVI_TRACE_MSG(level, fmt, ...)  \
		CVI_TRACE(level, CVI_ID_MSG, "%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __CVI_COMM_SYS_H__ */

