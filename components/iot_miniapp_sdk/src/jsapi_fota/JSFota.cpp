#include "jqutil_v2/jqutil.h"
#include "utils/log.h"
#include "JSFotaImpl.h"

using namespace jqutil_v2;
namespace aiot
{
class JSFota : public JQPublishObject
{
public:
    void start(JQAsyncInfo &info);
    void stop(JQAsyncInfo &info);
    void getState(JQAsyncInfo &info);
    void versionCheck(JQAsyncInfo &info);
    void download(JQAsyncInfo &info);
    void restart(JQAsyncInfo &info);

private:
    JSFotaImpl _impl;
};

void JSFota::start(JQAsyncInfo &info){
    int ret = _impl.start();
    info.post(ret);
}

void JSFota::stop(JQAsyncInfo &info){
    int ret = _impl.stop();
    info.post(ret);
}

void JSFota::getState(JQAsyncInfo &info){
    std::string state = _impl.getState();
    info.post(state);
}

void JSFota::versionCheck(JQAsyncInfo &info){
    int ret = _impl.versionCheck();
    info.post(ret);
}

void JSFota::download(JQAsyncInfo &info){
    int ret = _impl.download();
    info.post(ret);
}

void JSFota::restart(JQAsyncInfo &info){
    int ms = info[0].int_value();
    int ret = _impl.restart(ms);
    info.post(ret);
}

static JSValue createJSFota(JQModuleEnv* env)
{
    JQFunctionTemplateRef tpl = JQFunctionTemplate::New(env, "voice");

    // 设定 C++ 对象工厂函数
    tpl->InstanceTemplate()->setObjectCreator([]() {
        return new JSFota();
    });
       tpl->SetProtoMethodPromise("start", &JSFota::start);
       tpl->SetProtoMethodPromise("stop", &JSFota::stop);
       tpl->SetProtoMethodPromise("getState", &JSFota::getState);
       tpl->SetProtoMethodPromise("versionCheck", &JSFota::versionCheck);
       tpl->SetProtoMethodPromise("download", &JSFota::download);
       tpl->SetProtoMethodPromise("restart", &JSFota::restart);

    return tpl->CallConstructor();
}

// extern
JSValue init_aiot_fota(JQModuleEnv* env)
{
    // 返回模块
    return createJSFota(env);
}
}  // namespace aiot
