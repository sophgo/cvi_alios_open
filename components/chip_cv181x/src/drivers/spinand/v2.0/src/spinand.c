/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     spinand.c
 * @brief
 * @version
 * @date     2020-03-18
 ******************************************************************************/
#include <string.h>
#include <drv/spinand.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>
#include "spinand_vendor.h"
#include "cvi_spinand.h"

spinand_msg_prio_e SPINAND_PRINTK_LEVEL = SPINAND_WARN;


/* FLASH operation parameters */
#define DO_DIV(X,Y) X%Y;X /=Y

static uint32_t  g_spinand_probe_dev_flag = 0;

static void spinand_offset_to_nandpos(csi_spinand_t *spinand, uint64_t loff, csi_nand_pos_t *nand_pos)
{
    uint64_t temp = loff;
    nand_pos->offset = DO_DIV(temp,spinand->chip_info->memorg.pagesize);
    nand_pos->page   = DO_DIV(temp,spinand->chip_info->memorg.pages_per_eraseblock);
    nand_pos->block  = DO_DIV(temp,spinand->chip_info->memorg.eraseblocks_per_lun);
    nand_pos->plane  = nand_pos->block % spinand->chip_info->memorg.planes_per_lun;
    nand_pos->lun    = DO_DIV(temp,spinand->chip_info->memorg.luns_per_target);
    nand_pos->target = temp;

    return;
}

static  uint32_t csi_spinand_pos_to_row(csi_spinand_t *spinand, csi_nand_pos_t *pos)
{
   csi_nand_mem_layout_t *pMemOrg = &spinand->chip_info->memorg;
   uint32_t row = pos->page  \
                  + pos->block * pMemOrg->pages_per_eraseblock \
                  + pos->block * pMemOrg->pages_per_eraseblock * pos->lun;
   return row;
}

csi_error_t csi_spinand_select_target(csi_spinand_t *spinand, uint32_t target)
{
    csi_error_t        ret;
    csi_spinand_info_t *pInfo = (csi_spinand_info_t*)spinand->chip_info;

    if(target >= pInfo->memorg.ntargets){
        return CSI_ERROR;
    }

    if(pInfo->memorg.ntargets == 1){
        spinand->cur_target = target;
        return CSI_OK;

    }

    ret = pInfo->select_target(spinand,target);
    if(ret){
        return ret;
    }

    spinand->cur_target = target;

    return CSI_OK;
}

csi_error_t csi_spinand_write_enable_op(csi_spinand_t *spinand)
{
    spi_mem_op_t op = SPINAND_WR_EN_DIS_OP(1);
    return spinand->spi_mem(spinand,&op);
}

csi_error_t csi_spinand_load_page_op(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos)
{
    uint32_t row = csi_spinand_pos_to_row(spinand, nand_pos);
    spi_mem_op_t op = SPINAND_PAGE_READ_OP(row);

    return spinand->spi_mem(spinand,&op);
}

csi_error_t csi_spinand_program_page_op(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos)
{
    uint32_t row = csi_spinand_pos_to_row(spinand,nand_pos);
    spi_mem_op_t op = SPINAND_PROG_EXEC_OP(row);

    return spinand->spi_mem(spinand,&op);
}

csi_error_t csi_spinand_erase_block_op(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos)
{
    uint32_t row = csi_spinand_pos_to_row(spinand,nand_pos);
    spi_mem_op_t op = SPINAND_BLK_ERASE_OP(row);

    return spinand->spi_mem(spinand,&op);
}

csi_error_t csi_spinand_read_reg_op(csi_spinand_t *spinand,uint8_t reg, uint8_t *val)
{
    spi_mem_op_t  op = SPINAND_GET_FEATURE_OP(reg,spinand->scractbuf);
    csi_error_t   ret;

    ret = spinand->spi_mem(spinand,&op);
    if(ret){
        return ret;
    }

    *val = spinand->scractbuf[0];
    return 0;
}

csi_error_t csi_spinand_write_reg_op(csi_spinand_t *spinand,uint8_t reg, uint8_t val)
{
    spi_mem_op_t  op = SPINAND_SET_FEATURE_OP(reg,spinand->scractbuf);
    csi_error_t   ret;

    spinand->scractbuf[0] = val;
    ret = spinand->spi_mem(spinand,&op);
    if(ret){
        return ret;
    }

    return 0;
}
csi_error_t csi_spinand_get_config(csi_spinand_t *spinand,uint8_t *cfg)
{
    csi_error_t ret;

    ret = csi_spinand_read_reg_op(spinand,REG_CFG,cfg);
    return ret;
}

