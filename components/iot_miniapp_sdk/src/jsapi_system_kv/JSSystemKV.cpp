#include "jqutil_v2/jqutil.h"
#include "utils/log.h"
#include "JSSystemKVImpl.h"

using namespace jqutil_v2;
namespace aiot
{
class JSSystemKV : public JQPublishObject
{
public:
    void setItem(JQAsyncInfo &info);
    void getItem(JQAsyncInfo &info);
    void removeItem(JQAsyncInfo &info);
    void clear(JQAsyncInfo &info);

private:
    JSSystemKVImpl _impl;
};

void JSSystemKV::setItem(JQAsyncInfo &info){
    std::string key = info[0].string_value();
    std::string value = info[1].string_value();
    if(key.empty()){
        info.postError("key should not be empty");
        return ;
    }
    if(value.empty()){
        info.postError("value should not be empty");
        return ;
    }
    _impl.setItem(key, value);
    info.post(0);
}

void JSSystemKV::getItem(JQAsyncInfo &info){
    std::string key = info[0].string_value();
    std::string defaultValue = info[1].string_value();
    if(key.empty()){
        info.postError("key should not be empty");
        return ;
    }
    std::string value = _impl.getItem(key, defaultValue);
    info.post(value);
}

void JSSystemKV::removeItem(JQAsyncInfo &info){
    std::string key = info[0].string_value();
    if(key.empty()){
        info.postError("key should not be empty");
        return ;
    }
    _impl.removeItem(key);
    info.post(0);
}

void JSSystemKV::clear(JQAsyncInfo &info){
    _impl.clear();
    info.post(0);
}

static JSValue createJSSystemKV(JQModuleEnv* env)
{
    JQFunctionTemplateRef tpl = JQFunctionTemplate::New(env, "system_kv");

    // 设定 C++ 对象工厂函数
    tpl->InstanceTemplate()->setObjectCreator([]() {
        return new JSSystemKV();
    });
       tpl->SetProtoMethodPromise("setItem", &JSSystemKV::setItem);
       tpl->SetProtoMethodPromise("getItem", &JSSystemKV::getItem);
       tpl->SetProtoMethodPromise("removeItem", &JSSystemKV::removeItem);
       tpl->SetProtoMethodPromise("clear", &JSSystemKV::clear);

    return tpl->CallConstructor();
}

// extern
JSValue init_aiot_system_kv(JQModuleEnv* env)
{
    // 返回模块
    return createJSSystemKV(env);
}
}  // namespace aiot
