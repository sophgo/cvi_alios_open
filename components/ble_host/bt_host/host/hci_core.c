/* hci_core.c - HCI core Bluetooth handling */

/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ble_os.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <atomic.h>
#include <misc/util.h>
#include <misc/slist.h>
#include <misc/byteorder.h>
#include <misc/stack.h>
#include <misc/__assert.h>

#include <aos/bt.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_vs.h>
#include <bluetooth/hci_driver.h>

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BT_DEBUG_HCI_CORE)
#define LOG_MODULE_NAME bt_hci_core
#include "common/log.h"
#include <common/common.h>
#include "common/rpa.h"
#include "keys.h"
#include "monitor.h"

#include "hci_core.h"
#include "hci_ecc.h"
#include "ecc.h"

#include "conn_internal.h"
#include "l2cap_internal.h"
#include "gatt_internal.h"
#include "smp.h"
#include "bluetooth/crypto.h"
#include "settings.h"
#include <hci_api.h>

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
#include "fsm.h"
#endif

#if !(defined(CONFIG_BT_EXT_ADV_LEGACY_SUPPORT) && CONFIG_BT_EXT_ADV_LEGACY_SUPPORT)
#undef BT_FEAT_LE_EXT_ADV
#define BT_FEAT_LE_EXT_ADV(feat)  1
#endif

#ifndef __WEAK
#define __WEAK __attribute__((weak))
#endif

/* Peripheral timeout to initialize Connection Parameter Update procedure */
#define CONN_UPDATE_TIMEOUT  K_MSEC(CONFIG_BT_CONN_PARAM_UPDATE_TIMEOUT)
#define RPA_TIMEOUT_MS       (CONFIG_BT_RPA_TIMEOUT * MSEC_PER_SEC)
#define RPA_TIMEOUT          K_MSEC(RPA_TIMEOUT_MS)
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
#define HCI_CMD_TIMEOUT      K_SECONDS(10)
#endif
/* Stacks for the threads */
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
#if !(defined(CONFIG_BT_RECV_IS_RX_THREAD) && CONFIG_BT_RECV_IS_RX_THREAD)
static struct k_thread rx_thread_data;
static BT_STACK_NOINIT(rx_thread_stack, CONFIG_BT_RX_STACK_SIZE);
#endif
#endif
static struct k_thread tx_thread_data;
static BT_STACK_NOINIT(tx_thread_stack, CONFIG_BT_HCI_TX_STACK_SIZE);
static void init_work(struct k_work *work);

struct bt_dev bt_dev = {0};

static bt_ready_cb_t ready_cb;

static bt_le_scan_cb_t *scan_dev_found_cb;

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
static int set_le_scan_enable(u8_t enable);
static sys_slist_t scan_cbs = SYS_SLIST_STATIC_INIT(&scan_cbs);
#endif /* (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) */

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
static struct bt_le_ext_adv adv_pool[CONFIG_BT_EXT_ADV_MAX_ADV_SET];

#if (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC)
static struct bt_le_per_adv_sync *get_pending_per_adv_sync(void);
static struct bt_le_per_adv_sync per_adv_sync_pool[CONFIG_BT_PER_ADV_SYNC_MAX];
static sys_slist_t pa_sync_cbs = SYS_SLIST_STATIC_INIT(&pa_sync_cbs);
#endif /* (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC) */
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */

#if (defined(CONFIG_BT_HCI_VS_EVT_USER) && CONFIG_BT_HCI_VS_EVT_USER)
static bt_hci_vnd_evt_cb_t *hci_vnd_evt_cb;
#endif /* CONFIG_BT_HCI_VS_EVT_USER */

#if (defined(CONFIG_BT_ECC) && CONFIG_BT_ECC)
static u8_t pub_key[64];
static struct bt_pub_key_cb *pub_key_cb;
static bt_dh_key_cb_t dh_key_cb;
#endif /* CONFIG_BT_ECC */

#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
static bt_br_discovery_cb_t *discovery_cb;
struct bt_br_discovery_result *discovery_results;
static size_t discovery_results_size;
static size_t discovery_results_count;
#endif /* CONFIG_BT_BREDR */

struct bt_ad {
	const struct bt_data *data;
	size_t len;
};
static int set_data_add_complete(uint8_t *set_data, uint8_t set_data_len_max,
			const struct bt_ad *ad, size_t ad_len, uint8_t *data_len);

struct cmd_state_set {
	atomic_t *target;
	int bit;
	bool val;
};

void cmd_state_set_init(struct cmd_state_set *state, atomic_t *target, int bit,
			bool val)
{
	state->target = target;
	state->bit = bit;
	state->val = val;
}

extern void ble_event_init_done(int16_t err);
extern void ble_event_adv_start(int16_t err);
extern void ble_event_adv_stop(int16_t err);
extern void ble_event_scan_start(int16_t err);
extern void ble_event_scan_stop(int16_t err);

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
int bt_fsm_hci_cmd_send(void *msg, void *args, bt_fsm_handle_t src_handle, bt_fsm_ev_en ev);
static int hci_common_init(void);
static int hci_common_init_complete(u8_t status, u16_t opcode, struct net_buf *rsp);
static int hci_le_init();
static int hci_le_init_complete(u8_t status, u16_t opcode, struct net_buf *rsp);
static int handle_hci_cmd_done_event(struct net_buf *buf, u16_t opcode, u8_t status);
static int bt_setup_public_id_addr_cb(u16_t opcode, u8_t status, struct net_buf *rsp, void *args);
static int hci_le_init_read_random_address_complete(u16_t opcode, u8_t status, struct net_buf *rsp, void *args);
static int hci_cmd_le_set_private_addr_create(u8_t id, struct net_buf **ret_buf);
int bt_hci_cmd_send_cb(u16_t opcode, struct net_buf *buf, bt_hci_cmd_cb_t *cb);
static void adv_delete_legacy(void);
static int hci_le_init_read_random_address_complete_default_handler(u16_t opcode, u8_t status, struct net_buf *rsp, void *args);
static int hci_le_set_ext_adv_param_default_handler(u16_t opcode, u8_t status, struct net_buf *rsp, void *args);
int bt_fsm_le_scan_update(bool fast_scan, bt_fsm_handle_t fsm);
static void hci_cmd_sent_work(struct k_work *work);

int bt_le_create_conn_ext(const struct bt_conn *conn);
int bt_le_create_conn_legacy(const struct bt_conn *conn);
int hci_cmd_le_read_max_data_len(struct bt_conn *conn);
extern void hci_drvier_rx_process();
extern void k_poll_signal_data_recv(struct k_poll_signal *signal, int result);
int bt_fsm_hci_core_init();
int hci_cmd_fsm_init();
int bt_le_adv_start_ext(struct bt_le_ext_adv *adv,
			const struct bt_le_adv_param *param,
			const struct bt_data *ad, size_t ad_len,
			const struct bt_data *sd, size_t sd_len);

int bt_le_adv_start_legacy(const struct bt_le_adv_param *param,
			   const struct bt_data *ad, size_t ad_len,
			   const struct bt_data *sd, size_t sd_len);
int bt_fsm_le_adv_start(const struct bt_le_adv_param *param,
		    const struct bt_data *ad, size_t ad_len,
		    const struct bt_data *sd, size_t sd_len,
			void *msg, void *args, bt_fsm_handle_t src_handle);

int bt_fsm_le_scan_start(const struct bt_le_scan_param *param, bt_le_scan_cb_t cb, void *msg, void *args, bt_fsm_handle_t src_handle);
int bt_fsm_le_scan_stop(void *msg, void *args, bt_fsm_handle_t src_handle);
struct cmd_data {
	/** HCI status of the command completion */
	u8_t  status;

	/** The command OpCode that the buffer contains */
	u16_t opcode;

	/** The state to update when command completes with success. */
	struct cmd_state_set state;
	bt_fsm_handle_t fsm_handle;
	bt_hci_cmd_cb_t cb;
};
#else
struct cmd_data {
	/** HCI status of the command completion */
	u8_t  status;

	/** The command OpCode that the buffer contains */
	u16_t opcode;

	/** The state to update when command completes with success. */
	struct cmd_state_set *state;

	/** Used by bt_hci_cmd_send_sync. */
	struct k_sem *sync;
};
#endif

struct acl_data {
	/** BT_BUF_ACL_IN */
	u8_t  type;

	/* Index into the bt_conn storage array */
	u8_t  index;

	/** ACL connection handle */
	u16_t handle;
};

#define acl(buf) ((struct acl_data *)net_buf_user_data(buf))

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
static struct cmd_data cmd_data[CONFIG_BT_HCI_CMD_COUNT];

#define cmd(buf) (&cmd_data[net_buf_id(buf)])

/* HCI command buffers. Derive the needed size from BT_BUF_RX_SIZE since
 * the same buffer is also used for the response.
 */
#ifndef CONFIG_CMD_BUF_TX_SIZE
#define CMD_BUF_SIZE (BT_BUF_RX_SIZE)
#else
#define CMD_BUF_SIZE (CONFIG_CMD_BUF_TX_SIZE)
#endif

NET_BUF_POOL_FIXED_DEFINE(hci_cmd_pool, CONFIG_BT_HCI_CMD_COUNT,
			  CMD_BUF_SIZE, NULL);
#endif

NET_BUF_POOL_FIXED_DEFINE(hci_rx_pool, CONFIG_BT_RX_BUF_COUNT,
			  BT_BUF_RX_SIZE, NULL);

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
#define NUM_COMLETE_EVENT_SIZE BT_BUF_SIZE(                            \
	sizeof(struct bt_hci_evt_hdr) +                                \
	sizeof(struct bt_hci_cp_host_num_completed_packets) +          \
	CONFIG_BT_MAX_CONN * sizeof(struct bt_hci_handle_count))
/* Dedicated pool for HCI_Number_of_Completed_Packets. This event is always
 * consumed synchronously by bt_recv_prio() so a single buffer is enough.
 * Having a dedicated pool for it ensures that exhaustion of the RX pool
 * cannot block the delivery of this priority event.
 */
NET_BUF_POOL_FIXED_DEFINE(num_complete_pool, 1, NUM_COMLETE_EVENT_SIZE, NULL);
#endif /* CONFIG_BT_CONN */

#if (CONFIG_BT_DISCARDABLE_BUF_COUNT > 0)
#define DISCARDABLE_EVENT_SIZE BT_BUF_SIZE(CONFIG_BT_DISCARDABLE_BUF_SIZE)
NET_BUF_POOL_FIXED_DEFINE(discardable_pool, CONFIG_BT_DISCARDABLE_BUF_COUNT,
			  DISCARDABLE_EVENT_SIZE, NULL);
#endif /* CONFIG_BT_DISCARDABLE_BUF_COUNT */
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
struct event_handler {
	u8_t event;
	u8_t min_len;
	void (*handler)(struct net_buf *buf);
};

#define EVENT_HANDLER(_evt, _handler, _min_len) \
{ \
	.event = _evt, \
	.handler = _handler, \
	.min_len = _min_len, \
}
#endif

void handle_event(u8_t event, struct net_buf *buf,
				const struct event_handler *handlers,
				size_t num_handlers)
{
	size_t i;

	for (i = 0; i < num_handlers; i++) {
		const struct event_handler *handler = &handlers[i];

		if (handler->event != event) {
			continue;
		}

		if (buf->len < handler->min_len) {
			BT_ERR("Too small (%u bytes) event 0x%02x",
			       buf->len, event);
			return;
		}

		handler->handler(buf);
		return;
	}

	BT_WARN("Unhandled event 0x%02x len %u: %s", event,
		buf->len, bt_hex(buf->data, buf->len));
}

#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL)
static void report_completed_packet(struct net_buf *buf)
{
	u16_t handle = acl(buf)->handle;
	struct bt_conn *conn;

	net_buf_destroy(buf);

	/* Do nothing if controller to host flow control is not supported */
	if (!BT_CMD_TEST(bt_dev.supported_commands, 10, 5)) {
		return;
	}

	conn = bt_conn_lookup_index(acl(buf)->index);
	if (!conn) {
		BT_WARN("Unable to look up conn with index 0x%02x",
			acl(buf)->index);
		return;
	}

	if (conn->state != BT_CONN_CONNECTED &&
	    conn->state != BT_CONN_DISCONNECT) {
		BT_WARN("Not reporting packet for non-connected conn");
		bt_conn_unref(conn);
		return;
	}

	bt_conn_unref(conn);

	BT_DBG("Reporting completed packet for handle %u", handle);

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_host_num_completed_packets *cp;
	struct bt_hci_handle_count *hc;
	buf = bt_hci_cmd_create(BT_HCI_OP_HOST_NUM_COMPLETED_PACKETS,
				sizeof(*cp) + sizeof(*hc));
	if (!buf) {
		BT_ERR("Unable to allocate new HCI command");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->num_handles = sys_cpu_to_le16(1);

	hc = net_buf_add(buf, sizeof(*hc));
	hc->handle = sys_cpu_to_le16(handle);
	hc->count  = sys_cpu_to_le16(1);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_hci_cmd_send_cb(BT_HCI_OP_HOST_NUM_COMPLETED_PACKETS, buf, NULL);
#else
	bt_hci_cmd_send(BT_HCI_OP_HOST_NUM_COMPLETED_PACKETS, buf);
#endif

#else
	struct handle_count hc;
	hc.handle = handle;
	hc.count = 1;

	hci_api_host_num_complete_packets(1, &hc);
#endif
}

#define ACL_IN_SIZE BT_L2CAP_BUF_SIZE(CONFIG_BT_L2CAP_RX_MTU)
NET_BUF_POOL_DEFINE(acl_in_pool, CONFIG_BT_ACL_RX_COUNT, ACL_IN_SIZE,
		    sizeof(struct acl_data), report_completed_packet);
#endif /* CONFIG_BT_HCI_ACL_FLOW_CONTROL */
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
struct net_buf *bt_hci_cmd_create(u16_t opcode, u8_t param_len)
{
	struct bt_hci_cmd_hdr *hdr;
	struct net_buf *buf;

	BT_DBG("opcode 0x%04x param_len %u", opcode, param_len);

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	buf = net_buf_alloc(&hci_cmd_pool, 0);
	if (!buf)
	{
		return NULL;
	}
#else
	buf = net_buf_alloc(&hci_cmd_pool, K_FOREVER);
#endif
	__ASSERT_NO_MSG(buf);

	BT_DBG("buf %p", buf);

	net_buf_reserve(buf, BT_BUF_RESERVE);

	bt_buf_set_type(buf, BT_BUF_CMD);

	cmd(buf)->opcode = opcode;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	memset(&cmd(buf)->state, 0, sizeof(cmd(buf)->state));
	memset(&cmd(buf)->cb, 0, sizeof(cmd(buf)->cb));
	cmd(buf)->fsm_handle = BT_FSM_HANDLE_UNUSED;
#else
	cmd(buf)->sync = NULL;
	cmd(buf)->state = NULL;
#endif
	hdr = net_buf_add(buf, sizeof(*hdr));
	hdr->opcode = sys_cpu_to_le16(opcode);
	hdr->param_len = param_len;

	return buf;
}

int bt_hci_cmd_send(u16_t opcode, struct net_buf *buf)
{
	if (!buf) {
		buf = bt_hci_cmd_create(opcode, 0);
		if (!buf) {
			return -ENOBUFS;
		}
	}

	BT_DBG("opcode 0x%04x len %u", opcode, buf->len);

	/* Host Number of Completed Packets can ignore the ncmd value
	 * and does not generate any cmd complete/status events.
	 */
	if (opcode == BT_HCI_OP_HOST_NUM_COMPLETED_PACKETS) {
		int err;

		err = bt_send(buf);
		if (err) {
			BT_ERR("Unable to send to driver (err %d)", err);
			net_buf_unref(buf);
		}

		return err;
	}

	net_buf_put(&bt_dev.cmd_tx_queue, buf);

	return 0;
}
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
int bt_hci_cmd_send_sync(u16_t opcode, struct net_buf *buf,
			 struct net_buf **rsp)
{
	struct k_sem sync_sem;
	u8_t status;
	int err;

	if (!buf) {
		buf = bt_hci_cmd_create(opcode, 0);
		if (!buf) {
			return -ENOBUFS;
		}
	}

	BT_DBG("buf %p opcode 0x%04x len %u", buf, opcode, buf->len);

	k_sem_init(&sync_sem, 0, 1);
	cmd(buf)->sync = &sync_sem;

	/* Make sure the buffer stays around until the command completes */
	net_buf_ref(buf);

	net_buf_put(&bt_dev.cmd_tx_queue, buf);

	err = k_sem_take(&sync_sem, HCI_CMD_TIMEOUT);
	BT_ASSERT_MSG(err == 0, "k_sem_take failed with err %d", err);

    k_sem_delete(&sync_sem);

	status = cmd(buf)->status;
	if (status) {
		BT_WARN("opcode 0x%04x status 0x%02x", opcode, status);
		net_buf_unref(buf);

		switch (status) {
		case BT_HCI_ERR_CONN_LIMIT_EXCEEDED:
			return -ECONNREFUSED;
		default:
			return -EIO;
		}
	}

	BT_DBG("rsp %p opcode 0x%04x len %u", buf, opcode, buf->len);

	if (rsp) {
		*rsp = buf;
	} else {
		net_buf_unref(buf);
	}

	return 0;
}
#endif
#endif

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) || (defined(CONFIG_BT_BROADCASTER) && CONFIG_BT_BROADCASTER)
const bt_addr_le_t *bt_lookup_id_addr(u8_t id, const bt_addr_le_t *addr)
{
	if (IS_ENABLED(CONFIG_BT_SMP)) {
		struct bt_keys *keys;

		keys = bt_keys_find_irk(id, addr);
		if (keys) {
			BT_DBG("Identity %s matched RPA %s",
			       bt_addr_le_str(&keys->addr),
			       bt_addr_le_str(addr));
			return &keys->addr;
		}
	}

	return addr;
}
#endif /* CONFIG_BT_OBSERVER || CONFIG_BT_CONN */

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
u8_t bt_le_ext_adv_get_index(struct bt_le_ext_adv *adv)
{
	u8_t index = adv - adv_pool;

	__ASSERT(index < ARRAY_SIZE(adv_pool), "Invalid bt_adv pointer");
	return index;
}

static struct bt_le_ext_adv *adv_new(void)
{
	struct bt_le_ext_adv *adv = NULL;
	int i;

	for (i = 0; i < ARRAY_SIZE(adv_pool); i++) {
		if (!atomic_test_bit(adv_pool[i].flags, BT_ADV_CREATED)) {
			adv = &adv_pool[i];
			break;
		}
	}

	if (!adv) {
		return NULL;
	}

	(void)memset(adv, 0, sizeof(*adv));
	atomic_set_bit(adv_pool[i].flags, BT_ADV_CREATED);
	adv->handle = i;

	return adv;
}

static void adv_delete(struct bt_le_ext_adv *adv)
{
	atomic_clear_bit(adv->flags, BT_ADV_CREATED);
}

#if (defined(CONFIG_BT_BROADCASTER) && CONFIG_BT_BROADCASTER)
static struct bt_le_ext_adv *bt_adv_lookup_handle(u8_t handle)
{
	if (handle < ARRAY_SIZE(adv_pool) &&
	    atomic_test_bit(adv_pool[handle].flags, BT_ADV_CREATED)) {
		return &adv_pool[handle];
	}

	return NULL;
}
#endif /* CONFIG_BT_BROADCASTER */
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */

static void bt_adv_foreach(void (*func)(struct bt_le_ext_adv *adv, void *data),
			   void *data)
{
#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	for (size_t i = 0; i < ARRAY_SIZE(adv_pool); i++) {
		if (atomic_test_bit(adv_pool[i].flags, BT_ADV_CREATED)) {
			func(&adv_pool[i], data);
		}
	}
#else
	func(&bt_dev.adv, data);
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */
}

static struct bt_le_ext_adv *adv_new_legacy(void)
{
#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	if (bt_dev.adv) {
		return NULL;
	}

	bt_dev.adv = adv_new();
	return bt_dev.adv;
#else
	return &bt_dev.adv;
#endif
}

static void adv_delete_legacy(void)
{
#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	if (bt_dev.adv) {
		atomic_clear_bit(bt_dev.adv->flags, BT_ADV_CREATED);
		bt_dev.adv = NULL;
	}
#endif
}

struct bt_le_ext_adv *bt_adv_lookup_legacy(void)
{
#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	return bt_dev.adv;
#else
	return &bt_dev.adv;
#endif
}

static int set_le_adv_enable_legacy(struct bt_le_ext_adv *adv, bool enable)
{
	int err;

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_ADV_ENABLE, 1);
	if (!buf) {
		return -ENOBUFS;
	}

	if (enable) {
		net_buf_add_u8(buf, BT_HCI_LE_ADV_ENABLE);
	} else {
		net_buf_add_u8(buf, BT_HCI_LE_ADV_DISABLE);
	}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	cmd_state_set_init(&cmd(buf)->state, adv->flags, BT_ADV_ENABLED, enable);

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_ADV_ENABLE, buf, NULL);
#else
	struct cmd_state_set state;

	cmd_state_set_init(&state, adv->flags, BT_ADV_ENABLED, enable);
	cmd(buf)->state = &state;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_ADV_ENABLE, buf, NULL);
#endif
#else
	u8_t adv_enable;

	if (enable) {
		adv_enable = BT_HCI_LE_ADV_ENABLE;
	} else {
		adv_enable = BT_HCI_LE_ADV_DISABLE;
	}

	err = hci_api_le_adv_enable(adv_enable);
	if (!err)
	{
		atomic_set_bit_to(adv->flags, BT_ADV_ENABLED, enable);
	}
#endif
	if (err) {
		return err;
	}

	return 0;
}

static int set_random_address(const bt_addr_t *addr)
{
	int err;

	BT_DBG("%s", bt_addr_str(addr));

	/* Do nothing if we already have the right address */
	if (!bt_addr_cmp(addr, &bt_dev.random_addr.a)) {
		return 0;
	}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_RANDOM_ADDRESS, sizeof(*addr));
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_mem(buf, addr, sizeof(*addr));
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_RANDOM_ADDRESS, buf, NULL);
	bt_addr_copy(&bt_dev.pending_random_addr.a, addr);
	return err;
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_RANDOM_ADDRESS, buf, NULL);
#endif
#else
	err = hci_api_le_set_random_addr(addr->val);
#endif
	if (err) {
		return err;
	}

	bt_addr_copy(&bt_dev.random_addr.a, addr);
	bt_dev.random_addr.type = BT_ADDR_LE_RANDOM;
	return 0;
}

static int set_le_adv_enable_ext(struct bt_le_ext_adv *adv,
				 bool enable,
				 const struct bt_le_ext_adv_start_param *param)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct net_buf *buf;
	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EXT_ADV_ENABLE, 6);
	if (!buf) {
		return -ENOBUFS;
	}

	if (enable) {
		net_buf_add_u8(buf, BT_HCI_LE_ADV_ENABLE);
	} else {
		net_buf_add_u8(buf, BT_HCI_LE_ADV_DISABLE);
	}

	net_buf_add_u8(buf, 1);

	net_buf_add_u8(buf, adv->handle);
	net_buf_add_le16(buf, param ? sys_cpu_to_le16(param->timeout) : 0);
	net_buf_add_u8(buf, param ? param->num_events : 0);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	cmd_state_set_init(&cmd(buf)->state, adv->flags, BT_ADV_ENABLED, enable);

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_EXT_ADV_ENABLE, buf, NULL);
#else
	struct cmd_state_set state;

	cmd_state_set_init(&state, adv->flags, BT_ADV_ENABLED, enable);
	cmd(buf)->state = &state;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_EXT_ADV_ENABLE, buf, NULL);
#endif
#else
	struct ext_adv_set_t adv_sets[1];
	u8_t adv_enable;

	if (enable) {
		adv_enable = BT_HCI_LE_ADV_ENABLE;
	} else {
		adv_enable = BT_HCI_LE_ADV_DISABLE;
	}

	adv_sets[0].adv_handle = adv->handle;
	adv_sets[0].duration = param ? param->timeout : 0;
	adv_sets[0].max_ext_adv_evts = param ? param->num_events : 0;
	err = hci_api_le_ext_adv_enable(adv_enable, 1, adv_sets);
	if (!err)
	{
		atomic_set_bit_to(adv->flags, BT_ADV_ENABLED, enable);
	}
#endif
	if (err) {
		return err;
	}

	return 0;
}

static int set_le_adv_enable(struct bt_le_ext_adv *adv, bool enable)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return set_le_adv_enable_ext(adv, enable, NULL);
	}

	return set_le_adv_enable_legacy(adv, enable);
}

static int set_adv_random_address(struct bt_le_ext_adv *adv,
				  const bt_addr_t *addr)
{
	int err;

	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
		return set_random_address(addr);
	}

	BT_DBG("%s", bt_addr_str(addr));

	if (!atomic_test_bit(adv->flags, BT_ADV_PARAMS_SET)) {
		bt_addr_copy(&adv->random_addr.a, addr);
		adv->random_addr.type = BT_ADDR_LE_RANDOM;
		atomic_set_bit(adv->flags, BT_ADV_RANDOM_ADDR_PENDING);
		return 0;
	}
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_adv_set_random_addr *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_ADV_SET_RANDOM_ADDR,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	cp->handle = adv->handle;
	bt_addr_copy(&cp->bdaddr, addr);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_ADV_SET_RANDOM_ADDR, buf, NULL);
	bt_addr_copy(&bt_dev.pending_random_addr.a, addr);
	return err;
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_ADV_SET_RANDOM_ADDR, buf,
				   NULL);
#endif
#else
	err = hci_api_le_set_adv_random_addr(adv->handle, addr->val);
#endif
	if (err) {
		return err;
	}

	bt_addr_copy(&adv->random_addr.a, addr);
	adv->random_addr.type = BT_ADDR_LE_RANDOM;
	return 0;
}

int bt_addr_from_str(const char *str, bt_addr_t *addr)
{
	int i, j;
	u8_t tmp;

	if (strlen(str) != 17U) {
		return -EINVAL;
	}

	for (i = 5, j = 1; *str != '\0'; str++, j++) {
		if (!(j % 3) && (*str != ':')) {
			return -EINVAL;
		} else if (*str == ':') {
			i--;
			continue;
		}

		addr->val[i] = addr->val[i] << 4;

		if (char2hex(*str, &tmp) < 0) {
			return -EINVAL;
		}

		addr->val[i] |= tmp;
	}

	return 0;
}

int bt_addr_le_from_str(const char *str, const char *type, bt_addr_le_t *addr)
{
	int err;

	err = bt_addr_from_str(str, &addr->a);
	if (err < 0) {
		return err;
	}

	if (!strcmp(type, "public") || !strcmp(type, "(public)")) {
		addr->type = BT_ADDR_LE_PUBLIC;
	} else if (!strcmp(type, "random") || !strcmp(type, "(random)")) {
		addr->type = BT_ADDR_LE_RANDOM;
	} else if (!strcmp(type, "public-id") || !strcmp(type, "(public-id)")) {
		addr->type = BT_ADDR_LE_PUBLIC_ID;
	} else if (!strcmp(type, "random-id") || !strcmp(type, "(random-id)")) {
		addr->type = BT_ADDR_LE_RANDOM_ID;
	} else {
		return -EINVAL;
	}

	return 0;
}

static void adv_rpa_invalidate(struct bt_le_ext_adv *adv, void *data)
{
	if (!atomic_test_bit(adv->flags, BT_ADV_LIMITED)) {
		atomic_clear_bit(adv->flags, BT_ADV_RPA_VALID);
	}
}

static void le_rpa_invalidate(void)
{
	/* RPA must be submitted */
	atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_TIMEOUT_SET);

	/* Invalidate RPA */
	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED))) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);
	}

	bt_adv_foreach(adv_rpa_invalidate, NULL);
}

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
static void le_rpa_timeout_submit(void)
{
	/* Check if RPA timer is running. */
	if (atomic_test_and_set_bit(bt_dev.flags, BT_DEV_RPA_TIMEOUT_SET)) {
		return;
	}

	k_delayed_work_submit(&bt_dev.rpa_update, RPA_TIMEOUT);
}

/* this function sets new RPA only if current one is no longer valid */
static int le_set_private_addr(u8_t id)
{
	bt_addr_t rpa;
	int err;

	/* check if RPA is valid */
	if (atomic_test_bit(bt_dev.flags, BT_DEV_RPA_VALID)) {
		return 0;
	}

	err = bt_rpa_create(bt_dev.irk[id], &rpa);
	if (!err) {
		err = set_random_address(&rpa);
		if (!err) {
			atomic_set_bit(bt_dev.flags, BT_DEV_RPA_VALID);
		}
	}

	le_rpa_timeout_submit();
	return err;
}

static int le_adv_set_private_addr(struct bt_le_ext_adv *adv)
{
	bt_addr_t rpa;
	int err;

	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
		return le_set_private_addr(adv->id);
	}

	/* check if RPA is valid */
	if (atomic_test_bit(adv->flags, BT_ADV_RPA_VALID)) {
		return 0;
	}

	if (adv == bt_adv_lookup_legacy() && adv->id == BT_ID_DEFAULT) {
		/* Make sure that a Legacy advertiser using default ID has same
		 * RPA address as scanner roles.
		 */
		err = le_set_private_addr(BT_ID_DEFAULT);
		if (err) {
			return err;
		}

		err = set_adv_random_address(adv, &bt_dev.random_addr.a);
		if (!err) {
			atomic_set_bit(adv->flags, BT_ADV_RPA_VALID);
		}

		return 0;
	}

	err = bt_rpa_create(bt_dev.irk[adv->id], &rpa);
	if (!err) {
		err = set_adv_random_address(adv, &rpa);
		if (!err) {
			atomic_set_bit(adv->flags, BT_ADV_RPA_VALID);
		}
	}

	if (!atomic_test_bit(adv->flags, BT_ADV_LIMITED)) {
		le_rpa_timeout_submit();
	}

	return err;
}
#else
static int le_set_private_addr(u8_t id)
{
	bt_addr_t nrpa;
	int err;

	err = bt_rand(nrpa.val, sizeof(nrpa.val));
	if (err) {
		return err;
	}

	nrpa.val[5] &= 0x3f;

	return set_random_address(&nrpa);
}

static int le_adv_set_private_addr(struct bt_le_ext_adv *adv)
{
	bt_addr_t nrpa;
	int err;

	err = bt_rand(nrpa.val, sizeof(nrpa.val));
	if (err) {
		return err;
	}

	nrpa.val[5] &= 0x3f;

	return set_adv_random_address(adv, &nrpa);
}
#endif /* (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY) */

static void adv_update_rpa(struct bt_le_ext_adv *adv, void *data)
{
	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED) &&
	    !atomic_test_bit(adv->flags, BT_ADV_LIMITED) &&
	    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
		int err;

		set_le_adv_enable_ext(adv, false, NULL);

		err = le_adv_set_private_addr(adv);
		if (err) {
			BT_WARN("Failed to update advertiser RPA address (%d)",
				err);
		}

		set_le_adv_enable_ext(adv, true, NULL);
	}
}

static void le_update_private_addr(void)
{
	struct bt_le_ext_adv *adv;
	bool adv_enabled = false;
	u8_t id = BT_ID_DEFAULT;
	int err;

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		bt_adv_foreach(adv_update_rpa, NULL);
	}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	bool scan_enabled = false;

	if (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_ACTIVE_SCAN) &&
	    !(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED))) {
		set_le_scan_enable(BT_HCI_LE_SCAN_DISABLE);
		scan_enabled = true;
	}
#endif
	if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
	    IS_ENABLED(CONFIG_BT_WHITELIST) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_INITIATING)) {
		/* Canceled initiating procedure will be restarted by
		 * connection complete event.
		 */
		bt_le_create_conn_cancel();
	}

	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
		adv = bt_adv_lookup_legacy();

		if (adv &&
		    atomic_test_bit(adv->flags, BT_ADV_ENABLED) &&
		    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
			adv_enabled = true;
			id = adv->id;
			set_le_adv_enable_legacy(adv, false);
		}
	}

	/* If both advertiser and scanner is running then the advertiser
	 * ID must be BT_ID_DEFAULT, this will update the RPA address
	 * for both roles.
	 */
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
    struct net_buf * buf  = NULL;
    err  =  hci_cmd_le_set_private_addr_create(id, &buf);
	if(err) {
        BT_ERR("Set private addr failed");
		return;
	} else {
        err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_RANDOM_ADDRESS, buf, NULL);
        if(err) {
            BT_ERR("Set random address hci cmd send failed");
			return;
		}
	}
#else
	err = le_set_private_addr(id);
	if (err) {
		BT_WARN("Failed to update RPA address (%d)", err);
		return;
	}
#endif
	if (adv_enabled) {
		set_le_adv_enable_legacy(adv, true);
	}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	if (scan_enabled) {
		set_le_scan_enable(BT_HCI_LE_SCAN_ENABLE);
	}
#endif
}

struct adv_id_check_data {
	u8_t id;
	bool adv_enabled;
};

static void adv_id_check_func(struct bt_le_ext_adv *adv, void *data)
{
	struct adv_id_check_data *check_data = data;

	if (IS_ENABLED(CONFIG_BT_EXT_ADV)) {
		/* Only check if the ID is in use, as the advertiser can be
		 * started and stopped without reconfiguring parameters.
		 */
		if (check_data->id == adv->id) {
			check_data->adv_enabled = true;
		}
	} else {
		if (check_data->id == adv->id &&
		    atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
			check_data->adv_enabled = true;
		}
	}
}

static void adv_id_check_connectable_func(struct bt_le_ext_adv *adv, void *data)
{
	struct adv_id_check_data *check_data = data;

	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED) &&
	    atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE) &&
	    check_data->id != adv->id) {
		check_data->adv_enabled = true;
	}
}

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
static void adv_is_limited_enabled(struct bt_le_ext_adv *adv, void *data)
{
	bool *adv_enabled = data;

	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED) &&
	    atomic_test_bit(adv->flags, BT_ADV_LIMITED)) {
		*adv_enabled = true;
	}
}

static void adv_pause_enabled(struct bt_le_ext_adv *adv, void *data)
{
	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		atomic_set_bit(adv->flags, BT_ADV_PAUSED);
		set_le_adv_enable(adv, false);
	}
}

static void adv_unpause_enabled(struct bt_le_ext_adv *adv, void *data)
{
	if (atomic_test_and_clear_bit(adv->flags, BT_ADV_PAUSED)) {
		set_le_adv_enable(adv, true);
	}
}
#endif /* (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) */

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
static void adv_is_private_enabled(struct bt_le_ext_adv *adv, void *data)
{
	bool *adv_enabled = data;

	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED) &&
	    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
		*adv_enabled = true;
	}
}

static void rpa_timeout(struct k_work *work)
{
	bool adv_enabled = false;

	BT_DBG("");

	if (IS_ENABLED(CONFIG_BT_CENTRAL)) {
		struct bt_conn *conn =
			bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL,
						BT_CONN_CONNECT_SCAN);

		if (conn) {
			bt_conn_unref(conn);
			bt_le_create_conn_cancel();
		}
	}

	le_rpa_invalidate();

	bt_adv_foreach(adv_is_private_enabled, &adv_enabled);

	/* IF no roles using the RPA is running we can stop the RPA timer */
	if (!(adv_enabled ||
	      atomic_test_bit(bt_dev.flags, BT_DEV_INITIATING) ||
	      (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING) &&
	       atomic_test_bit(bt_dev.flags, BT_DEV_ACTIVE_SCAN)))) {
		return;
	}

	le_update_private_addr();
}
#endif /* CONFIG_BT_PRIVACY */

bool bt_le_scan_random_addr_check(void)
{
	struct bt_le_ext_adv *adv;

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		/* Advertiser and scanner using different random address */
		return true;
	}

	adv = bt_adv_lookup_legacy();
	if (!adv) {
		return true;
	}

	/* If the advertiser is not enabled or not active there is no issue */
	if (!IS_ENABLED(CONFIG_BT_BROADCASTER) ||
	    !atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return true;
	}

	/* When privacy is enabled the random address will not be set
	 * immediately before starting the role, because the RPA might still be
	 * valid and only updated on RPA timeout.
	 */
	if (IS_ENABLED(CONFIG_BT_PRIVACY)) {
		/* Cannot start scannor or initiator if the random address is
		 * used by the advertiser for an RPA with a different identity
		 * or for a random static identity address.
		 */
		if ((atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY) &&
		     bt_dev.id_addr[adv->id].type == BT_ADDR_LE_RANDOM) ||
		    adv->id != BT_ID_DEFAULT) {
			return false;
		}
	}

	/* If privacy is not enabled then the random address will be attempted
	 * to be set before enabling the role. If another role is already using
	 * the random address then this command will fail, and should return
	 * the error code to the application.
	 */
	return true;
}

static bool bt_le_adv_random_addr_check(const struct bt_le_adv_param *param)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		/* Advertiser and scanner using different random address */
		return true;
	}

	/* If scanner roles are not enabled or not active there is no issue. */
	if (!IS_ENABLED(CONFIG_BT_OBSERVER) ||
	    !(atomic_test_bit(bt_dev.flags, BT_DEV_INITIATING) ||
	      atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING))) {
		return true;
	}

	/* When privacy is enabled the random address will not be set
	 * immediately before starting the role, because the RPA might still be
	 * valid and only updated on RPA timeout.
	 */
	if (IS_ENABLED(CONFIG_BT_PRIVACY)) {
		/* Cannot start an advertiser with random static identity or
		 * using an RPA generated for a different identity than scanner
		 * roles.
		 */
		if (((param->options & BT_LE_ADV_OPT_USE_IDENTITY) &&
		     bt_dev.id_addr[param->id].type == BT_ADDR_LE_RANDOM) ||
		    param->id != BT_ID_DEFAULT) {
			return false;
		}
	} else if (IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY) &&
		   atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING) &&
		   bt_dev.id_addr[BT_ID_DEFAULT].type == BT_ADDR_LE_RANDOM) {
		/* Scanning with random static identity. Stop the advertiser
		 * from overwriting the passive scanner identity address.
		 * In this case the LE Set Random Address command does not
		 * protect us in the case of a passive scanner.
		 * Explicitly stop it here.
		 */

		if (!(param->options & BT_LE_ADV_OPT_CONNECTABLE) &&
		     (param->options & BT_LE_ADV_OPT_USE_IDENTITY)) {
			/* Attempt to set non-connectable NRPA */
			return false;
		} else if (bt_dev.id_addr[param->id].type ==
			   BT_ADDR_LE_RANDOM &&
			   param->id != BT_ID_DEFAULT) {
			/* Attempt to set connectable, or non-connectable with
			 * identity different than scanner.
			 */
			return false;
		}
	}

	/* If privacy is not enabled then the random address will be attempted
	 * to be set before enabling the role. If another role is already using
	 * the random address then this command will fail, and should return
	 * the error code to the application.
	 */
	return true;
}


#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
static int set_le_ext_scan_enable(u8_t enable, u16_t duration)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_ext_scan_enable *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EXT_SCAN_ENABLE, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	if (enable == BT_HCI_LE_SCAN_ENABLE) {
		cp->filter_dup = atomic_test_bit(bt_dev.flags,
						 BT_DEV_SCAN_FILTER_DUP);
	} else {
		cp->filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE;
	}

	cp->enable = enable;
	cp->duration = sys_cpu_to_le16(duration);
	cp->period = 0;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	cmd_state_set_init(&cmd(buf)->state, bt_dev.flags, BT_DEV_SCANNING,
				   enable == BT_HCI_LE_SCAN_ENABLE);

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_EXT_SCAN_ENABLE, buf, NULL);
#else
	struct cmd_state_set state;

	cmd_state_set_init(&state, bt_dev.flags, BT_DEV_SCANNING,
				   enable == BT_HCI_LE_SCAN_ENABLE);
	cmd(buf)->state = &state;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_EXT_SCAN_ENABLE, buf, NULL);
#endif
#else
	u8_t filter_dup;
	if (enable == BT_HCI_LE_SCAN_ENABLE) {
		filter_dup = atomic_test_bit(bt_dev.flags,
						 BT_DEV_SCAN_FILTER_DUP);
	} else {
		filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE;
	}

	err = hci_api_le_ext_scan_enable(enable, filter_dup, sys_cpu_to_le16(duration), 0);
	if (!err)
	{
		atomic_set_bit_to(bt_dev.flags, BT_DEV_SCANNING, enable == BT_HCI_LE_SCAN_ENABLE);
	}
#endif
	if (err) {
		return err;
	}

	return 0;
}

static int set_le_scan_enable_legacy(u8_t enable)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_scan_enable *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_SCAN_ENABLE, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	if (enable == BT_HCI_LE_SCAN_ENABLE) {
		cp->filter_dup = atomic_test_bit(bt_dev.flags,
						 BT_DEV_SCAN_FILTER_DUP);
	} else {
		cp->filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE;
	}

	cp->enable = enable;

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	cmd_state_set_init(&cmd(buf)->state, bt_dev.flags, BT_DEV_SCANNING,
				   enable == BT_HCI_LE_SCAN_ENABLE);

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_SCAN_ENABLE, buf, NULL);
#else
	struct cmd_state_set state;

	cmd_state_set_init(&state, bt_dev.flags, BT_DEV_SCANNING,
				   enable == BT_HCI_LE_SCAN_ENABLE);
	cmd(buf)->state = &state;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_SCAN_ENABLE, buf, NULL);
#endif
#else
	u8_t filter_dup;
	if (enable == BT_HCI_LE_SCAN_ENABLE) {
		filter_dup = atomic_test_bit(bt_dev.flags,
						 BT_DEV_SCAN_FILTER_DUP);
	} else {
		filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE;
	}
	err = hci_api_le_scan_enable(enable, filter_dup);
	if (!err)
	{
		atomic_set_bit_to(bt_dev.flags, BT_DEV_SCANNING, enable == BT_HCI_LE_SCAN_ENABLE);
	}
#endif
	if (err) {
		return err;
	}

	return 0;
}

static int set_le_scan_enable(u8_t enable)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return set_le_ext_scan_enable(enable, 0);
	}

	return set_le_scan_enable_legacy(enable);
}
#endif /* CONFIG_BT_OBSERVER */

static inline bool rpa_is_new(void)
{
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	/* RPA is considered new if there is less than half a second since the
	 * timeout was started.
	 */
	return k_delayed_work_remaining_get(&bt_dev.rpa_update) >
	       (RPA_TIMEOUT_MS - 500);
#else
	return false;
#endif
}

int hci_le_read_max_data_len(u16_t *tx_octets, u16_t *tx_time)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_rp_le_read_max_data_len *rp;
	struct net_buf *rsp;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_READ_MAX_DATA_LEN, NULL, &rsp);
	if (err) {
		BT_ERR("Failed to read DLE max data len");
		return err;
	}

	rp = (void *)rsp->data;
	*tx_octets = sys_le16_to_cpu(rp->max_tx_octets);
	*tx_time = sys_le16_to_cpu(rp->max_tx_time);
	net_buf_unref(rsp);
#else
	err = hci_api_le_get_max_data_len(tx_octets, tx_time);
	if (err) {
		BT_ERR("Failed to read DLE max data len");
		return err;
	}
#endif
	return 0;
}

#if ((defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) && (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)) \
	|| (defined(CONFIG_BT_USER_PHY_UPDATE) && CONFIG_BT_USER_PHY_UPDATE)
static u8_t get_phy(u8_t hci_phy)
{
	switch (hci_phy) {
	case BT_HCI_LE_PHY_1M:
		return BT_GAP_LE_PHY_1M;
	case BT_HCI_LE_PHY_2M:
		return BT_GAP_LE_PHY_2M;
	case BT_HCI_LE_PHY_CODED:
		return BT_GAP_LE_PHY_CODED;
	default:
		return 0;
	}
}
#endif /* (BT_OBSERVER && BT_EXT_ADV) || USER_PHY_UPDATE */

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
static void hci_acl(struct net_buf *buf)
{
	struct bt_hci_acl_hdr *hdr;
	u16_t handle, len;
	struct bt_conn *conn;
	u8_t flags;

	BT_DBG("buf %p", buf);

	BT_ASSERT(buf->len >= sizeof(*hdr));

	hdr = net_buf_pull_mem(buf, sizeof(*hdr));
	len = sys_le16_to_cpu(hdr->len);
	handle = sys_le16_to_cpu(hdr->handle);
	flags = bt_acl_flags(handle);

	acl(buf)->handle = bt_acl_handle(handle);
	acl(buf)->index = BT_CONN_INDEX_INVALID;

	BT_DBG("handle %u len %u flags %u", acl(buf)->handle, len, flags);

	if (buf->len != len) {
		BT_ERR("ACL data length mismatch (%u != %u)", buf->len, len);
		net_buf_unref(buf);
		return;
	}

	conn = bt_conn_lookup_handle(acl(buf)->handle);
	if (!conn) {
		BT_ERR("Unable to find conn for handle %u", acl(buf)->handle);
		net_buf_unref(buf);
		return;
	}

	acl(buf)->index = bt_conn_index(conn);

	bt_conn_recv(conn, buf, flags);
	bt_conn_unref(conn);
}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
static void hci_data_buf_overflow(struct net_buf *buf)
{
	struct bt_hci_evt_data_buf_overflow *evt = (void *)buf->data;

	BT_WARN("Data buffer overflow (link type 0x%02x)", evt->link_type);
	(void)evt;
}
#endif

void bt_hci_num_complete_packets(u16_t handle, u16_t count)
{
	struct bt_conn *conn;
	unsigned int key;

	BT_DBG("handle %u count %u\n", handle, count);

	key = irq_lock();

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		irq_unlock(key);
		BT_ERR("No connection for handle %u", handle);
		return;
	}

	irq_unlock(key);

	while (count--) {
		struct bt_conn_tx *tx;
		sys_snode_t *node;

		key = irq_lock();

		if (conn->pending_no_cb) {
			conn->pending_no_cb--;
			irq_unlock(key);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
				atomic_inc(&bt_dev.le.pkts);
#else
				k_sem_give(bt_conn_get_pkts(conn));
#endif
			continue;
		}

		node = sys_slist_get(&conn->tx_pending);
		irq_unlock(key);

		if (!node) {
			BT_ERR("packets count mismatch");
			break;
		}

		tx = CONTAINER_OF(node, struct bt_conn_tx, node);

		key = irq_lock();
		conn->pending_no_cb = tx->pending_no_cb;
		tx->pending_no_cb = 0U;
		sys_slist_append(&conn->tx_complete, &tx->node);
		irq_unlock(key);

		k_work_submit(&conn->tx_complete_work);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
				atomic_inc(&bt_dev.le.pkts);
#else
				k_sem_give(bt_conn_get_pkts(conn));
#endif

	}

	bt_conn_unref(conn);

	return;
}

static void hci_num_completed_packets(struct net_buf *buf)
{
	struct bt_hci_evt_num_completed_packets *evt = (void *)buf->data;
	int i;

	BT_DBG("num_handles %u", evt->num_handles);

	for (i = 0; i < evt->num_handles; i++) {
		u16_t handle, count;
		struct bt_conn *conn;
		unsigned int key;

		handle = sys_le16_to_cpu(evt->h[i].handle);
		count = sys_le16_to_cpu(evt->h[i].count);

		BT_DBG("handle %u count %u", handle, count);

		key = irq_lock();

		conn = bt_conn_lookup_handle(handle);
		if (!conn) {
			irq_unlock(key);
			BT_ERR("No connection for handle %u", handle);
			continue;
		}

		irq_unlock(key);

		while (count--) {
			struct bt_conn_tx *tx;
			sys_snode_t *node;

			key = irq_lock();

			if (conn->pending_no_cb) {
				conn->pending_no_cb--;
				irq_unlock(key);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
				atomic_inc(&bt_dev.le.pkts);
#else
				k_sem_give(bt_conn_get_pkts(conn));
#endif
				continue;
			}

			node = sys_slist_get(&conn->tx_pending);
			irq_unlock(key);

			if (!node) {
				BT_ERR("packets count mismatch");
				break;
			}

			tx = CONTAINER_OF(node, struct bt_conn_tx, node);

			key = irq_lock();
			conn->pending_no_cb = tx->pending_no_cb;
			tx->pending_no_cb = 0U;
			sys_slist_append(&conn->tx_complete, &tx->node);
			irq_unlock(key);
			k_work_submit(&conn->tx_complete_work);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
			atomic_inc(&bt_dev.le.pkts);
#else
			k_sem_give(bt_conn_get_pkts(conn));
#endif
		}

		bt_conn_unref(conn);
	}
}

static inline bool rpa_timeout_valid_check(void)
{
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	/* Check if create conn timeout will happen before RPA timeout. */
	return k_delayed_work_remaining_get(&bt_dev.rpa_update) >
	       (10 * bt_dev.create_param.timeout);
#else
	return true;
#endif
}

#if (defined(CONFIG_BT_CENTRAL) && CONFIG_BT_CENTRAL)
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int le_create_conn_set_random_addr(bool use_filter, u8_t *own_addr_type)
{
	int err;

	if (IS_ENABLED(CONFIG_BT_PRIVACY)) {
		if (use_filter || rpa_timeout_valid_check()) {
			err = le_set_private_addr(BT_ID_DEFAULT);
			if (err) {
				return err;
			}
		} else {
			/* Force new RPA timeout so that RPA timeout is not
			 * triggered while direct initiator is active.
			 */
			le_rpa_invalidate();
			le_update_private_addr();
		}

		if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
			*own_addr_type = BT_HCI_OWN_ADDR_RPA_OR_RANDOM;
		} else {
			*own_addr_type = BT_ADDR_LE_RANDOM;
		}
	} else {
		const bt_addr_le_t *addr = &bt_dev.id_addr[BT_ID_DEFAULT];

		/* If Static Random address is used as Identity address we
		 * need to restore it before creating connection. Otherwise
		 * NRPA used for active scan could be used for connection.
		 */
		if (addr->type == BT_ADDR_LE_RANDOM) {
			err = set_random_address(&addr->a);
			if (err) {
				return err;
			}
		}

		*own_addr_type = addr->type;
	}

	return 0;
}
#endif

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
static void set_phy_conn_param(const struct bt_conn *conn,
			       struct bt_hci_ext_conn_phy *phy)
{
	phy->conn_interval_min = sys_cpu_to_le16(conn->le.interval_min);
	phy->conn_interval_max = sys_cpu_to_le16(conn->le.interval_max);
	phy->conn_latency = sys_cpu_to_le16(conn->le.latency);
	phy->supervision_timeout = sys_cpu_to_le16(conn->le.timeout);

	phy->min_ce_len = 0;
	phy->max_ce_len = 0;
}
#endif

#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
int bt_le_create_conn_ext(const struct bt_conn *conn)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_ext_create_conn *cp;
	struct bt_hci_ext_conn_phy *phy;
	struct cmd_state_set state;
	bool use_filter = false;
	struct net_buf *buf;
	u8_t own_addr_type;
	u8_t num_phys;
	int err;

	if (IS_ENABLED(CONFIG_BT_WHITELIST)) {
		use_filter = atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT);
	}

	err = le_create_conn_set_random_addr(use_filter, &own_addr_type);
	if (err) {
		return err;
	}

	num_phys = (!(bt_dev.create_param.options &
		      BT_CONN_LE_OPT_NO_1M) ? 1 : 0) +
		   ((bt_dev.create_param.options &
		      BT_CONN_LE_OPT_CODED) ? 1 : 0);

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_EXT_CREATE_CONN, sizeof(*cp) +
				num_phys * sizeof(*phy));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	if (use_filter) {
		/* User Initiated procedure use fast scan parameters. */
		bt_addr_le_copy(&cp->peer_addr, BT_ADDR_LE_ANY);
		cp->filter_policy = BT_HCI_LE_CREATE_CONN_FP_WHITELIST;
	} else {
		const bt_addr_le_t *peer_addr = &conn->le.dst;

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		if (!bt_dev.le.rl_size ||
		    bt_dev.le.rl_entries > bt_dev.le.rl_size) {
			/* Host resolving is used, use the RPA directly. */
			peer_addr = &conn->le.resp_addr;
		}
#endif
		bt_addr_le_copy(&cp->peer_addr, peer_addr);
		cp->filter_policy = BT_HCI_LE_CREATE_CONN_FP_DIRECT;
	}

	cp->own_addr_type = own_addr_type;
	cp->phys = 0;

	if (!(bt_dev.create_param.options & BT_CONN_LE_OPT_NO_1M)) {
		cp->phys |= BT_HCI_LE_EXT_SCAN_PHY_1M;
		phy = net_buf_add(buf, sizeof(*phy));
		phy->scan_interval = sys_cpu_to_le16(
			bt_dev.create_param.interval);
		phy->scan_window = sys_cpu_to_le16(
			bt_dev.create_param.window);
		set_phy_conn_param(conn, phy);
	}

	if (bt_dev.create_param.options & BT_CONN_LE_OPT_CODED) {
		cp->phys |= BT_HCI_LE_EXT_SCAN_PHY_CODED;
		phy = net_buf_add(buf, sizeof(*phy));
		phy->scan_interval = sys_cpu_to_le16(
			bt_dev.create_param.interval_coded);
		phy->scan_window = sys_cpu_to_le16(
			bt_dev.create_param.window_coded);
		set_phy_conn_param(conn, phy);
	}

	cmd_state_set_init(&state, bt_dev.flags, BT_DEV_INITIATING, true);
	cmd(buf)->state = &state;

	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_EXT_CREATE_CONN, buf, NULL);
#else
	bool use_filter = false;
	u8_t own_addr_type;
	u8_t num_phys;
	u8_t filter_policy;
	u8_t init_phys;
	int err;
	bt_addr_le_t peer_addr;

	if (IS_ENABLED(CONFIG_BT_WHITELIST)) {
		use_filter = atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT);
	}

	err = le_create_conn_set_random_addr(use_filter, &own_addr_type);
	if (err) {
		return err;
	}

	num_phys = (!(bt_dev.create_param.options &
		      BT_CONN_LE_OPT_NO_1M) ? 1 : 0) +
		   ((bt_dev.create_param.options &
		      BT_CONN_LE_OPT_CODED) ? 1 : 0);

	struct ext_conn_phy_params_t phys[num_phys];
	struct ext_conn_phy_params_t *phy = phys;

	memset(phy, 0, sizeof(struct ext_conn_phy_params_t) * num_phys);

	if (use_filter) {
		/* User Initiated procedure use fast scan parameters. */
		bt_addr_le_copy(&peer_addr, BT_ADDR_LE_ANY);
		filter_policy = BT_HCI_LE_CREATE_CONN_FP_WHITELIST;
	} else {
		const bt_addr_le_t *ppeer_addr = &conn->le.dst;

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		if (!bt_dev.le.rl_size ||
		    bt_dev.le.rl_entries > bt_dev.le.rl_size) {
			/* Host resolving is used, use the RPA directly. */
			ppeer_addr = &conn->le.resp_addr;
		}
#endif
		bt_addr_le_copy(&peer_addr, ppeer_addr);
		filter_policy = BT_HCI_LE_CREATE_CONN_FP_DIRECT;
	}

	init_phys = 0;

	if (!(bt_dev.create_param.options & BT_CONN_LE_OPT_NO_1M)) {
		init_phys |= BT_HCI_LE_EXT_SCAN_PHY_1M;
		phy->scan_interval = bt_dev.create_param.interval;
		phy->scan_window = bt_dev.create_param.window;
		phy->conn_interval_min = conn->le.interval_min;
		phy->conn_interval_max = conn->le.interval_max;
		phy->conn_latency = conn->le.latency;
		phy->supervision_timeout = conn->le.timeout;
		phy->min_ce_len = 0;
		phy->max_ce_len = 0;
		phy++;
	}

	if (bt_dev.create_param.options & BT_CONN_LE_OPT_CODED) {
		init_phys |= BT_HCI_LE_EXT_SCAN_PHY_CODED;
		phy->scan_interval = bt_dev.create_param.interval_coded;
		phy->scan_window = bt_dev.create_param.window_coded;
		phy->conn_interval_min = conn->le.interval_min;
		phy->conn_interval_max = conn->le.interval_max;
		phy->conn_latency = conn->le.latency;
		phy->supervision_timeout = conn->le.timeout;
		phy->min_ce_len = 0;
		phy->max_ce_len = 0;
	}

	err = hci_api_le_create_conn_ext(filter_policy, own_addr_type, peer_addr.type, peer_addr.a.val, init_phys, phys);
	if (!err)
	{
		atomic_set_bit_to(bt_dev.flags, BT_DEV_INITIATING, true);
	}
	return err;
#endif
}
#endif

int bt_le_create_conn_legacy(const struct bt_conn *conn)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_create_conn *cp;
	struct cmd_state_set state;
	bool use_filter = false;
	struct net_buf *buf;
	u8_t own_addr_type;
	int err;

	if (IS_ENABLED(CONFIG_BT_WHITELIST)) {
		use_filter = atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT);
	}

	err = le_create_conn_set_random_addr(use_filter, &own_addr_type);
	if (err) {
		return err;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_CREATE_CONN, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	memset(cp, 0, sizeof(*cp));
	cp->own_addr_type = own_addr_type;

	if (use_filter) {
		/* User Initiated procedure use fast scan parameters. */
		bt_addr_le_copy(&cp->peer_addr, BT_ADDR_LE_ANY);
		cp->filter_policy = BT_HCI_LE_CREATE_CONN_FP_WHITELIST;
	} else {
		const bt_addr_le_t *peer_addr = &conn->le.dst;

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		if (!bt_dev.le.rl_size ||
		    bt_dev.le.rl_entries > bt_dev.le.rl_size) {
			/* Host resolving is used, use the RPA directly. */
			peer_addr = &conn->le.resp_addr;
		}
#endif
		bt_addr_le_copy(&cp->peer_addr, peer_addr);
		cp->filter_policy = BT_HCI_LE_CREATE_CONN_FP_DIRECT;
	}

	cp->scan_interval = sys_cpu_to_le16(bt_dev.create_param.interval);
	cp->scan_window = sys_cpu_to_le16(bt_dev.create_param.window);

	cp->conn_interval_min = sys_cpu_to_le16(conn->le.interval_min);
	cp->conn_interval_max = sys_cpu_to_le16(conn->le.interval_max);
	cp->conn_latency = sys_cpu_to_le16(conn->le.latency);
	cp->supervision_timeout = sys_cpu_to_le16(conn->le.timeout);

	cmd_state_set_init(&state, bt_dev.flags, BT_DEV_INITIATING, true);
	cmd(buf)->state = &state;

	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_CREATE_CONN, buf, NULL);
#else
	bool use_filter = false;
	u8_t own_addr_type;
	u8_t filter_policy;
	bt_addr_le_t peer_addr;
	int err;

	if (IS_ENABLED(CONFIG_BT_WHITELIST)) {
		use_filter = atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT);
	}

	err = le_create_conn_set_random_addr(use_filter, &own_addr_type);
	if (err) {
		return err;
	}

	if (use_filter) {
		/* User Initiated procedure use fast scan parameters. */
		bt_addr_le_copy(&peer_addr, BT_ADDR_LE_ANY);
		filter_policy = BT_HCI_LE_CREATE_CONN_FP_WHITELIST;
	} else {
		const bt_addr_le_t *ppeer_addr = &conn->le.dst;

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		if (!bt_dev.le.rl_size ||
		    bt_dev.le.rl_entries > bt_dev.le.rl_size) {
			/* Host resolving is used, use the RPA directly. */
			ppeer_addr = &conn->le.resp_addr;
		}
#endif
		bt_addr_le_copy(&peer_addr, ppeer_addr);
		filter_policy = BT_HCI_LE_CREATE_CONN_FP_DIRECT;
	}

	err = hci_api_le_create_conn(bt_dev.create_param.interval,
									bt_dev.create_param.window,
									filter_policy,
									peer_addr.type,
									peer_addr.a.val,
									own_addr_type,
									conn->le.interval_min,
									conn->le.interval_max,
									conn->le.latency,
									conn->le.timeout,
									0, 0);
	if (!err)
	{
		atomic_set_bit_to(bt_dev.flags, BT_DEV_INITIATING, true);
	}
	return err;
#endif
}
#endif

int bt_le_create_conn(const struct bt_conn *conn)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
	    extern int bt_le_create_conn_ext(const struct bt_conn *conn);
		return bt_le_create_conn_ext(conn);
	}

	return bt_le_create_conn_legacy(conn);
}

int bt_le_create_conn_cancel(void)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_CREATE_CONN_CANCEL, 0);
	if (!buf)
	{
		return -ENOBUFS;
	}
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	cmd_state_set_init(&cmd(buf)->state, bt_dev.flags, BT_DEV_INITIATING, false);

	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_CREATE_CONN_CANCEL, buf, NULL);
#else
	struct cmd_state_set state;
	cmd_state_set_init(&state, bt_dev.flags, BT_DEV_INITIATING, false);
	cmd(buf)->state = &state;

	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_CREATE_CONN_CANCEL, buf, NULL);
#endif
#else
	int err;
	err = hci_api_le_create_conn_cancel();
	if (!err)
	{
		atomic_set_bit_to(bt_dev.flags, BT_DEV_INITIATING, false);
	}
	return err;
#endif
}
#endif /* CONFIG_BT_CENTRAL */

int bt_hci_disconnect(u16_t handle, u8_t reason)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct net_buf *buf;
	struct bt_hci_cp_disconnect *disconn;

	buf = bt_hci_cmd_create(BT_HCI_OP_DISCONNECT, sizeof(*disconn));
	if (!buf) {
		return -ENOBUFS;
	}

	disconn = net_buf_add(buf, sizeof(*disconn));
	disconn->handle = sys_cpu_to_le16(handle);
	disconn->reason = reason;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_DISCONNECT, buf, NULL);
#else
	return bt_hci_cmd_send(BT_HCI_OP_DISCONNECT, buf);
#endif
#else
	return hci_api_le_disconnect(handle, reason);
#endif
}

static void hci_disconn_complete(struct net_buf *buf)
{
	struct bt_hci_evt_disconn_complete *evt = (void *)buf->data;
	u16_t handle = sys_le16_to_cpu(evt->handle);
	struct bt_conn *conn;

	BT_DBG("status 0x%02x handle %u reason 0x%02x", evt->status, handle,
	       evt->reason);

	if (evt->status) {
		return;
	}

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to look up conn with handle %u", handle);
		goto advertise;
	}

	conn->err = evt->reason;

	bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
	conn->handle = 0U;

	if (conn->type != BT_CONN_TYPE_LE) {
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
		if (conn->type == BT_CONN_TYPE_SCO) {
			bt_sco_cleanup(conn);
			return;
		}
		/*
		 * If only for one connection session bond was set, clear keys
		 * database row for this connection.
		 */
		if (conn->type == BT_CONN_TYPE_BR &&
		    atomic_test_and_clear_bit(conn->flags, BT_CONN_BR_NOBOND)) {
			bt_keys_link_key_clear(conn->br.link_key);
		}
#endif
		bt_conn_unref(conn);
		return;
	}

#if (defined(CONFIG_BT_CENTRAL) && CONFIG_BT_CENTRAL) && !(defined(CONFIG_BT_WHITELIST) && CONFIG_BT_WHITELIST)
	if (atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT)) {
		bt_conn_set_state(conn, BT_CONN_CONNECT_SCAN);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		bt_fsm_le_scan_update(false, bt_dev.fsm);
#else
		bt_le_scan_update(false);
#endif
	}
#endif /* (defined(CONFIG_BT_CENTRAL) && CONFIG_BT_CENTRAL) && !(defined(CONFIG_BT_WHITELIST) && CONFIG_BT_WHITELIST) */

	bt_conn_unref(conn);

advertise:
	if (IS_ENABLED(CONFIG_BT_PERIPHERAL)) {
		bt_le_adv_resume();
	}
}

static int hci_le_read_remote_features(struct bt_conn *conn)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_read_remote_features *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_READ_REMOTE_FEATURES,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(conn->handle);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_REMOTE_FEATURES, buf, NULL);
#else
	bt_hci_cmd_send(BT_HCI_OP_LE_READ_REMOTE_FEATURES, buf);
#endif
#else
	hci_api_le_read_remote_features(conn->handle);
#endif
	return 0;
}

static int hci_read_remote_version(struct bt_conn *conn)
{
	if (conn->state != BT_CONN_CONNECTED) {
		return -ENOTCONN;
	}

	/* Remote version cannot change. */
	if (atomic_test_bit(conn->flags, BT_CONN_AUTO_VERSION_INFO)) {
		return 0;
	}

	if (conn->state != BT_CONN_CONNECTED) {
		return -ENOTCONN;
	}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_read_remote_version_info *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_READ_REMOTE_VERSION_INFO,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(conn->handle);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_READ_REMOTE_VERSION_INFO, buf,
				    NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_READ_REMOTE_VERSION_INFO, buf,
				    NULL);
#endif
#else
	return hci_api_le_read_remote_version(conn->handle);
#endif
}

/* LE Data Length Change Event is optional so this function just ignore
 * error and stack will continue to use default values.
 */
int bt_le_set_data_len(struct bt_conn *conn, u16_t tx_octets, u16_t tx_time)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_data_len *cp;
	struct net_buf *buf;

	if (conn->state != BT_CONN_CONNECTED) {
		return -ENOTCONN;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_DATA_LEN, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(conn->handle);
	cp->tx_octets = sys_cpu_to_le16(tx_octets);
	cp->tx_time = sys_cpu_to_le16(tx_time);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_DATA_LEN, buf, NULL);
#else
	return bt_hci_cmd_send(BT_HCI_OP_LE_SET_DATA_LEN, buf);
#endif
#else
	return hci_api_le_set_data_len(conn->handle, tx_octets, tx_time);
#endif
}

#if (defined(CONFIG_BT_USER_PHY_UPDATE) && CONFIG_BT_USER_PHY_UPDATE)
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int hci_le_read_phy_cb(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	struct bt_hci_rp_le_read_phy *rp;

	if (!status)
	{
		rp = (void *)buf->data;
		struct bt_conn *conn = bt_conn_lookup_handle(rp->handle);
		if (conn)
		{
			conn->le.phy.tx_phy = get_phy(rp->tx_phy);
			conn->le.phy.rx_phy = get_phy(rp->rx_phy);

			if (IS_ENABLED(CONFIG_BT_AUTO_PHY_UPDATE) &&
			    conn->le.phy.tx_phy == BT_HCI_LE_PHY_PREFER_2M &&
			    conn->le.phy.rx_phy == BT_HCI_LE_PHY_PREFER_2M) {
				/* Already on 2M, skip auto-phy update. */
				atomic_set_bit(conn->flags,
					       BT_CONN_AUTO_PHY_COMPLETE);
			}

			bt_conn_unref(conn);
		}
	}

	net_buf_unref(buf);

	return 0;
}
#endif
static int hci_le_read_phy(struct bt_conn *conn)
{
	int err = 0;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_read_phy *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_READ_PHY, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(conn->handle);

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_hci_cmd_cb_t cb =
	{
		.func = hci_le_read_phy_cb,
	};

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_PHY, buf, &cb);
#else
	struct bt_hci_rp_le_read_phy *rp;
	struct net_buf *rsp;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_READ_PHY, buf, &rsp);
	if (err) {
		return err;
	}

	rp = (void *)rsp->data;
	conn->le.phy.tx_phy = get_phy(rp->tx_phy);
	conn->le.phy.rx_phy = get_phy(rp->rx_phy);
	net_buf_unref(rsp);
#endif
#else
	u8_t  tx_phy;
	u8_t  rx_phy;
	err = hci_api_le_read_phy(conn->handle, &tx_phy, &rx_phy);
	if (err)
	{
		return err;
	}
	conn->le.phy.tx_phy = get_phy(tx_phy);
	conn->le.phy.rx_phy = get_phy(rx_phy);
#endif
	return err;
}
#endif /* (defined(CONFIG_BT_USER_PHY_UPDATE) && CONFIG_BT_USER_PHY_UPDATE) */

int bt_le_set_phy(struct bt_conn *conn, u8_t pref_tx_phy, u8_t pref_rx_phy)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_phy *cp;
	struct net_buf *buf;

	if (conn->state != BT_CONN_CONNECTED) {
		return -ENOTCONN;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_PHY, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(conn->handle);
	cp->all_phys = 0U;
	cp->tx_phys = pref_tx_phy;
	cp->rx_phys = pref_rx_phy;
	cp->phy_opts = BT_HCI_LE_PHY_CODED_ANY;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_PHY, buf, NULL);
#else
	return bt_hci_cmd_send(BT_HCI_OP_LE_SET_PHY, buf);
#endif
#else
	return hci_api_le_set_phy(conn->handle, 0U, pref_tx_phy, pref_rx_phy, BT_HCI_LE_PHY_CODED_ANY);
#endif
}

static void slave_update_conn_param(struct bt_conn *conn)
{
	if (!IS_ENABLED(CONFIG_BT_PERIPHERAL)) {
		return;
	}

	/* don't start timer again on PHY update etc */
	if (atomic_test_bit(conn->flags, BT_CONN_SLAVE_PARAM_UPDATE)) {
		return;
	}

	/*
	 * Core 4.2 Vol 3, Part C, 9.3.12.2
	 * The Peripheral device should not perform a Connection Parameter
	 * Update procedure within 5 s after establishing a connection.
	 */
	k_delayed_work_submit(&conn->update_work, CONN_UPDATE_TIMEOUT);
}

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
static void pending_id_update(struct bt_keys *keys, void *data)
{
	if (keys->state & BT_KEYS_ID_PENDING_ADD) {
		keys->state &= ~BT_KEYS_ID_PENDING_ADD;
		bt_id_add(keys);
		return;
	}

	if (keys->state & BT_KEYS_ID_PENDING_DEL) {
		keys->state &= ~BT_KEYS_ID_PENDING_DEL;
		bt_id_del(keys);
		return;
	}
}

static void pending_id_keys_update_set(struct bt_keys *keys, u8_t flag)
{
	atomic_set_bit(bt_dev.flags, BT_DEV_ID_PENDING);
	keys->state |= flag;
}

static void pending_id_keys_update(void)
{
	if (atomic_test_and_clear_bit(bt_dev.flags, BT_DEV_ID_PENDING)) {
		if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
		    IS_ENABLED(CONFIG_BT_PRIVACY)) {
			bt_keys_foreach(BT_KEYS_ALL, pending_id_update, NULL);
		} else {
			bt_keys_foreach(BT_KEYS_IRK, pending_id_update, NULL);
		}
	}
}
#endif /* (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) */

static struct bt_conn *find_pending_connect(u8_t role, bt_addr_le_t *peer_addr)
{
	struct bt_conn *conn;

	/*
	 * Make lookup to check if there's a connection object in
	 * CONNECT or CONNECT_AUTO state associated with passed peer LE address.
	 */
	if (IS_ENABLED(CONFIG_BT_CENTRAL) && role == BT_HCI_ROLE_MASTER) {
		conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, peer_addr,
					       BT_CONN_CONNECT);
		if (IS_ENABLED(CONFIG_BT_WHITELIST) && !conn) {
			conn = bt_conn_lookup_state_le(BT_ID_DEFAULT,
						       BT_ADDR_LE_NONE,
						       BT_CONN_CONNECT_AUTO);
		}

		return conn;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && role == BT_HCI_ROLE_SLAVE) {
		conn = bt_conn_lookup_state_le(bt_dev.adv_conn_id, peer_addr,
					       BT_CONN_CONNECT_DIR_ADV);
		if (!conn) {
			conn = bt_conn_lookup_state_le(bt_dev.adv_conn_id,
						       BT_ADDR_LE_NONE,
						       BT_CONN_CONNECT_ADV);
		}

		return conn;
	}

	return NULL;
}

static void conn_auto_initiate(struct bt_conn *conn)
{
	int err;

	if (conn->state != BT_CONN_CONNECTED) {
		/* It is possible that connection was disconnected directly from
		 * connected callback so we must check state before doing
		 * connection parameters update.
		 */
		return;
	}

	if (!atomic_test_bit(conn->flags, BT_CONN_AUTO_FEATURE_EXCH) &&
	    ((conn->role == BT_HCI_ROLE_MASTER) ||
	     BT_FEAT_LE_SLAVE_FEATURE_XCHG(bt_dev.le.features))) {
		err = hci_le_read_remote_features(conn);
		if (!err) {
			return;
		}
	}

	if (IS_ENABLED(CONFIG_BT_REMOTE_VERSION) &&
	    !atomic_test_bit(conn->flags, BT_CONN_AUTO_VERSION_INFO)) {
		err = hci_read_remote_version(conn);
		if (!err) {
			return;
		}
	}

	if (IS_ENABLED(CONFIG_BT_AUTO_PHY_UPDATE) &&
	    !atomic_test_bit(conn->flags, BT_CONN_AUTO_PHY_COMPLETE) &&
	    BT_FEAT_LE_PHY_2M(bt_dev.le.features)) {
		err = bt_le_set_phy(conn,
				    BT_HCI_LE_PHY_PREFER_2M,
				    BT_HCI_LE_PHY_PREFER_2M);
		if (!err) {
			atomic_set_bit(conn->flags, BT_CONN_AUTO_PHY_UPDATE);
			return;
		}

		BT_ERR("Failed to set LE PHY (%d)", err);
	}

	if (IS_ENABLED(CONFIG_BT_AUTO_DATA_LEN_UPDATE) &&
	    BT_FEAT_LE_DLE(bt_dev.le.features)) {
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		err = hci_cmd_le_read_max_data_len(conn);
		if (err) {
			BT_ERR("Failed to read data len (%d)", err);
		}
#else
		u16_t tx_octets, tx_time;

		err = hci_le_read_max_data_len(&tx_octets, &tx_time);
		if (!err) {
			err = bt_le_set_data_len(conn, tx_octets, tx_time);
			if (err) {
				BT_ERR("Failed to set data len (%d)", err);
			}
		}
#endif
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    conn->role == BT_CONN_ROLE_SLAVE) {
		slave_update_conn_param(conn);
	}
}

static void le_conn_complete_cancel(void)
{
	struct bt_conn *conn;

	/* Handle create connection cancel.
	 *
	 * There is no need to check ID address as only one
	 * connection in master role can be in pending state.
	 */
	conn = find_pending_connect(BT_HCI_ROLE_MASTER, NULL);
	if (!conn) {
		BT_ERR("No pending master connection");
		return;
	}

	conn->err = BT_HCI_ERR_UNKNOWN_CONN_ID;

	/* Handle cancellation of outgoing connection attempt. */
	if (!IS_ENABLED(CONFIG_BT_WHITELIST)) {
		/* We notify before checking autoconnect flag
		 * as application may choose to change it from
		 * callback.
		 */
		bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
		/* Check if device is marked for autoconnect. */
		if (atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT)) {
			/* Restart passive scanner for device */
			bt_conn_set_state(conn, BT_CONN_CONNECT_SCAN);
		}
	} else {
		if (atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT)) {
			/* Restart whitelist initiator after RPA timeout. */
			bt_le_create_conn(conn);
		} else {
			/* Create connection canceled by timeout */
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
		}
	}

	bt_conn_unref(conn);
}

static void le_conn_complete_adv_timeout(void)
{
	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
		struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();
		struct bt_conn *conn;

		/* Handle advertising timeout after high duty cycle directed
		 * advertising.
		 */

		atomic_clear_bit(adv->flags, BT_ADV_ENABLED);

		/* There is no need to check ID address as only one
		 * connection in slave role can be in pending state.
		 */
		conn = find_pending_connect(BT_HCI_ROLE_SLAVE, NULL);
		if (!conn) {
			BT_ERR("No pending slave connection");
			return;
		}

		conn->err = BT_HCI_ERR_ADV_TIMEOUT;
		bt_conn_set_state(conn, BT_CONN_DISCONNECTED);

		bt_conn_unref(conn);
	}
}

static void enh_conn_complete(struct bt_hci_evt_le_enh_conn_complete *evt)
{
	u16_t handle = sys_le16_to_cpu(evt->handle);
	bt_addr_le_t peer_addr, id_addr;
	struct bt_conn *conn;

	BT_DBG("status 0x%02x handle %u role %u peer %s peer RPA %s",
	       evt->status, handle, evt->role, bt_addr_le_str(&evt->peer_addr),
	       bt_addr_str(&evt->peer_rpa));
	BT_DBG("local RPA %s", bt_addr_str(&evt->local_rpa));

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	pending_id_keys_update();
#endif

	if (evt->status) {
		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
		    evt->status == BT_HCI_ERR_ADV_TIMEOUT) {
			le_conn_complete_adv_timeout();
			return;
		}

		if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
		    evt->status == BT_HCI_ERR_UNKNOWN_CONN_ID) {
			le_conn_complete_cancel();
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
			bt_fsm_le_scan_update(false, bt_dev.fsm);
#else
			bt_le_scan_update(false);
#endif
			return;
		}

		BT_WARN("Unexpected status 0x%02x", evt->status);

		return;
	}

	/* Translate "enhanced" identity address type to normal one */
	if (evt->peer_addr.type == BT_ADDR_LE_PUBLIC_ID ||
	    evt->peer_addr.type == BT_ADDR_LE_RANDOM_ID) {
		bt_addr_le_copy(&id_addr, &evt->peer_addr);
		id_addr.type -= BT_ADDR_LE_PUBLIC_ID;

		bt_addr_copy(&peer_addr.a, &evt->peer_rpa);
		peer_addr.type = BT_ADDR_LE_RANDOM;
	} else {
		u8_t id = evt->role == BT_HCI_ROLE_SLAVE ? bt_dev.adv_conn_id :
							   BT_ID_DEFAULT;

		bt_addr_le_copy(&id_addr,
				bt_lookup_id_addr(id, &evt->peer_addr));
		bt_addr_le_copy(&peer_addr, &evt->peer_addr);
	}

	conn = find_pending_connect(evt->role, &id_addr);
	if (!conn)
	{
		/* If no pending connection, try alloc a new conn if passiable */
		u8_t id = evt->role == BT_HCI_ROLE_SLAVE ? bt_dev.adv_conn_id :
							   BT_ID_DEFAULT;
		conn = bt_conn_add_le(id, &id_addr);
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    evt->role == BT_HCI_ROLE_SLAVE &&
	    !(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
		struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();
		/* Clear advertising even if we are not able to add connection
		 * object to keep host in sync with controller state.
		 */
		atomic_clear_bit(adv->flags, BT_ADV_ENABLED);
	}

	if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
	    evt->role == BT_HCI_ROLE_MASTER) {
		/* Clear initiating even if we are not able to add connection
		 * object to keep the host in sync with controller state.
		 */
		atomic_clear_bit(bt_dev.flags, BT_DEV_INITIATING);
	}

	if (!conn) {
		BT_ERR("Unable to add new conn for handle %u", handle);
		bt_hci_disconnect(handle, BT_HCI_ERR_MEM_CAPACITY_EXCEEDED);
		return;
	}

	conn->handle = handle;
	bt_addr_le_copy(&conn->le.dst, &id_addr);
	conn->le.interval = sys_le16_to_cpu(evt->interval);
	conn->le.latency = sys_le16_to_cpu(evt->latency);
	conn->le.timeout = sys_le16_to_cpu(evt->supv_timeout);
	conn->role = evt->role;
	conn->err = 0U;

#if (defined(CONFIG_BT_USER_DATA_LEN_UPDATE) && CONFIG_BT_USER_DATA_LEN_UPDATE)
	conn->le.data_len.tx_max_len = BT_GAP_DATA_LEN_DEFAULT;
	conn->le.data_len.tx_max_time = BT_GAP_DATA_TIME_DEFAULT;
	conn->le.data_len.rx_max_len = BT_GAP_DATA_LEN_DEFAULT;
	conn->le.data_len.rx_max_time = BT_GAP_DATA_TIME_DEFAULT;
#endif

#if (defined(CONFIG_BT_USER_PHY_UPDATE) && CONFIG_BT_USER_PHY_UPDATE)
	conn->le.phy.tx_phy = BT_GAP_LE_PHY_1M;
	conn->le.phy.rx_phy = BT_GAP_LE_PHY_1M;
#endif
	/*
	 * Use connection address (instead of identity address) as initiator
	 * or responder address. Only slave needs to be updated. For master all
	 * was set during outgoing connection creation.
	 */
	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    conn->role == BT_HCI_ROLE_SLAVE) {
		bt_addr_le_copy(&conn->le.init_addr, &peer_addr);

		if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
		      BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
			struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();

			if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
			    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
				conn->le.resp_addr.type = BT_ADDR_LE_RANDOM;
				if (bt_addr_cmp(&evt->local_rpa,
						BT_ADDR_ANY) != 0) {
					bt_addr_copy(&conn->le.resp_addr.a,
						     &evt->local_rpa);
				} else {
					bt_addr_copy(&conn->le.resp_addr.a,
						     &bt_dev.random_addr.a);
				}
			} else {
				bt_addr_le_copy(&conn->le.resp_addr,
						&bt_dev.id_addr[conn->id]);
			}
		} else {
			/* Copy the local RPA and handle this in advertising set
			 * terminated event.
			 */
			if (!bt_addr_cmp(&evt->local_rpa, BT_ADDR_ANY)) {
				bt_addr_le_copy(&conn->le.resp_addr, &bt_dev.id_addr[conn->id]);
			}
			else {
				bt_addr_copy(&conn->le.resp_addr.a, &evt->local_rpa);
			}
		}

		/* if the controller supports, lets advertise for another
		 * slave connection.
		 * check for connectable advertising state is sufficient as
		 * this is how this le connection complete for slave occurred.
		 */
		if (BT_LE_STATES_SLAVE_CONN_ADV(bt_dev.le.states)) {
			bt_le_adv_resume();
		}

		if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
		    !BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
			struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();
			/* No advertising set terminated event, must be a
			 * legacy advertiser set.
			 */
			if (!atomic_test_bit(adv->flags, BT_ADV_PERSIST)) {
				adv_delete_legacy();
			}
		}
	}

	if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
	    conn->role == BT_HCI_ROLE_MASTER) {
		bt_addr_le_copy(&conn->le.resp_addr, &peer_addr);

		if (IS_ENABLED(CONFIG_BT_PRIVACY)) {
			conn->le.init_addr.type = BT_ADDR_LE_RANDOM;
			if (bt_addr_cmp(&evt->local_rpa, BT_ADDR_ANY) != 0) {
				bt_addr_copy(&conn->le.init_addr.a,
					     &evt->local_rpa);
			} else {
				bt_addr_copy(&conn->le.init_addr.a,
					     &bt_dev.random_addr.a);
			}
		} else {
			bt_addr_le_copy(&conn->le.init_addr,
					&bt_dev.id_addr[conn->id]);
		}
	}

#if (defined(CONFIG_BT_USER_PHY_UPDATE) && CONFIG_BT_USER_PHY_UPDATE)
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		int err;

		err = hci_le_read_phy(conn);
		if (err) {
			BT_WARN("Failed to read PHY (%d)", err);
		} else {
			if (IS_ENABLED(CONFIG_BT_AUTO_PHY_UPDATE) &&
			    conn->le.phy.tx_phy == BT_HCI_LE_PHY_PREFER_2M &&
			    conn->le.phy.rx_phy == BT_HCI_LE_PHY_PREFER_2M) {
				/* Already on 2M, skip auto-phy update. */
				atomic_set_bit(conn->flags,
					       BT_CONN_AUTO_PHY_COMPLETE);
			}
		}
	}
#endif /* (defined(CONFIG_BT_USER_PHY_UPDATE) && CONFIG_BT_USER_PHY_UPDATE) */

	bt_conn_set_state(conn, BT_CONN_CONNECTED);

	/* Start auto-initiated procedures */
	conn_auto_initiate(conn);

	bt_conn_unref(conn);

	if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
	    conn->role == BT_HCI_ROLE_MASTER) {
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		bt_fsm_le_scan_update(false, bt_dev.fsm);
#else
		bt_le_scan_update(false);
#endif
	}
}

static void le_enh_conn_complete(struct net_buf *buf)
{
	enh_conn_complete((void *)buf->data);
}

static void le_legacy_conn_complete(struct net_buf *buf)
{
	struct bt_hci_evt_le_conn_complete *evt = (void *)buf->data;
	struct bt_hci_evt_le_enh_conn_complete enh;

	BT_DBG("status 0x%02x role %u %s", evt->status, evt->role,
	       bt_addr_le_str(&evt->peer_addr));

	enh.status         = evt->status;
	enh.handle         = evt->handle;
	enh.role           = evt->role;
	enh.interval       = evt->interval;
	enh.latency        = evt->latency;
	enh.supv_timeout   = evt->supv_timeout;
	enh.clock_accuracy = evt->clock_accuracy;

	bt_addr_le_copy(&enh.peer_addr, &evt->peer_addr);

	if (IS_ENABLED(CONFIG_BT_PRIVACY)) {
		bt_addr_copy(&enh.local_rpa, &bt_dev.random_addr.a);
	} else {
		bt_addr_copy(&enh.local_rpa, BT_ADDR_ANY);
	}

	bt_addr_copy(&enh.peer_rpa, BT_ADDR_ANY);

	enh_conn_complete(&enh);
}

static void le_remote_feat_complete(struct net_buf *buf)
{
	struct bt_hci_evt_le_remote_feat_complete *evt = (void *)buf->data;
	u16_t handle = sys_le16_to_cpu(evt->handle);
	struct bt_conn *conn;

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to lookup conn for handle %u", handle);
		return;
	}

	if (!evt->status) {
		memcpy(conn->le.features, evt->features,
		       sizeof(conn->le.features));
	}

	atomic_set_bit(conn->flags, BT_CONN_AUTO_FEATURE_EXCH);

	if (IS_ENABLED(CONFIG_BT_REMOTE_INFO) &&
	    !IS_ENABLED(CONFIG_BT_REMOTE_VERSION)) {
		notify_remote_info(conn);
	}

	/* Continue with auto-initiated procedures */
	conn_auto_initiate(conn);

	bt_conn_unref(conn);
}

#if (defined(CONFIG_BT_DATA_LEN_UPDATE) && CONFIG_BT_DATA_LEN_UPDATE)
static void le_data_len_change(struct net_buf *buf)
{
	struct bt_hci_evt_le_data_len_change *evt = (void *)buf->data;
	u16_t max_tx_octets = sys_le16_to_cpu(evt->max_tx_octets);
	u16_t max_rx_octets = sys_le16_to_cpu(evt->max_rx_octets);
	u16_t max_tx_time = sys_le16_to_cpu(evt->max_tx_time);
	u16_t max_rx_time = sys_le16_to_cpu(evt->max_rx_time);
	u16_t handle = sys_le16_to_cpu(evt->handle);
	struct bt_conn *conn;

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to lookup conn for handle %u", handle);
		return;
	}

	BT_DBG("max. tx: %u (%uus), max. rx: %u (%uus)", max_tx_octets,
	       max_tx_time, max_rx_octets, max_rx_time);

    bt_dev.le.mtu = max_tx_octets;

#if (defined(CONFIG_BT_USER_DATA_LEN_UPDATE) && CONFIG_BT_USER_DATA_LEN_UPDATE)
	if (IS_ENABLED(CONFIG_BT_AUTO_DATA_LEN_UPDATE)) {
		atomic_set_bit(conn->flags, BT_CONN_AUTO_DATA_LEN_COMPLETE);
	}

	conn->le.data_len.tx_max_len = max_tx_octets;
	conn->le.data_len.tx_max_time = max_tx_time;
	conn->le.data_len.rx_max_len = max_rx_octets;
	conn->le.data_len.rx_max_time = max_rx_time;
	notify_le_data_len_updated(conn);
#endif

	(void)max_tx_octets;
	(void)max_rx_octets;
	(void)max_tx_time;
	(void)max_rx_time;

	bt_conn_unref(conn);
}
#endif /* CONFIG_BT_DATA_LEN_UPDATE */

#if (defined(CONFIG_BT_PHY_UPDATE) && CONFIG_BT_PHY_UPDATE)
static void le_phy_update_complete(struct net_buf *buf)
{
	struct bt_hci_evt_le_phy_update_complete *evt = (void *)buf->data;
	u16_t handle = sys_le16_to_cpu(evt->handle);
	struct bt_conn *conn;

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to lookup conn for handle %u", handle);
		return;
	}

	BT_DBG("PHY updated: status: 0x%02x, tx: %u, rx: %u",
	       evt->status, evt->tx_phy, evt->rx_phy);

	if (IS_ENABLED(CONFIG_BT_AUTO_PHY_UPDATE) &&
	    atomic_test_and_clear_bit(conn->flags, BT_CONN_AUTO_PHY_UPDATE)) {
		atomic_set_bit(conn->flags, BT_CONN_AUTO_PHY_COMPLETE);

		/* Continue with auto-initiated procedures */
		conn_auto_initiate(conn);
	}

#if (defined(CONFIG_BT_USER_PHY_UPDATE) && CONFIG_BT_USER_PHY_UPDATE)
	conn->le.phy.tx_phy = get_phy(evt->tx_phy);
	conn->le.phy.rx_phy = get_phy(evt->rx_phy);
	notify_le_phy_updated(conn);
#endif

	bt_conn_unref(conn);
}
#endif /* CONFIG_BT_PHY_UPDATE */

bool bt_le_conn_params_valid(const struct bt_le_conn_param *param)
{
	/* All limits according to BT Core spec 5.0 [Vol 2, Part E, 7.8.12] */

	if (param->interval_min > param->interval_max ||
	    param->interval_min < 6 || param->interval_max > 3200) {
		return false;
	}

	if (param->latency > 499) {
		return false;
	}

	if (param->timeout < 10 || param->timeout > 3200 ||
	    ((param->timeout * 4U) <=
	     ((1U + param->latency) * param->interval_max))) {
		return false;
	}

	return true;
}

static void le_conn_param_neg_reply(u16_t handle, u8_t reason)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_conn_param_req_neg_reply *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_CONN_PARAM_REQ_NEG_REPLY,
				sizeof(*cp));
	if (!buf) {
		BT_ERR("Unable to allocate buffer");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	cp->reason = sys_cpu_to_le16(reason);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_hci_cmd_send_cb(BT_HCI_OP_LE_CONN_PARAM_REQ_NEG_REPLY, buf, NULL);
#else
	bt_hci_cmd_send(BT_HCI_OP_LE_CONN_PARAM_REQ_NEG_REPLY, buf);
#endif
#else
	hci_api_le_conn_param_neg_reply(handle, reason);
#endif
}

static int le_conn_param_req_reply(u16_t handle,
				   const struct bt_le_conn_param *param)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_conn_param_req_reply *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_CONN_PARAM_REQ_REPLY, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->handle = sys_cpu_to_le16(handle);
	cp->interval_min = sys_cpu_to_le16(param->interval_min);
	cp->interval_max = sys_cpu_to_le16(param->interval_max);
	cp->latency = sys_cpu_to_le16(param->latency);
	cp->timeout = sys_cpu_to_le16(param->timeout);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_CONN_PARAM_REQ_REPLY, buf, NULL);
#else
	return bt_hci_cmd_send(BT_HCI_OP_LE_CONN_PARAM_REQ_REPLY, buf);
#endif
#else
	return hci_api_le_conn_param_req_reply(handle, param->interval_min,param->interval_max, 
		param->latency,param->timeout, 0, 0 );
#endif
}

static void le_conn_param_req(struct net_buf *buf)
{
	struct bt_hci_evt_le_conn_param_req *evt = (void *)buf->data;
	struct bt_le_conn_param param;
	struct bt_conn *conn;
	u16_t handle;

	handle = sys_le16_to_cpu(evt->handle);
	param.interval_min = sys_le16_to_cpu(evt->interval_min);
	param.interval_max = sys_le16_to_cpu(evt->interval_max);
	param.latency = sys_le16_to_cpu(evt->latency);
	param.timeout = sys_le16_to_cpu(evt->timeout);

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to lookup conn for handle %u", handle);
		le_conn_param_neg_reply(handle, BT_HCI_ERR_UNKNOWN_CONN_ID);
		return;
	}

	if (!le_param_req(conn, &param)) {
		le_conn_param_neg_reply(handle, BT_HCI_ERR_INVALID_LL_PARAM);
	} else {
		le_conn_param_req_reply(handle, &param);
	}

	bt_conn_unref(conn);
}

static void le_conn_update_complete(struct net_buf *buf)
{
	struct bt_hci_evt_le_conn_update_complete *evt = (void *)buf->data;
	struct bt_conn *conn;
	u16_t handle;

	handle = sys_le16_to_cpu(evt->handle);

	BT_DBG("status 0x%02x, handle %u", evt->status, handle);

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to lookup conn for handle %u", handle);
		return;
	}

	if (!evt->status) {
		conn->le.interval = sys_le16_to_cpu(evt->interval);
		conn->le.latency = sys_le16_to_cpu(evt->latency);
		conn->le.timeout = sys_le16_to_cpu(evt->supv_timeout);
		notify_le_param_updated(conn);
	} else if (evt->status == BT_HCI_ERR_UNSUPP_REMOTE_FEATURE &&
		   conn->role == BT_HCI_ROLE_SLAVE &&
		   !atomic_test_and_set_bit(conn->flags,
					    BT_CONN_SLAVE_PARAM_L2CAP)) {
		/* CPR not supported, let's try L2CAP CPUP instead */
		struct bt_le_conn_param param;

		param.interval_min = conn->le.interval_min;
		param.interval_max = conn->le.interval_max;
		param.latency = conn->le.pending_latency;
		param.timeout = conn->le.pending_timeout;

		bt_l2cap_update_conn_param(conn, &param);
	}

	bt_conn_unref(conn);
}

#if (defined(CONFIG_BT_CENTRAL) && CONFIG_BT_CENTRAL)
static void check_pending_conn(const bt_addr_le_t *id_addr,
			       const bt_addr_le_t *addr, u8_t adv_props)
{
	struct bt_conn *conn;

	/* No connections are allowed during explicit scanning */
	if (atomic_test_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN)) {
		return;
	}

	/* Return if event is not connectable */
	if (!(adv_props & BT_HCI_LE_ADV_EVT_TYPE_CONN)) {
		return;
	}

	conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, id_addr,
				       BT_CONN_CONNECT_SCAN);
	if (!conn) {
		return;
	}

	if (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING) &&
	    set_le_scan_enable(BT_HCI_LE_SCAN_DISABLE)) {
		goto failed;
	}

	bt_addr_le_copy(&conn->le.resp_addr, addr);
	if (bt_le_create_conn(conn)) {
		goto failed;
	}

	bt_conn_set_state(conn, BT_CONN_CONNECT);
	bt_conn_unref(conn);
	return;

failed:
	conn->err = BT_HCI_ERR_UNSPECIFIED;
	bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
	bt_conn_unref(conn);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_fsm_le_scan_update(false, bt_dev.fsm);
#else
	bt_le_scan_update(false);
#endif
}
#endif /* CONFIG_BT_CENTRAL */

#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL)
static int set_flow_control(void)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_host_buffer_size *hbs;
	struct net_buf *buf;
	int err;

	/* Check if host flow control is actually supported */
	if (!BT_CMD_TEST(bt_dev.supported_commands, 10, 5)) {
		BT_WARN("Controller to host flow control not supported");
		return 0;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_HOST_BUFFER_SIZE,
				sizeof(*hbs));
	if (!buf) {
		return -ENOBUFS;
	}

	hbs = net_buf_add(buf, sizeof(*hbs));
	(void)memset(hbs, 0, sizeof(*hbs));
	hbs->acl_mtu = sys_cpu_to_le16(CONFIG_BT_L2CAP_RX_MTU +
				       sizeof(struct bt_l2cap_hdr));
	hbs->acl_pkts = sys_cpu_to_le16(CONFIG_BT_ACL_RX_COUNT);

	err = bt_hci_cmd_send_sync(BT_HCI_OP_HOST_BUFFER_SIZE, buf, NULL);
	if (err) {
		return err;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_SET_CTL_TO_HOST_FLOW, 1);
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_u8(buf, BT_HCI_CTL_TO_HOST_FLOW_ENABLE);
	return bt_hci_cmd_send_sync(BT_HCI_OP_SET_CTL_TO_HOST_FLOW, buf, NULL);
#else
	int err;

	/* Check if host flow control is actually supported */
	if (!BT_CMD_TEST(bt_dev.supported_commands, 10, 5)) {
		BT_WARN("Controller to host flow control not supported");
		return 0;
	}

	err = hci_api_set_host_buffer_size(CONFIG_BT_L2CAP_RX_MTU+sizeof(struct bt_l2cap_hdr), 0, CONFIG_BT_ACL_RX_COUNT, 0);
	if (err) {
		return err;
	}

	return hci_api_set_host_flow_enable(BT_HCI_CTL_TO_HOST_FLOW_ENABLE);
#endif
}
#endif /* CONFIG_BT_HCI_ACL_FLOW_CONTROL */

static void unpair(u8_t id, const bt_addr_le_t *addr)
{
	struct bt_keys *keys = NULL;
	struct bt_conn *conn = bt_conn_lookup_addr_le(id, addr);
	if (conn) {
		/* Clear the conn->le.keys pointer since we'll invalidate it,
		 * and don't want any subsequent code (like disconnected
		 * callbacks) accessing it.
		 */
		if (conn->type == BT_CONN_TYPE_LE) {
			keys = conn->le.keys;
			conn->le.keys = NULL;
		}

		bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		bt_conn_unref(conn);
	}

	if (IS_ENABLED(CONFIG_BT_BREDR)) {
		/* LE Public may indicate BR/EDR as well */
		if (addr->type == BT_ADDR_LE_PUBLIC) {
			bt_keys_link_key_clear_addr(&addr->a);
		}

		conn = bt_conn_lookup_addr_br(&(addr->a));
		if (conn) {
			bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
			bt_conn_unref(conn);
		}
	}

	if (IS_ENABLED(CONFIG_BT_SMP)) {
		if (!keys) {
			keys = bt_keys_find_addr(id, addr);
		}

		if (keys) {
			bt_keys_clear(keys);
		}
	}

	bt_gatt_clear(id, addr);
}

static void unpair_remote(const struct bt_bond_info *info, void *data)
{
	u8_t *id = (u8_t *) data;

	unpair(*id, &info->addr);
}

int bt_unpair(u8_t id, const bt_addr_le_t *addr)
{
	if (id >= CONFIG_BT_ID_MAX) {
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_BT_SMP) &&
	    (!addr || !bt_addr_le_cmp(addr, BT_ADDR_LE_ANY))) {
		bt_foreach_bond(id, unpair_remote, &id);
		return 0;
	}

	unpair(id, addr);
	return 0;
}

#endif /* CONFIG_BT_CONN */

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) || (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
static enum bt_security_err security_err_get(u8_t hci_err)
{
	switch (hci_err) {
	case BT_HCI_ERR_SUCCESS:
		return BT_SECURITY_ERR_SUCCESS;
	case BT_HCI_ERR_AUTH_FAIL:
		return BT_SECURITY_ERR_AUTH_FAIL;
	case BT_HCI_ERR_PIN_OR_KEY_MISSING:
		return BT_SECURITY_ERR_PIN_OR_KEY_MISSING;
	case BT_HCI_ERR_PAIRING_NOT_SUPPORTED:
		return BT_SECURITY_ERR_PAIR_NOT_SUPPORTED;
	case BT_HCI_ERR_PAIRING_NOT_ALLOWED:
		return BT_SECURITY_ERR_PAIR_NOT_ALLOWED;
	case BT_HCI_ERR_INVALID_PARAM:
		return BT_SECURITY_ERR_INVALID_PARAM;
	default:
		return BT_SECURITY_ERR_UNSPECIFIED;
	}
}

static void reset_pairing(struct bt_conn *conn)
{
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
	if (conn->type == BT_CONN_TYPE_BR) {
		atomic_clear_bit(conn->flags, BT_CONN_BR_PAIRING);
		atomic_clear_bit(conn->flags, BT_CONN_BR_PAIRING_INITIATOR);
		atomic_clear_bit(conn->flags, BT_CONN_BR_LEGACY_SECURE);
	}
#endif /* CONFIG_BT_BREDR */

	/* Reset required security level to current operational */
	conn->required_sec_level = conn->sec_level;
}
#endif /* (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) || (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR) */

#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
static int reject_conn(const bt_addr_t *bdaddr, u8_t reason)
{
	struct bt_hci_cp_reject_conn_req *cp;
	struct net_buf *buf;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_REJECT_CONN_REQ, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_copy(&cp->bdaddr, bdaddr);
	cp->reason = reason;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_REJECT_CONN_REQ, buf, NULL);
	if (err) {
		return err;
	}

	return 0;
}

static int accept_sco_conn(const bt_addr_t *bdaddr, struct bt_conn *sco_conn, int link_type)
{
	struct bt_hci_cp_accept_sync_conn_req *cp;
	struct net_buf *buf;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_ACCEPT_SYNC_CONN_REQ, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_copy(&cp->bdaddr, bdaddr);
	cp->pkt_type = sco_conn->sco.pkt_type;
	cp->tx_bandwidth = 0x00001f40;
	cp->rx_bandwidth = 0x00001f40;

	if (link_type == BT_HCI_SCO) {
		cp->max_latency = 0xffff;
		cp->retrans_effort = 0xff;
	} else {
		cp->max_latency = 0x000c;
		cp->retrans_effort = 0x01;
	}
	cp->content_format = BT_VOICE_CVSD_16BIT;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_ACCEPT_SYNC_CONN_REQ, buf, NULL);
	if (err) {
		return err;
	}

	return 0;
}

static int accept_conn(const bt_addr_t *bdaddr)
{
	struct bt_hci_cp_accept_conn_req *cp;
	struct net_buf *buf;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_ACCEPT_CONN_REQ, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_copy(&cp->bdaddr, bdaddr);
	cp->role = BT_HCI_ROLE_SLAVE;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_ACCEPT_CONN_REQ, buf, NULL);
	if (err) {
		return err;
	}

	return 0;
}

static void bt_esco_conn_req(struct bt_hci_evt_conn_request *evt)
{
	struct bt_conn *sco_conn;

	sco_conn = bt_conn_add_sco(&evt->bdaddr, evt->link_type);
	if (!sco_conn) {
		reject_conn(&evt->bdaddr, BT_HCI_ERR_INSUFFICIENT_RESOURCES);
		return;
	}

	if (accept_sco_conn(&evt->bdaddr, sco_conn, evt->link_type)) {
		BT_ERR("Error accepting connection from %s",
		       bt_addr_str(&evt->bdaddr));
		reject_conn(&evt->bdaddr, BT_HCI_ERR_UNSPECIFIED);
		bt_sco_cleanup(sco_conn);
		return;
	}

	sco_conn->role = BT_HCI_ROLE_SLAVE;
	bt_conn_set_state(sco_conn, BT_CONN_CONNECT);
	bt_conn_unref(sco_conn);
}

static void conn_req(struct net_buf *buf)
{
	struct bt_hci_evt_conn_request *evt = (void *)buf->data;
	struct bt_conn *conn;

	BT_DBG("conn req from %s, type 0x%02x", bt_addr_str(&evt->bdaddr),
	       evt->link_type);

	if (evt->link_type != BT_HCI_ACL) {
		bt_esco_conn_req(evt);
		return;
	}

	conn = bt_conn_add_br(&evt->bdaddr);
	if (!conn) {
		reject_conn(&evt->bdaddr, BT_HCI_ERR_INSUFFICIENT_RESOURCES);
		return;
	}

	accept_conn(&evt->bdaddr);
	conn->role = BT_HCI_ROLE_SLAVE;
	bt_conn_set_state(conn, BT_CONN_CONNECT);
	bt_conn_unref(conn);
}

static bool br_sufficient_key_size(struct bt_conn *conn)
{
	struct bt_hci_cp_read_encryption_key_size *cp;
	struct bt_hci_rp_read_encryption_key_size *rp;
	struct net_buf *buf, *rsp;
	u8_t key_size;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_READ_ENCRYPTION_KEY_SIZE,
				sizeof(*cp));
	if (!buf) {
		BT_ERR("Failed to allocate command buffer");
		return false;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(conn->handle);

	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_ENCRYPTION_KEY_SIZE,
				   buf, &rsp);
	if (err) {
		BT_ERR("Failed to read encryption key size (err %d)", err);
		return false;
	}

	if (rsp->len < sizeof(*rp)) {
		BT_ERR("Too small command complete for encryption key size");
		net_buf_unref(rsp);
		return false;
	}

	rp = (void *)rsp->data;
	key_size = rp->key_size;
	net_buf_unref(rsp);

	BT_DBG("Encryption key size is %u", key_size);

	if (conn->sec_level == BT_SECURITY_L4) {
		return key_size == BT_HCI_ENCRYPTION_KEY_SIZE_MAX;
	}

	return key_size >= BT_HCI_ENCRYPTION_KEY_SIZE_MIN;
}

static bool update_sec_level_br(struct bt_conn *conn)
{
	if (!conn->encrypt) {
		conn->sec_level = BT_SECURITY_L1;
		return true;
	}

	if (conn->br.link_key) {
		if (conn->br.link_key->flags & BT_LINK_KEY_AUTHENTICATED) {
			if (conn->encrypt == 0x02) {
				conn->sec_level = BT_SECURITY_L4;
			} else {
				conn->sec_level = BT_SECURITY_L3;
			}
		} else {
			conn->sec_level = BT_SECURITY_L2;
		}
	} else {
		BT_WARN("No BR/EDR link key found");
		conn->sec_level = BT_SECURITY_L2;
	}

	if (!br_sufficient_key_size(conn)) {
		BT_ERR("Encryption key size is not sufficient");
		bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL);
		return false;
	}

	if (conn->required_sec_level > conn->sec_level) {
		BT_ERR("Failed to set required security level");
		bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL);
		return false;
	}

	return true;
}

static void synchronous_conn_complete(struct net_buf *buf)
{
	struct bt_hci_evt_sync_conn_complete *evt = (void *)buf->data;
	struct bt_conn *sco_conn;
	u16_t handle = sys_le16_to_cpu(evt->handle);

	BT_DBG("status 0x%02x, handle %u, type 0x%02x", evt->status, handle,
	       evt->link_type);

	sco_conn = bt_conn_lookup_addr_sco(&evt->bdaddr);
	if (!sco_conn) {
		BT_ERR("Unable to find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	if (evt->status) {
		sco_conn->err = evt->status;
		bt_conn_set_state(sco_conn, BT_CONN_DISCONNECTED);
		bt_sco_cleanup(sco_conn);
		return;
	}

	sco_conn->handle = handle;
	bt_conn_set_state(sco_conn, BT_CONN_CONNECTED);
	bt_conn_unref(sco_conn);
}

static void conn_complete(struct net_buf *buf)
{
	struct bt_hci_evt_conn_complete *evt = (void *)buf->data;
	struct bt_conn *conn;
	struct bt_hci_cp_read_remote_features *cp;
	u16_t handle = sys_le16_to_cpu(evt->handle);

	BT_DBG("status 0x%02x, handle %u, type 0x%02x", evt->status, handle,
	       evt->link_type);

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Unable to find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	if (evt->status) {
		conn->err = evt->status;
		bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
		bt_conn_unref(conn);
		return;
	}

	conn->handle = handle;
	conn->err = 0U;
	conn->encrypt = evt->encr_enabled;

	if (!update_sec_level_br(conn)) {
		bt_conn_unref(conn);
		return;
	}

	bt_conn_set_state(conn, BT_CONN_CONNECTED);
	bt_conn_unref(conn);

	buf = bt_hci_cmd_create(BT_HCI_OP_READ_REMOTE_FEATURES, sizeof(*cp));
	if (!buf) {
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = evt->handle;

	bt_hci_cmd_send_sync(BT_HCI_OP_READ_REMOTE_FEATURES, buf, NULL);
}

static void pin_code_req(struct net_buf *buf)
{
	struct bt_hci_evt_pin_code_req *evt = (void *)buf->data;
	struct bt_conn *conn;

	BT_DBG("");

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	bt_conn_pin_code_req(conn);
	bt_conn_unref(conn);
}

static void link_key_notify(struct net_buf *buf)
{
	struct bt_hci_evt_link_key_notify *evt = (void *)buf->data;
	struct bt_conn *conn;

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	BT_DBG("%s, link type 0x%02x", bt_addr_str(&evt->bdaddr), evt->key_type);

	if (!conn->br.link_key) {
		conn->br.link_key = bt_keys_get_link_key(&evt->bdaddr);
	}
	if (!conn->br.link_key) {
		BT_ERR("Can't update keys for %s", bt_addr_str(&evt->bdaddr));
		bt_conn_unref(conn);
		return;
	}

	/* clear any old Link Key flags */
	conn->br.link_key->flags = 0U;

	switch (evt->key_type) {
	case BT_LK_COMBINATION:
		/*
		 * Setting Combination Link Key as AUTHENTICATED means it was
		 * successfully generated by 16 digits wide PIN code.
		 */
		if (atomic_test_and_clear_bit(conn->flags,
					      BT_CONN_BR_LEGACY_SECURE)) {
			conn->br.link_key->flags |= BT_LINK_KEY_AUTHENTICATED;
		}
		memcpy(conn->br.link_key->val, evt->link_key, 16);
		break;
	case BT_LK_AUTH_COMBINATION_P192:
		conn->br.link_key->flags |= BT_LINK_KEY_AUTHENTICATED;
		/* fall through */
	case BT_LK_UNAUTH_COMBINATION_P192:
		/* Mark no-bond so that link-key is removed on disconnection */
		if (bt_conn_ssp_get_auth(conn) < BT_HCI_DEDICATED_BONDING) {
			atomic_set_bit(conn->flags, BT_CONN_BR_NOBOND);
		}

		memcpy(conn->br.link_key->val, evt->link_key, 16);
		break;
	case BT_LK_AUTH_COMBINATION_P256:
		conn->br.link_key->flags |= BT_LINK_KEY_AUTHENTICATED;
		/* fall through */
	case BT_LK_UNAUTH_COMBINATION_P256:
		conn->br.link_key->flags |= BT_LINK_KEY_SC;

		/* Mark no-bond so that link-key is removed on disconnection */
		if (bt_conn_ssp_get_auth(conn) < BT_HCI_DEDICATED_BONDING) {
			atomic_set_bit(conn->flags, BT_CONN_BR_NOBOND);
		}

		memcpy(conn->br.link_key->val, evt->link_key, 16);
		break;
	default:
		BT_WARN("Unsupported Link Key type %u", evt->key_type);
		(void)memset(conn->br.link_key->val, 0,
			     sizeof(conn->br.link_key->val));
		break;
	}

#if (defined(CONFIG_BT_SETTINGS) && CONFIG_BT_SETTINGS)
	bt_br_keys_store(conn->br.link_key);
#endif

	bt_conn_unref(conn);
}

static void link_key_neg_reply(const bt_addr_t *bdaddr)
{
	struct bt_hci_cp_link_key_neg_reply *cp;
	struct net_buf *buf;

	BT_DBG("");

	buf = bt_hci_cmd_create(BT_HCI_OP_LINK_KEY_NEG_REPLY, sizeof(*cp));
	if (!buf) {
		BT_ERR("Out of command buffers");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_copy(&cp->bdaddr, bdaddr);
	bt_hci_cmd_send_sync(BT_HCI_OP_LINK_KEY_NEG_REPLY, buf, NULL);
}

static void link_key_reply(const bt_addr_t *bdaddr, const u8_t *lk)
{
	struct bt_hci_cp_link_key_reply *cp;
	struct net_buf *buf;

	BT_DBG("");

	buf = bt_hci_cmd_create(BT_HCI_OP_LINK_KEY_REPLY, sizeof(*cp));
	if (!buf) {
		BT_ERR("Out of command buffers");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_copy(&cp->bdaddr, bdaddr);
	memcpy(cp->link_key, lk, 16);
	bt_hci_cmd_send_sync(BT_HCI_OP_LINK_KEY_REPLY, buf, NULL);
}

static void link_key_req(struct net_buf *buf)
{
	struct bt_hci_evt_link_key_req *evt = (void *)buf->data;
	struct bt_conn *conn;

	BT_DBG("%s", bt_addr_str(&evt->bdaddr));

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		link_key_neg_reply(&evt->bdaddr);
		return;
	}

	if (!conn->br.link_key) {
		conn->br.link_key = bt_keys_find_link_key(&evt->bdaddr);
	}

	if (!conn->br.link_key) {
		link_key_neg_reply(&evt->bdaddr);
		bt_conn_unref(conn);
		return;
	}

	/*
	 * Enforce regenerate by controller stronger link key since found one
	 * in database not covers requested security level.
	 */
	if (!(conn->br.link_key->flags & BT_LINK_KEY_AUTHENTICATED) &&
	    conn->required_sec_level > BT_SECURITY_L2) {
		link_key_neg_reply(&evt->bdaddr);
		bt_conn_unref(conn);
		return;
	}

	link_key_reply(&evt->bdaddr, conn->br.link_key->val);
	bt_conn_unref(conn);
}

static void io_capa_neg_reply(const bt_addr_t *bdaddr, const u8_t reason)
{
	struct bt_hci_cp_io_capability_neg_reply *cp;
	struct net_buf *resp_buf;

	resp_buf = bt_hci_cmd_create(BT_HCI_OP_IO_CAPABILITY_NEG_REPLY,
				     sizeof(*cp));
	if (!resp_buf) {
		BT_ERR("Out of command buffers");
		return;
	}

	cp = net_buf_add(resp_buf, sizeof(*cp));
	bt_addr_copy(&cp->bdaddr, bdaddr);
	cp->reason = reason;
	bt_hci_cmd_send_sync(BT_HCI_OP_IO_CAPABILITY_NEG_REPLY, resp_buf, NULL);
}

static void io_capa_resp(struct net_buf *buf)
{
	struct bt_hci_evt_io_capa_resp *evt = (void *)buf->data;
	struct bt_conn *conn;

	BT_DBG("remote %s, IOcapa 0x%02x, auth 0x%02x",
	       bt_addr_str(&evt->bdaddr), evt->capability, evt->authentication);

	if (evt->authentication > BT_HCI_GENERAL_BONDING_MITM) {
		BT_ERR("Invalid remote authentication requirements");
		io_capa_neg_reply(&evt->bdaddr,
				  BT_HCI_ERR_UNSUPP_FEATURE_PARAM_VAL);
		return;
	}

	if (evt->capability > BT_IO_NO_INPUT_OUTPUT) {
		BT_ERR("Invalid remote io capability requirements");
		io_capa_neg_reply(&evt->bdaddr,
				  BT_HCI_ERR_UNSUPP_FEATURE_PARAM_VAL);
		return;
	}

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Unable to find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	conn->br.remote_io_capa = evt->capability;
	conn->br.remote_auth = evt->authentication;
	atomic_set_bit(conn->flags, BT_CONN_BR_PAIRING);
	bt_conn_unref(conn);
}

static void io_capa_req(struct net_buf *buf)
{
	struct bt_hci_evt_io_capa_req *evt = (void *)buf->data;
	struct net_buf *resp_buf;
	struct bt_conn *conn;
	struct bt_hci_cp_io_capability_reply *cp;
	u8_t auth;

	BT_DBG("");

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	resp_buf = bt_hci_cmd_create(BT_HCI_OP_IO_CAPABILITY_REPLY,
				     sizeof(*cp));
	if (!resp_buf) {
		BT_ERR("Out of command buffers");
		bt_conn_unref(conn);
		return;
	}

	/*
	 * Set authentication requirements when acting as pairing initiator to
	 * 'dedicated bond' with MITM protection set if local IO capa
	 * potentially allows it, and for acceptor, based on local IO capa and
	 * remote's authentication set.
	 */
	if (atomic_test_bit(conn->flags, BT_CONN_BR_PAIRING_INITIATOR)) {
		if (bt_conn_get_io_capa() != BT_IO_NO_INPUT_OUTPUT) {
			auth = BT_HCI_DEDICATED_BONDING_MITM;
		} else {
			auth = BT_HCI_DEDICATED_BONDING;
		}
	} else {
		auth = bt_conn_ssp_get_auth(conn);
	}

	cp = net_buf_add(resp_buf, sizeof(*cp));
	bt_addr_copy(&cp->bdaddr, &evt->bdaddr);
	cp->capability = bt_conn_get_io_capa();
	cp->authentication = auth;
	cp->oob_data = 0U;
	bt_hci_cmd_send_sync(BT_HCI_OP_IO_CAPABILITY_REPLY, resp_buf, NULL);
	bt_conn_unref(conn);
}

static void ssp_complete(struct net_buf *buf)
{
	struct bt_hci_evt_ssp_complete *evt = (void *)buf->data;
	struct bt_conn *conn;

	BT_DBG("status 0x%02x", evt->status);

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	bt_conn_ssp_auth_complete(conn, security_err_get(evt->status));
	if (evt->status) {
		bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL);
	}

	bt_conn_unref(conn);
}

static void user_confirm_req(struct net_buf *buf)
{
	struct bt_hci_evt_user_confirm_req *evt = (void *)buf->data;
	struct bt_conn *conn;

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	bt_conn_ssp_auth(conn, sys_le32_to_cpu(evt->passkey));
	bt_conn_unref(conn);
}

static void user_passkey_notify(struct net_buf *buf)
{
	struct bt_hci_evt_user_passkey_notify *evt = (void *)buf->data;
	struct bt_conn *conn;

	BT_DBG("");

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	bt_conn_ssp_auth(conn, sys_le32_to_cpu(evt->passkey));
	bt_conn_unref(conn);
}

static void user_passkey_req(struct net_buf *buf)
{
	struct bt_hci_evt_user_passkey_req *evt = (void *)buf->data;
	struct bt_conn *conn;

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	bt_conn_ssp_auth(conn, 0);
	bt_conn_unref(conn);
}

struct discovery_priv {
	u16_t clock_offset;
	u8_t pscan_rep_mode;
	u8_t resolving;
} __packed;

static int request_name(const bt_addr_t *addr, u8_t pscan, u16_t offset)
{
	struct bt_hci_cp_remote_name_request *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_REMOTE_NAME_REQUEST, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	bt_addr_copy(&cp->bdaddr, addr);
	cp->pscan_rep_mode = pscan;
	cp->reserved = 0x00; /* reserver, should be set to 0x00 */
	cp->clock_offset = offset;

	return bt_hci_cmd_send_sync(BT_HCI_OP_REMOTE_NAME_REQUEST, buf, NULL);
}

#define EIR_SHORT_NAME		0x08
#define EIR_COMPLETE_NAME	0x09

static bool eir_has_name(const u8_t *eir)
{
	int len = 240;

	while (len) {
		if (len < 2) {
			break;
		};

		/* Look for early termination */
		if (!eir[0]) {
			break;
		}

		/* Check if field length is correct */
		if (eir[0] > len - 1) {
			break;
		}

		switch (eir[1]) {
		case EIR_SHORT_NAME:
		case EIR_COMPLETE_NAME:
			if (eir[0] > 1) {
				return true;
			}
			break;
		default:
			break;
		}

		/* Parse next AD Structure */
		len -= eir[0] + 1;
		eir += eir[0] + 1;
	}

	return false;
}

static void report_discovery_results(void)
{
	bool resolving_names = false;
	int i;

	for (i = 0; i < discovery_results_count; i++) {
		struct discovery_priv *priv;

		priv = (struct discovery_priv *)&discovery_results[i]._priv;

		if (eir_has_name(discovery_results[i].eir)) {
			continue;
		}

		if (request_name(&discovery_results[i].addr,
				 priv->pscan_rep_mode, priv->clock_offset)) {
			continue;
		}

		priv->resolving = 1U;
		resolving_names = true;
	}

	if (resolving_names) {
		return;
	}

	atomic_clear_bit(bt_dev.flags, BT_DEV_INQUIRY);

	discovery_cb(discovery_results, discovery_results_count);

	discovery_cb = NULL;
	discovery_results = NULL;
	discovery_results_size = 0;
	discovery_results_count = 0;
}

static void inquiry_complete(struct net_buf *buf)
{
	struct bt_hci_evt_inquiry_complete *evt = (void *)buf->data;

	if (evt->status) {
		BT_ERR("Failed to complete inquiry");
	}

	report_discovery_results();
}

static struct bt_br_discovery_result *get_result_slot(const bt_addr_t *addr,
						      s8_t rssi)
{
	struct bt_br_discovery_result *result = NULL;
	size_t i;

	/* check if already present in results */
	for (i = 0; i < discovery_results_count; i++) {
		if (!bt_addr_cmp(addr, &discovery_results[i].addr)) {
			return &discovery_results[i];
		}
	}

	/* Pick a new slot (if available) */
	if (discovery_results_count < discovery_results_size) {
		bt_addr_copy(&discovery_results[discovery_results_count].addr,
			     addr);
		return &discovery_results[discovery_results_count++];
	}

	/* ignore if invalid RSSI */
	if (rssi == 0xff) {
		return NULL;
	}

	/*
	 * Pick slot with smallest RSSI that is smaller then passed RSSI
	 * TODO handle TX if present
	 */
	for (i = 0; i < discovery_results_size; i++) {
		if (discovery_results[i].rssi > rssi) {
			continue;
		}

		if (!result || result->rssi > discovery_results[i].rssi) {
			result = &discovery_results[i];
		}
	}

	if (result) {
		BT_DBG("Reusing slot (old %s rssi %d dBm)",
		       bt_addr_str(&result->addr), result->rssi);

		bt_addr_copy(&result->addr, addr);
	}

	return result;
}

static void inquiry_result_with_rssi(struct net_buf *buf)
{
	u8_t num_reports = net_buf_pull_u8(buf);

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_INQUIRY)) {
		return;
	}

	BT_DBG("number of results: %u", num_reports);

	while (num_reports--) {
		struct bt_hci_evt_inquiry_result_with_rssi *evt;
		struct bt_br_discovery_result *result;
		struct discovery_priv *priv;

		if (buf->len < sizeof(*evt)) {
			BT_ERR("Unexpected end to buffer");
			return;
		}

		evt = net_buf_pull_mem(buf, sizeof(*evt));
		BT_DBG("%s rssi %d dBm", bt_addr_str(&evt->addr), evt->rssi);

		result = get_result_slot(&evt->addr, evt->rssi);
		if (!result) {
			return;
		}

		priv = (struct discovery_priv *)&result->_priv;
		priv->pscan_rep_mode = evt->pscan_rep_mode;
		priv->clock_offset = evt->clock_offset;

		memcpy(result->cod, evt->cod, 3);
		result->rssi = evt->rssi;

		/* we could reuse slot so make sure EIR is cleared */
		(void)memset(result->eir, 0, sizeof(result->eir));
	}
}

static void extended_inquiry_result(struct net_buf *buf)
{
	struct bt_hci_evt_extended_inquiry_result *evt = (void *)buf->data;
	struct bt_br_discovery_result *result;
	struct discovery_priv *priv;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_INQUIRY)) {
		return;
	}

	BT_DBG("%s rssi %d dBm", bt_addr_str(&evt->addr), evt->rssi);

	result = get_result_slot(&evt->addr, evt->rssi);
	if (!result) {
		return;
	}

	priv = (struct discovery_priv *)&result->_priv;
	priv->pscan_rep_mode = evt->pscan_rep_mode;
	priv->clock_offset = evt->clock_offset;

	result->rssi = evt->rssi;
	memcpy(result->cod, evt->cod, 3);
	memcpy(result->eir, evt->eir, sizeof(result->eir));
}

static void remote_name_request_complete(struct net_buf *buf)
{
	struct bt_hci_evt_remote_name_req_complete *evt = (void *)buf->data;
	struct bt_br_discovery_result *result;
	struct discovery_priv *priv;
	int eir_len = 240;
	u8_t *eir;
	int i;

	result = get_result_slot(&evt->bdaddr, 0xff);
	if (!result) {
		return;
	}

	priv = (struct discovery_priv *)&result->_priv;
	priv->resolving = 0U;

	if (evt->status) {
		goto check_names;
	}

	eir = result->eir;

	while (eir_len) {
		if (eir_len < 2) {
			break;
		};

		/* Look for early termination */
		if (!eir[0]) {
			size_t name_len;

			eir_len -= 2;

			/* name is null terminated */
			name_len = strlen((const char *)evt->name);

			if (name_len > eir_len) {
				eir[0] = eir_len + 1;
				eir[1] = EIR_SHORT_NAME;
			} else {
				eir[0] = name_len + 1;
				eir[1] = EIR_SHORT_NAME;
			}

			memcpy(&eir[2], evt->name, eir[0] - 1);

			break;
		}

		/* Check if field length is correct */
		if (eir[0] > eir_len - 1) {
			break;
		}

		/* next EIR Structure */
		eir_len -= eir[0] + 1;
		eir += eir[0] + 1;
	}

check_names:
	/* if still waiting for names */
	for (i = 0; i < discovery_results_count; i++) {
		struct discovery_priv *priv;

		priv = (struct discovery_priv *)&discovery_results[i]._priv;

		if (priv->resolving) {
			return;
		}
	}

	/* all names resolved, report discovery results */
	atomic_clear_bit(bt_dev.flags, BT_DEV_INQUIRY);

	discovery_cb(discovery_results, discovery_results_count);

	discovery_cb = NULL;
	discovery_results = NULL;
	discovery_results_size = 0;
	discovery_results_count = 0;
}

static void link_encr(const u16_t handle)
{
	struct bt_hci_cp_set_conn_encrypt *encr;
	struct net_buf *buf;

	BT_DBG("");

	buf = bt_hci_cmd_create(BT_HCI_OP_SET_CONN_ENCRYPT, sizeof(*encr));
	if (!buf) {
		BT_ERR("Out of command buffers");
		return;
	}

	encr = net_buf_add(buf, sizeof(*encr));
	encr->handle = sys_cpu_to_le16(handle);
	encr->encrypt = 0x01;

	bt_hci_cmd_send_sync(BT_HCI_OP_SET_CONN_ENCRYPT, buf, NULL);
}

static void auth_complete(struct net_buf *buf)
{
	struct bt_hci_evt_auth_complete *evt = (void *)buf->data;
	struct bt_conn *conn;
	u16_t handle = sys_le16_to_cpu(evt->handle);

	BT_DBG("status 0x%02x, handle %u", evt->status, handle);

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Can't find conn for handle %u", handle);
		return;
	}

	if (evt->status) {
		if (conn->state == BT_CONN_CONNECTED) {
			/*
			 * Inform layers above HCI about non-zero authentication
			 * status to make them able cleanup pending jobs.
			 */
			bt_l2cap_encrypt_change(conn, evt->status);
		}
		reset_pairing(conn);
	} else {
		link_encr(handle);
	}

	bt_conn_unref(conn);
}

static void read_remote_features_complete(struct net_buf *buf)
{
	struct bt_hci_evt_remote_features *evt = (void *)buf->data;
	u16_t handle = sys_le16_to_cpu(evt->handle);
	struct bt_hci_cp_read_remote_ext_features *cp;
	struct bt_conn *conn;

	BT_DBG("status 0x%02x handle %u", evt->status, handle);

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Can't find conn for handle %u", handle);
		return;
	}

	if (evt->status) {
		goto done;
	}

	memcpy(conn->br.features[0], evt->features, sizeof(evt->features));

	if (!BT_FEAT_EXT_FEATURES(conn->br.features)) {
		goto done;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_READ_REMOTE_EXT_FEATURES,
				sizeof(*cp));
	if (!buf) {
		goto done;
	}

	/* Read remote host features (page 1) */
	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = evt->handle;
	cp->page = 0x01;

	bt_hci_cmd_send_sync(BT_HCI_OP_READ_REMOTE_EXT_FEATURES, buf, NULL);

done:
	bt_conn_unref(conn);
}

static void read_remote_ext_features_complete(struct net_buf *buf)
{
	struct bt_hci_evt_remote_ext_features *evt = (void *)buf->data;
	u16_t handle = sys_le16_to_cpu(evt->handle);
	struct bt_conn *conn;

	BT_DBG("status 0x%02x handle %u", evt->status, handle);

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Can't find conn for handle %u", handle);
		return;
	}

	if (!evt->status && evt->page == 0x01) {
		memcpy(conn->br.features[1], evt->features,
		       sizeof(conn->br.features[1]));
	}

	bt_conn_unref(conn);
}

static void role_change(struct net_buf *buf)
{
	struct bt_hci_evt_role_change *evt = (void *)buf->data;
	struct bt_conn *conn;

	BT_DBG("status 0x%02x role %u addr %s", evt->status, evt->role,
	       bt_addr_str(&evt->bdaddr));

	if (evt->status) {
		return;
	}

	conn = bt_conn_lookup_addr_br(&evt->bdaddr);
	if (!conn) {
		BT_ERR("Can't find conn for %s", bt_addr_str(&evt->bdaddr));
		return;
	}

	if (evt->role) {
		conn->role = BT_CONN_ROLE_SLAVE;
	} else {
		conn->role = BT_CONN_ROLE_MASTER;
	}

	bt_conn_unref(conn);
}
#endif /* CONFIG_BT_BREDR */

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
static int le_set_privacy_mode(const bt_addr_le_t *addr, u8_t mode)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_privacy_mode cp;
	struct net_buf *buf;
	int err;

	/* Check if set privacy mode command is supported */
	if (!BT_CMD_TEST(bt_dev.supported_commands, 39, 2)) {
		BT_WARN("Set privacy mode command is not supported");
		return 0;
	}

	BT_DBG("addr %s mode 0x%02x", bt_addr_le_str(addr), mode);

	bt_addr_le_copy(&cp.id_addr, addr);
	cp.mode = mode;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_PRIVACY_MODE, sizeof(cp));
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_mem(buf, &cp, sizeof(cp));
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_PRIVACY_MODE, buf, NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_PRIVACY_MODE, buf, NULL);
#endif
	if (err) {
		return err;
	}

	return 0;
#else
	/* Check if set privacy mode command is supported */
	if (!BT_CMD_TEST(bt_dev.supported_commands, 39, 2)) {
		BT_WARN("Set privacy mode command is not supported");
		return 0;
	}

	BT_DBG("addr %s mode 0x%02x", bt_addr_le_str(addr), mode);

	return hci_api_le_set_privacy_mode(addr->type, (uint8_t *)addr->a.val, mode);
#endif
}

static int addr_res_enable(u8_t enable)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct net_buf *buf;

	BT_DBG("%s", enable ? "enabled" : "disabled");

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_ADDR_RES_ENABLE, 1);
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_u8(buf, enable);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_ADDR_RES_ENABLE,
				    buf, NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_ADDR_RES_ENABLE,
				    buf, NULL);
#endif
#else
	BT_DBG("%s", enable ? "enabled" : "disabled");

	return hci_api_le_set_addr_res_enable(enable);
#endif
}

static int hci_id_add(u8_t id, const bt_addr_le_t *addr, u8_t peer_irk[16])
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_add_dev_to_rl *cp;
	struct net_buf *buf;

	BT_DBG("addr %s", bt_addr_le_str(addr));

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_ADD_DEV_TO_RL, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_le_copy(&cp->peer_id_addr, addr);
	memcpy(cp->peer_irk, peer_irk, 16);

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	memcpy(cp->local_irk, bt_dev.irk[id], 16);
#else
	(void)memset(cp->local_irk, 0, 16);
#endif
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_ADD_DEV_TO_RL, buf, NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_ADD_DEV_TO_RL, buf, NULL);
#endif
#else
	u8_t local_irk[16] = {0};
	BT_DBG("addr %s", bt_addr_le_str(addr));

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	memcpy(local_irk, bt_dev.irk[id], 16);
#else
	(void)memset(local_irk, 0, 16);
#endif

	return hci_api_le_add_dev_to_rl(addr->type, (uint8_t *)addr->a.val, peer_irk, local_irk);
#endif
}

void bt_id_add(struct bt_keys *keys)
{
	struct bt_conn *conn;
	int err;

	BT_DBG("addr %s", bt_addr_le_str(&keys->addr));

	/* Nothing to be done if host-side resolving is used */
	if (!bt_dev.le.rl_size || bt_dev.le.rl_entries > bt_dev.le.rl_size) {
		bt_dev.le.rl_entries++;
		keys->state |= BT_KEYS_ID_ADDED;
		return;
	}

	conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL, BT_CONN_CONNECT);
	if (conn) {
		pending_id_keys_update_set(keys, BT_KEYS_ID_PENDING_ADD);
		bt_conn_unref(conn);
		return;
	}

	if (IS_ENABLED(CONFIG_BT_EXT_ADV)) {
		bool adv_enabled = false;

		bt_adv_foreach(adv_is_limited_enabled, &adv_enabled);
		if (adv_enabled) {
			pending_id_keys_update_set(keys,
						   BT_KEYS_ID_PENDING_ADD);
			return;
		}
	}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	bool scan_enabled = atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING);

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) && scan_enabled &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED)) {
		pending_id_keys_update_set(keys, BT_KEYS_ID_PENDING_ADD);
	}
#endif

	bt_adv_foreach(adv_pause_enabled, NULL);

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	if (scan_enabled) {
		set_le_scan_enable(BT_HCI_LE_SCAN_DISABLE);
	}
#endif /* CONFIG_BT_OBSERVER */

	/* If there are any existing entries address resolution will be on */
	if (bt_dev.le.rl_entries) {
		err = addr_res_enable(BT_HCI_ADDR_RES_DISABLE);
		if (err) {
			BT_WARN("Failed to disable address resolution");
			goto done;
		}
	}

	if (bt_dev.le.rl_entries == bt_dev.le.rl_size) {
		BT_WARN("Resolving list size exceeded. Switching to host.");
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_CLEAR_RL, NULL, NULL);
#else
		err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_CLEAR_RL, NULL, NULL);
#endif
#else
		err = hci_api_le_clear_rl();
#endif
		if (err) {
			BT_ERR("Failed to clear resolution list");
			goto done;
		}

		bt_dev.le.rl_entries++;
		keys->state |= BT_KEYS_ID_ADDED;

		goto done;
	}

	err = hci_id_add(keys->id, &keys->addr, keys->irk.val);
	if (err) {
		BT_ERR("Failed to add IRK to controller");
		goto done;
	}

	bt_dev.le.rl_entries++;
	keys->state |= BT_KEYS_ID_ADDED;

	/*
	 * According to Core Spec. 5.0 Vol 1, Part A 5.4.5 Privacy Feature
	 *
	 * By default, network privacy mode is used when private addresses are
	 * resolved and generated by the Controller, so advertising packets from
	 * peer devices that contain private addresses will only be accepted.
	 * By changing to the device privacy mode device is only concerned about
	 * its privacy and will accept advertising packets from peer devices
	 * that contain their identity address as well as ones that contain
	 * a private address, even if the peer device has distributed its IRK in
	 * the past.
	 */
	err = le_set_privacy_mode(&keys->addr, BT_HCI_LE_PRIVACY_MODE_DEVICE);
	if (err) {
		BT_ERR("Failed to set privacy mode");
		goto done;
	}

done:
	addr_res_enable(BT_HCI_ADDR_RES_ENABLE);

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	if (scan_enabled) {
		set_le_scan_enable(BT_HCI_LE_SCAN_ENABLE);
	}
#endif /* CONFIG_BT_OBSERVER */

	bt_adv_foreach(adv_unpause_enabled, NULL);
}

static void keys_add_id(struct bt_keys *keys, void *data)
{
	if (keys->state & BT_KEYS_ID_ADDED) {
		hci_id_add(keys->id, &keys->addr, keys->irk.val);
	}
}

static int hci_id_del(const bt_addr_le_t *addr)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_rem_dev_from_rl *cp;
	struct net_buf *buf;

	BT_DBG("addr %s", bt_addr_le_str(addr));

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_REM_DEV_FROM_RL, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_le_copy(&cp->peer_id_addr, addr);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_REM_DEV_FROM_RL, buf, NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_REM_DEV_FROM_RL, buf, NULL);
#endif
#else
	return hci_api_le_remove_dev_from_rl(addr->type, addr->a.val);
#endif
}

void bt_id_del(struct bt_keys *keys)
{
	struct bt_conn *conn;
	int err;

	BT_DBG("addr %s", bt_addr_le_str(&keys->addr));

	if (!bt_dev.le.rl_size ||
	    bt_dev.le.rl_entries > bt_dev.le.rl_size + 1) {
		bt_dev.le.rl_entries--;
		keys->state &= ~BT_KEYS_ID_ADDED;
		return;
	}

	conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL, BT_CONN_CONNECT);
	if (conn) {
		pending_id_keys_update_set(keys, BT_KEYS_ID_PENDING_DEL);
		bt_conn_unref(conn);
		return;
	}

	if (IS_ENABLED(CONFIG_BT_EXT_ADV)) {
		bool adv_enabled = false;

		bt_adv_foreach(adv_is_limited_enabled, &adv_enabled);
		if (adv_enabled) {
			pending_id_keys_update_set(keys,
						   BT_KEYS_ID_PENDING_ADD);
			return;
		}
	}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	bool scan_enabled = atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING);

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) && scan_enabled &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED)) {
		pending_id_keys_update_set(keys, BT_KEYS_ID_PENDING_DEL);
	}
#endif /* CONFIG_BT_OBSERVER */

	bt_adv_foreach(adv_pause_enabled, NULL);

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	if (scan_enabled) {
		set_le_scan_enable(BT_HCI_LE_SCAN_DISABLE);
	}
#endif /* CONFIG_BT_OBSERVER */

	err = addr_res_enable(BT_HCI_ADDR_RES_DISABLE);
	if (err) {
		BT_ERR("Disabling address resolution failed (err %d)", err);
		goto done;
	}

	/* We checked size + 1 earlier, so here we know we can fit again */
	if (bt_dev.le.rl_entries > bt_dev.le.rl_size) {
		bt_dev.le.rl_entries--;
		keys->state &= ~BT_KEYS_ID_ADDED;
		if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
		    IS_ENABLED(CONFIG_BT_PRIVACY)) {
			bt_keys_foreach(BT_KEYS_ALL, keys_add_id, NULL);
		} else {
			bt_keys_foreach(BT_KEYS_IRK, keys_add_id, NULL);
		}
		goto done;
	}

	err = hci_id_del(&keys->addr);
	if (err) {
		BT_ERR("Failed to remove IRK from controller");
		goto done;
	}

	bt_dev.le.rl_entries--;
	keys->state &= ~BT_KEYS_ID_ADDED;

done:
	/* Only re-enable if there are entries to do resolving with */
	if (bt_dev.le.rl_entries) {
		addr_res_enable(BT_HCI_ADDR_RES_ENABLE);
	}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	if (scan_enabled) {
		set_le_scan_enable(BT_HCI_LE_SCAN_ENABLE);
	}
#endif /* CONFIG_BT_OBSERVER */

	bt_adv_foreach(adv_unpause_enabled, NULL);
}

static void update_sec_level(struct bt_conn *conn)
{
	if (!conn->encrypt) {
		conn->sec_level = BT_SECURITY_L1;
		return;
	}

	if (conn->le.keys && (conn->le.keys->flags & BT_KEYS_AUTHENTICATED)) {
		if (conn->le.keys->flags & BT_KEYS_SC &&
		    conn->le.keys->enc_size == BT_SMP_MAX_ENC_KEY_SIZE) {
			conn->sec_level = BT_SECURITY_L4;
		} else {
			conn->sec_level = BT_SECURITY_L3;
		}
	} else {
		conn->sec_level = BT_SECURITY_L2;
	}

	if (conn->required_sec_level > conn->sec_level) {
		BT_ERR("Failed to set required security level");
		bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL);
	}
}
#endif /* CONFIG_BT_SMP */

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) || (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
static void hci_encrypt_change(struct net_buf *buf)
{
	struct bt_hci_evt_encrypt_change *evt = (void *)buf->data;
	u16_t handle = sys_le16_to_cpu(evt->handle);
	struct bt_conn *conn;

	BT_DBG("status 0x%02x handle %u encrypt 0x%02x", evt->status, handle,
	       evt->encrypt);

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to look up conn with handle %u", handle);
		return;
	}

	if (evt->status) {
		reset_pairing(conn);
		bt_l2cap_encrypt_change(conn, evt->status);
		bt_conn_security_changed(conn, security_err_get(evt->status));
		bt_conn_unref(conn);
		return;
	}

	conn->encrypt = evt->encrypt;

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	if (conn->type == BT_CONN_TYPE_LE) {
		/*
		 * we update keys properties only on successful encryption to
		 * avoid losing valid keys if encryption was not successful.
		 *
		 * Update keys with last pairing info for proper sec level
		 * update. This is done only for LE transport, for BR/EDR keys
		 * are updated on HCI 'Link Key Notification Event'
		 */
		if (conn->encrypt) {
			bt_smp_update_keys(conn);
		}
		update_sec_level(conn);
	}
#endif /* CONFIG_BT_SMP */
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
	if (conn->type == BT_CONN_TYPE_BR) {
		if (!update_sec_level_br(conn)) {
			bt_conn_unref(conn);
			return;
		}

		if (IS_ENABLED(CONFIG_BT_SMP)) {
			/*
			 * Start SMP over BR/EDR if we are pairing and are
			 * master on the link
			 */
			if (atomic_test_bit(conn->flags, BT_CONN_BR_PAIRING) &&
			    conn->role == BT_CONN_ROLE_MASTER) {
				bt_smp_br_send_pairing_req(conn);
			}
		}
	}
#endif /* CONFIG_BT_BREDR */
	reset_pairing(conn);

	bt_l2cap_encrypt_change(conn, evt->status);
	bt_conn_security_changed(conn, BT_SECURITY_ERR_SUCCESS);

	bt_conn_unref(conn);
}

static void hci_encrypt_key_refresh_complete(struct net_buf *buf)
{
	struct bt_hci_evt_encrypt_key_refresh_complete *evt = (void *)buf->data;
	struct bt_conn *conn;
	u16_t handle;

	handle = sys_le16_to_cpu(evt->handle);

	BT_DBG("status 0x%02x handle %u", evt->status, handle);

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to look up conn with handle %u", handle);
		return;
	}

	if (evt->status) {
		reset_pairing(conn);
		bt_l2cap_encrypt_change(conn, evt->status);
		bt_conn_security_changed(conn, security_err_get(evt->status));
		bt_conn_unref(conn);
		return;
	}

	/*
	 * Update keys with last pairing info for proper sec level update.
	 * This is done only for LE transport. For BR/EDR transport keys are
	 * updated on HCI 'Link Key Notification Event', therefore update here
	 * only security level based on available keys and encryption state.
	 */
#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	if (conn->type == BT_CONN_TYPE_LE) {
		bt_smp_update_keys(conn);
		update_sec_level(conn);
	}
#endif /* CONFIG_BT_SMP */
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
	if (conn->type == BT_CONN_TYPE_BR) {
		if (!update_sec_level_br(conn)) {
			bt_conn_unref(conn);
			return;
		}
	}
#endif /* CONFIG_BT_BREDR */

	reset_pairing(conn);
	bt_l2cap_encrypt_change(conn, evt->status);
	bt_conn_security_changed(conn, BT_SECURITY_ERR_SUCCESS);
	bt_conn_unref(conn);
}
#endif /* CONFIG_BT_SMP || CONFIG_BT_BREDR */

#if (defined(CONFIG_BT_REMOTE_VERSION) && CONFIG_BT_REMOTE_VERSION)
static void bt_hci_evt_read_remote_version_complete(struct net_buf *buf)
{
	struct bt_hci_evt_remote_version_info *evt;
	struct bt_conn *conn;

	evt = net_buf_pull_mem(buf, sizeof(*evt));
	conn = bt_conn_lookup_handle(evt->handle);
	if (!conn) {
		BT_ERR("No connection for handle %u", evt->handle);
		return;
	}

	if (!evt->status) {
		conn->rv.version = evt->version;
		conn->rv.manufacturer = sys_le16_to_cpu(evt->manufacturer);
		conn->rv.subversion = sys_le16_to_cpu(evt->subversion);
	}

	atomic_set_bit(conn->flags, BT_CONN_AUTO_VERSION_INFO);

	if (IS_ENABLED(CONFIG_BT_REMOTE_INFO)) {
		/* Remote features is already present */
		notify_remote_info(conn);
	}

	/* Continue with auto-initiated procedures */
	conn_auto_initiate(conn);

	bt_conn_unref(conn);
}
#endif /* CONFIG_BT_REMOTE_VERSION */

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
static void le_ltk_neg_reply(u16_t handle)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_ltk_req_neg_reply *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_LTK_REQ_NEG_REPLY, sizeof(*cp));
	if (!buf) {
		BT_ERR("Out of command buffers");

		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_hci_cmd_send_cb(BT_HCI_OP_LE_LTK_REQ_NEG_REPLY, buf, NULL);
#else
	bt_hci_cmd_send(BT_HCI_OP_LE_LTK_REQ_NEG_REPLY, buf);
#endif
#else
	hci_api_le_enctypt_ltk_req_neg_reply(handle);
#endif
}

static void le_ltk_reply(u16_t handle, u8_t *ltk)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_ltk_req_reply *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_LTK_REQ_REPLY,
				sizeof(*cp));
	if (!buf) {
		BT_ERR("Out of command buffers");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
	memcpy(cp->ltk, ltk, sizeof(cp->ltk));
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_hci_cmd_send_cb(BT_HCI_OP_LE_LTK_REQ_REPLY, buf, NULL);
#else
	bt_hci_cmd_send(BT_HCI_OP_LE_LTK_REQ_REPLY, buf);
#endif
#else
	hci_api_le_enctypt_ltk_req_reply(handle, ltk);
#endif
}

static void le_ltk_request(struct net_buf *buf)
{
	struct bt_hci_evt_le_ltk_request *evt = (void *)buf->data;
	struct bt_conn *conn;
	u16_t handle;
	u8_t ltk[16];

	handle = sys_le16_to_cpu(evt->handle);

	BT_DBG("handle %u", handle);

	conn = bt_conn_lookup_handle(handle);
	if (!conn) {
		BT_ERR("Unable to lookup conn for handle %u", handle);
		return;
	}

	if (bt_smp_request_ltk(conn, evt->rand, evt->ediv, ltk)) {
		le_ltk_reply(handle, ltk);
	} else {
		le_ltk_neg_reply(handle);
	}

	bt_conn_unref(conn);
}
#endif /* CONFIG_BT_SMP */

#if (defined(CONFIG_BT_ECC) && CONFIG_BT_ECC)
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
static void le_pkey_complete(struct net_buf *buf)
{
	struct bt_hci_evt_le_p256_public_key_complete *evt = (void *)buf->data;
	struct bt_pub_key_cb *cb;

	BT_DBG("status: 0x%02x", evt->status);

	atomic_clear_bit(bt_dev.flags, BT_DEV_PUB_KEY_BUSY);

	if (!evt->status) {
		memcpy(pub_key, evt->key, 64);
		atomic_set_bit(bt_dev.flags, BT_DEV_HAS_PUB_KEY);
	}

	for (cb = pub_key_cb; cb; cb = cb->_next) {
		cb->func(evt->status ? NULL : pub_key);
	}

	pub_key_cb = NULL;
}

static void le_dhkey_complete(struct net_buf *buf)
{
	struct bt_hci_evt_le_generate_dhkey_complete *evt = (void *)buf->data;

	BT_DBG("status: 0x%02x", evt->status);

	if (dh_key_cb) {
		dh_key_cb(evt->status ? NULL : evt->dhkey);
		dh_key_cb = NULL;
	}
}
#else
int hci_api_le_event_pkey_complete(u8_t status, u8_t key[64])
{
	struct bt_pub_key_cb *cb;

	BT_DBG("status: 0x%02x", status);

	atomic_clear_bit(bt_dev.flags, BT_DEV_PUB_KEY_BUSY);

	if (!status) {
		memcpy(pub_key, key, 64);
		atomic_set_bit(bt_dev.flags, BT_DEV_HAS_PUB_KEY);
	}

	for (cb = pub_key_cb; cb; cb = cb->_next) {
		cb->func(status ? NULL : key);
	}

	pub_key_cb = NULL;

	return 0;
}

static void le_pkey_complete(struct net_buf *buf)
{
	struct bt_hci_evt_le_p256_public_key_complete *evt = (void *)buf->data;

	hci_api_le_event_pkey_complete(evt->status, evt->key);
}

int hci_api_le_event_dhkey_complete(u8_t status, u8_t dhkey[32])
{
	BT_DBG("status: 0x%x", status);

	if (dh_key_cb) {
		dh_key_cb(status ? NULL : dhkey);
		dh_key_cb = NULL;
	}

	return 0;
}

static void le_dhkey_complete(struct net_buf *buf)
{
	struct bt_hci_evt_le_generate_dhkey_complete *evt = (void *)buf->data;

	hci_api_le_event_dhkey_complete(evt->status, evt->dhkey);
}

#endif

#endif /* CONFIG_BT_ECC */


#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
static void hci_reset_complete(struct net_buf *buf)
{
	u8_t status = buf->data[0];
	atomic_t flags;

	BT_DBG("status 0x%02x", status);

	if (status) {
		return;
	}

	scan_dev_found_cb = NULL;
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
	discovery_cb = NULL;
	discovery_results = NULL;
	discovery_results_size = 0;
	discovery_results_count = 0;
#endif /* CONFIG_BT_BREDR */

	flags = (atomic_get(bt_dev.flags) & BT_DEV_PERSISTENT_FLAGS);
	atomic_set(bt_dev.flags, flags);
}
#else
static void hci_reset_complete()
{
	atomic_t flags;

	scan_dev_found_cb = NULL;
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
	discovery_cb = NULL;
	discovery_results = NULL;
	discovery_results_size = 0;
	discovery_results_count = 0;
#endif /* CONFIG_BT_BREDR */

	flags = (atomic_get(bt_dev.flags) & BT_DEV_PERSISTENT_FLAGS);
	atomic_set(bt_dev.flags, flags);
}
#endif

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
static void hci_cmd_done(u16_t opcode, u8_t status, struct net_buf *buf)
{
	BT_DBG("opcode 0x%04x status 0x%02x buf %p", opcode, status, buf);

	if (net_buf_pool_get(buf->pool_id) != &hci_cmd_pool) {
		BT_WARN("opcode 0x%04x pool id %u pool %p != &hci_cmd_pool %p",
			opcode, buf->pool_id, net_buf_pool_get(buf->pool_id),
			&hci_cmd_pool);
		return;
	}

	if (cmd(buf)->opcode != opcode) {
		BT_WARN("OpCode 0x%04x completed instead of expected 0x%04x",
			opcode, cmd(buf)->opcode);
		return;
	}
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	if (cmd(buf)->state.target && !status) {
		struct cmd_state_set *update = &(cmd(buf)->state);

		atomic_set_bit_to(update->target, update->bit, update->val);
	}

	cmd(buf)->status = status;

	if (status)
	{
		//BT_ERR("opcode 0x%04x status 0x%02x buf %p", opcode, status, buf);
	}
#else
	if (cmd(buf)->state && !status) {
		struct cmd_state_set *update = cmd(buf)->state;

		atomic_set_bit_to(update->target, update->bit, update->val);
	}

	/* If the command was synchronous wake up bt_hci_cmd_send_sync() */
	if (cmd(buf)->sync) {
		cmd(buf)->status = status;
		k_sem_give(cmd(buf)->sync);
	}
#endif
}

static void hci_cmd_complete(struct net_buf *buf)
{
	struct bt_hci_evt_cmd_complete *evt;
	u8_t status, ncmd;
	u16_t opcode;

	evt = net_buf_pull_mem(buf, sizeof(*evt));
	ncmd = evt->ncmd;
	opcode = sys_le16_to_cpu(evt->opcode);

	BT_DBG("opcode 0x%04x", opcode);

	/* All command return parameters have a 1-byte status in the
	 * beginning, so we can safely make this generalization.
	 */
	status = buf->data[0];

	hci_cmd_done(opcode, status, buf);

	/* Allow next command to be sent */
	if (ncmd) {
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		k_delayed_work_cancel(&bt_dev.cmd_sent_work);
		atomic_inc(&bt_dev.ncmd);

		struct net_buf *buf = net_buf_get(&bt_dev.cmd_tx_pending_queue, 0);
		if (buf)
		{
			net_buf_put(&bt_dev.cmd_tx_queue, buf);
			BT_DBG("reput %p to cmd_tx_queue");
		}
#else
		k_sem_give(&bt_dev.ncmd_sem);
#endif
	}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)

	/* default handler first */
	int err = handle_hci_cmd_done_event(buf, opcode, status);
	if (err)
	{
		net_buf_unref(buf);
		BT_ERR("OpCode 0x%04x complete process err %d", opcode, err);
	}

	/* notify FSM with  BT_FSM_EV_DEFINE(HCI_CMD, CMD_SENT) event */
	if (cmd(buf)->fsm_handle != BT_FSM_HANDLE_UNUSED)
	{
		net_buf_ref(buf);
		err = bt_fsm_hci_cmd_send(buf, NULL, BT_FSM_HANDLE_UNUSED, BT_FSM_EV_DEFINE(HCI_CMD, CMD_CMPLETE));
		if (err)
		{
			net_buf_unref(buf);
			BT_ERR("OpCode 0x%04x complete process err %d", opcode, err);
		}
	}

	/* call hci command sent callback function */
	if (cmd(buf)->cb.func) {
		net_buf_ref(buf);
		err = cmd(buf)->cb.func(opcode, cmd(buf)->status, buf, cmd(buf)->cb.args);
		if (err)
		{
			net_buf_unref(buf);
			BT_ERR("OpCode 0x%04x complete process err %d", opcode, err);
		}
	}
	net_buf_unref(buf);
#endif
}

static void hci_cmd_status(struct net_buf *buf)
{
	struct bt_hci_evt_cmd_status *evt;
	u16_t opcode;
	u8_t ncmd;

	evt = net_buf_pull_mem(buf, sizeof(*evt));
	opcode = sys_le16_to_cpu(evt->opcode);
	ncmd = evt->ncmd;

	BT_DBG("opcode 0x%04x", opcode);

	hci_cmd_done(opcode, evt->status, buf);

	/* Allow next command to be sent */
	if (ncmd) {
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		k_delayed_work_cancel(&bt_dev.cmd_sent_work);
		atomic_inc(&bt_dev.ncmd);
		struct net_buf *buf = net_buf_get(&bt_dev.cmd_tx_pending_queue, 0);
		if (buf)
		{
			net_buf_put(&bt_dev.cmd_tx_queue, buf);
			BT_DBG("reput %p to cmd_tx_queue");
		}
#else
		k_sem_give(&bt_dev.ncmd_sem);
#endif
	}
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	/* default handler first */
	int err = handle_hci_cmd_done_event(buf, opcode, evt->status);
	if (err)
	{
		BT_ERR("OpCode 0x%04x complete process err %d", opcode, err);
	}

	/* notify FSM with  BT_FSM_EV_DEFINE(HCI_CMD, CMD_SENT) event */
	if (cmd(buf)->fsm_handle != BT_FSM_HANDLE_UNUSED)
	{
		net_buf_ref(buf);
		bt_fsm_hci_cmd_send(buf, NULL, BT_FSM_HANDLE_UNUSED, BT_FSM_EV_DEFINE(HCI_CMD, CMD_CMPLETE));
	}

	/* call hci command sent callback function */
	if (cmd(buf)->cb.func) {
		net_buf_ref(buf);
		cmd(buf)->cb.func(opcode, cmd(buf)->status, buf, cmd(buf)->cb.args);
	}

	net_buf_unref(buf);
#endif
}
#endif

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
static int le_scan_set_random_addr(bool active_scan, u8_t *own_addr_type)
{
	int err;

	if (IS_ENABLED(CONFIG_BT_PRIVACY)) {
		err = le_set_private_addr(BT_ID_DEFAULT);
		if (err) {
			return err;
		}

		if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
			*own_addr_type = BT_HCI_OWN_ADDR_RPA_OR_RANDOM;
		} else {
			*own_addr_type = BT_ADDR_LE_RANDOM;
		}
	} else {
		struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();

		*own_addr_type = bt_dev.id_addr[0].type;

		/* Use NRPA unless identity has been explicitly requested
		 * (through Kconfig).
		 * Use same RPA as legacy advertiser if advertising.
		 */
		if (!IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY) &&
		    !(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
		      BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) &&
		    adv && !atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
			err = le_set_private_addr(BT_ID_DEFAULT);
			if (err) {
				return err;
			}

			*own_addr_type = BT_ADDR_LE_RANDOM;
		} else if (IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY) &&
			   *own_addr_type == BT_ADDR_LE_RANDOM) {
			/* If scanning with Identity Address we must set the
			 * random identity address for both active and passive
			 * scanner in order to receive adv reports that are
			 * directed towards this identity.
			 */
			err = set_random_address(&bt_dev.id_addr[0].a);
			if (err) {
				return err;
			}
		}
	}

	return 0;
}

static int start_le_scan_ext(struct bt_hci_ext_scan_phy *phy_1m,
			     struct bt_hci_ext_scan_phy *phy_coded,
			     u16_t duration)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_ext_scan_param *set_param;
	struct net_buf *buf;

	u8_t own_addr_type;
	bool active_scan;
	int err;

	active_scan = (phy_1m && phy_1m->type == BT_HCI_LE_SCAN_ACTIVE) ||
		      (phy_coded && phy_coded->type == BT_HCI_LE_SCAN_ACTIVE);

	if (duration > 0) {
		atomic_set_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED);

		/* Allow bt_le_oob_get_local to be called directly before
		 * starting a scan limited by timeout.
		 */
		if (IS_ENABLED(CONFIG_BT_PRIVACY) && !rpa_is_new()) {
			atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);
		}
	}

	err = le_scan_set_random_addr(active_scan, &own_addr_type);
	if (err) {
		return err;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EXT_SCAN_PARAM,
				sizeof(*set_param) +
				(phy_1m ? sizeof(*phy_1m) : 0) +
				(phy_coded ? sizeof(*phy_coded) : 0));
	if (!buf) {
		return -ENOBUFS;
	}

	set_param = net_buf_add(buf, sizeof(*set_param));
	set_param->own_addr_type = own_addr_type;
	set_param->phys = 0;

	if (IS_ENABLED(CONFIG_BT_WHITELIST) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_WL)) {
		set_param->filter_policy = BT_HCI_LE_SCAN_FP_USE_WHITELIST;
	} else {
		set_param->filter_policy = BT_HCI_LE_SCAN_FP_NO_WHITELIST;
	}

	if (phy_1m) {
		set_param->phys |= BT_HCI_LE_EXT_SCAN_PHY_1M;
		net_buf_add_mem(buf, phy_1m, sizeof(*phy_1m));
	}

	if (phy_coded) {
		set_param->phys |= BT_HCI_LE_EXT_SCAN_PHY_CODED;
		net_buf_add_mem(buf, phy_coded, sizeof(*phy_coded));
	}

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_EXT_SCAN_PARAM, buf, NULL);
	if (err) {
		return err;
	}
#else
	u8_t own_addr_type;
	bool active_scan;
	int err;
	u8_t scan_phys = 0;
	u8_t filter_policy; 

	active_scan = (phy_1m && phy_1m->type == BT_HCI_LE_SCAN_ACTIVE) ||
		      (phy_coded && phy_coded->type == BT_HCI_LE_SCAN_ACTIVE);

	if (duration > 0) {
		atomic_set_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED);

		/* Allow bt_le_oob_get_local to be called directly before
		 * starting a scan limited by timeout.
		 */
		if (IS_ENABLED(CONFIG_BT_PRIVACY) && !rpa_is_new()) {
			atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);
		}
	}

	err = le_scan_set_random_addr(active_scan, &own_addr_type);
	if (err) {
		return err;
	}

	struct ext_scan_param_t phys[(phy_1m ? sizeof(*phy_1m) : 0) +
				(phy_coded ? sizeof(*phy_coded) : 0)];
	struct ext_scan_param_t *phy = phys;

	if (IS_ENABLED(CONFIG_BT_WHITELIST) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_WL)) {
		filter_policy = BT_HCI_LE_SCAN_FP_USE_WHITELIST;
	} else {
		filter_policy = BT_HCI_LE_SCAN_FP_NO_WHITELIST;
	}

	if (phy_1m) {
		scan_phys |= BT_HCI_LE_EXT_SCAN_PHY_1M;
		phy->type = phy_1m->type;
		phy->interval = phy_1m->interval;
		phy->window = phy_1m->window;
		phy++;
	}

	if (phy_coded) {
		scan_phys |= BT_HCI_LE_EXT_SCAN_PHY_CODED;
		phy->type = phy_coded->type;
		phy->interval = phy_coded->interval;
		phy->window = phy_coded->window;
	}

	err = hci_api_le_ext_scan_param_set(own_addr_type, filter_policy, scan_phys, phys);
	if (err) {
		return err;
	}
#endif
	err = set_le_ext_scan_enable(BT_HCI_LE_SCAN_ENABLE, duration);
	if (err) {
		return err;
	}

	atomic_set_bit_to(bt_dev.flags, BT_DEV_ACTIVE_SCAN, active_scan);

	return 0;
}

static int start_le_scan_legacy(u8_t scan_type, u16_t interval, u16_t window)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_scan_param set_param;
	struct net_buf *buf;
	int err;
	bool active_scan;

	(void)memset(&set_param, 0, sizeof(set_param));

	set_param.scan_type = scan_type;

	/* for the rest parameters apply default values according to
	 *  spec 4.2, vol2, part E, 7.8.10
	 */
	set_param.interval = sys_cpu_to_le16(interval);
	set_param.window = sys_cpu_to_le16(window);

	if (IS_ENABLED(CONFIG_BT_WHITELIST) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_WL)) {
		set_param.filter_policy = BT_HCI_LE_SCAN_FP_USE_WHITELIST;
	} else {
		set_param.filter_policy = BT_HCI_LE_SCAN_FP_NO_WHITELIST;
	}

	active_scan = scan_type == BT_HCI_LE_SCAN_ACTIVE;
	err = le_scan_set_random_addr(active_scan, &set_param.addr_type);
	if (err) {
		return err;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_SCAN_PARAM, sizeof(set_param));
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_mem(buf, &set_param, sizeof(set_param));

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_SCAN_PARAM, buf, NULL);
	if (err) {
		return err;
	}
#else
	int err;
	bool active_scan;
	u8_t filter_policy;
	u8_t addr_type;

	if (IS_ENABLED(CONFIG_BT_WHITELIST) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_WL)) {
		filter_policy = BT_HCI_LE_SCAN_FP_USE_WHITELIST;
	} else {
		filter_policy = BT_HCI_LE_SCAN_FP_NO_WHITELIST;
	}

	active_scan = scan_type == BT_HCI_LE_SCAN_ACTIVE;
	err = le_scan_set_random_addr(active_scan, &addr_type);
	if (err) {
		return err;
	}

	err = hci_api_le_scan_param_set(scan_type, 
									sys_cpu_to_le16(interval), 
									sys_cpu_to_le16(window),
									addr_type,
									filter_policy);
	if (err) {
		return err;
	}
#endif
	err = set_le_scan_enable(BT_HCI_LE_SCAN_ENABLE);
	if (err) {
		return err;
	}

	atomic_set_bit_to(bt_dev.flags, BT_DEV_ACTIVE_SCAN, active_scan);

	return 0;
}

static int start_passive_scan(bool fast_scan)
{
	u16_t interval, window;

	if (fast_scan) {
		interval = BT_GAP_SCAN_FAST_INTERVAL;
		window = BT_GAP_SCAN_FAST_WINDOW;
	} else {
		interval = CONFIG_BT_BACKGROUND_SCAN_INTERVAL;
		window = CONFIG_BT_BACKGROUND_SCAN_WINDOW;
	}

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		struct bt_hci_ext_scan_phy scan;

		scan.type = BT_HCI_LE_SCAN_PASSIVE;
		scan.interval = sys_cpu_to_le16(interval);
		scan.window = sys_cpu_to_le16(window);

		return start_le_scan_ext(&scan, NULL, 0);
	}

	return start_le_scan_legacy(BT_HCI_LE_SCAN_PASSIVE, interval, window);
}

int bt_le_scan_update(bool fast_scan)
{
	if (atomic_test_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN)) {
		return 0;
	}

	if (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
		int err;

		err = set_le_scan_enable(BT_HCI_LE_SCAN_DISABLE);
		if (err) {
			return err;
		}
	}

	if (IS_ENABLED(CONFIG_BT_CENTRAL)) {
		struct bt_conn *conn;

		/* don't restart scan if we have pending connection */
		conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL,
					       BT_CONN_CONNECT);
		if (conn) {
			bt_conn_unref(conn);
			return 0;
		}

		conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL,
					       BT_CONN_CONNECT_SCAN);
		if (!conn) {
			return 0;
		}

		atomic_set_bit(bt_dev.flags, BT_DEV_SCAN_FILTER_DUP);

		bt_conn_unref(conn);

		return start_passive_scan(fast_scan);
	}

#if (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC)
	if (get_pending_per_adv_sync()) {
		return start_passive_scan(fast_scan);
	}
#endif

	return 0;
}

void bt_data_parse(struct net_buf_simple *ad,
		   bool (*func)(struct bt_data *data, void *user_data),
		   void *user_data)
{
	while (ad->len > 1) {
		struct bt_data data;
		u8_t len;

		len = net_buf_simple_pull_u8(ad);
		if (len == 0U) {
			/* Early termination */
			return;
		}

		if (len > ad->len) {
			BT_WARN("Malformed data");
			return;
		}

		data.type = net_buf_simple_pull_u8(ad);
		data.data_len = len - 1;
		data.data = ad->data;

		if (!func(&data, user_data)) {
			return;
		}

		net_buf_simple_pull(ad, len - 1);
	}
}

/* Convert Legacy adv report evt_type field to adv props */
static u8_t get_adv_props(u8_t evt_type)
{
	switch (evt_type) {
	case BT_GAP_ADV_TYPE_ADV_IND:
		return BT_GAP_ADV_PROP_CONNECTABLE |
		       BT_GAP_ADV_PROP_SCANNABLE;

	case BT_GAP_ADV_TYPE_ADV_DIRECT_IND:
		return BT_GAP_ADV_PROP_CONNECTABLE |
		       BT_GAP_ADV_PROP_DIRECTED;

	case BT_GAP_ADV_TYPE_ADV_SCAN_IND:
		return BT_GAP_ADV_PROP_SCANNABLE;

	case BT_GAP_ADV_TYPE_ADV_NONCONN_IND:
		return 0;

	/* In legacy advertising report, we don't know if the scan
	 * response come from a connectable advertiser, so don't
	 * set connectable property bit.
	 */
	case BT_GAP_ADV_TYPE_SCAN_RSP:
		return BT_GAP_ADV_PROP_SCAN_RESPONSE |
		       BT_GAP_ADV_PROP_SCANNABLE;

	default:
		return 0;
	}
}

static void le_adv_recv(bt_addr_le_t *addr, struct bt_le_scan_recv_info *info,
			struct net_buf *buf, u8_t len)
{
	struct bt_le_scan_cb *listener;
	struct net_buf_simple_state state;
	bt_addr_le_t id_addr;

	BT_DBG("%s event %u, len %u, rssi %d dBm", bt_addr_le_str(addr),
	       info->adv_type, len, info->rssi);

	if (!IS_ENABLED(CONFIG_BT_PRIVACY) &&
	    !IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN) &&
	    (info->adv_props & BT_HCI_LE_ADV_PROP_DIRECT)) {
		BT_DBG("Dropped direct adv report");
		return;
	}

	if (addr->type == BT_ADDR_LE_PUBLIC_ID ||
	    addr->type == BT_ADDR_LE_RANDOM_ID) {
		bt_addr_le_copy(&id_addr, addr);
		id_addr.type -= BT_ADDR_LE_PUBLIC_ID;
	} else if (addr->type == BT_HCI_PEER_ADDR_ANONYMOUS) {
		bt_addr_le_copy(&id_addr, BT_ADDR_LE_ANY);
	} else {
		bt_addr_le_copy(&id_addr,
				bt_lookup_id_addr(BT_ID_DEFAULT, addr));
	}

	info->addr = &id_addr;

	bt_le_scan_cb_t *scan_dev_found_cb_tmp;
	scan_dev_found_cb_tmp = scan_dev_found_cb;
	if (scan_dev_found_cb_tmp) {
		net_buf_simple_save(&buf->b, &state);

		buf->len = len;
		scan_dev_found_cb_tmp(&id_addr, info->rssi, info->adv_type,
				  &buf->b);
		net_buf_simple_restore(&buf->b, &state);
	}


	SYS_SLIST_FOR_EACH_CONTAINER(&scan_cbs, listener, node) {
		net_buf_simple_save(&buf->b, &state);

		buf->len = len;
		if(listener->recv) {
            listener->recv(info, &buf->b);
		}

		if(listener->buf_recv) {
           listener->buf_recv(info,buf);
		}
		net_buf_simple_restore(&buf->b, &state);
	}

#if (defined(CONFIG_BT_CENTRAL) && CONFIG_BT_CENTRAL)
	check_pending_conn(&id_addr, addr, info->adv_props);
#endif /* CONFIG_BT_CENTRAL */
}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
static void le_scan_timeout(struct net_buf *buf)
{
	struct bt_le_scan_cb *listener;

	atomic_clear_bit(bt_dev.flags, BT_DEV_SCANNING);
	atomic_clear_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN);

	atomic_clear_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED);
	atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	pending_id_keys_update();
#endif

	SYS_SLIST_FOR_EACH_CONTAINER(&scan_cbs, listener, node) {
		listener->timeout();
	}
}

#define BT_LE_ADV_EXT_ADV_COMPLETE     (__DEPRECATED_MACRO 0x05)
#define BT_LE_ADV_EXT_ADV_TRUNCATED    (__DEPRECATED_MACRO 0x06)
#define BT_LE_ADV_EXT_ADV_INCOMPLETE   (__DEPRECATED_MACRO 0x07)

/* Convert Extended adv report evt_type field into adv type */
static u8_t get_adv_type(u8_t evt_type)
{
	switch (evt_type) {
	case (BT_HCI_LE_ADV_EVT_TYPE_CONN |
	      BT_HCI_LE_ADV_EVT_TYPE_SCAN |
	      BT_HCI_LE_ADV_EVT_TYPE_LEGACY):
		return BT_GAP_ADV_TYPE_ADV_IND;

	case (BT_HCI_LE_ADV_EVT_TYPE_CONN |
	      BT_HCI_LE_ADV_EVT_TYPE_DIRECT |
	      BT_HCI_LE_ADV_EVT_TYPE_LEGACY):
		return BT_GAP_ADV_TYPE_ADV_DIRECT_IND;

	case (BT_HCI_LE_ADV_EVT_TYPE_SCAN |
	      BT_HCI_LE_ADV_EVT_TYPE_LEGACY):
		return BT_GAP_ADV_TYPE_ADV_SCAN_IND;

	case BT_HCI_LE_ADV_EVT_TYPE_LEGACY:
		return BT_GAP_ADV_TYPE_ADV_NONCONN_IND;

	case (BT_HCI_LE_ADV_EVT_TYPE_SCAN_RSP |
	      BT_HCI_LE_ADV_EVT_TYPE_CONN |
	      BT_HCI_LE_ADV_EVT_TYPE_SCAN |
	      BT_HCI_LE_ADV_EVT_TYPE_LEGACY):
	case (BT_HCI_LE_ADV_EVT_TYPE_SCAN_RSP |
	      BT_HCI_LE_ADV_EVT_TYPE_SCAN |
	      BT_HCI_LE_ADV_EVT_TYPE_LEGACY):
		/* Scan response from connectable or non-connectable advertiser.
		 */
		return BT_GAP_ADV_TYPE_SCAN_RSP;
	case (BT_HCI_LE_ADV_EVT_TYPE_EXT_TRUNCATED):
		return BT_GAP_ADV_TYPE_EXT_ADV_TRUNCATED;

	case (BT_HCI_LE_ADV_EVT_TYPE_EXT_INCOMPLETE):
		return BT_GAP_ADV_TYPE_EXT_ADV_INCOMPLETE;

	default://0X00
		return BT_GAP_ADV_TYPE_EXT_ADV_COMPLETE;
	}
}

static void le_adv_ext_report(struct net_buf *buf)
{
	u8_t num_reports = net_buf_pull_u8(buf);

	BT_DBG("Adv number of reports %u",  num_reports);

	while (num_reports--) {
		struct bt_hci_evt_le_ext_advertising_info *evt;
		struct bt_le_scan_recv_info adv_info;

		if (buf->len < sizeof(*evt)) {
			BT_ERR("Unexpected end of buffer");
			break;
		}

		evt = net_buf_pull_mem(buf, sizeof(*evt));

		adv_info.primary_phy = get_phy(evt->prim_phy);
		adv_info.secondary_phy = get_phy(evt->sec_phy);
		adv_info.tx_power = evt->tx_power;
		adv_info.rssi = evt->rssi;
		adv_info.sid = evt->sid;
		adv_info.interval = sys_le16_to_cpu(evt->interval);

		adv_info.adv_type = get_adv_type(evt->evt_type);
		/* Convert "Legacy" property to Extended property. */
		adv_info.adv_props = evt->evt_type ^ BT_HCI_LE_ADV_PROP_LEGACY;

		le_adv_recv(&evt->addr, &adv_info, buf, evt->length);

		net_buf_pull(buf, evt->length);
	}
}

#if (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC)
static void per_adv_sync_delete(struct bt_le_per_adv_sync *per_adv_sync)
{
	atomic_clear(per_adv_sync->flags);
}

static struct bt_le_per_adv_sync *per_adv_sync_new(void)
{
	struct bt_le_per_adv_sync *per_adv_sync = NULL;

	for (int i = 0; i < ARRAY_SIZE(per_adv_sync_pool); i++) {
		if (!atomic_test_bit(per_adv_sync_pool[i].flags,
				     BT_PER_ADV_SYNC_CREATED)) {
			per_adv_sync = &per_adv_sync_pool[i];
			break;
		}
	}

	if (!per_adv_sync) {
		return NULL;
	}

	(void)memset(per_adv_sync, 0, sizeof(*per_adv_sync));
	atomic_set_bit(per_adv_sync->flags, BT_PER_ADV_SYNC_CREATED);

	return per_adv_sync;
}

static struct bt_le_per_adv_sync *get_pending_per_adv_sync(void)
{
	for (int i = 0; i < ARRAY_SIZE(per_adv_sync_pool); i++) {
		if (atomic_test_bit(per_adv_sync_pool[i].flags,
				    BT_PER_ADV_SYNC_SYNCING)) {
			return &per_adv_sync_pool[i];
		}
	}

	return NULL;
}

static struct bt_le_per_adv_sync *get_per_adv_sync(uint16_t handle)
{
	for (int i = 0; i < ARRAY_SIZE(per_adv_sync_pool); i++) {
		if (per_adv_sync_pool[i].handle == handle &&
		    atomic_test_bit(per_adv_sync_pool[i].flags,
				    BT_PER_ADV_SYNC_SYNCED)) {
			return &per_adv_sync_pool[i];
		}
	}

	return NULL;
}

static void le_per_adv_report(struct net_buf *buf)
{
	struct bt_hci_evt_le_per_advertising_report *evt;
	struct bt_le_per_adv_sync *per_adv_sync;
	struct bt_le_per_adv_sync_recv_info info;
	struct bt_le_per_adv_sync_cb *listener;
	struct net_buf_simple_state state;

	if (buf->len < sizeof(*evt)) {
		BT_ERR("Unexpected end of buffer");
		return;
	}

	evt = net_buf_pull_mem(buf, sizeof(*evt));

	per_adv_sync = get_per_adv_sync(sys_le16_to_cpu(evt->handle));

	if (!per_adv_sync) {
		BT_ERR("Unknown handle 0x%04X for periodic advertising report",
		       sys_le16_to_cpu(evt->handle));
		return;
	}

	if (atomic_test_bit(per_adv_sync->flags,
			    BT_PER_ADV_SYNC_RECV_DISABLED)) {
		BT_ERR("Received PA adv report when receive disabled");
		return;
	}

	info.tx_power = evt->tx_power;
	info.rssi = evt->rssi;
	info.cte_type = evt->cte_type;
	info.addr = &per_adv_sync->addr;

	SYS_SLIST_FOR_EACH_CONTAINER(&pa_sync_cbs, listener, node) {
		if (listener->recv) {
			net_buf_simple_save(&buf->b, &state);

			buf->len = evt->length;
			listener->recv(per_adv_sync, &info, &buf->b);

			net_buf_simple_restore(&buf->b, &state);
		}
	}
}

static int per_adv_sync_terminate(uint16_t handle)
{
	struct bt_hci_cp_le_per_adv_terminate_sync *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_PER_ADV_TERMINATE_SYNC,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->handle = sys_cpu_to_le16(handle);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_PER_ADV_TERMINATE_SYNC, buf,
				    NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_PER_ADV_TERMINATE_SYNC, buf,
				    NULL);
#endif
}

static void le_per_adv_sync_established(struct net_buf *buf)
{
	struct bt_hci_evt_le_per_adv_sync_established *evt =
		(struct bt_hci_evt_le_per_adv_sync_established *)buf->data;
	struct bt_le_per_adv_sync_synced_info sync_info;
	struct bt_le_per_adv_sync *pending_per_adv_sync;
	struct bt_le_per_adv_sync_cb *listener;
	int err;

	pending_per_adv_sync = get_pending_per_adv_sync();

	if (pending_per_adv_sync) {
		atomic_clear_bit(pending_per_adv_sync->flags,
				 BT_PER_ADV_SYNC_SYNCING);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		err = bt_fsm_le_scan_update(false, bt_dev.fsm);
#else
		err = bt_le_scan_update(false);
#endif

		if (err) {
			BT_ERR("Could not update scan (%d)", err);
		}
	}

	if (evt->status == BT_HCI_ERR_OP_CANCELLED_BY_HOST) {
		/* Cancelled locally, don't call CB */
		if (pending_per_adv_sync) {
			per_adv_sync_delete(pending_per_adv_sync);
		} else {
			BT_ERR("Unexpected per adv sync cancelled event");
		}

		return;
	}

	if (!pending_per_adv_sync ||
	    pending_per_adv_sync->sid != evt->sid ||
	    bt_addr_le_cmp(&pending_per_adv_sync->addr, &evt->adv_addr)) {
		struct bt_le_per_adv_sync_term_info term_info;

		BT_ERR("Unexpected per adv sync established event");
		per_adv_sync_terminate(sys_le16_to_cpu(evt->handle));

		if (pending_per_adv_sync) {
			/* Terminate the pending PA sync and notify app */
			term_info.addr = &pending_per_adv_sync->addr;
			term_info.sid = pending_per_adv_sync->sid;

			/* Deleting before callback, so the caller will be able
			 * to restart sync in the callback.
			 */
			per_adv_sync_delete(pending_per_adv_sync);


			SYS_SLIST_FOR_EACH_CONTAINER(&pa_sync_cbs,
						     listener,
						     node) {
				if (listener->term) {
					listener->term(pending_per_adv_sync,
						       &term_info);
				}
			}
		}
		return;
	}

	atomic_set_bit(pending_per_adv_sync->flags, BT_PER_ADV_SYNC_SYNCED);

	pending_per_adv_sync->handle = sys_le16_to_cpu(evt->handle);
	pending_per_adv_sync->interval = sys_le16_to_cpu(evt->interval);
	pending_per_adv_sync->clock_accuracy =
		sys_le16_to_cpu(evt->clock_accuracy);
	pending_per_adv_sync->phy = evt->phy;

	memset(&sync_info, 0, sizeof(sync_info));
	sync_info.interval = pending_per_adv_sync->interval;
	sync_info.phy = get_phy(pending_per_adv_sync->phy);
	sync_info.addr = &pending_per_adv_sync->addr;
	sync_info.sid = pending_per_adv_sync->sid;

	sync_info.recv_enabled =
		!atomic_test_bit(pending_per_adv_sync->flags,
				 BT_PER_ADV_SYNC_RECV_DISABLED);

	SYS_SLIST_FOR_EACH_CONTAINER(&pa_sync_cbs, listener, node) {
		if (listener->synced) {
			listener->synced(pending_per_adv_sync, &sync_info);
		}
	}
}

static void le_per_adv_sync_lost(struct net_buf *buf)
{
	struct bt_hci_evt_le_per_adv_sync_lost *evt =
		(struct bt_hci_evt_le_per_adv_sync_lost *)buf->data;
	struct bt_le_per_adv_sync_term_info term_info;
	struct bt_le_per_adv_sync *per_adv_sync;
	struct bt_le_per_adv_sync_cb *listener;

	per_adv_sync = get_per_adv_sync(sys_le16_to_cpu(evt->handle));

	if (!per_adv_sync) {
		BT_ERR("Unknown handle 0x%04Xfor periodic adv sync lost",
		       sys_le16_to_cpu(evt->handle));
		return;
	}

	term_info.addr = &per_adv_sync->addr;
	term_info.sid = per_adv_sync->sid;

	/* Deleting before callback, so the caller will be able to restart
	 * sync in the callback
	 */
	per_adv_sync_delete(per_adv_sync);


	SYS_SLIST_FOR_EACH_CONTAINER(&pa_sync_cbs, listener, node) {
		if (listener->term) {
			listener->term(per_adv_sync, &term_info);
		}
	}
}

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
static void le_past_received(struct net_buf *buf)
{
	struct bt_hci_evt_le_past_received *evt =
		(struct bt_hci_evt_le_past_received *)buf->data;
	struct bt_le_per_adv_sync_synced_info sync_info;
	struct bt_le_per_adv_sync_cb *listener;
	struct bt_le_per_adv_sync *per_adv_sync;

	if (evt->status) {
		/* No sync created, don't notify app */
		BT_DBG("PAST receive failed with status 0x%02X", evt->status);
		return;
	}

	sync_info.conn = bt_conn_lookup_handle(
				sys_le16_to_cpu(evt->conn_handle));

	if (!sync_info.conn) {
		BT_ERR("Could not lookup connection handle from PAST");
		per_adv_sync_terminate(sys_le16_to_cpu(evt->sync_handle));
		return;
	}

	per_adv_sync = per_adv_sync_new();
	if (!per_adv_sync) {
		BT_WARN("Could not allocate new PA sync from PAST");
		per_adv_sync_terminate(sys_le16_to_cpu(evt->sync_handle));
		return;
	}

	atomic_set_bit(per_adv_sync->flags, BT_PER_ADV_SYNC_SYNCED);

	per_adv_sync->handle = sys_le16_to_cpu(evt->sync_handle);
	per_adv_sync->interval = sys_le16_to_cpu(evt->interval);
	per_adv_sync->clock_accuracy = sys_le16_to_cpu(evt->clock_accuracy);
	per_adv_sync->phy = evt->phy;
	bt_addr_le_copy(&per_adv_sync->addr, &evt->addr);
	per_adv_sync->sid = evt->adv_sid;

	sync_info.interval = per_adv_sync->interval;
	sync_info.phy = get_phy(per_adv_sync->phy);
	sync_info.addr = &per_adv_sync->addr;
	sync_info.sid = per_adv_sync->sid;
	sync_info.service_data = sys_le16_to_cpu(evt->service_data);

	SYS_SLIST_FOR_EACH_CONTAINER(&pa_sync_cbs, listener, node) {
		if (listener->synced) {
			listener->synced(per_adv_sync, &sync_info);
		}
	}
}
#endif /* CONFIG_BT_CONN */
#endif /* (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC) */
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */

static void le_adv_report(struct net_buf *buf)
{
	u8_t num_reports = net_buf_pull_u8(buf);
	struct bt_hci_evt_le_advertising_info *evt;

	BT_DBG("Adv number of reports %u",  num_reports);

	while (num_reports--) {
		struct bt_le_scan_recv_info adv_info;

		if (buf->len < sizeof(*evt)) {
			BT_ERR("Unexpected end of buffer");
			break;
		}

		evt = net_buf_pull_mem(buf, sizeof(*evt));

		adv_info.rssi = evt->data[evt->length];
		adv_info.primary_phy = BT_GAP_LE_PHY_1M;
		adv_info.secondary_phy = 0;
		adv_info.tx_power = BT_GAP_TX_POWER_INVALID;
		adv_info.sid = BT_GAP_SID_INVALID;
		adv_info.interval = 0U;

		adv_info.adv_type = evt->evt_type;
		adv_info.adv_props = get_adv_props(evt->evt_type);

		le_adv_recv(&evt->addr, &adv_info, buf, evt->length);

		net_buf_pull(buf, evt->length + sizeof(adv_info.rssi));
	}
}
#endif /* CONFIG_BT_OBSERVER */

static void le_adv_stop_free_conn(const struct bt_le_ext_adv *adv, u8_t status)
{
	struct bt_conn *conn;

	if (!bt_addr_le_cmp(&adv->target_addr, BT_ADDR_LE_ANY)) {
		conn = bt_conn_lookup_state_le(adv->id, BT_ADDR_LE_NONE,
					       BT_CONN_CONNECT_ADV);
	} else {
		conn = bt_conn_lookup_state_le(adv->id, &adv->target_addr,
					       BT_CONN_CONNECT_DIR_ADV);
	}

	if (conn) {
		conn->err = status;
		bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
		bt_conn_unref(conn);
	}
}


#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
#if (defined(CONFIG_BT_BROADCASTER) && CONFIG_BT_BROADCASTER)
static void le_adv_set_terminated(struct net_buf *buf)
{
	struct bt_hci_evt_le_adv_set_terminated *evt;
	struct bt_le_ext_adv *adv;

	evt = (void *)buf->data;
	adv = bt_adv_lookup_handle(evt->adv_handle);

	BT_DBG("status 0x%02x adv_handle %u conn_handle 0x%02x num %u",
	       evt->status, evt->adv_handle, evt->conn_handle,
	       evt->num_completed_ext_adv_evts);

	if (!adv) {
		BT_ERR("No valid adv");
		return;
	}

	atomic_clear_bit(adv->flags, BT_ADV_ENABLED);

	if (evt->status && IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		/* Only set status for legacy advertising API.
		 * This will call connected callback for high duty cycle
		 * directed advertiser timeout.
		 */
		le_adv_stop_free_conn(adv, adv == bt_dev.adv ? evt->status : 0);
	}

	if (IS_ENABLED(CONFIG_BT_CONN) && !evt->status) {
		struct bt_conn *conn = bt_conn_lookup_handle(evt->conn_handle);

		if (conn) {
			if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
			    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
				/* Set Responder address unless already set */
				conn->le.resp_addr.type = BT_ADDR_LE_RANDOM;
				if (bt_addr_cmp(&conn->le.resp_addr.a,
						BT_ADDR_ANY) == 0) {
					bt_addr_copy(&conn->le.resp_addr.a,
						     &adv->random_addr.a);
				}
			} else {
				bt_addr_le_copy(&conn->le.resp_addr,
					&bt_dev.id_addr[conn->id]);
			}

			if (adv->cb && adv->cb->connected) {
				struct bt_le_ext_adv_connected_info info = {
					.conn = conn,
				};

				adv->cb->connected(adv, &info);
			}

			bt_conn_unref(conn);
		}
	}

	if (atomic_test_and_clear_bit(adv->flags, BT_ADV_LIMITED)) {
		atomic_clear_bit(adv->flags, BT_ADV_RPA_VALID);

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		pending_id_keys_update();
#endif

		if (adv->cb && adv->cb->sent) {
			struct bt_le_ext_adv_sent_info info = {
				.num_sent = evt->num_completed_ext_adv_evts,
			};

			adv->cb->sent(adv, &info);
		}
	}

	if (!atomic_test_bit(adv->flags, BT_ADV_PERSIST) && adv == bt_dev.adv) {
		adv_delete_legacy();
	}
}

static void le_scan_req_received(struct net_buf *buf)
{
	struct bt_hci_evt_le_scan_req_received *evt;
	struct bt_le_ext_adv *adv;

	evt = (void *)buf->data;
	adv = bt_adv_lookup_handle(evt->handle);

	BT_DBG("handle %u peer %s", evt->handle, bt_addr_le_str(&evt->addr));

	if (!adv) {
		BT_ERR("No valid adv");
		return;
	}

	if (adv->cb && adv->cb->scanned) {
		struct bt_le_ext_adv_scanned_info info;
		bt_addr_le_t id_addr;

		if (evt->addr.type == BT_ADDR_LE_PUBLIC_ID ||
		    evt->addr.type == BT_ADDR_LE_RANDOM_ID) {
			bt_addr_le_copy(&id_addr, &evt->addr);
			id_addr.type -= BT_ADDR_LE_PUBLIC_ID;
		} else {
			bt_addr_le_copy(&id_addr,
					bt_lookup_id_addr(adv->id, &evt->addr));
		}

		info.addr = &id_addr;
		adv->cb->scanned(adv, &info);
	}
}
#endif /* (defined(CONFIG_BT_BROADCASTER) && CONFIG_BT_BROADCASTER) */
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */

int bt_hci_get_conn_handle(const struct bt_conn *conn, u16_t *conn_handle)
{
	if (conn->state != BT_CONN_CONNECTED) {
		return -ENOTCONN;
	}

	*conn_handle = conn->handle;
	return 0;
}

#if (defined(CONFIG_BT_HCI_VS_EVT_USER) && CONFIG_BT_HCI_VS_EVT_USER)
int bt_hci_register_vnd_evt_cb(bt_hci_vnd_evt_cb_t cb)
{
	hci_vnd_evt_cb = cb;
	return 0;
}
#endif /* CONFIG_BT_HCI_VS_EVT_USER */

static void hci_vendor_event(struct net_buf *buf)
{
	bool handled = false;

#if (defined(CONFIG_BT_HCI_VS_EVT_USER) && CONFIG_BT_HCI_VS_EVT_USER)
	if (hci_vnd_evt_cb) {
		struct net_buf_simple_state state;

		net_buf_simple_save(&buf->b, &state);

		handled = hci_vnd_evt_cb(&buf->b);

		net_buf_simple_restore(&buf->b, &state);
	}
#endif /* CONFIG_BT_HCI_VS_EVT_USER */

	if (IS_ENABLED(CONFIG_BT_HCI_VS_EXT) && !handled) {
		/* do nothing at present time */
		BT_WARN("Unhandled vendor-specific event: %s",
			bt_hex(buf->data, buf->len));
	}
}

static const struct event_handler meta_events[] = {
#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	EVENT_HANDLER(BT_HCI_EVT_LE_ADVERTISING_REPORT, le_adv_report,
		      sizeof(struct bt_hci_evt_le_advertising_report)),
#endif /* CONFIG_BT_OBSERVER */
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	EVENT_HANDLER(BT_HCI_EVT_LE_CONN_COMPLETE, le_legacy_conn_complete,
		      sizeof(struct bt_hci_evt_le_conn_complete)),
	EVENT_HANDLER(BT_HCI_EVT_LE_ENH_CONN_COMPLETE, le_enh_conn_complete,
		      sizeof(struct bt_hci_evt_le_enh_conn_complete)),
	EVENT_HANDLER(BT_HCI_EVT_LE_CONN_UPDATE_COMPLETE,
		      le_conn_update_complete,
		      sizeof(struct bt_hci_evt_le_conn_update_complete)),
	EVENT_HANDLER(BT_HCI_EVT_LE_REMOTE_FEAT_COMPLETE,
		      le_remote_feat_complete,
		      sizeof(struct bt_hci_evt_le_remote_feat_complete)),
	EVENT_HANDLER(BT_HCI_EVT_LE_CONN_PARAM_REQ, le_conn_param_req,
		      sizeof(struct bt_hci_evt_le_conn_param_req)),
#if (defined(CONFIG_BT_DATA_LEN_UPDATE) && CONFIG_BT_DATA_LEN_UPDATE)
	EVENT_HANDLER(BT_HCI_EVT_LE_DATA_LEN_CHANGE, le_data_len_change,
		      sizeof(struct bt_hci_evt_le_data_len_change)),
#endif /* CONFIG_BT_DATA_LEN_UPDATE */
#if (defined(CONFIG_BT_PHY_UPDATE) && CONFIG_BT_PHY_UPDATE)
	EVENT_HANDLER(BT_HCI_EVT_LE_PHY_UPDATE_COMPLETE,
		      le_phy_update_complete,
		      sizeof(struct bt_hci_evt_le_phy_update_complete)),
#endif /* CONFIG_BT_PHY_UPDATE */
#endif /* CONFIG_BT_CONN */
#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	EVENT_HANDLER(BT_HCI_EVT_LE_LTK_REQUEST, le_ltk_request,
		      sizeof(struct bt_hci_evt_le_ltk_request)),
#endif /* CONFIG_BT_SMP */
#if (defined(CONFIG_BT_ECC) && CONFIG_BT_ECC)
	EVENT_HANDLER(BT_HCI_EVT_LE_P256_PUBLIC_KEY_COMPLETE, le_pkey_complete,
		      sizeof(struct bt_hci_evt_le_p256_public_key_complete)),
	EVENT_HANDLER(BT_HCI_EVT_LE_GENERATE_DHKEY_COMPLETE, le_dhkey_complete,
		      sizeof(struct bt_hci_evt_le_generate_dhkey_complete)),
#endif /* CONFIG_BT_SMP */
#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
#if (defined(CONFIG_BT_BROADCASTER) && CONFIG_BT_BROADCASTER)
	EVENT_HANDLER(BT_HCI_EVT_LE_ADV_SET_TERMINATED, le_adv_set_terminated,
		      sizeof(struct bt_hci_evt_le_adv_set_terminated)),
	EVENT_HANDLER(BT_HCI_EVT_LE_SCAN_REQ_RECEIVED, le_scan_req_received,
		      sizeof(struct bt_hci_evt_le_scan_req_received)),
#endif
#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	EVENT_HANDLER(BT_HCI_EVT_LE_SCAN_TIMEOUT, le_scan_timeout,
		      0),
	EVENT_HANDLER(BT_HCI_EVT_LE_EXT_ADVERTISING_REPORT, le_adv_ext_report,
		      sizeof(struct bt_hci_evt_le_ext_advertising_report)),
#if (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC)
	EVENT_HANDLER(BT_HCI_EVT_LE_PER_ADV_SYNC_ESTABLISHED,
		      le_per_adv_sync_established,
		      sizeof(struct bt_hci_evt_le_per_adv_sync_established)),
	EVENT_HANDLER(BT_HCI_EVT_LE_PER_ADVERTISING_REPORT, le_per_adv_report,
		      sizeof(struct bt_hci_evt_le_per_advertising_report)),
	EVENT_HANDLER(BT_HCI_EVT_LE_PER_ADV_SYNC_LOST, le_per_adv_sync_lost,
		      sizeof(struct bt_hci_evt_le_per_adv_sync_lost)),
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	EVENT_HANDLER(BT_HCI_EVT_LE_PAST_RECEIVED, le_past_received,
		      sizeof(struct bt_hci_evt_le_past_received)),
#endif /* CONFIG_BT_CONN */
#endif /* (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC) */
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */
#endif /* (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) */
};

static void hci_le_meta_event(struct net_buf *buf)
{
	struct bt_hci_evt_le_meta_event *evt;

	evt = net_buf_pull_mem(buf, sizeof(*evt));

	BT_DBG("subevent 0x%02x", evt->subevent);

	handle_event(evt->subevent, buf, meta_events, ARRAY_SIZE(meta_events));
}

static const struct event_handler normal_events[] = {
	EVENT_HANDLER(BT_HCI_EVT_VENDOR, hci_vendor_event,
		      sizeof(struct bt_hci_evt_vs)),
	EVENT_HANDLER(BT_HCI_EVT_LE_META_EVENT, hci_le_meta_event,
		      sizeof(struct bt_hci_evt_le_meta_event)),
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
	EVENT_HANDLER(BT_HCI_EVT_CONN_REQUEST, conn_req,
		      sizeof(struct bt_hci_evt_conn_request)),
	EVENT_HANDLER(BT_HCI_EVT_CONN_COMPLETE, conn_complete,
		      sizeof(struct bt_hci_evt_conn_complete)),
	EVENT_HANDLER(BT_HCI_EVT_PIN_CODE_REQ, pin_code_req,
		      sizeof(struct bt_hci_evt_pin_code_req)),
	EVENT_HANDLER(BT_HCI_EVT_LINK_KEY_NOTIFY, link_key_notify,
		      sizeof(struct bt_hci_evt_link_key_notify)),
	EVENT_HANDLER(BT_HCI_EVT_LINK_KEY_REQ, link_key_req,
		      sizeof(struct bt_hci_evt_link_key_req)),
	EVENT_HANDLER(BT_HCI_EVT_IO_CAPA_RESP, io_capa_resp,
		      sizeof(struct bt_hci_evt_io_capa_resp)),
	EVENT_HANDLER(BT_HCI_EVT_IO_CAPA_REQ, io_capa_req,
		      sizeof(struct bt_hci_evt_io_capa_req)),
	EVENT_HANDLER(BT_HCI_EVT_SSP_COMPLETE, ssp_complete,
		      sizeof(struct bt_hci_evt_ssp_complete)),
	EVENT_HANDLER(BT_HCI_EVT_USER_CONFIRM_REQ, user_confirm_req,
		      sizeof(struct bt_hci_evt_user_confirm_req)),
	EVENT_HANDLER(BT_HCI_EVT_USER_PASSKEY_NOTIFY, user_passkey_notify,
		      sizeof(struct bt_hci_evt_user_passkey_notify)),
	EVENT_HANDLER(BT_HCI_EVT_USER_PASSKEY_REQ, user_passkey_req,
		      sizeof(struct bt_hci_evt_user_passkey_req)),
	EVENT_HANDLER(BT_HCI_EVT_INQUIRY_COMPLETE, inquiry_complete,
		      sizeof(struct bt_hci_evt_inquiry_complete)),
	EVENT_HANDLER(BT_HCI_EVT_INQUIRY_RESULT_WITH_RSSI,
		      inquiry_result_with_rssi,
		      sizeof(struct bt_hci_evt_inquiry_result_with_rssi)),
	EVENT_HANDLER(BT_HCI_EVT_EXTENDED_INQUIRY_RESULT,
		      extended_inquiry_result,
		      sizeof(struct bt_hci_evt_extended_inquiry_result)),
	EVENT_HANDLER(BT_HCI_EVT_REMOTE_NAME_REQ_COMPLETE,
		      remote_name_request_complete,
		      sizeof(struct bt_hci_evt_remote_name_req_complete)),
	EVENT_HANDLER(BT_HCI_EVT_AUTH_COMPLETE, auth_complete,
		      sizeof(struct bt_hci_evt_auth_complete)),
	EVENT_HANDLER(BT_HCI_EVT_REMOTE_FEATURES,
		      read_remote_features_complete,
		      sizeof(struct bt_hci_evt_remote_features)),
	EVENT_HANDLER(BT_HCI_EVT_REMOTE_EXT_FEATURES,
		      read_remote_ext_features_complete,
		      sizeof(struct bt_hci_evt_remote_ext_features)),
	EVENT_HANDLER(BT_HCI_EVT_ROLE_CHANGE, role_change,
		      sizeof(struct bt_hci_evt_role_change)),
	EVENT_HANDLER(BT_HCI_EVT_SYNC_CONN_COMPLETE, synchronous_conn_complete,
		      sizeof(struct bt_hci_evt_sync_conn_complete)),
#endif /* CONFIG_BT_BREDR */
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	EVENT_HANDLER(BT_HCI_EVT_DISCONN_COMPLETE, hci_disconn_complete,
		      sizeof(struct bt_hci_evt_disconn_complete)),
#endif /* CONFIG_BT_CONN */
#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) || (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
	EVENT_HANDLER(BT_HCI_EVT_ENCRYPT_CHANGE, hci_encrypt_change,
		      sizeof(struct bt_hci_evt_encrypt_change)),
	EVENT_HANDLER(BT_HCI_EVT_ENCRYPT_KEY_REFRESH_COMPLETE,
		      hci_encrypt_key_refresh_complete,
		      sizeof(struct bt_hci_evt_encrypt_key_refresh_complete)),
#endif /* CONFIG_BT_SMP || CONFIG_BT_BREDR */
#if (defined(CONFIG_BT_REMOTE_VERSION) && CONFIG_BT_REMOTE_VERSION)
	EVENT_HANDLER(BT_HCI_EVT_REMOTE_VERSION_INFO,
		      bt_hci_evt_read_remote_version_complete,
		      sizeof(struct bt_hci_evt_remote_version_info)),
#endif /* CONFIG_BT_REMOTE_VERSION */
};

static void hci_event(struct net_buf *buf)
{
	struct bt_hci_evt_hdr *hdr;

	BT_ASSERT(buf->len >= sizeof(*hdr));
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	struct net_buf_simple_state state;

	net_buf_simple_save(&buf->b, &state);

	hdr = net_buf_pull_mem(buf, sizeof(*hdr));
	BT_DBG("event 0x%02x", hdr->evt);

	if (bt_hci_evt_is_prio(hdr->evt))
	{
		net_buf_simple_restore(&buf->b, &state);
		bt_recv_prio(buf);
		return;
	}
#else
	hdr = net_buf_pull_mem(buf, sizeof(*hdr));
	BT_DBG("event 0x%02x", hdr->evt);
	BT_ASSERT(!bt_hci_evt_is_prio(hdr->evt));
#endif
	handle_event(hdr->evt, buf, normal_events, ARRAY_SIZE(normal_events));

	net_buf_unref(buf);
}
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
#else
extern struct k_sem * bt_conn_get_pkts(struct bt_conn *conn);
#endif
int has_tx_sem(struct k_poll_event *event)
{
    struct bt_conn *conn = NULL;

    if (IS_ENABLED(CONFIG_BT_CONN)) {
        if (event->tag == BT_EVENT_CONN_TX_QUEUE) {
            conn = CONTAINER_OF(event->fifo, struct bt_conn, tx_queue);
        }
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
        if (conn && (bt_dev.le.pkts == 0)) {
            return 0;
        }
#else
        if (conn && (k_sem_count_get(bt_conn_get_pkts(conn)) == 0)) {
            return 0;
        }
#endif
    }
    return 1;
}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
void send_cmd(void)
{
	struct net_buf *buf;
	int err;

	/* Get next command */
	BT_DBG("calling net_buf_get");
	buf = net_buf_get(&bt_dev.cmd_tx_queue, K_NO_WAIT);
	if (!buf)
	{
		BT_DBG("return net_buf_get %p", buf);
		return;
	}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	if (atomic_get(&bt_dev.ncmd) == 0)
	{
		net_buf_put(&bt_dev.cmd_tx_pending_queue, buf);
		buf = net_buf_get(&bt_dev.cmd_tx_queue, K_NO_WAIT);
		while(buf) {
			net_buf_put(&bt_dev.cmd_tx_pending_queue, buf);
			BT_DBG("put %p to cmd_tx_pending_queue");
			buf = net_buf_get(&bt_dev.cmd_tx_queue, K_NO_WAIT);
		}

		return;
	}
#endif

	/* Wait until ncmd > 0 */
	BT_DBG("calling sem_take_wait");
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	atomic_dec(&bt_dev.ncmd);
	k_delayed_work_submit(&bt_dev.cmd_sent_work, HCI_CMD_TIMEOUT);
#else
	k_sem_take(&bt_dev.ncmd_sem, K_FOREVER);
#endif

/* Clear out any existing sent command */
	if (bt_dev.sent_cmd) {
		BT_ERR("Uncleared pending sent_cmd");
		net_buf_unref(bt_dev.sent_cmd);
		bt_dev.sent_cmd = NULL;
	}

	bt_dev.sent_cmd = net_buf_ref(buf);

	BT_DBG("Sending command 0x%04x (buf %p) to driver",
	       cmd(buf)->opcode, buf);

	err = bt_send(buf);
	if (err) {
		BT_ERR("Unable to send to driver (err %d)", err);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		atomic_inc(&bt_dev.ncmd);
#else
		k_sem_give(&bt_dev.ncmd_sem);
#endif
		hci_cmd_done(cmd(buf)->opcode, BT_HCI_ERR_UNSPECIFIED, buf);
		net_buf_unref(bt_dev.sent_cmd);
		bt_dev.sent_cmd = NULL;
		net_buf_unref(buf);
	}
}

void send_cmd_buf(struct net_buf *buf)
{

	int err;

	if (!buf)
	{
		BT_DBG("return net_buf_get %p", buf);
		return;
	}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	if (atomic_get(&bt_dev.ncmd) == 0)
	{
		net_buf_put(&bt_dev.cmd_tx_pending_queue, buf);
		buf = net_buf_get(&bt_dev.cmd_tx_queue, K_NO_WAIT);
		while(buf) {
			net_buf_put(&bt_dev.cmd_tx_pending_queue, buf);
			BT_DBG("put %p to cmd_tx_pending_queue");
			buf = net_buf_get(&bt_dev.cmd_tx_queue, K_NO_WAIT);
		}

		return;
	}
#endif

	/* Wait until ncmd > 0 */
	BT_DBG("calling sem_take_wait");
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	atomic_dec(&bt_dev.ncmd);
	k_delayed_work_submit(&bt_dev.cmd_sent_work, HCI_CMD_TIMEOUT);
#else
	k_sem_take(&bt_dev.ncmd_sem, K_FOREVER);
#endif

/* Clear out any existing sent command */
	if (bt_dev.sent_cmd) {
		BT_ERR("Uncleared pending sent_cmd");
		net_buf_unref(bt_dev.sent_cmd);
		bt_dev.sent_cmd = NULL;
	}

	bt_dev.sent_cmd = net_buf_ref(buf);

	BT_DBG("Sending command 0x%04x (buf %p) to driver",
	       cmd(buf)->opcode, buf);

	err = bt_send(buf);

	if (err) {
		BT_ERR("Unable to send to driver (err %d)", err);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		atomic_inc(&bt_dev.ncmd);
#else
		k_sem_give(&bt_dev.ncmd_sem);
#endif
		hci_cmd_done(cmd(buf)->opcode, BT_HCI_ERR_UNSPECIFIED, buf);
		net_buf_unref(bt_dev.sent_cmd);
		bt_dev.sent_cmd = NULL;
		net_buf_unref(buf);
	}
}

void process_events(struct k_poll_event *ev, int count)
{
	//BT_DBG("count %d", count);

	for (; count; ev++, count--) {
		//BT_DBG("ev->state %u", ev->state);

		switch (ev->state) {
		case K_POLL_STATE_SIGNALED:
			break;
		case K_POLL_STATE_FIFO_DATA_AVAILABLE:
			if (ev->tag == BT_EVENT_CMD_TX) {
				send_cmd();
			} else if (IS_ENABLED(CONFIG_BT_CONN)) {
				struct bt_conn *conn;

				if (ev->tag == BT_EVENT_CONN_TX_QUEUE) {
					conn = CONTAINER_OF(ev->fifo,
							    struct bt_conn,
							    tx_queue);
					bt_conn_process_tx(conn);
				}
			}
			break;
		case K_POLL_STATE_NOT_READY:
			break;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		case K_POLL_STATE_DATA_RECV:
			bt_fsm_process();
			hci_drvier_rx_process();
			break;
#endif
		default:
			BT_WARN("Unexpected k_poll event state %u", ev->state);
			break;
		}
	}
}

static void read_local_ver_complete(struct net_buf *buf)
{
	struct bt_hci_rp_read_local_version_info *rp = (void *)buf->data;

	BT_DBG("status 0x%02x", rp->status);

	bt_dev.hci_version = rp->hci_version;
	bt_dev.hci_revision = sys_le16_to_cpu(rp->hci_revision);
	bt_dev.lmp_version = rp->lmp_version;
	bt_dev.lmp_subversion = sys_le16_to_cpu(rp->lmp_subversion);
	bt_dev.manufacturer = sys_le16_to_cpu(rp->manufacturer);
}

static void read_le_features_complete(struct net_buf *buf)
{
	struct bt_hci_rp_le_read_local_features *rp = (void *)buf->data;

	BT_DBG("status 0x%02x", rp->status);

	memcpy(bt_dev.le.features, rp->features, sizeof(bt_dev.le.features));
}

#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
static void read_buffer_size_complete(struct net_buf *buf)
{
	struct bt_hci_rp_read_buffer_size *rp = (void *)buf->data;
	u16_t pkts;

	BT_DBG("status 0x%02x", rp->status);

	bt_dev.br.mtu = sys_le16_to_cpu(rp->acl_max_len);
	pkts = sys_le16_to_cpu(rp->acl_max_num);

	BT_DBG("ACL BR/EDR buffers: pkts %u mtu %u", pkts, bt_dev.br.mtu);

	k_sem_init(&bt_dev.br.pkts, pkts, pkts);
}
#elif (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static void read_buffer_size_complete(struct net_buf *buf)
{
	struct bt_hci_rp_read_buffer_size *rp = (void *)buf->data;
	u16_t pkts;

	BT_DBG("status 0x%02x", rp->status);

	/* If LE-side has buffers we can ignore the BR/EDR values */
	if (bt_dev.le.mtu) {
		return;
	}

	bt_dev.le.mtu = sys_le16_to_cpu(rp->acl_max_len);
	pkts = sys_le16_to_cpu(rp->acl_max_num);

	BT_DBG("ACL BR/EDR buffers: pkts %u mtu %u", pkts, bt_dev.le.mtu);

	k_sem_init(&bt_dev.le.pkts, pkts, pkts);
}
#endif
#endif

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
static void le_read_buffer_size_complete(struct net_buf *buf)
{
	struct bt_hci_rp_le_read_buffer_size *rp = (void *)buf->data;

	BT_DBG("status 0x%02x", rp->status);

	bt_dev.le.mtu = sys_le16_to_cpu(rp->le_max_len);
	if (!bt_dev.le.mtu) {
		return;
	}

	BT_DBG("ACL LE buffers: pkts %u mtu %u", rp->le_max_num, bt_dev.le.mtu);
    bt_dev.le.mtu_init = bt_dev.le.mtu;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	atomic_set(&bt_dev.le.pkts, rp->le_max_num);
#else
	k_sem_init(&bt_dev.le.pkts, rp->le_max_num, rp->le_max_num);
#endif
}
#endif

static void read_supported_commands_complete(struct net_buf *buf)
{
	struct bt_hci_rp_read_supported_commands *rp = (void *)buf->data;

	BT_DBG("status 0x%02x", rp->status);

	memcpy(bt_dev.supported_commands, rp->commands,
	       sizeof(bt_dev.supported_commands));

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	/*
	 * Report "LE Read Local P-256 Public Key" and "LE Generate DH Key" as
	 * supported if TinyCrypt ECC is used for emulation.
	 */
	if (IS_ENABLED(CONFIG_BT_TINYCRYPT_ECC)) {
		bt_dev.supported_commands[34] |= 0x02;
		bt_dev.supported_commands[34] |= 0x04;
	}
#endif
}

static void read_local_features_complete(struct net_buf *buf)
{
	struct bt_hci_rp_read_local_features *rp = (void *)buf->data;

	BT_DBG("status 0x%02x", rp->status);

	memcpy(bt_dev.features[0], rp->features, sizeof(bt_dev.features[0]));
}

static void le_read_supp_states_complete(struct net_buf *buf)
{
	struct bt_hci_rp_le_read_supp_states *rp = (void *)buf->data;

	BT_DBG("status 0x%02x", rp->status);

	bt_dev.le.states = sys_get_le64(rp->le_states);
}

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
static void le_read_resolving_list_size_complete(struct net_buf *buf)
{
	struct bt_hci_rp_le_read_rl_size *rp = (void *)buf->data;

	BT_DBG("Resolving List size %u", rp->rl_size);

	bt_dev.le.rl_size = rp->rl_size;
}
#endif /* (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) */

#else  /* !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API) */
void process_events(struct k_poll_event *ev, int count)
{
	BT_DBG("count %d", count);

	for (; count; ev++, count--) {
		BT_DBG("ev->state %u", ev->state);

		switch (ev->state) {
		case K_POLL_STATE_SIGNALED:
			break;
		case K_POLL_STATE_FIFO_DATA_AVAILABLE:
			if (ev->tag == BT_EVENT_CMD_TX) {
				//send_cmd();
			} else if (IS_ENABLED(CONFIG_BT_CONN)) {
				struct bt_conn *conn;

				if (ev->tag == BT_EVENT_CONN_TX_QUEUE) {
					conn = CONTAINER_OF(ev->fifo,
							    struct bt_conn,
							    tx_queue);
					bt_conn_process_tx(conn);
				}
			}
			break;
		case K_POLL_STATE_NOT_READY:
			break;
		default:
			BT_WARN("Unexpected k_poll event state %u", ev->state);
			break;
		}
	}
}

static void read_buffer_size_complete(u16_t acl_max_len, uint8_t  sco_max_len, u16_t acl_max_num, u16_t sco_max_num)
{
	u16_t pkts;

	/* If LE-side has buffers we can ignore the BR/EDR values */
	if (bt_dev.le.mtu) {
		return;
	}

	bt_dev.le.mtu = acl_max_len;
	pkts = acl_max_num;

	BT_DBG("ACL BR/EDR buffers: pkts %u mtu %u", pkts, bt_dev.le.mtu);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	atomic_set(&bt_dev.le.pkts, pkts);
#else
	k_sem_init(&bt_dev.le.pkts, pkts, pkts);
#endif
}
#endif /* !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API) */

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
/* command FIFO + conn_change signal + rx FIFO + MAX_CONN */
#define EV_COUNT (3 + CONFIG_BT_MAX_CONN)
#else
/* command FIFO */
#define EV_COUNT 1 + 1
#endif

static struct k_poll_signal hci_rx =
		K_POLL_SIGNAL_INITIALIZER(hci_rx);

void hci_rx_signal()
{
	k_poll_signal_data_recv(&hci_rx, 1);
}

static void hci_tx_thread(void *p1)
{
	static struct k_poll_event events[EV_COUNT] = {
#if  !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
		K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE,
										K_POLL_MODE_NOTIFY_ONLY,
										&bt_dev.cmd_tx_queue, BT_EVENT_CMD_TX),
#endif
		K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_DATA_RECV,
										K_POLL_MODE_NOTIFY_ONLY,
										&hci_rx, BT_EVENT_RX),
	};

	extern void scheduler_loop(struct k_poll_event *events);
	scheduler_loop(events);
}

#else
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
/* command FIFO + conn_change signal + MAX_CONN */
#define EV_COUNT (2 + CONFIG_BT_MAX_CONN)
#else
/* command FIFO */
#define EV_COUNT 1
#endif

static void hci_tx_thread(void *p1)
{
	static struct k_poll_event events[EV_COUNT] = {
#if  !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
		K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE,
										K_POLL_MODE_NOTIFY_ONLY,
										&bt_dev.cmd_tx_queue, BT_EVENT_CMD_TX),
#endif
	};

	extern void scheduler_loop(struct k_poll_event *events);
	scheduler_loop(events);
}
#endif
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int common_init(void)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct net_buf *rsp;
	int err;

	if (!(bt_dev.drv->quirks & BT_QUIRK_NO_RESET)) {
		/* Send HCI_RESET */
		err = bt_hci_cmd_send_sync(BT_HCI_OP_RESET, NULL, &rsp);
		if (err) {
			return err;
		}
		hci_reset_complete(rsp);
		net_buf_unref(rsp);
	}

	/* Read Local Supported Features */
	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_LOCAL_FEATURES, NULL, &rsp);
	if (err) {
		return err;
	}
	read_local_features_complete(rsp);
	net_buf_unref(rsp);

	/* Read Local Version Information */
	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_LOCAL_VERSION_INFO, NULL,
				   &rsp);
	if (err) {
		return err;
	}
	read_local_ver_complete(rsp);
	net_buf_unref(rsp);

	/* Read Local Supported Commands */
	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_SUPPORTED_COMMANDS, NULL,
				   &rsp);
	if (err) {
		return err;
	}
	read_supported_commands_complete(rsp);
	net_buf_unref(rsp);
#else
	int err;

	if (!(bt_dev.drv->quirks & BT_QUIRK_NO_RESET)) {
		err = hci_api_reset();
		if (err)
		{
			return err;
		}
		hci_reset_complete();
	}

	/* Read Local Supported Features */
	err = hci_api_read_local_feature(bt_dev.features[0]);
	if (err) {
		return err;
	}

	/* Read Local Version Information */
	err = hci_api_read_local_version_info(&bt_dev.hci_version, 
											&bt_dev.lmp_version, 
											&bt_dev.hci_revision,
											&bt_dev.lmp_subversion,
											&bt_dev.manufacturer);
	if (err) {
		return err;
	}

	/* Read Local Supported Commands */
	err = hci_api_read_local_support_command(bt_dev.supported_commands);
	if (err) {
		return err;
	}
#endif

	if (IS_ENABLED(CONFIG_BT_HOST_CRYPTO)) {
		/* Initialize the PRNG so that it is safe to use it later
		 * on in the initialization process.
		 */
		extern int bt_crypto_rand_init(void);
		err = bt_crypto_rand_init();
		if (err) {
			return err;
		}
	}

#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL)
	err = set_flow_control();
	if (err) {
		return err;
	}
#endif /* CONFIG_BT_HCI_ACL_FLOW_CONTROL */

	return 0;
}

static int le_set_event_mask(void)
{
	u64_t mask = 0U;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_event_mask *cp_mask;
	struct net_buf *buf;

	/* Set LE event mask */
	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EVENT_MASK, sizeof(*cp_mask));
	if (!buf) {
		return -ENOBUFS;
	}

	cp_mask = net_buf_add(buf, sizeof(*cp_mask));
#endif

	mask |= BT_EVT_MASK_LE_ADVERTISING_REPORT;

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		mask |= BT_EVT_MASK_LE_ADV_SET_TERMINATED;
		mask |= BT_EVT_MASK_LE_SCAN_REQ_RECEIVED;
		mask |= BT_EVT_MASK_LE_EXT_ADVERTISING_REPORT;
		mask |= BT_EVT_MASK_LE_SCAN_TIMEOUT;
		if (IS_ENABLED(CONFIG_BT_PER_ADV_SYNC)) {
			mask |= BT_EVT_MASK_LE_PER_ADV_SYNC_ESTABLISHED;
			mask |= BT_EVT_MASK_LE_PER_ADVERTISING_REPORT;
			mask |= BT_EVT_MASK_LE_PER_ADV_SYNC_LOST;
			mask |= BT_EVT_MASK_LE_PAST_RECEIVED;
		}
	}

	if (IS_ENABLED(CONFIG_BT_CONN)) {
		if ((IS_ENABLED(CONFIG_BT_SMP) &&
		     BT_FEAT_LE_PRIVACY(bt_dev.le.features)) ||
		    (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
		     BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
			/* C24:
			 * Mandatory if the LE Controller supports Connection
			 * State and either LE Feature (LL Privacy) or
			 * LE Feature (Extended Advertising) is supported, ...
			 */
			mask |= BT_EVT_MASK_LE_ENH_CONN_COMPLETE;
		} else {
			mask |= BT_EVT_MASK_LE_CONN_COMPLETE;
		}

		mask |= BT_EVT_MASK_LE_CONN_UPDATE_COMPLETE;
		mask |= BT_EVT_MASK_LE_REMOTE_FEAT_COMPLETE;

		if (BT_FEAT_LE_CONN_PARAM_REQ_PROC(bt_dev.le.features)) {
			mask |= BT_EVT_MASK_LE_CONN_PARAM_REQ;
		}

		if (IS_ENABLED(CONFIG_BT_DATA_LEN_UPDATE) &&
		    BT_FEAT_LE_DLE(bt_dev.le.features)) {
			mask |= BT_EVT_MASK_LE_DATA_LEN_CHANGE;
		}

		if (IS_ENABLED(CONFIG_BT_PHY_UPDATE) &&
		    (BT_FEAT_LE_PHY_2M(bt_dev.le.features) ||
		     BT_FEAT_LE_PHY_CODED(bt_dev.le.features))) {
			mask |= BT_EVT_MASK_LE_PHY_UPDATE_COMPLETE;
		}
	}

	if (IS_ENABLED(CONFIG_BT_SMP) &&
	    BT_FEAT_LE_ENCR(bt_dev.le.features)) {
		mask |= BT_EVT_MASK_LE_LTK_REQUEST;
	}

	/*
	 * If "LE Read Local P-256 Public Key" and "LE Generate DH Key" are
	 * supported we need to enable events generated by those commands.
	 */
	if (IS_ENABLED(CONFIG_BT_ECC) &&
	    (BT_CMD_TEST(bt_dev.supported_commands, 34, 1)) &&
	    (BT_CMD_TEST(bt_dev.supported_commands, 34, 2))) {
		mask |= BT_EVT_MASK_LE_P256_PUBLIC_KEY_COMPLETE;
		mask |= BT_EVT_MASK_LE_GENERATE_DHKEY_COMPLETE;
	}
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	sys_put_le64(mask, cp_mask->events);
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_EVENT_MASK, buf, NULL);
#else
	return hci_api_le_set_event_mask(mask);
#endif
}

static int le_init(void)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_write_le_host_supp *cp_le;
	struct net_buf *buf, *rsp;
	int err;

	/* For now we only support LE capable controllers */
	if (!BT_FEAT_LE(bt_dev.features)) {
		BT_ERR("Non-LE capable controller detected!");
		return -ENODEV;
	}

	/* Read Low Energy Supported Features */
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_READ_LOCAL_FEATURES, NULL,
				   &rsp);
	if (err) {
		return err;
	}

	read_le_features_complete(rsp);
	net_buf_unref(rsp);

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	/* Read LE Buffer Size */
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_READ_BUFFER_SIZE,
				   NULL, &rsp);
	if (err) {
		return err;
	}
	le_read_buffer_size_complete(rsp);
	net_buf_unref(rsp);
#endif

	if (BT_FEAT_BREDR(bt_dev.features)) {
		buf = bt_hci_cmd_create(BT_HCI_OP_LE_WRITE_LE_HOST_SUPP,
					sizeof(*cp_le));
		if (!buf) {
			return -ENOBUFS;
		}

		cp_le = net_buf_add(buf, sizeof(*cp_le));

		/* Explicitly enable LE for dual-mode controllers */
		cp_le->le = 0x01;
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
		cp_le->simul = 0x01;
#else
		cp_le->simul = 0x00;
#endif
		err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_WRITE_LE_HOST_SUPP, buf,
					   NULL);
		if (err) {
			return err;
		}
	}

	/* Read LE Supported States */
	if (BT_CMD_LE_STATES(bt_dev.supported_commands)) {
		err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_READ_SUPP_STATES, NULL,
					   &rsp);
		if (err) {
			return err;
		}

		le_read_supp_states_complete(rsp);
		net_buf_unref(rsp);
	}

	if (IS_ENABLED(CONFIG_BT_CONN) &&
	    IS_ENABLED(CONFIG_BT_DATA_LEN_UPDATE) &&
	    BT_FEAT_LE_DLE(bt_dev.le.features)) {
		struct bt_hci_cp_le_write_default_data_len *cp;
		u16_t tx_octets, tx_time;

		err = hci_le_read_max_data_len(&tx_octets, &tx_time);
		if (err) {
			return err;
		}

		buf = bt_hci_cmd_create(BT_HCI_OP_LE_WRITE_DEFAULT_DATA_LEN,
					sizeof(*cp));
		if (!buf) {
			return -ENOBUFS;
		}

		cp = net_buf_add(buf, sizeof(*cp));
		cp->max_tx_octets = sys_cpu_to_le16(tx_octets);
		cp->max_tx_time = sys_cpu_to_le16(tx_time);

		err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_WRITE_DEFAULT_DATA_LEN,
					   buf, NULL);
		if (err) {
			return err;
		}
	}

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
		struct bt_hci_cp_le_set_rpa_timeout *cp;

		buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_RPA_TIMEOUT,
					sizeof(*cp));
		if (!buf) {
			return -ENOBUFS;
		}

		cp = net_buf_add(buf, sizeof(*cp));
		cp->rpa_timeout = sys_cpu_to_le16(CONFIG_BT_RPA_TIMEOUT);
		err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_RPA_TIMEOUT, buf,
					   NULL);
		if (err) {
			return err;
		}
#endif /* (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY) */

		err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_READ_RL_SIZE, NULL,
					   &rsp);
		if (err) {
			return err;
		}
		le_read_resolving_list_size_complete(rsp);
		net_buf_unref(rsp);
	}
#endif

#else
	int err;
	/* For now we only support LE capable controllers */
	if (!BT_FEAT_LE(bt_dev.features)) {
		BT_ERR("Non-LE capable controller detected!");
		return -ENODEV;
	}

	/* Read Low Energy Supported Features */
	err = hci_api_le_read_local_feature(bt_dev.le.features);
	if (err) {
		return err;
	}


#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	bt_dev.le.mtu_init = 27;
	bt_dev.le.mtu = 27;
	u8_t le_max_num;
	u16_t le_max_mtu;
	/* Read LE Buffer Size */
	err = hci_api_le_read_buffer_size_complete(&le_max_mtu, &le_max_num);
	(void)le_max_mtu;
	if (err) {
		return err;
	}
	if (bt_dev.le.mtu)
	{
		bt_dev.le.mtu_init = bt_dev.le.mtu;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		atomic_set(&bt_dev.le.pkts, le_max_num);
#else
		k_sem_init(&bt_dev.le.pkts, le_max_num, le_max_num);
#endif
	}
#endif

	if (BT_FEAT_BREDR(bt_dev.features)) {
		/* Explicitly enable LE for dual-mode controllers */
		err = hci_api_le_write_host_supp(0x01, 0x00);
		if (err) {
			return err;
		}
	}

	/* Read LE Supported States */
	if (BT_CMD_LE_STATES(bt_dev.supported_commands)) {
		err = hci_api_le_read_support_states(&bt_dev.le.states);
		if (err) {
			return err;
		}
	}

	if (IS_ENABLED(CONFIG_BT_CONN) &&
	    IS_ENABLED(CONFIG_BT_DATA_LEN_UPDATE) &&
	    BT_FEAT_LE_DLE(bt_dev.le.features)) {
		u16_t tx_octets, tx_time;

		err = hci_le_read_max_data_len(&tx_octets, &tx_time);
		if (err) {
			return err;
		}

		err = hci_api_le_set_default_data_len(tx_octets, tx_time);
		if (err) {
			return err;
		}
	}

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
		err = hci_api_le_rpa_timeout_set(CONFIG_BT_RPA_TIMEOUT);
		if (err) {
			return err;
		}
#endif /* (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY) */

		err = hci_api_le_read_rl_size(&bt_dev.le.rl_size);
		if (err) {
			return err;
		}

		BT_DBG("Resolving List size %u", bt_dev.le.rl_size);
	}
#endif /* (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) */

#endif /* (defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API) */
	return  le_set_event_mask();
}
#endif
#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
static int read_ext_features(void)
{
	int i;

	/* Read Local Supported Extended Features */
	for (i = 1; i < LMP_FEAT_PAGES_COUNT; i++) {
		struct bt_hci_cp_read_local_ext_features *cp;
		struct bt_hci_rp_read_local_ext_features *rp;
		struct net_buf *buf, *rsp;
		int err;

		buf = bt_hci_cmd_create(BT_HCI_OP_READ_LOCAL_EXT_FEATURES,
					sizeof(*cp));
		if (!buf) {
			return -ENOBUFS;
		}

		cp = net_buf_add(buf, sizeof(*cp));
		cp->page = i;

		err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_LOCAL_EXT_FEATURES,
					   buf, &rsp);
		if (err) {
			return err;
		}

		rp = (void *)rsp->data;

		memcpy(&bt_dev.features[i], rp->ext_features,
		       sizeof(bt_dev.features[i]));

		if (rp->max_page <= i) {
			net_buf_unref(rsp);
			break;
		}

		net_buf_unref(rsp);
	}

	return 0;
}

void device_supported_pkt_type(void)
{
	/* Device supported features and sco packet types */
	if (BT_FEAT_HV2_PKT(bt_dev.features)) {
		bt_dev.br.esco_pkt_type |= (HCI_PKT_TYPE_ESCO_HV2);
	}

	if (BT_FEAT_HV3_PKT(bt_dev.features)) {
		bt_dev.br.esco_pkt_type |= (HCI_PKT_TYPE_ESCO_HV3);
	}

	if (BT_FEAT_LMP_ESCO_CAPABLE(bt_dev.features)) {
		bt_dev.br.esco_pkt_type |= (HCI_PKT_TYPE_ESCO_EV3);
	}

	if (BT_FEAT_EV4_PKT(bt_dev.features)) {
		bt_dev.br.esco_pkt_type |= (HCI_PKT_TYPE_ESCO_EV4);
	}

	if (BT_FEAT_EV5_PKT(bt_dev.features)) {
		bt_dev.br.esco_pkt_type |= (HCI_PKT_TYPE_ESCO_EV5);
	}

	if (BT_FEAT_2EV3_PKT(bt_dev.features)) {
		bt_dev.br.esco_pkt_type |= (HCI_PKT_TYPE_ESCO_2EV3);
	}

	if (BT_FEAT_3EV3_PKT(bt_dev.features)) {
		bt_dev.br.esco_pkt_type |= (HCI_PKT_TYPE_ESCO_3EV3);
	}

	if (BT_FEAT_3SLOT_PKT(bt_dev.features)) {
		bt_dev.br.esco_pkt_type |= (HCI_PKT_TYPE_ESCO_2EV5 |
					    HCI_PKT_TYPE_ESCO_3EV5);
	}
}

int bt_br_get_eir_data(u8_t *fec_required, u8_t eir_data[240])
{
	int err;
	struct net_buf *buf;
	struct bt_hci_cp_read_ext_inquiry_response *rp;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_EXT_INQUIRY_RESPONSE, NULL, &buf);
	if (err) {
		return err;
	}

	rp = (void *)buf->data;

	if (rp->status)
	{
		BT_ERR("get eir data err %d", rp->status);
		return -EIO;
	}

	*fec_required = rp->fec_required;
	memcpy(eir_data, rp->eir_data, sizeof(rp->eir_data));

	net_buf_unref(buf);
	return 0;
}

int bt_br_set_eir_data(u8_t fec_required, const struct bt_data *eir_data, size_t eir_len)
{
	int err;
	uint8_t eir_data_len;
	struct net_buf *buf;
	struct bt_hci_cp_write_ext_inquiry_response *eir_cp;
	struct bt_ad d[1] = {};

	/* Set ext inquiry response */
	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_EXT_INQUIRY_RESPONSE, sizeof(*eir_cp));
	if (!buf) {
		return -ENOBUFS;
	}

	eir_cp = net_buf_add(buf, sizeof(*eir_cp));
	memset(eir_cp->eir_data, 0, sizeof(eir_cp->eir_data));

	eir_cp->fec_required = fec_required;

	d[0].data = eir_data;
	d[0].len = 1;
	err = set_data_add_complete(eir_cp->eir_data, sizeof(eir_cp->eir_data),
			   d, 1, &eir_data_len);
	if (err) {
		net_buf_unref(buf);
		return err;
	}

	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_EXT_INQUIRY_RESPONSE, buf, NULL);
	if (err) {
		return err;
	}

	return 0;
}

int bt_br_set_cod(u16_t service_clase, u8_t major_class, u8_t minor_class)
{
	int err;
	struct net_buf *buf;
	struct bt_hci_cp_write_class_of_device *cod_cp;
	u32_t cod = 0;

	cod = ((u32_t)service_clase << 13) | ((u32_t)major_class << 8) | (minor_class << 2);

	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, sizeof(*cod_cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cod_cp = net_buf_add(buf, sizeof(*cod_cp));

	cod_cp->cod[0] = cod & 0xff;
	cod_cp->cod[1] = (cod >> 8) & 0xff;
	cod_cp->cod[2] = (cod >> 16) & 0xff;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, buf, NULL);
	if (err) {
		return err;
	}

	return 0;
}

int bt_br_get_cod(u16_t *service_clase, u8_t *major_class, u8_t *minor_class)
{
	int err;
	struct net_buf *buf;
	struct bt_hci_cp_read_class_of_device *rp;
	u32_t cod = 0;

	if (NULL == service_clase || NULL == major_class || NULL == minor_class)
	{
		return -EINVAL;
	}

	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_CLASS_OF_DEVICE, NULL, &buf);
	if (err) {
		return err;
	}

	rp = (void *)buf->data;

	if (rp->status)
	{
		BT_ERR("read cod err %d", rp->status);
		return -EIO;
	}

	cod = (u32_t)(rp->cod[0]) | (rp->cod[1] << 8) | (rp->cod[2] << 16);

	*service_clase = (u16_t)(cod >> 13) & 0x7ff;
	*major_class   = (u8_t)(cod >> 8) & 0x1f;
	*minor_class   = (u8_t)(cod >> 2) & 0x3f;

	net_buf_unref(buf);

	return 0;
}

int bt_br_write_local_name(const char *name)
{
	int err;
	struct net_buf *buf;
	struct bt_hci_write_local_name *name_cp;

	/* Set local name */
	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_LOCAL_NAME, sizeof(*name_cp));
	if (!buf) {
		return -ENOBUFS;
	}

	name_cp = net_buf_add(buf, sizeof(*name_cp));
	memcpy(name_cp->local_name, name, MIN(strlen(name) + 1, sizeof(name_cp->local_name)));

	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_LOCAL_NAME, buf, NULL);
	if (err) {
		return err;
	}

	return 0;
}

static int br_init(void)
{
	struct net_buf *buf;
	struct bt_hci_cp_write_ssp_mode *ssp_cp;
	struct bt_hci_cp_write_inquiry_mode *inq_cp;

	int err;

	/* Read extended local features */
	if (BT_FEAT_EXT_FEATURES(bt_dev.features)) {
		err = read_ext_features();
		if (err) {
			return err;
		}
	}

	/* Add local supported packet types to bt_dev */
	device_supported_pkt_type();

	/* Get BR/EDR buffer size */
	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_BUFFER_SIZE, NULL, &buf);
	if (err) {
		return err;
	}

	read_buffer_size_complete(buf);
	net_buf_unref(buf);

	/* Set SSP mode */
	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_SSP_MODE, sizeof(*ssp_cp));
	if (!buf) {
		return -ENOBUFS;
	}

	ssp_cp = net_buf_add(buf, sizeof(*ssp_cp));
	ssp_cp->mode = 0x01;
	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_SSP_MODE, buf, NULL);
	if (err) {
		return err;
	}

	/* Enable Inquiry results with RSSI or extended Inquiry */
	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_INQUIRY_MODE, sizeof(*inq_cp));
	if (!buf) {
		return -ENOBUFS;
	}

	inq_cp = net_buf_add(buf, sizeof(*inq_cp));
	inq_cp->mode = 0x02;
	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_INQUIRY_MODE, buf, NULL);
	if (err) {
		return err;
	}

	/* Set local name */
	err = bt_br_write_local_name(bt_get_name());
	if (err) {
		return err;
	}

	struct bt_data eir_data[1];
	eir_data[0].type = BT_DATA_NAME_COMPLETE;
	eir_data[0].data_len = strlen(bt_get_name());
	eir_data[0].data = (const u8_t *)bt_get_name();
	err = bt_br_set_eir_data(1, eir_data, 1);
	if (err)
	{
		return err;
	}

	err = bt_br_set_cod(  BT_STACK_COD_SERVICE_RENDERING
			| BT_STACK_COD_SERVICE_CAPTURING
			| BT_STACK_COD_SERVICE_AUDIO,
			BT_STACK_COD_MAJOR_AV, 
			BT_STACK_CODE_MINOR_AV_LOUDSPEAKER);
	if (err)
	{
		return err;
	}

	/* Set page timeout*/
	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_PAGE_TIMEOUT, sizeof(u16_t));
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_le16(buf, CONFIG_BT_PAGE_TIMEOUT);

	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_PAGE_TIMEOUT, buf, NULL);
	if (err) {
		return err;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_PAGE_SCAN_CONF, sizeof(u16_t) * 2);
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_le16(buf, CONFIG_BT_CONN_INTERVAL);
	net_buf_add_le16(buf, CONFIG_BT_CONN_WINDOW);

	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_PAGE_SCAN_CONF, buf, NULL);
	if (err) {
		return err;
	}

	/* Enable BR/EDR SC if supported */
	if (BT_FEAT_SC(bt_dev.features)) {
		struct bt_hci_cp_write_sc_host_supp *sc_cp;

		buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_SC_HOST_SUPP,
					sizeof(*sc_cp));
		if (!buf) {
			return -ENOBUFS;
		}

		sc_cp = net_buf_add(buf, sizeof(*sc_cp));
		sc_cp->sc_support = 0x01;

		err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_SC_HOST_SUPP, buf,
					   NULL);
		if (err) {
			return err;
		}
	}

	return 0;
}
#else
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int br_init(void)
{
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct net_buf *rsp;
	int err;

	if (bt_dev.le.mtu) {
		return 0;
	}

	/* Use BR/EDR buffer size if LE reports zero buffers */
	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_BUFFER_SIZE, NULL, &rsp);
	if (err) {
		return err;
	}

	read_buffer_size_complete(rsp);
	net_buf_unref(rsp);

#else
	int err;
	u16_t acl_max_len = 0;
	u8_t  sco_max_len = 0;
	u16_t acl_max_num = 0;
	u16_t sco_max_num = 0;

	if (bt_dev.le.mtu) {
		return 0;
	}

	/* Use BR/EDR buffer size if LE reports zero buffers */
	err = hci_api_read_buffer_size(&acl_max_len, &sco_max_len, &acl_max_num, &sco_max_num);
	if (err) {
		return err;
	}

	read_buffer_size_complete(acl_max_len, sco_max_len, acl_max_num, sco_max_num);
#endif
#endif /* CONFIG_BT_CONN */
	return 0;
}
#endif
#endif
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int set_event_mask(void)
{
	u64_t mask = 0U;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_set_event_mask *ev;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_SET_EVENT_MASK, sizeof(*ev));
	if (!buf) {
		return -ENOBUFS;
	}

	ev = net_buf_add(buf, sizeof(*ev));
#endif

	if (IS_ENABLED(CONFIG_BT_BREDR)) {
		/* Since we require LE support, we can count on a
		 * Bluetooth 4.0 feature set
		 */
		mask |= BT_EVT_MASK_INQUIRY_COMPLETE;
		mask |= BT_EVT_MASK_CONN_COMPLETE;
		mask |= BT_EVT_MASK_CONN_REQUEST;
		mask |= BT_EVT_MASK_AUTH_COMPLETE;
		mask |= BT_EVT_MASK_REMOTE_NAME_REQ_COMPLETE;
		mask |= BT_EVT_MASK_REMOTE_FEATURES;
		mask |= BT_EVT_MASK_ROLE_CHANGE;
		mask |= BT_EVT_MASK_PIN_CODE_REQ;
		mask |= BT_EVT_MASK_LINK_KEY_REQ;
		mask |= BT_EVT_MASK_LINK_KEY_NOTIFY;
		mask |= BT_EVT_MASK_INQUIRY_RESULT_WITH_RSSI;
		mask |= BT_EVT_MASK_REMOTE_EXT_FEATURES;
		mask |= BT_EVT_MASK_SYNC_CONN_COMPLETE;
		mask |= BT_EVT_MASK_EXTENDED_INQUIRY_RESULT;
		mask |= BT_EVT_MASK_IO_CAPA_REQ;
		mask |= BT_EVT_MASK_IO_CAPA_RESP;
		mask |= BT_EVT_MASK_USER_CONFIRM_REQ;
		mask |= BT_EVT_MASK_USER_PASSKEY_REQ;
		mask |= BT_EVT_MASK_SSP_COMPLETE;
		mask |= BT_EVT_MASK_USER_PASSKEY_NOTIFY;
	}

	mask |= BT_EVT_MASK_HARDWARE_ERROR;
	mask |= BT_EVT_MASK_DATA_BUFFER_OVERFLOW;
	mask |= BT_EVT_MASK_LE_META_EVENT;

	if (IS_ENABLED(CONFIG_BT_CONN)) {
		mask |= BT_EVT_MASK_DISCONN_COMPLETE;
		mask |= BT_EVT_MASK_REMOTE_VERSION_INFO;
	}

	if (IS_ENABLED(CONFIG_BT_SMP) &&
	    BT_FEAT_LE_ENCR(bt_dev.le.features)) {
		mask |= BT_EVT_MASK_ENCRYPT_CHANGE;
		mask |= BT_EVT_MASK_ENCRYPT_KEY_REFRESH_COMPLETE;
	}
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	sys_put_le64(mask, ev->events);
	return bt_hci_cmd_send_sync(BT_HCI_OP_SET_EVENT_MASK, buf, NULL);
#else
	return hci_api_set_event_mask(mask);
#endif
}
#endif
static inline int create_random_addr(bt_addr_le_t *addr)
{
	addr->type = BT_ADDR_LE_RANDOM;

	return bt_rand(addr->a.val, 6);
}

int bt_addr_le_create_nrpa(bt_addr_le_t *addr)
{
	int err;

	err = create_random_addr(addr);
	if (err) {
		return err;
	}

	BT_ADDR_SET_NRPA(&addr->a);

	return 0;
}

int bt_addr_le_create_static(bt_addr_le_t *addr)
{
	int err;

	err = create_random_addr(addr);
	if (err) {
		return err;
	}

	BT_ADDR_SET_STATIC(&addr->a);

	return 0;
}
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static u8_t bt_read_public_addr(bt_addr_le_t *addr)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_rp_read_bd_addr *rp;
	struct net_buf *rsp;
	int err;

	/* Read Bluetooth Address */
	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_BD_ADDR, NULL, &rsp);
	if (err) {
		BT_WARN("Failed to read public address");
		return 0U;
	}

	rp = (void *)rsp->data;

	if (!bt_addr_cmp(&rp->bdaddr, BT_ADDR_ANY) ||
	    !bt_addr_cmp(&rp->bdaddr, BT_ADDR_NONE)) {
		BT_DBG("Controller has no public address");
		net_buf_unref(rsp);
		return 0U;
	}

	bt_addr_copy(&addr->a, &rp->bdaddr);
	addr->type = BT_ADDR_LE_PUBLIC;

	net_buf_unref(rsp);
#else
	int err = hci_api_read_bdaddr(addr->a.val);
	if (err) {
		BT_WARN("Failed to read public address");
		return 0U;
	}
	addr->type = BT_ADDR_LE_PUBLIC;
#endif
	return 1U;
}
#endif
#if (defined(CONFIG_BT_DEBUG) && CONFIG_BT_DEBUG)
__attribute__((unused)) static const char *ver_str(u8_t ver)
{
	const char * const str[] = {
		"1.0b", "1.1", "1.2", "2.0", "2.1", "3.0", "4.0", "4.1", "4.2",
		"5.0", "5.1", "5.2"
	};

	if (ver < ARRAY_SIZE(str)) {
		return str[ver];
	}

	return "unknown";
}

static void bt_dev_show_info(void)
{
	int i;

	BT_INFO("Identity%s: %s", bt_dev.id_count > 1 ? "[0]" : "",
		bt_addr_le_str(&bt_dev.id_addr[0]));

	for (i = 1; i < bt_dev.id_count; i++) {
		BT_INFO("Identity[%d]: %s",
			i, bt_addr_le_str(&bt_dev.id_addr[i]));
	}

	BT_INFO("HCI: version %s (0x%02x) revision 0x%04x, manufacturer 0x%04x",
		ver_str(bt_dev.hci_version), bt_dev.hci_version,
		bt_dev.hci_revision, bt_dev.manufacturer);
	BT_INFO("LMP: version %s (0x%02x) subver 0x%04x",
		ver_str(bt_dev.lmp_version), bt_dev.lmp_version,
		bt_dev.lmp_subversion);
}
#else
static inline void bt_dev_show_info(void)
{
}
#endif /* CONFIG_BT_DEBUG */

#if (defined(CONFIG_BT_HCI_VS_EXT) && CONFIG_BT_HCI_VS_EXT)
#if (defined(CONFIG_BT_DEBUG) && CONFIG_BT_DEBUG)
static const char *vs_hw_platform(u16_t platform)
{
	static const char * const plat_str[] = {
		"reserved", "Intel Corporation", "Nordic Semiconductor",
		"NXP Semiconductors" };

	if (platform < ARRAY_SIZE(plat_str)) {
		return plat_str[platform];
	}

	return "unknown";
}

static const char *vs_hw_variant(u16_t platform, u16_t variant)
{
	static const char * const nordic_str[] = {
		"reserved", "nRF51x", "nRF52x", "nRF53x"
	};

	if (platform != BT_HCI_VS_HW_PLAT_NORDIC) {
		return "unknown";
	}

	if (variant < ARRAY_SIZE(nordic_str)) {
		return nordic_str[variant];
	}

	return "unknown";
}

static const char *vs_fw_variant(u8_t variant)
{
	static const char * const var_str[] = {
		"Standard Bluetooth controller",
		"Vendor specific controller",
		"Firmware loader",
		"Rescue image",
	};

	if (variant < ARRAY_SIZE(var_str)) {
		return var_str[variant];
	}

	return "unknown";
}
#endif /* CONFIG_BT_DEBUG */

static void hci_vs_init(void)
{
	union {
		struct bt_hci_rp_vs_read_version_info *info;
		struct bt_hci_rp_vs_read_supported_commands *cmds;
		struct bt_hci_rp_vs_read_supported_features *feat;
	} rp;
	struct net_buf *rsp;
	int err;

	/* If heuristics is enabled, try to guess HCI VS support by looking
	 * at the HCI version and identity address. We haven't set any addresses
	 * at this point. So we need to read the public address.
	 */
	if (IS_ENABLED(CONFIG_BT_HCI_VS_EXT_DETECT)) {
		bt_addr_le_t addr;

		if ((bt_dev.hci_version < BT_HCI_VERSION_5_0) ||
		    bt_read_public_addr(&addr)) {
			BT_WARN("Controller doesn't seem to support "
				"Zephyr vendor HCI");
			return;
		}
	}

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_VERSION_INFO, NULL, &rsp);
	if (err) {
		BT_WARN("Vendor HCI extensions not available");
		return;
	}

	if (IS_ENABLED(CONFIG_BT_HCI_VS_EXT_DETECT) &&
	    rsp->len != sizeof(struct bt_hci_rp_vs_read_version_info)) {
		BT_WARN("Invalid Vendor HCI extensions");
		net_buf_unref(rsp);
		return;
	}

#if (defined(CONFIG_BT_DEBUG) && CONFIG_BT_DEBUG)
	rp.info = (void *)rsp->data;
	BT_INFO("HW Platform: %s (0x%04x)",
		vs_hw_platform(sys_le16_to_cpu(rp.info->hw_platform)),
		sys_le16_to_cpu(rp.info->hw_platform));
	BT_INFO("HW Variant: %s (0x%04x)",
		vs_hw_variant(sys_le16_to_cpu(rp.info->hw_platform),
			      sys_le16_to_cpu(rp.info->hw_variant)),
		sys_le16_to_cpu(rp.info->hw_variant));
	BT_INFO("Firmware: %s (0x%02x) Version %u.%u Build %u",
		vs_fw_variant(rp.info->fw_variant), rp.info->fw_variant,
		rp.info->fw_version, sys_le16_to_cpu(rp.info->fw_revision),
		sys_le32_to_cpu(rp.info->fw_build));
#endif /* CONFIG_BT_DEBUG */

	net_buf_unref(rsp);

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_SUPPORTED_COMMANDS,
				   NULL, &rsp);
	if (err) {
		BT_WARN("Failed to read supported vendor commands");
		return;
	}

	if (IS_ENABLED(CONFIG_BT_HCI_VS_EXT_DETECT) &&
	    rsp->len != sizeof(struct bt_hci_rp_vs_read_supported_commands)) {
		BT_WARN("Invalid Vendor HCI extensions");
		net_buf_unref(rsp);
		return;
	}

	rp.cmds = (void *)rsp->data;
	memcpy(bt_dev.vs_commands, rp.cmds->commands, BT_DEV_VS_CMDS_MAX);
	net_buf_unref(rsp);

	if (BT_VS_CMD_SUP_FEAT(bt_dev.vs_commands)) {
		err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_SUPPORTED_FEATURES,
					   NULL, &rsp);
		if (err) {
			BT_WARN("Failed to read supported vendor features");
			return;
		}

		if (IS_ENABLED(CONFIG_BT_HCI_VS_EXT_DETECT) &&
		    rsp->len !=
		    sizeof(struct bt_hci_rp_vs_read_supported_features)) {
			BT_WARN("Invalid Vendor HCI extensions");
			net_buf_unref(rsp);
			return;
		}

		rp.feat = (void *)rsp->data;
		memcpy(bt_dev.vs_features, rp.feat->features,
		       BT_DEV_VS_FEAT_MAX);
		net_buf_unref(rsp);
	}
}
#endif /* CONFIG_BT_HCI_VS_EXT */
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int hci_init(void)
{
	int err;

	err = common_init();
	if (err) {
		return err;
	}

	err = le_init();
	if (err) {
		return err;
	}

	if (BT_FEAT_BREDR(bt_dev.features)) {
		err = br_init();
		if (err) {
			return err;
		}
	} else if (IS_ENABLED(CONFIG_BT_BREDR)) {
		BT_ERR("Non-BR/EDR controller detected");
		return -EIO;
	}

	err = set_event_mask();
	if (err) {
		return err;
	}

#if (defined(CONFIG_BT_HCI_VS_EXT) && CONFIG_BT_HCI_VS_EXT)
	hci_api_vs_init();
#endif
	if (!IS_ENABLED(CONFIG_BT_SETTINGS) && !bt_dev.id_count) {
		BT_DBG("No user identity. Trying to set public.");

		bt_setup_public_id_addr();
	}

	if (!IS_ENABLED(CONFIG_BT_SETTINGS) && !bt_dev.id_count) {
		BT_DBG("No public address. Trying to set static random.");

		err = bt_setup_random_id_addr();
		if (err) {
			BT_ERR("Unable to set identity address");
			return err;
		}

		/* The passive scanner just sends a dummy address type in the
		 * command. If the first activity does this, and the dummy type
		 * is a random address, it needs a valid value, even though it's
		 * not actually used.
		 */
		err = set_random_address(&bt_dev.id_addr[0].a);
		if (err) {
			BT_ERR("Unable to set random address");
			return err;
		}
	}

	return 0;
}
#endif
int bt_send(struct net_buf *buf)
{
	BT_DBG("buf %p len %u type %u", buf, buf->len, bt_buf_get_type(buf));

	bt_monitor_send(bt_monitor_opcode(buf), buf->data, buf->len);
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	if (IS_ENABLED(CONFIG_BT_TINYCRYPT_ECC)) {
		return bt_hci_ecc_send(buf);
	}
#endif
	return bt_dev.drv->send(buf);
}

int bt_recv(struct net_buf *buf)
{
	bt_monitor_send(bt_monitor_opcode(buf), buf->data, buf->len);

	BT_DBG("buf %p len %u", buf, buf->len);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	switch (bt_buf_get_type(buf)) {
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	case BT_BUF_ACL_IN:
		hci_acl(buf);
		return 0;
#endif /* BT_CONN */
	case BT_BUF_EVT:
		hci_event(buf);
		return 0;
	default:
		BT_ERR("Invalid buf type %u", bt_buf_get_type(buf));
		net_buf_unref(buf);
		return -EINVAL;
	}
#else
	switch (bt_buf_get_type(buf)) {
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	case BT_BUF_ACL_IN:
#if (defined(CONFIG_BT_RECV_IS_RX_THREAD) && CONFIG_BT_RECV_IS_RX_THREAD)
		hci_acl(buf);
#else
		net_buf_put(&bt_dev.rx_queue, buf);
#endif
		return 0;
#endif /* BT_CONN */
	case BT_BUF_EVT:
#if (defined(CONFIG_BT_RECV_IS_RX_THREAD) && CONFIG_BT_RECV_IS_RX_THREAD)
		hci_event(buf);
#else
		net_buf_put(&bt_dev.rx_queue, buf);
#endif
		return 0;
	default:
		BT_ERR("Invalid buf type %u", bt_buf_get_type(buf));
		net_buf_unref(buf);
		return -EINVAL;
	}
#endif
}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
static const struct event_handler prio_events[] = {
	EVENT_HANDLER(BT_HCI_EVT_CMD_COMPLETE, hci_cmd_complete,
		      sizeof(struct bt_hci_evt_cmd_complete)),
	EVENT_HANDLER(BT_HCI_EVT_CMD_STATUS, hci_cmd_status,
		      sizeof(struct bt_hci_evt_cmd_status)),
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	EVENT_HANDLER(BT_HCI_EVT_DATA_BUF_OVERFLOW,
		      hci_data_buf_overflow,
		      sizeof(struct bt_hci_evt_data_buf_overflow)),
	EVENT_HANDLER(BT_HCI_EVT_NUM_COMPLETED_PACKETS,
		      hci_num_completed_packets,
		      sizeof(struct bt_hci_evt_num_completed_packets)),
#endif /* CONFIG_BT_CONN */
};

int bt_recv_prio(struct net_buf *buf)
{
	struct bt_hci_evt_hdr *hdr;

	bt_monitor_send(bt_monitor_opcode(buf), buf->data, buf->len);

	BT_ASSERT(bt_buf_get_type(buf) == BT_BUF_EVT);
	BT_ASSERT(buf->len >= sizeof(*hdr));

	hdr = net_buf_pull_mem(buf, sizeof(*hdr));
	BT_ASSERT(bt_hci_evt_is_prio(hdr->evt));

	handle_event(hdr->evt, buf, prio_events, ARRAY_SIZE(prio_events));

	net_buf_unref(buf);

	return 0;
}
#endif

int bt_hci_driver_register(const struct bt_hci_driver *drv)
{
	if (bt_dev.drv) {
		return -EALREADY;
	}

	if (!drv->open || !drv->send) {
		return -EINVAL;
	}

	bt_dev.drv = drv;

	BT_DBG("Registered %s", drv->name ? drv->name : "");

	bt_monitor_new_index(BT_MONITOR_TYPE_PRIMARY, drv->bus,
			     BT_ADDR_ANY, drv->name ? drv->name : "bt0");

	return 0;
}

void bt_finalize_init(void)
{
	atomic_set_bit(bt_dev.flags, BT_DEV_READY);

	if (IS_ENABLED(CONFIG_BT_OBSERVER)) {
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		bt_fsm_le_scan_update(false, bt_dev.fsm);
#else
		bt_le_scan_update(false);
#endif
	}

	bt_dev_show_info();

	ble_event_init_done(0);
}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int bt_init(void)
{
	return hci_common_init();
}

#else
static int bt_init(void)
{
	int err;

	err = hci_init();
	if (err) {
		return err;
	}

	if (IS_ENABLED(CONFIG_BT_CONN)) {
		err = bt_conn_init();
		if (err) {
			return err;
		}
	}

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	k_delayed_work_init(&bt_dev.rpa_update, rpa_timeout);
#endif

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		if (!bt_dev.id_count) {
			BT_INFO("No ID address. App must call settings_load()");
			ble_event_init_done(0);
			return 0;
		}

		atomic_set_bit(bt_dev.flags, BT_DEV_PRESET_ID);
	}

	bt_finalize_init();
	return 0;
}
#endif
static void init_work(struct k_work *work)
{
	int err;

	err = bt_init();
	if (ready_cb) {
		ready_cb(err);
	}
}
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
#if !(defined(CONFIG_BT_RECV_IS_RX_THREAD) && CONFIG_BT_RECV_IS_RX_THREAD)
static void hci_rx_thread(void * arg)
{
	struct net_buf *buf = NULL;

	BT_DBG("started");

	while (1) {
		BT_DBG("calling fifo_get_wait");
		buf = net_buf_get(&bt_dev.rx_queue, K_FOREVER);
        if(!buf) {
           BT_ERR("buf get failed");
		   continue;
		}
		BT_DBG("buf %p type %u len %u", buf, bt_buf_get_type(buf),
		       buf->len);

		switch (bt_buf_get_type(buf)) {
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
		case BT_BUF_ACL_IN:
			hci_acl(buf);
			break;
#endif /* CONFIG_BT_CONN */
		case BT_BUF_EVT:
			hci_event(buf);
			break;
		default:
			BT_ERR("Unknown buf type %u", bt_buf_get_type(buf));
			net_buf_unref(buf);
			break;
		}

		/* Make sure we don't hog the CPU if the rx_queue never
		 * gets empty.
		 */
		k_yield();
	}
}
#endif /* !CONFIG_BT_RECV_IS_RX_THREAD */
#endif
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
u16_t bt_hci_get_cmd_opcode(struct net_buf *buf)
{
    return cmd(buf)->opcode;
}
#endif
int bt_enable(bt_ready_cb_t cb)
{
	int err;

	if (!bt_dev.drv) {
		BT_ERR("No HCI driver registered");
		return -ENODEV;
	}

	if (atomic_test_and_set_bit(bt_dev.flags, BT_DEV_ENABLE)) {
		return -EALREADY;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		err = bt_settings_init();
		if (err) {
			return err;
		}
	}

	strncpy(bt_dev.name, CONFIG_BT_DEVICE_NAME, sizeof(bt_dev.name) - 1);
	bt_dev.name[sizeof(bt_dev.name) - 1] = '\0';

	NET_BUF_POOL_INIT(hci_rx_pool);
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	NET_BUF_POOL_INIT(num_complete_pool);
#endif /* CONFIG_BT_CONN */

#if (CONFIG_BT_DISCARDABLE_BUF_COUNT > 0)
	NET_BUF_POOL_INIT(discardable_pool);
#endif /* CONFIG_BT_DISCARDABLE_BUF_COUNT */

#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL)
	NET_BUF_POOL_INIT(acl_in_pool);
#endif

	k_work_q_start();
	k_work_init(&bt_dev.init, init_work);

#if (defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	err = hci_api_init();
	if (err) {
		return err;
	}
#endif

#if !(defined(CONFIG_BT_RECV_IS_RX_THREAD) && CONFIG_BT_RECV_IS_RX_THREAD)
	k_fifo_init(&bt_dev.rx_queue);
#endif

	ready_cb = cb;

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	atomic_set(&bt_dev.ncmd, 1);
	k_delayed_work_init(&bt_dev.cmd_sent_work, hci_cmd_sent_work);
	k_fifo_init(&bt_dev.cmd_tx_pending_queue);

	err = bt_fsm_hci_core_init();
	if (err >= 0)
	{
		bt_dev.fsm = err;
	}
	else
	{
		return err;
	}

	hci_cmd_fsm_init();
#else
#if !(defined(CONFIG_BT_WAIT_NOP) && CONFIG_BT_WAIT_NOP)
	k_sem_init(&bt_dev.ncmd_sem,1,1);
#else
	k_sem_init(&bt_dev.ncmd_sem,0,1);
#endif /* !(defined(CONFIG_BT_WAIT_NOP) && CONFIG_BT_WAIT_NOP) */
#endif

	NET_BUF_POOL_INIT(hci_cmd_pool);
	k_fifo_init(&bt_dev.cmd_tx_queue);

#endif /* !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API) */

	/* TX thread */
	k_thread_spawn(&tx_thread_data, "hci tx", (uint32_t *)tx_thread_stack,
				K_THREAD_STACK_SIZEOF(tx_thread_stack),
				hci_tx_thread, NULL, CONFIG_BT_HCI_TX_PRIO);

#if !(defined(CONFIG_BT_RECV_IS_RX_THREAD) && CONFIG_BT_RECV_IS_RX_THREAD)
	/* RX thread */
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
#else
	k_thread_spawn(&rx_thread_data, "hci rx", (uint32_t *)rx_thread_stack, K_THREAD_STACK_SIZEOF(rx_thread_stack),\
				hci_rx_thread, NULL, CONFIG_BT_RX_PRIO);
#endif
#endif
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	if (IS_ENABLED(CONFIG_BT_TINYCRYPT_ECC) && IS_ENABLED(CONFIG_BT_ECC)) {
		bt_hci_ecc_init();
	}
#endif
	err = bt_dev.drv->open();
	if (err) {
		BT_ERR("HCI driver open failed (%d)", err);
		return err;
	}

	bt_monitor_send(BT_MONITOR_OPEN_INDEX, NULL, 0);

	if (!cb) {
		return bt_init();
	}

	k_work_submit(&bt_dev.init);
	return 0;
}

static int set_data_add_complete(uint8_t *set_data, uint8_t set_data_len_max,
			const struct bt_ad *ad, size_t ad_len, uint8_t *data_len)
{
	uint8_t set_data_len = 0;

	for (size_t i = 0; i < ad_len; i++) {
		const struct bt_data *data = ad[i].data;

		for (size_t j = 0; j < ad[i].len; j++) {
			size_t len = data[j].data_len;
			uint8_t type = data[j].type;

			/* Check if ad fit in the remaining buffer */
			if ((set_data_len + len + 2) > set_data_len_max) {
				ssize_t shortened_len = set_data_len_max -
							(set_data_len + 2);

				if (!(type == BT_DATA_NAME_COMPLETE &&
				      shortened_len > 0)) {
					BT_ERR("Too big advertising data");
					return -EINVAL;
				}

				type = BT_DATA_NAME_SHORTENED;
				len = shortened_len;
			}

			set_data[set_data_len++] = len + 1;
			set_data[set_data_len++] = type;

			memcpy(&set_data[set_data_len], data[j].data, len);
			set_data_len += len;
		}
	}

	*data_len = set_data_len;
	return 0;
}

static int hci_set_ad(u16_t hci_op, const struct bt_ad *ad, size_t ad_len)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_adv_data *set_data;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(hci_op, sizeof(*set_data));
	if (!buf) {
		return -ENOBUFS;
	}

	set_data = net_buf_add(buf, sizeof(*set_data));
	(void)memset(set_data, 0, sizeof(*set_data));

	err = set_data_add_complete(set_data->data, BT_GAP_ADV_MAX_ADV_DATA_LEN,
			   ad, ad_len, &set_data->len);
	if (err) {
		net_buf_unref(buf);
		return err;
	}
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(hci_op, buf, NULL);
#else
	return bt_hci_cmd_send_sync(hci_op, buf, NULL);
#endif
#else
	u8_t data[31] = {0};
	u8_t len;
	err = set_data_add_complete(data, BT_GAP_ADV_MAX_ADV_DATA_LEN, ad, ad_len, &len);
	if (err) {
		return err;
	}

	if (hci_op == BT_HCI_OP_LE_SET_ADV_DATA)
	{
		return hci_api_le_set_ad_data(len, data);
	}
	else if (hci_op == BT_HCI_OP_LE_SET_SCAN_RSP_DATA)
	{
		return hci_api_le_set_sd_data(len, data);
	}

    return 0;
#endif
}

static int hci_set_adv_ext_complete(struct bt_le_ext_adv *adv, uint16_t hci_op,
				    const struct bt_ad *ad, size_t ad_len)
{

	int err;
	uint8_t max_data_size;

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_ext_adv_data *set_data;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(hci_op, sizeof(*set_data));
	if (!buf) {
		return -ENOBUFS;
	}

	set_data = net_buf_add(buf, sizeof(*set_data));
	(void)memset(set_data, 0, sizeof(*set_data));

	if (atomic_test_bit(adv->flags, BT_ADV_EXT_ADV)) {
		max_data_size = BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN;
	} else {
		max_data_size = BT_GAP_ADV_MAX_ADV_DATA_LEN;
	}

	err = set_data_add_complete(set_data->data, max_data_size, ad, ad_len,
				    &set_data->len);

	if (err) {
		net_buf_unref(buf);
		return err;
	}

	set_data->handle = adv->handle;
	set_data->op = BT_HCI_LE_EXT_ADV_OP_COMPLETE_DATA;
	set_data->frag_pref = BT_HCI_LE_EXT_ADV_FRAG_DISABLED;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(hci_op, buf, NULL);
#else
	return bt_hci_cmd_send_sync(hci_op, buf, NULL);
#endif
#else
	static u8_t data[251] = {0};
	u8_t len;

	if (atomic_test_bit(adv->flags, BT_ADV_EXT_ADV)) {
		max_data_size = BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN;
	} else {
		max_data_size = BT_GAP_ADV_MAX_ADV_DATA_LEN;
	}

	err = set_data_add_complete(data, max_data_size, ad, ad_len, &len);
	if (err) {
		return err;
	}

	if (hci_op == BT_HCI_OP_LE_SET_EXT_ADV_DATA)
	{
		return hci_api_le_set_ext_ad_data(adv->handle, BT_HCI_LE_EXT_ADV_OP_COMPLETE_DATA, BT_HCI_LE_EXT_ADV_FRAG_DISABLED, len, data);
	}
	else if (hci_op == BT_HCI_OP_LE_SET_EXT_SCAN_RSP_DATA)
	{
		return hci_api_le_set_ext_sd_data(adv->handle, BT_HCI_LE_EXT_ADV_OP_COMPLETE_DATA, BT_HCI_LE_EXT_ADV_FRAG_DISABLED, len, data);
	}

#endif
}

static int hci_set_adv_ext_fragmented(struct bt_le_ext_adv *adv, uint16_t hci_op,
				      const struct bt_ad *ad, size_t ad_len)
{
	struct bt_hci_cp_le_set_ext_adv_data *set_data;
	struct net_buf *buf;
	int err;

	for (size_t i = 0; i < ad_len; i++) {

		const struct bt_data *data = ad[i].data;

		for (size_t j = 0; j < ad[i].len; j++) {
			size_t len = data[j].data_len;
			uint8_t type = data[j].type;
			size_t offset = 0;

			/* We can't necessarily set one AD field in a single step. */
			while (offset < data[j].data_len) {
				buf = bt_hci_cmd_create(hci_op, sizeof(*set_data));
				if (!buf) {
					return -ENOBUFS;
				}

				set_data = net_buf_add(buf, sizeof(*set_data));
				(void)memset(set_data, 0, sizeof(*set_data));

				set_data->handle = adv->handle;
				set_data->frag_pref = BT_HCI_LE_EXT_ADV_FRAG_DISABLED;

				/* Determine the operation parameter value. */
				if ((i == 0) && (j == 0) && (offset == 0)) {
					set_data->op = BT_HCI_LE_EXT_ADV_OP_FIRST_FRAG;
				} else if ((i == ad_len - 1) && (j == ad[i].len - 1)) {
					/* The last AD field may be split into
					 * one or two commands.
					 */
					if (offset != 0) {
						/* We can always set the data in two operations
						 * Therefore, we know that this is the last.
						 */
						set_data->op = BT_HCI_LE_EXT_ADV_OP_LAST_FRAG;
					} else if (len + 2 <= BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN) {
						/* First part fits. */
						set_data->op = BT_HCI_LE_EXT_ADV_OP_LAST_FRAG;
					} else {
						/* The data must be split into two
						 * commands.
						 */
						set_data->op = BT_HCI_LE_EXT_ADV_OP_INTERM_FRAG;
					}
				} else {
					set_data->op = BT_HCI_LE_EXT_ADV_OP_INTERM_FRAG;
				}

				if (offset == 0) {
					set_data->len = MIN(len + 2,
							    BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN);
				} else {
					/* No need to take min operation here,
					 * as we can always fit the second part.
					 */
					set_data->len = len - offset;
				}

				if (offset == 0) {
					set_data->data[0] = len + 1;
					set_data->data[1] = type;
					memcpy(&set_data->data[2], data[j].data, set_data->len);
					offset += set_data->len - 2;
				} else {
					memcpy(&set_data->data[0], &data[j].data[offset],
					       set_data->len);
					offset += set_data->len;
				}
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
				err = bt_hci_cmd_send_cb(hci_op, buf, NULL);
#else
				err = bt_hci_cmd_send_sync(hci_op, buf, NULL);
#endif
				if (err) {
					return err;
				}
			}

		}
	}

	return 0;
}

static int hci_set_ad_ext(struct bt_le_ext_adv *adv, uint16_t hci_op,
			  const struct bt_ad *ad, size_t ad_len)
{
	size_t total_len_bytes = 0;

	for (size_t i = 0; i < ad_len; i++) {
		for (size_t j = 0; j < ad[i].len; j++) {
			total_len_bytes += ad[i].data[j].data_len + 2;
		}
	}

	if ((total_len_bytes > BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN) &&
	    atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		/* It is not allowed to set advertising data in multiple
		 * operations while the advertiser is running.
		 */
		return -EAGAIN;
	}

	if (total_len_bytes <= BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN) {
		/* If possible, set all data at once.
		 * This allows us to update advertising data while advertising.
		 */
		return hci_set_adv_ext_complete(adv, hci_op, ad, ad_len);
	} else {
		return hci_set_adv_ext_fragmented(adv, hci_op, ad, ad_len);
	}

	return 0;
}

#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int set_ad(struct bt_le_ext_adv *adv, const struct bt_ad *ad,
		  size_t ad_len)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return hci_set_ad_ext(adv, BT_HCI_OP_LE_SET_EXT_ADV_DATA,
				      ad, ad_len);
	}

	return hci_set_ad(BT_HCI_OP_LE_SET_ADV_DATA, ad, ad_len);
}
#endif
static int set_sd(struct bt_le_ext_adv *adv, const struct bt_ad *sd,
		  size_t sd_len)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return hci_set_ad_ext(adv, BT_HCI_OP_LE_SET_EXT_SCAN_RSP_DATA,
				      sd, sd_len);
	}

	return hci_set_ad(BT_HCI_OP_LE_SET_SCAN_RSP_DATA, sd, sd_len);
}

#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)

struct eir_data_upate_t {
	u8_t find: 1;          /* 0: not find update eir type, 1: found */
	u8_t update: 1;        /* 0: not update, cause not find the type or error happen  1: update success */
	u8_t update_err: 1;    /* 0: success, 1: some error happen, eg. no enough space */
	size_t old_data_num;   /* the origin eir data total number */
	size_t old_data_len;   /* the origin eir data total length */
    struct bt_data *data;  /* the update eir data*/
	struct net_buf *buf;   /* the new updated eir data buffer */
};

#define EIR_DATA_MATCH_NONE       (0)
#define EIR_DATA_MATCH_TYPE_OTHER (1)
#define EIR_DATA_MATCH_TYPE_NAME  (2)

static inline int eir_data_match(struct bt_data *data, struct eir_data_upate_t *update)
{
	if (((update->data->type == BT_DATA_NAME_SHORTENED) || (update->data->type == BT_DATA_NAME_COMPLETE))
	   && ((data->type == BT_DATA_NAME_SHORTENED) || (data->type == BT_DATA_NAME_COMPLETE)))
	{
		return EIR_DATA_MATCH_TYPE_NAME;
	}
	else if (update->data->type == data->type)
	{
		return EIR_DATA_MATCH_TYPE_OTHER;
	}

	return EIR_DATA_MATCH_NONE;
}

static inline int eir_data_update(struct bt_data *data, void *arg)
{
	struct eir_data_upate_t *update = arg;

	if (!update)
	{
		return 1;
	}

	int match = eir_data_match(data, update);

	/* if update eir data type is Name,  handle type BT_DATA_NAME_SHORTENED and BT_DATA_NAME_COMPLETE in same way */
	if (match == EIR_DATA_MATCH_TYPE_NAME)
	{
		update->find = 1;
		size_t append_size = update->data->data_len > data->data_len ? update->data->data_len - data->data_len : 0;
		/* If the reserve space is not enough, use BT_DATA_NAME_SHORTENED type */
		if (append_size && (BT_HCI_EIR_DATA_MAX_SIZE - update->old_data_len) < append_size)
		{
			data->type = BT_DATA_NAME_SHORTENED;
			data->data_len = data->data_len + (BT_HCI_EIR_DATA_MAX_SIZE - update->old_data_len);
			data->data = update->data->data;
		}
		else
		{
			data->type = update->data->type;
			data->data_len = update->data->data_len;
			data->data = update->data->data;
		}
		update->update = 1;
	}
	else if (match == EIR_DATA_MATCH_TYPE_OTHER)
	{
		update->find = 1;
		size_t append_size = update->data->data_len > data->data_len ? update->data->data_len - data->data_len : 0;

		/* If the reserve space is not enough and the type is not Name, return err */
		if (append_size && (BT_HCI_EIR_DATA_MAX_SIZE - update->old_data_len) < append_size)
		{
			update->update_err = 1;
			return 1;
		}

		data->type = update->data->type;
		data->data_len = update->data->data_len;
		data->data = update->data->data;
		update->update = 1;
	}

	/* build new eir data */
	net_buf_add_u8(update->buf, data->data_len + 1);
	net_buf_add_u8(update->buf, data->type);
	net_buf_add_mem(update->buf, data->data, data->data_len);

	return 0;
}

static inline int eir_data_find_update(struct bt_data *data, void *arg)
{
	struct eir_data_upate_t *update = arg;

	if (update)
	{
		int match = eir_data_match(data, update);

		if (match)
		{
			update->find = 1;
		}
	}

	update->old_data_num++;
	update->old_data_len += data->data_len + 2;

	return 0;
}

static inline int eir_data_parse(u8_t *data, size_t data_len, int (* cb)(struct bt_data *data, void *arg), void *cb_arg)
{
    u8_t *pdata = data;
	size_t len = data_len;
    struct bt_data ad = {0};
    int ret;

    while (len) {
        if (pdata[0] == 0) {
            return 0;
        }

        if (len < pdata[0] + 1) {
            return 0;
        };

        ad.data_len = pdata[0] - 1;

        ad.type = pdata[1];

        ad.data = &pdata[2];

        if (cb) {
            ret = cb(&ad, cb_arg);

            if (ret) {
                break;
            }
        }

        len -= (pdata[0] + 1);
        pdata += (pdata[0] + 1);
    }

    return 0;
}

int bt_br_update_eir(struct bt_data *update_data)
{
	int err;
	struct net_buf *buf;
	struct eir_data_upate_t update;
	struct net_buf *rsp_buf;
	struct bt_hci_cp_read_ext_inquiry_response *rp;
	size_t eir_len = sizeof(rp->eir_data);
	struct net_buf_simple_state state;
	u8_t *eir;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_READ_EXT_INQUIRY_RESPONSE, NULL, &rsp_buf);
	if (err) {
		return err;
	}

	rp = (void *)rsp_buf->data;

	if (rp->status)
	{
		BT_ERR("get eir data status %d", rp->status);
		net_buf_unref(rsp_buf);
		return -EIO;
	}

	eir = rp->eir_data;

	memset(&update, 0, sizeof(update));

	update.data = update_data;

	/* find eir data info, eg. total eir data lenght, total eir data number, if update eir data exist*/
	err = eir_data_parse(eir, eir_len, eir_data_find_update, &update);
	if (err)
	{
		BT_ERR("find eir data err %d", err);
		net_buf_unref(rsp_buf);
		return -EINVAL;
	}

	struct bt_hci_cp_write_ext_inquiry_response *eir_cp;

	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_EXT_INQUIRY_RESPONSE, sizeof(*eir_cp));
	if (!buf) {
		net_buf_unref(rsp_buf);
		return -ENOBUFS;
	}

	net_buf_simple_save(&buf->b, &state);

	eir_cp = net_buf_add(buf, sizeof(*eir_cp));

	/* set all eir data to zero */
	memset(eir_cp, 0, sizeof(*eir_cp));

	net_buf_simple_restore(&buf->b, &state);

	/* set fec_quired flag */
	net_buf_add_u8(buf, rp->fec_required);

	/* upate the new eir data */
	if (update.find)
	{
		update.buf = buf;
		/* find eir data info, eg. total eir data lenght, total eir data number, if update eir data exist*/
		err = eir_data_parse(eir, eir_len, eir_data_update, &update);
		if (err || update.update_err)
		{
			BT_ERR("update eir data err %d", err);
			net_buf_unref(buf);
			net_buf_unref(rsp_buf);
			return -EINVAL;
		}

		BT_DBG("update eir type %d", update_data->type);
	}
	else
	{
		if(BT_HCI_EIR_DATA_MAX_SIZE - update.old_data_len < 2)
		{
			BT_ERR("no enough space for new eir data");
			net_buf_unref(buf);
			net_buf_unref(rsp_buf);
			return -ENOMEM;
		}

		net_buf_add_mem(buf, eir, update.old_data_len);

		if ((BT_HCI_EIR_DATA_MAX_SIZE - update.old_data_len) < update_data->data_len + 2)
		{
			/* If the reserve space is not enough, use BT_DATA_NAME_SHORTENED type */
			if (update_data->type == BT_DATA_NAME_SHORTENED || update_data->type == BT_DATA_NAME_COMPLETE)
			{
				net_buf_add_u8(buf, BT_HCI_EIR_DATA_MAX_SIZE - update.old_data_len - 1);
				net_buf_add_u8(buf, BT_DATA_NAME_SHORTENED);
				net_buf_add_mem(buf, update_data->data, BT_HCI_EIR_DATA_MAX_SIZE - update.old_data_len - 2);
			}
			else
			{
				BT_ERR("no enough space for new eir data");
				net_buf_unref(buf);
				net_buf_unref(rsp_buf);
				return -ENOMEM;
			}
		}
		else
		{
			net_buf_add_u8(buf, update_data->data_len + 1);
			net_buf_add_u8(buf, update_data->type);
			net_buf_add_mem(buf, update_data->data, update_data->data_len);
		}
	}

	if (buf->len < sizeof(*eir_cp))
	{
		/* reset buf size */
		net_buf_simple_restore(&buf->b, &state);
		net_buf_add(buf, sizeof(*eir_cp));
	}

	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_EXT_INQUIRY_RESPONSE, buf, NULL);
	if (err) {
		net_buf_unref(rsp_buf);
		return err;
	}

	net_buf_unref(rsp_buf);

	return 0;
}

int bt_br_update_eir_name(const char *name)
{
	struct bt_data update_data;

	update_data.type = BT_DATA_NAME_COMPLETE;
	update_data.data_len = strlen(name);
	update_data.data = (const u8_t *)name;

	return bt_br_update_eir(&update_data);
}
#endif

int bt_set_name(const char *name)
{
#if (defined(CONFIG_BT_DEVICE_NAME_DYNAMIC) && CONFIG_BT_DEVICE_NAME_DYNAMIC)
	struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();
	size_t len = strlen(name);
	int err;

	if (len >= sizeof(bt_dev.name)) {
		return -ENOMEM;
	}

	if ((!bt_dev.name_update_force) && !strcmp(bt_dev.name, name)) {
		return 0;
	}

	strncpy(bt_dev.name, name, sizeof(bt_dev.name) - 1);
	bt_dev.name[sizeof(bt_dev.name) - 1] = '\0';

	/* Update advertising name if in use */
	if (adv && atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME)) {
		struct bt_data data[] = { BT_DATA(BT_DATA_NAME_COMPLETE, name,
						strlen(name)) };
		struct bt_ad sd = { data, ARRAY_SIZE(data) };

		set_sd(adv, &sd, 1);
	}

#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
	/* Update local name */
	err = bt_br_write_local_name(name);
	if (err) {
		return err;
	}

	/* Update device name in EIR */
	err = bt_br_update_eir_name(name);
	if (err) {
		return err;
	}
#endif

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		err = settings_save_one("bt/name", bt_dev.name, len);
		if (err) {
			BT_WARN("Unable to store name");
		}
	}

	bt_dev.name_update_force = 0;

	return 0;
#else
	return -ENOMEM;
#endif
}

const char *bt_get_name(void)
{
#if (defined(CONFIG_BT_DEVICE_NAME_DYNAMIC) && CONFIG_BT_DEVICE_NAME_DYNAMIC)
	return bt_dev.name;
#else
	return CONFIG_BT_DEVICE_NAME;
#endif
}

int bt_set_id_addr(const bt_addr_le_t *addr)
{
	bt_addr_le_t non_const_addr;

	if (atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		BT_ERR("Setting identity not allowed after bt_enable()");
		return -EBUSY;
	}

	bt_addr_le_copy(&non_const_addr, addr);

	return bt_id_create(&non_const_addr, NULL);
}

__WEAK  int hci_api_le_set_bdaddr(uint8_t bdaddr[6])
{
    return -ENOTSUP;
}

__WEAK int bt_set_bdaddr(const bt_addr_le_t *addr)
{
	return hci_api_le_set_bdaddr((uint8_t *)addr->a.val);
}

void bt_id_get(bt_addr_le_t *addrs, size_t *count)
{
	size_t to_copy = MIN(*count, bt_dev.id_count);

	memcpy(addrs, bt_dev.id_addr, to_copy * sizeof(bt_addr_le_t));
	*count = to_copy;
}

static int id_find(const bt_addr_le_t *addr)
{
	u8_t id;

	for (id = 0U; id < bt_dev.id_count; id++) {
		if (!bt_addr_le_cmp(addr, &bt_dev.id_addr[id])) {
			return id;
		}
	}

	return -ENOENT;
}

static void id_create(u8_t id, bt_addr_le_t *addr, u8_t *irk)
{
	if (addr && bt_addr_le_cmp(addr, BT_ADDR_LE_ANY)) {
		bt_addr_le_copy(&bt_dev.id_addr[id], addr);
	} else {
		bt_addr_le_t new_addr;

		do {
			bt_addr_le_create_static(&new_addr);
			/* Make sure we didn't generate a duplicate */
		} while (id_find(&new_addr) >= 0);

		bt_addr_le_copy(&bt_dev.id_addr[id], &new_addr);

		if (addr) {
			bt_addr_le_copy(addr, &bt_dev.id_addr[id]);
		}
	}

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	{
		u8_t zero_irk[16] = { 0 };

		if (irk && memcmp(irk, zero_irk, 16)) {
			memcpy(&bt_dev.irk[id], irk, 16);
		} else {
			bt_rand(&bt_dev.irk[id], 16);
			if (irk) {
				memcpy(irk, &bt_dev.irk[id], 16);
			}
		}
	}
#endif
	/* Only store if stack was already initialized. Before initialization
	 * we don't know the flash content, so it's potentially harmful to
	 * try to write anything there.
	 */
	if (IS_ENABLED(CONFIG_BT_SETTINGS) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		bt_settings_save_id();
	}
}

int bt_id_create(bt_addr_le_t *addr, u8_t *irk)
{
	int new_id;

	if (addr && bt_addr_le_cmp(addr, BT_ADDR_LE_ANY)) {
		if (addr->type != BT_ADDR_LE_RANDOM ||
		    !BT_ADDR_IS_STATIC(&addr->a)) {
			BT_ERR("Only static random identity address supported");
			return -EINVAL;
		}

		if (id_find(addr) >= 0) {
			return -EALREADY;
		}
	}

	if (!IS_ENABLED(CONFIG_BT_PRIVACY) && irk) {
		return -EINVAL;
	}

	if (bt_dev.id_count == ARRAY_SIZE(bt_dev.id_addr)) {
		return -ENOMEM;
	}

	new_id = bt_dev.id_count++;
	id_create(new_id, addr, irk);

	return new_id;
}

int bt_id_reset(u8_t id, bt_addr_le_t *addr, u8_t *irk)
{
	struct adv_id_check_data check_data = {
		.id = id,
		.adv_enabled = false,
	};

	if (addr && bt_addr_le_cmp(addr, BT_ADDR_LE_ANY)) {
		if (addr->type != BT_ADDR_LE_RANDOM ||
		    !BT_ADDR_IS_STATIC(&addr->a)) {
			BT_ERR("Only static random identity address supported");
			return -EINVAL;
		}

		if (id_find(addr) >= 0) {
			return -EALREADY;
		}
	}

	if (!IS_ENABLED(CONFIG_BT_PRIVACY) && irk) {
		return -EINVAL;
	}

	if (id == BT_ID_DEFAULT || id >= bt_dev.id_count) {
		return -EINVAL;
	}

	bt_adv_foreach(adv_id_check_func, &check_data);
	if (check_data.adv_enabled) {
		return -EBUSY;
	}

	if (IS_ENABLED(CONFIG_BT_CONN) &&
	    bt_addr_le_cmp(&bt_dev.id_addr[id], BT_ADDR_LE_ANY)) {
		int err;

		err = bt_unpair(id, NULL);
		if (err) {
			return err;
		}
	}

	id_create(id, addr, irk);

	return id;
}

int bt_id_delete(u8_t id)
{
	struct adv_id_check_data check_data = {
		.id = id,
		.adv_enabled = false,
	};

	if (id == BT_ID_DEFAULT || id >= bt_dev.id_count) {
		return -EINVAL;
	}

	if (!bt_addr_le_cmp(&bt_dev.id_addr[id], BT_ADDR_LE_ANY)) {
		return -EALREADY;
	}

	bt_adv_foreach(adv_id_check_func, &check_data);
	if (check_data.adv_enabled) {
		return -EBUSY;
	}

	if (IS_ENABLED(CONFIG_BT_CONN)) {
		int err;

		err = bt_unpair(id, NULL);
		if (err) {
			return err;
		}
	}

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	(void)memset(bt_dev.irk[id], 0, 16);
#endif
	bt_addr_le_copy(&bt_dev.id_addr[id], BT_ADDR_LE_ANY);

	if (id == bt_dev.id_count - 1) {
		bt_dev.id_count--;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		bt_settings_save_id();
	}

	return 0;
}

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
static void bt_read_identity_root(u8_t *ir)
{
	/* Invalid IR */
	memset(ir, 0, 16);

#if (defined(CONFIG_BT_HCI_VS_EXT) && CONFIG_BT_HCI_VS_EXT)
	struct bt_hci_rp_vs_read_key_hierarchy_roots *rp;
	struct net_buf *rsp;
	int err;

	if (!BT_VS_CMD_READ_KEY_ROOTS(bt_dev.vs_commands)) {
		return;
	}

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_KEY_HIERARCHY_ROOTS, NULL,
				   &rsp);
	if (err) {
		BT_WARN("Failed to read identity root");
		return;
	}

	if (IS_ENABLED(CONFIG_BT_HCI_VS_EXT_DETECT) &&
	    rsp->len != sizeof(struct bt_hci_rp_vs_read_key_hierarchy_roots)) {
		BT_WARN("Invalid Vendor HCI extensions");
		net_buf_unref(rsp);
		return;
	}

	rp = (void *)rsp->data;
	memcpy(ir, rp->ir, 16);

	net_buf_unref(rsp);
#endif /* (defined(CONFIG_BT_HCI_VS_EXT) && CONFIG_BT_HCI_VS_EXT) */
}
#endif /* (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY) */

#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
void bt_setup_public_id_addr(void)
{
	bt_addr_le_t addr;
	u8_t *irk = NULL;

	bt_dev.id_count = bt_read_public_addr(&addr);

	if (!bt_dev.id_count) {
		return;
	}

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	u8_t ir_irk[16];
	u8_t ir[16];

	bt_read_identity_root(ir);

	if (!bt_smp_irk_get(ir, ir_irk)) {
		irk = ir_irk;
	} else if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		atomic_set_bit(bt_dev.flags, BT_DEV_STORE_ID);
	}
#endif /* (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY) */

	id_create(BT_ID_DEFAULT, &addr, irk);
}
#endif

#if (defined(CONFIG_BT_HCI_VS_EXT) && CONFIG_BT_HCI_VS_EXT)
u8_t bt_read_static_addr(struct bt_hci_vs_static_addr addrs[], u8_t size)
{
	struct bt_hci_rp_vs_read_static_addrs *rp;
	struct net_buf *rsp;
	int err, i;
	u8_t cnt;

	if (!BT_VS_CMD_READ_STATIC_ADDRS(bt_dev.vs_commands)) {
		BT_WARN("Read Static Addresses command not available");
		return 0;
	}

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_READ_STATIC_ADDRS, NULL, &rsp);
	if (err) {
		BT_WARN("Failed to read static addresses");
		return 0;
	}

	if (IS_ENABLED(CONFIG_BT_HCI_VS_EXT_DETECT) &&
	    rsp->len < sizeof(struct bt_hci_rp_vs_read_static_addrs)) {
		BT_WARN("Invalid Vendor HCI extensions");
		net_buf_unref(rsp);
		return 0;
	}

	rp = (void *)rsp->data;
	cnt = MIN(rp->num_addrs, size);

	if (IS_ENABLED(CONFIG_BT_HCI_VS_EXT_DETECT) &&
	    rsp->len != (sizeof(struct bt_hci_rp_vs_read_static_addrs) +
			 rp->num_addrs *
			 sizeof(struct bt_hci_vs_static_addr))) {
		BT_WARN("Invalid Vendor HCI extensions");
		net_buf_unref(rsp);
		return 0;
	}

	for (i = 0; i < cnt; i++) {
		memcpy(&addrs[i], rp->a, sizeof(struct bt_hci_vs_static_addr));
	}

	net_buf_unref(rsp);
	if (!cnt) {
		BT_WARN("No static addresses stored in controller");
	}

	return cnt;
}
#endif /* CONFIG_BT_HCI_VS_EXT */

int bt_setup_random_id_addr(void)
{
#if (defined(CONFIG_BT_HCI_VS_EXT) && CONFIG_BT_HCI_VS_EXT) || (defined(CONFIG_BT_CTLR) && CONFIG_BT_CTLR)
	/* Only read the addresses if the user has not already configured one or
	 * more identities (!bt_dev.id_count).
	 */
	if (!bt_dev.id_count) {
		struct bt_hci_vs_static_addr addrs[CONFIG_BT_ID_MAX];

		bt_dev.id_count = bt_read_static_addr(addrs, CONFIG_BT_ID_MAX);

		if (bt_dev.id_count) {
			for (u8_t i = 0; i < bt_dev.id_count; i++) {
				bt_addr_le_t addr;
				u8_t *irk = NULL;
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
				u8_t ir_irk[16];

				if (!bt_smp_irk_get(addrs[i].ir, ir_irk)) {
					irk = ir_irk;
				} else if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
					atomic_set_bit(bt_dev.flags,
						       BT_DEV_STORE_ID);
				}
#endif /* CONFIG_BT_PRIVACY */

				bt_addr_copy(&addr.a, &addrs[i].bdaddr);
				addr.type = BT_ADDR_LE_RANDOM;

				id_create(i, &addr, irk);
			}

			return 0;
		}
	}
#endif /* (defined(CONFIG_BT_HCI_VS_EXT) && CONFIG_BT_HCI_VS_EXT) || (defined(CONFIG_BT_CTLR) && CONFIG_BT_CTLR) */

	if (IS_ENABLED(CONFIG_BT_PRIVACY) && IS_ENABLED(CONFIG_BT_SETTINGS)) {
		atomic_set_bit(bt_dev.flags, BT_DEV_STORE_ID);
	}

	return bt_id_create(NULL, NULL);
}

bool bt_addr_le_is_bonded(u8_t id, const bt_addr_le_t *addr)
{
	if (IS_ENABLED(CONFIG_BT_SMP)) {
		struct bt_keys *keys = bt_keys_find_addr(id, addr);

		/* if there are any keys stored then device is bonded */
		return keys && keys->keys;
	} else {
		return false;
	}
}

#if (defined(CONFIG_BT_PER_ADV) && CONFIG_BT_PER_ADV)

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int le_set_per_adv_param_cb(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	struct bt_le_ext_adv *adv = args;
	if (!status)
	{
		atomic_set_bit(adv->flags, BT_PER_ADV_PARAMS_SET);
	}

	net_buf_unref(buf);

	return status;
}
#endif

int bt_le_per_adv_set_param(struct bt_le_ext_adv *adv,
			    const struct bt_le_per_adv_param *param)
{
	struct bt_hci_cp_le_set_per_adv_param *cp;
	struct net_buf *buf;
	int err;

	if (atomic_test_bit(adv->flags, BT_ADV_SCANNABLE)) {
		return -EINVAL;
	} else if (atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		return -EINVAL;
	}

	if (param->interval_min < 0x0006 ||
	    param->interval_max > 0xFFFF ||
	    param->interval_min > param->interval_max) {
		return -EINVAL;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_PER_ADV_PARAM, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->handle = adv->handle;
	cp->min_interval = sys_cpu_to_le16(param->interval_min);
	cp->max_interval = sys_cpu_to_le16(param->interval_max);

	if (param->options & BT_LE_PER_ADV_OPT_USE_TX_POWER) {
		cp->props |= BT_HCI_LE_ADV_PROP_TX_POWER;
	}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_hci_cmd_cb_t cb = {
		.func = le_set_per_adv_param_cb,
	};

	cb.args = adv;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_PER_ADV_PARAM, buf,
				    &cb);
	if (err) {
		net_buf_unref(buf);
		return err;
	}

	return err;
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_PER_ADV_PARAM, buf, NULL);

	if (err) {
		return err;
	}

	atomic_set_bit(adv->flags, BT_PER_ADV_PARAMS_SET);

	return 0;
#endif
}

int bt_le_per_adv_set_data(const struct bt_le_ext_adv *adv,
			   const struct bt_data *ad, size_t ad_len)
{
	struct bt_hci_cp_le_set_per_adv_data *cp;
	struct net_buf *buf;
	struct bt_ad d = { .data = ad, .len = ad_len };
	int err;

	if (!atomic_test_bit(adv->flags, BT_PER_ADV_PARAMS_SET)) {
		return -EINVAL;
	}

	if (!ad_len || !ad) {
		return -EINVAL;
	}

	if (ad_len > BT_HCI_LE_PER_ADV_FRAG_MAX_LEN) {
		return -EINVAL;
	}


	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_PER_ADV_DATA, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->handle = adv->handle;

	/* TODO: If data is longer than what the controller can manage,
	 * split the data. Read size from controller on boot.
	 */
	cp->op = BT_HCI_LE_PER_ADV_OP_COMPLETE_DATA;

	err = set_data_add_complete(cp->data, BT_HCI_LE_PER_ADV_FRAG_MAX_LEN, &d, 1,
			   &cp->len);
	if (err) {
		return err;
	}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_PER_ADV_DATA, buf, NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_PER_ADV_DATA, buf, NULL);
#endif
	if (err) {
		return err;
	}

	return 0;
}

static int bt_le_per_adv_enable(struct bt_le_ext_adv *adv, bool enable)
{
	struct bt_hci_cp_le_set_per_adv_enable *cp;
	struct net_buf *buf;
	int err;

	/* TODO: We could setup some default ext adv params if not already set*/
	if (!atomic_test_bit(adv->flags, BT_PER_ADV_PARAMS_SET)) {
		return -EINVAL;
	}

	#if 0
	if (atomic_test_bit(adv->flags, BT_PER_ADV_ENABLED) == enable) {
		return -EALREADY;
	}
	#endif

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_PER_ADV_ENABLE, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->handle = adv->handle;
	cp->enable = enable ? 1 : 0;

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	cmd_state_set_init(&cmd(buf)->state, adv->flags,
				  BT_PER_ADV_ENABLED, enable);
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_PER_ADV_ENABLE, buf, NULL);
#else
	struct cmd_state_set state;
	cmd_state_set_init(&state, adv->flags,
				  BT_PER_ADV_ENABLED, enable);
	cmd(buf)->state = &state;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_PER_ADV_ENABLE, buf, NULL);
#endif
	if (err) {
		return err;
	}

	return 0;
}

int bt_le_per_adv_start(struct bt_le_ext_adv *adv)
{
	return bt_le_per_adv_enable(adv, true);
}

int bt_le_per_adv_stop(struct bt_le_ext_adv *adv)
{
	return bt_le_per_adv_enable(adv, false);
}

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
int bt_le_per_adv_set_info_transfer(const struct bt_le_ext_adv *adv,
				    const struct bt_conn *conn,
				    uint16_t service_data)
{
	struct bt_hci_cp_le_per_adv_set_info_transfer *cp;
	struct net_buf *buf;

	if (!BT_FEAT_LE_PAST_SEND(bt_dev.le.features)) {
		return -ENOTSUP;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_PER_ADV_SET_INFO_TRANSFER,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->conn_handle = sys_cpu_to_le16(conn->handle);
	cp->adv_handle = adv->handle;
	cp->service_data = sys_cpu_to_le16(service_data);

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_PER_ADV_SET_INFO_TRANSFER, buf, NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_PER_ADV_SET_INFO_TRANSFER, buf, NULL);
#endif
}
#endif /* CONFIG_BT_CONN */
#endif /* CONFIG_BT_PER_ADV */

#if (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC)
uint8_t bt_le_per_adv_sync_get_index(struct bt_le_per_adv_sync *per_adv_sync)
{
	ptrdiff_t index = per_adv_sync - per_adv_sync_pool;

	__ASSERT(0 <= index && index < ARRAY_SIZE(per_adv_sync_pool),
		 "Invalid per_adv_sync pointer");
	return (uint8_t)index;
}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int le_set_per_adv_create_sync_cb(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	int err;
	struct bt_le_per_adv_sync *per_adv_sync = args;

	if (!status) {
		per_adv_sync_delete(per_adv_sync);
		net_buf_unref(buf);
		return status;
	}

	/* Syncing requires that scan is enabled. If the caller doesn't enable
	 * scan first, we enable it here, and disable it once the sync has been
	 * established. We don't need to use any callbacks since we rely on
	 * the advertiser address in the sync params.
	 */
	if (!atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
		err = bt_fsm_le_scan_update(true, bt_dev.fsm);

		if (err) {
			bt_le_per_adv_sync_delete(per_adv_sync);
			return err;
		}
	}

	net_buf_unref(buf);

	return 0;
}
#endif

int bt_le_per_adv_sync_create(const struct bt_le_per_adv_sync_param *param,
			      struct bt_le_per_adv_sync **out_sync)
{
	struct bt_hci_cp_le_per_adv_create_sync *cp;
	struct net_buf *buf;
	struct bt_le_per_adv_sync *per_adv_sync;
	int err;

	if (!BT_FEAT_LE_EXT_PER_ADV(bt_dev.le.features)) {
		return -ENOTSUP;
	}

	if (get_pending_per_adv_sync()) {
		return -EBUSY;
	}

	if (param->sid > BT_GAP_SID_MAX ||
		   param->skip > BT_GAP_PER_ADV_MAX_MAX_SKIP ||
		   param->timeout > BT_GAP_PER_ADV_MAX_MAX_TIMEOUT) {
		return -EINVAL;
	}

	per_adv_sync = per_adv_sync_new();
	if (!per_adv_sync) {
		return -ENOMEM;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_PER_ADV_CREATE_SYNC, sizeof(*cp));
	if (!buf) {
		per_adv_sync_delete(per_adv_sync);
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));


	bt_addr_le_copy(&cp->addr, &param->addr);

	if (param->options & BT_LE_PER_ADV_SYNC_OPT_USE_PER_ADV_LIST) {
		cp->options |= BT_HCI_LE_PER_ADV_CREATE_SYNC_FP_USE_LIST;
	}

	if (param->options & BT_LE_PER_ADV_SYNC_OPT_DONT_SYNC_AOA) {
		cp->cte_type |= BT_HCI_LE_PER_ADV_CREATE_SYNC_CTE_TYPE_NO_AOA;
	}

	if (param->options & BT_LE_PER_ADV_SYNC_OPT_DONT_SYNC_AOD_1US) {
		cp->cte_type |=
			BT_HCI_LE_PER_ADV_CREATE_SYNC_CTE_TYPE_NO_AOD_1US;
	}

	if (param->options & BT_LE_PER_ADV_SYNC_OPT_DONT_SYNC_AOD_2US) {
		cp->cte_type |=
			BT_HCI_LE_PER_ADV_CREATE_SYNC_CTE_TYPE_NO_AOD_2US;
	}

	if (param->options & BT_LE_PER_ADV_SYNC_OPT_SYNC_ONLY_CONST_TONE_EXT) {
		cp->cte_type |= BT_HCI_LE_PER_ADV_CREATE_SYNC_CTE_TYPE_ONLY_CTE;
	}

	if (param->options &
	    BT_LE_PER_ADV_SYNC_OPT_REPORTING_INITIALLY_DISABLED) {
		cp->options |=
			BT_HCI_LE_PER_ADV_CREATE_SYNC_FP_REPORTS_DISABLED;

		atomic_set_bit(per_adv_sync->flags,
			       BT_PER_ADV_SYNC_RECV_DISABLED);
	}

	cp->sid = param->sid;
	cp->skip = sys_cpu_to_le16(param->skip);
	cp->sync_timeout = sys_cpu_to_le16(param->timeout);

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	bt_hci_cmd_cb_t cb = {
		.func = le_set_per_adv_create_sync_cb,
	};

	cb.args = per_adv_sync;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_PER_ADV_CREATE_SYNC, buf, &cb);
	if (err)
	{
		net_buf_unref(buf);
		return err;
	}

	atomic_set_bit(per_adv_sync->flags, BT_PER_ADV_SYNC_SYNCING);
	*out_sync = per_adv_sync;
	bt_addr_le_copy(&per_adv_sync->addr, &param->addr);
	per_adv_sync->sid = param->sid;
	return 0;
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_PER_ADV_CREATE_SYNC, buf, NULL);
	if (err) {
		per_adv_sync_delete(per_adv_sync);
		return err;
	}

	atomic_set_bit(per_adv_sync->flags, BT_PER_ADV_SYNC_SYNCING);

	/* Syncing requires that scan is enabled. If the caller doesn't enable
	 * scan first, we enable it here, and disable it once the sync has been
	 * established. We don't need to use any callbacks since we rely on
	 * the advertiser address in the sync params.
	 */
	if (!atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
		err = bt_le_scan_update(true);

		if (err) {
			bt_le_per_adv_sync_delete(per_adv_sync);
			return err;
		}
	}

	*out_sync = per_adv_sync;
	bt_addr_le_copy(&per_adv_sync->addr, &param->addr);
	per_adv_sync->sid = param->sid;

	return 0;
#endif
}

static int bt_le_per_adv_sync_create_cancel(
	struct bt_le_per_adv_sync *per_adv_sync)
{
	struct net_buf *buf;
	int err;

	if (get_pending_per_adv_sync() != per_adv_sync) {
		return -EINVAL;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_PER_ADV_CREATE_SYNC_CANCEL, 0);
	if (!buf) {
		return -ENOBUFS;
	}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_PER_ADV_CREATE_SYNC_CANCEL, buf,
				    NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_PER_ADV_CREATE_SYNC_CANCEL, buf,
				   NULL);
#endif

	if (err) {
		return err;
	}

	return 0;
}

static int bt_le_per_adv_sync_terminate(struct bt_le_per_adv_sync *per_adv_sync)
{
	int err;

	if (!atomic_test_bit(per_adv_sync->flags, BT_PER_ADV_SYNC_SYNCED)) {
		return -EINVAL;
	}

	err = per_adv_sync_terminate(per_adv_sync->handle);

	if (err) {
		return err;
	}

	return 0;
}

int bt_le_per_adv_sync_delete(struct bt_le_per_adv_sync *per_adv_sync)
{
	int err = 0;

	if (atomic_test_bit(per_adv_sync->flags, BT_PER_ADV_SYNC_SYNCED)) {
		err = bt_le_per_adv_sync_terminate(per_adv_sync);

		if (!err) {
			per_adv_sync_delete(per_adv_sync);
		}
	} else if (get_pending_per_adv_sync() == per_adv_sync) {
		err = bt_le_per_adv_sync_create_cancel(per_adv_sync);
		/* Delete of the per_adv_sync will be done in the event
		 * handler when cancelling */
	}

	return err;
}

void bt_le_per_adv_sync_cb_register(struct bt_le_per_adv_sync_cb *cb)
{
	sys_slist_append(&pa_sync_cbs, &cb->node);
}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int le_set_per_adv_recv_enable_cb(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	struct bt_le_per_adv_sync_state_info info;
	struct bt_le_per_adv_sync *per_adv_sync = args;
	struct bt_le_per_adv_sync_cb *listener;

	if (!status)
	{
		info.recv_enabled = !atomic_test_bit(per_adv_sync->flags,
					     BT_PER_ADV_SYNC_RECV_DISABLED);

		SYS_SLIST_FOR_EACH_CONTAINER(&pa_sync_cbs, listener, node) {
			if (listener->state_changed) {
				listener->state_changed(per_adv_sync, &info);
			}
		}
	}

	net_buf_unref(buf);

	return 0;
}
#endif

static int bt_le_set_per_adv_recv_enable(
	struct bt_le_per_adv_sync *per_adv_sync, bool enable)
{
	struct bt_hci_cp_le_set_per_adv_recv_enable *cp;
	struct net_buf *buf;
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (!BT_FEAT_LE_EXT_PER_ADV(bt_dev.le.features)) {
		return -ENOTSUP;
	}

	if (!atomic_test_bit(per_adv_sync->flags, BT_PER_ADV_SYNC_SYNCED)) {
		return -EINVAL;
	}

	if ((enable && !atomic_test_bit(per_adv_sync->flags,
					BT_PER_ADV_SYNC_RECV_DISABLED)) ||
	    (!enable && atomic_test_bit(per_adv_sync->flags,
					BT_PER_ADV_SYNC_RECV_DISABLED))) {
		return -EALREADY;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_PER_ADV_RECV_ENABLE,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->handle = sys_cpu_to_le16(per_adv_sync->handle);
	cp->enable = enable ? 1 : 0;

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	cmd_state_set_init(&cmd(buf)->state, per_adv_sync->flags,
				  BT_PER_ADV_SYNC_RECV_DISABLED,
				  enable);

	bt_hci_cmd_cb_t cb = {
		.func = le_set_per_adv_recv_enable_cb,
	};

	cb.args = per_adv_sync;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_PER_ADV_RECV_ENABLE, buf, &cb);
	if (err)
	{
		net_buf_unref(buf);
	}

	return err;
#else
	struct bt_le_per_adv_sync_cb *listener;
	struct cmd_state_set state;
	struct bt_le_per_adv_sync_state_info info;

	cmd_state_set_init(&state, per_adv_sync->flags,
				  BT_PER_ADV_SYNC_RECV_DISABLED,
				  enable);
	cmd(buf)->state = &state;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_PER_ADV_RECV_ENABLE,
				   buf, NULL);

	if (err) {
		return err;
	}

	info.recv_enabled = !atomic_test_bit(per_adv_sync->flags,
					     BT_PER_ADV_SYNC_RECV_DISABLED);

	SYS_SLIST_FOR_EACH_CONTAINER(&pa_sync_cbs, listener, node) {
		if (listener->state_changed) {
			listener->state_changed(per_adv_sync, &info);
		}
	}

	return 0;
#endif
}

int bt_le_per_adv_sync_recv_enable(struct bt_le_per_adv_sync *per_adv_sync)
{
	return bt_le_set_per_adv_recv_enable(per_adv_sync, true);
}

int bt_le_per_adv_sync_recv_disable(struct bt_le_per_adv_sync *per_adv_sync)
{
	return bt_le_set_per_adv_recv_enable(per_adv_sync, false);
}

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
int bt_le_per_adv_sync_transfer(const struct bt_le_per_adv_sync *per_adv_sync,
				const struct bt_conn *conn,
				uint16_t service_data)
{
	struct bt_hci_cp_le_per_adv_sync_transfer *cp;
	struct net_buf *buf;

	if (!BT_FEAT_LE_PAST_SEND(bt_dev.le.features)) {
		return -ENOTSUP;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_PER_ADV_SYNC_TRANSFER,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->conn_handle = sys_cpu_to_le16(conn->handle);
	cp->sync_handle = sys_cpu_to_le16(per_adv_sync->handle);
	cp->service_data = sys_cpu_to_le16(service_data);

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_PER_ADV_SYNC_TRANSFER, buf,
				    NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_PER_ADV_SYNC_TRANSFER, buf,
				    NULL);
#endif
}

static bool valid_past_param(
	const struct bt_le_per_adv_sync_transfer_param *param)
{
	if (param->skip > 0x01f3 ||
	    param->timeout < 0x000A ||
	    param->timeout > 0x4000) {
		return false;
	}

	return true;
}

static int past_param_set(const struct bt_conn *conn, uint8_t mode,
			  uint16_t skip, uint16_t timeout, uint8_t cte_type)
{
	struct bt_hci_cp_le_past_param *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_PAST_PARAM, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->conn_handle = sys_cpu_to_le16(conn->handle);
	cp->mode = mode;
	cp->skip = sys_cpu_to_le16(skip);
	cp->timeout = sys_cpu_to_le16(timeout);
	cp->cte_type = cte_type;

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_PAST_PARAM, buf,
				    NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_PAST_PARAM, buf, NULL);
#endif
}

static int default_past_param_set(uint8_t mode, uint16_t skip, uint16_t timeout,
				  uint8_t cte_type)
{
	struct bt_hci_cp_le_default_past_param *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_DEFAULT_PAST_PARAM, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->mode = mode;
	cp->skip = sys_cpu_to_le16(skip);
	cp->timeout = sys_cpu_to_le16(timeout);
	cp->cte_type = cte_type;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_DEFAULT_PAST_PARAM, buf,
				    NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_DEFAULT_PAST_PARAM, buf, NULL);
#endif
}

int bt_le_per_adv_sync_transfer_subscribe(
	const struct bt_conn *conn,
	const struct bt_le_per_adv_sync_transfer_param *param)
{
	uint8_t cte_type = 0;

	if (!BT_FEAT_LE_PAST_RECV(bt_dev.le.features)) {
		return -ENOTSUP;
	}

	if (!valid_past_param(param)) {
		return -EINVAL;
	}

	if (param->options & BT_LE_PER_ADV_SYNC_TRANSFER_OPT_SYNC_NO_AOA) {
		cte_type |= BT_HCI_LE_PAST_CTE_TYPE_NO_AOA;
	}

	if (param->options & BT_LE_PER_ADV_SYNC_TRANSFER_OPT_SYNC_NO_AOD_1US) {
		cte_type |= BT_HCI_LE_PAST_CTE_TYPE_NO_AOD_1US;
	}

	if (param->options & BT_LE_PER_ADV_SYNC_TRANSFER_OPT_SYNC_NO_AOD_2US) {
		cte_type |= BT_HCI_LE_PAST_CTE_TYPE_NO_AOD_2US;
	}

	if (param->options & BT_LE_PER_ADV_SYNC_TRANSFER_OPT_SYNC_ONLY_CTE) {
		cte_type |= BT_HCI_LE_PAST_CTE_TYPE_ONLY_CTE;
	}

	if (conn) {
		return past_param_set(conn, BT_HCI_LE_PAST_MODE_SYNC,
				      param->skip, param->timeout, cte_type);
	} else {
		return default_past_param_set(BT_HCI_LE_PAST_MODE_SYNC,
					      param->skip, param->timeout,
					      cte_type);
	}
}

int bt_le_per_adv_sync_transfer_unsubscribe(const struct bt_conn *conn)
{
	if (!BT_FEAT_LE_PAST_RECV(bt_dev.le.features)) {
		return -ENOTSUP;
	}

	if (conn) {
		return past_param_set(conn, BT_HCI_LE_PAST_MODE_NO_SYNC, 0,
				      0x0a, 0);
	} else {
		return default_past_param_set(BT_HCI_LE_PAST_MODE_NO_SYNC, 0,
					      0x0a, 0);
	}
}
#endif /* CONFIG_BT_CONN */

int bt_le_per_adv_list_add(const bt_addr_le_t *addr, uint8_t sid)
{
	struct bt_hci_cp_le_add_dev_to_per_adv_list *cp;
	struct net_buf *buf;
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_ADD_DEV_TO_PER_ADV_LIST,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_le_copy(&cp->addr, addr);
	cp->sid = sid;

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_ADD_DEV_TO_PER_ADV_LIST, buf,
				    NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_ADD_DEV_TO_PER_ADV_LIST, buf,
				   NULL);
#endif
	if (err) {
		BT_ERR("Failed to add device to periodic advertiser list");

		return err;
	}

	return 0;
}

int bt_le_per_adv_list_remove(const bt_addr_le_t *addr, uint8_t sid)
{
	struct bt_hci_cp_le_rem_dev_from_per_adv_list *cp;
	struct net_buf *buf;
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_REM_DEV_FROM_PER_ADV_LIST,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_le_copy(&cp->addr, addr);
	cp->sid = sid;
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_REM_DEV_FROM_PER_ADV_LIST, buf,
				    NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_REM_DEV_FROM_PER_ADV_LIST, buf,
				   NULL);
#endif
	if (err) {
		BT_ERR("Failed to remove device from periodic advertiser list");
		return err;
	}

	return 0;
}

int bt_le_per_adv_list_clear(void)
{
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_CLEAR_PER_ADV_LIST, NULL, NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_CLEAR_PER_ADV_LIST, NULL, NULL);
#endif
	if (err) {
		BT_ERR("Failed to clear periodic advertiser list");
		return err;
	}

	return 0;
}
#endif /* (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC) */

static bool valid_adv_ext_param(const struct bt_le_adv_param *param)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		if (param->peer &&
		    !(param->options & BT_LE_ADV_OPT_EXT_ADV) &&
		    !(param->options & BT_LE_ADV_OPT_CONNECTABLE)) {
			/* Cannot do directed non-connectable advertising
			 * without extended advertising.
			 */
			return false;
		}

		if (param->peer &&
		    (param->options & BT_LE_ADV_OPT_EXT_ADV) &&
		    !(param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY)) {
			/* High duty cycle directed connectable advertising
			 * shall not be used with Extended Advertising.
			 */
			return false;
		}

		if (!(param->options & BT_LE_ADV_OPT_EXT_ADV) &&
		    param->options & (BT_LE_ADV_OPT_EXT_ADV |
				      BT_LE_ADV_OPT_CODED |
				      BT_LE_ADV_OPT_ANONYMOUS |
				      BT_LE_ADV_OPT_USE_TX_POWER)) {
			/* Extended options require extended advertising. */
			return false;
		}

		if ((param->options & BT_LE_ADV_OPT_EXT_ADV) &&
		    (param->options & BT_LE_ADV_OPT_CONNECTABLE) &&
			(param->options & BT_LE_ADV_OPT_SCANNABLE)) {
			/* The Extended advertisement shall not be both connectable and scannable */
			return false;
		}
	}

	if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
	    param->peer &&
	    (param->options & BT_LE_ADV_OPT_USE_IDENTITY) &&
	    (param->options & BT_LE_ADV_OPT_DIR_ADDR_RPA)) {
		/* own addr type used for both RPAs in directed advertising. */
		return false;
	}

	if (param->id >= bt_dev.id_count ||
	    !bt_addr_le_cmp(&bt_dev.id_addr[param->id], BT_ADDR_LE_ANY)) {
		return false;
	}

	if (!(param->options & BT_LE_ADV_OPT_CONNECTABLE)) {
		/*
		 * BT Core 4.2 [Vol 2, Part E, 7.8.5]
		 * The Advertising_Interval_Min and Advertising_Interval_Max
		 * shall not be set to less than 0x00A0 (100 ms) if the
		 * Advertising_Type is set to ADV_SCAN_IND or ADV_NONCONN_IND.
		 */
#ifndef CONFIG_BT_MESH_FAST_ADV
		if (bt_dev.hci_version < BT_HCI_VERSION_5_0 &&
		    param->interval_min < 0x00a0) {
			return false;
		}
#endif
	}

	if ((param->options & (BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY |
			       BT_LE_ADV_OPT_DIR_ADDR_RPA)) &&
	    !param->peer) {
		return false;
	}

	if ((param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY) ||
	    !param->peer) {
		if (param->interval_min > param->interval_max ||
		    param->interval_min < 0x0020 ||
		    param->interval_max > 0x4000) {
			return false;
		}
	}

	return true;
}

static bool valid_adv_param(const struct bt_le_adv_param *param)
{
	if (param->options & BT_LE_ADV_OPT_EXT_ADV) {
		return false;
	}

	if (param->peer && !(param->options & BT_LE_ADV_OPT_CONNECTABLE)) {
		return false;
	}

	return valid_adv_ext_param(param);
}

static inline bool ad_has_name(const struct bt_data *ad, size_t ad_len)
{
	size_t i;

	for (i = 0; i < ad_len; i++) {
		if (ad[i].type == BT_DATA_NAME_COMPLETE ||
		    ad[i].type == BT_DATA_NAME_SHORTENED) {
			return true;
		}
	}

	return false;
}
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int le_adv_update(struct bt_le_ext_adv *adv,
			 const struct bt_data *ad, size_t ad_len,
			 const struct bt_data *sd, size_t sd_len,
			 bool scannable, bool use_name)
{
	struct bt_ad d[2] = {};
	struct bt_data data;
	size_t d_len;
	int err;

	if (use_name) {
		const char *name = bt_get_name();

		if ((ad && ad_has_name(ad, ad_len)) ||
			(sd && ad_has_name(sd, sd_len))) {
			/* Cannot use name if name is already set */
			return -EINVAL;
		}

		data = (struct bt_data)BT_DATA(
			BT_DATA_NAME_COMPLETE,
			name, strlen(name));
	}

	if (ad && ad_len > 0)
	{
		d_len = 1;
		d[0].data = ad;
		d[0].len = ad_len;

		if (use_name && !scannable) {
			d[1].data = &data;
			d[1].len = 1;
			d_len = 2;
		}

		err = set_ad(adv, d, d_len);
		if (err) {
			return err;
		}
	}

	if (scannable) {
		d_len = 1;
		d[0].data = sd;
		d[0].len = sd_len;

		if (use_name) {
			d[1].data = &data;
			d[1].len = 1;
			d_len = 2;
		}

		err = set_sd(adv, d, d_len);
		if (err) {
			return err;
		}
	}

	atomic_set_bit(adv->flags, BT_ADV_DATA_SET);
	return 0;
}

int bt_le_adv_update_data(const struct bt_data *ad, size_t ad_len,
			  const struct bt_data *sd, size_t sd_len)
{
	struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();
	bool scannable, use_name;

	if (!adv) {
		return -EINVAL;
	}

	if (!atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EAGAIN;
	}

	scannable = atomic_test_bit(adv->flags, BT_ADV_SCANNABLE);
	use_name = atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME);

	return le_adv_update(adv, ad, ad_len, sd, sd_len, scannable,
			     use_name);
}
#endif

static u8_t get_filter_policy(u8_t options)
{
	if (!IS_ENABLED(CONFIG_BT_WHITELIST)) {
		return BT_LE_ADV_FP_NO_WHITELIST;
	} else if ((options & BT_LE_ADV_OPT_FILTER_SCAN_REQ) &&
		   (options & BT_LE_ADV_OPT_FILTER_CONN)) {
		return BT_LE_ADV_FP_WHITELIST_BOTH;
	} else if (options & BT_LE_ADV_OPT_FILTER_SCAN_REQ) {
		return BT_LE_ADV_FP_WHITELIST_SCAN_REQ;
	} else if (options & BT_LE_ADV_OPT_FILTER_CONN) {
		return BT_LE_ADV_FP_WHITELIST_CONN_IND;
	} else {
		return BT_LE_ADV_FP_NO_WHITELIST;
	}
}
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int le_adv_set_random_addr(struct bt_le_ext_adv *adv, u32_t options,
				  bool dir_adv, u8_t *own_addr_type)
{
	const bt_addr_le_t *id_addr;
	int err = 0;

	/* Set which local identity address we're advertising with */
	id_addr = &bt_dev.id_addr[adv->id];

	if (options & BT_LE_ADV_OPT_CONNECTABLE) {
		if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
		    !(options & BT_LE_ADV_OPT_USE_IDENTITY)) {
			err = le_adv_set_private_addr(adv);
			if (err) {
				return err;
			}

			if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
				*own_addr_type = BT_HCI_OWN_ADDR_RPA_OR_RANDOM;
			} else {
				*own_addr_type = BT_ADDR_LE_RANDOM;
			}
		} else {
			/*
			 * If Static Random address is used as Identity
			 * address we need to restore it before advertising
			 * is enabled. Otherwise NRPA used for active scan
			 * could be used for advertising.
			 */
			if (id_addr->type == BT_ADDR_LE_RANDOM) {
				err = set_adv_random_address(adv, &id_addr->a);
				if (err) {
					return err;
				}
			}

			*own_addr_type = id_addr->type;
		}

		if (dir_adv) {
			if (IS_ENABLED(CONFIG_BT_SMP) &&
			    !IS_ENABLED(CONFIG_BT_PRIVACY) &&
			    BT_FEAT_LE_PRIVACY(bt_dev.le.features) &&
			    (options & BT_LE_ADV_OPT_DIR_ADDR_RPA)) {
				/* This will not use RPA for our own address
				 * since we have set zeroed out the local IRK.
				 */
				*own_addr_type |= BT_HCI_OWN_ADDR_RPA_MASK;
			}
		}
	} else {
		if (options & BT_LE_ADV_OPT_USE_IDENTITY) {
			if (id_addr->type == BT_ADDR_LE_RANDOM) {
				err = set_adv_random_address(adv, &id_addr->a);
			}

			*own_addr_type = id_addr->type;
		} else if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
			     BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
			/* In case advertising set random address is not
			 * available we must handle the shared random address
			 * problem.
			 */
#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
			bool scan_enabled = false;

			/* If active scan with NRPA is ongoing refresh NRPA */
			if (!IS_ENABLED(CONFIG_BT_PRIVACY) &&
			    !IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY) &&
			    atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING) &&
			    atomic_test_bit(bt_dev.flags, BT_DEV_ACTIVE_SCAN)) {
				scan_enabled = true;
				set_le_scan_enable(false);
			}
#endif /* (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) */
			err = le_adv_set_private_addr(adv);
			*own_addr_type = BT_ADDR_LE_RANDOM;

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
			if (scan_enabled) {
				set_le_scan_enable(true);
			}
#endif /* (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) */
		} else {
			err = le_adv_set_private_addr(adv);
			*own_addr_type = BT_ADDR_LE_RANDOM;
		}

		if (err) {
			return err;
		}
	}

	return 0;
}
#endif
static int le_adv_start_add_conn(const struct bt_le_ext_adv *adv,
				 struct bt_conn **out_conn)
{
	struct adv_id_check_data check_data = {
		.id = adv->id,
		.adv_enabled = false
	};
	struct bt_conn *conn;

	bt_adv_foreach(adv_id_check_connectable_func, &check_data);
	if (check_data.adv_enabled) {
		return -ENOTSUP;
	}

	bt_dev.adv_conn_id = adv->id;

	if (!bt_addr_le_cmp(&adv->target_addr, BT_ADDR_LE_ANY)) {
		/* Undirected advertising */
		conn = bt_conn_add_le(adv->id, BT_ADDR_LE_NONE);
		if (!conn) {
			return -ENOMEM;
		}

		bt_conn_set_state(conn, BT_CONN_CONNECT_ADV);
		*out_conn = conn;
		return 0;
	}

	if (bt_conn_exists_le(adv->id, &adv->target_addr)) {
		return -EINVAL;
	}

	conn = bt_conn_add_le(adv->id, &adv->target_addr);
	if (!conn) {
		return -ENOMEM;
	}

	bt_conn_set_state(conn, BT_CONN_CONNECT_DIR_ADV);
	*out_conn = conn;
	return 0;
}

#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
int bt_le_adv_start_legacy(const struct bt_le_adv_param *param,
			   const struct bt_data *ad, size_t ad_len,
			   const struct bt_data *sd, size_t sd_len)
{
	struct bt_conn *conn = NULL;
	bool dir_adv = (param->peer != NULL), scannable;
	int err;
	struct bt_le_ext_adv *adv;

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_adv_param set_param;
	struct net_buf *buf;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (!valid_adv_param(param)) {
		return -EINVAL;
	}

	if (!bt_le_adv_random_addr_check(param)) {
		return -EINVAL;
	}

	(void)memset(&set_param, 0, sizeof(set_param));

	set_param.min_interval = sys_cpu_to_le16(param->interval_min);
	set_param.max_interval = sys_cpu_to_le16(param->interval_max);
	set_param.channel_map  = param->channel_map ? param->channel_map : 0x07;
	set_param.filter_policy = get_filter_policy(param->options);

	adv = adv_new_legacy();
	if (!adv || atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EALREADY;
	}

	if (adv->id != param->id) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);
	}

	adv->id = param->id;
	bt_dev.adv_conn_id = adv->id;

	err = le_adv_set_random_addr(adv, param->options, dir_adv,
				     &set_param.own_addr_type);
	if (err) {
		return err;
	}

	if (dir_adv) {
		bt_addr_le_copy(&adv->target_addr, param->peer);
	} else {
		bt_addr_le_copy(&adv->target_addr, BT_ADDR_LE_ANY);
	}

	if (param->options & BT_LE_ADV_OPT_CONNECTABLE) {
		scannable = true;

		if (dir_adv) {
			if (param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY) {
				set_param.type = BT_HCI_ADV_DIRECT_IND_LOW_DUTY;
			} else {
				set_param.type = BT_HCI_ADV_DIRECT_IND;
			}

			bt_addr_le_copy(&set_param.direct_addr, param->peer);
		} else {
			set_param.type = BT_HCI_ADV_IND;
		}
	} else {
		scannable = sd || (param->options & BT_LE_ADV_OPT_USE_NAME);

		set_param.type = scannable ? BT_HCI_ADV_SCAN_IND :
					     BT_HCI_ADV_NONCONN_IND;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_ADV_PARAM, sizeof(set_param));
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_mem(buf, &set_param, sizeof(set_param));

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_ADV_PARAM, buf, NULL);
#else
	u8_t type;
	u8_t own_addr_type;
	u8_t peer_addr_type = 0;
	u8_t peer_addr[6] = {0};

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (!valid_adv_param(param)) {
		return -EINVAL;
	}

	if (!bt_le_adv_random_addr_check(param)) {
		return -EINVAL;
	}

	adv = adv_new_legacy();
	if (!adv || atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EALREADY;
	}

	if (adv->id != param->id) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);
	}

	adv->id = param->id;
	bt_dev.adv_conn_id = adv->id;

	err = le_adv_set_random_addr(adv, param->options, dir_adv,
				     &own_addr_type);
	if (err) {
		return err;
	}

	if (dir_adv) {
		bt_addr_le_copy(&adv->target_addr, param->peer);
	} else {
		bt_addr_le_copy(&adv->target_addr, BT_ADDR_LE_ANY);
	}

	if (param->options & BT_LE_ADV_OPT_CONNECTABLE) {
		scannable = true;

		if (dir_adv) {
			if (param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY) {
				type = BT_HCI_ADV_DIRECT_IND_LOW_DUTY;
			} else {
				type = BT_HCI_ADV_DIRECT_IND;
			}

			peer_addr_type = param->peer->type;
			memcpy(peer_addr, param->peer->a.val, 6);
		} else {
			type = BT_HCI_ADV_IND;
		}
	} else {
		scannable = sd || (param->options & BT_LE_ADV_OPT_USE_NAME);

		type = scannable ? BT_HCI_ADV_SCAN_IND :
					     BT_HCI_ADV_NONCONN_IND;
	}

	err = hci_api_le_adv_param(param->interval_min, 
								param->interval_max,
								type,
								own_addr_type,
								peer_addr_type,
								peer_addr,
								param->channel_map ? param->channel_map : 0x07,
								get_filter_policy(param->options));
#endif
	if (err) {
		return err;
	}
	if (!dir_adv) {
		err = le_adv_update(adv, ad, ad_len, sd, sd_len, scannable,
				    param->options & BT_LE_ADV_OPT_USE_NAME);
		if (err) {
			return err;
		}
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    (param->options & BT_LE_ADV_OPT_CONNECTABLE)) {
		err = le_adv_start_add_conn(adv, &conn);
		if (err) {
			return err;
		}
	}

	err = set_le_adv_enable(adv, true);
	if (err) {
		BT_ERR("Failed to start advertiser");
		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
			bt_conn_unref(conn);
		}

		return err;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
		/* If undirected connectable advertiser we have created a
		 * connection object that we don't yet give to the application.
		 * Since we don't give the application a reference to manage in
		 * this case, we need to release this reference here
		 */
		bt_conn_unref(conn);
	}

	atomic_set_bit_to(adv->flags, BT_ADV_PERSIST, !dir_adv &&
			  !(param->options & BT_LE_ADV_OPT_ONE_TIME));

	atomic_set_bit_to(adv->flags, BT_ADV_INCLUDE_NAME,
			  param->options & BT_LE_ADV_OPT_USE_NAME);

	atomic_set_bit_to(adv->flags, BT_ADV_CONNECTABLE,
			  param->options & BT_LE_ADV_OPT_CONNECTABLE);

	atomic_set_bit_to(adv->flags, BT_ADV_SCANNABLE, scannable);

	atomic_set_bit_to(adv->flags, BT_ADV_USE_IDENTITY,
			  param->options & BT_LE_ADV_OPT_USE_IDENTITY);

	return 0;
}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
static int le_ext_adv_param_set(struct bt_le_ext_adv *adv,
				const struct bt_le_adv_param *param,
				bool  has_scan_data)
{
	bool dir_adv = param->peer != NULL, scannable;
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_ext_adv_param *cp;
	struct net_buf *buf, *rsp;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EXT_ADV_PARAM, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	err = le_adv_set_random_addr(adv, param->options, dir_adv,
				     &cp->own_addr_type);
	if (err) {
		return err;
	}

	if (dir_adv) {
		bt_addr_le_copy(&adv->target_addr, param->peer);
	} else {
		bt_addr_le_copy(&adv->target_addr, BT_ADDR_LE_ANY);
	}

	cp->handle = adv->handle;
	sys_put_le24(param->interval_min, cp->prim_min_interval);
	sys_put_le24(param->interval_max, cp->prim_max_interval);
	cp->prim_channel_map  = 0x07;
	cp->filter_policy = get_filter_policy(param->options);
	cp->tx_power = BT_HCI_LE_ADV_TX_POWER_NO_PREF;

	cp->prim_adv_phy = BT_HCI_LE_PHY_1M;
	if (param->options & BT_LE_ADV_OPT_EXT_ADV) {
		if (param->options & BT_LE_ADV_OPT_NO_2M) {
			cp->sec_adv_phy = BT_HCI_LE_PHY_1M;
		} else {
			cp->sec_adv_phy = BT_HCI_LE_PHY_2M;
		}
	}

	if (param->options & BT_LE_ADV_OPT_CODED) {
		cp->prim_adv_phy = BT_HCI_LE_PHY_CODED;
		cp->sec_adv_phy = BT_HCI_LE_PHY_CODED;
	}

	if (!(param->options & BT_LE_ADV_OPT_EXT_ADV)) {
		cp->props |= BT_HCI_LE_ADV_PROP_LEGACY;
	}

	if (param->options & BT_LE_ADV_OPT_USE_TX_POWER) {
		cp->props |= BT_HCI_LE_ADV_PROP_TX_POWER;
	}

	if (param->options & BT_LE_ADV_OPT_ANONYMOUS) {
		cp->props |= BT_HCI_LE_ADV_PROP_ANON;
	}

	if (param->options & BT_LE_ADV_OPT_NOTIFY_SCAN_REQ) {
		cp->scan_req_notify_enable = BT_HCI_LE_ADV_SCAN_REQ_ENABLE;
	}

	if (param->options & BT_LE_ADV_OPT_CONNECTABLE) {
		cp->props |= BT_HCI_LE_ADV_PROP_CONN;
		if (!dir_adv && !(param->options & BT_LE_ADV_OPT_EXT_ADV)) {
			/* When using non-extended adv packets then undirected
			 * advertising has to be scannable as well.
			 * We didn't require this option to be set before, so
			 * it is implicitly set instead in this case.
			 */
			cp->props |= BT_HCI_LE_ADV_PROP_SCAN;
		}
	}

	if ((param->options & BT_LE_ADV_OPT_SCANNABLE) || has_scan_data) {
		cp->props |= BT_HCI_LE_ADV_PROP_SCAN;
	}

	scannable = !!(cp->props & BT_HCI_LE_ADV_PROP_SCAN);

	if (dir_adv) {
		cp->props |= BT_HCI_LE_ADV_PROP_DIRECT;
		if (!(param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY)) {
			cp->props |= BT_HCI_LE_ADV_PROP_HI_DC_CONN;
		}

		bt_addr_le_copy(&cp->peer_addr, param->peer);
	}

    cp->sec_adv_max_skip = param->secondary_max_skip;
	cp->sid = param->sid;
    cp->tx_power = 0;
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_EXT_ADV_PARAM, buf, &rsp);
	if (err) {
		return err;
	}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	struct bt_hci_rp_le_set_ext_adv_param *rp = (void *)rsp->data;

	adv->tx_power = rp->tx_power;
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */

	net_buf_unref(rsp);
#else
	u8_t own_addr_type;
	u8_t peer_addr_type = 0;
	u8_t peer_addr[6] = {0};
	u8_t prim_adv_phy = 0;
	u8_t sec_adv_phy = 0;
	u16_t props = 0;
	u8_t scan_req_notify_enable = 0;

	err = le_adv_set_random_addr(adv, param->options, dir_adv,
				     &own_addr_type);
	if (err) {
		return err;
	}

	if (dir_adv) {
		bt_addr_le_copy(&adv->target_addr, param->peer);
	} else {
		bt_addr_le_copy(&adv->target_addr, BT_ADDR_LE_ANY);
	}

	if (param->options & BT_LE_ADV_OPT_EXT_ADV) {
		if (param->options & BT_LE_ADV_OPT_NO_2M) {
			sec_adv_phy = BT_HCI_LE_PHY_1M;
		} else {
			sec_adv_phy = BT_HCI_LE_PHY_2M;
		}
	}

	prim_adv_phy = BT_HCI_LE_PHY_1M;
	if (param->options & BT_LE_ADV_OPT_CODED) {
		prim_adv_phy = BT_HCI_LE_PHY_CODED;
		sec_adv_phy = BT_HCI_LE_PHY_CODED;
	}

	if (!(param->options & BT_LE_ADV_OPT_EXT_ADV)) {
		props |= BT_HCI_LE_ADV_PROP_LEGACY;
	}

	if (param->options & BT_LE_ADV_OPT_USE_TX_POWER) {
		props |= BT_HCI_LE_ADV_PROP_TX_POWER;
	}

	if (param->options & BT_LE_ADV_OPT_ANONYMOUS) {
		props |= BT_HCI_LE_ADV_PROP_ANON;
	}

	if (param->options & BT_LE_ADV_OPT_NOTIFY_SCAN_REQ) {
		scan_req_notify_enable = BT_HCI_LE_ADV_SCAN_REQ_ENABLE;
	}

	if (param->options & BT_LE_ADV_OPT_CONNECTABLE) {
		props |= BT_HCI_LE_ADV_PROP_CONN;
		if (!dir_adv && !(param->options & BT_LE_ADV_OPT_EXT_ADV)) {
			/* When using non-extended adv packets then undirected
			 * advertising has to be scannable as well.
			 * We didn't require this option to be set before, so
			 * it is implicitly set instead in this case.
			 */
			props |= BT_HCI_LE_ADV_PROP_SCAN;
		}
	}

	if ((param->options & BT_LE_ADV_OPT_SCANNABLE) || has_scan_data) {
		props |= BT_HCI_LE_ADV_PROP_SCAN;
	}

	scannable = !!(props & BT_HCI_LE_ADV_PROP_SCAN);

	if (dir_adv) {
		props |= BT_HCI_LE_ADV_PROP_DIRECT;
		if (!(param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY)) {
			props |= BT_HCI_LE_ADV_PROP_HI_DC_CONN;
		}

		peer_addr_type = param->peer->type;
		memcpy(peer_addr, param->peer->a.val, 6);
	}

    int8_t tx_power = 0;
	err = hci_api_le_ext_adv_param_set (adv->handle,
										props,
										param->interval_min,
										param->interval_max,
										0x07,
										own_addr_type,
										peer_addr_type,
										peer_addr,
										get_filter_policy(param->options),
										BT_HCI_LE_ADV_TX_POWER_NO_PREF,
										prim_adv_phy,
										0,
										sec_adv_phy,
										param->sid,
										scan_req_notify_enable,
										&tx_power);
	if (err) {
		return err;
	}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	adv->tx_power = tx_power;
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */

#endif
	atomic_set_bit(adv->flags, BT_ADV_PARAMS_SET);

	if (atomic_test_and_clear_bit(adv->flags, BT_ADV_RANDOM_ADDR_PENDING)) {
		err = set_adv_random_address(adv, &adv->random_addr.a);
		if (err) {
			return err;
		}
	}

	/* Flag only used by bt_le_adv_start API. */
	atomic_set_bit_to(adv->flags, BT_ADV_PERSIST, false);

	atomic_set_bit_to(adv->flags, BT_ADV_INCLUDE_NAME,
			  param->options & BT_LE_ADV_OPT_USE_NAME);

	atomic_set_bit_to(adv->flags, BT_ADV_CONNECTABLE,
			  param->options & BT_LE_ADV_OPT_CONNECTABLE);

	atomic_set_bit_to(adv->flags, BT_ADV_SCANNABLE, scannable);

	atomic_set_bit_to(adv->flags, BT_ADV_USE_IDENTITY,
			  param->options & BT_LE_ADV_OPT_USE_IDENTITY);

	atomic_set_bit_to(adv->flags, BT_ADV_EXT_ADV,
			  param->options & BT_LE_ADV_OPT_EXT_ADV);
	return 0;
}

int bt_le_adv_start_ext(struct bt_le_ext_adv *adv,
			const struct bt_le_adv_param *param,
			const struct bt_data *ad, size_t ad_len,
			const struct bt_data *sd, size_t sd_len)
{
	struct bt_le_ext_adv_start_param start_param = {
		.timeout = 0,
		.num_events = 0,
	};
	bool dir_adv = (param->peer != NULL);
	struct bt_conn *conn = NULL;
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (!valid_adv_ext_param(param)) {
		return -EINVAL;
	}

	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EALREADY;
	}

	adv->id = param->id;
	err = le_ext_adv_param_set(adv, param, sd ||
				   (param->options & BT_LE_ADV_OPT_USE_NAME));
	if (err) {
		return err;
	}

	if (!dir_adv) {
		err = bt_le_ext_adv_set_data(adv, ad, ad_len, sd, sd_len);
		if (err) {
			return err;
		}
	} else {
		if (!(param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY)) {
			start_param.timeout =
				BT_GAP_ADV_HIGH_DUTY_CYCLE_MAX_TIMEOUT;
			atomic_set_bit(adv->flags, BT_ADV_LIMITED);
		}
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    (param->options & BT_LE_ADV_OPT_CONNECTABLE)) {
		err = le_adv_start_add_conn(adv, &conn);
		if (err) {
			return err;
		}
	}

	err = set_le_adv_enable_ext(adv, true, &start_param);
	if (err) {
		BT_ERR("Failed to start advertiser");
		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
			bt_conn_unref(conn);
		}

		return err;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
		/* If undirected connectable advertiser we have created a
		 * connection object that we don't yet give to the application.
		 * Since we don't give the application a reference to manage in
		 * this case, we need to release this reference here
		 */
		bt_conn_unref(conn);
	}

	/* Flag always set to false by le_ext_adv_param_set */
	atomic_set_bit_to(adv->flags, BT_ADV_PERSIST, !dir_adv &&
			  !(param->options & BT_LE_ADV_OPT_ONE_TIME));

	return 0;
}
#endif
#endif

#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
int bt_le_adv_start(const struct bt_le_adv_param *param,
		    const struct bt_data *ad, size_t ad_len,
		    const struct bt_data *sd, size_t sd_len)
{
#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		struct bt_le_ext_adv *adv = adv_new_legacy();
		int err;

		if (!adv) {
			return -ENOMEM;
		}

		err = bt_le_adv_start_ext(adv, param, ad, ad_len, sd, sd_len);
		if (err) {
			adv_delete_legacy();
		}

		return err;
	}
#endif
	return bt_le_adv_start_legacy(param, ad, ad_len, sd, sd_len);
}

int bt_le_adv_stop(void)
{
	struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();
	int err;

	if (!adv) {
		BT_ERR("No valid legacy adv");
		return 0;
	}

	/* Make sure advertising is not re-enabled later even if it's not
	 * currently enabled (i.e. BT_DEV_ADVERTISING is not set).
	 */
	atomic_clear_bit(adv->flags, BT_ADV_PERSIST);

	if (!atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		/* Legacy advertiser exists, but is not currently advertising.
		 * This happens when keep advertising behavior is active but
		 * no conn object is available to do connectable advertising.
		 */
		adv_delete_legacy();
		return 0;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		le_adv_stop_free_conn(adv, 0);
	}

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		err = set_le_adv_enable_ext(adv, false, NULL);
		if (err) {
			return err;
		}
	} else {
		err = set_le_adv_enable_legacy(adv, false);
		if (err) {
			return err;
		}
	}

	adv_delete_legacy();

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) &&
	    !IS_ENABLED(CONFIG_BT_PRIVACY) &&
	    !IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY)) {
		/* If scan is ongoing set back NRPA */
		if (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
			set_le_scan_enable(BT_HCI_LE_SCAN_DISABLE);
			le_set_private_addr(BT_ID_DEFAULT);
			set_le_scan_enable(BT_HCI_LE_SCAN_ENABLE);
		}
	}
#endif /* (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) */

	return 0;
}
#endif

static inline int parse_ad(uint8_t *data, uint16_t len, int (* cb)(struct bt_data *data, void *arg), void *cb_arg)
{
    int num = 0;
    uint8_t *pdata = data;
    struct bt_data ad = {0};
    int ret;

    while (len) {
        if (pdata[0] == 0) {
            return num;
        }

        if (len < pdata[0] + 1) {
            return -1;
        };

        ad.data_len = pdata[0] - 1;

        ad.type = pdata[1];

        ad.data = &pdata[2];

        if (cb) {
            ret = cb(&ad, cb_arg);

            if (ret) {
                break;
            }
        }

        num++;
        len -= (pdata[0] + 1);
        pdata += (pdata[0] + 1);
    }

    return num;
}

static inline int adv_ad_callback(struct bt_data *ad, void *arg)
{
    struct bt_data **pad = (struct bt_data **)arg;

    (*pad)->type = ad->type;
    (*pad)->data_len = ad->data_len;
    (*pad)->data = ad->data;
    (*pad)++;
    return 0;
}

int bt_le_adv_start_instant(const struct bt_le_adv_param *param,
		      uint8_t *ad_data, size_t ad_len,
		      uint8_t *sd_data, size_t sd_len)

{
    struct bt_data ad[10] = {0};
    struct bt_data *pad = NULL;
    struct bt_data sd[10] = {0};
    struct bt_data *psd = NULL; 
    int ad_num = 0;
    int sd_num = 0;

    if (ad_data && ad_len)
    {
        pad = ad;
        ad_num = parse_ad(ad_data, ad_len, adv_ad_callback, (void *)&pad);
        pad = ad;
    }

    if (sd_data && sd_len)
    {
        psd = sd;
        sd_num = parse_ad(sd_data, sd_len, adv_ad_callback, (void *)&psd);
        psd = sd;
    }

	return bt_le_adv_start(param, pad, ad_num, psd, sd_num);
}

int bt_le_adv_stop_instant(void)
{
    return bt_le_adv_stop();
}

#if (defined(CONFIG_BT_PERIPHERAL) && CONFIG_BT_PERIPHERAL)
void bt_le_adv_resume(void)
{
	struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();
	struct bt_conn *conn;
	int err;

	if (!adv) {
		BT_DBG("No valid legacy adv");
		return;
	}

	if (!(atomic_test_bit(adv->flags, BT_ADV_PERSIST) &&
	      !atomic_test_bit(adv->flags, BT_ADV_ENABLED))) {
		return;
	}

	if (!atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		return;
	}

	err = le_adv_start_add_conn(adv, &conn);
	if (err) {
		BT_DBG("Cannot resume connectable advertising (%d)", err);
		return;
	}

	BT_DBG("Resuming connectable advertising");

	if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
	    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
		le_adv_set_private_addr(adv);
	}

	err = set_le_adv_enable(adv, true);
	if (err) {
		bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
	}

	/* Since we don't give the application a reference to manage in
	 * this case, we need to release this reference here.
	 */
	bt_conn_unref(conn);
}
#endif /* (defined(CONFIG_BT_PERIPHERAL) && CONFIG_BT_PERIPHERAL) */

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
int bt_le_ext_adv_get_info(const struct bt_le_ext_adv *adv,
			   struct bt_le_ext_adv_info *info)
{
	info->id = adv->id;
	info->tx_power = adv->tx_power;

	return 0;
}
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
int bt_le_ext_adv_create(const struct bt_le_adv_param *param,
			 const struct bt_le_ext_adv_cb *cb,
			 struct bt_le_ext_adv **out_adv)
{
	struct bt_le_ext_adv *adv;
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (!valid_adv_ext_param(param)) {
		return -EINVAL;
	}

	adv = adv_new();
	if (!adv) {
		return -ENOMEM;
	}

	adv->id = param->id;
	adv->cb = cb;

	err = le_ext_adv_param_set(adv, param, false);
	if (err) {
		adv_delete(adv);
		return err;
	}

	*out_adv = adv;
	return 0;
}

int bt_le_ext_adv_update_param(struct bt_le_ext_adv *adv,
			       const struct bt_le_adv_param *param)
{
	if (!valid_adv_ext_param(param)) {
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_BT_PER_ADV) &&
	    atomic_test_bit(adv->flags, BT_PER_ADV_PARAMS_SET)) {
		/* If params for per adv has been set, do not allow setting
		 * connectable, scanable or use legacy adv
		 */
		if (param->options & BT_LE_ADV_OPT_CONNECTABLE ||
		    param->options & BT_LE_ADV_OPT_SCANNABLE ||
		    !(param->options & BT_LE_ADV_OPT_EXT_ADV) ||
		    param->options & BT_LE_ADV_OPT_ANONYMOUS) {
			return -EINVAL;
		}
	}

	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EINVAL;
	}

	if (param->id != adv->id) {
		atomic_clear_bit(adv->flags, BT_ADV_RPA_VALID);
	}

	return le_ext_adv_param_set(adv, param, false);
}

int bt_le_ext_adv_start(struct bt_le_ext_adv *adv,
			struct bt_le_ext_adv_start_param *param)
{
	struct bt_conn *conn = NULL;
	int err;

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		err = le_adv_start_add_conn(adv, &conn);
		if (err) {
			return err;
		}
	}

	atomic_set_bit_to(adv->flags, BT_ADV_LIMITED, param &&
			  (param->timeout > 0 || param->num_events > 0));

	if (atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
		    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
			le_adv_set_private_addr(adv);
		}
	} else {
		if (!atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
			le_adv_set_private_addr(adv);
		}
	}

	if (atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME) &&
	    !atomic_test_bit(adv->flags, BT_ADV_DATA_SET)) {
		/* Set the advertiser name */
		bt_le_ext_adv_set_data(adv, NULL, 0, NULL, 0);
	}

	err = set_le_adv_enable_ext(adv, true, param);
	if (err) {
		BT_ERR("Failed to start advertiser");
		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
			bt_conn_unref(conn);
		}

		return err;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
		/* If undirected connectable advertiser we have created a
		 * connection object that we don't yet give to the application.
		 * Since we don't give the application a reference to manage in
		 * this case, we need to release this reference here
		 */
		bt_conn_unref(conn);
	}

	return 0;
}

int bt_le_ext_adv_stop(struct bt_le_ext_adv *adv)
{
	atomic_clear_bit(adv->flags, BT_ADV_PERSIST);

	if (!atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return 0;
	}

	if (atomic_test_and_clear_bit(adv->flags, BT_ADV_LIMITED)) {
		atomic_clear_bit(adv->flags, BT_ADV_RPA_VALID);

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		pending_id_keys_update();
#endif
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		le_adv_stop_free_conn(adv, 0);
	}

	return set_le_adv_enable_ext(adv, false, NULL);
}

int bt_le_ext_adv_set_data(struct bt_le_ext_adv *adv,
			   const struct bt_data *ad, size_t ad_len,
			   const struct bt_data *sd, size_t sd_len)
{
	bool scannable, use_name;

	scannable = atomic_test_bit(adv->flags, BT_ADV_SCANNABLE);
	use_name = atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME);

	return le_adv_update(adv, ad, ad_len, sd, sd_len, scannable,
			     use_name);
}

int bt_le_ext_adv_delete(struct bt_le_ext_adv *adv)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_remove_adv_set *cp;
	struct net_buf *buf;
#endif

	if (!BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return -ENOTSUP;
	}

	/* Advertising set should be stopped first */
	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EINVAL;
	}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	buf = bt_hci_cmd_create(BT_HCI_OP_LE_REMOVE_ADV_SET, sizeof(*cp));
	if (!buf) {
		BT_WARN("No HCI buffers");
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = adv->handle;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_REMOVE_ADV_SET, buf, NULL);
#else
	err = hci_api_le_remove_adv_set(adv->handle);
#endif
	if (err) {
		return err;
	}

	adv_delete(adv);

	return 0;
}
#endif
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
static bool valid_le_scan_param(const struct bt_le_scan_param *param)
{
	if (param->type != BT_HCI_LE_SCAN_PASSIVE &&
	    param->type != BT_HCI_LE_SCAN_ACTIVE) {
		return false;
	}

	if (param->options & ~(BT_LE_SCAN_OPT_FILTER_DUPLICATE |
			       BT_LE_SCAN_OPT_FILTER_WHITELIST |
			       BT_LE_SCAN_OPT_CODED |
			       BT_LE_SCAN_OPT_NO_1M)) {
		return false;
	}

	if (param->interval < 0x0004 || param->interval > 0x4000) {
		return false;
	}

	if (param->window < 0x0004 || param->window > 0x4000) {
		return false;
	}

	if (param->window > param->interval) {
		return false;
	}

	return true;
}
#if !(defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
int bt_le_scan_start(const struct bt_le_scan_param *param, bt_le_scan_cb_t cb)
{
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	/* Check that the parameters have valid values */
	if (!valid_le_scan_param(param)) {
		return -EINVAL;
	}

	if (param->type && !bt_le_scan_random_addr_check()) {
		return -EINVAL;
	}

	/* Return if active scan is already enabled */
	if (atomic_test_and_set_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN)) {
		return -EALREADY;
	}

	if (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
		err = set_le_scan_enable(BT_HCI_LE_SCAN_DISABLE);
		if (err) {
			atomic_clear_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN);
			return err;
		}
	}

	atomic_set_bit_to(bt_dev.flags, BT_DEV_SCAN_FILTER_DUP,
			  param->options & BT_LE_SCAN_OPT_FILTER_DUPLICATE);

#if (defined(CONFIG_BT_WHITELIST) && CONFIG_BT_WHITELIST)
	atomic_set_bit_to(bt_dev.flags, BT_DEV_SCAN_WL,
			  param->options & BT_LE_SCAN_OPT_FILTER_WHITELIST);
#endif /* (defined(CONFIG_BT_WHITELIST) && CONFIG_BT_WHITELIST) */

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		struct bt_hci_ext_scan_phy param_1m;
		struct bt_hci_ext_scan_phy param_coded;

		struct bt_hci_ext_scan_phy *phy_1m = NULL;
		struct bt_hci_ext_scan_phy *phy_coded = NULL;

		if (!(param->options & BT_LE_SCAN_OPT_NO_1M)) {
			param_1m.type = param->type;
			param_1m.interval = sys_cpu_to_le16(param->interval);
			param_1m.window = sys_cpu_to_le16(param->window);

			phy_1m = &param_1m;
		}

		if (param->options & BT_LE_SCAN_OPT_CODED) {
			u16_t interval = param->interval_coded ?
				param->interval_coded :
				param->interval;
			u16_t window = param->window_coded ?
				param->window_coded :
				param->window;

			param_coded.type = param->type;
			param_coded.interval = sys_cpu_to_le16(interval);
			param_coded.window = sys_cpu_to_le16(window);
			phy_coded = &param_coded;
		}

		err = start_le_scan_ext(phy_1m, phy_coded, param->timeout);
	} else {
		if (param->timeout) {
			return -ENOTSUP;
		}

		err = start_le_scan_legacy(param->type, param->interval,
					   param->window);
	}

	if (err) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN);
		return err;
	}

	scan_dev_found_cb = cb;

	return 0;
}

int bt_le_scan_stop(void)
{
	/* Return if active scanning is already disabled */
	if (!atomic_test_and_clear_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN)) {
		return -EALREADY;
	}

	scan_dev_found_cb = NULL;

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    atomic_test_and_clear_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED)) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		pending_id_keys_update();
#endif
	}

	return bt_le_scan_update(false);
}
#endif
void bt_le_scan_cb_register(struct bt_le_scan_cb *cb)
{
	sys_slist_find_and_remove(&scan_cbs, &cb->node);
	sys_slist_append(&scan_cbs, &cb->node);
}

void bt_le_scan_cb_unregister(struct bt_le_scan_cb *cb)
{
	sys_slist_find_and_remove(&scan_cbs, &cb->node);
}
#endif /* CONFIG_BT_OBSERVER */

#if (defined(CONFIG_BT_WHITELIST) && CONFIG_BT_WHITELIST)

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
static int le_whitelist_cb(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	struct {
		struct k_sem *sync;
		struct net_buf *rsp;
	} *arg;

	arg = args;

	if (arg && arg->sync)
	{
		arg->rsp = buf;
		k_sem_give(arg->sync);
	}
	else
	{
		net_buf_unref(buf);
	}

	return 0;
}
#endif

int bt_le_whitelist_add(const bt_addr_le_t *addr)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_add_dev_to_wl *cp;
	struct net_buf *buf;
#endif

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	buf = bt_hci_cmd_create(BT_HCI_OP_LE_ADD_DEV_TO_WL, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_le_copy(&cp->addr, addr);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	struct net_buf *rsp;

	struct {
		struct k_sem *sync;
		struct net_buf *rsp;
	} args = {0};

	struct k_sem sync_sem;

	k_sem_init(&sync_sem, 0, 1);

	bt_hci_cmd_cb_t cb =
	{
		.func = le_whitelist_cb,
	};

	args.sync = &sync_sem;

	cb.args = &args;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_ADD_DEV_TO_WL, buf, &cb);
	if (err)
	{
		return err;
	}

	err = k_sem_take(&sync_sem, HCI_CMD_TIMEOUT);
	if (err)
	{
		return err;
	}

	k_sem_delete(&sync_sem);

	rsp = args.rsp;

	if (rsp->data[0])
	{
		err = -EIO;
	}

	net_buf_unref(rsp);

#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_ADD_DEV_TO_WL, buf, NULL);
#endif
#else
	err = hci_api_white_list_add(addr->type, (u8_t *)addr->a.val);
#endif
	if (err) {
		BT_ERR("Failed to add device to whitelist");

		return err;
	}

	return 0;
}

int bt_le_whitelist_rem(const bt_addr_le_t *addr)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_rem_dev_from_wl *cp;
	struct net_buf *buf;
#endif

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	buf = bt_hci_cmd_create(BT_HCI_OP_LE_REM_DEV_FROM_WL, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	bt_addr_le_copy(&cp->addr, addr);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)

	struct net_buf *rsp;

	struct {
		struct k_sem *sync;
		struct net_buf *rsp;
	} args = {0};

	struct k_sem sync_sem;

	k_sem_init(&sync_sem, 0, 1);

	bt_hci_cmd_cb_t cb =
	{
		.func = le_whitelist_cb,
	};

	args.sync = &sync_sem;

	cb.args = &args;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_REM_DEV_FROM_WL, buf, &cb);
	if (err)
	{
		return err;
	}

	err = k_sem_take(&sync_sem, HCI_CMD_TIMEOUT);
	if (err)
	{
		return err;
	}

	k_sem_delete(&sync_sem);

	rsp = args.rsp;

	if (rsp->data[0])
	{
		err = -EIO;
	}

	net_buf_unref(rsp);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_REM_DEV_FROM_WL, buf, NULL);
#endif
#else
	err = hci_api_white_list_remove(addr->type, (u8_t *)addr->a.val);
#endif
	if (err) {
		BT_ERR("Failed to remove device from whitelist");
		return err;
	}

	return 0;
}

int bt_le_whitelist_clear(void)
{
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_CLEAR_WL, NULL, NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_CLEAR_WL, NULL, NULL);
#endif
#else
	err = hci_api_white_list_clear();
#endif
	if (err) {
		BT_ERR("Failed to clear whitelist");
		return err;
	}

	return 0;
}

int bt_le_whitelist_size(u8_t *size)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	int err;
	struct net_buf *rsp;

	struct {
		struct k_sem *sync;
		struct net_buf *rsp;
	} args = {0};

	struct k_sem sync_sem;

	k_sem_init(&sync_sem, 0, 1);

	bt_hci_cmd_cb_t cb =
	{
		.func = le_whitelist_cb,
	};

	args.sync = &sync_sem;

	cb.args = &args;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_WL_SIZE, NULL, &cb);
	if (err) {
		return err;
	}

	err = k_sem_take(&sync_sem, HCI_CMD_TIMEOUT);
	if (err)
	{
		return err;
	}

	k_sem_delete(&sync_sem);

	rsp = args.rsp;

	struct bt_hci_rp_le_read_wl_size *rp = (void *)rsp->data;

	if (rp->status) {
		net_buf_unref(rsp);
		return rp->status;
	}

	*size = rp->wl_size;

	net_buf_unref(rsp);
    return 0;
#else
	int err;
	struct net_buf *rsp;
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_READ_WL_SIZE, NULL, &rsp);

	if (err) {
		return err;
	}

	struct bt_hci_rp_le_read_wl_size *rp = (void *)rsp->data;

	if (rp->status) {
		net_buf_unref(rsp);
		return rp->status;
	}

	*size = rp->wl_size;

	net_buf_unref(rsp);
    return 0;
#endif
#else
	return hci_api_white_list_size(size);
#endif
}

#endif /* (defined(CONFIG_BT_WHITELIST) && CONFIG_BT_WHITELIST) */

int bt_le_set_chan_map(u8_t chan_map[5])
{
	if (!IS_ENABLED(CONFIG_BT_CENTRAL)) {
		return -ENOTSUP;
	}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_set_host_chan_classif *cp;
	struct net_buf *buf;

	if (!BT_CMD_TEST(bt_dev.supported_commands, 27, 3)) {
		BT_WARN("Set Host Channel Classification command is "
			"not supported");
		return -ENOTSUP;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_HOST_CHAN_CLASSIF,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	memcpy(&cp->ch_map[0], &chan_map[0], 4);
	cp->ch_map[4] = chan_map[4] & BIT_MASK(5);
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_HOST_CHAN_CLASSIF,
				    buf, NULL);
#else
	return bt_hci_cmd_send_sync(BT_HCI_OP_LE_SET_HOST_CHAN_CLASSIF,
				    buf, NULL);
#endif
#else
	u8_t ch_map[5] = {0};
	memcpy(&ch_map[0], &chan_map[0], 4);
	ch_map[4] = chan_map[4] & BIT_MASK(5);
	return hci_api_le_set_host_chan_classif(ch_map);
#endif
}

struct net_buf *bt_buf_get_rx(enum bt_buf_type type, k_timeout_t timeout)
{
	struct net_buf *buf;

	__ASSERT(type == BT_BUF_EVT || type == BT_BUF_ACL_IN,
		 "Invalid buffer type requested");

#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL)
	if (type == BT_BUF_EVT) {
		buf = net_buf_alloc(&hci_rx_pool, timeout);
	} else {
		buf = net_buf_alloc(&acl_in_pool, timeout);
	}
#else
	buf = net_buf_alloc(&hci_rx_pool, timeout);
#endif

	if (buf) {
		net_buf_reserve(buf, BT_BUF_RESERVE);
		bt_buf_set_type(buf, type);
	}

	return buf;
}

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
struct net_buf *bt_buf_get_cmd_complete(k_timeout_t timeout)
{
	struct net_buf *buf;
	unsigned int key;

	key = irq_lock();
	buf = bt_dev.sent_cmd;
	bt_dev.sent_cmd = NULL;
	irq_unlock(key);

	BT_DBG("sent_cmd %p", buf);

	if (buf) {
		bt_buf_set_type(buf, BT_BUF_EVT);
		buf->len = 0U;
		net_buf_reserve(buf, BT_BUF_RESERVE);

		return buf;
	}

	return bt_buf_get_rx(BT_BUF_EVT, timeout);
}
#endif

struct net_buf *bt_buf_get_evt(u8_t evt, bool discardable, k_timeout_t timeout)
{
	switch (evt) {
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	case BT_HCI_EVT_NUM_COMPLETED_PACKETS:
		{
			struct net_buf *buf;

			buf = net_buf_alloc(&num_complete_pool, timeout);
			if (buf) {
				net_buf_reserve(buf, BT_BUF_RESERVE);
				bt_buf_set_type(buf, BT_BUF_EVT);
			}

			return buf;
		}
#endif /* CONFIG_BT_CONN */
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	case BT_HCI_EVT_CMD_COMPLETE:
	case BT_HCI_EVT_CMD_STATUS:
		return bt_buf_get_cmd_complete(timeout);
#endif
	default:
#if (CONFIG_BT_DISCARDABLE_BUF_COUNT > 0)
		if (discardable) {
			struct net_buf *buf;

			buf = net_buf_alloc(&discardable_pool, timeout);
			if (buf) {
				net_buf_reserve(buf, BT_BUF_RESERVE);
				bt_buf_set_type(buf, BT_BUF_EVT);
			}

			return buf;
		}
#endif /* CONFIG_BT_DISCARDABLE_BUF_COUNT */

		return bt_buf_get_rx(BT_BUF_EVT, timeout);
	}
}

#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
static int br_start_inquiry(const struct bt_br_discovery_param *param)
{
	const u8_t iac[3] = { 0x33, 0x8b, 0x9e };
	struct bt_hci_op_inquiry *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_INQUIRY, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	cp->length = param->length;
	cp->num_rsp = 0xff; /* we limit discovery only by time */

	memcpy(cp->lap, iac, 3);
	if (param->limited) {
		cp->lap[0] = 0x00;
	}

	return bt_hci_cmd_send_sync(BT_HCI_OP_INQUIRY, buf, NULL);
}

static bool valid_br_discov_param(const struct bt_br_discovery_param *param,
				  size_t num_results)
{
	if (!num_results || num_results > 255) {
		return false;
	}

	if (!param->length || param->length > 0x30) {
		return false;
	}

	return true;
}

int bt_br_discovery_start(const struct bt_br_discovery_param *param,
			  struct bt_br_discovery_result *results, size_t cnt,
			  bt_br_discovery_cb_t cb)
{
	int err;

	BT_DBG("");

	if (!valid_br_discov_param(param, cnt)) {
		return -EINVAL;
	}

	if (atomic_test_bit(bt_dev.flags, BT_DEV_INQUIRY)) {
		return -EALREADY;
	}

	err = br_start_inquiry(param);
	if (err) {
		return err;
	}

	atomic_set_bit(bt_dev.flags, BT_DEV_INQUIRY);

	(void)memset(results, 0, sizeof(*results) * cnt);

	discovery_cb = cb;
	discovery_results = results;
	discovery_results_size = cnt;
	discovery_results_count = 0;

	return 0;
}

int bt_br_discovery_stop(void)
{
	int err;
	int i;

	BT_DBG("");

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_INQUIRY)) {
		return -EALREADY;
	}

	err = bt_hci_cmd_send_sync(BT_HCI_OP_INQUIRY_CANCEL, NULL, NULL);
	if (err) {
		return err;
	}

	for (i = 0; i < discovery_results_count; i++) {
		struct discovery_priv *priv;
		struct bt_hci_cp_remote_name_cancel *cp;
		struct net_buf *buf;

		priv = (struct discovery_priv *)&discovery_results[i]._priv;

		if (!priv->resolving) {
			continue;
		}

		buf = bt_hci_cmd_create(BT_HCI_OP_REMOTE_NAME_CANCEL,
					sizeof(*cp));
		if (!buf) {
			continue;
		}

		cp = net_buf_add(buf, sizeof(*cp));
		bt_addr_copy(&cp->bdaddr, &discovery_results[i].addr);

		bt_hci_cmd_send_sync(BT_HCI_OP_REMOTE_NAME_CANCEL, buf, NULL);
	}

	atomic_clear_bit(bt_dev.flags, BT_DEV_INQUIRY);

	discovery_cb = NULL;
	discovery_results = NULL;
	discovery_results_size = 0;
	discovery_results_count = 0;

	return 0;
}

static int write_scan_enable(u8_t scan)
{
	struct net_buf *buf;
	int err;

	BT_DBG("type %u", scan);

	buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_SCAN_ENABLE, 1);
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_u8(buf, scan);
	err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_SCAN_ENABLE, buf, NULL);
	if (err) {
		return err;
	}

	atomic_set_bit_to(bt_dev.flags, BT_DEV_ISCAN,
			  (scan & BT_BREDR_SCAN_INQUIRY));
	atomic_set_bit_to(bt_dev.flags, BT_DEV_PSCAN,
			  (scan & BT_BREDR_SCAN_PAGE));

	return 0;
}

int bt_br_set_connectable(bool enable)
{
	if (enable) {
		if (atomic_test_bit(bt_dev.flags, BT_DEV_PSCAN)) {
			return -EALREADY;
		} else {
			return write_scan_enable(BT_BREDR_SCAN_PAGE);
		}
	} else {
		if (!atomic_test_bit(bt_dev.flags, BT_DEV_PSCAN)) {
			return -EALREADY;
		} else {
			return write_scan_enable(BT_BREDR_SCAN_DISABLED);
		}
	}
}

int bt_br_set_discoverable(bool enable)
{
	if (enable) {
		if (atomic_test_bit(bt_dev.flags, BT_DEV_ISCAN)) {
			return -EALREADY;
		}

		if (!atomic_test_bit(bt_dev.flags, BT_DEV_PSCAN)) {
			return -EPERM;
		}

		return write_scan_enable(BT_BREDR_SCAN_INQUIRY |
					 BT_BREDR_SCAN_PAGE);
	} else {
		if (!atomic_test_bit(bt_dev.flags, BT_DEV_ISCAN)) {
			return -EALREADY;
		}

		return write_scan_enable(BT_BREDR_SCAN_PAGE);
	}
}
#endif /* CONFIG_BT_BREDR */

#if (defined(CONFIG_BT_ECC) && CONFIG_BT_ECC)
int bt_pub_key_gen(struct bt_pub_key_cb *new_cb)
{
	int err;

	/*
	 * We check for both "LE Read Local P-256 Public Key" and
	 * "LE Generate DH Key" support here since both commands are needed for
	 * ECC support. If "LE Generate DH Key" is not supported then there
	 * is no point in reading local public key.
	 */
	if (!BT_CMD_TEST(bt_dev.supported_commands, 34, 1) ||
	    !BT_CMD_TEST(bt_dev.supported_commands, 34, 2)) {
		BT_WARN("ECC HCI commands not available");
		return -ENOTSUP;
	}

	new_cb->_next = pub_key_cb;
	pub_key_cb = new_cb;

	if (atomic_test_and_set_bit(bt_dev.flags, BT_DEV_PUB_KEY_BUSY)) {
		return 0;
	}

	atomic_clear_bit(bt_dev.flags, BT_DEV_HAS_PUB_KEY);

#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_P256_PUBLIC_KEY, NULL, NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_P256_PUBLIC_KEY, NULL, NULL);
#endif
#else
	err = hci_api_le_gen_p256_pubkey();
#endif
	if (err) {
		BT_ERR("Sending LE P256 Public Key command failed");
		atomic_clear_bit(bt_dev.flags, BT_DEV_PUB_KEY_BUSY);
		pub_key_cb = NULL;
		return err;
	}

	return 0;
}

int bt_pub_key_regen()
{
	return bt_pub_key_gen(pub_key_cb);
}

const u8_t *bt_pub_key_get(void)
{
	if (atomic_test_bit(bt_dev.flags, BT_DEV_HAS_PUB_KEY)) {
		return pub_key;
	}

	return NULL;
}

void bt_pub_key_clear(void)
{
	atomic_clear_bit(bt_dev.flags, BT_DEV_HAS_PUB_KEY);
	memset(pub_key, 0x00, sizeof(pub_key));
}

int bt_dh_key_gen(const u8_t remote_pk[64], bt_dh_key_cb_t cb)
{
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_generate_dhkey *cp;
	struct net_buf *buf;
	int err;

	if (dh_key_cb || atomic_test_bit(bt_dev.flags, BT_DEV_PUB_KEY_BUSY)) {
		return -EBUSY;
	}

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_HAS_PUB_KEY)) {
		return -EADDRNOTAVAIL;
	}

	dh_key_cb = cb;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_GENERATE_DHKEY, sizeof(*cp));
	if (!buf) {
		dh_key_cb = NULL;
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	memcpy(cp->key, remote_pk, sizeof(cp->key));
#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_GENERATE_DHKEY, buf, NULL);
#else
	err = bt_hci_cmd_send_sync(BT_HCI_OP_LE_GENERATE_DHKEY, buf, NULL);
#endif
	if (err) {
		dh_key_cb = NULL;
		return err;
	}

	return 0;
#else
	int err = 0;

	if (dh_key_cb || atomic_test_bit(bt_dev.flags, BT_DEV_PUB_KEY_BUSY)) {
		return -EBUSY;
	}

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_HAS_PUB_KEY)) {
		return -EADDRNOTAVAIL;
	}

	dh_key_cb = cb;

	err =  hci_api_le_gen_dhkey((uint8_t *)remote_pk);
	if (err)
	{
		dh_key_cb = NULL;
	}

	return err;
#endif
}
#endif /* CONFIG_BT_ECC */

#if (defined(CONFIG_BT_BREDR) && CONFIG_BT_BREDR)
int bt_br_oob_get_local(struct bt_br_oob *oob)
{
	bt_addr_copy(&oob->addr, &bt_dev.id_addr[0].a);

	return 0;
}
#endif /* CONFIG_BT_BREDR */

int bt_le_oob_get_local(u8_t id, struct bt_le_oob *oob)
{
	struct bt_le_ext_adv *adv;
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (id >= CONFIG_BT_ID_MAX) {
		return -EINVAL;
	}

	adv = bt_adv_lookup_legacy();

	if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
	    !(adv && adv->id == id &&
	      atomic_test_bit(adv->flags, BT_ADV_ENABLED) &&
	      atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY) &&
	      bt_dev.id_addr[id].type == BT_ADDR_LE_RANDOM)) {
		if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
		    atomic_test_bit(bt_dev.flags, BT_DEV_INITIATING)) {
			struct bt_conn *conn;

			conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL,
						       BT_CONN_CONNECT_SCAN);
			if (conn) {
				/* Cannot set new RPA while creating
				 * connections.
				 */
				bt_conn_unref(conn);
				return -EINVAL;
			}
		}

		if (adv &&
		    atomic_test_bit(adv->flags, BT_ADV_ENABLED) &&
		    atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY) &&
		    (bt_dev.id_addr[id].type == BT_ADDR_LE_RANDOM)) {
			/* Cannot set a new RPA address while advertising with
			 * random static identity address for a different
			 * identity.
			 */
			return -EINVAL;
		}

		if (IS_ENABLED(CONFIG_BT_OBSERVER) &&
		    id != BT_ID_DEFAULT &&
		    (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING) ||
		     atomic_test_bit(bt_dev.flags, BT_DEV_INITIATING))) {
			/* Cannot switch identity of scanner or initiator */
			return -EINVAL;
		}

		le_rpa_invalidate();
		le_update_private_addr();

		bt_addr_le_copy(&oob->addr, &bt_dev.random_addr);
	} else {
		bt_addr_le_copy(&oob->addr, &bt_dev.id_addr[id]);
	}

	if (IS_ENABLED(CONFIG_BT_SMP)) {
		err = bt_smp_le_oob_generate_sc_data(&oob->le_sc_data);
		if (err) {
			return err;
		}
	}

	return 0;
}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
int bt_le_ext_adv_oob_get_local(struct bt_le_ext_adv *adv,
				struct bt_le_oob *oob)
{
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
	    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
		/* Don't refresh RPA addresses if the RPA is new.
		 * This allows back to back calls to this function or
		 * bt_le_oob_get_local to not invalidate the previously set
		 * RPAs.
		 */
		if (!atomic_test_bit(adv->flags, BT_ADV_LIMITED) &&
		    !rpa_is_new()) {
			if (IS_ENABLED(CONFIG_BT_CENTRAL) &&
			    atomic_test_bit(bt_dev.flags, BT_DEV_INITIATING)) {
				struct bt_conn *conn;

				conn = bt_conn_lookup_state_le(
					BT_ID_DEFAULT, NULL,
					BT_CONN_CONNECT_SCAN);

				if (conn) {
					/* Cannot set new RPA while creating
					 * connections.
					 */
					bt_conn_unref(conn);
					return -EINVAL;
				}
			}

			le_rpa_invalidate();
			le_update_private_addr();
		}

		bt_addr_le_copy(&oob->addr, &adv->random_addr);
	} else {
		bt_addr_le_copy(&oob->addr, &bt_dev.id_addr[adv->id]);
	}

	if (IS_ENABLED(CONFIG_BT_SMP) &&
	    !IS_ENABLED(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) {
		err = bt_smp_le_oob_generate_sc_data(&oob->le_sc_data);
		if (err) {
			return err;
		}
	}

	return 0;
}
#endif /* (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV) */

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
#if !(defined(CONFIG_BT_SMP_SC_PAIR_ONLY) && CONFIG_BT_SMP_SC_PAIR_ONLY)
int bt_le_oob_set_legacy_tk(struct bt_conn *conn, const u8_t *tk)
{
	return bt_smp_le_oob_set_tk(conn, tk);
}
#endif /* !(defined(CONFIG_BT_SMP_SC_PAIR_ONLY) && CONFIG_BT_SMP_SC_PAIR_ONLY) */

#if !(defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)
int bt_le_oob_set_sc_data(struct bt_conn *conn,
			  const struct bt_le_oob_sc_data *oobd_local,
			  const struct bt_le_oob_sc_data *oobd_remote)
{
	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	return bt_smp_le_oob_set_sc_data(conn, oobd_local, oobd_remote);
}

int bt_le_oob_get_sc_data(struct bt_conn *conn,
			  const struct bt_le_oob_sc_data **oobd_local,
			  const struct bt_le_oob_sc_data **oobd_remote)
{
	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	return bt_smp_le_oob_get_sc_data(conn, oobd_local, oobd_remote);
}
#endif /* !(defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) */
#endif /* (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP) */

int bt_le_hci_version_get()
{
    return bt_dev.hci_version;
}

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)

static void hci_cmd_sent_work(struct k_work *work)
{
	struct net_buf *buf;

	u8_t status = BT_HCI_ERR_UNSPECIFIED;

	buf = bt_dev.sent_cmd;

	BT_ERR("HCI Cmd sent timeout");

	if (buf)
	{
		BT_ERR("buf %p, Opcode 0x%04x Sent timeout", buf, cmd(buf)->opcode);

		hci_cmd_done(cmd(buf)->opcode, status, buf);

		bt_dev.sent_cmd = NULL;
		net_buf_unref(buf);
		atomic_inc(&bt_dev.ncmd);

#if (defined(CONFIG_BT_HOST_OPTIMIZE) && CONFIG_BT_HOST_OPTIMIZE)
		/* default handler first */
		int err = handle_hci_cmd_done_event(buf, cmd(buf)->opcode, status);
		if (err)
		{
			net_buf_unref(buf);
			BT_ERR("OpCode 0x%04x complete process err %d", cmd(buf)->opcode, err);
		}

		/* notify FSM with  BT_FSM_EV_DEFINE(HCI_CMD, CMD_SENT) event */
		if (cmd(buf)->fsm_handle != BT_FSM_HANDLE_UNUSED)
		{
			net_buf_ref(buf);
			err = bt_fsm_hci_cmd_send(buf, NULL, BT_FSM_HANDLE_UNUSED, BT_FSM_EV_DEFINE(HCI_CMD, CMD_CMPLETE));
			if (err)
			{
				net_buf_unref(buf);
				BT_ERR("OpCode 0x%04x complete process err %d", cmd(buf)->opcode, err);
			}
		}

		/* call hci command sent callback function */
		if (cmd(buf)->cb.func) {
			net_buf_ref(buf);
			err = cmd(buf)->cb.func(cmd(buf)->opcode, cmd(buf)->status, buf, cmd(buf)->cb.args);
			if (err)
			{
				net_buf_unref(buf);
				BT_ERR("OpCode 0x%04x complete process err %d", cmd(buf)->opcode, err);
			}
		}
		net_buf_unref(buf);
		net_buf_unref(buf);

		buf = net_buf_get(&bt_dev.cmd_tx_pending_queue, 0);
		if (buf)
		{
			net_buf_put(&bt_dev.cmd_tx_queue, buf);
				BT_DBG("reput %p to cmd_tx_queue");
		}
#endif
	}
}

int bt_hci_cmd_send_cb(u16_t opcode, struct net_buf *buf, bt_hci_cmd_cb_t *cb)
{
	if (!buf) {
		buf = bt_hci_cmd_create(opcode, 0);
		if (!buf) {
			BT_ERR("opcode 0x%04x create fail", opcode);
			return -ENOBUFS;
		}
	}

	BT_DBG("buf %p opcode 0x%04x len %u", buf, opcode, buf->len);

	/* Host Number of Completed Packets can ignore the ncmd value
	 * and does not generate any cmd complete/status events.
	 */
	if (opcode == BT_HCI_OP_HOST_NUM_COMPLETED_PACKETS) {
		int err;

		err = bt_send(buf);
		if (err) {
			BT_ERR("Unable to send to driver (err %d)", err);
			net_buf_unref(buf);
		}

		return err;
	}

	if (cb)
	{
		memcpy(&(cmd(buf)->cb), cb, sizeof(bt_hci_cmd_cb_t));
	}

	/* Make sure the buffer stays around until the command completes */
	net_buf_ref(buf);

	send_cmd_buf(buf);

	return 0;
}

static int hci_cmd_le_set_adv_enable_legacy_create(struct bt_le_ext_adv *adv, bool enable, struct net_buf **ret_buf)
{
	struct net_buf *buf = NULL;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_ADV_ENABLE, 1);
	if (!buf) {
		return -ENOBUFS;
	}

	if (enable) {
		net_buf_add_u8(buf, BT_HCI_LE_ADV_ENABLE);
	} else {
		net_buf_add_u8(buf, BT_HCI_LE_ADV_DISABLE);
	}

	cmd_state_set_init(&cmd(buf)->state, adv->flags, BT_ADV_ENABLED, enable);

	*ret_buf = buf;

	return 0;
}

static int set_random_address_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	bt_hci_cmd_cb_t *cb = args;

	bt_addr_copy(&bt_dev.random_addr.a, &bt_dev.pending_random_addr.a);
	bt_dev.random_addr.type = BT_ADDR_LE_RANDOM;

	memset(&bt_dev.pending_random_addr, 0, sizeof(bt_dev.pending_random_addr));

	if (cb)
	{
		return cb->func(opcode, status, buf, cb->args);
	}

	return 0;
}
static int hci_cmd_le_set_random_address_create(const bt_addr_t *addr, struct net_buf **ret_buf)
{
	struct net_buf *buf;

	/* Do nothing if we already have the right address */
	if (!bt_addr_cmp(addr, &bt_dev.random_addr.a)) {
		return 0;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_RANDOM_ADDRESS, sizeof(*addr));
	if (!buf) {
		return -ENOBUFS;
	}

	bt_addr_copy(&bt_dev.pending_random_addr.a, addr);
	bt_dev.pending_random_addr.type = BT_ADDR_LE_RANDOM;

	net_buf_add_mem(buf, addr, sizeof(*addr));

	*ret_buf = buf;

	return 0;
}

static int set_random_address_cb(const bt_addr_t *addr, bt_hci_cmd_cb_t *cb)
{
	int err;

	BT_DBG("%s", bt_addr_str(addr));

	/* Do nothing if we already have the right address */
	if (!bt_addr_cmp(addr, &bt_dev.random_addr.a)) {
		return 0;
	}

	static bt_hci_cmd_cb_t saved_cb;
	bt_hci_cmd_cb_t set_cb = 
	{
		.func = set_random_address_complete,
	};

	if (cb)
	{
		memcpy(&saved_cb, cb, sizeof(bt_hci_cmd_cb_t));
		set_cb.args = &saved_cb;
	}

	struct net_buf *buf = NULL;

	err = hci_cmd_le_set_random_address_create(addr, &buf);
	if (err) {
		return err;
	}

	if (!buf)
	{
		return 0;
	}

	bt_addr_copy(&bt_dev.pending_random_addr.a, addr);
	bt_dev.pending_random_addr.type = BT_ADDR_LE_RANDOM;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_RANDOM_ADDRESS, buf, &set_cb);
	if (err) {
		return err;
	}

	return 0;
}

static int hci_cmd_le_set_adv_enable_ext_create(struct bt_le_ext_adv *adv,
				 bool enable,
				 const struct bt_le_ext_adv_start_param *param, struct net_buf **ret_buf)
{
	struct net_buf *buf;
	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EXT_ADV_ENABLE, 6);
	if (!buf) {
		return -ENOBUFS;
	}

	if (enable) {
		net_buf_add_u8(buf, BT_HCI_LE_ADV_ENABLE);
	} else {
		net_buf_add_u8(buf, BT_HCI_LE_ADV_DISABLE);
	}

	net_buf_add_u8(buf, 1);

	net_buf_add_u8(buf, adv->handle);
	net_buf_add_le16(buf, param ? sys_cpu_to_le16(param->timeout) : 0);
	net_buf_add_u8(buf, param ? param->num_events : 0);

	cmd_state_set_init(&cmd(buf)->state, adv->flags, BT_ADV_ENABLED, enable);

	*ret_buf = buf;

	return 0;
}

int hci_cmd_le_set_adv_enable_create(struct bt_le_ext_adv *adv, bool enable, struct net_buf **ret_buf)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return hci_cmd_le_set_adv_enable_ext_create(adv, enable, NULL, ret_buf);
	}

	return hci_cmd_le_set_adv_enable_legacy_create(adv, enable, ret_buf);
}

static int hci_cmd_adv_random_address_create(struct bt_le_ext_adv *adv,
				  const bt_addr_t *addr, struct net_buf **ret_buf)
{
	struct net_buf *buf;

	BT_DBG("%s", bt_addr_str(addr));

	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
		return hci_cmd_le_set_random_address_create(addr, ret_buf);
	}

	if (!atomic_test_bit(adv->flags, BT_ADV_PARAMS_SET)) {
		bt_addr_copy(&adv->random_addr.a, addr);
		adv->random_addr.type = BT_ADDR_LE_RANDOM;
		atomic_set_bit(adv->flags, BT_ADV_RANDOM_ADDR_PENDING);
		return 0;
	}

	struct bt_hci_cp_le_set_adv_set_random_addr *cp;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_ADV_SET_RANDOM_ADDR,
				sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	cp->handle = adv->handle;
	bt_addr_copy(&cp->bdaddr, addr);

	bt_addr_copy(&adv->pending_random_addr.a, addr);
	adv->pending_random_addr.type = BT_ADDR_LE_RANDOM;

	*ret_buf = buf;

	return 0;
}

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
static int hci_cmd_le_set_private_addr_create(u8_t id, struct net_buf **ret_buf)
{
	bt_addr_t rpa;
	int err;

	/* check if RPA is valid */
	if (atomic_test_bit(bt_dev.flags, BT_DEV_RPA_VALID)) {
		return 0;
	}

	err = bt_rpa_create(bt_dev.irk[id], &rpa);
	if (!err) {
		err = hci_cmd_le_set_random_address_create(&rpa, ret_buf);
		if (!err) {
			atomic_set_bit(bt_dev.flags, BT_DEV_RPA_VALID);
		}
	}

	le_rpa_timeout_submit();

	return err;
}

static int hci_cmd_le_adv_set_private_addr_create(struct bt_le_ext_adv *adv, struct net_buf **ret_buf)
{
	bt_addr_t rpa;
	int err;

	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
		  err = bt_rpa_create(bt_dev.irk[adv->id], &rpa);
		  if (!err) {
              err = hci_cmd_le_set_random_address_create(&rpa, ret_buf);
              if (!err) {
			      atomic_set_bit(adv->flags, BT_ADV_RPA_VALID);
			  }
          }
		return 0;
	}

	/* check if RPA is valid */
	if (atomic_test_bit(adv->flags, BT_ADV_RPA_VALID)) {
		return 0;
	}

	if (adv == bt_adv_lookup_legacy() && adv->id == BT_ID_DEFAULT) {
		/* Make sure that a Legacy advertiser using default ID has same
		 * RPA address as scanner roles.
		 */
		err = le_set_private_addr(BT_ID_DEFAULT);
		if (err) {
			return err;
		}

		err = hci_cmd_adv_random_address_create(adv, &bt_dev.random_addr.a, ret_buf);
		if (!err) {
			atomic_set_bit(adv->flags, BT_ADV_RPA_VALID);
		}
		return 0;
	}

	err = bt_rpa_create(bt_dev.irk[adv->id], &rpa);
	if (!err) {
		err = hci_cmd_adv_random_address_create(adv, &rpa, ret_buf);
		if (!err) {
			atomic_set_bit(adv->flags, BT_ADV_RPA_VALID);
		}
	}

	if (!atomic_test_bit(adv->flags, BT_ADV_LIMITED)) {
		le_rpa_timeout_submit();
	}

	return err;
}

#else
static int hci_cmd_le_set_private_addr_create(u8_t id, struct net_buf **ret_buf)
{
	bt_addr_t nrpa;
	int err;

	err = bt_rand(nrpa.val, sizeof(nrpa.val));
	if (err) {
		return err;
	}

	nrpa.val[5] &= 0x3f;

	return hci_cmd_le_set_random_address_create(&nrpa, ret_buf);
}

static int hci_cmd_le_adv_set_private_addr_create(struct bt_le_ext_adv *adv, struct net_buf **ret_buf)
{
	bt_addr_t nrpa;
	int err;

	err = bt_rand(nrpa.val, sizeof(nrpa.val));
	if (err) {
		return err;
	}

	nrpa.val[5] &= 0x3f;

	return hci_cmd_adv_random_address_create(adv, &nrpa, ret_buf);
}
#endif
#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
static int hci_cmd_set_le_ext_scan_enable_create(u8_t enable, u16_t duration, struct net_buf **ret_buf)
{
	struct bt_hci_cp_le_set_ext_scan_enable *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EXT_SCAN_ENABLE, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	if (enable == BT_HCI_LE_SCAN_ENABLE) {
		cp->filter_dup = atomic_test_bit(bt_dev.flags,
						 BT_DEV_SCAN_FILTER_DUP);
	} else {
		cp->filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE;
	}

	cp->enable = enable;
	cp->duration = sys_cpu_to_le16(duration);
	cp->period = 0;

	cmd_state_set_init(&cmd(buf)->state, bt_dev.flags, BT_DEV_SCANNING,
				   enable == BT_HCI_LE_SCAN_ENABLE);

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_set_le_scan_enable_legacy_create(u8_t enable, struct net_buf **ret_buf)
{
	struct bt_hci_cp_le_set_scan_enable *cp;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_SCAN_ENABLE, sizeof(*cp));
	if (!buf) {
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));

	if (enable == BT_HCI_LE_SCAN_ENABLE) {
		cp->filter_dup = atomic_test_bit(bt_dev.flags,
						 BT_DEV_SCAN_FILTER_DUP);
	} else {
		cp->filter_dup = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE;
	}

	cp->enable = enable;

	cmd_state_set_init(&cmd(buf)->state, bt_dev.flags, BT_DEV_SCANNING,
				   enable == BT_HCI_LE_SCAN_ENABLE);

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_set_le_scan_enable_create(u8_t enable, struct net_buf **ret_buf)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return hci_cmd_set_le_ext_scan_enable_create(enable, 0, ret_buf);
	}

	return hci_cmd_set_le_scan_enable_legacy_create(enable, ret_buf);
}
#endif

int bt_hci_cmd_le_read_max_data_len_cb(u16_t opcode, u8_t status, struct net_buf *rsp, void *args)
{
	struct bt_hci_rp_le_read_max_data_len *rp;
	u16_t tx_octets, tx_time;

	rp = (void *)rsp->data;
	tx_octets = sys_le16_to_cpu(rp->max_tx_octets);
	tx_time = sys_le16_to_cpu(rp->max_tx_time);
	net_buf_unref(rsp);
	
	struct bt_conn *conn = args;

	return bt_le_set_data_len(conn, tx_octets, tx_time);
}

int hci_cmd_le_read_max_data_len(struct bt_conn *conn)
{
	int err;

	static bt_hci_cmd_cb_t read_max_data_cb = {
		.func = bt_hci_cmd_le_read_max_data_len_cb,
	};

	read_max_data_cb.args = conn;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_MAX_DATA_LEN, NULL, &read_max_data_cb);
	if (err) {
		BT_ERR("Failed to read DLE max data len");
		return err;
	}

	return 0;
}

#if (defined(CONFIG_BT_CENTRAL) && CONFIG_BT_CENTRAL)

static int hci_cmd_le_create_conn_set_random_addr(bool use_filter, u8_t *own_addr_type, struct net_buf **ret_buf)
{
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;
	int err;

	if (IS_ENABLED(CONFIG_BT_PRIVACY)) {
		if (use_filter || rpa_timeout_valid_check()) {
			err = hci_cmd_le_set_private_addr_create(BT_ID_DEFAULT, &buf);
			if(err) {
				return err;
			}
		} else {
			/* Force new RPA timeout so that RPA timeout is not
			 * triggered while direct initiator is active.
			 */
			le_rpa_invalidate();
			le_update_private_addr();
		}

		if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
			*own_addr_type = BT_HCI_OWN_ADDR_RPA_OR_RANDOM;
		} else {
			*own_addr_type = BT_ADDR_LE_RANDOM;
		}
	} else {
		const bt_addr_le_t *addr = &bt_dev.id_addr[BT_ID_DEFAULT];

		/* If Static Random address is used as Identity address we
		 * need to restore it before creating connection. Otherwise
		 * NRPA used for active scan could be used for connection.
		 */
		if (addr->type == BT_ADDR_LE_RANDOM) {
			err = hci_cmd_le_set_random_address_create(&addr->a, &buf_tmp);
			if (err) {
				return err;
			}
			if (buf_tmp)
			{
				buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
			}
		}

		*own_addr_type = addr->type;
	}

	*ret_buf = buf;

	return 0;
}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
int bt_le_create_conn_ext(const struct bt_conn *conn)
{
	struct bt_hci_cp_le_ext_create_conn *cp;
	struct bt_hci_ext_conn_phy *phy;
	bool use_filter = false;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;
	u8_t own_addr_type;
	u8_t num_phys;
	int err;

	if (IS_ENABLED(CONFIG_BT_WHITELIST)) {
		use_filter = atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT);
	}

	err = hci_cmd_le_create_conn_set_random_addr(use_filter, &own_addr_type, &buf_tmp);
	if (err)
	{
		return err;
	}

	if (buf_tmp) {
		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	num_phys = (!(bt_dev.create_param.options &
		      BT_CONN_LE_OPT_NO_1M) ? 1 : 0) +
		   ((bt_dev.create_param.options &
		      BT_CONN_LE_OPT_CODED) ? 1 : 0);

	buf_tmp = bt_hci_cmd_create(BT_HCI_OP_LE_EXT_CREATE_CONN, sizeof(*cp) +
				num_phys * sizeof(*phy));
	if (!buf_tmp) {
		net_buf_frag_del_all(buf);
		return -ENOBUFS;
	}

	buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	cp = net_buf_add(buf_tmp, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	if (use_filter) {
		/* User Initiated procedure use fast scan parameters. */
		bt_addr_le_copy(&cp->peer_addr, BT_ADDR_LE_ANY);
		cp->filter_policy = BT_HCI_LE_CREATE_CONN_FP_WHITELIST;
	} else {
		const bt_addr_le_t *peer_addr = &conn->le.dst;

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		if (!bt_dev.le.rl_size ||
		    bt_dev.le.rl_entries > bt_dev.le.rl_size) {
			/* Host resolving is used, use the RPA directly. */
			peer_addr = &conn->le.resp_addr;
		}
#endif
		bt_addr_le_copy(&cp->peer_addr, peer_addr);
		cp->filter_policy = BT_HCI_LE_CREATE_CONN_FP_DIRECT;
	}

	cp->own_addr_type = own_addr_type;
	cp->phys = 0;

	if (!(bt_dev.create_param.options & BT_CONN_LE_OPT_NO_1M)) {
		cp->phys |= BT_HCI_LE_EXT_SCAN_PHY_1M;
		phy = net_buf_add(buf, sizeof(*phy));
		phy->scan_interval = sys_cpu_to_le16(
			bt_dev.create_param.interval);
		phy->scan_window = sys_cpu_to_le16(
			bt_dev.create_param.window);
		set_phy_conn_param(conn, phy);
	}

	if (bt_dev.create_param.options & BT_CONN_LE_OPT_CODED) {
		cp->phys |= BT_HCI_LE_EXT_SCAN_PHY_CODED;
		phy = net_buf_add(buf, sizeof(*phy));
		phy->scan_interval = sys_cpu_to_le16(
			bt_dev.create_param.interval_coded);
		phy->scan_window = sys_cpu_to_le16(
			bt_dev.create_param.window_coded);
		set_phy_conn_param(conn, phy);
	}

	cmd_state_set_init(&cmd(buf_tmp)->state, bt_dev.flags, BT_DEV_INITIATING, true);

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_CONN_CREATE));
	err = bt_fsm_hci_cmd_send(buf, NULL, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_frag_del_all(buf);
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_CONN_CREATE));
		return err;
	}

	return 0;
}
#endif

int bt_le_create_conn_legacy(const struct bt_conn *conn)
{
	struct bt_hci_cp_le_create_conn *cp;
	bool use_filter = false;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;
	u8_t own_addr_type;
	int err;

	if (IS_ENABLED(CONFIG_BT_WHITELIST)) {
		use_filter = atomic_test_bit(conn->flags, BT_CONN_AUTO_CONNECT);
	}

	err = hci_cmd_le_create_conn_set_random_addr(use_filter, &own_addr_type, &buf_tmp);
	if (err)
	{
		return err;
	}
	if (buf_tmp) {
		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	buf_tmp = bt_hci_cmd_create(BT_HCI_OP_LE_CREATE_CONN, sizeof(*cp));
	if (!buf_tmp) {
		net_buf_frag_del_all(buf);
		return -ENOBUFS;
	}

	cp = net_buf_add(buf_tmp, sizeof(*cp));
	memset(cp, 0, sizeof(*cp));
	cp->own_addr_type = own_addr_type;

	if (use_filter) {
		/* User Initiated procedure use fast scan parameters. */
		bt_addr_le_copy(&cp->peer_addr, BT_ADDR_LE_ANY);
		cp->filter_policy = BT_HCI_LE_CREATE_CONN_FP_WHITELIST;
	} else {
		const bt_addr_le_t *peer_addr = &conn->le.dst;

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		if (!bt_dev.le.rl_size ||
		    bt_dev.le.rl_entries > bt_dev.le.rl_size) {
			/* Host resolving is used, use the RPA directly. */
			peer_addr = &conn->le.resp_addr;
		}
#endif
		bt_addr_le_copy(&cp->peer_addr, peer_addr);
		cp->filter_policy = BT_HCI_LE_CREATE_CONN_FP_DIRECT;
	}

	cp->scan_interval = sys_cpu_to_le16(bt_dev.create_param.interval);
	cp->scan_window = sys_cpu_to_le16(bt_dev.create_param.window);

	cp->conn_interval_min = sys_cpu_to_le16(conn->le.interval_min);
	cp->conn_interval_max = sys_cpu_to_le16(conn->le.interval_max);
	cp->conn_latency = sys_cpu_to_le16(conn->le.latency);
	cp->supervision_timeout = sys_cpu_to_le16(conn->le.timeout);

	cmd_state_set_init(&cmd(buf_tmp)->state, bt_dev.flags, BT_DEV_INITIATING, true);

	buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CONN_CREATE));
	err = bt_fsm_hci_cmd_send(buf, NULL, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_frag_del_all(buf);
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CONN_CREATE));
		return err;
	}

	return 0;
}
#endif

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)

int hci_cmd_le_scan_update_create(bool fast_scan, struct net_buf **ret_buf);

int bt_fsm_le_scan_update(bool fast_scan, bt_fsm_handle_t fsm)
{
	int err;
	struct net_buf *buf = NULL;
	err = hci_cmd_le_scan_update_create(true, &buf);
	if (err) {
		return err;
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_UPDATE));
	err = bt_fsm_hci_cmd_send(buf, NULL, fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_UPDATE));
		net_buf_frag_del_all(buf);
		return err;
	}

	return err;
}

static int hci_cmd_le_scan_set_random_addr_create(bool active_scan, u8_t *own_addr_type, struct net_buf **ret_buf)
{
	int err;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;

	if (IS_ENABLED(CONFIG_BT_PRIVACY)) {
		err = hci_cmd_le_set_private_addr_create(BT_ID_DEFAULT, &buf_tmp);
		if (err)
		{
			return err;
		}
		if (buf_tmp) {
			buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
		}
		if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
			*own_addr_type = BT_HCI_OWN_ADDR_RPA_OR_RANDOM;
		} else {
			*own_addr_type = BT_ADDR_LE_RANDOM;
		}
	} else {
		struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();

		*own_addr_type = bt_dev.id_addr[0].type;

		/* Use NRPA unless identity has been explicitly requested
		 * (through Kconfig).
		 * Use same RPA as legacy advertiser if advertising.
		 */
		if (!IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY) &&
		    !(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
		      BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) &&
		    adv && !atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
				
			err = hci_cmd_le_set_private_addr_create(BT_ID_DEFAULT, &buf_tmp);
			if (err)
			{
				return err;
			}
			if (buf_tmp) {
				buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
			}

			*own_addr_type = BT_ADDR_LE_RANDOM;
		} else if (IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY) &&
			   *own_addr_type == BT_ADDR_LE_RANDOM) {
			/* If scanning with Identity Address we must set the
			 * random identity address for both active and passive
			 * scanner in order to receive adv reports that are
			 * directed towards this identity.
			 */
			err = hci_cmd_le_set_random_address_create(&bt_dev.id_addr[0].a, &buf_tmp);
			if (err)
			{
				return err;
			}
			if (buf_tmp) {
				buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
			}
		}
	}

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_start_le_scan_ext_create(struct bt_hci_ext_scan_phy *phy_1m,
			     struct bt_hci_ext_scan_phy *phy_coded,
			     u16_t duration, struct net_buf **ret_buf)
{
	struct bt_hci_cp_le_set_ext_scan_param *set_param;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp;
	int err;

	u8_t own_addr_type;
	bool active_scan;

	active_scan = (phy_1m && phy_1m->type == BT_HCI_LE_SCAN_ACTIVE) ||
		      (phy_coded && phy_coded->type == BT_HCI_LE_SCAN_ACTIVE);

	if (duration > 0) {
		atomic_set_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED);

		/* Allow bt_le_oob_get_local to be called directly before
		 * starting a scan limited by timeout.
		 */
		if (IS_ENABLED(CONFIG_BT_PRIVACY) && !rpa_is_new()) {
			atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);
		}
	}

	/* set ramdon address may return NULL, means there is no need to set */
	err = hci_cmd_le_scan_set_random_addr_create(active_scan, &own_addr_type, &buf_tmp);
	if (err)
	{
		return err;
	}

	if (buf_tmp)
	{
		buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	buf_tmp = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EXT_SCAN_PARAM,
				sizeof(*set_param) +
				(phy_1m ? sizeof(*phy_1m) : 0) +
				(phy_coded ? sizeof(*phy_coded) : 0));
	if (!buf_tmp) {
		net_buf_frag_del_all(buf);
		return -ENOBUFS;
	}

	set_param = net_buf_add(buf_tmp, sizeof(*set_param));
	set_param->own_addr_type = own_addr_type;
	set_param->phys = 0;

	if (IS_ENABLED(CONFIG_BT_WHITELIST) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_WL)) {
		set_param->filter_policy = BT_HCI_LE_SCAN_FP_USE_WHITELIST;
	} else {
		set_param->filter_policy = BT_HCI_LE_SCAN_FP_NO_WHITELIST;
	}

	if (phy_1m) {
		set_param->phys |= BT_HCI_LE_EXT_SCAN_PHY_1M;
		net_buf_add_mem(buf_tmp, phy_1m, sizeof(*phy_1m));
	}

	if (phy_coded) {
		set_param->phys |= BT_HCI_LE_EXT_SCAN_PHY_CODED;
		net_buf_add_mem(buf_tmp, phy_coded, sizeof(*phy_coded));
	}

	buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	err = hci_cmd_set_le_ext_scan_enable_create(BT_HCI_LE_SCAN_ENABLE, duration, &buf_tmp);
	if (err)
	{
		net_buf_frag_del_all(buf);
		return err;
	}

	buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	atomic_set_bit_to(bt_dev.flags, BT_DEV_ACTIVE_SCAN, active_scan);

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_le_scan_legacy_create(u8_t scan_type, u16_t interval, u16_t window, struct net_buf **ret_buf)
{
	struct bt_hci_cp_le_set_scan_param set_param;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp;
	bool active_scan;
	int err;

	(void)memset(&set_param, 0, sizeof(set_param));

	set_param.scan_type = scan_type;

	/* for the rest parameters apply default values according to
	 *  spec 4.2, vol2, part E, 7.8.10
	 */
	set_param.interval = sys_cpu_to_le16(interval);
	set_param.window = sys_cpu_to_le16(window);

	if (IS_ENABLED(CONFIG_BT_WHITELIST) &&
	    atomic_test_bit(bt_dev.flags, BT_DEV_SCAN_WL)) {
		set_param.filter_policy = BT_HCI_LE_SCAN_FP_USE_WHITELIST;
	} else {
		set_param.filter_policy = BT_HCI_LE_SCAN_FP_NO_WHITELIST;
	}

	active_scan = scan_type == BT_HCI_LE_SCAN_ACTIVE;
	/* set ramdon address may return NULL, means there is no need to set */
	err = hci_cmd_le_scan_set_random_addr_create(active_scan, &set_param.addr_type, &buf_tmp);
	if (err)
	{
		return err;
	}

	if (buf_tmp)
	{
		buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	buf_tmp = bt_hci_cmd_create(BT_HCI_OP_LE_SET_SCAN_PARAM, sizeof(set_param));
	if (!buf_tmp) {
		net_buf_frag_del_all(buf);
		return -ENOBUFS;
	}

	net_buf_add_mem(buf_tmp, &set_param, sizeof(set_param));
	buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	err = hci_cmd_set_le_scan_enable_create(BT_HCI_LE_SCAN_ENABLE, &buf_tmp);
	if (err) {
		net_buf_frag_del_all(buf);
		return err;
	}

	net_buf_add_mem(buf_tmp, &set_param, sizeof(set_param));
	buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	atomic_set_bit_to(bt_dev.flags, BT_DEV_ACTIVE_SCAN, active_scan);

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_passive_scan_create(bool fast_scan, struct net_buf **ret_buf)
{
	u16_t interval, window;

	if (fast_scan) {
		interval = BT_GAP_SCAN_FAST_INTERVAL;
		window = BT_GAP_SCAN_FAST_WINDOW;
	} else {
		interval = CONFIG_BT_BACKGROUND_SCAN_INTERVAL;
		window = CONFIG_BT_BACKGROUND_SCAN_WINDOW;
	}

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		struct bt_hci_ext_scan_phy scan;

		scan.type = BT_HCI_LE_SCAN_PASSIVE;
		scan.interval = sys_cpu_to_le16(interval);
		scan.window = sys_cpu_to_le16(window);

		return hci_cmd_start_le_scan_ext_create(&scan, NULL, 0, ret_buf);
	}

	return hci_cmd_le_scan_legacy_create(BT_HCI_LE_SCAN_PASSIVE, interval, window, ret_buf);
}

int hci_cmd_le_scan_update_create(bool fast_scan, struct net_buf **ret_buf)
{
	int err;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp;

	if (atomic_test_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN)) {
		return 0;
	}

	if (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
		err = hci_cmd_set_le_scan_enable_create(BT_HCI_LE_SCAN_DISABLE, &buf_tmp);
		if (err) {
			return err;
		}

		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	if (IS_ENABLED(CONFIG_BT_CENTRAL)) {
		struct bt_conn *conn;

		/* don't restart scan if we have pending connection */
		conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL,
					       BT_CONN_CONNECT);
		if (conn) {
			bt_conn_unref(conn);
			*ret_buf = buf;
			return 0;
		}

		conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL,
					       BT_CONN_CONNECT_SCAN);
		if (!conn) {
			*ret_buf = buf;
			return 0;
		}

		atomic_set_bit(bt_dev.flags, BT_DEV_SCAN_FILTER_DUP);

		bt_conn_unref(conn);

		err = hci_cmd_passive_scan_create(fast_scan, &buf_tmp);
		if (err) {
			net_buf_frag_del_all(buf);
			return err;
		}

		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

		*ret_buf = buf;

		return 0;
	}

#if (defined(CONFIG_BT_PER_ADV_SYNC) && CONFIG_BT_PER_ADV_SYNC)
	if (get_pending_per_adv_sync()) {
		err = hci_cmd_passive_scan_create(fast_scan, &buf_tmp);
		if (err) {
			net_buf_frag_del_all(buf);
			return err;
		}

		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

		*ret_buf = buf;

		return 0;
	}
#endif
	*ret_buf = buf;
	return 0;
}

#endif

static int hci_cmd_reset_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_common_init_complete(status, BT_HCI_OP_RESET, buf);
}

static int hci_cmd_read_local_features_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_common_init_complete(status, BT_HCI_OP_READ_LOCAL_FEATURES, buf);
}

static int hci_cmd_read_local_ver_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_common_init_complete(status, BT_HCI_OP_READ_LOCAL_VERSION_INFO, buf);
}

static int hci_cmd_read_supported_commands_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_common_init_complete(status, BT_HCI_OP_READ_SUPPORTED_COMMANDS, buf);
}

#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL) && (CONFIG_BT_HCI_ACL_FLOW_CONTROL)
static int hci_cmd_host_buffer_size_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_common_init_complete(status, BT_HCI_OP_HOST_BUFFER_SIZE, buf);
}

static int hci_cmd_set_ctl_to_host_flow_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_common_init_complete(status, BT_HCI_OP_SET_CTL_TO_HOST_FLOW, buf);
}
#endif

static int hci_cmd_read_le_features_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_READ_LOCAL_FEATURES, buf);
}

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
static int hci_cmd_le_read_buffer_size_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_READ_BUFFER_SIZE, buf);
}
#endif

static int hci_cmd_le_write_le_host_supp_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_WRITE_LE_HOST_SUPP, buf);
}

static int hci_cmd_le_read_supp_states_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_READ_SUPP_STATES, buf);
}

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN) && (defined(CONFIG_BT_DATA_LEN_UPDATE) && CONFIG_BT_DATA_LEN_UPDATE)
static int hci_cmd_le_read_max_data_len_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_READ_MAX_DATA_LEN, buf);
}

static int hci_cmd_write_max_data_len_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_WRITE_DEFAULT_DATA_LEN, buf);
}
#endif

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
static int hci_cmd_le_set_rpa_timeout_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_SET_RPA_TIMEOUT, buf);
}
#endif

static int hci_cmd_le_read_resolving_list_size_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_READ_RL_SIZE, buf);
}
#endif

static int hci_cmd_le_set_event_mask_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_SET_EVENT_MASK, buf);
}

static int hci_cmd_set_event_mask_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_SET_EVENT_MASK, buf);
}

static int hci_cmd_read_bd_addr_complete(u16_t opcode, u8_t status, struct net_buf *buf, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_READ_BD_ADDR, buf);
}

struct hci_cmd_done_handler_t {
	u16_t opcode;
	bt_hci_cmd_func_t handler;
};

#define HCI_CMD_DONE_HANDER(op, handler) {op, handler}

static const struct hci_cmd_done_handler_t hci_cmd_done_handler[] = {
	HCI_CMD_DONE_HANDER(BT_HCI_OP_RESET, hci_cmd_reset_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_READ_LOCAL_FEATURES, hci_cmd_read_local_features_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_READ_LOCAL_VERSION_INFO, hci_cmd_read_local_ver_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_READ_SUPPORTED_COMMANDS, hci_cmd_read_supported_commands_complete),
#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL)
	HCI_CMD_DONE_HANDER(BT_HCI_OP_HOST_BUFFER_SIZE, hci_cmd_host_buffer_size_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_SET_CTL_TO_HOST_FLOW, hci_cmd_set_ctl_to_host_flow_complete),
#endif
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_READ_LOCAL_FEATURES, hci_cmd_read_le_features_complete),
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_READ_BUFFER_SIZE, hci_cmd_le_read_buffer_size_complete),
#endif
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_WRITE_LE_HOST_SUPP, hci_cmd_le_write_le_host_supp_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_READ_SUPP_STATES, hci_cmd_le_read_supp_states_complete),
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)  && CONFIG_BT_CONN && (defined(CONFIG_BT_DATA_LEN_UPDATE) && CONFIG_BT_DATA_LEN_UPDATE)
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_READ_MAX_DATA_LEN, hci_cmd_le_read_max_data_len_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_WRITE_DEFAULT_DATA_LEN, hci_cmd_write_max_data_len_complete),
#endif
#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_SET_RPA_TIMEOUT, hci_cmd_le_set_rpa_timeout_complete),
#endif
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_READ_RL_SIZE, hci_cmd_le_read_resolving_list_size_complete),
#endif
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_SET_EVENT_MASK, hci_cmd_le_set_event_mask_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_SET_EVENT_MASK, hci_cmd_set_event_mask_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_READ_BD_ADDR, hci_cmd_read_bd_addr_complete),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_SET_RANDOM_ADDRESS, hci_le_init_read_random_address_complete_default_handler),
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_SET_ADV_SET_RANDOM_ADDR, hci_le_init_read_random_address_complete_default_handler),
#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	HCI_CMD_DONE_HANDER(BT_HCI_OP_LE_SET_EXT_ADV_PARAM, hci_le_set_ext_adv_param_default_handler),
#endif
};

static int handle_hci_cmd_done_event(struct net_buf *buf, u16_t opcode, u8_t status)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(hci_cmd_done_handler); i++) {
		const struct hci_cmd_done_handler_t *handler = &hci_cmd_done_handler[i];

		if (handler->opcode != opcode) {
			continue;
		}

		if (handler->handler) {
			/* keep buf in use, should unref in handler */
			net_buf_ref(buf);
			return handler->handler(opcode, status, buf, NULL);
		}
	}
	return 0;
}

static int hci_common_init(void)
{
	atomic_set_bit(bt_dev.flags, BT_DEV_ENABLE);
	return hci_common_init_complete(0, 0, NULL);
}

static int hci_common_init_complete(u8_t status, u16_t opcode, struct net_buf *rsp)
{
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_ENABLE) || atomic_test_bit(bt_dev.flags, BT_ST_INIT_DONE))
	{
		return 0;
	}

	if (status)
	{
		BT_ERR("opcode 0x%04x status %d", opcode, status);
		return -EIO;
	}

	switch(opcode)
	{
		case (0):
			goto start;
		case (BT_HCI_OP_RESET):
			goto OP_RESET_COMPLETE;
		case (BT_HCI_OP_READ_LOCAL_FEATURES):
			goto OP_READ_LOCAL_FEATURES_COMPLETE;
		case (BT_HCI_OP_READ_LOCAL_VERSION_INFO):
			goto OP_READ_LOCAL_VERSION_INFO_COMPLETE;
		case (BT_HCI_OP_READ_SUPPORTED_COMMANDS):
			goto OP_READ_SUPPORTED_COMMANDS_COMPLETE;
#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL)
		case (BT_HCI_OP_HOST_BUFFER_SIZE):
			goto OP_HOST_BUFFER_SIZE_COMPLETE;
		case (BT_HCI_OP_SET_CTL_TO_HOST_FLOW):
			goto OP_SET_CTL_TO_HOST_FLOW_COMPLETE;
#endif
		default:
			return 0;
	}

start:
	if (!(bt_dev.drv->quirks & BT_QUIRK_NO_RESET)) {
		/* Send HCI_RESET */
		return bt_hci_cmd_send_cb(BT_HCI_OP_RESET, NULL, NULL);
	}
OP_RESET_COMPLETE:
	if (!(bt_dev.drv->quirks & BT_QUIRK_NO_RESET)) {
		hci_reset_complete(rsp);
		net_buf_unref(rsp);
	}
	/* Read Local Supported Features */
	return bt_hci_cmd_send_cb(BT_HCI_OP_READ_LOCAL_FEATURES, NULL, NULL);
OP_READ_LOCAL_FEATURES_COMPLETE:
	read_local_features_complete(rsp);
	net_buf_unref(rsp);
	/* Read Local Version Information */
	return bt_hci_cmd_send_cb(BT_HCI_OP_READ_LOCAL_VERSION_INFO, NULL,
				   NULL);
OP_READ_LOCAL_VERSION_INFO_COMPLETE:
	read_local_ver_complete(rsp);
	net_buf_unref(rsp);


	/* Read Local Supported Commands */
	return bt_hci_cmd_send_cb(BT_HCI_OP_READ_SUPPORTED_COMMANDS, NULL,
				   NULL);
	if (err) {
		return err;
	}
OP_READ_SUPPORTED_COMMANDS_COMPLETE:
	read_supported_commands_complete(rsp);
	net_buf_unref(rsp);


	if (IS_ENABLED(CONFIG_BT_HOST_CRYPTO)) {
		/* Initialize the PRNG so that it is safe to use it later
		 * on in the initialization process.
		 */
		extern int bt_crypto_rand_init(void);
		err = bt_crypto_rand_init();
		if (err) {
			return err;
		}
	}

#if (defined(CONFIG_BT_HCI_ACL_FLOW_CONTROL) && CONFIG_BT_HCI_ACL_FLOW_CONTROL)
	struct net_buf *buf;
	struct bt_hci_cp_host_buffer_size *hbs;

	/* Check if host flow control is actually supported */
	if (!BT_CMD_TEST(bt_dev.supported_commands, 10, 5)) {
		BT_WARN("Controller to host flow control not supported");
		return 0;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_HOST_BUFFER_SIZE,
				sizeof(*hbs));
	if (!buf) {
		return -ENOBUFS;
	}

	hbs = net_buf_add(buf, sizeof(*hbs));
	(void)memset(hbs, 0, sizeof(*hbs));
	hbs->acl_mtu = sys_cpu_to_le16(CONFIG_BT_L2CAP_RX_MTU +
				       sizeof(struct bt_l2cap_hdr));
	hbs->acl_pkts = sys_cpu_to_le16(CONFIG_BT_ACL_RX_COUNT);

	return bt_hci_cmd_send_cb(BT_HCI_OP_HOST_BUFFER_SIZE, buf, NULL);

OP_HOST_BUFFER_SIZE_COMPLETE:
	net_buf_unref(rsp);
	buf = bt_hci_cmd_create(BT_HCI_OP_SET_CTL_TO_HOST_FLOW, 1);
	if (!buf) {
		return -ENOBUFS;
	}

	net_buf_add_u8(buf, BT_HCI_CTL_TO_HOST_FLOW_ENABLE);
	return bt_hci_cmd_send_cb(BT_HCI_OP_SET_CTL_TO_HOST_FLOW, buf, NULL);

OP_SET_CTL_TO_HOST_FLOW_COMPLETE:
	net_buf_unref(rsp);
#endif /* CONFIG_BT_HCI_ACL_FLOW_CONTROL */

	return hci_le_init();
}

static int hci_le_set_event_mask(void)
{
	u64_t mask = 0U;
	struct bt_hci_cp_le_set_event_mask *cp_mask;
	struct net_buf *buf;

	/* Set LE event mask */
	buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EVENT_MASK, sizeof(*cp_mask));
	if (!buf) {
		return -ENOBUFS;
	}

	cp_mask = net_buf_add(buf, sizeof(*cp_mask));

	mask |= BT_EVT_MASK_LE_ADVERTISING_REPORT;

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		mask |= BT_EVT_MASK_LE_ADV_SET_TERMINATED;
		mask |= BT_EVT_MASK_LE_SCAN_REQ_RECEIVED;
		mask |= BT_EVT_MASK_LE_EXT_ADVERTISING_REPORT;
		mask |= BT_EVT_MASK_LE_SCAN_TIMEOUT;
	}

	if (IS_ENABLED(CONFIG_BT_CONN)) {
		if ((IS_ENABLED(CONFIG_BT_SMP) &&
		     BT_FEAT_LE_PRIVACY(bt_dev.le.features)) ||
		    (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
		     BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
			/* C24:
			 * Mandatory if the LE Controller supports Connection
			 * State and either LE Feature (LL Privacy) or
			 * LE Feature (Extended Advertising) is supported, ...
			 */
			mask |= BT_EVT_MASK_LE_ENH_CONN_COMPLETE;
		} else {
			mask |= BT_EVT_MASK_LE_CONN_COMPLETE;
		}

		mask |= BT_EVT_MASK_LE_CONN_UPDATE_COMPLETE;
		mask |= BT_EVT_MASK_LE_REMOTE_FEAT_COMPLETE;

		if (BT_FEAT_LE_CONN_PARAM_REQ_PROC(bt_dev.le.features)) {
			mask |= BT_EVT_MASK_LE_CONN_PARAM_REQ;
		}

		if (IS_ENABLED(CONFIG_BT_DATA_LEN_UPDATE) &&
		    BT_FEAT_LE_DLE(bt_dev.le.features)) {
			mask |= BT_EVT_MASK_LE_DATA_LEN_CHANGE;
		}

		if (IS_ENABLED(CONFIG_BT_PHY_UPDATE) &&
		    (BT_FEAT_LE_PHY_2M(bt_dev.le.features) ||
		     BT_FEAT_LE_PHY_CODED(bt_dev.le.features))) {
			mask |= BT_EVT_MASK_LE_PHY_UPDATE_COMPLETE;
		}
	}

	if (IS_ENABLED(CONFIG_BT_SMP) &&
	    BT_FEAT_LE_ENCR(bt_dev.le.features)) {
		mask |= BT_EVT_MASK_LE_LTK_REQUEST;
	}

	/*
	 * If "LE Read Local P-256 Public Key" and "LE Generate DH Key" are
	 * supported we need to enable events generated by those commands.
	 */
	if (IS_ENABLED(CONFIG_BT_ECC) &&
	    (BT_CMD_TEST(bt_dev.supported_commands, 34, 1)) &&
	    (BT_CMD_TEST(bt_dev.supported_commands, 34, 2))) {
		mask |= BT_EVT_MASK_LE_P256_PUBLIC_KEY_COMPLETE;
		mask |= BT_EVT_MASK_LE_GENERATE_DHKEY_COMPLETE;
	}
	sys_put_le64(mask, cp_mask->events);
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_EVENT_MASK, buf, NULL);
}

static int hci_set_event_mask(void)
{
	u64_t mask = 0U;
	struct bt_hci_cp_set_event_mask *ev;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(BT_HCI_OP_SET_EVENT_MASK, sizeof(*ev));
	if (!buf) {
		return -ENOBUFS;
	}

	ev = net_buf_add(buf, sizeof(*ev));

	if (IS_ENABLED(CONFIG_BT_BREDR)) {
		/* Since we require LE support, we can count on a
		 * Bluetooth 4.0 feature set
		 */
		mask |= BT_EVT_MASK_INQUIRY_COMPLETE;
		mask |= BT_EVT_MASK_CONN_COMPLETE;
		mask |= BT_EVT_MASK_CONN_REQUEST;
		mask |= BT_EVT_MASK_AUTH_COMPLETE;
		mask |= BT_EVT_MASK_REMOTE_NAME_REQ_COMPLETE;
		mask |= BT_EVT_MASK_REMOTE_FEATURES;
		mask |= BT_EVT_MASK_ROLE_CHANGE;
		mask |= BT_EVT_MASK_PIN_CODE_REQ;
		mask |= BT_EVT_MASK_LINK_KEY_REQ;
		mask |= BT_EVT_MASK_LINK_KEY_NOTIFY;
		mask |= BT_EVT_MASK_INQUIRY_RESULT_WITH_RSSI;
		mask |= BT_EVT_MASK_REMOTE_EXT_FEATURES;
		mask |= BT_EVT_MASK_SYNC_CONN_COMPLETE;
		mask |= BT_EVT_MASK_EXTENDED_INQUIRY_RESULT;
		mask |= BT_EVT_MASK_IO_CAPA_REQ;
		mask |= BT_EVT_MASK_IO_CAPA_RESP;
		mask |= BT_EVT_MASK_USER_CONFIRM_REQ;
		mask |= BT_EVT_MASK_USER_PASSKEY_REQ;
		mask |= BT_EVT_MASK_SSP_COMPLETE;
		mask |= BT_EVT_MASK_USER_PASSKEY_NOTIFY;
	}

	mask |= BT_EVT_MASK_HARDWARE_ERROR;
	mask |= BT_EVT_MASK_DATA_BUFFER_OVERFLOW;
	mask |= BT_EVT_MASK_LE_META_EVENT;

	if (IS_ENABLED(CONFIG_BT_CONN)) {
		mask |= BT_EVT_MASK_DISCONN_COMPLETE;
		mask |= BT_EVT_MASK_REMOTE_VERSION_INFO;
	}

	if (IS_ENABLED(CONFIG_BT_SMP) &&
	    BT_FEAT_LE_ENCR(bt_dev.le.features)) {
		mask |= BT_EVT_MASK_ENCRYPT_CHANGE;
		mask |= BT_EVT_MASK_ENCRYPT_KEY_REFRESH_COMPLETE;
	}

	sys_put_le64(mask, ev->events);
	return bt_hci_cmd_send_cb(BT_HCI_OP_SET_EVENT_MASK, buf, NULL);
}

static int hci_le_init()
{
	return hci_le_init_complete(0, 0, NULL);
}

static int hci_le_init_read_random_address_complete_default_handler(u16_t opcode, u8_t status, struct net_buf *rsp, void *args)
{
	bt_addr_copy(&bt_dev.random_addr.a, &bt_dev.pending_random_addr.a);
	bt_dev.random_addr.type = BT_ADDR_LE_RANDOM;

	memset(&bt_dev.pending_random_addr, 0, sizeof(&bt_dev.pending_random_addr));

	net_buf_unref(rsp);
	return 0;
}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
static int hci_le_set_ext_adv_param_default_handler(u16_t opcode, u8_t status, struct net_buf *rsp, void *args)
{
	struct bt_le_ext_adv *adv = args;
	struct bt_hci_rp_le_set_ext_adv_param *rp = (void *)rsp->data;

	if (!status && adv && rp)
	{
		adv->tx_power = rp->tx_power;
	}

	net_buf_unref(rsp);

	return 0;
}
#endif

static int hci_le_init_read_random_address_complete(u16_t opcode, u8_t status, struct net_buf *rsp, void *args)
{
	return hci_le_init_complete(status, BT_HCI_OP_LE_SET_RANDOM_ADDRESS, rsp);
}

static int hci_le_init_complete(u8_t status, u16_t opcode, struct net_buf *rsp)
{
	struct bt_hci_cp_write_le_host_supp *cp_le;
	struct net_buf *buf;
	int err = 0;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_ENABLE) || atomic_test_bit(bt_dev.flags, BT_ST_INIT_DONE))
	{
		net_buf_unref(rsp);
		BT_DBG("");
		return 0;
	}

	if (status)
	{
		BT_ERR("opcode 0x%04x status %d", opcode, status);
		ble_event_init_done(-EIO);
		return -EIO;
	}

	/* For now we only support LE capable controllers */
	if (!BT_FEAT_LE(bt_dev.features)) {
		BT_ERR("Non-LE capable controller detected!");
		return -ENODEV;
	}

	switch(opcode)
	{
		case (0):
			goto start;
		case (BT_HCI_OP_LE_READ_LOCAL_FEATURES):
			goto OP_LE_READ_LOCAL_FEATURES_COMPLETE;
		case (BT_HCI_OP_LE_READ_BUFFER_SIZE):
			goto OP_LE_READ_BUFFER_SIZE_COMPLETE;
		case (BT_HCI_OP_LE_WRITE_LE_HOST_SUPP):
			goto OP_LE_WRITE_LE_HOST_SUPP_COMPLETE;
		case (BT_HCI_OP_LE_READ_SUPP_STATES):
			goto OP_LE_READ_SUPP_STATES_COMPLETE;
		case (BT_HCI_OP_LE_READ_MAX_DATA_LEN):
			goto OP_LE_READ_MAX_DATA_LEN_COMPLETE;
		case (BT_HCI_OP_LE_WRITE_DEFAULT_DATA_LEN):
			goto OP_LE_WRITE_DEFAULT_DATA_LEN_COMPLETE;
		case (BT_HCI_OP_LE_SET_RPA_TIMEOUT):
			goto OP_LE_SET_RPA_TIMEOUT_COMPLETE;
		case (BT_HCI_OP_LE_READ_RL_SIZE):
			goto OP_LE_READ_RL_SIZE_COMPLETE;
		case (BT_HCI_OP_LE_SET_EVENT_MASK):
			goto OP_LE_SET_EVENT_MASK_COMPLETE;
		case (BT_HCI_OP_SET_EVENT_MASK):
			goto OP_SET_EVENT_MASK_COMPLETE;
		case (BT_HCI_OP_READ_BD_ADDR):
			goto OP_READ_BD_ADDR_COMPLETE;
		case (BT_HCI_OP_LE_SET_RANDOM_ADDRESS):
			goto OP_LE_SET_RANDOM_ADDRESS;
		default:
			return 0;
	}

start:
	/* Read Low Energy Supported Features */
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_LOCAL_FEATURES, NULL, NULL);

OP_LE_READ_LOCAL_FEATURES_COMPLETE:
	read_le_features_complete(rsp);
	net_buf_unref(rsp);

#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	/* Read LE Buffer Size */
	return bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_BUFFER_SIZE,
				   NULL, NULL);
#endif
OP_LE_READ_BUFFER_SIZE_COMPLETE:
#if (defined(CONFIG_BT_CONN) && CONFIG_BT_CONN)
	le_read_buffer_size_complete(rsp);
	net_buf_unref(rsp);
#endif

	if (BT_FEAT_BREDR(bt_dev.features)) {
		buf = bt_hci_cmd_create(BT_HCI_OP_LE_WRITE_LE_HOST_SUPP,
					sizeof(*cp_le));
		if (!buf) {
			return -ENOBUFS;
		}

		cp_le = net_buf_add(buf, sizeof(*cp_le));

		/* Explicitly enable LE for dual-mode controllers */
		cp_le->le = 0x01;
		cp_le->simul = 0x00;
		return bt_hci_cmd_send_cb(BT_HCI_OP_LE_WRITE_LE_HOST_SUPP, buf,
					   NULL);
	}
OP_LE_WRITE_LE_HOST_SUPP_COMPLETE:
	if (BT_FEAT_BREDR(bt_dev.features)) {
		net_buf_unref(rsp);
	}
	/* Read LE Supported States */
	if (BT_CMD_LE_STATES(bt_dev.supported_commands)) {
		return bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_SUPP_STATES, NULL,
					   NULL);
	}
OP_LE_READ_SUPP_STATES_COMPLETE:
	if (BT_CMD_LE_STATES(bt_dev.supported_commands)) {
		le_read_supp_states_complete(rsp);
		net_buf_unref(rsp);
	}

	if (IS_ENABLED(CONFIG_BT_CONN) &&
	    IS_ENABLED(CONFIG_BT_DATA_LEN_UPDATE) &&
	    BT_FEAT_LE_DLE(bt_dev.le.features)) {
		return bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_MAX_DATA_LEN, NULL, NULL);
	}
OP_LE_READ_MAX_DATA_LEN_COMPLETE:
	if (IS_ENABLED(CONFIG_BT_CONN) &&
	    IS_ENABLED(CONFIG_BT_DATA_LEN_UPDATE) &&
	    BT_FEAT_LE_DLE(bt_dev.le.features)) {
			struct bt_hci_rp_le_read_max_data_len *rp;
			u16_t tx_octets, tx_time;
			rp = (void *)rsp->data;
			tx_octets = sys_le16_to_cpu(rp->max_tx_octets);
			tx_time = sys_le16_to_cpu(rp->max_tx_time);
			net_buf_unref(rsp);

			struct bt_hci_cp_le_write_default_data_len *cp;
			
			buf = bt_hci_cmd_create(BT_HCI_OP_LE_WRITE_DEFAULT_DATA_LEN,
						sizeof(*cp));
			if (!buf) {
				return -ENOBUFS;
			}

			cp = net_buf_add(buf, sizeof(*cp));
			cp->max_tx_octets = sys_cpu_to_le16(tx_octets);
			cp->max_tx_time = sys_cpu_to_le16(tx_time);

			return bt_hci_cmd_send_cb(BT_HCI_OP_LE_WRITE_DEFAULT_DATA_LEN,
						buf, NULL);
	}
OP_LE_WRITE_DEFAULT_DATA_LEN_COMPLETE:
	if (IS_ENABLED(CONFIG_BT_CONN) &&
	    IS_ENABLED(CONFIG_BT_DATA_LEN_UPDATE) &&
	    BT_FEAT_LE_DLE(bt_dev.le.features)) {
			net_buf_unref(rsp);
	}
#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
		struct bt_hci_cp_le_set_rpa_timeout *cp;

		buf = bt_hci_cmd_create(BT_HCI_OP_LE_SET_RPA_TIMEOUT,
					sizeof(*cp));
		if (!buf) {
			return -ENOBUFS;
		}

		cp = net_buf_add(buf, sizeof(*cp));
		cp->rpa_timeout = sys_cpu_to_le16(CONFIG_BT_RPA_TIMEOUT);
		return bt_hci_cmd_send_cb(BT_HCI_OP_LE_SET_RPA_TIMEOUT, buf,
					   NULL);
#endif /* (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY) */
	}
#endif
OP_LE_SET_RPA_TIMEOUT_COMPLETE:

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
		net_buf_unref(rsp);
#endif /* (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY) */
		return bt_hci_cmd_send_cb(BT_HCI_OP_LE_READ_RL_SIZE, NULL,
					   NULL);
	}
#endif
OP_LE_READ_RL_SIZE_COMPLETE:
#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
	if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
		le_read_resolving_list_size_complete(rsp);
		net_buf_unref(rsp);
	}
#endif
	return hci_le_set_event_mask();
OP_LE_SET_EVENT_MASK_COMPLETE:
	net_buf_unref(rsp);
	return hci_set_event_mask();
	
OP_SET_EVENT_MASK_COMPLETE:
	net_buf_unref(rsp);

	if (!IS_ENABLED(CONFIG_BT_SETTINGS) && !bt_dev.id_count) {
			BT_DBG("No user identity. Trying to set public.");
		return bt_hci_cmd_send_cb(BT_HCI_OP_READ_BD_ADDR, NULL, NULL);
	}
OP_READ_BD_ADDR_COMPLETE:
	if (!IS_ENABLED(CONFIG_BT_SETTINGS) && !bt_dev.id_count) {
		bt_setup_public_id_addr_cb(BT_HCI_OP_READ_BD_ADDR, status, rsp, NULL);
	}

	if (!IS_ENABLED(CONFIG_BT_SETTINGS) && !bt_dev.id_count) {
		net_buf_unref(rsp);
		BT_DBG("No public address. Trying to set static random.");
		err = bt_setup_random_id_addr();
		if (err) {
			BT_ERR("Unable to set identity address");
			return err;
		}
		bt_hci_cmd_cb_t cb = {
			.func = hci_le_init_read_random_address_complete,
		};
		return set_random_address_cb(&bt_dev.id_addr[0].a, &cb);
	}

OP_LE_SET_RANDOM_ADDRESS:
	if (!IS_ENABLED(CONFIG_BT_SETTINGS) && !bt_dev.id_count) {
		net_buf_unref(rsp);
	}
	atomic_set_bit(bt_dev.flags, BT_ST_INIT_DONE);

	if (IS_ENABLED(CONFIG_BT_CONN)) {
		err = bt_conn_init();
		if (err) {
			BT_ERR("bt_conn_init fail %d", err);
			return err;
		}
	}

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	k_delayed_work_init(&bt_dev.rpa_update, rpa_timeout);
#endif

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		if (!bt_dev.id_count) {
			BT_INFO("No ID address. App must call settings_load()");
			return 0;
		}

		atomic_set_bit(bt_dev.flags, BT_DEV_PRESET_ID);
	}

	bt_finalize_init();

	return 0;
}

static u8_t bt_hci_cmd_read_public_addr(bt_hci_cmd_cb_t *cb)
{
	int err;

	/* Read Bluetooth Address */
	err = bt_hci_cmd_send_cb(BT_HCI_OP_READ_BD_ADDR, NULL, cb);
	if (err) {
		BT_WARN("Failed to read public address");
		return 0U;
	}

	return 0;
}

static int hci_cmd_le_set_ad_create(u16_t hci_op, const struct bt_ad *ad, size_t ad_len, struct net_buf **ret_buf)
{
	int err;
	struct bt_hci_cp_le_set_adv_data *set_data;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(hci_op, sizeof(*set_data));
	if (!buf) {
		return -ENOBUFS;
	}

	set_data = net_buf_add(buf, sizeof(*set_data));
	(void)memset(set_data, 0, sizeof(*set_data));

	err = set_data_add_complete(set_data->data, BT_GAP_ADV_MAX_ADV_DATA_LEN,
			   ad, ad_len, &set_data->len);
	if (err) {
		net_buf_unref(buf);
		return err;
	}

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_le_set_adv_ext_complete(struct bt_le_ext_adv *adv, uint16_t hci_op,
				    const struct bt_ad *ad, size_t ad_len, struct net_buf **ret_buf)
{

	int err;
	uint8_t max_data_size;
	struct bt_hci_cp_le_set_ext_adv_data *set_data;
	struct net_buf *buf;

	buf = bt_hci_cmd_create(hci_op, sizeof(*set_data));
	if (!buf) {
		return -ENOBUFS;
	}

	set_data = net_buf_add(buf, sizeof(*set_data));
	(void)memset(set_data, 0, sizeof(*set_data));

	if (atomic_test_bit(adv->flags, BT_ADV_EXT_ADV)) {
		max_data_size = BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN;
	} else {
		max_data_size = BT_GAP_ADV_MAX_ADV_DATA_LEN;
	}

	err = set_data_add_complete(set_data->data, max_data_size, ad, ad_len,
				    &set_data->len);

	if (err) {
		net_buf_unref(buf);
		return err;
	}

	set_data->handle = adv->handle;
	set_data->op = BT_HCI_LE_EXT_ADV_OP_COMPLETE_DATA;
	set_data->frag_pref = BT_HCI_LE_EXT_ADV_FRAG_DISABLED;

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_le_set_adv_ext_fragmented(struct bt_le_ext_adv *adv, uint16_t hci_op,
				      const struct bt_ad *ad, size_t ad_len, struct net_buf **ret_buf)
{
	struct bt_hci_cp_le_set_ext_adv_data *set_data;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp;

	for (size_t i = 0; i < ad_len; i++) {

		const struct bt_data *data = ad[i].data;

		for (size_t j = 0; j < ad[i].len; j++) {
			size_t len = data[j].data_len;
			uint8_t type = data[j].type;
			size_t offset = 0;

			/* We can't necessarily set one AD field in a single step. */
			while (offset < data[j].data_len) {
				buf_tmp = bt_hci_cmd_create(hci_op, sizeof(*set_data));
				if (!buf_tmp) {
					net_buf_frag_del_all(buf);
					return -ENOBUFS;
				}

				set_data = net_buf_add(buf_tmp, sizeof(*set_data));
				(void)memset(set_data, 0, sizeof(*set_data));

				set_data->handle = adv->handle;
				set_data->frag_pref = BT_HCI_LE_EXT_ADV_FRAG_DISABLED;

				/* Determine the operation parameter value. */
				if ((i == 0) && (j == 0) && (offset == 0)) {
					set_data->op = BT_HCI_LE_EXT_ADV_OP_FIRST_FRAG;
				} else if ((i == ad_len - 1) && (j == ad[i].len - 1)) {
					/* The last AD field may be split into
					 * one or two commands.
					 */
					if (offset != 0) {
						/* We can always set the data in two operations
						 * Therefore, we know that this is the last.
						 */
						set_data->op = BT_HCI_LE_EXT_ADV_OP_LAST_FRAG;
					} else if (len + 2 <= BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN) {
						/* First part fits. */
						set_data->op = BT_HCI_LE_EXT_ADV_OP_LAST_FRAG;
					} else {
						/* The data must be split into two
						 * commands.
						 */
						set_data->op = BT_HCI_LE_EXT_ADV_OP_INTERM_FRAG;
					}
				} else {
					set_data->op = BT_HCI_LE_EXT_ADV_OP_INTERM_FRAG;
				}

				if (offset == 0) {
					set_data->len = MIN(len + 2,
							    BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN);
				} else {
					/* No need to take min operation here,
					 * as we can always fit the second part.
					 */
					set_data->len = len - offset;
				}

				if (offset == 0) {
					set_data->data[0] = len + 1;
					set_data->data[1] = type;
					memcpy(&set_data->data[2], data[j].data, set_data->len);
					offset += set_data->len - 2;
				} else {
					memcpy(&set_data->data[0], &data[j].data[offset],
					       set_data->len);
					offset += set_data->len;
				}

				buf = buf ?  net_buf_frag_add(buf, buf_tmp) : buf_tmp;
			}

		}
	}

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_le_set_ad_ext_create(struct bt_le_ext_adv *adv, uint16_t hci_op,
			  const struct bt_ad *ad, size_t ad_len, struct net_buf **ret_buf)
{
	size_t total_len_bytes = 0;

	for (size_t i = 0; i < ad_len; i++) {
		for (size_t j = 0; j < ad[i].len; j++) {
			total_len_bytes += ad[i].data[j].data_len + 2;
		}
	}

	if ((total_len_bytes > BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN) &&
	    atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		/* It is not allowed to set advertising data in multiple
		 * operations while the advertiser is running.
		 */
		return -EAGAIN;
	}

	if (total_len_bytes <= BT_HCI_LE_EXT_ADV_FRAG_MAX_LEN) {
		/* If possible, set all data at once.
		 * This allows us to update advertising data while advertising.
		 */
		return hci_cmd_le_set_adv_ext_complete(adv, hci_op, ad, ad_len, ret_buf);
	} else {
		return hci_cmd_le_set_adv_ext_fragmented(adv, hci_op, ad, ad_len, ret_buf);
	}

	return 0;
}

static int hci_cmd_ad_create(struct bt_le_ext_adv *adv, const struct bt_ad *ad,
		  size_t ad_len, struct net_buf **ret_buf)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return hci_cmd_le_set_ad_ext_create(adv, BT_HCI_OP_LE_SET_EXT_ADV_DATA,
				      ad, ad_len, ret_buf);
	}

	return hci_cmd_le_set_ad_create(BT_HCI_OP_LE_SET_ADV_DATA, ad, ad_len, ret_buf);
}

static int hci_cmd_sd_create(struct bt_le_ext_adv *adv, const struct bt_ad *sd,
		  size_t sd_len, struct net_buf **buf)
{
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return hci_cmd_le_set_ad_ext_create(adv, BT_HCI_OP_LE_SET_EXT_SCAN_RSP_DATA,
				      sd, sd_len, buf);
	}

	return hci_cmd_le_set_ad_create(BT_HCI_OP_LE_SET_SCAN_RSP_DATA, sd, sd_len, buf);
}

static int read_public_addr_complete(bt_addr_le_t *addr, struct net_buf *rsp)
{
	struct bt_hci_rp_read_bd_addr *rp;
	rp = (void *)rsp->data;

	if (!bt_addr_cmp(&rp->bdaddr, BT_ADDR_ANY) ||
	    !bt_addr_cmp(&rp->bdaddr, BT_ADDR_NONE)) {
		BT_DBG("Controller has no public address");
		net_buf_unref(rsp);
		return -1;
	}

	bt_addr_copy(&addr->a, &rp->bdaddr);
	addr->type = BT_ADDR_LE_PUBLIC;

	net_buf_unref(rsp);

	return 0;
}

static int bt_setup_public_id_addr_cb(u16_t opcode, u8_t status, struct net_buf *rsp, void *args)
{
	bt_addr_le_t addr;
	u8_t *irk = NULL;

	if (read_public_addr_complete(&addr, rsp)) {
		BT_DBG("Controller has no public address");
		return 0U;
	}

	bt_dev.id_count = 1;

#if (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY)
	u8_t ir_irk[16];
	u8_t ir[16];

	bt_read_identity_root(ir);

	if (!bt_smp_irk_get(ir, ir_irk)) {
		irk = ir_irk;
	} else if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		atomic_set_bit(bt_dev.flags, BT_DEV_STORE_ID);
	}
#endif /* (defined(CONFIG_BT_PRIVACY) && CONFIG_BT_PRIVACY) */

	id_create(BT_ID_DEFAULT, &addr, irk);

	return 0;
}

void bt_setup_public_id_addr(void)
{
	bt_hci_cmd_cb_t set_cb = {
		.func = bt_setup_public_id_addr_cb,
	};
	bt_hci_cmd_read_public_addr(&set_cb);
}

static int hci_cmd_le_adv_data_create(struct bt_le_ext_adv *adv,
			 const struct bt_data *ad, size_t ad_len,
			 const struct bt_data *sd, size_t sd_len,
			 bool scannable, bool use_name, 
			struct net_buf **ret_buf)
{
	struct bt_ad d[2] = {};
	struct bt_data data;
	size_t d_len;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;
	int err;

	if (use_name) {
		const char *name = bt_get_name();

		if ((ad && ad_has_name(ad, ad_len)) ||
			(sd && ad_has_name(sd, sd_len))) {
			/* Cannot use name if name is already set */
			return -EINVAL;
		}

		data = (struct bt_data)BT_DATA(
			BT_DATA_NAME_COMPLETE,
			name, strlen(name));
	}

	if (ad && ad_len > 0)
	{
		d_len = 1;
		d[0].data = ad;
		d[0].len = ad_len;

		if (use_name && !scannable) {
			d[1].data = &data;
			d[1].len = 1;
			d_len = 2;
		}

		err = hci_cmd_ad_create(adv, d, d_len, &buf);
		if (err) {
			return err;
		}
	}

	if (scannable) {
		d_len = 1;
		d[0].data = sd;
		d[0].len = sd_len;

		if (use_name) {
			d[1].data = &data;
			d[1].len = 1;
			d_len = 2;
		}

		err = hci_cmd_sd_create(adv, d, d_len, &buf_tmp);
		if (err) {
			net_buf_frag_del_all(buf);
			return err;
		}

		buf = buf ?  net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	atomic_set_bit(adv->flags, BT_ADV_DATA_SET);

	*ret_buf = buf;

	return 0;
}

static int hci_cmd_le_adv_set_random_addr_create(struct bt_le_ext_adv *adv, u32_t options,
				  bool dir_adv, u8_t *own_addr_type, struct net_buf **ret_buf)
{
	const bt_addr_le_t *id_addr;
	struct net_buf *buf = NULL;
	int err;

	/* Set which local identity address we're advertising with */
	id_addr = &bt_dev.id_addr[adv->id];

	if (options & BT_LE_ADV_OPT_CONNECTABLE) {
		if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
		    !(options & BT_LE_ADV_OPT_USE_IDENTITY)) {
			err = hci_cmd_le_adv_set_private_addr_create(adv, &buf);
			if (err)
			{
				return err;
			}

			if (BT_FEAT_LE_PRIVACY(bt_dev.le.features)) {
				*own_addr_type = BT_HCI_OWN_ADDR_RPA_OR_RANDOM;
			} else {
				*own_addr_type = BT_ADDR_LE_RANDOM;
			}
		} else {
			/*
			 * If Static Random address is used as Identity
			 * address we need to restore it before advertising
			 * is enabled. Otherwise NRPA used for active scan
			 * could be used for advertising.
			 */
			if (id_addr->type == BT_ADDR_LE_RANDOM) {
				err = hci_cmd_adv_random_address_create(adv, &id_addr->a, &buf);
				if (err)
				{
					return err;
				}
			}

			*own_addr_type = id_addr->type;
		}

		if (dir_adv) {
			if (IS_ENABLED(CONFIG_BT_SMP) &&
			    !IS_ENABLED(CONFIG_BT_PRIVACY) &&
			    BT_FEAT_LE_PRIVACY(bt_dev.le.features) &&
			    (options & BT_LE_ADV_OPT_DIR_ADDR_RPA)) {
				/* This will not use RPA for our own address
				 * since we have set zeroed out the local IRK.
				 */
				*own_addr_type |= BT_HCI_OWN_ADDR_RPA_MASK;
			}
		}
	} else {
		if (options & BT_LE_ADV_OPT_USE_IDENTITY) {
			if (id_addr->type == BT_ADDR_LE_RANDOM) {
				err = hci_cmd_adv_random_address_create(adv, &id_addr->a, &buf);
				if (err)
				{
					return err;
				}
			}

			*own_addr_type = id_addr->type;
		} else if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
			     BT_FEAT_LE_EXT_ADV(bt_dev.le.features))) {
			/* In case advertising set random address is not
			 * available we must handle the shared random address
			 * problem.
			 */
#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
			bool scan_enabled = false;

			/* If active scan with NRPA is ongoing refresh NRPA */
			if (!IS_ENABLED(CONFIG_BT_PRIVACY) &&
			    !IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY) &&
			    atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING) &&
			    atomic_test_bit(bt_dev.flags, BT_DEV_ACTIVE_SCAN)) {
				scan_enabled = true;
				err = hci_cmd_set_le_scan_enable_create(false, &buf);
				if (err)
				{
					return err;
				}
			}
#endif /* (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) */
			struct net_buf *buf_tmp;
			err = hci_cmd_le_adv_set_private_addr_create(adv, &buf_tmp);
			if (err)
			{
				net_buf_frag_del_all(buf);
				return err;
			}

			buf = buf ?  net_buf_frag_add(buf, buf_tmp) : buf_tmp;
			*own_addr_type = BT_ADDR_LE_RANDOM;

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
			if (scan_enabled) {
				err = hci_cmd_set_le_scan_enable_create(true, &buf_tmp);
				if (err)
				{
					net_buf_frag_del_all(buf);
					return err;
				}

				buf = buf ?  net_buf_frag_add(buf, buf_tmp) : buf_tmp;
			}
#endif /* (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) */
		} else {
			err = hci_cmd_le_adv_set_private_addr_create(adv, &buf);
			if (err)
			{
				return err;
			}

			*own_addr_type = BT_ADDR_LE_RANDOM;
		}
	}

	*ret_buf = buf;

	return 0;
}

int bt_le_adv_start_legacy(const struct bt_le_adv_param *param,
			   const struct bt_data *ad, size_t ad_len,
			   const struct bt_data *sd, size_t sd_len)
{
	struct bt_conn *conn = NULL;
	bool dir_adv = (param->peer != NULL), scannable;
	int err;
	struct bt_le_ext_adv *adv;
	struct bt_hci_cp_le_set_adv_param set_param;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING))) {
		return -EBUSY;
	}

	if (!valid_adv_param(param)) {
		return -EINVAL;
	}

	if (!bt_le_adv_random_addr_check(param)) {
		return -EINVAL;
	}

	(void)memset(&set_param, 0, sizeof(set_param));

	set_param.min_interval = sys_cpu_to_le16(param->interval_min);
	set_param.max_interval = sys_cpu_to_le16(param->interval_max);
	set_param.channel_map  = param->channel_map ? param->channel_map : 0x07;
	set_param.filter_policy = get_filter_policy(param->options);

	adv = adv_new_legacy();
	if (!adv || atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EALREADY;
	}

	if (adv->id != param->id) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);
	}

	adv->id = param->id;
	bt_dev.adv_conn_id = adv->id;

	/* set ramdon address may return NULL, means there is no need to set */
	err = hci_cmd_le_adv_set_random_addr_create(adv, param->options, dir_adv,
				     &set_param.own_addr_type, &buf_tmp);
	if (err)
	{
		return err;
	}
	if (buf_tmp) {
		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	if (dir_adv) {
		bt_addr_le_copy(&adv->target_addr, param->peer);
	} else {
		bt_addr_le_copy(&adv->target_addr, BT_ADDR_LE_ANY);
	}

	if (param->options & BT_LE_ADV_OPT_CONNECTABLE) {
		scannable = true;

		if (dir_adv) {
			if (param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY) {
				set_param.type = BT_HCI_ADV_DIRECT_IND_LOW_DUTY;
			} else {
				set_param.type = BT_HCI_ADV_DIRECT_IND;
			}

			bt_addr_le_copy(&set_param.direct_addr, param->peer);
		} else {
			set_param.type = BT_HCI_ADV_IND;
		}
	} else {
		scannable = sd || (param->options & BT_LE_ADV_OPT_USE_NAME);

		set_param.type = scannable ? BT_HCI_ADV_SCAN_IND :
					     BT_HCI_ADV_NONCONN_IND;
	}

	buf_tmp = bt_hci_cmd_create(BT_HCI_OP_LE_SET_ADV_PARAM, sizeof(set_param));
	if (!buf_tmp) {
		if (buf) {
			net_buf_frag_del_all(buf);
		}
		return -ENOBUFS;
	}

	net_buf_add_mem(buf_tmp, &set_param, sizeof(set_param));

	buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	if (!dir_adv) {
		err = hci_cmd_le_adv_data_create(adv, ad, ad_len, sd, sd_len, scannable,
				    param->options & BT_LE_ADV_OPT_USE_NAME, &buf_tmp);
		if (err) {
			net_buf_frag_del_all(buf);
			return err;
		}
		buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    (param->options & BT_LE_ADV_OPT_CONNECTABLE)) {
		err = le_adv_start_add_conn(adv, &conn);
		if (err) {
			net_buf_frag_del_all(buf);
			return err;
		}
	}

	err = hci_cmd_le_set_adv_enable_create(adv, true, &buf_tmp);
	if (err) {
		BT_ERR("Failed to start advertiser");
		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
			bt_conn_unref(conn);
		}
		net_buf_frag_del_all(buf);
		return err;
	}

	buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
		/* If undirected connectable advertiser we have created a
		 * connection object that we don't yet give to the application.
		 * Since we don't give the application a reference to manage in
		 * this case, we need to release this reference here
		 */
		bt_conn_unref(conn);
	}

	atomic_set_bit_to(adv->flags, BT_ADV_PERSIST, !dir_adv &&
			  !(param->options & BT_LE_ADV_OPT_ONE_TIME));

	atomic_set_bit_to(adv->flags, BT_ADV_INCLUDE_NAME,
			  param->options & BT_LE_ADV_OPT_USE_NAME);

	atomic_set_bit_to(adv->flags, BT_ADV_CONNECTABLE,
			  param->options & BT_LE_ADV_OPT_CONNECTABLE);

	atomic_set_bit_to(adv->flags, BT_ADV_SCANNABLE, scannable);

	atomic_set_bit_to(adv->flags, BT_ADV_USE_IDENTITY,
			  param->options & BT_LE_ADV_OPT_USE_IDENTITY);

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_START));
	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));
	bt_fsm_wait_event(bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SENT));

	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_frag_del_all(buf);
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_START));
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));
		bt_fsm_set_ready(bt_dev.fsm);
		return err;
	}

	return 0;
}

int bt_le_adv_update_data(const struct bt_data *ad, size_t ad_len,
			  const struct bt_data *sd, size_t sd_len)
{

	int err;
	struct net_buf *buf = NULL;

	struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();

	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_DATA_UPDATE))) {
		return -EALREADY;
	}

	bool scannable, use_name;

	scannable = atomic_test_bit(adv->flags, BT_ADV_SCANNABLE);
	use_name = atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME);

	err = hci_cmd_le_adv_data_create(adv, ad, ad_len, sd, sd_len, scannable,
			     use_name, &buf);
	if (err) {
		return err;
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_DATA_UPDATE));
	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_unref(buf);
		return -EIO;
	}
	return 0;
}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
int le_ext_adv_param_set(struct bt_le_ext_adv *adv,
				const struct bt_le_adv_param *param,
				bool  has_scan_data, struct net_buf **ret_buf)
{
	int err;
	bool dir_adv = param->peer != NULL, scannable;
	struct bt_hci_cp_le_set_ext_adv_param *cp;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;
	u8_t         own_addr_type;

	/* set ramdon address may return NULL, means there is no need to set */
	err = hci_cmd_le_adv_set_random_addr_create(adv, param->options, dir_adv,
				     &own_addr_type, &buf_tmp);
	if (err)
	{
		return err;
	}

	if (buf_tmp) {
		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	buf_tmp = bt_hci_cmd_create(BT_HCI_OP_LE_SET_EXT_ADV_PARAM, sizeof(*cp));
	if (!buf_tmp) {
		return -ENOBUFS;
	}

	buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	cp = net_buf_add(buf, sizeof(*cp));
	(void)memset(cp, 0, sizeof(*cp));

	cp->own_addr_type = own_addr_type;

	if (dir_adv) {
		bt_addr_le_copy(&adv->target_addr, param->peer);
	} else {
		bt_addr_le_copy(&adv->target_addr, BT_ADDR_LE_ANY);
	}

	cp->handle = adv->handle;
	sys_put_le24(param->interval_min, cp->prim_min_interval);
	sys_put_le24(param->interval_max, cp->prim_max_interval);
	cp->prim_channel_map  = 0x07;
	cp->filter_policy = get_filter_policy(param->options);
	cp->tx_power = BT_HCI_LE_ADV_TX_POWER_NO_PREF;

	cp->prim_adv_phy = BT_HCI_LE_PHY_1M;
	if (param->options & BT_LE_ADV_OPT_EXT_ADV) {
		if (param->options & BT_LE_ADV_OPT_NO_2M) {
			cp->sec_adv_phy = BT_HCI_LE_PHY_1M;
		} else {
			cp->sec_adv_phy = BT_HCI_LE_PHY_2M;
		}
	}

	if (param->options & BT_LE_ADV_OPT_CODED) {
		cp->prim_adv_phy = BT_HCI_LE_PHY_CODED;
		cp->sec_adv_phy = BT_HCI_LE_PHY_CODED;
	}

	if (!(param->options & BT_LE_ADV_OPT_EXT_ADV)) {
		cp->props |= BT_HCI_LE_ADV_PROP_LEGACY;
	}

	if (param->options & BT_LE_ADV_OPT_USE_TX_POWER) {
		cp->props |= BT_HCI_LE_ADV_PROP_TX_POWER;
	}

	if (param->options & BT_LE_ADV_OPT_ANONYMOUS) {
		cp->props |= BT_HCI_LE_ADV_PROP_ANON;
	}

	if (param->options & BT_LE_ADV_OPT_NOTIFY_SCAN_REQ) {
		cp->scan_req_notify_enable = BT_HCI_LE_ADV_SCAN_REQ_ENABLE;
	}

	if (param->options & BT_LE_ADV_OPT_CONNECTABLE) {
		cp->props |= BT_HCI_LE_ADV_PROP_CONN;
		if (!dir_adv && !(param->options & BT_LE_ADV_OPT_EXT_ADV)) {
			/* When using non-extended adv packets then undirected
			 * advertising has to be scannable as well.
			 * We didn't require this option to be set before, so
			 * it is implicitly set instead in this case.
			 */
			cp->props |= BT_HCI_LE_ADV_PROP_SCAN;
		}
	}

	if ((param->options & BT_LE_ADV_OPT_SCANNABLE) || has_scan_data) {
		cp->props |= BT_HCI_LE_ADV_PROP_SCAN;
	}

	scannable = !!(cp->props & BT_HCI_LE_ADV_PROP_SCAN);

	if (dir_adv) {
		cp->props |= BT_HCI_LE_ADV_PROP_DIRECT;
		if (!(param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY)) {
			cp->props |= BT_HCI_LE_ADV_PROP_HI_DC_CONN;
		}

		bt_addr_le_copy(&cp->peer_addr, param->peer);
	}

    cp->sec_adv_max_skip = param->secondary_max_skip;
	cp->sid = param->sid;
    cp->tx_power = 0;

	atomic_set_bit(adv->flags, BT_ADV_PARAMS_SET);

	if (atomic_test_and_clear_bit(adv->flags, BT_ADV_RANDOM_ADDR_PENDING)) {
		err = hci_cmd_adv_random_address_create(adv, &adv->random_addr.a, &buf_tmp);
		if (err)
		{
			return err;
		}
		if (buf_tmp) {
			buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
		}
	}

	/* Flag only used by bt_le_adv_start API. */
	atomic_set_bit_to(adv->flags, BT_ADV_PERSIST, false);

	atomic_set_bit_to(adv->flags, BT_ADV_INCLUDE_NAME,
			  param->options & BT_LE_ADV_OPT_USE_NAME);

	atomic_set_bit_to(adv->flags, BT_ADV_CONNECTABLE,
			  param->options & BT_LE_ADV_OPT_CONNECTABLE);

	atomic_set_bit_to(adv->flags, BT_ADV_SCANNABLE, scannable);

	atomic_set_bit_to(adv->flags, BT_ADV_USE_IDENTITY,
			  param->options & BT_LE_ADV_OPT_USE_IDENTITY);
	atomic_set_bit_to(adv->flags, BT_ADV_EXT_ADV,
			  param->options & BT_LE_ADV_OPT_EXT_ADV);

	*ret_buf = buf;
	return 0;
}

int bt_le_adv_start_ext(struct bt_le_ext_adv *adv,
			const struct bt_le_adv_param *param,
			const struct bt_data *ad, size_t ad_len,
			const struct bt_data *sd, size_t sd_len)
{
	struct bt_le_ext_adv_start_param start_param = {
		.timeout = 0,
		.num_events = 0,
	};
	bool dir_adv = (param->peer != NULL);
	struct bt_conn *conn = NULL;
	int err;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (!valid_adv_ext_param(param)) {
		return -EINVAL;
	}

	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_START))) {
		return -EALREADY;
	}

	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EALREADY;
	}

	adv->id = param->id;
	err = le_ext_adv_param_set(adv, param, sd ||
				   (param->options & BT_LE_ADV_OPT_USE_NAME), &buf_tmp);
	if (err) {
		return err;
	}

	buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	if (!dir_adv) {
		bool scannable, use_name;
		scannable = atomic_test_bit(adv->flags, BT_ADV_SCANNABLE);
		use_name = atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME);

		err = hci_cmd_le_adv_data_create(adv, ad, ad_len, sd, sd_len, scannable,
				    use_name, &buf_tmp);
		if (err) {
			net_buf_frag_del_all(buf);
			return err;
		}
		buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	} else {
		if (!(param->options & BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY)) {
			start_param.timeout =
				BT_GAP_ADV_HIGH_DUTY_CYCLE_MAX_TIMEOUT;
			atomic_set_bit(adv->flags, BT_ADV_LIMITED);
		}
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    (param->options & BT_LE_ADV_OPT_CONNECTABLE)) {
		err = le_adv_start_add_conn(adv, &conn);
		if (err) {
			if (buf)
			{
				net_buf_frag_del_all(buf);
			}
			return err;
		}
	}

	err = hci_cmd_le_set_adv_enable_ext_create(adv, true, &start_param, &buf_tmp);
	if (err) {
		net_buf_frag_del_all(buf);

		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
			bt_conn_unref(conn);
		}

		return err;
	}

	buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_START));
	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		if (buf)
		{
			net_buf_frag_del_all(buf);
		}
		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
			bt_conn_unref(conn);
		}
		BT_ERR("Failed to start advertiser");
		return -EIO;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
		/* If undirected connectable advertiser we have created a
		 * connection object that we don't yet give to the application.
		 * Since we don't give the application a reference to manage in
		 * this case, we need to release this reference here
		 */
		bt_conn_unref(conn);
	}

	/* Flag always set to false by le_ext_adv_param_set */
	atomic_set_bit_to(adv->flags, BT_ADV_PERSIST, !dir_adv &&
			  !(param->options & BT_LE_ADV_OPT_ONE_TIME));

	return 0;
}

int bt_le_ext_adv_create(const struct bt_le_adv_param *param,
			 const struct bt_le_ext_adv_cb *cb,
			 struct bt_le_ext_adv **out_adv)
{
	struct bt_le_ext_adv *adv;
	int err;
	struct net_buf *buf = NULL;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (!valid_adv_ext_param(param)) {
		return -EINVAL;
	}

	adv = adv_new();
	if (!adv) {
		return -ENOMEM;
	}

	adv->id = param->id;
	adv->cb = cb;

	err = le_ext_adv_param_set(adv, param, false, &buf);
	if (err) {
		adv_delete(adv);
		return err;
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_CREATE));
	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_unref(buf);
		adv_delete(adv);
		BT_ERR("Failed to start advertiser");
		return -EIO;
	}

	*out_adv = adv;
	return 0;
}

int bt_le_ext_adv_update_param(struct bt_le_ext_adv *adv,
			       const struct bt_le_adv_param *param)
{
	int err;
	struct net_buf *buf = NULL;
	if (!valid_adv_ext_param(param)) {
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_BT_PER_ADV) &&
	    atomic_test_bit(adv->flags, BT_PER_ADV_PARAMS_SET)) {
		/* If params for per adv has been set, do not allow setting
		 * connectable, scanable or use legacy adv
		 */
		if (param->options & BT_LE_ADV_OPT_CONNECTABLE ||
		    param->options & BT_LE_ADV_OPT_SCANNABLE ||
		    !(param->options & BT_LE_ADV_OPT_EXT_ADV) ||
		    param->options & BT_LE_ADV_OPT_ANONYMOUS) {
			return -EINVAL;
		}
	}

	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EINVAL;
	}

	if (param->id != adv->id) {
		atomic_clear_bit(adv->flags, BT_ADV_RPA_VALID);
	}

	err = le_ext_adv_param_set(adv, param, false, &buf);
	if (err) {
		return err;
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_PARAM_UPDATE));
	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_unref(buf);
		return -EIO;
	}

	return 0;
}

int bt_le_ext_adv_start(struct bt_le_ext_adv *adv,
			struct bt_le_ext_adv_start_param *param)
{
	struct bt_conn *conn = NULL;
	int err;
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		err = le_adv_start_add_conn(adv, &conn);
		if (err) {
			return err;
		}
	}

	atomic_set_bit_to(adv->flags, BT_ADV_LIMITED, param &&
			  (param->timeout > 0 || param->num_events > 0));

	if (atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		if (IS_ENABLED(CONFIG_BT_PRIVACY) &&
		    !atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
			err = hci_cmd_le_adv_set_private_addr_create(adv, &buf_tmp);
			if (err)
			{
				return err;
			}
		}
	} else {
		if (!atomic_test_bit(adv->flags, BT_ADV_USE_IDENTITY)) {
			err = hci_cmd_le_adv_set_private_addr_create(adv, &buf_tmp);
			if (err)
			{
				return err;
			}
		}
	}

	if (buf_tmp) {
		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	if (atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME) &&
	    !atomic_test_bit(adv->flags, BT_ADV_DATA_SET)) {
		/* Set the advertiser name */
		bool scannable, use_name;

		scannable = atomic_test_bit(adv->flags, BT_ADV_SCANNABLE);
		use_name = atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME);

		err = hci_cmd_le_adv_data_create(adv, NULL, 0, NULL, 0, scannable, use_name, &buf_tmp);
		if (err)
		{
			return err;
		}
		if (buf_tmp) {
			buf = buf? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
		}
	}

	err = hci_cmd_le_set_adv_enable_ext_create(adv, true, param, &buf_tmp);
	if (err) {
		net_buf_frag_del_all(buf);

		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
			bt_conn_unref(conn);
		}

		return err;
	}

	buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_START));
	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		if (buf)
		{
			net_buf_frag_del_all(buf);
		}
		if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
			bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
			bt_conn_unref(conn);
		}
		BT_ERR("Failed to start advertiser");
		return -EIO;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) && conn) {
		/* If undirected connectable advertiser we have created a
		 * connection object that we don't yet give to the application.
		 * Since we don't give the application a reference to manage in
		 * this case, we need to release this reference here
		 */
		bt_conn_unref(conn);
	}

	return 0;
}

int bt_le_ext_adv_stop(struct bt_le_ext_adv *adv)
{
	int err;
	struct net_buf *buf = NULL;

	atomic_clear_bit(adv->flags, BT_ADV_PERSIST);

	if (!atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return 0;
	}

	if (atomic_test_and_clear_bit(adv->flags, BT_ADV_LIMITED)) {
		atomic_clear_bit(adv->flags, BT_ADV_RPA_VALID);

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		pending_id_keys_update();
#endif
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		le_adv_stop_free_conn(adv, 0);
	}

	err = hci_cmd_le_set_adv_enable_ext_create(adv, false, NULL, &buf);
	if (err) {
		return err;
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_STOP));
	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_unref(buf);
		return -EIO;
	}
	return 0;
}

int bt_le_ext_adv_set_data(struct bt_le_ext_adv *adv,
			   const struct bt_data *ad, size_t ad_len,
			   const struct bt_data *sd, size_t sd_len)
{
	int err;
	struct net_buf *buf = NULL;
	bool scannable, use_name;

	scannable = atomic_test_bit(adv->flags, BT_ADV_SCANNABLE);
	use_name = atomic_test_bit(adv->flags, BT_ADV_INCLUDE_NAME);

	err = hci_cmd_le_adv_data_create(adv, ad, ad_len, sd, sd_len, scannable,
			     use_name, &buf);
	if (err) {
		return err;
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_DATA_UPDATE));
	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_unref(buf);
		return -EIO;
	}
	return 0;
}

int bt_le_ext_adv_delete(struct bt_le_ext_adv *adv)
{
	int err;
#if !(defined(CONFIG_BT_USE_HCI_API) && CONFIG_BT_USE_HCI_API)
	struct bt_hci_cp_le_remove_adv_set *cp;
	struct net_buf *buf;
#endif

	if (!BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		return -ENOTSUP;
	}

	/* Advertising set should be stopped first */
	if (atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		return -EINVAL;
	}

	buf = bt_hci_cmd_create(BT_HCI_OP_LE_REMOVE_ADV_SET, sizeof(*cp));
	if (!buf) {
		BT_WARN("No HCI buffers");
		return -ENOBUFS;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = adv->handle;

	err = bt_hci_cmd_send_cb(BT_HCI_OP_LE_REMOVE_ADV_SET, buf, NULL);
	if (err) {
		return err;
	}

	adv_delete(adv);

	return 0;
}

#endif //(defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)


int bt_le_adv_start(const struct bt_le_adv_param *param,
		    const struct bt_data *ad, size_t ad_len,
		    const struct bt_data *sd, size_t sd_len)
{
	return bt_fsm_le_adv_start(param, ad, ad_len, sd, sd_len, NULL, NULL, BT_FSM_HANDLE_UNUSED);
}

int bt_le_adv_stop(void)
{
	return bt_fsm_le_adv_stop(NULL, NULL, BT_FSM_HANDLE_UNUSED);
}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
int bt_le_scan_start(const struct bt_le_scan_param *param, bt_le_scan_cb_t cb)
{
	return bt_fsm_le_scan_start(param, cb, NULL, NULL, BT_FSM_HANDLE_UNUSED);
}

int bt_le_scan_stop(void)
{
	return bt_fsm_le_scan_stop(NULL, NULL, BT_FSM_HANDLE_UNUSED);
}
#endif /* CONFIG_BT_OBSERVER */

struct hci_cmd_fsm_t {
	bt_fsm_handle_t handle;
	struct hci_cmd_fsm_msg_t *cur_msg;
} hci_cmd_fsm = {0};

static int bt_fsm_hci_cmd_action_done(u8_t status, struct hci_cmd_fsm_msg_t *msg)
{
	struct net_buf *rsp_buf = msg->msg;
	(void)rsp_buf;

	if (!bt_fsm_test_state(hci_cmd_fsm.handle, BT_FSM_ST_DEFINE(HCI_CMD, SET_PENDING)))
	{
		BT_ERR("fsm %d state error", hci_cmd_fsm.handle);
		return -1;
	}

	if (status)
	{
		BT_ERR("hci Opcode 0x%04x status %d",cmd(rsp_buf)->opcode, status);
	}

	if (hci_cmd_fsm.cur_msg && hci_cmd_fsm.cur_msg->msg)
	{
		/* release all pending hci cmd buffers */
		net_buf_unref(hci_cmd_fsm.cur_msg->msg);
	}

	if (hci_cmd_fsm.cur_msg)
	{
		if (bt_fsm_get_msg_from(hci_cmd_fsm.cur_msg) != BT_FSM_HANDLE_UNUSED)
		{
			hci_cmd_fsm.cur_msg->status = status;
			bt_fsm_reverse_msg(hci_cmd_fsm.cur_msg);
			bt_fsm_set_event(BT_FSM_EV_DEFINE(HCI_CMD, CMD_SENT), hci_cmd_fsm.cur_msg);
		}
		else
		{
			bt_fsm_free_msg(hci_cmd_fsm.cur_msg);
			hci_cmd_fsm.cur_msg = NULL;
		}
	}

	bt_fsm_clear_state(hci_cmd_fsm.handle, BT_FSM_ST_DEFINE(HCI_CMD, SET_PENDING));

	return 0;
}

static int bt_fsm_action_hci_cmd_complete(void *msg)
{
	struct hci_cmd_fsm_msg_t *hci_cmd_msg = NULL;
	int ret;

	struct net_buf *rsp_buf = NULL;

	if (!bt_fsm_test_state(hci_cmd_fsm.handle, BT_FSM_ST_DEFINE(HCI_CMD, SET_PENDING)))
	{
		BT_ERR("fsm %d state error", hci_cmd_fsm.handle);
		return -1;
	}

	hci_cmd_msg = msg;
	rsp_buf = hci_cmd_msg->msg;

	u8_t status = cmd(rsp_buf)->status;

	net_buf_unref(rsp_buf);

	if (status)
	{
		goto done;
	}

	struct net_buf *buf = hci_cmd_fsm.cur_msg->msg;

	if (buf)
	{
		/* avoid destroy when call  net_buf_frag_del*/
		net_buf_ref(buf);

		/* remove from pending buffer list */
		hci_cmd_fsm.cur_msg->msg = net_buf_frag_del(NULL, buf);
		cmd(buf)->fsm_handle = hci_cmd_fsm.handle;
		ret =  bt_hci_cmd_send_cb(cmd(buf)->opcode, buf, NULL);
		if (ret)
		{
			return ret;
		}

		ret = bt_fsm_wait_event(hci_cmd_fsm.handle, BT_FSM_EV_DEFINE(HCI_CMD, CMD_CMPLETE));
		if (ret)
		{
			BT_ERR("wait event error %d", ret);
			return ret;
		}

		return ret;
	}

done:
	return bt_fsm_hci_cmd_action_done(status, msg);
}

static int bt_fsm_action_hci_cmd_send(void *msg)
{
	struct hci_cmd_fsm_msg_t *hci_cmd_msg = NULL;
	int ret = 0;

	struct net_buf *buf = NULL;
	struct net_buf *pending_buf = NULL;

	if (bt_fsm_test_state(hci_cmd_fsm.handle, BT_FSM_ST_DEFINE(HCI_CMD, SET_PENDING)))
	{
		return FSM_RET_MSG_RETAIN;
	}

	/* skip fsm msg head */
	hci_cmd_msg = msg;
	buf = hci_cmd_msg->msg;

	/* avoid destroy when call net_buf_frag_del*/
	net_buf_ref(buf);

	/* remove from pending buffer list */
	pending_buf = net_buf_frag_del(NULL, buf);

	bt_fsm_set_state(hci_cmd_fsm.handle, BT_FSM_ST_DEFINE(HCI_CMD, SET_PENDING));

	cmd(buf)->fsm_handle = hci_cmd_fsm.handle;

	ret = bt_hci_cmd_send_cb(cmd(buf)->opcode, buf, NULL);
	if (ret)
	{
		bt_fsm_clear_state(hci_cmd_fsm.handle, BT_FSM_ST_DEFINE(HCI_CMD, SET_PENDING));
		return ret;
	}

	/* update next hci cmd buffer */
	hci_cmd_msg->msg = pending_buf;

    /* all hci cmd msg will release after recv hci cmd complete event */
	hci_cmd_fsm.cur_msg = hci_cmd_msg;

	ret = bt_fsm_wait_event(hci_cmd_fsm.handle, BT_FSM_EV_DEFINE(HCI_CMD, CMD_CMPLETE));
	if (ret)
	{
		BT_ERR("wait event error %d", ret);
		return ret;
	}

	return FSM_RET_MSG_NOFREE;
}

BT_FSM_HANDLER_DEVINE(HCI_CMD,
	BT_FSM_ACTION_DEFINE(HCI_CMD, CMD_SEND,    bt_fsm_action_hci_cmd_send);
	BT_FSM_ACTION_DEFINE(HCI_CMD, CMD_CMPLETE, bt_fsm_action_hci_cmd_complete);
)

int hci_cmd_fsm_init()
{
	hci_cmd_fsm.handle = bt_fsm_init(BT_FSM_HANDLER_FUNC_NAME(HCI_CMD));
	if (hci_cmd_fsm.handle > 0)
	{
		return 0;
	}

	return -1;
}

int bt_fsm_hci_cmd_send(void *msg, void *args, bt_fsm_handle_t src_handle, bt_fsm_ev_en ev)
{
	struct hci_cmd_fsm_msg_t *hci_cmd_msg = bt_fsm_create_msg(sizeof(struct hci_cmd_fsm_msg_t), hci_cmd_fsm.handle, src_handle);
	if (!hci_cmd_msg)
	{
		return -ENOMEM;
	}

	if (!msg)
	{
		return -EINVAL;
	}

	hci_cmd_msg->msg = msg;
	hci_cmd_msg->args = args;

	return bt_fsm_set_event(ev, hci_cmd_msg);
}

static struct hci_core_fsm_t {
	bt_fsm_handle_t handle;
	struct hci_core_fsm_msg_t *cur_msg;
} hci_core_fsm = {0};

static int bt_fsm_action_hci_core_hci_cmd_sent(void *msg)
{
	struct hci_cmd_fsm_msg_t *hci_cmd_msg = msg;
	int generate_event = BT_FSM_EV_NONE;

	BT_DBG("hci process complelte");

	if (hci_cmd_msg->status)
	{
		BT_ERR("hci_cmd_msg->status error %d", hci_cmd_msg->status);
	}

	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_START)))
	{
		BT_DBG("ADV started %d", hci_cmd_msg->status);
		generate_event = BT_FSM_EV_DEFINE(HCI_CORE, ADV_STARTED);

		if (hci_cmd_msg->status)
		{
			adv_delete_legacy();

			struct bt_conn * conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL,
					       BT_CONN_CONNECT_ADV);
			if (conn)
			{
				conn->err = 0;
				bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
				bt_conn_unref(conn);
			}
		}
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_START));
		ble_event_adv_start(hci_cmd_msg->status);
	}
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_STOP)))
	{
		BT_DBG("ADV stop");
		generate_event = BT_FSM_EV_DEFINE(HCI_CORE, ADV_STOPED);
		adv_delete_legacy();
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_STOP));
		ble_event_adv_stop(hci_cmd_msg->status);
	}
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_DATA_UPDATE)))
	{
		BT_DBG("ADV data update");
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_DATA_UPDATE));
	}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_START)))
	{
		BT_DBG("scan started");
		generate_event = BT_FSM_EV_DEFINE(HCI_CORE, SCAN_STARTED);
		if (hci_cmd_msg->status)
		{
			atomic_clear_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN);
		}
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_START));
		ble_event_scan_start(hci_cmd_msg->status);
	}
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_STOP)))
	{
		BT_DBG("scan stop");
		generate_event = BT_FSM_EV_DEFINE(HCI_CORE, SCAN_STOPED);
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_STOP));
		ble_event_scan_stop(hci_cmd_msg->status);
	}

	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_UPDATE)))
	{
		BT_DBG("scan update");
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_UPDATE));
	}
#endif

#if (defined(CONFIG_BT_CENTRAL) && CONFIG_BT_CENTRAL)
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CONN_CREATE)))
	{
		BT_DBG("connect create");
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CONN_CREATE));

		if (hci_cmd_msg->status)
		{
			struct bt_conn * conn = bt_conn_lookup_state_le(BT_ID_DEFAULT, NULL,
					       BT_CONN_CONNECT);
			if (conn)
			{
				conn->err = hci_cmd_msg->status;
				bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
				bt_conn_unref(conn);
				/*conn unref, this is ref when create connection*/
				bt_conn_unref(conn);
				bt_fsm_le_scan_update(false, bt_dev.fsm);
			}
		}
	}
#endif

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_CREATE)))
	{
		BT_DBG("ext adv create");
		if (hci_cmd_msg->status)
		{
			if (hci_cmd_msg->args)
			{
				adv_delete((struct bt_le_ext_adv *)hci_cmd_msg->args);
			}
		}
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_CREATE));
	}
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_PARAM_UPDATE)))
	{
		BT_DBG("ext adv param update");
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_PARAM_UPDATE));
	}
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_START)))
	{
		BT_DBG("ext adv started");
		if (hci_cmd_msg->status)
		{
			if (IS_ENABLED(CONFIG_BT_PERIPHERAL)) {
				u8_t id = (struct bt_le_ext_adv *)hci_cmd_msg->args ? ((struct bt_le_ext_adv *)(hci_cmd_msg->args))->id : BT_ID_DEFAULT;
				struct bt_conn * conn = bt_conn_lookup_state_le(id, NULL,
							BT_CONN_CONNECT_ADV);
				if (conn)
				{
					bt_conn_set_state(conn, BT_CONN_DISCONNECTED);
					bt_conn_unref(conn);
				}
			}
		}

		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_START));
	}
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_STOP)))
	{
		BT_DBG("ext adv stop");
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_STOP));
	}
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_DATA_UPDATE)))
	{
		BT_DBG("ext adv data update");
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_ADV_DATA_UPDATE));
	}
	else
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_CONN_CREATE)))
	{
		BT_DBG("ext conn create");
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, EXT_CONN_CREATE));
	}
#endif

	if (hci_core_fsm.cur_msg)
	{
		if (bt_fsm_get_msg_from(hci_core_fsm.cur_msg) != BT_FSM_HANDLE_UNUSED && generate_event)
		{
			hci_core_fsm.cur_msg->status = hci_cmd_msg->status;
			bt_fsm_reverse_msg(hci_core_fsm.cur_msg);
			bt_fsm_set_event(generate_event, hci_core_fsm.cur_msg);
		}
		else
		{
			bt_fsm_free_msg(hci_core_fsm.cur_msg);
		}
		hci_core_fsm.cur_msg = NULL;
	}

	bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));

	return 0;
}

BT_FSM_HANDLER_DEVINE(HCI_CORE,
	BT_FSM_ACTION_DEFINE(HCI_CMD, CMD_SENT, bt_fsm_action_hci_core_hci_cmd_sent);
)

int bt_fsm_hci_core_init()
{
	int err = bt_fsm_init(BT_FSM_HANDLER_FUNC_NAME(HCI_CORE));
	if (err >= 0)
	{
		hci_core_fsm.handle = err;
	}

	return err;
}

int bt_fsm_le_adv_start(const struct bt_le_adv_param *param,
		    const struct bt_data *ad, size_t ad_len,
		    const struct bt_data *sd, size_t sd_len,
			void *msg, void *args, bt_fsm_handle_t src_handle)
{
	int err;

	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING))) {
		return -EBUSY;
	}

	if (src_handle != BT_FSM_HANDLE_UNUSED)
	{
		struct hci_core_fsm_msg_t *hci_core_msg = bt_fsm_create_msg(sizeof(struct hci_core_fsm_msg_t), hci_core_fsm.handle, src_handle);
		if (!hci_core_msg)
		{
			return -ENOMEM;
		}

		hci_core_msg->msg = msg;
		hci_core_msg->args = args;
		hci_core_fsm.cur_msg = hci_core_msg;
	}

#if (defined(CONFIG_BT_EXT_ADV) && CONFIG_BT_EXT_ADV)
	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		struct bt_le_ext_adv *adv = adv_new_legacy();

		if (!adv) {
			err = -ENOMEM;
			return err;
		}

		err = bt_le_adv_start_ext(adv, param, ad, ad_len, sd, sd_len);
		if (err) {
			adv_delete_legacy();
		}
		goto done;
	}
#endif

	err = bt_le_adv_start_legacy(param, ad, ad_len, sd, sd_len);
	if (err)
	{
		goto done;
	}

done:
	if (err)
	{
		if (hci_core_fsm.cur_msg)
		{
			hci_core_fsm.cur_msg->status = err;
			bt_fsm_reverse_msg(hci_core_fsm.cur_msg);
			bt_fsm_set_event(BT_FSM_EV_DEFINE(HCI_CORE, ADV_STARTED), hci_core_fsm.cur_msg);
			hci_core_fsm.cur_msg = NULL;
		}
	}

	return err;
}

static int _bt_fsm_le_adv_stop()
{
	struct bt_le_ext_adv *adv = bt_adv_lookup_legacy();
	struct net_buf *buf;
	int err;

	if (!adv) {
		return -EALREADY;
	}

	/* Make sure advertising is not re-enabled later even if it's not
	 * currently enabled (i.e. BT_DEV_ADVERTISING is not set).
	 */
	atomic_clear_bit(adv->flags, BT_ADV_PERSIST);

	if (!atomic_test_bit(adv->flags, BT_ADV_ENABLED)) {
		/* Legacy advertiser exists, but is not currently advertising.
		 * This happens when keep advertising behavior is active but
		 * no conn object is available to do connectable advertising.
		 */
		adv_delete_legacy();

		if (hci_core_fsm.cur_msg)
		{
			bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_STOP));
			hci_core_fsm.cur_msg->status = 0;
			bt_fsm_reverse_msg(hci_core_fsm.cur_msg);
			bt_fsm_set_event(BT_FSM_EV_DEFINE(HCI_CORE, ADV_STOPED), hci_core_fsm.cur_msg);
			hci_core_fsm.cur_msg = NULL;
		}

		return 0;
	}

	if (IS_ENABLED(CONFIG_BT_PERIPHERAL) &&
	    atomic_test_bit(adv->flags, BT_ADV_CONNECTABLE)) {
		le_adv_stop_free_conn(adv, 0);
	}

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		err = hci_cmd_le_set_adv_enable_ext_create(adv, false, NULL, &buf);
		if (err) {
			return err;
		}
	} else {
		err = hci_cmd_le_set_adv_enable_legacy_create(adv, false, &buf);
		if (err) {
			return err;
		}
	}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
	if (!(IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	      BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) &&
	    !IS_ENABLED(CONFIG_BT_PRIVACY) &&
	    !IS_ENABLED(CONFIG_BT_SCAN_WITH_IDENTITY)) {
		/* If scan is ongoing set back NRPA */
		if (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
			struct net_buf *buf_tmp;

			err = hci_cmd_set_le_scan_enable_create(BT_HCI_LE_SCAN_DISABLE, &buf_tmp);
			if (err) {
				net_buf_unref(buf);
				return err;
			}

			net_buf_frag_add(buf, buf_tmp);

			err = hci_cmd_le_set_private_addr_create(BT_ID_DEFAULT, &buf_tmp);
			if (err)
			{
				net_buf_frag_del_all(buf);
				return err;
			}
			if (buf_tmp) {
				buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
			}

			err = hci_cmd_set_le_scan_enable_create(BT_HCI_LE_SCAN_ENABLE, &buf_tmp);
			if (err) {
				net_buf_frag_del_all(buf);
				return err;
			}

			net_buf_frag_add(buf, buf_tmp);
		}
	}
#endif /* (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER) */

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));
	bt_fsm_wait_event(bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SENT));

	err = bt_fsm_hci_cmd_send(buf, adv, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_frag_del_all(buf);
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));
		bt_fsm_set_ready(bt_dev.fsm);
		return err;
	}

	return 0;
}

int bt_fsm_le_adv_stop(void *msg, void *args, bt_fsm_handle_t src_handle)
{
	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING))) {
		return -EBUSY;
	}

	if (src_handle != BT_FSM_HANDLE_UNUSED)
	{
		struct hci_core_fsm_msg_t *hci_core_msg = bt_fsm_create_msg(sizeof(struct hci_core_fsm_msg_t), hci_core_fsm.handle, src_handle);
		if (!hci_core_msg)
		{
			return -ENOMEM;
		}

		hci_core_msg->msg = msg;
		hci_core_msg->args = args;
		hci_core_fsm.cur_msg = hci_core_msg;
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_STOP));

	int err = _bt_fsm_le_adv_stop();
	if (err)
	{
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, ADV_STOP));
		if (hci_core_fsm.cur_msg)
		{
			if (err == -EALREADY)
			{
				err = 0;
			}
			hci_core_fsm.cur_msg->status = err;
			bt_fsm_reverse_msg(hci_core_fsm.cur_msg);
			bt_fsm_set_event(BT_FSM_EV_DEFINE(HCI_CORE, ADV_STOPED), hci_core_fsm.cur_msg);
			hci_core_fsm.cur_msg = NULL;
		}
	}

	return err;
}

#if (defined(CONFIG_BT_OBSERVER) && CONFIG_BT_OBSERVER)
static inline int _bt_fsm_le_scan_start(const struct bt_le_scan_param *param)
{
	struct net_buf *buf = NULL;
	struct net_buf *buf_tmp = NULL;
	int err;

	if (!atomic_test_bit(bt_dev.flags, BT_DEV_READY)) {
		return -EAGAIN;
	}

	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING)))
	{
		return -EBUSY;
	}

	/* Check that the parameters have valid values */
	if (!valid_le_scan_param(param)) {
		return -EINVAL;
	}

	if (param->type && !bt_le_scan_random_addr_check()) {
		return -EINVAL;
	}

	/* Return if active scan is already enabled */
	if (atomic_test_and_set_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN)) {
		return -EALREADY;
	}

	if (atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
		err = hci_cmd_set_le_scan_enable_create(BT_HCI_LE_SCAN_DISABLE, &buf_tmp);
		if (err) {
			atomic_clear_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN);
			return err;
		}
		//cmd(buf_tmp)->fsm = &bt_dev.fsm;
		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	atomic_set_bit_to(bt_dev.flags, BT_DEV_SCAN_FILTER_DUP,
			  param->options & BT_LE_SCAN_OPT_FILTER_DUPLICATE);

#if (defined(CONFIG_BT_WHITELIST) && CONFIG_BT_WHITELIST)
	atomic_set_bit_to(bt_dev.flags, BT_DEV_SCAN_WL,
			  param->options & BT_LE_SCAN_OPT_FILTER_WHITELIST);
#endif /* (defined(CONFIG_BT_WHITELIST) && CONFIG_BT_WHITELIST) */

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    BT_FEAT_LE_EXT_ADV(bt_dev.le.features)) {
		struct bt_hci_ext_scan_phy param_1m;
		struct bt_hci_ext_scan_phy param_coded;

		struct bt_hci_ext_scan_phy *phy_1m = NULL;
		struct bt_hci_ext_scan_phy *phy_coded = NULL;

		if (!(param->options & BT_LE_SCAN_OPT_NO_1M)) {
			param_1m.type = param->type;
			param_1m.interval = sys_cpu_to_le16(param->interval);
			param_1m.window = sys_cpu_to_le16(param->window);

			phy_1m = &param_1m;
		}

		if (param->options & BT_LE_SCAN_OPT_CODED) {
			u16_t interval = param->interval_coded ?
				param->interval_coded :
				param->interval;
			u16_t window = param->window_coded ?
				param->window_coded :
				param->window;

			param_coded.type = param->type;
			param_coded.interval = sys_cpu_to_le16(interval);
			param_coded.window = sys_cpu_to_le16(window);
			phy_coded = &param_coded;
		}

		err = hci_cmd_start_le_scan_ext_create(phy_1m, phy_coded, param->timeout, &buf_tmp);
		if (err)
		{
			net_buf_frag_del_all(buf);
			return err;
		}

		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;

	} else {
		if (param->timeout) {
			return -ENOTSUP;
		}

		err = hci_cmd_le_scan_legacy_create(param->type, param->interval,
					   param->window, &buf_tmp);
		if (err)
		{
			net_buf_frag_del_all(buf);
			return err;
		}

		buf = buf ? net_buf_frag_add(buf, buf_tmp) : buf_tmp;
	}

	if (!buf) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN);
		return -ENOBUFS;
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_START));
	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));

	bt_fsm_wait_event(bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SENT));

	err = bt_fsm_hci_cmd_send(buf, NULL, bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SEND));
	if (err)
	{
		net_buf_frag_del_all(buf);
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_START));
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));
		bt_fsm_set_ready(bt_dev.fsm);

		return err;
	}

	return 0;
}
int bt_fsm_le_scan_start(const struct bt_le_scan_param *param, bt_le_scan_cb_t cb, void *msg, void *args, bt_fsm_handle_t src_handle)
{
	int err;

	if (src_handle != BT_FSM_HANDLE_UNUSED)
	{
		struct hci_core_fsm_msg_t *hci_core_msg = bt_fsm_create_msg(sizeof(struct hci_core_fsm_msg_t), hci_core_fsm.handle, src_handle);
		if (!hci_core_msg)
		{
			return -ENOMEM;
		}

		hci_core_msg->msg = msg;
		hci_core_msg->args = args;
		hci_core_fsm.cur_msg = hci_core_msg;
	}

	scan_dev_found_cb = cb;

	err = _bt_fsm_le_scan_start(param);
	if (err)
	{
		scan_dev_found_cb = NULL;
		if (hci_core_fsm.cur_msg)
		{
			hci_core_fsm.cur_msg->status = err;
			bt_fsm_reverse_msg(hci_core_fsm.cur_msg);
			bt_fsm_set_event(BT_FSM_EV_DEFINE(HCI_CORE, SCAN_STARTED), hci_core_fsm.cur_msg);
			hci_core_fsm.cur_msg = NULL;
		}
	}

	return err;
}

int bt_fsm_le_scan_stop(void *msg, void *args, bt_fsm_handle_t src_handle)
{
	int err;

	if (bt_fsm_test_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING)))
	{
		return -EBUSY;
	}

	if (src_handle != BT_FSM_HANDLE_UNUSED)
	{
		struct hci_core_fsm_msg_t *hci_core_msg = bt_fsm_create_msg(sizeof(struct hci_core_fsm_msg_t), hci_core_fsm.handle, src_handle);
		if (!hci_core_msg)
		{
			return -ENOMEM;
		}

		hci_core_msg->msg = msg;
		hci_core_msg->args = args;

		hci_core_fsm.cur_msg = hci_core_msg;
	}

	/* Return if active scanning is already disabled */
	if (!atomic_test_and_clear_bit(bt_dev.flags, BT_DEV_EXPLICIT_SCAN)) {
		err = -EALREADY;
		goto fail;
	}

	scan_dev_found_cb = NULL;

	if (IS_ENABLED(CONFIG_BT_EXT_ADV) &&
	    atomic_test_and_clear_bit(bt_dev.flags, BT_DEV_SCAN_LIMITED)) {
		atomic_clear_bit(bt_dev.flags, BT_DEV_RPA_VALID);

#if (defined(CONFIG_BT_SMP) && CONFIG_BT_SMP)
		pending_id_keys_update();
#endif
	}

	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_STOP));
	bt_fsm_set_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));
	bt_fsm_wait_event(bt_dev.fsm, BT_FSM_EV_DEFINE(HCI_CMD, CMD_SENT));

	err = bt_fsm_le_scan_update(false, bt_dev.fsm);
	if (err)
	{
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, SCAN_STOP));
		bt_fsm_clear_state(bt_dev.fsm, BT_FSM_ST_DEFINE(HCI_CORE, CFG_PENDING));
		bt_fsm_set_ready(bt_dev.fsm);
		goto fail;
	}

	return 0;

fail:
	if (err && err != -EALREADY)
	{
		if (hci_core_fsm.cur_msg)
		{
			hci_core_fsm.cur_msg->status = err;
			bt_fsm_reverse_msg(hci_core_fsm.cur_msg);
			bt_fsm_set_event(BT_FSM_EV_DEFINE(HCI_CORE, SCAN_STOPED), hci_core_fsm.cur_msg);
			hci_core_fsm.cur_msg = NULL;
		}
	}

	return err;
}
#endif /* CONFIG_BT_OBSERVER */

#endif