csi_error_t csi_spinand_set_config(csi_spinand_t *spinand, uint8_t cfg)
{
    csi_error_t ret;
    ret = csi_spinand_write_reg_op(spinand,REG_CFG,cfg);
    return ret;
}

csi_error_t csi_spinand_update_config(csi_spinand_t *spinand, uint8_t mask, uint8_t val)
{
    csi_error_t ret;
    uint8_t     cfg;

    /* get config */
    ret = csi_spinand_get_config(spinand,&cfg);
    if(ret){
        return ret;
    }
    /* change config value */
    cfg &=~mask;
    cfg |= val;

    /* set config */
    ret = csi_spinand_set_config(spinand,cfg);
    return ret;
}

static  csi_error_t spinand_ecc_enable(csi_spinand_t *spinand,uint32_t enable)
{
	return csi_spinand_update_config(spinand, CFG_ECC_ENABLE, enable ? CFG_ECC_ENABLE : 0);
}

csi_error_t csi_spinand_read_status(csi_spinand_t *spinand,uint8_t *status)
{
    return csi_spinand_read_reg_op(spinand,REG_STATUS,status);
}

csi_error_t csi_spinand_wait(csi_spinand_t *spinand, uint8_t *status)
{
    #define  SPINAND_MAX_TIME_OUT      (500)
    uint32_t time_end = csi_tick_get_ms() + SPINAND_MAX_TIME_OUT;
    uint8_t  st;
    csi_error_t ret;
    do{
        ret = csi_spinand_read_status(spinand,&st);
        if(ret){
            return ret;
        }

        if(!(st & STATUS_BUSY)){
            break;
        }
    }while(csi_tick_get_ms() < time_end);

    if(status){
        *status = st;
    }

    return st & STATUS_BUSY ? CSI_TIMEOUT : CSI_OK;
}

csi_error_t csi_spinand_read_id(csi_spinand_t *spinand,uint8_t naddr, uint8_t dummy, uint8_t *buf)
{
    csi_error_t ret;
    spi_mem_op_t op = SPINAND_READID_OP(naddr,dummy,spinand->scractbuf,4);

    ret = spinand->spi_mem(spinand,&op);
    if(!ret){
        memcpy(buf,spinand->scractbuf,4);
    }

    return ret;
}


csi_error_t csi_spinand_reset(csi_spinand_t *spinand)
{
    spi_mem_op_t op = SPINAND_RESET_OP;
    csi_error_t ret;

    ret = spinand->spi_mem(spinand,&op);
    if(ret){
        return ret;
    }

    return csi_spinand_wait(spinand,NULL);
}


csi_error_t csi_spinand_id_probe(csi_spinand_t *spinand)
{
    csi_error_t ret;
    uint8_t id[4];

    /* reset device */
    ret = csi_spinand_reset(spinand);
    if(ret){
        return ret;
    }

    ret = csi_spinand_read_id(spinand,0,0,id);
    if(ret){
        return ret;
    }

    /* read id and match */
    ret = csi_spinand_match_vendor(spinand,id);
    if(!ret){
        return ret;
    }

    ret = csi_spinand_read_id(spinand,1,0,id);
    if(ret){
        return ret;
    }

    ret = csi_spinand_match_vendor(spinand,id);
    if(!ret){
        return ret;
    }

    ret = csi_spinand_read_id(spinand,0,1,id);
    if(ret){
		printf("id %02x %02x %02x %02x \n",id[0],id[1],id[2],id[3]);
        return ret;
    }

    return csi_spinand_match_vendor(spinand,id);
}

csi_error_t csi_spinand_unlock_blocks(csi_spinand_t *spinand)
{
    return  csi_spinand_write_reg_op(spinand,REG_BLOCK_LOCK,0);
}

csi_error_t csi_spinand_quad_enable(csi_spinand_t *spinand,int32_t enable)
{
    if(!(spinand->chip_info->flags)){
        return CSI_OK;
    }

    return csi_spinand_update_config(spinand,CFG_QUAD_ENABLE,enable? CFG_QUAD_ENABLE:0);
}

static  uint32_t spinand_get_rowaddr(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos)
{
    return nand_pos->page + nand_pos->block * spinand->chip_info->memorg.pages_per_eraseblock \
                    + nand_pos->lun * spinand->chip_info->memorg.eraseblocks_per_lun;
}

