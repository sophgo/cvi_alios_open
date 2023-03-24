#include "jqutil_v2/jqutil.h"
// #include "utils/PathUtil.h"
#include "JSWifiImpl.h"

using namespace jqutil_v2;

namespace aiot {

class JSWifi: public JQPublishObject {
public:
    void OnCtor(JQFunctionInfo &info);
    // wifi 扫描&通知机制
    void scan(JQAsyncInfo &info);

    // wifi 配置增删改查接口
    void addConfig(JQAsyncInfo &info);
    void removeConfig(JQAsyncInfo &info);
    void changeConfig(JQAsyncInfo &info);
    void listConfig(JQAsyncInfo &info);

    // 发起连接和断开
    void connect(JQAsyncInfo &info);
    void disconnect(JQAsyncInfo &info);

private:
    JSWifiImpl _impl;
};

void JSWifi::OnCtor(JQFunctionInfo &info)
{
    _impl.init(this);
}

void JSWifi::scan(JQAsyncInfo &info)
{
    // 模拟通知 JS 空间扫描结果
    JQuick::sp<JSWifi> self = this;
    _impl.scan([info, self](std::vector<WifiApRecord> &recordList){
        Bson::array result;
        for (auto &record: recordList) {
            result.push_back(record.toBson());
        }
        self->publish("scan_result", result);
        info.post(result);
    });
}

void JSWifi::addConfig(JQAsyncInfo &info)
{
    if (!info[0].is_object()) {
        info.postError("first param should be object of WifiConfig");
        return;
    }

    std::string ssid = info[0]["ssid"].string_value();
    std::string psk = info[0]["psk"].string_value();
    if (ssid.empty()) {
        info.postError("ssid should not be empty");
        return;
    }

    _impl.addConfig(ssid, psk);
    info.post(0);
}

void JSWifi::removeConfig(JQAsyncInfo &info)
{
    std::string ssid = info[0].string_value();
    if (ssid.empty()) {
        info.postError("first arg should be ssid, but got empty");
        return;
    }

    _impl.removeConfig(ssid);
    info.post(0);
}

void JSWifi::changeConfig(JQAsyncInfo &info)
{
    if (!info[0].is_object()) {
        info.postError("first param should be object of WifiConfig");
        return;
    }

    std::string ssid = info[0]["ssid"].string_value();
    std::string psk = info[0]["psk"].string_value();
    if (ssid.empty()) {
        info.postError("ssid should not be empty");
        return;
    }

    _impl.changeConfig(ssid, psk);
    info.post(0);
}

void JSWifi::listConfig(JQAsyncInfo &info)
{
    std::vector<WifiConfig> configList = _impl.listConfig();

    Bson::array result;
    for (auto &item: configList) {
        Bson::object config;
        config["ssid"] = item.ssid;
        config["psk"] = item.psk;
        result.push_back(config);
    }

    info.post(result);
}

void JSWifi::connect(JQAsyncInfo &info)
{
    std::string ssid = info[0].string_value();
    if (ssid.empty()) {
        info.postError("first arg should be ssid, but got empty");
        return;
    }
    _impl.connect(ssid);
    info.post(0);
}

void JSWifi::disconnect(JQAsyncInfo &info)
{
    _impl.disconnect();
    info.post(0);
}

static JSValue createWifi(JQModuleEnv* env)
{
    JQFunctionTemplateRef tpl = JQFunctionTemplate::New(env, "wifi");

    // 设定 C++ 对象工厂函数
    tpl->InstanceTemplate()->setObjectCreator([]() {
      return new JSWifi();
    });
    tpl->SetProtoMethodPromise("scan", &JSWifi::scan);
    tpl->SetProtoMethodPromise("addConfig", &JSWifi::addConfig);
    tpl->SetProtoMethodPromise("removeConfig", &JSWifi::removeConfig);
    tpl->SetProtoMethodPromise("changeConfig", &JSWifi::changeConfig);
    tpl->SetProtoMethodPromise("listConfig", &JSWifi::listConfig);
    tpl->SetProtoMethodPromise("connect", &JSWifi::connect);
    tpl->SetProtoMethodPromise("disconnect", &JSWifi::disconnect);
    JQPublishObject::InitTpl(tpl);

    return tpl->CallConstructor();
}

// extern
JSValue init_aiot_wifi(JQModuleEnv* env)
{
    // 返回模块
    return createWifi(env);
}

}  // namespace aiot

