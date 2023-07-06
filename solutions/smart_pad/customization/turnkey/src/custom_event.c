#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"
#include "gui_display.h"
#include "custom_videomedia.h"
#include "touchscreen.h"
#include "display_panel.h"

int APP_CustomEventStart(void)
{
    //APP_CustomEventMedia();
    Touchscreen_Init();
    Display_Panel_Init();
    return 0;
}