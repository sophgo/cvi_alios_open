#pragma once
#include <string>
#include "jqutil_v2/jqutil.h"

using namespace jqutil_v2;

namespace aiot
{
static std::string SCREEN_EVENT_STATUS_CHANGE = "StatusChange";

// screen info
typedef struct ScreenManageInfo {
    bool isOn;
    bool isAutoBrightness;
    int brightness;
    int autoOffTimeout;
    bool isAutoOff;
    inline Bson toBson()
    {
        Bson::object result;
        result["isOn"] = isOn;
        result["isAutoBrightness"] = isAutoBrightness;
        result["brightness"] = (int)brightness;
        result["autoOffTimeout"] = (int)autoOffTimeout;
        result["isAutoOff"] = isAutoOff;
        return result;
    }
} ScreenManageInfo;

// screen info
typedef struct ScreenStatusInfo {
    bool isOn;
    inline Bson toBson()
    {
        Bson::object result;
        result["isOn"] = isOn;
        return result;
    }
} ScreenStatusInfo;

class JSScreenImpl
{
public:
    using ScreenCallback = std::function< void(ScreenManageInfo& recordInfo) >;

public:
    void init(JQuick::sp< JQPublishObject > pub);
    void getInfo(ScreenCallback cb);
    void turnOn();
    void turnOff();
    void setAutoBrightness(bool isAuto);
    void setBrightness(int percent);
    void setAutoOff(bool isAuto);
    void setAutoOffTimeout(long time);

private:
    JQuick::sp< JQPublishObject > _pub;
};
};  // namespace aiot
