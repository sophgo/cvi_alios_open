/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file    avfs.c
 * @brief
 * @version
 * @date     01. Dec 2020
 ******************************************************************************/

#include <drv/cvi_irq.h>
#include <drv/tick.h>
#include "drv/iic.h"
#include "drv/common.h"
#include <drv/avfs.h>
#include "wj_avfs_ll.h"

#define AVFS_COMMON_ERR_MSK (WJ_AVFS_INTR_PMIC_AHB_ERR_MASK | \
                            WJ_AVFS_INTR_FREQ_NOT_MATCH_MASK| \
							WJ_AVFS_INTR_VOL_ERR_MASK)

static csi_error_t csi_avfs_fill_pmic_cmd(csi_avfs_t *avfs,uint32_t cmd_grp, uint32_t cmd_num, cmd_param_t *cmd_param);
 
extern csi_error_t csi_iic_dev_addr(csi_iic_t *iic, uint32_t dev_addr);

static void wj_avfs_irqhandler(void *args)
{
	csi_avfs_t *avfs = (csi_avfs_t*)args;
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	uint32_t    temperature_int_status, avfs_int_status;
	
	temperature_int_status = wj_avfs_intr_tmp_get_status(avfs_base);
	avfs_int_status        = wj_avfs_intr_get_status(avfs_base);

	if(avfs->call_back == NULL) {
		wj_avfs_intr_clear(avfs_base,avfs_int_status);
		wj_avfs_intr_tmp_clear(avfs_base,temperature_int_status);
		return;
	}

	/* check temperature int status */
	if(temperature_int_status) {
		if(temperature_int_status & WJ_AVFS_TMP_INTR_TMP_LOW_WARN_MASK) {
			avfs->call_back(avfs,AVFS_EVENT_TMP_LOW,avfs->arg);
		}
		
		if(temperature_int_status & WJ_AVFS_TMP_INTR_TMP_HIGH_WARN_MASK) {
			avfs->call_back(avfs,AVFS_EVENT_TMP_HIGH,avfs->arg);
		}
		wj_avfs_intr_tmp_clear(avfs_base,temperature_int_status);
	}

	if(avfs_int_status) {
		if(avfs_int_status & WJ_AVFS_INTR_PS_ERR_MASK) {
			avfs->call_back(avfs,AVFS_EVENT_ERROR_PS_CMD,avfs->arg);
		}

		if(avfs_int_status & WJ_AVFS_INTR_VS_ERR_MASK) {
			avfs->call_back(avfs,AVFS_EVENT_ERROR_VS_CMD,avfs->arg);
		}

		if(avfs_int_status & WJ_AVFS_INTR_TS_ERR_MASK) {
			avfs->call_back(avfs,AVFS_EVENT_ERROR_TS_CMD,avfs->arg);
		}

		if(avfs_int_status & WJ_AVFS_INTR_PS_ITERATE_MAX_EXCEED_MASK) {
			avfs->call_back(avfs,AVFS_EVENT_ERROR_ITER_EXCEED,avfs->arg);
		}

		if(avfs_int_status & WJ_AVFS_INTR_PS_BOUNCE_LOOP_MASK) {
			avfs->call_back(avfs,AVFS_EVENT_ERROR_ITER_BACK,avfs->arg);
		}

		if(avfs_int_status & WJ_AVFS_INTR_HW_DONE_MASK) {
			avfs->call_back(avfs,AVFS_EVENT_AVFS_DONE,avfs->arg);
		}

		if(avfs_int_status & AVFS_COMMON_ERR_MSK) {
			avfs->call_back(avfs,AVFS_EVENT_ERROR_COMMON,avfs->arg);
		}

		wj_avfs_intr_clear(avfs_base,avfs_int_status);
	}

	return;
}

