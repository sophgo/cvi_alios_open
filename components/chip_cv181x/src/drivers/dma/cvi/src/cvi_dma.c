/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <stdint.h>
#include <drv/cvi_irq.h>
#include <cvi_dma.h>
#include <errno.h>

dw_dma_t *dma_array[1] = {0};

static uint32_t g_inited_ctrl_num = 0U;

static uint8_t convert_msize(int items)
{
	uint8_t ret;

	if (items > 1)
		ret = __fls(items) - 2;
	else
		ret = 0;

	return ret;
}

static uint32_t get_power(uint32_t base, uint32_t power)
{
	uint32_t ret, i;
	ret =  1U;

	for (i = 0U; i < power; i++) {
		ret *= base;
	}

	return ret;
}

static uint64_t dwc_prepare_default_ctl(struct dw_dma_channel *dwc)
{
	uint64_t ctl = 0;
	struct dw_dma_cfg *cfg = &dwc->cfg;
	int group_len = cfg->group_len;

	ctl |= DWC_CTL_SMS(cfg->master);
	ctl |= DWC_CTL_DMS(cfg->master);
	switch(cfg->trans_dir){
	case CVI_DMA_MEM2PERH:
		group_len /= get_power(2U, (uint32_t)(cfg->dst_tw));
		ctl |= DWC_CTL_SRC_MSIZE(convert_msize(group_len));
		ctl |= DWC_CTL_DST_MSIZE(convert_msize(group_len));
		break;
	case CVI_DMA_PERH2MEM:
		group_len /= get_power(2U, (uint32_t)cfg->src_tw);
		ctl |= DWC_CTL_SRC_MSIZE(convert_msize(group_len));
		ctl |= DWC_CTL_DST_MSIZE(convert_msize(group_len));
		break;
	case CVI_DMA_MEM2MEM:
		ctl |= DWC_CTL_SRC_MSIZE(DW_DMA_MSIZE_32);
		ctl |= DWC_CTL_DST_MSIZE(DW_DMA_MSIZE_32);
		break;
	}

	return ctl;
}

static void *dw_desc_alloc(void)
{
	struct dw_desc *desc;
	uint64_t tmp;

	//64bytes align
	tmp = (uint64_t)malloc(sizeof(struct dw_desc) + (1 << 6) - 1);
	if(!tmp){
		return NULL;
	}
	memset((void *)tmp, 0, sizeof(struct dw_desc) + (1 << 6) - 1);

	dma_dbg("desc_alloc tmp=%lx, size=%lu\r\n", tmp, sizeof(struct dw_desc) + (1 << 6) - 1);
	desc = (void *)((tmp+ (1 << 6) - 1) & ~((1 << 6) - 1));
	desc->raw_addr = (void *)tmp;
	dma_dbg("desc_alloc desc->raw_addr=%p\r\n", desc->raw_addr);

	return desc;
}

static inline struct dw_desc * dw_desc_get()
{
	return dw_desc_alloc();
}

static void dw_clk_enable(struct dw_dma *dw)
{
	size_t irq_state;

	irq_state = csi_irq_save();
	dw->clk_enable_count++;
	if(dw->clk_enable_count == 1){
		hal_dma_dwc_clk_set(1);
		hal_dma_on(dw);
	}
	csi_irq_restore(irq_state);
}

static void dw_clk_disable(struct dw_dma *dw)
{
	size_t irq_state;

	irq_state = csi_irq_save();
	dw->clk_enable_count--;
	if(!dw->clk_enable_count){
		hal_dma_off(dw);
		hal_dma_dwc_clk_set(0);
	}else if(dw->clk_enable_count < 0){
		dma_err("[BUG] dma clk_enable_count < 0\r\n");
		dma_err("[BUG] dma clk_enable_count=%d\r\n", dw->clk_enable_count);
		dw->clk_enable_count = 0;
	}
	csi_irq_restore(irq_state);
}

