#include <stdio.h>
#include <unistd.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"


int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);
    return 0;
}