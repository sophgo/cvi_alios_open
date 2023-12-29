
#include <drv/adc.h>
#include "cvi_adc.h"

csi_error_t csi_adc_init(csi_adc_t *adc, uint32_t idx)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    if (target_get(DEV_CVI_ADC_TAG, idx, &adc->dev) != CSI_OK) {
        aos_cli_printf("adc %d init failed!\n", idx);
        ret = CSI_ERROR;
	}

	cvi_adc_t *cvi_adc = (cvi_adc_t *)malloc(sizeof(*cvi_adc));
	if (!cvi_adc)
	{
		aos_cli_printf("adc %d malloc failed!\n", idx);
		return CSI_ERROR;
	}

	cvi_adc->dev.reg_base = GET_DEV_REG_BASE(adc);
	cvi_adc->dev.idx = GET_DEV_IDX(adc);
	cvi_adc->dev.irq_num = GET_DEV_IRQ_NUM(adc);
	ret = (csi_error_t)cvi_adc_init(cvi_adc);

	adc->priv = cvi_adc;

    return ret;
}

void csi_adc_uninit(csi_adc_t *adc)
{
    CSI_PARAM_CHK_NORETVAL(adc);
	cvi_adc_uninit(adc->priv);
	free(adc->priv);
}

csi_error_t csi_adc_set_buffer(csi_adc_t *adc, uint32_t *data, uint32_t num)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(num, CSI_ERROR);
	return cvi_adc_set_buffer(adc->priv, data, num);
}

csi_error_t csi_adc_start(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
	return cvi_adc_start(adc->priv);
}


csi_error_t csi_adc_start_async(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
	return cvi_adc_start_async(adc->priv);
}

csi_error_t csi_adc_stop(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return cvi_adc_stop(adc->priv);
}

csi_error_t csi_adc_stop_async(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return cvi_adc_stop_async(adc->priv);
}

csi_error_t csi_adc_channel_enable(csi_adc_t *adc, uint8_t ch_id, bool is_enable)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return cvi_adc_channel_enable(adc->priv, ch_id, is_enable);
}

// csi_error_t csi_adc_channel_sampling_time(csi_adc_t *adc, uint8_t ch_id, uint16_t clock_num)
// {
//     return CSI_OK;
// }

csi_error_t csi_adc_sampling_time(csi_adc_t *adc, uint16_t clock_num)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return cvi_adc_sampling_time(adc->priv, clock_num);
}

csi_error_t csi_adc_continue_mode(csi_adc_t *adc, bool is_enable)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return CSI_UNSUPPORTED;
}

uint32_t csi_adc_freq_div(csi_adc_t *adc, uint32_t div)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return cvi_adc_freq_div(adc->priv, div);
}

int32_t csi_adc_read(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return cvi_adc_read(adc->priv);
}

csi_error_t csi_adc_get_state(csi_adc_t *adc, csi_state_t *state)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return cvi_adc_get_state(adc->priv, (cvi_state_t *)state);
}

uint32_t csi_adc_get_freq(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, 0U);
	return cvi_adc_get_freq(adc->priv);
}

csi_error_t csi_adc_attach_callback(csi_adc_t *adc, void *callback, void *arg)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    CSI_PARAM_CHK(callback, CSI_ERROR);
	return cvi_adc_configure_irq(adc->priv, callback, arg);
}

void csi_adc_detach_callback(csi_adc_t *adc)
{
    CSI_PARAM_CHK_NORETVAL(adc);
	cvi_adc_configure_irq(adc->priv, NULL, NULL);
}

#define ADC_IRQ_TEST 0

#if ADC_IRQ_TEST
static void adc_irq_test(csi_adc_t *adc)
{
	aos_cli_printf("goto %s() get result: 0x%x\n", __func__, adc->data);
    cvi_adc_start(adc);
}
#endif

csi_error_t csi_adc_link_dma(csi_adc_t *adc, csi_dma_ch_t *dma)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return CSI_UNSUPPORTED;
}