static void release_descriptor(struct dw_dma * dw, dlist_t *list)
{
	size_t irq_state;
	dlist_t tmp_list;
	dlist_t *plist;
	struct dw_desc *tmp_desc;
	struct dw_desc *node;

	dlist_init(&tmp_list);

	irq_state = csi_irq_save();
	if(dlist_empty(list)){
		csi_irq_restore(irq_state);
		return;
	}
	cvi_list_splice_init(list, &tmp_list);
	csi_irq_restore(irq_state);

	dlist_for_each_entry_safe(&tmp_list, plist, node, struct dw_desc, list){
		dw_clk_disable(dw);
		dlist_del(&node->list);
		while(node){
			tmp_desc = node->next;
			free(node->raw_addr);
			node = tmp_desc;
		}
	}
}

#if 0
static void dump_lli_content(uint64_t * llp_first, int count)
{
	int i;
	uint64_t * llp = llp_first;
	for(i = 0;i < count;i++,llp += 4) {
		printf("lli			: %d, %p\r\n", i, llp);
		printf("sar			: 0x%lx\r\n", *llp++);
		printf("dar			: 0x%lx\r\n", *llp++);
		printf("block ts	: 0x%lx\r\n", *llp++);
		printf("llp			: 0x%lx\r\n", *llp++);
		printf("ctl			: 0x%lx\r\n", *llp++);
		printf("stat		: 0x%lx\r\n", *llp++);
		printf("llp_stat	: 0x%lx\r\n", *llp);
		printf("\r\n");
	}
}
#endif

static void dwc_fill_register(dw_dma_channel_t *dwc)
{
	uint64_t cfg;
	uint64_t int_status_reg;
	struct dw_desc *desc = get_first_desc(&dwc->active_list);

	cfg = DWC_CFG_SRC_OSR_LMT((uint64_t)DW_DMA_MAX_NR_REQUESTS - 1)
			| DWC_CFG_DST_OSR_LMT((uint64_t)DW_DMA_MAX_NR_REQUESTS - 1)
			| DWC_CFG_CH_PRIOR((uint64_t)(DW_DMA_MAX_NR_CHANNELS - dwc->ch_id -1))
			| DWC_CFG_DST_MULTBLK_TYPE(LINK_LIST)
			| DWC_CFG_SRC_MULTBLK_TYPE(LINK_LIST);

	switch(dwc->cfg.trans_dir){
	case CVI_DMA_MEM2PERH:
		cfg |= DWC_CFG_DST_PER((uint64_t)dwc->cfg.handshake);
		cfg |= dwc->cfg.hs_polarity ? 	\
			   DWC_CFG_DST_HWHS_POL_L : DWC_CFG_DST_HWHS_POL_H;
		cfg |= DWC_CFG_TT_FC(DW_DMA_FC_D_M2P);
		cfg |= DWC_CFG_HS_SEL_DST_HW;
		break;

	case CVI_DMA_PERH2MEM:
		cfg |= DWC_CFG_SRC_PER((uint64_t)dwc->cfg.handshake);
		cfg |= dwc->cfg.hs_polarity ? 	\
			   DWC_CFG_SRC_HWHS_POL_L : DWC_CFG_SRC_HWHS_POL_H;
		cfg |= DWC_CFG_TT_FC((uint64_t)DW_DMA_FC_D_P2M);
		cfg |= DWC_CFG_HS_SEL_SRC_HW;
		break;

	case CVI_DMA_MEM2MEM:
		cfg |= DWC_CFG_TT_FC((uint64_t)DW_DMA_FC_D_M2M);
		break;
	}

	dma_dbg("cfg write, CFG=%lx\r\n", cfg);
	hal_dma_write_cfg(dwc, cfg);

	int_status_reg =  DWC_CH_INTSTA_DMA_TFR_DONE;

	dma_dbg("enable irq, int_status_reg=%lx\r\n", int_status_reg);
	hal_dma_enable_irq(dwc, int_status_reg);
	hal_dma_write_llp(dwc, (uint64_t)&desc->lli);
}

