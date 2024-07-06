/**
 * @file lv_port_disp_templ.c
 *
 */

#include <stdbool.h>
#include <assert.h>
#include <devices/display.h>
#include <ulog/ulog.h>

#include "../../lvgl.h"

static rvm_dev_t *    disp_dev;
static rvm_hal_display_info_t disp_info;
static void           disp_init(void);

static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *color_p);
static void disp_event(rvm_dev_t *dev, int event_id, void *priv);

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();
    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t * disp = lv_display_create(disp_info.x_res, disp_info.y_res);
    lv_display_set_flush_cb(disp, disp_flush);

    if (disp_info.supported_feature & DISPLAY_FEATURE_ONE_FB) {
        lv_display_set_buffers(disp, disp_info.smem_start[0], NULL, disp_info.x_res * disp_info.y_res, LV_DISPLAY_RENDER_MODE_FULL);
    } else if (disp_info.supported_feature & DISPLAY_FEATURE_DOUBLE_FB) {
        lv_display_set_buffers(disp, disp_info.smem_start[0], (uint8_t *)disp_info.smem_start[1], disp_info.x_res * disp_info.y_res, LV_DISPLAY_RENDER_MODE_FULL);
    } else {
        /** TODO: support display_write_area_async display_write_area */
        assert(0);
    }

    rvm_hal_display_set_event(disp_dev, disp_event, &disp);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void disp_event(rvm_dev_t *dev, int event_id, void *priv)
{
    lv_disp_drv_t *drv = (lv_disp_drv_t*)priv;

    if (event_id == DISPLAY_EVENT_FLUSH_DONE) {
        /*IMPORTANT!!!
        *Inform the graphics library that you are ready with the flushing*/
        lv_disp_flush_ready(drv);
    }
}

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    disp_dev = rvm_hal_display_open("disp");
    assert(disp_dev);

    rvm_hal_display_get_info(disp_dev, &disp_info);
    printf("LV_COLOR_DEPTH=%d\n", LV_COLOR_DEPTH);
    /** assert here cause display pixel format mismatch */
    assert(disp_info.bits_per_pixel == LV_COLOR_DEPTH);
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *color_p)
{
    static int first = 0;

    rvm_hal_display_pan_display(disp_dev);

    if (first == 0) {
        LOGI("lvgl", "first frame out");
        first = 1;
    }
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
// static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}
