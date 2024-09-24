#include <stdio.h>
#include <unistd.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"
#if CONFIG_DUALOS_NO_CROP
#include "gui_display.h"
#include "app_ai.h"
#endif //#if CONFIG_DUALOS_NO_CROP

int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);
#if (CONFIG_APP_GUI_SUPPORT == 1)
    GUI_Display_Start();
#endif
    return 0;
}