csi_error_t csi_avfs_if_config_spilite(csi_avfs_t *avfs, uint32_t hz, uint32_t rx_sample_delay,uint8_t rd_pol)
{
	int32_t div;
	CSI_PARAM_CHK(avfs,CSI_ERROR);

	if(hz > soc_get_avfs_freq() / 2) {
		return CSI_ERROR;
	}

	if(avfs->if_type != PMIC_IF_SPI_LITE) {
		return CSI_ERROR;
	}
	
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	div = soc_get_avfs_freq()/(hz*2) -1;
	if(div >= 0) {
		wj_avfs_vol_spi_set_baud_div(avfs_base,div);
	}

	avfs->spilite_rd_pol = rd_pol;
	wj_avfs_vol_spi_set_rx_sample_dly(avfs_base,rx_sample_delay);
	return CSI_OK; 
}

csi_error_t csi_avfs_if_config_iic(csi_avfs_t *avfs, uint32_t i2c_dev_addr,csi_iic_addr_mode_t addr_mode,csi_iic_speed_t speed)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	csi_error_t ret;
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	if(avfs->if_type != PMIC_IF_IIC) {
		return CSI_ERROR;
	}
	avfs->iic.dev_addr = i2c_dev_addr;
	
	ret = csi_iic_mode((csi_iic_t*)(&avfs->iic),IIC_MODE_MASTER);
	if(ret) {
		return ret;
	}
	
	/* init i2c base addr */
	wj_avfs_vol_set_i2c_base_addr(avfs_base,avfs->iic.i2c_base);
	ret = csi_iic_addr_mode((csi_iic_t*)(&avfs->iic),addr_mode);
	if(ret) {
		return ret;
	}
	ret = csi_iic_speed((csi_iic_t*)(&avfs->iic),speed);
	if(ret) {
		return ret;
	}

	/* init iic slave addr */
	ret = csi_iic_dev_addr((csi_iic_t*)(&avfs->iic),i2c_dev_addr);
	if(ret) {
		return ret;
	}
	return CSI_OK;
}

static csi_error_t spi_write_reg(csi_avfs_t *avfs,uint32_t offset, uint32_t val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);

	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	wj_avfs_vol_spi_write_reg(avfs_base,offset,val,avfs->spilite_rd_pol);
	return CSI_OK;
}

static csi_error_t spi_read_reg(csi_avfs_t *avfs,uint32_t offset, uint32_t *val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);

	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	*val = wj_avfs_vol_spi_read_reg(avfs_base,offset,avfs->spilite_rd_pol);
	return CSI_OK;
}

static csi_error_t iic_write_reg(csi_avfs_t *avfs,uint32_t offset, uint32_t val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	int32_t num;
	uint8_t temp[2] = {0};
	temp[0] = offset;
	temp[1] = (uint8_t)val;

	num = csi_iic_master_send((csi_iic_t*)(&avfs->iic),avfs->iic.dev_addr,temp,sizeof(temp),10);
	if(num != sizeof(temp)) {
		return CSI_ERROR;
	}

	return CSI_OK;
}

static csi_error_t iic_read_reg(csi_avfs_t *avfs,uint32_t offset, uint32_t *val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	int32_t num;
	uint8_t temp = (uint8_t)offset;

	num = csi_iic_master_send((csi_iic_t*)(&avfs->iic),avfs->iic.dev_addr,&temp,1,10);
	if(num != 1) {
		return CSI_ERROR;
	}

	num = csi_iic_master_receive((csi_iic_t*)(&avfs->iic),avfs->iic.dev_addr,&temp,1,10);
	if(num != 1) {
		return CSI_ERROR;
	} 

	*val = temp;
	return CSI_OK;
}


csi_error_t csi_avfs_spilite_init(csi_avfs_t *avfs)
{
	csi_error_t ret;
	wj_avfs_ctrl_regs_t *avfs_base;
	CSI_PARAM_CHK(avfs,CSI_ERROR);

	/* get dev info */
	ret = target_get(DEV_WJ_AVFS_TAG,0,&avfs->dev);
	if(ret){
		return ret;
	}

	/* init handle */
	avfs_base            = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	avfs->pmic_read_reg  = spi_read_reg;
	avfs->pmic_write_reg = spi_write_reg;
	avfs->call_back      = NULL;
	avfs->arg            = NULL;
	avfs->if_type        = PMIC_IF_SPI_LITE;
	avfs->priv           = NULL;
	avfs->pmic_page      = 0;

	/* disable avfs engine */
	wj_avfs_misc_hw_auto_disable(avfs_base);
	/* set pmic interface as spi-lite */
	wj_avfs_vol_set_if_dev(avfs_base,AVFS_PMIC_IF_SPI);
	
	return CSI_OK;
}

