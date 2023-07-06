/*
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "guider_fonts.h"

typedef struct
{
	lv_obj_t *screen;
	bool screen_del;
	lv_obj_t *screen_main_show;
	lv_obj_t *screen_tileview_1;
	lv_obj_t *tileview_1_name_1;
	lv_obj_t *screen_label_1;
	lv_obj_t *screen_img_1;
	lv_obj_t *screen_btn_2;
	lv_obj_t *screen_btn_2_label;
	lv_obj_t *screen_btn_3;
	lv_obj_t *screen_btn_3_label;
	lv_obj_t *screen_1;
	bool screen_1_del;
	lv_obj_t *screen_1_cont_1;
	lv_obj_t *screen_1_label_2;
	lv_obj_t *screen_1_imgbtn_1;
	lv_obj_t *screen_1_imgbtn_1_label;
	lv_obj_t *screen_1_player;
	lv_obj_t *screen_1_img_album;
	lv_obj_t *screen_1_imgbtn_play;
	lv_obj_t *screen_1_imgbtn_play_label;
	lv_obj_t *screen_1_img_wave_top;
	lv_obj_t *screen_1_img_wave_bottom;
	lv_obj_t *screen_1_img_icn_rnd;
	lv_obj_t *screen_1_img_icn_loop;
	lv_obj_t *screen_1_img_icn_left;
	lv_obj_t *screen_1_img_icn_right;
	lv_obj_t *screen_1_slider_1;
	lv_obj_t *screen_1_img_icn_msg;
	lv_obj_t *screen_1_img_icn_heart;
	lv_obj_t *screen_1_img_icn_donwload;
	lv_obj_t *screen_1_img_icn_chart;
	lv_obj_t *screen_1_label_slider_time;
	lv_obj_t *screen_1_label_tracks;
	lv_obj_t *screen_1_btn_tracks;
	lv_obj_t *screen_1_btn_tracks_label;
	lv_obj_t *screen_1_label_title_music;
	lv_obj_t *screen_1_label_title_author;
	lv_obj_t *screen_1_cont_2;
	lv_obj_t *screen_1_label_7;
	lv_obj_t *screen_1_label_6;
	lv_obj_t *screen_1_label_5;
	lv_obj_t *screen_1_label_4;
	lv_obj_t *screen_1_label_3;
	lv_obj_t *screen_1_bar_2;
	lv_obj_t *screen_1_bar_1;
	lv_obj_t *screen_1_btn_9;
	lv_obj_t *screen_1_btn_9_label;
	lv_obj_t *screen_2;
	bool screen_2_del;
	lv_obj_t *screen_2_cont_1;
	lv_obj_t *screen_2_line_1;
	lv_obj_t *screen_2_btn_1;
	lv_obj_t *screen_2_btn_1_label;
	lv_obj_t *screen_2_cont_3;
	lv_obj_t *screen_2_img_3;
	lv_obj_t *screen_2_btn_3;
	lv_obj_t *screen_2_btn_3_label;
	lv_obj_t *screen_2_btn_2;
	lv_obj_t *screen_2_btn_2_label;
	lv_obj_t *screen_2_label_2;
	lv_obj_t *screen_2_ta_1;
	lv_obj_t *screen_2_label_1;
	lv_obj_t *screen_2_ta_2;
	lv_obj_t *screen_2_label_3;
	lv_obj_t *screen_2_cont_4;
	lv_obj_t *screen_2_label_8;
	lv_obj_t *screen_2_label_7;
	lv_obj_t *screen_2_label_6;
	lv_obj_t *screen_2_label_5;
	lv_obj_t *screen_2_label_4;
}lv_ui;

void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
extern lv_ui guider_ui;
void setup_scr_screen(lv_ui *ui);
void setup_scr_screen_1(lv_ui *ui);
void setup_scr_screen_2(lv_ui *ui);
LV_IMG_DECLARE(_mainbround_480x480);
LV_IMG_DECLARE(_995735691730d719011343f856b05665_64x64);
LV_IMG_DECLARE(_cover_1_105x105);
LV_IMG_DECLARE(_btn_play_alpha_42x42);
LV_IMG_DECLARE(_icn_slider_15x15);
LV_IMG_DECLARE(_wave_top_480x51);
LV_IMG_DECLARE(_wave_bottom_480x51);
LV_IMG_DECLARE(_btn_loop_15x15);
LV_IMG_DECLARE(_btn_rnd_15x15);
LV_IMG_DECLARE(_btn_prev_37x37);
LV_IMG_DECLARE(_btn_next_37x37);
LV_IMG_DECLARE(_btn_pause_alpha_42x42);
LV_IMG_DECLARE(_123123_alpha_128x128);
LV_IMG_DECLARE(_icn_chart_15x15);
LV_IMG_DECLARE(_icn_heart_15x15);
LV_IMG_DECLARE(_icn_download_15x15);
LV_IMG_DECLARE(_icn_chat_15x15);

#ifdef __cplusplus
}
#endif
#endif