static void dwc_do_first_queue(dw_dma_channel_t *dwc)
{
	struct dw_dma *dw_dma = dma_array[dwc->ctrl_id];
	struct dw_dma_cfg *cfg = &dwc->cfg;
	uint32_t irq_state;

	irq_state = csi_irq_save();
	if(dlist_empty(&dwc->queue_list)){
		dma_dbg("queue_list empty\r\n");
		csi_irq_restore(irq_state);
		return;
	}

	dlist_move(&dwc->queue_list, &dwc->active_list);
	csi_irq_restore(irq_state);

	dwc_fill_register(dwc);

	switch(cfg->trans_dir){
	case CVI_DMA_MEM2PERH:
		csi_dcache_clean_range(cfg->src_addr, cfg->length);
		break;
	case CVI_DMA_PERH2MEM:
		csi_dcache_clean_invalid_range(cfg->dst_addr, cfg->length);
		break;
	case CVI_DMA_MEM2MEM:
		csi_dcache_clean_range(cfg->src_addr, cfg->length);
		csi_dcache_clean_invalid_range(cfg->dst_addr, cfg->length);
		break;
	}

	hal_dma_ch_on(dw_dma, dwc->ch_mask);

	dma_dbg("dwc_do_first_queue finish\r\n");
}

void dwc_complete_all(dw_dma_channel_t *dwc)
{
	struct dw_dma *dw = dma_array[dwc->ctrl_id];
	dlist_t dlist;
	size_t irq_state;

	dlist_init(&dlist);
	irq_state = csi_irq_save();
	if(!dlist_empty(&dwc->active_list))
		cvi_list_splice_init(&dwc->active_list, &dlist);
	csi_irq_restore(irq_state);

	dwc_do_first_queue(dwc);

	if (dwc->callback){
		dma_dbg("entering callback\r\n");
		dwc->callback(dwc->ch_id, dwc->args);
		dma_dbg("callback finish\r\n");
	}
	release_descriptor(dw, &dlist);
}

void dw_dma_irq_handler(uint32_t irqn, void *arg)
{
	dw_dma_t *dma = arg;
	uint64_t status;
	uint64_t dwc_status;
	dw_dma_channel_t *dwc;

	dma_dbg("enter dw_dma_irq_handler\r\n");

	status = hal_dma_get_intstatus(dma);
	/* Check if we have any interrupt from the DMAC */
	dma_dbg("intstatus = %lx\r\n", status);
	if (!status)
		return ;

	hal_dma_clear_comm_intstatus(dma); /* clear all common interrupts */
	for (int i = 0; i < dma->nr_channels; i++) {
		dwc = &dma->chans[i];
		dwc_status = hal_dma_dwc_read_clear_intstatus(dwc);
		if(dwc_status & DWC_CH_INTSTA_DMA_TFR_DONE){
			dwc_complete_all(dwc);
		}
	}
}

void dw_init(dw_dma_t *dw_dma)
{
	dw_dma->nr_masters = DW_DMA_MAX_NR_MASTERS;
	dw_dma->nr_channels = DW_DMA_MAX_NR_CHANNELS;
	dw_dma->block_ts = DW_DWC_MAX_BLOCK_TS;

	for(int i = 0; i < dw_dma->nr_masters; i++){
		dw_dma->data_width[i] = DW_DMA_MAX_DATA_WIDTH;
	}
}

static uint32_t dw_dma_bytes2block(struct dw_dma *dw,
				  size_t bytes, unsigned int width, size_t *len)
{
	uint32_t block;

	if ((bytes >> width) > dw->block_ts) {
		block = dw->block_ts;
		*len = dw->block_ts << width;
	} else {
		block = bytes >> width;
		*len = bytes;
	}

	return block;
}

static inline uint64_t addr_fix_or_inc(struct dw_dma_cfg *cfg)
{
	uint64_t ctl = 0;

	switch(cfg->src_inc) {
	case CVI_DMA_ADDR_INC:
		ctl |= DWC_CTL_SRC_INC;
		break;
	case CVI_DMA_ADDR_CONSTANT:
		ctl |= DWC_CTL_SRC_FIX;
		break;
	default:break;
	}

	switch(cfg->dst_inc) {
	case CVI_DMA_ADDR_INC:
		ctl |= DWC_CTL_DST_INC;
		break;
	case CVI_DMA_ADDR_CONSTANT:
		ctl |= DWC_CTL_DST_FIX;
		break;
	default:break;
	}

	return ctl;
}