static  uint32_t spinand_get_coladdr(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos)
{
    return (nand_pos->offset | (nand_pos->plane<<12));
}

static csi_error_t spinand_load_page(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos)
{
    uint32_t row_addr;

    row_addr = spinand_get_rowaddr(spinand,nand_pos);
    spi_mem_op_t op = SPINAND_PAGE_READ_OP(row_addr);
    return spinand->spi_mem(spinand,&op);

}

static csi_error_t spinand_copydata_to_userspace(csi_spinand_t *spinand, void *usr_buf)
{
    /* if rxfer_origin_len == 0, no need to copy data from temp_buf(spinand->xfer.xfer_buf) to usr_buf */
    if( spinand->xfer.rxfer_origin_len == 0 ){
        return CSI_OK;
    }
    else{
        memcpy(usr_buf,(uint8_t*)(spinand->xfer.xfer_buf)+ spinand->xfer.rxfer_copy_offset,spinand->xfer.rxfer_origin_len);
        spinand->xfer.rxfer_origin_len = spinand->xfer.rxfer_copy_offset = 0;
    }

    return CSI_OK;
}

static csi_error_t spinand_adjust_rxfer_by_32bit(csi_spinand_t *spinand, spi_mem_op_t *op)
{
    uint32_t length;

    /* both offset and length are word aligned, no need adjust xfer buf */
    if( (op->addr.val & 0x03) == 0 && (op->data.nbytes & 0x03) == 0 ){
        spinand->xfer.rxfer_copy_offset = 0;
        spinand->xfer.rxfer_origin_len  = 0;
        return CSI_OK;
    }
    else{
        /* adjust transfer addr and length */
        if(spinand->xfer.xfer_buf == NULL){
            return CSI_ERROR;
        }
        else{
            length = ( (op->addr.val & 0x03) + op->data.nbytes + (sizeof(uint32_t)-1)) & (~0x03);
            /* adjust buf */
            op->data.buf.in  = spinand->xfer.xfer_buf;
            /* adjust addr (word aligned) */
            spinand->xfer.rxfer_copy_offset = op->addr.val & 0x03;
            op->addr.val     = op->addr.val & (~0x03);
            /* adjust buf length */
            spinand->xfer.rxfer_origin_len = op->data.nbytes;
            op->data.nbytes  = length;
        }
    }

    return CSI_OK;
}
// static csi_error_t spinand_adjust_wxfer_by_32bit(csi_spinand_t *spinand, spi_mem_op_t *op)
// {
//     uint32_t offset;
//     uint32_t length;

//     /* both offset and length are word aligned, no need adjust xfer buf */
//     if( (op->addr.val & 0x03) == 0 && (op->data.nbytes &0x03) == 0 ){
//         return CSI_OK;
//     }
//     else{
//         /* adjust transfer addr and length */
//         if(spinand->xfer.xfer_buf == NULL){
//             return CSI_ERROR;
//         }
//         else{
//             length = ( (op->addr.val & 0x03) + op->data.nbytes + (sizeof(uint32_t)-1)) & (~0x03);     
//             memset(spinand->xfer.xfer_buf,0xff,length);
//             offset=op->addr.val & 0x03;
//             memcpy((uint8_t*)(spinand->xfer.xfer_buf)+offset,op->data.buf.out,op->data.nbytes);
//             /* adjust buf */
//             op->data.buf.out = spinand->xfer.xfer_buf;
//             /* adjust addr (word aligned) */
//             op->addr.val     = op->addr.val & (~0x03);
//             /* adjust buf length */
//             op->data.nbytes  = length;  
//         }
//     }

//     return CSI_OK;
// }
static csi_error_t spinand_read_from_cache(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos,void *buf, uint32_t size)
{
    csi_error_t  ret;
    uint32_t col_addr = spinand_get_coladdr(spinand, nand_pos);
    spi_mem_op_t op = SPINAND_PAGE_READ_FROM_CACHE_X4_OP(col_addr, 1, buf, size);

    ret = spinand_adjust_rxfer_by_32bit(spinand,&op);
    if(ret){
        return ret;
    }

    ret = spinand->spi_mem(spinand,&op);
    if(ret < 0){
        return ret;
    }

    return spinand_copydata_to_userspace(spinand,buf);
}

// static csi_error_t spinand_write_to_cache(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos,void *buf, uint32_t size)
// {
//     csi_error_t ret;
// 	/* use reset-cache buf cmd */
//     spi_mem_op_t op = SPINAND_PROG_LOAD_X4(1,nand_pos->offset,buf,size);

