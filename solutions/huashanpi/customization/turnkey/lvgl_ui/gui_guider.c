/*
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"


void init_scr_del_flag(lv_ui *ui){
	ui->screen_del = true;
	ui->screen_1_del = true;
	ui->screen_2_del = true;
}

void setup_ui(lv_ui *ui){
	init_scr_del_flag(ui);
	setup_scr_screen(ui);
	setup_scr_screen_1(ui);
	setup_scr_screen_2(ui);
	lv_scr_load(ui->screen);
}