void prep_transfer(dw_dma_channel_t *dwc, void *srcaddr, void *dstaddr, uint32_t length)
{
	dw_dma_t *dw_dma = dma_array[dwc->ctrl_id];
	struct dw_dma_cfg *cfg = &dwc->cfg;
	unsigned int data_width;
	size_t dlen;
	struct dw_desc *desc;
	uint64_t ctl;
	uint64_t block_ts_shift = 0;
	uint32_t block;
	uint64_t src_width = 0;
	uint64_t dst_width = 0;
	uint64_t sar = (uint64_t)srcaddr;
	uint64_t dar = (uint64_t)dstaddr;
	size_t irq_state;

	dma_dbg("enter prep_transfer\r\n");

	cfg->src_addr = srcaddr;
	cfg->dst_addr = dstaddr;
	cfg->length = length;

	ctl = (dwc_prepare_default_ctl(dwc)	\
			| addr_fix_or_inc(cfg)	\
			| DWC_CTL_DST_STA_EN	\
			| DWC_CTL_SRC_STA_EN	\
			| DWC_CTL_SHADOWREG_OR_LLI_VALID);

	data_width = dw_dma->data_width[dwc->cfg.master];

	dma_dbg("channel data width=%d\r\n", data_width);

	switch(cfg->trans_dir){
	case CVI_DMA_MEM2PERH:
		dst_width = cfg->dst_tw;
		src_width = __ffs((uint64_t)data_width | length | sar);
		break;

	case CVI_DMA_PERH2MEM:
		src_width = cfg->src_tw;
		dst_width = __ffs((uint64_t)data_width | length | dar);
		break;

	case CVI_DMA_MEM2MEM:
		src_width = __ffs((uint64_t)data_width | length | sar);
		dst_width = __ffs((uint64_t)data_width | length | dar);;
		break;
	}

	dma_dbg("src_width=%lx, dst_width=%lx\r\n", src_width, dst_width);
	dma_dbg("length=%d\r\n", length);

	ctl |= DWC_CTL_SRC_WIDTH(src_width);
	ctl |= DWC_CTL_DST_WIDTH(dst_width);

	block_ts_shift = src_width;

	struct dw_desc *first = NULL;
	struct dw_desc *prev = NULL;

	while(length){
		if(is_slave(cfg))
			block = dw_dma_bytes2block(dw_dma, length, block_ts_shift, &dlen);
		else{
			block = length >> block_ts_shift;
			dlen = length;
		}
		dma_dbg("remain bytes=%d\r\n", length);
		dma_dbg("dlen=%ld\r\n", dlen);

		desc = dw_desc_get();
		if(!desc){
			dma_err("dw_desc_get failed\r\n");
			goto err;
		}

		dma_dbg("desc_get addr=%p\r\n", desc);

		lli_write(desc, sar, sar);
		lli_write(desc, dar, dar);

		if(cfg->src_inc == CVI_DMA_ADDR_INC)
			sar += dlen;
		if(cfg->dst_inc == CVI_DMA_ADDR_INC)
			dar += dlen;

		length -=dlen;
		if(!length)
			ctl |= DWC_CTL_SHADOWREG_OR_LLI_LAST;

		lli_write(desc, ctl, ctl);
		dma_dbg("start write block_ts:block=%d\r\n", block);
		lli_write(desc, block_ts, block - 1);

		if(!first){
			first = desc;
		}else{
			lli_write(prev, llp, (uint64_t)desc);
			csi_dcache_clean_range((uint64_t *)&prev->lli, sizeof(struct dw_lli));
			prev->next = desc;
		}
		prev = desc;
		dma_dbg("desc->lli.llp=%lx\r\n", desc->lli.llp);
		dma_dbg("desc->next=%p\r\n", desc->next);
	}

	csi_dcache_clean_range((uint64_t *)&prev->lli, sizeof(struct dw_lli));

	irq_state = csi_irq_save();
	dlist_add(&first->list, &dwc->queue_list);
	csi_irq_restore(irq_state);

	dma_dbg("prep_transfer finish\r\n");

	return;

err:
	dma_dbg("err\r\n");
	if(first){
		struct dw_desc *tmp_desc;
		dlist_del(&first->list);
		while(first){
			tmp_desc = first->next;
			free(first->raw_addr);
			first = tmp_desc;
		}
	}

	return;
}

