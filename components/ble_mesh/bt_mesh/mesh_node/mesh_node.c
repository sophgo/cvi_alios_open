/*
 * Copyright (C) 2019-2022 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>

#include <api/mesh.h>
#include <aos/debug.h>
#include <aos/kernel.h>
#include <aos/kv.h>
#include <aos/ble.h>
#include <errno.h>
#include <mesh.h>
#include "mesh_node.h"
#include "mesh_model/mesh_model.h"
#include "bluetooth/addr.h"
#include "bluetooth/conn.h"
#include "host/conn_internal.h"
#include "inc/net.h"
#include "inc/proxy.h"
#include "vendor/vendor_model_srv.h"

#ifdef CONFIG_BT_MESH_PROVISIONER
#include "provisioner_prov.h"
#include "provisioner_main.h"
#include "provisioner_proxy.h"
#endif

#ifdef CONFIG_OTA_CLIENT
#include "ota_module.h"
#endif

#define COMMON_SUB_ADDR        0xF000
#define COMMON_PUB_ADDR        0xF001
#define COMMON_HB_SUB_PUB_ADDR 0xF002
#ifndef CONFIG_BT_MESH_NODE_VERSION_GET_TTL
#define MESH_NODE_VERSION_GET_TTL (6)
#else
#define MESH_NODE_VERSION_GET_TTL CONFIG_BT_MESH_NODE_VERSION_GET_TTL
#endif

#define TIMEOUT 402

static uint8_t dev_uuid[16];
static uint8_t g_static_data[16] = { 0 };

#define TAG "BT_MESH_NODE"

#ifdef BT_MESH_BEACON_RSSI_REPORT
typedef struct {
    uint8_t mac[6];
    uint8_t rssi;
    uint8_t rssi_calibration;
} beacon_rssi_data;

typedef struct {
    uint8_t          front;
    uint8_t          rear;
    uint8_t          length;
    beacon_rssi_data data[16];
} rssi_data_queue;
#endif
typedef void (*node_event_cb)(uint16_t event, void *p_arg);

typedef struct _mesh_node {
    const struct bt_mesh_comp *comp;
    model_event_cb             user_model_cb;
    uint8_t                    init_flag;
    prov_event_cb              user_prov_cb;
    uint8_t                    app_key_add_flag : 1;
    k_timer_t                  auto_config_timer;
    uint8_t                    auto_config_status;
    uint32_t                   auto_config_timeout;
    uint8_t                    auto_config_retry;
    uint8_t                    auto_config_ack_flag;
    uint16_t                   prov_addr;
    bt_mesh_input_action_t     input_act;
    uint8_t                    input_size;
    health_faults_t            test_faults[CONFIG_BT_MESH_CUR_TEST_ID_MAX];
#ifdef CONFIG_BT_MESH_PROVISIONER
    uint8_t              prov_init_flag   : 1;
    uint8_t              prov_config_flag : 1;
    uint8_t              prov_enable_flag : 1;
    provisioner_cb       user_provisioner_cb;
    k_timer_t            prov_add_appkey_timer;
    uint32_t             prov_add_appkey_timeout;
    uint8_t              prov_app_key_add_retry;
    uint16_t             prov_auto_add_appkey_idx;
    k_timer_t            prov_auto_config_timer;
    uint16_t             prov_auto_config_addr;
    uint32_t             prov_auto_config_timeout;
    uint32_t             prov_auto_config_retry;
    model_auto_config_t  prov_auto_config;
    provisioner_config_t prov_config;
#endif

#ifdef BT_MESH_BEACON_RSSI_REPORT
    uint8_t         rssi_report_enable : 1;
    k_timer_t       beacon_rssi_timer;
    rssi_data_queue rssi_queue;
#endif
    uint8_t       node_provisioned;
    health_srv_cb health_cb;
    node_at_cb    at_cb;
} mesh_node;

mesh_node g_mesh_node;

#ifdef BT_MESH_BEACON_RSSI_REPORT
static int init_rssi_report();
#endif

extern void                   bt_mesh_store_mod_bind(struct bt_mesh_model *mod);
extern void                   bt_mesh_store_mod_sub(struct bt_mesh_model *mod);
extern void                   bt_mesh_store_mod_pub(struct bt_mesh_model *mod);
extern u16_t *                bt_mesh_model_find_group(struct bt_mesh_model *mod, u16_t addr);
extern int                    bt_mesh_store_mod_pending_check();
extern u8_t                   bt_mesh_default_ttl_get(void);
extern struct bt_mesh_hb_pub *bt_mesh_hb_pub_get(void);
extern struct bt_mesh_hb_sub *bt_mesh_hb_sub_get(void);
extern int                    ble_mesh_cfg_srv_hb_pub_ttl_set(u8_t ttl);

static int report_auto_config_status(uint16_t dst_addr, uint8_t status)
{
#ifdef CONFIG_BT_MESH_MODEL_VENDOR_SRV
    struct bt_mesh_model *vnd_model = ble_mesh_model_find(0, BT_MESH_MODEL_VND_MODEL_SRV, CONFIG_CID_TAOBAO);

    if (!vnd_model) {
        return -1;
    }

    vnd_model_msg vnd_msg = { 0 };
    vnd_msg.netkey_idx    = 0;
    vnd_msg.appkey_idx    = 0;
    vnd_msg.dst_addr      = dst_addr;
    vnd_msg.model         = vnd_model;
    vnd_msg.opid          = VENDOR_OP_ATTR_MESH_AUTOCONFIG_STATUS;
    vnd_msg.len           = 1;
    vnd_msg.data          = (uint8_t *)&status;
    return ble_mesh_vendor_srv_model_msg_send(&vnd_msg);
#else
    return 0;
#endif
}
static void auto_config_timeout_cb(void *timer, void *arg)
{
    int16_t ret;

    k_timer_stop(&g_mesh_node.auto_config_timer);

    if (!bt_mesh_store_mod_pending_check()) {
        g_mesh_node.auto_config_status = BT_MESH_AUTO_CONFIG_SUCCEED;
        g_mesh_node.node_provisioned   = 1;
        if (g_mesh_node.auto_config_ack_flag) {
            ret = report_auto_config_status(g_mesh_node.prov_addr, g_mesh_node.auto_config_status);

            if (ret) {
                LOGE(TAG, "report auto config status failed");
            }
        }

        ret = aos_kv_setint("AUTO_CONFIG", BT_MESH_AUTO_CONFIG_SUCCEED);

        if (ret) {
            LOGE(TAG, "save auto config status failed");
        }

        if (g_mesh_node.user_prov_cb) {
            g_mesh_node.user_prov_cb(BT_MESH_EVENT_NODE_AUTOCONFIG_SUCCESS, NULL);
        }

#ifdef BT_MESH_BEACON_RSSI_REPORT
        if (g_mesh_node.rssi_report_enable) {
            init_rssi_report();
        }
#endif

#if defined(CONFIG_BT_MESH_HB_AUTO_PUB) && CONFIG_BT_MESH_HB_AUTO_PUB > 0
        ret = bt_mesh_mod_hb_pub_set(
            CONFIG_BT_MESH_AUTOCONFIG_HB_PUB_NET_IDX, CONFIG_BT_MESH_AUTOCONFIG_HB_PUB_STATUS_REPORT_ADDR,
            CONFIG_BT_MESH_AUTOCONFIG_HB_PUB_DST_ADDR, CONFIG_BT_MESH_AUTOCONFIG_HB_PUB_COUNT_LOG,
            CONFIG_BT_MESH_AUTOCONFIG_HB_PUB_PERIOD_LOG, CONFIG_BT_MESH_AUTOCONFIG_HB_PUB_TTL,
            CONFIG_BT_MESH_AUTOCONFIG_HB_PUB_FEAD);
        if (ret) {
            LOGE(TAG, "hb upper pub set failed %d", ret);
        }
#endif
    } else {
        LOGI(TAG, "store is pending");

        if (g_mesh_node.auto_config_retry) {
            k_timer_start(&g_mesh_node.auto_config_timer, g_mesh_node.auto_config_timeout);
            g_mesh_node.auto_config_retry--;
        }
    }
}

#ifdef CONFIG_BT_MESH_PROVISIONER

static uint16_t last_addr;

static struct bt_mesh_prov node_prov = {
    .uuid           = dev_uuid,
    .link_open      = NULL,
    .link_close     = NULL,
    .complete       = NULL,
    .reset          = NULL,
    .static_val     = g_static_data,
    .static_val_len = 0,
    .input_size     = 0,
    .input_actions  = 0,
    .input          = NULL,
    .output_size    = 0,
    .output_actions = 0,
    .output_number  = NULL,
    .output_string  = NULL,
};
#else
static void link_open(bt_mesh_prov_bearer_t bearer);
static void link_close(bt_mesh_prov_bearer_t bearer);
static void prov_complete(uint16_t net_idx, uint16_t addr);
static void prov_reset(void);
static int output_number(bt_mesh_output_action_t action, u32_t number);
static int output_string(const char *str);
static int node_input(bt_mesh_input_action_t act, u8_t size);

static struct bt_mesh_prov node_prov = {
    .uuid = dev_uuid,
    .link_open = link_open,
    .link_close = link_close,
    .complete = prov_complete,
    .reset = prov_reset,
    .static_val = g_static_data,
    .static_val_len = 0,
    .input_size = 0,
    .input_actions = 0,
    .input = node_input,
    .output_size = 0,
    .output_actions = 0,
    .output_number = output_number,
    .output_string = output_string,
};
#endif

typedef struct auto_config_t {
    uint16_t mod_id;
    uint16_t cid;
    uint16_t appkey_idx;
    uint16_t sub_addr;
    uint16_t pub_addr;
} auto_config_t;

static void node_config(struct bt_mesh_model *mod, struct bt_mesh_elem *elem, bool vnd, bool primary, void *user_data)
{
    auto_config_t *config = user_data;

    if (!config) {
        return;
    }

    if (!vnd && mod->id == BT_MESH_MODEL_ID_CFG_SRV) {
        return;
    }

    if (!vnd && mod->id == BT_MESH_MODEL_ID_CFG_CLI) { // for cfg cli should add sub for getting rst status
        bt_mesh_mod_sub_add(mod, config->sub_addr);
        return;
    }

    if (!vnd) {
        if (config->mod_id != 0xffff && config->mod_id != mod->id) {
            return;
        }
    } else {
        if ((config->mod_id != 0xffff && config->cid != 0xffff)
            && (config->mod_id != mod->vnd.id || config->cid != mod->vnd.company))
        {
            return;
        }
    }

    bt_mesh_mod_bind(mod, config->appkey_idx);

    bt_mesh_mod_sub_add(mod, config->sub_addr);

    bt_mesh_mod_pub_set(mod, config->pub_addr);
}

static int16_t auto_config(auto_config_t *config)
{
    if (!config) {
        return -EINVAL;
    }

    LOGD(TAG, "bind appkey:%04x sub addr:%04x pub addr:%04x", config->appkey_idx, config->sub_addr, config->pub_addr);
    extern void bt_mesh_model_foreach(
        void (*func)(struct bt_mesh_model * mod, struct bt_mesh_elem * elem, bool vnd, bool primary, void *user_data),
        void *user_data);
    bt_mesh_model_foreach(node_config, config);

    return 0;
}

#ifdef CONFIG_BT_MESH_PROVISIONER
static int _ble_mesh_node_model_autoconfig(uint16_t netkey_idx, uint16_t appkey_idx, uint16_t addr,
                                           model_auto_config_t auto_conf)
{
#ifdef CONFIG_BT_MESH_MODEL_VENDOR_CLI
    struct bt_mesh_model *vendor_model;
    vnd_model_msg         vnd_msg = { 0 };
    int16_t               ret;
    uint16_t              send_data;
    vendor_model = ble_mesh_model_find(0, BT_MESH_MODEL_VND_MODEL_CLI, BT_MESH_MODEL_VND_COMPANY_ID);

    if (!vendor_model) {
        return -ENOTSUP;
    }

    if (!BT_MESH_ADDR_IS_GROUP(auto_conf.sub_addr)) {
        return -EINVAL;
    }

    send_data          = auto_conf.sub_addr;
    vnd_msg.netkey_idx = netkey_idx;
    vnd_msg.appkey_idx = appkey_idx;
    vnd_msg.dst_addr   = addr;
    vnd_msg.model      = vendor_model;
    vnd_msg.len        = 2;
    vnd_msg.opid       = VENDOR_OP_ATTR_MESH_AUTOCONFIG;
    vnd_msg.data       = (uint8_t *)&send_data;

    ret = ble_mesh_vendor_cli_model_msg_send(&vnd_msg);

    if (ret) {
        return -1;
    }
    g_mesh_node.prov_auto_config_addr     = addr;
    g_mesh_node.prov_auto_config.sub_addr = auto_conf.sub_addr;

    return 0;
#endif
    return -ENOTSUP;
}

static void notify_autoconfig_failed_local(uint8_t unicast_addr, uint8_t reason)
{
    uint8_t ven_data[2] = { 0x00 };
    ven_data[1]         = reason;
    model_message message;
    message.source_addr        = unicast_addr;
    message.ven_data.data_len  = sizeof(ven_data);
    message.ven_data.user_data = ven_data;
    message.status_data        = NULL;

    if (g_mesh_node.at_cb._model_cb) {
        g_mesh_node.at_cb._model_cb(BT_MESH_MODEL_VENDOR_MESH_AUTOCONFIG_STATUS, &message);
    }

    if (g_mesh_node.user_model_cb) {
        g_mesh_node.user_model_cb(BT_MESH_MODEL_VENDOR_MESH_AUTOCONFIG_STATUS, &message);
    }
}

void prov_auto_config_timeout(void *timer, void *args)
{
    int ret = 0;
    k_timer_stop(&g_mesh_node.prov_auto_config_timer);

    if (g_mesh_node.prov_auto_config_retry) {
        ret = _ble_mesh_node_model_autoconfig(0, 0, g_mesh_node.prov_auto_config_addr, g_mesh_node.prov_auto_config);
        if (ret) {
            LOGE(TAG, "send auto config failed");
        }

        k_timer_start(&g_mesh_node.prov_auto_config_timer, g_mesh_node.prov_auto_config_timeout);
        g_mesh_node.prov_auto_config_retry--;
    } else {
        LOGE(TAG,
             "Provisioner autoconfig timeout, the device %04x appkey may not added or caused by some other reason\r\n",
             g_mesh_node.prov_auto_config_addr);
        notify_autoconfig_failed_local(g_mesh_node.prov_auto_config_addr, BT_MESH_AUTO_CONFIG_FAILED);
        g_mesh_node.prov_auto_config_addr = 0;
    }
}

int ble_mesh_node_appkey_add(uint16_t netkey_idx, uint16_t appkey_idx, uint16_t unicast_addr)
{
    const uint8_t *app_key;
    int16_t        ret;

    app_key = bt_mesh_provisioner_local_app_key_get(netkey_idx, appkey_idx);

    if (NULL == app_key) {
        return -1;
    }

    ret = bt_mesh_cfg_app_key_add(0, unicast_addr, netkey_idx, appkey_idx, app_key, NULL);

    if (ret && TIMEOUT != ret) {
        LOGE(TAG, "mesh appkey add set status send fail %d", ret);
        return ret;
    }

    return 0;
}

int ble_mesh_node_model_autoconfig(uint16_t netkey_idx, uint16_t appkey_idx, uint16_t addr,
                                   model_auto_config_t auto_conf)
{
#ifdef CONFIG_BT_MESH_MODEL_VENDOR_CLI
    int ret = 0;
    k_timer_stop(&g_mesh_node.prov_auto_config_timer);
    ret = _ble_mesh_node_model_autoconfig(netkey_idx, appkey_idx, addr, auto_conf);
    if (ret) {
        LOGE(TAG, "prov autoconfig set failed %d\r\n", ret);
        if (ret == -ENOTSUP || ret == -EINVAL) {
            return ret;
        }
    }

    g_mesh_node.prov_auto_config_timeout = DEF_PROV_AUTOCONFIG_TIMEOUT;

    k_timer_start(&g_mesh_node.prov_auto_config_timer, g_mesh_node.prov_auto_config_timeout);

    if (BT_MESH_ADDR_IS_UNICAST(addr)) {
        g_mesh_node.prov_auto_config_retry = DEF_UNICAST_ADDR_AUTO_CONFIG_RETRY;
    } else {
        g_mesh_node.prov_auto_config_retry = DEF_GROUP_ADDR_AUTO_CONFIG_RETRY;
    }

    return 0;
#endif
    return -ENOTSUP;
}

int ble_mesh_node_get_node_app_version(uint16_t netkey_idx, uint16_t appkey_idx, uint16_t addr)
{

#ifdef CONFIG_BT_MESH_MODEL_VENDOR_CLI
    struct bt_mesh_model *vendor_model;
    vnd_model_msg         vnd_msg = { 0 };
    uint8_t               send_data[3];
    vendor_model = ble_mesh_model_find(0, BT_MESH_MODEL_VND_MODEL_CLI, BT_MESH_MODEL_VND_COMPANY_ID);

    if (!vendor_model) {
        return -ENOTSUP;
    }

    send_data[0]         = ATTR_TYPE_REPORT_VERSION & 0xff;
    send_data[1]         = (ATTR_TYPE_REPORT_VERSION >> 8) & 0xff;
    send_data[2]         = MESH_NODE_VERSION_GET_TTL;
    vnd_msg.netkey_idx   = netkey_idx;
    vnd_msg.appkey_idx   = appkey_idx;
    vnd_msg.dst_addr     = addr;
    vnd_msg.model        = vendor_model;
    vnd_msg.len          = 3;
    vnd_msg.opid         = VENDOR_OP_ATTR_GET_STATUS;
    vnd_msg.data         = (uint8_t *)&send_data;
    vnd_msg.net_transmit = BT_MESH_TRANSMIT(6, 20);
    return ble_mesh_vendor_cli_model_msg_send_with_ttl(&vnd_msg, MESH_NODE_VERSION_GET_TTL);
#endif
    return -ENOTSUP;
}

int ble_mesh_node_active_check(uint16_t net_idx, uint16_t appkey_idx, uint16_t addr, uint8_t period_log)
{
#ifdef CONFIG_BT_MESH_MODEL_VENDOR_CLI
    struct bt_mesh_model * vendor_model;
    vnd_model_msg          vnd_msg      = { 0 };
    uint8_t                send_data[6] = { 0x0 };
    struct bt_mesh_hb_sub *sub          = NULL;
    int16_t                ret          = 0;

    vendor_model = ble_mesh_model_find(0, BT_MESH_MODEL_VND_MODEL_CLI, BT_MESH_MODEL_VND_COMPANY_ID);

    if (!vendor_model) {
        return -ENOTSUP;
    }

    sub = bt_mesh_hb_sub_get();
    if (!sub) {
        LOGE(TAG, "bt mesh hb sub get failed %d", ret);
        return -EIO;
    }

    if (sub->dst == BT_MESH_ADDR_UNASSIGN) {
        LOGE(TAG, "bt mesh hb sub addr not set,do hb sub first");
        return -1;
    }

    send_data[0] = ATTR_TYPE_ACTIVE_CHECK & 0xFF;
    send_data[1] = (ATTR_TYPE_ACTIVE_CHECK >> 8) & 0xFF;
    send_data[2] = bt_mesh_default_ttl_get();
    send_data[3] = period_log;
    send_data[4] = sub->dst & 0xFF;
    send_data[5] = (sub->dst >> 8) & 0xFF;

    vnd_msg.netkey_idx = net_idx;
    vnd_msg.appkey_idx = appkey_idx;
    vnd_msg.dst_addr   = addr;
    vnd_msg.model      = vendor_model;
    vnd_msg.len        = sizeof(send_data);
    vnd_msg.opid       = VENDOR_OP_ATTR_GET_STATUS;

    vnd_msg.data = (uint8_t *)send_data;

    ret = ble_mesh_vendor_cli_model_msg_send(&vnd_msg);
    if (ret) {
        return ret;
    }
    return 0;
#endif
    return -ENOTSUP;
}

int ble_mesh_node_overwrite_node_sublist(uint16_t net_idx, uint16_t appkey_idx, uint16_t addr, uint16_t mod_id,
                                         uint8_t sublist_size, uint16_t *subaddr, uint16_t CID)
{

#ifdef CONFIG_BT_MESH_MODEL_VENDOR_CLI
    struct bt_mesh_model *vendor_model;
    vnd_model_msg         vnd_msg = { 0 };
    int16_t               ret;
    if (sublist_size > CONFIG_MAX_SUBLIST_OVERWRITE_SIZE) {
        LOGE(TAG, "Max sublist overwrite size should no more than %d\r\n", CONFIG_MAX_SUBLIST_OVERWRITE_SIZE);
        return -EINVAL;
    }

    uint16_t *send_data = NULL;

    send_data = (uint16_t *)aos_zalloc(2 * (sublist_size + 4));
    if (!send_data) {
        LOGE(TAG, "sublist malloc failed");
        return -ENOMEM;
    }

    vendor_model = ble_mesh_model_find(0, BT_MESH_MODEL_VND_MODEL_CLI, BT_MESH_MODEL_VND_COMPANY_ID);

    if (!vendor_model) {
        aos_free(send_data);
        return -ENOTSUP;
    }

    send_data[0] = ATTR_TYPE_OVERWRITE_SUBLIST;
    send_data[1] = addr;
    send_data[2] = mod_id;
    send_data[3] = CID;

    for (int i = 0; i < sublist_size; i++) {
        send_data[4 + i] = subaddr[i];
    }
    vnd_msg.netkey_idx = net_idx;
    vnd_msg.appkey_idx = appkey_idx;
    vnd_msg.dst_addr   = addr;
    vnd_msg.model      = vendor_model;
    vnd_msg.len        = 2 * (sublist_size + 4);
    vnd_msg.opid       = VENDOR_OP_ATTR_SET_ACK;

    vnd_msg.data = (uint8_t *)send_data;

    ret = ble_mesh_vendor_cli_model_msg_send(&vnd_msg);

    if (ret) {
        aos_free(send_data);
        return -1;
    }
    aos_free(send_data);
    return 0;
#endif
    return -ENOTSUP;
}

static void prov_add_appkey_timeout_cb(void *timer, void *arg)
{
    int16_t ret;
    k_timer_stop(&g_mesh_node.prov_add_appkey_timer);

    if (g_mesh_node.prov_app_key_add_retry) {
        ret = ble_mesh_node_appkey_add(0, g_mesh_node.prov_auto_add_appkey_idx, last_addr);

        if (ret) {
            LOGE(TAG, "add appkey send failed");
        }

        g_mesh_node.prov_app_key_add_retry--;
        k_timer_start(&g_mesh_node.prov_add_appkey_timer, g_mesh_node.prov_add_appkey_timeout);
    } else {
        LOGE(TAG, "add appkey failed");
    }
}

static void provisioner_node_cb(mesh_provisioner_event_en event, void *p_arg)
{
    switch (event) {
            // provisioner message

        case BT_MESH_EVENT_PROV_COMP: {
            if (p_arg) {
                mesh_node_t *node            = (mesh_node_t *)p_arg;
                uint8_t      auto_add_appkey = 0;
                extern int   ble_mesh_provisioner_get_add_appkey_flag(u16_t unicast_addr);
                auto_add_appkey = ble_mesh_provisioner_get_add_appkey_flag(node->prim_unicast);

                if (auto_add_appkey) {
                    g_mesh_node.prov_app_key_add_retry   = DEF_ADD_APPKEY_RETRY;
                    g_mesh_node.prov_add_appkey_timeout  = DEF_ADD_APPKEY_TIMEOUT;
                    g_mesh_node.prov_auto_add_appkey_idx = 0;
                    last_addr                            = node->prim_unicast;
                    k_timer_stop(&g_mesh_node.prov_add_appkey_timer);
                    k_timer_start(&g_mesh_node.prov_add_appkey_timer, DEF_ADD_APPKEY_DELAY);
                }
            }
        } break;

        default:
            break;
    }

    if (g_mesh_node.at_cb._provisioner_cb) {
        g_mesh_node.at_cb._provisioner_cb(event, p_arg);
    }

    if (g_mesh_node.user_provisioner_cb) {
        g_mesh_node.user_provisioner_cb(event, p_arg);
    }
}

enum
{
    PROVISIONER_CONFIG_UNSET,
    PROVISIONER_CONFIG_SET,
};

enum
{
    PROVISIONER_ENABLE_FLAG,
    PROVISIONER_DISABLE_FLAG,
};

int ble_mesh_node_provisioner_init(provisioner_cb cb)
{
    if (!g_mesh_node.init_flag) {
        LOGE(TAG, "mesh node not init");
        return -EINVAL;
    }

    int ret         = 0;
    int config_flag = 0;
    ret             = aos_kv_getint("PROVISIONER_CONFIG_SET_FLAG", &config_flag);

    if (ret || config_flag != PROVISIONER_CONFIG_SET) {
        LOGW(TAG, "provisioner not config ,call provisioner config then");
    }

    if (config_flag == PROVISIONER_CONFIG_SET) {
        provisioner_node_config_t node_config;
        int                       get_len = sizeof(provisioner_node_config_t);
        ret                               = aos_kv_get("PROVISIONER_CONFIG", &node_config, &get_len);

        if (ret) {
            LOGW(TAG, "provisioner config may bt lost,call provisioner config then");
            return 0;
        } else {

            g_mesh_node.prov_config.unicast_addr_start = node_config.unicast_start_addr;
            g_mesh_node.prov_config.unicast_addr_end   = node_config.unicast_end_addr;
            g_mesh_node.prov_config.attention_time     = node_config.attention_time;
            g_mesh_node.prov_config.cb                 = provisioner_node_cb;
            ret                                        = ble_mesh_provisioner_init(&g_mesh_node.prov_config);

            if (ret) {
                LOGE(TAG, "ble mesh provisioner init failed %d", ret);
                return ret;
            }

            const ble_mesh_provisioner_t *prov_comp = ble_mesh_provisioner_get_provisioner_data();

            if (!prov_comp) {
                return ret;
            }

            g_mesh_node.prov_config_flag = 1;

            ret = provisioner_prov_init(prov_comp->provisioner);

            if (ret) {
                LOGE(TAG, "provisioner prov init failed %d", ret);
                return ret;
            }

            g_mesh_node.prov_init_flag = 1;

            if (bt_mesh_is_provisioned()) {
                int enable_flag = 0;
                ret             = aos_kv_getint("PROV_ENABLE_FLAG", &enable_flag);

                if (!ret) {
                    if (enable_flag == PROVISIONER_ENABLE_FLAG) {
                        ret = ble_mesh_node_provisioner_enable();

                        if (ret) {
                            LOGW(TAG, "mesh node provisioner enable failed %d", ret);
                        } else {
                            LOGW(TAG, "mesh node provisioner enable success");
                        }
                    } else {
                        LOGI(TAG, "mesh node provisioner not enable");
                    }
                }
            }
        }
    }

    g_mesh_node.user_provisioner_cb = cb;
    g_mesh_node.prov_init_flag      = 1;

    return 0;
}

int ble_mesh_node_provisioner_config(provisioner_node_config_t *config)
{
    if (!g_mesh_node.init_flag) {
        LOGE(TAG, "mesh node not init");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_init_flag) {
        LOGE(TAG, "mesh provisioner not init");
        return -EINVAL;
    }

    if (config->unicast_start_addr >= config->unicast_end_addr) {
        LOGE(TAG, "provisioner config start addr should larger than end addr");
        return -EINVAL;
    }

    const struct bt_mesh_comp *mesh_comp = ble_mesh_model_get_comp_data();

    if (!mesh_comp) {
        LOGE(TAG, "provisioner config get model comp data failed");
        return -EINVAL;
    }

    if (mesh_comp->elem_count > (config->unicast_end_addr - config->unicast_start_addr)) {
        LOGE(TAG, "no enough addr for provisioner");
        return -EINVAL;
    }

    if (g_mesh_node.node_provisioned) {
        if (mesh_comp->elem[0].addr != config->unicast_start_addr) { // TODO end address
            LOGE(TAG, "the provisioner node has been proved,the start address is %02x but set %02x",
                 mesh_comp->elem[0].addr, config->unicast_start_addr);
            return -EINVAL;
        }
    }

    int ret = 0;

    if (!g_mesh_node.prov_config_flag) {
        g_mesh_node.prov_config.unicast_addr_start = config->unicast_start_addr;
        g_mesh_node.prov_config.unicast_addr_end   = config->unicast_end_addr;
        g_mesh_node.prov_config.attention_time     = config->attention_time;
        g_mesh_node.prov_config.cb                 = provisioner_node_cb;
        ret                                        = ble_mesh_provisioner_init(&g_mesh_node.prov_config);

        if (ret) {
            LOGE(TAG, "ble mesh provisioner init failed %d", ret);
            return ret;
        }

        const ble_mesh_provisioner_t *prov_comp = ble_mesh_provisioner_get_provisioner_data();

        if (!prov_comp) {
            LOGE(TAG, "provisioner config get provisioner data failed");
            return ret;
        }

        ret = provisioner_prov_init(prov_comp->provisioner);
        if (ret) {
            LOGE(TAG, "provisioner prov init failed %d", ret);
            return ret;
        }

    } else {
        ret = provisioner_prov_reconfig(config->unicast_start_addr, config->unicast_end_addr, mesh_comp->elem_count,
                                        config->attention_time);
        LOGE(TAG, "provisioner reconfig failed %d", ret);
        return ret;
    }

    ret = aos_kv_set("PROVISIONER_CONFIG", config, sizeof(provisioner_node_config_t), 0);

    if (ret) {
        LOGE(TAG, "save provisioner config fail");
        return -1;
    } else {
        ret = aos_kv_setint("PROVISIONER_CONFIG_SET_FLAG", PROVISIONER_CONFIG_SET);

        if (ret) {
            LOGE(TAG, "save config set flag fail");
            return -1;
        }
    }

    g_mesh_node.prov_config_flag = 1;
    return 0;
}

int ble_mesh_node_provisioner_autoprov()
{
    if (!g_mesh_node.init_flag) {
        LOGE(TAG, "mesh node not init");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_init_flag) {
        LOGE(TAG, "mesh provisioner not init");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_config_flag) {
        LOGE(TAG, "mesh provisioner not config");
        return -EINVAL;
    }

    if (g_mesh_node.node_provisioned) {
        LOGW(TAG, "The provisioner node has been proved");
        return -EALREADY;
    }

    int ret = 0;

    ret = bt_mesh_provisioner_local_provision();

    if (ret < 0 && ret != -EALREADY) {
        LOGE(TAG, "provisioner local prov failed %d", ret);
        return ret;
    }

    uint16_t appkey_idx = 0;

    ret = bt_mesh_provisioner_local_app_key_add(NULL, 0, &appkey_idx);

    if (ret < 0 && -EEXIST != ret) {
        LOGE(TAG, "provisioner local add appkey failed %d", ret);
        return ret;
    } else if (0 == ret || -EEXIST == ret) {
        // bind all model to appkey0
        extern u16_t bt_mesh_primary_addr(void);
        bt_mesh_cfg_app_key_add(0, bt_mesh_primary_addr(), 0, appkey_idx,
                                bt_mesh_provisioner_local_app_key_get(0, appkey_idx), NULL);
        auto_config_t auto_conf = {
            .appkey_idx = appkey_idx,
            .sub_addr   = CONFIG_BT_MESH_AUTOCONFIG_LOCAL_SUB_ADDR,
            .pub_addr   = CONFIG_BT_MESH_AUTOCONFIG_LOCAL_PUB_ADDR,
            .mod_id     = 0xffff,
            .cid        = 0xffff,
        };
        auto_config(&auto_conf);
    }

    return 0;
}

int ble_mesh_node_provisioner_enable()
{
    if (!g_mesh_node.init_flag) {
        LOGE(TAG, "mesh node not init");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_init_flag) {
        LOGE(TAG, "mesh provisioner not init");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_config_flag) {
        LOGE(TAG, "mesh provisioner not config");
        return -EINVAL;
    }

    if (g_mesh_node.prov_enable_flag) {
        LOGE(TAG, "mesh provisioner enable already");
        return -EALREADY;
    }

    if (!bt_mesh_is_provisioned()) {
        LOGE(TAG, "Prov the provisioner first");
        return -1;
    }

    int ret = 0;

#if defined(CONFIG_BT_MESH_GATT_PROXY)
    ret = bt_mesh_proxy_gatt_disable();

    if (ret < 0 && ret != -EALREADY) {
        LOGE(TAG, "bt mesh proxy gatt disable failed %d", ret);
    }

    bt_mesh_proxy_adv_stop();

    ret = provisioner_proxy_init();

    if (ret < 0 && ret != -EALREADY) {
        LOGE(TAG, "provisioner proxy init fail");
        return ret;
    }

#endif

    ret = ble_mesh_provisioner_enable();

    if (ret && ret != -EALREADY) {
        LOGE(TAG, "provisioner enable failed %d", ret);
        return ret;
    }

    k_timer_init(&g_mesh_node.prov_add_appkey_timer, prov_add_appkey_timeout_cb, NULL);
    k_timer_init(&g_mesh_node.prov_auto_config_timer, prov_auto_config_timeout, NULL);

    ret = aos_kv_setint("PROV_ENABLE_FLAG", PROVISIONER_ENABLE_FLAG);

    if (ret) {
        LOGW(TAG, "save provisioner enable flag failed");
    }

    g_mesh_node.prov_enable_flag = 1;

    return 0;
}

int ble_mesh_node_provisioner_disable()
{
    if (!g_mesh_node.init_flag) {
        LOGE(TAG, "mesh node not init");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_init_flag) {
        LOGE(TAG, "mesh provisioner not init");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_config_flag) {
        LOGE(TAG, "mesh provisioner not config");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_enable_flag) {
        LOGE(TAG, "mesh provisioner disable already");
        return -EALREADY;
    }

    if (!bt_mesh_is_provisioned()) {
        LOGE(TAG, "Prov the provisioner first");
        return -1;
    }

    int ret = 0;
#if defined(CONFIG_BT_MESH_GATT_PROXY)
    ret = bt_mesh_proxy_gatt_enable();

    if (ret < 0 && ret != -EALREADY) {
        LOGE(TAG, "bt mesh proxy gatt enable failed %d", ret);
    }

    ret = bt_mesh_proxy_adv_start();

    if (ret < 0 && ret != -EALREADY) {
        LOGE(TAG, "bt mesh proxy gatt adv start failed %d", ret);
    }

#endif
    k_timer_stop(&g_mesh_node.prov_add_appkey_timer);
    k_timer_stop(&g_mesh_node.prov_auto_config_timer);

    ret = aos_kv_setint("PROV_ENABLE_FLAG", PROVISIONER_DISABLE_FLAG);

    if (ret) {
        LOGW(TAG, "save provisioner disable flag failed");
    }

    g_mesh_node.prov_enable_flag = 0;

    return ble_mesh_provisioner_disable();
}

bool ble_mesh_node_provisioner_is_enabled()
{
    return g_mesh_node.prov_enable_flag;
}

int ble_mesh_node_provisioner_get_config(provisioner_node_config_t *config)
{
    if (!config) {
        return -EINVAL;
    }

    if (!g_mesh_node.init_flag) {
        LOGE(TAG, "mesh node not init");
        return -EINVAL;
    }

    if (!g_mesh_node.prov_init_flag) {
        LOGE(TAG, "mesh provisioner not init");
        return -EINVAL;
    }

    config->unicast_start_addr = g_mesh_node.prov_config.unicast_addr_start;
    config->unicast_end_addr   = g_mesh_node.prov_config.unicast_addr_end;
    config->attention_time     = g_mesh_node.prov_config.attention_time;
    return 0;
}

int ble_mesh_node_at_cb_register(node_at_cb cb)
{
    if (!cb._model_cb && !cb._provisioner_cb && !cb._prov_cb) {
        return -EINVAL;
    } else {
        memcpy(&g_mesh_node.at_cb, &cb, sizeof(node_at_cb));
    }

    return 0;
}

int ble_mesh_node_at_cb_unregister()
{
    memset(&g_mesh_node.at_cb, 0x00, sizeof(node_at_cb));
    return 0;
}

#else

__attribute__((unused)) static const char *bearer2str(int bearer)
{
    switch (bearer) {
        case BT_MESH_PROV_ADV:
            return "PB-ADV";

        case BT_MESH_PROV_GATT:
            return "PB-GATT";

        default:
            return "unknown";
    }
}

static void link_open(bt_mesh_prov_bearer_t bearer)
{
    LOGI(TAG, "Provisioning link opened on %s\n", bearer2str(bearer));
}

static void link_close(bt_mesh_prov_bearer_t bearer)
{
    LOGI(TAG, "Provisioning link closed on %s\n", bearer2str(bearer));
}

static void prov_complete(uint16_t net_idx, uint16_t addr)
{
    LOGI(TAG, "provisioning complete netid %d, primary element address 0x%x", net_idx, addr);
    mesh_node_local_t node_info;
    node_info.prim_unicast = addr;
    g_mesh_node.auto_config_status = BT_MESH_AUTO_CONFIG_NOT_START;
    g_mesh_node.app_key_add_flag = 0;

    if (g_mesh_node.user_prov_cb) {
        g_mesh_node.user_prov_cb(BT_MESH_EVENT_NODE_PROV_COMP, &node_info);
    }
}

static void prov_reset(void)
{
    LOGD(TAG, "Node Reset");
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

    if (g_mesh_node.user_prov_cb) {
        g_mesh_node.user_prov_cb(BT_MESH_EVENT_NODE_REST, NULL);
    }
}

static int output_number(bt_mesh_output_action_t action, u32_t number)
{
    LOGI(TAG, "OOB Number: %u\n", number);
    return 0;
}

static int output_string(const char *str)
{
    LOGI(TAG, "OOB String: %s\n", str);
    return 0;
}

static int node_input(bt_mesh_input_action_t act, u8_t size)
{
    switch (act) {
        case BT_MESH_ENTER_NUMBER: {
            LOGI(TAG, "Enter a number (max %u digits) with: input-num <num>\n", size);

            if (g_mesh_node.user_prov_cb) {
                g_mesh_node.user_prov_cb(BT_MESH_EVENT_NODE_OOB_INPUT_NUM, &size);
            }
        } break;

        case BT_MESH_ENTER_STRING: {
            LOGI(TAG, "Enter a string (max %u chars) with: input-str <str>\n", size);

            if (g_mesh_node.user_prov_cb) {
                g_mesh_node.user_prov_cb(BT_MESH_EVENT_NODE_OOB_INPUT_STRING, &size);
            }
        } break;

        default: {
            LOGI(TAG, "Unknown input action %u (size %u) requested!\n", act, size);
            return -EINVAL;
        } break;
    }

    g_mesh_node.input_act = act;
    g_mesh_node.input_size = size;
    return 0;
}

#ifdef BT_MESH_BEACON_RSSI_REPORT
static bool is_rssi_data_queue_empty(rssi_data_queue *queue)
{
    return queue->front == queue->rear;
}

static bool is_rssi_data_queue_full(rssi_data_queue *queue)
{
    if ((queue->rear + 1) % 16 == queue->front) {
        return true;
    } else {
        return false;
    }
}

static beacon_rssi_data *get_rssi_data(rssi_data_queue *queue)
{
    if (is_rssi_data_queue_empty(queue)) {
        return NULL;
    } else {
        beacon_rssi_data *data = &queue->data[queue->front];
        queue->front = (queue->front + 1) % 16;
        return data;
    }
}

static int put_rssi_data(rssi_data_queue *queue, const uint8_t *mac, int8_t rssi, int8_t rssi_calibration)
{
    if (is_rssi_data_queue_full(queue)) {
        return -1;
    }

    memcpy(queue->data[queue->rear].mac, mac, 6);
    queue->data[queue->rear].rssi = rssi;
    queue->data[queue->rear].rssi_calibration = rssi_calibration;
    queue->rear = (queue->rear + 1) % 16;
    return 0;
}

static uint8_t data_complent_u8(int8_t data)
{
    uint8_t out;

    if (data <= 0) {
        out = data;
    } else {
        uint8_t in = -data;
        out = ~in + 1;
    }

    return out;
}

static void beacon_rssi_cb(const struct adv_addr_t *addr, s8_t rssi, u8_t adv_type, void *user_data, uint16_t len)
{
    if (!g_mesh_node.node_provisioned) {
        return;
    }

    if (!user_data || !len) {
        return;
    }

    struct net_buf_simple *buf = (struct net_buf_simple *)user_data;

    uint8_t data_len = net_buf_simple_pull_u8(buf);

    if (!data_len) {
        return;
    }

    uint8_t opid = net_buf_simple_pull_u8(buf);

    if (0x02 != opid) {
        return;
    }

    int8_t rssi_calibration = net_buf_simple_pull_u8(buf);

    put_rssi_data(&g_mesh_node.rssi_queue, addr->val, data_complent_u8(rssi), data_complent_u8(rssi_calibration));
}

void rssi_repot_timeout(void *timer, void *args)
{
    beacon_rssi_data *data = NULL;
    k_timer_start(&g_mesh_node.beacon_rssi_timer, RSSI_BEACON_REPORT_PERIOD);
    data = get_rssi_data(&g_mesh_node.rssi_queue);

    if (!data) {
        return;
    }

    struct bt_mesh_model *vendor_model;

    vendor_model = ble_mesh_model_find(0, BT_MESH_MODEL_VND_MODEL_SRV, CONFIG_CID_TAOBAO);

    if (!vendor_model || !vendor_model->pub || BT_MESH_ADDR_UNASSIGNED == vendor_model->pub->addr) {
        return;
    }

    vnd_model_msg vnd_data = { 0 };
    uint8_t rssi_data[60];
    snprintf((char *)rssi_data, sizeof(rssi_data), "rssi%02x,%02x,%02x%02x%02x%02x%02x%02x", data->rssi,
             data->rssi_calibration, data->mac[5], data->mac[4], data->mac[3], data->mac[2], data->mac[1],
             data->mac[0]);
    vnd_data.netkey_idx = 0;
    vnd_data.appkey_idx = 0;
    vnd_data.dst_addr = vendor_model->pub->addr;
    vnd_data.model = vendor_model;
    vnd_data.opid = VENDOR_OP_ATTR_TRANS_MSG;
    vnd_data.data = rssi_data;
    vnd_data.retry = 0;
    vnd_data.len = strlen((char *)rssi_data);
    ble_mesh_vendor_srv_model_msg_send(&vnd_data);
}

static int init_rssi_report()
{
    int16_t ret;
    ret = bt_mesh_vnd_adv_set_cb(beacon_rssi_cb);

    if (ret) {
        return ret;
    }

    g_mesh_node.rssi_queue.front = 0;
    g_mesh_node.rssi_queue.rear = 0;
    g_mesh_node.rssi_queue.length = 16;
    k_timer_init(&g_mesh_node.beacon_rssi_timer, rssi_repot_timeout, NULL);
    k_timer_start(&g_mesh_node.beacon_rssi_timer, RSSI_BEACON_REPORT_PERIOD);
    return 0;
}
#endif

int ble_mesh_node_OOB_input_string(const char *str)
{
    if (!g_mesh_node.init_flag) {
        return -1;
    }

    int16_t ret;

    if (g_mesh_node.input_act != BT_MESH_ENTER_STRING) {
        LOGE(TAG, "A string hasn't been requested!\n");
        return -1;
    }

    if (strlen(str) < g_mesh_node.input_size) {
        LOGE(TAG, "Too short input (%u characters required)\n", g_mesh_node.input_size);
        return -1;
    }

    ret = bt_mesh_input_string(str);

    if (ret) {
        LOGE(TAG, "String input failed (err %d)\n", ret);
        return -1;
    }

    g_mesh_node.input_act = BT_MESH_NO_INPUT;

    return 0;
}

int ble_mesh_node_OOB_input_num(uint32_t num)
{
    if (!g_mesh_node.init_flag) {
        return -1;
    }

    int16_t ret;

    if (g_mesh_node.input_act != BT_MESH_ENTER_STRING) {
        LOGE(TAG, "A num hasn't been requested!\n");
        return -1;
    }

    ret = bt_mesh_input_number(num);

    if (ret) {
        LOGE(TAG, "num input failed (err %d)\n", ret);
        return -1;
    }

    g_mesh_node.input_act = BT_MESH_NO_INPUT;

    return 0;
}

#endif

#if defined(CONFIG_BT_MESH_MODEL_VENDOR_SRV) && CONFIG_BT_MESH_MODEL_VENDOR_SRV > 0
static int version_char_to_u32(char *ver_str, unsigned int *version)
{
    int v1 = 0, v2 = 0, v3 = 0;
    if (*ver_str == 'v') {
        sscanf(ver_str, "v%d.%d.%d", &v1, &v2, &v3);
    } else {
        sscanf(ver_str, "%d.%d.%d", &v1, &v2, &v3);
    }
    *version = (v1 << 16) | (v2 << 8) | (v3);
    return 0;
}

static unsigned int ota_version_appver_get(void)
{
    unsigned int version = 0;
    int          ret     = 0;
    extern char *aos_get_app_version(void);
    char *       version_str = aos_get_app_version();
    if (!version_str) {
        LOGE(TAG, "get version failed\r\n");
    }
    ret = version_char_to_u32(version_str, &version);
    if (ret < 0) {
        LOGE(TAG, "trans version failed %d\r\n", ret);
        return 0;
    }
    return version;
}

static void set_support_ota_protocol(uint8_t *ota_protocol)
{
#if !defined(CONFIG_OTA_CLIENT) || CONFIG_OTA_CLIENT <= 0
    *ota_protocol = NODE_OTA_NOT_SUPPORT;
    return;
#endif

#if defined(CONFIG_OTA_CLIENT_PROCTOCOL_AIS) && CONFIG_OTA_CLIENT_PROCTOCOL_AIS > 0                                    \
    && defined(CONFIG_OTA_CLIENT_TRANS_GATT) && CONFIG_OTA_CLIENT_TRANS_GATT
    *ota_protocol |= (1 << NODE_OTA_AIS_BY_GATT);
#endif

#if defined(CONFIG_OTA_CLIENT_PROCTOCOL_MESH) && CONFIG_OTA_CLIENT_PROCTOCOL_MESH > 0
    *ota_protocol |= (1 << NODE_OTA_MESH_BY_EXT_1M);
    *ota_protocol |= (1 << NODE_OTA_MESH_BY_EXT_2M);
#endif
    return;
}
#endif

int ble_mesh_vendor_srv_model_version_report(uint16_t dst_addr, uint8_t ack_ttl)
{
#if defined(CONFIG_BT_MESH_MODEL_VENDOR_SRV) && CONFIG_BT_MESH_MODEL_VENDOR_SRV > 0
    struct bt_mesh_model *vendor_model;
    vendor_model = ble_mesh_model_find(0, BT_MESH_MODEL_VND_MODEL_SRV, CONFIG_CID_TAOBAO);
    if (!vendor_model) {
        return -EINVAL;
    }
    vnd_model_msg vnd_data         = { 0 };
    uint8_t       data[7]          = { 0 };
    uint8_t       support_protocol = 0x0;
    uint32_t      version          = ota_version_appver_get();
    set_support_ota_protocol(&support_protocol);
    data[0] = ATTR_TYPE_REPORT_VERSION & 0xff;
    data[1] = (ATTR_TYPE_REPORT_VERSION >> 8) & 0xff;
    data[2] = (version >> 24) & 0xff;
    data[3] = (version >> 16) & 0xff;
    data[4] = (version >> 8) & 0xff;
    data[5] = version & 0xff;
    data[6] = support_protocol;

    vnd_data.netkey_idx   = 0;
    vnd_data.appkey_idx   = 0;
    vnd_data.dst_addr     = dst_addr;
    vnd_data.model        = vendor_model;
    vnd_data.opid         = VENDOR_OP_ATTR_STATUS;
    vnd_data.data         = data;
    vnd_data.retry        = DEF_VERSION_REPORT_RETRY;
    vnd_data.len          = sizeof(data);
    vnd_data.net_transmit = BT_MESH_TRANSMIT(5, 20);
    return ble_mesh_vendor_srv_model_msg_send_with_ttl(&vnd_data, ack_ttl);
#else
    return -ENOTSUP;
#endif
}

#if !defined(CONFIG_BT_MESH_PROVISIONER) || !CONFIG_BT_MESH_PROVISIONER

static int _node_active_check(uint8_t msg_hop, uint16_t sub_addr, uint8_t period_log, hb_set_status *result)
{
    int ret = 0;
    memset(result, 0x00, sizeof(hb_set_status));
    struct bt_mesh_hb_pub *pub = NULL;

    pub = bt_mesh_hb_pub_get();
    if (!pub) {
        LOGE(TAG, "no hb pub");
        result->check_status.hb_enable = NODE_HB_NOT_ENABLE;
        return 0;
    }

    if (!pub->count && !pub->period && !pub->dst) {
        LOGW(TAG, "hb not enable");
        result->check_status.hb_enable = NODE_HB_NOT_ENABLE;
        return 0;
    } else {
        result->check_status.hb_enable = NODE_HB_ENABLE;
    }

    result->period_log = pub->period;

    if (pub->dst != sub_addr) {
        LOGE(TAG, "hb pub addr:%04x, but the sub node sub addr %04x", pub->dst, sub_addr);
        result->check_status.hb_sub_pub_check = NODE_HB_SUB_PUB_MISMATCH;
    } else {
        result->check_status.hb_sub_pub_check = NODE_HB_SUB_PUB_MATCH;
    }

    if (msg_hop + 1 > pub->ttl) {
        LOGE(TAG, "Pub ttl should be larger than msg hop %d, but pub ttl is %d", msg_hop, pub->ttl);
        result->check_status.hb_ttl_check = NODE_HB_TTL_INVALID;
        ret = ble_mesh_cfg_srv_hb_pub_ttl_set(msg_hop + 1 + 2); /* extra add 2 for asymmetric case */
        if (ret) {
            LOGE(TAG, "hb pub ttl reconfig failed %d", ret);
        } else {
            LOGD(TAG, "Hb pub ttl recofig to %d", msg_hop + 1 + 2);
        }
    } else {
        result->check_status.hb_ttl_check = NODE_HB_TTL_VAILD;
    }

    if (!pub->count) {
        LOGE(TAG, "Pub count null");
        result->check_status.hb_pub_count_check = NODE_HB_COUNT_INVALID;
    } else {
        result->check_status.hb_pub_count_check = NODE_HB_COUNT_VALID;
    }

    if (pub->period != period_log) {
        if (period_log == HB_PUB_PERIOD_NOT_SET) {
            LOGW(TAG, "Pub period no compare");
            result->check_status.hb_period_check = NODE_HB_PERIOD_NO_CHECK;
        } else {
            LOGE(TAG, "Pub period log %02x, but sub node using period log %02x", pub->period, period_log);
            result->check_status.hb_period_check = NODE_HB_PERIOD_MISMATCH;
        }
    } else {
        result->check_status.hb_period_check = NODE_HB_PERIOD_MATCH;
    }
    return 0;
}

