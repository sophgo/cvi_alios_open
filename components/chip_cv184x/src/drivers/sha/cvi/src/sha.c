#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <csi_core.h>

#include "drv/common.h"
#include "csi_crypto.h"
#include "mmio.h"
#include "drv/sha.h"
#include "drv/tick.h"

static uint8_t is_little_endian()
{
    static uint32_t _endian_x_ = 1;
    return ((const uint8_t *)(& _endian_x_))[0];
}

static uint8_t  is_big_endian()
{
    return !is_little_endian();
}
//reverse byte order
static  uint32_t reverse_32bit(uint32_t data)
{
    data = (data >> 16) | (data << 16);
    return ((data & 0xff00ff00UL) >> 8) | ((data & 0x00ff00ffUL) << 8);
}

//host byte order to big endian
static uint32_t os_htobe32(uint32_t data)
{
    if (is_big_endian()) {
        return data;
    }
    return reverse_32bit(data);
}

static inline uint64_t reverse_64bit(uint64_t data)
{
    data = (data >> 32) | (data << 32);
    data = ((data & 0xff00ff00ff00ff00ULL) >> 8) | ((data & 0x00ff00ff00ff00ffULL) << 8);

    return ((data & 0xffff0000ffff0000ULL) >> 16) | ((data & 0x0000ffff0000ffffULL) << 16);
}

//host to big endian
static uint64_t os_htobe64(uint64_t data)
{
    if (is_big_endian()) {
        return data;
    }

    return reverse_64bit(data);
}

static csi_error_t _csi_sha_block_process(csi_sha_t *sha, csi_sha_context_t *context, const void *input, uint32_t size)
{
  if (size != 64) return CSI_ERROR;

  __aligned(64) uint32_t dma_descriptor[32] = { 0 };
  __aligned(64) uint8_t data[64] = { 0 };

  uint64_t src = (uintptr_t)data;
  uint32_t status, result_word = 0;
  int i;

  memcpy(data, input, 64);

  if (SHA_MODE_256 == context->mode) {
    result_word = 8;
  } else if (SHA_MODE_1 == context->mode) {
    result_word = 5;
  }

  // Prepare descriptor
  dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY | DES_USE_DESCRIPTOR_IV | DES_USE_SHA | 0xF;
  if (context->mode == SHA_MODE_256) {
    dma_descriptor[CRYPTODMA_CIPHER] = SHA256 | SHA_LOAD_PARAM;
  } else if (context->mode == SHA_MODE_1) {
    dma_descriptor[CRYPTODMA_CIPHER] = SHA_LOAD_PARAM;
  }

  dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
  dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

  dma_descriptor[CRYPTODMA_DST_ADDR_L] = 0;
  dma_descriptor[CRYPTODMA_DST_ADDR_H] = 0;

  dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = 64;
  dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = 0;

  // Set cryptodma control
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
                  (uint32_t)((uintptr_t)dma_descriptor & 0xFFFFFFFF));
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
                  (uint32_t)((uintptr_t)dma_descriptor >> 32));

  for (i = 0; i < result_word; i++) {
    dma_descriptor[CRYPTODMA_KEY + i] = context->state[i];
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_SHA_PAR + i * 4, 0x12345678);
  }

  status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

  csi_dcache_clean_range((uint64_t *)dma_descriptor, sizeof(dma_descriptor));
  csi_dcache_clean_range((uint64_t *)data, 64);

  // Clear interrupt
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
  // Trigger cryptodma engine
  mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
                  DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

  uint32_t time_start = csi_tick_get_ms();
  do {
    status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
    if ((csi_tick_get_ms() - time_start) >= 5) {
      printf("sha timeout\n");
      return CSI_ERROR;
    }
  } while (status == 0);
  
  for (i = 0; i < result_word; i++) {
    uintptr_t a = SEC_CRYPTODMA_BASE + CRYPTODMA_SHA_PAR + i * 4;
    context->state[i] = mmio_read_32(a);
  }

  return CSI_OK;
}

