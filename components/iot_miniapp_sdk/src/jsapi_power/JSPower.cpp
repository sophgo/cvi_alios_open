#include "jqutil_v2/jqutil.h"
#include "JSPowerImpl.h"

using namespace jqutil_v2;
namespace aiot {
class JSPower: public JQPublishObject{
public:
    void OnCtor(JQFunctionInfo &info);
    void getInfo(JQAsyncInfo &info);
    void setAutoHibernate(JQAsyncInfo &info);
    void setHibernateTime(JQAsyncInfo &info);
    void shutdown(JQAsyncInfo &info);
    void reboot(JQAsyncInfo &info);
private:
    JSPowerImpl _impl;
};

void JSPower::OnCtor(JQFunctionInfo &info){
    _impl.init(this);
}
void JSPower::getInfo(JQAsyncInfo &info){
   _impl.getInfo([info](PowerManageInfo &record){
       info.post(record.toBson());
   });
}

void JSPower::setAutoHibernate(JQAsyncInfo &info){
  bool isOn = info[0].bool_value();
  _impl.setAutoHibernate(isOn);
  info.post(0);
}
void JSPower::setHibernateTime(JQAsyncInfo &info){
    long time = info[0].number_value();
    _impl.setHibernateTime(time);
    info.post(0);
}
void JSPower::shutdown(JQAsyncInfo &info){
    _impl.shutdown();
    info.post(0);
}
void JSPower::reboot(JQAsyncInfo &info){
    _impl.reboot();
    info.post(0);
}

static JSValue createPower(JQModuleEnv* env)
{
    JQFunctionTemplateRef tpl = JQFunctionTemplate::New(env, "power");

    // 设定 C++ 对象工厂函数
    tpl->InstanceTemplate()->setObjectCreator([]() {
        return new JSPower();
    });
    tpl->SetProtoMethodPromise("getInfo", &JSPower::getInfo);
    tpl->SetProtoMethodPromise("setAutoHibernate", &JSPower::setAutoHibernate);
    tpl->SetProtoMethodPromise("setHibernateTime", &JSPower::setHibernateTime);
    tpl->SetProtoMethodPromise("shutdown", &JSPower::shutdown);
    tpl->SetProtoMethodPromise("reboot", &JSPower::reboot);
    JQPublishObject::InitTpl(tpl);

    return tpl->CallConstructor();
}

// extern
JSValue init_aiot_power(JQModuleEnv* env)
{
    // 返回模块
    return createPower(env);
}
}