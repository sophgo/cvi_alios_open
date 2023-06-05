#include "platform_os.h"
#include "csi_core.h"

unsigned int g_sch_discnt = 0;

unsigned int platform_irq_save(void)
{
    unsigned int flag = csi_irq_save();

    return flag;
}

void platform_irq_restore(unsigned int flag)
{
    csi_irq_restore(flag);
}
