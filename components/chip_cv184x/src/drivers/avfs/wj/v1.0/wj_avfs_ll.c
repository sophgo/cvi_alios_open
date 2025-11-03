/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file    wj_avfs_ll.c
 * @brief
 * @version
 * @date     01. Dec 2020
 ******************************************************************************/

#include <drv/avfs.h>
#include <drv/cvi_irq.h>
#include "wj_avfs_ll.h"
#include <drv/tick.h>

csi_error_t wj_avfs_misc_set_freq_lvl(wj_avfs_ctrl_regs_t *avfs_base, avfs_tar_freq_lvl_t lvl, uint32_t freq_val)
{
	if(lvl >= TAR_FREQ_LVL_MAX) {
		return CSI_ERROR;
	}
	if(lvl & 0x01) {
		avfs_base->misc_base.FREQ_LVL_VAL[lvl>>1] &= ~(0xfff<<16);
		avfs_base->misc_base.FREQ_LVL_VAL[lvl>>1] |= ((freq_val&0xfff)<<16);
	}
	else {
		avfs_base->misc_base.FREQ_LVL_VAL[lvl>>1] &= ~0xfff;
		avfs_base->misc_base.FREQ_LVL_VAL[lvl>>1] |= (freq_val&0xfff);
	}

	return CSI_OK;
}

void wj_avfs_vol_config(wj_avfs_ctrl_regs_t *avfs_base,uint32_t step_val, uint32_t margin_val, uint32_t dvs_rate)
{
	uint32_t cfg = (step_val & 0xff) | \
	               ((margin_val & 0xff) << WJ_AVFS_VOL_CFG_MARGIN_POS ) |
				   ((dvs_rate & 0xff) << WJ_AVFS_VOL_CFG_PMIC_DVS_RATE_POS);
	avfs_base->vol_base.VOL_CFG = cfg;
	return;
}

void wj_avfs_vol_fill_cmd(wj_avfs_ctrl_regs_t * avfs_base,
                                pmic_cmd_grp_t        grp_num,
								uint32_t              cmd_idx,
								bool                  bcmd,
								bool                  bvltg0,
								bool                  bvltg1,
								uint32_t              addr,
								uint32_t              data)
{
	uint32_t       cmd = 0;
	__IOM uint32_t *p_grp_base = (grp_num == AVFS_PMIC_CMD_GRP_A) ? avfs_base->vol_base.PMIC_CMD_A : avfs_base->vol_base.PMIC_CMD_B;
	cmd = ((bcmd)?WJ_AVFS_VOL_CMD_GRP_PARAM_CMD_VLD:0 ) | \
	      ((bvltg0) ? WJ_AVFS_VOL_CMD_GRP_PARAM_VLTG0_VLD : 0) | \
	      ((bvltg1) ? WJ_AVFS_VOL_CMD_GRP_PARAM_VLTG1_VLD : 0) | \
	      (addr << WJ_AVFS_VOL_CMD_GRP_PARAM_ADDR_POS) | \
		  data;
	p_grp_base[cmd_idx] = cmd;

	return;
}								

void wj_avfs_vol_spi_write_reg(wj_avfs_ctrl_regs_t *avfs_base, uint8_t reg_addr, uint8_t reg_val,uint8_t rd_pol)
{
	uint32_t val;
	 val =  WJ_AVFS_VOL_SPI_SW_SPI_EN_EN | \
	        (reg_addr << WJ_AVFS_VOL_SPI_CMD0_ADDR_POS) | \
			((rd_pol)? 0 : WJ_AVFS_VOL_SPI_CMD0_CMD_RW_MASK) | \
			reg_val;
	avfs_base->vol_base.PMIC_SPI_CMD0 = val;
	udelay(1);
	while(wj_avfs_vol_is_spi_busy(avfs_base));
	return;
}

uint8_t wj_avfs_vol_spi_read_reg(wj_avfs_ctrl_regs_t *avfs_base,uint8_t reg_addr,uint8_t rd_pol) 
{
	
	avfs_base->vol_base.PMIC_SPI_CMD0 = WJ_AVFS_VOL_SPI_SW_SPI_EN_EN | \
										((rd_pol)? WJ_AVFS_VOL_SPI_CMD0_CMD_RW_MASK : 0) | \
	                                    (reg_addr << WJ_AVFS_VOL_SPI_CMD0_ADDR_POS);
	
	udelay(1);
	while(wj_avfs_vol_is_spi_busy(avfs_base));
	return avfs_base->vol_base.PMIC_SPI_CMD1;
}

void wj_avfs_cmu_pll_set_switch_param(wj_avfs_ctrl_regs_t *avfs_base,uint8_t *clk_switch_item,uint32_t item_num)
{
	uint32_t index = 0;
	while(index < CGM_PLL_GEAR_CONFIG_NUM && index < item_num)
	{
		avfs_base->freq_base.PLL_SWITCH_CFG[index >> 2] &= ~(0xff << ((index & 0x3)<<3));
		avfs_base->freq_base.PLL_SWITCH_CFG[index >> 2] |= (*clk_switch_item << ((index & 0x3)<< 3));
		index++;
		clk_switch_item++;
	}	
}