/**
  \brief       Initialize SHA Interface. Initializes the resources needed for the SHA interface
  \param[in]   sha    Operate handle
  \param[in]   idx    Index of SHA
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sha_init(csi_sha_t *sha, uint32_t idx)
{
  CSI_PARAM_CHK(sha, CSI_ERROR);
  
  sha->dma_in = NULL;
  sha->state.busy = 0U;
  sha->state.error = 0U;

  return CSI_OK;
}

/**
  \brief       De-initialize SHA Interface. Stops operation and releases the software resources used by the interface
  \param[in]   sha    SHA handle to operate
  \return      None
*/
void csi_sha_uninit(csi_sha_t *sha)
{
  CSI_PARAM_CHK_NORETVAL(sha);

  if (sha->state.busy) {
    sha->state.error = 1U; 
  }
}

/**
  \brief       Attach the callback handler to SHA
  \param[in]   sha         Handle to operate
  \param[in]   callback    Callback function
  \param[in]   arg         Callback's param
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sha_attach_callback(csi_sha_t *sha, void *callback, void *arg)
{
  return CSI_UNSUPPORTED;
}

/**
  \brief       Detach the callback handler
  \param[in]   sha    Handle to operate
  \return      None
*/
void csi_sha_detach_callback(csi_sha_t *sha)
{

}

