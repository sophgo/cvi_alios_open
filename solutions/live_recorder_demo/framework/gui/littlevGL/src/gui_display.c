#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/cli.h>
#include <aos/kernel.h>
#include <lvgl.h>
#include "gui_display.h"
#include "aos/cli.h"
#include "cvi_mipi_tx.h"
#include "cvi_region.h"
#include "fatfs_vfs.h"
#include "cvi_math.h"
#include "cvi_vb.h"
#include "cvi_vo.h"
#include "cvi_buffer.h"
#include "vfs.h"
#include "cvi_param.h"

#define OverlayMinHandle 10

static CVI_S32 g_GuiRunStatus = 0;
static pthread_t g_GuiTaskId;
static RGN_CANVAS_INFO_S g_stCanvasInfo = {0};
static int g_GuiDisplayCloseStatus = 0;
lv_obj_t *g_GuiLableObj = NULL;

static void lv_event_cb(struct _lv_obj_t * obj, lv_event_t event)
{
    char *data = (char *)lv_event_get_data();
    if(data) {
        //printf("lv_event_cb data is %s \n",data);
    }
    if(event == LV_EVENT_PRESSED) {
        lv_label_set_text(obj,data);
    } else {
        lv_label_set_text(obj," ");
    }
}

void Gui_Label_SendEvent(CVI_U8 EventType,void *Data)
{
    if(g_GuiRunStatus == 1) {
        lv_event_send(g_GuiLableObj,EventType,Data);
    }
}

static void gui_label_create(void)
{
    g_GuiLableObj = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(g_GuiLableObj, LV_LABEL_LONG_BREAK);
    lv_label_set_align(g_GuiLableObj, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_pos(g_GuiLableObj, 0, 0);
    lv_obj_set_size(g_GuiLableObj, 128, 128);
    lv_label_set_text(g_GuiLableObj, " ");
    lv_obj_set_event_cb(g_GuiLableObj,lv_event_cb);
}

static void *_gui_display_task(void *arg)
{
    char szThreadName[20]="gui_display_task";
    prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
    gui_label_create();
    while(g_GuiRunStatus) {
        lv_task_handler();
        aos_msleep(3);
        lv_tick_inc(1);
    }
    return NULL;
}

static int osd_create_overlayex()
{
    int s32Ret;
    RGN_ATTR_S stRegion = {0};
    RGN_CHN_ATTR_S stRgnChnAttr = {0};
    MMF_CHN_S stMMFChn = {0};

    stRegion.enType = OVERLAY_RGN;
    stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
#if (CONFIG_PANEL_ILI9488 == 1)
    stRegion.unAttr.stOverlay.stSize.u32Width = PANEL_WIDTH;
    stRegion.unAttr.stOverlay.stSize.u32Height = PANEL_HEIGHT;
#else
    stRegion.unAttr.stOverlay.stSize.u32Width = PANEL_HEIGHT;
    stRegion.unAttr.stOverlay.stSize.u32Height = PANEL_WIDTH;
#endif
    stRegion.unAttr.stOverlay.u32BgColor = 0x00000000; // transparent
    stRegion.unAttr.stOverlay.u32CanvasNum = 1;

    s32Ret = CVI_RGN_Create(OverlayMinHandle, &stRegion);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    stMMFChn.enModId = CVI_ID_VPSS;
#ifdef CONFIG_GUI_RGN_DEVID
    stMMFChn.s32DevId = CONFIG_GUI_RGN_DEVID;
#else
    stMMFChn.s32DevId = 0;
#endif
#ifdef CONFIG_GUI_RGN_CHNID
    stMMFChn.s32ChnId = CONFIG_GUI_RGN_CHNID;
#else
    stMMFChn.s32ChnId = 1;
#endif
    stRgnChnAttr.bShow = CVI_TRUE;
    stRgnChnAttr.enType = OVERLAY_RGN;
    stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
    stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
    stRgnChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;

    s32Ret = CVI_RGN_AttachToChn(OverlayMinHandle, &stMMFChn, &stRgnChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    return s32Ret;
}

static void GUI_DisplayFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    lv_coord_t i = 0,j = 0;
    uint16_t area_hor = area->x2 - area->x1 + 1;
    unsigned int offset = 0;
    for(j = area->y1; j <= area->y2; j++) {
        offset = (area->x1 + (j * disp_drv->hor_res)) * sizeof(lv_color_t);
        memcpy(g_stCanvasInfo.pu8VirtAddr + offset, color_p + area_hor * i, area_hor * sizeof(lv_color_t));
        i++;
    }
    if(g_GuiDisplayCloseStatus == 1) {
        if(g_stCanvasInfo.enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
            uint16_t * tmpbuf = (uint16_t *) g_stCanvasInfo.pu8VirtAddr;
            for(i = 0; i < g_stCanvasInfo.stSize.u32Width; i++) {
                for(j = 0; j < g_stCanvasInfo.stSize.u32Height; j++) {
                    tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] &= 0x7FFF;
                }
            }
        }
        if(g_stCanvasInfo.enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
            uint32_t * tmpbuf = (uint32_t *) g_stCanvasInfo.pu8VirtAddr;
            for(i = 0; i < g_stCanvasInfo.stSize.u32Width; i++) {
                for(j = 0; j < g_stCanvasInfo.stSize.u32Height; j++) {
                    tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] &= 0x00FFFFFF;
                }
            }
        }
    }
