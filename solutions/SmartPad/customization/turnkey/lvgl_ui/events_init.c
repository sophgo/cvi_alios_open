/*
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "events_init.h"
#include <stdio.h>
#include <lvgl.h>
#include <yoc/netmgr.h>
#include <yoc/netmgr_service.h>
#include <devices/netdrv.h>
#include <devices/netdrv.h>
#include <devices/wifi.h>
#include "wifi_if.h"
#include "gui_guider_custom.h"
static int record_start_bias = 0;
static int record_time = 0;
static int record_play_time = 0;
static int play_start_bias = 0;
static lv_timer_t *  counter_timer;

static void event_timer_cb(lv_timer_t *t)
{
	CVI_CHAR Text_String[64] = {0};
	unsigned char mac[6] = {0};
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;
	rvm_dev_t *dev;
	rvm_hal_wifi_ap_record_t ap_info = {0};
#if (CONFIG_APP_WIFI_SUPPORT == 1)
	dev = rvm_hal_device_find("wifi", 0);
#endif
	/** ifconfig */
	if(dev == NULL) {
		return ;
	}
	rvm_hal_net_get_ipaddr(dev, &ipaddr, &netmask, &gw);
	rvm_hal_net_get_mac_addr(dev, mac);

	/** iw dev wlan0 link */
	rvm_hal_wifi_sta_get_link_status(dev, &ap_info);
	if ((ap_info.link_status == WIFI_STATUS_LINK_UP) ||
		(ap_info.link_status == WIFI_STATUS_GOTIP)) {
		snprintf(Text_String, sizeof(Text_String), "Wifi_Name:%s", ap_info.ssid);
		lv_label_set_text(guider_ui.screen_2_label_5, Text_String);
		snprintf(Text_String, sizeof(Text_String), "IP:%s", ipaddr_ntoa(&ipaddr));
		lv_label_set_text(guider_ui.screen_2_label_8, Text_String);
		snprintf(Text_String, sizeof(Text_String), "Gwaddr:%s", ipaddr_ntoa(&gw));
		lv_label_set_text(guider_ui.screen_2_label_6, Text_String);
		snprintf(Text_String, sizeof(Text_String), "MAC:%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		lv_label_set_text(guider_ui.screen_2_label_7, Text_String);
	} else {
		snprintf(Text_String, sizeof(Text_String), "Wifi_Name:");
		lv_label_set_text(guider_ui.screen_2_label_5, Text_String);
		snprintf(Text_String, sizeof(Text_String), "IP:");
		lv_label_set_text(guider_ui.screen_2_label_8, Text_String);
		snprintf(Text_String, sizeof(Text_String), "Gwaddr:");
		lv_label_set_text(guider_ui.screen_2_label_6, Text_String);
		//snprintf(Text_String, sizeof(Text_String), "MAC:%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		snprintf(Text_String, sizeof(Text_String), "MAC: wait connect");
		lv_label_set_text(guider_ui.screen_2_label_7, Text_String);
	}
	if(record_start_bias && record_time > 0) {
		record_time --;
	} else if(record_start_bias && record_play_time > 0) {
		if (play_start_bias == 0) {
			play_start_bias = 1;
			gui_custom_record_capture_stop();
			gui_custom_record_play();
		}
		record_play_time --;
	} else if(record_start_bias != 0) {
		snprintf(Text_String, sizeof(Text_String), "0:00");
		lv_label_set_text(guider_ui.screen_1_label_6, Text_String);
		lv_label_set_text(guider_ui.screen_1_label_7, Text_String);
		lv_slider_set_value(guider_ui.screen_1_bar_1, 0, LV_ANIM_OFF);
		lv_slider_set_value(guider_ui.screen_1_bar_2, 0, LV_ANIM_OFF);
		lv_obj_add_flag(guider_ui.screen_1_cont_2, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(guider_ui.screen_1_btn_9, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(guider_ui.screen_1_btn_tracks, LV_OBJ_FLAG_HIDDEN);
		gui_custom_record_stop_play();
		record_start_bias = 0;
		play_start_bias = 0;
	}
	if(record_start_bias !=0 && (record_play_time != 0 || record_time != 0)) {
		//label_6
		snprintf(Text_String, sizeof(Text_String), "0:%02d", record_time);
		lv_label_set_text(guider_ui.screen_1_label_6, Text_String);
		snprintf(Text_String, sizeof(Text_String), "0:%02d", record_play_time);
		lv_label_set_text(guider_ui.screen_1_label_7, Text_String);
		lv_slider_set_value(guider_ui.screen_1_bar_1, record_time, LV_ANIM_ON);
		lv_slider_set_value(guider_ui.screen_1_bar_2, record_play_time, LV_ANIM_ON);
	}
}

void events_init(lv_ui *ui)
{
}

static void screen_btn_2_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		//lv_disp_t * d = lv_obj_get_disp(lv_scr_act());
		//if (d->prev_scr == NULL && d->scr_to_load == NULL)
		//{
			lv_scr_load(guider_ui.screen_2);
		//}
		guider_ui.screen_del = true;
	}
		break;
	default:
		break;
	}
}

