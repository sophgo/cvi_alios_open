#include "JSWifiImpl.h"
#include <yoc/netmgr.h>
#include <yoc/netmgr_service.h>
#include <uservice/eventid.h>
#include <uservice/event.h>
#include <devices/wifi.h>

#define TAG "JS_WiFi"

static std::vector<aiot::WifiApRecord> records;

static int g_net_gotip = 0;
static netmgr_hdl_t app_netmgr_hdl = NULL;

/* app & cli & event */
void user_local_event_cb(uint32_t event_id, const void *param, void *context)
{
    if (event_id == EVENT_NETMGR_GOT_IP) {
        g_net_gotip = 1;
        // LOGI(TAG, "Got IP");
    } else if (event_id == EVENT_NETMGR_NET_DISCON) {
        g_net_gotip = 0;
        // LOGI(TAG, "Net down");
    } else {
        ;
    }    /*do exception process */
    // app_exception_event(event_id);
}


void app_network_init(void)
{
    // app_netmgr_hdl = netmgr_dev_wifi_init();

    // if (app_netmgr_hdl) {
    //     utask_t *task = utask_new("netmgr", 10 * 1024, QUEUE_MSG_COUNT, AOS_DEFAULT_APP_PRI);
    //     netmgr_service_init(task);
    //     netmgr_start(app_netmgr_hdl);
    // }

    event_subscribe(EVENT_NETMGR_GOT_IP, user_local_event_cb, NULL);
    event_subscribe(EVENT_NETMGR_NET_DISCON, user_local_event_cb, NULL);
    /* 使用系统事件的定时器 */
    // event_subscribe(EVENT_NTP_RETRY_TIMER, user_local_event_cb, NULL);
    // event_subscribe(EVENT_NET_NTP_SUCCESS, user_local_event_cb, NULL);
}

// static netmgr_hdl_t netmgr_hdl()
// {
//     if (!app_netmgr_hdl) {
//         app_network_init();
//     }
//     return app_netmgr_hdl;
// }

static void wifi_cb_sta_connect_fail(rvm_dev_t *dev, rvm_hal_wifi_err_code_t err, void *arg)
{
    LOGI(TAG, "%s\n", __FUNCTION__);
}

static void wifi_cb_status_change(rvm_dev_t *dev, rvm_hal_wifi_event_id_t stat, void *arg)
{
    LOGI(TAG, "%s\n", __FUNCTION__);
}

static void print_encrypt_mode(rvm_hal_wifi_encrypt_type_t encryptmode)
{
    switch (encryptmode) {
        case WIFI_ENC_TKIP:
            printf("TKIP");
            break;

        case WIFI_ENC_AES:
            printf("CCMP");
            break;

        case WIFI_ENC_TKIP_AES_MIX:
            printf("TKIP+CCMP");
            break;

        default:
            printf("ERR");
            break;
    }
}

static void wifi_cb_scan_compeleted(rvm_dev_t *dev, uint16_t number, rvm_hal_wifi_ap_record_t *ap_records)
{
    int i;

    printf("\nbssid / channel / signal dbm / flags / ssid\n\n");

    for (i = 0; i < number; i++) {
        rvm_hal_wifi_ap_record_t *ap_record = &ap_records[i];

        printf("%02x:%02x:%02x:%02x:%02x:%02x %02d %d ",
               ap_record->bssid[0], ap_record->bssid[1], ap_record->bssid[2],
               ap_record->bssid[3], ap_record->bssid[4], ap_record->bssid[5],
               ap_record->channel, ap_record->rssi);

        aiot::WifiApRecord record;
        if (ap_record->link_status == WIFI_STATUS_GOTIP) {
            record.status = aiot::WIFI_STATUS_COMPLETED;
        } else {
            record.status = aiot::WIFI_STATUS_DISCONNECTED;
        }
        record.bssid = jq_printf("%02x:%02x:%02x:%02x:%02x:%02x",
               ap_record->bssid[0], ap_record->bssid[1], ap_record->bssid[2],
               ap_record->bssid[3], ap_record->bssid[4], ap_record->bssid[5]);
        record.ssid = (const char*)ap_record->ssid;
        record.rssi = ap_record->rssi;
        record.isEncrypt = ap_record->authmode == WIFI_AUTH_OPEN;
        records.push_back(record);

        switch (ap_record->authmode) {
            case WIFI_AUTH_OPEN:
                printf("[OPEN]");
                break;

            case WIFI_AUTH_WEP:
                printf("[WEP]");
                break;

            case WIFI_AUTH_WPA_PSK:
                printf("[WPA-PSK-");
                print_encrypt_mode(ap_record->encryptmode);
                printf("]");
                break;

            case WIFI_AUTH_WPA2_PSK:
                printf("[WPA2-PSK-");
                print_encrypt_mode(ap_record->encryptmode);
                printf("]");
                break;

            case WIFI_AUTH_WPA_WPA2_PSK:
                printf("[WPA-PSK-");
                print_encrypt_mode(ap_record->encryptmode);
                printf("]");
                printf("[WPA2-PSK-");
                print_encrypt_mode(ap_record->encryptmode);
                printf("]");
                break;

            default:
                printf("[NOT SUPPORT]");
                break;
        }

        printf("    %s\n",  ap_record->ssid);
    }

}

