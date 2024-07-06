#include "efuse_fastboot.h"

#if (CONFIG_ENABLE_FASTBOOT == 1)
void efuse_fastboot() {
    csi_efuse_t efuse = {0};
    csi_efuse_init(&efuse, 0);
    int ret = CVI_EFUSE_EnableFastBoot();
    if (ret == CVI_SUCCESS) {
        printf("fast boot enable\n");
    } else {
        printf("CVI_EFUSE_EnableFastBoot ret=%d\n", ret);
    }

    ret = CVI_EFUSE_IsFastBootEnabled();
    if (ret == CVI_SUCCESS) {
        printf("fast boot enable\n");
    } else {
        printf("CVI_EFUSE_IsFastBootEnabled ret=%d\n", ret);
    }

    csi_efuse_uninit(&efuse);
}
#endif