static void screen_btn_3_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		//lv_disp_t * d = lv_obj_get_disp(lv_scr_act());
		//if (d->prev_scr == NULL && d->scr_to_load == NULL)
		//{
			//if (guider_ui.screen_1_del == true) {
				lv_scr_load(guider_ui.screen_1);
			//}
		//}
		guider_ui.screen_del = true;
	}
		break;
	default:
		break;
	}
}

void events_init_screen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->screen_btn_2, screen_btn_2_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_btn_3, screen_btn_3_event_handler, LV_EVENT_ALL, NULL);
}

static void screen_2_btn_1_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		//lv_disp_t * d = lv_obj_get_disp(lv_scr_act());
		//if (d->prev_scr == NULL && d->scr_to_load == NULL)
		//{
			//if (guider_ui.screen_del == true) {
				lv_scr_load(guider_ui.screen);
			//}
		//}
		guider_ui.screen_2_del = true;
	}
		break;
	default:
		break;
	}
}

static void screen_2_btn_3_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_textarea_set_text(guider_ui.screen_2_ta_1,"");
		lv_textarea_set_text(guider_ui.screen_2_ta_2,"");
	}
		break;
	default:
		break;
	}
}

static void screen_2_btn_2_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_PRESSED:
	{
		//lv_textarea_get_text();
		const char * pSsid = lv_textarea_get_text(guider_ui.screen_2_ta_1);
		const char * pPasswd = lv_textarea_get_text(guider_ui.screen_2_ta_2);
		printf("connect pSsid [%s] , pPasswd [%s]\r\n", pSsid, pPasswd);
		if(strlen(pSsid) > 0) {
			WifiConnect((CVI_U8 *)pSsid, strlen(pSsid), (CVI_U8 *)pPasswd, strlen(pPasswd));
		}
	}
		break;
	default:
		break;
	}
}

void events_init_screen_2(lv_ui *ui)
{
	counter_timer = lv_timer_create(event_timer_cb, 1000, NULL);
	lv_obj_add_event_cb(ui->screen_2_btn_1, screen_2_btn_1_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_2_btn_3, screen_2_btn_3_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_2_btn_2, screen_2_btn_2_event_handler, LV_EVENT_ALL, NULL);
}

static void screen_1_cont_1_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		//lv_demo_music_play(0);
	}
		break;
	default:
		break;
	}
}

static void screen_1_imgbtn_play_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_RELEASED:
	{
				if (lv_obj_has_state(guider_ui.screen_1_imgbtn_play, LV_STATE_CHECKED))
				{
					lv_demo_music_resume();
				} else {
					lv_demo_music_pause();
				}
	}
		break;
	default:
		break;
	}
}

static void screen_1_img_icn_left_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		//lv_demo_music_album_next(false);
	}
		break;
	default:
		break;
	}
}

static void screen_1_img_icn_right_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		//lv_demo_music_album_next(true);
	}
		break;
	default:
		break;
	}
}

static void screen_1_btn_tracks_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
				//if (lv_obj_has_state(guider_ui.screen_1_imgbtn_play, LV_STATE_CHECKED) == 1)
				//{
				gui_guider_music_stop();
				//}
				if (tracks_is_up())
				{
					tracks_down();
				} else {
					tracks_up();
				}
	}
		break;
	default:
		break;
	}
}

static void screen_1_btn_9_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		//lv_disp_t * d = lv_obj_get_disp(lv_scr_act());
		//if (d->prev_scr == NULL && d->scr_to_load == NULL)
		//{
			//if (guider_ui.screen_del == true) {
				//if (lv_obj_has_state(guider_ui.screen_1_imgbtn_play, LV_STATE_CHECKED) == 1)
				//{
				gui_guider_music_stop();
				//}
				lv_scr_load(guider_ui.screen);
			//}
		//}
		guider_ui.screen_1_del = true;
	}
		break;
	default:
		break;
	}
}

static void screen_1_imgbtn_1_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		//开启录音
		lv_obj_add_flag(guider_ui.screen_1_btn_tracks, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(guider_ui.screen_1_cont_2, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(guider_ui.screen_1_btn_9, LV_OBJ_FLAG_HIDDEN);
		record_time = 10;
		record_play_time = 10;
		record_start_bias = 1;
		play_start_bias = 0;
		gui_custom_record_capture_start();
	}
		break;
	default:
		break;
	}
}

void events_init_screen_1(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->screen_1_btn_9, screen_1_btn_9_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_1_imgbtn_1, screen_1_imgbtn_1_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_1_imgbtn_play, screen_1_imgbtn_play_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_1_img_icn_left, screen_1_img_icn_left_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_1_img_icn_right, screen_1_img_icn_right_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_1_btn_tracks, screen_1_btn_tracks_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_1_cont_1, screen_1_cont_1_event_handler, LV_EVENT_ALL, NULL);
}