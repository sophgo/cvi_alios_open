/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */


#include <irq_test.h>

/**
 * description: 测试中断开启或关闭
 */
int test_irq_handle(void *args)
{
    uint32_t psr_state = 0;

    TEST_CASE_TIPS("begin test IRQ ");
    /*禁止CPU 中断*/
    __disable_irq();
    /*禁止CPU 异常和中断*/
    __disable_excp_irq();
    /*使能CPU 中断*/
    __enable_irq();
    /*使能CPU 异常和中断*/
    __enable_excp_irq();

    /*保存处理器状态寄存器值，并禁止CPU 中断*/
    psr_state = csi_irq_save();
    TEST_CASE_TIPS("IRQ psr_state:0x%x", psr_state);

    /*恢复处理器状态寄存器值*/
    csi_irq_restore(psr_state);
    TEST_CASE_TIPS("finish test IRQ ");

    return 0;
}
