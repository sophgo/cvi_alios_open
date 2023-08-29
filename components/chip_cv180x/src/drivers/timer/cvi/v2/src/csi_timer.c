/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

/*******************************************************
 * @file    dw_timer.c
 * @brief   source file for timer csi driver
 * @version V1.0
 * @date    23. Sep 2020
 * ******************************************************/

#include <csi_config.h>
#include <drv/timer.h>
#include <drv/cvi_irq.h>
#include <cvi_timer.h>

/**
  \brief       Initialize TIMER Interface. 1. Initializes the resources needed for the TIMER interface 2.registers callback function
  \param[in]   timer    handle timer handle to operate
  \param[in]   idx      timer index
  \return      error code \ref csi_error_t
*/
csi_error_t csi_timer_init(csi_timer_t *timer, uint32_t idx)
{
    CSI_PARAM_CHK(timer, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    uint8_t irq_num;
    dw_timer_regs_t *timer_base = NULL;

    if (0 == target_get(DEV_DW_TIMER_TAG, idx, &timer->dev)) {
        cvi_timer_t *cvi_timer = malloc(sizeof(cvi_timer_t));
        if(!cvi_timer)
            return CSI_ERROR;
        memset(cvi_timer, 0, sizeof(cvi_timer_t));
        timer_base = (dw_timer_regs_t *)HANDLE_REG_BASE(timer);
        irq_num = timer->dev.irq_num;
        ret = cvi_timer_init(cvi_timer, idx, timer_base, irq_num);
        timer->priv = cvi_timer;
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}
/**
  \brief       De-initialize TIMER Interface. stops operation and releases the software resources used by the interface
  \param[in]   timer    handle timer handle to operate
  \return      None
*/
void csi_timer_uninit(csi_timer_t *timer)
{
    CSI_PARAM_CHK_NORETVAL(timer);

    cvi_timer_t *cvi_timer = timer->priv;
    cvi_timer_uninit(cvi_timer);
}
/**
  \brief       Start timer
  \param[in]   timer         handle timer handle to operate
  \param[in]   timeout_us    the timeout for timer
  \return      error code \ref csi_error_t
*/
csi_error_t csi_timer_start(csi_timer_t *timer, uint32_t timeout_us)
{
    CSI_PARAM_CHK(timer, CSI_ERROR);
    CSI_PARAM_CHK(timeout_us, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    ret = cvi_timer_start(timer->priv, timeout_us);

    return ret;
}
/**
  \brief       Stop timer
  \param[in]   timer    handle timer handle to operate
  \return      None
*/
void csi_timer_stop(csi_timer_t *timer)
{
    CSI_PARAM_CHK_NORETVAL(timer);

    cvi_timer_stop(timer->priv);
}
/**
  \brief       Get timer remaining value
  \param[in]   timer    handle timer handle to operate
  \return      the remaining value
*/
uint32_t csi_timer_get_remaining_value(csi_timer_t *timer)
{
    CSI_PARAM_CHK(timer, 0U);

    return cvi_timer_get_remaining_value(timer->priv);
}
/**
  \brief       Get timer load value
  \param[in]   timer    handle timer handle to operate
  \return      the load value
*/
uint32_t csi_timer_get_load_value(csi_timer_t *timer)
{
    CSI_PARAM_CHK(timer, 0U);

    return (cvi_timer_get_load_value(timer->priv));
}
/**
  \brief       Check timer is running
  \param[in]   timer    handle timer handle to operate
  \return      true->running, false->stopped
*/
bool csi_timer_is_running(csi_timer_t *timer)
{
    CSI_PARAM_CHK(timer, false);

    return (cvi_timer_is_running(timer->priv));
}

/**
  \brief       Attach the callback handler to timer
  \param[in]   timer       operate handle.
  \param[in]   callback    callback function
  \param[in]   arg         callback's param
  \return      error code \ref csi_error_t
*/
csi_error_t csi_timer_attach_callback(csi_timer_t *timer, void *callback, void *arg)
{
    CSI_PARAM_CHK(timer, CSI_ERROR);

    return cvi_timer_attach_callback(timer->priv, callback, arg);
}

/**
  \brief       Detach the callback handler
  \param[in]   timer    operate handle.
*/
void csi_timer_detach_callback(csi_timer_t *timer)
{
    CSI_PARAM_CHK_NORETVAL(timer);

    cvi_timer_detach_callback(timer->priv);
}

#ifdef CONFIG_PM
csi_error_t dw_timer_pm_action(csi_dev_t *dev, csi_pm_dev_action_t action)
{
    CSI_PARAM_CHK(dev, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    csi_pm_dev_t *pm_dev = &dev->pm_dev;

    switch (action) {
        case PM_DEV_SUSPEND:
            csi_pm_dev_save_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 1U);
            csi_pm_dev_save_regs(pm_dev->reten_mem + 1, (uint32_t *)(dev->reg_base + 8U), 1U);
            break;

        case PM_DEV_RESUME:
            csi_pm_dev_restore_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 1U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 1, (uint32_t *)(dev->reg_base + 8U), 1U);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

csi_error_t csi_timer_enable_pm(csi_timer_t *timer)
{
    return csi_pm_dev_register(&timer->dev, dw_timer_pm_action, 8U, 0U);
}

void csi_timer_disable_pm(csi_timer_t *timer)
{
    csi_pm_dev_unregister(&timer->dev);
}
#endif