csi_error_t csi_avfs_write_pmic_reg(csi_avfs_t *avfs,uint32_t reg_offset, uint32_t reg_val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	return avfs->pmic_write_reg(avfs,reg_offset,reg_val);	
}

csi_error_t csi_avfs_read_pmic_reg(csi_avfs_t *avfs,uint32_t reg_offset, uint32_t *reg_val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	return avfs->pmic_read_reg(avfs,reg_offset,reg_val);	
}

csi_error_t csi_avfs_i2c_init(csi_avfs_t *avfs, uint32_t i2c_idx)
{
	csi_error_t ret;
	wj_avfs_ctrl_regs_t *avfs_base;
	CSI_PARAM_CHK(avfs,CSI_ERROR);

	/* get dev info */
	ret = target_get(DEV_WJ_AVFS_TAG,0,&avfs->dev);
	if(ret){
		return ret;
	}

	/* init handle */
	avfs_base            = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	avfs->pmic_read_reg  = iic_read_reg;
	avfs->pmic_write_reg = iic_write_reg;
	avfs->call_back      = NULL;
	avfs->arg            = NULL;
	avfs->if_type        = PMIC_IF_IIC;
	avfs->priv           = NULL;

	/* disable avfs engine */
	wj_avfs_misc_hw_auto_disable(avfs_base);
	/* set pmic interface as spi-lite */
	wj_avfs_vol_set_if_dev(avfs_base,AVFS_PMIC_IF_I2C);

	ret = csi_iic_init(&(avfs->iic.handle),i2c_idx);
	if(ret) {
		return ret;
	}
	avfs->iic.i2c_base = avfs->iic.handle.dev.reg_base;
	return CSI_OK;
}
static void avfs_disable_all_irq(wj_avfs_ctrl_regs_t *avfs_base)
{
	wj_avfs_intr_clear(avfs_base,WJ_AVFS_INTR_MASK);
	wj_avfs_intr_disable(avfs_base,WJ_AVFS_INTR_MASK);
	wj_avfs_intr_tmp_clear(avfs_base,WJ_AVFS_TMP_INTR_MASK);
	wj_avfs_intr_tmp_disable(avfs_base,WJ_AVFS_TMP_INTR_MASK);
}

void csi_avfs_uninit(csi_avfs_t *avfs)
{
	wj_avfs_ctrl_regs_t *avfs_base;
	CSI_PARAM_CHK_NORETVAL(avfs);

	if(avfs->if_type == AVFS_PMIC_IF_I2C) {
		csi_iic_uninit(&(avfs->iic.handle));
	}

	avfs_base            = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	avfs_disable_all_irq(avfs_base);
	csi_irq_disable(avfs->dev.irq_num);
	avfs->pmic_read_reg  = NULL;
	avfs->pmic_write_reg = NULL;
	avfs->call_back      = NULL;
	avfs->arg            = NULL;
	avfs->if_type           = PMIC_IF_MAX;
	avfs->priv           = NULL;

}

csi_error_t csi_avfs_attach_callback(csi_avfs_t *avfs, void *callback, void *arg)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	CSI_PARAM_CHK(callback,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	avfs->arg = arg;
	avfs->call_back = callback;
	uint32_t msk;

	wj_avfs_intr_clear(avfs_base,WJ_AVFS_INTR_MASK);
	wj_avfs_intr_tmp_clear(avfs_base,WJ_AVFS_TMP_INTR_MASK);

	/* enable all temperature irq */
	msk = WJ_AVFS_TMP_INTR_MASK;
	wj_avfs_intr_tmp_enable(avfs_base,msk);
	
	/* enable important intr irq */
	msk = WJ_AVFS_INTR_MASK;
	msk &=~(WJ_AVFS_INTR_VS_DONE_MASK | \
	        WJ_AVFS_INTR_TS_DONE_MASK | \
			WJ_AVFS_INTR_PS_DONE_MASK | \
			WJ_AVFS_INTR_PMIC_DONE_MASK);
	wj_avfs_intr_enable(avfs_base,msk);
	csi_irq_attach(avfs->dev.irq_num, wj_avfs_irqhandler, &avfs->dev);

	csi_irq_enable(avfs->dev.irq_num);

	return CSI_OK;
}