static int _report_active_status(uint16_t dst_addr, uint8_t node_type, hb_set_status hb)
{
#if defined(CONFIG_BT_MESH_MODEL_VENDOR_SRV) && CONFIG_BT_MESH_MODEL_VENDOR_SRV > 0
    struct bt_mesh_model *vendor_model;
    int                   ret = 0;
    vendor_model              = ble_mesh_model_find(0, BT_MESH_MODEL_VND_MODEL_SRV, CONFIG_CID_TAOBAO);
    if (!vendor_model) {
        return -EINVAL;
    }

    vnd_model_msg vnd_data = { 0 };
    uint8_t       data[5]  = { 0 };

    data[0] = ATTR_TYPE_ACTIVE_CHECK & 0xff;
    data[1] = (ATTR_TYPE_ACTIVE_CHECK >> 8) & 0xff;
    data[2] = node_type;
    memcpy(&data[3], &hb.check_status, 1);
    data[4] = hb.period_log;

    vnd_data.netkey_idx   = 0;
    vnd_data.appkey_idx   = 0;
    vnd_data.dst_addr     = dst_addr;
    vnd_data.model        = vendor_model;
    vnd_data.opid         = VENDOR_OP_ATTR_STATUS;
    vnd_data.data         = data;
    vnd_data.retry        = DEF_VERSION_REPORT_RETRY;
    vnd_data.len          = sizeof(data);
    vnd_data.net_transmit = BT_MESH_TRANSMIT(5, 20);
    ret                   = ble_mesh_vendor_srv_model_msg_send_with_ttl(&vnd_data, bt_mesh_default_ttl_get());
    if (ret) {
        LOGE(TAG, "vendor send failed %d", ret);
    }
    return ret;
#else
    return -ENOTSUP;
#endif
}

