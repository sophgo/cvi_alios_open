
#include "efuse_test.h"


int test_efuse_readprogram(void *args)
{
    csi_efuse_info_t info;
    csi_error_t ret;
    csi_efuse_t efuse;
    test_efuse_args_t efuse_args;
    uint8_t *tx_data = NULL;
    uint8_t *rx_data = NULL;

    efuse_args.idx  = *((uint64_t *)args);
    efuse_args.addr = *((uint64_t *)args + 1);
    efuse_args.size = *((uint64_t *)args + 2);
    TEST_CASE_TIPS("test efuse idx is %d, addr is %d, size is %d", efuse_args.idx, efuse_args.addr, efuse_args.size);

    ret = csi_efuse_init(&efuse, 0);
    TEST_CASE_ASSERT(ret != CSI_ERROR, "efuse init error");

    ret = csi_efuse_get_info(&efuse, &info);
    TEST_CASE_ASSERT(ret != CSI_ERROR, "efuse get info error");
    TEST_CASE_TIPS("efuse info -- satrt %d", info.start);
    TEST_CASE_TIPS("efuse info -- end %d", info.end);


    tx_data = (uint8_t *)malloc(sizeof(uint8_t) * efuse_args.size);
    TEST_CASE_ASSERT(tx_data != NULL, "tx_data malloc error");
    rx_data = (uint8_t *)malloc(sizeof(uint8_t) * efuse_args.size);
    TEST_CASE_ASSERT(rx_data != NULL, "rx_data malloc error");

    memset(rx_data, 0, sizeof(uint8_t) * efuse_args.size);
    ret = csi_efuse_read(&efuse, efuse_args.addr, rx_data, efuse_args.size);
    TEST_CASE_ASSERT(ret != CSI_ERROR, "efuse read error");    
    memset(tx_data, 0xf, sizeof(uint8_t) * efuse_args.size);
    ret = memcmp(tx_data, rx_data, efuse_args.size);
    TEST_CASE_ASSERT(ret != 0, "this efus adr has been write,c please choose another adr");

    ret = csi_efuse_program(&efuse, efuse_args.addr, tx_data, efuse_args.size);
    TEST_CASE_ASSERT(ret != CSI_ERROR, "efuse program error");

    memset(rx_data, 0, sizeof(uint8_t) * efuse_args.size);
    ret = csi_efuse_read(&efuse, efuse_args.addr, rx_data, efuse_args.size);
    TEST_CASE_ASSERT(ret != CSI_ERROR, "efuse read error");    
    ret = memcmp(tx_data, rx_data, efuse_args.size);
    TEST_CASE_ASSERT(ret == 0, "csi efuse read and program not equal");

    memset(tx_data, 0x0, sizeof(uint8_t) * efuse_args.size);
    memset(rx_data, 0x0, sizeof(uint8_t) * efuse_args.size);
    ret = csi_efuse_program(&efuse, efuse_args.addr, tx_data, efuse_args.size);
    TEST_CASE_ASSERT(ret != CSI_ERROR, "efuse program error");
    ret = csi_efuse_read(&efuse, efuse_args.addr, rx_data, efuse_args.size);
    TEST_CASE_ASSERT(ret != CSI_ERROR, "efuse read error");    
    memset(tx_data, 0xf, sizeof(uint8_t) * efuse_args.size);
    ret = memcmp(tx_data, rx_data, efuse_args.size);
    TEST_CASE_ASSERT(ret == 0, "csi efuse read and program not equal");

    free(tx_data);
    free(rx_data);

    csi_efuse_uninit(&efuse);

    return 0;
}