csi_error_t csi_avfs_detach_callback(csi_avfs_t *avfs)
{
	unsigned long status;
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	
	status = csi_irq_save();
	avfs->arg = NULL;
	avfs->call_back = NULL;
	csi_irq_restore(status);

	return CSI_OK;
}

csi_error_t csi_avfs_set_hw_constant_param(csi_avfs_t *avfs, csi_avfs_param_id_t param_tag, void *pdata)
{
	CSI_PARAM_CHK(pdata,CSI_ERROR);
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);	
	uint32_t item_num = 0;
	csi_error_t ret = CSI_OK;

	if(param_tag != *((csi_avfs_param_id_t*)pdata) ) {
		return CSI_ERROR;
	}

	switch(param_tag) {
		case AVFS_HW_PARAM_CPU_FREQ_PATTERN:
		{
			csi_avfs_param_cpu_freq_pattern_t *param = (csi_avfs_param_cpu_freq_pattern_t*)pdata;
			while (item_num < param->len && item_num < TAR_FREQ_LVL_MAX)
			{
				ret = wj_avfs_misc_set_freq_lvl(avfs_base,item_num,param->freq_pattern[item_num]);
				item_num++;
			}
		}	
		break;

		case AVFS_HW_PARAM_VM_ARRAY_LVL:
		{
			csi_avfs_param_vm_array_t *param = (csi_avfs_param_vm_array_t*)pdata;
			wj_avfs_vol_set_memarray_vltg(avfs_base,param->mem_array_vltg[0],\
			                                              param->mem_array_vltg[1],\
													      param->mem_array_vltg[2]);

			wj_avfs_vol_set_voltage_mask(avfs_base,param->cmd_vltg_mask[0], \
			                                                    param->cmd_vltg_mask[1]);
		}
		break;

		case AVFS_HW_PARAM_CGM_PATTERN:
		{
			csi_avfs_param_cgm_pattern_t *param = (csi_avfs_param_cgm_pattern_t*)pdata;
			wj_avfs_cgm_set_fix_patern(avfs_base,param->fix_pattern);
			while (item_num < param->len && item_num < CGM_INC_PATTERN_NUM)
			{
				wj_avfs_cgm_set_inc_pattern(avfs_base,item_num,param->inc_pattern[item_num]);
				item_num++;
			}
		}
		break;

		case AVFS_HW_PARAM_CMU_GEAR_CFG:
		{
			csi_avfs_param_cmu_gear_t *param = (csi_avfs_param_cmu_gear_t*)pdata;
			if(param->len > CGM_PLL_GEAR_CONFIG_NUM){
				ret = CSI_ERROR;
				break;
			}
			wj_avfs_cmu_set_pll_gear_params(avfs_base,(pll_cfg_t*)param->pll_param,param->len);
		}
		break;

		case AVFS_HW_PARAM_PLL_SWITCH_CFG:
		{
			csi_avfs_param_pll_switch_t* param = (csi_avfs_param_pll_switch_t*)pdata;
			wj_avfs_cmu_pll_set_switch_param(avfs_base,param->pll_switch,param->len);
		}
		break;

		case AVFS_HW_PARAM_ACLK_TLD_CFG:
		{
			csi_avfs_param_aclk_tld_t *param = (csi_avfs_param_aclk_tld_t*)pdata;
			while(item_num < param->len && item_num < FREQ_TLD_NUM)
			{
				wj_avfs_aclk_tld_config(avfs_base,item_num,param->aclk_tld_cfg[item_num]);
				item_num++;
			}
		}
		break;

		case AVFS_HW_PARAM_FPV_CFG:
		{
			csi_avfs_param_fpv_t *param = (csi_avfs_param_fpv_t*)pdata;
			while (item_num < param->len && item_num < FPV_TABLE_LEN)
			{
				wj_avfs_misc_config_fpv(avfs_base,item_num,param->fpv[item_num].p,param->fpv[item_num].vddx,param->fpv[item_num].vddm);	
				item_num++;
			}
		}
		break;

		case AVFS_HW_PARAM_PS_WEIGHT:
		{
			csi_avfs_param_psensor_weight_t *param = (csi_avfs_param_psensor_weight_t*)pdata;
			while (item_num < param->len && item_num < PS_WEIGHT_LEN)
			{
				wj_avfs_misc_config_psensor_low_weight(avfs_base,item_num,param->pwt_low[item_num]);
				wj_avfs_misc_config_psensor_normal_weight(avfs_base,item_num,param->pwt_nor[item_num]);
				wj_avfs_misc_config_psensor_high_weight(avfs_base,item_num,param->pwt_high[item_num]);
				item_num++;
			}
		}
		break;

		case AVFS_HW_PARAM_PMIC_CMD_A:
		case AVFS_HW_PARAM_PMIC_CMD_B:
		{
			csi_avfs_param_cmd_t *param = (csi_avfs_param_cmd_t*)pdata;
			uint32_t grp = (param_tag == AVFS_HW_PARAM_PMIC_CMD_A)? AVFS_PMIC_CMD_GRP_A:AVFS_PMIC_CMD_GRP_B; 
			csi_avfs_fill_pmic_cmd(avfs,grp,param->len,(cmd_param_t*)(param->cmd));
		}
		break;

		default:
			return CSI_UNSUPPORTED;
			
	}

	return ret;
}

