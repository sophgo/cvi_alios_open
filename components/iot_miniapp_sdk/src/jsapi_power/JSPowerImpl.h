#pragma once
#include <string>
#include "jqutil_v2/jqutil.h"

using namespace jqutil_v2;

namespace aiot{

static std::string POWER_EVENT_BATTERY_CHANGE ="BatteryChange";
static std::string POWER_EVENT_CHARGE ="Charge";
static std::string POWER_EVENT_BATTERY_LOW ="BatteryLow";
static std::string POWER_EVENT_BATTERY_EMERGENCY ="BatteryEmergency";

// power info
typedef struct PowerManageInfo{
    bool autoHibernate;
    int hibernateTimeout;
    int batteryPercent;
    bool isCharging;
    inline Bson toBson()
    {
        Bson::object result;
        result["autoHibernate"] = autoHibernate;
        result["hibernateTimeout"] = hibernateTimeout;
        result["batteryPercent"] = (int)batteryPercent;
        result["isCharging"] = (bool)isCharging;
        return result;
    }
} PowerManageInfo;

// battery info
typedef struct PowerBatteryInfo{
    std::string name;
    int battery;
    bool isCharging;
    inline Bson toBson()
    {
        Bson::object result;
        result["name"] = name;
        result["battery"] = (int)battery;
        result["isCharging"] = (bool)isCharging;
        return result;
    }
}PowerBatteryInfo;

class JSPowerImpl{
public:
    using PowerCallback = std::function<void(PowerManageInfo &recordInfo)>;

public:
    void init(JQuick::sp<JQPublishObject> pub);
    void getInfo(PowerCallback cb);
    void setAutoHibernate(bool isOn);
    void setHibernateTime(long time);
    void shutdown();
    void reboot();
private:
    JQuick::sp<JQPublishObject> _pub;
};
};