int cvi_set_ch_hw_param(dw_dma_channel_t *dwc, uint8_t master, uint8_t hs_polarity)
{
	if(!dwc){
		dma_err("dwc is NULL\r\n");
		return -EINVAL;
	}

	struct dw_dma *dw = dma_array[dwc->ctrl_id];

	if(master >= dw->nr_masters || hs_polarity > 1){
		dma_err("master or hs_polarity error\r\n");
		return  -EINVAL;
	}

	dwc->cfg.master = master;
	dwc->cfg.hs_polarity = hs_polarity;

	return 0;
}

int cvi_dma_init(dw_dma_t *dma, int8_t ctrl_id, unsigned long reg_base, uint32_t irq_num)
{
	int irq_flags;

	/* current soc just have dma 0 */
	if (ctrl_id || !dma){
		dma_err("cvi_dma_init: dma=%p, ctrl_id: %d.\r\n", dma, ctrl_id);
		return -EINVAL;
	}

	irq_flags = csi_irq_save();
	if(dma_array[ctrl_id]){
		csi_irq_restore(irq_flags);
		return -EEXIST;
	}
	dma_array[ctrl_id] = dma;
	csi_irq_restore(irq_flags);

	dw_init(dma);

	dma_array[ctrl_id]->nr_channels = DW_DMA_MAX_NR_CHANNELS;

	dma->regs = (struct dw_dma_regs *)reg_base;
	dma->idx = 0;

	dma_dbg("dma->regs=%p\r\n", dma->regs);

	dw_clk_enable(dma);

	hal_dma_reset(dma);

	dma->irq_num = irq_num;

	dma_dbg("dma->irq_num=%d\r\n", dma->irq_num);

#ifdef __riscv
	hal_sdma_dma_int_mux_set_c906b();
	dma_dbg("sdma_dma_int_mux=0x%x\r\n", sdma_dma_int_mux_get());
#endif

	request_irq((uint32_t)dma->irq_num, &dw_dma_irq_handler, 0, "dma", dma);

	//init channel
	dma->alloc_status = 0U;

	/* init each channel */
	for (int i = 0; i < dma->nr_channels; i++) {
		dw_dma_channel_t *dwc = &dma->chans[i];
		dwc->regs = CH_REG_BASE(dma, i);
		dma_dbg("dwc->regs[%d]=%p\r\n", i, dwc->regs);
		dlist_init(&dwc->queue_list);
		dlist_init(&dwc->active_list);
		//clear int status
		hal_dma_dwc_clear_intstatus(dwc);

		//turn off all channel
		hal_dma_turn_off_chans(dma);
	}

	dw_clk_disable(dma);

	irq_flags = csi_irq_save();
	g_inited_ctrl_num++;
	csi_irq_restore(irq_flags);

	dma_hs_remap_init();

	return 0;
}

void cvi_dma_uninit(dw_dma_t *dma)
{
	int irq_flags;

	if(!dma)
		return;

	hal_dma_off(dma);
	hal_dma_dwc_clk_set(0);

	csi_irq_detach(dma->irq_num);
	csi_irq_disable(dma->irq_num);

	irq_flags = csi_irq_save();
	dma_array[dma->idx] = NULL;
	g_inited_ctrl_num--;
	csi_irq_restore(irq_flags);

	for (int i = 0; i < dma->nr_channels; i++) {
		struct dw_dma_channel *dwc = &dma->chans[i];
		release_descriptor(dma, &dwc->queue_list);
		release_descriptor(dma, &dwc->active_list);
	}
}

uint32_t cvi_get_alloc_status_irqsave(uint8_t ctrl_idx, uint32_t *alloc_status)
{
	uint32_t irq_flags;

	irq_flags = csi_irq_save();

	*alloc_status = dma_array[ctrl_idx]->alloc_status;

	return irq_flags;
}

void cvi_release_alloc_status_irqrestore(uint32_t irq_flags)
{
	csi_irq_restore(irq_flags);
}

