# Copyright 2022 NXP
# SPDX-License-Identifier: MIT
# The auto-generated can only be used on NXP devices

import SDL
import utime as time
import usys as sys
import lvgl as lv
import lodepng as png
import ustruct

lv.init()
SDL.init(w=480,h=480)

# Register SDL display driver.
disp_buf1 = lv.disp_draw_buf_t()
buf1_1 = bytearray(480*10)
disp_buf1.init(buf1_1, None, len(buf1_1)//4)
disp_drv = lv.disp_drv_t()
disp_drv.init()
disp_drv.draw_buf = disp_buf1
disp_drv.flush_cb = SDL.monitor_flush
disp_drv.hor_res = 480
disp_drv.ver_res = 480
disp_drv.register()

# Regsiter SDL mouse driver
indev_drv = lv.indev_drv_t()
indev_drv.init() 
indev_drv.type = lv.INDEV_TYPE.POINTER
indev_drv.read_cb = SDL.mouse_read
indev_drv.register()

# Below: Taken from https://github.com/lvgl/lv_binding_micropython/blob/master/driver/js/imagetools.py#L22-L94

COLOR_SIZE = lv.color_t.__SIZE__
COLOR_IS_SWAPPED = hasattr(lv.color_t().ch,'green_h')

class lodepng_error(RuntimeError):
    def __init__(self, err):
        if type(err) is int:
            super().__init__(png.error_text(err))
        else:
            super().__init__(err)

# Parse PNG file header
# Taken from https://github.com/shibukawa/imagesize_py/blob/ffef30c1a4715c5acf90e8945ceb77f4a2ed2d45/imagesize.py#L63-L85

def get_png_info(decoder, src, header):
    # Only handle variable image types

    if lv.img.src_get_type(src) != lv.img.SRC.VARIABLE:
        return lv.RES.INV

    data = lv.img_dsc_t.__cast__(src).data
    if data == None:
        return lv.RES.INV

    png_header = bytes(data.__dereference__(24))

    if png_header.startswith(b'\211PNG\r\n\032\n'):
        if png_header[12:16] == b'IHDR':
            start = 16
        # Maybe this is for an older PNG version.
        else:
            start = 8
        try:
            width, height = ustruct.unpack(">LL", png_header[start:start+8])
        except ustruct.error:
            return lv.RES.INV
    else:
        return lv.RES.INV

    header.always_zero = 0
    header.w = width
    header.h = height
    header.cf = lv.img.CF.TRUE_COLOR_ALPHA

    return lv.RES.OK

def convert_rgba8888_to_bgra8888(img_view):
    for i in range(0, len(img_view), lv.color_t.__SIZE__):
        ch = lv.color_t.__cast__(img_view[i:i]).ch
        ch.red, ch.blue = ch.blue, ch.red

# Read and parse PNG file

def open_png(decoder, dsc):
    img_dsc = lv.img_dsc_t.__cast__(dsc.src)
    png_data = img_dsc.data
    png_size = img_dsc.data_size
    png_decoded = png.C_Pointer()
    png_width = png.C_Pointer()
    png_height = png.C_Pointer()
    error = png.decode32(png_decoded, png_width, png_height, png_data, png_size)
    if error:
        raise lodepng_error(error)
    img_size = png_width.int_val * png_height.int_val * 4
    img_data = png_decoded.ptr_val
    img_view = img_data.__dereference__(img_size)

    if COLOR_SIZE == 4:
        convert_rgba8888_to_bgra8888(img_view)
    else:
        raise lodepng_error("Error: Color mode not supported yet!")

    dsc.img_data = img_data
    return lv.RES.OK

# Above: Taken from https://github.com/lvgl/lv_binding_micropython/blob/master/driver/js/imagetools.py#L22-L94

decoder = lv.img.decoder_create()
decoder.info_cb = get_png_info
decoder.open_cb = open_png

def anim_x_cb(obj, v):
    obj.set_x(v)

def anim_y_cb(obj, v):
    obj.set_y(v)

def ta_event_cb(e,kb):
    code = e.get_code()
    ta = e.get_target()
    if code == lv.EVENT.FOCUSED:
        kb.set_textarea(ta)
        kb.move_foreground()
        kb.clear_flag(lv.obj.FLAG.HIDDEN)

    if code == lv.EVENT.DEFOCUSED:
        kb.set_textarea(None)
        kb.move_background()
        kb.add_flag(lv.obj.FLAG.HIDDEN)


screen = lv.obj()
screen.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_screen_main_main_default
style_screen_main_main_default = lv.style_t()
style_screen_main_main_default.init()
style_screen_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_screen_main_main_default.set_bg_opa(0)

# add style for screen
screen.add_style(style_screen_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_main_show = lv.obj(screen)
screen_main_show.set_pos(int(0),int(0))
screen_main_show.set_size(480,480)
screen_main_show.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_screen_main_show_main_main_default
style_screen_main_show_main_main_default = lv.style_t()
style_screen_main_show_main_main_default.init()
style_screen_main_show_main_main_default.set_radius(0)
style_screen_main_show_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_main_show_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_main_show_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_main_show_main_main_default.set_bg_opa(0)
style_screen_main_show_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_main_show_main_main_default.set_border_width(0)
style_screen_main_show_main_main_default.set_border_opa(0)
style_screen_main_show_main_main_default.set_pad_left(0)
style_screen_main_show_main_main_default.set_pad_right(0)
style_screen_main_show_main_main_default.set_pad_top(0)
style_screen_main_show_main_main_default.set_pad_bottom(0)

# add style for screen_main_show
screen_main_show.add_style(style_screen_main_show_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_tileview_1 = lv.tileview(screen)
screen_tileview_1.set_pos(int(0),int(0))
screen_tileview_1.set_size(480,480)
screen_tileview_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.ON)
tileview_1_name_1 = screen_tileview_1.add_tile(0, 0, lv.DIR.RIGHT)
screen_label_1 = lv.label(tileview_1_name_1)
screen_label_1.set_pos(int(133),int(85))
screen_label_1.set_size(212,39)
screen_label_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_label_1.set_text("智慧面板方案")
screen_label_1.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_label_1_main_main_default
style_screen_label_1_main_main_default = lv.style_t()
style_screen_label_1_main_main_default.init()
style_screen_label_1_main_main_default.set_radius(0)
style_screen_label_1_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_label_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_label_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_label_1_main_main_default.set_bg_opa(0)
style_screen_label_1_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_label_1_main_main_default.set_text_font(lv.font_simsun_30)
except AttributeError:
    try:
        style_screen_label_1_main_main_default.set_text_font(lv.font_montserrat_30)
    except AttributeError:
        style_screen_label_1_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_label_1_main_main_default.set_text_letter_space(2)
style_screen_label_1_main_main_default.set_text_line_space(0)
style_screen_label_1_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_screen_label_1_main_main_default.set_pad_left(0)
style_screen_label_1_main_main_default.set_pad_right(0)
style_screen_label_1_main_main_default.set_pad_top(0)
style_screen_label_1_main_main_default.set_pad_bottom(0)

# add style for screen_label_1
screen_label_1.add_style(style_screen_label_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_img_1 = lv.img(tileview_1_name_1)
screen_img_1.set_pos(int(0),int(0))
screen_img_1.set_size(480,480)
screen_img_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_img_1.add_flag(lv.obj.FLAG.CLICKABLE)
try:
    with open('D:\\NXP\\project\\SmartPad_32bit\\generated\\mPythonImages\\mp1124676183.png','rb') as f:
        screen_img_1_img_data = f.read()
except:
    print('Could not open D:\\NXP\\project\\SmartPad_32bit\\generated\\mPythonImages\\mp1124676183.png')
    sys.exit()

screen_img_1_img = lv.img_dsc_t({
  'data_size': len(screen_img_1_img_data),
  'header': {'always_zero': 0, 'w': 480, 'h': 480, 'cf': lv.img.CF.TRUE_COLOR},
  'data': screen_img_1_img_data
})

screen_img_1.set_src(screen_img_1_img)
screen_img_1.set_pivot(0,0)
screen_img_1.set_angle(0)
# create style style_screen_img_1_main_main_default
style_screen_img_1_main_main_default = lv.style_t()
style_screen_img_1_main_main_default.init()
style_screen_img_1_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_screen_img_1_main_main_default.set_img_recolor_opa(0)
style_screen_img_1_main_main_default.set_img_opa(255)

# add style for screen_img_1
screen_img_1.add_style(style_screen_img_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_screen_tileview_1_main_main_default
style_screen_tileview_1_main_main_default = lv.style_t()
style_screen_tileview_1_main_main_default.init()
style_screen_tileview_1_main_main_default.set_radius(0)
style_screen_tileview_1_main_main_default.set_bg_color(lv.color_make(0xf6,0xf6,0xf6))
style_screen_tileview_1_main_main_default.set_bg_grad_color(lv.color_make(0xf6,0xf6,0xf6))
style_screen_tileview_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_tileview_1_main_main_default.set_bg_opa(255)

# add style for screen_tileview_1
screen_tileview_1.add_style(style_screen_tileview_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_screen_tileview_1_main_scrollbar_default
style_screen_tileview_1_main_scrollbar_default = lv.style_t()
style_screen_tileview_1_main_scrollbar_default.init()
style_screen_tileview_1_main_scrollbar_default.set_radius(0)
style_screen_tileview_1_main_scrollbar_default.set_bg_color(lv.color_make(0xea,0xef,0xf3))
style_screen_tileview_1_main_scrollbar_default.set_bg_opa(255)

# add style for screen_tileview_1
screen_tileview_1.add_style(style_screen_tileview_1_main_scrollbar_default, lv.PART.SCROLLBAR|lv.STATE.DEFAULT)

screen_btn_2 = lv.btn(screen)
screen_btn_2.set_pos(int(190),int(381))
screen_btn_2.set_size(100,50)
screen_btn_2.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_btn_2_label = lv.label(screen_btn_2)
screen_btn_2_label.set_text("WIFI设置")
screen_btn_2.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_2_label.align(lv.ALIGN.CENTER,0,0)
# create style style_screen_btn_2_main_main_default
style_screen_btn_2_main_main_default = lv.style_t()
style_screen_btn_2_main_main_default.init()
style_screen_btn_2_main_main_default.set_radius(20)
style_screen_btn_2_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_btn_2_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_btn_2_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_btn_2_main_main_default.set_bg_opa(255)
style_screen_btn_2_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_btn_2_main_main_default.set_border_width(0)
style_screen_btn_2_main_main_default.set_border_opa(255)
style_screen_btn_2_main_main_default.set_text_color(lv.color_make(0xff,0xff,0xff))
try:
    style_screen_btn_2_main_main_default.set_text_font(lv.font_simsun_14)
except AttributeError:
    try:
        style_screen_btn_2_main_main_default.set_text_font(lv.font_montserrat_14)
    except AttributeError:
        style_screen_btn_2_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_btn_2_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)

# add style for screen_btn_2
screen_btn_2.add_style(style_screen_btn_2_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_btn_3 = lv.btn(screen)
screen_btn_3.set_pos(int(190),int(322))
screen_btn_3.set_size(100,50)
screen_btn_3.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_btn_3_label = lv.label(screen_btn_3)
screen_btn_3_label.set_text("播放/录音")
screen_btn_3.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_3_label.align(lv.ALIGN.CENTER,0,0)
# create style style_screen_btn_3_main_main_default
style_screen_btn_3_main_main_default = lv.style_t()
style_screen_btn_3_main_main_default.init()
style_screen_btn_3_main_main_default.set_radius(20)
style_screen_btn_3_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_btn_3_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_btn_3_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_btn_3_main_main_default.set_bg_opa(255)
style_screen_btn_3_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_btn_3_main_main_default.set_border_width(0)
style_screen_btn_3_main_main_default.set_border_opa(255)
style_screen_btn_3_main_main_default.set_text_color(lv.color_make(0xff,0xff,0xff))
try:
    style_screen_btn_3_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_btn_3_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_btn_3_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_btn_3_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)

# add style for screen_btn_3
screen_btn_3.add_style(style_screen_btn_3_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_1 = lv.obj()
screen_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_screen_1_main_main_default
style_screen_1_main_main_default = lv.style_t()
style_screen_1_main_main_default.init()
style_screen_1_main_main_default.set_bg_color(lv.color_make(0x00,0x79,0xfa))
style_screen_1_main_main_default.set_bg_opa(0)

# add style for screen_1
screen_1.add_style(style_screen_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2 = lv.obj()
screen_2.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# create style style_screen_2_main_main_default
style_screen_2_main_main_default = lv.style_t()
style_screen_2_main_main_default.init()
style_screen_2_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_screen_2_main_main_default.set_bg_opa(0)

# add style for screen_2
screen_2.add_style(style_screen_2_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_cont_1 = lv.obj(screen_2)
screen_2_cont_1.set_pos(int(0),int(0))
screen_2_cont_1.set_size(480,480)
screen_2_cont_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_line_1 = lv.line(screen_2_cont_1)
screen_2_line_1.set_pos(int(0),int(265))
screen_2_line_1.set_size(480,10)
screen_2_line_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
line_points = [
                   {"x":0, "y":0}, 
                   {"x":4800, "y":0}, 
			  ]
screen_2_line_1.set_points(line_points, 2)
# create style style_screen_2_line_1_main_main_default
style_screen_2_line_1_main_main_default = lv.style_t()
style_screen_2_line_1_main_main_default.init()
style_screen_2_line_1_main_main_default.set_line_color(lv.color_make(0x75,0x75,0x75))
style_screen_2_line_1_main_main_default.set_line_width(2)
style_screen_2_line_1_main_main_default.set_line_rounded(True)

# add style for screen_2_line_1
screen_2_line_1.add_style(style_screen_2_line_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_screen_2_cont_1_main_main_default
style_screen_2_cont_1_main_main_default = lv.style_t()
style_screen_2_cont_1_main_main_default.init()
style_screen_2_cont_1_main_main_default.set_radius(0)
style_screen_2_cont_1_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_screen_2_cont_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_cont_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_cont_1_main_main_default.set_bg_opa(255)
style_screen_2_cont_1_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_cont_1_main_main_default.set_border_width(0)
style_screen_2_cont_1_main_main_default.set_border_opa(255)
style_screen_2_cont_1_main_main_default.set_pad_left(0)
style_screen_2_cont_1_main_main_default.set_pad_right(0)
style_screen_2_cont_1_main_main_default.set_pad_top(0)
style_screen_2_cont_1_main_main_default.set_pad_bottom(0)

# add style for screen_2_cont_1
screen_2_cont_1.add_style(style_screen_2_cont_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_btn_1 = lv.btn(screen_2)
screen_2_btn_1.set_pos(int(4),int(1))
screen_2_btn_1.set_size(100,50)
screen_2_btn_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_btn_1_label = lv.label(screen_2_btn_1)
screen_2_btn_1_label.set_text("返回主界面")
screen_2_btn_1.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_2_btn_1_label.align(lv.ALIGN.CENTER,0,0)
# create style style_screen_2_btn_1_main_main_default
style_screen_2_btn_1_main_main_default = lv.style_t()
style_screen_2_btn_1_main_main_default.init()
style_screen_2_btn_1_main_main_default.set_radius(5)
style_screen_2_btn_1_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_btn_1_main_main_default.set_bg_opa(0)
style_screen_2_btn_1_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_1_main_main_default.set_border_width(0)
style_screen_2_btn_1_main_main_default.set_border_opa(255)
style_screen_2_btn_1_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_btn_1_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_btn_1_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_btn_1_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_btn_1_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)

# add style for screen_2_btn_1
screen_2_btn_1.add_style(style_screen_2_btn_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_cont_3 = lv.obj(screen_2)
screen_2_cont_3.set_pos(int(0),int(73))
screen_2_cont_3.set_size(480,163)
screen_2_cont_3.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_img_3 = lv.img(screen_2_cont_3)
screen_2_img_3.set_pos(int(293),int(20))
screen_2_img_3.set_size(64,64)
screen_2_img_3.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_img_3.add_flag(lv.obj.FLAG.CLICKABLE)
try:
    with open('D:\\NXP\\project\\SmartPad_32bit\\generated\\mPythonImages\\mp1351478559.png','rb') as f:
        screen_2_img_3_img_data = f.read()
except:
    print('Could not open D:\\NXP\\project\\SmartPad_32bit\\generated\\mPythonImages\\mp1351478559.png')
    sys.exit()

screen_2_img_3_img = lv.img_dsc_t({
  'data_size': len(screen_2_img_3_img_data),
  'header': {'always_zero': 0, 'w': 64, 'h': 64, 'cf': lv.img.CF.TRUE_COLOR_ALPHA},
  'data': screen_2_img_3_img_data
})

screen_2_img_3.set_src(screen_2_img_3_img)
screen_2_img_3.set_pivot(0,0)
screen_2_img_3.set_angle(0)
# create style style_screen_2_img_3_main_main_default
style_screen_2_img_3_main_main_default = lv.style_t()
style_screen_2_img_3_main_main_default.init()
style_screen_2_img_3_main_main_default.set_img_recolor(lv.color_make(0xff,0xff,0xff))
style_screen_2_img_3_main_main_default.set_img_recolor_opa(0)
style_screen_2_img_3_main_main_default.set_img_opa(255)

# add style for screen_2_img_3
screen_2_img_3.add_style(style_screen_2_img_3_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_btn_3 = lv.btn(screen_2_cont_3)
screen_2_btn_3.set_pos(int(150),int(115))
screen_2_btn_3.set_size(100,50)
screen_2_btn_3.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_btn_3_label = lv.label(screen_2_btn_3)
screen_2_btn_3_label.set_text("cannel")
screen_2_btn_3.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_2_btn_3_label.align(lv.ALIGN.CENTER,0,0)
# create style style_screen_2_btn_3_main_main_default
style_screen_2_btn_3_main_main_default = lv.style_t()
style_screen_2_btn_3_main_main_default.init()
style_screen_2_btn_3_main_main_default.set_radius(20)
style_screen_2_btn_3_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_3_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_3_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_btn_3_main_main_default.set_bg_opa(255)
style_screen_2_btn_3_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_3_main_main_default.set_border_width(0)
style_screen_2_btn_3_main_main_default.set_border_opa(255)
style_screen_2_btn_3_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_btn_3_main_main_default.set_text_font(lv.font_simsun_15)
except AttributeError:
    try:
        style_screen_2_btn_3_main_main_default.set_text_font(lv.font_montserrat_15)
    except AttributeError:
        style_screen_2_btn_3_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_btn_3_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)

# add style for screen_2_btn_3
screen_2_btn_3.add_style(style_screen_2_btn_3_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_btn_2 = lv.btn(screen_2_cont_3)
screen_2_btn_2.set_pos(int(42),int(115))
screen_2_btn_2.set_size(100,50)
screen_2_btn_2.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_btn_2_label = lv.label(screen_2_btn_2)
screen_2_btn_2_label.set_text("connect")
screen_2_btn_2.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_2_btn_2_label.align(lv.ALIGN.CENTER,0,0)
# create style style_screen_2_btn_2_main_main_default
style_screen_2_btn_2_main_main_default = lv.style_t()
style_screen_2_btn_2_main_main_default.init()
style_screen_2_btn_2_main_main_default.set_radius(20)
style_screen_2_btn_2_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_2_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_2_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_btn_2_main_main_default.set_bg_opa(255)
style_screen_2_btn_2_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_btn_2_main_main_default.set_border_width(0)
style_screen_2_btn_2_main_main_default.set_border_opa(255)
style_screen_2_btn_2_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_btn_2_main_main_default.set_text_font(lv.font_simsun_15)
except AttributeError:
    try:
        style_screen_2_btn_2_main_main_default.set_text_font(lv.font_montserrat_15)
    except AttributeError:
        style_screen_2_btn_2_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_btn_2_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)

# add style for screen_2_btn_2
screen_2_btn_2.add_style(style_screen_2_btn_2_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_label_2 = lv.label(screen_2_cont_3)
screen_2_label_2.set_pos(int(24),int(61))
screen_2_label_2.set_size(100,12)
screen_2_label_2.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_2.set_text("PASSWD")
screen_2_label_2.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_2_label_2_main_main_default
style_screen_2_label_2_main_main_default = lv.style_t()
style_screen_2_label_2_main_main_default.init()
style_screen_2_label_2_main_main_default.set_radius(0)
style_screen_2_label_2_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_2_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_2_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_label_2_main_main_default.set_bg_opa(0)
style_screen_2_label_2_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_label_2_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_label_2_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_label_2_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_label_2_main_main_default.set_text_letter_space(2)
style_screen_2_label_2_main_main_default.set_text_line_space(0)
style_screen_2_label_2_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_screen_2_label_2_main_main_default.set_pad_left(0)
style_screen_2_label_2_main_main_default.set_pad_right(0)
style_screen_2_label_2_main_main_default.set_pad_top(0)
style_screen_2_label_2_main_main_default.set_pad_bottom(0)

# add style for screen_2_label_2
screen_2_label_2.add_style(style_screen_2_label_2_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_ta_1 = lv.textarea(screen_2_cont_3)
screen_2_ta_1.set_pos(int(42),int(26))
screen_2_ta_1.set_size(200,20)
screen_2_ta_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)


try:
    screen_2_cont_3_kb
except NameError:
    screen_2_cont_3_kb = lv.keyboard(screen_2_cont_3,)

screen_2_cont_3_kb.add_flag(lv.obj.FLAG.HIDDEN)
screen_2_cont_3_kb.set_textarea(screen_2_ta_1)
screen_2_ta_1.add_event_cb(lambda e: ta_event_cb(e,screen_2_cont_3_kb), lv.EVENT.ALL, None)
# create style style_screen_2_ta_1_main_main_default
style_screen_2_ta_1_main_main_default = lv.style_t()
style_screen_2_ta_1_main_main_default.init()
style_screen_2_ta_1_main_main_default.set_radius(4)
style_screen_2_ta_1_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_screen_2_ta_1_main_main_default.set_bg_grad_color(lv.color_make(0xff,0xff,0xff))
style_screen_2_ta_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_ta_1_main_main_default.set_bg_opa(255)
style_screen_2_ta_1_main_main_default.set_border_color(lv.color_make(0xe6,0xe6,0xe6))
style_screen_2_ta_1_main_main_default.set_border_width(2)
style_screen_2_ta_1_main_main_default.set_border_opa(255)
style_screen_2_ta_1_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_ta_1_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_ta_1_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_ta_1_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_ta_1_main_main_default.set_text_letter_space(2)
style_screen_2_ta_1_main_main_default.set_text_align(lv.TEXT_ALIGN.LEFT)
style_screen_2_ta_1_main_main_default.set_pad_left(4)
style_screen_2_ta_1_main_main_default.set_pad_right(4)
style_screen_2_ta_1_main_main_default.set_pad_top(4)
style_screen_2_ta_1_main_main_default.set_pad_bottom(4)

# add style for screen_2_ta_1
screen_2_ta_1.add_style(style_screen_2_ta_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_screen_2_ta_1_main_scrollbar_default
style_screen_2_ta_1_main_scrollbar_default = lv.style_t()
style_screen_2_ta_1_main_scrollbar_default.init()
style_screen_2_ta_1_main_scrollbar_default.set_radius(0)
style_screen_2_ta_1_main_scrollbar_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_ta_1_main_scrollbar_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_ta_1_main_scrollbar_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_ta_1_main_scrollbar_default.set_bg_opa(255)

# add style for screen_2_ta_1
screen_2_ta_1.add_style(style_screen_2_ta_1_main_scrollbar_default, lv.PART.SCROLLBAR|lv.STATE.DEFAULT)

screen_2_label_1 = lv.label(screen_2_cont_3)
screen_2_label_1.set_pos(int(11),int(6))
screen_2_label_1.set_size(100,12)
screen_2_label_1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_1.set_text("SSID")
screen_2_label_1.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_2_label_1_main_main_default
style_screen_2_label_1_main_main_default = lv.style_t()
style_screen_2_label_1_main_main_default.init()
style_screen_2_label_1_main_main_default.set_radius(0)
style_screen_2_label_1_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_1_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_1_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_label_1_main_main_default.set_bg_opa(0)
style_screen_2_label_1_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_label_1_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_label_1_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_label_1_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_label_1_main_main_default.set_text_letter_space(2)
style_screen_2_label_1_main_main_default.set_text_line_space(0)
style_screen_2_label_1_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_screen_2_label_1_main_main_default.set_pad_left(0)
style_screen_2_label_1_main_main_default.set_pad_right(0)
style_screen_2_label_1_main_main_default.set_pad_top(0)
style_screen_2_label_1_main_main_default.set_pad_bottom(0)

# add style for screen_2_label_1
screen_2_label_1.add_style(style_screen_2_label_1_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_screen_2_cont_3_main_main_default
style_screen_2_cont_3_main_main_default = lv.style_t()
style_screen_2_cont_3_main_main_default.init()
style_screen_2_cont_3_main_main_default.set_radius(0)
style_screen_2_cont_3_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_cont_3_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_cont_3_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_cont_3_main_main_default.set_bg_opa(0)
style_screen_2_cont_3_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_cont_3_main_main_default.set_border_width(0)
style_screen_2_cont_3_main_main_default.set_border_opa(255)
style_screen_2_cont_3_main_main_default.set_pad_left(0)
style_screen_2_cont_3_main_main_default.set_pad_right(0)
style_screen_2_cont_3_main_main_default.set_pad_top(0)
style_screen_2_cont_3_main_main_default.set_pad_bottom(0)

# add style for screen_2_cont_3
screen_2_cont_3.add_style(style_screen_2_cont_3_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_ta_2 = lv.textarea(screen_2)
screen_2_ta_2.set_pos(int(42),int(157))
screen_2_ta_2.set_size(200,20)
screen_2_ta_2.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)


try:
    screen_2_kb
except NameError:
    screen_2_kb = lv.keyboard(screen_2,)

screen_2_kb.add_flag(lv.obj.FLAG.HIDDEN)
screen_2_kb.set_textarea(screen_2_ta_2)
screen_2_ta_2.add_event_cb(lambda e: ta_event_cb(e,screen_2_kb), lv.EVENT.ALL, None)
# create style style_screen_2_ta_2_main_main_default
style_screen_2_ta_2_main_main_default = lv.style_t()
style_screen_2_ta_2_main_main_default.init()
style_screen_2_ta_2_main_main_default.set_radius(4)
style_screen_2_ta_2_main_main_default.set_bg_color(lv.color_make(0xff,0xff,0xff))
style_screen_2_ta_2_main_main_default.set_bg_grad_color(lv.color_make(0xff,0xff,0xff))
style_screen_2_ta_2_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_ta_2_main_main_default.set_bg_opa(255)
style_screen_2_ta_2_main_main_default.set_border_color(lv.color_make(0xe6,0xe6,0xe6))
style_screen_2_ta_2_main_main_default.set_border_width(2)
style_screen_2_ta_2_main_main_default.set_border_opa(255)
style_screen_2_ta_2_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_ta_2_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_ta_2_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_ta_2_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_ta_2_main_main_default.set_text_letter_space(2)
style_screen_2_ta_2_main_main_default.set_text_align(lv.TEXT_ALIGN.LEFT)
style_screen_2_ta_2_main_main_default.set_pad_left(4)
style_screen_2_ta_2_main_main_default.set_pad_right(4)
style_screen_2_ta_2_main_main_default.set_pad_top(4)
style_screen_2_ta_2_main_main_default.set_pad_bottom(4)

# add style for screen_2_ta_2
screen_2_ta_2.add_style(style_screen_2_ta_2_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_screen_2_ta_2_main_scrollbar_default
style_screen_2_ta_2_main_scrollbar_default = lv.style_t()
style_screen_2_ta_2_main_scrollbar_default.init()
style_screen_2_ta_2_main_scrollbar_default.set_radius(0)
style_screen_2_ta_2_main_scrollbar_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_ta_2_main_scrollbar_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_ta_2_main_scrollbar_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_ta_2_main_scrollbar_default.set_bg_opa(255)

# add style for screen_2_ta_2
screen_2_ta_2.add_style(style_screen_2_ta_2_main_scrollbar_default, lv.PART.SCROLLBAR|lv.STATE.DEFAULT)

screen_2_label_3 = lv.label(screen_2)
screen_2_label_3.set_pos(int(181),int(51))
screen_2_label_3.set_size(100,32)
screen_2_label_3.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_3.set_text("WIFI链接")
screen_2_label_3.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_2_label_3_main_main_default
style_screen_2_label_3_main_main_default = lv.style_t()
style_screen_2_label_3_main_main_default.init()
style_screen_2_label_3_main_main_default.set_radius(0)
style_screen_2_label_3_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_3_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_3_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_label_3_main_main_default.set_bg_opa(0)
style_screen_2_label_3_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_label_3_main_main_default.set_text_font(lv.font_Alatsi_Regular_16)
except AttributeError:
    try:
        style_screen_2_label_3_main_main_default.set_text_font(lv.font_montserrat_16)
    except AttributeError:
        style_screen_2_label_3_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_label_3_main_main_default.set_text_letter_space(2)
style_screen_2_label_3_main_main_default.set_text_line_space(0)
style_screen_2_label_3_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_screen_2_label_3_main_main_default.set_pad_left(0)
style_screen_2_label_3_main_main_default.set_pad_right(0)
style_screen_2_label_3_main_main_default.set_pad_top(0)
style_screen_2_label_3_main_main_default.set_pad_bottom(0)

# add style for screen_2_label_3
screen_2_label_3.add_style(style_screen_2_label_3_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_cont_4 = lv.obj(screen_2)
screen_2_cont_4.set_pos(int(0),int(275))
screen_2_cont_4.set_size(480,149)
screen_2_cont_4.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_8 = lv.label(screen_2_cont_4)
screen_2_label_8.set_pos(int(20),int(104))
screen_2_label_8.set_size(360,12)
screen_2_label_8.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_8.set_text("DB")
screen_2_label_8.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_2_label_8_main_main_default
style_screen_2_label_8_main_main_default = lv.style_t()
style_screen_2_label_8_main_main_default.init()
style_screen_2_label_8_main_main_default.set_radius(0)
style_screen_2_label_8_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_8_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_8_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_label_8_main_main_default.set_bg_opa(0)
style_screen_2_label_8_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_label_8_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_label_8_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_label_8_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_label_8_main_main_default.set_text_letter_space(2)
style_screen_2_label_8_main_main_default.set_text_line_space(0)
style_screen_2_label_8_main_main_default.set_text_align(lv.TEXT_ALIGN.LEFT)
style_screen_2_label_8_main_main_default.set_pad_left(0)
style_screen_2_label_8_main_main_default.set_pad_right(0)
style_screen_2_label_8_main_main_default.set_pad_top(0)
style_screen_2_label_8_main_main_default.set_pad_bottom(0)

# add style for screen_2_label_8
screen_2_label_8.add_style(style_screen_2_label_8_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_label_7 = lv.label(screen_2_cont_4)
screen_2_label_7.set_pos(int(20),int(81))
screen_2_label_7.set_size(360,12)
screen_2_label_7.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_7.set_text("MAC")
screen_2_label_7.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_2_label_7_main_main_default
style_screen_2_label_7_main_main_default = lv.style_t()
style_screen_2_label_7_main_main_default.init()
style_screen_2_label_7_main_main_default.set_radius(0)
style_screen_2_label_7_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_7_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_7_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_label_7_main_main_default.set_bg_opa(0)
style_screen_2_label_7_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_label_7_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_label_7_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_label_7_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_label_7_main_main_default.set_text_letter_space(2)
style_screen_2_label_7_main_main_default.set_text_line_space(0)
style_screen_2_label_7_main_main_default.set_text_align(lv.TEXT_ALIGN.LEFT)
style_screen_2_label_7_main_main_default.set_pad_left(0)
style_screen_2_label_7_main_main_default.set_pad_right(0)
style_screen_2_label_7_main_main_default.set_pad_top(0)
style_screen_2_label_7_main_main_default.set_pad_bottom(0)

# add style for screen_2_label_7
screen_2_label_7.add_style(style_screen_2_label_7_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_label_6 = lv.label(screen_2_cont_4)
screen_2_label_6.set_pos(int(20),int(60))
screen_2_label_6.set_size(360,12)
screen_2_label_6.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_6.set_text("Wifi_Passwd")
screen_2_label_6.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_2_label_6_main_main_default
style_screen_2_label_6_main_main_default = lv.style_t()
style_screen_2_label_6_main_main_default.init()
style_screen_2_label_6_main_main_default.set_radius(0)
style_screen_2_label_6_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_6_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_6_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_label_6_main_main_default.set_bg_opa(0)
style_screen_2_label_6_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_label_6_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_label_6_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_label_6_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_label_6_main_main_default.set_text_letter_space(2)
style_screen_2_label_6_main_main_default.set_text_line_space(0)
style_screen_2_label_6_main_main_default.set_text_align(lv.TEXT_ALIGN.LEFT)
style_screen_2_label_6_main_main_default.set_pad_left(0)
style_screen_2_label_6_main_main_default.set_pad_right(0)
style_screen_2_label_6_main_main_default.set_pad_top(0)
style_screen_2_label_6_main_main_default.set_pad_bottom(0)

# add style for screen_2_label_6
screen_2_label_6.add_style(style_screen_2_label_6_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_label_5 = lv.label(screen_2_cont_4)
screen_2_label_5.set_pos(int(20),int(37))
screen_2_label_5.set_size(360,12)
screen_2_label_5.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_5.set_text("Wifi_Name")
screen_2_label_5.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_2_label_5_main_main_default
style_screen_2_label_5_main_main_default = lv.style_t()
style_screen_2_label_5_main_main_default.init()
style_screen_2_label_5_main_main_default.set_radius(0)
style_screen_2_label_5_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_5_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_5_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_label_5_main_main_default.set_bg_opa(0)
style_screen_2_label_5_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_label_5_main_main_default.set_text_font(lv.font_simsun_12)
except AttributeError:
    try:
        style_screen_2_label_5_main_main_default.set_text_font(lv.font_montserrat_12)
    except AttributeError:
        style_screen_2_label_5_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_label_5_main_main_default.set_text_letter_space(2)
style_screen_2_label_5_main_main_default.set_text_line_space(0)
style_screen_2_label_5_main_main_default.set_text_align(lv.TEXT_ALIGN.LEFT)
style_screen_2_label_5_main_main_default.set_pad_left(0)
style_screen_2_label_5_main_main_default.set_pad_right(0)
style_screen_2_label_5_main_main_default.set_pad_top(0)
style_screen_2_label_5_main_main_default.set_pad_bottom(0)

# add style for screen_2_label_5
screen_2_label_5.add_style(style_screen_2_label_5_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

screen_2_label_4 = lv.label(screen_2_cont_4)
screen_2_label_4.set_pos(int(181),int(4))
screen_2_label_4.set_size(100,32)
screen_2_label_4.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
screen_2_label_4.set_text("WIFI状态")
screen_2_label_4.set_long_mode(lv.label.LONG.WRAP)
# create style style_screen_2_label_4_main_main_default
style_screen_2_label_4_main_main_default = lv.style_t()
style_screen_2_label_4_main_main_default.init()
style_screen_2_label_4_main_main_default.set_radius(0)
style_screen_2_label_4_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_4_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_label_4_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_label_4_main_main_default.set_bg_opa(0)
style_screen_2_label_4_main_main_default.set_text_color(lv.color_make(0x00,0x00,0x00))
try:
    style_screen_2_label_4_main_main_default.set_text_font(lv.font_Alatsi_Regular_16)
except AttributeError:
    try:
        style_screen_2_label_4_main_main_default.set_text_font(lv.font_montserrat_16)
    except AttributeError:
        style_screen_2_label_4_main_main_default.set_text_font(lv.font_montserrat_16)
style_screen_2_label_4_main_main_default.set_text_letter_space(2)
style_screen_2_label_4_main_main_default.set_text_line_space(0)
style_screen_2_label_4_main_main_default.set_text_align(lv.TEXT_ALIGN.CENTER)
style_screen_2_label_4_main_main_default.set_pad_left(0)
style_screen_2_label_4_main_main_default.set_pad_right(0)
style_screen_2_label_4_main_main_default.set_pad_top(0)
style_screen_2_label_4_main_main_default.set_pad_bottom(0)

# add style for screen_2_label_4
screen_2_label_4.add_style(style_screen_2_label_4_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)

# create style style_screen_2_cont_4_main_main_default
style_screen_2_cont_4_main_main_default = lv.style_t()
style_screen_2_cont_4_main_main_default.init()
style_screen_2_cont_4_main_main_default.set_radius(0)
style_screen_2_cont_4_main_main_default.set_bg_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_cont_4_main_main_default.set_bg_grad_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_cont_4_main_main_default.set_bg_grad_dir(lv.GRAD_DIR.NONE)
style_screen_2_cont_4_main_main_default.set_bg_opa(0)
style_screen_2_cont_4_main_main_default.set_border_color(lv.color_make(0x21,0x95,0xf6))
style_screen_2_cont_4_main_main_default.set_border_width(0)
style_screen_2_cont_4_main_main_default.set_border_opa(255)
style_screen_2_cont_4_main_main_default.set_pad_left(0)
style_screen_2_cont_4_main_main_default.set_pad_right(0)
style_screen_2_cont_4_main_main_default.set_pad_top(0)
style_screen_2_cont_4_main_main_default.set_pad_bottom(0)

# add style for screen_2_cont_4
screen_2_cont_4.add_style(style_screen_2_cont_4_main_main_default, lv.PART.MAIN|lv.STATE.DEFAULT)



def screen_btn_2_released_1_event_cb(e,screen_2):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(screen_2, lv.SCR_LOAD_ANIM.OVER_TOP, 0, 0, False)
screen_btn_2.add_event_cb(lambda e: screen_btn_2_released_1_event_cb(e,screen_2), lv.EVENT.RELEASED, None)


def screen_btn_3_clicked_1_event_cb(e,screen_1):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(screen_1, lv.SCR_LOAD_ANIM.OVER_TOP, 0, 0, False)
screen_btn_3.add_event_cb(lambda e: screen_btn_3_clicked_1_event_cb(e,screen_1), lv.EVENT.CLICKED, None)




def screen_2_btn_1_clicked_1_event_cb(e,screen):
    src = e.get_target()
    code = e.get_code()
    lv.scr_load_anim(screen, lv.SCR_LOAD_ANIM.OVER_TOP, 0, 0, False)
screen_2_btn_1.add_event_cb(lambda e: screen_2_btn_1_clicked_1_event_cb(e,screen), lv.EVENT.CLICKED, None)



# content from custom.py

# Load the default screen
lv.scr_load(screen)

while SDL.check():
    time.sleep_ms(5)
