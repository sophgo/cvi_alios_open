/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     qspi_device.c
 * @brief
 * @version
 * @date     2020-03-18
 ******************************************************************************/
#include <string.h>
#include <drv/spinand.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>
#include "spinand_vendor.h"
#include <drv/qspi.h>

/* communication parameters */
#define DEFUALT_QSPI_DEVICE_TX_RX_TIMEOUT        0x100000U
#define DEFUALT_QSPI_DEVICE_PHASE                SPI_FORMAT_CPOL0_CPHA0
#define DEFUALT_QSPI_DEVICE_SALVE_IDX            0U
#define MAX_TRANSFER_SIZE_ONCE                   (256-2) ///< max transfer 254 uint32 
#define QSPI_DMA_XFER
#define ATTRIBUTE_DATA

#ifdef QSPI_DMA_XFER
typedef enum{
    XFER_STAT_START,
    XFER_STAT_COMPLTE_DONE,
    XFER_STAT_ERROR,
    XFER_STAT_UNKNOW,
}qspi_xfer_async_stat_t;

csi_dma_ch_t                  g_qspi_rx_dma;
csi_dma_ch_t                  g_qspi_tx_dma;
static volatile qspi_xfer_async_stat_t g_qspi_xfrer_stat = 0;
#endif

static csi_qspi_command_t command;
extern csi_error_t csi_spinand_dev_init(csi_spinand_t *spinand);

#ifdef QSPI_DMA_XFER
static void qspi_mem_transfer_callback(csi_qspi_t *qspi,csi_qspi_event_t event, void *arg)
{
    switch (event)
    {
    case QSPI_EVENT_COMMAND_COMPLETE:
        g_qspi_xfrer_stat = XFER_STAT_COMPLTE_DONE;
        break;
    case QSPI_EVENT_ERROR:
        g_qspi_xfrer_stat = XFER_STAT_ERROR;
        break;
    default:
        g_qspi_xfrer_stat = XFER_STAT_UNKNOW;
        break;
    }
    return;
}
#endif

