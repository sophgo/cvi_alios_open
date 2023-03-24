#include <stdio.h>
#include "fc_custom_media.h"
#include "custom_event.h"
#include "gui_display.h"
#include "rtsp_func.h"

int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);
#if (CONFIG_APP_GUI_SUPPORT == 1)
    GUI_Display_Start();
#endif
#if (CONFIG_APP_RTSP_SUPPORT == 1)
    cvi_rtsp_init();
#endif
    return 0;
}