csi_error_t csi_avfs_hw_disable(csi_avfs_t *avfs)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	wj_avfs_misc_hw_auto_disable(avfs_base);
	return CSI_OK;
}

csi_error_t csi_avfs_hw_enable(csi_avfs_t *avfs)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	wj_avfs_misc_hw_auto_enable(avfs_base);

	return CSI_OK;
}

csi_error_t csi_avfs_freq_change_request(csi_avfs_t *avfs,uint16_t freq_in, uint16_t *freq_out, uint32_t time_out)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	//uint32_t time_start = csi_tick_get_ms();


	/*wait avfs engine idle */
	while (wj_avfs_misc_is_state_busy(avfs_base))
	{
		//if(csi_tick_get_ms() > time_start + time_out) {
		//	return CSI_TIMEOUT;
		//}
	}
	
	wj_avfs_misc_set_target_freq(avfs_base,freq_in);
	wj_avfs_misc_freq_change_enable(avfs_base);
	
	/* wait avfs freq done */
	while (wj_avfs_misc_is_state_busy(avfs_base))
	{
		//if(csi_tick_get_ms() > time_start + time_out) {
		//	return CSI_TIMEOUT;
		//}
	}

	/* check freq change result */
	*freq_out = wj_avfs_cmu_get_cur_freq_lvl(avfs_base);
	if(*freq_out != freq_in) {
		return CSI_ERROR;
	}

	return CSI_OK;
}

csi_error_t csi_avfs_config_mainctrl(csi_avfs_t *avfs,csi_avfs_mainctrl_cfg_t *pdata)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	CSI_PARAM_CHK(pdata,CSI_ERROR);

	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	wj_avfs_misc_wkmode_set_iteration_count(avfs_base,pdata->max_iterate_num);

	/* whether bypass aclk */
	if(pdata->bp_aclk) {
		wj_avfs_misc_wkmode_bypass_aclk(avfs_base,1);
	}
	else {
		wj_avfs_misc_wkmode_bypass_aclk(avfs_base,0);
	}

	/* whether bypass vsensor */
	if(pdata->bp_vsensor) {
		wj_avfs_misc_wkmode_bypass_vsensor(avfs_base,1);
	}
	else {
		wj_avfs_misc_wkmode_bypass_vsensor(avfs_base,0);
	}
	/* whether bypas vol-adjust */
	if(pdata->bp_voltage) {
		wj_avfs_misc_wkmode_bypass_voltage(avfs_base,1);
	}
	else {
		wj_avfs_misc_wkmode_bypass_voltage(avfs_base,0);
	}

	/* whether bypass psensor */
	if(pdata->bp_psensor) {
		wj_avfs_misc_wkmode_bypass_psensor(avfs_base,1);
	}
	else {
		wj_avfs_misc_wkmode_bypass_psensor(avfs_base,0);
	}

	/* whether bypass freq-down */
	if(pdata->bp_freq_down) {
		wj_avfs_misc_wkmode_bypass_freq_down(avfs_base,1);
	}
	else {
		wj_avfs_misc_wkmode_bypass_freq_down(avfs_base,0);
	}

	uint32_t cycles = (soc_get_avfs_freq()/1000000) * pdata->monitor_timer;

	wj_avfs_misc_set_monitor_timer(avfs_base,cycles);

	return CSI_OK;
}

