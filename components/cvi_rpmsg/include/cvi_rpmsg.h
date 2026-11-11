/*
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Public API of the cvitek rpmsg platform adaptation (AliOS / C906L side).
 *
 * cvi_rpmsg is the REMOTE/DEVICE end of the virtio rpmsg link between the
 * Linux big core (A53, HOST) and the AliOS small core (C906L, REMOTE).  After
 * cvi_rpmsg_init() returns, the rpmsg bus is up and one or more endpoints can
 * be created with cvi_rpmsg_create_ept() for multi-channel communication.
 *
 * This header is the stable, dependency-free public surface: it intentionally
 * does not include or expose any open_amp / libmetal types.  Endpoint handles
 * are opaque.
 */
#ifndef CVI_RPMSG_H
#define CVI_RPMSG_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Address wildcard: auto-assign the local address (src) or discover the
 * remote address (dest) via the rpmsg name service. */
#define CVI_RPMSG_ADDR_ANY       (0xFFFFFFFFu)

/* Maximum number of simultaneously live endpoints. */
#define CVI_RPMSG_MAX_ENDPOINTS  8

/* Opaque endpoint handle.  Obtained from cvi_rpmsg_create_ept(), used with
 * cvi_rpmsg_send() / cvi_rpmsg_destroy_ept(). */
typedef struct cvi_rpmsg_ept cvi_rpmsg_ept_t;

/* Receive callback.  Invoked from the rpmsg worker task when a message
 * arrives at the endpoint.
 *   ept   : the endpoint that received the message
 *   data  : payload pointer (valid only for the duration of the callback)
 *   len   : payload length in bytes
 *   src   : remote source address (the sender) — reply with
 *           cvi_rpmsg_sendto(ept, data, len, src) so each sender gets its own
 *           response.  Do NOT rely on cvi_rpmsg_send() (ept->dest_addr): it
 *           tracks only one peer and breaks with multiple concurrent senders.
 *   priv  : user private pointer registered with the endpoint
 * Returns 0 on success.  Do NOT call cvi_rpmsg_destroy_ept() on @ept from
 * within this callback (the worker is still processing it). */
typedef int (*cvi_rpmsg_rx_cb_t)(cvi_rpmsg_ept_t *ept, const void *data,
                                 size_t len, uint32_t src, void *priv);

/* Name-service bind callback.  Invoked when the Linux host announces a
 * service the local side has not bound yet.  The typical reaction is to call
 *   cvi_rpmsg_create_ept(name, CVI_RPMSG_ADDR_ANY, remote_addr, cb, unbind_cb, priv)
 * to bind on demand.  May be NULL. */
typedef void (*cvi_rpmsg_ns_bind_cb_t)(const char *name, uint32_t remote_addr);

/* Name-service unbind callback.  Invoked when the Linux host destroys the
 * service this endpoint is bound to; the endpoint is about to be removed by
 * the rpmsg layer.  Use it to drop any user state tied to the channel.  May be
 * NULL.  Do NOT call cvi_rpmsg_destroy_ept() on @ept from here (it is already
 * being torn down); cvi_rpmsg_destroy_ept() can be called after this returns. */
typedef void (*cvi_rpmsg_ns_unbind_cb_t)(cvi_rpmsg_ept_t *ept);

/**
 * Initialize the cvitek rpmsg platform on the AliOS small core.  Performs, in
 * order: libmetal init, mailbox + ISR, resource table, remoteproc + vdev +
 * vring init, then blocks until the Linux host sets DRIVER_OK, and finally
 * brings up the rpmsg bus.
 *
 * @param ns_bind_cb  callback for host-announced services; may be NULL.
 * @return 0 on success, negative on failure.
 */
int  cvi_rpmsg_init(cvi_rpmsg_ns_bind_cb_t ns_bind_cb);

/** Deinitialize the platform, destroying all endpoints. */
void cvi_rpmsg_deinit(void);

/** Return non-zero if cvi_rpmsg_init() has completed successfully. */
int  cvi_rpmsg_is_ready(void);