/**
  \brief       Start the engine
  \param[in]   sha        Handle to operate
  \param[in]   context    Pointer to the SHA context \ref csi_sha_context_t
  \param[in]   mode       SHA mode \ref csi_sha_mode_t
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sha_start(csi_sha_t *sha, csi_sha_context_t *context, csi_sha_mode_t mode)
{
  CSI_PARAM_CHK(sha, CSI_ERROR);
  CSI_PARAM_CHK(context, CSI_ERROR);

  if (SHA_MODE_256 != mode && SHA_MODE_1 != mode) return CSI_UNSUPPORTED;

  context->mode = mode;

  if (SHA_MODE_256 == context->mode) {
    context->state[0] = os_htobe32(0x6A09E667);
    context->state[1] = os_htobe32(0xBB67AE85);
    context->state[2] = os_htobe32(0x3C6EF372);
    context->state[3] = os_htobe32(0xA54FF53A);
    context->state[4] = os_htobe32(0x510E527F);
    context->state[5] = os_htobe32(0x9B05688C);
    context->state[6] = os_htobe32(0x1F83D9AB);
    context->state[7] = os_htobe32(0x5BE0CD19);
  } else if (SHA_MODE_1 == context->mode) {
    context->state[0] = os_htobe32(0x67452301);
    context->state[1] = os_htobe32(0xEFCDAB89);
    context->state[2] = os_htobe32(0x98BADCFE);
    context->state[3] = os_htobe32(0x10325476);
    context->state[4] = os_htobe32(0xC3D2E1F0);
  }

  context->total[0] = 0;

  return CSI_OK;
}

/**
  \brief       Update the engine
  \param[in]   sha        Handle to operate
  \param[in]   context    Pointer to the SHA context \ref csi_sha_context_t
  \param[in]   input      Pointer to the Source data
  \param[in]   size       The data size
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sha_update(csi_sha_t *sha, csi_sha_context_t *context, const void *input, uint32_t size)
{
  CSI_PARAM_CHK(sha, CSI_ERROR);
  CSI_PARAM_CHK(context, CSI_ERROR);
  CSI_PARAM_CHK(input, CSI_ERROR);
  if (size == 0) return CSI_ERROR;

  uint32_t left = context->total[0] & 0x3F;
  uint32_t total_size = left + size;
  uint8_t *p = (uint8_t *)input;

  sha->state.busy = 1U;
  while (total_size >= 64) {
    if (left > 0) {
      memcpy(context->buffer + left, p, (64 - left));
      p += (64 - left);
      left = 0;
    } else {
      memcpy(context->buffer, p, 64);
      p += 64;
    }

    _csi_sha_block_process(sha, context, context->buffer, 64);
    total_size -= 64;
  }

  if (total_size) {
    if (left > 0) {
      memcpy(context->buffer + left, p, size);
    } else {
      memcpy(context->buffer, p, total_size);
    }
  }

  context->total[0] += size;
  sha->state.busy = 0U;

  return CSI_OK;
}

/**
  \brief       Accumulate the engine (async mode)
  \param[in]   sha        Handle to operate
  \param[in]   context    Pointer to the SHA context \ref csi_sha_context_t
  \param[in]   input      Pointer to the Source data
  \param[in]   size       The data size
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sha_update_async(csi_sha_t *sha, csi_sha_context_t *context, const void *input, uint32_t size)
{
  return CSI_UNSUPPORTED;  
}

/**
  \brief       Finish the engine
  \param[in]   sha         Handle to operate
  \param[in]   context     Pointer to the SHA context \ref csi_sha_context_t
  \param[out]  output      Pointer to the result data
  \param[out]  out_size    Pointer to the result data size(bytes)
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sha_finish(csi_sha_t *sha, csi_sha_context_t *context, void *output, uint32_t *out_size)
{
  CSI_PARAM_CHK(sha, CSI_ERROR);
  CSI_PARAM_CHK(context, CSI_ERROR);
  CSI_PARAM_CHK(output, CSI_ERROR);

  uint32_t left = context->total[0] & 0x3F;
  int i;

  if (left >= 56) {
    sha->state.busy = 1U;
    uint64_t bits_len_be = os_htobe64(context->total[0] << 3);
    context->buffer[left] = 0x80;
    if ((left + 1) < 64) {
      memset(&context->buffer[left + 1], 0, (64 - 1 - left));
    }
    _csi_sha_block_process(sha, context, context->buffer, 64);
    memset(context->buffer, 0, 64);
    memcpy(&context->buffer[56], &bits_len_be, 8);
    _csi_sha_block_process(sha, context, context->buffer, 64);
    sha->state.busy = 0U;
  } else if (left > 0) {
    sha->state.busy = 1U;
    uint64_t bits_len_be = os_htobe64(context->total[0] << 3);
    context->buffer[left] = 0x80;
    if ((left + 1) < 56) {
      memset(&context->buffer[left + 1], 0, (56 - 1 - left));
    }
    memcpy(&context->buffer[56], &bits_len_be, 8);
    _csi_sha_block_process(sha, context, context->buffer, 64);
    sha->state.busy = 0U;
  } else if (left == 0) {
    sha->state.busy = 1U;
    memset(context->buffer, 0, 64);
    uint64_t bits_len_be = os_htobe64(context->total[0] << 3);
    context->buffer[0] = 0x80;
    memcpy(&context->buffer[56], &bits_len_be, 8);
    _csi_sha_block_process(sha, context, context->buffer, 64);
    sha->state.busy = 0U;
  }

  uint32_t result_word = 0;
  if (SHA_MODE_256 == context->mode) {
    result_word = 8;
  } else if (SHA_MODE_1 == context->mode) {
    result_word = 5;
  }

  for (i = 0; i < result_word; i++) {
    uint32_t state = context->state[i];
    memcpy(output + 4*i, &state, 4);
  }
  *out_size = (result_word << 2);

  return CSI_OK;
}

/**
  \brief       Get SHA state
  \param[in]   sha      Handle to operate
  \param[out]  state    SHA state \ref csi_sha_state_t
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sha_get_state(csi_sha_t *sha, csi_sha_state_t *state)
{
  CSI_PARAM_CHK(sha, CSI_ERROR);
  CSI_PARAM_CHK(state, CSI_ERROR);

  *state = sha->state;

  return CSI_OK;
}

/**
  \brief       Enable SHA power manage
  \param[in]   sha     Handle to operate
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sha_enable_pm(csi_sha_t *sha)
{
  return CSI_UNSUPPORTED;
}

/**
  \brief       Disable SHA power manage
  \param[in]   sha    Handle to operate
  \return      None
*/
void csi_sha_disable_pm(csi_sha_t *sha)
{

}
