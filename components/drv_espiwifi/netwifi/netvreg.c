#include "netvreg.h"
#include "platform_os.h"

vnet_reg_t g_vnet_reg;

vnet_reg_t *vnet_reg_get_addr(void)
{
    return &g_vnet_reg;
}

void vnet_reg_init(void)
{
    vnet_reg_t *vreg = vnet_reg_get_addr();

    platform_memory_set(vreg, 0, sizeof(vnet_reg_t));
}
