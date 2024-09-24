/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <stdint.h>
#include <soc.h>
#include <csi_core.h>
#include <csi_config.h>
#include <drv/common.h>
#include <drv/pin.h>
#include <drv/cvi_irq.h>
#include <drv/dma.h>
#include <cvi_dma.h>

csi_error_t csi_dma_init(csi_dma_t *dma, int8_t ctrl_id)
{
	CSI_PARAM_CHK(dma, CSI_ERROR);
	csi_error_t ret = CSI_OK;
	dw_dma_t *dw_dma;

	ret = target_get(DEV_DW_DMA_TAG, (uint32_t) ctrl_id, &dma->dev);
	if(ret){
		return ret;
	}

	dma_dbg("dma get: ctrl_id=%d, dev->reg_base=%lx\r\n", ctrl_id, dma->dev.reg_base);

	dw_dma = malloc(sizeof(dw_dma_t));
	if(!dw_dma){
		dma_err("error, %s, %d\r\n", __func__, __LINE__);
		return CSI_ERROR;
	}
	memset(dw_dma, 0, sizeof(dw_dma_t));

	dma->priv = dw_dma;

	dma->ch_num = DW_DMA_MAX_NR_CHANNELS;

	ret = cvi_dma_init(dw_dma, ctrl_id, dma->dev.reg_base, dma->dev.irq_num);
	if(ret){
		dma_err("cvi_dma_init failed\r\n");
		free(dw_dma);
		return CSI_ERROR;
	}

	return 0;
}

void csi_dma_uninit(csi_dma_t *dma)
{
	CSI_PARAM_CHK_NORETVAL(dma);
	struct dw_dma *dw = dma->priv;

	cvi_dma_uninit(dw);

	free(dw);
}

extern const csi_dma_ch_spt_list_t dma_spt_list[];

csi_error_t cvi_get_ch_info_from_dev(csi_dev_t *dev_info, void *ch_info)
{
	uint32_t spt_id, ch_info_id;
    int32_t is_found = 0;
    uint8_t ctrl_id;
    csi_dma_ch_desc_t *dma_ch_info = (csi_dma_ch_desc_t *)ch_info;
	uint32_t alloc_status;
	uint32_t irq_flags;

	if(!dev_info)
		return CSI_ERROR;

	/* the MEM2PERH mode or PERH2MEM mode */
	for (spt_id = 0U; dma_spt_list[spt_id].dev_tag != 0xFFFFU; spt_id++) {
		if ((dma_spt_list[spt_id].dev_tag == dev_info->dev_tag) && ((uint8_t)dma_spt_list[spt_id].ctrl_idx == dev_info->idx)) {
			break;
		}
	}

	if (dma_spt_list[spt_id].dev_tag == 0xFFFFU) {
		return CSI_ERROR;
	}

	for (ch_info_id = 0U;; ch_info_id++) {
		if (dma_spt_list[spt_id].ch_list[ch_info_id].ctrl_idx == 0xFF) {
			return CSI_ERROR;
		}

		for (ctrl_id = 0U; ctrl_id < cvi_dma_get_inited_num(); ctrl_id++) {
			irq_flags = cvi_get_alloc_status_irqsave(ctrl_id, &alloc_status);

			if ((ctrl_id == (uint8_t)dma_spt_list[spt_id].ch_list[ch_info_id].ctrl_idx) &&
				!(alloc_status & ((uint32_t)1 << dma_spt_list[spt_id].ch_list[ch_info_id].ch_idx))) {
				(void)cvi_dma_lock_ch(ctrl_id, dma_spt_list[spt_id].ch_list[ch_info_id].ch_idx);
				dma_ch_info->ch_idx = dma_spt_list[spt_id].ch_list[ch_info_id].ch_idx;
				dma_ch_info->ctrl_idx = dma_spt_list[spt_id].ch_list[ch_info_id].ctrl_idx;
				/* find the channel */
				is_found = 1;
				break;
			}

			cvi_release_alloc_status_irqrestore(irq_flags);
		}

		if (is_found) {
			break;
		}
	}

	if(!is_found)
		return CSI_ERROR;

	return 0;
}

csi_error_t csi_dma_ch_alloc(csi_dma_ch_t *dma_ch, int8_t ch_id, int8_t ctrl_id)
{
	CSI_PARAM_CHK(dma_ch, CSI_ERROR);
	csi_error_t ret = CSI_ERROR;

	cvi_dma_ch_desc_t ch_info = {-1, -1};
	csi_dev_t *dev_info;
	dev_info = (csi_dev_t *)(dma_ch->parent);
	uint32_t irq_flags = csi_irq_save();

	ret = CSI_ERROR;
	if(ctrl_id >= 0 && ch_id >= 0){
		ch_info.ctrl_idx = ctrl_id;
		ch_info.ch_idx = ch_id;
		ret = cvi_dma_lock_ch(ch_info.ctrl_idx, ch_info.ch_idx);
	}
	if(ret){
		if(dev_info){
			ret = cvi_get_ch_info_from_dev(dev_info, &ch_info);
			if(ret){
				dma_err("get_ch_info_from_dev failed\r\n");
				goto err;
			}
		} else {
			ret = cvi_get_ch_info_random(&ch_info);
			if(ret){
				dma_err("cvi_get_ch_info_random failed\r\n");
				goto err;
			}
		}
	}

	_cvi_dma_ch_alloc(ch_info.ctrl_idx, ch_info.ch_idx);

	dma_ch->ch_id = ch_info.ch_idx;
	dma_ch->ctrl_id = ch_info.ctrl_idx;

	csi_irq_restore(irq_flags);

	dma_ch->etb_ch_id = -1;

	return 0;

err:
	csi_irq_restore(irq_flags);

	return ret;
}

