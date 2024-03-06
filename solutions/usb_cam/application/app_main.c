/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include "common_yocsystem.h"
#include "media_video.h"
#include "gui_display.h"
#include "media_audio.h"
#include "platform.h"
#include "custom_event.h"
#include "cvi_param.h"
#include "wifi_if.h"
#include "ethernet_init.h"

#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

#define TAG "app"

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
extern unsigned long long timer_get_boot_us(void);
extern int csi_uart_set_output_stat(int stat);

#if CONFIG_QUICK_STARTUP_SUPPORT
#define CONFIG_DUMP_RECORD_TIME
#endif
#ifdef CONFIG_DUMP_RECORD_TIME
#define TIME_RECORDS_ADDR 0x0e000010
#define TIME_RECORDS_SIZE (2 * 14)
struct _time_records {
	uint16_t fsbl_start;
	uint16_t ddr_init_start;
	uint16_t ddr_init_end;
	uint16_t release_blcp_2nd;
	uint16_t load_loader_2nd_end;
	uint16_t fsbl_decomp_start;
	uint16_t fsbl_decomp_end;
	uint16_t fsbl_exit;
	uint16_t boot_main;
	uint16_t load_yoc_start;
	uint16_t load_yoc_end;
	uint16_t boot_end;
	uint16_t app_main;
} __packed;
static struct _time_records t_r ={};
static void _dump_record_time(void)
{
	t_r.fsbl_start = *(unsigned short *)(TIME_RECORDS_ADDR);
	t_r.ddr_init_start = *(unsigned short *)(TIME_RECORDS_ADDR+2);
	t_r.ddr_init_end = *(unsigned short *)(TIME_RECORDS_ADDR+4);
	t_r.release_blcp_2nd = *(unsigned short *)(TIME_RECORDS_ADDR+6);
	t_r.load_loader_2nd_end = *(unsigned short *)(TIME_RECORDS_ADDR+8);
	t_r.fsbl_decomp_start = *(unsigned short *)(TIME_RECORDS_ADDR+10);
	t_r.fsbl_decomp_end = *(unsigned short *)(TIME_RECORDS_ADDR+12);
	t_r.fsbl_exit = *(unsigned short *)(TIME_RECORDS_ADDR+14);
	t_r.boot_main = *(unsigned short *)(TIME_RECORDS_ADDR+16);
	t_r.load_yoc_start = *(unsigned short *)(TIME_RECORDS_ADDR+18);
	t_r.load_yoc_end = *(unsigned short *)(TIME_RECORDS_ADDR+20);
	t_r.boot_end = *(unsigned short *)(TIME_RECORDS_ADDR+22);
	// t_r.kernel_run_init_start = *(unsigned short *)(TIME_RECORDS_ADDR+24);
}

static void _print_record_time(void)
{
	printf("fsbl_start %d\n", t_r.fsbl_start);
	printf("ddr_init_start %d\n", t_r.ddr_init_start);
	printf("ddr_init_end %d\n", t_r.ddr_init_end);
	printf("release_blcp_2nd %d\n", t_r.release_blcp_2nd);
	printf("load_loader_2nd_end %d\n", t_r.load_loader_2nd_end);
	printf("fsbl_decomp_start %d\n", t_r.fsbl_decomp_start);
	printf("fsbl_decomp_end %d\n", t_r.fsbl_decomp_end);
	printf("fsbl_exit %d\n", t_r.fsbl_exit);
	printf("boot_main %d\n", t_r.boot_main);
	printf("load_yoc_start %d\n", t_r.load_yoc_start);
	printf("load_yoc_end %d\n", t_r.load_yoc_end);
	printf("boot_end %d\n", t_r.boot_end);
	printf("app_main %d\n", t_r.app_main);
}
#endif

int main(int argc, char *argv[])
{
#ifdef CONFIG_DUMP_RECORD_TIME
	t_r.app_main = DIV_ROUND_UP(timer_get_boot_us(), 1000);
	_dump_record_time();
#endif
	YOC_SYSTEM_Init();
	//board pinmux init
	PLATFORM_IoInit();
	//Fs init
	YOC_SYSTEM_FsVfsInit();
	//load cfg
	PARAM_LoadCfg();
	//media video sys init
	MEDIA_VIDEO_SysInit();
	//custom_evenet_pre
	//media video
	MEDIA_VIDEO_Init();
	//media_audio
	MEDIA_AUDIO_Init();
	//network
	#if (CONFIG_APP_ETHERNET_SUPPORT == 1)
	ethernet_init();
	#endif
	#if (CONFIG_APP_WIFI_SUPPORT == 1)
	APP_WifiInit();
	#endif
	//cli and ulog init
	YOC_SYSTEM_ToolInit();
	#if (CONFIG_PQTOOL_SUPPORT == 1)
	usleep(12 * 1000);
	isp_daemon2_init(5566);
	#endif
	LOGI(TAG, "app start........\n");
	APP_CustomEventStart();
#ifdef CONFIG_DUMP_RECORD_TIME
	aos_msleep(300);
	csi_uart_set_output_stat(1);
	_print_record_time();
#endif
	while (1) {
		aos_msleep(3000);
	};
}