csi_error_t csi_avfs_config_pmic(csi_avfs_t *avfs, csi_avfs_pmic_cfg_t *pdata)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	CSI_PARAM_CHK(pdata,CSI_ERROR);

	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	wj_avfs_vol_cfg_margin(avfs_base,pdata->vol_margin);
	wj_avfs_vol_cfg_pmic_dvs_rate(avfs_base,pdata->pmic_dvs_rate);
	wj_avfs_vol_cfg_mi_rg_dual_rail_mem_offset(avfs_base,pdata->rg_dual_rail_mem_offset);
	wj_avfs_vol_cfg_step_value(avfs_base,pdata->vol_step);
	wj_avfs_vol_set_counter(avfs_base,pdata->pmic_cnt_base);
	if(pdata->dua_rail_mem_vltg_ena) {
		wj_avfs_vol_dual_rail_mem_vltg_enable(avfs_base);
	}
	else {
		wj_avfs_vol_dual_rail_mem_vltg_disable(avfs_base);
	}
    wj_avfs_vol_dual_rail_mem_vltg_mode(avfs_base,pdata->dual_rail_mem_seperate_mode);
    wj_avfs_vol_config_cur_voltage(avfs_base,pdata->cur_vmem_val,pdata->cur_vdd_val);
	wj_avfs_vol_dual_rail_mem_config(avfs_base,
	                                pdata->dual_rail_mem_vol_step_down,
									pdata->dual_rail_mem_magin_down,
									pdata->dual_rail_mem_margin_up,
									pdata->dual_rail_mem_vol_step_up);
	if(pdata->cmd_grp_ena) {
		wj_avfs_vol_cmd_grp_enable(avfs_base);
	} else {
		wj_avfs_vol_cmd_grp_disable(avfs_base);
	}

	return CSI_OK;
}

csi_error_t csi_avfs_config_pvt(csi_avfs_t *avfs, csi_avfs_pvt_cfg_t *pdata)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	CSI_PARAM_CHK(pdata,CSI_ERROR);

	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	wj_avfs_sensor_set_pmargin(avfs_base,pdata->ps_margin);
	wj_avfs_sensor_cfg_psensor_type(avfs_base,pdata->ps_sampe_type);
	wj_avfs_sensor_ps_mgr_en(avfs_base,pdata->ps_mgr_en);

	wj_avfs_sensor_set_tthd(avfs_base,pdata->ts_thd_low,pdata->ts_thd_high);
	wj_avfs_sensor_set_twarn(avfs_base,pdata->ts_warn_low,pdata->ts_warn_high);
	wj_avfs_sensor_ts_config(avfs_base,pdata->ts_mgr_en,pdata->ts_sampe_type,pdata->ts_freq_down);

	wj_avfs_sensor_set_vthd(avfs_base,pdata->vs_thd_low,pdata->vs_thd_high);
	wj_avfs_sensor_vs_mgr_en(avfs_base,pdata->vs_mgr_en);

	return CSI_OK;
}