static void sleep_random()
{
    uint8_t delay;
    bt_rand(&delay, 1);
    if (delay < 20) {
        delay = 20;
    } else if (delay > 100) {
        delay = 100;
    }
    k_sleep(delay);
}

static uint8_t get_send_ttl(uint8_t cli_ttl, uint8_t recv_ttl)
{
    if (cli_ttl == TTL_NOT_SET || recv_ttl > cli_ttl) {
        return bt_mesh_default_ttl_get();
    } else {
        return cli_ttl - recv_ttl == 0 ? 0 : cli_ttl - recv_ttl + 1;
    }
}

static int _vendor_model_get_status_process(model_message *msg)
{
    uint16_t status = 0x00;

    if (!msg || !msg->ven_data.user_data || !msg->ven_data.data_len) {
        LOGE(TAG, "No vend data set");
        return -EINVAL;
    }

    uint8_t *p_data = (uint8_t *)msg->ven_data.user_data;

    if (msg->ven_data.data_len >= 2) {
        status = p_data[0] | (p_data[1] << 8);
    } else {
        LOGE(TAG, "Err vend data len");
        return -EINVAL;
    }

    switch (status) {
        case ATTR_TYPE_REPORT_VERSION: {
            static long long get_last = 0;
            uint8_t          cli_ttl  = p_data[0];
            uint8_t          ack_ttl  = get_send_ttl(cli_ttl, p_data[2]);
            if (get_last != 0 && aos_now_ms() - get_last <= DEF_GET_VERSION_RETRY_TIMEOUT) {
                ack_ttl = bt_mesh_default_ttl_get();
            }
            get_last = aos_now_ms();
            if (BT_MESH_ADDR_IS_GROUP(msg->dst_addr)) {
                sleep_random();
            }
            if (ble_mesh_vendor_srv_model_version_report(msg->source_addr, ack_ttl)) {
                LOGE(TAG, "node report verison failed");
            }
        } break;
        case ATTR_TYPE_ACTIVE_CHECK: {
            if (msg->ven_data.data_len < 6) {
                LOGE(TAG, "Err vend active check len");
                return -EINVAL;
            }

            uint8_t  ttl_origin = p_data[2];
            uint8_t  period_log = p_data[3];
            uint16_t sub_addr   = p_data[4] | (p_data[5] << 8);
            uint8_t  node_type  = BT_MESH_NODE_NO_LPM;
#if defined(CONFIG_BT_MESH_LPM) && CONFIG_BT_MESH_LPM > 0
#if defined(CONFIG_BT_MESH_LPM_MODE_TX_ONLY) && CONFIG_BT_MESH_LPM_MODE_TX_ONLY > 0
            node_type = BT_MESH_NODE_LPM_NO_RX;
#elif defined(CONFIG_BT_MESH_LPM_MODE_TX_RX) && CONFIG_BT_MESH_LPM_MODE_TX_RX > 0
            node_type = BT_MESH_NODE_LPM_TX_RX;
#elif defined(CONFIG_BT_MESH_LPM_MODE_RX_TX) && CONFIG_BT_MESH_LPM_MODE_RX_TX > 0
            node_type = BT_MESH_NODE_LPM_RX_TX;
#endif
#endif
            uint8_t       hop = ttl_origin - msg->recv_ttl + 1;
            hb_set_status check_status;
            if (_node_active_check(hop, sub_addr, period_log, &check_status)) {
                LOGE(TAG, "node status check failed");
            }
            if (BT_MESH_ADDR_IS_GROUP(msg->dst_addr)) {
                sleep_random();
            }
            if (_report_active_status(msg->source_addr, node_type, check_status)) {
                LOGE(TAG, "node active status send failed");
            }
        } break;
        default:
            break;
    }

    return 0;
}
#endif

