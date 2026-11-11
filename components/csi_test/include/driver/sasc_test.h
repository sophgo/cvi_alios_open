#ifndef __SASC_TEST__
#define __SASC_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <drv/sasc.h>


// typedef struct {
//     uint32_t    region_id;
//     uint32_t    addr_base;
//     uint32_t    size;
// }test_sasc_args_t;

typedef struct {
    char *name;
    test_func function;
    uint8_t args_num;
} test_sasc_info_t;


extern void ram_rw_trap(void);
extern void ram_rw_no_trap(void);
extern void ram_readNoTrap_writeTrap(void);
extern void ram_readTrap_writeNoTrap(void);

extern int test_sasc_ram_config(void *args);
extern int test_sasc_ram_access_1(void *args);
extern int test_sasc_ram_access_3(void *args);
extern int test_sasc_ram_access_5(void *args);
extern int test_sasc_flash_config(void *args);
extern int test_sasc_flash_access_1(void *args);
extern int test_sasc_flash_access_3(void *args);
extern int test_sasc_flash_access_5(void *args);

#endif