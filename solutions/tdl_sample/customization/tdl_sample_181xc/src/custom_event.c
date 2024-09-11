#include <stdio.h>
#include <unistd.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"
#include "rtsp_func.h"
#include "gui_display.h"
#include "app_ai.h"
#include "rtsp_func.h"

int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);
    return 0;
}