//     ret = spinand_adjust_wxfer_by_32bit(spinand,&op);
//     if(ret){
//         return ret;
//     }
//     return spinand->spi_mem(spinand,&op);
// }

static csi_error_t spinand_write_to_cache_ext(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos,void *buf, uint32_t size)
{
    int32_t ret;
	uint32_t    nbytes;
	uint32_t    adjust_len;
    uint32_t col_addr = spinand_get_coladdr(spinand, nand_pos);
	/* use reset cache buf cmd */
    spi_mem_op_t op = SPINAND_PROG_LOAD_X4(1,0,spinand->xfer.xfer_buf,0);

    /*page-buf-size = page-size + oob-size */
	nbytes = spinand->chip_info->memorg.pagesize + spinand->chip_info->memorg.oobsize;
	memset(spinand->xfer.xfer_buf,0xff,nbytes);

	/*write page-cache */
	memcpy(spinand->xfer.xfer_buf+col_addr,buf,size);

	while( nbytes ) {
		if(nbytes > spinand->max_tx_size) {
			adjust_len = spinand->max_tx_size;
			op.data.nbytes = adjust_len;
		}
		else {
			adjust_len = nbytes;
			op.data.nbytes = adjust_len;
		}
		ret = spinand->spi_mem(spinand,&op);
		if(ret < 0) {
			return ret;
		}
		nbytes -= adjust_len;
		op.addr.val += adjust_len;
		op.data.buf.out += adjust_len;
	}

   return 0;
}

static csi_error_t spinand_check_ecc_status(csi_spinand_t *spinand,uint8_t status)
{
    if(spinand->chip_info->check_ecc_status){
        return spinand->chip_info->check_ecc_status(spinand,status);
    }

    switch(status & STATUS_ECC_MASK)
    {
        case STATUS_ECC_NO_BITFLIPS:
            return CSI_OK;
            break;

        case STATUS_ECC_HAS_BITFLIPS:
            return CSI_OK;
            break;

        case STATUS_ECC_UNCOR_ERROR:
        case STATUS_ECC_UNCOR_ERROR1:
            return CSI_ERROR;
            break;
        default:
            break;
    }
    return CSI_ERROR;
}

static csi_error_t spinand_page_write(csi_spinand_t *spinand,csi_nand_pos_t *nand_pos, void *data, uint32_t size)
{
    uint8_t status;
    csi_error_t ret;

    ret = csi_spinand_select_target(spinand,nand_pos->target);
    if(ret){
        return ret;
    }

    ret = spinand_ecc_enable(spinand,1);
    if(ret){
        return ret;
    }

    ret = csi_spinand_write_enable_op(spinand);
    if(ret){
        return ret;
    }

    ret = spinand_write_to_cache_ext(spinand,nand_pos,data,size);
    if(ret){
        return ret;
    }

    ret = csi_spinand_program_page_op(spinand,nand_pos);
    if(ret){
        return ret;
    }

    ret = csi_spinand_wait(spinand,&status);
    if(ret){
        return ret;
    }

    return (status & STATUS_PROG_FAILED)?CSI_ERROR:CSI_OK;
}

static int32_t spinand_page_read(csi_spinand_t *spinand, csi_nand_pos_t *nand_pos, void *data, uint32_t size,uint32_t ecc_nanble)
{
    csi_error_t ret;
    uint8_t     status;

    do {

        ret = spinand_ecc_enable(spinand,1);
        if(ret){
          return ret;
        }

        /*   load page */
        ret = spinand_load_page(spinand,nand_pos);
        if(ret){
            return ret;
        }
        /*   spinand wait loading done */
        ret = csi_spinand_wait(spinand,&status);
        if(ret){
            return ret;
        }
        /*   read page from  cache */
        ret = spinand_read_from_cache(spinand,nand_pos,data,size);
        if(ret){
            return ret;
        }

    } while (0);

    if(!ecc_nanble){
        return CSI_OK;
    }

    /* whether check ecc satus */
    return spinand_check_ecc_status(spinand,status);
}