int cvi_dma_lock_ch(uint8_t ctrl_idx, uint8_t ch_idx)
{
	uint32_t irq_flags;
	int ret;
	uint32_t *alloc_status = &dma_array[ctrl_idx]->alloc_status;

	irq_flags = csi_irq_save();

	if (*alloc_status & ((uint32_t)1 << ch_idx)){
		ret = -EEXIST;
	} else {
		*alloc_status |= ((uint32_t)1 << ch_idx);
		ret = 0;
	}

	csi_irq_restore(irq_flags);

	return ret;
}

uint32_t cvi_dma_get_inited_num()
{
	return g_inited_ctrl_num;
}

int cvi_get_ch_info_random(cvi_dma_ch_desc_t *ch_info)
{
	uint8_t ctrl_id, ch_id;
	uint32_t irq_flags;
	uint32_t alloc_status;
	int is_found = 0;
	int ret = 0;

	for (ctrl_id = 0U; ctrl_id < cvi_dma_get_inited_num(); ctrl_id++) {
		irq_flags = cvi_get_alloc_status_irqsave(ctrl_id, &alloc_status);

		for(ch_id = 0; ch_id < dma_array[ctrl_id]->nr_channels; ch_id++){
			if(!(alloc_status & ((uint32_t)1 << ch_id))){
				ret = cvi_dma_lock_ch(ctrl_id, ch_id);
				if( ret == CSI_OK)
				{
					ch_info->ctrl_idx = ctrl_id;
					ch_info->ch_idx = ch_id;
					is_found = 1;
					break;
				}
			}
		}

		cvi_release_alloc_status_irqrestore(irq_flags);
	}

	if(!is_found){
		return -ENOENT;
	}

	dma_dbg("channel found, ctrl_id=%u, ch_id=%u\r\n", ch_info->ctrl_idx, ch_info->ch_idx);

	return 0;
}

void _cvi_dma_ch_alloc(uint8_t ctrl_idx, uint8_t ch_idx)
{
	struct dw_dma *dma;
	struct dw_dma_channel *dwc;

	dma = dma_array[ctrl_idx];
	dwc = &dma->chans[ch_idx];
	memset(dwc, 0, sizeof(dwc->cfg));
	dwc->ch_mask = 1 << ch_idx;
	dwc->regs = CH_REG_BASE(dma, ch_idx);
	dwc->callback = dwc->args = NULL;
	dwc->ctrl_id = ctrl_idx;
	dwc->ch_id = ch_idx;
}

int cvi_dma_ch_alloc(uint8_t ctrl_idx, uint8_t ch_idx)
{
	uint32_t irq_flags;

	if((g_inited_ctrl_num <= ctrl_idx) || \
		(!dma_array[ctrl_idx]) || \
		(ctrl_idx < 0 || ch_idx < 0)){
		return -EINVAL;
	}

	irq_flags = csi_irq_save();

	if (dma_array[ctrl_idx]->alloc_status & (uint32_t)((uint32_t)1U << (uint32_t)ch_idx)) {
		csi_irq_restore(irq_flags);
		return -EEXIST;
	}
	dma_array[ctrl_idx]->alloc_status |= (uint32_t)((uint32_t)1U << (uint32_t)ch_idx);

	csi_irq_restore(irq_flags);

	_cvi_dma_ch_alloc(ctrl_idx, ch_idx);

	return 0;
}

void cvi_dma_ch_stop(int ctrl_idx, int ch_idx)
{
	dw_dma_t *dma = dma_array[ctrl_idx];
	struct dw_dma_channel *dwc = &dma->chans[ch_idx];

	hal_dma_ch_off(dma, dwc->ch_mask);

	release_descriptor(dma, &dwc->queue_list);
	release_descriptor(dma, &dwc->active_list);
}

void cvi_dma_ch_pause(int ctrl_idx, int ch_idx)
{
	uint32_t irq_flags;
	dw_dma_t *dma = dma_array[ctrl_idx];
	struct dw_dma_channel *dwc = &dma->chans[ch_idx];

	irq_flags = csi_irq_save();
	hal_dma_ch_pause(dma, dwc->ch_mask);

	csi_irq_restore(irq_flags);
}

