#include "jqutil_v2/jqutil.h"
#include "utils/log.h"
#include "JSVoiceImpl.h"

using namespace jqutil_v2;
namespace aiot
{
class JSVoice : public JQPublishObject
{
public:
    void reqNLP(JQAsyncInfo &info);
    void reqTTS(JQAsyncInfo &info);
    void setMute(JQAsyncInfo &info);
    void getMute(JQAsyncInfo &info);
    void pushToTalk(JQAsyncInfo &info);
private:
    JSVoiceImpl _impl;
};

void JSVoice::reqNLP(JQAsyncInfo &info){
    int ret = _impl.reqNLP();
    info.post(ret);
}

void JSVoice::reqTTS(JQAsyncInfo &info){
    std::string text = info[0].string_value();
    if(text.empty()){
        info.postError("key should not be empty");
        info.post(-1);
        return;
    }
    int ret = _impl.reqTTS(text);
    info.post(ret);
}

void JSVoice::setMute(JQAsyncInfo &info){
    int status = info[0].int_value();
    int ret = _impl.setMute(status);
    info.post(ret);
}

void JSVoice::getMute(JQAsyncInfo &info){
    int status = _impl.getMute();
    info.post(status);
}

void JSVoice::pushToTalk(JQAsyncInfo &info){
    int ret = _impl.pushToTalk();
    info.post(ret);
}

static JSValue createJSVoice(JQModuleEnv* env)
{
    JQFunctionTemplateRef tpl = JQFunctionTemplate::New(env, "voice");

    // 设定 C++ 对象工厂函数
    tpl->InstanceTemplate()->setObjectCreator([]() {
        return new JSVoice();
    });
       tpl->SetProtoMethodPromise("reqNLP", &JSVoice::reqNLP);
       tpl->SetProtoMethodPromise("reqTTS", &JSVoice::reqTTS);
       tpl->SetProtoMethodPromise("setMute", &JSVoice::setMute);
       tpl->SetProtoMethodPromise("getMute", &JSVoice::getMute);
       tpl->SetProtoMethodPromise("pushToTalk", &JSVoice::pushToTalk);

    return tpl->CallConstructor();
}

// extern
JSValue init_aiot_voice(JQModuleEnv* env)
{
    // 返回模块
    return createJSVoice(env);
}
}  // namespace aiot