int32_t csi_spinand_read(csi_spinand_t *spinand, uint64_t offset, void *data, uint32_t size)
{
    int ret = 0;
    uint64_t              end_addr, current_size, current_addr;
    uint8_t               *read_data;
    csi_spinand_info_t    *param;
    csi_nand_pos_t         nand_pos;
    CSI_PARAM_CHK(spinand, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);

    read_data = (uint8_t *)data;
    param        = (csi_spinand_info_t *)spinand->chip_info;

    /* calculation of the size between the read address and the end of the page */
    current_size = param->memorg.pagesize - (offset % param->memorg.pagesize);

    /* check if the size of the data is less than the remaining place in the page */
    if (current_size >= size) {
        current_size = size;
    }

    /* initialize the adress variables */
    current_addr = offset;
    end_addr = offset + size;

    /* perform the read page by page */
    do {
        spinand_offset_to_nandpos(spinand,current_addr,&nand_pos);
        ret = spinand_page_read(spinand, &nand_pos, (void *)read_data, current_size,1);

        if (ret) {
            return ret;
        }
        /* update the address and size variables for next page read */
        current_addr += current_size;
        read_data += current_size;
        current_size = ((current_addr + param->memorg.pagesize ) > end_addr) ? (end_addr - current_addr) : param->memorg.pagesize;
    } while (current_addr < end_addr);

    return size;
}

int32_t csi_spinand_write(csi_spinand_t *spinand, uint64_t offset, const void *data, uint64_t size)
{
    CSI_PARAM_CHK(spinand, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);

    int ret ;
    uint64_t end_addr, current_size, current_addr;
    uint8_t *write_data = (uint8_t*)data;
    csi_nand_mem_layout_t *pMemOrg = &spinand->chip_info->memorg;
    csi_nand_pos_t         nand_pos;

    current_size = pMemOrg->pagesize - (offset % pMemOrg->pagesize);
    if(current_size >= size){
        current_size = size;
    }

    current_addr = offset;
    end_addr     = offset + size ;

    do{
        spinand_offset_to_nandpos(spinand,current_addr,&nand_pos);
		/* page program */
        ret = spinand_page_write(spinand,&nand_pos,write_data,current_size);
        if(ret){
            return ret;
        }

        current_addr += current_size;
        write_data +=current_size;
        current_size = ((current_addr + pMemOrg->pagesize) > end_addr) ? (end_addr - current_addr) : pMemOrg->pagesize;
    }while(current_addr < end_addr);

    return size;
}

int32_t csi_spinand_read_spare_data(csi_spinand_t *spinand,uint64_t page_addr,uint32_t spare_offset,void *data, uint32_t size)
{
    csi_nand_mem_layout_t *pMemOrg = &spinand->chip_info->memorg;
    uint64_t              temp_addr;
    csi_nand_pos_t        nand_pos;
	int32_t               ret;
    if(spare_offset +size > pMemOrg->oobsize || !data)
    {
        return CSI_ERROR;
    }
    /* addr need page-size aligned */
    temp_addr = page_addr & (~(pMemOrg->pagesize-1));
    spinand_offset_to_nandpos(spinand,temp_addr,&nand_pos);
    nand_pos.offset = pMemOrg->pagesize + spare_offset;
    ret = spinand_page_read(spinand,&nand_pos,data,size,1);
    return (ret)?ret:size;
}
int32_t csi_spinand_write_spare_data(csi_spinand_t *spinand,uint64_t page_addr,uint32_t spare_offset,void *data, uint32_t size)
{
    csi_nand_mem_layout_t *pMemOrg = &spinand->chip_info->memorg;
    uint64_t              temp_addr;
    csi_nand_pos_t        nand_pos;
	int32_t               ret;

    if(spare_offset +size > pMemOrg->oobsize || !data)
    {
        return CSI_ERROR;
    }

    temp_addr = page_addr & (~(pMemOrg->pagesize-1));
    spinand_offset_to_nandpos(spinand,temp_addr,&nand_pos);
    nand_pos.offset = pMemOrg->pagesize + spare_offset;
	ret = spinand_page_write(spinand,&nand_pos,data,size);
    return (ret)?ret:size;
}

int32_t  csi_spinand_block_is_bad(csi_spinand_t *spinand,uint64_t block_addr)
{
    csi_nand_mem_layout_t *pMemOrg = &spinand->chip_info->memorg;
    uint64_t temp_addr ;
    uint16_t   marker;
    int32_t    ret;
    temp_addr = block_addr & (~(pMemOrg->pagesize*pMemOrg->pages_per_eraseblock-1));
    ret = csi_spinand_read_spare_data(spinand,temp_addr,0,&marker,2);
    if(ret != 2)
    {
        return CSI_ERROR;
    }
    return (marker == 0xffff)?0:1;
}