static void csi_spi_mem_op2_qspi_cmd(spi_mem_op_t* op, csi_qspi_command_t *command)
{
    /* cmd portion */
    command->instruction.disabled = false;
    command->instruction.bus_width = QSPI_CFG_BUS_SINGLE;
    command->instruction.value    = op->cmd.opcode;

    /* addr portion */
    command->address.disabled = (op->addr.nbytes)?false:true;
    command->address.size     = op->addr.nbytes;
    command->address.value    = op->addr.val;
    command->address.bus_width= QSPI_CFG_BUS_SINGLE;

    /* altnative portion == disabled */       
    command->alt.disabled     = true;
    command->ddr_enable       = false;

    /* dummy portion */
    command->dummy_count      = op->dummy.nbytes;

    /* data portion */
    switch (op->data.buswidth)
    {
    case 1:
        command->data.bus_width   = QSPI_CFG_BUS_SINGLE;
        break;
    case 2:
        command->data.bus_width   = QSPI_CFG_BUS_DUAL;
        break;
    case 4:
        command->data.bus_width   = QSPI_CFG_BUS_QUAD;
        break;
            
    default:
        break;
    }
    
    return;
}
#ifdef QSPI_DMA_XFER
static csi_error_t   csi_spi_mem_dma(void *spinand, spi_mem_op_t *op)
{
    int32_t ret;    
    csi_spinand_t *nand = (csi_spinand_t*)spinand;
    csi_qspi_command_t command = {0};

    csi_spi_mem_op2_qspi_cmd(op,&command);    

    if(op->data.dir == SPI_MEM_NODATA || op->data.dir == SPI_MEM_DATA_OUT)
    {
        if(nand->spi_cs_callback != NULL){
            nand->spi_cs_callback(0);
        }
        g_qspi_xfrer_stat = XFER_STAT_START;
        ret = csi_qspi_send_async(&nand->spi_qspi.qspi,&command,op->data.buf.out,op->data.nbytes);
        if (ret)
        {
		  udelay(50);
          nand->spi_cs_callback(1);
          return ret; 
        }
        //todo timeout dispatch
        while(!g_qspi_xfrer_stat);
        if(nand->spi_cs_callback != NULL){
		  udelay(50);
          nand->spi_cs_callback(1);
        }
    }
    else if(op->data.dir == SPI_MEM_DATA_IN)
    {
        if(nand->spi_cs_callback != NULL){
            nand->spi_cs_callback(0);
        }
        g_qspi_xfrer_stat = XFER_STAT_START;
        ret = csi_qspi_receive_async(&nand->spi_qspi.qspi,&command,op->data.buf.in,op->data.nbytes);
        if (ret)
        {
          nand->spi_cs_callback(1);
          return ret; 
        }
        // to do timeout dispatch
        while(!g_qspi_xfrer_stat);
        if(nand->spi_cs_callback != NULL){
            nand->spi_cs_callback(1);
        }
    }
    else
    {
        return CSI_ERROR;
    }

    return ret < 0? CSI_ERROR:CSI_OK;
}
#else
static csi_error_t csi_spi_mem_polling(void *spinand, spi_mem_op_t *op)
{
    int32_t ret;    
    csi_spinand_t *nand = (csi_spinand_t*)spinand;
    csi_qspi_command_t command = {0};

    csi_spi_mem_op2_qspi_cmd(op,&command);    

    if(op->data.dir == SPI_MEM_NODATA || op->data.dir == SPI_MEM_DATA_OUT)
    {
        if(nand->spi_cs_callback != NULL){
            nand->spi_cs_callback(0);
        }
        ret = csi_qspi_send(&nand->spi_qspi.qspi,&command,op->data.buf.out,op->data.nbytes,SPIANND_DEF_MAX_WAIT_TIME);
        if(nand->spi_cs_callback != NULL){
            nand->spi_cs_callback(1);
        }
    }
    else if(op->data.dir == SPI_MEM_DATA_IN)
    {
        if(nand->spi_cs_callback != NULL){
            nand->spi_cs_callback(0);
        }
        ret = csi_qspi_receive(&nand->spi_qspi.qspi,&command,op->data.buf.in,op->data.nbytes,SPIANND_DEF_MAX_WAIT_TIME);
        if(nand->spi_cs_callback != NULL){
            nand->spi_cs_callback(1);
        }
    }
    else
    {
        return CSI_ERROR;
    }

    return ret < 0? CSI_ERROR:CSI_OK;
}
#endif
csi_error_t csi_spinand_qspi_init(csi_spinand_t *spinand, uint32_t qspi_idx,void *gpio_cs_callback)
{
    CSI_PARAM_CHK(spinand, CSI_ERROR);

    csi_error_t ret = CSI_OK;
 
    spinand->spi_cs_callback = gpio_cs_callback;
	/* max op size = (qspi fifo depth) - 1 entry(cmd) - 1 entry(addr)  */
	spinand->max_op_size     = MAX_TRANSFER_SIZE_ONCE<<2;

    /* select transfer (polling, interrupt,dma) */
    #ifdef QSPI_DMA_XFER
    spinand->spi_mem = csi_spi_mem_dma;
    #else
    spinand->spi_mem = csi_spi_mem_polling;
    #endif

    memset(&command, (int)0, sizeof(csi_qspi_command_t));

    /* qspi controler init */
    ret = csi_qspi_init(&spinand->spi_qspi.qspi, qspi_idx);
    if (ret != CSI_OK) {
        return ret;
    }

    #ifdef QSPI_DMA_XFER
    /* register async-callback */
    ret = csi_qspi_attach_callback(&spinand->spi_qspi.qspi,qspi_mem_transfer_callback,NULL);
    if(ret){
        return ret;
    } 

    /*  link dma channel */
    ret = csi_qspi_link_dma(&spinand->spi_qspi.qspi,&g_qspi_tx_dma,&g_qspi_rx_dma);
    if(ret){
        return ret;
    }
    #endif
    /*init qspi cs state == high*/
    if(spinand->spi_cs_callback){
        spinand->spi_cs_callback(1);
    }

    /* set spi mode 0 */
    ret = csi_qspi_mode(&spinand->spi_qspi.qspi,QSPI_CLOCK_MODE_0);
    if(ret){
        return ret;
    }  

    ret = csi_qspi_frequence(&spinand->spi_qspi.qspi,SPINAND_DEF_SPEED);
    if(!ret){
        return ret;
    }

    ret = csi_spinand_dev_init(spinand);
    return ret;
}

void csi_spinand_qspi_uninit(csi_spinand_t *spinand)
{
    CSI_PARAM_CHK_NORETVAL(spinand);
    /* manufacture uninit */
    if(spinand->maf->ops->uninit){
        spinand->maf->ops->uninit(spinand);
    }
    csi_qspi_uninit(&spinand->spi_qspi.qspi);
}
