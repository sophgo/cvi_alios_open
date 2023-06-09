#include <stdio.h>
#include <unistd.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"
#include "rtsp_func.h"
#include "gui_display.h"
#include "app_ai.h"
#include "rtsp_func.h"
#include "touchscreen.h"
#include "display_panel.h"

int APP_CustomEventStart(void)
{
//     printf("%s %d.\n", __FUNCTION__, __LINE__);
// #if (CONFIG_APP_GUI_SUPPORT == 1)
//     GUI_Display_Start();
// #endif
    Touchscreen_Init();
    Display_Panel_Init();
#if (CONFIG_APP_RTSP_SUPPORT == 1)
    cvi_rtsp_init();
#endif

    return 0;
}