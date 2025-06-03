/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

#include <string.h>
#include <soc.h>
#include <drv/mbox.h>
#include <drv/cvi_irq.h>
#include <drv/porting.h>
#include <wj_mbox_ll.h>

static void wj_mbox_irq_handler(void *arg)
{
        CSI_PARAM_CHK_NORETVAL(arg);
        csi_mbox_t *mbox = (csi_mbox_t *)arg;
        uint8_t i, id_map;
        struct e902_mbox_priv *priv = &e902priv;
        uint32_t sta;
        uint32_t info0_data;
        sta = e902_mbox_read(priv,E902_MBOX_STA);
        while(sta)
        {
            for(i = 0;i < (E902_MBOX_CHANS -1);i++)
            {
                if(!(sta & BIT(i)))
                    continue;
                /*clear chan irq bit in STA register*/
                e902_mbox_rmw(priv, E902_MBOX_CLR,BIT(i),0);

                if (i == E902_MBOX_ICU_CPU2)
                   id_map = i + 1;
		else
                   id_map = i;

                /*info0 is the protocol word, should not be zero!*/
                info0_data = e902_mbox_chan_read(priv,id_map,E902_MBOX_INFO0,false);
                if(info0_data){
                    if(mbox->callback){
                        mbox->callback(mbox,MBOX_EVENT_RECEIVED,id_map,0,mbox->arg);
                    }
                }

                if(!info0_data){
                    printf("not expected chan[%d] interrupt\n",i);
                    if(mbox->callback){
                        mbox->callback(mbox,MBOX_EVENT_ERROR,id_map,0,mbox->arg);
                    }
                }
            }
            sta = e902_mbox_read(priv,E902_MBOX_STA);
        }
}

/**
  \brief       Initialize MAILBOX Interface. 1. Initializes the resources needed for the MAILBOX interface 2.registers event callback function
  \param[in]   mbox    operate handle
  \param[in]   idx     the device idx
  \return      error code
*/
csi_error_t csi_mbox_init(csi_mbox_t *mbox, uint32_t idx)
{
    uint32_t data[8] = {0};
    int ret = 0;
    uint8_t i;
    uint8_t bitmap = 0;
    struct e902_mbox_priv *priv = &e902priv;
    for(i = 0;i < E902_MBOX_CHANS;i++)
    {
        if(i == priv->cur_icu_cpu_id)
            continue;

        /*clear local and remote generate and info0~info7*/
        e902_mbox_chan_rmw(priv,i,E902_MBOX_GEN,0x0,0xff,true);
        e902_mbox_chan_rmw(priv,i,E902_MBOX_GEN,0x0,0xff,false);
        e902_mbox_chan_wr_ack(priv,i,&data[7],true);
        e902_mbox_chan_wr_data(priv,i,&data[0],false);

        /*enable the chan mask*/
        e902_mbox_rmw(priv,E902_MBOX_MASK,BIT(bitmap),0);
        bitmap++;
    }

    ret = target_get(DEV_WJ_MBOX_TAG, idx, &mbox->dev);
    return ret;
}

/**
  \brief       De-initialize MAILBOX Interface. stops operation and releases the software resources used by the interface
  \param[in]   mbox    operate handle
*/
void csi_mbox_uninit(csi_mbox_t *mbox)
{
    CSI_PARAM_CHK_NORETVAL(mbox);
    uint8_t i;
    /*clear the chan mask*/
    struct e902_mbox_priv *priv = &e902priv;
    for(i = 0;i < E902_MBOX_CHANS-1;i++)
    {
        e902_mbox_rmw(priv,E902_MBOX_MASK,0,BIT(i));
    }

    csi_irq_detach((uint32_t)mbox->dev.irq_num);
    csi_irq_disable((uint32_t)mbox->dev.irq_num);
}

/**
  \brief       Start sending data to MAILBOX transmitter,(received data is ignored).
               This function is non-blocking,\ref mailbox_event_e is signaled when operation completes or error happens.
               \ref csi_mailbox_get_status can get operation status.
  \param[in]   mbox       operate handle
  \param[in]   channel_id index of channel
  \param[in]   data       Pointer to buffer with data to send to MAILBOX transmitter.
  \param[in]   size       size of data items to send
  \return      sent number of data
*/
int32_t csi_mbox_send(csi_mbox_t *mbox, uint32_t channel_id, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    struct e902_mbox_priv *priv = &e902priv;
    uint32_t ack_magic;

    e902_mbox_chan_wr_data(priv,channel_id,data,true);
    e902_mbox_chan_rmw(priv,channel_id,E902_MBOX_GEN,E902_MBOX_GEN_TX_DATA,0,true);

    /* wait magic ack */
    do {
         ack_magic = e902_mbox_chan_read(priv,channel_id,E902_MBOX_INFO7,false);

         /* TBD: add timeout process */
    } while (ack_magic != E902_MBOX_ACK_MAGIC);

    return 0;
}

/**
  \brief       Start Receiving data from Mailbox receiver.
  \param[in]   mbox   operate handle.
  \param[in]   channel_id index of channel
  \param[out]  data   Pointer to buffer with data to receive from mailbox.
  \param[in]   size   Number of data items to receive
  \return      received number or  error code
*/
int32_t csi_mbox_receive(csi_mbox_t *mbox, uint32_t channel_id, void *data, uint32_t size)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    uint32_t ack_magic = E902_MBOX_ACK_MAGIC;
    struct e902_mbox_priv *priv = &e902priv;
    /*read info0~info6 data*/
    e902_mbox_chan_rd_data(priv,channel_id,data,false);
    /*clear local info0 */
    e902_mbox_chan_write(priv,channel_id,0x0,E902_MBOX_INFO0,false);

    /*notify remote cpu*/
    e902_mbox_chan_wr_ack(priv,channel_id,&ack_magic,true);
    e902_mbox_chan_rmw(priv,channel_id,E902_MBOX_GEN,E902_MBOX_GEN_TX_ACK,0,true);

    return 0;
}

/**
\brief       Attach callback to the mbox
\param[in]   mbox    operate handle.
\param[in]   cb      event callback function \ref csi_mbox_callback_t
\param[in]   arg     user private param  for event callback
\return      error code
*/
csi_error_t csi_mbox_attach_callback(csi_mbox_t *mbox, void *callback, void *arg)
{
    CSI_PARAM_CHK(mbox, CSI_ERROR);
    CSI_PARAM_CHK(callback, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    mbox->callback = callback;
    mbox->arg = arg;
    csi_irq_attach((uint32_t)mbox->dev.irq_num, &wj_mbox_irq_handler, &mbox->dev);
    csi_irq_enable((uint32_t)mbox->dev.irq_num);
    return ret;
}

/**
\brief       detach callback from the mbox
\param[in]   mbox   operate handle.
*/
void csi_mbox_detach_callback(csi_mbox_t *mbox)
{
    CSI_PARAM_CHK_NORETVAL(mbox);
    mbox->callback  = NULL;
    mbox->arg = NULL;
    csi_irq_detach((uint32_t)mbox->dev.irq_num);
    csi_irq_disable((uint32_t)mbox->dev.irq_num);
}