static void wifi_cb_fatal_err(rvm_dev_t *dev, void *arg)
{

}

static rvm_hal_wifi_event_func evt_cb = {
    .sta_connect_fail = wifi_cb_sta_connect_fail,
    .status_change = wifi_cb_status_change,
    .scan_compeleted = wifi_cb_scan_compeleted,
    .fatal_err = wifi_cb_fatal_err
};

static void wifihal_scan()
{
    rvm_dev_t *dev = netmgr_get_dev(netmgr_get_handle("wifi"));

    wifi_scan_config_t scan_config;
    memset(scan_config.ssid, 0, sizeof(scan_config.ssid));
    memset(scan_config.bssid, 0, sizeof(scan_config.bssid));
    scan_config.channel = 0;
    scan_config.show_hidden = 0;
    scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    scan_config.scan_time.active.max = 0;
    scan_config.scan_time.active.min = 0;
    scan_config.scan_time.passive = 0;


    rvm_hal_wifi_install_event_cb(dev, &evt_cb);
    // LOGI(TAG, "wifi block scan");
    rvm_hal_wifi_start_scan(dev, &scan_config, 1);
    //LOGI(TAG, "wifi block scan done");

    aos_msleep(1000);
    // LOGI(TAG, "wifi not block scan");
    rvm_hal_wifi_start_scan(dev, &scan_config, 0);
    // LOGI(TAG, "wifi not block scan done");
}

namespace aiot {

void JSWifiImpl::init(JQuick::sp<JQPublishObject> pub)
{
    _pub = pub;

    // TODO: register WIFI_EVENT: disconnect\ completed
    WifiConnectInfo disconnectedInfo;  // TODO: fill info
    _pub->publish(WIFI_EVENT_DISCONNECTED, disconnectedInfo.toBson());

    WifiConnectInfo completedInfo; // TODO: fill info
    _pub->publish(WIFI_EVENT_COMPLETED, completedInfo.toBson());
}

void JSWifiImpl::scan(ScanCallback cb)
{
    wifi_scan_config_t scan_config;
    rvm_dev_t *dev = netmgr_get_dev(netmgr_get_handle("wifi"));

    memset(scan_config.ssid, 0, sizeof(scan_config.ssid));
    memset(scan_config.bssid, 0, sizeof(scan_config.bssid));
    scan_config.channel = 0;
    scan_config.show_hidden = 0;
    scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    scan_config.scan_time.active.max = 0;
    scan_config.scan_time.active.min = 0;
    scan_config.scan_time.passive = 0;


    rvm_hal_wifi_install_event_cb(dev, &evt_cb);
    // LOGI(TAG, "wifi block scan");
    rvm_hal_wifi_start_scan(dev, &scan_config, 1);

    cb(records);
    records.clear();
}

void JSWifiImpl::addConfig(const std::string &ssid, const std::string &psk)
{
    // TODO: add config with ssid and psk
}

void JSWifiImpl::removeConfig(const std::string &ssid)
{
    // TODO: remove config by ssid
}

void JSWifiImpl::changeConfig(const std::string &ssid, const std::string &psk)
{
    // TODO: change config with ssid and psk
}

std::vector<WifiConfig> JSWifiImpl::listConfig()
{
    // TODO: get config list
    std::vector<WifiConfig> configList;
    configList.push_back({.ssid="ssidabc", .psk="pass01"});
    return configList;
}

void JSWifiImpl::connect(const std::string &ssid)
{
    // TODO: connect by ssid
}

void JSWifiImpl::disconnect()
{
    // TODO: do disconnect current ssid connection
}

}  // namespace aiot