csi_error_t csi_avfs_config_cgm(csi_avfs_t *avfs, csi_avfs_cgm_cfg_t *pdata)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	CSI_PARAM_CHK(pdata,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	wj_avfs_cgm_set_ramp_rate(avfs_base,pdata->ramp_rate);
	if(pdata->cgm_direct_up) {
		wj_avfs_cgm_freq_direct_up_enable(avfs_base);
	}
	else {
		wj_avfs_cgm_freq_direct_up_disable(avfs_base);
	}

	if(pdata->cgm_freq_dirc_up_cgm_en) {
		wj_avfs_cgm_freq_cgm_up_enable(avfs_base);
	}
	else {
		wj_avfs_cgm_freq_cgm_up_disable(avfs_base);
	}
	
	if(pdata->cgm_freq_down) {
		wj_avfs_cgm_freq_down_bf_volup_enable(avfs_base);
	}
	else {
		wj_avfs_cgm_freq_down_bf_volup_disable(avfs_base);
	}

	wj_avfs_cgm_set_gap(avfs_base,pdata->cgm_gap);
	wj_avfs_cgm_set_gap_margin(avfs_base,pdata->cgm_gap_margin);
	wj_avfs_cgm_set_ramp_rate(avfs_base,pdata->ramp_rate);


	return CSI_OK;
}

csi_error_t csi_avfs_config_cmu(csi_avfs_t *avfs, csi_avfs_cmu_cfg_t *pdata)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	CSI_PARAM_CHK(pdata,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	wj_avfs_cmu_set_clk_switch_wait_time(avfs_base,pdata->clk_sw_time);
	wj_avfs_cmu_set_pll_disable_time(avfs_base,pdata->pll_disable_time);
	wj_avfs_cmu_set_pll_lock_time(avfs_base,pdata->pll_lock_time);
	wj_avfs_cmu_set_pll_lock_hold_time(avfs_base,pdata->pll_lock_hd_time);
	wj_avfs_cmu_set_freq_stable_time(avfs_base,pdata->freq_stable_time);

	if(pdata->lock_timer_enable) {
		wj_avfs_cmu_pll_lock_timer_enable(avfs_base);
	}
	else {
		wj_avfs_cmu_pll_lock_timer_disable(avfs_base);
	}

	return CSI_OK;
}

csi_error_t csi_avfs_config_aclk(csi_avfs_t *avfs)
{
	return CSI_OK;
}

csi_error_t csi_avfs_start_avfs_monitor_timer(csi_avfs_t *avfs)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	wj_avfs_misc_wkmode_periodic_sensor_detect(avfs_base,1);
	return CSI_OK;
}

csi_error_t csi_avfs_stop_avfs_monitor_timer(csi_avfs_t *avfs)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	wj_avfs_misc_wkmode_periodic_sensor_detect(avfs_base,0);
	return CSI_OK;
}

static csi_error_t csi_avfs_fill_pmic_cmd(csi_avfs_t *avfs,uint32_t cmd_grp, uint32_t cmd_num, cmd_param_t *cmd_param)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	CSI_PARAM_CHK(cmd_param,CSI_ERROR);
	uint32_t index = 0;	
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	if(cmd_num > CMD_GRP_LEN || cmd_grp >= AVFS_PMIC_CMD_GRP_MAX) {
		return CSI_ERROR;
	}

	while(index < cmd_num) {
		wj_avfs_vol_fill_cmd(avfs_base,
		                     cmd_grp,
							 index,
							 cmd_param->cmd_vld,
							 cmd_param->vltg0_vld,
							 cmd_param->vltg1_vld,
							 cmd_param->addr,
							 cmd_param->data);
		index++;
		cmd_param++;
							
	}

	/* set invalid cmd */
	if(index < CMD_GRP_LEN) {
		wj_avfs_vol_fill_cmd(avfs_base,
		                     cmd_grp,
							 index,
							 0,
							 0,
							 0,
							 0,
							 0);
	}

	return CSI_OK;	
}

csi_error_t csi_avfs_wait_ts_ready(csi_avfs_t *avfs,uint32_t time_out)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	uint32_t timestart = csi_tick_get_ms();
	while (0 == wj_avfs_get_ts_data(avfs_base,0) )
	{
		if(csi_tick_get_ms() - timestart >= (timestart+time_out)) {
			return CSI_TIMEOUT;
		}
	}

	return CSI_OK;
}

void csi_avfs_pvt_single_write_reg(csi_avfs_t *avfs,pvt_type_t type, uint32_t sensor_index,uint32_t sensor_reg,uint8_t val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	wj_avfs_pvt_single_write_reg(avfs_base,type,sensor_index,sensor_reg,val);
	return;
}

