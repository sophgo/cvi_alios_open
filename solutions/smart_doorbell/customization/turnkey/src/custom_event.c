#include <stdio.h>
#include <unistd.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"
#include "rtsp_func.h"
#include "gui_display.h"
#include "app_ai.h"
#include "rtsp_func.h"

#if CONFIG_PIXALGO_FACEKIT_SUPPORT
#include "uart_communication.h"
#include "protocol.h"
#include "cvi_tpu_interface.h"
#include "pix_algokit_api_custom_face.h"
#endif
int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);
#if (CONFIG_APP_RTSP_SUPPORT == 1)
    cvi_rtsp_init();
#endif
#if CONFIG_PIXALGO_FACEKIT_SUPPORT
    int ret;
    ret = UartComm_Init();
    printf("uart 1 init done(%d)\n", ret);
    protocol_init();
    cvi_tpu_init();
    facekit_init();
#endif
    return 0;
}