static void node_model_message_cb(mesh_model_event_en event, void *p_arg)
{
    int ret;

    switch (event) {
#ifdef CONFIG_BT_MESH_PROVISIONER

        case BT_MESH_MODEL_CFG_APPKEY_STATUS: {

            if (p_arg) {
                model_message message = *(model_message *)p_arg;
                uint8_t       status  = message.status_data->data[0];

                k_timer_stop(&g_mesh_node.prov_add_appkey_timer);

                if (status) {
                    if (g_mesh_node.prov_app_key_add_retry) {
                        ret = ble_mesh_node_appkey_add(0, g_mesh_node.prov_auto_add_appkey_idx, last_addr);

                        if (ret) {
                            LOGE(TAG, "add appkey failed");
                        } else {
                            g_mesh_node.prov_app_key_add_retry--;
                        }

                    } else {
                        LOGE(TAG, "add appkey failed");
                    }
                }
            }
        } break;
#else

        case BT_MESH_MODEL_CFG_APP_KEY_ADD: {
#if defined(CONFIG_BT_MESH_MODEL_VENDOR_SRV)

            if (p_arg) {
                appkey_status *add_status = (appkey_status *)p_arg;

                if (!add_status->status) {
                    if (g_mesh_node.app_key_add_flag) {
                        break;
                    }

                    g_mesh_node.app_key_add_flag = 1;
                    auto_config_t config = {
                        .appkey_idx = add_status->appkey_idx,
                        .sub_addr = COMMON_SUB_ADDR,
                        .pub_addr = COMMON_PUB_ADDR,
                        .mod_id = BT_MESH_MODEL_VND_MODEL_SRV,
                        .cid = BT_MESH_MODEL_VND_COMPANY_ID,
                    };

                    auto_config(&config);
                }
            }

#endif
        } break;

        case BT_MESH_MODEL_VENDOR_GET_STATUS: {
            if (p_arg) {
                model_message *msg = (model_message *)p_arg;
                ret = _vendor_model_get_status_process(msg);
                if (ret) {
                    LOGE(TAG, "vendor get status process failed");
                }
            }
        } break;

        case BT_MESH_MODEL_VENDOR_MESH_AUTOCONFIG: {
            if (p_arg && BT_MESH_AUTO_CONFIG_START != g_mesh_node.auto_config_status) {
                // uint8_t tid;
                int16_t ret;
                model_message *msg = (model_message *)p_arg;
                if (!g_mesh_node.app_key_add_flag) {
                    g_mesh_node.auto_config_status = BT_MESH_AUTO_CONFIG_FAILED;

                    if (BT_MESH_ADDR_IS_UNICAST(msg->dst_addr)) {
                        ret = report_auto_config_status(msg->source_addr, g_mesh_node.auto_config_status);

                        if (ret) {
                            LOGE(TAG, "report auto config status failed");
                        }
                    }

                    return;
                }
                if (BT_MESH_ADDR_IS_UNICAST(msg->dst_addr)) {
                    g_mesh_node.auto_config_ack_flag = 1;
                } else {
                    g_mesh_node.auto_config_ack_flag = 0;
                }
                if (BT_MESH_AUTO_CONFIG_SUCCEED == g_mesh_node.auto_config_status) {
                    if (g_mesh_node.auto_config_ack_flag) {
                        ret = report_auto_config_status(msg->source_addr, g_mesh_node.auto_config_status);

                        if (ret) {
                            LOGE(TAG, "report auto config status failed");
                        }
                    }
                    return;
                }
                g_mesh_node.auto_config_status = BT_MESH_AUTO_CONFIG_START;

                net_buf_simple_pull_u8(msg->ven_data.user_data);
                auto_config_t config = { 0 };
                config.appkey_idx = 0;
                config.sub_addr = net_buf_simple_pull_le16(msg->ven_data.user_data);
                config.pub_addr = 0; // net_buf_simple_pull_le16(msg->ven_data.user_data);
                config.mod_id = 0xffff;
                config.cid = 0xffff;
                ret = auto_config(&config);
                if (ret) {
                    g_mesh_node.auto_config_status = BT_MESH_AUTO_CONFIG_FAILED;
                    ret = aos_kv_setint("AUTO_CONFIG", BT_MESH_AUTO_CONFIG_FAILED);

                    if (ret) {
                        LOGE(TAG, "save auto config status failed");
                    }
                }

                if (!ret) {
                    g_mesh_node.prov_addr = msg->source_addr;
                    g_mesh_node.auto_config_retry = DEF_AUTOCONFIG_STORE_CHECK_RETRY;
                    g_mesh_node.auto_config_timeout = DEF_AUTOCONFIG_STORE_CHECK_TIMEOUT;
                    k_timer_start(&g_mesh_node.auto_config_timer, g_mesh_node.auto_config_timeout);
                }
            }
        }

        break;

        case BT_MESH_MODEL_VENDOR_MESH_AUTOCONFIG_GET: {
            if (p_arg) {
                model_message *message = (model_message *)p_arg;

                if (!bt_mesh_store_mod_pending_check()) {
                    ret = report_auto_config_status(message->source_addr, g_mesh_node.auto_config_status);

                    if (ret) {
                        LOGE(TAG, "report auto config status failed");
                    }
                }
            }
        } break;
#endif

        case BT_MESH_MODEL_CFG_RST_STATUS: {
            ret = aos_kv_setint("AUTO_CONFIG", BT_MESH_AUTO_CONFIG_NOT_START);

            if (ret) {
                LOGE(TAG, "save auto config status failed");
            }
        } break;
#ifdef CONFIG_BT_MESH_PROVISIONER

        case BT_MESH_MODEL_VENDOR_MESH_AUTOCONFIG_STATUS: {
            model_message *message = (model_message *)p_arg;
            // uint8_t *ven_data = (uint8_t *)message->ven_data.user_data;
            // uint8_t status = ven_data[1];

            if (BT_MESH_ADDR_IS_UNICAST(g_mesh_node.prov_auto_config_addr)) {
                if (g_mesh_node.prov_auto_config_addr == message->source_addr) {
                    k_timer_stop(&g_mesh_node.prov_auto_config_timer);
                }
            }
        } break;
#endif

        default:
            break;
    }

    if (g_mesh_node.at_cb._model_cb) {
        g_mesh_node.at_cb._model_cb(event, p_arg);
    }

    if (g_mesh_node.user_model_cb) {
        g_mesh_node.user_model_cb(event, p_arg);
    }
}