void wj_avfs_pvt_send_user_cmd(wj_avfs_ctrl_regs_t *avfs_base,
                                  pvt_type_t type,
								  uint32_t   cmd,
								  uint32_t   sensor_index,
								  uint32_t   sensor_reg,
								  uint8_t    value)
{
	uint32_t sw_cfg = 0;
	sw_cfg = sensor_reg | \
	         (sensor_index << WJ_AVFS_SENSOR_PVT_SENSOR_IDX_P0S) | \
			 (cmd << WJ_AVFS_SENSOR_PVT_CMD_TYPE_POS) | \
			 (value << WJ_AVFS_SENSOR_PVT_WDATA_POS) | \
			 WJ_AVFS_SENSOR_PVT_CMD_EN;
	avfs_base->sensor_base.PVT_SW_CFG[type] = sw_cfg;
	
	while(!(avfs_base->sensor_base.PVT_SW_CFG[type] & WJ_AVFS_SENSOR_PVT_CMD_READY_MASK));
	return;
}

void wj_avfs_pvt_single_write_reg(wj_avfs_ctrl_regs_t *avfs_base,
                                  pvt_type_t type,
								  uint32_t   sensor_index,
								  uint32_t   sensor_reg,
								  uint8_t    value)
{
	uint32_t sw_cfg = 0;
	sw_cfg = sensor_reg | \
	         (sensor_index << WJ_AVFS_SENSOR_PVT_SENSOR_IDX_P0S) | \
			 (WJ_AVFS_SENSOR_PVT_CMD_SING_WRITE << WJ_AVFS_SENSOR_PVT_CMD_TYPE_POS) | \
			 (value << WJ_AVFS_SENSOR_PVT_WDATA_POS) | \
			 WJ_AVFS_SENSOR_PVT_CMD_EN;
	avfs_base->sensor_base.PVT_SW_CFG[type] = sw_cfg;
	
	while(!(avfs_base->sensor_base.PVT_SW_CFG[type] & WJ_AVFS_SENSOR_PVT_CMD_READY_MASK));
	return;
}

void wj_avfs_pvt_searial_write_reg(wj_avfs_ctrl_regs_t *avfs_base,
                                  pvt_type_t type,
								  uint32_t   sensor_reg,
								  uint8_t    value)
{
	uint32_t sw_cfg = 0;

	sw_cfg = sensor_reg | \
			 (WJ_AVFS_SENSOR_PVT_CMD_SERIAL_WRITE << WJ_AVFS_SENSOR_PVT_CMD_TYPE_POS) | \
			 (value << WJ_AVFS_SENSOR_PVT_WDATA_POS) | \
			 WJ_AVFS_SENSOR_PVT_CMD_EN;
	avfs_base->sensor_base.PVT_SW_CFG[type] = sw_cfg;

	while(!(avfs_base->sensor_base.PVT_SW_CFG[type] & WJ_AVFS_SENSOR_PVT_CMD_READY_MASK));
	return;
}

csi_error_t wj_avfs_pvt_get_sensor_single_data(wj_avfs_ctrl_regs_t *avfs_base,
                                            pvt_type_t type,
											uint32_t   sensor_index,
											uint16_t   *pdata)
{
	
	switch (type)
	{
	case PVT_TYPE_PROCESS:
		if(sensor_index >= PS_DATA_LEN) {
			return CSI_ERROR;
		}
		*pdata = (uint16_t)(avfs_base->mem_data.P_DATA[sensor_index]);
		break;

	case PVT_TYPE_VOL:
		if(sensor_index >= VS_DATA_LEN) {
			return CSI_ERROR;
		}
		*pdata = (uint16_t)(avfs_base->mem_data.V_DATA[sensor_index]);
		break;

	case PVT_TYPE_TMP:
		if(sensor_index >= TS_DATA_LEN) {
			return CSI_ERROR;
		}
		
		*pdata = (sensor_index & 0x1) ? (avfs_base->sensor_base.TSENSOR_DATA[sensor_index >> 1] >> 16) \
		                                : (avfs_base->sensor_base.TSENSOR_DATA[sensor_index >> 1] & 0xfff);

		break;

	default:
		return CSI_UNSUPPORTED;
	}

	return CSI_OK;
}

csi_error_t wj_avfs_pvt_get_sensor_all_data(wj_avfs_ctrl_regs_t *avfs_base,
                                            pvt_type_t type,
											uint16_t  item_start,
											uint16_t  itme_num,
											uint16_t *pdata)
{
	uint32_t i;
	csi_error_t ret;
	uint16_t    *data = pdata;
	for(i = 0; i < itme_num; i++) {
		ret = wj_avfs_pvt_get_sensor_single_data(avfs_base,type,i+item_start,data);
		if(ret) {
			break;
		}
		data++;
	}

	return ret;
}