#if 0
    if(g_stCanvasInfo.enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
        uint16_t * tmpbuf = (uint16_t *) g_stCanvasInfo.pu8VirtAddr;
        uint8_t _r5 = 0;
        uint8_t _g6 = 0;
        uint8_t _b5 = 0;
        for(i = 0; i < g_stCanvasInfo.stSize.u32Width; i++) {
            for(j = 0; j < g_stCanvasInfo.stSize.u32Height; j++) {
                _r5 = (tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] >> 11) & 0x1F;
                _g6 = (tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] >> 5) & 0x3F;
                _b5 = (tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] ) & 0x1F;
                //printf("_r5 %d _g6 %d _b5 %d \n",_r5,_g6,_b5);
                if(_r5 >= 28 && _g6 >= 57 && _b5 >= 28) {
                    tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] &= 0x7FFF;//ARGB  A set 0
                } else {
                    tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] |= 0x8000;//ARGB  A set 1
                }
            }
        }
    }
    if(g_stCanvasInfo.enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
        uint32_t * tmpbuf = (uint32_t *) g_stCanvasInfo.pu8VirtAddr;
        uint8_t _r8 = 0;
        uint8_t _g8 = 0;
        uint8_t _b8 = 0;
        for(i = 0; i < g_stCanvasInfo.stSize.u32Width; i++) {
            for(j = 0; j < g_stCanvasInfo.stSize.u32Height; j++) {
                _r8 = (tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] >> 16) &0xFF;
                _g8 = (tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] >> 8) &0xFF;
                _b8 = (tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i]) &0xFF;
                if((_r8 > 230 && _b8 > 230 && _g8 > 230)) {
                    tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] &= 0x00FFFFFF;
                } else {
                    tmpbuf[j * g_stCanvasInfo.stSize.u32Width + i] |= 0xFF000000;
                }
            }
        }
    }
#endif
    lv_disp_flush_ready(disp_drv);
}

CVI_S32 GUI_Display_LVGLInit()
{
    lv_init();
    static lv_disp_buf_t guiDispBuf = {0};
    static lv_color_t *flushbuf = NULL;
    if(flushbuf == NULL) {
        flushbuf = (lv_color_t *)aos_malloc(g_stCanvasInfo.stSize.u32Height * g_stCanvasInfo.stSize.u32Width* sizeof(lv_color_t));
    }
    if(flushbuf) {
        lv_disp_buf_init(&guiDispBuf, flushbuf, NULL, g_stCanvasInfo.stSize.u32Height * g_stCanvasInfo.stSize.u32Width);
    }
    lv_disp_drv_t disp_drv = {0};
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/
    disp_drv.buffer   = &guiDispBuf;
    disp_drv.flush_cb = GUI_DisplayFlush;
    disp_drv.rotated  = 0;
    lv_disp_drv_register(&disp_drv);
    return CVI_SUCCESS;
}

CVI_S32 GUI_Display_Start()
{
    CVI_S32 s32Ret = 0;

    if(g_stCanvasInfo.pu8VirtAddr == NULL) {
        osd_create_overlayex();
        s32Ret = CVI_RGN_GetCanvasInfo(OverlayMinHandle, &g_stCanvasInfo);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
            return CVI_FAILURE;
        }
    }

    printf("stCanvasInfo.pu8VirtAddr:%p,width:%d height:%d\n",
        g_stCanvasInfo.pu8VirtAddr, g_stCanvasInfo.stSize.u32Width, g_stCanvasInfo.stSize.u32Height);
    GUI_Display_LVGLInit();
    g_GuiRunStatus = 1;
    if (pthread_create(&g_GuiTaskId, NULL, _gui_display_task, NULL) == -1) {
        g_GuiRunStatus = 0;
        printf("create main ui task fail\n");
        return CVI_FAILURE;
    }
    printf("GUI_Display_Start success \n");
    return CVI_SUCCESS;
}

CVI_S32 GUI_Display_Stop()
{
    g_GuiRunStatus = 0;
    pthread_join(g_GuiTaskId,NULL);
    lv_deinit();
    return CVI_SUCCESS;
}

//Gui_Label_SendEvent(LV_EVENT_PRESSED,"12345");
void Send_FaceRecognition_event(int argc, char **argv)
{
    Gui_Label_SendEvent(LV_EVENT_PRESSED,"recognition success");
}

void Send_DisFaceRecognition_event(int argc, char **argv)
{
    Gui_Label_SendEvent(LV_EVENT_PRESS_LOST," ");
}

void GUI_Dispaly_Switch(int argc, char **argv)
{
    if(argc < 2) {
        printf("Input 0 close gui\n");
        printf("Input 1 open gui\n");
        return ;
    }
    lv_obj_t *ptmp = lv_scr_act();
    int display_switch = atoi(argv[1]);
    if(display_switch == 0) {
        printf("close display \n");
        g_GuiDisplayCloseStatus = 1;
        lv_obj_set_hidden(ptmp,true);
    } else {
        printf("open display \n");
        g_GuiDisplayCloseStatus = 0;
        lv_obj_set_hidden(ptmp,false);
    }
}

ALIOS_CLI_CMD_REGISTER(Send_FaceRecognition_event,test_gui_FaceRecognition,test_gui_press)
ALIOS_CLI_CMD_REGISTER(Send_DisFaceRecognition_event,test_gui_DisFaceRecognition,test_gui_dispress)
ALIOS_CLI_CMD_REGISTER(GUI_Dispaly_Switch,gui_display_switch,gui_display_switch)