void node_attention_on(struct bt_mesh_model *model)
{
    if (g_mesh_node.health_cb.att_on) {
        g_mesh_node.health_cb.att_on();
    }
}

void node_attention_off(struct bt_mesh_model *model)
{
    if (g_mesh_node.health_cb.att_off) {
        g_mesh_node.health_cb.att_off();
    }
}

int node_faults_test(struct bt_mesh_model *model, u8_t test_id, u16_t company_id)
{
    if (g_mesh_node.health_cb.faults_test) {
        return g_mesh_node.health_cb.faults_test(test_id, company_id);
    }
    return 0;
}

static void get_faults(u8_t *faults, u8_t faults_size, u8_t *dst, u8_t *count)
{
    u8_t i, limit = *count;

    for (i = 0, *count = 0; i < faults_size && *count < limit; i++) {
        if (faults[i]) {
            *dst++ = faults[i];
            (*count)++;
        }
    }
}

static int node_fault_get_reg(struct bt_mesh_model *model, u16_t cid, u8_t *test_id, u8_t *faults, u8_t *fault_count)
{
    if (cid != CONFIG_BT_MESH_VENDOR_COMPANY_ID) {
        LOGE(TAG, "Faults requested for unknown Company ID 0x%04x", cid);
        return -EINVAL;
    }

    for (int i = 0; i < sizeof(g_mesh_node.test_faults) / sizeof(g_mesh_node.test_faults[0]); i++) {
        if (g_mesh_node.test_faults[i].test_id == *test_id && g_mesh_node.test_faults[i].faults_num) {
            get_faults(g_mesh_node.test_faults[i].faults, sizeof(g_mesh_node.test_faults[i].faults), faults,
                       fault_count);
        }
    }
    return 0;
}