void cvi_dma_ch_resume(int ctrl_idx, int ch_idx)
{
	uint32_t irq_flags;
	dw_dma_t *dma = dma_array[ctrl_idx];
	struct dw_dma_channel *dwc = &dma->chans[ch_idx];

	irq_flags = csi_irq_save();
	hal_dma_ch_resume(dma, dwc->ch_mask);
	
	csi_irq_restore(irq_flags);
}

void cvi_dma_ch_free(int ctrl_idx, int ch_idx)
{
	dw_dma_t *dma = dma_array[ctrl_idx];
	uint32_t temp_u32;
	uint32_t irq_state;

	temp_u32 = 1U << (uint8_t)(ch_idx);
	irq_state = csi_irq_save();
	if(!(dma_array[ctrl_idx]->alloc_status & temp_u32)){
		csi_irq_restore(irq_state);
		return;
	}
	csi_irq_restore(irq_state);

	cvi_dma_ch_stop(ctrl_idx, ch_idx);

	irq_state = csi_irq_save();

	if (dma->alloc_status & temp_u32)
		dma->alloc_status &= ~((uint32_t)1 << (uint32_t)ch_idx);

	csi_irq_restore(irq_state);
}

void cvi_dma_ch_attach_callback(int ctrl_idx, int ch_idx, void *callback, void *args)
{
	dw_dma_t *dma = dma_array[ctrl_idx];
	struct dw_dma_channel *dwc = &dma->chans[ch_idx];

	if(callback){
		dwc->callback = callback;
		dwc->args =  args;
	}
}

void cvi_dma_ch_detach_callback(int ctrl_idx, int ch_idx)
{
	dw_dma_t *dma = dma_array[ctrl_idx];
	struct dw_dma_channel *dwc = &dma->chans[ch_idx];

	dwc->callback = NULL;
	dwc->callback = NULL;
}

int cvi_dma_ch_config(int ctrl_idx, int ch_idx, struct dw_dma_cfg *dw_cfg)
{
	struct dw_dma *dma;
	dma = dma_array[ctrl_idx];

	if(!dw_cfg)
		return -EINVAL;

	dw_dma_channel_t *dwc = &dma->chans[ch_idx];

	memcpy(&dwc->cfg, dw_cfg, sizeof(struct dw_dma_cfg));

	return 0;
}

void cvi_dma_ch_start(int ctrl_idx, int ch_idx, void *srcaddr, void *dstaddr, uint32_t length)
{
	int irq_flags;
	struct dw_dma *dma = dma_array[ctrl_idx];
	dw_dma_channel_t *dwc = &dma->chans[ch_idx];

	dw_clk_enable(dma);
	dma_dbg("enter csi_dma_ch_start\r\n");
	dma_dbg("srcaddr=%p, dstaddr=%p, length=%x\r\n", srcaddr, dstaddr, length);

	prep_transfer(dwc, srcaddr, dstaddr, length);

	irq_flags = csi_irq_save();
	if(dlist_empty(&dwc->active_list)){
		csi_irq_restore(irq_flags);
		dwc_do_first_queue(dwc);
	}else {
		csi_irq_restore(irq_flags);
	}
}

/* ---------------------- show debug info -------------------------- */
#define DW_CHECK_UNINITED(ptr) {if(!(ptr) && dma_err("dma uninited\r\n")) return;}

void dma_show_info(void)
{
	dw_dma_t *dw = dma_array[0];
	DW_CHECK_UNINITED(dw);

	dw_clk_enable(dw);
	hal_print_dma_reg(dw);
	dw_clk_disable(dw);
}

void dma_show_ch_info_all()
{
	struct dw_dma *dw = dma_array[0];
	int i;

	DW_CHECK_UNINITED(dw);

	dw_clk_enable(dw);

	for (i = 0; i < dw->nr_channels; i++) {
		hal_print_ch_info(dw, i);
	}

	dw_clk_disable(dw);
}

void dma_show_ch_info(int ch_id)
{
	struct dw_dma *dw = dma_array[0];

	DW_CHECK_UNINITED(dw);

	dw_clk_enable(dw);

	hal_print_ch_info(dw, ch_id);

	dw_clk_disable(dw);
}
