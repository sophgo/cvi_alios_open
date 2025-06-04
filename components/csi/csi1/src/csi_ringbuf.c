#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <csi_core.h>
#include "drv/ringbuf.h"

#define min(a, b)  (((a) < (b)) ? (a) : (b))

/**
  * \brief  Removes the entire FIFO contents.
  * \param  [in] fifo: The fifo to be emptied.
  * \return None.
  */
void csi_ringbuf_reset(csi_ringbuf_t *fifo)
{
    uint32_t stat = csi_irq_save();
    fifo->write = fifo->read = 0;
    fifo->data_len = 0;
    csi_irq_restore(stat);
}

/**
  * \brief  Returns the size of the FIFO in bytes.
  * \param  [in] fifo: The fifo to be used.
  * \return The size of the FIFO.
  */
static inline uint32_t csi_ringbuf_size(csi_ringbuf_t *fifo)
{
    return fifo->size;
}

/**
  * \brief  Returns the number of used bytes in the FIFO.
  * \param  [in] fifo: The fifo to be used.
  * \return The number of used bytes.
  */
uint32_t csi_ringbuf_len(csi_ringbuf_t *fifo)
{
    return fifo->data_len;
}

/**
  * \brief  Returns the number of bytes available in the FIFO.
  * \param  [in] fifo: The fifo to be used.
  * \return The number of bytes available.
  */
uint32_t csi_ringbuf_avail(csi_ringbuf_t *fifo)
{
    return csi_ringbuf_size(fifo) - csi_ringbuf_len(fifo);
}

/**
  * \brief  Is the FIFO empty?
  * \param  [in] fifo: The fifo to be used.
  * \retval true:      Yes.
  * \retval false:     No.
  */
bool csi_ringbuf_is_empty(csi_ringbuf_t *fifo)
{
    return csi_ringbuf_len(fifo) == 0;
}

/**
  * \brief  Is the FIFO full?
  * \param  [in] fifo: The fifo to be used.
  * \retval true:      Yes.
  * \retval false:     No.
  */
bool csi_ringbuf_is_full(csi_ringbuf_t *fifo)
{
    return csi_ringbuf_avail(fifo) == 0;
}

/**
  * \brief  Puts some data into the FIFO.
  * \param  [in] fifo: The fifo to be used.
  * \param  [in] in:   The data to be added.
  * \param  [in] len:  The length of the data to be added.
  * \return The number of bytes copied.
  * \note   This function copies at most @len bytes from the @in into
  *         the FIFO depending on the free space, and returns the number
  *         of bytes copied.
  */
uint32_t csi_ringbuf_in(csi_ringbuf_t *fifo, const void *datptr, uint32_t len)
{
    uint32_t writelen = 0, tmplen = 0;

    if(csi_ringbuf_is_full(fifo))
        return 0;

    tmplen = fifo->size - fifo->data_len;
    writelen = tmplen > len ? len : tmplen;

    if(fifo->write < fifo->read) {
        memcpy((void*)&fifo->buffer[fifo->write], (void*)datptr, writelen);
    } else {
        tmplen = fifo->size - fifo->write;
        if(writelen <= tmplen) {
            memcpy((void*)&fifo->buffer[fifo->write], (void*)datptr, writelen);
        } else {
            memcpy((void*)&fifo->buffer[fifo->write], (void*)datptr, tmplen);
            memcpy((void*)fifo->buffer, (uint8_t*)datptr + tmplen, writelen - tmplen);
        }
    }

    uint32_t stat = csi_irq_save();
    fifo->write = (fifo->write + writelen) % fifo->size;
    fifo->data_len += writelen;
    csi_irq_restore(stat);

    return writelen;
}

/**
  * \brief  Gets some data from the FIFO.
  * \param  [in] fifo: The fifo to be used.
  * \param  [in] out:  Where the data must be copied.
  * \param  [in] len:  The size of the destination buffer.
  * \return The number of copied bytes.
  * \note   This function copies at most @len bytes from the FIFO into
  *         the @out and returns the number of copied bytes.
  */
uint32_t csi_ringbuf_out(csi_ringbuf_t *fifo, void *outbuf, uint32_t len)
{
    uint32_t readlen = 0, tmplen = 0;
    if(csi_ringbuf_is_empty(fifo))
        return 0;

    uint32_t data_len = fifo->data_len;
    readlen = len > data_len ? data_len : len;
    tmplen = fifo->size - fifo->read;

    if(NULL != outbuf) {
        if(readlen <= tmplen) {
            memcpy((void*)outbuf, (void*)&fifo->buffer[fifo->read], readlen);
        } else {
            memcpy((void*)outbuf,(void*)&fifo->buffer[fifo->read], tmplen);
            memcpy((uint8_t*)outbuf + tmplen,(void*)fifo->buffer,readlen - tmplen);
        }
    }

    uint32_t stat = csi_irq_save();
    fifo->read = (fifo->read + readlen) % fifo->size;
    fifo->data_len -= readlen;
    csi_irq_restore(stat);

    return readlen;
}

/**
  * \brief  Move FIFO buffer to another FIFO.
  * \param  [in] fifo_in: The fifo to be used.
  * \param  [in] fifo_out: The fifo to be used.
  * \return The number of copied bytes.
  * \note   This function copies at most @len bytes from the FIFO into
  *         the @out and returns the number of copied bytes.
  */
uint32_t csi_ringbuf_move(csi_ringbuf_t *fifo_in, csi_ringbuf_t *fifo_out)
{
    uint32_t readlen = 0, tmplen_out = 0;
    if(csi_ringbuf_is_empty(fifo_out))
        return 0;

    int len = csi_ringbuf_avail(fifo_in);

    uint32_t data_len = fifo_out->data_len;
    readlen = len > data_len ? data_len : len;
    tmplen_out = fifo_out->size - fifo_out->read;

    if(readlen <= tmplen_out) {
        csi_ringbuf_in(fifo_in, (void*)&fifo_out->buffer[fifo_out->read], readlen);
    } else {
        csi_ringbuf_in(fifo_in, (void*)&fifo_out->buffer[fifo_out->read], tmplen_out);
        csi_ringbuf_in(fifo_in, (void*)fifo_out->buffer, readlen - tmplen_out);
    }

    uint32_t stat = csi_irq_save();
    fifo_out->read = (fifo_out->read + readlen) % fifo_out->size;
    fifo_out->data_len -= readlen;
    csi_irq_restore(stat);

    return readlen;
}

