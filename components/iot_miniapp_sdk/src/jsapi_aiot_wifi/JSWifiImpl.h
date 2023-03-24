#pragma once
#include "jqutil_v2/jqutil.h"
#include "utils/StrongPtr.h"
#include <vector>
#include <string>

using namespace jqutil_v2;

namespace aiot {

static std::string WIFI_STATUS_DISCONNECTED = "disconnected";
static std::string WIFI_STATUS_COMPLETED = "completed";
typedef struct WifiApRecord {
    std::string status;      /* current ap status */
    std::string bssid;           /* MAC address of AP */
    std::string ssid;            /* SSID of AP */
    int8_t  rssi;                /* signal strength of AP */
    bool isEncrypt;              /* whether has encrypt mode of AP */
    inline Bson toBson()
    {
        Bson::object result;
        result["status"] = status;
        result["bssid"] = bssid;
        result["ssid"] = ssid;
        result["rssi"] = (int)rssi;
        result["isEncrypt"] = isEncrypt;
        return result;
    }
} WifiApRecord;

// 全局监听wifi事件，并使用 JQPublisObject::publish 将结果抛送到JS层:
//  scan_result 对应 WifiApRecord 列表
//  disconnected/ completed 对应 WifiConnectInfo
static std::string WIFI_EVENT_SCAN_RESULT = "scan_result";
static std::string WIFI_EVENT_DISCONNECTED = "disconnected";
static std::string WIFI_EVENT_COMPLETED = "completed";

// 在 WIFI_EVENT_DISCONNECTED 事件中，reason 字段有如下两种情况
static std::string WIFI_DISCONNECT_REASON_CONNECT_FAILED = "connect_failed";
static std::string WIFI_DISCONNECT_REASON_AUTH_FAILED = "auth_failed";
typedef struct WifiConnectInfo {
    std::string reason;
    std::string bssid;           /* MAC address of AP */
    std::string ssid;            /* SSID of AP */
    inline Bson toBson()
    {
        Bson::object result;
        result["reason"] = reason;
        result["bssid"] = bssid;
        result["ssid"] = ssid;
        return result;
    }
} WifiConnectInfo;

// WIFI config 数据结构
typedef struct {
    std::string ssid;  // primary key of config
    std::string psk;
} WifiConfig;

class JSWifiImpl {
public:
    using ScanCallback = std::function<void(std::vector<WifiApRecord> &recordList)>;

public:
    void init(JQuick::sp<JQPublishObject> pub);

    void scan(ScanCallback cb);

    void addConfig(const std::string &ssid, const std::string &psk);
    void removeConfig(const std::string &ssid);
    void changeConfig(const std::string &ssid, const std::string &psk);
    std::vector<WifiConfig> listConfig();

    void connect(const std::string &ssid);
    void disconnect();

private:
    JQuick::sp<JQPublishObject> _pub;
};

}  // namespace aiot