/**
 * Create an rpmsg endpoint bound to a name service.  Multiple endpoints
 * (different names) can coexist for multi-channel communication, up to
 * CVI_RPMSG_MAX_ENDPOINTS.
 *
 * @param name      service name, e.g. "rpmsg-echo-test".
 * @param src       local address; CVI_RPMSG_ADDR_ANY to auto-assign.
 * @param dest      remote address; CVI_RPMSG_ADDR_ANY to discover via NS announce.
 * @param cb        receive callback (must not be NULL).
 * @param unbind_cb called when the remote destroys this service; may be NULL.
 * @param priv      user pointer passed back to @cb.
 * @return endpoint handle, or NULL on failure / no free slot.
 */
cvi_rpmsg_ept_t *cvi_rpmsg_create_ept(const char *name, uint32_t src,
                                       uint32_t dest, cvi_rpmsg_rx_cb_t cb,
                                       cvi_rpmsg_ns_unbind_cb_t unbind_cb,
                                       void *priv);

/** Destroy an endpoint returned by cvi_rpmsg_create_ept().  Safe on NULL. */
void cvi_rpmsg_destroy_ept(cvi_rpmsg_ept_t *ept);

/**
 * Send a message on an endpoint (blocking until queued; 15s timeout).
 * @return number of bytes sent, or negative on failure.
 */
int  cvi_rpmsg_send(cvi_rpmsg_ept_t *ept, const void *data, int len);

/**
 * Send a message to an explicit remote address (blocking; uses the
 * endpoint's source address).
 * @return number of bytes sent, or negative on failure.
 */
int  cvi_rpmsg_sendto(cvi_rpmsg_ept_t *ept, const void *data, int len,
                       uint32_t dst);

/**
 * Non-blocking send: returns immediately with -ENOMEM if no TX buffer is
 * available.  @return number of bytes sent, or negative on failure.
 */
int  cvi_rpmsg_trysend(cvi_rpmsg_ept_t *ept, const void *data, int len);

/** Non-blocking send to an explicit remote address. */
int  cvi_rpmsg_trysendto(cvi_rpmsg_ept_t *ept, const void *data, int len,
                          uint32_t dst);

/* ---- Zero-copy send --------------------------------------------------------
 * Get a TX payload buffer, fill it directly, then send it without an
 * intermediate copy.  Useful for large transfers.  After a successful
 * send_nocopy/sendto_nocopy the buffer is no longer owned by the caller. */

/**
 * Get a TX payload buffer to fill directly (process context only, NOT from
 * the rx callback).
 * @param len   in: desired size hint (0 = default); out: actual buffer size.
 * @param wait  block until a buffer is available.
 * @return buffer pointer, or NULL on failure.  Fill it and pass to
 *         cvi_rpmsg_send_nocopy() / cvi_rpmsg_sendto_nocopy().
 */
void *cvi_rpmsg_get_tx_payload_buffer(cvi_rpmsg_ept_t *ept, uint32_t *len,
                                       int wait);

/** Send a TX buffer from cvi_rpmsg_get_tx_payload_buffer() (blocking). */
int  cvi_rpmsg_send_nocopy(cvi_rpmsg_ept_t *ept, const void *data, int len);

/** Send a TX buffer to an explicit remote address (zero-copy). */
int  cvi_rpmsg_sendto_nocopy(cvi_rpmsg_ept_t *ept, const void *data,
                              int len, uint32_t dst);

/* ---- Deferred RX processing ------------------------------------------------
 * Instead of copying the payload in the rx callback, hold the buffer and
 * process it later (process context), then release it back to the vring. */

/** Hold an RX buffer past the callback (rx-callback context only). */
void cvi_rpmsg_hold_rx_buffer(cvi_rpmsg_ept_t *ept, void *rxbuf);

/** Release an RX buffer held with cvi_rpmsg_hold_rx_buffer(). */
void cvi_rpmsg_release_rx_buffer(cvi_rpmsg_ept_t *ept, void *rxbuf);

/** Return non-zero if the endpoint is bound and ready to send/receive. */
int  cvi_rpmsg_ept_ready(cvi_rpmsg_ept_t *ept);

#ifdef __cplusplus
}
#endif

#endif /* CVI_RPMSG_H */
