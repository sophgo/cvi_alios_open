/*
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"


void setup_scr_screen(lv_ui *ui){

	//Write codes screen
	ui->screen = lv_obj_create(NULL);
	lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_main_main_default
	static lv_style_t style_screen_main_main_default;
	if (style_screen_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_main_main_default);
	else
		lv_style_init(&style_screen_main_main_default);
	lv_style_set_bg_color(&style_screen_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_opa(&style_screen_main_main_default, 0);
	lv_obj_add_style(ui->screen, &style_screen_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_main_show
	ui->screen_main_show = lv_obj_create(ui->screen);
	lv_obj_set_pos(ui->screen_main_show, 0, 0);
	lv_obj_set_size(ui->screen_main_show, 480, 480);
	lv_obj_set_scrollbar_mode(ui->screen_main_show, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_main_show_main_main_default
	static lv_style_t style_screen_main_show_main_main_default;
	if (style_screen_main_show_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_main_show_main_main_default);
	else
		lv_style_init(&style_screen_main_show_main_main_default);
	lv_style_set_radius(&style_screen_main_show_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_main_show_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_main_show_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_main_show_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_main_show_main_main_default, 0);
	lv_style_set_border_color(&style_screen_main_show_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_main_show_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_main_show_main_main_default, 0);
	lv_style_set_pad_left(&style_screen_main_show_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_main_show_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_main_show_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_main_show_main_main_default, 0);
	lv_obj_add_style(ui->screen_main_show, &style_screen_main_show_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_tileview_1
	ui->screen_tileview_1 = lv_tileview_create(ui->screen);
	lv_obj_set_pos(ui->screen_tileview_1, 0, 0);
	lv_obj_set_size(ui->screen_tileview_1, 480, 480);
	lv_obj_set_scrollbar_mode(ui->screen_tileview_1, LV_SCROLLBAR_MODE_ON);

	//Write style state: LV_STATE_DEFAULT for style_screen_tileview_1_main_main_default
	static lv_style_t style_screen_tileview_1_main_main_default;
	if (style_screen_tileview_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_tileview_1_main_main_default);
	else
		lv_style_init(&style_screen_tileview_1_main_main_default);
	lv_style_set_radius(&style_screen_tileview_1_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_tileview_1_main_main_default, lv_color_make(0xf6, 0xf6, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_tileview_1_main_main_default, lv_color_make(0xf6, 0xf6, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_tileview_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_tileview_1_main_main_default, 255);
	lv_obj_add_style(ui->screen_tileview_1, &style_screen_tileview_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_tileview_1_main_scrollbar_default
	static lv_style_t style_screen_tileview_1_main_scrollbar_default;
	if (style_screen_tileview_1_main_scrollbar_default.prop_cnt > 1)
		lv_style_reset(&style_screen_tileview_1_main_scrollbar_default);
	else
		lv_style_init(&style_screen_tileview_1_main_scrollbar_default);
	lv_style_set_radius(&style_screen_tileview_1_main_scrollbar_default, 0);
	lv_style_set_bg_color(&style_screen_tileview_1_main_scrollbar_default, lv_color_make(0xea, 0xef, 0xf3));
	lv_style_set_bg_opa(&style_screen_tileview_1_main_scrollbar_default, 255);
	lv_obj_add_style(ui->screen_tileview_1, &style_screen_tileview_1_main_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

	//add new tile tileview_1_name_1
	ui->tileview_1_name_1 = lv_tileview_add_tile(ui->screen_tileview_1, 0, 0, LV_DIR_RIGHT);

	//Write codes screen_label_1
	ui->screen_label_1 = lv_label_create(ui->tileview_1_name_1);
	lv_obj_set_pos(ui->screen_label_1, 133, 85);
	lv_obj_set_size(ui->screen_label_1, 212, 39);
	lv_obj_set_scrollbar_mode(ui->screen_label_1, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_label_1, "智慧面板方案");
	lv_label_set_long_mode(ui->screen_label_1, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_label_1_main_main_default
	static lv_style_t style_screen_label_1_main_main_default;
	if (style_screen_label_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_label_1_main_main_default);
	else
		lv_style_init(&style_screen_label_1_main_main_default);
	lv_style_set_radius(&style_screen_label_1_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_label_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_label_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_label_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_label_1_main_main_default, 0);
	lv_style_set_text_color(&style_screen_label_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_label_1_main_main_default, &lv_font_simsun_30);
	lv_style_set_text_letter_space(&style_screen_label_1_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_label_1_main_main_default, 0);
	lv_style_set_text_align(&style_screen_label_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_label_1_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_label_1_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_label_1_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_label_1_main_main_default, 0);
	lv_obj_add_style(ui->screen_label_1, &style_screen_label_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_img_1
	ui->screen_img_1 = lv_img_create(ui->tileview_1_name_1);
	lv_obj_set_pos(ui->screen_img_1, 0, 0);
	lv_obj_set_size(ui->screen_img_1, 480, 480);
	lv_obj_set_scrollbar_mode(ui->screen_img_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_img_1_main_main_default
	static lv_style_t style_screen_img_1_main_main_default;
	if (style_screen_img_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_img_1_main_main_default);
	else
		lv_style_init(&style_screen_img_1_main_main_default);
	lv_style_set_img_recolor(&style_screen_img_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_img_1_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_img_1_main_main_default, 255);
	lv_obj_add_style(ui->screen_img_1, &style_screen_img_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_img_1, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_1,&_mainbround_480x480);
	lv_img_set_pivot(ui->screen_img_1, 0,0);
	lv_img_set_angle(ui->screen_img_1, 0);

	//Write codes screen_btn_2
	ui->screen_btn_2 = lv_btn_create(ui->screen);
	lv_obj_set_pos(ui->screen_btn_2, 190, 381);
	lv_obj_set_size(ui->screen_btn_2, 100, 50);
	lv_obj_set_scrollbar_mode(ui->screen_btn_2, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_btn_2_main_main_default
	static lv_style_t style_screen_btn_2_main_main_default;
	if (style_screen_btn_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_btn_2_main_main_default);
	else
		lv_style_init(&style_screen_btn_2_main_main_default);
	lv_style_set_radius(&style_screen_btn_2_main_main_default, 20);
	lv_style_set_bg_color(&style_screen_btn_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_btn_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_btn_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_btn_2_main_main_default, 255);
	lv_style_set_border_color(&style_screen_btn_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_btn_2_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_btn_2_main_main_default, 255);
	lv_style_set_text_color(&style_screen_btn_2_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_screen_btn_2_main_main_default, &lv_font_simsun_14);
	lv_style_set_text_align(&style_screen_btn_2_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_btn_2, &style_screen_btn_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_btn_2_label = lv_label_create(ui->screen_btn_2);
	lv_label_set_text(ui->screen_btn_2_label, "WIFI设置");
	lv_obj_set_style_pad_all(ui->screen_btn_2, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->screen_btn_2_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes screen_btn_3
	ui->screen_btn_3 = lv_btn_create(ui->screen);
	lv_obj_set_pos(ui->screen_btn_3, 190, 322);
	lv_obj_set_size(ui->screen_btn_3, 100, 50);
	lv_obj_set_scrollbar_mode(ui->screen_btn_3, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_btn_3_main_main_default
	static lv_style_t style_screen_btn_3_main_main_default;
	if (style_screen_btn_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_btn_3_main_main_default);
	else
		lv_style_init(&style_screen_btn_3_main_main_default);
	lv_style_set_radius(&style_screen_btn_3_main_main_default, 20);
	lv_style_set_bg_color(&style_screen_btn_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_btn_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_btn_3_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_btn_3_main_main_default, 255);
	lv_style_set_border_color(&style_screen_btn_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_btn_3_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_btn_3_main_main_default, 255);
	lv_style_set_text_color(&style_screen_btn_3_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_screen_btn_3_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_align(&style_screen_btn_3_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_btn_3, &style_screen_btn_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_btn_3_label = lv_label_create(ui->screen_btn_3);
	lv_label_set_text(ui->screen_btn_3_label, "播放与录音");
	lv_obj_set_style_pad_all(ui->screen_btn_3, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->screen_btn_3_label, LV_ALIGN_CENTER, 0, 0);

	//Init events for screen
	events_init_screen(ui);
}