static int node_fault_clear(struct bt_mesh_model *model, uint16_t cid)
{
    if (cid != CONFIG_BT_MESH_VENDOR_COMPANY_ID) {
        return -EINVAL;
    }
    memset(&g_mesh_node.test_faults, 0x00, sizeof(g_mesh_node.test_faults));
    return 0;
}

struct bt_mesh_health_srv_cb g_health_cb = {
    .fault_test    = node_faults_test,
    .fault_get_reg = node_fault_get_reg,
    .fault_clear   = node_fault_clear,
    .attn_on       = node_attention_on,
    .attn_off      = node_attention_off,
};

int ble_mesh_node_faults_clear()
{
    if (!g_mesh_node.init_flag) {
        LOGE(TAG, "mesh node not init");
        return -EINVAL;
    }
    memset(&g_mesh_node.test_faults, 0x00, sizeof(g_mesh_node.test_faults));
    return 0;
}

int ble_mesh_node_fault_add(uint8_t test_id, uint8_t fault)
{
    if (!g_mesh_node.init_flag) {
        LOGE(TAG, "mesh node not init");
        return -EINVAL;
    }
    uint8_t found_index = 0xFF;

    for (int i = 0; i < sizeof(g_mesh_node.test_faults) / sizeof(g_mesh_node.test_faults[0]); i++) {
        if (g_mesh_node.test_faults[i].test_id == test_id) {
            if (g_mesh_node.test_faults[i].faults_num == sizeof(g_mesh_node.test_faults[i])) {
                LOGE(TAG, "test id %d faults full, clear first", test_id);
                return -ENOMEM;
            } else {
                g_mesh_node.test_faults[i].faults[g_mesh_node.test_faults[i].faults_num++] = fault;
                return 0;
            }
        } else {
            if (found_index == 0xFF && !g_mesh_node.test_faults[i].faults_num) {
                found_index = i;
            }
        }
    }

    if (found_index != 0xFF) {
        g_mesh_node.test_faults[found_index].faults[g_mesh_node.test_faults[found_index].faults_num++] = fault;
    } else {
        LOGE(TAG, "test id %d faults array full, clear first");
        return -ENOMEM;
    }
    return 0;
}

