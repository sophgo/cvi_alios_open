#ifndef __CV181X_TPU_CFG__
#define __CV181X_TPU_CFG__

#define CV181X_VER                                       182202
#define CV181X_HW_NPU_SHIFT                              3
#define CV181X_HW_EU_SHIFT                               4
#define CV181X_HW_LMEM_SHIFT                             15
#define CV181X_HW_LMEM_BANKS                             8
#define CV181X_HW_LMEM_BANK_SIZE                         0x1000
#define CV181X_HW_NODE_CHIP_SHIFT                        0
#define CV181X_HW_NPU_NUM                                (1 << CV181X_HW_NPU_SHIFT)
#define CV181X_HW_EU_NUM                                 (1 << CV181X_HW_EU_SHIFT)
#define CV181X_HW_LMEM_SIZE                              (1 << CV181X_HW_LMEM_SHIFT)
#define CV181X_HW_LMEM_START_ADDR                        0x0C000000
#define CV181X_HW_NODE_CHIP_NUM                          (1 << CV181X_HW_NODE_CHIP_SHIFT)

#if (CV181X_HW_LMEM_SIZE != (CV181X_HW_LMEM_BANK_SIZE * CV181X_HW_LMEM_BANKS))
#error "Set wrong TPU configuration."
#endif

#define CV181X_GLOBAL_MEM_START_ADDR                     0x0
#define CV181X_GLOBAL_MEM_SIZE                           0x100000000 // 

#define CV181X_GLOBAL_TIU_CMDBUF_ADDR                    0x00000000
#define CV181X_GLOBAL_TDMA_CMDBUF_ADDR                   0x00800000
#define CV181X_GLOBAL_TIU_CMDBUF_RESERVED_SIZE           0x00800000 // 8MB
#define CV181X_GLOBAL_TDMA_CMDBUF_RESERVED_SIZE          0x00800000 // 8MB
#define CV181X_GLOBAL_POOL_RESERVED_SIZE                (CV181X_GLOBAL_MEM_SIZE - CV181X_GLOBAL_TIU_CMDBUF_RESERVED_SIZE - CV181X_GLOBAL_TDMA_CMDBUF_RESERVED_SIZE)

#define CV181X_UART_CTLR_BASE_ADDR                       0x04140000

#define CV181X_TDMA_ENGINE_BASE_ADDR                     0x0C100000
#define CV181X_TDMA_ENGINE_END_ADDR                      (CV181X_TDMA_ENGINE_BASE_ADDR + 0x1000)

#define CV181X_TIU_ENGINE_BASE_ADDR                      0x0C101000 //"NPS Register" in memory map?
#define CV181X_TIU_ENGINE_END_ADDR                       (CV181X_TIU_ENGINE_BASE_ADDR + 0x1000)

#endif