csi_error_t  csi_spinand_block_mark_bad(csi_spinand_t *spinand, uint64_t block_addr)
{
    csi_nand_mem_layout_t *pMemOrg = &spinand->chip_info->memorg;
    uint64_t temp_addr ;
    uint16_t   marker = 0;
	int32_t    ret;
    temp_addr = block_addr & (~(pMemOrg->pagesize*pMemOrg->pages_per_eraseblock-1));
    ret = csi_spinand_write_spare_data(spinand,temp_addr,0,&marker,2);

	return (ret==2)?CSI_OK:CSI_ERROR;
}

static csi_error_t csi_spinand_erase_one_block(csi_spinand_t *spinand, csi_nand_pos_t *pos)
{
    csi_error_t ret;
    uint8_t     status = 0;

    ret = csi_spinand_select_target(spinand,pos->target);
    if(ret){
        return ret;
    }

    ret = csi_spinand_write_enable_op(spinand);
    if(ret){
        return ret;
    }

    ret = csi_spinand_erase_block_op(spinand,pos);
    if(ret){
        return ret;
    }

    ret = csi_spinand_wait(spinand,&status);
    if(ret){
        return ret;
    }
   
    return (status & STATUS_ERASE_FAILED) ? CSI_ERROR:CSI_OK;
}

csi_error_t csi_spinand_erase(csi_spinand_t *spinand, uint64_t offset, uint64_t size,uint64_t *last_fail_addr)
{
    CSI_PARAM_CHK(spinand, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);
    csi_error_t ret;
    uint64_t temp_offset = offset;
    csi_nand_pos_t start;
    csi_nand_mem_layout_t *pMemOrg = &spinand->chip_info->memorg;
    uint32_t blk_size;
    
    blk_size = pMemOrg->pagesize*pMemOrg->pages_per_eraseblock;

    if( (offset & ( blk_size-1) ) || !size || size % blk_size){
        return CSI_ERROR;
    }

    while(temp_offset < offset + size){

        /* check whether block is bad ,if yes then ignore */

        /* erase one block */
        spinand_offset_to_nandpos(spinand,temp_offset,&start);
        ret = csi_spinand_erase_one_block(spinand,&start);
         if(ret){
             *last_fail_addr = temp_offset;
             return ret;
         }

        /* increment next block addr */
        temp_offset += blk_size;
    }

    return CSI_OK;
}

csi_error_t csi_spinand_dev_init(csi_spinand_t *spinand)
{
    /*  spinand device probe */
   uint32_t    i;
   csi_error_t ret;

    ret = csi_spinand_id_probe(spinand);
    if(ret){
        return ret;
    }
    /* quad enable */
    ret = csi_spinand_quad_enable(spinand,1);
    if(ret){
        return ret;
    }

    /*  spinand device specific init */
    if(spinand->maf->ops->init){
         spinand->maf->ops->init(spinand);
    }

    /* unblock all blocks */
    for(i = 0; i < spinand->chip_info->memorg.ntargets; i++)
    {
        ret = csi_spinand_select_target(spinand,i);
        if(ret)
        {
            return ret;
        }
        ret = csi_spinand_unlock_blocks(spinand);
        if(ret)
        {
            return ret;
        }
    }

    g_spinand_probe_dev_flag = 1;
    return ret;
}

csi_error_t csi_spinand_get_flash_info(csi_spinand_t *spinand, csi_spinand_dev_params_t *flash_info)
{
    csi_nand_mem_layout_t *pMemOrg = &spinand->chip_info->memorg;

    if(g_spinand_probe_dev_flag)
    {
        flash_info->max_bad_blocks = pMemOrg->max_bad_eraseblocks_per_lun * pMemOrg->ntargets * pMemOrg->luns_per_target;
        flash_info->model_name = spinand->chip_info->model;
        flash_info->oob_size  = pMemOrg->oobsize;
        flash_info->page_size = pMemOrg->pagesize;
        flash_info->pages_per_block = pMemOrg->pages_per_eraseblock;
        flash_info->total_blocks = pMemOrg->eraseblocks_per_lun * pMemOrg->luns_per_target * pMemOrg->ntargets;
        return CSI_OK;
    }
    else{
        return CSI_ERROR;
    }

}

uint32_t csi_spinand_frequence(csi_spinand_t *spinand, uint32_t hz)
{
    CSI_PARAM_CHK(spinand, 0U);
    spinand_printf(SPINAND_ERR, "Not support adjust frequency, to do function\n");

    return 0;//csi_qspi_frequence((csi_qspi_t*)(&spinand->spi_qspi.qspi), hz);
}


