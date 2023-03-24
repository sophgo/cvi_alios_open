/*
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"

static lv_obj_t * g_kb;
static void kb_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *kb = lv_event_get_target(e);
	if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL){
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}
static void ta_event_cb(lv_event_t *e)
{

	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);
	lv_obj_t *kb = lv_event_get_user_data(e);
	if (code == LV_EVENT_FOCUSED)
	{
		lv_keyboard_set_textarea(kb, ta);
		lv_obj_move_foreground(kb);
		lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
	if (code == LV_EVENT_DEFOCUSED)
	{
		lv_keyboard_set_textarea(kb, NULL);
		lv_obj_move_background(kb);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}

void setup_scr_screen_2(lv_ui *ui){

	//Write codes screen_2
	ui->screen_2 = lv_obj_create(NULL);
	lv_obj_set_scrollbar_mode(ui->screen_2, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_main_main_default
	static lv_style_t style_screen_2_main_main_default;
	if (style_screen_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_main_main_default);
	else
		lv_style_init(&style_screen_2_main_main_default);
	lv_style_set_bg_color(&style_screen_2_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_opa(&style_screen_2_main_main_default, 0);
	lv_obj_add_style(ui->screen_2, &style_screen_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_cont_1
	ui->screen_2_cont_1 = lv_obj_create(ui->screen_2);
	lv_obj_set_pos(ui->screen_2_cont_1, 0, 0);
	lv_obj_set_size(ui->screen_2_cont_1, 480, 480);
	lv_obj_set_scrollbar_mode(ui->screen_2_cont_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_cont_1_main_main_default
	static lv_style_t style_screen_2_cont_1_main_main_default;
	if (style_screen_2_cont_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_cont_1_main_main_default);
	else
		lv_style_init(&style_screen_2_cont_1_main_main_default);
	lv_style_set_radius(&style_screen_2_cont_1_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_cont_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_color(&style_screen_2_cont_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_cont_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_cont_1_main_main_default, 255);
	lv_style_set_border_color(&style_screen_2_cont_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_2_cont_1_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_2_cont_1_main_main_default, 255);
	lv_style_set_pad_left(&style_screen_2_cont_1_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_cont_1_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_cont_1_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_cont_1_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_cont_1, &style_screen_2_cont_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_line_1
	ui->screen_2_line_1 = lv_line_create(ui->screen_2_cont_1);
	lv_obj_set_pos(ui->screen_2_line_1, 0, 292);
	lv_obj_set_size(ui->screen_2_line_1, 480, 10);
	lv_obj_set_scrollbar_mode(ui->screen_2_line_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_line_1_main_main_default
	static lv_style_t style_screen_2_line_1_main_main_default;
	if (style_screen_2_line_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_line_1_main_main_default);
	else
		lv_style_init(&style_screen_2_line_1_main_main_default);
	lv_style_set_line_color(&style_screen_2_line_1_main_main_default, lv_color_make(0x75, 0x75, 0x75));
	lv_style_set_line_width(&style_screen_2_line_1_main_main_default, 2);
	lv_style_set_line_rounded(&style_screen_2_line_1_main_main_default, true);
	lv_obj_add_style(ui->screen_2_line_1, &style_screen_2_line_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	static lv_point_t screen_2_line_1[] ={{0, 0},{4800, 0},};
	lv_line_set_points(ui->screen_2_line_1,screen_2_line_1,2);

	//Write codes screen_2_btn_1
	ui->screen_2_btn_1 = lv_btn_create(ui->screen_2);
	lv_obj_set_pos(ui->screen_2_btn_1, 4, 1);
	lv_obj_set_size(ui->screen_2_btn_1, 100, 50);
	lv_obj_set_scrollbar_mode(ui->screen_2_btn_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_btn_1_main_main_default
	static lv_style_t style_screen_2_btn_1_main_main_default;
	if (style_screen_2_btn_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_btn_1_main_main_default);
	else
		lv_style_init(&style_screen_2_btn_1_main_main_default);
	lv_style_set_radius(&style_screen_2_btn_1_main_main_default, 5);
	lv_style_set_bg_color(&style_screen_2_btn_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_btn_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_btn_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_btn_1_main_main_default, 0);
	lv_style_set_border_color(&style_screen_2_btn_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_2_btn_1_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_2_btn_1_main_main_default, 255);
	lv_style_set_text_color(&style_screen_2_btn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_btn_1_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_align(&style_screen_2_btn_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_2_btn_1, &style_screen_2_btn_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_2_btn_1_label = lv_label_create(ui->screen_2_btn_1);
	lv_label_set_text(ui->screen_2_btn_1_label, "返回主界面");
	lv_obj_set_style_pad_all(ui->screen_2_btn_1, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->screen_2_btn_1_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes screen_2_cont_3
	ui->screen_2_cont_3 = lv_obj_create(ui->screen_2);
	lv_obj_set_pos(ui->screen_2_cont_3, 0, 73);
	lv_obj_set_size(ui->screen_2_cont_3, 479, 205);
	lv_obj_set_scrollbar_mode(ui->screen_2_cont_3, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_cont_3_main_main_default
	static lv_style_t style_screen_2_cont_3_main_main_default;
	if (style_screen_2_cont_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_cont_3_main_main_default);
	else
		lv_style_init(&style_screen_2_cont_3_main_main_default);
	lv_style_set_radius(&style_screen_2_cont_3_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_cont_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_cont_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_cont_3_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_cont_3_main_main_default, 0);
	lv_style_set_border_color(&style_screen_2_cont_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_2_cont_3_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_2_cont_3_main_main_default, 255);
	lv_style_set_pad_left(&style_screen_2_cont_3_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_cont_3_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_cont_3_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_cont_3_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_cont_3, &style_screen_2_cont_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_img_3
	ui->screen_2_img_3 = lv_img_create(ui->screen_2_cont_3);
	lv_obj_set_pos(ui->screen_2_img_3, 293, 20);
	lv_obj_set_size(ui->screen_2_img_3, 64, 64);
	lv_obj_set_scrollbar_mode(ui->screen_2_img_3, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_img_3_main_main_default
	static lv_style_t style_screen_2_img_3_main_main_default;
	if (style_screen_2_img_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_img_3_main_main_default);
	else
		lv_style_init(&style_screen_2_img_3_main_main_default);
	lv_style_set_img_recolor(&style_screen_2_img_3_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_2_img_3_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_2_img_3_main_main_default, 255);
	lv_obj_add_style(ui->screen_2_img_3, &style_screen_2_img_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_2_img_3, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_2_img_3,&_995735691730d719011343f856b05665_64x64);
	lv_img_set_pivot(ui->screen_2_img_3, 0,0);
	lv_img_set_angle(ui->screen_2_img_3, 0);

	//Write codes screen_2_btn_3
	ui->screen_2_btn_3 = lv_btn_create(ui->screen_2_cont_3);
	lv_obj_set_pos(ui->screen_2_btn_3, 150, 140);
	lv_obj_set_size(ui->screen_2_btn_3, 100, 50);
	lv_obj_set_scrollbar_mode(ui->screen_2_btn_3, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_btn_3_main_main_default
	static lv_style_t style_screen_2_btn_3_main_main_default;
	if (style_screen_2_btn_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_btn_3_main_main_default);
	else
		lv_style_init(&style_screen_2_btn_3_main_main_default);
	lv_style_set_radius(&style_screen_2_btn_3_main_main_default, 20);
	lv_style_set_bg_color(&style_screen_2_btn_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_btn_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_btn_3_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_btn_3_main_main_default, 255);
	lv_style_set_border_color(&style_screen_2_btn_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_2_btn_3_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_2_btn_3_main_main_default, 255);
	lv_style_set_text_color(&style_screen_2_btn_3_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_btn_3_main_main_default, &lv_font_simsun_15);
	lv_style_set_text_align(&style_screen_2_btn_3_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_2_btn_3, &style_screen_2_btn_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_2_btn_3_label = lv_label_create(ui->screen_2_btn_3);
	lv_label_set_text(ui->screen_2_btn_3_label, "cannel");
	lv_obj_set_style_pad_all(ui->screen_2_btn_3, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->screen_2_btn_3_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes screen_2_btn_2
	ui->screen_2_btn_2 = lv_btn_create(ui->screen_2_cont_3);
	lv_obj_set_pos(ui->screen_2_btn_2, 42, 140);
	lv_obj_set_size(ui->screen_2_btn_2, 100, 50);
	lv_obj_set_scrollbar_mode(ui->screen_2_btn_2, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_btn_2_main_main_default
	static lv_style_t style_screen_2_btn_2_main_main_default;
	if (style_screen_2_btn_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_btn_2_main_main_default);
	else
		lv_style_init(&style_screen_2_btn_2_main_main_default);
	lv_style_set_radius(&style_screen_2_btn_2_main_main_default, 20);
	lv_style_set_bg_color(&style_screen_2_btn_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_btn_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_btn_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_btn_2_main_main_default, 255);
	lv_style_set_border_color(&style_screen_2_btn_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_2_btn_2_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_2_btn_2_main_main_default, 255);
	lv_style_set_text_color(&style_screen_2_btn_2_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_btn_2_main_main_default, &lv_font_simsun_15);
	lv_style_set_text_align(&style_screen_2_btn_2_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(ui->screen_2_btn_2, &style_screen_2_btn_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	ui->screen_2_btn_2_label = lv_label_create(ui->screen_2_btn_2);
	lv_label_set_text(ui->screen_2_btn_2_label, "connect");
	lv_obj_set_style_pad_all(ui->screen_2_btn_2, 0, LV_STATE_DEFAULT);
	lv_obj_align(ui->screen_2_btn_2_label, LV_ALIGN_CENTER, 0, 0);

	//Write codes screen_2_label_2
	ui->screen_2_label_2 = lv_label_create(ui->screen_2_cont_3);
	lv_obj_set_pos(ui->screen_2_label_2, 24, 85);
	lv_obj_set_size(ui->screen_2_label_2, 100, 12);
	lv_obj_set_scrollbar_mode(ui->screen_2_label_2, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_2_label_2, "PASSWD");
	lv_label_set_long_mode(ui->screen_2_label_2, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_label_2_main_main_default
	static lv_style_t style_screen_2_label_2_main_main_default;
	if (style_screen_2_label_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_label_2_main_main_default);
	else
		lv_style_init(&style_screen_2_label_2_main_main_default);
	lv_style_set_radius(&style_screen_2_label_2_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_label_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_label_2_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_label_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_label_2_main_main_default, 0);
	lv_style_set_text_color(&style_screen_2_label_2_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_label_2_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_2_label_2_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_2_label_2_main_main_default, 0);
	lv_style_set_text_align(&style_screen_2_label_2_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_2_label_2_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_label_2_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_label_2_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_label_2_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_label_2, &style_screen_2_label_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_ta_1
	ui->screen_2_ta_1 = lv_textarea_create(ui->screen_2);
	lv_obj_set_pos(ui->screen_2_ta_1, 42, 116);
	lv_obj_set_size(ui->screen_2_ta_1, 200, 30);
	lv_obj_set_scrollbar_mode(ui->screen_2_ta_1, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_ta_1_main_main_default
	static lv_style_t style_screen_2_ta_1_main_main_default;
	if (style_screen_2_ta_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_ta_1_main_main_default);
	else
		lv_style_init(&style_screen_2_ta_1_main_main_default);
	lv_style_set_radius(&style_screen_2_ta_1_main_main_default, 4);
	lv_style_set_bg_color(&style_screen_2_ta_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_color(&style_screen_2_ta_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_dir(&style_screen_2_ta_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_ta_1_main_main_default, 255);
	lv_style_set_border_color(&style_screen_2_ta_1_main_main_default, lv_color_make(0xe6, 0xe6, 0xe6));
	lv_style_set_border_width(&style_screen_2_ta_1_main_main_default, 2);
	lv_style_set_border_opa(&style_screen_2_ta_1_main_main_default, 255);
	lv_style_set_text_color(&style_screen_2_ta_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_letter_space(&style_screen_2_ta_1_main_main_default, 2);
	lv_style_set_text_align(&style_screen_2_ta_1_main_main_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_left(&style_screen_2_ta_1_main_main_default, 4);
	lv_style_set_pad_right(&style_screen_2_ta_1_main_main_default, 4);
	lv_style_set_pad_top(&style_screen_2_ta_1_main_main_default, 4);
	lv_style_set_pad_bottom(&style_screen_2_ta_1_main_main_default, 4);
	lv_obj_add_style(ui->screen_2_ta_1, &style_screen_2_ta_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_ta_1_main_scrollbar_default
	static lv_style_t style_screen_2_ta_1_main_scrollbar_default;
	if (style_screen_2_ta_1_main_scrollbar_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_ta_1_main_scrollbar_default);
	else
		lv_style_init(&style_screen_2_ta_1_main_scrollbar_default);
	lv_style_set_radius(&style_screen_2_ta_1_main_scrollbar_default, 0);
	lv_style_set_bg_color(&style_screen_2_ta_1_main_scrollbar_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_ta_1_main_scrollbar_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_ta_1_main_scrollbar_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_ta_1_main_scrollbar_default, 255);
	lv_obj_add_style(ui->screen_2_ta_1, &style_screen_2_ta_1_main_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	g_kb = lv_keyboard_create(ui->screen_2);
	lv_obj_add_event_cb(g_kb, kb_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(g_kb, LV_OBJ_FLAG_HIDDEN);
	lv_keyboard_set_textarea(g_kb, ui->screen_2_ta_1);
	lv_obj_add_event_cb(ui->screen_2_ta_1, ta_event_cb, LV_EVENT_ALL, g_kb);

	//Write codes screen_2_label_1
	ui->screen_2_label_1 = lv_label_create(ui->screen_2_cont_3);
	lv_obj_set_pos(ui->screen_2_label_1, 11, 26);
	lv_obj_set_size(ui->screen_2_label_1, 100, 12);
	lv_obj_set_scrollbar_mode(ui->screen_2_label_1, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_2_label_1, "SSID");
	lv_label_set_long_mode(ui->screen_2_label_1, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_label_1_main_main_default
	static lv_style_t style_screen_2_label_1_main_main_default;
	if (style_screen_2_label_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_label_1_main_main_default);
	else
		lv_style_init(&style_screen_2_label_1_main_main_default);
	lv_style_set_radius(&style_screen_2_label_1_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_label_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_label_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_label_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_label_1_main_main_default, 0);
	lv_style_set_text_color(&style_screen_2_label_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_label_1_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_2_label_1_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_2_label_1_main_main_default, 0);
	lv_style_set_text_align(&style_screen_2_label_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_2_label_1_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_label_1_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_label_1_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_label_1_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_label_1, &style_screen_2_label_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_ta_2
	ui->screen_2_ta_2 = lv_textarea_create(ui->screen_2);
	lv_obj_set_pos(ui->screen_2_ta_2, 42, 177);
	lv_obj_set_size(ui->screen_2_ta_2, 200, 30);
	lv_obj_set_scrollbar_mode(ui->screen_2_ta_2, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_ta_2_main_main_default
	static lv_style_t style_screen_2_ta_2_main_main_default;
	if (style_screen_2_ta_2_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_ta_2_main_main_default);
	else
		lv_style_init(&style_screen_2_ta_2_main_main_default);
	lv_style_set_radius(&style_screen_2_ta_2_main_main_default, 4);
	lv_style_set_bg_color(&style_screen_2_ta_2_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_color(&style_screen_2_ta_2_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_dir(&style_screen_2_ta_2_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_ta_2_main_main_default, 255);
	lv_style_set_border_color(&style_screen_2_ta_2_main_main_default, lv_color_make(0xe6, 0xe6, 0xe6));
	lv_style_set_border_width(&style_screen_2_ta_2_main_main_default, 2);
	lv_style_set_border_opa(&style_screen_2_ta_2_main_main_default, 255);
	lv_style_set_text_color(&style_screen_2_ta_2_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_letter_space(&style_screen_2_ta_2_main_main_default, 2);
	lv_style_set_text_align(&style_screen_2_ta_2_main_main_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_left(&style_screen_2_ta_2_main_main_default, 4);
	lv_style_set_pad_right(&style_screen_2_ta_2_main_main_default, 4);
	lv_style_set_pad_top(&style_screen_2_ta_2_main_main_default, 4);
	lv_style_set_pad_bottom(&style_screen_2_ta_2_main_main_default, 4);
	lv_obj_add_style(ui->screen_2_ta_2, &style_screen_2_ta_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_ta_2_main_scrollbar_default
	static lv_style_t style_screen_2_ta_2_main_scrollbar_default;
	if (style_screen_2_ta_2_main_scrollbar_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_ta_2_main_scrollbar_default);
	else
		lv_style_init(&style_screen_2_ta_2_main_scrollbar_default);
	lv_style_set_radius(&style_screen_2_ta_2_main_scrollbar_default, 0);
	lv_style_set_bg_color(&style_screen_2_ta_2_main_scrollbar_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_ta_2_main_scrollbar_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_ta_2_main_scrollbar_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_ta_2_main_scrollbar_default, 255);
	lv_obj_add_style(ui->screen_2_ta_2, &style_screen_2_ta_2_main_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
	g_kb = lv_keyboard_create(ui->screen_2);
	lv_obj_add_event_cb(g_kb, kb_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(g_kb, LV_OBJ_FLAG_HIDDEN);
	lv_keyboard_set_textarea(g_kb, ui->screen_2_ta_2);
	lv_obj_add_event_cb(ui->screen_2_ta_2, ta_event_cb, LV_EVENT_ALL, g_kb);

	//Write codes screen_2_label_3
	ui->screen_2_label_3 = lv_label_create(ui->screen_2);
	lv_obj_set_pos(ui->screen_2_label_3, 181, 51);
	lv_obj_set_size(ui->screen_2_label_3, 100, 32);
	lv_obj_set_scrollbar_mode(ui->screen_2_label_3, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_2_label_3, "WIFI Connect");
	lv_label_set_long_mode(ui->screen_2_label_3, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_label_3_main_main_default
	static lv_style_t style_screen_2_label_3_main_main_default;
	if (style_screen_2_label_3_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_label_3_main_main_default);
	else
		lv_style_init(&style_screen_2_label_3_main_main_default);
	lv_style_set_radius(&style_screen_2_label_3_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_label_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_label_3_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_label_3_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_label_3_main_main_default, 0);
	lv_style_set_text_color(&style_screen_2_label_3_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_label_3_main_main_default, &lv_font_Alatsi_Regular_16);
	lv_style_set_text_letter_space(&style_screen_2_label_3_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_2_label_3_main_main_default, 0);
	lv_style_set_text_align(&style_screen_2_label_3_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_2_label_3_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_label_3_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_label_3_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_label_3_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_label_3, &style_screen_2_label_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_cont_4
	ui->screen_2_cont_4 = lv_obj_create(ui->screen_2);
	lv_obj_set_pos(ui->screen_2_cont_4, 0, 315);
	lv_obj_set_size(ui->screen_2_cont_4, 480, 149);
	lv_obj_set_scrollbar_mode(ui->screen_2_cont_4, LV_SCROLLBAR_MODE_OFF);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_cont_4_main_main_default
	static lv_style_t style_screen_2_cont_4_main_main_default;
	if (style_screen_2_cont_4_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_cont_4_main_main_default);
	else
		lv_style_init(&style_screen_2_cont_4_main_main_default);
	lv_style_set_radius(&style_screen_2_cont_4_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_cont_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_cont_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_cont_4_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_cont_4_main_main_default, 0);
	lv_style_set_border_color(&style_screen_2_cont_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_border_width(&style_screen_2_cont_4_main_main_default, 0);
	lv_style_set_border_opa(&style_screen_2_cont_4_main_main_default, 255);
	lv_style_set_pad_left(&style_screen_2_cont_4_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_cont_4_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_cont_4_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_cont_4_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_cont_4, &style_screen_2_cont_4_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_label_8
	ui->screen_2_label_8 = lv_label_create(ui->screen_2_cont_4);
	lv_obj_set_pos(ui->screen_2_label_8, 20, 104);
	lv_obj_set_size(ui->screen_2_label_8, 360, 12);
	lv_obj_set_scrollbar_mode(ui->screen_2_label_8, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_2_label_8, "IP:");
	lv_label_set_long_mode(ui->screen_2_label_8, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_label_8_main_main_default
	static lv_style_t style_screen_2_label_8_main_main_default;
	if (style_screen_2_label_8_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_label_8_main_main_default);
	else
		lv_style_init(&style_screen_2_label_8_main_main_default);
	lv_style_set_radius(&style_screen_2_label_8_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_label_8_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_label_8_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_label_8_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_label_8_main_main_default, 0);
	lv_style_set_text_color(&style_screen_2_label_8_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_label_8_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_2_label_8_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_2_label_8_main_main_default, 0);
	lv_style_set_text_align(&style_screen_2_label_8_main_main_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_left(&style_screen_2_label_8_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_label_8_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_label_8_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_label_8_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_label_8, &style_screen_2_label_8_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_label_7
	ui->screen_2_label_7 = lv_label_create(ui->screen_2_cont_4);
	lv_obj_set_pos(ui->screen_2_label_7, 20, 81);
	lv_obj_set_size(ui->screen_2_label_7, 360, 12);
	lv_obj_set_scrollbar_mode(ui->screen_2_label_7, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_2_label_7, "MAC:");
	lv_label_set_long_mode(ui->screen_2_label_7, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_label_7_main_main_default
	static lv_style_t style_screen_2_label_7_main_main_default;
	if (style_screen_2_label_7_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_label_7_main_main_default);
	else
		lv_style_init(&style_screen_2_label_7_main_main_default);
	lv_style_set_radius(&style_screen_2_label_7_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_label_7_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_label_7_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_label_7_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_label_7_main_main_default, 0);
	lv_style_set_text_color(&style_screen_2_label_7_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_label_7_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_2_label_7_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_2_label_7_main_main_default, 0);
	lv_style_set_text_align(&style_screen_2_label_7_main_main_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_left(&style_screen_2_label_7_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_label_7_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_label_7_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_label_7_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_label_7, &style_screen_2_label_7_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_label_6
	ui->screen_2_label_6 = lv_label_create(ui->screen_2_cont_4);
	lv_obj_set_pos(ui->screen_2_label_6, 20, 60);
	lv_obj_set_size(ui->screen_2_label_6, 360, 12);
	lv_obj_set_scrollbar_mode(ui->screen_2_label_6, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_2_label_6, "Gwaddr:");
	lv_label_set_long_mode(ui->screen_2_label_6, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_label_6_main_main_default
	static lv_style_t style_screen_2_label_6_main_main_default;
	if (style_screen_2_label_6_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_label_6_main_main_default);
	else
		lv_style_init(&style_screen_2_label_6_main_main_default);
	lv_style_set_radius(&style_screen_2_label_6_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_label_6_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_label_6_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_label_6_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_label_6_main_main_default, 0);
	lv_style_set_text_color(&style_screen_2_label_6_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_label_6_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_2_label_6_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_2_label_6_main_main_default, 0);
	lv_style_set_text_align(&style_screen_2_label_6_main_main_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_left(&style_screen_2_label_6_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_label_6_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_label_6_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_label_6_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_label_6, &style_screen_2_label_6_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_label_5
	ui->screen_2_label_5 = lv_label_create(ui->screen_2_cont_4);
	lv_obj_set_pos(ui->screen_2_label_5, 20, 37);
	lv_obj_set_size(ui->screen_2_label_5, 360, 12);
	lv_obj_set_scrollbar_mode(ui->screen_2_label_5, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_2_label_5, "Wifi_Name:");
	lv_label_set_long_mode(ui->screen_2_label_5, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_label_5_main_main_default
	static lv_style_t style_screen_2_label_5_main_main_default;
	if (style_screen_2_label_5_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_label_5_main_main_default);
	else
		lv_style_init(&style_screen_2_label_5_main_main_default);
	lv_style_set_radius(&style_screen_2_label_5_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_label_5_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_label_5_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_label_5_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_label_5_main_main_default, 0);
	lv_style_set_text_color(&style_screen_2_label_5_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_label_5_main_main_default, &lv_font_simsun_12);
	lv_style_set_text_letter_space(&style_screen_2_label_5_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_2_label_5_main_main_default, 0);
	lv_style_set_text_align(&style_screen_2_label_5_main_main_default, LV_TEXT_ALIGN_LEFT);
	lv_style_set_pad_left(&style_screen_2_label_5_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_label_5_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_label_5_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_label_5_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_label_5, &style_screen_2_label_5_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_2_label_4
	ui->screen_2_label_4 = lv_label_create(ui->screen_2_cont_4);
	lv_obj_set_pos(ui->screen_2_label_4, 181, 4);
	lv_obj_set_size(ui->screen_2_label_4, 100, 32);
	lv_obj_set_scrollbar_mode(ui->screen_2_label_4, LV_SCROLLBAR_MODE_OFF);
	lv_label_set_text(ui->screen_2_label_4, "WIFI Status");
	lv_label_set_long_mode(ui->screen_2_label_4, LV_LABEL_LONG_WRAP);

	//Write style state: LV_STATE_DEFAULT for style_screen_2_label_4_main_main_default
	static lv_style_t style_screen_2_label_4_main_main_default;
	if (style_screen_2_label_4_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_2_label_4_main_main_default);
	else
		lv_style_init(&style_screen_2_label_4_main_main_default);
	lv_style_set_radius(&style_screen_2_label_4_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_2_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_2_label_4_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_2_label_4_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_2_label_4_main_main_default, 0);
	lv_style_set_text_color(&style_screen_2_label_4_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_2_label_4_main_main_default, &lv_font_Alatsi_Regular_16);
	lv_style_set_text_letter_space(&style_screen_2_label_4_main_main_default, 2);
	lv_style_set_text_line_space(&style_screen_2_label_4_main_main_default, 0);
	lv_style_set_text_align(&style_screen_2_label_4_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_pad_left(&style_screen_2_label_4_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_2_label_4_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_2_label_4_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_2_label_4_main_main_default, 0);
	lv_obj_add_style(ui->screen_2_label_4, &style_screen_2_label_4_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Init events for screen
	events_init_screen_2(ui);
}