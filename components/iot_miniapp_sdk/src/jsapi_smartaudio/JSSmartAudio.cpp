#include "jqutil_v2/jqutil.h"
#include "utils/log.h"
#include "JSSmartAudioImpl.h"

using namespace jqutil_v2;
namespace aiot
{
class JSSmartAudio : public JQPublishObject
{
public:
    void play(JQAsyncInfo &info);
    void stop(JQAsyncInfo &info);
    void pause(JQAsyncInfo &info);
    void resume(JQAsyncInfo &info);
    void mute(JQAsyncInfo &info);
    void unmute(JQAsyncInfo &info);
    void getVol(JQAsyncInfo &info);
    void setVol(JQAsyncInfo &info);
    void seek(JQAsyncInfo &info);
    void setSpeed(JQAsyncInfo &info);
    void getSpeed(JQAsyncInfo &info);
    void getStatus(JQAsyncInfo &info);
    void getCurTime(JQAsyncInfo &info);
    void getMediaInfo(JQAsyncInfo &info);
private:
    JSSmartAudioImpl _impl;
};

void JSSmartAudio::play(JQAsyncInfo &info){
    std::string url = info[0].string_value();
    int type = info[0].int_value();
    int ret = _impl.play(url, type);
    info.post(ret);
}

void JSSmartAudio::stop(JQAsyncInfo &info){
    int ret = _impl.stop();
    info.post(ret);
}

void JSSmartAudio::pause(JQAsyncInfo &info){
    int ret = _impl.pause();
    info.post(ret);
}

void JSSmartAudio::resume(JQAsyncInfo &info){
    int ret = _impl.resume();
    info.post(ret);
}

void JSSmartAudio::mute(JQAsyncInfo &info){
    int ret = _impl.mute();
    info.post(ret);
}

void JSSmartAudio::unmute(JQAsyncInfo &info){
    int ret = _impl.unmute();
    info.post(ret);
}

void JSSmartAudio::getVol(JQAsyncInfo &info){
    int ret = _impl.getVol();
    info.post(ret);
}
    
void JSSmartAudio::setVol(JQAsyncInfo &info){
    int vol = info[0].int_value();
    int ret = _impl.setVol(vol);
    info.post(ret);
}
    
void JSSmartAudio::seek(JQAsyncInfo &info){
    int ms = info[0].int_value();
    int ret = _impl.seek(ms);
    info.post(ret);
}
    
void JSSmartAudio::setSpeed(JQAsyncInfo &info){
    double speed = info[0].number_value();
    int ret = _impl.setSpeed(speed);
    info.post(ret);
}
    
void JSSmartAudio::getSpeed(JQAsyncInfo &info){
    double ret = _impl.getSpeed();
    info.post(ret);
}
    
void JSSmartAudio::getStatus(JQAsyncInfo &info){
    int ret = _impl.getStatus();
    info.post(ret);
}
    
void JSSmartAudio::getCurTime(JQAsyncInfo &info){
    int ret = _impl.getCurTime();
    info.post(ret);
}
    
void JSSmartAudio::getMediaInfo(JQAsyncInfo &info){
    SMTMediaInfo media_info;
    int ret = _impl.getMediaInfo(media_info);

    Bson::object obj;
    obj["url"] = media_info.url;
    obj["duration"] = media_info.duration;

    info.post(obj);
}

static JSValue createJSSmartAudio(JQModuleEnv* env)
{
    JQFunctionTemplateRef tpl = JQFunctionTemplate::New(env, "smartaudio");

    // 设定 C++ 对象工厂函数
    tpl->InstanceTemplate()->setObjectCreator([]() {
        return new JSSmartAudio();
    });

    tpl->SetProtoMethodPromise("play", &JSSmartAudio::play);
    tpl->SetProtoMethodPromise("stop", &JSSmartAudio::stop);
    tpl->SetProtoMethodPromise("pause", &JSSmartAudio::pause);
    tpl->SetProtoMethodPromise("resume", &JSSmartAudio::resume);
    tpl->SetProtoMethodPromise("mute", &JSSmartAudio::mute);
    tpl->SetProtoMethodPromise("unmute", &JSSmartAudio::unmute);
    tpl->SetProtoMethodPromise("getVol", &JSSmartAudio::getVol);
    tpl->SetProtoMethodPromise("setVol", &JSSmartAudio::setVol);
    tpl->SetProtoMethodPromise("seek", &JSSmartAudio::seek);
    tpl->SetProtoMethodPromise("setSpeed", &JSSmartAudio::setSpeed);
    tpl->SetProtoMethodPromise("getSpeed", &JSSmartAudio::getSpeed);
    tpl->SetProtoMethodPromise("getStatus", &JSSmartAudio::getStatus);
    tpl->SetProtoMethodPromise("getCurTime", &JSSmartAudio::getCurTime);
    tpl->SetProtoMethodPromise("getMediaInfo", &JSSmartAudio::getMediaInfo);
    
    return tpl->CallConstructor();
}

// extern
JSValue init_aiot_smartaudio(JQModuleEnv* env)
{
    // 返回模块
    return createJSSmartAudio(env);
}
}  // namespace aiot