void csi_avfs_pvt_searial_write_reg(csi_avfs_t *avfs, pvt_type_t type, uint32_t sensor_reg, uint8_t val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	wj_avfs_pvt_searial_write_reg(avfs_base,type,sensor_reg,val);
	return;
}

csi_error_t csi_avfs_pvt_single_read_reg(csi_avfs_t *avfs, pvt_type_t type, uint32_t sensor_index, uint32_t sensor_reg, uint16_t *val)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	csi_error_t ret;
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	ret = wj_avfs_pvt_single_read_reg(avfs_base,type,sensor_index,sensor_reg,val);
	return ret;	
}

csi_error_t csi_avfs_pvt_serial_read_reg(csi_avfs_t *avfs, pvt_type_t type, uint32_t sensor_reg,uint16_t *sensor_data,uint16_t sensor_begin,uint16_t num)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	csi_error_t ret;
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	ret = wj_avfs_pvt_searial_read_reg(avfs_base,type,sensor_reg,sensor_data,sensor_begin,num);
	return ret;	
}

csi_error_t csi_avfs_cmu_sw_pll_config(csi_avfs_t *avfs,pll_param_t *pll_param,uint32_t pll_idx)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	CSI_PARAM_CHK(pll_param,CSI_ERROR);
	csi_error_t ret = CSI_OK;
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	pll_cfg_t pll;
	uint32_t timestart = csi_tick_get_ms();

	pll.pll_cfg0 = pll_param->cfg0;
	pll.pll_cfg1 = pll_param->cfg1;

	/* config pll */
	wj_avfs_cmu_cfg_pll(avfs_base,&pll);
	/* select pll and send freq change */
	wj_avfs_cmu_pll_ctrl(avfs_base,pll_idx);

	/* wait pll stable */
	while(!(wj_avfs_cmu_get_pll_status(avfs_base) & (1 << pll_idx))) {
		if(csi_tick_get_ms() - timestart > 10) {
			ret = CSI_TIMEOUT;
			break;
		}
	}
	udelay(10);

	return ret;
}

void csi_avfs_cmu_clk_switch(csi_avfs_t *avfs,csi_avfs_clk_switch_t *clk_switch)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	uint32_t sw_config;

	sw_config = clk_switch->pll_switch | \
	            (clk_switch->pll0_sw0 << WJ_AVFS_CMU_CLK_SWITCH_PLL0_SW0_POS) | \
				(clk_switch->pll0_sw1 << WJ_AVFS_CMU_CLK_SWITCH_PLL0_SW1_POS) | \
				(clk_switch->pll1_sw0 << WJ_AVFS_CMU_CLK_SWITCH_PLL1_SW0_POS) | \
				(clk_switch->pll1_sw1 << WJ_AVFS_CMU_CLK_SWITCH_PLL1_SW1_POS) | \
				(clk_switch->inc_mode_req << WJ_AVFS_CMU_INC_MODE_REQ_POS)    | \
				(clk_switch->di_aclk_cali_req << WJ_AVFS_CMU_DI_ACLK_CALI_POS) | \
				(clk_switch->di_aclk_freq_down << WJ_AVFS_CMU_DI_ACLK_FREQ_DOWN_POS);
	avfs_base->freq_base.CLK_SWITCH = sw_config;

	return ;

}

void csi_avfs_cgm_sw_freq_down_by_fix_patten(csi_avfs_t *avfs, bool ena)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);

	if(ena) {
		wj_avfs_cgm_ctrl_fix_mode_enable(avfs_base);
	}
	else {

		wj_avfs_cgm_ctrl_fix_mode_disable(avfs_base);
	}
}

uint32_t csi_avfs_get_psensor_err_status(csi_avfs_t *avfs)
{
	CSI_PARAM_CHK(avfs,CSI_ERROR);
	uint32_t status;

	wj_avfs_ctrl_regs_t *avfs_base = (wj_avfs_ctrl_regs_t*)HANDLE_REG_BASE(avfs);
	status = wj_avfs_sensor_get_err_status(avfs_base);
	wj_avfs_sensor_clear_ps_err_status(avfs_base,status);
	return status;
}