void csi_dma_ch_stop(csi_dma_ch_t *dma_ch)
{
	CSI_PARAM_CHK_NORETVAL(dma_ch);

	cvi_dma_ch_stop(dma_ch->ctrl_id, dma_ch->ch_id);
}

void csi_dma_ch_free(csi_dma_ch_t *dma_ch)
{
	CSI_PARAM_CHK_NORETVAL(dma_ch);

	cvi_dma_ch_free(dma_ch->ctrl_id, dma_ch->ch_id);
}

void cvi_dma_ch_callback_default(uint8_t ch_id, void *args)
{
	csi_dma_ch_t *dma_ch = args;

	if (dma_ch->callback) {
		dma_ch->callback(dma_ch, DMA_EVENT_TRANSFER_DONE, dma_ch->arg);
	}
}

csi_error_t csi_dma_ch_attach_callback(csi_dma_ch_t *dma_ch, void *callback, void *arg)
{
	csi_error_t ret = CSI_OK;
	CSI_PARAM_CHK(dma_ch, CSI_ERROR);
	dma_ch->callback = callback;
	dma_ch->arg = arg;

	cvi_dma_ch_attach_callback(dma_ch->ctrl_id, dma_ch->ch_id, cvi_dma_ch_callback_default, dma_ch);

	return ret;
}

void csi_dma_ch_detach_callback(csi_dma_ch_t *dma_ch)
{
	CSI_PARAM_CHK_NORETVAL(dma_ch);
	dma_ch->callback = NULL;
	dma_ch->arg = NULL;

	cvi_dma_ch_detach_callback(dma_ch->ctrl_id, dma_ch->ch_id);
}

csi_error_t set_dw_config(struct dw_dma_cfg *dw_cfg, csi_dma_ch_config_t *config)
{
	if(config->src_reload_en || config->dst_reload_en){
		dma_err("src/dst reload_en not supported\r\n");
		return CSI_UNSUPPORTED;
	}

	if(config->half_int_en){
		dma_err("half_int_en not supported\r\n");
		return CSI_UNSUPPORTED;
	}

	if(config->src_inc == DMA_ADDR_DEC || config->dst_inc == DMA_ADDR_DEC){
		dma_err("DMA_ADDR_DEC not supported\r\n");
		return CSI_UNSUPPORTED;
	}

	dw_cfg->dst_inc = (cvi_dma_addr_inc_t)config->dst_inc;
	dw_cfg->dst_tw = (cvi_dma_data_width_t)config->dst_tw;
	dw_cfg->group_len = config->group_len;
	dw_cfg->handshake = config->handshake;
	dw_cfg->src_inc = (cvi_dma_addr_inc_t)config->src_inc;
	dw_cfg->src_tw = (cvi_dma_data_width_t)config->src_tw;
	dw_cfg->trans_dir = (cvi_dma_trans_dir_t)config->trans_dir;

	return CSI_OK;
}

csi_error_t csi_dma_ch_config(csi_dma_ch_t *dma_ch, csi_dma_ch_config_t *config)
{
	CSI_PARAM_CHK(dma_ch, CSI_ERROR);
	CSI_PARAM_CHK(config, CSI_OK);
	csi_error_t ret = CSI_OK;
	struct dw_dma_cfg dw_cfg;

	memset(&dw_cfg, 0, sizeof(dw_cfg));

	ret = set_dw_config(&dw_cfg, config);
	if(ret)
		return ret;

	cvi_dma_ch_config(dma_ch->ctrl_id, dma_ch->ch_id, &dw_cfg);

	return ret;
}

void csi_dma_ch_start(csi_dma_ch_t *dma_ch, void *srcaddr, void *dstaddr, uint32_t length)
{
	CSI_PARAM_CHK_NORETVAL(dma_ch);

	dma_dbg("csi_dma_ch_start: ctrl_id=%d, ch_id=%d, srcaddr=%p, dstaddr=%p, length=%u\r\n",
			dma_ch->ctrl_id, dma_ch->ch_id, srcaddr, dstaddr, length);

	cvi_dma_ch_start(dma_ch->ctrl_id, dma_ch->ch_id, srcaddr, dstaddr, length);
}
