/*
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "gui_guider_custom.h"

void setup_scr_screen_1(lv_ui *ui){

	//Write codes screen_1
	ui->screen_1 = lv_obj_create(NULL);
	lv_obj_set_scrollbar_mode(ui->screen_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_main_main_default
	static lv_style_t style_screen_1_main_main_default;
	if (style_screen_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_main_main_default);
	else
		lv_style_init(&style_screen_1_main_main_default);
	lv_style_set_bg_color(&style_screen_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_opa(&style_screen_1_main_main_default, 0);
	lv_obj_add_style(ui->screen_1, &style_screen_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_cont_1
	ui->screen_1_cont_1 = lv_obj_create(ui->screen_1);
	lv_obj_set_pos(ui->screen_1_cont_1, 0, 19);
	lv_obj_set_size(ui->screen_1_cont_1, 480, 460);
	lv_obj_set_scrollbar_mode(ui->screen_1_cont_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_cont_1_main_main_default
	static lv_style_t style_screen_1_cont_1_main_main_default;
	if (style_screen_1_cont_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_cont_1_main_main_default);
	else
		lv_style_init(&style_screen_1_cont_1_main_main_default);
	lv_style_set_radius(&style_screen_1_cont_1_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_cont_1_main_main_default, lv_color_make(0x34, 0x32, 0x47));
	lv_style_set_bg_grad_color(&style_screen_1_cont_1_main_main_default, lv_color_make(0x34, 0x32, 0x47));
	lv_style_set_bg_grad_dir(&style_screen_1_cont_1_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_cont_1_main_main_default, 255);
	lv_style_set_border_color(&style_screen_1_cont_1_main_main_default, lv_color_make(0x4a, 0x52, 0x59));
	lv_style_set_border_width(&style_screen_1_cont_1_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_1_cont_1_main_main_default, 255);
	lv_style_set_pad_left(&style_screen_1_cont_1_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_cont_1_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_cont_1_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_cont_1_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_cont_1, &style_screen_1_cont_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_2
	ui->screen_1_label_2 = lv_label_create(ui->screen_1_cont_1);
	lv_obj_set_pos(ui->screen_1_label_2, 113, 71);
	lv_obj_set_size(ui->screen_1_label_2, 241, 26);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_2, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_2, "Record Please   Press ");
	lv_label_set_long_mode(ui->screen_1_label_2, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_2_main_main_default
	static lv_style_t style_screen_1_label_2_main_main_default;
	if (style_screen_1_label_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_2_main_main_default);
	else
		lv_style_init(&style_screen_1_label_2_main_main_default);
	lv_style_set_radius(&style_screen_1_label_2_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_label_2_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_2_main_main_default, lv_color_make(0xfa, 0xf9, 0xf9));
	lv_style_set_text_font(&style_screen_1_label_2_main_main_default, &lv_font_arial_12);
	lv_style_set_text_letter_space(&style_screen_1_label_2_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_1_label_2_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_2_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_2_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_2_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_2_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_2_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_2, &style_screen_1_label_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_imgbtn_1
	ui->screen_1_imgbtn_1 = lv_imgbtn_create(ui->screen_1_cont_1);
	lv_obj_set_pos(ui->screen_1_imgbtn_1, 171, 132);
	lv_obj_set_size(ui->screen_1_imgbtn_1, 128, 128);
	lv_obj_set_scrollbar_mode(ui->screen_1_imgbtn_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_imgbtn_1_main_main_default
	static lv_style_t style_screen_1_imgbtn_1_main_main_default;
	if (style_screen_1_imgbtn_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_imgbtn_1_main_main_default);
	else
		lv_style_init(&style_screen_1_imgbtn_1_main_main_default);
	lv_style_set_text_color(&style_screen_1_imgbtn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_align(&style_screen_1_imgbtn_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_img_recolor(&style_screen_1_imgbtn_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_1_imgbtn_1_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_1_imgbtn_1_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_imgbtn_1, &style_screen_1_imgbtn_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_PRESSED for style_screen_1_imgbtn_1_main_main_pressed
	static lv_style_t style_screen_1_imgbtn_1_main_main_pressed;
	if (style_screen_1_imgbtn_1_main_main_pressed.prop_cnt > 1)
		lv_style_reset(&style_screen_1_imgbtn_1_main_main_pressed);
	else
		lv_style_init(&style_screen_1_imgbtn_1_main_main_pressed);
	lv_style_set_text_color(&style_screen_1_imgbtn_1_main_main_pressed, lv_color_make(0xFF, 0x33, 0xFF));
	lv_style_set_text_align(&style_screen_1_imgbtn_1_main_main_pressed, LV_TEXT_ALIGN_CENTER);
	lv_style_set_img_recolor(&style_screen_1_imgbtn_1_main_main_pressed, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_img_recolor_opa(&style_screen_1_imgbtn_1_main_main_pressed, 0);
	lv_style_set_img_opa(&style_screen_1_imgbtn_1_main_main_pressed, 255);
	lv_obj_add_style(ui->screen_1_imgbtn_1, &style_screen_1_imgbtn_1_main_main_pressed, LV_PART_MAIN|LV_STATE_PRESSED);

	//Write style state: LV_STATE_CHECKED for style_screen_1_imgbtn_1_main_main_checked
	static lv_style_t style_screen_1_imgbtn_1_main_main_checked;
	if (style_screen_1_imgbtn_1_main_main_checked.prop_cnt > 1)
		lv_style_reset(&style_screen_1_imgbtn_1_main_main_checked);
	else
		lv_style_init(&style_screen_1_imgbtn_1_main_main_checked);
	lv_style_set_text_color(&style_screen_1_imgbtn_1_main_main_checked, lv_color_make(0xFF, 0x33, 0xFF));
	lv_style_set_text_align(&style_screen_1_imgbtn_1_main_main_checked, LV_TEXT_ALIGN_CENTER);
	lv_style_set_img_recolor(&style_screen_1_imgbtn_1_main_main_checked, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_img_recolor_opa(&style_screen_1_imgbtn_1_main_main_checked, 0);
	lv_style_set_img_opa(&style_screen_1_imgbtn_1_main_main_checked, 255);
	lv_obj_add_style(ui->screen_1_imgbtn_1, &style_screen_1_imgbtn_1_main_main_checked, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_imgbtn_set_src(ui->screen_1_imgbtn_1, LV_IMGBTN_STATE_RELEASED, NULL, &_123123_alpha_128x128, NULL);
	lv_imgbtn_set_src(ui->screen_1_imgbtn_1, LV_IMGBTN_STATE_PRESSED, NULL, &_123123_alpha_128x128, NULL);
	lv_imgbtn_set_src(ui->screen_1_imgbtn_1, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_123123_alpha_128x128, NULL);
	lv_imgbtn_set_src(ui->screen_1_imgbtn_1, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &_123123_alpha_128x128, NULL);
	lv_obj_add_flag(ui->screen_1_imgbtn_1, LV_OBJ_FLAG_CHECKABLE);

	//Write codes screen_1_player
	ui->screen_1_player = lv_obj_create(ui->screen_1);
	lv_obj_set_pos(ui->screen_1_player, 0, 0);
	lv_obj_set_size(ui->screen_1_player, 480, 480);
	lv_obj_set_scrollbar_mode(ui->screen_1_player, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_player_main_main_default
	static lv_style_t style_screen_1_player_main_main_default;
	if (style_screen_1_player_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_player_main_main_default);
	else
		lv_style_init(&style_screen_1_player_main_main_default);
	lv_style_set_radius(&style_screen_1_player_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_player_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_color(&style_screen_1_player_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_dir(&style_screen_1_player_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_player_main_main_default, 255);
	lv_style_set_border_color(&style_screen_1_player_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_1_player_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_1_player_main_main_default, 255);
	lv_style_set_pad_left(&style_screen_1_player_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_player_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_player_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_player_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_player, &style_screen_1_player_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_img_album
	ui->screen_1_img_album = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_album, 187, 148);
	lv_obj_set_size(ui->screen_1_img_album, 105, 105);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_album, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_img_album_main_main_default
	static lv_style_t style_screen_1_img_album_main_main_default;
	if (style_screen_1_img_album_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_img_album_main_main_default);
	else
		lv_style_init(&style_screen_1_img_album_main_main_default);
	lv_style_set_img_recolor(&style_screen_1_img_album_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_1_img_album_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_1_img_album_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_img_album, &style_screen_1_img_album_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_1_img_album, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_album,&_cover_1_105x105);
	lv_img_set_pivot(ui->screen_1_img_album, 0,0);
	lv_img_set_angle(ui->screen_1_img_album, 0);

	//Write codes screen_1_imgbtn_play
	ui->screen_1_imgbtn_play = lv_imgbtn_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_imgbtn_play, 212, 351);
	lv_obj_set_size(ui->screen_1_imgbtn_play, 42, 42);
	lv_obj_set_scrollbar_mode(ui->screen_1_imgbtn_play, LV_SCROLLBAR_MODE_OFF);
	lv_imgbtn_set_src(ui->screen_1_imgbtn_play, LV_IMGBTN_STATE_RELEASED, NULL, &_btn_play_alpha_42x42, NULL);
	lv_imgbtn_set_src(ui->screen_1_imgbtn_play, LV_IMGBTN_STATE_PRESSED, NULL, &_btn_play_alpha_42x42, NULL);
	lv_imgbtn_set_src(ui->screen_1_imgbtn_play, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &_btn_pause_alpha_42x42, NULL);
	lv_imgbtn_set_src(ui->screen_1_imgbtn_play, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &_btn_pause_alpha_42x42, NULL);
	lv_obj_add_flag(ui->screen_1_imgbtn_play, LV_OBJ_FLAG_CHECKABLE);

	//Write codes screen_1_img_wave_top
	ui->screen_1_img_wave_top = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_wave_top, 0, 0);
	lv_obj_set_size(ui->screen_1_img_wave_top, 480, 51);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_wave_top, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_img_wave_top_main_main_default
	static lv_style_t style_screen_1_img_wave_top_main_main_default;
	if (style_screen_1_img_wave_top_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_img_wave_top_main_main_default);
	else
		lv_style_init(&style_screen_1_img_wave_top_main_main_default);
	lv_style_set_img_recolor(&style_screen_1_img_wave_top_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_1_img_wave_top_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_1_img_wave_top_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_img_wave_top, &style_screen_1_img_wave_top_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_1_img_wave_top, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_wave_top,&_wave_top_480x51);
	lv_img_set_pivot(ui->screen_1_img_wave_top, 0,0);
	lv_img_set_angle(ui->screen_1_img_wave_top, 0);

	//Write codes screen_1_img_wave_bottom
	ui->screen_1_img_wave_bottom = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_wave_bottom, 0, 428);
	lv_obj_set_size(ui->screen_1_img_wave_bottom, 480, 51);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_wave_bottom, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_img_wave_bottom_main_main_default
	static lv_style_t style_screen_1_img_wave_bottom_main_main_default;
	if (style_screen_1_img_wave_bottom_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_img_wave_bottom_main_main_default);
	else
		lv_style_init(&style_screen_1_img_wave_bottom_main_main_default);
	lv_style_set_img_recolor(&style_screen_1_img_wave_bottom_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_1_img_wave_bottom_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_1_img_wave_bottom_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_img_wave_bottom, &style_screen_1_img_wave_bottom_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_1_img_wave_bottom, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_wave_bottom,&_wave_bottom_480x51);
	lv_img_set_pivot(ui->screen_1_img_wave_bottom, 0,0);
	lv_img_set_angle(ui->screen_1_img_wave_bottom, 0);

	//Write codes screen_1_img_icn_rnd
	ui->screen_1_img_icn_rnd = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_icn_rnd, 42, 384);
	lv_obj_set_size(ui->screen_1_img_icn_rnd, 15, 15);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_icn_rnd, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_flag(ui->screen_1_img_icn_rnd, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_icn_rnd,&_btn_rnd_15x15);
	lv_img_set_pivot(ui->screen_1_img_icn_rnd, 0,0);
	lv_img_set_angle(ui->screen_1_img_icn_rnd, 0);

	//Write codes screen_1_img_icn_loop
	ui->screen_1_img_icn_loop = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_icn_loop, 421, 384);
	lv_obj_set_size(ui->screen_1_img_icn_loop, 15, 15);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_icn_loop, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_flag(ui->screen_1_img_icn_loop, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_icn_loop,&_btn_loop_15x15);
	lv_img_set_pivot(ui->screen_1_img_icn_loop, 0,0);
	lv_img_set_angle(ui->screen_1_img_icn_loop, 0);

	//Write codes screen_1_img_icn_left
	ui->screen_1_img_icn_left = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_icn_left, 122, 360);
	lv_obj_set_size(ui->screen_1_img_icn_left, 37, 37);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_icn_left, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_flag(ui->screen_1_img_icn_left, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_icn_left,&_btn_prev_37x37);
	lv_img_set_pivot(ui->screen_1_img_icn_left, 0,0);
	lv_img_set_angle(ui->screen_1_img_icn_left, 0);

	//Write codes screen_1_img_icn_right
	ui->screen_1_img_icn_right = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_icn_right, 318, 360);
	lv_obj_set_size(ui->screen_1_img_icn_right, 37, 37);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_icn_right, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_flag(ui->screen_1_img_icn_right, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_icn_right,&_btn_next_37x37);
	lv_img_set_pivot(ui->screen_1_img_icn_right, 0,0);
	lv_img_set_angle(ui->screen_1_img_icn_right, 0);

	//Write codes screen_1_slider_1
	ui->screen_1_slider_1 = lv_slider_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_slider_1, 42, 441);
	lv_obj_set_size(ui->screen_1_slider_1, 342, 1);
	lv_obj_set_scrollbar_mode(ui->screen_1_slider_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_slider_1_main_main_default
	static lv_style_t style_screen_1_slider_1_main_main_default;
	if (style_screen_1_slider_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_slider_1_main_main_default);
	else
		lv_style_init(&style_screen_1_slider_1_main_main_default);
	lv_style_set_radius(&style_screen_1_slider_1_main_main_default, 50);
	lv_style_set_bg_color(&style_screen_1_slider_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_slider_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_slider_1_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_slider_1_main_main_default, 100);
	lv_style_set_outline_color(&style_screen_1_slider_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_outline_width(&style_screen_1_slider_1_main_main_default, 0);
	lv_style_set_outline_opa(&style_screen_1_slider_1_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_slider_1, &style_screen_1_slider_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_slider_1_main_indicator_default
	static lv_style_t style_screen_1_slider_1_main_indicator_default;
	if (style_screen_1_slider_1_main_indicator_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_slider_1_main_indicator_default);
	else
		lv_style_init(&style_screen_1_slider_1_main_indicator_default);
	lv_style_set_radius(&style_screen_1_slider_1_main_indicator_default, 50);
	lv_style_set_bg_color(&style_screen_1_slider_1_main_indicator_default, lv_color_make(0x56, 0x91, 0xf8));
	lv_style_set_bg_grad_color(&style_screen_1_slider_1_main_indicator_default, lv_color_make(0xa6, 0x66, 0xf1));
	lv_style_set_bg_grad_dir(&style_screen_1_slider_1_main_indicator_default, LV_GRAD_DIR_HOR);
	lv_style_set_bg_opa(&style_screen_1_slider_1_main_indicator_default, 255);
	lv_obj_add_style(ui->screen_1_slider_1, &style_screen_1_slider_1_main_indicator_default, LV_PART_INDICATOR|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_slider_1_main_knob_default
	static lv_style_t style_screen_1_slider_1_main_knob_default;
	if (style_screen_1_slider_1_main_knob_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_slider_1_main_knob_default);
	else
		lv_style_init(&style_screen_1_slider_1_main_knob_default);
	lv_style_set_radius(&style_screen_1_slider_1_main_knob_default, 50);
	lv_style_set_bg_color(&style_screen_1_slider_1_main_knob_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_slider_1_main_knob_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_slider_1_main_knob_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_slider_1_main_knob_default, 0);
	lv_obj_add_style(ui->screen_1_slider_1, &style_screen_1_slider_1_main_knob_default, LV_PART_KNOB|LV_STATE_DEFAULT);
	lv_slider_set_range(ui->screen_1_slider_1,0, 100);
	lv_slider_set_value(ui->screen_1_slider_1,0,false);

	//Write codes screen_1_img_icn_msg
	ui->screen_1_img_icn_msg = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_icn_msg, 381, 72);
	lv_obj_set_size(ui->screen_1_img_icn_msg, 15, 15);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_icn_msg, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_img_icn_msg_main_main_default
	static lv_style_t style_screen_1_img_icn_msg_main_main_default;
	if (style_screen_1_img_icn_msg_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_img_icn_msg_main_main_default);
	else
		lv_style_init(&style_screen_1_img_icn_msg_main_main_default);
	lv_style_set_img_recolor(&style_screen_1_img_icn_msg_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_1_img_icn_msg_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_1_img_icn_msg_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_img_icn_msg, &style_screen_1_img_icn_msg_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_1_img_icn_msg, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_icn_msg,&_icn_chat_15x15);
	lv_img_set_pivot(ui->screen_1_img_icn_msg, 0,0);
	lv_img_set_angle(ui->screen_1_img_icn_msg, 0);

	//Write codes screen_1_img_icn_heart
	ui->screen_1_img_icn_heart = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_icn_heart, 84, 72);
	lv_obj_set_size(ui->screen_1_img_icn_heart, 15, 15);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_icn_heart, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_img_icn_heart_main_main_default
	static lv_style_t style_screen_1_img_icn_heart_main_main_default;
	if (style_screen_1_img_icn_heart_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_img_icn_heart_main_main_default);
	else
		lv_style_init(&style_screen_1_img_icn_heart_main_main_default);
	lv_style_set_img_recolor(&style_screen_1_img_icn_heart_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_1_img_icn_heart_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_1_img_icn_heart_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_img_icn_heart, &style_screen_1_img_icn_heart_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_1_img_icn_heart, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_icn_heart,&_icn_heart_15x15);
	lv_img_set_pivot(ui->screen_1_img_icn_heart, 0,0);
	lv_img_set_angle(ui->screen_1_img_icn_heart, 0);

	//Write codes screen_1_img_icn_donwload
	ui->screen_1_img_icn_donwload = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_icn_donwload, 282, 72);
	lv_obj_set_size(ui->screen_1_img_icn_donwload, 15, 15);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_icn_donwload, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_img_icn_donwload_main_main_default
	static lv_style_t style_screen_1_img_icn_donwload_main_main_default;
	if (style_screen_1_img_icn_donwload_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_img_icn_donwload_main_main_default);
	else
		lv_style_init(&style_screen_1_img_icn_donwload_main_main_default);
	lv_style_set_img_recolor(&style_screen_1_img_icn_donwload_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_1_img_icn_donwload_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_1_img_icn_donwload_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_img_icn_donwload, &style_screen_1_img_icn_donwload_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_1_img_icn_donwload, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_icn_donwload,&_icn_download_15x15);
	lv_img_set_pivot(ui->screen_1_img_icn_donwload, 0,0);
	lv_img_set_angle(ui->screen_1_img_icn_donwload, 0);

	//Write codes screen_1_img_icn_chart
	ui->screen_1_img_icn_chart = lv_img_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_img_icn_chart, 183, 72);
	lv_obj_set_size(ui->screen_1_img_icn_chart, 15, 15);
	lv_obj_set_scrollbar_mode(ui->screen_1_img_icn_chart, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_img_icn_chart_main_main_default
	static lv_style_t style_screen_1_img_icn_chart_main_main_default;
	if (style_screen_1_img_icn_chart_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_img_icn_chart_main_main_default);
	else
		lv_style_init(&style_screen_1_img_icn_chart_main_main_default);
	lv_style_set_img_recolor(&style_screen_1_img_icn_chart_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_1_img_icn_chart_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_1_img_icn_chart_main_main_default, 255);
	lv_obj_add_style(ui->screen_1_img_icn_chart, &style_screen_1_img_icn_chart_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_1_img_icn_chart, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_1_img_icn_chart,&_icn_chart_15x15);
	lv_img_set_pivot(ui->screen_1_img_icn_chart, 0,0);
	lv_img_set_angle(ui->screen_1_img_icn_chart, 0);

	//Write codes screen_1_label_slider_time
	ui->screen_1_label_slider_time = lv_label_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_label_slider_time, 420, 432);
	lv_obj_set_size(ui->screen_1_label_slider_time, 36, 19);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_slider_time, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_slider_time, "0:00");
	lv_label_set_long_mode(ui->screen_1_label_slider_time, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_slider_time_main_main_default
	static lv_style_t style_screen_1_label_slider_time_main_main_default;
	if (style_screen_1_label_slider_time_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_slider_time_main_main_default);
	else
		lv_style_init(&style_screen_1_label_slider_time_main_main_default);
	lv_style_set_radius(&style_screen_1_label_slider_time_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_slider_time_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_slider_time_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_slider_time_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_label_slider_time_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_slider_time_main_main_default, lv_color_make(0x8a, 0x86, 0xb8));
	lv_style_set_text_font(&style_screen_1_label_slider_time_main_main_default, &lv_font_arial_12);
	lv_style_set_text_letter_space(&style_screen_1_label_slider_time_main_main_default, 0);
	lv_style_set_text_line_space(&style_screen_1_label_slider_time_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_slider_time_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_slider_time_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_slider_time_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_slider_time_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_slider_time_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_slider_time, &style_screen_1_label_slider_time_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_tracks
	ui->screen_1_label_tracks = lv_label_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_label_tracks, 234, 476);
	lv_obj_set_size(ui->screen_1_label_tracks, 12, 1);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_tracks, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_tracks, "");
	lv_label_set_long_mode(ui->screen_1_label_tracks, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_tracks_main_main_default
	static lv_style_t style_screen_1_label_tracks_main_main_default;
	if (style_screen_1_label_tracks_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_tracks_main_main_default);
	else
		lv_style_init(&style_screen_1_label_tracks_main_main_default);
	lv_style_set_radius(&style_screen_1_label_tracks_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_tracks_main_main_default, lv_color_make(0x8a, 0x86, 0xb8));
	lv_style_set_bg_grad_color(&style_screen_1_label_tracks_main_main_default, lv_color_make(0x8a, 0x86, 0xb8));
	lv_style_set_bg_grad_dir(&style_screen_1_label_tracks_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_label_tracks_main_main_default, 229);
	lv_style_set_text_color(&style_screen_1_label_tracks_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_letter_space(&style_screen_1_label_tracks_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_1_label_tracks_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_tracks_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_tracks_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_tracks_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_tracks_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_tracks_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_tracks, &style_screen_1_label_tracks_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_btn_tracks
	ui->screen_1_btn_tracks = lv_btn_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_btn_tracks, 210, 451);
	lv_obj_set_size(ui->screen_1_btn_tracks, 60, 28);
	lv_obj_set_scrollbar_mode(ui->screen_1_btn_tracks, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_btn_tracks_main_main_default
	static lv_style_t style_screen_1_btn_tracks_main_main_default;
	if (style_screen_1_btn_tracks_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_btn_tracks_main_main_default);
	else
		lv_style_init(&style_screen_1_btn_tracks_main_main_default);
	lv_style_set_radius(&style_screen_1_btn_tracks_main_main_default, 5);
	lv_style_set_bg_color(&style_screen_1_btn_tracks_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_btn_tracks_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_btn_tracks_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_btn_tracks_main_main_default, 0);
	lv_style_set_border_color(&style_screen_1_btn_tracks_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_1_btn_tracks_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_1_btn_tracks_main_main_default, 255);
	lv_style_set_text_color(&style_screen_1_btn_tracks_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_1_btn_tracks_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_align(&style_screen_1_btn_tracks_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_1_btn_tracks, &style_screen_1_btn_tracks_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_1_btn_tracks_label = lv_label_create(ui->screen_1_btn_tracks);
	lv_label_set_text(ui->screen_1_btn_tracks_label, "ALL TRACK");
	lv_obj_set_style_pad_all(ui->screen_1_btn_tracks, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->screen_1_btn_tracks_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes screen_1_label_title_music
	ui->screen_1_label_title_music = lv_label_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_label_title_music, 150, 8);
	lv_obj_set_size(ui->screen_1_label_title_music, 180, 31);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_title_music, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_title_music, "Waiting for true love");
	lv_label_set_long_mode(ui->screen_1_label_title_music, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_title_music_main_main_default
	static lv_style_t style_screen_1_label_title_music_main_main_default;
	if (style_screen_1_label_title_music_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_title_music_main_main_default);
	else
		lv_style_init(&style_screen_1_label_title_music_main_main_default);
	lv_style_set_radius(&style_screen_1_label_title_music_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_title_music_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_title_music_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_title_music_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_label_title_music_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_title_music_main_main_default, lv_color_make(0x50, 0x4d, 0x6d));
	lv_style_set_text_font(&style_screen_1_label_title_music_main_main_default, &lv_font_arial_14);
	lv_style_set_text_letter_space(&style_screen_1_label_title_music_main_main_default, 0);
	lv_style_set_text_line_space(&style_screen_1_label_title_music_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_title_music_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_title_music_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_title_music_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_title_music_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_title_music_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_title_music, &style_screen_1_label_title_music_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_title_author
	ui->screen_1_label_title_author = lv_label_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_label_title_author, 150, 38);
	lv_obj_set_size(ui->screen_1_label_title_author, 180, 31);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_title_author, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_title_author, "The John Smith Band");
	lv_label_set_long_mode(ui->screen_1_label_title_author, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_title_author_main_main_default
	static lv_style_t style_screen_1_label_title_author_main_main_default;
	if (style_screen_1_label_title_author_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_title_author_main_main_default);
	else
		lv_style_init(&style_screen_1_label_title_author_main_main_default);
	lv_style_set_radius(&style_screen_1_label_title_author_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_title_author_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_title_author_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_title_author_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_1_label_title_author_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_title_author_main_main_default, lv_color_make(0x50, 0x4d, 0x6d));
	lv_style_set_text_font(&style_screen_1_label_title_author_main_main_default, &lv_font_arial_10);
	lv_style_set_text_letter_space(&style_screen_1_label_title_author_main_main_default, 0);
	lv_style_set_text_line_space(&style_screen_1_label_title_author_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_title_author_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_title_author_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_title_author_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_title_author_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_title_author_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_title_author, &style_screen_1_label_title_author_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_cont_2
	ui->screen_1_cont_2 = lv_obj_create(ui->screen_1);
	lv_obj_set_pos(ui->screen_1_cont_2, 78, 114);
	lv_obj_set_size(ui->screen_1_cont_2, 328, 257);
	lv_obj_set_scrollbar_mode(ui->screen_1_cont_2, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_flag(ui->screen_1_cont_2, LV_OBJ_FLAG_HIDDEN);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_cont_2_main_main_default
	static lv_style_t style_screen_1_cont_2_main_main_default;
	if (style_screen_1_cont_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_cont_2_main_main_default);
	else
		lv_style_init(&style_screen_1_cont_2_main_main_default);
	lv_style_set_radius(&style_screen_1_cont_2_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_cont_2_main_main_default, lv_color_make(0xee, 0xee, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_cont_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_cont_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_cont_2_main_main_default, 255);
	lv_style_set_border_color(&style_screen_1_cont_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_1_cont_2_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_1_cont_2_main_main_default, 255);
	lv_style_set_pad_left(&style_screen_1_cont_2_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_cont_2_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_cont_2_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_cont_2_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_cont_2, &style_screen_1_cont_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_7
	ui->screen_1_label_7 = lv_label_create(ui->screen_1_cont_2);
	lv_obj_set_pos(ui->screen_1_label_7, 274, 172);
	lv_obj_set_size(ui->screen_1_label_7, 51, 20);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_7, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_7, "0:00");
	lv_label_set_long_mode(ui->screen_1_label_7, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_7_main_main_default
	static lv_style_t style_screen_1_label_7_main_main_default;
	if (style_screen_1_label_7_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_7_main_main_default);
	else
		lv_style_init(&style_screen_1_label_7_main_main_default);
	lv_style_set_radius(&style_screen_1_label_7_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_7_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_7_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_7_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_label_7_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_7_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_1_label_7_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_1_label_7_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_1_label_7_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_7_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_7_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_7_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_7_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_7_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_7, &style_screen_1_label_7_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_6
	ui->screen_1_label_6 = lv_label_create(ui->screen_1_cont_2);
	lv_obj_set_pos(ui->screen_1_label_6, 275, 125);
	lv_obj_set_size(ui->screen_1_label_6, 51, 20);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_6, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_6, "0:00");
	lv_label_set_long_mode(ui->screen_1_label_6, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_6_main_main_default
	static lv_style_t style_screen_1_label_6_main_main_default;
	if (style_screen_1_label_6_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_6_main_main_default);
	else
		lv_style_init(&style_screen_1_label_6_main_main_default);
	lv_style_set_radius(&style_screen_1_label_6_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_6_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_6_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_6_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_label_6_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_6_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_1_label_6_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_1_label_6_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_1_label_6_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_6_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_6_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_6_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_6_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_6_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_6, &style_screen_1_label_6_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_5
	ui->screen_1_label_5 = lv_label_create(ui->screen_1_cont_2);
	lv_obj_set_pos(ui->screen_1_label_5, 11, 171);
	lv_obj_set_size(ui->screen_1_label_5, 51, 18);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_5, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_5, "Play");
	lv_label_set_long_mode(ui->screen_1_label_5, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_5_main_main_default
	static lv_style_t style_screen_1_label_5_main_main_default;
	if (style_screen_1_label_5_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_5_main_main_default);
	else
		lv_style_init(&style_screen_1_label_5_main_main_default);
	lv_style_set_radius(&style_screen_1_label_5_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_5_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_5_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_5_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_label_5_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_5_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_1_label_5_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_1_label_5_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_1_label_5_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_5_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_5_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_5_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_5_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_5_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_5, &style_screen_1_label_5_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_4
	ui->screen_1_label_4 = lv_label_create(ui->screen_1_cont_2);
	lv_obj_set_pos(ui->screen_1_label_4, 9, 127);
	lv_obj_set_size(ui->screen_1_label_4, 54, 21);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_4, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_4, "Record");
	lv_label_set_long_mode(ui->screen_1_label_4, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_4_main_main_default
	static lv_style_t style_screen_1_label_4_main_main_default;
	if (style_screen_1_label_4_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_4_main_main_default);
	else
		lv_style_init(&style_screen_1_label_4_main_main_default);
	lv_style_set_radius(&style_screen_1_label_4_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_4_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_label_4_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_4_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_1_label_4_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_1_label_4_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_1_label_4_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_4_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_4_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_4_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_4_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_4_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_4, &style_screen_1_label_4_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_label_3
	ui->screen_1_label_3 = lv_label_create(ui->screen_1_cont_2);
	lv_obj_set_pos(ui->screen_1_label_3, 76, 45);
	lv_obj_set_size(ui->screen_1_label_3, 181, 23);
	lv_obj_set_scrollbar_mode(ui->screen_1_label_3, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_1_label_3, "Record 10S and Play");
	lv_label_set_long_mode(ui->screen_1_label_3, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_label_3_main_main_default
	static lv_style_t style_screen_1_label_3_main_main_default;
	if (style_screen_1_label_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_label_3_main_main_default);
	else
		lv_style_init(&style_screen_1_label_3_main_main_default);
	lv_style_set_radius(&style_screen_1_label_3_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_1_label_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_label_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_label_3_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_label_3_main_main_default, 0);
	lv_style_set_text_color(&style_screen_1_label_3_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_1_label_3_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_1_label_3_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_1_label_3_main_main_default, 0);
	lv_style_set_text_align(&style_screen_1_label_3_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_1_label_3_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_1_label_3_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_1_label_3_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_1_label_3_main_main_default, 0);
	lv_obj_add_style(ui->screen_1_label_3, &style_screen_1_label_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_1_bar_2
	ui->screen_1_bar_2 = lv_bar_create(ui->screen_1_cont_2);
	lv_obj_set_pos(ui->screen_1_bar_2, 67, 169);
	lv_obj_set_size(ui->screen_1_bar_2, 205, 20);
	lv_obj_set_scrollbar_mode(ui->screen_1_bar_2, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_bar_2_main_main_default
	static lv_style_t style_screen_1_bar_2_main_main_default;
	if (style_screen_1_bar_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_bar_2_main_main_default);
	else
		lv_style_init(&style_screen_1_bar_2_main_main_default);
	lv_style_set_radius(&style_screen_1_bar_2_main_main_default, 10);
	lv_style_set_bg_color(&style_screen_1_bar_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_bar_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_bar_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_bar_2_main_main_default, 60);
	lv_obj_add_style(ui->screen_1_bar_2, &style_screen_1_bar_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_bar_2_main_indicator_default
	static lv_style_t style_screen_1_bar_2_main_indicator_default;
	if (style_screen_1_bar_2_main_indicator_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_bar_2_main_indicator_default);
	else
		lv_style_init(&style_screen_1_bar_2_main_indicator_default);
	lv_style_set_radius(&style_screen_1_bar_2_main_indicator_default, 10);
	lv_style_set_bg_color(&style_screen_1_bar_2_main_indicator_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_bar_2_main_indicator_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_bar_2_main_indicator_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_bar_2_main_indicator_default, 255);
	lv_obj_add_style(ui->screen_1_bar_2, &style_screen_1_bar_2_main_indicator_default, LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_anim_time(ui->screen_1_bar_2, 10, 0);
	lv_bar_set_mode(ui->screen_1_bar_2, LV_BAR_MODE_NORMAL);
	lv_bar_set_value(ui->screen_1_bar_2, 0, LV_ANIM_OFF);
	lv_bar_set_range(ui->screen_1_bar_2, 0, 10);
	//Write codes screen_1_bar_1
	ui->screen_1_bar_1 = lv_bar_create(ui->screen_1_cont_2);
	lv_obj_set_pos(ui->screen_1_bar_1, 66, 124);
	lv_obj_set_size(ui->screen_1_bar_1, 205, 20);
	lv_obj_set_scrollbar_mode(ui->screen_1_bar_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_bar_1_main_main_default
	static lv_style_t style_screen_1_bar_1_main_main_default;
	if (style_screen_1_bar_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_bar_1_main_main_default);
	else
		lv_style_init(&style_screen_1_bar_1_main_main_default);
	lv_style_set_radius(&style_screen_1_bar_1_main_main_default, 10);
	lv_style_set_bg_color(&style_screen_1_bar_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_bar_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_bar_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_bar_1_main_main_default, 60);
	lv_obj_add_style(ui->screen_1_bar_1, &style_screen_1_bar_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_bar_1_main_indicator_default
	static lv_style_t style_screen_1_bar_1_main_indicator_default;
	if (style_screen_1_bar_1_main_indicator_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_bar_1_main_indicator_default);
	else
		lv_style_init(&style_screen_1_bar_1_main_indicator_default);
	lv_style_set_radius(&style_screen_1_bar_1_main_indicator_default, 10);
	lv_style_set_bg_color(&style_screen_1_bar_1_main_indicator_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_bar_1_main_indicator_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_bar_1_main_indicator_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_bar_1_main_indicator_default, 255);
	lv_obj_add_style(ui->screen_1_bar_1, &style_screen_1_bar_1_main_indicator_default, LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_anim_time(ui->screen_1_bar_1, 10, 0);
	lv_bar_set_mode(ui->screen_1_bar_1, LV_BAR_MODE_NORMAL);
	lv_bar_set_value(ui->screen_1_bar_1, 0, LV_ANIM_OFF);
	lv_bar_set_range(ui->screen_1_bar_1, 0, 10);
	//Write codes screen_1_btn_9
	ui->screen_1_btn_9 = lv_btn_create(ui->screen_1_player);
	lv_obj_set_pos(ui->screen_1_btn_9, 4, 1);
	lv_obj_set_size(ui->screen_1_btn_9, 100, 50);
	lv_obj_set_scrollbar_mode(ui->screen_1_btn_9, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_1_btn_9_main_main_default
	static lv_style_t style_screen_1_btn_9_main_main_default;
	if (style_screen_1_btn_9_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_1_btn_9_main_main_default);
	else
		lv_style_init(&style_screen_1_btn_9_main_main_default);
	lv_style_set_radius(&style_screen_1_btn_9_main_main_default, 5);
	lv_style_set_bg_color(&style_screen_1_btn_9_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_1_btn_9_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_1_btn_9_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_1_btn_9_main_main_default, 0);
	lv_style_set_border_color(&style_screen_1_btn_9_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_1_btn_9_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_1_btn_9_main_main_default, 255);
	lv_style_set_text_color(&style_screen_1_btn_9_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_1_btn_9_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_align(&style_screen_1_btn_9_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_1_btn_9, &style_screen_1_btn_9_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_1_btn_9_label = lv_label_create(ui->screen_1_btn_9);
	lv_label_set_text(ui->screen_1_btn_9_label, "返回主界面");
	lv_obj_set_style_pad_all(ui->screen_1_btn_9, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->screen_1_btn_9_label, LV_ALIGN_CENTER, 0, 0);
	//Init events for screen
	events_init_screen_1(ui);
	gui_custom_init(ui);
}