csi_error_t wj_avfs_pvt_single_read_reg(wj_avfs_ctrl_regs_t *avfs_base,
                                  pvt_type_t type,
								  uint32_t   sensor_index,
								  uint32_t   sensor_reg,
								  uint16_t    *value)
{
	uint32_t sw_cfg = 0;
	csi_error_t ret;
	uint16_t    data = 0;	
	/* build send read cmd ,then send cmd */
	sw_cfg = sensor_reg | \
	         (sensor_index << WJ_AVFS_SENSOR_PVT_SENSOR_IDX_P0S) | \
			 (WJ_AVFS_SENSOR_PVT_CMD_SING_READ << WJ_AVFS_SENSOR_PVT_CMD_TYPE_POS) | \
			 WJ_AVFS_SENSOR_PVT_CMD_EN;
	avfs_base->sensor_base.PVT_SW_CFG[type] = sw_cfg;

	/* wait bus idle */
	while(!(avfs_base->sensor_base.PVT_SW_CFG[type] & WJ_AVFS_SENSOR_PVT_CMD_READY_MASK));

	/* get sensor-data */
	ret = wj_avfs_pvt_get_sensor_single_data(avfs_base,type,sensor_index,&data);
	if(ret) {
		return ret;
	}
	*value = data;
	return CSI_OK;
}

csi_error_t wj_avfs_pvt_searial_read_reg(wj_avfs_ctrl_regs_t *avfs_base,
                                  pvt_type_t type,
								  uint32_t   sensor_reg,
								  uint16_t   *item_array,
								  uint16_t    item_start,
								  uint16_t    item_num)
{
	uint32_t sw_cfg = 0;
	csi_error_t ret;
    /* check read item */
	if(type == PVT_TYPE_PROCESS && (item_start + item_num) > PS_DATA_LEN) {
		return CSI_ERROR;
	}

	if(type == PVT_TYPE_VOL && (item_start + item_num) > VS_DATA_LEN) {
		return CSI_ERROR;
	}

	if(type == PVT_TYPE_TMP && (item_start + item_num) > TS_DATA_LEN) {
		return CSI_ERROR;
	}

	sw_cfg = sensor_reg | \
			 (WJ_AVFS_SENSOR_PVT_CMD_SERIAL_READ << WJ_AVFS_SENSOR_PVT_CMD_TYPE_POS) | \
			 WJ_AVFS_SENSOR_PVT_CMD_EN;
	avfs_base->sensor_base.PVT_SW_CFG[type] = sw_cfg;

	while(!(avfs_base->sensor_base.PVT_SW_CFG[type] & WJ_AVFS_SENSOR_PVT_CMD_READY_MASK));
	ret = wj_avfs_pvt_get_sensor_all_data(avfs_base,
	                                      type,
										  item_start,
										  item_num,
										  item_array);	
	return ret;
}

ts_range_t wj_avfs_sensor_get_tmp_range(wj_avfs_ctrl_regs_t *avfs_base)
{
	uint32_t tmp = (avfs_base->sensor_base.TSENSOR_CFG & WJ_AVFS_TSENSOR_RNG_MASK) >> WJ_AVFS_TSENSOR_RNG_POS ;
	ts_range_t ret = TS_RNG_UNKNOW;

	switch (tmp)
	{
	case WJ_AVFS_TSENSOR_RNG_LOW:
		ret = TS_RNG_LOW;
		break;
	
	case WJ_AVFS_TSENSOR_RNG_NORMAL:
		ret = TS_RNG_NORMAL;
		break;

	case WJ_AVFS_TSENSOR_RNG_HIGH:
		ret = TS_RNG_HIGH;
		break;
	default:
		break;
	}

	return ret;
}

void wj_avfs_sensor_ts_config(wj_avfs_ctrl_regs_t *avfs_base,bool mgr_ena,ts_sample_type_t type,bool bauto_freq_adjust)
{
	uint32_t cfg = 0,tmp = 0;
	if(type == TS_SAMPLE_MIN) {
		tmp = WJ_AVFS_TSENSOR_CFG_TYPE_MIN << WJ_AVFS_TSENSOR_CFG_TYPE_POS;
	}
	else if(type == TS_SAMPLE_MAX) {
		tmp = WJ_AVFS_TSENSOR_CFG_TYPE_MAX << WJ_AVFS_TSENSOR_CFG_TYPE_POS;
	}
	else {
		tmp = WJ_AVFS_TSENSOR_CFG_TYPE_AVG << WJ_AVFS_TSENSOR_CFG_TYPE_POS;
	}

	cfg = ((mgr_ena) ? WJ_AVFS_TSENSOR_CFG_MGR_MASK : 0 ) | \
	      (tmp) | \
		  ((bauto_freq_adjust) ? WJ_AVFS_TSENSOR_ERR_FREQ_CHG_EN : 0);
	
	avfs_base->sensor_base.TSENSOR_CFG = cfg;
}