int ble_mesh_node_init(node_config_t *param)
{
    if (!param) {
        return -1;
    }

    int ret;

    memset(&g_mesh_node, 0x00, sizeof(mesh_node));

    const struct bt_mesh_comp *mesh_comp = ble_mesh_model_get_comp_data();

    if (!mesh_comp) {
        return -1;
    }

    g_mesh_node.comp = mesh_comp;

    if (param->node_oob.static_oob_data) {
        memcpy(g_static_data, param->node_oob.static_oob_data, 16);
        node_prov.static_val_len = 16;
    }

    if (ACTION_NUM == param->node_oob.input_action && param->node_oob.input_max_size > 0) {
        node_prov.input_actions = BT_MESH_ENTER_NUMBER;
        node_prov.input_size    = param->node_oob.input_max_size;
    } else if (ACTION_STR == param->node_oob.input_action && param->node_oob.input_max_size > 0) {
        node_prov.input_actions = BT_MESH_ENTER_STRING;
        node_prov.input_size    = param->node_oob.input_max_size;
    } else {
        node_prov.input_actions = BT_MESH_NO_INPUT;
        node_prov.input_size    = 0;
    }

    if (ACTION_NUM == param->node_oob.output_action && param->node_oob.output_max_size > 0) {
        node_prov.output_actions = BT_MESH_DISPLAY_NUMBER;
        node_prov.output_size    = param->node_oob.output_max_size;
    } else if (ACTION_STR == param->node_oob.output_action && param->node_oob.output_max_size > 0) {
        node_prov.output_actions = BT_MESH_DISPLAY_STRING;
        node_prov.output_size    = param->node_oob.output_max_size;
    } else {
        node_prov.output_actions = BT_MESH_NO_OUTPUT;
        node_prov.output_size    = 0;
    }

    memcpy(dev_uuid, param->dev_uuid, 16);

    g_mesh_node.user_prov_cb = param->user_prov_cb;

    ret = bt_mesh_init(&node_prov, mesh_comp, NULL);

    if (ret) {
        LOGE(TAG, "bt mesh init failed %d", ret);
        return ret;
    }

    ret = ble_stack_setting_load();
    if (ret) {
        return ret;
    }

    ret = ble_stack_set_name((char *)param->dev_name);
    if (ret) {
        return ret;
    }

#ifdef CONFIG_BT_MESH_NODE_REPEAT_RESET
    if (bt_mesh_is_provisioned()) {
        extern void reset_by_repeat_init(void);
        reset_by_repeat_init();
    }
#endif

#ifdef CONFIG_OTA_CLIENT
    extern int ota_client_init();
    ret = ota_client_init();
    if (ret) {
        return ret;
    }
#endif

    /* This will be a no-op if settings_load() loaded provisioning info */
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

    g_mesh_node.node_provisioned = bt_mesh_is_provisioned();

    if (g_mesh_node.node_provisioned) {
        int auto_config_status = 0;
        ret                    = aos_kv_getint("AUTO_CONFIG", &auto_config_status);

        if (ret) {
            g_mesh_node.auto_config_status = BT_MESH_AUTO_CONFIG_NOT_START;
        } else {
            g_mesh_node.auto_config_status = auto_config_status;
        }
    }

    k_timer_init(&g_mesh_node.auto_config_timer, auto_config_timeout_cb, NULL);

    extern struct bt_mesh_app_key *bt_mesh_app_key_find(u16_t app_idx);

    if (!bt_mesh_app_key_find(0)) {
        g_mesh_node.app_key_add_flag = 0;
    } else {
        g_mesh_node.app_key_add_flag = 1;
    }

#ifdef BT_MESH_BEACON_RSSI_REPORT
    g_mesh_node.rssi_report_enable = param->rssi_report_enable;

    if (g_mesh_node.node_provisioned && g_mesh_node.rssi_report_enable) {
        init_rssi_report();
    }

#endif

    ble_mesh_model_set_cb(node_model_message_cb);

    extern int mesh_event_port_init();
    mesh_event_port_init();

    if (param->health_cb) {
        extern int health_srv_cb_register(struct bt_mesh_health_srv_cb * health_cb);

        if (health_srv_cb_register(&g_health_cb)) {
            LOGE(TAG, "register health srv cb failed");
        }

        g_mesh_node.health_cb.att_on      = param->health_cb->att_on;
        g_mesh_node.health_cb.att_off     = param->health_cb->att_off;
        g_mesh_node.health_cb.faults_test = param->health_cb->faults_test;
    }

#if defined(CONFIG_BT_MESH_MODEL_BLOB_CLI) && CONFIG_BT_MESH_MODEL_BLOB_CLI > 0
    extern int ble_mesh_blob_cli_init();
    ret = ble_mesh_blob_cli_init();
    if (ret) {
        LOGE(TAG, "Blob cli init failed %d", ret);
        return ret;
    }
#endif

    if (bt_mesh_is_provisioned()) {
        ret = ble_mesh_vendor_srv_model_version_report(CONFIG_BT_MESH_AUTOCONFIG_DST_PUB_ADDR, DEF_VERSION_REPORT_TTL);
        if (ret) {
            LOGE(TAG, "node version report failed %d", ret);
        }
    }

    g_mesh_node.user_model_cb = param->user_model_cb;
    g_mesh_node.init_flag     = 1;

    return g_mesh_node.node_provisioned;
}

int ble_mesh_node_reset()
{
    bt_mesh_reset();
#ifdef CONFIG_BT_MESH_PROVISIONER
    aos_kv_del("PROVISIONER_CONFIG");
    aos_kv_del("PROVISIONER_CONFIG_SET_FLAG");
    aos_kv_del("PROV_ENABLE_FLAG");
#endif
    aos_kv_del("AUTO_CONFIG");
    return 0;
}
