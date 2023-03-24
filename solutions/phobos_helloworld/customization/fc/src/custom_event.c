#include <stdio.h>
#include "fc_custom_media.h"
#include "custom_event.h"
#include "wifi_dispatch.h"
#include "event_dispatch.h"
#include "global_sleep_flag.h"
#include "cvi_ae.h"
#include "rtsp_func.h"

int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1 && CONFIG_APP_CX_CLOUD_SUPPORT == 1)
    DoorbellModule_WifiDispatchStart();
    DoorbellModule_EventDispatchStart();
#endif
#if (CONFIG_APP_RTSP_SUPPORT == 1)
    cvi_rtsp_init();
#endif
    return 0;
}
